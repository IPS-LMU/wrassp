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
* File:     asspana.c                                                  *
* Contents: Support functions for speech analyses.                     *
*           Prototypes of these functions are in asspana.h.            *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: asspana.c,v 1.14 2010/07/14 13:44:30 mtms Exp $ */

#include <stdio.h>     /* NULL */
#include <stdlib.h>    /* calloc() free() */
#include <string.h>    /* memset() strcpy() strchr() */

#include <miscdefs.h>  /* TRUE FALSE */
#include <asspmess.h>  /* error message handler */
#include <assptime.h>  /* standard conversion macros */
#include <asspana.h>   /* AOPTS ATIME */
#include <asspdsp.h>   /* wfunc_e wfType() wfSpecs() */
#include <asspfio.h>   /* AFO_READ */
#include <dataobj.h>   /* DOBJ */

/*DOC

This function converts timing parameters like window size and shift and 
analysis range in the analysis options structure pointed to by "aoPtr" 
to sample/frame numbers using the data in the audio object pointed to 
by "smpDOp" and stores them in the structure pointed to by "tPtr". None 
of these pointers may be NULL.
This function will return -1 upon error and 1 if the analysis interval 
is empty. Otherwise it will return 0.

Note:
 - This is a fairly low level function: it is assumed that all data 
   provided have already been verified. 

DOC*/

/** 
 * Analysis Timing. This function converts timing parameters like window size and shift 
 * and analysis range in the analysis options structure pointed to by  
 * "aoPtr" to sample/frame numbers using the data in the audio object
 * pointed to by "smpDOp" and stores them in the structure pointed to by
 * "tPtr". None of these pointers may be NULL.
 * @param smpDOp 
 * @param aoPtr 
 * @param tPtr 
 * @return This function will return -1 upon error and 1 if the analysis 
 * interval is empty. Otherwise it will return 0.
 * @note This is a fairly low level function; it is assumed that all
 * data provided have already been verified.
 */
int anaTiming(DOBJ *smpDOp, AOPTS *aoPtr, ATIME *tPtr)
{
  long    begSn, endSn; 
  long    begSmpNr, begFrmNr, endSmpNr, endFrmNr;
  double  winSize;
  wfunc_e winFunc;

  if(smpDOp == NULL || aoPtr == NULL || tPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "anaTiming");
    return(-1);
  }
  clrAsspMsg();                       /* we may need to set a warning */
  /*
   * Set frame timing parameters.
   */
  tPtr->sampFreq = smpDOp->sampFreq;
  if(aoPtr->options & AOPT_USE_CTIME) {          /* overrules msShift */
    tPtr->frameShift = 1;
  }
  else {
    tPtr->frameShift = TIMEtoSMPNR((aoPtr->msShift)/1000.0, tPtr->sampFreq);
    if(tPtr->frameShift < 1)/* allow zero value to get smallest shift */
      tPtr->frameShift = 1;
  }
  if(aoPtr->options & AOPT_USE_ENBW) {            /* overrules msSize */
    if(aoPtr->bandwidth <= 0.0) {
      if(aoPtr->FFTLen <= 0) {
	setAsspMsg(AEG_ERR_BUG, "anaTiming: bandwidth undefined");
	return(-1);
      }
      tPtr->frameSize = aoPtr->FFTLen; /* smallest possible bandwidth */
    }
    else {
      winSize = 1.0 / aoPtr->bandwidth;
      if(strlen(aoPtr->winFunc) != 0) {
	winFunc = wfType(aoPtr->winFunc);
	if(winFunc <= WF_ERROR || winFunc >= WF_NUM_FIX) {
	  setAsspMsg(AEB_BAD_WIN, aoPtr->winFunc);
	  return(-1);
	}
	if(winFunc != WF_NONE && winFunc != WF_RECTANGLE)
	  winSize *= (wfSpecs(winFunc))->enbw;
      }
      tPtr->frameSize = TIMEtoSMPNR(winSize, tPtr->sampFreq);
    }
    if(tPtr->frameSize < WF_MIN_SIZE) {
      setAsspMsg(AED_ERR_SIZE, "anaTiming");
      return(-1);
    }
  }
  else if(aoPtr->msSize <= 0.0) {
    /* this is possible (e.g. f0_ksv, f0_mhs ..) */
    if(aoPtr->options & AOPT_USE_CTIME) {    /* but not in event mode */
      setAsspMsg(AED_ERR_SIZE, "anaTiming");
      return(-1);
    }
    tPtr->frameSize = 0;
  }
  else {
    winSize = aoPtr->msSize / 1000.0;
    if(aoPtr->options & AOPT_EFFECTIVE) {         /* effective length */
      if(strlen(aoPtr->winFunc) != 0) {
	winFunc = wfType(aoPtr->winFunc);
	if(winFunc <= WF_ERROR || winFunc >= WF_NUM_FIX) {
	  setAsspMsg(AEB_BAD_WIN, aoPtr->winFunc);
	  return(-1);
	}
	if(winFunc != WF_NONE && winFunc != WF_RECTANGLE)
	  winSize *= (wfSpecs(winFunc))->enbw;
      }
    }
    tPtr->frameSize = TIMEtoSMPNR(winSize, tPtr->sampFreq);
    if(tPtr->frameSize < WF_MIN_SIZE) {
      setAsspMsg(AED_ERR_SIZE, "anaTiming");
      return(-1);
    }
  }
  if(aoPtr->msSmooth > 0.0)  /* value assumed to be checked by caller */
    tPtr->smoothSize = TIMEtoSMPNR((aoPtr->msSmooth)/1000.0, tPtr->sampFreq);
  else
    tPtr->smoothSize = 0;
  /*
   * Round analysis interval to frame boundaries within available range.
   * If the audio object refers to a file opened for reading, this will  
   * be the data range in the file, otherwise that in the data buffer.
   */
  /* if(smpDOp->fp != NULL && (smpDOp->openMode & AFO_READ)) { */
  if(smpDOp->fp != NULL) {  /* will find out when not readable */
    begSn = smpDOp->startRecord;
    endSn = begSn + smpDOp->numRecords;
  }
  else {
    begSn = smpDOp->bufStartRec;
    endSn = begSn + smpDOp->bufNumRecs;
  }
  if(aoPtr->options & AOPT_USE_CTIME) {    /* overrules begin/endTime */
    /* NOTE: frameShift is in this case p.d. 1 (see above) */
    begFrmNr = begSmpNr = TIMEtoSMPNR(aoPtr->centreTime, tPtr->sampFreq);
    if(begSmpNr < begSn || begSmpNr >= endSn) {
      setAsspMsg(AED_ERR_RANGE, "anaTiming");
      return(-1);
    }
    endFrmNr = begFrmNr + 1;
  }
  else {
    if(aoPtr->beginTime <= 0.0)                 /* use begin of range */
      begFrmNr = 0;
    else
      begFrmNr = TIMEtoFRMNR(aoPtr->beginTime, tPtr->sampFreq,\
			     tPtr->frameShift);
    begSmpNr = FRMNRtoSMPNR(begFrmNr, tPtr->frameShift);
    if(begSmpNr < begSn) {
      begFrmNr = SMPNRtoFRMNR(begSn, tPtr->frameShift);
      begSmpNr = FRMNRtoSMPNR(begFrmNr, tPtr->frameShift);
    }
    if(aoPtr->endTime <= 0.0)                     /* use end of range */
      endFrmNr = SMPNRtoFRMNR(endSn, tPtr->frameShift);
    else
      endFrmNr = TIMEtoFRMNR(aoPtr->endTime, tPtr->sampFreq,\
			     tPtr->frameShift);
    endSmpNr = FRMNRtoSMPNR(endFrmNr, tPtr->frameShift);
    if(endSmpNr > endSn) {
      endFrmNr = SMPNRtoFRMNR(endSn, tPtr->frameShift);
      endSmpNr = FRMNRtoSMPNR(endFrmNr, tPtr->frameShift);
    }
    if(begFrmNr >= endFrmNr || begSmpNr >= endSn) {
      setAsspMsg(AWD_NO_DATA, "anaTiming");              /* set warning */
      endFrmNr = begFrmNr;                        /* set empty interval */
    }
  }
  tPtr->begFrameNr = begFrmNr;
  tPtr->endFrameNr = endFrmNr;
  if(asspWarning)
    return(1);
  return(0);
}

/*DOC

This function verifies whether the data buffers in the audio object 
pointed to by "smpDOp" and the analysis data object pointed to by 
"anaDOp" are present and have an appropriate size. If this is not the 
case, they will be (re-)allocated. For the analysis data object, care 
will be taken that valid data in the buffer will not be destroyed. For 
this the arguments "begFrameNr" and "endFrameNr" should specify the 
current analysis interval. "frameSamples" should typically be set to 
the total number of samples needed to compute one output frame
This function returns -1 upon error, otherwise it returns 0.

Note:
 - This function makes a number of assumptions and is therefore not for 
   public use. In particular, it is tuned to be used with the functions 
   'getSmpFrame' or 'getSmpPtr'.
 - The items 'startRecord' and 'numRecords' in "anaDOp" need be managed 
   by the calling function.

DOC*/

int checkDataBufs(DOBJ *smpDOp, DOBJ *anaDOp, long frameSamples,\
		  long begFrameNr, long endFrameNr)
{
  uint8_t *bPtr;
  long     i, startRecord, numRecords, frameShift, numFrames;
  long     bufBegFN, bufEndFN;

  frameShift = anaDOp->frameDur;
  numFrames = endFrameNr - begFrameNr;
  if(frameShift < 1 || numFrames < 1) {
    setAsspMsg(AEB_BAD_CALL, "checkDataBufs");
    return(-1);
  }
  clrAsspMsg();
  /* check audio buffer */
  if(smpDOp->fp != NULL) {
    if(smpDOp->dataBuffer == NULL || smpDOp->maxBufRecs < frameSamples) {
      if(smpDOp->dataBuffer != NULL) {
	if(smpDOp->doFreeDataBuf == NULL) {       /* apparently fixed */
	  setAsspMsg(AEB_BUF_SPACE, "(checkDataBufs)");
	  return(-1);
	}
	freeDataBuf(smpDOp);
      }
      /* now have no audio buffer; determine size and allocate it */
      if(smpDOp->recordSize < 1) {
	setAsspMsg(AEG_ERR_BUG, "checkDataBufs: invalid recordSize");
	return(-1);
      }
      numRecords = ANA_BUF_BYTES / smpDOp->recordSize;
      if(numRecords < frameSamples) {
	numRecords = frameSamples;
	if(frameSamples / frameShift >= 4) {
	  if(numFrames > 4)
	    numRecords += (4 * frameShift);
	  else
	    numRecords += ((numFrames-1) * frameShift);
	}
      }
      /* else { */ /* OBSOLETE: for record/frameIndex() */
      /*   i = frameSamples + (numFrames-1)*frameShift; */
      /*   if(i < numRecords) */
      /*     numRecords = i; */
      /* } */
      if(numRecords > smpDOp->numRecords)/* NEW: zero padding done in */
	numRecords = smpDOp->numRecords;  /* frame buffer / workspace */
      if(allocDataBuf(smpDOp, numRecords) == NULL)
	return(-1);
    } /* else sufficient space */
    smpDOp->bufStartRec = smpDOp->bufNumRecs = 0; /* discard contents */
  } /* else MEMORY_IN (checked by calling function) */
  /* check analysis buffer */
  if(anaDOp->fp != NULL) {
    if(anaDOp->dataBuffer == NULL) {
      if(anaDOp->recordSize < 1) {
	setAsspMsg(AEG_ERR_BUG, "checkDataBufs: invalid recordSize");
	return(-1);
      }
      numRecords = ANA_BUF_BYTES / anaDOp->recordSize;
      if(numRecords < 64)    /* let's set a minimum for large records */
	numRecords = 64;
      if(numRecords > numFrames)
	numRecords = numFrames;
      if(allocDataBuf(anaDOp, numRecords) == NULL)
	return(-1);
    } /* else any size will do */
    anaDOp->bufNumRecs = 0;                /* no valid data in buffer */
    anaDOp->bufNeedsSave = FALSE;
  }
  else { /* results to be kept in memory */
    if(anaDOp->dataBuffer != NULL) {
      if(anaDOp->bufNumRecs <= 0) {        /* no valid data in buffer */
	if(anaDOp->maxBufRecs < numFrames) {     /* insufficient size */
	  if(anaDOp->doFreeDataBuf == NULL) {     /* apparently fixed */
	    setAsspMsg(AEB_BUF_SPACE, "(checkDataBufs)");
	    return(-1);
	  }
	  freeDataBuf(anaDOp);          /* will be re-allocated below */
	}
	else {
	  anaDOp->bufNumRecs = 0;
	  anaDOp->bufStartRec = begFrameNr;
	  anaDOp->bufNeedsSave = FALSE;
	}
      }
      else {    /* complicated: want to keep valid data outside range */
	bufBegFN = anaDOp->bufStartRec;
	bufEndFN = bufBegFN + anaDOp->bufNumRecs;
	if(bufBegFN > begFrameNr ||\
	   bufBegFN + anaDOp->maxBufRecs < endFrameNr) {
	  if(bufBegFN < begFrameNr)
	    startRecord = bufBegFN;
	  else
	    startRecord = begFrameNr;
	  if(bufBegFN + anaDOp->maxBufRecs > endFrameNr)
	    numRecords = bufBegFN + anaDOp->maxBufRecs - startRecord;
	  else
	    numRecords = endFrameNr - startRecord;
	  if(anaDOp->recordSize < 1) {
	    setAsspMsg(AEG_ERR_BUG, "checkDataBufs: invalid recordSize");
	    return(-1);
	  }
	  if(anaDOp->doFreeDataBuf == NULL) {     /* apparently fixed */
	    setAsspMsg(AEB_BUF_SPACE, "(checkDataBufs)");
	    return(-1);
	  }
	  bPtr = (uint8_t *)calloc((size_t)numRecords, anaDOp->recordSize);
	  if(bPtr == NULL) {
	    setAsspMsg(AEG_ERR_MEM, "(checkDataBufs)");
	    return(-1);
	  }
	  i = (bufBegFN - startRecord) * anaDOp->recordSize;
	  memcpy((void *)&bPtr[i], anaDOp->dataBuffer,\
		 anaDOp->bufNumRecs * anaDOp->recordSize);
	  freeDataBuf(anaDOp);
	  anaDOp->dataBuffer = (void *)bPtr;
	  anaDOp->doFreeDataBuf = (DOfreeFunc)free;
	  anaDOp->maxBufRecs = numRecords;
	  anaDOp->bufStartRec = startRecord;
	  anaDOp->bufNumRecs = bufEndFN - startRecord;
	  anaDOp->bufNeedsSave = TRUE;
	}
     /* else analysis range is within buffer range */
      }
    }
    if(anaDOp->dataBuffer == NULL) {
      if(allocDataBuf(anaDOp, numFrames) == NULL)
	return(-1);
      anaDOp->bufStartRec = begFrameNr;
    }
  }
  return(0);
}
