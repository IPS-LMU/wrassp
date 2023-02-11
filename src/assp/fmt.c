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
* File:     fmt.c                                                      *
* Contents: Functions implementing the formant estimator 'forest'      *
*           based on root-solving of the LP polynomial followed by     *
*           formant classification using the Pisarenko frequencies     *
*           from the Split-Levinson-Algorithm and dynamic programming. *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: fmt.c,v 1.10 2012/03/19 16:16:38 lasselasse Exp $ */

#include <stdio.h>    /* FILE NULL etc. */
#include <stdlib.h>   /* strtol() strtod() calloc() */
#include <stddef.h>   /* size_t */
#include <string.h>   /* str..() */
#include <ctype.h>    /* isspace() isdigit() */
#include <inttypes.h> /* int8_t int16_t */
#include <math.h>     /* fabs() log10() exp() pow() */
#include <float.h>    /* DBL_EPSILON */

#include <miscdefs.h> /* TRUE FALSE LOCAL NATIVE_EOL PI TWO_PI */
#include <misc.h>     /* strxcmp() STAT and statistical functions */
#include <trace.h>    /* trace handler */
#include <asspmess.h> /* message handler */
#include <assptime.h> /* standard conversion macros */
#include <asspana.h>  /* AOPTS anaTiming() (includes fmt.h) */
#include <asspdsp.h>  /* windows getACF() lpSLA() BAIRSTOW ... */
#include <asspfio.h>  /* file handler */
#include <dataobj.h>  /* DOBJ getSmpCaps() getSmpFrame() */
#include <headers.h>  /* KDTAB */
#include <aucheck.h>  /* checkSound() */

/*
 * export variables
 */
STAT statPF, statPQ, statP[FMT_NUM_PSTATS], statF[FMT_NUM_FSTATS];
unsigned long totFMTfiles, totFMTframes, totFMTsilent, totFMTfail;

/*
 * local global variables and arrays
 */
LOCAL char trgepFormat[64], fpbFormat[32];

LOCAL double *rmsBuf=NULL; /* buffer for RMS calculation (allocated) */
LOCAL double *frame=NULL;  /* frame buffer incl. leading sample (allocated) */
LOCAL double *wfc=NULL;    /* window function coefficients (allocated) */
LOCAL double  wfGain=1.0;  /* gain of window function (linear) */

/* fixed size arrays */
LOCAL double refFreq[MAXFORMANTS];
typedef struct formant_limits {
  double min;  /* absolute lowest frequency */
  double pLo;  /* lowest frequency of non-overlapping range */
  double nom;  /* nominal or measured median frequency */
  double pHi;  /* highest frequency of non-overlapping range */
  double max;  /* absolute highest frequency */
} FMTLIMS;
LOCAL FMTLIMS limits[FMT_MAX_BUF];

typedef struct formant_data {
  double RMS;               /* RMS amplitude (dB) */
  double LP1;               /* coefficient of 1st order LP */
  double gain;              /* gain (RMS of prediction error in dB) */
  double prob;              /* end probability of mapping */
  double rf[FMT_MAX_BUF];   /* resonance frequencies */
  double bw[FMT_MAX_BUF];   /* resonance bandwidths */
  double pf[FMT_MAX_BUF];   /* pisarenko frequencies */
  int8_t slot[FMT_MAX_BUF]; /* formant slot (count starts at 0) */
  int8_t lock[FMT_MAX_BUF]; /* indicator: formant number fixed */
} FMTDATA;
LOCAL FMTDATA sortBuf;
/* LOCAL long sortBufBfn, sortBufEfn; */ /* no tracking over time yet */

typedef struct dynamic_programming_values {
  double pc[FMT_MAX_BUF]; /* (conditional) probabilities R = Fn */
  int    bt[FMT_MAX_BUF]; /* back trace */
} FMT_DP;
LOCAL FMT_DP dp[FMT_MAX_BUF];

/* #define TP_FACTOR 0.75 */  /* factor for transition probabilities */
/* #define TP_FACTOR sqrt(0.5) */    /* NEW in R2.0 */
#define TP_FACTOR 0.5         /* NEW in R2.0 */
LOCAL double tp[FMT_MAX_BUF]; /* transition probabilities */

LOCAL BAIRSTOW term;          /* termination criteria for bairstow() */

/*
 * prototypes of private functions
 */
LOCAL void setOrder(FMT_GD *gd, AOPTS *aoPtr, double sampFreq);
LOCAL int  setPreEmph(FMT_GD *gd, AOPTS *aoPtr, double sampFreq);
LOCAL int  setGlobals(DOBJ *dop);
LOCAL void freeGlobals(void);
LOCAL void setRefTables(double F1);
LOCAL void nomFData(FMTDATA *fPtr, int N);
LOCAL void pqStart(double *freq, double *pqp, int N, double sampFreq);
LOCAL int  sortPQ(double *pqp, int N);
LOCAL int  classFmt(long frameNr, FMTDATA *fPtr, int numFmt, DOBJ *dop);
LOCAL int  cleanRsn(FMTDATA *fPtr, int numFmt, DOBJ *dop);
LOCAL int  probRiFn(FMTDATA *fPtr, int numFmt, FMT_GD *gd, double probEq);
LOCAL int  getNumRsn(FMTDATA *fPtr);
LOCAL int  tryMerge(FMTDATA *fPtr, int n, double maxBW, double maxDist);
LOCAL void mergeFmt(FMTDATA *fPtr, int fmtNr);
LOCAL void shiftFmt(FMTDATA *fPtr, int fmtNr, int direction);
LOCAL void putDummy(FMTDATA *fPtr, int fmtNr);
LOCAL void fillGaps(FMTDATA *fPtr, int numFmt, double sampFreq);
LOCAL int  storeFMT(FMTDATA *fPtr, long frameNr, DOBJ *dop);
LOCAL int  printRaw(FMTDATA *fPtr, long frameNr, int numFmt, DOBJ *dop);

/* ======================== public functions ======================== */

/*DOC

Function 'printFMTrefs'

Prints the references for the algorithms on which 'forest' is based and 
a reference for the earlier version named 'klara'.

DOC*/

void printFMTrefs(void)
{
#ifndef WRASSP
  printf("\nReferences:\n");
  printf("Delsarte, P. and Genin, Y.V. (1986). \"The Split Levinson\n");
  printf("   Algorithm,\" IEEE Trans. ASSP, Vol. 34, 470-478.\n");
  printf("Willems, L.F. (1987), \"Robust formant analysis for speech\n");
  printf("   synthesis applications,\" Proc. European Conference on Speech\n");
  printf("   Technology, Vol. 1, 250-253.\n");
  printf("Scheffers, M. and Simpson, A. (1995). \"LACS: Label Assisted Copy\n");
  printf("   Synthesis, \" Proc. XIIIth Int. Congress of Phonetic Sciences\n");
  printf("   Vol. 2, 346-349.\n");
#endif
  return;
}

/*DOC

Function 'setFMTgenderDefaults'

Sets the items 'gender', 'msSize' and 'nomF1' in the analysis options 
structure pointed to by "aoPtr" to the default values for "gender".
Returns 0 upon success and -1 upon error.

DOC*/

int setFMTgenderDefaults(AOPTS *aoPtr, char gender)
{
  if(aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setFMTgenderDefaults");
    return(-1);
  }
  switch(gender) {
  case 'F': /* female */
  case 'f':
    aoPtr->msSize = FMT_DEF_EFFLENf;
    aoPtr->nomF1 = FMT_DEF_NOMF1f;
    break;
  case 'M': /* male */
  case 'm':
  case 'U': /* unknown */
  case 'u':
    aoPtr->msSize = FMT_DEF_EFFLENm;
    aoPtr->nomF1 = FMT_DEF_NOMF1m;
    break;
  default:
    setAsspMsg(AEG_ERR_BUG, NULL);
    snprintf(applMessage, sizeof(applMessage), "setFMTgenderDefaults: invalid gender code '%c'",\
	    gender);
    return(-1);
  }
  aoPtr->gender = tolower((int)gender);
  aoPtr->options |= AOPT_EFFECTIVE;   /* p.d. effective window length */
  return(0);
}

/*DOC

Function 'setFMTdefaults'

Sets the items in the analysis options structure relevant to formant 
estimation to their default values. Clears all other items.
Returns 0 upon success and -1 upon error.

DOC*/

int setFMTdefaults(AOPTS *aoPtr)
{
  if(aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setFMTdefaults");
    return(-1);
  }
  memset((void *)aoPtr, 0, sizeof(AOPTS));         /* clear all items */
  aoPtr->msShift = FMT_DEF_SHIFT;                 /* frame shift (ms) */
  aoPtr->preEmph = FMT_DEF_FIXEMPH;        /* pre-emphasis (if fixed) */
  aoPtr->order = FMT_DEF_ORDER;                     /* analysis order */
  aoPtr->increment = FMT_DEF_INCR;       /* increment/decrement order */
  aoPtr->numFormants = FMT_DEF_OUT;      /* number of output formants */
  aoPtr->threshold = FMT_DEF_SILENCE;            /* silence threshold */
  aoPtr->channel = FMT_DEF_CHANNEL;         /* channel to be analysed */
  aoPtr->accuracy = FMT_DEF_DIGITSA;     /* digits accuracy for ASCII */
  strcpy(aoPtr->format, FMT_DEF_FORMAT);        /* output file format */
  strcpy(aoPtr->winFunc, FMT_DEF_WINDOW);          /* window function */
  /* gender-specific defaults for msSize and nomF1 */
  if(setFMTgenderDefaults(aoPtr, FMT_DEF_GENDER) < 0)
    return(-1);
  return(0);
}

/*DOC

Function 'createFMT'

Allocates memory for a data object to hold formant estimates and 
initializes it on the basis of the information in the audio object 
pointed to by "smpDOp" and the analysis options in the structure pointed 
to by "aoPtr". Neither of these pointers may be NULL.
Returns a pointer to the data object or NULL upon error.

Note:
 - This function DOES NOT allocate memory for the data buffer. Depending 
   on your application you can either use 'allocDataBuf' for this or 
   delegate it to 'computeFMT'.

DOC*/

DOBJ *createFMT(DOBJ *smpDOp, AOPTS *aoPtr)
{
  int     N;
  long    auCaps;
  ATIME   aTime, *tPtr;
  FMT_GD *gd=NULL;
  DOBJ   *dop=NULL;
  DDESC  *dd=NULL;
  KDTAB  *entry;

  if(smpDOp == NULL || aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "createFMT");
    return(NULL);
  }
  /* verify audio object */
  if((auCaps=getSmpCaps(FMT_PFORMAT)) <= 0)
    return(NULL);
  auCaps |= FMT_I_CHANS;
  if(aoPtr->channel < 1)
    aoPtr->channel = 1;
  if(checkSound(smpDOp, auCaps, aoPtr->channel) <= 0)
    return(NULL);
  tPtr = &aTime;
  if(anaTiming(smpDOp, aoPtr, tPtr) < 0)
    return(NULL);
  clrAsspMsg();                               /* ignore warnings here */
  if((gd=(FMT_GD *)malloc(sizeof(FMT_GD))) == NULL) {
    setAsspMsg(AEG_ERR_MEM, "(createFMT)");
    return(NULL);
  }
  strcpy(gd->ident, FMT_GD_IDENT);
  gd->options = aoPtr->options;
  gd->frameSize = tPtr->frameSize;
  gd->begFrameNr = tPtr->begFrameNr;
  gd->endFrameNr = tPtr->endFrameNr;
  gd->nomF1 = aoPtr->nomF1;
  if(setPreEmph(gd, aoPtr, smpDOp->sampFreq) < 0) {
     freeFMT_GD((void *)gd);
     strcat(applMessage, " (createFMT)");
     return(NULL);
  }
  setOrder(gd, aoPtr, smpDOp->sampFreq);
  if((gd->lpOrder + 1) >= gd->frameSize) {
    freeFMT_GD((void *)gd);
    setAsspMsg(AED_ERR_SIZE, "(createFMT)");
    return(NULL);
  }
  if(aoPtr->numFormants < 1 || aoPtr->numFormants > FMT_MAX_OUT) {
    freeFMT_GD((void *)gd);
    setAsspMsg(AEG_ERR_BUG, "createFMT: invalid number of formants");
    return(NULL);
  }
  gd->numFormants = aoPtr->numFormants;
  N = gd->lpOrder / 2;                  /* maximum number of formants */
  if(gd->numFormants > N)
    gd->numFormants = N;
  gd->rmsSil = aoPtr->threshold;
  gd->winFunc = wfType(aoPtr->winFunc);
  if(gd->winFunc <= WF_NONE) {
    freeFMT_GD((void *)gd);
    setAsspMsg(AEB_BAD_WIN, aoPtr->winFunc);
    return(NULL);
  }
  gd->channel = aoPtr->channel;
  gd->accuracy = aoPtr->accuracy;

  if((dop=allocDObj()) == NULL) {
    freeFMT_GD((void *)gd);
    strcpy(applMessage, "(createFMT)");
    return(NULL);
  }
  dd = addDDesc(dop);   /* one for frequencies and one for bandwidths */
  if(dd != NULL && gd->options & FMT_OPT_PE_ADAPT)
    dd = addDDesc(dop);            /* another one for LP1 coefficient */
  if(dd == NULL) {
    freeFMT_GD((void *)gd);
    dop = freeDObj(dop);
    strcpy(applMessage, "(createFMT)");
    return(dop);
  }
  if(strxcmp(aoPtr->format, "SSFF") == 0) {
    dop->fileFormat = FF_SSFF;
    dop->fileData = FDF_BIN;
    strcpy(dop->eol, SSFF_EOL_STR);
  }
  else { /* fall through to default and let user adjust */
    dop->fileFormat = FF_RAW;
    dop->fileData = FDF_ASC;
    strcpy(dop->sepChars, "\t");                    /* between fields */
    strcpy(dop->eol, NATIVE_EOL);
  }
  SETENDIAN(dop->fileEndian);
  dop->sampFreq = tPtr->sampFreq;
  dop->frameDur = tPtr->frameShift;
  dop->startRecord = gd->begFrameNr;
  dop->numRecords = 0;                         /* nothing written yet */
  dop->generic = (void *)gd;
  dop->doFreeGeneric = (DOfreeFunc)freeFMT_GD;

  dd = &(dop->ddl);           /* reset pointer to 1st data descriptor */
  if(gd->options & FMT_OPT_PE_ADAPT) {
    dd->type = DT_LP1;
    dd->coding = DC_LIN;
    dd->format = FMT_PEFORMAT;
    dd->numFields = 1;
    if(dop->fileFormat == FF_SSFF) {
      entry = dtype2entry(dd->type, KDT_SSFF); /* search SSFF keyword */
      if(entry != NULL && entry->keyword != NULL) {
	dd->ident = strdup(entry->keyword);
	if(entry->factor != NULL)
	  strcpy(dd->factor, entry->factor);
	if(entry->unit != NULL)
	  strcpy(dd->unit, entry->unit);
      }
      else {
	dop = freeDObj(dop);
	setAsspMsg(AEB_ERR_TRACK, "for data type LP1 (createFMT)");
	return(dop);
      }
    }
    else {
      dd->ident = strdup("LP1");
      strcpy(dd->sepChars, " ");                      /* within field */
      snprintf(dd->ascFormat, member_size(DDESC, ascFormat), "%%+.%de", gd->accuracy);
    }
    dd = dd->next;              /* set pointer to 2nd data descriptor */
  }

  dd->type = DT_FFR;
  strcpy(dd->unit, "Hz");
  dd->coding = DC_LIN;
  dd->format = FMT_FBFORMAT;
  dd->numFields = gd->numFormants;
  if(dop->fileFormat == FF_SSFF) {
    entry = dtype2entry(dd->type, KDT_SSFF);   /* search SSFF keyword */
    if(entry != NULL && entry->keyword != NULL) {
      dd->ident = strdup(entry->keyword);
    }
    else {
      dop = freeDObj(dop);
      setAsspMsg(AEB_ERR_TRACK, "for data type FFR (createFMT)");
      return(dop);
    }
  }
  else {
    dd->ident = strdup("Fn");
    strcpy(dd->sepChars, " ");                        /* within field */
  }

  dd = dd->next;               /* set pointer to next data descriptor */
  dd->type = DT_FBW;
  strcpy(dd->unit, "Hz");
  dd->coding = DC_LIN;
  dd->format = FMT_FBFORMAT;
  dd->numFields = gd->numFormants;
  if(dop->fileFormat == FF_SSFF) {
    entry = dtype2entry(dd->type, KDT_SSFF);   /* search SSFF keyword */
    if(entry != NULL && entry->keyword != NULL) {
      dd->ident = strdup(entry->keyword);
    }
    else {
      dop = freeDObj(dop);
      setAsspMsg(AEB_ERR_TRACK, "for data type FBW (createFMT)");
      return(dop);
    }
  }
  else {
    dd->ident = strdup("Bn");
    strcpy(dd->sepChars, " ");                        /* within field */
  }
  setRecordSize(dop);
  setStart_Time(dop);
  return(dop);
}

/*DOC

Function 'computeFMT'

Rerforms formant estimation of the speech signal referred to by "smpDOp" 
using the parameter settings specified in the generic data structure of 
the output data object pointed to by "fmtDOp". If "fmtDOp" is a NULL-
pointer "aoPtr" may not be a NULL-pointer and this function will create 
the output data object (see 'createFMT' for details).
If "aoPtr" is not a NULL-pointer this function will verify and - if 
necessary - (re-)allocate the data buffers in the data objects pointed 
to by "smpDOp" and "pitDOp" to have appropriate size.
Analysis results will be returned in the data buffer of the object 
pointed to by "fmtDOp" or written to file if that object refers to a 
file opened for writing. 
Returns a pointer to the formant data object or NULL upon error.

Note:
 - This function may be used in a file-to-file, file-to-memory, memory-
   to-file and memory-to-memory mode.
 - This function DOES NOT verify whether the parameters in the general 
   analysis structure pointed to by "aoPtr" comply with those in the 
   generic data structure of "fmtDOp". You will have to implement your 
   own verification if your application allows parameter changes between 
   calls (see 'createFMT' and e.g. 'verifyACF' for what needs to be 
   done).
   If there are incompatibilities it will probably be easiest just to 
   destroy the data object and let this function create a new one.

DOC*/

DOBJ *computeFMT(DOBJ *smpDOp, AOPTS *aoPtr, DOBJ *fmtDOp)
{
  char   *bPtr;
  int     FILE_IN, FILE_OUT, CREATED, USE_DURBIN, RESET_PQ, PF_VALID;
  int     err, n, m, anaFormants, bufFormants;
  long    i, fn, frameSize, frameShift, head, tail;
  double *dPtr, sampFreq;
  double  atc[MAXLPORDER+1], lpc[MAXLPORDER+1];
  double  pqp[MAXFORMANTS*2], ffb[MAXFORMANTS*2];
  double  pf[MAXFORMANTS];
  FMT_GD *gd;

  if(smpDOp == NULL || (aoPtr == NULL && fmtDOp == NULL)) {
    setAsspMsg(AEB_BAD_ARGS, "computeFMT");
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
      setAsspMsg(AED_NO_DATA, "(computeFMT)");
      return(NULL);
    }
  }
  /* check status of output object */
  if(fmtDOp == NULL) {
    if((fmtDOp=createFMT(smpDOp, aoPtr)) == NULL)
      return(NULL);
    CREATED = TRUE;
  }
  gd = (FMT_GD *)(fmtDOp->generic);
  if(fmtDOp->fp != NULL) {
    FILE_OUT = TRUE;
    gd->writeOpts = AFW_CLEAR;          /* discard data after writing */
    if(fmtDOp->fileData == FDF_ASC)
      gd->writeOpts |= AFW_ADD_TIME;
  }
  else
    gd->writeOpts = AFW_KEEP;

  frameSize = gd->frameSize;
  frameShift = fmtDOp->frameDur;
  sampFreq = smpDOp->sampFreq;
  head = 1; /* for pre-emphasis */
  tail = 0;
  if(aoPtr != NULL) {
    if(checkDataBufs(smpDOp, fmtDOp, head + frameSize + tail,\
		     gd->begFrameNr, gd->endFrameNr) < 0) {
      if(CREATED)
	freeDObj(fmtDOp);
      return(NULL);
    }
    if(aoPtr->options & AOPT_INIT_ONLY) {
      aoPtr->options &= ~AOPT_INIT_ONLY;                /* clear flag */
      return(fmtDOp);                                  /* no analysis */
    }
  }
  /* set global values and allocate local buffer space */
  if(setGlobals(fmtDOp) < 0) {
    if(CREATED)
      freeDObj(fmtDOp);
    return(NULL);
  }
  /* check silence threshold (we'll rely on it later) */
  if(gd->rmsSil < RMS_MIN_dB + 3.0)
    gd->rmsSil = RMS_MIN_dB + 3.0; /* > 3 dB above 'absolute' silence */
  /* determine how many formants we are going to analyse/evaluate */
  anaFormants = gd->lpOrder / 2;    /* number of formants in raw data */
  bufFormants = MIN(anaFormants, FMT_MAX_BUF);   /* maximum in buffer */
#ifndef WRASSP
  if(TRACE['A']) {
    fprintf(traceFP, "Analysis parameters\n");
    fprintf(traceFP, "  sample rate = %.1f Hz\n", sampFreq);
    fprintf(traceFP, "  window size = %ld samples\n", frameSize);
    fprintf(traceFP, "  window shift = %ld samples\n", frameShift);
    fprintf(traceFP, "  window function = %s\n",	\
	    wfSpecs(gd->winFunc)->entry->code);
    fprintf(traceFP, "  silence threshold = %.1f dB\n", gd->rmsSil);
    if(gd->options & FMT_OPT_PE_ADAPT)
      fprintf(traceFP, "  pre-emphasis = signal-adaptive\n");
    else
      fprintf(traceFP, "  pre-emphasis = %.7f\n", gd->preEmph);
    fprintf(traceFP, "  LP order = %d\n", gd->lpOrder);
    fprintf(traceFP, "  output formants = %d\n", gd->numFormants);
    fprintf(traceFP, "  nominal F1 = %.1f Hz\n", gd->nomF1);
    fprintf(traceFP, "  selected channel = %d\n", gd->channel);
    fprintf(traceFP, "  start frame = %ld\n", gd->begFrameNr);
    fprintf(traceFP, "  end frame = %ld\n", gd->endFrameNr);
    fprintf(traceFP, "  processing mode = %s-to-%s\n",\
	    FILE_IN ? "file" : "memory", FILE_OUT ? "file" : "memory");
    fprintf(traceFP, "  bairstow: maxIter = %d\n", term.maxIter);
    fprintf(traceFP, "            absPeps = %.4e relPeps %.4e\n" \
	             "            absQeps = %.4e relQeps %.4e\n",\
	    term.absPeps, term.relPeps, term.absQeps, term.relQeps);
  }
#endif
  /* loop over frames */
  clrAsspMsg();
  RESET_PQ = TRUE;
  for(fn = gd->begFrameNr; fn < gd->endFrameNr; fn++) {
    err = 0;
    PF_VALID = FALSE;
    USE_DURBIN = (gd->options & FMT_NOT_USE_PF);
    if(getSmpFrame(smpDOp, fn, frameSize, frameShift, head, tail,\
		   gd->channel, (void *)frame, FMT_PFORMAT) < 0) {
      err = -1;
      break;
    }
    dPtr = &frame[head];
    for(i = 0; i < frameSize; i++)
      rmsBuf[i] = *(dPtr++);
    if(gd->winFunc > WF_RECTANGLE)
      mulSigWF(rmsBuf, wfc, frameSize);
    getACF(rmsBuf, atc, frameSize, 1);
    if(atc[0] <= 0.0) {
      sortBuf.RMS = RMS_MIN_dB;
      sortBuf.LP1 = 0.0;
    }
    else {
      sortBuf.RMS = sqrt(atc[0]/(double)frameSize) / wfGain;
      if(sortBuf.RMS <= RMS_MIN_AMP) /* bottom clip for dB conversion */
	sortBuf.RMS = RMS_MIN_dB;
      else
	sortBuf.RMS = LINtodB(sortBuf.RMS);
      sortBuf.LP1 = -atc[1]/atc[0];
    }
    if(sortBuf.RMS < gd->rmsSil) {                 /* below threshold */
      if(TRACE['s'])
	totFMTsilent++;
      sortBuf.gain = GAIN_MIN_dB;           /* set to a defined value */
      nomFData(&sortBuf, bufFormants);     /* reset to nominal values */
      RESET_PQ = TRUE; /* NEW in R2.3 */
#ifndef WRASSP
      if(TRACE['0'] || TRACE['1'] || TRACE['2'] ||
	 TRACE['3'] || TRACE['4']) {
	printRaw(&sortBuf, fn, bufFormants, fmtDOp);
      }
#endif
    }
    else { /* run LP analysis */
      dPtr = &frame[head];                           /* reset pointer */
      if(gd->options & FMT_OPT_PE_ADAPT) /* NEW in R2.0 */
	gd->preEmph = sortBuf.LP1;
      preEmphasis(dPtr, gd->preEmph, frame[0], frameSize);
      if(gd->winFunc > WF_RECTANGLE)
	mulSigWF(dPtr, wfc, frameSize);
      getACF(dPtr, atc, frameSize, gd->lpOrder);
      if(!USE_DURBIN) {
	if((i=lpSLA(atc, lpc, &(sortBuf.gain), gd->lpOrder, pf,\
		    sampFreq)) < 0) {
	  bPtr = &applMessage[strlen(applMessage)];
	  if(asspWarning) {                        /* instable filter */
	    if(FILE_IN)
	      snprintf(bPtr, sizeof(applMessage) - strlen(applMessage), "\nat T = %.4f in %s",\
		      FRMNRtoTIME(fn, sampFreq, frameShift),\
		      myfilename(smpDOp->filePath));
	    else
	      snprintf(bPtr, sizeof(applMessage) - strlen(applMessage), "\nat T = %.4f",\
		      FRMNRtoTIME(fn, sampFreq, frameShift));
#ifndef WRASSP
	    if(TRACE['F'] || TRACE['f'])
	      prtAsspMsg(traceFP);
#endif
	    err = 1;
	  }
	  else {                         /* FATAL error in PF search */
	    if(FILE_IN)
	      snprintf(bPtr, sizeof(applMessage) - strlen(applMessage), "\nat T = %.4f in %s",\
		      FRMNRtoTIME(fn, sampFreq, frameShift),\
		      myfilename(smpDOp->filePath));
	    else
	      snprintf(bPtr, sizeof(applMessage) - strlen(applMessage), "\nat T = %.4f",\
		      FRMNRtoTIME(fn, sampFreq, frameShift));
	    err = -1;
	    /* break; */
	    /* NEW in R2.8: don't break off; replace error */
	    /*              by warning and try 'durbin' */
	    asspMsgNum = AWG_WARN_BUG;
	    for(n = 0; n < FMT_MAX_BUF; n++)  /* signal to 'probRiFn' */
	      sortBuf.pf[n] = 0.0;
	    USE_DURBIN = TRUE;
	  }
	}
	if(err == 0) {
	  for(n = 0; n < bufFormants; n++)         /* PFs p.d. sorted */
	    sortBuf.pf[n] = pf[n];
	  while(n < FMT_MAX_BUF)
	    sortBuf.pf[n++] = 0.0;
	  PF_VALID = TRUE;
	  if(TRACE['s']) {
	    statAddVal(&statPF, (double)i);
	    for(n = 0; n < FMT_NUM_PSTATS; n++) {
	      if(sortBuf.pf[n] > 0.0)
		statAddVal(&statP[n], sortBuf.pf[n]);
	    }
	  }
	}
      }
      if(USE_DURBIN) {
	err = 0; /* in case lpSLA failed */;
	if(asspDurbin(atc, lpc, NULL, &(sortBuf.gain), gd->lpOrder) < 0) {
	  bPtr = &applMessage[strlen(applMessage)];
	  if(FILE_IN)
	    snprintf(bPtr, sizeof(applMessage) - strlen(applMessage), "\nat T = %.4f in %s",\
		    FRMNRtoTIME(fn, sampFreq, frameShift),\
		    myfilename(smpDOp->filePath));
	  else
	    snprintf(bPtr, sizeof(applMessage) - strlen(applMessage), "\nat T = %.4f",\
		    FRMNRtoTIME(fn, sampFreq, frameShift));
#ifndef WRASSP
	  if(TRACE['F'] || TRACE['f'])
	    prtAsspMsg(traceFP);
#endif
	  err = 1;                                 /* instable filter */
	}
      }
      if(err != 0) {
	sortBuf.gain = GAIN_MIN_dB;         /* set to a defined value */
	nomFData(&sortBuf, bufFormants);   /* reset to nominal values */
	RESET_PQ = TRUE; /* NEW in R2.3 */
      }
      else {
	/* compute RMS amplitude of predition error in dB */
	sortBuf.gain = sqrt(sortBuf.gain/(double)frameSize) / wfGain;
	if(sortBuf.gain <= GAIN_MIN_LIN)
	  sortBuf.gain = GAIN_MIN_dB;
	else
	  sortBuf.gain = LINtodB(sortBuf.gain);
	/* get formant data from root-solving */
	if(RESET_PQ) {                /* (re-)estimate initial values */
	  if(PF_VALID)                   /* use Pisarenko frequencies */
	    pqStart(pf, pqp, anaFormants, sampFreq);
	  else                             /* use nominal frequencies */
	    pqStart(refFreq, pqp, anaFormants, sampFreq);
	  if(!(gd->options & FMT_NOT_TRACK_PQ) )
	    RESET_PQ = FALSE;
	}
	else if(!(gd->options & FMT_NOT_SORT_PQ) )
	  sortPQ(pqp, anaFormants); /* NEW in R0.8 */
	m = 1;
	if(!(gd->options & FMT_NOT_RETRY_PQ) ) { /* NEW in R2.3 */
	  m++;
	  if(PF_VALID)                  /* first try PFs then nominal */
	    m++;
	}
	for(n = 0; n < m; n++) {
	  i = lpc2pqp(lpc, pqp, gd->lpOrder, &term);
	  if(i >= 0 || n >= m-1)
	    break;
	  /* re-initialze PQ pairs and try again */
	  if(PF_VALID && n == 0)
	    pqStart(pf, pqp, anaFormants, sampFreq);
	  else
	    pqStart(refFreq, pqp, anaFormants, sampFreq);
	}
	if(i < 0) {
#ifndef WRASSP
	  if(TRACE['c'] || TRACE['0'] || TRACE['1'] ||
	     TRACE['2'] || TRACE['3'] || TRACE['4']) {
	    if(FILE_IN)
	      fprintf(traceFP, "Bairstow failed to converge at t = %f s"\
		      " in %s\n", FRMNRtoTIME(fn, sampFreq, frameShift),\
		      myfilename(smpDOp->filePath));
	    else
	      fprintf(traceFP, "Bairstow failed to converge at t = %f s\n",\
		      FRMNRtoTIME(fn, sampFreq, frameShift));
	    fprintf(traceFP, "RMS = %.1f  gain = %.1f  LP1 = %.2e\n",\
		    sortBuf.RMS, sortBuf.gain, sortBuf.LP1);
	  }
	  if(TRACE['s'])
	    totFMTfail++;
#endif
	  /* OLD: leave previous values in output buffer */
	  nomFData(&sortBuf, bufFormants); /* NEW in R2.0  reset */
	  RESET_PQ = TRUE; /* NEW in R2.3 */
	}
	else {
	  if(TRACE['s'])
	    statAddVal(&statPQ, (double)i);
	  pqp2rfb(pqp, ffb, anaFormants, sampFreq);
	  for(n = m = 0; n < bufFormants; n++) {   /* store in buffer */
	    if(ffb[m] <= 0.0)
	      break;
	    sortBuf.rf[n] = ffb[m++];
	    sortBuf.bw[n] = ffb[m++];
	    sortBuf.lock[n] = FALSE;
	    sortBuf.slot[n] = -1;
	  }
	  while(n < FMT_MAX_BUF) {                /* pad with dummies */
	    putDummy(&sortBuf, n);
	    n++;
	  }
	  classFmt(fn, &sortBuf, bufFormants, fmtDOp); 
	  if(TRACE['s']) {
	    for(n = 0; n < FMT_NUM_FSTATS; n++) {
	      if(sortBuf.rf[n] > 0.0)
		statAddVal(&statF[n], sortBuf.rf[n]);
	    }
	  }
	}
	if(gd->options & FMT_OPT_INS_ESTS) {
	  fillGaps(&sortBuf, gd->numFormants, sampFreq);
#ifndef WRASSP
	  if(TRACE['5'])
	    printRaw(&sortBuf, fn, bufFormants, fmtDOp);
#endif
	}
      }
    }
    if(err < 0)
      break;
    if((err=storeFMT(&sortBuf, fn, fmtDOp)) < 0)
      break;
  } /* END loop over frames */
  freeGlobals();
  if(err >= 0 && FILE_OUT)
    err = asspFFlush(fmtDOp, gd->writeOpts);
  if(err < 0) {
    if(CREATED)
      freeDObj(fmtDOp);
    return(NULL);
  }
  if(TRACE['s'])
    totFMTframes += (gd->endFrameNr - gd->begFrameNr);
  return(fmtDOp);
}

/*DOC

Function 'freeFMT_GD'

Returns all memory allocated for the generic data in an FMT data object.

DOC*/

void freeFMT_GD(void *generic)
{
  if(generic != NULL) {
    free(generic);
  }
  return;
}

/*DOC

Function 'initFMTstats'

Initializes the statistical variables for recording file, frame, fail 
and iteration counts and Pisarenko and formant frequencies.

DOC*/

void initFMTstats(void)
{
  int i;

  totFMTfiles = totFMTframes = totFMTsilent = totFMTfail = 0;
  statInit(&statPF);
  statInit(&statPQ);
  for(i = 0; i < FMT_NUM_PSTATS; i++)
    statInit(&statP[i]);
  for(i = 0; i < FMT_NUM_FSTATS; i++)
    statInit(&statF[i]);
  return;
}

/*DOC

Function 'freeFMTstats'

Returns all memory allocated for the statistical variables.

DOC*/

void freeFMTstats(void)
{
  int i;

  statFree(&statPF);
  statFree(&statPQ);
  for(i = 0; i < FMT_NUM_PSTATS; i++)
    statFree(&statP[i]);
  for(i = 0; i < FMT_NUM_FSTATS; i++)
    statFree(&statF[i]);
  return;
}

/* ======================= private  functions ======================= */

/***********************************************************************
* Compute and/or set the analysis order.                               *
* Note: This function may have to become public for verification.      *
***********************************************************************/
LOCAL void setOrder(FMT_GD *gd, AOPTS *aoPtr, double sampFreq)
{
  gd->nomF1 = aoPtr->nomF1;                   /* make sure it's set */
  if(aoPtr->options & FMT_OPT_LPO_FIXED) {
    gd->lpOrder = aoPtr->order;
  }
  else { /* use rule for default */
    gd->lpOrder = (int)floor(sampFreq / (2.0 * gd->nomF1 - 1)) + 1;
    if(ODD(gd->lpOrder))
      (gd->lpOrder)++;
    if(aoPtr->increment != 0) { /* NEW in R1.0 */
      gd->lpOrder += (2 * (aoPtr->increment));
    }
  }
  if(gd->lpOrder > MAXLPORDER)
    gd->lpOrder = MAXLPORDER;
  else if(gd->lpOrder < FMT_MIN_ORDER)
    gd->lpOrder = FMT_MIN_ORDER;
  return;
}
/***********************************************************************
* Compute and/or set the pre-emphasis coefficient.                     *
* Returns -1 for an invalid coefficient.                               *
***********************************************************************/
LOCAL int setPreEmph(FMT_GD *gd, AOPTS *aoPtr, double sampFreq)
{
  if(aoPtr->options & FMT_OPT_PE_ADAPT) /* NEW in R2.0 */
    return(0);
  gd->nomF1 = aoPtr->nomF1;                   /* make sure it's set */
  if(aoPtr->options & FMT_OPT_PE_FIXED) {
    if(aoPtr->preEmph < -1.0 || aoPtr->preEmph > 0.0) {
      setAsspMsg(AEB_ERR_EMPH, "(FMT: setPreEmph)");
      return(-1);
    }
    gd->preEmph = aoPtr->preEmph;
  }
  else { /* NEW in R1.0 dependent on sample rate and nominal F1 */
    gd->preEmph = -freq2emph(gd->nomF1, sampFreq);
  }
  return(0);
}
/***********************************************************************
* Allocate memory for the frame buffers and the window coefficients    *
* and set local global values.                                         *
***********************************************************************/
LOCAL int setGlobals(DOBJ *dop)
{
  int     i, nd, wFlags;
  long    frameSize, frameShift;
  double  sampFreq, shift, twoPiT;
  FMT_GD *gd;

  /* allocate memory for global buffers */
  rmsBuf = frame = wfc = NULL;
  gd = (FMT_GD *)(dop->generic);
  frameSize = gd->frameSize;
  frameShift = dop->frameDur;
  if(gd->winFunc > WF_RECTANGLE) {
    /* because of the relationship between the autocorrelation and */
    /* the power spectrum, we use the 'proper' periodical window */
    wFlags = WF_PERIODIC;
    if((ODD(frameSize) && EVEN(frameShift)) ||
       (EVEN(frameSize) && ODD(frameShift)) )
      wFlags = WF_ASYMMETRIC;       /* align window and frame centres */
    wfc = makeWF(gd->winFunc, frameSize, wFlags);
    if(wfc == NULL) {
      setAsspMsg(AEG_ERR_MEM, "(FMT: setGlobals)");
      return(-1);
    }
    wfGain = wfCohGain(wfc, frameSize);
  }
  else {
    /* wfc = NULL; */
    wfGain = 1.0;
  }
  rmsBuf = (double *)calloc((size_t)frameSize, sizeof(double));
  /* one sample extra for pre-emphasis */
  frame = (double *)calloc((size_t)(frameSize + 1), sizeof(double));
  if(rmsBuf == NULL || frame == NULL) {
    freeGlobals();
    setAsspMsg(AEG_ERR_MEM, "FMT: setGlobals");
    return(-1);
  }
  /* initialize the tables */
  setRefTables(gd->nomF1);

  tp[0] = 0.0;    /* allowability of double classification (variable) */
  tp[1] = 1.0;                      /* ideal case: successive numbers */
  for(i = 2; i < FMT_MAX_BUF; i++)   /* probablities missing formants */
    tp[i] = TP_FACTOR * tp[i-1];

  /* termination criteria for bairstow() */
  /* classical values: */
  /* term.maxIter = 100; */
  /* term.absPeps = 1.0e-12; */
  /* term.relPeps = 1.0e-6; */
  /* term.absQeps = 1.0e-12; */
  /* term.relQeps = 1.0e-6; */
  /*
    from q = exp(-2*PI*B/Fs)
    and  p = -2*sqrt(q)*cos(2*PI*F/Fs)
    follows  dp/dF = +2*sqrt(q)*2*PI/Fs*sin(2*PI*F/Fs)
    and      dq/dB = -2*PI/Fs*exp(-2*PI*B/Fs)
    relative dp/dF/p = -2*PI/Fs*tan(2*PI*F/Fs)
    and      dq/dB/q = -2*PI/Fs
    can set lower limits for F = 50 Hz and B = Fs
    problem: this only holds for complex conjugate roots! 
  */
  sampFreq = dop->sampFreq;
  twoPiT = TWO_PI / sampFreq;
  term.maxIter = 40 + (int)myrint(sampFreq/200.0); /* NEW in R2.0 */
  term.absPeps = 2.0 * exp(-PI) * twoPiT * sin(50.0 * twoPiT);
  term.relPeps = twoPiT * tan(50.0 * twoPiT);
  term.absQeps = twoPiT * exp(-TWO_PI);
  term.relQeps = twoPiT;
  /* reduce epses for real roots */
  term.absQeps /= 1000.0; /* generally too large */
  if(term.absQeps < DBL_EPSILON) /* but let's not overdo it */
    term.absQeps = DBL_EPSILON;
  term.absPeps /= 1000.0;
  if(term.absPeps < DBL_EPSILON)
    term.absPeps = DBL_EPSILON;
  term.relPeps /= 10.0; /* yields absolutely minimal improvement! */
  term.relQeps /= 10.0;

  /* construct formats for trace output in printRaw() */
  shift = SMPNRtoTIME(frameShift, sampFreq);
  nd = numDecim(shift, 12);
  if(nd <= 0)
    nd = 1;
  snprintf(trgepFormat, sizeof(trgepFormat), "%%%d.%df %%5.1f %%5.1f %%+.2e %%.1e", nd+3+1, nd);
  strcpy(fpbFormat, "  %c%d %4.0f/%-4.0f %4.0f");
  return(0);
}
/***********************************************************************
* return memory allocated for global arrays                            *
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
  return;
}
/***********************************************************************
* Set tables with resonance frequencies of an open tube and with       *
* heuristic values for the frequency ranges of each formant.           *
* BEWARE: Neighbouring formants MUST overlap or at least abut !!!      *
***********************************************************************/
LOCAL void setRefTables(double F1)
{
  int    i;
  double minF, maxF;

  for(i = 0; i < MAXFORMANTS; i++)
    refFreq[i] = (double)((2 * i) + 1) * F1;
  /*
   * insert option here to read table from file
   */
  /* maxF = 1.8; */
  maxF = 1.75; /* NEW in R0.8 */
  /* limits[0].min = 1.0; */
  limits[0].min = 50.0; /* NEW in R2.0 */
  limits[0].nom = myrint(refFreq[0]);
  limits[0].max = ceil(refFreq[0] + maxF * F1);
  minF = 2.0;
  /* maxF = 2.5; */ /* NEW in R0.8 F2 of female voices may go very high */
  maxF = 2.75; /* NEW in R2.0 apparently still not high enough */
  limits[1].min = floor(refFreq[1] - minF * F1);
  limits[1].nom = myrint(refFreq[1]);
  limits[1].max = ceil(refFreq[1] + maxF * F1);
  maxF = sqrt(maxF);
  limits[2].min = floor(refFreq[2] - minF * F1);
  limits[2].nom = myrint(refFreq[2]);
  limits[2].max = ceil(refFreq[2] + maxF * F1);
  minF = 1.75; /* NEW in R2.0  keep min(F4) low */
  maxF = sqrt(maxF);
  for(i = 3; i < FMT_MAX_BUF; i++) {
    /* minF = sqrt(maxF); */               /* rapidly converge to 1.0 */
    /* maxF = sqrt(maxF); */
    if(minF < 1.2) /* NEW in R2.3 leave some overlap */
      minF = 1.2;
    if(maxF < 1.2)
      maxF = 1.2;
    limits[i].min = floor(refFreq[i] - minF * F1);
    limits[i].nom = myrint(refFreq[i]);
    limits[i].max = ceil(refFreq[i] + maxF * F1);
    minF = sqrt(minF); /* NEW in R2.0 converge starting from F5 */
    maxF = sqrt(maxF);
  }
  /* these values are just to make classification simple and clear */
  for(i = 0; i < FMT_MAX_BUF; i++) {
    if(i == 0)
      limits[i].pLo = limits[i].min;
    else
      limits[i].pLo = limits[i-1].max + 1.0;
    if(i == (FMT_MAX_BUF-1))
      limits[i].pHi = limits[i].max;
    else
      limits[i].pHi = limits[i+1].min - 1.0;
    if(limits[i].pLo >= limits[i].pHi)
      limits[i].pLo = limits[i].pHi - 1.0;
    if(limits[i].pLo > limits[i].nom) /* NEW in R1.0 */
      limits[i].pLo = limits[i].nom;/* nom typically below pLo for F3 */
    if(limits[i].pHi < limits[i].nom)
      limits[i].pHi = limits[i].nom;
  }
#ifndef WRASSP
  if(TRACE['l']) {
    fprintf(traceFP, "Formant limits and private range:\n");
    for(i = 0; i < FMT_MAX_BUF; i++) {
      fprintf(traceFP, "  F%-2d  %5.0f - %5.0f - %5.0f - %5.0f - %5.0f\n",\
	      i+1, limits[i].min, limits[i].pLo, limits[i].nom,\
	      limits[i].pHi, limits[i].max);
    }
    TRACE['l'] = FALSE;                            /* print only once */
  }
#endif
  return;
}
/***********************************************************************
* Set elements of sort buffer to nominal values.                       *
***********************************************************************/
LOCAL void nomFData(FMTDATA *fPtr, int N)
{
  int n;

  for(n = 0; n < N; n++) {
    fPtr->rf[n] = fPtr->bw[n] = 0.0;
    fPtr->pf[n] = refFreq[n];
    fPtr->slot[n] = -1;
    fPtr->lock[n] = FALSE;
  }
  while(n < FMT_MAX_BUF) {
    fPtr->rf[n] = fPtr->bw[n] = fPtr->pf[n] = 0.0;
    fPtr->slot[n] = -1;
    fPtr->lock[n] = FALSE;
    n++;
  }
  return;
}
/***********************************************************************
* Estimate PQ start values for bairstow() from a set of frequencies.   *
***********************************************************************/
LOCAL void pqStart(double *freq, double *pqp, int N, double sampFreq)
{
  int    m, n;
  double fbp[MAXLPORDER]; /* frequency-bandwidth pair */

  for(m = n = 0; n < N; n++) {
    fbp[m++] = freq[n];
    fbp[m++] = freq[n] / 4.0;    /* bandwidths shouldn't be too small */
  }
  ffb2pqp(fbp, pqp, N, sampFreq);       /* convert to PQ start values */
  return;
}
/***********************************************************************
* NEW in R0.8 (slightly modified in R2.0)                              *
* Sort PQ pairs to reduce cumulative error in bairstow(). It turns out *
* that this at least reduces the number of convergence errors by about *
* 40% and -despite this- the maximum number of iterations by about the *
* same amount and the mean number of iterations by about 10%.          *
***********************************************************************/
LOCAL int sortPQ(register double *pqp, int N)
{
  register int i, j, n, ncc, M;
  double P, Q;

  M = 2 * N; /* number of values in 'pqp' */
  n = 0;
  ncc = M;
  while(n < ncc) {
    P = pqp[n];
    Q = pqp[n+1];
    if(hasCCR(P, Q)) {               /* complex conjugate roots first */
      for(i = 0; i < n; i += 2) {
	if(Q > pqp[i+1]) {    /* sort on decreasing Q (increasing BW) */
	  for(j = n-1; j >= i; --j)    /* shift upwards to make place */
	    pqp[j+2] = pqp[j];
	  pqp[i++] = P;
	  pqp[i] = Q;
	  break;                                  /* leave for() loop */
	}
      }
      n += 2;                              /* continue with next pair */
    }
    else {                         /* move real roots to end of array */
      ncc -= 2;                                       /* one CCR less */
      pqp[n] = pqp[ncc]; pqp[ncc] = P;                    /* exchange */
      pqp[n+1] = pqp[ncc+1]; pqp[ncc+1] = Q;
      /* DON'T increase 'n', test exchanged root or leave while () */
    }
  }
  /* elements ncc through M now contain the unsorted real roots */
  for(n = ncc+2; n < M; n += 2) {   /* implies: at least 2 real roots */
    P = pqp[n];
    for(i = ncc; i < n; i += 2) {
      if(fabs(P) > fabs(pqp[i])) {          /* sort on decreasing |P| */
	Q = pqp[n+1];
	for(j = n-1; j >= i; --j)      /* shift upwards to make place */
	  pqp[j+2] = pqp[j];
	pqp[i++] = P;
	pqp[i] = Q;
	break;                                    /* leave for() loop */
      }
    }
  }
  return(n/2);
}
/***********************************************************************
* Classify resonances as formants.                                     *
***********************************************************************/
#define FMT_MAX_PASSES 3
LOCAL int classFmt(long frameNr, FMTDATA *fPtr, int numFmt, DOBJ *dop)
{
  int     m, n, pass, merges, clashes;
  double  probEq, maxBW, maxDist, bwFact;
  FMT_GD *gd;

#ifndef WRASSP
  if(TRACE['0']) {
    printRaw(fPtr, frameNr, numFmt, dop);
  }
#endif
  gd = (FMT_GD *)(dop->generic);
  /* first remove non-interesting poles */
  numFmt = cleanRsn(fPtr, numFmt, dop);
  if(numFmt <= gd->numFormants)
    numFmt = gd->numFormants + 1;

#ifndef WRASSP
  if(TRACE['1']) {
    printRaw(fPtr, frameNr, numFmt, dop);
  }
#endif
  /*
   * Estimate formant numbers of resonances.
   * NEW in R1.0
   * We make up to 3 passes: in the first pass there is less penalty
   * for two resonances to be assigned the same formant number than for
   * a  missing formant in order to detect 'shaping' resonances. 
   * In the next passes the penalty for double assignments is increased 
   * and the maximum bandwidth for merging is decreased.
   */
  probEq = (tp[1] + tp[2]) / 2.0; /* slight penalty for equal numbers */
  bwFact = 2.5;                 /* start with fairly large bandwidths */
  maxDist = 0.5 * (gd->nomF1);                            /* constant */
  for(pass = 1; pass <= FMT_MAX_PASSES; pass++) {
    probRiFn(fPtr, numFmt, gd, probEq);  /* get most probable mapping */
#ifndef WRASSP
    if(TRACE['2']) {
      printRaw(fPtr, frameNr, numFmt, dop);
      if(TRACE['p']) {
	fprintf(traceFP, "%32s", " ");
	for(n = 0; n < numFmt; n++) {
	  if(fPtr->slot[n] >= 0)
	    fprintf(traceFP, "%19.7f", dp[n].pc[(int)(fPtr->slot[n])]);
	}
	fprintf(traceFP, "\n");
      }
    }
#endif
    /*
     * Try to solve double assignments by merging nearby resonances
     * one of which having a large bandwidth.
     */
    merges = clashes = 0;
    maxBW = bwFact * (gd->nomF1);
    for(n = 0; n < (numFmt-1); n++) {
      if(fPtr->rf[n+1] <= 0.0)
	break;
      if(fPtr->slot[n] == fPtr->slot[n+1]) {               /* clash ! */
	m = -1;
	if(fPtr->bw[n] > maxBW || fPtr->bw[n+1] > maxBW) {
	  m = tryMerge(fPtr, n, maxBW, maxDist);
	}
	else {   /* see whether we can merge with one lower or higher */
	  if(n > 0 && fPtr->bw[n-1] > maxBW)
	    m = tryMerge(fPtr, n-1, maxBW, maxDist);
	  if(m < 0) {
	    if(n < (numFmt-2) && fPtr->bw[n+2] > maxBW)
	      m = tryMerge(fPtr, n+1, maxBW, maxDist);
	  }
	}
	if(m < 0) {                             /* still not solved ? */
	  m = n + 1;
	  if(fPtr->bw[n] > fPtr->bw[m])
	    m = n;
	  if(fPtr->bw[m] > fPtr->rf[m] || 
	     (pass < FMT_MAX_PASSES && fPtr->bw[m] > maxBW))
	    shiftFmt(fPtr, m+1, -1);                     /* remove it */
	  else
	    m = -1;
	}
	if(m >= 0) {
	  merges++;
	  n = m - 1;
	}
	else
	  clashes++;
      }
    }
    if(merges > 0 || clashes > 0) {
      probEq *= TP_FACTOR; /* decrease probability of double mappings */
      bwFact -= 0.5;                  /* decrease mergeable bandwidth */
    }
    else                               /* no problems; can leave loop */
      break;
  }
  /* NEW in R1.0
   * When not completely solved enforce a mapping without double
   * assignments. When there are remaining clashes, this might be
   * the place to look for nasalization resonances.
   */
  if(merges > 0 || clashes > 0) {
    probEq = 0.0;                       /* no double mappings allowed */
    probRiFn(fPtr, numFmt, gd, probEq);
#ifndef WRASSP
    if(TRACE['3']) {
      printRaw(fPtr, frameNr, numFmt, dop);
      if(TRACE['p']) {
	fprintf(traceFP, "%32s", " ");
	for(n = 0; n < numFmt; n++) {
	  if(fPtr->slot[n] >= 0)
	    fprintf(traceFP, "%19.7f", dp[n].pc[(int)(fPtr->slot[n])]);
	}
	fprintf(traceFP, "\n");
      }
    }
#endif
  }
  /* finally, shift each resonance into its formant slot */
  for(n = 0; n < FMT_MAX_BUF; n++) {              /* go back to front */
    if(fPtr->slot[n] < 0)
      break;
  }
  while(n > 0) {
    --n;
    m = fPtr->slot[n];
    if(m != n) {
      if(m < FMT_MAX_BUF) {
	fPtr->rf[m] = fPtr->rf[n];          /* pf kept for comparison */
	fPtr->bw[m] = fPtr->bw[n];
	fPtr->slot[m] = m;
	fPtr->lock[m] = fPtr->lock[n];
      } /* else shifted out of array */
      putDummy(fPtr, n);
      fPtr->slot[n] = n;
    }
  }
#ifndef WRASSP
  if(TRACE['4']) {
    printRaw(fPtr, frameNr, numFmt, dop);
  }
#endif
  return(0);
}
/***********************************************************************
* Remove frequency/bandwidth pairs wich cannot represent a formant by  *
* merging with nearest neighbour or plain deletion.                    *
***********************************************************************/
LOCAL int cleanRsn(FMTDATA *fPtr, int numFmt, DOBJ *dop)
{
  int     m, n;
  double  maxF, maxBW, maxBW2, maxDist;
  FMT_GD *gd;

  /* remove pseudo resonances (excessive bandwidth) */
  /* maxBW = dop->sampFreq / 4.0; */                /* can't resonate */
  /* for(n = 0; n < numFmt; n++) { */
  /*   if(fPtr->rf[n] <= 0) */
  /*     break; */
  /*   if(fPtr->bw[n] > maxBW) { */
  /*     shiftFmt(fPtr, n+1, -1); */
  /*     n--; */
  /*   } */
  /* } */
  /*
   * test whether a resonance with an excessive bandwidth can be merged
   * with a nearby neighbour, if not remove it
   */
  gd = (FMT_GD *)(dop->generic);
  maxBW = 3.0 * (gd->nomF1);
  maxDist = 0.5 * (gd->nomF1);
  for(n = 0; n < numFmt; n++) {
    if(fPtr->rf[n] <= 0.0)
      break;
    if(fPtr->rf[n] <= gd->nomF1) {      /* F1 cases we'd like to keep */
      maxBW2 = 2.0 * (fPtr->rf[n]);
      maxBW2 = MIN(maxBW, maxBW2);
      if(fPtr->bw[n] > maxBW2) {
 	m = tryMerge(fPtr, n, maxBW2, maxDist);
	if(m < 0) {                                 /* couldn't merge */
	  shiftFmt(fPtr, n+1, -1);                     /* then remove */
	  n--;
	}
	else
	  n = m - 1;
      }
    }
    else {
      maxBW2 = 1.5 * (fPtr->rf[n]);
      maxBW2 = MIN(maxBW, maxBW2);
      if(fPtr->bw[n] > maxBW2) {
	m = tryMerge(fPtr, n, maxBW2, maxDist);
	if(m < 0) {
	  shiftFmt(fPtr, n+1, -1);
	  n--;
	}
	else
	  n = m - 1;
      }
    }
  }
  /*
   * remove poles with a frequency outside the formant range of interest
   */
  for(n = 0; n < numFmt; n++) { /* NEW in R2.0 also have minimum */
    if(fPtr->rf[n] <= 0.0)
      break;
    if(fPtr->rf[n] < limits[0].min) {
      shiftFmt(fPtr, n+1, -1);
      n--;
    }
    else
      break;
  }
  if((dop->sampFreq / 2.0) > INT16_MAX) /* NEW in R2.0 */
    maxF = INT16_MAX;
  else
    maxF = dop->sampFreq / 2.0;                     /* Nyquist rate */
  m = gd->numFormants + 1;                     /* let's be generous */
  if(m >= FMT_MAX_BUF)
    m = FMT_MAX_BUF - 1;
  maxF = MIN(maxF, limits[m].max);
  for(n = 0; n < numFmt; n++) {
    if(fPtr->rf[n] <= 0.0)
      break;
    if(fPtr->rf[n] > maxF) {
      shiftFmt(fPtr, n+1, -1);
      n--;
    }
  }
  return(MAX(n, m));                     /* number of remaining poles */
}
/***********************************************************************
* Assign resonances their most probable formant number using Pisarenko *
* frequencies for lower resonance frequencies and the limits table for *
* the remainder.                                                       *
***********************************************************************/
LOCAL int probRiFn(FMTDATA *fPtr, int numFmt, FMT_GD *gd, double probEq)
{
  int    i, n, m, numRsn, nMax;
  double R, P, F, B, r, d;
  double prob, minProb, maxProb, probMiss;

  nMax = FMT_MAX_BUF - 1;                    /* we'll often need this */
  fPtr->prob = 0.0;                    /* clear all related variables */
  for(i = 0; i <= nMax; i++) {
    fPtr->slot[i] = -1;
    fPtr->lock[i] = FALSE;
    for(n = 0; n <= nMax; n++) {
      dp[i].pc[n] = 0.0;
      dp[i].bt[n] = -1;
    }
  }
  if(!(gd->options & FMT_NOT_USE_PF)) {
    /*
     * Initial classification using Pisarenko frequencies:
     * Ri will be fixed at Fi when it has a small bandwidth and is close
     * to Pi.
     *
     * Note: Even for vowels, P3 need not correspond to F3 for male 
     *       voices and P2 to F2 for female voices; therefore break 
     *       classification off as soon as the conditions are not met.
     */
    for(i = 0; i < numFmt; i++) {
      if(fPtr->rf[i] <= 0.0) {
	break;
      }
      R = fPtr->rf[i];
      P = fPtr->pf[i];
      B = fPtr->bw[i];
      if(fabs(R - P)/(R + P) < 0.02 || /* NEW in R2.0 diff < 1% */
         (R <= limits[i].max + B && /* NEW in R1.0 */
	  B <= (0.5 * gd->nomF1) &&             /* absolute threshold */
	  B <= (0.25 * R + 0.05 * gd->nomF1) &&  /* relative + offset */
	  fabs(R - P) <= (0.5 * B))) {
	fPtr->slot[i] = i;
	fPtr->lock[i] = TRUE;
	dp[i].pc[i] = 1.0;
      }
      else if(i > 0) {
	break;                               /* don't try any further */
      }
    }
  }
  /*
   * Compute probabilities for the remainder using the limits table.
   */
  numRsn = numFmt;
  for(i = 0; i < numFmt; i++) {
    if((F=fPtr->rf[i]) <= 0.0) {
      numRsn = i;
      break;
    }
    if(!(fPtr->lock[i])) {                               /* not fixed */
      for(n = 0; n <= nMax; n++) {
	d = fabs(F - limits[n].nom);            /* distance to median */
	minProb = 1.0 / (d + 1.0);             /* minimum probability */
	if(F >= limits[n].min && F <= limits[n].max) {
	  /*
	   * Within formant range, consider 3 cases:
	   *
	   *                    (1)      (2)           (3)
	   * F[n+1]                              |--------------- - - -
	   * F[ n ]          |--------------------------------|
	   * F[n-1]  - - - ---------|^          ^             ^
	   *                 ^       |          |             |
	   *                 |       |          |             |
	   *                min     pLo        pHi           max
	   */
	  if(n > 0 && F < limits[n].pLo) {
	    /* d = F - limits[n].min + 1.0; */
	    d = F - limits[n].min; /* NEW in R2.0 */
	    /* r = limits[n].pLo - limits[n].min + 1.0; */
	    r = limits[n].pLo - limits[n].min; /* NEW in R2.0 */
	    /* dp[i].pc[n] = sin(HLF_PI * d / r); */
	    dp[i].pc[n] = sqrt(sin(HLF_PI * d / r)); /* NEW in R2.0 */
	    if(dp[i].pc[n] < minProb)
	      dp[i].pc[n] = minProb;
	  }
	  else if(n == nMax || F <= limits[n].pHi) {
	    dp[i].pc[n] = 1.0;
	  }
	  else {
	    /* d = limits[n].max - F + 1.0; */
	    d = limits[n].max - F; /* NEW in R2.0 */
	    /* r = limits[n].max - limits[n].pHi + 1.0; */
	    r = limits[n].max - limits[n].pHi; /* NEW in R2.0 */
	    /* dp[i].pc[n] = sin(HLF_PI * d / r); */
	    dp[i].pc[n] = sqrt(sin(HLF_PI * d / r)); /* NEW in R2.0 */
	    if(dp[i].pc[n] < minProb)
	      dp[i].pc[n] = minProb;     /* ensure that it's non-zero */
	  }
	}
	else {
	  /* NEW in R1.0
	   * Outside range; set probabilities, depending on 'probEq':
	   * if probEq > tp[2], probability = 0 in order to detect 
	   *    spurious resonances due to the fact that the LP order 
	   *    will -typically- be too high
	   * if probEq == 0, probability = minimum to allow enforcing a 
	   *    path in which two resonances cannot be asigned the same 
	   *    formant number
	   * in between, we allow a resonance to be assigned one formant
	   *    number outside its range but with minimum probability
	   */
	  if(probEq <= tp[2]) {
	    if(probEq <= 0.0)
	      dp[i].pc[n] = minProb;
	    else if(probEq > tp[3]) {
	      if((F < limits[n].min && n > 0 && F > limits[n-1].nom) ||
		 (F > limits[n].max && n < nMax && F < limits[n+1].nom))
		dp[i].pc[n] = minProb;
	    }
	    else {
	      if((F < limits[n].min && n > 0 && F > limits[n-1].pLo) ||
		 (F > limits[n].max && n < nMax && F < limits[n+1].pHi))
		dp[i].pc[n] = minProb;
	    }
	  } /* else dp[i].pc[n] = 0.0; */
	}
      }
    }
  }
  /*
   * Find the most probable resonance-to-formant mapping using dynamic 
   * programming.
   */
  /* NEW in R2.0 penalty for missing lower formants here ! */
  if(!fPtr->lock[0]) {
    probMiss = (fPtr->LP1 + 1.0) / 2.0;      /* estimated probability */
    if(probMiss < TP_FACTOR)                      /* don't go too low */
      probMiss = TP_FACTOR;
    for(n = 1; n < numRsn; n++) {
      dp[0].pc[n] *= pow(probMiss, (double)n);
    }
  }
  tp[0] = probEq;                    /* probality of double assgnment */
  for(i = 1; i < numRsn; i++) {  /* compute conditional probabilities */
    for(n = 0; n <= nMax; n++) {
      if(dp[i].pc[n] > 0.0) {
	maxProb = 0.0;
	for(m = 0; m <= n; m++) {
	  if((prob=dp[i-1].pc[m]) > 0.0) {
	    prob *= tp[n-m];
	    if(prob > maxProb) {
	      maxProb = prob;
	      dp[i].bt[n] = m;                     /* keep back trace */
	    }
	  }
	}
	dp[i].pc[n] *= maxProb;
      }
    }
  }
  i = numRsn - 1;                          /* determine best endpoint */
  maxProb = 0.0;
  for(n = 0; n <= nMax; n++) {
    if((prob=dp[i].pc[n]) > maxProb) {
      maxProb = prob;
      fPtr->slot[i] = n;
    }
  }
  while(i > 0) {                           /* trace back best mapping */
    fPtr->slot[i-1] = dp[i].bt[(int)(fPtr->slot[i])];
    --i;
  }
  fPtr->prob = maxProb;
  return(numRsn);
}
/***********************************************************************
* Determine the number of resonances in the sort buffer.               *
* (search backwards because a pseudo resonance may have been inserted) *
***********************************************************************/
LOCAL int getNumRsn(register FMTDATA *fPtr)
{
  register int n;

  for(n = FMT_MAX_BUF-1; n >= 0; n--) {
    if(fPtr->rf[n] > 0.0)
      break;
  }
  return(n+1);
}
/***********************************************************************
* Test whether resonance n or n+1 can be merged with neighbour.        *
* Conditions for merging are: bandwidth larger than maxBW and distance *
* to neighbour at most maxDist. If this is the case, merge and return  *
* index of merged resonances, else return -1.                          *
***********************************************************************/
LOCAL int tryMerge(register FMTDATA *fPtr, register int n,\
		   double maxBW, double maxDist)
{
  register int m, numRsn;
  double dl, dr;

  m = n + 1;
  numRsn = getNumRsn(fPtr);
  if(n >= numRsn || n < 0 || m >= numRsn)
    return(-1);
  if(fPtr->bw[n] > maxBW || fPtr->bw[m] > maxBW) {
    if(fPtr->bw[n] > fPtr->bw[m])
      m = n;                /* index of resonance which may be merged */
    dl = dr = 2.0 * maxDist;               /* initialize too far away */
    if(m > 0)
      dl = fPtr->rf[m] - fPtr->rf[m-1];
    if(m < (numRsn - 1))
      dr = fPtr->rf[m+1] - fPtr->rf[m];
    if(dl <= dr && dl <= maxDist) {
      mergeFmt(fPtr, m-1);                /* result in m-1; m invalid */
      shiftFmt(fPtr, m+1, -1); /* shift m+1 and higher one place down */
      return(m-1);
    }
    else if(dr <= maxDist) {
      mergeFmt(fPtr, m);                  /* result in m; m+1 invalid */
      shiftFmt(fPtr, m+2, -1); /* shift m+2 and higher one place down */
      return(m);
    }
  }
  return(-1);                                      /* can't be merged */
}
/***********************************************************************
* Merge resonances 'fmtNr' and 'fmtNr'+1                               *
***********************************************************************/
LOCAL void mergeFmt(register FMTDATA *fPtr, register int fmtNr)
{
  double F1, B1, F2, B2;
/*   double dB; */
  
  F1 = fPtr->rf[fmtNr];
  B1 = fPtr->bw[fmtNr];
  F2 = fPtr->rf[fmtNr+1];
  B2 = fPtr->bw[fmtNr+1];
  fPtr->rf[fmtNr] = (F1 * B2 + F2 * B1) / (B1 + B2);
/*   dB = fabs(B1 - B2); */
/*   if(dB > 100.0) */
/*     fPtr->bw[fmtNr] = (int)floor((B1 * B2)/dB + 0.5); */
/*   else */
  fPtr->bw[fmtNr] = MIN(B1, B2); /* negate ??? */
  return;
}
/***********************************************************************
* Shift resonances one place up or down and insert dummy in gap        *
***********************************************************************/
LOCAL void shiftFmt(register FMTDATA *fPtr, register int fmtNr,\
		    int direction)
{
  register int n;

  if(direction < 0) {
    for(n = fmtNr; n < FMT_MAX_BUF; n++) {
      if(n > 0) {
	fPtr->rf[n-1] = fPtr->rf[n];
	fPtr->bw[n-1] = fPtr->bw[n];
	fPtr->slot[n-1] = fPtr->slot[n];
	fPtr->lock[n-1] = fPtr->lock[n];
      }
    }
    putDummy(fPtr, n-1);
  }
  else if(direction > 0) {
    for(n = FMT_MAX_BUF-1; n > fmtNr; n--) {
      if(n > 0) {
	fPtr->rf[n] = fPtr->rf[n-1];
	fPtr->bw[n] = fPtr->bw[n-1];
	fPtr->slot[n] = fPtr->slot[n-1];
	fPtr->lock[n] = fPtr->lock[n-1];
      }
    }
    if(fmtNr < FMT_MAX_BUF)
      putDummy(fPtr, fmtNr);
  }
  return;
}
/***********************************************************************
* Put dummy values in formant data structure.                          *
***********************************************************************/
LOCAL void putDummy(FMTDATA *fPtr, int fmtNr)
{
  if(fmtNr >= 0 && fmtNr < FMT_MAX_BUF) {
    fPtr->rf[fmtNr] = 0.0;
    fPtr->bw[fmtNr] = 0.0;
    fPtr->slot[fmtNr] = -1;
    fPtr->lock[fmtNr] = FALSE;
  }
  return;
}
/***********************************************************************
* Set missing formant frequencies to Pisarenko frequencies.            *
***********************************************************************/
LOCAL void fillGaps(FMTDATA *fPtr, int numFmt, double sampFreq)
{
  int    m, n;
  double fl, fr, pf, max;

  max = floor(sampFreq / 2.0);
  for(n = 0; n < numFmt && n < FMT_MAX_BUF; n++) {
    if(fPtr->rf[n] <= 0.0) {                       /* missing formant */
      fPtr->bw[n] = 0.0;                         /* just to make sure */
      fl = 0.0;                        /* set range of fitting values */
      if(n > 0)
	fl = fPtr->rf[n-1];
      fr = max;
      if(n < (numFmt - 1)) {
	fr = fPtr->rf[n+1];
	if(fr <= 0.0)
	  fr = limits[n].max;
      }
      for(m = n; m < numFmt &&  m < FMT_MAX_BUF; m++) {
	pf = fPtr->pf[m];
	if(pf <= 0.0 || pf >= fr)
	  break;                                              /* pity */
	if(pf > fl && pf < fr) {
	  fPtr->rf[n] = pf;
	  break;
	}
      }
    }
  }
  return;
}
/***********************************************************************
* Copy/convert frame data to output buffer; handle data writes.        *
***********************************************************************/
LOCAL int storeFMT(FMTDATA *fPtr, long frameNr, DOBJ *dop)
{
  int      FILE_OUT;
  size_t   numBytes, n;
  long     ndx;
  char    *vPtr;
  int16_t *i16Ptr;
  double  *dPtr;
  DDESC   *dd;
  FMT_GD  *gd;

  FILE_OUT = (dop->fp != NULL);
  gd = (FMT_GD *)(dop->generic);
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
      setAsspMsg(AEG_ERR_BUG, "storeFMT: buffer overflow");
      return(-1);
    }
  }
  ndx = frameNr - dop->bufStartRec;
  numBytes = ndx * dop->recordSize;           /* offset to frame data */
  vPtr = (char *)dop->dataBuffer + numBytes;
  dd = &(dop->ddl);
  if(dd->type == DT_LP1) {
    dPtr = (double *)vPtr;
    *dPtr = fPtr->LP1;
    vPtr += sizeof(double);
    dd = dd->next;
  }
  i16Ptr = (int16_t *)vPtr;
  dPtr = fPtr->rf;
  for(n = 0; n < dd->numFields; n++)
    *(i16Ptr++) = (int16_t)myrint(*(dPtr++));
  dd = dd->next;
  dPtr = fPtr->bw;
  for(n = 0; n < dd->numFields; n++)
    *(i16Ptr++) = (int16_t)myrint(*(dPtr++));
  if(ndx >= dop->bufNumRecs)
    dop->bufNumRecs = ndx + 1;
  dop->bufNeedsSave = TRUE;
  return(0);
}
/***********************************************************************
* Print raw analysis data to trace output stream.                      *
***********************************************************************/
LOCAL int printRaw(FMTDATA *fPtr, long frameNr, int numFMT, DOBJ *dop)
{
  int n;

  fprintf(traceFP, trgepFormat,\
	  FRMNRtoTIME(frameNr, dop->sampFreq, dop->frameDur),\
	  fPtr->RMS, fPtr->gain, fPtr->LP1, fPtr->prob);
  for(n = 0; n < numFMT; n++) {
    fflush(traceFP);
    fprintf(traceFP, fpbFormat, fPtr->lock[n] ? '#':' ',\
	    (int)fPtr->slot[n]+1, fPtr->rf[n], fPtr->pf[n], fPtr->bw[n]);
  }
  fprintf(traceFP, "\n");
  return(0);
}
