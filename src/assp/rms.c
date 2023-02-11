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
* File:     rms.c                                                      *
* Contents: Functions for the analysis of Root Mean Square amplitudes. *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: rms.c,v 1.18 2010/07/14 13:47:04 mtms Exp $ */

#include <stdio.h>     /* NULL */
#include <stdlib.h>    /* malloc() calloc() free() */
#include <string.h>    /* memset() strcpy() strchr() memcpy() */
#include <inttypes.h>  /* int16_t */

#include <miscdefs.h>  /* TRUE FALSE LOCAL NATIVE_EOL */
#include <misc.h>      /* strxcmp() */
#include <trace.h>     /* trace handler */
#include <asspmess.h>  /* error message handler */
#include <assptime.h>  /* standard conversion macros */
#include <asspana.h>   /* AOPTS anaTiming() (includes rms.h) */
#include <asspdsp.h>   /* makeWF() freeWF() mulSigWF() getRMS() */
#include <asspfio.h>   /* asspFFlush() */
#include <dataobj.h>   /* DOBJ getSmpCaps() getSmpFrame() */
#include <headers.h>   /* KDTAB */
#include <aucheck.h>   /* checkSound() */

/*
 * local global variables and arrays
 */
LOCAL double *frame=NULL; /* frame buffer (allocated) */
LOCAL double *wfc=NULL;   /* window function coefficients (allocated) */

/*
 * prototypes of private functions
 */
LOCAL int  setGlobals(DOBJ *dop);
LOCAL void freeGlobals(void);
LOCAL int  storeRMS(float *vals, long frameNr, DOBJ *dop);

/* ======================== public functions ======================== */

/*DOC

Function 'setRMSdefaults'

Sets the items in the analysis options structure relevant to the analysis 
of the RMS amplitude course of a signal to their default values. Clears 
all other items.
Returns 0 upon success and -1 upon error.

DOC*/

int setRMSdefaults(AOPTS *aoPtr)
{
  if(aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setRMSdefaults");
    return(-1);
  }
  memset((void *)aoPtr, 0, sizeof(AOPTS));         /* clear all items */
  aoPtr->msSize = RMS_DEF_SIZE;                    /* frame size (ms) */
  aoPtr->options = AOPT_EFFECTIVE;              /* use effective size */
  aoPtr->msShift = RMS_DEF_SHIFT;                 /* frame shift (ms) */
  aoPtr->channel = RMS_DEF_CHANNEL;   /* default multi-channel in/out */
  aoPtr->precision = RMS_DEF_DIGITS;    /* digits precision for ASCII */
  strcpy(aoPtr->format, RMS_DEF_FORMAT);        /* output file format */
  strcpy(aoPtr->winFunc, RMS_DEF_WINDOW);          /* window function */
  return(0);
}

/*DOC

Function 'createRMS'

Allocates memory for a data object to hold RMS amplitude analysis data 
and initializes it on the basis of the information in the audio object 
pointed to by "smpDOp" and the analysis options in the structure pointed 
to by "aoPtr". Neither of these pointers may be NULL.
Returns a pointer to the data object or NULL upon error.

Note:
 - This function DOES NOT allocate memory for the data buffer. Depending 
   on your application you can either use 'allocDataBuf' for this or 
   delegate it to 'computeRMS'.

DOC*/

DOBJ *createRMS(DOBJ *smpDOp, AOPTS *aoPtr)
{
  long    auCaps;
  ATIME   aTime, *tPtr;
  DOBJ   *dop=NULL;
  DDESC  *dd=NULL;
  RMS_GD *gd=NULL;
  KDTAB  *entry;

  if(smpDOp == NULL || aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "createRMS");
    return(NULL);
  }
  clrAsspMsg();
  /* verify audio object */
  if((auCaps=getSmpCaps(RMS_PFORMAT)) <= 0)
    return(NULL);
  auCaps |= RMS_I_CHANS;
  if(aoPtr->channel < 0)
    aoPtr->channel = 0;
  if(checkSound(smpDOp, auCaps, aoPtr->channel) <= 0)
    return(NULL);
  tPtr = &aTime;
  if(anaTiming(smpDOp, aoPtr, tPtr) < 0)
    return(NULL);
  clrAsspMsg();                               /* ignore warnings here */
  if((gd=(RMS_GD *)malloc(sizeof(RMS_GD))) == NULL) {
    setAsspMsg(AEG_ERR_MEM, "createRMS");
    return(NULL);
  }
  strcpy(gd->ident, RMS_GD_IDENT);
  gd->options = aoPtr->options;
  gd->frameSize = tPtr->frameSize;
  gd->begFrameNr = tPtr->begFrameNr;
  gd->endFrameNr = tPtr->endFrameNr;
  gd->winFunc = wfType(aoPtr->winFunc);
  if(gd->winFunc <= WF_NONE) {
    freeRMS_GD((void *)gd);
    setAsspMsg(AEB_BAD_WIN, aoPtr->winFunc);
    return(NULL);
  }
  gd->channel = aoPtr->channel;
  gd->precision = aoPtr->precision;

  if((dop=allocDObj()) == NULL) {
    freeRMS_GD((void *)gd);
    return(NULL);
  }
  if(strxcmp(aoPtr->format, "SSFF") == 0) {
    dop->fileFormat = FF_SSFF;
    dop->fileData = FDF_BIN;
    strcpy(dop->eol, SSFF_EOL_STR);
  }
  else if(strxcmp(aoPtr->format, "XASSP") == 0) {
    dop->fileFormat = FF_XASSP;
    dop->fileData = FDF_ASC;
    strcpy(dop->eol, NATIVE_EOL);
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
  dop->doFreeGeneric = (DOfreeFunc)freeRMS_GD;
  dd = &(dop->ddl);                 /* set pointer to data descriptor */
  dd->type = DT_RMS;
  dd->coding = DC_LIN;
  dd->format = RMS_DFORMAT;
  if(gd->channel < 1)
    dd->numFields = smpDOp->ddl.numFields;  /* possibly multi-channel */
  else
    dd->numFields = 1;                       /* only selected channel */
  if(dop->fileFormat == FF_SSFF) {
    entry = dtype2entry(dd->type, KDT_SSFF);   /* search SSFF keyword */
    if(entry != NULL && entry->keyword != NULL) {
      dd->ident = strdup(entry->keyword);
      if(!(gd->options & RMS_OPT_LINEAR)) {
	if(entry->factor != NULL)
	  strcpy(dd->factor, entry->factor);
	if(entry->unit != NULL)
	  strcpy(dd->unit, entry->unit);
      }
    }
    else {
      dop = freeDObj(dop);
      setAsspMsg(AEB_ERR_TRACK, "(createRMS)");
      return(dop);
    }
  }
  else if(dop->fileFormat == FF_XASSP) {
    entry = dtype2entry(dd->type, KDT_XASSP); /* search XASSP keyword */
    if(entry != NULL && entry->keyword != NULL) {
      dd->ident = strdup(entry->keyword);
      if(!(gd->options & RMS_OPT_LINEAR)) {
	if(entry->factor != NULL)
	  strcpy(dd->factor, entry->factor);
	if(entry->unit != NULL)
	  strcpy(dd->unit, entry->unit);
      }
    }
    else {
      dop = freeDObj(dop);
      setAsspMsg(AEB_ERR_TRACK, "(createRMS)");
      return(dop);
    }
    strcpy(dop->sepChars, "\t");                    /* between fields */
    strcpy(dd->sepChars, " ");                        /* within field */
    snprintf(dd->ascFormat, member_size(DDESC, ascFormat), "%%.%df", gd->precision);
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

Function 'computeRMS'

Performs an RMS amplitude analysis of the audio signal referred to by 
"smpDOp" using the parameter settings specified in the generic data 
structure of the output data object pointed to by "rmsDOp". If "rmsDOp" 
is a NULL-pointer "aoPtr" may not be a NULL-pointer and this function 
will create the output data object (see 'createRMS' for details).
If "aoPtr" is not a NULL-pointer this function will verify and - if 
necessary - (re-)allocate the data buffers in the data objects pointed 
to by "smpDOp" and "rmsDOp" to have appropriate size.
Analysis results will be returned in the data buffer of the object 
pointed to by "rmsDOp" or written to file if that object refers to a 
file opened for writing. 
Returns the pointer to the RMS data object or NULL upon error.

Note:
 - This function may be used in a file-to-file, file-to-memory, memory-
   to-file and memory-to-memory mode.
 - This function DOES NOT verify whether the analysis parameters in the 
   generic data structure of "rmsDOp" comply with those in the general 
   analysis structure pointed to by "aoPtr". You should use the function 
   'verifyRMS' if parameter changes may occur between calls.

DOC*/

DOBJ *computeRMS(DOBJ *smpDOp, AOPTS *aoPtr, DOBJ *rmsDOp)
{
  int     FILE_IN, FILE_OUT, CREATED;
  int     err, cn, numChans;
  long    fn, frameSize, frameShift;
  float   rmsVal[RMS_O_CHANS];
  double  wfGain, rmsAmp;
  RMS_GD *gd;

  if(smpDOp == NULL || (aoPtr == NULL && rmsDOp == NULL)) {
    setAsspMsg(AEB_BAD_ARGS, "computeRMS");
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
      setAsspMsg(AED_NO_DATA, "(computeRMS)");
      return(NULL);
    }
  }
  /* check status of output object */
  if(rmsDOp == NULL) {
    if((rmsDOp=createRMS(smpDOp, aoPtr)) == NULL)
      return(NULL);
    CREATED = TRUE;
  }
  gd = (RMS_GD *)rmsDOp->generic;
  if(rmsDOp->fp != NULL) {
    FILE_OUT = TRUE;
    gd->writeOpts = AFW_CLEAR;          /* discard data after writing */
    if(rmsDOp->fileData == FDF_ASC)
      gd->writeOpts |= AFW_ADD_TIME;
  }
  else
    gd->writeOpts = AFW_KEEP;

  frameSize = gd->frameSize;
  frameShift = rmsDOp->frameDur;
  if(aoPtr != NULL) {
    if(checkDataBufs(smpDOp, rmsDOp, frameSize,\
		     gd->begFrameNr, gd->endFrameNr) < 0) {
      if(CREATED)
	freeDObj(rmsDOp);
      return(NULL);
    }
    if(aoPtr->options & AOPT_INIT_ONLY) {
      aoPtr->options &= ~AOPT_INIT_ONLY;                /* clear flag */
      return(rmsDOp);                                  /* no analysis */
    }
  }
  /* set global values and allocate local buffer space */
  if(setGlobals(rmsDOp) < 0) {
    if(CREATED)
      freeDObj(rmsDOp);
    return(NULL);
  }
  if(gd->winFunc > WF_RECTANGLE)
    wfGain = wfCohGain(wfc, frameSize);
  else
    wfGain = 1.0;
  numChans = (int)(rmsDOp->ddl.numFields);
  if(TRACE['A']) {
    fprintf(traceFP, "Analysis parameters\n");
    fprintf(traceFP, "  sample rate = %.1f Hz\n", rmsDOp->sampFreq);
    fprintf(traceFP, "  window size = %ld samples\n", frameSize);
    fprintf(traceFP, "  window shift = %ld samples\n", frameShift);
    fprintf(traceFP, "  window function = %s\n",
	    wfSpecs(gd->winFunc)->entry->code);
    fprintf(traceFP, "  output coding = %s\n",\
	    (gd->options & RMS_OPT_LINEAR) ? "linear":"dB");
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
        if((err=getSmpFrame(smpDOp, fn, frameSize, frameShift, 0, 0,\
			    gd->channel, frame, RMS_PFORMAT)) < 0)
          break;
      }
      else { /* possibly multi-channel */
        if((err=getSmpFrame(smpDOp, fn, frameSize, frameShift, 0, 0,\
			    cn+1, frame, RMS_PFORMAT)) < 0)
          break;
      }
      if(gd->winFunc > WF_RECTANGLE)
        mulSigWF(frame, wfc, frameSize);
      rmsAmp = getRMS(frame, frameSize);
      if(gd->winFunc > WF_RECTANGLE)
        rmsAmp /= wfGain;
      if(!(gd->options & RMS_OPT_LINEAR)) {          /* convert to dB */
        if(rmsAmp <= RMS_MIN_AMP) {                    /* bottom clip */
          if(TRACE['c']) {
	    if(gd->channel > 0)
	      fprintf(traceFP, "T = %.4f  c = %d  R[0] = %f\n",\
		      FRMNRtoTIME(fn, rmsDOp->sampFreq, frameShift),\
		      gd->channel, rmsAmp);
	    else
	      fprintf(traceFP, "T = %.4f  c = %d  R[0] = %f\n",\
		      FRMNRtoTIME(fn, rmsDOp->sampFreq, frameShift),\
		      cn+1, rmsAmp);
	  }
          rmsAmp = RMS_MIN_dB;
        }
        else
          rmsAmp = LINtodB(rmsAmp);
      }
      rmsVal[cn] = (float)rmsAmp;
    } /* END loop over channels */
    if(err < 0) break;
    if((err=storeRMS(rmsVal, fn, rmsDOp)) < 0) break;
  } /* END loop over frames */
  if(err >= 0 && FILE_OUT)
    err = asspFFlush(rmsDOp, gd->writeOpts);
  freeGlobals();
  if(err < 0) {
    if(CREATED)
      freeDObj(rmsDOp);
    return(NULL);
  }
  return(rmsDOp);
}

/*DOC

Function 'verifyRMS'

Verifies whether the analysis parameters of the RMS amplitude data 
object pointed to by "rmsDOp" differ from those specified in the general 
analysis parameter structure pointed to by "aoPtr" for the audio object 
pointed to by "smpDOp". None of the three pointers may be NULL. 
If there are differences, it is checked whether they lead to clashes 
with existing analysis data or header settings or require data buffers 
to be reallocated. If there are no clashes, the items in the generic 
data structure will be updated to match the general analysis parameter 
settings.
This function should be called when between a call to 'createRMS' and to 
'computeRMS', changes have been made in the analysis parameters or the 
audio object (e.g. a length change). 
Returns 0 when there are no problems, -1 upon error or +1 if there are 
warnings.

DOC*/

int verifyRMS(DOBJ *rmsDOp, DOBJ *smpDOp, AOPTS *aoPtr)
{
  int     err;
  long    auCaps;
  double  frameRate;
  DDESC  *dd;
  RMS_GD *gd;
  ATIME   aTime, *tPtr;

  if(rmsDOp == NULL || smpDOp == NULL || aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "verifyRMS");
    return(-1);
  }
  dd = &(rmsDOp->ddl);
  if(dd->type != DT_RMS || dd->format != RMS_DFORMAT || dd->next != NULL) {
    setAsspMsg(AEG_ERR_BUG, "Not a regular RMS object");
    return(-1);
  }
  gd = (RMS_GD *)(rmsDOp->generic);
  if(gd == NULL ||
     (gd != NULL && strcmp(gd->ident, RMS_GD_IDENT) != 0)) {
    setAsspMsg(AEG_ERR_BUG, "RMS generic data missing or invalid");
    return(-1);
  }
  clrAsspMsg();
  /* verify audio object */
  if(aoPtr->channel < 0)
    aoPtr->channel = 0;
  if((auCaps=getSmpCaps(RMS_PFORMAT)) <= 0)
    return(-1);
  auCaps |= RMS_I_CHANS;
  if(checkSound(smpDOp, auCaps, aoPtr->channel) <= 0)
    return(-1);
  tPtr = &aTime;
  if((err=anaTiming(smpDOp, aoPtr, tPtr)) < 0)
    return(-1);
  if(rmsDOp->sampFreq != tPtr->sampFreq ||\
     rmsDOp->frameDur != tPtr->frameShift) {
    frameRate = tPtr->sampFreq / (double)(tPtr->frameShift);
    if(rmsDOp->dataRate != frameRate) {
      if(rmsDOp->fp != NULL &&\
	 (rmsDOp->numRecords > 0 || ftell(rmsDOp->fp) != 0)) {
	setAsspMsg(AED_ERR_RATE, rmsDOp->filePath);
	return(-1);
      }
      clearDataBuf(rmsDOp);/* contents invalid; size maybe sufficient */
    }
    rmsDOp->sampFreq = tPtr->sampFreq;
    rmsDOp->frameDur = tPtr->frameShift;
    rmsDOp->dataRate = frameRate;
    rmsDOp->startRecord = tPtr->begFrameNr;
    rmsDOp->numRecords = 0;
    setStart_Time(rmsDOp);
  }
  if((aoPtr->channel < 1 && dd->numFields != smpDOp->ddl.numFields) ||\
     (aoPtr->channel > 0 && dd->numFields != 1) ||\
     (aoPtr->channel != gd->channel) ) {
    if(rmsDOp->fp != NULL &&\
       (rmsDOp->numRecords > 0 || ftell(rmsDOp->fp) != 0)) {
      setAsspMsg(AEG_ERR_APPL, "verifyRMS: can't change channels "\
		 "in existing data");
      return(-1);
    }
    if((aoPtr->channel < 1 && dd->numFields != smpDOp->ddl.numFields) ||\
       (aoPtr->channel > 0 && dd->numFields != 1) ) {
      if(rmsDOp->doFreeDataBuf == NULL) {
	setAsspMsg(AEG_ERR_APPL, "verifyRMS: can't reallocate data buffer");
	return(-1);
      }
      freeDataBuf(rmsDOp);  /* will be reallocated in checkDataBufs() */
    }
    else
      clearDataBuf(rmsDOp);                       /* contents invalid */
    if(aoPtr->channel < 1)
      dd->numFields = smpDOp->ddl.numFields;
    else
      dd->numFields = 1;
    setRecordSize(rmsDOp);
  }
  if(tPtr->begFrameNr < rmsDOp->startRecord) {
    if(rmsDOp->fp != NULL &&\
       (rmsDOp->numRecords > 0 || ftell(rmsDOp->fp) != 0)) {
      setAsspMsg(AEG_ERR_APPL, "verifyRMS: can't change start time "\
		 "in existing data");
      return(-1);
    }
    rmsDOp->startRecord = tPtr->begFrameNr;
    rmsDOp->numRecords = 0;
    setStart_Time(rmsDOp);
  }
  /* now copy relevant parameters */
  gd->options = aoPtr->options;
  gd->frameSize = tPtr->frameSize;
  gd->begFrameNr = tPtr->begFrameNr;
  gd->endFrameNr = tPtr->endFrameNr;
  gd->winFunc = wfType(aoPtr->winFunc);
  if(gd->winFunc <= WF_NONE) {
    setAsspMsg(AEB_BAD_WIN, aoPtr->winFunc);
    return(-1);
  }
  gd->channel = aoPtr->channel;
  gd->precision = aoPtr->precision;
  return(err);
}

/*DOC

Function 'freeRMS_GD'

Returns all memory allocated for the generic data in an RMS data object.

DOC*/

void freeRMS_GD(void *generic)
{
  if(generic != NULL) {
    free(generic);
  }
  return;
}

/* ======================= private  functions ======================= */

/***********************************************************************
* allocate memory for the frame buffer and the window coefficients     *
***********************************************************************/
LOCAL int setGlobals(DOBJ *dop)
{
  int     wFlags;
  RMS_GD *gd;

  frame = wfc = NULL;
  gd = (RMS_GD *)(dop->generic);
  if(gd->winFunc > WF_RECTANGLE) {
    wFlags = WF_PERIODIC;
    if((ODD(gd->frameSize) && EVEN(dop->frameDur)) ||
       (EVEN(gd->frameSize) && ODD(dop->frameDur)) )
      wFlags = WF_ASYMMETRIC; /* align window centre and frame centre */
    wfc = makeWF(gd->winFunc, gd->frameSize, wFlags);
    if(wfc == NULL) {
      setAsspMsg(AEG_ERR_MEM, "RMS: setGlobals");
      return(-1);
    }
  }
  frame = (double *)calloc((size_t)(gd->frameSize), sizeof(double));
  if(frame == NULL) {
    freeGlobals();
    setAsspMsg(AEG_ERR_MEM, "RMS: setGlobals");
    return(-1);
  }
  return(0);
}

/***********************************************************************
* return memory allocated for the global arrays                        *
***********************************************************************/
LOCAL void freeGlobals(void)
{
  if(frame != NULL)
    free((void *)frame);
  freeWF(wfc);
  frame = wfc = NULL;
  return;
}

/***********************************************************************
* copy frame data to output buffer; handle data writes                 *
***********************************************************************/
LOCAL int storeRMS(float *vals, long frameNr, DOBJ *dop)
{
  int     FILE_OUT;
  size_t  chans;
  long    ndx;
  float  *fPtr;
  RMS_GD *gd;

  FILE_OUT = (dop->fp != NULL);
  gd = (RMS_GD *)dop->generic;
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
      setAsspMsg(AEG_ERR_BUG, "storeRMS: buffer overflow");
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
