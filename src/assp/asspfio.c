/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 2010  Michel Scheffers                          *
*                            IPdS, CAU Kiel                            *
*                            Leibnizstr. 10                            *
*                            24118 Kiel                                *
*                            Germany                                   *
*                            ms@ipds.uni-kiel.de                       *
*                                                                      *
* This library is free software: you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation, either version 3 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
* This library is distributed in the hope that it will be useful,      *
* but WITHOUT ANY WARRANTY; without even the implied warranty of       *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
* GNU General Public License for more details.                         *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this library. If not, see <http://www.gnu.org/licenses/>. *
*                                                                      *
*----------------------------------------------------------------------*
*                                                                      *
* File:     asspfio.c                                                  *
* Contents: Functions for handling speech/audio related data files.    *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: asspfio.c,v 1.8 2010/02/01 09:59:59 mtms Exp $ */

#include <stdio.h>      /* FILE NULL fopen() ... stdin stdout stderr */
#include <stddef.h>     /* size_t */
#include <stdlib.h>     /* malloc() calloc() */
#include <string.h>     /* str...() memset() */

#include <miscdefs.h>   /* TRUE FALSE NATIVE_EOL */
#include <trace.h>      /* TRACE[] */
#include <asspendian.h> /* byte-order stuff */
#include <asspmess.h>   /* message codes; reference to globals */
#include <assptime.h>   /* conversion macros */
#include <asspfio.h>    /* constants and prototypes */
#include <dataobj.h>    /* data object definitions and handler */
#include <headers.h>    /* header definitions and handler */

/* OS check for printing %llu and %lli*/
#ifdef __unix__

#elif defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) 
  #define OS_Windows
#endif

/*DOC

Function 'asspFOpen'

Opens the data file "filePath" in mode "mode" and returns a pointer to 
a data object structure or NULL upon error. Use "asspWarning" to check 
for warnings.
"mode" should be set to one of the following:
  AFO_READ    open existing file for reading
  AFO_WRITE   open/create file for writing; truncate an existing file
  AFO_UPDATE  open existing file for reading and writing
Files are normally opened in binary mode. Add the flag AFO_TEXT if you 
wish open/create a text file.

If "doPtr" equals NULL, the data object will be allocated memory; use 
asspFClose() or freeDObj() to return it. If the file is opened in 
AFO_READ or AFO_UPDATE mode, the header of the file will be read and 
the data object will be filled out. If the file could be opened but its 
format could not be established it will be closed again. Upon failure 
you should therefore check "asspMsgNum" and, if appropriate, use other 
means to open the file. Note that in that case you will have to FULLY 
fill out the data object by hand.
If a call in AFO_WRITE mode is successful, the file will be empty and 
only the items 'filePath', 'fp' and 'openMode' will be set in the data 
object.

If "doPtr" does not equal NULL, the actions in AFO_READ and AFO_UPDATE 
mode depend on the settings of 'fileFormat' in that data object. 
If set to 'FF_UNDEF', actions are as above, except that "doPtr" will be 
returned upon success, not a pointer to a newly allocated object. 
If it is set to 'FF_RAW', the values in the object will remain unchanged 
except for 'numRecords',  which - if possible - will be established. 
For all other values of 'fileFormat' the function will verify whether 
the file is in that format. If that is the case, the data object will be 
filled out. If not, you should use clearDObj() to re-initialize the data 
object and return any memory that may have been allocated.
A call in AFO_WRITE mode will in this case result in also writing the 
header corresponding to the specified format to the output file. Some 
items in the object - like 'fileEndian' and 'headerSize' - may hereby be 
adjusted. 
Beware that not all formats have a fixed header size: it may thus not be 
possible to change the header once data have been written to the file. 

DOC*/

DOBJ *asspFOpen(char *filePath, int mode, DOBJ *doPtr)
{
  char  Cmode[8];
  int   err=0;
  int   CLEAR=FALSE;
  DOBJ *dop=NULL;

  if(filePath == NULL || !(mode & AFO_READ || mode & AFO_WRITE)) {
    setAsspMsg(AEB_BAD_ARGS, "asspFOpen");
    return(NULL);
  }
  if(strlen(filePath) == 0) {
    setAsspMsg(AEB_BAD_ARGS, "asspFOpen");
    return(NULL);
  }
  if(doPtr != NULL) {
    if(doPtr->fp != NULL) {
      setAsspMsg(AEB_BAD_CALL, "asspFOpen");
      return(NULL);
    }
    if(mode & AFO_WRITE && !(mode & AFO_READ)) {
      if(doPtr->fileFormat <= FF_UNDEF ||
	 doPtr->fileFormat >= NUM_FILE_FORMATS) {
	setAsspMsg(AEB_BAD_ARGS, "asspFOpen");
	return(NULL);
      }
    }
    else {
      if(doPtr->fileFormat <= FF_UNDEF)
	CLEAR = TRUE;                      /* we may clear upon error */
    }
    dop = doPtr;
  }
  else
    dop = allocDObj();
  if(dop != NULL) {
    clrAsspMsg();
    dop->filePath = filePath;
    dop->openMode = AFO_NONE;
    if(mode & AFO_READ) {                    /* open an existing file */
      if(strcmp(filePath, "stdout") == 0 ||
	 strcmp(filePath, "stderr") == 0) {
	if(dop != doPtr)
	  freeDObj(dop);
	else if(CLEAR)
	  clearDObj(dop);
	setAsspMsg(AEF_ERR_OPEN, filePath);
	return(NULL);
      }
      strcpy(Cmode, "r");
      if(mode & AFO_WRITE)                     /* open in update mode */
	strcat(Cmode, "+");
      if(!(mode & AFO_TEXT))
	strcat(Cmode, "b");
      if(strcmp(filePath, "stdin") == 0)
	dop->fp = stdin;
      else
	dop->fp = fopen(filePath, Cmode);
      if(!(dop->fp)) {
	if(dop != doPtr)
	  freeDObj(dop);
	else if(CLEAR)
	  clearDObj(dop);
	setAsspMsg(AEF_ERR_OPEN, filePath);
	return(NULL);
      }
      err = getHeader(dop);
      if(err < 0) {
	fclose(dop->fp);
	if(dop != doPtr)
	  freeDObj(dop);
	else if(CLEAR)
	  clearDObj(dop);
	return(NULL);
      } /* else retain warning if set */
    }
    else if(mode & AFO_WRITE) {                    /* create/truncate */
      if(strcmp(filePath, "stdin") == 0) {
	if(dop != doPtr)
	  freeDObj(dop);
	else if(CLEAR)
	  clearDObj(dop);
	setAsspMsg(AEF_ERR_OPEN, filePath);
	return(NULL);
      }
      strcpy(Cmode, "w+");
      if(!(mode & AFO_TEXT))
	strcat(Cmode, "b");
#ifndef WRASSP
      if(strcmp(filePath, "stdout") == 0)
	dop->fp = stdout;
      else if(strcmp(filePath, "stderr") == 0)
	dop->fp = stderr;
      else
#endif
	dop->fp = fopen(filePath, Cmode);
      if(dop->fp == NULL) {
	if(dop != doPtr)
	  freeDObj(dop);
	else if(CLEAR)
	  clearDObj(dop);
	setAsspMsg(AEF_ERR_OPEN, filePath);
	return(NULL);
      }
      if(dop == doPtr) {
	err = putHeader(dop);
	if(err < 0) {
	  fclose(dop->fp);
	  dop->fp = NULL;
	  return(NULL);
	} /* else retain warning if set */
      }
    }
    dop->openMode = mode;                      /* successfully opened */
  }
  return(dop);
}

/*DOC

Function 'asspFClose'

Closes the data file referred to in the data object pointed to by "dop".
"action" should be set to one of the following:
AFC_KEEP   Leave the data object as it is; only clear 'fp' and 'openMode'.
           This is useful if you wish to retain the data descriptors
           and the contents of the data buffer.
AFC_CLEAR  Free all memory allocated in the data object and descriptors 
           but do not free the object itself. Use this only if the data 
           object was not allocated by asspFOpen() but declared fixed.
AFC_FREE   As AFC_CLEAR but also free the data object itself. Use 
           always if the object was allocated by asspFOpen() and you do 
           not wish to retain it.
Note that the actions are mutually exclusive.

DOC*/

int asspFClose(DOBJ *dop, int action)
{
  if(dop == NULL || !(action == AFC_KEEP || action == AFC_CLEAR ||
	              action == AFC_FREE) ) {
    setAsspMsg(AEB_BAD_ARGS, "asspFClose");
    return(-1);
  }
  if(dop->fp != NULL) {
#ifndef WRASSP
    if(dop->fp != stdout && dop->fp != stderr && dop->fp != stdin)
      fclose(dop->fp);
#else
      fclose(dop->fp);
#endif
    dop->fp = NULL;
  }
  if(action == AFC_FREE)
    freeDObj(dop);
  else if(action == AFC_CLEAR)
    clearDObj(dop);
  else /* AFC_KEEP */
    dop->openMode = AFO_NONE;
/*   clrAsspMsg(); leave message; may be forced to close before handling */
  return(0);
}

/*DOC

Function 'asspFSeek'

Positions the file pointer to the data file referred to in the data object 
pointed to by "dop" to the first byte of record "recordNr". "recordNr" 
hereby refers to absolute time, NOT to the begin of the file.
Returns position of the file pointer as number of records from begin of 
file, or -1 upon error.

DOC*/

long asspFSeek(DOBJ *dop, long recordNr)
{
  long offset;

  if(dop == NULL || recordNr < 0) {
    setAsspMsg(AEB_BAD_ARGS, "asspFSeek");
    return(-1);
  }
  if(dop->fp == NULL || dop->headerSize < 0 ||\
     dop->fileData != FDF_BIN || dop->recordSize < 1) {
    setAsspMsg(AEB_BAD_CALL, "asspFSeek");
    return(-1);
  }

  recordNr -= (dop->startRecord);        /* convert to record in file */
  if(recordNr < 0) {
    setAsspMsg(AEB_TOO_SOON, "(asspFSeek)");
    return(-1);
  }
  if(recordNr > dop->numRecords) {
    setAsspMsg(AEB_TOO_LATE, "(asspFSeek)");
    return(-1);
  }
  offset = dop->headerSize + (recordNr * (long)(dop->recordSize));
  if(fseek(dop->fp, offset, SEEK_SET) != 0) {
    setAsspMsg(AEF_ERR_SEEK, dop->filePath);
    return(-1);
  }
/*   clrAsspMsg(); */
  return(recordNr);
}

/*DOC

Function 'asspFTell'

Returns the position the file pointer of the data file referred to by 
"dop" as an absolute record number. If the file pointer is not positioned 
at a record boundary, the position will be rounded downwards. Subtract 
dop->startRecord to get the position as number of records from begin of 
file. This function returns -1 upon error.

DOC*/

long asspFTell(DOBJ *dop)
{
  long bytePos, recordNr;

  if(dop == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "asspFTell");
    return(-1);
  }
  if(dop->fp == NULL || dop->headerSize < 0 ||\
     dop->fileData != FDF_BIN || dop->recordSize < 1) {
    setAsspMsg(AEB_BAD_CALL, "asspFTell");
    return(-1);
  }

  bytePos = ftell(dop->fp);
  if(bytePos < 0) {
    setAsspMsg(AEF_ERR_SEEK, dop->filePath);
    return(-1);
  }
/*   clrAsspMsg(); */
  recordNr = (bytePos - dop->headerSize) / (long)(dop->recordSize);
  if(recordNr < 0)                  /* we're apparently in the header */
    recordNr = 0;
  return(recordNr + dop->startRecord);
}

/*DOC

Function 'asspFRead'

Low-level raw sequential binary read function: Reads next "numRecords" 
records from the file referred to in the data object pointed to by 
"dop" into "buffer". 
Returns the number of records actually read, which may be less than 
"numRecords" when there are fewer records available in the file; or -1 
upon error.
Use asspFSeek() to position the file pointer to a record boundary, 
perform EOF checks, etc. 

NOTE: This function does not change data object items. 

DOC*/

long asspFRead(void *buffer, long numRecords, DOBJ *dop)
{
  size_t numRead;

  if(dop == NULL || buffer == NULL || numRecords < 0) {
    setAsspMsg(AEB_BAD_ARGS, "asspFRead");
    return(-1);
  }
  if(dop->fp == NULL || dop->fileData != FDF_BIN || dop->recordSize < 1) {
    setAsspMsg(AEB_BAD_CALL, "asspFRead");
    return(-1);
  }

  if(numRecords > 0) {
    clearerr(dop->fp);    /* because we'll have to test on these later */
    numRead = fread(buffer, dop->recordSize, (size_t)numRecords, dop->fp);
    if((numRead == 0 && feof(dop->fp)) || ferror(dop->fp)) {
      setAsspMsg(AEF_ERR_READ, dop->filePath);
      return(-1);
    }
    numRecords = (long)numRead;
  }
/*   clrAsspMsg(); */
  return(numRecords);
}

/*DOC

Function 'asspFWrite'

Low-level raw sequential binary write function: Copies "numRecords" 
records from "buffer" to the file referred to in the data object 
pointed to by "dop". 
Returns the number of records written or -1 upon error.
Use asspFSeek() to position the file pointer to a record boundary, 
perform EOF checks, etc. 

NOTE - It is assumed that the byte order of the data in the buffer 
       matches that of the file.
     - This function does not change data object items. 

DOC*/

long asspFWrite(void *buffer, long numRecords, DOBJ *dop)
{
  size_t numWrite;

  if(numRecords <= 0)                                /* nothing to do */
    return(0);
  if(buffer == NULL || dop == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "asspFWrite");
    return(-1);
  }
  if(dop->fp == NULL || dop->fileData != FDF_BIN || dop->recordSize < 1) {
    setAsspMsg(AEB_BAD_CALL, "asspFWrite");
    return(-1);
  }

  clearerr(dop->fp);     /* because we'll have to test on these later */
  numWrite = fwrite(buffer, dop->recordSize, (size_t)numRecords, dop->fp);
  if((long)numWrite != numRecords ||
     feof(dop->fp) || ferror(dop->fp)) {
    setAsspMsg(AEF_ERR_WRIT, dop->filePath);
    return(-1);
  }
  fflush(dop->fp);
/*   clrAsspMsg(); */
  return(numRecords);
}

/*DOC

Function 'asspFPrint'

Low-level sequential ASCII print function: Writes "numRecords" of data 
from "buffer" to the file referred to in the data object pointed to by 
"dop". The data of each record will be put on a separate line. The 
formatting of the line can be controlled by the items 'sepChars' in the 
data object and the data descriptors. For floating point data, the 
item 'ascFormat' is provided in the corresponding data descriptor to 
specify the output format (precision, F or E format, ...).
If "extra" contains the flag AFW_ADD_TIME, an initial field will be 
included with the start time of each record. In this case you have to 
provide the correct value for "startRecord".
Returns the number of records printed or -1 upon error.

NOTE - This function will set the data object items 'eol', 'sepChars' 
       and 'ascFormat' if they were not defined.
     - For files in ASCII format, it is not possible to - reliably - 
       relate a record number to a line in the file. Use this function 
       with caution!

DOC*/

long asspFPrint(void *buffer, long startRecord, long numRecords,\
		DOBJ *dop, int extra)
{
  uint8_t  *bPtr;
  char      tFormat[16], dFormat[16];
  int       n, nd, rn, err;
  double    time, frameRate=0.0;
  DDESC    *dd;
  char     *cPtr;    /* pointers for the various data formats */
  uint8_t  *u8Ptr;
  int8_t   *i8Ptr;
  uint16_t *u16Ptr;
  int16_t  *i16Ptr;
  uint32_t *u32Ptr;
  int32_t  *i32Ptr;
  uint64_t *u64Ptr;
  int64_t  *i64Ptr;
  float    *f32Ptr;
  double   *f64Ptr;

  if(numRecords <= 0)                                /* nothing to do */
    return(0);
  if(buffer == NULL || dop == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "asspFPrint");
    return(-1);
  }
  if(dop->fp == NULL || dop->fileData != FDF_ASC || dop->recordSize < 1) {
    setAsspMsg(AEB_BAD_CALL, "asspFPrint");
    return(-1);
  }
  if(strlen(dop->sepChars) == 0)
    strcpy(dop->sepChars, "\t");
  if(strlen(dop->eol) == 0)
    strcpy(dop->eol, NATIVE_EOL);
  dd = &(dop->ddl);
  while(dd != NULL) {              /* basic check of data descriptors */
    if(dd->coding != DC_LIN && dd->coding != DC_PCM) {
      setAsspMsg(AEB_BAD_CALL, "asspFPrint");
      return(-1);
    }
    if(strlen(dd->sepChars) == 0)
      strcpy(dd->sepChars, " ");
    switch(dd->format) {
    case DF_REAL32:
    case DF_REAL64:
      if(strlen(dd->ascFormat) == 0)
	strcpy(dd->ascFormat, "%f");
      break;
    default:
      break;
    }
    dd = dd->next;
  }
  if(extra & AFW_ADD_TIME) {/* construct format string for time field */
    if(startRecord < 0) {
      setAsspMsg(AEB_BAD_CALL, "asspFPrint");
      return(-1);
    }
    if(dop->sampFreq > 0.0 && dop->frameDur > 0)
      frameRate = dop->sampFreq / (double)dop->frameDur;
    else if(dop->dataRate > 0.0)
      frameRate = dop->dataRate;
    else {
      setAsspMsg(AEB_BAD_CALL, "asspFPrint");
      return(-1);
    }
    nd = numDecim(1.0/frameRate, 12);
    if(nd <= 0) nd = 1;
    time = SMPNRtoTIME(startRecord, frameRate);
    time = foreignTime(time, dop, FALSE);  /* ASSP ==> foreign time */
    n = numDecim(time, 12);
    if(n > nd) nd = n;
    sprintf(tFormat, "%%%i.%if%s", nd+4, nd, dop->sepChars);
  }

  err = 0;
  bPtr = (uint8_t *)buffer;
  for(rn = 0; rn < numRecords; rn++) {
    if(extra & AFW_ADD_TIME) {                    /* print time field */
      time = SMPNRtoTIME(startRecord + rn, frameRate);
      time = foreignTime(time, dop, FALSE);  /* ASSP ==> foreign time */
      if((err=fprintf(dop->fp, tFormat, time)) < 0)
	break;
    }
    dd = &(dop->ddl);
    while(dd != NULL) {
      switch(dd->format) {
      case DF_CHAR:
	cPtr = (char *)&bPtr[dd->offset];
	for(n = 0; n < dd->numFields; n++) {
	  if(n == 0)
	    err = fprintf(dop->fp, "%c", *cPtr);
	  else
	    err = fprintf(dop->fp, "%s%c", dd->sepChars, cPtr[n]);
	  if(err < 0) break;
	}
	break;
      case DF_UINT8:
	u8Ptr = &bPtr[dd->offset];
	for(n = 0; n < dd->numFields; n++) {
	  if(n == 0)
	    err = fprintf(dop->fp, "%u", (unsigned int)(*u8Ptr));
	  else
	    err = fprintf(dop->fp, "%s%u", dd->sepChars,\
			  (unsigned int)u8Ptr[n]);
	  if(err < 0) break;
	}
	break;
      case DF_INT8:
	i8Ptr = (int8_t *)&bPtr[dd->offset];
	for(n = 0; n < dd->numFields; n++) {
	  if(n == 0)
	    err = fprintf(dop->fp, "%i", (int)(*i8Ptr));
	  else
	    err = fprintf(dop->fp, "%s%i", dd->sepChars, (int)i8Ptr[n]);
	  if(err < 0) break;
	}
	break;
      case DF_UINT16:
	u16Ptr = (uint16_t *)&bPtr[dd->offset];
	for(n = 0; n < dd->numFields; n++) {
	  if(n == 0)
	    err = fprintf(dop->fp, "%u", (unsigned int)(*u16Ptr));
	  else
	    err = fprintf(dop->fp, "%s%u", dd->sepChars,\
			  (unsigned int)u16Ptr[n]);
	  if(err < 0) break;
	}
	break;
      case DF_INT16:
	i16Ptr = (int16_t *)&bPtr[dd->offset];
	for(n = 0; n < dd->numFields; n++) {
	  if(n == 0)
	    err = fprintf(dop->fp, "%i", (int)(*i16Ptr));
	  else
	    err = fprintf(dop->fp, "%s%i", dd->sepChars, (int)i16Ptr[n]);
	  if(err < 0) break;
	}
	break;
      case DF_UINT32:
	u32Ptr = (uint32_t *)&bPtr[dd->offset];
	for(n = 0; n < dd->numFields; n++) {
	  if(n == 0)
	    err = fprintf(dop->fp, "%lu", (unsigned long)(*u32Ptr));
	  else
	    err = fprintf(dop->fp, "%s%lu", dd->sepChars,\
			  (unsigned long)u32Ptr[n]);
	  if(err < 0) break;
	}
	break;
      case DF_INT32:
	i32Ptr = (int32_t *)&bPtr[dd->offset];
	for(n = 0; n < dd->numFields; n++) {
	  if(n == 0)
	    err = fprintf(dop->fp, "%li", (long)(*i32Ptr));
	  else
	    err = fprintf(dop->fp, "%s%li", dd->sepChars, (long)i32Ptr[n]);
	  if(err < 0) break;
	}
	break;
      case DF_UINT64:
	u64Ptr = (uint64_t *)&bPtr[dd->offset];
	if(sizeof(long) >= sizeof(int64_t)) {
	  for(n = 0; n < dd->numFields; n++) {
	    if(n == 0)
	      err = fprintf(dop->fp, "%lu", (unsigned long)(*u64Ptr));
	    else
	      err = fprintf(dop->fp, "%s%lu", dd->sepChars,\
			    (unsigned long)u64Ptr[n]);
	    if(err < 0) break;
	  }
	}
	else {
	  for(n = 0; n < dd->numFields; n++) {
	    if(n == 0)
        #ifdef OS_Windows
          err = fprintf(dop->fp, "%I64u", (unsigned long long)(*u64Ptr));
        #else
	        err = fprintf(dop->fp, "%llu", (unsigned long long)(*u64Ptr));
        #endif
	    else
        #ifdef OS_Windows
	        err = fprintf(dop->fp, "%s%I64u", dd->sepChars,\
			      (unsigned long long)u64Ptr[n]);
        #else
          err = fprintf(dop->fp, "%s%llu", dd->sepChars,\
			      (unsigned long long)u64Ptr[n]);
        #endif
	    if(err < 0) break;
	  }
	}
	break;
      case DF_INT64:
	i64Ptr = (int64_t *)&bPtr[dd->offset];
	if(sizeof(long) >= sizeof(int64_t)) {
	  for(n = 0; n < dd->numFields; n++) {
	    if(n == 0)
	      err = fprintf(dop->fp, "%li", (long)(*i64Ptr));
	    else
	      err = fprintf(dop->fp, "%s%li", dd->sepChars,\
			    (long)i64Ptr[n]);
	    if(err < 0) break;
	  }
	}
	else {
	  for(n = 0; n < dd->numFields; n++) {
	    if(n == 0)
        #ifdef OS_Windows
	        err = fprintf(dop->fp, "%I64d", (long long)(*i64Ptr));
        #else
          err = fprintf(dop->fp, "%lli", (long long)(*i64Ptr));
        #endif
	    else
        #ifdef OS_Windows
	        err = fprintf(dop->fp, "%s%I64d", dd->sepChars,\
			      (long long)i64Ptr[n]);
        #else
          err = fprintf(dop->fp, "%s%lli", dd->sepChars,\
			      (long long)i64Ptr[n]);        
        #endif
	    if(err < 0) break;
	  }
	}
	break;
      case DF_REAL32:
	sprintf(dFormat, "%s%s", dd->sepChars, dd->ascFormat);
	f32Ptr = (float *)&bPtr[dd->offset];
	for(n = 0; n < dd->numFields; n++) {
	  if(n == 0)
	    err = fprintf(dop->fp, dd->ascFormat, (double)(*f32Ptr));
	  else
	    err = fprintf(dop->fp, dFormat, (double)f32Ptr[n]);
	  if(err < 0) break;
	}
	break;
      case DF_REAL64:
	sprintf(dFormat, "%s%s", dd->sepChars, dd->ascFormat);
	f64Ptr = (double *)&bPtr[dd->offset];
	for(n = 0; n < dd->numFields; n++) {
	  if(n == 0)
	    err = fprintf(dop->fp, dd->ascFormat, *f64Ptr);
	  else
	    err = fprintf(dop->fp, dFormat, f64Ptr[n]);
	  if(err < 0) break;
	}
	break;
      default:
	setAsspMsg(AED_NOHANDLE, "(asspFPrint)");
	return(-1);
      }  /* end switch */
      if(err < 0) {
	setAsspMsg(AEF_ERR_WRIT, dop->filePath);
	return(-1);
      }
      dd = dd->next;
      if(dd != NULL) {                             /* field following */
	if(fprintf(dop->fp, "%s", dop->sepChars) < 0) {
	  setAsspMsg(AEF_ERR_WRIT, dop->filePath);
	  return(-1);
	}
      }
    }  /* end loop over descriptors */
    if(fprintf(dop->fp, "%s", dop->eol) < 0) {
      setAsspMsg(AEF_ERR_WRIT, dop->filePath);
      return(-1);
    }
    fflush(dop->fp);
    bPtr = &bPtr[dop->recordSize];
  }  /* end loop over records */
  if(err < 0) {
    setAsspMsg(AEF_ERR_WRIT, dop->filePath);
    return(-1);
  }
  return(numRecords);
}

/*DOC

Function 'asspFFill'

Reads records from the file referred to by the data object pointed to 
by "dop" into the data buffer of that object. This function uses the 
data object items 'bufStartRec', 'maxBufRecs' and 'numRecords' to 
determine the range of data to be read. If the byte order of the file 
differs from that of the system the data will automatically be swapped.
Returns the number of records actually read (which may be 0) or -1 
upon error.

NOTE: This function sets the data object items 'bufNumRecs' to the 
      number of records read. It does not change the item 'numRecords'. 

DOC*/

long asspFFill(DOBJ *dop)
{
  long   eofRecNr, numRead;
  ENDIAN sysEndian={MSB};

  if(dop == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "asspFFill");
    return(-1);
  }
  if(dop->fp == NULL || dop->recordSize < 1 || dop->fileData != FDF_BIN ||\
     dop->dataBuffer == NULL || dop->maxBufRecs < 1 || dop->bufStartRec < 0) {
    setAsspMsg(AEB_BAD_CALL, "asspFFill");
    return(-1);
  }

  if(dop->bufStartRec < dop->startRecord) {
    setAsspMsg(AEB_TOO_SOON, "(asspFFill)");
    return(-1);
  }
  eofRecNr = dop->startRecord + dop->numRecords;
  if(dop->bufStartRec > eofRecNr) { /* allow attempt to read from EOF */
    setAsspMsg(AEF_ERR_EOF, dop->filePath);
    return(-1);
  }
  numRead = dop->maxBufRecs;
  if(numRead > (eofRecNr - dop->bufStartRec))
    numRead = eofRecNr - dop->bufStartRec;
  if(numRead > 0) {
    if(asspFSeek(dop, dop->bufStartRec) < 0) {
      return(-1);
    }
    numRead = asspFRead(dop->dataBuffer, numRead, dop);
    if(numRead < 0) {
      dop->bufNumRecs = 0;              /* no valid records in buffer */
      return(-1);
    }
  }
  dop->bufNumRecs = numRead;
  dop->bufNeedsSave = FALSE;
  if(DIFFENDIAN(dop->fileEndian, sysEndian))
    swapDataBuf(dop);
  return(numRead);
}

/*DOC

Function 'asspFFlush'

Writes the contents of the data buffer in the data object pointed to by 
"dop" to the file referred to by "dop". If the item 'fileData' has been 
set to FDF_BIN, the values in the data buffer will be swapped when 
necessary, the file pointer will be set to the record 'bufStartRec' and 
then a call will be made to 'asspFWrite()'. For ASCII data, the file 
pointer CAN NOT be positioned this way, so writing occurs starting from 
the current file position (see 'asspFPrint()' for further details).
If 'opts' contains the flag 'AFW_KEEP', the values in the data buffer 
will be restored if swapped and no further action will be taken.
Otherwise, the buffer will be cleared and the item 'bufStartRec' will 
be increased by the number of records written. For binary data, the 
item 'numRecords' will be adjusted if necessary. For ASCII data, it will 
always be increased by the number of records written. If writing was 
successfull, the item 'bufNeedsSave' will be set to FALSE.
This function returns the number of records written or -1 upon error.

DOC*/

long asspFFlush(DOBJ *dop, int opts)
{
  int    swapped;
  long   numWrite, fileRecs, endRecNr;
  ENDIAN sysEndian={MSB};

  if(dop == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "asspFFlush");
    return(-1);
  }
  if(dop->fp == NULL || dop->recordSize < 1 ||
     dop->dataBuffer == NULL || dop->maxBufRecs < 1) {
    setAsspMsg(AEB_BAD_CALL, "asspFFlush");
    return(-1);
  }

  if(dop->bufNumRecs < 1) {                      /* got nothing to do */
    dop->bufNumRecs = 0;
    dop->bufNeedsSave = FALSE;
/*     clrAsspMsg(); */
    return(0);
  }
  swapped = 0;
  if(dop->fileData == FDF_BIN) {
    fileRecs = asspFSeek(dop, dop->bufStartRec);
    if(fileRecs < 0)
      return(-1);
    if(DIFFENDIAN(dop->fileEndian, sysEndian)) {
      swapped = swapDataBuf(dop);
      if(swapped < 0)
	return(-1);
    }
    numWrite = asspFWrite(dop->dataBuffer, dop->bufNumRecs, dop);
    if(numWrite < 0) {
      if(swapped)                                     /* restore data */
	swapDataBuf(dop);
      return(-1);
    } /* else numWrite == dop->bufNumRecs */
    fileRecs += numWrite;
    if(fileRecs > dop->numRecords)
      dop->numRecords = fileRecs;
  }
  else {
    numWrite = asspFPrint(dop->dataBuffer, dop->bufStartRec,\
			  dop->bufNumRecs, dop, opts);
    if(numWrite < 0)
      return(-1);
    endRecNr = dop->bufStartRec + dop->bufNumRecs;
    if((dop->startRecord + dop->numRecords) < endRecNr)
      dop->numRecords = endRecNr - dop->startRecord;
  }
  dop->bufNeedsSave = FALSE;
  if(opts & AFW_KEEP) {
    if(swapped)
      swapDataBuf(dop);
  }
  else {
    dop->bufStartRec += numWrite;                /* update can't harm */
    clearDataBuf(dop);                         /* zeroise data buffer */
  }
/*   clrAsspMsg(); */
  return(numWrite);
}

/*DOC

Function 'recordIndex'

Determines the index of the first byte of the record "nr" in the data 
buffer of the data object pointed to by  "dop". "head" and "tail" 
( >= 0 ) can be used when processing requires records before and/or 
behind the record in question.
If the record range "nr" - "head" ... "nr" + "tail" is not in the data 
buffer, it will be read from the file referred to in "dop". Record "nr" 
MUST be in the file. Records in the head or the tail outside the range 
in the file will be set to zero.
Returns the index ( >= 0 ) in the data buffer or -1 upon error.

NOTE - The records in the data buffer will always be in the system's 
       byte order.

DOC*/

long recordIndex(DOBJ *dop, long nr, long head, long tail)
{
  char  *rPtr;
  size_t recSize;
  long   recordNr, eofRecNr, numRead;
  ENDIAN sysEndian={MSB};

  if(TRACE[0]) {
    if(dop == NULL || nr < 0 || head < 0 || tail < 0) {
      setAsspMsg(AEB_BAD_ARGS, "recordIndex");
      return(-1);
    }
    if(dop->fp == NULL || dop->recordSize < 1 ||\
       dop->dataBuffer == NULL || dop->maxBufRecs < 1) {
      setAsspMsg(AEB_BAD_CALL, "recordIndex");
      return(-1);
    }
  }
/*
 * Check whether requested range is already in the data buffer;
 * if not, load from file.
 */
  if((nr - head) < dop->bufStartRec || 
     (nr + tail) > (dop->bufStartRec + dop->bufNumRecs)) {
    if(nr < dop->startRecord) {
      setAsspMsg(AEB_TOO_SOON, "(recordIndex)");
      return(-1);
    }
    eofRecNr = dop->startRecord + dop->numRecords;
    if(nr >= eofRecNr) {
      setAsspMsg(AEB_TOO_LATE, "(recordIndex)");
      return(-1);
    }
    if(dop->maxBufRecs < (head + 1 + tail)) {
      setAsspMsg(AEB_BUF_SPACE, "(recordIndex)");
      return(-1);
    }
    if(dop->fp == NULL) {
      setAsspMsg(AEF_NOT_OPEN, dop->filePath);
      return(-1);
    }
    dop->bufStartRec = recordNr = nr - head;
    dop->bufNumRecs = 0;
    recSize = dop->recordSize;
    rPtr = dop->dataBuffer;
    while(recordNr < dop->startRecord) {       /* head off with zeros */
      memset(rPtr, 0, recSize);
      rPtr += recSize;
      (dop->bufNumRecs)++;
      recordNr++;
    }
    if(asspFSeek(dop, recordNr) < 0)
      return(-1);
    numRead = dop->maxBufRecs - dop->bufNumRecs;
    if(numRead > (eofRecNr - recordNr))
      numRead = eofRecNr - recordNr;
    if((numRead=asspFRead(rPtr, numRead, dop)) < 0)
      return(-1);
    dop->bufNumRecs += numRead;  /* number of valid records in buffer */
    if(DIFFENDIAN(dop->fileEndian, sysEndian)) {
      if(swapDataBuf(dop) < 0)
	return(-1);
    }
    rPtr += (numRead * recSize);
    while(dop->bufNumRecs < dop->maxBufRecs && tail > 0) {
      memset(rPtr, 0, recSize);    /* pas with maximally "tail" zeros */
      rPtr += recSize;
      (dop->bufNumRecs)++;
      tail--;
    }
  }
  return((nr - dop->bufStartRec)*(long)(dop->recordSize));
}

/*DOC

Function 'frameIndex'

Determines the index of the first byte of the frame specified by "nr", 
"size" and "shift" in the data buffer of the audio object pointed to by 
"smpDOp". The arguments "head" and "tail" ( >= 0 ) should specify how 
many samples before and/or behind the frame should at least be in the 
buffer. If the frame range including "head" and "tail" is not in the 
data buffer, it will be read from the file referred to in "smpDOp" 
which must have been opened with 'asspFOpen'. At least one sample of 
the kernel of the frame (i.e. given by "shift") MUST be in the file 
range. Samples, outside that range will be set to zero.
Returns the index ( >= 0 ) in the data buffer or -1 upon error.

NOTE - The samples in the data buffer will always be in the system's 
       byte order.
     - This function can handle all audio codings whose units are an 
       integral number of bytes. It can also handle multi-channel 
       files.
     - If this function is used in random-access mode, the data buffer 
       should be as small as possible (i.e. "head" + "size" + "tail") 
       to avoid unnecessary file access.

DOC*/

long frameIndex(DOBJ *smpDOp, long nr, long size, long shift,\
		long head, long tail)
{
  char  *rPtr;
  size_t recSize;
  long   frameSn, begRecNr, recordNr, endRecNr, eofRecNr, numRead;
  ENDIAN sysEndian={MSB};

  if(TRACE[0]) {
    if(smpDOp == NULL || nr < 0 || size < 1 ||\
       shift < 1 || head < 0 || tail < 0) {
      setAsspMsg(AEB_BAD_ARGS, "frameIndex");
      return(-1);
    }
    if(smpDOp->recordSize < 1 || smpDOp->dataBuffer == NULL ||\
       smpDOp->maxBufRecs < 1) {
      setAsspMsg(AEB_BAD_CALL, "frameIndex");
      return(-1);
    }
  }
  begRecNr = FRMNRtoSMPNR(nr, shift) - FRAMEHEAD(size, shift);
  endRecNr = begRecNr + size + tail;
/*
 * Check whether requested range is already in the data buffer;
 * if not, load from file.
 */
  if((begRecNr - head) < smpDOp->bufStartRec ||\
     endRecNr > (smpDOp->bufStartRec + smpDOp->bufNumRecs)) {
    frameSn = FRMNRtoSMPNR(nr, shift);
    if((frameSn + shift) <= smpDOp->startRecord) {
      setAsspMsg(AEB_TOO_SOON, "(frameIndex)");
      return(-1);
    }
    eofRecNr = smpDOp->startRecord + smpDOp->numRecords;
    if(frameSn >= eofRecNr) {
      setAsspMsg(AEB_TOO_LATE, "(frameIndex)");
      return(-1);
    }
    if(smpDOp->maxBufRecs < (head + size + tail)) {
      setAsspMsg(AEB_BUF_SPACE, "(frameIndex)");
      return(-1);
    }
    if(smpDOp->fp == NULL) {
      setAsspMsg(AEF_NOT_OPEN, smpDOp->filePath);
      return(-1);
    }
    /* run a maximum load starting from the required record */
    smpDOp->bufStartRec = recordNr = begRecNr - head;
    smpDOp->bufNumRecs = 0;             /* no valid records in buffer */
    rPtr = smpDOp->dataBuffer;
    recSize = smpDOp->recordSize;
    while(recordNr < smpDOp->startRecord) {    /* head off with zeros */
      memset(rPtr, 0, recSize);
      rPtr += recSize;
      recordNr++;
      (smpDOp->bufNumRecs)++;
    }
    if(asspFSeek(smpDOp, recordNr) < 0)
      return(-1);
    numRead = smpDOp->maxBufRecs - smpDOp->bufNumRecs; /* get maximum */
    if(numRead > (eofRecNr - recordNr))
      numRead = eofRecNr - recordNr;
    if((numRead=asspFRead(rPtr, numRead, smpDOp)) < 0)
      return(-1);
    smpDOp->bufNumRecs += numRead;
    if(DIFFENDIAN(smpDOp->fileEndian, sysEndian)) {
      if(swapDataBuf(smpDOp) < 0)
	return(-1);
    }
    rPtr += (numRead * recSize);
    recordNr += numRead;
    while(recordNr < endRecNr) {         /* append zeros (should have */
      memset(rPtr, 0, recSize);        /* sufficient space in buffer) */
      rPtr += recSize;
      recordNr++;
      (smpDOp->bufNumRecs)++;
      tail--;               /* keep track of number of zeros appended */
    }
    if(smpDOp->bufNumRecs < smpDOp->maxBufRecs) {
      tail += (shift - 1 + FRAMETAIL(size, shift));  /* maximum zeros */
      while(smpDOp->bufNumRecs < smpDOp->maxBufRecs && tail > 0) {
	memset(rPtr, 0, recSize);
	rPtr += recSize;
	(smpDOp->bufNumRecs)++;
	tail--;
      }
    }
  }
  return((begRecNr - smpDOp->bufStartRec)*(long)(smpDOp->recordSize));
}

/*DOC

Function 'putRecord'

Transfers the data of the record "recordNr" from the data buffer of the 
data object pointed to by "src" to buffer of the one pointed to by "dst".
If - prior to transfer - "recordNr" is found to exceed the full range 
of the destination buffer, the contents of that buffer will be written 
to the file referred to in "dst" and then discarded.
See mapRecord() and asspFFlush() for further details.
The function returns the number of data values transferred or -1 upon 
error.

DOC*/

long putRecord(DOBJ *dst, DOBJ *src, long recordNr)
{
  if(TRACE[0]) {
    if(src == NULL || dst == NULL || recordNr < 0) {
      setAsspMsg(AEB_BAD_ARGS, "putRecord");
      return(-1);
    }
    if(src->recordSize < 1 || dst->recordSize < 1 ||
       src->dataBuffer == NULL || dst->dataBuffer == NULL ||
       src->maxBufRecs < 1 || dst->maxBufRecs < 1) {
      setAsspMsg(AEB_BAD_CALL, "putRecord");
      return(-1);
    }
  }
  if(recordNr < src->bufStartRec ||
     recordNr >= src->bufStartRec + src->bufNumRecs) {
    setAsspMsg(AEB_BUF_RANGE, "(putRecord)");
    return(-1);
  }
  if(dst->bufNumRecs < 1) { /* no valid records in destination buffer */
    dst->bufStartRec = recordNr;
    dst->bufNumRecs = 0;
    dst->bufNeedsSave = FALSE;
  }
  else if(recordNr >= dst->bufStartRec + dst->maxBufRecs) {
    if(asspFFlush(dst, AFW_CLEAR) < 0)
      return(-1);
  }
  return(mapRecord(dst, src, recordNr));
}

