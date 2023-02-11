/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 2010  Michel Scheffers                          *
*                            IPdS, CAU Kiel                            *
*                            Leibnizstr. 10                            *
*                            24118 Kiel, Germany                       *
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
* File:     zcr.c                                                      *
* Contents: Functions for the analysis of zero-crossing rates.         *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: zcr.c,v 1.15 2010/07/14 13:47:04 mtms Exp $ */

#include <stdio.h>     /* NULL */
#include <stdlib.h>    /* malloc() calloc() free() */
#include <string.h>    /* memset() strcpy() strchr() memcpy() */
#include <inttypes.h>  /* int16_t */

#include <miscdefs.h>  /* TRUE FALSE LOCAL NATIVE_EOL */
#include <misc.h>      /* strxcmp() */
#include <trace.h>     /* trace handler */
#include <asspmess.h>  /* error message handler */
#include <assptime.h>  /* standard conversion macros */
#include <asspana.h>   /* AOPTS anaTiming() (includes zcr.h) */
#include <asspdsp.h>   /* getZCR() */
#include <asspfio.h>   /* asspFFlush() */
#include <dataobj.h>   /* DOBJ getSmpCaps() getSmpFrame() */
#include <headers.h>   /* KDTAB */
#include <aucheck.h>   /* checkSound() */

/*
 * local global variables and arrays
 */
LOCAL double *frame=NULL; /* frame buffer (allocated) */

/*
 * prototypes of private functions
 */
LOCAL int  setGlobals(DOBJ *dop);
LOCAL void freeGlobals(void);
LOCAL int  storeZCR(float *vals, long frameNr, DOBJ *dop);

/* ======================== public functions ======================== */

/*DOC

Function 'setZCRdefaults'

Sets the items in the analysis options structure relevant to the analysis 
of the zero-crossing rates of a signal to their default values. Clears 
all other items.
Returns 0 upon success and -1 upon error.

DOC*/

int setZCRdefaults(AOPTS *aoPtr)
{
  if(aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setZCRdefaults");
    return(-1);
  }
  memset((void *)aoPtr, 0, sizeof(AOPTS));         /* clear all items */
  aoPtr->msSize = ZCR_DEF_SIZE;                    /* frame size (ms) */
  aoPtr->msShift = ZCR_DEF_SHIFT;                 /* frame shift (ms) */
  aoPtr->channel = ZCR_DEF_CHANNEL;   /* default multi-channel in/out */
  aoPtr->precision = ZCR_DEF_DIGITS;    /* precision for ASCII output */
  strcpy(aoPtr->format, ZCR_DEF_FORMAT);        /* output file format */
  return(0);
}

/*DOC

Function 'createZCR'

Allocates memory for a data object to hold zero-crossing analysis data 
and initializes it on the basis of the information in the audio object 
pointed to by "smpDOp" and the analysis options in the structure pointed 
to by "aoPtr". Neither of these pointers may be NULL.
Returns a pointer to the data object or NULL upon error.

Note:
 - This function DOES NOT allocate memory for the data buffer. Depending 
   on your application you can either use 'allocDataBuf' for this or 
   delegate it to 'computeZCR'.

DOC*/

DOBJ *createZCR(DOBJ *smpDOp, AOPTS *aoPtr)
{
  long    auCaps;
  ATIME   aTime, *tPtr;
  DOBJ   *dop=NULL;
  DDESC  *dd=NULL;
  ZCR_GD *gd=NULL;
  KDTAB  *entry;

  if(smpDOp == NULL || aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "createZCR");
    return(NULL);
  }
  clrAsspMsg();
  /* verify audio object */
  if((auCaps=getSmpCaps(ZCR_PFORMAT)) <= 0)
    return(NULL);
  auCaps |= ZCR_I_CHANS;
  if(aoPtr->channel < 0)
    aoPtr->channel = 0;
  if(checkSound(smpDOp, auCaps, aoPtr->channel) <= 0)
    return(NULL);
  tPtr = &aTime;
  if(anaTiming(smpDOp, aoPtr, tPtr) < 0)      /* ignore warnings here */
    return(NULL);
  if((gd=(ZCR_GD *)malloc(sizeof(ZCR_GD))) == NULL) {
    setAsspMsg(AEG_ERR_MEM, "createZCR");
    return(NULL);
  }
  strcpy(gd->ident, ZCR_GD_IDENT);
  gd->options = aoPtr->options;
  gd->frameSize = tPtr->frameSize;
  gd->begFrameNr = tPtr->begFrameNr;
  gd->endFrameNr = tPtr->endFrameNr;
  gd->channel = aoPtr->channel;
  gd->precision = aoPtr->precision;

  if((dop=allocDObj()) == NULL) {
    freeZCR_GD((void *)gd);
    return(NULL);
  }
  if(strxcmp(aoPtr->format, "SSFF") == 0) {
    dop->fileFormat = FF_SSFF;
    dop->fileData = FDF_BIN;
    strcpy(dop->eol, SSFF_EOL_STR);
  }
  else { /* fall through to raw ASCII */
    dop->fileFormat = FF_RAW;
    dop->fileData = FDF_ASC;
    strcpy(dop->eol, NATIVE_EOL);
  }
  SETENDIAN(dop->fileEndian);
  dop->sampFreq = tPtr->sampFreq;
  dop->frameDur = tPtr->frameShift;
  dop->startRecord = gd->begFrameNr;
  dop->numRecords = 0;                         /* nothing written yet */
  dop->generic = (void *)gd;
  dop->doFreeGeneric = (DOfreeFunc)freeZCR_GD;
  dd = &(dop->ddl);                 /* set pointer to data descriptor */
  dd->type = DT_ZCR;
  dd->coding = DC_LIN;
  dd->format = ZCR_DFORMAT;
  if(gd->channel < 1)
    dd->numFields = smpDOp->ddl.numFields;  /* possibly multi-channel */
  else
    dd->numFields = 1;                       /* only selected channel */
  if(dop->fileFormat == FF_SSFF) {
    entry = dtype2entry(dd->type, KDT_SSFF);   /* search SSFF keyword */
    if(entry != NULL && entry->keyword != NULL) {
      dd->ident = strdup(entry->keyword);
      if(entry->factor != NULL)
	strcpy(dd->factor, entry->factor);
      if(entry->unit != NULL)
	strcpy(dd->unit, entry->unit);
    }
    else {
      dop = freeDObj(dop);
      setAsspMsg(AEB_ERR_TRACK, "(createZCR)");
      return(dop);
    }
  }
  else { /* fall through to raw ASCII */
    strcpy(dop->sepChars, "\t");                    /* between fields */
    strcpy(dd->sepChars, " ");                        /* within field */
    snprintf(dd->ascFormat, member_size(DDESC, ascFormat), "%%.%df", gd->precision);
  }
  setRecordSize(dop);
  setStart_Time(dop);
  return(dop);
}

/*DOC

Function 'computeZCR'

Performs a zero-crossing analysis of the audio signal referred to by 
"smpDOp" using the parameter settings specified in the generic data 
structure of the output data object pointed to by "zcrDOp". If "zcrDOp" 
is a NULL-pointer "aoPtr" may not be a NULL-pointer and this function 
will create the output data object (see 'createZCR' for details).
If "aoPtr" is not a NULL-pointer this function will verify and - if 
necessary - (re-)allocate the data buffers in the data objects pointed 
to by "smpDOp" and "zcrDOp" to have appropriate size.
Analysis results will be returned in the data buffer of the object 
pointed to by "zcrDOp" or written to file if that object refers to a 
file opened for writing. 
Returns a pointer to the zero-crossing data object or NULL upon error.

Note:
 - This function may be used in a file-to-file, file-to-memory, memory-
   to-file and memory-to-memory mode.
 - This function DOES NOT verify whether the analysis parameters in the 
   generic data structure of "zcrDOp" comply with those in the general 
   analysis structure pointed to by "aoPtr". You should use the function 
   'verifyZCR' if parameter changes may occur between calls.

DOC*/

DOBJ *computeZCR(DOBJ *smpDOp, AOPTS *aoPtr, DOBJ *zcrDOp)
{
  int     FILE_IN, FILE_OUT, CREATED;
  int     err, cn, numChans;
  long    fn, frameSize, frameShift, numSamples;
  float   zxRate[ZCR_O_CHANS];
  ZCR_GD *gd;

  if(smpDOp == NULL || (aoPtr == NULL && zcrDOp == NULL)) {
    setAsspMsg(AEB_BAD_ARGS, "computeZCR");
    return(NULL);
  }
  FILE_IN = FILE_OUT = CREATED = FALSE;
  /* check input object */
  if(smpDOp->fp != NULL) {
    if(smpDOp->numRecords <= 0) {
      setAsspMsg(AEF_EMPTY, smpDOp->filePath);
      return(NULL);
    }
    FILE_IN = TRUE;
  }
  else if(aoPtr == NULL ||\
	  (aoPtr != NULL && !(aoPtr->options & AOPT_INIT_ONLY))) {
    if(smpDOp->dataBuffer == NULL || smpDOp->bufNumRecs <= 0) {
      setAsspMsg(AED_NO_DATA, "(computeZCR)");
      return(NULL);
    }
  }
  /* check status of output object */
  if(zcrDOp == NULL) {
    if((zcrDOp=createZCR(smpDOp, aoPtr)) == NULL)
      return(NULL);
    CREATED = TRUE;
  }
  gd = (ZCR_GD *)zcrDOp->generic;
  if(zcrDOp->fp != NULL) {
    FILE_OUT = TRUE;
    gd->writeOpts = AFW_CLEAR;          /* discard data after writing */
    if(zcrDOp->fileData == FDF_ASC)
      gd->writeOpts |= AFW_ADD_TIME;
  }
  else
    gd->writeOpts = AFW_KEEP;

  if(setGlobals(zcrDOp) < 0) {
    if(CREATED)
      freeDObj(zcrDOp);
    return(NULL);
  }
  frameSize = gd->frameSize;
  frameShift = zcrDOp->frameDur;
  numSamples = frameSize + ZCR_HEAD + ZCR_TAIL;
  if(aoPtr != NULL) {
    if(checkDataBufs(smpDOp, zcrDOp, numSamples,\
		     gd->begFrameNr, gd->endFrameNr) < 0) {
      if(CREATED)
	freeDObj(zcrDOp);
      return(NULL);
    }
    if(aoPtr->options & AOPT_INIT_ONLY) {
      aoPtr->options &= ~AOPT_INIT_ONLY;                /* clear flag */
      return(zcrDOp);                                  /* no analysis */
    }
  }
  numChans = (int)(zcrDOp->ddl.numFields);
  if(TRACE['A']) {
    fprintf(traceFP, "Analysis parameters\n");
    fprintf(traceFP, "  sample rate = %.1f Hz\n", zcrDOp->sampFreq);
    fprintf(traceFP, "  window size = %ld samples\n", frameSize);
    fprintf(traceFP, "  window shift = %ld samples\n", frameShift);
    if(gd->channel < 1)
      fprintf(traceFP, "  number of channels = %d\n", numChans);
    else
      fprintf(traceFP, "  selected channel = %d\n", gd->channel);
    fprintf(traceFP, "  start frame = %ld\n", gd->begFrameNr);
    fprintf(traceFP, "  end frame = %ld\n", gd->endFrameNr);
    fprintf(traceFP, "  processing mode = %s-to-%s\n",\
	    FILE_IN ? "file" : "memory", FILE_OUT ? "file" : "memory");
  }
  /* loop over frames */
  for(err = 0, fn = gd->begFrameNr; fn < gd->endFrameNr; fn++) {
    /* loop over channels */
    for(cn = 0; cn < numChans; cn++) {
      if(gd->channel > 0) { /* single channel */
        if((err=getSmpFrame(smpDOp, fn, frameSize, frameShift, ZCR_HEAD,\
			    ZCR_TAIL, gd->channel, frame, ZCR_PFORMAT)) < 0)
	  break;
      }
      else { /* possibly multi-channel */
	if((err=getSmpFrame(smpDOp, fn, frameSize, frameShift, ZCR_HEAD,\
			    ZCR_TAIL, cn+1, frame, ZCR_PFORMAT)) < 0)
	  break;
      }
      zxRate[cn] = (float)getZCR(frame, numSamples, smpDOp->sampFreq);
    } /* END loop over channels */
    if(err < 0) break;
    if((err=storeZCR(zxRate, fn, zcrDOp)) < 0) break;
  } /* END loop over frames */
  if(err >= 0 && FILE_OUT)
    err = asspFFlush(zcrDOp, gd->writeOpts);
  freeGlobals();
  if(err < 0) {
    if(CREATED)
      freeDObj(zcrDOp);
    return(NULL);
  }
  return(zcrDOp);
}

/*DOC

Function 'verifyZCR'

Verifies whether the analysis parameters of the zero-crossing data 
object pointed to by "zcrDOp" differ from those specified in the general 
analysis parameter structure pointed to by "aoPtr" for the audio object 
pointed to by "smpDOp". None of the three pointers may be NULL. If there 
are differences, it is checked whether they lead to clashes with 
existing analysis data or header settings or require data buffers to be 
reallocated. If there are no clashes, the items in the generic data 
structure will be updated to match the general analysis parameter 
settings.
This function should be called when between a call to 'createZCR' and to 
'computeZCR', changes have been made in the analysis parameters or the 
audio object (e.g. a length change). 
Returns 0 when there are no problems, -1 upon error or +1 if there are 
warnings.

DOC*/

int verifyZCR(DOBJ *zcrDOp, DOBJ *smpDOp, AOPTS *aoPtr)
{
  int     err;
  long    auCaps;
  double  frameRate;
  DDESC  *dd;
  ZCR_GD *gd;
  ATIME   aTime, *tPtr;

  if(zcrDOp == NULL || smpDOp == NULL || aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "verifyZCR");
    return(-1);
  }
  dd = &(zcrDOp->ddl);
  if(dd->type != DT_ZCR || dd->format != ZCR_DFORMAT || dd->next != NULL) {
    setAsspMsg(AEG_ERR_BUG, "Not a regular ZCR object");
    return(-1);
  }
  gd = (ZCR_GD *)(zcrDOp->generic);
  if(gd == NULL ||
     (gd != NULL && strcmp(gd->ident, ZCR_GD_IDENT) != 0)) {
    setAsspMsg(AEG_ERR_BUG, "ZCR generic data missing or invalid");
    return(-1);
  }
  clrAsspMsg();
  /* verify audio object */
  if(aoPtr->channel < 0)
    aoPtr->channel = 0;
  if((auCaps=getSmpCaps(ZCR_PFORMAT)) <= 0)
    return(-1);
  auCaps |= ZCR_I_CHANS;
  if(checkSound(smpDOp, auCaps, aoPtr->channel) <= 0)
    return(-1);
  tPtr = &aTime;
  if((err=anaTiming(smpDOp, aoPtr, tPtr)) < 0)
    return(-1);
  if(zcrDOp->sampFreq != tPtr->sampFreq ||\
     zcrDOp->frameDur != tPtr->frameShift) {
    frameRate = tPtr->sampFreq / (double)(tPtr->frameShift);
    if(zcrDOp->dataRate != frameRate) {
      if(zcrDOp->fp != NULL &&\
	 (zcrDOp->numRecords > 0 || ftell(zcrDOp->fp) != 0)) {
	setAsspMsg(AED_ERR_RATE, zcrDOp->filePath);
	return(-1);
      }
      clearDataBuf(zcrDOp);/* contents invalid; size maybe sufficient */
    }
    zcrDOp->sampFreq = tPtr->sampFreq;
    zcrDOp->frameDur = tPtr->frameShift;
    zcrDOp->dataRate = frameRate;
    zcrDOp->startRecord = tPtr->begFrameNr;
    zcrDOp->numRecords = 0;
    setStart_Time(zcrDOp);
  }
  if((aoPtr->channel < 1 && dd->numFields != smpDOp->ddl.numFields) ||\
     (aoPtr->channel > 0 && dd->numFields != 1) ||\
     (aoPtr->channel != gd->channel) ) {
    if(zcrDOp->fp != NULL &&\
       (zcrDOp->numRecords > 0 || ftell(zcrDOp->fp) != 0)) {
      setAsspMsg(AEG_ERR_APPL, "verifyZCR: can't change channels "\
		 "in existing data");
      return(-1);
    }
    if((aoPtr->channel < 1 && dd->numFields != smpDOp->ddl.numFields) ||\
       (aoPtr->channel > 0 && dd->numFields != 1) ) {
      if(zcrDOp->doFreeDataBuf == NULL) {
	setAsspMsg(AEG_ERR_APPL, "verifyZCR: can't reallocate data buffer");
	return(-1);
      }
      freeDataBuf(zcrDOp);  /* will be reallocated in checkDataBufs() */
    }
    else
      clearDataBuf(zcrDOp);                       /* contents invalid */
    if(aoPtr->channel < 1)
      dd->numFields = smpDOp->ddl.numFields;
    else
      dd->numFields = 1;
    setRecordSize(zcrDOp);
  }
  if(tPtr->begFrameNr < zcrDOp->startRecord) {
    if(zcrDOp->fp != NULL &&\
       (zcrDOp->numRecords > 0 || ftell(zcrDOp->fp) != 0)) {
      setAsspMsg(AEG_ERR_APPL, "verifyZCR: can't change start time "\
		 "in existing data");
      return(-1);
    }
    zcrDOp->startRecord = tPtr->begFrameNr;
    zcrDOp->numRecords = 0;
    setStart_Time(zcrDOp);
  }
  /* now copy relevant parameters */
  gd->options = aoPtr->options;
  gd->frameSize = tPtr->frameSize;
  gd->begFrameNr = tPtr->begFrameNr;
  gd->endFrameNr = tPtr->endFrameNr;
  gd->channel = aoPtr->channel;
  gd->precision = aoPtr->precision;
  return(err);
}

/*DOC

Function 'freeZCR_GD'

Returns all memory allocated for the generic data in a ZCR data object.

DOC*/

void freeZCR_GD(void *generic)
{
  if(generic != NULL) {
    free(generic);
  }
  return;
}

/* ======================= private  functions ======================= */

/***********************************************************************
* allocate memory for the frame buffer                                 *
***********************************************************************/
LOCAL int setGlobals(DOBJ *dop)
{
  size_t  bufSize;
  ZCR_GD *gd;

  frame = NULL;
  gd = (ZCR_GD *)(dop->generic);
  bufSize = (size_t)(gd->frameSize + ZCR_HEAD + ZCR_TAIL);
  frame = (double *)calloc(bufSize, sizeof(double));
  if(frame == NULL) {
    setAsspMsg(AEG_ERR_MEM, "ZCR: setGlobals");
    return(-1);
  }
  return(0);
}

/***********************************************************************
* return memory allocated for the global arrays                        *
***********************************************************************/
LOCAL void freeGlobals(void)
{
  if(frame != NULL) {
    free((void *)frame);
    frame = NULL;
  }
  return;
}

/***********************************************************************
* copy frame data to output buffer; handle data writes                 *
***********************************************************************/
LOCAL int storeZCR(float *vals, long frameNr, DOBJ *dop)
{
  int     FILE_OUT;
  size_t  chans;
  long    ndx;
  float  *fPtr;
  ZCR_GD *gd;

  FILE_OUT = (dop->fp != NULL);
  gd = (ZCR_GD *)dop->generic;
  if(dop->bufNumRecs <= 0) {
    dop->bufNumRecs = 0;
    dop->bufStartRec = frameNr;
  }
  else if(frameNr >= (dop->bufStartRec + dop->maxBufRecs)) {
    if(FILE_OUT) {
      if(asspFFlush(dop, gd->writeOpts) < 0)
	return(-1);
    }
    else {
      setAsspMsg(AEG_ERR_BUG, "storeZCR: buffer overflow");
      return(-1);
    }
  }
  chans = dop->ddl.numFields;
  ndx = frameNr - dop->bufStartRec;
  fPtr = (float *)(dop->dataBuffer);
  fPtr = &fPtr[ndx * chans];
  memcpy((void *)fPtr, (void *)vals, chans * sizeof(float));
  if(ndx >= dop->bufNumRecs)
    dop->bufNumRecs = ndx + 1;
  dop->bufNeedsSave = TRUE;
  return(0);
}
