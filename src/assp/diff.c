/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 2003 - 2010  Michel Scheffers                          *
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
* File:     diff.c                                                     *
* Contents: Functions for the differentiation of audio/EGG signals     *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: diff.c,v 1.2 2010/03/30 14:38:05 mtms Exp $ */

#include <stdio.h>     /* NULL */
#include <inttypes.h>  /* int16_t etc. */
#include <string.h>    /* memset() */
#include <math.h>      /* fabs() */

#include <miscdefs.h>  /* TRUE FALSE LOCAL */
#include <misc.h>      /* myrint() */
#include <mylimits.h>  /* INT16_MAX etc. */
#include <trace.h>     /* trace handler */
#include <asspmess.h>  /* error message handler */
#include <dataobj.h>   /* DOBJ getSmpCaps() getSmpPtr() */
#include <diff.h>      /* DIFF_... */
#include <aucheck.h>   /* checkSound() */
#include <auconv.h>    /* int32_to_int24() */
#include <asspana.h>   /* AOPTS */
#include <asspfio.h>   /* asspFFlush() */

/*
 * local global variables and arrays
 */
LOCAL DOBJ *workDOp=NULL; /* work object (allocated) */

/*
 * prototypes of private functions
 */
LOCAL int  verifyDiffBufs(DOBJ *inpDOp, DOBJ *outDOp, long options);
LOCAL void freeDiffBufs(void);
LOCAL int  storeDiff(int32_t value, long sampleNr, DOBJ *dop);

/* ======================== public functions ======================== */

/*DOC

This function sets the items in the analysis options structure relevant 
to the differentiation of a signal to their default values. It will 
clear all other items.
The function returns 0 upon success and -1 upon error.

DOC*/

int setDiffDefaults(AOPTS *aoPtr)
{
  if(aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setDiffDefaults");
    return(-1);
  }
  memset((void *)aoPtr, 0, sizeof(AOPTS));         /* clear all items */
  aoPtr->beginTime = aoPtr->endTime = -1.0;      /* always full range */
  aoPtr->centreTime = -1.0;                           /* de-activate  */
  aoPtr->channel = DIFF_DEF_CHANNEL;              /* selected channel */
  return(0);
}

/*DOC

This function allocates memory for a data object to hold a differentiated 
signal and initializes it on the basis of the audio object pointed to by 
"inpDOp" and the option settings in the analysis options structure 
pointed to by "aoPtr". Neither of these pointers may be NULL.
It returns a pointer to the data object or NULL upon error.

Note:
 - This function DOES NOT allocate memory for the data buffer. Depending 
   on your application you can either use 'allocDataBuf' for this or 
   delegate it to 'diffSignal'.
 - If "inpDOp" does not refer to an open file, the items 'startRecord'
   and 'numRecords' in the returned object will be set to correspond to 
   the contents of the data buffer of "inpDOp". They might need to be 
   corrected.

DOC*/

DOBJ *createDiff(DOBJ *inpDOp, AOPTS *aoPtr)
{
  long  auCaps;
  DOBJ *dop=NULL;

  if(inpDOp == NULL || aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "createDiff");
    return(NULL);
  }
  /* verify input object */
  if((auCaps=getSmpCaps(DIFF_PFORMAT)) <= 0)
    return(NULL);
  auCaps |= DIFF_I_CHANS;
  if(aoPtr->channel < 1)
    aoPtr->channel = 1;
  if(checkSound(inpDOp, auCaps, aoPtr->channel) <= 0)
    return(NULL);
  if((dop=allocDObj()) == NULL)
    return(NULL);
  if(copyDObj(dop, inpDOp) < 0) {
    freeDObj(dop);
    return(NULL);
  }
  if(dop->ddl.numFields > DIFF_O_CHANS) {
    dop->ddl.numFields = DIFF_O_CHANS;
    setRecordSize(dop);                   /* needs to be recalculated */
  }
  if(inpDOp->fp == NULL) {
    /* for memory-to-file mode: prepare for writing header */
    if(dop->fileFormat == FF_SSFF) {
      dop->startRecord = inpDOp->bufStartRec;
      setStart_Time(dop);
    }
    else {
      dop->startRecord = 0;
      dop->Start_Time = dop->Time_Zero = 0.0;
    }
    dop->numRecords = inpDOp->bufNumRecs;
  } /* otherwise copied from "inpDOp" */
  clrAsspMsg();
  return(dop);
}

/*DOC

This function differentiates the audio-formatted signal referred to by 
"inpDOp" using the settings specified in the analysis options structure 
pointed to by "aoPtr". Neither of these pointers may be NULL. 
The differentiated signal will be returned in the data buffer of the 
object pointed to by "outDOp" or written to file if that object refers 
to a file opened for writing. If "outDOp" is a NULL-pointer the output 
object will be created (see 'createDiff' for details).
This function will verify and - if necessary - (re-)allocate the data 
buffers in the data objects pointed to by "inpDOp" and "outDOp" to have 
appropriate size.
This function returns a pointer to the output data object or NULL upon 
error.

Note:
 - This function may be used in a file-to-file, file-to-memory, memory-
   to-file and memory-to-memory mode.

DOC*/

DOBJ *diffSignal(DOBJ *inpDOp, AOPTS *aoPtr, DOBJ *outDOp)
{
  int      FILE_IN, FILE_OUT, CREATED, STORE;
  int      err, pass;
  long     auCaps, sn, osn, begSmpNr, endSmpNr, head, tail;
  int32_t *sPtr=NULL;
  double   diff, absMaxMag, maxMag, scaleFac;

  if(inpDOp == NULL || aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "diffSignal");
    return(NULL);
  }
  FILE_IN = FILE_OUT = CREATED = FALSE;
  /* check input object */
  if(inpDOp->fp != NULL) {
    if(inpDOp->numRecords <= 0) {
      setAsspMsg(AEF_EMPTY, inpDOp->filePath);
      return(NULL);
    }
    FILE_IN = TRUE;
  }
  else if(!(aoPtr->options & AOPT_INIT_ONLY)) {
    if(inpDOp->dataBuffer == NULL || inpDOp->bufNumRecs <= 0) {
      setAsspMsg(AED_NO_DATA, "(diffSignal)");
      return(NULL);
    }
  }
  /* check status of output object */
  if(outDOp == NULL) {
    if((outDOp=createDiff(inpDOp, aoPtr)) == NULL)
      return(NULL);
    CREATED = TRUE;
  }
  else {
    if((auCaps=getSmpCaps(DIFF_PFORMAT)) <= 0)
      return(NULL);
    auCaps |= DIFF_I_CHANS;
    if(aoPtr->channel < 1)
      aoPtr->channel = 1;
    if(checkSound(inpDOp, auCaps, aoPtr->channel) <= 0)
      return(NULL);
    auCaps &= ~AUC_CHAN_MASK;
    auCaps |= DIFF_O_CHANS;
    if(checkSound(outDOp, auCaps, 0) <= 0)
      return(NULL);
    if(checkSound(outDOp, auCapsFF(outDOp->fileFormat), 0) <= 0)
      return(NULL); /* early warning */
    FILE_OUT = (outDOp->fp != NULL);
  }
  if(verifyDiffBufs(inpDOp, outDOp, aoPtr->options) < 0) {
    if(CREATED)
      freeDObj(outDOp);
    return(NULL);
  }

  if(TRACE['p']) {
    fprintf(traceFP, "Processing parameters\n");
    fprintf(traceFP, "  selected channel = %d\n", aoPtr->channel);
    if(aoPtr->options & DIFF_OPT_BACKWARD)
      fprintf(traceFP, "  difference = backward\n");
    else if(aoPtr->options & DIFF_OPT_CENTRAL)
      fprintf(traceFP, "  difference = central\n");
    else
      fprintf(traceFP, "  difference = forward\n");
    fprintf(traceFP, "  processing mode = %s-to-%s\n",\
	    FILE_IN ? "file" : "memory", FILE_OUT ? "file" : "memory");
  }

  /* set processing range */
  if(FILE_IN) {
    begSmpNr = inpDOp->startRecord;
    endSmpNr = begSmpNr + inpDOp->numRecords;
  }
  else {
    begSmpNr = inpDOp->bufStartRec;
    endSmpNr = begSmpNr + inpDOp->bufNumRecs;
  }
  if(aoPtr->options & DIFF_OPT_BACKWARD) {
    head = 1;
    tail = 0;
  }
  else if(aoPtr->options & DIFF_OPT_CENTRAL) {
    head = tail = 1;
  }
  else {
    head = 0;
    tail = 1;
  }
  /* set overflow threshold */
  switch(outDOp->ddl.format) {
  case DF_INT16:
    absMaxMag = INT16_MAX;
    break;
  case DF_INT24:
    absMaxMag = INT24_MAX;
    break;
  case DF_INT32:
    /* AIFF specifies that the upper byte should be used */
    absMaxMag = INT32_MAX;
    break;
  default: /* to appease the compiler */
    absMaxMag = INT16_MAX;
    break;
  }
  maxMag = 0.0;
  scaleFac = 1.0;
  /* make up to 2 passes to detect and - if necessary - correct overflow */
  for(pass = 1; pass <= 2; pass++) {
    err = 0;
    STORE = TRUE;
    /* loop over samples */
    osn = outDOp->startRecord;  /* may be different !!! */
    for(sn = begSmpNr; sn < endSmpNr; sn++, osn++) {
      /* do some checking here to avoid overhead in 'getSmpPtr' */
      if(sn == begSmpNr ||\
	 sn >= workDOp->bufStartRec + workDOp->bufNumRecs - tail) {
	sPtr = (int32_t *)getSmpPtr(inpDOp, sn, head, tail,\
				    aoPtr->channel, workDOp);
	if(sPtr == NULL) {
	  err = -1;
	  break;
	}
      } 
      if(aoPtr->options & DIFF_OPT_BACKWARD) {
	sPtr--;                      /* sn - 1 */
	diff = -(double)(*(sPtr++)); /* sn */
	diff += (double)(*(sPtr++)); /* sn + 1 */
      }
      else if(aoPtr->options & DIFF_OPT_CENTRAL) {
	sPtr--;                      /* sn - 1 */
	diff = -(double)(*(sPtr++)); /* sn */
	sPtr++;                      /* sn + 1 */
	diff += (double)(*sPtr);
	diff /= 2.0;
      }
      else {
	diff = -(double)(*(sPtr++)); /* sn + 1 */
	diff += (double)(*sPtr);
      }
      diff = myrint(diff);
      if(pass == 1) {
	if(fabs(diff) > maxMag) {
	  maxMag = fabs(diff);
	  if(maxMag > absMaxMag)
	    STORE = FALSE;
	}
      }
      else {  /* pass == 2 : scale down */
	diff *= scaleFac;
      }
      if(STORE) {
	if((err=storeDiff((int32_t)diff, osn, outDOp)) < 0)
	  break;
      }
    } /* END loop over samples */
    if(err < 0) break;
    if(pass == 1) {
      if(STORE) break; /* no overflow */
      scaleFac = absMaxMag * (AF_MAX_GAIN / 100.0) / maxMag;
    }
  } /* END loop over passes */
  if(err >= 0 && FILE_OUT)
    err = asspFFlush(outDOp, AFW_CLEAR);
  freeDiffBufs();
  if(err < 0) {
    if(CREATED)
      freeDObj(outDOp);
    return(NULL);
  }
  return(outDOp);
}

/* ======================= private  functions ======================= */

/***********************************************************************
* verify size of data buffers of input and output objects, initialize  *
* work object and allocate memory for its data buffer                  *
***********************************************************************/
LOCAL int verifyDiffBufs(DOBJ *inpDOp, DOBJ *outDOp, long options)
{
  int  FILE_IN, FILE_OUT;
  long maxSamples, numSamples;

  FILE_IN = (inpDOp->fp != NULL);
  if(FILE_IN) {
    maxSamples = inpDOp->numRecords;
    if(inpDOp->dataBuffer == NULL || inpDOp->maxBufRecs < 3) {
      if(inpDOp->dataBuffer != NULL) {
	if(inpDOp->doFreeDataBuf == NULL) {       /* apparently fixed */
	  setAsspMsg(AEB_BUF_SPACE, "(verifyDiffBufs)");
	  return(-1);
	}
	freeDataBuf(inpDOp);
      }
      /* now have no input buffer; determine size and allocate it */
      if(inpDOp->recordSize < 1) {
	setAsspMsg(AEG_ERR_BUG, "verifyDiffBufs: invalid recordSize");
	return(-1);
      }
      numSamples = ANA_BUF_BYTES / inpDOp->recordSize;
      if(numSamples > maxSamples)
	numSamples = maxSamples;
      if(allocDataBuf(inpDOp, numSamples) == NULL)
	return(-1);
    } /* else any size will do */
  }
  else /* MEMORY_IN */
    maxSamples = inpDOp->bufNumRecs;
  FILE_OUT = (outDOp->fp != NULL);
  if(outDOp->dataBuffer == NULL ||\
     (FILE_OUT && outDOp->maxBufRecs < 1) ||\
     (!FILE_OUT && outDOp->maxBufRecs < maxSamples)) {
    if(outDOp->dataBuffer != NULL) {
      if(outDOp->doFreeDataBuf == NULL) {
	setAsspMsg(AEB_BUF_SPACE, "(verifyDiffBufs)");
	return(-1);
      }
      freeDataBuf(outDOp);
    }
    if(outDOp->recordSize < 1) {
      setAsspMsg(AEG_ERR_BUG, "verifyDiffBufs: invalid recordSize");
      return(-1);
    }
    if(FILE_OUT) {
      numSamples = ANA_BUF_BYTES / outDOp->recordSize;
      if(numSamples > maxSamples)
	numSamples = maxSamples;
    }
    else /* MEMORY_OUT */
      numSamples = maxSamples;
    if(allocDataBuf(outDOp, numSamples) == NULL)
      return(-1);
  }
  workDOp = allocDObj();
  if(workDOp == NULL)
    return(-1);
  if(copyDObj(workDOp, outDOp) < 0) {
    freeDiffBufs();
    return(-1);
  }
  workDOp->ddl.format = DIFF_PFORMAT;        /* set processing format */
  setRecordSize(workDOp);                      /* adjust if necessary */
  numSamples = ANA_BUF_BYTES / workDOp->recordSize;
  /* optimize transfer in 'getSmpPtr' */
  if(FILE_IN) {
    if(numSamples > inpDOp->maxBufRecs)
      numSamples = inpDOp->maxBufRecs;
  }
  else {
    if(numSamples > inpDOp->bufNumRecs)
      numSamples = inpDOp->bufNumRecs;
  }
  numSamples++;      /* always need one extra sample for head or tail */
  if(options & DIFF_OPT_CENTRAL)
    numSamples++;
  if(allocDataBuf(workDOp, numSamples) == NULL) {
    freeDiffBufs();
    return(-1);
  }
  return(0);
}

/***********************************************************************
* return memory allocated for the workspace                            *
***********************************************************************/
LOCAL void freeDiffBufs(void)
{
  if(workDOp != NULL) {
    workDOp = freeDObj(workDOp);
  }
  return;
}

/***********************************************************************
* convert differentiated signal to output format; handle data writes   *
***********************************************************************/
LOCAL int storeDiff(int32_t value, long sampleNr, DOBJ *dop)
{
  uint8_t *u8Ptr;
  int16_t *i16Ptr;
  int32_t *i32Ptr;
  int      FILE_OUT;
  long     ndx;

  FILE_OUT = (dop->fp != NULL);
  if(dop->bufNumRecs <= 0) {
    dop->bufNumRecs = 0;
    dop->bufStartRec = sampleNr;
  }
  else if(sampleNr >= (dop->bufStartRec + dop->maxBufRecs)) {
    if(FILE_OUT) {
      if(asspFFlush(dop, AFW_CLEAR) < 0)
	return(-1);
    }
    else {
      setAsspMsg(AEG_ERR_BUG, "storeDiff: buffer overflow");
      return(-1);
    }
  }
  ndx = sampleNr - dop->bufStartRec;
  if(ndx >= dop->maxBufRecs) {
    setAsspMsg(AEG_ERR_BUG, "storeDiff: buffer overflow");
    return(-1);
  }
  u8Ptr = (uint8_t *)(dop->dataBuffer);
  u8Ptr += (ndx * dop->recordSize);
  switch(dop->ddl.format) {
  case DF_INT16:
    i16Ptr = (int16_t *)u8Ptr;
    *i16Ptr = (int16_t)value;
    break;
  case DF_INT24:
    int32_to_int24(value, u8Ptr);
    break;
  case DF_INT32:
    i32Ptr = (int32_t *)u8Ptr;
    *i32Ptr = value;
    break;
  default: /* to appease the compiler */
    break;
  }
  if(ndx >= dop->bufNumRecs)
    dop->bufNumRecs = ndx + 1;
  dop->bufNeedsSave = TRUE;
  return(0);
}
