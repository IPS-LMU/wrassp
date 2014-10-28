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
* File:     filter.c                                                   *
* Contents: Functions for filtering audio signals.                     *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: filter.c,v 1.4 2010/08/06 13:25:07 mtms Exp $ */

#include <stdio.h>    /* FILE NULL etc. tmpfile() */
#include <stddef.h>   /* size_t */
#include <stdlib.h>   /* malloc() calloc() free() */
#include <string.h>   /* mem...() str...() */
#include <inttypes.h> /* int16_t etc. */
#include <math.h>     /* fabs() */

#include <miscdefs.h> /* TRUE FALSE LOCAL */
#include <misc.h>     /* strxcmp() mybasename() myrint() */
#include <mylimits.h> /* INT.._MAX */
#include <trace.h>    /* trace handler */
#include <asspmess.h> /* error message handler */
#include <assptime.h> /* standard conversion macros */
#include <dataobj.h>  /* DOBJ getSmpCaps() getSmpPtr() */
#include <aucheck.h>  /* AUC-flags checkSound() */
#include <auconv.h>   /* int32_to_int24() */
#include <filter.h>   /* FILT_... FILT_GD */
#include <asspana.h>  /* AOPTS */
#include <asspdsp.h>  /* AF_.. FILTER FIR IIR2 FILTER_... randTPDF() */
#include <asspfio.h>  /* asspFFill() asspFFlush() */

//#include <R.h>

/* OS check for creating temporary file*/
#ifdef __unix__         

#elif defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64) 
  #define OS_Windows
  #include <windows.h>
#endif

/*
 * local global variables and arrays
 */

LOCAL size_t  numTempFiles, samplesPerTempBlock;
LOCAL long    blocksPerTempFile;
LOCAL double *blockBuffer=NULL;
LOCAL FILE   *tempFP[AF_MAX_TEMP]={NULL};

/*
 * prototypes of local functions
 */
LOCAL int  setGlobals(long totSamples);
LOCAL void freeBufs(void);
LOCAL int  createTempFiles(void);
LOCAL void removeTempFiles(void);
LOCAL int  storeBlock(long begSn, long num, DOBJ *dop);

/* ======================== public functions ======================== */

/*DOC

This function sets the items in the analysis options structure relevant 
to the filtering of audio signals to their default values. It will clear 
all other items.
The function returns 0 upon success and -1 upon error.

DOC*/

int setFILTdefaults(AOPTS *aoPtr)
{
  if(aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setFILTdefaults");
    return(-1);
  }
  memset((void *)aoPtr, 0, sizeof(AOPTS));         /* clear all items */
  aoPtr->beginTime = aoPtr->endTime = -1.0; /* i.e. begin/end of data */
  aoPtr->centreTime = -1.0;                           /* de-activated */
  aoPtr->hpCutOff = FILT_DEF_HP_CO;
  aoPtr->lpCutOff = FILT_DEF_LP_CO;
  aoPtr->stopDB = FILT_DEF_ATTEN;
  aoPtr->tbWidth = FILT_DEF_WIDTH;
  aoPtr->channel = FILT_DEF_CHANNEL;
  strcpy(aoPtr->winFunc, FILT_FIR_WINFUNC);            /* PRELIMINARY */
  /* presets (actived through option flags) */
  aoPtr->gain = AF_DEF_GAIN;
  aoPtr->order = FILT_DEF_SECTS;
  return(0);
}

/*DOC

Determines the characteristics of the audio filter, specified by the 
items 'hpCutOff' and/or 'lpCutOff' in the structure pointed to by 
"aoPtr". See 'setFilterType' in 'filters.c' for further details. 
If "suffix" is not a NULL-pointer, the corresponding file name suffix 
will be copied to that string which must be at least 5 byte long.
The function returns the type code (> 0) upon success and -1 upon error.

Note:
 - This function does not support (anti-)resonance filters.

DOC*/

int getFILTtype(AOPTS *aoPtr, char *suffix)
{
  int fType;

  fType = FILTER_NONE;
  if(aoPtr->hpCutOff > 0.0) {
    if(aoPtr->lpCutOff > 0.0) {
      if(aoPtr->lpCutOff >= aoPtr->hpCutOff)
	fType = FILTER_BP;
      else
	fType = FILTER_BS;
    }
    else
      fType = FILTER_HP;
  }
  else if(aoPtr->lpCutOff > 0.0) {
    fType = FILTER_LP;
  }
  else {
    setAsspMsg(AEG_ERR_APPL, "Neither high- nor low-pass defined");
    return(-1);
  }
  if(suffix != NULL) {
    switch(fType) {
    case FILTER_HP:
      strcpy(suffix, ".hpf");
      break;
    case FILTER_LP:
      strcpy(suffix, ".lpf");
      break;
    case FILTER_BP:
      strcpy(suffix, ".bpf");
      break;
    case FILTER_BS:
      strcpy(suffix, ".bsf");
      break;
    }
  }
  return(fType);
}

/*DOC

This function allocates memory for a data object to hold audio filter 
parameters in 'generic' and a workspace in 'dataBuffer'. It will compute 
the filter coefficients on the basis of the information in the audio 
object pointed to by "smpDOp" and the options in the structure pointed 
to by "aoPtr". Neither of these pointers may be NULL.
"smpDOp" must point to a valid audio object i.e., either one referring 
to a non-empty file opened for reading or one in which all samples to be 
filtered are contained in its data buffer.
It returns a pointer to the data object or NULL upon error.

Note:
 - This is a slight abuse of the data object structure because it only 
   indirectly refers to a time series and doesn't need file I/O.
 - Use the function 'destroyFilter' to return all memory allocated for 
   this object.

DOC*/

DOBJ *createFilter(DOBJ *smpDOp, AOPTS *aoPtr)
{
  int      err;
  long     auCaps, numRecords, L, workSamples;
  DOBJ    *dop=NULL;
  DDESC   *dd=NULL;
  FILT_GD *gd=NULL;
  FILTER  *fip=NULL;
  FIR     *fir=NULL;
  IIR2    *iir2=NULL;

  if(smpDOp == NULL || aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "createFilter");
    return(NULL);
  }
  clrAsspMsg();
  /* verify audio object */
  if((auCaps=getSmpCaps(FILT_PFORMAT)) <= 0)
    return(NULL);
  auCaps |= FILT_I_CHANS;
  if(aoPtr->channel < 1)
    aoPtr->channel = FILT_DEF_CHANNEL;
  if(checkSound(smpDOp, auCaps, aoPtr->channel) <= 0)
    return(NULL);
  if(smpDOp->fp != NULL) { /* file object */
    numRecords = smpDOp->numRecords;
    if(numRecords <= 0) {
      setAsspMsg(AEF_EMPTY, smpDOp->filePath);
      return(NULL);
    }
  }
  else { /* memory object */
    numRecords = smpDOp->bufNumRecs;
    if(smpDOp->dataBuffer == NULL || numRecords <= 0) {
      setAsspMsg(AED_NO_DATA, "(createFilter)");
      return(NULL);
    }
  }
  /* allocate memory for generic data and filter structures */
  /* copy parameters and check */
  if(aoPtr->options & FILT_OPT_AUTOGAIN) {
    if(aoPtr->gain < AF_MIN_GAIN || aoPtr->gain > AF_MAX_GAIN) {
      setAsspMsg(AEB_ERR_GAIN, "(createFilter)");
      return(NULL);
    }
  }
  gd = (FILT_GD *)calloc(1, sizeof(FILT_GD)); /* alloc and clear */
  fip = (FILTER *)calloc(1, sizeof(FILTER));  /* "" */
  if(gd == NULL || fip == NULL) {
    setAsspMsg(AEG_ERR_MEM, "(createFilter)");
    if(gd != NULL)
      free((void *)gd);
    if(fip != NULL)
      free((void *)fip);
    return(NULL);
  }
  strcpy(gd->ident, FILT_GD_IDENT);
  gd->options = aoPtr->options;
  gd->channel = aoPtr->channel;
  gd->gain = aoPtr->gain;
  gd->fPtr = fip;
  fip->sampFreq = smpDOp->sampFreq;
  fip->hpCutOff = aoPtr->hpCutOff;
  fip->lpCutOff = aoPtr->lpCutOff;
  fip->centreFreq = -1.0; /* just make sure it's off */
  fir = &(fip->data.fir);
  iir2 = &(fip->data.iir2);
  if(gd->options & FILT_OPT_USE_IIR) {
    fip->type = FILTER_IIR2;
    iir2->numSections = (size_t)(aoPtr->order);
  }
  else {
    fip->type = FILTER_FIR;
    fip->stopDB = aoPtr->stopDB;
    fip->tbWidth = aoPtr->tbWidth;
    fip->winFunc = wfType(aoPtr->winFunc); /* PRELIMINARY */
  }
  if(setFilterType(fip) <= 0) {
    freeFILT_GD((void *)gd);
    return(NULL);
  }
  if(checkFilter(fip) < 0) {
    freeFILT_GD((void *)gd);
    return(NULL);
  }
  /* allocate memory for the filter coefficients and compute them */
  if((fip->type & FILT_MASK_S) == FILTER_FIR)
    err = designFIR(fip);
  else
    err = designIIR2(fip);
  if(err < 0) {
    freeFILT_GD((void *)gd);
    return(NULL);
  }
  /* allocate memory for the filter object and fill it out */
  /* Note: we only set items necessary to address the workspace */
  if((dop=allocDObj()) == NULL) {
    freeFILT_GD((void *)gd);
    return(NULL);
  }
  dop->sampFreq = smpDOp->sampFreq;
  dop->frameDur = 1;
  dop->generic = (void *)gd;
  dop->doFreeGeneric = (DOfreeFunc)freeFILT_GD;
  dd = &(dop->ddl); /* set pointer to data descriptor */
  dd->type = DT_FILTER;
  dd->coding = DC_LIN;
  dd->format = FILT_PFORMAT;
  dd->numFields = FILT_O_CHANS;
  setRecordSize(dop);
  L = 0; /* IIR2 filter needs no past if we filter complete signal */
  workSamples = ANA_BUF_BYTES / dop->recordSize;
  if((fip->type & FILT_MASK_S) == FILTER_FIR) {
    L = (long)(fir->numCoeffs);
    if(workSamples < 2 * L)
      workSamples = 2 * L; /* avoid frequent reloading */
    L--; /* omit centre sample in count */
  }
  if(workSamples > (numRecords + L))
    workSamples = numRecords + L;
  if(allocDataBuf(dop, workSamples) == NULL) {
    freeDObj(dop);
    return(NULL);
  }
  return(dop);
}

/*DOC

This function filters the audio signal referred to by "inpDOp" using 
the filter specified in the generic data structure of the data object 
pointed to by "filtDOp". The latter MUST have been obtained by a call 
to 'createFilter' with the same input signal. The filtered signal will 
be returned in the data buffer of the object pointed to by "outDOp" or 
written to file if that object refers to a file opened for writing.
If "outDOp" is a NULL-pointer the output object will be created and its 
format will be set to equal that of the input.
This function will verify and - if necessary - (re-)allocate the data 
buffers in the data objects pointed to by "inpDOp" and "outDOp" to have 
appropriate size.
This function returns a pointer to the output data object or NULL upon 
error.

Note:
 - This function may be used in a file-to-file, file-to-memory, memory-
   to-file and memory-to-memory mode.
 - In memory-to-memory mode "inpDOp" and "outDOp" may point to the same
   data object provided it is a single-channel one.
 - If "outDOp" is not a NULL-pointer, the contents of its data buffer
   will be destroyed.
 - If this function fails and "outDOp" refers to a file, its header
   and/or data will in all probability be invalid.

DOC*/

DOBJ *filterSignal(DOBJ *inpDOp, DOBJ *filtDOp, DOBJ *outDOp)
{
  int      FILE_IN, FILE_OUT, ODO_CREATED;
  int      USING_FIR, NORMALIZED, RESCALE, DITHER;
  size_t   fn, sn, nr;
  long     auCaps, head, tail, bn, n, N;
  long     smpNr, begSmpNr, endSmpNr, totSamples;
  uint32_t seed;
  double  *smpPtr, sample, maxInpMag, maxOutMag, maxMag;
  double   overflowFac, scaleFac, ditherMag;
  FILT_GD *gd;
  FILTER  *fip;
  FIR     *fir=NULL;
  IIR2    *iir2=NULL;
  DDESC   *idd, *odd;

  if(inpDOp == NULL || filtDOp == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "filterSignal");
    return(NULL);
  }
  if(filtDOp->ddl.type != DT_FILTER || 
     filtDOp->ddl.format != FILT_PFORMAT ||
     filtDOp->generic == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "filterSignal");
    return(NULL);
  }
  gd = (FILT_GD *)(filtDOp->generic);
  if(strcmp(gd->ident, FILT_GD_IDENT) != 0) {
    setAsspMsg(AEB_BAD_ARGS, "filterSignal");
    return(NULL);
  }
  if(gd->channel < 1)
    gd->channel = 1;
  fip = gd->fPtr;
  if((fip->type & FILT_MASK_S) == FILTER_FIR) {
    USING_FIR = TRUE;
    fir = &(fip->data.fir);
  }
  else {
    USING_FIR = FALSE;
    iir2 = &(fip->data.iir2);
  }
  FILE_IN = FILE_OUT = ODO_CREATED = FALSE;
  /* check input object (length checked in createFilter() */
  if(inpDOp->fp != NULL) {
    totSamples = inpDOp->numRecords; /* always filter full data range */
    begSmpNr = inpDOp->startRecord;
    FILE_IN = TRUE;
  }
  else {
    totSamples = inpDOp->bufNumRecs;
    begSmpNr = inpDOp->bufStartRec;
  }
  /* check status of output object */
  if(outDOp == NULL) {
    if((outDOp=allocDObj()) == NULL)
      return(NULL);
    if(copyDObj(outDOp, inpDOp) < 0) {
      freeDObj(outDOp);
      return(NULL);
    }
    if(outDOp->ddl.numFields > FILT_O_CHANS) {
      outDOp->ddl.numFields = FILT_O_CHANS;
      setRecordSize(outDOp);                 /* needs be recalculated */
    }
    ODO_CREATED = TRUE;
  }
  else {
    if((auCaps=getSmpCaps(filtDOp->ddl.format)) <= 0)
      return(NULL);
    auCaps |= FILT_O_CHANS;
    if(checkSound(outDOp, auCaps, 0) <= 0)
      return(NULL);
    if(checkSound(outDOp, auCapsFF(outDOp->fileFormat), 0) <= 0)
      return(NULL); /* early warning */
    if(outDOp->fp != NULL)
      FILE_OUT = TRUE;
  }
  
  if(setGlobals(totSamples) < 0) {
    if(ODO_CREATED)
      freeDObj(outDOp);
    return(NULL);
  }
  /* check data buffers of in- and output objects */
  if(FILE_IN) {
    n = filtDOp->maxBufRecs; /* optimal size */
    if(n > totSamples)
      n = totSamples;
    if(inpDOp->dataBuffer == NULL || inpDOp->maxBufRecs < n) {
      if(inpDOp->dataBuffer != NULL) {
        if(inpDOp->doFreeDataBuf == NULL) { /* apparently fixed */
	  freeBufs();
	  if(ODO_CREATED)
	    freeDObj(outDOp);
          setAsspMsg(AEB_BUF_SPACE, "(filterSignal: input buffer)");
          return(NULL);
        }
        freeDataBuf(inpDOp);
      }
      if(allocDataBuf(inpDOp, n) == NULL) {
	freeBufs();
	if(ODO_CREATED)
	  freeDObj(outDOp);
	return(NULL);
      }
    } /* else oversize will also do */
  } /* else */ /* per definition optimal */
  if(FILE_OUT) {
    if(outDOp->dataBuffer == NULL || outDOp->maxBufRecs < 1) {
      if(outDOp->dataBuffer != NULL) {
        if(outDOp->doFreeDataBuf == NULL) { /* apparently fixed */
	  freeBufs();
          setAsspMsg(AEB_BUF_SPACE, "(filterSignal: output buffer)");
          return(NULL);
        }
        freeDataBuf(outDOp);
      }
      n = (long)samplesPerTempBlock;
      if(allocDataBuf(outDOp, n) == NULL) {
	freeBufs();
	return(NULL);
      }
    } /* else any size will do */
  }
  else {
    if(outDOp->dataBuffer == NULL || outDOp->maxBufRecs < totSamples) {
      if(outDOp->dataBuffer != NULL) {
        if(outDOp->doFreeDataBuf == NULL) { /* apparently fixed */
	  freeBufs();
	  if(ODO_CREATED)
	    freeDObj(outDOp);
          setAsspMsg(AEB_BUF_SPACE, "(filterSignal: output buffer)");
          return(NULL);
        }
        freeDataBuf(outDOp);
      }
      if(allocDataBuf(outDOp, totSamples) == NULL) {
	freeBufs();
	if(ODO_CREATED)
	  freeDObj(outDOp);
	return(NULL);
      }
    } /* else oversize will also do */
  }
  if(createTempFiles() < 0) {
    freeBufs();
    if(ODO_CREATED)
      freeDObj(outDOp);
    return(NULL);
  }
  if(USING_FIR) {
    head = (fir->numCoeffs) / 2;
    tail = fir->numCoeffs - head - 1; /* exclude centre sample */
  }
  else {
    head = tail = 0;
    clearTaps(fip);
  }

  if(TRACE['c']) {
    if((fip->type & FILT_MASK_S) == FILTER_FIR) {
      N = (long)(fir->numCoeffs);
      fprintf(traceFP, "FIR filter length: %ld\n", N);
      N = (N+1) / 2; /* print only half because symmetric */
      for(n = 0; n < N; n++)
	fprintf(traceFP, "%+.15e\n", fir->c[n]);
      fprintf(traceFP, "\n");
    }
    else {
      N = (long)(iir2->numSections);
      fprintf(traceFP, "Number of IIR2 sections: %ld\n", N);
      fprintf(traceFP, "a0");
      for(n = 0; n < N; n++)
	fprintf(traceFP, " %+.15e", iir2->a0[n]);
      fprintf(traceFP, "\n");
      fprintf(traceFP, "a1");
      for(n = 0; n < N; n++)
	  fprintf(traceFP, " %+.15e", iir2->a1[n]);
      fprintf(traceFP, "\n");
      fprintf(traceFP, "a2");
      for(n = 0; n < N; n++)
	fprintf(traceFP, " %+.15e", iir2->a2[n]);
      fprintf(traceFP, "\n");
      fprintf(traceFP, "b1");
      for(n = 0; n < N; n++)
	fprintf(traceFP, " %+.15e", iir2->b1[n]);
      fprintf(traceFP, "\n");
      fprintf(traceFP, "b2");
      for(n = 0; n < N; n++)
	fprintf(traceFP, " %+.15e", iir2->b2[n]);
      fprintf(traceFP, "\n\n");
    }
  }

  /*
   * pass 1: read samples and convert to double; filter and store in
   * temporary file(s)
   */
  maxInpMag = maxOutMag = 0.0;       /* keep track of both magnitudes */
  smpNr = begSmpNr;
  endSmpNr = smpNr + totSamples;
  filtDOp->bufStartRec = begSmpNr - head;
  filtDOp->bufNumRecs = 0; /* no valid data in buffer */
  while(smpNr < endSmpNr) {
    for(fn = 0; fn < numTempFiles; fn++) {
      for(bn = 0; bn < blocksPerTempFile; bn++) {
	for(sn = 0; sn < samplesPerTempBlock; NIX) { /* increment below */
	  /* could avoid some overhead; see ksv.c */
	  smpPtr = (double *)getSmpPtr(inpDOp, smpNr, head, tail,\
				       gd->channel, filtDOp);
	  if(smpPtr == NULL) {
	    removeTempFiles();
	    freeBufs();
	    if(ODO_CREATED)
	      freeDObj(outDOp);
	    return(NULL);
	  }
	  if(fabs(*smpPtr) > maxInpMag)
	    maxInpMag = fabs(*smpPtr);
	  if(USING_FIR)
	    sample = FIRfilter(fip, smpPtr - head);
	  else
	    sample = IIR2filter(fip, *smpPtr);
	  if(fabs(sample) > maxOutMag)
	    maxOutMag = fabs(sample);
	  blockBuffer[sn++] = sample;   /* sn++ here for fwrite below */
	  smpNr++;           /* increment here because of local check */
	  if(smpNr >= endSmpNr)         /* block may be partly filled */
	    break;
	}
	if(fwrite((void *)blockBuffer, sizeof(double), sn, tempFP[fn]) != sn) {
	  removeTempFiles();
	  freeBufs();
	  if(ODO_CREATED)
	    freeDObj(outDOp);
	  setAsspMsg(AEF_ERR_WRIT, "(filterSignal: temporary file)");
	  return(NULL);
	}
	if(smpNr >= endSmpNr)      /* last block may be partly filled */
	  break;
      }
      if(smpNr >= endSmpNr)    /* last temp file may be partly filled */
	break;
    }
  }
  /* set maximum magnitude of output signal for detecting overflow */
  /* and determining scaling factors */
  idd = &(inpDOp->ddl);
  odd = &(outDOp->ddl);
  NORMALIZED = ((idd->format == DF_REAL32 ||
		 idd->format == DF_REAL64) && maxInpMag <= 1.0);
  switch(odd->format) {
  case DF_INT16:
    odd->numBits = 16; /* may have more valid bits than before */
    maxMag = INT16_MAX;
    break;
  case DF_INT24:
    odd->numBits = 24;
    maxMag = INT24_MAX;
    break;
  case DF_INT32:
    /* both AIF and WAV specify left alignment if fewer than 32 bits */
    odd->numBits = 32;
    maxMag = INT32_MAX;
    break;
  case DF_REAL32:
    odd->numBits = 32;
    if((idd->format == DF_REAL32 ||
	idd->format == DF_REAL64) && !NORMALIZED)
      /* have to set some value; this one is based on the fact that */
      /* 32-bit float has 25 bit integral precision: 23 bit + 1 */
      /* (hidden) bit in mantissa + sign bit */
      maxMag = INT24_MAX;
    else
      maxMag = 1.0;       /* both AIFC and WAVE specify normalization */
    break;
  case DF_REAL64:
    odd->numBits = 64;
    if((idd->format == DF_REAL32 ||
	idd->format == DF_REAL64) && !NORMALIZED)
      /* double has 52 bit mantissa but it will be some */
      /* time until we have true 24-bit converters */
      maxMag = INT24_MAX;
    else
      maxMag = 1.0;
    break;
  default: /* can't happen, but ... */
    maxMag = INT24_MAX; /* to appease the compiler */
    break;
  }
  overflowFac = AF_MAX_GAIN / 100.0;       /* leave a little headroom */
  if((RESCALE=(gd->options & FILT_OPT_AUTOGAIN)) )
    maxMag *= ((gd->gain) / 100.0); /* gain specified in % full scale */
  else /* set overflow threshold */
    maxMag *= overflowFac;
  /* determine scaling factor */
  scaleFac = 1.0;
  switch(odd->format) {
  case DF_INT16:
  case DF_INT24:
  case DF_INT32:
    switch(idd->format) {
    case DF_INT16:
    case DF_INT24:
    case DF_INT32:
      if(maxOutMag > maxMag) /* overflow */
	RESCALE = TRUE;
      break;
    case DF_REAL32:
    case DF_REAL64:
      if(NORMALIZED) {
	if(maxOutMag > overflowFac)
	  RESCALE = TRUE;
	else if(!RESCALE) {
	  /* BUGGER! have to upscale but hate to lose precious bits */
	  /* well, neither the place nor the time to worry about it */
	  maxOutMag = 1.0;
	  RESCALE = TRUE;
	}
      }
      else if(maxOutMag > maxMag) /* overflow */
	RESCALE = TRUE;
      break;
    default: /* can't happen, but ... */
      break;
    }
    break;
  case DF_REAL32:
  case DF_REAL64:
    switch(idd->format) {
    case DF_INT16:
      if(maxOutMag > overflowFac * INT16_MAX)
	RESCALE = TRUE;
      else if(!RESCALE) { /* always normalize */
	maxOutMag = INT16_MAX;
	RESCALE = TRUE;
      }
      break;
    case DF_INT24:
      if(maxOutMag > overflowFac * INT24_MAX)
	RESCALE = TRUE;
      else if(!RESCALE) {
	maxOutMag = INT24_MAX;
	RESCALE = TRUE;
      }
      break;
    case DF_INT32:
      if(maxOutMag > overflowFac * INT32_MAX)
	RESCALE = TRUE;
      else if(!RESCALE) {
	maxOutMag = INT32_MAX;
	RESCALE = TRUE;
      }
      break;
    case DF_REAL32: /* no worries: either both normalized or both not */
    case DF_REAL64:
      if(maxOutMag > maxMag) /* overflow */
	RESCALE = TRUE;
      break;
    default: /* can't happen, but ... */
      break;
    }
    break;
  default: /* can't happen, but ... */
    break;
  }
  if(RESCALE)
    scaleFac = maxMag / maxOutMag;

  if(TRACE['m']) {
    fprintf(traceFP, "magnitude: input: %f  output: %f\n",\
	    maxInpMag, maxOutMag);
    fprintf(traceFP, "threshold: %f  scale factor %f\n",\
	    maxMag, scaleFac);
  }

  /* to dither or not to dither */
  DITHER = FALSE;
  ditherMag = 0.0;
  seed = 0;
  if(!(gd->options & FILT_NOPT_DITHER) &&
     odd->format == DF_INT16 && RESCALE && scaleFac <= 1.0) {
    DITHER = TRUE;
    ditherMag = DITHER_MAG;
    seed = START_SEED;
  }
  /*
   * check/adjust timing of output; (re-)write header if to-file
   */
  if(begSmpNr != 0 && outDOp->fileFormat != FF_SSFF)
    begSmpNr = 0;
  outDOp->startRecord = begSmpNr;
  setStart_Time(outDOp);
  outDOp->numRecords = totSamples; /* prepare for header */
  outDOp->bufStartRec = begSmpNr; /* prepare for mapping */
  outDOp->bufNumRecs = 0; /* no valid data in buffer */
  if(FILE_OUT) { /* ensure header is present/correct */
    if(putHeader(outDOp) < 0) {
      removeTempFiles();
      freeBufs();
      if(ODO_CREATED)
	freeDObj(outDOp);
      return(NULL);
    }
  }
  clrAsspMsg(); /* we may set a warning */
  if(RESCALE && !(gd->options & FILT_OPT_AUTOGAIN)) {
    asspMsgNum = AWG_WARN_APPL;
    if(FILE_OUT)
      sprintf(applMessage, "amplitude reduced by %.4f dB to avoid "\
	      "numerical overflow\n         in file %s",\
	      -LINtodB(scaleFac), myfilename(outDOp->filePath));
    else
      sprintf(applMessage, "amplitude reduced by %.4f dB to avoid "\
	      "numerical overflow", -LINtodB(scaleFac));
  }
  /*
   * pass 2: read sample blocks from temporary file(s), rescale and 
   * dither if required and store in output object
   */
  smpNr = begSmpNr;
  endSmpNr = smpNr + totSamples;
  while(smpNr < endSmpNr) {
    for(fn = 0; fn < numTempFiles; fn++) {
      rewind(tempFP[fn]);
      for(bn = 0; bn < blocksPerTempFile; bn++) {
	nr = fread((void *)blockBuffer, sizeof(double),	\
		   samplesPerTempBlock, tempFP[fn]);
	if(ferror(tempFP[fn])) {
	  removeTempFiles();
	  freeBufs();
	  if(ODO_CREATED)
	    freeDObj(outDOp);
	  setAsspMsg(AEF_ERR_READ, "(filterSignal: temporary file)");
	  return(NULL);
	}
	if(RESCALE) {
	  for(sn = 0; sn < nr; sn++)
	    blockBuffer[sn] *= scaleFac;
	}
	if(DITHER) {
	  for(sn = 0; sn < nr; sn++)
	    blockBuffer[sn] += (ditherMag * randTPDF(&seed));
	}
	if(storeBlock(smpNr, nr, outDOp) < 0) {
	  removeTempFiles();
	  freeBufs();
	  if(ODO_CREATED)
	    freeDObj(outDOp);
	  return(NULL);
	}
	smpNr += nr;
	if(smpNr >= endSmpNr)
	  break;
      }
      fclose(tempFP[fn]); /* should also remove it */
      tempFP[fn] = NULL;
      if(smpNr >= endSmpNr)
	break;
    }
  }
  freeBufs();
  if(FILE_OUT) {
    if(asspFFlush(outDOp, AFW_CLEAR) < 0)
      return(NULL);
  }
  return(outDOp);
}

/*DOC

This function frees all memory allocated for a FILT data object.
It always returns a NULL-pointer 

DOC*/

DOBJ *destroyFilter(DOBJ *filtDOp)
{
  if(filtDOp != NULL)
    filtDOp = freeDObj(filtDOp);
  return(filtDOp);
}

/*DOC

This function returns all memory allocated for the generic data in a 
FILT data object.

DOC*/

void freeFILT_GD(void *generic)
{
  FILT_GD *gd;

  if(generic != NULL) {
    gd = (FILT_GD *)generic;
    if(gd->fPtr != NULL) {
      freeFilter(gd->fPtr); /* doesn't free structure itself */
      free((void *)gd->fPtr); /* so do it separately */
    }
    free(generic);
  }
  return;
}

/* ======================= private  functions ======================= */

/***********************************************************************
* set local global variables and allocate memory for block buffer      *
***********************************************************************/
LOCAL int setGlobals(long totSamples)
{
  size_t bytePerTempBlock, fn;
  long   samplesPerTempFile;

  /* clear pointers for temporary files */
  for(fn = 0; fn < AF_MAX_TEMP; fn++)
    tempFP[fn] = NULL;

  samplesPerTempBlock = 512;
  bytePerTempBlock = samplesPerTempBlock * sizeof(double);
  blocksPerTempFile = INT32_MAX / bytePerTempBlock; /* take 32-bit OS */
  samplesPerTempFile = blocksPerTempFile * samplesPerTempBlock;
  numTempFiles = totSamples / samplesPerTempFile;
  if((totSamples % samplesPerTempFile) != 0)
    numTempFiles++;
  if(numTempFiles > AF_MAX_TEMP) {
    setAsspMsg(AEG_ERR_BUG, "filterSignal: need too may temp files");
    return(-1);
  }
  blockBuffer = (double *)calloc(samplesPerTempBlock, sizeof(double));
  if(blockBuffer == NULL) {
    setAsspMsg(AEG_ERR_MEM, NULL);
    return(-1);
  }
  return(0);
}

/***********************************************************************
* return memory allocated for local buffers                            *
***********************************************************************/
LOCAL void freeBufs(void)
{
  if(blockBuffer != NULL) {
    free((void *)blockBuffer);
    blockBuffer = NULL;
  }
  return;
}

/***********************************************************************
* create/open temporary files                                          *
***********************************************************************/
LOCAL int createTempFiles(void)
{
  size_t fn;

  for(fn = 0; fn < numTempFiles; fn++) {

    #ifdef OS_Windows
      char lpTempPathBuffer[MAX_PATH];
      char szTempFileName[MAX_PATH];
      GetTempPath(MAX_PATH,lpTempPathBuffer);
      GetTempFileName(lpTempPathBuffer, // directory for tmp files
                              TEXT("wrassp"),     // temp file name prefix 
                              0,                // create unique name 
                              szTempFileName);  // buffer for name 
      tempFP[fn] = fopen(szTempFileName, "w+b");
      //Rprintf("This is the tmp dir:%s\n", lpTempPathBuffer);
      //Rprintf("This is the tmp file:%s\n", szTempFileName);
    #else
      tempFP[fn] = tmpfile();
    #endif
    
    if(tempFP[fn] == NULL) {
      removeTempFiles();
      setAsspMsg(AEF_ERR_OPEN, "(createTempFiles)");
      return(-1);
    }
  }
  return(0);
}

/***********************************************************************
* remove temporary files                                               *
***********************************************************************/
LOCAL void removeTempFiles(void)
{
  size_t fn;

  for(fn = 0; fn < AF_MAX_TEMP; fn++) {
    if(tempFP[fn] != NULL) {
      fclose(tempFP[fn]);
      tempFP[fn] = NULL;
    }
  }
  return;
}

/***********************************************************************
* copy/convert sample range in global block buffer to output buffer;   *
* handle data writes                                                   *
***********************************************************************/
LOCAL int storeBlock(long begSn, register long num, DOBJ *dop)
{
  register long n;
  char    *srcPtr, *dstPtr;
  int      FILE_OUT;
  size_t   dstSize;
  long     offset, space;
  int16_t *i16Ptr;
  uint8_t *i24Ptr;
  int32_t *i32Ptr;
  float   *f32Ptr;
  DDESC   *dd;

  FILE_OUT = (dop->fp != NULL);
  dstSize = dop->recordSize;     /* only support single-channel audio */
  dd = &(dop->ddl);
  if(dop->bufNumRecs <= 0) {
    dop->bufNumRecs = 0;
    dop->bufStartRec = begSn;
  }
  else if(begSn >= (dop->bufStartRec + dop->maxBufRecs)) {
    if(FILE_OUT) {
      if(asspFFlush(dop, AFW_CLEAR) < 0)
	return(-1);
    }
    else {
      setAsspMsg(AEG_ERR_BUG, "storeBlock: buffer overflow");
      return(-1);
    }
  }
  if(FILE_OUT) {
    if((begSn + num) <= (dop->bufStartRec + dop->maxBufRecs))
      FILE_OUT = FALSE; /* range fits data buffer */
  }
  if(FILE_OUT)                     /* convert in-place before transfer */
    dstPtr = (void *)blockBuffer;
  else {                            /* convert and transfer in one go */
    offset = (begSn - dop->bufStartRec) * (long)dstSize;
    dstPtr = (char *)dop->dataBuffer + offset;
  }
  switch(dd->format) {
  case DF_INT16:
    i16Ptr = (int16_t *)dstPtr;
    for(n = 0; n < num; n++)
      *(i16Ptr++) = (int16_t)myrint(blockBuffer[n]);
    break;
  case DF_INT24:
    i24Ptr = (uint8_t *)dstPtr;
    for(n = 0; n < num; i24Ptr += dstSize, n++)
      int32_to_int24((int32_t)myrint(blockBuffer[n]), i24Ptr);
    break;
  case DF_INT32:
    i32Ptr = (int32_t *)dstPtr;
    for(n = 0; n < num; n++)
      *(i32Ptr++) = (int32_t)myrint(blockBuffer[n]);
    break;
  case DF_REAL32:
    f32Ptr = (float *)dstPtr;
    for(n = 0; n < num; n++)
      *(f32Ptr++) = (float)blockBuffer[n];
    break;
  case DF_REAL64:
    if(!FILE_OUT)                /* transfer directly to output buffer */
      memcpy(dstPtr, (void *)blockBuffer, (size_t)num * dstSize);
    /* else *//* no action because in-place */
    break;
  default: /* can't happen, but ... */
    break;
  }
  if(FILE_OUT) {
    srcPtr = (void *)blockBuffer;
    while(num > 0) {
      offset = (begSn - dop->startRecord);
      space = dop->maxBufRecs - offset;
      if(space > 0) {
	if(space > num)
	  space = num;
	dstPtr = (char *)dop->dataBuffer + offset * dstSize;
	memcpy(dstPtr, srcPtr, (size_t)space * dstSize);
	srcPtr += ((size_t)space * dstSize);
	begSn += space;
	num -= space;
	dop->bufNeedsSave = TRUE;
      }
      if(num > 0) {
	if(asspFFlush(dop, AFW_CLEAR) < 0)
	  return(-1);
      }
    }
  }
  else {
    if((begSn + num) > (dop->bufStartRec + dop->bufNumRecs))
      dop->bufNumRecs = begSn - dop->bufStartRec + num;
    dop->bufNeedsSave = TRUE;
  }
  return(0);
}
