#include "wrassp.h"
#include <dataobj.h>
#include <asspfio.h>
#include <asspmess.h>


SEXP getDObj(SEXP fname) {

   SEXP ans, dPtr, class, rate, tracks, startTime, origRate;
     DOBJ DATA, * data = &(DATA);
     DDESC * desc = NULL;
     long numRecs;
     int i, n;
     // read the data
     data = asspFOpen(CHAR(STRING_ELT(fname, 0)), AFO_READ, (DOBJ*)NULL);
     if (data == NULL)
	  error(getAsspMsg(asspMsgNum));
	  //error(CHAR(STRING_ELT(fname,0)));
     allocDataBuf(data, data->numRecords);
     data->bufStartRec = data->startRecord;
     if ((numRecs = asspFFill(data)) < 0)
	  error(getAsspMsg(asspMsgNum));
     asspFClose(data, AFC_KEEP);
     // count tracks
     for (n=0, desc = &(data->ddl); desc !=NULL; desc=desc->next)
     {
	  n++;
	  //Rprintf("Cur n=%d\n", n);
     }
     
     // create result, a list with a matrix for each track
     PROTECT(ans = allocVector(VECSXP, n));
     // create list of tracks
     PROTECT(tracks = allocVector(STRSXP, n));
     for (i=0, desc = &(data->ddl); desc !=NULL; desc=desc->next, i++)
     {
     	  SET_STRING_ELT(tracks, i, mkChar(desc->ident));
	  // fill tracks with data
	  Rprintf("Loading track %s.\n", desc->ident);
	  SET_VECTOR_ELT(ans, i, getDObjTrackData(data, desc));
     }
     // set the names
     setAttrib(ans, R_NamesSymbol, tracks);
     dPtr = R_MakeExternalPtr(data, install("DOBJ"), install("something"));
     PROTECT(dPtr);
     R_RegisterCFinalizerEx(dPtr, DObjFinalizer, TRUE);
     setAttrib(ans, install("data pointer"), dPtr);
     PROTECT(rate = allocVector(REALSXP,1));
     REAL(rate)[0] = data->dataRate;
     setAttrib(ans, install("samplerate"), rate);
     PROTECT(origRate = allocVector(REALSXP,1));
     if (data->fileFormat == FF_SSFF) {
	REAL(origRate)[0] = data->sampFreq;
     } else {
	REAL(origRate)[0] = 0;
     }
     setAttrib(ans, install("origFreq"), origRate);
     PROTECT(startTime = allocVector(REALSXP, 1));
     REAL(startTime)[0] = data->Start_Time;
     setAttrib(ans, install("start_time"), startTime);
     PROTECT(class = allocVector(STRSXP, 1));
     SET_STRING_ELT(class, 0, mkChar("dobj"));
     classgets(ans, class);
     UNPROTECT(6);
     return ans;
}


static void DObjFinalizer(SEXP dPtr) {
     DOBJ * data = R_ExternalPtrAddr(dPtr);
     asspFClose(data, AFC_FREE);
     R_ClearExternalPtr(dPtr); /* not really needed */
}

SEXP getDObjTracks(SEXP dobj)
{
     SEXP ans, ptr;
     ptr = getAttrib(dobj, install("data pointer"));
     DOBJ * data = R_ExternalPtrAddr(ptr);
     DDESC * desc;
     int i=0, n=0;
     // count tracks
     for (desc = &(data->ddl); desc !=NULL; desc=desc->next)
     {
	  n++;
     }
     //Rprintf("Number of descs = %i.", n);
     // create result
     PROTECT(ans = allocVector(STRSXP, n));
     for (desc = &(data->ddl); desc !=NULL; desc=desc->next)
     {
     	  SET_STRING_ELT(ans, i, mkChar(desc->ident));
     	  i++;
     }
     /* for (; i<n; i++) */
     /* 	  SET_STRING_ELT(ans, i, mkChar("")); */
     UNPROTECT(1);
     return(ans);
}

SEXP getDObjTrackData(DOBJ * data, DDESC * desc)
{
     SEXP ans;
     void *tempBuffer, *bufPtr;
     int i, m, n;
     tempBuffer = malloc((size_t) data->recordSize);
     // various pointers for variuos data sizes
     uint8_t *u8Ptr;
     int8_t *i8Ptr;
     uint16_t *u16Ptr;
     int16_t *i16Ptr;
     uint32_t *u32Ptr;
     int32_t *i32Ptr;
     float *f32Ptr;
     double *f64Ptr;

     double * Rans;
     int * Ians;
     uint8_t *bPtr;
     bPtr = (uint8_t *) tempBuffer;
     i = 0;			//initial index in buffer

     switch (desc->format) {
     case DF_UINT8:
     case DF_INT8:
     case DF_UINT16:
     case DF_INT16:
     case DF_UINT32:
     case DF_INT32:
     {
     	  PROTECT(ans=allocMatrix(INTSXP, data->numRecords, desc->numFields));
     	  Ians = INTEGER(ans);
     }
     break;
     case DF_REAL32:
     case DF_REAL64:
     {
	  PROTECT(ans=allocMatrix(REALSXP, data->numRecords, desc->numFields));
	  Rans = REAL(ans);
     }
     break;
     default:
     {
	  error("Unsupported data format.");
	  free(tempBuffer);
     }
     break;
     }

     for (m = 0; m < data->numRecords; m++)
     {
	  bufPtr = data->dataBuffer + m * data->recordSize;
	  memcpy(tempBuffer, bufPtr, (size_t) data->recordSize);
	  switch (desc->format) {
	  case DF_UINT8:
	  {
	       u8Ptr = &bPtr[desc->offset];
	       for (n=0; n < desc->numFields; n++) {
		    Ians[m + n * data->numRecords] = (unsigned int) u8Ptr[n];
	       }
	  }
	  break;
	  case DF_INT8:
	  {
	       i8Ptr = (int8_t *) &bPtr[desc->offset];
	       for (n=0; n < desc->numFields; n++) {
		    Ians[m + n * data->numRecords] = (int) u8Ptr[n];
	       }
	  }
	  break;
	  case DF_UINT16:
	  {
	       u16Ptr = (uint16_t *) &bPtr[desc->offset];
	       for (n=0; n < desc->numFields; n++) {
		    Ians[m + n * data->numRecords] = (unsigned int) u16Ptr[n];
	       }
	  }
	  break;
	  case DF_INT16:
	  {
	       i16Ptr = (int16_t *) &bPtr[desc->offset];
	       for (n=0; n < desc->numFields; n++) {
		    Ians[m + n * data->numRecords] = (int) i16Ptr[n];
	       }
	  }
	  break;
	  case DF_UINT32:
	  {
	       u32Ptr = (uint32_t *) &bPtr[desc->offset];
	       for (n=0; n < desc->numFields; n++) {
		    Ians[m + n * data->numRecords] = (unsigned long) u32Ptr[n];
	       }
	  }
	  break;
	  case DF_INT32:
	  {
	       i32Ptr = (int32_t *) &bPtr[desc->offset];
	       for (n=0; n < desc->numFields; n++) {
		    Ians[m + n * data->numRecords] = (long) i32Ptr[n];
	       }
	  }
	  break;
	  case DF_REAL32:
	  {
	       f32Ptr = (float *) &bPtr[desc->offset];
	       for (n=0; n < desc->numFields; n++) {
		    Rans[m + n * data->numRecords] = (double) f32Ptr[n];
	       }
	  }
	  break;
	  case DF_REAL64:
	  {
	       f64Ptr = (double *) &bPtr[desc->offset];
	       for (n=0; n < desc->numFields; n++) {
		    Rans[m + n * data->numRecords] = (double) f64Ptr[n];
	       }
	  }
	  break;

	  }
     }
     free(tempBuffer);
     UNPROTECT(1);
     return(ans);
}
