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
* File:     dataobj.c                                                  *
* Contents: Basic functions for handling ASSP data objects.            *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: dataobj.c,v 1.10 2012/01/24 15:48:10 lasselasse Exp $ */

#include <stdio.h>      /* NULL */
#include <stddef.h>     /* size_t */
#include <stdlib.h>     /* malloc() calloc() free() */
#include <string.h>     /* strdup() memcpy() memset() strlen() strcmp() */
#include <inttypes.h>   /* int.._t uint.._t */
#include <math.h>       /* fabs() */

#include <misc.h>       /* EOS TRUE FALSE */
#include <asspendian.h> /* byte-order stuff */
#include <trace.h>      /* TRACE[] */
#include <asspmess.h>   /* message codes; reference to globals */
#include <assptime.h>   /* conversion macros */
#include <asspfio.h>    /* asspFSEEK() asspFRead() */
#include <dataobj.h>    /* DOBJ DDESC dform_e */
#include <aucheck.h>    /* AUC_... */
#include <auconv.h>     /* int24_to_int32() */


/*DOC

Allocates memory for a data object structure and initializes it.
Returns a pointer to the structure or NULL when out of memory.

DOC*/

DOBJ *allocDObj(void)
{
  DOBJ *dop;

  dop = (DOBJ *)malloc(sizeof(DOBJ));
  if(dop == NULL)
    setAsspMsg(AEG_ERR_MEM, NULL);
  else
    initDObj(dop);            /* make sure all items are properly set */
  return(dop);
}

/*DOC

Frees all memory allocated for the data object pointed by "dop", which 
must have been returned by a previous call to allocDObj().
This function always returns a NULL pointer.

DOC*/

DOBJ *freeDObj(DOBJ *dop)
{
  if(dop != NULL) {
    clearDObj(dop);             /* also frees memory in substructures */
    free((void *)dop);
  }
  return((DOBJ *)NULL);
}

/*DOC

Frees all allocated memory in the data object pointed to by "dop"
and re-initialzes the data object structure.
This function does NOT return memory allocated for the data object 
itself; use freeDObj() for this.

DOC*/

void clearDObj(DOBJ *dop)
{
  if(dop != NULL) {
    freeDDList(dop);
    freeMeta(dop);
    freeGeneric(dop);
    freeDataBuf(dop);
    initDObj(dop);                              /* now zero all items */
  }
  return;
}

/*DOC

Frees all memory allocated for the data descriptor list of the data 
object pointed to by "dop" and re-initializes the first (fixed) 
descriptor.

DOC*/

void freeDDList(DOBJ *dop)
{
  DDESC *dd;

  if(dop != NULL) {
    dd = clearDDesc(&(dop->ddl));/* can't use freeDDesc() while fixed */
    while(dd != NULL)  /* other descriptors in the list are allocated */
      dd = freeDDesc(dd);
  }
  return;
}

/*DOC

Frees all memory allocated for the generic variable list of the data 
object pointed to by "dop" and re-initializes the first (fixed) 
variable.

DOC*/

void freeMeta(DOBJ *dop)
{
  TSSFF_Generic *genVar;

  if(dop != NULL) {
    genVar = clearTSSFF_Generic(&(dop->meta));/* can't use freeTSSFF_Generic() while fixed */
    while(genVar != NULL)  /* other variables in the list are allocated */
      genVar = freeTSSFF_Generic(genVar);
  }
  return;
}

/*DOC

Sets all items in the data object structure pointed to by "dop" to 
defined (zero) values.

Note:
 - All valid items, including pointers to allocated memory, should be 
   saved before calling this function.

DOC*/

void initDObj(DOBJ *dop)
{
  if(dop != NULL) {
    dop->filePath = NULL;
    dop->fp = NULL;
    dop->openMode = 0;
    dop->fileFormat = FF_UNDEF;
    dop->fileData = FDF_UNDEF;
    CLRENDIAN(dop->fileEndian);
    dop->version = 0;
    dop->headerSize = 0;
    dop->sampFreq = 0.0;
    dop->dataRate = 0.0;
    dop->frameDur = 0;
    dop->recordSize = 0;
    dop->startRecord = 0;
    dop->numRecords = 0;
    dop->Time_Zero = 0.0;
    dop->Start_Time = 0.0;
    dop->sepChars[0] = EOS;
    dop->eol[0] = EOS;
    initDDesc(&(dop->ddl));
    initTSSFF_Generic(&(dop->meta));
    dop->generic = NULL;
    dop->doFreeGeneric = NULL;
    dop->dataBuffer = NULL;
    dop->doFreeDataBuf = NULL;
    dop->maxBufRecs = 0;
    dop->bufStartRec = 0;
    dop->bufNumRecs = 0;
    dop->bufNeedsSave = FALSE;
    dop->userData = NULL;
  }
  return;
}

/*DOC

Copies items in the data object pointed to by "src" to the one pointed 
to by "dst", except for items like 'filePath', 'fp', 'openMode', 
'generic' and 'dataBuffer' but including (newly allocated) data 
descriptors. 
The function returns -1 upon error, otherwise 0.

Note:
 - Prior to copying, his function calls initDObj() on "dst" (see above). 
   The calling function should therefore keep copies of elements which 
   may (unwittingly) be cleared by that function.

DOC*/

int copyDObj(DOBJ *dst, DOBJ *src)
{
  DDESC *dd, *next;
  TSSFF_Generic *genVar, *nextG;

  if(src == NULL || dst == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "copyDObj");
    return(-1);
  }
  initDObj(dst);
  dst->fileFormat = src->fileFormat;
  dst->fileData = src->fileData;
  CPYENDIAN(dst->fileEndian, src->fileEndian);
  dst->version = src->version;
  dst->headerSize = src->headerSize;
  dst->sampFreq = src->sampFreq;
  dst->dataRate = src->dataRate;
  dst->frameDur = src->frameDur;
  dst->recordSize = src->recordSize;
  dst->startRecord = src->startRecord;
  dst->numRecords = src->numRecords;
  dst->Time_Zero = src->Time_Zero;
  dst->Start_Time = src->Start_Time;
  strcpy(dst->sepChars, src->sepChars);
  strcpy(dst->eol, src->eol);
  if(copyDDesc(&(dst->ddl), &(src->ddl)) < 0) {
    clearDObj(dst);
    return(-1);
  }
  next = (src->ddl).next;
  while(next != NULL) {
    dd = addDDesc(dst);
    if(dd == NULL) {
      clearDObj(dst);
      return(-1);
    }
    if(copyDDesc(dd, next) < 0) {
      clearDObj(dst);
      return(-1);
    }
    next = next->next;
  }

  if(copyTSSFF_Generic(&(dst->meta), &(src->meta)) < 0) {
    clearDObj(dst);
    return(-1);
  }
  nextG = (src->meta).next;
  while(next != NULL) {
    genVar = addTSSFF_Generic(dst);
    if(genVar == NULL) {
      clearDObj(dst);
      return(-1);
    }
    if(copyTSSFF_Generic(genVar, nextG) < 0) {
      clearDObj(dst);
      return(-1);
    }
    nextG = nextG->next;
  }
  return(0);
}

/*DOC

Adds (appends) a data descriptor to the list in the data object pointed 
to by "dop". The descriptor will be initialized with zeros.
The function returns a pointer to the new descriptor or NULL if there 
is insufficient memory available.

DOC*/

DDESC *addDDesc(DOBJ *dop)
{
  DDESC *last, *new=NULL;

  if(dop != NULL) {
    new = (DDESC *)malloc(sizeof(DDESC));
    if(new != NULL) {
      initDDesc(new);                          /* initialize elements */
      last = &(dop->ddl);                /* can't be NULL while fixed */
      while(last->next != NULL)        /* search last element in list */
  last = last->next;
      last->next = new;                   /* append the new structure */
    }
    else
      setAsspMsg(AEG_ERR_MEM, NULL);
  }
  return(new);
}

/*DOC

Frees memory allocated for the elements of the data descriptor pointed 
to by "dd" and for the structure itself.
Returns a pointer to the next element in the descriptor list.

DOC*/

DDESC *freeDDesc(DDESC *dd)
{
  DDESC *next=NULL;

  if(dd != NULL) {
    next = clearDDesc(dd);
    free((void *)dd);
  }
  return(next);
}

/*DOC

Frees memory allocated for the elements of the data descriptor pointed 
to by "dd" and re-initializes it.
Returns a pointer to the next element in the descriptor list.

Note:
 - This function does NOT free the memory allocated for the data 
   descriptor structure itself; use freeDDesc() for this.

DOC*/

DDESC *clearDDesc(DDESC *dd)
{
  DDESC *next=NULL;

  if(dd != NULL) {
    if(dd->ident != NULL)
      free((void *)(dd->ident));
    next = dd->next;
    initDDesc(dd);
  }
  return(next);
}

/*DOC

Sets items in data descriptor structure to defined (empty) values.

DOC*/

void initDDesc(DDESC *dd)
{
  if(dd != NULL) {
    dd->ident = NULL;
    dd->unit[0] = EOS;
    dd->factor[0] = EOS;
    dd->type = DT_UNDEF;
    dd->format = DF_UNDEF;
    dd->coding = DC_UNDEF;
    CLRENDIAN(dd->orientation);
    dd->numBits = 0;
    dd->zeroValue = 0;
    dd->offset = 0;
    dd->numFields = 0;
    dd->ascFormat[0] = EOS;
    dd->sepChars[0] = EOS;
    dd->next = NULL;
  }
  return;
}

/*DOC

Copies items in the data descriptor structure pointed to by "src" to 
the one pointed to by "dst". The item 'next' in "dst" will be set to 
NULL. Returns -1 upon error, otherwise 0.

Note:
 - This function assumes that the descriptor structure "dst" does not 
   contain valid data.

DOC*/

int copyDDesc(DDESC *dst, DDESC *src)
{
  if(src == NULL || dst == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "copyDDesc");
    return(-1);
  }
  if(src->ident != NULL)
    dst->ident = strdup(src->ident);
  else
    dst->ident = NULL;
  strcpy(dst->unit, src->unit);
  strcpy(dst->factor, src->factor);
  dst->type = src->type;
  dst->format = src->format;
  dst->coding = src->coding;
  CPYENDIAN(dst->orientation, src->orientation);
  dst->numBits = src->numBits;
  dst->zeroValue = src->zeroValue;
  dst->numFields = src->numFields;
  strcpy(dst->ascFormat, src->ascFormat);
  strcpy(dst->sepChars, src->sepChars);
  dst->offset = src->offset;           /* may have to be recalculated */
  dst->next = NULL;
  return(0);
}

/*DOC

Determines whether the data object pointed to by "dop" contains a data 
descriptor whose type code matches "type" and/or identification string 
matches "ident". If this is the case, the function will return a pointer 
to this descriptor, otherwise, and upon error, it will return NULL.
If both "type" and "ident" are specified, both items must match.

DOC*/

DDESC *findDDesc(DOBJ *dop, dtype_e type, char *ident)
{
  int    checkID=FALSE;
  DDESC *dd=NULL;

  if(dop != NULL) {
    if(ident != NULL && strlen(ident) > 0)
      checkID = TRUE;
    for(dd = &(dop->ddl); dd != NULL; dd = dd->next) {
      if(type > DT_UNDEF) {
  if(dd->type == type) {
    if(checkID) {
      if(dd->ident != NULL && strcmp(dd->ident, ident) == 0)
        break; /* FOUND ! */
    }
    else
      break; /* FOUND ! */
  }
      }
      else if(checkID) {
  if(dd->ident != NULL && strcmp(dd->ident, ident) == 0)
    break; /* FOUND ! */
      }
      else /* ERROR */
  return(NULL);
    }
  }
  return(dd);
}

//////// FROM NOW ON REPLACE DDESC WITH SSFFST
/*DOC

Adds (appends) a data descriptor to the list in the data object pointed 
to by "dop". The descriptor will be initialized with zeros.
The function returns a pointer to the new descriptor or NULL if there 
is insufficient memory available.

DOC*/

TSSFF_Generic *addTSSFF_Generic(DOBJ *dop)
{
  TSSFF_Generic *last, *new=NULL;

  if(dop != NULL) {
    new = (TSSFF_Generic *)malloc(sizeof(TSSFF_Generic));
    if(new != NULL) {
      initTSSFF_Generic(new);                          /* initialize elements */
      last = &(dop->meta);                /* can't be NULL while fixed */
      while(last->next != NULL)        /* search last element in list */
	last = last->next;
      last->next = new;                   /* append the new structure */
    }
    else
      setAsspMsg(AEG_ERR_MEM, NULL);
  }
  return(new);
}

/*DOC

Frees memory allocated for the elements of the generic variable pointed 
to by "genVar" and for the structure itself.
Returns a pointer to the next element in the descriptor list.

DOC*/

TSSFF_Generic *freeTSSFF_Generic(TSSFF_Generic *genVar)
{
  TSSFF_Generic *next=NULL;

  if(genVar != NULL) {
    next = clearTSSFF_Generic(genVar);
    free((void *)genVar);
  }
  return(next);
}

/*DOC

Frees memory allocated for the elements of the data descriptor pointed 
to by "genVar" and re-initializes it.
Returns a pointer to the next element in the descriptor list.

Note:
 - This function does NOT free the memory allocated for the data 
   descriptor structure itself; use freeTSSFF_Generic() for this.

DOC*/

TSSFF_Generic *clearTSSFF_Generic(TSSFF_Generic *genVar)
{
  TSSFF_Generic *next=NULL;

  if(genVar != NULL) {
    if(genVar->ident != NULL)
      free((void *)(genVar->ident));
    if(genVar->data != NULL)
      free((void*)(genVar->data));
    next = genVar->next;
    initTSSFF_Generic(genVar);
  }
  return(next);
}

/*DOC

Sets items in data descriptor structure to defined (empty) values.

DOC*/

void initTSSFF_Generic(TSSFF_Generic *genVar)
{
  if(genVar != NULL) {
    genVar->ident = NULL;
    genVar->data = NULL;
    genVar->type = SSFF_UNDEF;
    genVar->next = NULL;
  }
  return;
}

/*DOC

Copies items in the data descriptor structure pointed to by "src" to 
the one pointed to by "dst". The item 'next' in "dst" will be set to 
NULL. Returns -1 upon error, otherwise 0.

Note:
 - This function assumes that the descriptor structure "dst" does not 
   contain valid data.

DOC*/

int copyTSSFF_Generic(TSSFF_Generic *dst, TSSFF_Generic *src)
{
  if(src == NULL || dst == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "copyTSSFF_Generic");
    return(-1);
  }
  if(src->ident != NULL)
    dst->ident = strdup(src->ident);
  else
    dst->ident = NULL;
  if (src->data != NULL)
    dst->data = strdup(src->data);
  else
    dst->data = NULL;
  dst->type = src->type;
  dst->next = NULL;
  return(0);
}

/*DOC

Determines whether the data object pointed to by "dop" contains a data 
descriptor whose type code matches "type" and/or identification string 
matches "ident". If this is the case, the function will return a pointer 
to this descriptor, otherwise, and upon error, it will return NULL.
If both "type" and "ident" are specified, both items must match.

DOC*/

TSSFF_Generic *findTSSFF_Generic(DOBJ *dop, char *ident)
{
  TSSFF_Generic *genVar=NULL;

  if(dop != NULL) {
    genVar = &(dop->meta);
    for (; genVar != NULL; genVar=genVar->next) {
      if (strcmp(genVar->ident, ident))
        break; // found. If it doesn't break here, genVar can only be NULL
    }
  }
  return(genVar);
}

/*DOC

Calculates and sets the record size in the data object pointed to by 
"dop" and sets the data offsets in byte in its data descriptors.
This function returns 1 upon success, 0 if the record size is not 
fixed ('recordSize' will be set to 0) and -1 upon error.

DOC*/

int setRecordSize(DOBJ *dop)
{
  DDESC *dd;

  if(dop == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setRecordSize");
    return(-1);
  }
  dop->recordSize = 0;
/*   if(dop->fileData != FDF_BIN) */
/*     return(0); */
  dd = &(dop->ddl);
  while(dd != NULL) {
    if(dd->numFields < 1) {
      setAsspMsg(AEB_BAD_CALL, "setRecordSize");
      return(-1);
    }
    dd->offset = dop->recordSize;        /* equals recordSize thusfar */
    switch(dd->format) {
    case DF_CHAR:
    case DF_UINT8:
    case DF_INT8:
      dop->recordSize += (dd->numFields * 1);
      break;
    case DF_UINT16:
    case DF_INT16:
      dop->recordSize += (dd->numFields * 2);
      break;
    case DF_UINT24:
    case DF_INT24:
      dop->recordSize += (dd->numFields * 3);
      break;
    case DF_UINT32:
    case DF_INT32:
    case DF_REAL32:
      dop->recordSize += (dd->numFields * 4);
      break;
    case DF_UINT64:
    case DF_INT64:
    case DF_REAL64:
      dop->recordSize += (dd->numFields * 8);
      break;
/*     case DF_REAL80: */
/*       dop->recordSize += (dd->numFields * 10); */
/*       break; */
/*     case DF_REAL128: */
/*       dop->recordSize += (dd->numFields * 16); */
/*       break; */
    default:
      dop->recordSize = 0;         /* not fixed (e.g. BIT/STR format) */
      return(0);
    }
    dd = dd->next;
  }
  return(1);
}

/*DOC

The items 'sampFreq', 'dataRate' and 'frameDur' are interdependent.
This function defines how to treat them systematically on the basis of 
the value of 'frameDur'. It is assumed, however, that 'sampFreq' and 
'dataRate' have - at least to some degree - been set sensibly.
This function will set/adjust these items if necessary. It returns -1 
upon error, otherwise 0.

DOC*/

int checkRates(DOBJ *dop)
{
  double fract;
  
  if(dop == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "checkRates");
    return(-1);
  }
  if(dop->fileFormat == FF_XASSP && dop->ddl.type == DT_DATA_LOG)
    return(0); /* no rates defined */
  if(dop->ddl.type == DT_SMP)
    dop->frameDur = 1; /* per definition */
  if(dop->frameDur > 1) {
    /* parametric data following ASSP conventions */
    if(dop->sampFreq > 0.0)
      dop->dataRate = (dop->sampFreq) / (double)(dop->frameDur);
    else {
      if(dop->dataRate <= 0.0) {
	setAsspMsg(AED_ERR_RATE, NULL);
	return(-1);
      } /* else assume 'dataRate' to be correctly set */
      dop->sampFreq = (dop->dataRate) * (double)(dop->frameDur);
    }
  }
  else if(dop->frameDur == 1) {
    /* plain sampled data */
    if(dop->sampFreq > 0.0)
      dop->dataRate = dop->sampFreq;
    else {
      if(dop->dataRate <= 0.0) {
	setAsspMsg(AED_ERR_RATE, NULL);
	return(-1);
      }
      dop->sampFreq = dop->dataRate;
    }
  }
  else if(dop->frameDur == 0) {
    /* data at sample points but not evenly spaced (e.g. labels) */
    if(dop->sampFreq <= 0.0) {
      if(dop->dataRate <= 0.0) {
	setAsspMsg(AED_ERR_RATE, NULL);
	return(-1);
      }
      dop->sampFreq = dop->dataRate;
    }
    dop->dataRate = 0.0; /* indicate: no fixed rate */
  }
  else { /* dop->frameDur < 0 */
    /* data at a fixed rate but possibly jittered (non-ASSP stuff) */
    if(dop->dataRate <= 0.0) {
      if(dop->sampFreq <= 0.0) {
	setAsspMsg(AED_ERR_RATE, NULL);
	return(-1);
      }
      dop->dataRate = dop->sampFreq;
      dop->sampFreq = 0.0;         /* reference sampling rate unknown */
    }
    else if(dop->sampFreq >= dop->dataRate) {
      fract = (dop->sampFreq) / (dop->dataRate);
      if(fabs(fract - myrint(fract))/fract < 0.5e-10) {
	/* accurate to one record @ maximum duration */
	dop->frameDur = (long)myrint(fract);
      }
    }
  }
  return(0);
}

/*DOC

This function sets the item 'Start_Time' in the data object pointed to 
by "dop" on the basis of the item 'startRecord' according to the SSFF 
conventions (the beginning of the record for sampled data and the centre 
for parametric data). It assumes that the data object has been properly 
configured, especially that 'startRecord' has been set correctly. 
For an object in SSFF format the item 'Time_Zero' should also have the 
correct value (see adjustTiming() in headers.c).
For all other formats 'Time_Zero' will be set to 0, indicating that the 
timing always confirms to the ASSP conventions.
This function is called in getHeader() for all formats except SSFF. If 
you create a data object and you wish to store it in an SSFF formatted 
file or to handle it according to the SSFF conventions, you will have 
to call this function AFTER having configured the data object.
The function returns -1 upon error, otherwise 0.

DOC*/

int setStart_Time(DOBJ *dop)
{
  double start=0.0;
  
  if(dop == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setStart_Time");
    return(-1);
  }
  if(checkRates(dop) != 0)    /* sync sampFreq, frameDur and dataRate */
    return(-1);
  if(dop->fileFormat == FF_XASSP && dop->ddl.type == DT_DATA_LOG)
    start = 0.0; /* not a regular data object */
  else if(dop->frameDur > 0) {
    start = FRMNRtoTIME(dop->startRecord, dop->sampFreq, dop->frameDur);
    if(dop->ddl.type != DT_SMP)                  /* the inconsistency */
      start += (FRMNRtoTIME(1, dop->sampFreq, dop->frameDur)/2.0);
  }
  else if(dop->frameDur == 0) { /* startRecord ought to be 0, but ... */
    /* start = SMPNRtoTIME(dop->startRecord, dop->sampFreq); */
    dop->startRecord = 0;                           /* per definition */
    start = 0.0;
  }
  else {           /* frameDur < 0; should never be the case, but ... */
    start = SMPNRtoTIME(dop->startRecord, dop->dataRate);
    start += (0.5 / (dop->dataRate));
  }
  if(dop->fileFormat == FF_SSFF)   /* adjust for foreign time offsets */
    start += (dop->Time_Zero);    /* this value MUST be correctly set */
  else                          /* we are p.d. using ASSP conventions */
    dop->Time_Zero = 0.0;
  if(fabs(start) < 0.5e-12)                         /* rounding error */
    start = 0.0;
  dop->Start_Time = start;
  return(0);
}

/*DOC

For data produced by programs with different timing conventions than 
ASSP, this function converts from the 'real' time for the data to the 
one matching ASSP conventions (if "TO_ASSP" = TRUE) and vice versa. 

DOC*/

double foreignTime(double t, DOBJ *dop, int TO_ASSP)
{
  if(TO_ASSP)  /* from file time with foreign conventions */
    return(t - dop->Time_Zero);
  return(t + dop->Time_Zero);
}

/*DOC

Allocates a buffer for "numRecords" in the data object pointed to by 
"dop". The buffer will be initialized with zeros and 'doFreeDataBuf'
will be set to free().
Returns a pointer to the buffer, or NULL upon error.

Note:
 - This function can only be used when 'recordSize' is fixed and it is 
   assumed that that value is correct.
 - It is also assumed that "dataBuffer" does not already point to 
   allocated memory.

DOC*/

void *allocDataBuf(DOBJ *dop, long numRecords)
{
  if(dop == NULL || numRecords < 1) {
    setAsspMsg(AEB_BAD_ARGS, "allocDataBuf");
    return(NULL);
  }
  if(dop->recordSize < 1 || dop->dataBuffer != NULL) {
    setAsspMsg(AEB_BAD_CALL, "allocDataBuf");
    return(NULL);
  }
  dop->dataBuffer = calloc((size_t)numRecords, dop->recordSize);
  if(dop->dataBuffer == NULL) {
    setAsspMsg(AEG_ERR_MEM, NULL);
    dop->maxBufRecs = 0;
    dop->doFreeDataBuf = NULL;
  }
  else {
    dop->maxBufRecs = numRecords;
    dop->doFreeDataBuf = (DOfreeFunc)free;
  }
  dop->bufStartRec = 0;                     /* no valid data in buffer */
  dop->bufNumRecs = 0;
  dop->bufNeedsSave = FALSE;
  return(dop->dataBuffer);
}

/*DOC

Zeroes all records in the data buffer of the data object pointed to 
by "dop". This function will also set the item 'bufNumRecs' to zero 
to indicate that the buffer does not contain valid data and clear the 
flag 'bufNeedsSave'.

DOC*/


void clearDataBuf(DOBJ *dop)
{
  register size_t numBytes;
  register long   n, N;
  register char  *ptr;

  if(dop != NULL) {
    if(dop->dataBuffer != NULL && dop->recordSize > 0) {
      N = dop->maxBufRecs;
      ptr = dop->dataBuffer;
      numBytes = dop->recordSize;
      for(n = 0; n < N; n++) {
	memset(ptr, 0, numBytes);
	ptr += numBytes;
      }
      dop->bufNumRecs = 0;
      dop->bufNeedsSave = FALSE;
    }
  }
  return;
}

/*DOC

Frees memory allocated for 'dataBuffer' in the data object pointed to 
by "dop". If no freeing function has been defined in the object, the 
'dataBuffer' pointer will simply be NULLed.

DOC*/

void freeDataBuf(DOBJ *dop)
{
  if(dop != NULL) {
    dop->maxBufRecs = 0;
    dop->bufStartRec = 0;
    dop->bufNumRecs = 0;
    dop->bufNeedsSave = FALSE;
    if(dop->dataBuffer != NULL) {
      if(dop->doFreeDataBuf != NULL)
	(*(dop->doFreeDataBuf))(dop->dataBuffer);
/*       else */
/* 	free(dop->dataBuffer); */
      dop->dataBuffer = NULL;
    }
  }
  return;
}

/*DOC

Frees memory allocated for 'generic' data in the data object pointed to 
by "dop". If no freeing function has been defined in the object, the 
'generic' pointer will simply be NULLed.

DOC*/

void freeGeneric(DOBJ *dop)
{
  if(dop != NULL) {
    if(dop->generic != NULL) {
      if(dop->doFreeGeneric)
	(*(dop->doFreeGeneric))(dop->generic);
/*       else */
/* 	free(dop->generic); */
      dop->generic = NULL;
    }
  }
  return;
}

/*DOC

Swaps all records in the data buffer of the data object pointed to by 
"dop".
Returns 1 if data successfully swapped, 0 if swapping was not necessary 
and -1 upon error

DOC*/

int swapDataBuf(DOBJ *dop)
{
  register uint8_t *rPtr;
  register size_t   len;
  register long     n, num;
  size_t   N;
  int      size;

  if(TRACE[0]) {
    if(dop == NULL) {
      setAsspMsg(AEB_BAD_ARGS, "swapDataBuf");
      return(-1);
    }
    if(dop->recordSize < 1) {
      setAsspMsg(AEB_BAD_CALL, "swapDataBuf");
      return(-1);
    }
  }
  if(dop->dataBuffer == NULL || dop->bufNumRecs < 1 || dop->recordSize < 2)
    return(0);                                   /* got nothing to do */
  if((size=blockSwap(dop, &N)) < 0)
    return(-1);
  rPtr = (uint8_t *)(dop->dataBuffer);
  if(size < 1) { /* can't swap 'en bloc' */
    num = dop->bufNumRecs;
    len = dop->recordSize;
    for(n = 0; n < num; n++) {
      if(swapRecord(dop, rPtr) < 0)
	return(-1);
      rPtr += len;                      /* set pointer to next record */
    }
    return(1);
  }
  if(size > 1) {
    memswab(rPtr, rPtr, (size_t)size, N * (size_t)(dop->bufNumRecs));
    return(1);
  }
  return(0); /* size == 1, needs no swapping */
}

/*DOC

Swaps the record pointed to by "rPtr" and described by the data 
descriptor(s) of the data object pointed to by "dop".
Returns 1 if data indeed swapped, 0 if swapping was not necessary 
and -1 upon error.

Note:
 - This is a general function and therefore not exactly the most 
   efficient one.

DOC*/

int swapRecord(DOBJ *dop, void *rPtr)
{
  register uint8_t *bPtr, *pPtr;
  register size_t   numBytes;
  register int      swapped=0;
  register DDESC   *dd;

  if(TRACE[0]) {
    if(dop == NULL || rPtr == NULL) {
      setAsspMsg(AEB_BAD_ARGS, "swapRecord");
      return(-1);
    }
  }
  bPtr = (uint8_t *)rPtr;
  dd = &(dop->ddl);        /* set pointer to parameter descriptor list */
  while(dd != NULL) {
    switch(dd->format) {
    case DF_UINT16:  /* 2-byte units */
    case DF_INT16:
      numBytes = 2;
      break;
    case DF_UINT24:  /* 3-byte units */
    case DF_INT24:
      numBytes = 3;
      break;
    case DF_UINT32:  /* 4-byte units */
    case DF_INT32:
    case DF_REAL32:
      numBytes = 4;
      break;
    case DF_UINT64:  /* 8-byte units */
    case DF_INT64:
    case DF_REAL64:
      numBytes = 8;
      break;
/*     case DF_REAL80: */ /* 10-byte units */
/*       numBytes = 10; */
/*       break; */
/*     case DF_REAL128:*/ /* 16-byte units */
/*       numBytes = 16; */
/*       break; */
    default:         /* rest needs no swapping */
      numBytes = 0;
    }
    if(numBytes > 1) {
      pPtr = bPtr + dd->offset;     /* set pointer to parameter field */
      memswab(pPtr, pPtr, numBytes, dd->numFields);
      swapped = 1;
    }
    dd = dd->next;                                  /* next parameter */
  }
  return(swapped);
}

/*DOC

Verifies whether a record can be swapped 'en bloc' because all units 
have the same size. If this is the case, this function will return 
the size of the units in byte and, when the pointer to "numUnits" is 
not NULL, will set it to the number of units in a record. Otherwise 
it will return 0 (FALSE). It will return -1 upon error.
This function does not check whether data need swapping at all but a 
return value of 1 (one, single-byte data) indicates that this is not 
necessary.

DOC*/

int blockSwap(DOBJ *dop, size_t *numUnits)
{
  int    numBytes;
  size_t totFields;
  DDESC *dd;

  if(dop == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "blockSwap");
    return(-1);
  }
  numBytes = 0;
  totFields = 0;
  if(numUnits != NULL)
    *numUnits = 0;
  dd = &(dop->ddl);            /* set pointer to data descriptor list */
  while(dd != NULL) {
    switch(dd->format) {
    case DF_CHAR:    /* 1-byte units */
    case DF_UINT8:
    case DF_INT8:
      if(numBytes == 0)                /* first parameter in the list */
	numBytes = 1;
      else if(numBytes != 1)           /* previous had different size */
	return(0);                          /* no block swap possible */
      totFields += (dd->numFields);              /* update unit count */
      break;
    case DF_UINT16:  /* 2-byte units */
    case DF_INT16:
      if(numBytes == 0)
	numBytes = 2;
      else if(numBytes != 2)
	return(0);
      totFields += (dd->numFields);
      break;
    case DF_UINT24:  /* 3-byte units */
    case DF_INT24:
      if(numBytes == 0)
	numBytes = 3;
      else if(numBytes != 3)
	return(0);
      totFields += (dd->numFields);
      break;
    case DF_UINT32:  /* 4-byte units */
    case DF_INT32:
    case DF_REAL32:
      if(numBytes == 0)
	numBytes = 4;
      else if(numBytes != 4)
	return(0);
      totFields += (dd->numFields);
      break;
    case DF_UINT64:  /* 8-byte units */
    case DF_INT64:
    case DF_REAL64:
      if(numBytes == 0)
	numBytes = 8;
      else if(numBytes != 8)
	return(0);
      totFields += (dd->numFields);
      break;
/*     case DF_REAL80: */ /* 10-byte units */
/*       if(numBytes == 0) */
/* 	numBytes = 10; */
/*       else if(numBytes != 10) */
/* 	return(0); */
/*       totFields += (dd->numFields); */
/*       break; */
/*     case DF_REAL128:*/ /* 16-byte units */
/*       if(numBytes == 0) */
/* 	numBytes = 16; */
/*       else if(numBytes != 16) */
/* 	return(0); */
/*       totFields += (dd->numFields); */
/*       break; */
    default:
      setAsspMsg(AEG_ERR_BUG, "blockSwap: incorrect data format");
      return(-1);
    }
    dd = dd->next;                                  /* next parameter */
  }
  if(numUnits != NULL)
    *numUnits = totFields;
  return(numBytes);
}

/*DOC

Maps data of the record "recordNr" in the data buffer of the data object 
pointed to by "src" to that of the one pointed to by "dst". Only data 
for a descriptor item in "src" matching one in "dst" will be mapped. 
This function can thus be used both to extract certain parameter values 
from a packed multi-parameter array and to store them in such an array. 
It will automatically convert the value types where necessary. 
"recordNr" must be in the data range of the source buffer and in the 
buffer range of the destination buffer. Missing records in the 
destination buffer will be set to zero.
The function returns the number of data values mapped or -1 upon error.

Note:
 - At present, mapping is restricted to numerical data in 2's complement 
   or IEEE 754 floating point encoding ('DC_LIN'/'DC_PCM'). Encodings 
   like DC_BINOFF, DC_ALAW, etc. are NOT supported).
 - The function contains no checks for numerical overflow e.g. in the 
   mapping from float to integer. This and any other restriction on the 
   value range are considered to be the responsibility of the user.
 - The return value will be less than the total number of fields in "src" 
   if a) a descriptor in "src" doesn't match any in "dst", 
   or b) the number of fields for a descriptor in "src" is larger than 
   the one in the matching descriptor in "dst". These are not considered 
   to be errors.
 - This is a versatile function and therefore not exactly the fastest 
   one. If you are looking for speed, use a dedicated procedure.

DOC*/

long mapRecord(DOBJ *dst, DOBJ *src, long recordNr)
{
  size_t    numBytes, numFields, n;
  long      numMapped;
  char     *srcRec, *dstRec, *srcDat, *dstDat;
  uint8_t  *u8Val;
  int8_t   *i8Val;
  uint16_t *u16Val;
  int16_t  *i16Val;
  uint32_t *u32Val;
  int32_t  *i32Val;
  uint64_t *u64Val;
  int64_t  *i64Val;
  float    *f32Val;
  double   *f64Val;
  DDESC    *srcDD, *dstDD;

  if(TRACE[0]) {
    if(src == NULL || dst == NULL || recordNr < 0) {
      setAsspMsg(AEB_BAD_ARGS, "mapRecord");
      return(-1);
    }
    if(src->recordSize < 1 || dst->recordSize < 1 ||
       src->dataBuffer == NULL || dst->dataBuffer == NULL ||
       src->maxBufRecs < 1 || dst->maxBufRecs < 1) {
      setAsspMsg(AEB_BAD_CALL, "mapRecord");
      return(-1);
    }
  }
  if(dst->bufNumRecs < 1) {
    dst->bufStartRec = recordNr;
    dst->bufNumRecs = 0;
  }
  if(recordNr < src->bufStartRec ||
     recordNr >= (src->bufStartRec + src->bufNumRecs) ||
     recordNr < dst->bufStartRec ||
     recordNr >= (dst->bufStartRec + dst->maxBufRecs)) {
    setAsspMsg(AEB_BUF_RANGE, "(mapRecord)");
    return(-1);
  }
  srcRec = (char *)src->dataBuffer + (recordNr - src->bufStartRec)*(src->recordSize);
  if(recordNr >= (dst->bufStartRec + dst->bufNumRecs)) {
    /* zero missing records in destination buffer */
    numBytes = dst->recordSize;
    dstRec = (char *)dst->dataBuffer + (dst->bufNumRecs)*numBytes;
    while(recordNr >= (dst->bufStartRec + dst->bufNumRecs)) {
      memset(dstRec, 0, numBytes);
      (dst->bufNumRecs)++;
      dstRec += numBytes;
    }
  }
  dstRec = (char *)dst->dataBuffer + (recordNr - dst->bufStartRec)*(dst->recordSize);
  numMapped = 0;
  for(srcDD = &(src->ddl); srcDD != NULL; srcDD = srcDD->next) {
    dstDD = findDDesc(dst, srcDD->type, srcDD->ident);
    if(dstDD != NULL) { /* if not, no error */
      srcDat = srcRec + srcDD->offset;
      dstDat = dstRec + dstDD->offset;
      numFields = dstDD->numFields;
      if(dstDD->next != NULL)
	numBytes = dstDD->next->offset - dstDD->offset;
      else
	numBytes = dst->recordSize - dstDD->offset;
      if(dstDD->numFields > srcDD->numFields) {
	/* memset(dstDat, 0, numBytes); *//* clear so we don't have to worry later */
	/* caller should ensure that missing data have the desired values */
	numFields = srcDD->numFields;
	if(srcDD->next != NULL)
	  numBytes = srcDD->next->offset - srcDD->offset;
	else
	  numBytes = src->recordSize - srcDD->offset;
      }
      if(srcDD->format == dstDD->format &&
	 srcDD->coding == dstDD->coding) {
	memcpy(dstDat, srcDat, numBytes);           /* use blind copy */
      }
      else {
	if(srcDD->coding != DC_LIN || dstDD->coding != DC_LIN) {
	  setAsspMsg(AEB_BAD_CALL, "mapRecord");
	  return(-1);
	}
	switch(srcDD->format) {
	case DF_UINT8:
	  u8Val = (uint8_t *)srcDat;
	  switch(dstDD->format) {
	  case DF_INT8:
	    i8Val = (int8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i8Val[n] = (int8_t)u8Val[n];
	    break;
	  case DF_UINT16:
	    u16Val = (uint16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u16Val[n] = (uint16_t)u8Val[n];
	    break;
	  case DF_INT16:
	    i16Val = (int16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i16Val[n] = (int16_t)u8Val[n];
	    break;
	  case DF_UINT32:
	    u32Val = (uint32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u32Val[n] = (uint32_t)u8Val[n];
	    break;
	  case DF_INT32:
	    i32Val = (int32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i32Val[n] = (int32_t)u8Val[n];
	    break;
	  case DF_UINT64:
	    u64Val = (uint64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u64Val[n] = (uint64_t)u8Val[n];
	    break;
	  case DF_INT64:
	    i64Val = (int64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i64Val[n] = (int64_t)u8Val[n];
	    break;
	  case DF_REAL32:
	    f32Val = (float *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f32Val[n] = (float)u8Val[n];
	    break;
	  case DF_REAL64:
	    f64Val = (double *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f64Val[n] = (double)u8Val[n];
	    break;
	  default:
	    setAsspMsg(AED_NOHANDLE, "(mapRecord)");
	    return(-1);
	  }
	  break;
	case DF_INT8:
	  i8Val = (int8_t *)srcDat;
	  switch(dstDD->format) {
	  case DF_UINT8:
	    u8Val = (uint8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u8Val[n] = (uint8_t)i8Val[n];
	    break;
	  case DF_UINT16:
	    u16Val = (uint16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u16Val[n] = (uint16_t)i8Val[n];
	    break;
	  case DF_INT16:
	    i16Val = (int16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i16Val[n] = (int16_t)i8Val[n];
	    break;
	  case DF_UINT32:
	    u32Val = (uint32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u32Val[n] = (uint32_t)i8Val[n];
	    break;
	  case DF_INT32:
	    i32Val = (int32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i32Val[n] = (int32_t)i8Val[n];
	    break;
	  case DF_UINT64:
	    u64Val = (uint64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u64Val[n] = (uint64_t)i8Val[n];
	    break;
	  case DF_INT64:
	    i64Val = (int64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i64Val[n] = (int64_t)i8Val[n];
	    break;
	  case DF_REAL32:
	    f32Val = (float *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f32Val[n] = (float)i8Val[n];
	    break;
	  case DF_REAL64:
	    f64Val = (double *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f64Val[n] = (double)i8Val[n];
	    break;
	  default:
	    setAsspMsg(AED_NOHANDLE, "(mapRecord)");
	    return(-1);
	  }
	  break;
	case DF_UINT16:
	  u16Val = (uint16_t *)srcDat;
	  switch(dstDD->format) {
	  case DF_UINT8:
	    u8Val = (uint8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u8Val[n] = (uint8_t)u16Val[n];
	    break;
	  case DF_INT8:
	    i8Val = (int8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i8Val[n] = (int8_t)u16Val[n];
	    break;
	  case DF_INT16:
	    i16Val = (int16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i16Val[n] = (int16_t)u16Val[n];
	    break;
	  case DF_UINT32:
	    u32Val = (uint32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u32Val[n] = (uint32_t)u16Val[n];
	    break;
	  case DF_INT32:
	    i32Val = (int32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i32Val[n] = (int32_t)u16Val[n];
	    break;
	  case DF_UINT64:
	    u64Val = (uint64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u64Val[n] = (uint64_t)u16Val[n];
	    break;
	  case DF_INT64:
	    i64Val = (int64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i64Val[n] = (int64_t)u16Val[n];
	    break;
	  case DF_REAL32:
	    f32Val = (float *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f32Val[n] = (float)u16Val[n];
	    break;
	  case DF_REAL64:
	    f64Val = (double *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f64Val[n] = (double)u16Val[n];
	    break;
	  default:
	    setAsspMsg(AED_NOHANDLE, "(mapRecord)");
	    return(-1);
	  }
	  break;
	case DF_INT16:
	  i16Val = (int16_t *)srcDat;
	  switch(dstDD->format) {
	  case DF_UINT8:
	    u8Val = (uint8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u8Val[n] = (uint8_t)i16Val[n];
	    break;
	  case DF_INT8:
	    i8Val = (int8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i8Val[n] = (int8_t)i16Val[n];
	    break;
	  case DF_UINT16:
	    u16Val = (uint16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u16Val[n] = (uint16_t)i16Val[n];
	    break;
	  case DF_UINT32:
	    u32Val = (uint32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u32Val[n] = (uint32_t)i16Val[n];
	    break;
	  case DF_INT32:
	    i32Val = (int32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i32Val[n] = (int32_t)i16Val[n];
	    break;
	  case DF_UINT64:
	    u64Val = (uint64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u64Val[n] = (uint64_t)i16Val[n];
	    break;
	  case DF_INT64:
	    i64Val = (int64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i64Val[n] = (int64_t)i16Val[n];
	    break;
	  case DF_REAL32:
	    f32Val = (float *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f32Val[n] = (float)i16Val[n];
	    break;
	  case DF_REAL64:
	    f64Val = (double *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f64Val[n] = (double)i16Val[n];
	    break;
	  default:
	    setAsspMsg(AED_NOHANDLE, "(mapRecord)");
	    return(-1);
	  }
	  break;
	case DF_UINT32:
	  u32Val = (uint32_t *)srcDat;
	  switch(dstDD->format) {
	  case DF_UINT8:
	    u8Val = (uint8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u8Val[n] = (uint8_t)u32Val[n];
	    break;
	  case DF_INT8:
	    i8Val = (int8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i8Val[n] = (int8_t)u32Val[n];
	    break;
	  case DF_UINT16:
	    u16Val = (uint16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u16Val[n] = (uint16_t)u32Val[n];
	    break;
	  case DF_INT16:
	    i16Val = (int16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i16Val[n] = (int16_t)u32Val[n];
	    break;
	  case DF_INT32:
	    i32Val = (int32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i32Val[n] = (int32_t)u32Val[n];
	    break;
	  case DF_UINT64:
	    u64Val = (uint64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u64Val[n] = (uint64_t)u32Val[n];
	    break;
	  case DF_INT64:
	    i64Val = (int64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i64Val[n] = (int64_t)u32Val[n];
	    break;
	  case DF_REAL32:
	    f32Val = (float *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f32Val[n] = (float)u32Val[n];
	    break;
	  case DF_REAL64:
	    f64Val = (double *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f64Val[n] = (double)u32Val[n];
	    break;
	  default:
	    setAsspMsg(AED_NOHANDLE, "(mapRecord)");
	    return(-1);
	  }
	  break;
	case DF_INT32:
	  i32Val = (int32_t *)srcDat;
	  switch(dstDD->format) {
	  case DF_UINT8:
	    u8Val = (uint8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u8Val[n] = (uint8_t)i32Val[n];
	    break;
	  case DF_INT8:
	    i8Val = (int8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i8Val[n] = (int8_t)i32Val[n];
	    break;
	  case DF_UINT16:
	    u16Val = (uint16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u16Val[n] = (uint16_t)i32Val[n];
	    break;
	  case DF_INT16:
	    i16Val = (int16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i16Val[n] = (int16_t)i32Val[n];
	    break;
	  case DF_UINT32:
	    u32Val = (uint32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u32Val[n] = (uint32_t)i32Val[n];
	    break;
	  case DF_UINT64:
	    u64Val = (uint64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u64Val[n] = (uint64_t)i32Val[n];
	    break;
	  case DF_INT64:
	    i64Val = (int64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i64Val[n] = (int64_t)i32Val[n];
	    break;
	  case DF_REAL32:
	    f32Val = (float *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f32Val[n] = (float)i32Val[n];
	    break;
	  case DF_REAL64:
	    f64Val = (double *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f64Val[n] = (double)i32Val[n];
	    break;
	  default:
	    setAsspMsg(AED_NOHANDLE, "(mapRecord)");
	    return(-1);
	  }
          break;
	case DF_UINT64:
	  u64Val = (uint64_t *)srcDat;
	  switch(dstDD->format) {
	  case DF_UINT8:
	    u8Val = (uint8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u8Val[n] = (uint8_t)u64Val[n];
	    break;
	  case DF_INT8:
	    i8Val = (int8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i8Val[n] = (int8_t)u64Val[n];
	    break;
	  case DF_UINT16:
	    u16Val = (uint16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u16Val[n] = (uint16_t)u64Val[n];
	    break;
	  case DF_INT16:
	    i16Val = (int16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i16Val[n] = (int16_t)u64Val[n];
	    break;
	  case DF_UINT32:
	    u32Val = (uint32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u32Val[n] = (uint32_t)u64Val[n];
	    break;
	  case DF_INT32:
	    i32Val = (int32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i32Val[n] = (int32_t)u64Val[n];
	    break;
	  case DF_INT64:
	    i64Val = (int64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i64Val[n] = (int64_t)u64Val[n];
	    break;
	  case DF_REAL32:
	    f32Val = (float *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f32Val[n] = (float)u64Val[n];
	    break;
	  case DF_REAL64:
	    f64Val = (double *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f64Val[n] = (double)u64Val[n];
	    break;
	  default:
	    setAsspMsg(AED_NOHANDLE, "(mapRecord)");
	    return(-1);
	  }
          break;
	case DF_INT64:
	  i64Val = (int64_t *)srcDat;
	  switch(dstDD->format) {
	  case DF_UINT8:
	    u8Val = (uint8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u8Val[n] = (uint8_t)i64Val[n];
	    break;
	  case DF_INT8:
	    i8Val = (int8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i8Val[n] = (int8_t)i64Val[n];
	    break;
	  case DF_UINT16:
	    u16Val = (uint16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u16Val[n] = (uint16_t)i64Val[n];
	    break;
	  case DF_INT16:
	    i16Val = (int16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i16Val[n] = (int16_t)i64Val[n];
	    break;
	  case DF_UINT32:
	    u32Val = (uint32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u32Val[n] = (uint32_t)i64Val[n];
	    break;
	  case DF_INT32:
	    i32Val = (int32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i32Val[n] = (int32_t)i64Val[n];
	    break;
	  case DF_UINT64:
	    u64Val = (uint64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u64Val[n] = (uint64_t)i64Val[n];
	    break;
	  case DF_REAL32:
	    f32Val = (float *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f32Val[n] = (float)i64Val[n];
	    break;
	  case DF_REAL64:
	    f64Val = (double *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f64Val[n] = (double)i64Val[n];
	    break;
	  default:
	    setAsspMsg(AED_NOHANDLE, "(mapRecord)");
	    return(-1);
	  }
          break;
	case DF_REAL32:
	  f32Val = (float *)srcDat;
	  switch(dstDD->format) {
	  case DF_UINT8:
	    u8Val = (uint8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u8Val[n] = (uint8_t)myrint((double)f32Val[n]);
	    break;
	  case DF_INT8:
	    i8Val = (int8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i8Val[n] = (int8_t)myrint((double)f32Val[n]);
	    break;
	  case DF_UINT16:
	    u16Val = (uint16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u16Val[n] = (uint16_t)myrint((double)f32Val[n]);
	    break;
	  case DF_INT16:
	    i16Val = (int16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i16Val[n] = (int16_t)myrint((double)f32Val[n]);
	    break;
	  case DF_UINT32:
	    u32Val = (uint32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u32Val[n] = (uint32_t)myrint((double)f32Val[n]);
	    break;
	  case DF_INT32:
	    i32Val = (int32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i32Val[n] = (int32_t)myrint((double)f32Val[n]);
	    break;
	  case DF_UINT64:
	    u64Val = (uint64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u64Val[n] = (uint64_t)myrint((double)f32Val[n]);
	    break;
	  case DF_INT64:
	    i64Val = (int64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i64Val[n] = (int64_t)myrint((double)f32Val[n]);
	    break;
	  case DF_REAL64:
	    f64Val = (double *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f64Val[n] = (double)f32Val[n];
	    break;
	  default:
	    setAsspMsg(AED_NOHANDLE, "(mapRecord)");
	    return(-1);
	  }
          break;
	case DF_REAL64:
	  f64Val = (double *)srcDat;
	  switch(dstDD->format) {
	  case DF_UINT8:
	    u8Val = (uint8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u8Val[n] = (uint8_t)myrint(f64Val[n]);
	    break;
	  case DF_INT8:
	    i8Val = (int8_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i8Val[n] = (int8_t)myrint(f64Val[n]);
	    break;
	  case DF_UINT16:
	    u16Val = (uint16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u16Val[n] = (uint16_t)myrint(f64Val[n]);
	    break;
	  case DF_INT16:
	    i16Val = (int16_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i16Val[n] = (int16_t)myrint(f64Val[n]);
	    break;
	  case DF_UINT32:
	    u32Val = (uint32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u32Val[n] = (uint32_t)myrint(f64Val[n]);
	    break;
	  case DF_INT32:
	    i32Val = (int32_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i32Val[n] = (int32_t)myrint(f64Val[n]);
	    break;
	  case DF_UINT64:
	    u64Val = (uint64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      u64Val[n] = (uint64_t)myrint(f64Val[n]);
	    break;
	  case DF_INT64:
	    i64Val = (int64_t *)dstDat;
	    for(n = 0; n < numFields; n++)
	      i64Val[n] = (int64_t)myrint(f64Val[n]);
	    break;
	  case DF_REAL32:
	    f32Val = (float *)dstDat;
	    for(n = 0; n < numFields; n++)
	      f32Val[n] = (float)f64Val[n];
	    break;
	  default:
	    setAsspMsg(AED_NOHANDLE, "(mapRecord)");
	    return(-1);
	  }
          break;
	default:
	  setAsspMsg(AED_NOHANDLE, "(mapRecord)");
	  return(-1);
	}
      }
      numMapped += (long)numFields;
    }
  }
  if(numMapped > 0)
    dst->bufNeedsSave = TRUE;
  return(numMapped);
}

/*DOC

Function 'getSmpCaps'

Returns a set of 'AUC' flags, indicating which audio encodings are 
supported by the functions 'getSmpFrame' and getSmpPtr' when called 
with the output format "format". After adding the maximum number of 
channels which can be handled to the return value, it may directly 
be used as "auCaps" in a call to 'checkSound'.
Returns -1 if "format" is not supported by the relevant functions.

DOC*/

long getSmpCaps(dform_e format)
{
  long auCaps;

  switch(format) {
  case DF_INT32:
    auCaps = (0L) | AUC_I16 | AUC_I24 | AUC_I32;
    break;
  case DF_REAL32:
    auCaps = (0L) | AUC_I16 | AUC_I24 | AUC_I32 | AUC_F32;
    break;
  case DF_REAL64:
    auCaps = (0L) | AUC_I16 | AUC_I24 | AUC_I32 | AUC_F32 | AUC_F64;
    break;
  default:
    setAsspMsg(AEB_BAD_ARGS, "getSmpCaps");
    strcat(applMessage, " (unsupported format)");
    return(-1L);
  }
  return(auCaps | AUC_MSB_X);
}

/*DOC

Function 'getSmpFrame'

Converts the samples corresponding to the frame specified by "nr", 
"size" and "shift" from the audio object pointed to by "smpDOp" to 
the format given by "format" and copies them to the array pointed to 
by "frame". Arguments "head" and "tail" ( >= 0 ) should specify how 
many samples before and/or behind the actual frame need be included 
in the copy. For multi-track data "channel" should specify which 
track has to be copied (count starts at 1). If "smpDOp" refers to a 
file opened with 'asspFOpen' and the requested range is not in the 
data buffer of the object, it will be read from the file.
At least one sample in the kernel of the frame (i.e. as given by 
"shift") MUST be in the file or data buffer range. Samples, outside 
that range will be set to zero.
This function returns -1 upon error, otherwise 0.

Note:
 - It is the caller's responsibility that "frame" points to a memory 
   area large enough to hold "head" + "size" + "tail" samples in the 
   requested format.
 - If audio data encoded in floating point are normalized to a full 
   scale range from -1 to 1, the calling function may have to 
   denormalize them.
 - The function 'getSmpCaps' shows which "format" supports which audio 
   encodings.

DOC*/

int getSmpFrame(DOBJ *smpDOp, long nr, long size, long shift, long head,\
		long tail, int channel, void *frame, dform_e format)
{
  register size_t   recSize, numChans;
  register long     n, numCopy;
  register uint8_t *u8Ptr;
  int32_t *lPtr;      /* destination pointers for copy */
  float   *fPtr;
  double  *dPtr;
  int16_t *i16Ptr;    /* source pointers for input buffer */
  int32_t *i32Ptr;
  float   *f32Ptr;
  double  *f64Ptr;
  int    FILE_IN=FALSE;
  size_t dstSize=8;  /* have to initialize */
  size_t smpSize, numZeros, offset;
  long   absBegSn, absEndSn, bufBegSn, bufEndSn;
  long   begSn, endSn, frameSn, numRead;
  DDESC *dd;
  ENDIAN sysEndian={MSB};

  if(TRACE[0]) {
    if(smpDOp == NULL || nr < 0 || size < 1 || shift < 1 ||
       head < 0 || tail < 0 || frame == NULL) {
      setAsspMsg(AEB_BAD_ARGS, "getSmpFrame");
      return(-1);
    }
    if(smpDOp->recordSize <= 0 || smpDOp->dataBuffer == NULL ||
       smpDOp->maxBufRecs < 1) {
      setAsspMsg(AEB_BAD_CALL, "getSmpFrame");
      return(-1);
    }
  }
  dd = &(smpDOp->ddl);
  numChans = dd->numFields;
  if(numChans == 1 && channel < 1)
    channel = 1;
  else if(channel < 1 || channel > numChans) {
    setAsspMsg(AEB_BAD_ARGS, "getSmpFrame (invalid channel number)");
    return(-1);
  }
  recSize = smpDOp->recordSize;     /* have to intialize these anyway */
  smpSize = recSize / numChans;
  numCopy = head + size + tail;
  /* first check whether the range is already in the input buffer */
  /* (this should normally most often be the case) */
  bufBegSn = smpDOp->bufStartRec;  /* range currently in input buffer */
  bufEndSn = bufBegSn + smpDOp->bufNumRecs;
  frameSn = FRMNRtoSMPNR(nr, shift);
  begSn = frameSn - FRAMEHEAD(size, shift) - head; /* requested range */
  endSn = begSn + numCopy;
  if(begSn < bufBegSn || endSn > bufEndSn) {
    /* not fully in buffer; set the available input range */
    if(smpDOp->fp != NULL) {
      FILE_IN = TRUE;
      absBegSn = smpDOp->startRecord;
      absEndSn = absBegSn + smpDOp->numRecords;
    }
    else { /* MEMORY MODE */
      FILE_IN = FALSE;
      absBegSn = bufBegSn;
      absEndSn = bufEndSn;
    }
    if(frameSn >= absEndSn) {
      setAsspMsg(AEB_TOO_LATE, "(getSmpFrame)");
      return(-1);
    }
    if((frameSn + shift) <= absBegSn) {
      setAsspMsg(AEB_TOO_SOON, "(getSmpFrame)");
      return(-1);
    }
    switch(format) {
    case DF_INT32:
      dstSize = sizeof(int32_t);
      break;
    case DF_REAL32:
      dstSize = sizeof(float);
      break;
    case DF_REAL64:
      dstSize = sizeof(double);
      break;
    default:
      setAsspMsg(AEB_BAD_ARGS, "getSmpFrame (invalid target format)");
      return(-1);
    }
    if(begSn < absBegSn) {                       /* set leading zeros */
      numZeros = (size_t)(absBegSn - begSn);
      memset(frame, 0, numZeros * dstSize);     /* 0 fits all formats */
      begSn += (long)numZeros;           /* adjust range to be copied */
      numCopy -= (long)numZeros;
      frame = (void *)((char *)frame + (numZeros * dstSize));
    }
    if(endSn > absEndSn) {              /* need to add trailing zeros */
      numZeros = (size_t)(endSn - absEndSn);
      numCopy -= (long)numZeros;                 /* exclude from copy */
    }
    else
      numZeros = 0;
    if(FILE_IN && (begSn < bufBegSn || (begSn+numCopy) > bufEndSn)) {
      if(smpDOp->maxBufRecs < numCopy) {
	setAsspMsg(AEB_BUF_SPACE, "(getSmpFrame: input buffer)");
	return(-1);
      }
      /* reload the data buffer; optimized for sequential access */
      smpDOp->bufStartRec = begSn;       /* start as late as possible */
      if(asspFSeek(smpDOp, begSn) < 0)
	return(-1);
      numRead = smpDOp->maxBufRecs;        /* get as much as possible */
      if(begSn + numRead > absEndSn) {
	/* NOT CHECKED in asspFRead !! (there may be trailing chunks) */
	numRead = absEndSn - begSn;
      }
      if((numRead=asspFRead(smpDOp->dataBuffer, numRead, smpDOp)) < 0)
	return(-1);
      smpDOp->bufNumRecs = numRead;
      if(DIFFENDIAN(smpDOp->fileEndian, sysEndian)) {
	if(swapDataBuf(smpDOp) < 0)
	  return(-1);
      }
      bufBegSn = begSn;
      bufEndSn = bufBegSn + numRead;
    }
  }
  else /* full range is in the input buffer */
    numZeros = 0;
  if(numCopy > 0) {
    /* we can now extract 'numCopy' samples */
    /* from the data buffer starting at 'begSn' */
    offset = (size_t)(begSn - bufBegSn) * recSize;
    if(channel > 1)                          /* add offset to channel */
      offset += ((channel-1) * smpSize);
    u8Ptr = (uint8_t *)(smpDOp->dataBuffer) + offset;
    switch(format) {
    case DF_INT32:
      lPtr = (int32_t *)frame;
      switch(dd->format) {
      case DF_INT16:
	i16Ptr = (int16_t *)u8Ptr;
	for(n = 0; n < numCopy; n++) {
	  *(lPtr++) = (int32_t)(*i16Ptr);
	  i16Ptr += numChans;
	}
	break;
      case DF_INT24:
	for(n = 0; n < numCopy; n++) {
	  *(lPtr++) = int24_to_int32(u8Ptr);
	  u8Ptr += recSize;
	}
	break;
      case DF_INT32:
	i32Ptr = (int32_t *)u8Ptr;
	for(n = 0; n < numCopy; n++) {
	  *(lPtr++) = *i32Ptr;
	  i32Ptr += numChans;
	}
	break;
      default:
	setAsspMsg(AED_NOHANDLE, "(getSmpFrame)");
	return(-1);
      }
      break;
    case DF_REAL32:
      fPtr = (float *)frame;
      switch(dd->format) {
      case DF_INT16:
	i16Ptr = (int16_t *)u8Ptr;
	for(n = 0; n < numCopy; n++) {
	  *(fPtr++) = (float)(*i16Ptr);
	  i16Ptr += numChans;
	}
	break;
      case DF_INT24:
	for(n = 0; n < numCopy; n++) {
	  *(fPtr++) = (float)int24_to_int32(u8Ptr);
	  u8Ptr += recSize;
	}
	break;
      case DF_INT32:
	i32Ptr = (int32_t *)u8Ptr;
	for(n = 0; n < numCopy; n++) {
	  *(fPtr++) = (float)(*i32Ptr);
	  i32Ptr += numChans;
	}
	break;
      case DF_REAL32:
	f32Ptr = (float *)u8Ptr;
	for(n = 0; n < numCopy; n++) {
	  *(fPtr++) = *f32Ptr;
	  f32Ptr += numChans;
	}
	break;
      default:
	setAsspMsg(AED_NOHANDLE, "(getSmpFrame)");
	return(-1);
      }
      break;
    case DF_REAL64:
      dPtr = (double *)frame;
      switch(dd->format) {
      case DF_INT16:
	i16Ptr = (int16_t *)u8Ptr;
	for(n = 0; n < numCopy; n++) {
	  *(dPtr++) = (double)(*i16Ptr);
	  i16Ptr += numChans;
	}
	break;
      case DF_INT24:
	for(n = 0; n < numCopy; n++) {
	  *(dPtr++) = (double)int24_to_int32(u8Ptr);
	  u8Ptr += recSize;
	}
	break;
      case DF_INT32:
	i32Ptr = (int32_t *)u8Ptr;
	for(n = 0; n < numCopy; n++) {
	  *(dPtr++) = (double)(*i32Ptr);
	  i32Ptr += numChans;
	}
	break;
      case DF_REAL32:
	f32Ptr = (float *)u8Ptr;
	for(n = 0; n < numCopy; n++) {
	  *(dPtr++) = (double)(*f32Ptr);
	  f32Ptr += numChans;
	}
	break;
      case DF_REAL64:
	f64Ptr = (double *)u8Ptr;
	for(n = 0; n < numCopy; n++) {
	  *(dPtr++) = *f64Ptr;
	  f64Ptr += numChans;
	}
	break;
      default:
	setAsspMsg(AED_NOHANDLE, "(getSmpFrame)");
	return(-1);
      }
      break;
    default:
      setAsspMsg(AEB_BAD_ARGS, "getSmpFrame (invalid target format)");
      return(-1);
    }
  }
  else {
    setAsspMsg(AEB_BUF_RANGE, "(getSmpFrame)");
    return(-1);
  }
  if(numZeros > 0) {                            /* set trailing zeros */
    frame = (void *)((char *)frame + (numCopy * dstSize));            /* adjust target pointer */
    memset(frame, 0, numZeros * dstSize);
  }
  return(0);
}

/*DOC

Function 'getSmpPtr'

Verifies whether the buffer of the data object pointed to by "workDOp" 
contains the sample range "smpNr" - "head" to "smpNr" + "tail". If this 
is not the case, the buffer will be reloaded from the audio object 
pointed to by "smpDOp", if necessary and possible by reading from the 
file it refers to. While reloading, the input samples will be converted 
to the data format of the work object. If "smpDOp" refers to a multi-
channel signal, "channel" should specify which one to extract.
"smpNr" MUST be contained in the buffer/file range respectively of the 
input audio object. Samples values in "head" or "tail" outside that 
range will be set to zero. "workDOp" must point to a single-channel 
audio object.
The function returns a pointer to the location of the first byte of the 
sample "smpNr" in the data buffer of "workDOp" or NULL upon error.

Note:
 - For sequential rather than random access, the data buffer of "workDOp" 
   should be substantially larger than "head" + 1 + "tail" in order to 
   avoid unnecessary data/file transfers.
 - To process more than one channel in a multi-channel file, multiple 
   work objects should be used, one for each channel.
 - The function 'getSmpCaps' shows which output formats are allowed 
   and which input audio encodings they support.
 - If input data encoded in floating point are normalized to a full 
   scale range from -1 to 1, the calling function may have to 
   denormalize them.

DOC*/

void *getSmpPtr(DOBJ *smpDOp, long smpNr, long head, long tail,\
		int channel, DOBJ *workDOp)
{
  register size_t   recSize, numChans;
  register long     n, numCopy;
  register uint8_t *srcPtr, *dstPtr;
  int32_t *lPtr;     /* destination pointers for copy */
  float   *fPtr;
  double  *dPtr;
  int16_t *i16Ptr;   /* source pointers for input buffer */
  int32_t *i32Ptr;
  float   *f32Ptr;
  double  *f64Ptr;
  int      FILE_IN=FALSE;
  size_t   smpSize, dstSize, offset;
  long     absBegSn, absEndSn, reqBegSn, reqEndSn;
  long     bufBegSn, bufEndSn, copyBegSn, copyEndSn, numRead, numZeros;
  DDESC   *idd, *odd;
  ENDIAN   sysEndian={MSB};

  if(TRACE[0]) {
    if(smpDOp == NULL || smpNr < 0 || head < 0 || tail < 0 ||
       workDOp == NULL) {
      setAsspMsg(AEB_BAD_ARGS, "getSmpPtr");
      return(NULL);
    }
  }
  idd = &(smpDOp->ddl);
  odd = &(workDOp->ddl);
  if(TRACE[0]) {
    if(smpDOp->recordSize <= 0 ||
       smpDOp->dataBuffer == NULL ||
       smpDOp->maxBufRecs < 1) {
      setAsspMsg(AEB_BAD_CALL, "getSmpPtr: input object");
      return(NULL);
    }
    if(workDOp->recordSize <= 0 ||
       /* odd->type != DT_SMP ||  affilter has DT_FILTER */
       odd->numFields != 1 ||
       workDOp->dataBuffer == NULL) {
      setAsspMsg(AEB_BAD_CALL, "getSmpPtr: work object");
      return(NULL);
    }
    if(workDOp->maxBufRecs < (head+1+tail)) {
      setAsspMsg(AEB_BUF_SPACE, "getSmpPtr: work buffer");
      return(NULL);
    }
    if(odd->format != DF_INT32 && odd->format != DF_REAL32 &&
       odd->format != DF_REAL64) {
      setAsspMsg(AEB_BAD_ARGS, "getSmpPtr (invalid target format)");
      return(NULL);
    }
  }
  numChans = idd->numFields;
  if(numChans == 1 && channel < 1)
    channel = 1;
  else if(channel < 1 || channel > numChans) {
    setAsspMsg(AEB_BAD_ARGS, "getSmpPtr (invalid channel number)");
    return(NULL);
  }
  recSize = smpDOp->recordSize;     /* have to intialize these anyway */
  smpSize = recSize / numChans;
  dstSize = workDOp->recordSize;
  /* first check whether range is already in the work buffer */
  /* (this ought normally most often be the case) */
  if(workDOp->bufNumRecs <= 0) {        /* initialize if not yet done */
    workDOp->bufNumRecs = 0;
    workDOp->bufStartRec = smpNr - head;
  }
  bufBegSn = workDOp->bufStartRec;  /* range currently in work buffer */
  bufEndSn = bufBegSn + workDOp->bufNumRecs;
  reqBegSn = smpNr - head;                         /* requested range */
  reqEndSn = smpNr + 1 + tail;
  if(reqBegSn < bufBegSn || reqEndSn > bufEndSn) {
    /* not fully in work buffer; set the available input range */
    if(smpDOp->fp != NULL) {
      FILE_IN = TRUE;
      absBegSn = smpDOp->startRecord;
      absEndSn = absBegSn + smpDOp->numRecords;
    }
    else { /* MEMORY MODE */
      FILE_IN = FALSE;
      absBegSn = smpDOp->bufStartRec;
      absEndSn = absBegSn + smpDOp->bufNumRecs;
    }
    /* check whether requested sample number is valid */
    if(smpNr < absBegSn) {
      setAsspMsg(AEB_TOO_SOON, "(getSmpPtr)");
      return(NULL);
    }
    if(smpNr >= absEndSn) {
      setAsspMsg(AEB_TOO_LATE, "(getSmpPtr)");
      return(NULL);
    }
    /* check whether the requested range is in the input buffer */
    /* if not, reload input buffer */
    if(FILE_IN) {     /* otherwise per definition in the input buffer */
      copyBegSn = (reqBegSn >= absBegSn) ? reqBegSn : absBegSn;
      copyEndSn = (reqEndSn <= absEndSn) ? reqEndSn : absEndSn;
      if(copyBegSn < smpDOp->bufStartRec ||
	 copyEndSn > smpDOp->bufStartRec + smpDOp->bufNumRecs) {
	numCopy = copyEndSn - copyBegSn;
	if(smpDOp->maxBufRecs < numCopy) {
	  setAsspMsg(AEB_BUF_SPACE, "getSmpPtr: input buffer");
	  return(NULL);
	}
	smpDOp->bufStartRec = copyBegSn; /* start as late as possible */
	if(asspFSeek(smpDOp, copyBegSn) < 0)
	  return(NULL);
	numRead = smpDOp->maxBufRecs;      /* get as much as possible */
	if(copyBegSn + numRead > absEndSn) {
	/* NOT CHECKED in asspFRead !! (there may be trailing chunks) */
	  numRead = absEndSn - copyBegSn;
	}
	if((numRead=asspFRead(smpDOp->dataBuffer, numRead, smpDOp)) < 0)
	  return(NULL);
	smpDOp->bufNumRecs = numRead;
	if(DIFFENDIAN(smpDOp->fileEndian, sysEndian)) {
	  if(swapDataBuf(smpDOp) < 0)
	    return(NULL);
	}
      }
    }
    /* check whether requested range would still fit the work buffer */
    /* if not, discard contents and align buffer */
    if(reqBegSn < bufBegSn ||
       reqEndSn > bufBegSn + workDOp->maxBufRecs) {
      workDOp->bufNumRecs = 0;
      workDOp->bufStartRec = bufBegSn = bufEndSn = reqBegSn;
    }
    /* now comes the tricky part: when to copy what where? */
    /* first check whether leading zeros have to be set */
    numZeros = absBegSn - bufEndSn;
    if(numZeros > 0) {
      offset = (size_t)(workDOp->bufNumRecs) * dstSize;
      dstPtr = (uint8_t *)(workDOp->dataBuffer) + offset;
      memset((void *)dstPtr, 0, (size_t)numZeros * dstSize);
      workDOp->bufNumRecs += numZeros;        /* adjust buffer ranges */
      bufEndSn += numZeros;
    }
    /* next, determine how many samples should/could be copied */
    copyBegSn = bufEndSn;             /* start at end of buffer range */
    if(copyBegSn < smpDOp->bufStartRec) {
      setAsspMsg(AEB_TOO_SOON, "(getSmpPtr)");
      return(NULL);
    }
    copyEndSn = smpDOp->bufStartRec + smpDOp->bufNumRecs;/* available */
    if(copyEndSn > (bufBegSn + workDOp->maxBufRecs))
      copyEndSn = bufBegSn + workDOp->maxBufRecs;         /* possible */
    numCopy = copyEndSn - copyBegSn;
    if(numCopy > 0) {
      offset = (size_t)(copyBegSn - smpDOp->bufStartRec) * recSize;
      if(channel > 1)
	offset += ((channel-1) * smpSize);
      srcPtr = (uint8_t *)(smpDOp->dataBuffer) + offset;
      offset = (size_t)(workDOp->bufNumRecs) * dstSize;
      dstPtr = (uint8_t *)(workDOp->dataBuffer) + offset;
      switch(odd->format) {
      case DF_INT32:
	lPtr = (int32_t *)dstPtr;
	switch(idd->format) {
	case DF_INT16:
	  i16Ptr = (int16_t *)srcPtr;
	  for(n = 0; n < numCopy; n++) {
	    *(lPtr++) = (int32_t)(*i16Ptr);
	    i16Ptr += numChans;
	  }
	  break;
	case DF_INT24:
	  for(n = 0; n < numCopy; n++) {
	    *(lPtr++) = int24_to_int32(srcPtr);
	    srcPtr += recSize;
	  }
	  break;
	case DF_INT32:
	  i32Ptr = (int32_t *)srcPtr;
	  for(n = 0; n < numCopy; n++) {
	    *(lPtr++) = *i32Ptr;
	    i32Ptr += numChans;
	  }
	  break;
	default:
	  setAsspMsg(AED_NOHANDLE, "(getSmpPtr)");
	  return(NULL);
	}
	break;
      case DF_REAL32:
	fPtr = (float *)dstPtr;
	switch(idd->format) {
	case DF_INT16:
	  i16Ptr = (int16_t *)srcPtr;
	  for(n = 0; n < numCopy; n++) {
	    *(fPtr++) = (float)(*i16Ptr);
	    i16Ptr += numChans;
	  }
	  break;
	case DF_INT24:
	  for(n = 0; n < numCopy; n++) {
	    *(fPtr++) = (float)int24_to_int32(srcPtr);
	    srcPtr += recSize;
	  }
	  break;
	case DF_INT32:
	  i32Ptr = (int32_t *)srcPtr;
	  for(n = 0; n < numCopy; n++) {
	    *(fPtr++) = (float)(*i32Ptr);
	    i32Ptr += numChans;
	  }
	  break;
	case DF_REAL32:
	  f32Ptr = (float *)srcPtr;
	  for(n = 0; n < numCopy; n++) {
	    *(fPtr++) = *f32Ptr;
	    f32Ptr += numChans;
	  }
	  break;
	default:
	  setAsspMsg(AED_NOHANDLE, "(getSmpPtr)");
	  return(NULL);
	}
	break;
      case DF_REAL64:
	dPtr = (double *)dstPtr;
	switch(idd->format) {
	case DF_INT16:
	  i16Ptr = (int16_t *)srcPtr;
	  for(n = 0; n < numCopy; n++) {
	    *(dPtr++) = (double)(*i16Ptr);
	    i16Ptr += numChans;
	  }
	  break;
	case DF_INT24:
	  for(n = 0; n < numCopy; n++) {
	    *(dPtr++) = (double)int24_to_int32(srcPtr);
	    srcPtr += recSize;
	  }
	  break;
	case DF_INT32:
	  i32Ptr = (int32_t *)srcPtr;
	  for(n = 0; n < numCopy; n++) {
	    *(dPtr++) = (double)(*i32Ptr);
	    i32Ptr += numChans;
	  }
	  break;
	case DF_REAL32:
	  f32Ptr = (float *)srcPtr;
	  for(n = 0; n < numCopy; n++) {
	    *(dPtr++) = (double)(*f32Ptr);
	    f32Ptr += numChans;
	  }
	  break;
	case DF_REAL64:
	  f64Ptr = (double *)srcPtr;
	  for(n = 0; n < numCopy; n++) {
	    *(dPtr++) = *f64Ptr;
	    f64Ptr += numChans;
	  }
	  break;
	default:
	  setAsspMsg(AED_NOHANDLE, "(getSmpPtr)");
	  return(NULL);
	}
	break;
      default:
	setAsspMsg(AEB_BAD_ARGS, "getSmpPtr (invalid target format)");
	return(NULL);
      }
      workDOp->bufNumRecs += numCopy;          /* adjust buffer ranges */
      bufEndSn += numCopy;
    }
    /* finally, determine whether to set trailing zeros */
    if(reqEndSn > absEndSn) {
      numZeros = absEndSn + tail - bufEndSn;   /* the most we'll need */
      if(numZeros > 0) {
	if(bufEndSn + numZeros > bufBegSn + workDOp->maxBufRecs)
	  numZeros = bufBegSn + workDOp->maxBufRecs - bufEndSn;
	if(numZeros > 0) {
	  offset = (size_t)(workDOp->bufNumRecs) * dstSize;
	  dstPtr = (uint8_t *)(workDOp->dataBuffer) + offset;
	  memset((void *)dstPtr, 0, (size_t)numZeros * dstSize);
	  workDOp->bufNumRecs += numZeros;     /* adjust buffer ranges */
	  bufEndSn += numZeros;
	}
      }
    }
  }
  offset = (size_t)(smpNr - bufBegSn) * dstSize;
  dstPtr = (uint8_t *)(workDOp->dataBuffer) + offset;
  return((void *)dstPtr);
}
