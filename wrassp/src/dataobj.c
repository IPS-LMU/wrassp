#include "wrassp.h"
#include <math.h>		// ceil, floor
#include <dataobj.h>
#include <asspfio.h>
#include <asspmess.h>

#include <R_ext/PrtUtil.h>

SEXP
getDObj (SEXP fname)
{

  DOBJ *data = NULL;
  long numRecs;
  // read the data
  data =
    asspFOpen (strdup (CHAR (STRING_ELT (fname, 0))), AFO_READ,
	       (DOBJ *) NULL);
  if (data == NULL)
    error (getAsspMsg (asspMsgNum));
  //error(CHAR(STRING_ELT(fname,0)));
  allocDataBuf (data, data->numRecords);
  data->bufStartRec = data->startRecord;
  if ((numRecs = asspFFill (data)) < 0)
    error (getAsspMsg (asspMsgNum));
  asspFClose (data, AFC_KEEP);
  return dobj2AsspDataObj (data);
}


SEXP
getDObj2 (SEXP args)
{

  SEXP el;
  DOBJ *data = NULL;
  long numRecs;
  int i;
  char *fName = NULL;
  const char *name;
  double begin = 0, end = 0;
  int isSample = 0;

  // parse args
  args = CDR (args);		// skip name of function
  el = CAR (args);
  fName = strdup (CHAR (STRING_ELT (el, 0)));

  args = CDR (args);
  for (i = 0; args != R_NilValue; i++, args = CDR (args))
    {
      name = isNull (TAG (args)) ? "" : CHAR (PRINTNAME (TAG (args)));
      el = CAR (args);
      if (strcmp (name, "begin") == 0)
	{
	  begin = REAL (el)[0];
	  if (begin < 0)
	    begin = 0;
	}
      else if (strcmp (name, "end") == 0)
	{
	  end = REAL (el)[0];
	  if (end < 0)
	    end = 0;
	}
      else if (strcmp (name, "samples") == 0)
	{
	  isSample = INTEGER (el)[0];
	}
      else
	{
	  error ("Bad option '%s'.", name);
	}
    }

  if (end < begin && end > 0)
    error ("End before begin. That's not clever, dude!");

  // open the file
  data = asspFOpen (fName, AFO_READ, (DOBJ *) NULL);
  if (data == NULL)
    error (getAsspMsg (asspMsgNum));
  // figure out timing
  if (isSample)
    {
      if (end == 0)
	end = data->startRecord + data->numRecords - 1;
    }
  else
    {
      begin = ceil (begin * data->dataRate) + data->startRecord;
      if (end == 0)
	end = data->startRecord + data->numRecords - 1;
      else
	end = floor (end * data->dataRate) + data->startRecord;
    }
  if (end > (data->startRecord + data->numRecords))
    end = data->startRecord + data->numRecords - 1;
  if (begin > (data->startRecord + data->numRecords))
    {
      asspFClose (data, AFC_FREE);
      error ("Begin after end of data. That's not clever, dude!");
    }

  numRecs = (long) (end - begin);
  // read the data
  allocDataBuf (data, numRecs);
  data->bufStartRec = (long) begin;
  if ((numRecs = asspFFill (data)) < 0)
    {
      asspFClose (data, AFC_FREE);
      error (getAsspMsg (asspMsgNum));
    }
  asspFClose (data, AFC_KEEP);
  return dobj2AsspDataObj (data);
}


static void
DObjFinalizer (SEXP dPtr)
{
  DOBJ *data = R_ExternalPtrAddr (dPtr);
  asspFClose (data, AFC_FREE);
  R_ClearExternalPtr (dPtr);	/* not really needed */
}

SEXP
dobj2AsspDataObj (DOBJ * data)
{
  SEXP ans, dPtr, class, rate, tracks, startTime, origRate, filePath,
    startRec, endRec;
  DDESC *desc = NULL;
  int i, n;

  // count tracks
  for (n = 0, desc = &(data->ddl); desc != NULL; desc = desc->next)
    {
      n++;
      //Rprintf("Cur n=%d\n", n);
    }

  // create result, a list with a matrix for each track
  PROTECT (ans = allocVector (VECSXP, n));
  // create list of tracks
  PROTECT (tracks = allocVector (STRSXP, n));
  for (i = 0, desc = &(data->ddl); desc != NULL; desc = desc->next, i++)
    {
      SET_STRING_ELT (tracks, i, mkChar (desc->ident));
      // fill tracks with data
      Rprintf ("Loading track %s.\n", desc->ident);
      SET_VECTOR_ELT (ans, i, getDObjTrackData (data, desc));
    }
  // set the names
  setAttrib (ans, R_NamesSymbol, tracks);
  PROTECT (dPtr = R_MakeExternalPtr (data, install ("DOBJ"), 
				    install ("something")));
  R_RegisterCFinalizerEx (dPtr, DObjFinalizer, TRUE);
  setAttrib (ans, install ("data pointer"), dPtr);
  PROTECT (rate = allocVector (REALSXP, 1));
  REAL (rate)[0] = data->dataRate;
  setAttrib (ans, install ("samplerate"), rate);
  if (data->filePath == NULL || strlen(data->filePath) == 0)
    protect (filePath = R_NilValue);
  else
    {
      protect (filePath = allocVector (STRSXP, 1));
      SET_STRING_ELT (filePath, 0, mkChar (data->filePath));
    }
  setAttrib (ans, install ("filePath"), filePath);
  PROTECT (origRate = allocVector (REALSXP, 1));
  if (data->fileFormat == FF_SSFF)
    {
      REAL (origRate)[0] = data->sampFreq;
    }
  else
    {
      REAL (origRate)[0] = 0;
    }
  setAttrib (ans, install ("origFreq"), origRate);
  PROTECT (startTime = allocVector (REALSXP, 1));
  REAL (startTime)[0] = data->Start_Time + (data->bufStartRec / data->dataRate);
  setAttrib (ans, install ("start_time"), startTime);

  PROTECT (startRec = allocVector (INTSXP, 1));
  INTEGER (startRec)[0] = (int) data->bufStartRec + 1;
  setAttrib (ans, install ("start_record"), startRec);
  PROTECT (endRec = allocVector (INTSXP, 1));
  INTEGER (endRec)[0] = (int) data->bufStartRec + data->bufNumRecs;
  setAttrib (ans, install ("end_record"), endRec);

  PROTECT (class = allocVector (STRSXP, 1));
  SET_STRING_ELT (class, 0, mkChar ("AsspDataObj"));
  classgets (ans, class);

  UNPROTECT (10);
  return ans;

}


SEXP
getDObjTracks (SEXP dobj)
{
  SEXP ans, ptr;
  ptr = getAttrib (dobj, install ("data pointer"));
  DOBJ *data = R_ExternalPtrAddr (ptr);
  DDESC *desc;
  int i = 0, n = 0;
  // count tracks
  for (desc = &(data->ddl); desc != NULL; desc = desc->next)
    {
      n++;
    }
  //Rprintf("Number of descs = %i.", n);
  // create result
  PROTECT (ans = allocVector (STRSXP, n));
  for (desc = &(data->ddl); desc != NULL; desc = desc->next)
    {
      SET_STRING_ELT (ans, i, mkChar (desc->ident));
      i++;
    }
  /* for (; i<n; i++) */
  /*           SET_STRING_ELT(ans, i, mkChar("")); */
  UNPROTECT (1);
  return (ans);
}

SEXP
getDObjTrackData (DOBJ * data, DDESC * desc)
{
  SEXP ans;
  void *tempBuffer, *bufPtr;
  int i, m, n;
  tempBuffer = malloc ((size_t) data->recordSize);
  // various pointers for variuos data sizes
  uint8_t *u8Ptr;
  int8_t *i8Ptr;
  uint16_t *u16Ptr;
  int16_t *i16Ptr;
  uint32_t *u32Ptr;
  int32_t *i32Ptr;
  float *f32Ptr;
  double *f64Ptr;

  double *Rans;
  int *Ians;
  uint8_t *bPtr;
  bPtr = (uint8_t *) tempBuffer;
  i = 0;			//initial index in buffer

  switch (desc->format)
    {
    case DF_UINT8:
    case DF_INT8:
    case DF_UINT16:
    case DF_INT16:
    case DF_UINT32:
    case DF_INT32:
      {
	PROTECT (ans =
		 allocMatrix (INTSXP, data->bufNumRecs, desc->numFields));
	Ians = INTEGER (ans);
      }
      break;
    case DF_REAL32:
    case DF_REAL64:
      {
	PROTECT (ans =
		 allocMatrix (REALSXP, data->bufNumRecs, desc->numFields));
	Rans = REAL (ans);
      }
      break;
    default:
      {
	error ("Unsupported data format.");
	free (tempBuffer);
      }
      break;
    }

  for (m = 0; m < data->bufNumRecs; m++)
    {
      bufPtr = data->dataBuffer + m * data->recordSize;
      memcpy (tempBuffer, bufPtr, (size_t) data->recordSize);
      switch (desc->format)
	{
	case DF_UINT8:
	  {
	    u8Ptr = &bPtr[desc->offset];
	    for (n = 0; n < desc->numFields; n++)
	      {
		Ians[m + n * data->bufNumRecs] = (unsigned int) u8Ptr[n];
	      }
	  }
	  break;
	case DF_INT8:
	  {
	    i8Ptr = (int8_t *) & bPtr[desc->offset];
	    for (n = 0; n < desc->numFields; n++)
	      {
		Ians[m + n * data->bufNumRecs] = (int) u8Ptr[n];
	      }
	  }
	  break;
	case DF_UINT16:
	  {
	    u16Ptr = (uint16_t *) & bPtr[desc->offset];
	    for (n = 0; n < desc->numFields; n++)
	      {
		Ians[m + n * data->bufNumRecs] = (unsigned int) u16Ptr[n];
	      }
	  }
	  break;
	case DF_INT16:
	  {
	    i16Ptr = (int16_t *) & bPtr[desc->offset];
	    for (n = 0; n < desc->numFields; n++)
	      {
		Ians[m + n * data->bufNumRecs] = (int) i16Ptr[n];
	      }
	  }
	  break;
	case DF_UINT32:
	  {
	    u32Ptr = (uint32_t *) & bPtr[desc->offset];
	    for (n = 0; n < desc->numFields; n++)
	      {
		Ians[m + n * data->bufNumRecs] = (unsigned long) u32Ptr[n];
	      }
	  }
	  break;
	case DF_INT32:
	  {
	    i32Ptr = (int32_t *) & bPtr[desc->offset];
	    for (n = 0; n < desc->numFields; n++)
	      {
		Ians[m + n * data->bufNumRecs] = (long) i32Ptr[n];
	      }
	  }
	  break;
	case DF_REAL32:
	  {
	    f32Ptr = (float *) &bPtr[desc->offset];
	    for (n = 0; n < desc->numFields; n++)
	      {
		Rans[m + n * data->bufNumRecs] = (double) f32Ptr[n];
	      }
	  }
	  break;
	case DF_REAL64:
	  {
	    f64Ptr = (double *) &bPtr[desc->offset];
	    for (n = 0; n < desc->numFields; n++)
	      {
		Rans[m + n * data->bufNumRecs] = (double) f64Ptr[n];
	      }
	  }
	  break;
	default:
	  error("Hi, I just landed in the default of a switch in dataobj.c."
		"I am sorry, I should be here and I don't know what to do.");
	  break;
	}
    }
  free (tempBuffer);
  UNPROTECT (1);
  return (ans);
}
