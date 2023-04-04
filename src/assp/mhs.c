/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1979 - 1983  Michel Scheffers                          *
*                            Institute for Perception Research (IP0)   *
*                            Den Dolech 2                              *
*                            5600 MB Eindhoven, The Netherlands        *
* Copyright (C) 1986 - 2010  Michel Scheffers                          *
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
* File:     mhs.c                                                      *
* Contents: Functions implementing the 'MHS' pitch algorithm.          *
*           (see printMHSrefs() for references)                        *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: mhs.c,v 1.19 2010/10/18 13:07:34 mtms Exp $ */

#include <stdio.h>     /* FILE NULL etc. */
#include <stdlib.h>    /* malloc() calloc() free() qsort() */
#include <stddef.h>    /* size_t */
#include <string.h>    /* memset() strcpy() strchr() memcpy() */
#include <ctype.h>     /* tolower() */
#include <inttypes.h>  /* int16_t */
#include <math.h>      /* log() pow() */

#include <miscdefs.h>  /* TRUE FALSE LOCAL NATIVE_EOL */
#include <misc.h>      /* strxcmp() myrint() */
#include <trace.h>     /* trace handler */
#include <asspmess.h>  /* error message handler */
#include <assptime.h>  /* standard conversion macros */
#include <mhs.h>       /* private constants, structures and prototypes */
#include <asspana.h>   /* AOPTS anaTiming() */
#include <asspdsp.h>   /* windowing, getMaxMag() getZCR() getCCF() etc. */
#include <asspfio.h>   /* asspFFlush() */
#include <dataobj.h>   /* DOBJ getSmpCaps() getSmpFrame() */
#include <headers.h>   /* KDTAB */
#include <aucheck.h>   /* checkSound() */

/*
 * structures
 */
typedef struct MHS_peak_data {
  double freq; /* in Hz */
  double amp;  /* in dB */
} MHS_PEAK;

typedef struct MHS_pitch_candidate {
  double F0;
  int    Q;    /* upscaled by MHS_Q_SCALE, then rounded */
} MHS_CAND;

/*
 * pitch tracking
 */
typedef struct MHS_pitch_link {
  struct MHS_pitch_link *prev;
  struct MHS_pitch_link *next;
  MHS_CAND cand;
} MHS_LINK;

typedef struct MHS_pitch_track {
  MHS_LINK *chain;    /* pointer to youngest member of track */
  double    duration; /* duration of track in ms */
  double    periods;  /* duration as estimated number of periods */
  double    trackQ;   /* estimate based on last few Q values */
  int       status;   /* track status flags (see below) */
} MHS_TRK;

#define MHS_TRK_UNUSED  0x00 /* track status flags */
#define MHS_TRK_IN_USE  0x01
#define MHS_TRK_UPDATED 0x02
#define MHS_TRK_PENDING 0x04
#define MHS_TRK_ACTIVE  0x08

/*
 * local global arrays and variables
 */
LOCAL char    secFormat[32];
LOCAL double  winShift;

LOCAL long    numFFT;
LOCAL double *fftBuf=NULL;                  /* FFT buffer (allocated) */
LOCAL double *logN=NULL; /* ln of indices for getSpectrum (allocated) */
LOCAL long    minBin, maxBin;                    /* convolution range */
LOCAL double  binFreq;                  /* resolution of FFT spectrum */

LOCAL double  wfGain;        /* gain of window function in dB ( < 0 ) */
LOCAL double  wfHSLL;    /* relative power of highest side lobe level */
LOCAL double *wfc=NULL;   /* window function coefficients (allocated) */

LOCAL size_t  minPeaks;
LOCAL size_t  maxMesh;                /* highest mesh number of sieve */
LOCAL double  meshWidth;                             /* width of mesh */
LOCAL double  meshTol;            /* tolerance to fit a mesh (factor) */
LOCAL double  sieveF0[MHS_MAXSIEVES];            /* list of sieve F0s */

LOCAL double  minF0Diff; /* min. difference in F0 candidates (factor) */
LOCAL double  maxDelta;  /* max. frame-to-frame change in F0 (factor) */
LOCAL MHS_TRK track[MHS_MAXTRACKS];
LOCAL size_t  maxNumTQ;    /* maximum number of frames for Q of track */

LOCAL size_t    pipeLength;               /* length of pipe in frames */
LOCAL MHS_CAND *pipe=NULL;       /* delay line for output (allocated) */
LOCAL long      pipeBegFn, pipeEndFn;    /* valid frame range in pipe */
LOCAL MHS_CAND  unv={0.0, 0};                       /* unvoiced frame */

/*
 * prototypes of local functions
 */
LOCAL void setFrameSize(DOBJ *dop);
LOCAL int  setGlobals(DOBJ *dop);
LOCAL void freeGlobals(void);
LOCAL double *getSpectrum(MHS_GD *gd);
LOCAL int  findPeaks(double *linPower, MHS_PEAK *peak, MHS_GD *gd);
LOCAL int  sievePeaks(MHS_PEAK *peak, int numPeaks, MHS_CAND *cand, MHS_GD *gd);
LOCAL int  trackPitch(long frameNr, MHS_CAND *cand, DOBJ *dop);
LOCAL int  addLink(MHS_TRK *tPtr, MHS_CAND *cand);
LOCAL int  pipeTrack(long frameNr, MHS_TRK *tPtr, DOBJ *dop);
LOCAL void delTrack(MHS_TRK *tPtr);
LOCAL int  pipeFrame(long frameNr, MHS_CAND *cand, DOBJ *dop);
LOCAL int  flushPipe(DOBJ *dop);
LOCAL int  storeMHS(float val, long frameNr, DOBJ *dop);
LOCAL int  insElement(void *array, size_t numElements, size_t elementSize,\
		      size_t index, void *element);
LOCAL int  rmvElement(void *array, size_t numElements, size_t elementSize,\
		      size_t index, void *fill);

/* ======================== public functions ======================== */

/*DOC

Function 'printMHSrefs'

Prints the references for the DWS and MDWS algorithms from which MHS is 
derived.

DOC*/

void printMHSrefs(void)
{
#ifndef WRASSP
  printf("\nReferences:\n");
  printf("Duifhuis, H., Willems, L.F., and Sluyter, R.J. (1982). \"Measurement\n");
  printf("   of pitch in speech: An implementation of Goldstein's theory of\n");
  printf("   pitch perception,\" J.Acoust.Soc.Am. 71, 1568-1580.\n");
  printf("Scheffers, M.T.M. (1983). \"Simulation of auditory analysis of\n");
  printf("   pitch: An elaboration on the DWS pitch meter,\"\n");
  printf("   J.Acoust.Soc.Am. 74, 1716-1725.\n");
  printf("Allik, J., Mihkla, M. and Ross, J. (1984). \"Comment on 'Measurement\n");
  printf("   of pitch in speech: An implementation of Goldstein's theory of\n");
  printf("   pitch perception',\" J.Acoust.Soc.Am. 75, 1855-1857.\n");
#endif
  return;
}

/*DOC

Function 'setMHSgenderDefaults'

Sets the items 'gender', 'minF' and 'maxF' in the analysis options 
structure pointed to by "aoPtr" to the default values for "gender".
Returns 0 upon success and -1 upon error.

DOC*/

int setMHSgenderDefaults(AOPTS *aoPtr, char gender)
{
  if(aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setMHSgenderDefaults");
    return(-1);
  }
  switch(gender) {
  case 'F': /* female */
  case 'f':
    aoPtr->minF = MHS_MINF0_f;
    aoPtr->maxF = MHS_MAXF0_f;
    break;
  case 'M': /* male */
  case 'm':
    aoPtr->minF = MHS_MINF0_m;
    aoPtr->maxF = MHS_MAXF0_m;
    break;
  case 'U': /* unknown */
  case 'u':
    aoPtr->minF = MHS_MINF0_u;
    aoPtr->maxF = MHS_MAXF0_u;
    break;
  default:
    setAsspMsg(AEG_ERR_BUG, NULL);
    snprintf(applMessage, sizeof(applMessage), "setMHSgenderDefaults: invalid gender code '%c'",\
	    gender);
    return(-1);
  }
  aoPtr->gender = tolower((int)gender);
  return(0);
}

/*DOC

Function 'setMHSdefaults'

Sets the items in the analysis options structure relevant to the MHS 
pitch analysis algorithm to their default values. Clears all other 
items.
Returns 0 upon success and -1 upon error.

DOC*/

int setMHSdefaults(AOPTS *aoPtr)
{
  if(aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setMHSdefaults");
    return(-1);
  }
  memset((void *)aoPtr, 0, sizeof(AOPTS));         /* clear all items */
  aoPtr->centreTime = -1.0;                          /* not supported */
  aoPtr->msSize = 0.0;                        /* adaptive window size */
  aoPtr->msShift = MHS_DEF_SHIFT;                 /* frame shift (ms) */
  aoPtr->voiProb = MHS_DEF_MINQVAL;
  aoPtr->voiMag = MHS_DEF_VOIMAG;
  aoPtr->voiZCR = MHS_DEF_VOIZCR;
  aoPtr->voiRMS = MHS_DEF_VOIRMS;
  aoPtr->voiAC1 = MHS_DEF_VOIAC1;
  aoPtr->channel = MHS_DEF_CHANNEL;         /* channel to be analysed */
  aoPtr->precision = MHS_DEF_DIGITS;    /* digits precision for ASCII */
  strcpy(aoPtr->format, MHS_DEF_FORMAT);        /* output file format */
  /* gender-specific defaults for min/maxF0 */
  if(setMHSgenderDefaults(aoPtr, MHS_DEF_GENDER) < 0)
    return(-1);
  return(0);
}

/*DOC

Function 'createMHS'

Allocates memory for a data object to hold MHS pitch analysis data and 
initializes it on the basis of the information in the audio object 
pointed to by "smpDOp" and the analysis options in the structure pointed 
to by "aoPtr". Neither of these pointers may be NULL.
Returns a pointer to the data object or NULL upon error.

Note:
 - This function DOES NOT allocate memory for the data buffer. Depending 
   on your application you can either use 'allocDataBuf' for this or 
   delegate it to 'computeMHS'.

DOC*/

DOBJ *createMHS(DOBJ *smpDOp, AOPTS *aoPtr)
{
  long    auCaps;
  ATIME   aTime, *tPtr;
  MHS_GD *gd=NULL;
  DOBJ   *dop=NULL;
  DDESC  *dd=NULL;
  KDTAB  *entry;

  if(smpDOp == NULL || aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "createMHS");
    return(NULL);
  }
  if(aoPtr->options & AOPT_USE_CTIME) {
    setAsspMsg(AEB_BAD_CALL, "createMHS: no single-frame analysis");
    return(NULL);
  }
  /* verify audio object */
  if((auCaps=getSmpCaps(MHS_PFORMAT)) <= 0)
    return(NULL);
  auCaps |= MHS_I_CHANS;
  if(aoPtr->channel < 1)
    aoPtr->channel = 1;
  if(checkSound(smpDOp, auCaps, aoPtr->channel) <= 0)
    return(NULL);
  /* ensure correct interpretation by anaTiming() */
  aoPtr->msSize = 0.0;                        /* adaptive window size */
  aoPtr->options &= ~AOPT_EFFECTIVE;
  tPtr = &aTime;
  if(anaTiming(smpDOp, aoPtr, tPtr) < 0)
    return(NULL);
  clrAsspMsg();                               /* ignore warnings here */
  if((aoPtr->maxF * MHS_MINPEAKS * 2.0) >= smpDOp->sampFreq) {
    asspMsgNum = AEG_ERR_APPL;
    if(smpDOp->filePath != NULL)
      snprintf(applMessage, sizeof(applMessage), "Maximum pitch too high for sample rate in %s",\
	      smpDOp->filePath);
    else
      snprintf(applMessage, sizeof(applMessage), "Maximum pitch too high for sample rate");
    return(NULL);
  }
  if(aoPtr->minF < MHS_ABSMIN_F0) {
    asspMsgNum = AEG_ERR_APPL;
    snprintf(applMessage, sizeof(applMessage), "Minimum pitch too low (minimally %d)",\
	    (int)MHS_ABSMIN_F0);
    return(NULL);
  }
  if(aoPtr->maxF <= aoPtr->minF) {
    asspMsgNum = AEG_ERR_APPL;
    snprintf(applMessage, sizeof(applMessage), "Maximum pitch <= minimum pitch");
    return(NULL);
  }
  if((gd=(MHS_GD *)malloc(sizeof(MHS_GD))) == NULL) {
    setAsspMsg(AEG_ERR_MEM, "(createMHS)");
    return(NULL);
  }
  strcpy(gd->ident, MHS_GD_IDENT);
  gd->options = aoPtr->options;
  gd->begFrameNr = tPtr->begFrameNr;
  gd->endFrameNr = tPtr->endFrameNr;
  /* frameSize set below after object has been filled out */
  gd->winFunc = wfType(MHS_WINFUNC);                         /* fixed */
  if(gd->winFunc <= WF_NONE) {
    freeMHS_GD((void *)gd);
    setAsspMsg(AEB_BAD_WIN, MHS_WINFUNC);
    return(NULL);
  }
  gd->minF0 = aoPtr->minF;
  gd->maxF0 = aoPtr->maxF;
  gd->minQval= myrint(aoPtr->voiProb * MHS_Q_SCALE);
  gd->voiAC1 = aoPtr->voiAC1;
  gd->voiMag = aoPtr->voiMag;
  gd->voiRMS = aoPtr->voiRMS;
  gd->voiZCR = aoPtr->voiZCR;
  gd->channel = aoPtr->channel;
  gd->precision = aoPtr->precision;

  if((dop=allocDObj()) == NULL) {
    freeMHS_GD((void *)gd);
    strcpy(applMessage, "(createMHS)");
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
  dop->doFreeGeneric = (DOfreeFunc)freeMHS_GD;
  setFrameSize(dop);

  dd = &(dop->ddl);                 /* set pointer to data descriptor */
  dd->type = DT_PIT;
  dd->coding = DC_LIN;
  dd->format = MHS_DFORMAT;
  dd->numFields = 1;
  if(dop->fileFormat == FF_SSFF) {
    dd->ident = strdup(MHS_SSFF_ID);
    strcpy(dd->unit, "Hz");
  }
  else if(dop->fileFormat == FF_XASSP) {
    entry = dtype2entry(dd->type, KDT_XASSP); /* search XASSP keyword */
    if(entry != NULL && entry->keyword != NULL) {
      dd->ident = strdup(entry->keyword);
      if(entry->factor != NULL)
	 strcpy(dd->factor, entry->factor);
      if(entry->unit != NULL)
	strcpy(dd->unit, entry->unit);
    }
    else {
      dop = freeDObj(dop);
      setAsspMsg(AEB_ERR_TRACK, "(createMHS)");
      return(NULL);
    }
    strcpy(dop->sepChars, "\t");                    /* between fields */
    strcpy(dd->sepChars, " ");                        /* within field */
    snprintf(dd->ascFormat, member_size(DDESC, ascFormat), "%%.%df", gd->precision);
  }
  else { /* fall through to raw ASCII */
    dd->ident = strdup("PITCH");
    strcpy(dd->unit, "Hz");
    strcpy(dop->sepChars, "\t");                    /* between fields */
    strcpy(dd->sepChars, " ");                        /* within field */
    snprintf(dd->ascFormat, member_size(DDESC, ascFormat), "%%.%df", gd->precision);
  }
  setRecordSize(dop);
  setStart_Time(dop);
  return(dop);
}

/*DOC

Function 'computeMHS'

Performs a pitch analysis of the audio signal referred to by "smpDOp" 
using the 'MHS' algorithm with parameter settings specified in the 
generic data structure of the output data object pointed to by "pitDOp".
If "pitDOp" is a NULL-pointer "aoPtr" may not be a NULL-pointer and 
this function will create the output data object (see 'createMHS' for
details).
If "aoPtr" is not a NULL-pointer this function will verify and - if 
necessary - (re-)allocate the data buffers in the data objects pointed 
to by "smpDOp" and "pitDOp" to have appropriate size.
Analysis results will be returned in the data buffer of the object 
pointed to by "pitDOp" or written to file if that object refers to a 
file opened for writing. 
Returns a pointer to the MHS-pitch data object or NULL upon error.

Note:
 - This function may be used in a file-to-file, file-to-memory, memory-
   to-file and memory-to-memory mode.
 - This function DOES NOT verify whether the analysis parameters in the 
   generic data structure of "pitDOp" comply with those in the general 
   analysis structure pointed to by "aoPtr". You will have to implement 
   your own verification if parameter changes may occur between calls 
   (see createMHS() and e.g. verifyACF() for what needs to be done). 
   If there are incompatibilities it is probably easiest just to destroy 
   the data object.

DOC*/

DOBJ *computeMHS(DOBJ *smpDOp, AOPTS *aoPtr, DOBJ *pitDOp)
{
  int      FILE_IN, FILE_OUT, CREATED, VOICED;
  int      err, n, numPeaks, numCands;
  long     lenMAG, lenZCR, lenACF;
  size_t   offMAG, offZCR, offACF;
  long     i, fn, frameSize, frameShift;
  double  *dPtr;
  double   MAG, ZCR, acf[2], RMS, AC1;
  double  *powSpect;
  MHS_GD  *gd;
  MHS_PEAK peak[MHS_MAXPEAKS];
  MHS_CAND cand[MHS_MAXCANDS];

  if(smpDOp == NULL || (aoPtr == NULL && pitDOp == NULL)) {
    setAsspMsg(AEB_BAD_ARGS, "computeMHS");
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
      setAsspMsg(AED_NO_DATA, "(computeMHS)");
      return(NULL);
    }
  }
  /* check status of output object */
  if(pitDOp == NULL) {
    if((pitDOp=createMHS(smpDOp, aoPtr)) == NULL)
      return(NULL);
    CREATED = TRUE;
  }
  gd = (MHS_GD *)pitDOp->generic;
  if(gd->options & AOPT_USE_CTIME) {
    setAsspMsg(AEB_BAD_CALL, "createMHS: no single-frame analysis");
    return(NULL);
  }
  if(pitDOp->fp != NULL) {
    FILE_OUT = TRUE;
    gd->writeOpts = AFW_CLEAR;        /* discard data after writing */
    if(pitDOp->fileData == FDF_ASC)
      gd->writeOpts |= AFW_ADD_TIME;
  }
  else
    gd->writeOpts = AFW_KEEP;        /* just for consistency's sake */

  frameSize = gd->frameSize;
  frameShift = pitDOp->frameDur;
  if(aoPtr != NULL) {
    if(checkDataBufs(smpDOp, pitDOp, frameSize,\
		     gd->begFrameNr, gd->endFrameNr) < 0) {
      if(CREATED)
	freeDObj(pitDOp);
      return(NULL);
    }
    if(aoPtr->options & AOPT_INIT_ONLY) {
      aoPtr->options &= ~AOPT_INIT_ONLY;                /* clear flag */
      return(pitDOp);                                  /* no analysis */
    }
  }
  /* set window lengths and offsets for voicing decision */
  lenMAG = lenACF = FREQtoPERIOD(gd->minF0, pitDOp->sampFreq);
  lenZCR = lenMAG + ZCR_HEAD;
  offMAG = (size_t)(gd->frameSize - lenMAG + 1) / 2;
  offZCR = (size_t)(gd->frameSize - lenZCR) / 2;
  offACF = (size_t)(gd->frameSize - lenACF + 1) / 2;
  /* set global values and allocate buffer space */
  if(setGlobals(pitDOp) < 0) {
    if(CREATED)
      freeDObj(pitDOp);
    return(NULL);
  }
#ifndef WRASSP
  if(TRACE['A']) {
    fprintf(traceFP, "Analysis parameters\n");
    fprintf(traceFP, "  sample rate = %.1f Hz\n", pitDOp->sampFreq);
    fprintf(traceFP, "  window size = %ld samples\n", frameSize);
    fprintf(traceFP, "  window shift = %ld samples\n", frameShift);
    fprintf(traceFP, "  window function = %s\n",\
	    wfSpecs(gd->winFunc)->entry->code);
    fprintf(traceFP, "  pitch range = %.1f to %.1f Hz\n",\
	    gd->minF0, gd->maxF0);
    fprintf(traceFP, "  FFT length = %ld\n", numFFT);
    fprintf(traceFP, "  voicing thresholds: MAG=%.0f  ZCR=%.1f  RMS=%.1f"\
                     "  AC1=%.3f  minQ=%3d\n",\
	    gd->voiMag, gd->voiZCR, gd->voiRMS, gd->voiAC1,\
	    gd->minQval);
    fprintf(traceFP, "  masked spectrum = %s\n",\
	    (gd->options & MHS_OPT_POWER) ? "OFF":"ON");
    fprintf(traceFP, "  harmonic sieve: %zd meshes  width %.4f"\
	             "  tolerance %.4f\n",\
	    maxMesh, meshWidth, meshTol);
    fprintf(traceFP, "  tracking: minF0Diff=%.4f  maxDelta=%.4f\n",\
	    minF0Diff, maxDelta);
    fprintf(traceFP, "  selected channel = %d\n", gd->channel);
    fprintf(traceFP, "  start frame = %ld\n", gd->begFrameNr);
    fprintf(traceFP, "  end frame = %ld\n", gd->endFrameNr);
    fprintf(traceFP, "  processing mode = %s-to-%s\n",\
	    FILE_IN ? "file" : "memory", FILE_OUT ? "file" : "memory");
  }
#endif
  /* loop over frames */
  for(err = 0, fn = gd->begFrameNr; fn < gd->endFrameNr; fn++) {
#ifndef WRASSP
    if(TRACE['c'] || TRACE['P'] || TRACE['v']) {
      fprintf(traceFP, secFormat, (double)fn * winShift);
      fflush(traceFP);
    }
#endif
    if((err=getSmpFrame(smpDOp, fn, frameSize, frameShift, 0, 0,\
			gd->channel, (void *)fftBuf, MHS_PFORMAT)) < 0) {
      break;
    }
    dPtr = &fftBuf[frameSize];
    for(i = frameSize; i < numFFT; i++)
      *(dPtr++) = 0.0;                            /* FFT zero-padding */
    /*
     * voicing detection
     */
    MAG = 0.0;  /* initialize to definitively unvoiced for TRACE['V'] */
    ZCR = 0.0;
    RMS = -99;
    AC1 = -9;
    numPeaks = 0;
    for(n = 0; n < MHS_MAXCANDS; n++) {      /* clear candidate array */
      cand[n].F0 = 0.0; 
      cand[n].Q = 0;
    }
    numCands = 0;
    VOICED = TRUE;                          /* think positive, though */

    MAG = getMaxMag(&fftBuf[offMAG], lenMAG);
    if(MAG < gd->voiMag)
      VOICED = FALSE;
    if(VOICED || TRACE['v']) {
      if(gd->voiZCR > gd->maxF0) {
	ZCR = getZCR(&fftBuf[offZCR], lenZCR, pitDOp->sampFreq);
	if(ZCR < gd->minF0  || ZCR > gd->voiZCR)
	  VOICED = FALSE;
      }
      if(VOICED || TRACE['v']) {
/*         getCCF(&fftBuf[offACF], &fftBuf[offACF], acf, lenACF, 1); */
/* 	RMS = sqrt(acf[0]/(double)lenACF); */
        getMeanACF(&fftBuf[offACF], acf, lenACF, 1);
	RMS = sqrt(acf[0]);
        if(RMS <= RMS_MIN_AMP)
          RMS = RMS_MIN_dB;
        else
	  RMS = LINtodB(RMS);
        if(RMS < gd->voiRMS)
          VOICED = FALSE;
        if(VOICED || TRACE['v']) {
          if(acf[0] > 0.0)                          /* avoid crashing */
            AC1 = acf[1] / acf[0];
       /* else keep initial value */
          if(AC1 < gd->voiAC1)
            VOICED = FALSE;
          if(VOICED || TRACE['v']) {
          /*
           * run actual analysis
           */
            powSpect = getSpectrum(gd);
	    numPeaks = findPeaks(powSpect, peak, gd);
	    if((numCands=sievePeaks(peak, numPeaks, cand, gd)) < 0) {
	      err = -1;
	      break;
	    }
	    if(numCands < 1)
	      VOICED = FALSE;
          }
	}
      }
    }
#ifndef WRASSP
    if(TRACE['v']) {
      fprintf(traceFP, "MAG=%5.0f  ZCR=%5.0f  RMS=%6.2f  AC1=%+4.2f "\
		       "NP=%2i  NC=%1i  Q[0]=%3i\n",\
	      MAG, ZCR, RMS, AC1, numPeaks, numCands, cand[0].Q);
    }
    if(TRACE['c']) {
      if(!TRACE['v'])
	fprintf(traceFP, "NC %1d", numCands);
      for(i = 0; i < numCands; i++) {
	if(!TRACE['v'] && cand[i].Q < gd->minQval)
	  break;
	fprintf(traceFP, "   F0 %5.1f  Q %3i", cand[i].F0, cand[i].Q);
      }
      fprintf(traceFP, "\n");
    }
#endif
    if(!VOICED) {
      for(n = 0; n < MHS_MAXTRACKS; n++)  /* no valid tracks possible */
	delTrack(&track[n]);
      if((err=pipeFrame(fn, &unv, pitDOp)) < 0)/* push unvoiced frame */
	break;
      if((err=flushPipe(pitDOp)) < 0) /* no pending tracks: may flush */
	break;
    }
    else if((err=trackPitch(fn, cand, pitDOp)) < 0)
      break;
  } /* END loop over frames */
  if(err >= 0) {
    err = flushPipe(pitDOp);
    if(err >= 0 && FILE_OUT)
      err = asspFFlush(pitDOp, gd->writeOpts);
  }
  freeGlobals();
  if(err < 0) {
    if(CREATED)
      freeDObj(pitDOp);
    return(NULL);
  }
  return(pitDOp);
}

/*DOC

Function 'freeMHS_GD'

Returns all memory allocated for the generic data in an MHS pitch data 
object.

DOC*/

void freeMHS_GD(void *generic)
{
  if(generic != NULL) {
    free(generic);
  }
  return;
}

/* ======================= private  functions ======================= */

/***********************************************************************
* as a function because we have to compute frameSize more than once    *
***********************************************************************/
LOCAL void setFrameSize(DOBJ *dop)
{
  MHS_GD *gd=(MHS_GD *)(dop->generic);

  /* given the fixed HAMMING window we need at least 2 periods */
  /* if other windows are allowed could use width of main lobe */
  gd->frameSize = 2 * FREQctPERIOD(gd->minF0, dop->sampFreq);
  if(gd->frameSize < 2 * dop->frameDur)
    gd->frameSize = 2 * dop->frameDur;        /* at least 50% overlap */
  return;
}

/***********************************************************************
* set local global values and allocate memory for the buffers          *
***********************************************************************/
LOCAL int setGlobals(DOBJ *dop)
{
  size_t  n;
  int     nd, wFlags;
  long    frameShift, numFrames;
  double  sampFreq, temp;
  MHS_GD *gd=(MHS_GD *)(dop->generic);

  sampFreq = dop->sampFreq;
  frameShift = dop->frameDur;
  numFrames = gd->endFrameNr - gd->begFrameNr;
  temp = FRMNRtoTIME(numFrames, sampFreq, frameShift);
  temp *= 1000.0;                                            /* in ms */
  if(numFrames <= 1 && temp < MHS_MINDURVS) {
    setAsspMsg(AEG_ERR_BUG, "setGlobals: analysis range too short");
    return(-1);
  }
  winShift = SMPNRtoTIME(frameShift, sampFreq);         /* in seconds */
  nd = numDecim(winShift, 12);
  snprintf(secFormat, sizeof(secFormat), "TIME %%%d.%df  ", nd+2+1, nd);     /* for TRACE */
/*
 * FFT and window parameters
 */
  numFFT = MIN_NFFT;             /* search appropriate FFT resolution */
  while(numFFT < gd->frameSize ||
	(sampFreq / (double)numFFT) > MHS_MINBINHZ)
    numFFT *= 2;
  wfGain = LINtodB(wfSpecs(gd->winFunc)->gain);
  wfHSLL = dBtoSQR(wfSpecs(gd->winFunc)->hsll);
/*
 * sieving parameters
 */
  meshWidth = MHS_MESHWIDTH;
  maxMesh = (size_t)floor(1.0 / meshWidth);      /* ensure no overlap */
  if(gd->options & MHS_OPT_POWER)
    meshWidth *= 0.75;           /* reduce mesh width but keep number */
  meshTol = 1.0 + meshWidth/2.0; /* tolerance (factor) to pass a mesh */
/*
 * parameters for masked spectrum and peak search
 */
  binFreq = sampFreq / (double)numFFT;                      /* Hz/bin */
  temp = gd->maxF0 * maxMesh * meshTol; /* highest frequency of interest */
  if(temp > MHS_MAXPKFREQ)
    temp = MHS_MAXPKFREQ;
  maxBin = (long)ceil(temp / binFreq);    /* corresponding bin number */
  if(maxBin > (numFFT/2 -1))
    maxBin = numFFT/2 -1;             /* one less for peak definition */
  temp = gd->minF0 / meshTol;       /* lowest frequency of interest */
  minBin = (long)floor(temp / binFreq);   /* corresponding bin number */
  if(minBin < 2)
    minBin = 2;                       /* one more for peak definition */
  minPeaks = MHS_MINPEAKS; /* as a variable in case we need to adjust */
/*
 * pitch tracking parameters
 */
  minF0Diff = st2rel(MHS_MINF0DIFF);            /* semitone => factor */
  temp = MHS_MAXDELTA * FRMNRtoTIME(1, sampFreq, frameShift);
  maxDelta = st2rel(temp) - 1.0;            /* ST/s => delta_F0/frame */
  if(maxDelta < minF0Diff - 1.0)                       /* bottom clip */
    maxDelta = minF0Diff - 1.0;
  maxNumTQ = TIMEtoFRMNR(MHS_MAXDURTQ/1000.0, sampFreq, frameShift);
  if(maxNumTQ < MHS_MINNUMTQ)
    maxNumTQ = MHS_MINNUMTQ;
  temp = ceil((1000.0 / gd->minF0) * MHS_MINPRDVS);  /* delay in ms */
  if(temp < MHS_MINDURVS)
    temp = MHS_MINDURVS;
  /* we need at least one frame more; let's take two */
  temp += (FRMNRtoTIME(2, sampFreq, frameShift) * 1000.0);
  if(temp < MHS_MINDELAY)
    temp = MHS_MINDELAY;
  temp /= 1000;                                         /* in seconds */
  pipeLength = TIMEctFRMNR(temp, sampFreq, frameShift);  /* in frames */
  pipeBegFn = pipeEndFn = gd->begFrameNr; 
/*
 * allocate memory
 */
  fftBuf = logN = wfc = NULL;                   /* clear all pointers */
  pipe = NULL;
  for(n = 0; n < MHS_MAXTRACKS; n++) {
    track[n].chain = NULL;
    track[n].status = MHS_TRK_UNUSED;
  }

  fftBuf = (double *)calloc((size_t)numFFT, sizeof(double));
  if(!(gd->options & MHS_OPT_POWER)) {
    logN = (double *)calloc((size_t)numFFT, sizeof(double));
    if(logN != NULL) {
      for(n = 1; n < numFFT; n++)
	logN[n] = log((double)n);
    }
  }
  /* for the power spectrum we use the 'proper' periodic window */
  wFlags = WF_PERIODIC;
  if((ODD(gd->frameSize) && EVEN(frameShift)) ||
     (EVEN(gd->frameSize) && ODD(frameShift)) )
    wFlags = WF_ASYMMETRIC;  /* align window centre with frame centre */
  wfc = makeWF(gd->winFunc, gd->frameSize, wFlags);
  pipe = (MHS_CAND *)calloc(pipeLength, sizeof(MHS_CAND));
/*
 * verify memory allocation
 */
  if(fftBuf == NULL || (!(gd->options & MHS_OPT_POWER) && logN == NULL) ||\
     wfc == NULL || pipe == NULL) {
    freeGlobals();
    setAsspMsg(AEG_ERR_MEM, "MHS: setGlobals");
    return(-1);
  }
  return(0);
}
/***********************************************************************
* free memory allocated for the global buffers                         *
***********************************************************************/
LOCAL void freeGlobals(void)
{
  size_t n;

  if(fftBuf != NULL) {
    free((void *)fftBuf);
    fftBuf = NULL;
  }
  if(logN != NULL) {
    free((void *)logN);
    logN = NULL;
  }
  freeWF(wfc);
  wfc= NULL;
  for(n = 0; n < MHS_MAXTRACKS; n++)
    delTrack(&track[n]);
  if(pipe != NULL) {
    free((void *)pipe);
    pipe = NULL;
  }
  return;
}
/**********************************************************************
* compute linear (masked) power spectrum                              *
**********************************************************************/
LOCAL double *getSpectrum(MHS_GD *gd)
{
  long    n, len, nc, ne, maxNc, maxNe;
  double  Lc, Le;
  double  logNc, LFslope, HFslope;
  double *logPow, *linPow;

  len = numFFT;
  mulSigWF(fftBuf, wfc, gd->frameSize);
  rfft(fftBuf, len, FFT_FORWARD);
  if(gd->options & MHS_OPT_POWER) {        /* omit masked spectrum */
    linPow = fftBuf;                          /* in-place conversion */
    rfftLinPow(fftBuf, linPow, len);
  }
  else {
/* NOTE:                                                             */
/*  o  unmasked spectrum in Bell (log SPL power) to avoid mul/div 10 */
/*  o  NEW: omitted SPL estimate and set HF-slope at a fixed value   */
/*         (used to be 108-Lc dB-SPL/oct)                            */
    logPow = fftBuf;                          /* in-place conversion */
    rfftLogPow(fftBuf, logPow, len);           /* log power spectrum */
    len /= 2;
    linPow = &fftBuf[len];                   /* 2nd half is now free */
    for(n = 0; n < len; n++)
      linPow[n] = 0.0;                        /* clear output buffer */
    LFslope = 12.0 / logN[2];                 /* fixed at 120 dB/oct */
    maxNc = (long)ceil(1.5 * maxBin);      /* highest component with */
    if(maxNc > len)              /* appreciable excitation at maxBin */
      maxNc = len;
/*    HFslope = 6.0 / logN[2];               NOW FIXED AT -60 dB/oct */
/*    HFslope = 6.6 / logN[2];               NOW FIXED AT -66 dB/oct */
    HFslope = 7.2 / logN[2];              /* NOW FIXED AT -72 dB/oct */
    maxNe = (long)ceil(1.25 * maxBin);       /* highest excition bin */
    if(maxNe > len)            /* avoiding artificial peak at maxBin */
      maxNe = len;
    for(nc = 1; nc < maxNc; nc++) {
      Lc = logPow[nc];
      if(Lc >= 1.0) {     /* RATHER ARBITRARY; NEEDS VERFICATION !!! */
	logNc = logN[nc];
	for(ne = nc-1; ne > 0; ne--) {                    /* LF-part */
	  if(ne < maxNe) {
	    Le = Lc - LFslope * (logNc - logN[ne]);
	    if(Le < 0.0)
	      break;
	    linPow[ne] += pow(10.0, Le);         /* add linear power */
	  }
	}
	linPow[nc] += pow(10.0, Lc);             /* component itself */
	for(ne = nc+1; ne < maxNe; ne++) {                /* HF-part */
	  Le = Lc - HFslope * (logN[ne] - logNc);
	  if(Le < 0.0)
	    break;
	  linPow[ne] += pow(10.0, Le);
	}
      }
    }
  }
  return(linPow);
}
/***********************************************************************
* search peaks in (masked) power spectrum                              *
***********************************************************************/
LOCAL int findPeaks(double *linPower, MHS_PEAK *peak, MHS_GD *gd)
{
  int    n, num, VALID;
  long   bin, len;
  double noise, threshold, maxBW;
  double relFreq, peakPower, scale, bandwidth;

  len = numFFT / 2;
  noise = linPower[0];
  for(bin = 1; bin < len; bin++) {              /* find highest level */
    if(linPower[bin] > noise)
      noise = linPower[bin];
  }
  noise *= wfHSLL;                           /* side lobe power level */
  noise *= 2.0;            /* add 3 dB to compensate for interference */
  if(noise < 1.0)
    noise = 1.0;                                       /* limit: 0 dB */

  for(num = 0, bin = minBin; bin < maxBin && num < MHS_MAXPEAKS; bin++) {
    if(linPower[bin] > noise && linPower[bin] > linPower[bin-1] &&\
       linPower[bin] >= linPower[bin+1]) {
      parabola(linPower[bin-1], linPower[bin], linPower[bin+1], binFreq,\
	       &relFreq, &peakPower, &scale);
      if(gd->options & MHS_OPT_POWER)
        threshold = peakPower / 2.818;  /* at least 4.5 dB above rest */
      else
/*      threshold = peakPower / 2.000;       at least 3 dB above rest */
/*      threshold = peakPower / 1.585;       at least 2 dB above rest */
        threshold = peakPower / 1.413;  /* at least 1.5 dB above rest */
/*      threshold = peakPower / 1.259;       at least 1 dB above rest */
      for(VALID = FALSE, n = bin-1; !VALID && n >= 0; n--) {
	if(linPower[n] <= threshold)
	  VALID = TRUE;
	else if(linPower[n] > linPower[n+1])
	  break;
      }
      if(VALID) {
	for(VALID = FALSE, n = bin+1; !VALID && n < len; n++) {
	  if(linPower[n] <= threshold)
	    VALID = TRUE;
	  else if(linPower[n] > linPower[n-1])
	    break;
	}
      }
      if(VALID) {
	peak[num].freq = bin * binFreq + relFreq;
	if(peak[num].freq < gd->minF0)
	  continue;
	peak[num].amp = SQRtodB(peakPower) - wfGain;
        if(gd->options & MHS_OPT_POWER) {
#ifndef WRASSP
	  if(TRACE['P']) {
	    fprintf(traceFP, "  F %.0f A %.1f",\
		    peak[num].freq, peak[num].amp);
	  }
#endif
	  num++;                         /* component always accepted */
	}
	else {                                     /* check bandwidth */
	  bandwidth = sqrt(-2.0 * peakPower / scale);        /* -3 dB */
#ifndef WRASSP
	  if(TRACE['P']) {
	    fprintf(traceFP, "  F %.0f A %.1f B %.1f",\
		    peak[num].freq, peak[num].amp, bandwidth);
	  }
#endif
	  maxBW = 0.25 * peak[num].freq;             /* maximally 25% */
	  if(maxBW < 125.0)
	    maxBW = 125.0;                     /* but at least 125 Hz */
	  if(bandwidth < maxBW)
	    num++;                              /* component accepted */
	}
      }
    }
  }
#ifndef WRASSP
  if(TRACE['P'] && num > 0)
    fprintf(traceFP, "\n");
#endif

  return(num);
}
/***********************************************************************
* pitch estimation using harmonic sieve                                *
***********************************************************************/
LOCAL int sievePeaks(MHS_PEAK *peak, int numPeaks, MHS_CAND *cand,\
		     MHS_GD *gd)
{
  int      n, i, meshNr, loMesh, hiMesh, numPass, numTest;
  int      newHN[MHS_MAXPEAKS];
  size_t   h, ns, numSieves;
  double   df, rdf, subHarm, estF0, peakFreq, meshFreq;
  double   avrDHN, curDHN, sumXN, sumNN;
  MHS_CAND new;

  if(numPeaks < minPeaks) {                /* insufficient components */
    /* could do something here for single strong peak etc. */
    return(0);                     /* candidate array cleared outside */
  }
  /*
   * Rather than using a constant number of sieve bases with a fixed 
   * step between them as in DWS and MDWS, we generate sub-harmonics 
   * of the detected peaks as candidate F0s. These may be slightly 
   * outside the range of F0 values allowed because of peak shifts.
   * We also add some intermediate F0s because it was found in MDWS 
   * that the optimal fit could sometimes just be missed.
   */
  df = 1.0 + meshWidth / 3.0;
  rdf = sqrt(df);
  for(numSieves = 0, n = 0; n < numPeaks && numSieves < MHS_MAXSIEVES; n++) {
    for(h = 1; h <= MHS_MAXSUBS && numSieves < MHS_MAXSIEVES; h++) {
      subHarm = peak[n].freq / (double)h;
      if(subHarm <= gd->minF0 / rdf)
	break;            /* below range; no need to search further */
      if(subHarm >= gd->maxF0 * rdf)
	continue;               /* above range; ignore and try next */
      sieveF0[numSieves++] = subHarm;
      if(numSieves < MHS_MAXSIEVES && subHarm > gd->minF0)
	sieveF0[numSieves++] = subHarm / df;
      if(numSieves < MHS_MAXSIEVES && subHarm < gd->maxF0)
	sieveF0[numSieves++] = subHarm * df;
    }
  }
#ifndef WRASSP
  if(TRACE['s']) {
    fprintf(traceFP, "NS = %zd\n", numSieves);
  }
#endif

  for(ns = 0; ns < numSieves; ns++) {
    estF0 = sieveF0[ns];
    loMesh = hiMesh = numPass = 0;
    for(n = 0; n < numPeaks; n++) {
      peakFreq = peak[n].freq;
      newHN[n] = 0;                              /* set to not passed */
      meshNr = (int)myrint(peakFreq / estF0);
      if(meshNr < 1)
	continue;
      if(meshNr > maxMesh)              /* reached open part of sieve */
	break;                   /* ignore this and higher components */
      meshFreq = meshNr * estF0;          /* centre frequency of mesh */
      if(peakFreq >= meshFreq / meshTol &&
	 peakFreq <= meshFreq * meshTol) {               /* fits mesh */
	if(loMesh <= 0)
	  loMesh = hiMesh = meshNr;          /* keep lowest mesh used */
	if(numPass > 0 && newHN[n-1] == meshNr) { /* two in same mesh */
	  if(fabs(peakFreq - meshFreq) <=\
	     fabs(peak[n-1].freq - meshFreq)) {     /* current closer */
	    newHN[n-1] = 0;                         /* clear previous */
	    newHN[n] = meshNr;                    /* classify current */
	  }
       /* else: ignore current */
	}
	else {
	  if(numPass >= minPeaks) {
          /* check for spurious high harmonics */
            if(numPass > 4 && numPass >= (n-1) &&
               (hiMesh-loMesh) == (numPass-1)) {
                       /* if we have more than 4 successive harmonics */
              if(meshNr > hiMesh+1)   /* don't let a missing harmonic */
                break;              /* spoil the quality-of-fit value */
            } 
	    avrDHN = (double)(hiMesh-loMesh+1) / (double)numPass;
	    curDHN = (double)(meshNr-hiMesh);
	    if(curDHN > 3.0 * avrDHN)                /* gap too large */
	      break;             /* ignore this and higher components */
	  }
	  hiMesh = meshNr;                  /* keep highest mesh used */
	  newHN[n] = meshNr;         /* keep harmonic number assigned */
	  numPass++;
	}
      }
    }
    if(numPass >= minPeaks) {
      numTest = n;          /* excludes components above highest mesh */
      sumXN = sumNN = 0.0;    /* maximum likelihood estimate of pitch */
      for(n = 0; n < numTest; n++) {
	if(newHN[n] > 0) {                  /* classified as harmonic */
	  sumXN += (peak[n].freq * (double)newHN[n]);
	  sumNN += (double)(newHN[n] * newHN[n]);
	}
      }
      new.F0 = sumXN / sumNN;
      if(new.F0 >= gd->minF0 && new.F0 <= gd->maxF0) {
	new.Q = (int)myrint(MHS_Q_SCALE * (double)(2 * numPass) /\
			    (double)(hiMesh + numTest));
	for(i = 0; i < MHS_MAXCANDS; i++) {
	  if(cand[i].Q <= 0)                     /* no more candidates */
	    break;
	  if(new.F0 < (cand[i].F0 * minF0Diff) &&
	     new.F0 > (cand[i].F0 / minF0Diff) ) {      /* about equal */
	    if(new.Q > cand[i].Q)              /* new candidate better */
	      rmvElement(cand, MHS_MAXCANDS, sizeof(MHS_CAND), i, &unv);
	    else {                            /* discard new candidate */
	      new.F0 = 0.0;
	      new.Q = 0;
	    }
	    break;
	  }
	}
	if(new.Q > 0) {                              /* not discarded */
	  for(i = 0; i < MHS_MAXCANDS; i++)   /* sort on decreasing Q */
	    if(new.Q > cand[i].Q)
	      break;
	  if(i < MHS_MAXCANDS)
	    insElement(cand, MHS_MAXCANDS, sizeof(MHS_CAND), i, &new);
	}
      }
    }
  }

  for(i = 0; i < MHS_MAXCANDS; i++) {
    if(cand[i].Q <= 0)
      break;
  }
  return(i);                       /* return number of top candidates */
}
/***********************************************************************
* perform - fairly simple - pitch tracking                             *
***********************************************************************/
LOCAL int trackPitch(long frameNr, MHS_CAND *cand, DOBJ *dop)
{
  int     i, n, topQ, bestN, bestQ;
  int     PUSHED, PENDING, active;
  int     numCands, used[MHS_MAXCANDS];
  double  bestTQ, prevF0, bestD, delta;
  MHS_GD *gd=(MHS_GD *)(dop->generic);

  topQ = (int)myrint(MHS_REL_TOPQ * (double)cand[0].Q);
  if(topQ < gd->minQval)
    topQ = gd->minQval;
  for(numCands = 0; numCands < MHS_MAXCANDS; numCands++) {
    used[numCands] = FALSE;    /* candidate may only be appended once */
    if(cand[numCands].Q < topQ)              /* select top candidates */
      break;
  }
  if(numCands <= 0) {                                     /* UNVOICED */
    for(n = 0; n < MHS_MAXTRACKS; n++)       /* can delete all tracks */
      delTrack(&track[n]);
    if(pipeFrame(frameNr, &unv, dop) < 0)   /* push an unvoiced frame */
      return(-1);
    return(flushPipe(dop));           /* no pending tracks: may flush */
  }

  active = -1;
  for(n = 0; n < MHS_MAXTRACKS; n++) {
    track[n].status &= ~MHS_TRK_UPDATED;    /* clear update flags and */
    if(track[n].status & MHS_TRK_ACTIVE)       /* check whether there */
      active = n;                               /* is an active track */
  }
  PUSHED = FALSE;          /* must know whether data have been pushed */
  if(active >= 0) {                     /* give active track priority */
    prevF0 = track[active].chain->cand.F0;
    bestD = prevF0 * maxDelta;     /* best candidate also a valid one */
    bestQ = 0;  /* give priority to matching candidate with highest Q */
    bestN = -1;                          /* haven't yet found a match */
    for(n = 0; n < numCands; n++) {
      delta = fabs(cand[n].F0 - prevF0);
      if(delta < bestD && cand[n].Q >= bestQ) {
	bestQ = cand[n].Q;
	bestD = delta;
	bestN = n;
      }
    }
    if(bestN >= 0) {                      /* matching candidate found */
      if(addLink(&track[active], &cand[bestN]) < 0)
	return(-1);
      if(pipeFrame(frameNr, &cand[bestN], dop) < 0)        /* push it */
	return(-1);
      used[bestN] = TRUE;                   /* mark candidate as used */
      PUSHED = TRUE;                       /* and data pushed to pipe */
    }
    else {                                     /* track not continued */
      delTrack(&track[active]);                          /* remove it */
      active = -1;
    }
  }
  PENDING = FALSE;                               /* clear global flag */
  for(i = 0; i < MHS_MAXTRACKS; i++) {  /* now check all other tracks */
    if(i != active && (track[i].status & MHS_TRK_IN_USE)) {
      prevF0 = track[i].chain->cand.F0;
      bestD = prevF0 * maxDelta;
      bestN = -1;
      for(n = 0; n < numCands; n++) {
	if(!used[n]) {
	  delta = fabs(cand[n].F0 - prevF0);
	  if(delta < bestD) {
	    bestD = delta;
	    bestN = n;
	  }
	}
      }
      if(bestN >= 0) {
	if(addLink(&track[i], &cand[bestN]) < 0)
	  return(-1);
	used[bestN] = TRUE;
	if(!(track[i].status & MHS_TRK_PENDING) ) {/* check durations */
	  if(track[i].duration >= MHS_MINDURVS &&
	     track[i].periods >= MHS_MINPRDVS) {
	    track[i].status |= MHS_TRK_PENDING;  /* mark as potential */
	    PENDING = TRUE;                        /* set global flag */
	  }
	}
	else
	  PENDING = TRUE;                           /* already marked */
      }
      else                                           /* not continued */
	delTrack(&track[i]);                             /* remove it */
    }
  }
  if(active < 0 && PENDING) { /* select new active track from pending */
    bestTQ = 0.0;
    bestN = -1;
    for(i = 0; i < MHS_MAXTRACKS; i++) {
      if(track[i].status & MHS_TRK_PENDING) {
	if(track[i].trackQ > bestTQ) {
	  bestTQ = track[i].trackQ;
	  bestN = i;
	}
      }
    }
    if(bestN >= 0) {                     /* should always be the case */
      if(pipeTrack(frameNr, &track[bestN], dop) < 0) /* push all F0's */
	return(-1);
      PUSHED = TRUE;
      track[bestN].status &= ~MHS_TRK_PENDING;  /* clear pending flag */
      track[bestN].status |= MHS_TRK_ACTIVE;       /* set active flag */
      active = bestN;
    }
    else {
      setAsspMsg(AEG_ERR_BUG, "trackPitch: didn't find pending track");
      return(-1);
    }
  }
  for(n = 0; n < numCands; n++) {      /* finally, create a new track */
    if(!used[n]) {                       /* for each unused candidate */
      for(i = 0; i < MHS_MAXTRACKS; i++) {
	if(!(track[i].status & MHS_TRK_IN_USE) ) {
	  if(addLink(&track[i], &cand[n]) < 0)
	    return(-1);
	  break;
	}
      }
    }
  }
  if(!PUSHED) {                           /* no track has pushed data */
    if(pipeFrame(frameNr, &unv, dop) < 0)   /* push an unvoiced frame */
      return(-1);
  }

#ifndef WRASSP
  if(TRACE['t']) {
    if(active >= 0)
      fprintf(traceFP, "* F0 = %.1f  Q = %i  TQ = %.1f  dur = %.1f\n",\
	      track[active].chain->cand.F0, track[active].chain->cand.Q,\
	      track[active].trackQ, track[active].duration);
    for(i = 0; i < MHS_MAXTRACKS; i++) {
      if(i != active && (track[i].status & MHS_TRK_IN_USE))
	fprintf(traceFP, "  F0 = %.1f  Q = %i  TQ = %.1f  dur = %.1f\n",\
		track[i].chain->cand.F0, track[i].chain->cand.Q,\
		track[i].trackQ, track[i].duration);
    }
  }
#endif
  return(0);
}
/***********************************************************************
* add a link to the chain and update track parameters and status       *
***********************************************************************/
LOCAL int addLink(MHS_TRK *tPtr, MHS_CAND *cand)
{
  size_t    num;
  double    sum, Q;
  MHS_LINK *lPtr;

  if(tPtr->status & MHS_TRK_IN_USE) {                 /* check length */
    lPtr = tPtr->chain;
    if(lPtr == NULL) {
      setAsspMsg(AEG_ERR_BUG, "addLink: chain has no links");
      return(-1);
    }
    for(num = 1; lPtr->prev != NULL; num++)            /* count links */
      lPtr = lPtr->prev;             /* while rewinding to first link */
    if(num >= pipeLength) {                      /* remove first link */
      if(lPtr->next != NULL)             /* should always be the case */
	lPtr->next->prev = NULL;                /* new begin of chain */
      free((void *)lPtr);                            /* return memory */
    }
  }
  lPtr = (MHS_LINK *)malloc(sizeof(MHS_LINK));
  if(lPtr == NULL) {
    setAsspMsg(AEG_ERR_MEM, "(addLink)");
    return(-1);
  }
  lPtr->cand.F0 = cand->F0;
  lPtr->cand.Q = cand->Q;
  lPtr->next = NULL;                              /* new end of chain */
  lPtr->prev = tPtr->chain;                  /* previous end of chain */
  if(lPtr->prev != NULL)                          /* existing chain ? */
    lPtr->prev->next = lPtr;                               /* link up */
  tPtr->chain = lPtr;                /* chain points to the last link */
  tPtr->duration = 0.0;
  tPtr->periods = 0.0;
  tPtr->trackQ = 0.0;
  sum = 0.0;
  num = 0;
  while(lPtr != NULL) {               /* recalculate track parameters */
    tPtr->periods += (winShift * lPtr->cand.F0); /* number of periods */
    if(num < maxNumTQ) {
/*      sum += (double)(lPtr->cand.Q); */                 /* sum Q values */
      Q = (double)(lPtr->cand.Q);
      sum += (Q * Q);                         /* sum squared Q values */
    }
    num++;                                        /* number of frames */
    lPtr = lPtr->prev;
  }
  tPtr->duration = (winShift * 1000.0 * num);       /* duration in ms */
  if(num > maxNumTQ)
    num = maxNumTQ;
/*  tPtr->trackQ = sum / (double)num; */               /* average Q value */
  tPtr->trackQ = sqrt(sum/(double)num);            /* RMS of Q values */
  tPtr->status |= (MHS_TRK_IN_USE | MHS_TRK_UPDATED);    /* set flags */
  return(0);
}
/***********************************************************************
* push all candidate data of a track into the pipe                     *
* ToDo: Existing data in pipe should only be overwritten starting from *
*       the frame which has a higher Q-value than the one in the pipe. *     
***********************************************************************/
LOCAL int pipeTrack(long frameNr, MHS_TRK *tPtr, DOBJ *dop)
{
  MHS_LINK *lPtr;

  if(!(tPtr->status & MHS_TRK_IN_USE) || tPtr->chain == NULL) {
    setAsspMsg(AEG_ERR_BUG, "pipeTrack: invalid track");
    return(-1);
  }
  lPtr = tPtr->chain;
  while(lPtr->prev != NULL) { /* rewind: oldest value to be pushed first */
    lPtr = lPtr->prev;
    frameNr--;
  }
  while(lPtr != NULL) {
    if(pipeFrame(frameNr, &(lPtr->cand), dop) < 0)
      return(-1);
    lPtr = lPtr->next;
    frameNr++;
  }

/*   int ndx, IGNORE = TRUE; */
/*   while(lPtr != NULL) { */
/*     if(IGNORE && frameNr >= pipeBegFn && frameNr < pipeEndFn) { */
/*       ndx = (int)(frameNr - pipeBegFn); */
/*       if(pipe[ndx].Q < lPtr->cand.Q) */
/* 	IGNORE = FALSE; */
/*     } */
/*     else */
/*       IGNORE = FALSE; */
/*     if(!IGNORE) { */
/*       if(pipeFrame(frameNr, &(lPtr->cand), dop) < 0) */
/* 	return(-1); */
/*     } */
/*     lPtr = lPtr->next; */
/*     frameNr++; */
/*   } */

  return(0);
}
/***********************************************************************
* delete a complete F0 track                                           *
***********************************************************************/
LOCAL void delTrack(MHS_TRK *tPtr)
{
  MHS_LINK *lPtr, *prev;

  lPtr = tPtr->chain;
  while(lPtr != NULL) {
    prev = lPtr->prev;                      /* rewind to first link */
    free((void *)lPtr);                     /* while deleting links */
    lPtr = prev;
  }
  tPtr->chain = NULL;
  tPtr->duration = 0.0;
  tPtr->periods = 0.0;
  tPtr->trackQ = 0.0;
  tPtr->status = MHS_TRK_UNUSED;
  return;
}
/***********************************************************************
* put frame data into pipe (delay line); if pipe full, shift oldest    *
* F0 value to output buffer before adding new data                     *
* NOTE: new data may overwrite existing data in pipe                   *
***********************************************************************/
LOCAL int pipeFrame(long frameNr, MHS_CAND *cand, DOBJ *dop)
{
  size_t numBytes;
  int    ndx;

  ndx = (int)(frameNr - pipeBegFn);
  if(ndx < 0) {
    setAsspMsg(AWG_WARN_BUG, "pipeFrame: frame before begin of pipe");
    return(1);
  }
  if(frameNr > pipeEndFn || ndx > pipeLength) {
    setAsspMsg(AEG_ERR_BUG, "pipeFrame: frame outside pipe");
    return(-1);
  }
  if(ndx == pipeLength) {                              /* make place */
    if(storeMHS((float)(pipe[0].F0), pipeBegFn, dop) < 0)
      return(-1);
    pipeBegFn++;
    ndx--;
    numBytes = (pipeLength - 1) * sizeof(MHS_CAND);
    memmove(pipe, &pipe[1], numBytes);
  }
  pipe[ndx].F0 = cand->F0;
  pipe[ndx].Q = cand->Q;
  if(pipeEndFn <= frameNr)
    pipeEndFn = frameNr + 1;
  return(0);
}
/***********************************************************************
* shift contents of pipe to output buffer                              *
***********************************************************************/
LOCAL int flushPipe(DOBJ *dop)
{
  int n;

  for(n = 0; pipeBegFn < pipeEndFn; n++, pipeBegFn++) {
    if(storeMHS((float)(pipe[n].F0), pipeBegFn, dop) < 0)
      return(-1);
  }
  return(0);
}
/***********************************************************************
* copy frame data to output buffer; handle data writes                 *
***********************************************************************/
LOCAL int storeMHS(float val, long frameNr, DOBJ *dop)
{
  int     FILE_OUT;
  long    ndx;
  float  *fPtr;
  MHS_GD *gd=(MHS_GD *)(dop->generic);

  FILE_OUT = (dop->fp != NULL);
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
      setAsspMsg(AEG_ERR_BUG, "storeMHS: buffer overflow");
      return(-1);
    }
  }
  ndx = frameNr - dop->bufStartRec;
  fPtr = (float *)(dop->dataBuffer);
  fPtr[ndx] = val;
  if(ndx >= dop->bufNumRecs)
    dop->bufNumRecs = ndx + 1;
  dop->bufNeedsSave = TRUE;
  return(0);
}

/***********************************************************************
* insert an element in a linear array                                  *
***********************************************************************/
LOCAL int insElement(void *array, size_t numElements, size_t elementSize,\
		     size_t index, void *element)
{
  char *srcPtr, *dstPtr;
  size_t  numBytes;

  if(index >= numElements || array == NULL ||\
     element == NULL || elementSize == 0)
    return(-1);
  if(index < (numElements-1)) {     /* shift elements to create space */
    srcPtr = (char *)array + index*elementSize;
    dstPtr = srcPtr + elementSize;
    numBytes = (numElements-1 - index)*elementSize;
    memmove(dstPtr, srcPtr, numBytes);
  }
  dstPtr = (char *)array + index*elementSize;           /* insert new element */
  memcpy(dstPtr, element, elementSize);
  return(0);
}
/***********************************************************************
* remove an element from a linear array and append 'fill' if non-NULL  *
***********************************************************************/
LOCAL int rmvElement(void *array, size_t numElements, size_t elementSize,\
		     size_t index, void *fill)
{
  char   *srcPtr, *dstPtr;
  size_t  numBytes;

  if(index >= numElements || array == NULL || elementSize == 0)
    return(-1);
  if(index < (numElements-1)) {          /* move elements to fill gap */
    dstPtr = (char *)array + index*elementSize;
    srcPtr = dstPtr + elementSize;
    numBytes = (numElements-1 - index)*elementSize;
    memmove(dstPtr, srcPtr, numBytes);
  }
  if(fill != NULL) {                    /* set filler at end of array */
    dstPtr = (char *)array + (numElements-1)*elementSize;
    memcpy(dstPtr, fill, elementSize);
  }
  return(0);
}
