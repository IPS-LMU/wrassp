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
* File:     acf.c                                                      *
* Contents: Functions for the analysis of autocorrelation function.    *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: acf.c,v 1.18 2010/07/14 13:47:04 mtms Exp $ */

#include <stdio.h>     /* NULL */
#include <stdlib.h>    /* malloc() calloc() free() */
#include <string.h>    /* memset() strcpy() strchr() memcpy() */
#include <inttypes.h>  /* int16_t */

#include <miscdefs.h>  /* TRUE FALSE LOCAL */
#include <misc.h>      /* strxcmp() */
#include <trace.h>     /* trace handler */
#include <asspmess.h>  /* error message handler */
#include <assptime.h>  /* standard conversion macros */
#include <asspana.h>   /* AOPTS anaTiming() (includes acf.h) */
#include <asspdsp.h>   /* makeWF() freeWF() mulSigWF() getACF() */
#include <asspfio.h>   /* asspFFlush() */
#include <dataobj.h>   /* DOBJ getSmpCaps() getSmpFrame() */
#include <headers.h>   /* KDTAB */
#include <aucheck.h>   /* checkSound() */

/*
 * prototypes of private functions
 */
LOCAL int  allocBufs(ACF_GD *gd, long frameShift);
LOCAL void freeBufs(ACF_GD *gd);
LOCAL int  storeACF(double *c, long frameNr, DOBJ *dop);

/* ======================== public functions ======================== */

/*DOC

Function 'setACFdefaults'

Sets the items in the analysis options structure relevant to the 
analysis of the autocorrelation coefficients of a signal to their 
default values. Clear all other items.
Returns 0 upon success and -1 upon error.

DOC*/

int setACFdefaults(AOPTS *aoPtr)
{
  if(aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setACFdefaults");
    return(-1);
  }
  memset((void *)aoPtr, 0, sizeof(AOPTS));         /* clear all items */
  aoPtr->msSize = ACF_DEF_SIZE;                    /* frame size (ms) */
  aoPtr->options = AOPT_EFFECTIVE;              /* use effective size */
  aoPtr->msShift = ACF_DEF_SHIFT;                 /* frame shift (ms) */
  aoPtr->order = ACF_DEF_ORDER;                     /* analysis order */
  aoPtr->channel = ACF_DEF_CHANNEL;         /* channel to be analysed */
  aoPtr->accuracy = ACF_DEF_DIGITS;      /* digits accuracy for ASCII */
  strcpy(aoPtr->format, ACF_DEF_FORMAT);        /* output file format */
  strcpy(aoPtr->winFunc, ACF_DEF_WINDOW);          /* window function */
  return(0);
}

/*DOC

Function 'createACF'

Allocates memory for a data object to hold autocorrelation coefficients 
and initializes it on the basis of the information in the audio object 
pointed to by "smpDOp" and the analysis options in the structure pointed 
to by "aoPtr". Neither of these pointers may be NULL.
Returns a pointer to the data object or NULL upon error.

Note:
 - This function DOES NOT allocate memory for the data buffer. Depending 
   on your application you can either use allocDataBuf() for this or 
   delegate it to computeAFC().

DOC*/

DOBJ *createACF(DOBJ *smpDOp, AOPTS *aoPtr)
{
  long    auCaps;
  ATIME   aTime, *tPtr;
  ACF_GD *gd=NULL;
  DOBJ   *dop=NULL;
  DDESC  *dd=NULL;
  KDTAB  *entry;

  if(smpDOp == NULL || aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "createACF");
    return(NULL);
  }
  /* verify audio object */
  if((auCaps=getSmpCaps(ACF_PFORMAT)) <= 0)
    return(NULL);
  auCaps |= ACF_I_CHANS;
  if(aoPtr->channel < 1)
    aoPtr->channel = ACF_DEF_CHANNEL;
  if(checkSound(smpDOp, auCaps, aoPtr->channel) <= 0)
    return(NULL);
  tPtr = &aTime;
  if(anaTiming(smpDOp, aoPtr, tPtr) < 0)
    return(NULL);
  clrAsspMsg();                               /* ignore warnings here */
  if((gd=(ACF_GD *)malloc(sizeof(ACF_GD))) == NULL) {
    setAsspMsg(AEG_ERR_MEM, "(createACF)");
    return(NULL);
  }
  strcpy(gd->ident, ACF_GD_IDENT);
  gd->options = aoPtr->options;
  gd->frameSize = tPtr->frameSize;
  gd->begFrameNr = tPtr->begFrameNr;
  gd->endFrameNr = tPtr->endFrameNr;
  if(aoPtr->order < 1)
    gd->order = DFLT_ORDER(tPtr->sampFreq);
  else
    gd->order = aoPtr->order;
  if((gd->order + 1) >= gd->frameSize) {
    free((void *)gd);
    setAsspMsg(AED_ERR_SIZE, "(createACF)");
    return(NULL);
  }
  gd->winFunc = wfType(aoPtr->winFunc);
  if(gd->winFunc <= WF_NONE) {
    freeACF_GD((void *)gd);
    setAsspMsg(AEB_BAD_WIN, aoPtr->winFunc);
    return(NULL);
  }
  gd->channel = aoPtr->channel;
  gd->accuracy = aoPtr->accuracy;
  gd->frame = NULL;
  gd->wfc = NULL;
  gd->acf = NULL;
  gd->gainCorr = 1.0;

  if((dop=allocDObj()) == NULL) {
    freeACF_GD((void *)gd);
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
  dop->doFreeGeneric = (DOfreeFunc)freeACF_GD;
  dd = &(dop->ddl);                 /* set pointer to data descriptor */
  dd->type = DT_ACF;
  dd->coding = DC_LIN;
  dd->format = ACF_DFORMAT;
  dd->numFields = gd->order + 1;
  if(dop->fileFormat == FF_SSFF) {
    entry = dtype2entry(dd->type, KDT_SSFF);   /* search SSFF keyword */
    if(entry != NULL && entry->keyword != NULL) {
      dd->ident = strdup(entry->keyword);
    }
    else {
      dop = freeDObj(dop);
      setAsspMsg(AEB_ERR_TRACK, "(createACF)");
      return(dop);
    }
  }
  else { /* fall through to raw ASCII */
    strcpy(dop->sepChars, "\t");                    /* between fields */
    strcpy(dd->sepChars, " ");                        /* within field */
    snprintf(dd->ascFormat, member_size(DDESC, ascFormat), "%%+.%de", gd->accuracy);
  }
  setRecordSize(dop);
  setStart_Time(dop);
  return(dop);
}

/*DOC

Function 'computeACF'

Performs an autocorrelation analysis of the audio signal referred to by 
"smpDOp" using the parameter settings specified in the generic data 
structure of the output data object pointed to by "acfDOp". If "acfDOp" 
is a NULL-pointer "aoPtr" may not be a NULL-pointer and this function 
will create the output data object (see 'createACF' for details).
If "aoPtr" is not a NULL-pointer this function will verify and - if 
necessary - (re-)allocate the data buffers in the data objects pointed 
to by "smpDOp" and "acfDOp" to have appropriate size. For on-the-fly, 
single-frame analyses it is advised to use an initial call with the 
option flag 'AOPT_INIT_ONLY' set in order to set up the data buffers. 
You may then suppress verification overhead in subsequent calls by 
passing NULL for "aoPtr". If the flag 'AOPT_INIT_ONLY' has been set it 
will be cleared after initialisation and verification and no analysis 
will be performed. 
Analysis results will be returned in the data buffer of the object 
pointed to by "acfDOp" or written to file if that object refers to a 
file opened for writing. 
Returns a pointer to the autocorrelation data object or NULL upon error.

Note:
 - This function may be used in a file-to-file, file-to-memory, memory-
   to-file and memory-to-memory mode.
 - This function DOES NOT verify whether the analysis parameters in the 
   generic data structure of "acfDOp" comply with those in the general 
   analysis structure pointed to by "aoPtr". You should use the function 
   'verifyACF' if parameter changes may occur between calls.

DOC*/

DOBJ *computeACF(DOBJ *smpDOp, AOPTS *aoPtr, DOBJ *acfDOp)
{
  int     FILE_IN, FILE_OUT, CREATED;
  int     err, m, order;
  long    fn, frameSize, frameShift;
  double  R0;
  ACF_GD *gd;

  if(smpDOp == NULL || (aoPtr == NULL && acfDOp == NULL)) {
    setAsspMsg(AEB_BAD_ARGS, "computeACF");
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
      setAsspMsg(AED_NO_DATA, "(computeACF)");
      return(NULL);
    }
  }
  /* check status of output object */
  if(acfDOp == NULL) {
    if((acfDOp=createACF(smpDOp, aoPtr)) == NULL)
      return(NULL);
    CREATED = TRUE;
  }
  gd = (ACF_GD *)acfDOp->generic;
  if(acfDOp->fp != NULL) {
    FILE_OUT = TRUE;
    gd->writeOpts = AFW_CLEAR;        /* discard data after writing */
    if(acfDOp->fileData == FDF_ASC)
      gd->writeOpts |= AFW_ADD_TIME;
  }
  else
    gd->writeOpts = AFW_KEEP;

  order = gd->order;
  frameSize = gd->frameSize;
  frameShift = acfDOp->frameDur;
  if(aoPtr != NULL) {
    if(checkDataBufs(smpDOp, acfDOp, frameSize,\
		     gd->begFrameNr, gd->endFrameNr) < 0) {
      if(CREATED)
	freeDObj(acfDOp);
      return(NULL);
    }
    if(allocBufs(gd, frameShift) < 0) {
      if(CREATED)
	freeDObj(acfDOp);
      return(NULL);
    }
    if(TRACE['A']) {
      fprintf(traceFP, "Analysis parameters\n");
      fprintf(traceFP, "  sample rate = %.1f Hz\n", acfDOp->sampFreq);
      fprintf(traceFP, "  window size = %ld samples\n", frameSize);
      fprintf(traceFP, "  window shift = %ld samples\n", frameShift);
      fprintf(traceFP, "  window function = %s\n",\
	      wfSpecs(gd->winFunc)->entry->code);
      fprintf(traceFP, "  analysis order = %d\n", order);
      fprintf(traceFP, "  length normalization %s\n",\
	      (gd->options & ACF_OPT_MEAN) ? "ON":"OFF");
      fprintf(traceFP, "  energy normalization %s\n",\
	      (gd->options & ACF_OPT_NORM) ? "ON":"OFF");
      fprintf(traceFP, "  selected channel = %d\n", gd->channel);
      fprintf(traceFP, "  start frame = %ld\n", gd->begFrameNr);
      fprintf(traceFP, "  end frame = %ld\n", gd->endFrameNr);
      fprintf(traceFP, "  processing mode = %s-to-%s\n",\
	      FILE_IN ? "file" : "memory", FILE_OUT ? "file" : "memory");
    }
    if(aoPtr->options & AOPT_INIT_ONLY) {
      aoPtr->options &= ~AOPT_INIT_ONLY;                /* clear flag */
      return(acfDOp);                                  /* no analysis */
    }
  }
  else if(gd->frame == NULL) {
    if(allocBufs(gd, frameShift) < 0)
      return(NULL);
  }
  /* loop over frames */
  for(err = 0, fn = gd->begFrameNr; fn < gd->endFrameNr; fn++) {
    if((err=getSmpFrame(smpDOp, fn, frameSize, frameShift, 0, 0,\
			gd->channel, gd->frame, ACF_PFORMAT)) < 0) {
      break;
    }
    if(gd->winFunc > WF_RECTANGLE)
      mulSigWF(gd->frame, gd->wfc, frameSize);
    if(gd->options & ACF_OPT_MEAN)
      getMeanACF(gd->frame, gd->acf, frameSize, order);
    else
      getACF(gd->frame, gd->acf, frameSize, order);
    if(gd->options & ACF_OPT_NORM) {
      R0 = gd->acf[0];
      gd->acf[0] = 1.0;
      if(R0 <= 0.0) {
	for(m = 1; m <= order; m++)
	  gd->acf[m] = 0.0;
      }
      else {
	for(m = 1; m <= order; m++)
	  gd->acf[m] /= R0;
      }
    }
    else if(gd->winFunc > WF_RECTANGLE) {  /* correct for window gain */
      for(m = 0; m <= order; m++)
	gd->acf[m] /= (gd->gainCorr);
    }
    if((err=storeACF(gd->acf, fn, acfDOp)) < 0) break;
  }       /* END LOOP OVER FRAMES */
  if(err >= 0 && FILE_OUT)
    err = asspFFlush(acfDOp, gd->writeOpts);
  if(err < 0) {
    if(CREATED)
      freeDObj(acfDOp);
    return(NULL);
  }
  return(acfDOp);
}

/*DOC

Function 'verifyACF'

Verifies whether the analysis parameters of the autocorrelation data 
object pointed to by "acfDOp" differ from those specified in the general 
analysis parameter structure pointed to by "aoPtr" for the audio object 
pointed to by "smpDOp". None of the three pointers may be NULL. If there 
are differences, it checks whether they lead to clashes with existing 
analysis data or header settings or require data buffers to be reallocated. 
If there are no clashes, the items in the generic data structure in 
"acfDOp" will be updated to match the general analysis parameter settings.
This function should be called when between a call to 'createACF' and to 
'computeACF', changes have been made in the analysis parameters or the 
audio object (e.g. a length change). 
Returns 0 when there are no problems, -1 upon error or +1 if there are 
warnings.

DOC*/

int verifyACF(DOBJ *acfDOp, DOBJ *smpDOp, AOPTS *aoPtr)
{
  int     err, order;
  long    auCaps;
  double  frameRate;
  DDESC  *dd;
  ACF_GD *gd;
  ATIME   aTime, *tPtr;

  if(acfDOp == NULL || smpDOp == NULL || aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "verifyACF");
    return(-1);
  }
  dd = &(acfDOp->ddl);
  if(dd->type != DT_ACF || dd->format != ACF_DFORMAT || dd->next != NULL) {
    setAsspMsg(AEG_ERR_BUG, "Not a regular ACF object");
    return(-1);
  }
  gd = (ACF_GD *)(acfDOp->generic);
  if(gd == NULL ||
     (gd != NULL && strcmp(gd->ident, ACF_GD_IDENT) != 0)) {
    setAsspMsg(AEG_ERR_BUG, "ACF generic data missing or invalid");
    return(-1);
  }
  clrAsspMsg();
  /* verify audio object */
  if(aoPtr->channel < 1)
    aoPtr->channel = ACF_DEF_CHANNEL;
  if((auCaps=getSmpCaps(ACF_PFORMAT)) <= 0)
    return(-1);
  auCaps |= ACF_I_CHANS;
  if(checkSound(smpDOp, auCaps, aoPtr->channel) <= 0)
    return(-1);
  tPtr = &aTime;
  if((err=anaTiming(smpDOp, aoPtr, tPtr)) < 0)
    return(-1);
  if(acfDOp->sampFreq != tPtr->sampFreq ||\
     acfDOp->frameDur != tPtr->frameShift) {
    frameRate = tPtr->sampFreq / (double)(tPtr->frameShift);
    if(acfDOp->dataRate != frameRate) {
      if(acfDOp->fp != NULL &&\
	 (acfDOp->numRecords > 0 || ftell(acfDOp->fp) != 0)) {
	setAsspMsg(AED_ERR_RATE, acfDOp->filePath);
	return(-1);
      }
      clearDataBuf(acfDOp);/* contents invalid; size maybe sufficient */
    }
    acfDOp->sampFreq = tPtr->sampFreq;
    acfDOp->frameDur = tPtr->frameShift;
    acfDOp->dataRate = frameRate;
    acfDOp->startRecord = tPtr->begFrameNr;
    acfDOp->numRecords = 0;
    setStart_Time(acfDOp);
  }
  if(aoPtr->order < 1)
    order = DFLT_ORDER(tPtr->sampFreq);
  else
    order = aoPtr->order;
  if(order != gd->order || order != (dd->numFields - 1)) {
    if((order + 1) >= tPtr->frameSize) {
      setAsspMsg(AED_ERR_SIZE, "(verifyACF)");
      return(-1);
    }
    if(acfDOp->fp != NULL &&\
       (acfDOp->numRecords > 0 || ftell(acfDOp->fp) != 0)) {
      setAsspMsg(AEG_ERR_BUG, "verifyACF: can't change analysis order "\
		 "in existing data");
      return(-1);
    }
    if(acfDOp->doFreeDataBuf == NULL) {
      setAsspMsg(AEG_ERR_APPL, "verifyACF: can't reallocate data buffer");
      return(-1);
    }
    freeDataBuf(acfDOp);    /* will be reallocated in checkDataBufs() */
    gd->order = order;
    dd->numFields = order + 1;
    setRecordSize(acfDOp);
  }
  if(aoPtr->channel != gd->channel)
    clearDataBuf(acfDOp);                         /* contents invalid */
  freeBufs(gd);                                /* overkill but simple */
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
  gd->accuracy = aoPtr->accuracy;
  return(err);
}

/*DOC

Function 'freeACF_GD'

Returns all memory allocated for the generic data in an ACF data object.

DOC*/

void freeACF_GD(void *generic)
{
  if(generic != NULL) {
    freeBufs((ACF_GD *)generic);
    free(generic);
  }
  return;
}

/* ======================= private  functions ======================= */

/***********************************************************************
* allocate memory for the frame buffer the window coefficients and the *
* ACF coefficients                                                     *
***********************************************************************/
LOCAL int allocBufs(ACF_GD *gd, long frameShift)
{
  int wFlags;

  gd->frame = gd->wfc = gd->acf = NULL;
  if(gd->winFunc > WF_RECTANGLE) {
    /* because of the relationship between the autocorrelation and */
    /* the power spectrum, we use the 'proper' periodic window */
    wFlags = WF_PERIODIC;
    if((ODD(gd->frameSize) && EVEN(frameShift)) ||
       (EVEN(gd->frameSize) && ODD(frameShift)) )
      wFlags = WF_ASYMMETRIC; /* align window centre and frame centre */
    gd->wfc = makeWF(gd->winFunc, gd->frameSize, wFlags);
    if(gd->wfc == NULL) {
      setAsspMsg(AEG_ERR_MEM, "ACF: allocBufs");
      return(-1);
    }
    gd->gainCorr = pow(wfCohGain(gd->wfc, gd->frameSize), 2.0);
  }
  else /* gd->wfc already NULL; */
    gd->gainCorr = 1.0;
  gd->frame = (double *)calloc((size_t)gd->frameSize, sizeof(double));
  gd->acf = (double *)calloc((size_t)(gd->order + 1), sizeof(double));
  if(gd->frame == NULL || gd->acf == NULL) {
    freeBufs(gd);
    setAsspMsg(AEG_ERR_MEM, "ACF: allocBufs");
    return(-1);
  }
  return(0);
}

/***********************************************************************
* free memory allocated in the generic data                            *
***********************************************************************/
LOCAL void freeBufs(ACF_GD *gd)
{
  if(gd != NULL) {
    if(gd->frame != NULL)
      free((void *)(gd->frame));
    freeWF(gd->wfc);
    if(gd->acf != NULL)
      free((void *)(gd->acf));
    gd->frame = gd->wfc = gd->acf = NULL;
    gd->gainCorr = 1.0;
  }
  return;
}

/***********************************************************************
* copy frame data to output buffer; handle data writes                 *
***********************************************************************/
LOCAL int storeACF(double *c, long frameNr, DOBJ *dop)
{
  int     FILE_OUT;
  long    ndx, m, M;
  double *dPtr;
  ACF_GD *gd;

  FILE_OUT = (dop->fp != NULL);
  gd = (ACF_GD *)dop->generic;
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
      setAsspMsg(AEG_ERR_BUG, "storeACF: buffer overflow");
      return(-1);
    }
  }
  M = dop->ddl.numFields;
  ndx = frameNr - dop->bufStartRec;
  dPtr = (double *)(dop->dataBuffer);
  dPtr = &dPtr[ndx*M];
  for(m = 0; m < M; m++)
    *(dPtr++) = *(c++);
  if(ndx >= dop->bufNumRecs)
    dop->bufNumRecs = ndx + 1;
  dop->bufNeedsSave = TRUE;

  return(0);
}
