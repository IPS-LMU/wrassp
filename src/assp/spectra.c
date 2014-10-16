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
* File:     spectra.c                                                  *
* Contents: Functions for calculating (smoothed) spectra.              *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: spectra.c,v 1.5 2012/03/19 16:16:38 lasselasse Exp $ */

#include <stdio.h>    /* FILE NULL etc. */
#include <stdlib.h>   /* malloc() calloc() free() */
#include <string.h>   /* str..() */
#include <math.h>     /* sqrt() log10() */

#include <miscdefs.h> /* TRUE FALSE LOCAL */
#include <misc.h>     /* strnxcmp() */
#include <trace.h>    /* trace handler */
#include <asspmess.h> /* error message handler */
#include <assptime.h> /* standard conversion macros */
#include <spectra.h>  /* processing parameters & SPECT functions */
#include <asspana.h>  /* AOPTS anaTiming() */
#include <asspdsp.h>  /* makeWF() freeWF() mulSigWF() [r]fft() etc. */
#include <asspfio.h>  /* asspFFlush() */
#include <dataobj.h>  /* DOBJ getSmpCaps() getSmpFrame() */
#include <headers.h>  /* KDTAB */
#include <aucheck.h>  /* checkSound() */

/*
 * table relating available spectrum types as string with data type
 * code and file name extension.
 */
SPECT_TYPE spectType[] = {
  {"DFT", DT_FTPOW, ".dft"},
  {"LPS", DT_FTLPS, ".lps"},
  {"CSS", DT_FTCSS, ".css"},
  {"CEP", DT_FTCEP, ".cep"},
  { NULL, DT_UNDEF, NULL}
};

/*
 * prototypes of private functions
 */
LOCAL int  allocBufs(SPECT_GD *gd, long frameShift);
LOCAL void freeBufs(SPECT_GD *gd);
LOCAL int  storeSPECT(long frameNr, DOBJ *dop);
LOCAL void lpInvLinAmp(double *c, double msqr, long N);
LOCAL void lpInvLinPow(double *c, double msqr, long N);
LOCAL void lpInvPower(double *c, double msqr, long N);
LOCAL void fftPower(double *c, long N);
LOCAL void fftlnMag(double *c, long N);

/* ======================== public functions ======================== */

/*DOC

Function 'getSPECTtype'

Determines whether the string pointed to by "str" corresponds to a 
valid spectrum type. If this is the case, returns the data type number 
code corresponding to that spectrum type and if "suffix" is not a NULL-
pointer, copies the corresponding file name suffix to that string.The 
string pointed to by "suffix" must be at least 5 byte long.
Returns 'DT_ERROR' for invalid strings.

DOC*/

dtype_e getSPECTtype(char *str, char *suffix)
{
  dtype_e spType;

  spType = DT_ERROR;
  if(str != NULL) {
    if(strnxcmp(str, "DFT", 2) == 0) {
      spType = DT_FTPOW;
      if(suffix != NULL)
	strcpy(suffix, ".dft");
    }
    else if(strnxcmp(str, "LPS", 2) == 0) {
      spType = DT_FTLPS;
      if(suffix != NULL)
	strcpy(suffix, ".lps");
    }
    else if(strnxcmp(str, "CSS", 2) == 0) {
      spType = DT_FTCSS;
      if(suffix != NULL)
	strcpy(suffix, ".css");
    }
    else if(strnxcmp(str, "CEP", 2) == 0) {
      spType = DT_FTCEP;
      if(suffix != NULL)
	strcpy(suffix, ".cep");
    }
    /* the following are not supported by 'spectrum' */
    else if(strnxcmp(str, "FTAMP", 4) == 0) {
      spType = DT_FTAMP;
      if(suffix != NULL)
	strcpy(suffix, ".fta");
    }
    else if(strnxcmp(str, "FTSQR", 4) == 0) {
      spType = DT_FTSQR;
      if(suffix != NULL)
	strcpy(suffix, ".fts");
    }
    else {
      setAsspMsg(AED_BAD_TYPE, "(getSPECTtype: \"");
      strcat(applMessage, str);
      strcat(applMessage, "\"");
    }
  }
  return(spType);
}

/*DOC

Function 'setSPECTdefaults'

Sets the items in the analysis options structure relevant to the spectral 
analysis of an audio signal to their default values. Clears all other 
items.
Returns 0 upon success and -1 upon error.

DOC*/

int setSPECTdefaults(AOPTS *aoPtr)
{
  dtype_e spType;

  if(aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setSPECTdefaults");
    return(-1);
  }
  memset((void *)aoPtr, 0, sizeof(AOPTS));         /* clear all items */
  aoPtr->msShift = SPECT_DEF_SHIFT;               /* frame shift (ms) */
  aoPtr->resolution = SPECT_DEF_RES;      /* spectral resolution (Hz) */
  aoPtr->channel = SPECT_DEF_CHANNEL;       /* channel to be analysed */
  aoPtr->accuracy = SPECT_DEF_DIGITSA;   /* digits accuracy for ASCII */
  aoPtr->precision = SPECT_DEF_DIGITSP; /* digits precision for ASCII */
  strcpy(aoPtr->format, SPECT_DEF_FORMAT);      /* output file format */
  strcpy(aoPtr->type, SPECT_DEF_TYPE);               /* spectrum type */
  strcpy(aoPtr->winFunc, SPECT_DEF_WINDOW);        /* window function */
  spType = getSPECTtype(aoPtr->type, NULL);
  switch(spType) {
  case DT_FTPOW:
  case DT_FTAMP:
  case DT_FTSQR:
    setDFTdefaults(aoPtr);
    break;
  case DT_FTLPS:
    setLPSdefaults(aoPtr);
    break;
  case DT_FTCSS:
    setCSSdefaults(aoPtr);
    break;
  case DT_FTCEP:
    setCEPdefaults(aoPtr);
    break;
  default:
    setAsspMsg(AEG_ERR_BUG, "setSPECTdefaults: invalid default type");
    return(-1);
  }
  return(0);
}

/*DOC

Function 'setDFTdefaults'

Sets the items in the analysis options structure specific for the 
analysis of plain power spectra to their default values.
Returns 0 upon success and -1 upon error.

Note:
 - This function may modify option flags.

DOC*/

int setDFTdefaults(AOPTS *aoPtr)
{
  if(aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setDFTdefaults");
    return(-1);
  }
  aoPtr->msSize = DFT_DEF_SIZE;
  aoPtr->bandwidth = DFT_DEF_BANDWIDTH;
  aoPtr->preEmph = DFT_DEF_PREEMPH;
  aoPtr->options &= ~(SPECT_OPT_LIN_AMP | SPECT_OPT_LIN_POW | LPS_OPT_DEEMPH);
  aoPtr->options |= AOPT_USE_ENBW;
  return(0);
}

/*DOC

Function 'setLPSdefaults'

Sets the items in the analysis options structure specific for the 
analysis of LP smoothed power spectra to their default values.
Returns 0 upon success and -1 upon error.

Note:
 - This function may modify option flags.

DOC*/

int setLPSdefaults(AOPTS *aoPtr)
{
  if(aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setLPSdefaults");
    return(-1);
  }
  aoPtr->msSize = LPS_DEF_SIZE;
  aoPtr->order = LPS_DEF_ORDER;
  aoPtr->preEmph = LPS_DEF_PREEMPH;
  aoPtr->options &= ~(SPECT_OPT_LIN_AMP | SPECT_OPT_LIN_POW | AOPT_USE_ENBW);
  aoPtr->options |= (AOPT_EFFECTIVE | LPS_OPT_DEEMPH);
  return(0);
}

/*DOC

Function 'setCSSdefaults'

Sets the items in the analysis options structure specific for the 
analysis of cepstral smoothed power spectra to their default values.
Returns 0 upon success and -1 upon error.

Note:
 - This function may modify option flags.

DOC*/

int setCSSdefaults(AOPTS *aoPtr)
{
  if(aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setCSSdefaults");
    return(-1);
  }
  aoPtr->msSize = CSS_DEF_SIZE;
  aoPtr->order = CSS_DEF_ORDER;
  aoPtr->preEmph = CSS_DEF_PREEMPH;
  aoPtr->options &= ~(SPECT_OPT_LIN_AMP | SPECT_OPT_LIN_POW | LPS_OPT_DEEMPH);
  aoPtr->options |= AOPT_USE_ENBW;
  return(0);
}

/*DOC

Function 'setCEPdefaults'

Sets the items in the analysis options structure specific for the 
analysis of real cepstra to their default values.
Returns 0 upon success and -1 upon error.

Note:
 - This function may modify option flags.

DOC*/

int setCEPdefaults(AOPTS *aoPtr)
{
  if(aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setCEPdefaults");
    return(-1);
  }
  aoPtr->msSize = CEP_DEF_SIZE;
  aoPtr->preEmph = CEP_DEF_PREEMPH;
  aoPtr->options &= ~(SPECT_OPT_LIN_POW| LPS_OPT_DEEMPH);
  aoPtr->options |= (SPECT_OPT_LIN_AMP | AOPT_USE_ENBW);
  return(0);
}

/*DOC

Function 'createSPECT'

Allocates memory for a data object to hold spectral/cepstral coefficients 
and initializes it on the basis of the information in the audio object 
pointed to by "smpDOp" and the analysis options in the structure pointed 
to by "aoPtr". Neither of these pointers may be NULL.
Returns a pointer to the data object or NULL upon error.

Note:
 - This function DOES NOT allocate memory for the data buffer. Depending 
   on your application you can either use 'allocDataBuf' for this or 
   delegate it to 'computeSPECT'.

DOC*/

DOBJ *createSPECT(DOBJ *smpDOp, AOPTS *aoPtr)
{
  long      auCaps, N, numFFT;
  double    fac;
  dtype_e   spType;
  ATIME     aTime, *tPtr;
  SPECT_GD *gd=NULL;
  DOBJ     *dop=NULL;
  DDESC    *dd=NULL;
  KDTAB    *entry;

  if(smpDOp == NULL || aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "createSPECT");
    return(NULL);
  }
  spType = getSPECTtype(aoPtr->type, NULL);
  if(spType == DT_ERROR)
    return(NULL);
  /* verify audio object */
  if((auCaps=getSmpCaps(SPECT_PFORMAT)) <= 0)
    return(NULL);
  auCaps |= SPECT_I_CHANS;
  if(aoPtr->channel < 1)
    aoPtr->channel = SPECT_DEF_CHANNEL;
  if(checkSound(smpDOp, auCaps, aoPtr->channel) <= 0)
    return(NULL);
  /* set FFT length first because might be used in anaTiming() */
  if(aoPtr->FFTLen <= 0) {
    if(aoPtr->resolution < SPECT_MIN_RES) {
      setAsspMsg(AEG_ERR_APPL, "createSPECT: resolution too small");
      return(NULL);
    }
    numFFT = (long)ceil(smpDOp->sampFreq / aoPtr->resolution);
  }
  else 
    numFFT = aoPtr->FFTLen;
  N = MIN_NFFT;
  while(N < numFFT) N *= 2;            /* round upwards to power of 2 */
  numFFT = N;
  N = aoPtr->FFTLen;                           /* save original value */
  aoPtr->FFTLen = numFFT;
  tPtr = &aTime;
  if(anaTiming(smpDOp, aoPtr, tPtr) < 0)
    return(NULL);
  if(N <= 0)
    aoPtr->FFTLen = 0;                     /* else keep rounded value */
  clrAsspMsg();                               /* ignore warnings here */
  if(spType != DT_FTLPS && tPtr->frameSize > numFFT) {
    setAsspMsg(AEG_ERR_APPL, "createSPECT: frame size exceeds FFT length");
    return(NULL);
  }
  if((gd=(SPECT_GD *)malloc(sizeof(SPECT_GD))) == NULL) {
    setAsspMsg(AEG_ERR_MEM, "(createSPECT)");
    return(NULL);
  }
  strcpy(gd->ident, SPECT_GD_IDENT);
  gd->options = aoPtr->options;
  gd->frameSize = tPtr->frameSize;
  gd->begFrameNr = tPtr->begFrameNr;
  gd->endFrameNr = tPtr->endFrameNr;
  gd->numFFT = numFFT;
  gd->spType = spType;
  if(spType == DT_FTLPS || spType == DT_FTCSS) {
    if(aoPtr->order < 1) {
      if(spType == DT_FTLPS)
	gd->order = DFLT_ORDER(tPtr->sampFreq);
      else
	gd->order = CSS_DFLT_LAGS(tPtr->sampFreq);
    }
    else
      gd->order = aoPtr->order;
    if(spType == DT_FTLPS && (2 * (gd->order + 2)) > gd->frameSize) {
      free((void *)gd);
      setAsspMsg(AED_ERR_SIZE, "(createSPECT: LPS)");
      return(NULL);
    }
    if(spType == DT_FTCSS && (2 * gd->order) >= gd->numFFT) {
      free((void *)gd);
      setAsspMsg(AEG_ERR_APPL, "createSPECT: number of CSS lags too high");
      return(NULL);
    }
  }
  gd->winFunc = wfType(aoPtr->winFunc);
  if(gd->winFunc <= WF_NONE) {
    freeSPECT_GD((void *)gd);
    setAsspMsg(AEB_BAD_WIN, aoPtr->winFunc);
    return(NULL);
  }
  if(spType == DT_FTCSS || spType == DT_FTCEP)
    gd->preEmph = 0.0;
  else {
    if(aoPtr->preEmph < -1.0 || aoPtr->preEmph > 1.0) {
      setAsspMsg(AEB_ERR_EMPH, "(createSPECT)");
      return(NULL);
    }
    gd->preEmph = aoPtr->preEmph;
  }
  gd->binWidth = tPtr->sampFreq / (double)numFFT;
  gd->channel = aoPtr->channel;
  gd->frame = NULL;
  gd->fftBuf = NULL;
  gd->wfc = NULL;
  gd->acf = NULL;
  /* determine correction factor for computed spectra so as to get    */ 
  /* the 'true' levels independent of window function, window size    */
  /* and number of FFT points                                         */
  /* NEW 2010: correction factor multiplicative (additive if in db)   */
  /*           rather than divisive/subtractive                       */
  /* ToDo: for FTAMP, FTSQR and FTPOW could save final multiplication */
  /*       loop by 'correcting' the window coefficients; would require*/
  /*       use of a 'true' rectangular window, though                 */
  fac = wfSpecs(gd->winFunc)->gain;        /* coherent gain of window */
  switch(spType) {
  case DT_FTAMP:
    fac *= ((double)gd->frameSize / (double)gd->numFFT);
    gd->corrFac = 1.0 / fac;
    break;
  case DT_FTSQR:
    fac *= ((double)gd->frameSize / (double)gd->numFFT);
    gd->corrFac = 1.0 / (fac * fac);
    break;
  case DT_FTPOW:
    fac *= ((double)gd->frameSize / (double)gd->numFFT);
    gd->corrFac = -LINtodB(fac);
    break;
  case DT_FTLPS:                  /* undo length correction of rfft() */
    fac = (double)gd->numFFT / fac;             /* for inverse filter */
    /* NEW 2010: always correct mean squared error signal and leave   */
    /* the conversion to amplitude/dB to the functions 'lpInv...'     */
    gd->corrFac = 1.0 / (fac * fac);
    break;
  case DT_FTCSS:
    fac *= (0.5 * (double)gd->frameSize);                  /* MAGIC ! */
    gd->corrFac = -LINtodB(fac);
    /* NEW 2010: always compute/correct the dB spectrum and leave the */
    /* conversion  to amplitude/power to the function 'getCSSpectrum' */
    break;
  case DT_FTCEP:
    gd->corrFac = 1.0 / fac;                                   /* ??? */
    break;
  default:
    gd->corrFac = 1.0;
  }
  gd->gain = aoPtr->gain;
  gd->range = aoPtr->range;
  gd->maxF = aoPtr->maxF;
  gd->minF = aoPtr->minF;
  gd->numLevels = aoPtr->numLevels;
  gd->accuracy = aoPtr->accuracy;
  gd->precision = aoPtr->precision;

  if((dop=allocDObj()) == NULL) {
    freeSPECT_GD((void *)gd);
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
  dop->doFreeGeneric = (DOfreeFunc)freeSPECT_GD;
  dd = &(dop->ddl);                 /* set pointer to data descriptor */
  dd->type = spType;
  dd->coding = DC_LIN;
  if(gd->options & SPECT_OPT_DOUBLE)
    dd->format = DF_REAL64;
  else
    dd->format = SPECT_DFORMAT;
  dd->numFields = gd->numFFT / 2 + 1;
  dd->numBits = smpDOp->ddl.numBits;              /* for auto-scaling */
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
      setAsspMsg(AEB_ERR_TRACK, "(createSPECT)");
      return(dop);
    }
  }
  else { /* fall through to raw ASCII */
    strcpy(dop->sepChars, "\t");                    /* between fields */
    strcpy(dd->sepChars, " ");                        /* within field */
    if(gd->options & SPECT_OPT_LIN_AMP || gd->options & SPECT_OPT_LIN_POW) {
      if(dd->format == DF_REAL64)
	strcpy(dd->ascFormat, "%+.14e");
      else
	sprintf(dd->ascFormat, "%%+.%de", gd->accuracy);
    }
    else
      sprintf(dd->ascFormat, "%%.%df", gd->precision);
  }
  setRecordSize(dop);
  setStart_Time(dop);
  return(dop);
}

/*DOC

Function 'computeSPECT'

Performs a spectral/cepstral analysis of the audio signal referred to 
by "smpDOp" using the parameter settings specified in the generic data 
structure of the output data object pointed to by "spectDOp".
If "spectDOp" is a NULL-pointer "aoPtr" may not be a NULL-pointer and 
this function will create the output data object (see 'createSPECT' for
details).
If "aoPtr" is not a NULL-pointer this function will verify and - if 
necessary - (re-)allocate the data buffers in the data objects pointed 
to by "smpDOp" and "spectDOp" to have appropriate size. For on-the-fly, 
single-frame analyses it is advised to use an initial call with the 
option flag 'AOPT_INIT_ONLY' set in order to set up the data buffers. 
You may then suppress verification overhead in subsequent calls by 
passing NULL for "aoPtr". If the flag 'AOPT_INIT_ONLY' has been set it 
will be cleared after initialisation and verification and no analysis 
will be performed. 
Analysis results will be returned in the data buffer of the object 
pointed to by "spectDOp" or written to file if that object refers to a 
file opened for writing. 
Returns a pointer to the output data object or NULL upon error.

Note:
 - This function may be used in a file-to-file, file-to-memory, memory-
   to-file and memory-to-memory mode.
 - This function DOES NOT verify whether the analysis parameters in the 
   generic data structure of "spectDOp" comply with those in the general 
   analysis structure pointed to by "aoPtr". If parameter changes occur 
   between calls, the output object should best be destroyed and then 
   recreated.

DOC*/

DOBJ *computeSPECT(DOBJ *smpDOp, AOPTS *aoPtr, DOBJ *spectDOp)
{
  char *bPtr;
  int   FILE_IN, FILE_OUT, CREATED;
  int   err;
  long  fn, frameSize, frameShift, head;
  SPECT_GD *gd;

  if(smpDOp == NULL || (aoPtr == NULL && spectDOp == NULL)) {
    setAsspMsg(AEB_BAD_ARGS, "computeSPECT");
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
      setAsspMsg(AED_NO_DATA, "(computeSPECT)");
      return(NULL);
    }
  }
  /* check status of output object */
  if(spectDOp == NULL) {
    if((spectDOp=createSPECT(smpDOp, aoPtr)) == NULL)
      return(NULL);
    CREATED = TRUE;
  }
  gd = (SPECT_GD *)spectDOp->generic;
  if(spectDOp->fp != NULL) {
    FILE_OUT = TRUE;
    gd->writeOpts = AFW_CLEAR;        /* discard data after writing */
    if(spectDOp->fileData == FDF_ASC)
      gd->writeOpts |= AFW_ADD_TIME;
  }
  else
    gd->writeOpts = AFW_KEEP;

  frameSize = gd->frameSize;
  frameShift = spectDOp->frameDur;
  if(aoPtr != NULL) {
    if(checkDataBufs(smpDOp, spectDOp, frameSize,\
		     gd->begFrameNr, gd->endFrameNr) < 0) {
      if(CREATED)
	freeDObj(spectDOp);
      return(NULL);
    }
    if(allocBufs(gd, frameShift) < 0) {
      if(CREATED)
	freeDObj(spectDOp);
      return(NULL);
    }

    if(TRACE['A']) {
      fprintf(traceFP, "Analysis parameters\n");
      fprintf(traceFP, "  sample rate = %.1f Hz\n", spectDOp->sampFreq);
      fprintf(traceFP, "  window size = %ld samples\n", frameSize);
      fprintf(traceFP, "  window shift = %ld samples\n", frameShift);
      fprintf(traceFP, "  window function = %s\n",\
	      wfSpecs(gd->winFunc)->entry->code);
      fprintf(traceFP, "  FFT length = %ld points\n", gd->numFFT);
      fprintf(traceFP, "  bin width = %.4f Hz\n", gd->binWidth);
      fprintf(traceFP, "  selected channel = %d\n", gd->channel);
      fprintf(traceFP, "  start frame = %ld\n", gd->begFrameNr);
      fprintf(traceFP, "  end frame = %ld\n", gd->endFrameNr);
      if(gd->spType == DT_FTAMP || gd->spType == DT_FTSQR ||
	 gd->spType == DT_FTPOW) {
	if(gd->spType == DT_FTAMP)
	  fprintf(traceFP, "  spectrum type = FTAMP\n");
	else if(gd->spType == DT_FTSQR)
	  fprintf(traceFP, "  spectrum type = FTSQR\n");
	else if(gd->spType == DT_FTPOW)
	  fprintf(traceFP, "  spectrum type = DFT\n");
	fprintf(traceFP, "  bandwidth = %.4f Hz\n",\
		frameSize2bandwidth(frameSize, gd->winFunc,\
				    spectDOp->sampFreq, gd->numFFT));
	fprintf(traceFP, "  pre-emphasis = %.7f\n", gd->preEmph);
      }
      else if(gd->spType == DT_FTLPS) {
	fprintf(traceFP, "  spectrum type = LPS\n");
	fprintf(traceFP, "  analysis order = %d\n", gd->order);
	fprintf(traceFP, "  pre-emphasis = %.7f\n", gd->preEmph);
	fprintf(traceFP, "  de-emphasis = %s\n",\
		(gd->options & LPS_OPT_DEEMPH) ? "ON" : "OFF");
      }
      else if(gd->spType == DT_FTCSS) {
	fprintf(traceFP, "  spectrum type = CSS\n");
	fprintf(traceFP, "  # lags = %d\n", gd->order);
      }
      else
	fprintf(traceFP, "  spectrum type = CEP\n");
      fprintf(traceFP, "  processing mode = %s-to-%s\n",\
	      FILE_IN ? "file" : "memory", FILE_OUT ? "file" : "memory");
    }

    if(aoPtr->options & AOPT_INIT_ONLY) {
      aoPtr->options &= ~AOPT_INIT_ONLY;                /* clear flag */
      return(spectDOp);                                /* no analysis */
    }
  }
  else if(gd->fftBuf == NULL) {
    if(allocBufs(gd, frameShift) < 0) {
      if(CREATED)
	freeDObj(spectDOp);
      return(NULL);
    }
  }
  if(gd->preEmph != 0.0)
    head = 1;
  else
    head = 0;
  /* loop over frames */
  err = 0;
  clrAsspMsg();
  for(fn = gd->begFrameNr; fn < gd->endFrameNr; fn++) {
    if((err=getSmpFrame(smpDOp, fn, frameSize, frameShift, head, 0,\
			gd->channel, gd->frame, SPECT_PFORMAT)) < 0) {
      break;
    }
    switch(gd->spType) {
    case DT_FTLPS:
      err = getLPSpectrum(spectDOp);
      if(err < 0) {
	bPtr = &applMessage[strlen(applMessage)];
	if(FILE_IN)
	  sprintf(bPtr, " at T = %.4f in %s",\
		  FRMNRtoTIME(fn, spectDOp->sampFreq, frameShift),\
		  myfilename(smpDOp->filePath));
	else
	  sprintf(bPtr, " at T = %.4f",\
		  FRMNRtoTIME(fn, spectDOp->sampFreq, frameShift));
	if(TRACE['d'])
	  prtAsspMsg(traceFP);
	err = 1;
      }
      break;
    case DT_FTCSS:
      err = getCSSpectrum(spectDOp);
      break;
    case DT_FTCEP:
      err = getCepstrum(spectDOp);
      break;
    default:
      err = getFTSpectrum(spectDOp);
      break;
    }
    if((err=storeSPECT(fn, spectDOp)) < 0) break;
  }       /* END LOOP OVER FRAMES */
  if(err >= 0 && FILE_OUT)
    err = asspFFlush(spectDOp, gd->writeOpts);
  if(err < 0) {
    if(CREATED)
      freeDObj(spectDOp);
    return(NULL);
  }
  return(spectDOp);
}

/*DOC

Function 'freeSPECT_GD'

Returns all memory allocated for the generic data in an SPECT data 
object.

DOC*/

void freeSPECT_GD(void *generic)
{
  SPECT_GD *gd;

  if(generic != NULL) {
    freeBufs((SPECT_GD *)generic);
    free(generic);
  }
  return;
}

/*DOC

Functions 'getFTSpectrum' 'getLPSpectrum' 'getCSSpectrum' 'getCepstrum'

Compute a single spectrum/cepstrum given the values in the SPECT data 
object pointed to by "dop". These are low-level functions but still 
provided as public since they are effecient enough to be used in the 
computation of e.g. a sonagram. See 'computeSPECT' on how to set up the 
data object in such a case. 

DOC*/

/***********************************************************************
* calculate unsmoothed spectrum                                        *
***********************************************************************/
int getFTSpectrum(DOBJ *dop)
{
  register long n, N, HN, L;
  register double *dPtr;
  register SPECT_GD *gd=(SPECT_GD *)(dop->generic);

  N = gd->numFFT;
  HN = N/2 +1;                       /* include value at Nyquist rate */
  L = gd->frameSize;
  dPtr = gd->frame;
  if(gd->preEmph != 0.0) {        /* leading value is in frame buffer */
    dPtr++;
    preEmphasis(dPtr, gd->preEmph, *(gd->frame), L);
  }
  if(gd->wfc != NULL)
    mulSigWF(dPtr, gd->wfc, L);
  for(n = 0; n < L; n++)                  /* copy frame to FFT buffer */
    gd->fftBuf[n] = *(dPtr++);
  while(n < N)                                     /* pad with zeroes */
    gd->fftBuf[n++] = 0.0;
  dPtr = gd->fftBuf;
  rfft(dPtr, N, FFT_FORWARD);
  if(gd->spType == DT_FTAMP) {           /* linear amplitude spectrum */
    rfftLinAmp(dPtr, dPtr, N);
    for(n = 0; n < HN; n++)             /* correction for window etc. */
      *(dPtr++) *= gd->corrFac;                   /* 'corrFac' linear */
  }
  else if(gd->spType == DT_FTSQR) {          /* linear power spectrum */
    rfftLinPow(dPtr, dPtr, N);
    for(n = 0; n < HN; n++)
      *(dPtr++) *= gd->corrFac;          /* 'corrFac' already squared */
  }
  else {                                      /* power spectrum in dB */
    rfftPower(dPtr, dPtr, N);
    for(n = 0; n < HN; n++)
      *(dPtr++) += gd->corrFac;            /* 'corrFac' already in dB */
  }
  return(0);
}
/***********************************************************************
* calculate LP smoothed spectrum                                       *
***********************************************************************/
int getLPSpectrum(DOBJ *dop)
{
  register long n, N, HN, L;
  register double *dPtr;
  int    err=0;
  double sqerr;
  SPECT_GD *gd=(SPECT_GD *)(dop->generic);

  N = gd->numFFT;
  HN = N/2 +1;                       /* include value at Nyquist rate */
  L = gd->frameSize;
  dPtr = gd->frame;
  if(gd->preEmph != 0.0) {        /* leading value is in frame buffer */
    dPtr++;
    preEmphasis(dPtr, gd->preEmph, *(gd->frame), L);
  }
  if(gd->wfc != NULL)
    mulSigWF(dPtr, gd->wfc, L);
  getACF(dPtr, gd->acf, L, (long)gd->order);
  err = asspDurbin(gd->acf, gd->fftBuf, NULL, &sqerr, gd->order);
  if(sqerr <= 0.0) {
    for(n = 0; n < HN; n++)
      gd->fftBuf[n] = TINYPdB;
  }
  else {
    sqerr /= (double)(L);         /* take mean to get the level right */
    sqerr *= gd->corrFac;               /* correction for window etc. */
    n = gd->order + 1;
    if(gd->preEmph != 0.0 && (gd->options & LPS_OPT_DEEMPH)) {
      /* de-emphasize coefficients */
      gd->fftBuf[n] = 0.0;    /* BUG FIX 070604: cell wasn't cleared! */
      for(NIX; n > 0; --n)
        gd->fftBuf[n] += (gd->preEmph * gd->fftBuf[n-1]);
      n = gd->order + 2;              /* now got one coefficient more */
    }
    while(n < N)                                   /* pad with zeroes */
      gd->fftBuf[n++] = 0.0;
    rfft(gd->fftBuf, N, FFT_FORWARD);
    if(gd->options & SPECT_OPT_LIN_AMP)  /* linear amplitude spectrum */
      lpInvLinAmp(gd->fftBuf, sqerr, N);
    else if(gd->options & SPECT_OPT_LIN_POW) /* linear power spectrum */
      lpInvLinPow(gd->fftBuf, sqerr, N);
    else                                      /* power spectrum in dB */
      lpInvPower(gd->fftBuf, sqerr, N);
  }
  return(err);
}
/***********************************************************************
* calculate cepstral smoothed spectrum the hard way                    *
***********************************************************************/
int getCSSpectrum(DOBJ *dop)
{
  register long n, N, M, L;
  register double *buf;
  SPECT_GD *gd=(SPECT_GD *)(dop->generic);
  double norm, val;
  
  N = gd->numFFT;
  L = gd->frameSize;
  buf = gd->fftBuf;
  if(gd->wfc != NULL)
    mulSigWF(gd->frame, gd->wfc, L);
  for(n = 0; n < L; n++) {                  /* copy to complex buffer */
    buf[2*n] = gd->frame[n];
    buf[2*n +1] = 0.0;                  /* set imaginary part to zero */
  }
  while(n < N){                                     /* pad with zeroes */
    buf[2*n +1] = 0.0;
    buf[2*n++] = 0.0;
  }
  fft(buf, N, FFT_FORWARD);
  fftPower(buf, N);                           /* power spectrum in dB */
  fft(buf, N, FFT_INVERSE);                    /* convert to cepstrum */

  if((TRACE['F'] || TRACE['f']) && TRACE['c']) {
    fprintf(traceFP, "Re  ");
    for(n = 0; n < 2*N; n += 2) {
      fflush(traceFP);
      fprintf(traceFP, "%+.4e ", buf[n]);
    }
    fprintf(traceFP, "\nIm  ");
    for(n = 1; n < 2*N; n += 2) {
      fflush(traceFP);
      fprintf(traceFP, "%+.4e ", buf[n]);
    }
    fprintf(traceFP, "\n");
  }

  norm = 1.0 / (double)N;                 /* fft() does not normalize */
  M = gd->order;                 /* here: number of lags/coefficients */
  for(n = 0; n <= M; n++) {         /* cep[0] not included in 'order' */
    buf[2*n] *= norm;                                    /* normalize */
    buf[2*n +1] = 0.0;            /* ensure imaginary part to be zero */
  }
  M = N - M;                                       /* symmetrical !!! */
  for(NIX; n < M; n++) {
    buf[2*n] = 0.0;                /* zero high cepstral coefficients */
    buf[2*n +1] = 0.0;            /* ensure imaginary part to be zero */
  }
  for(NIX; n < N; n++) {
    buf[2*n] *= norm;                             /* normalize mirror */
    buf[2*n +1] = 0.0;            /* ensure imaginary part to be zero */
  }

  if((TRACE['F'] || TRACE['f']) && TRACE['C']) {
    fprintf(traceFP, "Re  ");
    for(n = 0; n < 2*N; n += 2) {
      fflush(traceFP);
      fprintf(traceFP, "%+.4e ", buf[n]);
    }
    fprintf(traceFP, "\n");
  }

  fft(buf, N, FFT_FORWARD);                       /* back to spectrum */
  N = N / 2 + 1;                       /* up to and including Nyquist */
  for(n = 0; n < N; n++) {           /* extract real part and correct */
    val = gd->fftBuf[2*n] + gd->corrFac;         /* ought to be in dB */
    if(gd->options & SPECT_OPT_LIN_AMP)  /* linear amplitude spectrum */
      val = dBtoLIN(val);
    else if(gd->options & SPECT_OPT_LIN_POW) /* linear power spectrum */
      val = dBtoSQR(val);
    *(buf++) = val;
  }
  return(0);
}
/***********************************************************************
* calculate real cepstrum (based on natural logarithm)                 *
* NOTE: there appear to be several definitions for the cepstrum; this  *
* one corresponds to the Matlab code                                   *
***********************************************************************/
int getCepstrum(DOBJ *dop)
{
  register long n, N, M, L;
  register double *buf;
  SPECT_GD *gd=(SPECT_GD *)(dop->generic);
  double val, norm;
  
  N = gd->numFFT;
  L = gd->frameSize;
  buf = gd->fftBuf;
  if(gd->wfc != NULL)
    mulSigWF(gd->frame, gd->wfc, L);
  for(n = 0; n < L; n++) {                  /* copy to complex buffer */
    buf[2*n] = gd->frame[n];
    buf[2*n +1] = 0.0;                  /* set imaginary part to zero */
  }
  while(n < N){                                     /* pad with zeroes */
    buf[2*n +1] = 0.0;
    buf[2*n++] = 0.0;
  }
  fft(buf, N, FFT_FORWARD);
  fftlnMag(buf, N);                         /* log magnitude spectrum */
  fft(buf, N, FFT_INVERSE);                    /* convert to cepstrum */
  norm = 1.0 / (double)N;                 /* fft() does not normalize */
  norm *= (gd->corrFac);             /* include correction for window */
  N = N / 2 + 1;                       /* up to and including Nyquist */
  for(n = 0; n < N; n++) {           /* extract real part and correct */
    val = gd->fftBuf[2*n] * norm;
    if(gd->options & SPECT_OPT_LIN_POW)
      val *= val;
    *(buf++) = val;
  }
  return(0);
}

/* ======================= private  functions ======================= */

/***********************************************************************
* allocate memory for processing buffers and computee window function  *
***********************************************************************/
LOCAL int allocBufs(SPECT_GD *gd, long frameShift)
{
  int    wFlags=0;
  size_t frameSize;

  gd->frame = gd->fftBuf = gd->wfc = gd->acf = NULL;
  frameSize = (size_t)(gd->frameSize);
  if(gd->preEmph != 0.0)                 /* space for leading element */
    frameSize++;
  gd->frame = (double *)calloc(frameSize, sizeof(double));
  if(gd->spType == DT_FTCSS || gd->spType == DT_FTCEP) /* complex fft */
    gd->fftBuf = (double *)calloc(2*(size_t)gd->numFFT, sizeof(double));
  else                                        /* fft for real signals */
    gd->fftBuf = (double *)calloc((size_t)gd->numFFT, sizeof(double));
  if(gd->frame == NULL || gd->fftBuf == NULL) {
    freeBufs(gd);
    setAsspMsg(AEG_ERR_MEM, "(SPECT: allocBufs)");
    return(-1);
  }
  if(gd->winFunc > WF_RECTANGLE) {
    if(gd->spType != DT_FTLPS && gd->frameSize == gd->numFFT)
      wFlags = WF_ASYMMETRIC;                /* use proper DFT window */
    else {
      wFlags = WF_PERIODIC;
      if((ODD(gd->frameSize) && EVEN(frameShift)) ||
	 (EVEN(gd->frameSize) && ODD(frameShift)) )
	wFlags = WF_ASYMMETRIC;     /* align window and frame centres */
    }
    gd->wfc = makeWF(gd->winFunc, gd->frameSize, wFlags);
    if(gd->wfc == NULL) {
      freeBufs(gd);
      setAsspMsg(AEG_ERR_MEM, "(SPECT: allocBufs)");
      return(-1);
    }
  }
  if(gd->spType == DT_FTLPS) {
    gd->acf = (double *)calloc((size_t)(gd->order + 1), sizeof(double));
    if(gd->acf == NULL) {
      freeBufs(gd);
      setAsspMsg(AEG_ERR_MEM, "(SPECT: allocBufs)");
      return(-1);
    }
  }
  return(0);
}

/***********************************************************************
* free memory allocated in the generic data                            *
***********************************************************************/
LOCAL void freeBufs(SPECT_GD *gd)
{
  if(gd != NULL) {
    if(gd->frame != NULL)
      free((void *)(gd->frame));
    if(gd->fftBuf != NULL)
      free((void *)(gd->fftBuf));
    freeWF(gd->wfc);
    if(gd->acf != NULL)
      free((void *)(gd->acf));
    gd->frame = gd->fftBuf = gd->wfc = gd->acf = NULL;
  }
  return;
}

/***********************************************************************
* copy spectrum in 'fftBuf' in generic data of "dop" to output buffer; *
* handle data writes                                                   *
***********************************************************************/
LOCAL int storeSPECT(long frameNr, DOBJ *dop)
{
  register long      ndx, n, N;
  register float    *fPtr;
  register double   *sPtr, *dPtr;
  int       FILE_OUT;
  SPECT_GD *gd;

  FILE_OUT = (dop->fp != NULL);
  gd = (SPECT_GD *)dop->generic;
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
      setAsspMsg(AEG_ERR_BUG, "storeSPECT: buffer overflow");
      return(-1);
    }
  }
  N = dop->ddl.numFields;
  sPtr = gd->fftBuf;
  ndx = frameNr - dop->bufStartRec;
  if(dop->ddl.format == DF_REAL64) {
    dPtr = (double *)(dop->dataBuffer);
    dPtr = &dPtr[ndx*N];
    for(n = 0; n < N; n++)
      *(dPtr++) = *(sPtr++);
  }
  else {
    fPtr = (float *)(dop->dataBuffer);
    fPtr = &fPtr[ndx*N];
    for(n = 0; n < N; n++)
      *(fPtr++) = (float)(*(sPtr++));
  }
  if(ndx >= dop->bufNumRecs)
    dop->bufNumRecs = ndx + 1;
  dop->bufNeedsSave = TRUE;

  return(0);
}

/***********************************************************************
* In-place conversion of the output from 'rfft' in array pointed to by *
* "c" with length given by "N" to inverse LP spectrum. "msqr" should   *
* give the mean sum of squares of the LP residual.                     *
* 'lpInvLinAmp' computes the linear amplitude, 'lpInvLinPow' the linear*
* power spectrum and 'lpInvPower' the power spectrum in dB.            *
* These functions include compensation for the idiosyncratic -6 dB off *
* levels at DC and the Nyquist rate when using 'rfft'.                 *
***********************************************************************/
LOCAL void lpInvLinAmp(register double *c, double msqr, long N)
{
  register long    n, HN;
  register double *a;
  double Re, Im, invAmp;
  
  HN = N / 2;
  a = c;                       /* do in-place conversion */
  if(msqr <= 0.0) {            /* bottom clip */
    for(n = 0; n <= HN; n++)
      *(a++) = 0.0;
  }
  else {
    msqr = sqrt(msqr);         /* convert to RMS amplitude */
    invAmp = fabs(*(c++));     /* DC component; Im = 0 */
    invAmp *= 2.0;             /* apparently factor 2 off */
    if(invAmp != 0.0)
      *(a++) = msqr / invAmp;
    else
      *(a++) = 1.0 / TINYLIN;  /* set huge */
    for(n = 1; n < HN; n++) {
      Re = *(c++);
      Im = *(c++);
      invAmp = hypot(Re, Im);
      if(invAmp != 0.0)
	*(a++) = msqr / invAmp;
      else
	*(a++) = 1.0 / TINYLIN;
    }
    invAmp = fabs(*(c++));     /* Nyquist component; Im = 0 */
    invAmp *= 2.0;             /* apparently factor 2 off */
    if(invAmp != 0.0)
      *(a++) = msqr / invAmp;
    else
      *(a++) = 1.0 / TINYLIN;
  }
  return;
}

LOCAL void lpInvLinPow(register double *c, double msqr, long N)
{
  register long    n, HN;
  register double *p;
  double invSqr;
  
  HN = N / 2;
  p = c;                       /* do in-place conversion */
  if(msqr <= 0.0) {            /* bottom clip */
    for(n = 0; n <= HN; n++)
      *(p++) = 0.0;
  }
  else {
    invSqr = (*c) * (*c);      /* DC component; Im = 0 */
    invSqr *= 4.0;             /* apparently factor 4 off */
    c++;
    if(invSqr != 0.0)
      *(p++) = msqr / invSqr;
    else
      *(p++) = 1.0 / TINYSQR;  /* set huge */
    for(n = 1; n < HN; n++) {
      invSqr = (*c) * (*c);    /* Re ^ 2 */
      c++;
      invSqr += ((*c) * (*c)); /* Im ^ 2 */
      c++;
      if(invSqr != 0.0)
        *(p++) = msqr / invSqr;
      else
        *(p++) = 1.0 / TINYSQR;
    }
    invSqr = (*c) * (*c);      /* Nyquist component; Im = 0 */
    invSqr *= 4.0;             /* apparently factor 4 off */
    if(invSqr != 0.0)
      *p = msqr / invSqr;
    else
      *p = 1.0 / TINYSQR;
  }
  return;
}

LOCAL void lpInvPower(register double *c, double msqr, long N)
{
  register long    n, HN;
  register double *p;
  double invPow;
  
  HN = N / 2;
  p = c;                       /* do in-place conversion */
  if(msqr <= TINYSQR) {        /* bottom clip */
    for(n = 0; n <= HN; n++)
      *(p++) = TINYPdB;
  }
  else {
    msqr = SQRtodB(msqr);      /* convert to dB; divide => subtract */
    invPow = fabs(*(c++));     /* DC component; Im = 0 */
    invPow *= 2.0;             /* apparently factor 2 off */
    if(invPow != 0.0)
      *(p++) = msqr - LINtodB(invPow);
    else
      *(p++) = -TINYPdB;       /* set huge */
    for(n = 1; n < HN; n++) {
      invPow = (*c) * (*c);    /* Re ^ 2 */
      c++;
      invPow += ((*c) * (*c)); /* Im ^ 2 */
      c++;
      if(invPow != 0.0)
	*(p++) = msqr - SQRtodB(invPow);
      else
	*(p++) = -TINYPdB;
    }
    invPow = fabs(*c);         /* Nyquist component; Im = 0 */
    invPow *= 2.0;             /* apparently factor 2 off */
    if(invPow != 0.0)
      *p = msqr - LINtodB(invPow);
    else
      *p = -TINYPdB;
  }
  return;
}

/***********************************************************************
* convert output from 'fft' to power spectrum in dB for cepstrum       *
* dB values are stored on real coefficients, imaginary set to zero     *
***********************************************************************/
LOCAL void fftPower(register double *c, register long N)
{
  register long    n;
  register double *p;
  double pwr;
  
  p = c;                       /* do an in-place conversion */
  for(n = 0; n < N; n++) {
    pwr = (*c) * (*c);         /* Re^2 */
    c++;
    pwr += ((*c) * (*c));      /* + Im^2 */
    c++;
    if(pwr <= TINYSQR)
      *(p++) = TINYPdB;
    else
      *(p++) = SQRtodB(pwr);
    *(p++) = 0.0;              /* imaginary part set to zero */
  }
  return;
}

/***********************************************************************
* convert output from 'fft' to (natural) log magnitude spectrum        *
* log values are stored on real coefficients, imaginary set to zero    *
***********************************************************************/
LOCAL void fftlnMag(register double *c, register long N)
{
  register long    n;
  register double *p;
  double pwr;
  
  p = c;                       /* do an in-place conversion */
  for(n = 0; n < N; n++) {
    pwr = (*c) * (*c);         /* Re^2 */
    c++;
    pwr += ((*c) * (*c));      /* + Im^2 */
    c++;
    if(pwr <= TINYSQR)
      *(p++) = TINYLN;
    else
      *(p++) = 0.5 * log(pwr); /* div 2 to get magnitude */
    *(p++) = 0.0;              /* imaginary part set to zero */
  }
  return;
}
