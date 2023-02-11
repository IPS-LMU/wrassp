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
* File:     rfc.c                                                      *
* Contents: Functions for the analysis of reflection coefficients or   *
*           other linear prediction data using the autocorrelation     *
*           method and the Durbin recursion.                           *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: rfc.c,v 1.10 2012/03/19 16:16:38 lasselasse Exp $ */

#include <stdio.h>     /* NULL */
#include <stdlib.h>    /* malloc() calloc() free() */
#include <string.h>    /* memset() strcpy() strchr() memcpy() */
#include <inttypes.h>  /* int16_t */

#include <miscdefs.h>  /* TRUE FALSE LOCAL NATIVE_EOL */
#include <misc.h>      /* strxcmp() */
#include <trace.h>     /* trace handler */
#include <asspmess.h>  /* error message handler */
#include <assptime.h>  /* standard conversion macros */
#include <asspana.h>   /* AOPTS anaTiming() (includes rfc.h) */
#include <asspdsp.h>   /* makeWF() freeWF() mulSigWF() getACF() ... */
#include <asspfio.h>   /* asspFFlush() */
#include <dataobj.h>   /* DOBJ DT_xxx getSmpCaps() getSmpFrame() */
#include <headers.h>   /* KDTAB */
#include <aucheck.h>   /* AUC_... checkSound() */

/*
 * local global arrays and variables
 */
LOCAL double *rmsBuf=NULL; /* buffer for RMS calculation (allocated) */
LOCAL double *frame=NULL;  /* frame buffer incl. leading sample (allocated) */
LOCAL double  wfGain;      /* coherent gain of window function */
LOCAL double *wfc=NULL;    /* window function coefficients (allocated) */
LOCAL double *acf=NULL;    /* autocorrelation coefficients (allocated) */
LOCAL double *lpc=NULL;    /* linear prediction coefficients (allocated) */
LOCAL double *rfc=NULL;    /* reflection coefficients (allocated) */

typedef struct LP_output_frame {
  double  RMS;
  double  gain;
  double *lpData;          /* mapped to lpc or rfc above */
} LP_OUT;
LOCAL LP_OUT data;

/*
 * table relating available parameter codings as string with data type
 * code and file name extension.
 */
LP_TYPE lpType[] = {
  {"ARF", DT_ARF, ".arf"},
  {"LAR", DT_LAR, ".lar"},
  {"LPC", DT_LPC, ".lpc"},
  {"RFC", DT_RFC, ".rfc"},
  { NULL, DT_UNDEF, NULL}
};

/*
 * prototypes of local functions
 */
LOCAL int  setGlobals(DOBJ *dop);
LOCAL void freeGlobals(void);
LOCAL int  storeLP(LP_OUT *LPtr, long frameNr, DOBJ *dop);

/* ======================== public functions ======================== */

/*DOC

Function 'setLPdefaults'

Sets the items in the analysis options structure relevant to the 
analysis of linear prediction coefficients to their default values.
Clears all other items.
Returns 0 upon success and -1 upon error.

DOC*/

int setLPdefaults(AOPTS *aoPtr)
{
  if(aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setLPdefaults");
    return(-1);
  }
  memset((void *)aoPtr, 0, sizeof(AOPTS));         /* clear all items */
  aoPtr->msSize = LP_DEF_SIZE;                     /* frame size (ms) */
  aoPtr->options = AOPT_EFFECTIVE;              /* use effective size */
  aoPtr->msShift = LP_DEF_SHIFT;                  /* frame shift (ms) */
  aoPtr->preEmph = LP_DEF_PREEMPH;                     /* preemphasis */
  aoPtr->order = LP_DEF_ORDER;                      /* analysis order */
  aoPtr->channel = LP_DEF_CHANNEL;          /* channel to be analysed */
  aoPtr->accuracy = LP_DEF_DIGITSA;      /* digits accuracy for ASCII */
  aoPtr->precision = LP_DEF_DIGITSP;    /* digits precision for ASCII */
  strcpy(aoPtr->type, LP_DEF_TYPE);                 /* LP coding type */
  strcpy(aoPtr->format, LP_DEF_FORMAT);         /* output file format */
  strcpy(aoPtr->winFunc, LP_DEF_WINDOW);           /* window function */
  return(0);
}

/*DOC

Function 'createLP'

Allocates memory for a data object to hold linear prediction oefficients 
and initializes it on the basis of the information in the audio object 
pointed to by "smpDOp" and the analysis options in the structure pointed 
to by "aoPtr". Neither of these pointers may be NULL.
Returns a pointer to the data object or NULL upon error.

Note:
 - This function DOES NOT allocate memory for the data buffer. Depending 
   on your application you can either use 'allocDataBuf' for this or 
   delegate it to 'computeLP'.

DOC*/

DOBJ *createLP(DOBJ *smpDOp, AOPTS *aoPtr)
{
  char    *LPident;
  long     auCaps;
  ATIME    aTime, *tPtr;
  LP_TYPE *lPtr;
  LP_GD   *gd=NULL;
  DOBJ    *dop=NULL;
  DDESC   *dd=NULL;
  KDTAB   *entry;

  if(smpDOp == NULL || aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "createLP");
    return(NULL);
  }
  /* verify audio object */
  if((auCaps=getSmpCaps(LP_PFORMAT)) <= 0)
    return(NULL);
  auCaps |= LP_I_CHANS;
  if(aoPtr->channel < 1)
    aoPtr->channel = 1;
  if(checkSound(smpDOp, auCaps, aoPtr->channel) <= 0)
    return(NULL);
  tPtr = &aTime;
  if(anaTiming(smpDOp, aoPtr, tPtr) < 0)
    return(NULL);
  clrAsspMsg();                               /* ignore warnings here */
  if((gd=(LP_GD *)malloc(sizeof(LP_GD))) == NULL) {
    setAsspMsg(AEG_ERR_MEM, "(createLP)");
    return(NULL);
  }
  strcpy(gd->ident, LP_GD_IDENT);
  gd->options = aoPtr->options;
  gd->frameSize = tPtr->frameSize;
  gd->begFrameNr = tPtr->begFrameNr;
  gd->endFrameNr = tPtr->endFrameNr;
  if(aoPtr->preEmph < -1.0 || aoPtr->preEmph > 1.0) {
    freeLP_GD((void *)gd);
    setAsspMsg(AEB_ERR_EMPH, "(createLP)");
    return(NULL);
  }
  gd->preEmph = aoPtr->preEmph;
  if(aoPtr->order < 1)
    gd->order = DFLT_ORDER(tPtr->sampFreq);
  else
    gd->order = aoPtr->order;
  if((gd->order + 1) >= gd->frameSize) {
    freeLP_GD((void *)gd);
    setAsspMsg(AED_ERR_SIZE, "(createLP)");
    return(NULL);
  }
  LPident = NULL;
  for(lPtr = lpType; lPtr->ident != NULL; lPtr++) {/* search data type */
    if(strnxcmp(aoPtr->type, lPtr->ident, 2) == 0)
      break;
  }
  if(lPtr->ident == NULL) {
    freeLP_GD((void *)gd);
    setAsspMsg(AED_ERR_TYPE, aoPtr->type);
    return(NULL);
  }
  LPident = lPtr->ident;
  gd->dataType = lPtr->type;
  gd->winFunc = wfType(aoPtr->winFunc);
  if(gd->winFunc <= WF_NONE) {
    freeLP_GD((void *)gd);
    setAsspMsg(AEB_BAD_WIN, aoPtr->winFunc);
    return(NULL);
  }
  gd->channel = aoPtr->channel;
  gd->accuracy = aoPtr->accuracy;
  gd->precision = aoPtr->precision;

  if((dop=allocDObj()) == NULL) {
    freeLP_GD((void *)gd);
    strcpy(applMessage, "(createLP)");
    return(NULL);
  }
  dd = addDDesc(dop);                    /* 1st for RMS; 2nd for gain */
  if(dd != NULL)
    dd = addDDesc(dop);                    /* 3rd for LP coefficients */
  if(dd == NULL) {
    freeLP_GD((void *)gd);
    dop = freeDObj(dop);
    strcpy(applMessage, "(createLP)");
    return(dop);
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
  dop->doFreeGeneric = (DOfreeFunc)freeLP_GD;

  dd = &(dop->ddl);           /* reset pointer to 1st data descriptor */
  dd->type = DT_RMS;
  dd->coding = DC_LIN;
  dd->format = LP_RFORMAT;
  dd->numFields = 1;
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
      setAsspMsg(AEB_ERR_TRACK, "(createLP)");
      return(dop);
    }
  }
  else {
    dd->ident = strdup("RMS");
    strcpy(dd->unit, "dB");
    strcpy(dd->sepChars, " ");                        /* within field */
    snprintf(dd->ascFormat, member_size(DDESC, ascFormat), "%%.%df", gd->precision);
  }

  dd = dd->next;                /* set pointer to 2nd data descriptor */
  dd->type = DT_GAIN;
  dd->coding = DC_LIN;
  dd->format = LP_RFORMAT;
  dd->numFields = 1;
  if(dop->fileFormat == FF_SSFF) {
    entry = dtype2entry(dd->type, KDT_SSFF);   /* search SSFF keyword */
    if(entry != NULL && entry->keyword != NULL) {
      dd->ident = strdup(entry->keyword);
    }
    else {
      dop = freeDObj(dop);
      setAsspMsg(AEB_ERR_TRACK, "(createLP)");
      return(dop);
    }
  }
  else {
    dd->ident = strdup("GAIN");
    strcpy(dd->unit, "dB");
    strcpy(dd->sepChars, " ");                        /* within field */
    snprintf(dd->ascFormat, member_size(DDESC, ascFormat), "%%.%df", gd->precision);
  }

  dd = dd->next;                /* set pointer to 3rd data descriptor */
  dd->type = gd->dataType;
  dd->coding = DC_LIN;
  dd->format = LP_CFORMAT;
  if(dd->type == DT_LPC || dd->type == DT_ARF)
    dd->numFields = gd->order + 1;
  else
    dd->numFields = gd->order;
  if(dop->fileFormat == FF_SSFF) {
    entry = dtype2entry(dd->type, KDT_SSFF);   /* search SSFF keyword */
    if(entry != NULL && entry->keyword != NULL) {
      dd->ident = strdup(entry->keyword);
    }
    else {
      dop = freeDObj(dop);
      setAsspMsg(AEB_ERR_TRACK, "(createLP)");
      return(dop);
    }
  }
  else {
    dd->ident = strdup(LPident);
    strcpy(dd->sepChars, " ");                        /* within field */
    snprintf(dd->ascFormat, member_size(DDESC, ascFormat), "%%+.%de", gd->accuracy);
  }
  setRecordSize(dop);
  setStart_Time(dop);
  return(dop);
}

/*DOC

Function 'computeLP'

Performs a linear prediction analysis of the audio signal referred to by 
"smpDOp" using the parameter settings specified in the generic data 
structure of the output data object pointed to by "lpDOp". If "lpDOp" is 
a NULL-pointer "aoPtr" may not be a NULL-pointer and this function will 
create the output data object (see 'createLP' for details).
If "aoPtr" is not a NULL-pointer this function will verify and - if 
necessary - (re-)allocate the data buffers in the data objects pointed 
to by "smpDOp" and "lpDOp" to have appropriate size.
Analysis results will be returned in the data buffer of the object 
pointed to by "lpDOp" or written to file if that object refers to a 
file opened for writing. 
Returns a pointer to the linear prediction data object or NULL upon error.

Note:
 - This function may be used in a file-to-file, file-to-memory, memory-
   to-file and memory-to-memory mode.
 - This function DOES NOT verify whether the analysis parameters in the 
   generic data structure of "lpDOp" comply with those in the general 
   analysis structure pointed to by "aoPtr". You will have to implement 
   your own verification if parameter changes may occur between calls 
   (see 'createLP' and e.g. 'verifyACF' for what needs to be done). 
   If there are incompatibilities it is probably easiest just to destroy 
   the data object.

DOC*/

DOBJ *computeLP(DOBJ *smpDOp, AOPTS *aoPtr, DOBJ *lpDOp)
{
  char    *bPtr;
  int      FILE_IN, FILE_OUT, CREATED;
  int      err;
  long     i, fn, frameSize, frameShift, head, tail, order;
  double  *dPtr;
  LP_GD   *gd;
  LP_TYPE *lPtr;

  if(smpDOp == NULL || (aoPtr == NULL && lpDOp == NULL)) {
    setAsspMsg(AEB_BAD_ARGS, "computeLP");
    return(NULL);
  }
  err = 0;
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
      setAsspMsg(AED_NO_DATA, "(computeLP)");
      return(NULL);
    }
  }
  /* check status of output object */
  if(lpDOp == NULL) {
    if((lpDOp=createLP(smpDOp, aoPtr)) == NULL)
      return(NULL);
    CREATED = TRUE;
  }
  gd = (LP_GD *)lpDOp->generic;
  if(lpDOp->fp != NULL) {
    FILE_OUT = TRUE;
    gd->writeOpts = AFW_CLEAR;        /* discard data after writing */
    if(lpDOp->fileData == FDF_ASC)
      gd->writeOpts |= AFW_ADD_TIME;
  }
  else
    gd->writeOpts = AFW_KEEP;

  order = (long)(gd->order);
  frameSize = gd->frameSize;
  frameShift = lpDOp->frameDur;
  head = 1; /* for preemphasis */
  tail = 0;
  if(aoPtr != NULL) {
    if(checkDataBufs(smpDOp, lpDOp, head + frameSize + tail,\
		     gd->begFrameNr, gd->endFrameNr) < 0) {
      if(CREATED)
	freeDObj(lpDOp);
      return(NULL);
    }
    if(aoPtr->options & AOPT_INIT_ONLY) {
      aoPtr->options &= ~AOPT_INIT_ONLY;                /* clear flag */
      return(lpDOp);                                   /* no analysis */
    }
  }
  /* set global values and allocate local buffer space */
  if(setGlobals(lpDOp) < 0) {
    if(CREATED)
      freeDObj(lpDOp);
    return(NULL);
  }
  if(TRACE['A']) {
    fprintf(traceFP, "Analysis parameters\n");
    fprintf(traceFP, "  sample rate = %.1f Hz\n", lpDOp->sampFreq);
    fprintf(traceFP, "  window size = %ld samples\n", frameSize);
    fprintf(traceFP, "  window shift = %ld samples\n", frameShift);
    fprintf(traceFP, "  window function = %s\n",\
	    wfSpecs(gd->winFunc)->entry->code);
    for(lPtr = lpType; lPtr->ident != NULL; lPtr++) {
      if(gd->dataType == lPtr->type) {
	fprintf(traceFP, "  parameter type = %s\n", lPtr->ident);
	break;
      }
    }
    fprintf(traceFP, "  preemphasis = %.7f\n", gd->preEmph);
    fprintf(traceFP, "  analysis order = %ld\n", order);
    fprintf(traceFP, "  selected channel = %d\n", gd->channel);
    fprintf(traceFP, "  start frame = %ld\n", gd->begFrameNr);
    fprintf(traceFP, "  end frame = %ld\n", gd->endFrameNr);
    fprintf(traceFP, "  processing mode = %s-to-%s\n",\
	    FILE_IN ? "file" : "memory", FILE_OUT ? "file" : "memory");
  }
  /* loop over frames */
  err = 0;
  clrAsspMsg();
  for(fn = gd->begFrameNr; fn < gd->endFrameNr; fn++) {
    if((err=getSmpFrame(smpDOp, fn, frameSize, frameShift, head, tail,\
			gd->channel, (void *)frame, LP_PFORMAT)) < 0) {
      break;
    }
    dPtr = &frame[head];
    for(i = 0; i < frameSize; i++)
      rmsBuf[i] = *(dPtr++);
    if(gd->winFunc > WF_RECTANGLE)
      mulSigWF(rmsBuf, wfc, frameSize);
    data.RMS = getRMS(rmsBuf, frameSize) / wfGain;
    /* convert to dB */
    if(data.RMS <= RMS_MIN_AMP)
      data.RMS = RMS_MIN_dB;
    else
      data.RMS = LINtodB(data.RMS);
    dPtr = &frame[head];                           /* reset pointer */
    preEmphasis(dPtr, gd->preEmph, frame[0], frameSize);
    if(gd->winFunc > WF_RECTANGLE)
      mulSigWF(dPtr, wfc, frameSize);
    if(TRACE['N'])
      getMeanACF(dPtr, acf, frameSize, order);
    else
      getACF(dPtr, acf, frameSize, order);
    if(asspDurbin(acf, lpc, rfc, &(data.gain), order) < 0) {
      bPtr = &applMessage[strlen(applMessage)];
      if(FILE_IN)
	snprintf(bPtr, sizeof(applMessage) - strlen(applMessage), "\nat T = %.4f in %s",\
		FRMNRtoTIME(fn, smpDOp->sampFreq, frameShift),\
		myfilename(smpDOp->filePath));
      else
	snprintf(bPtr, sizeof(applMessage) - strlen(applMessage), "\nat T = %.4f",\
		FRMNRtoTIME(fn, smpDOp->sampFreq, frameShift));
      if(TRACE['F'] || TRACE['f'])
	prtAsspMsg(traceFP);
    }
    else {
      if(!TRACE['N'])
	data.gain /= (double)frameSize; /* mean */
      data.gain /= (wfGain * wfGain);   /* squared error! */
    }
    /* convert to dB */
    if(data.gain <= GAIN_MIN_SQR)
      data.gain = GAIN_MIN_dB;
    else
      data.gain = SQRtodB(data.gain);
    switch(gd->dataType) {               /* select further conversion */
    case DT_ARF:
      err = rfc2arf(rfc, data.lpData, order);
      break;
    case DT_LAR:
      err = rfc2lar(rfc, data.lpData, order);
      break;
    default:      /* stored via mapping of 'lpc' or 'rfc' on 'lpData' */
      break;
    }
    if((err=storeLP(&data, fn, lpDOp)) < 0) {
      break;
    }
  } /* END loop over frames */
  if(err >= 0 && FILE_OUT)
    err = asspFFlush(lpDOp, gd->writeOpts);
  freeGlobals();
  if(err < 0) {
    if(CREATED)
      freeDObj(lpDOp);
    return(NULL);
  }
  return(lpDOp);
}

/*DOC

Function 'freeLP_GD'

Returns all memory allocated for the generic data in an LP data object.

DOC*/

void freeLP_GD(void *generic)
{
  if(generic != NULL) {
    free(generic);
  }
  return;
}

/* ======================= private  functions ======================= */

/***********************************************************************
* set local global values and allocate memory for the frame buffers,   *
* the window coefficients and the LP coefficients                      *
***********************************************************************/
LOCAL int setGlobals(DOBJ *dop)
{
  int    wFlags;
  long   frameSize, frameShift;
  LP_GD *gd;

  rmsBuf = frame = wfc = acf = lpc = rfc = NULL;
  gd = (LP_GD *)(dop->generic);
  frameSize = gd->frameSize;
  frameShift = dop->frameDur;
  if(gd->winFunc > WF_RECTANGLE) {
    /* because of the relationship between the autocorrelation and */
    /* the power spectrum, we use the 'proper' periodical window */
    wFlags = WF_PERIODIC;
    if((ODD(frameSize) && EVEN(frameShift)) ||
       (EVEN(frameSize) && ODD(frameShift)) )
      wFlags = WF_ASYMMETRIC; /* align window centre and frame centre */
    wfc = makeWF(gd->winFunc, frameSize, wFlags);
    if(wfc == NULL) {
      setAsspMsg(AEG_ERR_MEM, "LP: setGlobals");
      return(-1);
    }
    wfGain = wfCohGain(wfc, frameSize);
  }
  else /* wfc already NULL */
    wfGain = 1.0;
  if(gd->dataType != DT_LPC) {      /* need reflection coefficients */
    rfc = (double *)calloc((size_t)(gd->order), sizeof(double));
    if(rfc == NULL) {
      freeGlobals();
      setAsspMsg(AEG_ERR_MEM, "LP: setGlobals");
      return(-1);
    }
  } /* else rfc = NULL; */
  rmsBuf = (double *)calloc((size_t)frameSize, sizeof(double));
  /* one sample extra for preemphasis */
  frame = (double *)calloc((size_t)(frameSize + 1), sizeof(double));
  acf = (double *)calloc((size_t)(gd->order + 1), sizeof(double));
  lpc = (double *)calloc((size_t)(gd->order + 1), sizeof(double));
  if(rmsBuf == NULL || frame == NULL || acf == NULL || lpc == NULL) {
    freeGlobals();
    setAsspMsg(AEG_ERR_MEM, "LP: setGlobals");
    return(-1);
  }
  if(gd->dataType == DT_RFC)
    data.lpData = rfc;
  else
    data.lpData = lpc;
  return(0);
}

/***********************************************************************
* free memory allocated for the local buffers                          *
***********************************************************************/
LOCAL void freeGlobals(void)
{
  freeWF(wfc);
  wfc = NULL;
  if(rmsBuf != NULL) {
    free((void *)rmsBuf);
    rmsBuf = NULL;
  }
  if(frame != NULL) {
    free((void *)frame);
    frame = NULL;
  }
  if(acf != NULL) {
    free((void *)acf);
    acf = NULL;
  }
  if(lpc != NULL) {
    free((void *)lpc);
    lpc = NULL;
  }
  if(rfc != NULL) {
    free((void *)rfc);
    rfc = NULL;
  }
  return;
}

/***********************************************************************
* copy frame data to output buffer; handle data writes                 *
***********************************************************************/
LOCAL int storeLP(LP_OUT *LPtr, long frameNr, DOBJ *dop)
{
  int    FILE_OUT;
  size_t numBytes;
  long   ndx;
  float *fPtr;
  DDESC *dd;
  LP_GD *gd;

  FILE_OUT = (dop->fp != NULL);
  gd = (LP_GD *)dop->generic;
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
      setAsspMsg(AEG_ERR_BUG, "storeLP: buffer overflow");
      return(-1);
    }
  }
  ndx = frameNr - dop->bufStartRec;
  numBytes = ndx * dop->recordSize;           /* offset to frame data */
  fPtr = (float *)((void *)((char *)dop->dataBuffer + numBytes));
  dd = &(dop->ddl);
  *(fPtr++) = (float)(LPtr->RMS);
  dd = dd->next;
  *(fPtr++) = (float)(LPtr->gain);
  dd = dd->next;
  numBytes = dd->numFields * sizeof(double);
  memcpy((void *)fPtr, (void *)(LPtr->lpData), numBytes);
  if(ndx >= dop->bufNumRecs)
    dop->bufNumRecs = ndx + 1;
  dop->bufNeedsSave = TRUE;
  return(0);
}
