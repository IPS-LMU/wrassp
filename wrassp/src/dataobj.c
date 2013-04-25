#include "wrassp.h"
#include <math.h>		// ceil, floor
#include <dataobj.h>
#include <asspfio.h>
#include <asspmess.h>
#include <headers.h> /* KDTAB */

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
      error ("%s (%s)", getAsspMsg (asspMsgNum), fName);
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

   numRecs = (long) (end - begin) + 1;
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
   SEXP ans, /* dPtr, */ class, rate, tracks, startTime, origRate, filePath,
      startRec, endRec, trackformats, finfo;
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
   // create list of tracks and formats
   PROTECT (tracks = allocVector (STRSXP, n));
   PROTECT (trackformats = allocVector (STRSXP, n));
   for (i = 0, desc = &(data->ddl); desc != NULL; desc = desc->next, i++)
   {
      SET_STRING_ELT (tracks, i, mkChar (desc->ident));
      SET_STRING_ELT (trackformats, i, 
		      mkChar (asspDF2ssffString(desc->format))); 
      // fill tracks with data
      // Rprintf ("Loading track %s.\n", desc->ident);
      SET_VECTOR_ELT (ans, i, getDObjTrackData (data, desc));
   }
   // set the names
   setAttrib (ans, R_NamesSymbol, tracks);
   setAttrib (ans, install ("trackformats"), trackformats);

   /* PROTECT (dPtr = R_MakeExternalPtr (data, install ("DOBJ"), */
   /* 				      install ("something"))); */
   /* R_RegisterCFinalizerEx (dPtr, DObjFinalizer, TRUE); */
   /* setAttrib (ans, install ("data pointer"), dPtr); */
   PROTECT (rate = allocVector (REALSXP, 1));
   REAL (rate)[0] = data->dataRate;
   setAttrib (ans, install ("samplerate"), rate);
   if (data->filePath == NULL || strlen(data->filePath) == 0)
      protect (filePath = R_NilValue);
   else
   {
      PROTECT (filePath = allocVector (STRSXP, 1));
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
   INTEGER (startRec)[0] = (int) (data->bufStartRec + 1);
   setAttrib (ans, install ("start_record"), startRec);
   PROTECT (endRec = allocVector (INTSXP, 1));
   INTEGER (endRec)[0] = (int) (data->bufStartRec + data->bufNumRecs);
   setAttrib (ans, install ("end_record"), endRec);

   PROTECT (class = allocVector (STRSXP, 1));
   SET_STRING_ELT (class, 0, mkChar (WRASSP_CLASS));
   classgets (ans, class);

   PROTECT (finfo = allocVector (INTSXP, 2));
   INTEGER (finfo)[0] = (int) data->fileFormat;
   INTEGER (finfo)[1] = (int) data->fileData;
   setAttrib (ans, install ("file_info"), finfo);

   UNPROTECT (11);
   asspFClose (data, AFC_FREE);
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
	       "I am sorry, I should not be here and I don't know what to do.");
	 break;
      }
   }
   free (tempBuffer);
   UNPROTECT (1);
   return (ans);
}

/* switch trough assp data formats and return corresponding ssff string */
char * 
asspDF2ssffString(int df)
{
   switch ((dform_e) df)
   {
   case DF_BIT: 
      return "BIT";
      break;
   case DF_STR: 
      return "STR";
      break;
   case  DF_CHAR: 
      return "CHAR";
      break;
   case  DF_UINT8: 
      return "UINT8";
      break;
   case  DF_INT8: 
      return "INT8";
      break;
   case  DF_UINT16: 
      return "UINT16";
      break;
   case  DF_INT16: 
      return "INT16";
      break;
   case  DF_UINT24: 
      return "UINT24";
      break;
   case  DF_INT24: 
      return "INT24";
      break;
   case  DF_UINT32: 
      return "UINT32";
      break;
   case  DF_INT32: 
      return "INT32";
      break;
   case  DF_UINT64: 
      return "UINT64";
      break;
   case  DF_INT64: 
      return "INT64";
      break;
   case  DF_REAL32: 
      return "REAL32";
      break;
   case  DF_REAL64: 
      return "REAL64";
      break;
   default:
      return NULL;
   }
}

DOBJ* sexp2dobj(SEXP rdobj) 
{ 
   DOBJ* dop = NULL; 
   DDESC * desc = NULL;
   int FIRST = 1, i = 0, myBool = 0;
   size_t numFields = -1;
   SEXP attr, tracks, formats, track;
   KDTAB * entry;
   char * format;
   
   // check for right class
   attr = getAttrib (rdobj, R_ClassSymbol);
   for (i = 0; i < LENGTH(attr); i++) 
   {
      if (strcmp (CHAR (STRING_ELT (attr, i)) , WRASSP_CLASS)==0) {
	 myBool = 1;
	 break;
      }	 
   }
   if (!myBool) // classname does not match
   {
      error("Argument must be of class %s", WRASSP_CLASS);
   }

   // create DObj
   dop = allocDObj ();
   desc = &(dop->ddl);
   if (dop == NULL) {
      error (getAsspMsg (asspMsgNum));
   }
   
   // assign attributes
   attr = getAttrib (rdobj, install ("samplerate"));
   if (isNull (attr))
   {
      freeDObj (dop);
      error("Invalid argument: no 'samplerate' attribute.");
   }
   dop->dataRate = REAL (attr)[0];

   attr = getAttrib (rdobj, install ("origFreq"));
   if (!isNull (attr))
      dop->sampFreq = REAL (attr)[0];

   attr = getAttrib (rdobj, install ("start_time"));
   if (!isNull (attr))
      dop->Start_Time = REAL (attr)[0];

   attr = getAttrib (rdobj, install ("start_record"));
   if (!isNull (attr))
      dop->startRecord = INTEGER (attr)[0];

   attr = getAttrib(rdobj, install ("file_info"));
   if (LENGTH (attr) != 2)
   {
      dop->fileFormat = FF_SSFF;
      dop->fileData = FDF_BIN;
      warning ("Incomplete 'file_info' attribute. Writing to binary" 
	       "SSFF format (dafault).");
   }
   dop->fileFormat = (fform_e) INTEGER (attr)[0];
   dop->fileData = (fdata_e) INTEGER (attr)[1];

   //
   // prepare ddescs
   //
   // check tracks and formats and are there enough data parts
   if ( isNull (tracks = getAttrib (rdobj, R_NamesSymbol)) ||
	LENGTH (tracks) == 0 ||
	TYPEOF (tracks) != STRSXP)
   {
      freeDObj (dop);
      error ("There are no data tracks!");
   }

   if (isNull (formats = getAttrib (rdobj, install ("trackformats"))) ||
       TYPEOF (tracks) != STRSXP)
   {
      freeDObj (dop);
      error ("There are no track format specifiers!");
   }
   if (LENGTH (tracks) > LENGTH (formats))
   {
      freeDObj (dop);
      error("Not enough format specifiers for the data tracks.");
   }

   for (i = 0; i < LENGTH (tracks); i++)
   {
      // get dimensions
      track = VECTOR_ELT (rdobj, i);
      attr = getAttrib (track, R_DimSymbol);
      // if there is more than one track, add descriptor
      if (FIRST)
      {
	 dop->numRecords = INTEGER (attr)[0];
	 FIRST = 0;
      }
      else {
	 desc = addDDesc (dop);
	 if (desc == NULL) 
	 {
	    freeDObj (dop);
	    error (getAsspMsg (asspMsgNum));
	 }
	 if (dop->numRecords != INTEGER(attr)[0])
	 {
	    freeDObj (dop);
	    error ("Dimensions of tracks do not match." 
		   "(%d rows in first track, but %d rows in track %d).",
		   dop->numRecords, INTEGER (attr)[0], i);
	 }
      }
      desc->ident = strdup(CHAR (STRING_ELT (tracks, i)));
      format = strdup(CHAR (STRING_ELT (formats, i)));
      entry = keyword2entry (desc->ident, KDT_SSFF); /* search SSFF info */
      if (entry != NULL)
      {
	 desc->type = entry->dataType;
	 if (entry->factor != NULL)
	    strcpy(desc->factor, entry->factor);
	 if(entry->unit != NULL)
	    strcpy(desc->unit, entry->unit);
      }
      if (strcmp (format, "BIT") == 0)
      {
	 desc->format = DF_BIT;
	 desc->numBits = 1;
      }
      else if (strcmp (format, "STR") == 0)
      {
	 desc->format = DF_STR;
	 desc->numBits = 1;
      }
      else if (strcmp (format, "CHAR") == 0)
      {
	 desc->format = DF_CHAR;
	 desc->numBits = 8;
      }
      else if (strcmp (format, "UINT8") == 0)
      {
	 desc->format = DF_UINT8;
	 desc->numBits = 8;
      }
      else if (strcmp (format, "INT8") == 0)
      {
	 desc->format = DF_INT8;
	 desc->numBits = 8;
      }
      else if (strcmp (format, "UINT16") == 0)
      {
	 desc->format = DF_UINT16;
	 desc->numBits = 16;
      }
      else if (strcmp (format, "INT16") == 0)
      {
	 desc->format = DF_INT16;
	 desc->numBits = 16;
      }
      else if (strcmp (format, "UINT32") == 0)
      {
	 desc->format = DF_UINT32;
	 desc->numBits = 32;
      }
      else if (strcmp (format, "INT32") == 0)
      {
	 desc->format = DF_INT32;
	 desc->numBits = 32;
      }
      else if (strcmp (format, "UINT64") == 0)
      {
	 desc->format = DF_UINT64;
	 desc->numBits = 64;
      }
      else if (strcmp (format, "INT64") == 0)
      {
	 desc->format = DF_INT64;
	 desc->numBits = 64;
      }
      else if (strcmp (format, "REAL32") == 0)
      {
	 desc->format = DF_REAL32;
	 desc->numBits = 32;
      }
      else if (strcmp (format, "REAL64") == 0)
      {
	 desc->format = DF_REAL64;
	 desc->numBits = 64;
      }
      else {
	 freeDObj(dop);
	 error("Cannot handle data format %s.", format);
      }
      
      desc->coding = DC_LIN;
      desc->numFields = (size_t) INTEGER (attr)[1];
   }
   setRecordSize (dop);
   dop->frameDur = -1;
   checkRates (dop);
   allocDataBuf (dop, dop->numRecords);
   if (dop->dataBuffer == NULL)
   {
      freeDObj (dop);
      error (getAsspMsg (asspMsgNum));
   }
   
   for (i = 0, desc = &(dop->ddl); i < LENGTH (tracks); i++, desc = desc->next)
   {
      track = VECTOR_ELT (rdobj, i);
      if (!addTrackData (dop, desc, track))
      {
	 freeDObj(dop);
	 error("Adding Trackdata did not work...");
      }
   }   
   dop->bufNumRecs = dop->numRecords;
   return dop;
}



SEXP
writeDObj( SEXP data , SEXP fname)
 {
    DOBJ * dop = sexp2dobj(data);
    dop = asspFOpen (strdup (CHAR (STRING_ELT (fname, 0))),
		     AFO_WRITE, dop);
    if (dop == NULL)
    {
       freeDObj(dop);
       error (getAsspMsg (asspMsgNum));
    }
    asspFWrite(dop->dataBuffer, dop->bufNumRecs, dop);
    asspFClose(dop, AFC_FREE);
    return R_NilValue;
 }


   
	
int
addTrackData (DOBJ * dop, DDESC * ddl, SEXP rdobj)
{
   void *bufPtr;
   int i, m, n, unp=0;
   // various pointers for variuos data sizes
   uint8_t *u8Ptr;
   int8_t *i8Ptr;
   uint16_t *u16Ptr;
   int16_t *i16Ptr;
   uint32_t *u32Ptr;
   int32_t *i32Ptr;
   float *f32Ptr;
   double *f64Ptr;

   SEXP numMat;
   double *numPtr;
   uint8_t *bPtr;
   
   if (isReal (rdobj))
      numMat = rdobj;
   else if (isInteger (rdobj))
   {
      PROTECT (numMat = coerceVector (rdobj, REALSXP));
      unp++;
   }
   else 
      error ("Bad data type, must be INTEGER or REAL.");
   numPtr = REAL (numMat);

   i = 0;			//initial index in buffer

   for (m = 0; m < dop->numRecords; m++)
   {
      bufPtr = dop->dataBuffer + m * dop->recordSize;
      bPtr = (uint8_t *) bufPtr;
      switch (ddl->format)
      {
      case DF_UINT8:
      {
	 u8Ptr = &bPtr[ddl->offset];
	 for (n = 0; n < ddl->numFields; n++)
	 {
	    u8Ptr[n] = (uint8_t) numPtr[m + n * dop->numRecords];
	 }
      }
      break;
      case DF_INT8:
      {
	 i8Ptr = (int8_t *) & bPtr[ddl->offset];
	 for (n = 0; n < ddl->numFields; n++)
	 {
	    u8Ptr[n]= (int8_t) numPtr[m + n * dop->numRecords];
	 }
      }
      break;
      case DF_UINT16:
      {
	 u16Ptr = (uint16_t *) & bPtr[ddl->offset];
	 for (n = 0; n < ddl->numFields; n++)
	 {
	    u16Ptr[n] = (uint16_t) numPtr[m + n * dop->numRecords];
	 }
      }
      break;
      case DF_INT16:
      {
	 i16Ptr = (int16_t *) & bPtr[ddl->offset];
	 for (n = 0; n < ddl->numFields; n++)
	 {
	    i16Ptr[n] = (int16_t) numPtr[m + n * dop->numRecords] ;
	 }
      }
      break;
      case DF_UINT32:
      {
	 u32Ptr = (uint32_t *) & bPtr[ddl->offset];
	 for (n = 0; n < ddl->numFields; n++)
	 {
	    u32Ptr[n] = (uint32_t) numPtr[m + n * dop->numRecords];
	 }
      }
      break;
      case DF_INT32:
      {
	 i32Ptr = (int32_t *) & bPtr[ddl->offset];
	 for (n = 0; n < ddl->numFields; n++)
	 {
	    i32Ptr[n] = (int32_t) numPtr[m + n * dop->numRecords];
	 }
      }
      break;
      case DF_REAL32:
      {
	 f32Ptr = (float *) &bPtr[ddl->offset];
	 for (n = 0; n < ddl->numFields; n++)
	 {
	    f32Ptr[n] = (float) numPtr[m + n * dop->numRecords];
	 }
      }
      break;
      case DF_REAL64:
      {
	 f64Ptr = (double *) &bPtr[ddl->offset];
	 for (n = 0; n < ddl->numFields; n++)
	 {
	    f64Ptr[n] = (double) numPtr[m + n * dop->numRecords];
	 }
      }
      break;
      default:
	 error("Hi, I just landed in the default of a switch in dataobj.c."
	       "I am sorry, I should not be here and I don't know what to do.");
	 break;
      }
   }
   
   UNPROTECT (unp);
   return 1;
}
