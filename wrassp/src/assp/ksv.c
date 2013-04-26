/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 2011  Michel Scheffers                          *
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
* File:     ksv.c                                                      *
* Contents: Functions implementing the K. Schäfer-Vincent Periodicity  *
*           Detection Algorithm. See printKSVrefs() for references.    *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
* Note:                                                                *
* - Differences between this implementation and the original one are   *
*   documented in the respective functions.                            *
*                                                                      *
***********************************************************************/
/* $Id: ksv.c,v 1.8 2011/01/06 10:50:39 mtms Exp $ */

#include <stdio.h>     /* FILE NULL etc. */
#include <stdlib.h>    /* malloc() calloc() free() realloc() */
#include <stddef.h>    /* size_t */
#include <string.h>    /* memset() strcpy() strchr() memcpy() */
#include <ctype.h>     /* tolower() */
#include <inttypes.h>  /* uint8_t */
#include <limits.h>    /* LONG_MIN */
#include <math.h>      /* fabs() */

#include <miscdefs.h>  /* TRUE FALSE LOCAL NATIVE_EOL */
#include <misc.h>      /* strxcmp() myrint() numDecim() */
#include <trace.h>     /* trace handler */
#include <asspmess.h>  /* error message handler */
#include <assptime.h>  /* standard conversion macros */
#include <ksv.h>       /* private constants, structures and prototypes */
#include <asspana.h>   /* AOPTS anaTiming() */
#include <asspfio.h>   /* asspFFlush() */
#include <dataobj.h>   /* DOBJ getSmpCaps() getSmpPtr() */
#include <labelobj.h>  /* LABEL and label functions & formats */
#include <headers.h>   /* KDTAB */
#include <aucheck.h>   /* checkSound() */

#ifndef LONG_MIN
#define LONG_MIN (0x80000000L)
#endif

/*
 * fixed parameter values
 */
/* #define KSV_PDTOLT 0.10    tolerance on period durations in twin */
#define KSV_PDTOLT 0.12   /* tolerance on period durations in twin */
#define KSV_PDTOLC 0.16   /* tolerance on period durations between twins */
#define KSV_GAPTOL 1.12   /* maximum size of gap in alive chain */
#define KSV_MIN_DUR  25.0 /* minimum duration of voiced segment (ms) */
#define KSV_MIN_TWIN 3    /* at least 3 twins in voiced segment */
/* #define KSV_MIN_PRD  2.71  at least 3 periods (1+(1-PDTOLT)*(2-PDTOLT)) */
#define KSV_MIN_PRD 2.6544 /* at least 3 periods (1+(1-PDTOLT)*(2-PDTOLT)) */
#define KSV_NUM_AMV 8      /* number of AMV values */

/*
 * structures
 */
typedef struct { /* signal extremum */
  float mag; /* absolute amplitude value */
  long  sn;  /* sample number */
} EXTR;

typedef struct { /* period twins linked to chains */
  int     dur12;  /* duration of older period */
  int     dur23;  /* duration of younger period */
  long    sn3;    /* sample number of youngest extremum (end-of-twin) */
  long    nTwins; /* number of period twins in chain */
  long    totDur; /* duration of chain up to and including this twin */
  int     link;   /* index of preceding twin in chain (-1 if no link) */
  uint8_t flags;  /* see below */
  uint8_t type;
} TWIN;
#define KSV_UNUSED 0x00 /* twin status flags */
#define KSV_IN_USE 0x01 /* element in use */
#define KSV_EOC    0x02 /* twin is end of chain */
#define KSV_MAX 0 /* extremum type BEWARE: also used as index !!! */
#define KSV_MIN 1

typedef struct { /* period data at sampling rate */
  long    val; /* (summed) period duration(s) in samples */
  uint8_t cnt; /* occurrences at this location */
  uint8_t tag; /* code for period markers */
} PRDS;
#define TAG_NONE 0x00
#define TAG_MAX  0x01
#define TAG_MIN  0x02

/*
 * local global arrays and variables
 */
LOCAL long  begSmpNr, endSmpNr; /* analysis interval in sample numbers */
LOCAL int   smpOverlap;  /* overlap in workspace for AMV computation */
LOCAL EXTR *extrBuf[2];  /* arrays for generalized maxima & minima */
LOCAL int   maxExtrema;  /* size of extrema buffer per type */
LOCAL int   numExtr[2];  /* current number of extrema of each type */
LOCAL TWIN *twinBuf;     /* array for period twins linked to chains */
LOCAL int   maxTwins;    /* size of period chain buffer */
LOCAL int   lastUsed;    /* index of last used element in twinBuf */
LOCAL int   aliveIndex;  /* index of last twin of alive period chain */
LOCAL PRDS *ringBuf;     /* ring buffer with (summed) period durations */
LOCAL int   ringLength;  /* size of period ring buffer at sampling rate */
LOCAL long  ringBsn;     /* sample number of begin of ring */
LOCAL int   ringHead;    /* corresponding index in ringBuf */
LOCAL long  ringEsn;     /* end sample number of ring */
LOCAL int   outputDelay; /* delay from ring to output buffer */
LOCAL int   minPrdLen;   /* minimum period length */
LOCAL int   maxPrdLen;   /* maximum period length */
LOCAL int  *maxPdT;      /* tables with tolerances within twin & of gap */
LOCAL int  *minPdT;
LOCAL int  *maxPdC;      /*   same between twins in alive chain */
LOCAL int  *minPdC;
LOCAL int   maxLenTwin;  /* maximum length of period twin */
LOCAL int   minDurChain; /* minimum duration of a chain in samples */
LOCAL int   minVoiced;   /* minimum number of voiced samples in frame */
LOCAL int   VOICED;      /* flag for marking voiced regions */
LOCAL DOBJ *workDOp;     /* data object with workspace */

/*
 * prototypes of local functions
 */
LOCAL int  setGlobals(DOBJ *f0DOp);
LOCAL int  checkTags(DOBJ *prdDOp, DOBJ *smpDOp);
LOCAL int  allocBufs(DOBJ *smpDOp);
LOCAL void freeBufs(void);
LOCAL int  ksvExtr(long *start, long end, long *sn, float *mag, int *type);
LOCAL int  ksvTwin(long sn3, float a3, int type);
LOCAL double ksvZCR(long bsn, int dur);
LOCAL int  ksvAMV(long bsn, int dur, double amv[]);
LOCAL int  ksvChain(long sn1, long sn2, long sn3, int type);
LOCAL int  putChain(int i, int OVERWRITE);
LOCAL void clrChain(int i);
LOCAL int  ksvConvert(long smpNr, int FINISH, DOBJ *f0DOp, DOBJ *tagDOP);
LOCAL int  storeF0(float F0, long frameNr, DOBJ *f0DOp);
LOCAL int  storeTag(char *name, long smpNr, DOBJ *tagDOp);

/* ======================== public functions ======================== */

/*DOC

Function 'printKSVrefs'

Prints the references for the KSV Periodicity Detection Algorithm.

DOC*/

void printKSVrefs(void)
{
#ifndef WRASSP
  printf("\nReferences:\n");
  printf("Schäfer-Vincent, K. (1982), \"Significant points: Pitch period\n");
  printf("   detection as a problem of segmentation,\" Phonetica 39,\n");
  printf("   pp. 241-253\n");
  printf("Schäfer-Vincent, K. (1983), \"Pitch period detection and chaining:\n");
  printf("   Method and evaluation,\" Phonetica 40, pp. 177-202\n");
#endif
  return;
}

/*DOC

Function 'setKSVgenderDefaults'

Sets the items 'gender', 'minF' and 'maxF' in the analysis options 
structure pointed to by "aoPtr" to the default values for "gender".
Returns 0 upon success and -1 upon error.

DOC*/

int setKSVgenderDefaults(AOPTS *aoPtr, char gender)
{
  if(aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setKSVgenderDefaults");
    return(-1);
  }
  switch(gender) {
  case 'F': /* female */
  case 'f':
    aoPtr->minF = KSV_MINF0_f;
    aoPtr->maxF = KSV_MAXF0_f;
    break;
  case 'M': /* male */
  case 'm':
    aoPtr->minF = KSV_MINF0_m;
    aoPtr->maxF = KSV_MAXF0_m;
    break;
  case 'U': /* unknown */
  case 'u':
    aoPtr->minF = KSV_MINF0_u;
    aoPtr->maxF = KSV_MAXF0_u;
    break;
  default:
    setAsspMsg(AEG_ERR_BUG, NULL);
    sprintf(applMessage, "setKSVgenderDefaults: invalid gender code '%c'",\
	    gender);
    return(-1);
  }
  aoPtr->gender = tolower((int)gender);
  return(0);
}

/*DOC

Function 'setKSVdefaults'

Sets the items in the analysis options structure relevant to the KSV F0 
analysis algorithm to their default values. Clears all other items.
Returns 0 upon success and -1 upon error.

DOC*/

int setKSVdefaults(AOPTS *aoPtr)
{
  if(aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "setKSVdefaults");
    return(-1);
  }
  memset((void *)aoPtr, 0, sizeof(AOPTS));         /* clear all items */
  aoPtr->centreTime = -1.0;                          /* not supported */
  aoPtr->msSize = -1.0;                       /* no fixed window size */
  aoPtr->msShift = KSV_DEF_SHIFT;                 /* frame shift (ms) */
  aoPtr->voiMag = KSV_DEF_VOIMAG;              /* magnitude threshold */
  aoPtr->voiZCR = KSV_DEF_VOIZCR;          /* zero crossing threshold */
  aoPtr->channel = KSV_DEF_CHANNEL;         /* channel to be analysed */
  aoPtr->precision = KSV_DEF_DIGITS;    /* digits precision for ASCII */
  strcpy(aoPtr->format, KSV_DEF_FORMAT);        /* output file format */
  /* gender-specific defaults for min/maxF0 */
  if(setKSVgenderDefaults(aoPtr, KSV_DEF_GENDER) < 0)
    return(-1);
  return(0);
}

/*DOC

Function 'createKSV'

Allocates memory for a data object to hold KSV F0 analysis data and 
initializes it on the basis of the information in the audio object 
pointed to by "smpDOp" and the analysis options in the structure 
pointed to by "aoPtr". Neither of these pointers may be NULL.
This function does NOT allocate memory for the data buffer.
It returns a pointer to the data object or NULL upon error.

DOC*/

DOBJ *createKSV(DOBJ *smpDOp, AOPTS *aoPtr)
{
  long    auCaps;
  ATIME   aTime, *tPtr;
  KSV_GD *gd=NULL;
  DOBJ   *dop=NULL;
  DDESC  *dd=NULL;
  KDTAB  *entry;

  if(smpDOp == NULL || aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "createKSV");
    return(NULL);
  }
  if(aoPtr->options & AOPT_USE_CTIME) {
    setAsspMsg(AEB_BAD_CALL, "createKSV: no single-frame analysis");
    return(NULL);
  }
  /* verify audio object */
  if((auCaps=getSmpCaps(KSV_PFORMAT)) <= 0)
    return(NULL);
  auCaps |= KSV_I_CHANS;
  if(aoPtr->channel < 1)
    aoPtr->channel = KSV_DEF_CHANNEL;
  if(checkSound(smpDOp, auCaps, aoPtr->channel) <= 0)
    return(NULL);
  /* ensure correct interpretation by anaTiming() */
  aoPtr->msSize = -1.0;                       /* no fixed window size */
  aoPtr->options &= ~AOPT_EFFECTIVE;
  tPtr = &aTime;
  if(anaTiming(smpDOp, aoPtr, tPtr) < 0)
    return(NULL);
  clrAsspMsg();                               /* ignore warnings here */
  if(aoPtr->minF < KSV_ABSMIN_F0) {
    asspMsgNum = AEG_ERR_APPL;
    sprintf(applMessage, "KSV: minimum F0 too low (minimally %d Hz)",\
	    (int)KSV_ABSMIN_F0);
    return(NULL);
  }
  if(aoPtr->maxF <= aoPtr->minF) {
    asspMsgNum = AEG_ERR_APPL;
    sprintf(applMessage, "KSV: maximum F0 <= minimum F0");
    return(NULL);
  }
  if((gd=(KSV_GD *)malloc(sizeof(KSV_GD))) == NULL) {
    setAsspMsg(AEG_ERR_MEM, "(createKSV)");
    return(NULL);
  }
  strcpy(gd->ident, KSV_GD_IDENT);
  gd->options = aoPtr->options;
  gd->begFrameNr = tPtr->begFrameNr;
  gd->endFrameNr = tPtr->endFrameNr;
  gd->minF0 = aoPtr->minF;
  gd->maxF0 = aoPtr->maxF;
  gd->voiMag = aoPtr->voiMag;
  gd->voiZCR = aoPtr->voiZCR;
  gd->channel = aoPtr->channel;
  gd->precision = aoPtr->precision;

  if((dop=allocDObj()) == NULL) {
    freeKSV_GD((void *)gd);
    strcpy(applMessage, "(createKSV)");
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
  dop->doFreeGeneric = (DOfreeFunc)freeKSV_GD;

  dd = &(dop->ddl);                 /* set pointer to data descriptor */
  dd->type = DT_PIT;
  dd->coding = DC_LIN;
  dd->format = KSV_DFORMAT;
  dd->numFields = 1;
  if(dop->fileFormat == FF_SSFF) {  /* can't use KDT_SSFF in his case */
    dd->ident = strdup(KSV_SSFF_ID);
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
      setAsspMsg(AEB_ERR_TRACK, "(createKSV)");
      return(NULL);
    }
    strcpy(dop->sepChars, "\t");                    /* between fields */
    strcpy(dd->sepChars, " ");                        /* within field */
    sprintf(dd->ascFormat, "%%.%df", gd->precision);
  }
  else { /* fall through to raw ASCII */
    dd->ident = strdup("F0");
    strcpy(dd->unit, "Hz");
    strcpy(dop->sepChars, "\t");                    /* between fields */
    strcpy(dd->sepChars, " ");                        /* within field */
    sprintf(dd->ascFormat, "%%.%df", gd->precision);
  }
  setRecordSize(dop);
  setStart_Time(dop);
  return(dop);
}

/*DOC

Function 'createPRD'

Allocates memory for a label object to hold voicing and period markers 
and initializes it on the basis of the information in the audio object 
pointed to by "smpDOp" and the analysis options in the structure 
pointed to by "aoPtr". Neither of these pointers may be NULL.
Returns a pointer to the label object or NULL upon error.

DOC*/

DOBJ *createPRD(DOBJ *smpDOp, AOPTS *aoPtr)
{
  DOBJ    *dop=NULL;
  DDESC   *dd=NULL;
  XLBL_GD *gd=NULL;

  if(smpDOp == NULL || aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "createPRD");
    return(NULL);
  }
  if(!(aoPtr->options & KSV_OPT_PRD_OUT)) {
    setAsspMsg(AEB_BAD_CALL, "createPRD");
    return(NULL);
  }
  if(!(aoPtr->options & KSV_OPT_PRD_MIX)) { /* ESPS xlabel format */
    if((gd=(XLBL_GD *)malloc(sizeof(XLBL_GD))) == NULL) {
      setAsspMsg(AEG_ERR_MEM, "(createPRD)");
      return(NULL);
    }
    strcpy(gd->ident, XLBL_GD_IDENT);
    if(smpDOp->filePath != NULL)
      gd->signal = strdup(myfilename(smpDOp->filePath));
    else
      gd->signal = NULL;
    gd->font = NULL;
    gd->color = XLBL_DEF_COLOR;
  }

  if((dop=allocDObj()) == NULL) {
    if(gd != NULL)
      freeXLBL_GD((void *)gd);
    strcpy(applMessage, "(createPRD)");
    return(NULL);
  }
  dd = &(dop->ddl);
  if(gd != NULL) {
    dop->fileFormat = FF_XLABEL;
    strcpy(dop->eol, XLBL_EOL_STR);
    dop->generic = (void *)gd;
    dop->doFreeGeneric = (DOfreeFunc)freeXLBL_GD;
    dd->coding = DC_XLBL;
    strcpy(dd->sepChars, XLBL_DEF_SEP);                 /* irrelevant */
  }
  else { /* IPdS MIX format */
    dop->fileFormat = FF_IPDS_M;
    strcpy(dop->eol, MIX_EOL_STR);
    dd->coding = DC_MIX;
  }
  dop->fileData = FDF_ASC;
  CLRENDIAN(dop->fileEndian);
  dop->sampFreq = smpDOp->sampFreq;
  dop->frameDur = 0;                                     /* irregular */
  dop->recordSize = 0;                                    /* variable */
  dop->startRecord = 0;                             /* per definition */
  dop->numRecords = 0;                             /* can't determine */
  dop->Start_Time = dop->Time_Zero = 0.0;
  strcpy(dop->sepChars, "");                        /* blanks or tabs */
  dd->ident = strdup("events");
  dd->type = DT_TAG;
  dd->format = DF_STR;
  SETEVENT(dd->orientation);
  dd->numFields = 1;
  return(dop);
}

/*DOC

Function 'computeKSV'

Performs an F0 analysis of the audio signal referred to by "smpDOp" 
using the KSV algorithm with parameter settings specified in the generic 
data structure of the output data object pointed to by "f0DOp". 
If "f0DOp" is a NULL-pointer "aoPtr" may not be a NULL-pointer and this 
function will create the output data object (see 'createKSV' for details). 
If "aoPtr" is not a NULL-pointer this function will verify and - if 
necessary (re-)allocate - the data buffers in the data objects pointed 
to by "smpDOp" and "f0DOp" to have appropriate size. Analysis results 
will be returned in the data buffer of the object pointed to by "f0DOp" 
or written to file if that object refers to a file opened for writing.
If "prdDOp" is not a NULL pointer it will be verified to refer to a 
label object capable of holding voicing and period markers. If this is 
the case they will be generated and either stored in the data buffer of 
that object or written to file if it refers to a file opened for writing.
Returns a pointer to the F0 data object or NULL upon error.

Note:
 - This function may be used in a file-to-file, file-to-memory, memory-
   to-file and memory-to-memory mode.
 - This function DOES NOT verify whether the analysis parameters in the 
   generic data structure of "f0DOp" comply with those in the general 
   analysis structure pointed to by "aoPtr". Use the function 'verifyKSV'
   if parameter changes may occur between calls.

DOC*/

DOBJ *computeKSV(DOBJ *smpDOp, AOPTS *aoPtr, DOBJ *f0DOp, DOBJ *prdDOp)
{
  char   *cPtr;
  int     FILE_IN, FILE_OUT, CREATED, EOR;
  int     err, nd, n, type;
  long    smpNr, start, end, head, tail, absEndSn, temp;
  float   mag;
  KSV_GD *gd;

  if(smpDOp == NULL || (aoPtr == NULL && f0DOp == NULL)) {
    setAsspMsg(AEB_BAD_ARGS, "computeKSV");
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
      setAsspMsg(AED_NO_DATA, "(computeKSV)");
      return(NULL);
    }
  }
  /* check status of output object */
  if(f0DOp == NULL) {
    if((f0DOp=createKSV(smpDOp, aoPtr)) == NULL)
      return(NULL);
    CREATED = TRUE;
  }
  gd = (KSV_GD *)(f0DOp->generic);
  if(gd->options & AOPT_USE_CTIME) {
    setAsspMsg(AEB_BAD_CALL, "computeKSV: no single-frame analysis");
    return(NULL);
  }
  if(f0DOp->fp != NULL) {
    FILE_OUT = TRUE;
    gd->writeOpts = AFW_CLEAR;        /* discard data after writing */
    if(f0DOp->fileData == FDF_ASC)
      gd->writeOpts |= AFW_ADD_TIME;
  }
  else
    gd->writeOpts = AFW_KEEP;        /* just for consistency's sake */
  /* set global values */
  if(setGlobals(f0DOp) < 0) {
    if(CREATED)
      freeDObj(f0DOp);
    return(NULL);
  }
  if(aoPtr != NULL) {
    if(checkDataBufs(smpDOp, f0DOp, smpOverlap,\
		     gd->begFrameNr, gd->endFrameNr) < 0) {
      if(CREATED)
	freeDObj(f0DOp);
      return(NULL);
    }
    if(aoPtr->options & AOPT_INIT_ONLY) {
      aoPtr->options &= ~AOPT_INIT_ONLY;                /* clear flag */
      return(f0DOp);                                   /* no analysis */
    }
  }
  if(allocBufs(smpDOp) < 0) {
    if(CREATED)
      freeDObj(f0DOp);
    return(NULL);
  }
  if(prdDOp != NULL) {
    if(checkTags(prdDOp, smpDOp) < 0) {
      freeBufs();
      if(CREATED)
	freeDObj(f0DOp);
      return(NULL);
    }
  }
  /* copy pointer to generic data of f0DOp to workDOp */
  /* but do not provide a freeing function */
  workDOp->generic = f0DOp->generic;
  workDOp->doFreeGeneric = (DOfreeFunc)NULL;

#ifndef WRASSP
  if(TRACE['A']) {
     fprintf(traceFP, "Analysis parameters\n");
     fprintf(traceFP, "  sample rate = %.1f Hz\n", f0DOp->sampFreq);
    fprintf(traceFP, "  frame shift = %ld samples\n", f0DOp->frameDur);
    fprintf(traceFP, "  F0 range = %.1f to %.1f Hz\n",\
	    gd->minF0, gd->maxF0);
    fprintf(traceFP, "  min. voiced magnitude = %.0f\n", gd->voiMag);
    fprintf(traceFP, "  max. zero crossing rate = %.0f\n", gd->voiZCR);
    fprintf(traceFP, "  selected channel = %d\n", gd->channel);
    fprintf(traceFP, "  start frame = %ld\n", gd->begFrameNr);
    fprintf(traceFP, "  end frame = %ld\n", gd->endFrameNr);
    fprintf(traceFP, "  processing mode = %s-to-%s\n",\
	    FILE_IN ? "file" : "memory", FILE_OUT ? "file" : "memory");
    fprintf(traceFP, "Buffer sizes etc.\n");
    fprintf(traceFP, "  extrema buffer : %d per type\n", maxExtrema);
    fprintf(traceFP, "  twin buffer    : %d elements\n", maxTwins);
    fprintf(traceFP, "  ring buffer    : %d samples\n", ringLength);
    fprintf(traceFP, "  delay to output: %d samples\n", outputDelay);
    fprintf(traceFP, "  overlap        : %d samples\n", smpOverlap);
    fprintf(traceFP, "  audio buffer   : %ld records\n",\
	    FILE_IN ? smpDOp->maxBufRecs : smpDOp->bufNumRecs);
    fprintf(traceFP, "  workspace      : %ld samples\n", workDOp->maxBufRecs);
    fprintf(traceFP, "  F0 buffer      : %ld frames\n", f0DOp->maxBufRecs);
  }
#endif
  /* enforce a full pre-load of the workspace */
  workDOp->bufNumRecs = 0;
  head = 0;
  tail = workDOp->maxBufRecs - 1;
  if(FILE_IN) {
    if(tail >= smpDOp->maxBufRecs)
      tail = smpDOp->maxBufRecs - 1;
    absEndSn = smpDOp->startRecord + smpDOp->numRecords;
  }
  else {
    if(tail >= smpDOp->bufNumRecs)
      tail = smpDOp->bufNumRecs - 1;
    absEndSn = smpDOp->bufStartRec + smpDOp->bufNumRecs;
  }
  EOR = FALSE;
  if(tail >= (endSmpNr - begSmpNr - 1)) {
    tail = endSmpNr - begSmpNr - 1;
    EOR = TRUE;
  }
  if(getSmpPtr(smpDOp, begSmpNr, head, tail,\
	       gd->channel, workDOp) == NULL) {
    freeBufs();
    if(CREATED)
      freeDObj(f0DOp);
    return(NULL);
  }
  if(TRACE[0] && workDOp->bufStartRec != begSmpNr) {
    setAsspMsg(AEG_ERR_BUG, "computeKSV: invalid start index");
    freeBufs();
    if(CREATED)
      freeDObj(f0DOp);
    return(NULL);
  }
  start = 0;
  if(workDOp->bufStartRec + workDOp->bufNumRecs >= endSmpNr) {
    end = endSmpNr - workDOp->bufStartRec;
    EOR = TRUE;
  }
  else
    end = workDOp->bufNumRecs;
  err = 0;
  while(TRUE) {
    if(ksvExtr(&start, end, &smpNr, &mag, &type) > 0) {      /* found */
      /* NEW 190410: always ensure maximum free space in ring buffer */
      if((err=ksvConvert(smpNr, FALSE, f0DOp, prdDOp)) < 0)
	break;
      if((err=ksvTwin(smpNr, mag, type)) < 0)  /* twinning & chaining */
        break;
    }
    else {                                        /* buffer exhausted */
      if(EOR) break; /* end-of-range: WE'RE THROUGH */
      begSmpNr = workDOp->bufStartRec + workDOp->bufNumRecs;
      if(begSmpNr >= endSmpNr || begSmpNr >= absEndSn)
	break;                                        /* safety catch */
      head = smpOverlap;
      tail = workDOp->maxBufRecs - head - 1;
      if(FILE_IN) {
	if(tail >= (smpDOp->maxBufRecs - head))
	  tail = smpDOp->maxBufRecs - head - 1;
      }
      else if(tail >= (smpDOp->bufNumRecs - head))
	tail = smpDOp->bufNumRecs - head - 1;
      if(tail >= (endSmpNr - begSmpNr - 1)) {
	tail = endSmpNr - begSmpNr - 1;
	EOR = TRUE;
      }
      workDOp->bufNumRecs = 0;                 /* enforce full reload */
      if(getSmpPtr(smpDOp, begSmpNr, head, tail,\
		   gd->channel, workDOp) == NULL) {
	err = -1;
	break;
      }
      if(TRACE[0] && workDOp->bufStartRec != (begSmpNr - head)) {
	setAsspMsg(AEG_ERR_BUG, "computeKSV: invalid start index");
	err = -1;
	break;
      }
      start = smpOverlap;
      if(workDOp->bufStartRec + workDOp->bufNumRecs >= endSmpNr) {
	end = endSmpNr - workDOp->bufStartRec;
	EOR = TRUE;
      }
      else
	end = workDOp->bufNumRecs;
    }
  }
  if(err >= 0) { /* convert and store last values */
    err = ksvConvert(endSmpNr, TRUE, f0DOp, prdDOp);
    if(err >= 0 && FILE_OUT)
      err = asspFFlush(f0DOp, gd->writeOpts);
    if(err >= 0 && FILE_IN && prdDOp != NULL) {
      temp = smpDOp->startRecord + smpDOp->numRecords;
      if(SMPNRtoFRMNR(temp, f0DOp->frameDur) == gd->endFrameNr)
	storeTag("EOF", temp, prdDOp);
    }
  }
  freeBufs();
  if(err < 0) {
    if(smpDOp->filePath != NULL) {
      cPtr = &applMessage[strlen(applMessage)];
      sprintf(cPtr, "\n       for file %s",\
	      myfilename(smpDOp->filePath));
    }
    if(CREATED)
      freeDObj(f0DOp);
    return(NULL);
  }
  return(f0DOp);
}

/*DOC

Function 'verifyKSV'

Verifies whether the analysis parameters of the F0 data object pointed 
to by "f0DOp" differ from those specified in the general analysis 
parameter structure pointed to by "aoPtr" for the audio object pointed 
to by "smpDOp". None of these three pointers may be NULL. If there are 
differences, it is checked whether they lead to clashes with existing 
analysis data or header settings or require data buffers to be 
reallocated. If there are no clashes, the items in the generic data 
structure will be updated to match the general analysis parameter 
settings.
This function should be called when between a call to 'createKSV' and to 
'computeKSV', changes have been made in the analysis parameters or the 
audio object (e.g. a length change). 
Returns 0 when there are no problems, -1 upon error or +1 if there are 
warnings.

DOC*/

int verifyKSV(DOBJ *f0DOp, DOBJ *smpDOp, AOPTS *aoPtr)
{
  int     err=0;
  long    auCaps;
  double  frameRate;
  DDESC  *dd;
  KSV_GD *gd;
  ATIME   aTime, *tPtr;

  if(f0DOp == NULL || smpDOp == NULL || aoPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "verifyKSV");
    return(-1);
  }
  dd = &(f0DOp->ddl);
  if(dd->type != DT_PIT || dd->format != KSV_DFORMAT ||\
     dd->numFields != 1 || dd->next != NULL) {
    setAsspMsg(AEG_ERR_BUG, "Not a regular KSV-F0 object");
    return(-1);
  }
  gd = (KSV_GD *)(f0DOp->generic);
  if(gd == NULL ||
     (gd != NULL && strcmp(gd->ident, KSV_GD_IDENT) != 0)) {
    setAsspMsg(AEG_ERR_BUG, "KSV generic data missing or invalid");
    return(-1);
  }
  clrAsspMsg();
  /* verify audio object */
  if(aoPtr->channel < 1)
    aoPtr->channel = KSV_DEF_CHANNEL;
  if((auCaps=getSmpCaps(KSV_PFORMAT)) <= 0)
    return(-1);
  auCaps |= KSV_I_CHANS;
  if(checkSound(smpDOp, auCaps, aoPtr->channel) <= 0)
    return(-1);
  /* ensure correct interpretation by anaTiming() */
  aoPtr->msSize = -1.0;                       /* no fixed window size */
  aoPtr->options &= ~AOPT_EFFECTIVE;
  tPtr = &aTime;
  if((err=anaTiming(smpDOp, aoPtr, tPtr)) < 0)
    return(-1);
  if(f0DOp->sampFreq != tPtr->sampFreq ||\
     f0DOp->frameDur != tPtr->frameShift) {
    frameRate = tPtr->sampFreq / (double)(tPtr->frameShift);
    if(f0DOp->dataRate != frameRate) {
      if(f0DOp->fp != NULL &&\
	 (f0DOp->numRecords > 0 || ftell(f0DOp->fp) != 0)) {
	setAsspMsg(AED_ERR_RATE, f0DOp->filePath);
	return(-1);
      }
      clearDataBuf(f0DOp);/* contents invalid; size may be sufficient */
    }
    f0DOp->sampFreq = tPtr->sampFreq;
    f0DOp->frameDur = tPtr->frameShift;
    f0DOp->dataRate = frameRate;
    f0DOp->startRecord = tPtr->begFrameNr;
    f0DOp->numRecords = 0;
    setStart_Time(f0DOp);
  }
  if(aoPtr->channel != gd->channel) {
    if(f0DOp->fp != NULL &&\
       (f0DOp->numRecords > 0 || ftell(f0DOp->fp) != 0)) {
      setAsspMsg(AEG_ERR_APPL, "verifyKSV: can't change channel "\
		 "in existing data");
      return(-1);
    }
    clearDataBuf(f0DOp);                       /* contents invalid */
  }
  if(tPtr->begFrameNr < f0DOp->startRecord) {
    if(f0DOp->fp != NULL &&\
       (f0DOp->numRecords > 0 || ftell(f0DOp->fp) != 0)) {
      setAsspMsg(AEG_ERR_APPL, "verifyKSV: can't change start time "\
		 "in existing data");
      return(-1);
    }
    f0DOp->startRecord = tPtr->begFrameNr;
    f0DOp->numRecords = 0;
    setStart_Time(f0DOp);
  }
  /* now copy all relevant parameters */
  gd->options = aoPtr->options;
  gd->begFrameNr = tPtr->begFrameNr;
  gd->endFrameNr = tPtr->endFrameNr;
  gd->minF0 = aoPtr->minF;
  gd->maxF0 = aoPtr->maxF;
  gd->voiMag = aoPtr->voiMag;
  gd->voiZCR = aoPtr->voiZCR;
  gd->channel = aoPtr->channel;
  gd->precision = aoPtr->precision;
  return(err);
}

/*DOC

Function 'freeKSV_GD'

Returns all memory allocated for the generic data in a KSV F0 data object.

DOC*/

void freeKSV_GD(void *generic)
{
  if(generic != NULL) {
    free(generic);
  }
  return;
}

/* ======================= private  functions ======================= */

/***********************************************************************
* set local global values                                              *
***********************************************************************/
LOCAL int setGlobals(DOBJ *f0DOp)
{
  long    frameShift, temp;
  double  sampFreq;
  KSV_GD *gd=(KSV_GD *)(f0DOp->generic);

  /* clear pointers to buffers, tables and workspace */
  extrBuf[0] = extrBuf[1] = NULL;
  twinBuf = NULL;
  ringBuf = NULL;
  minPdT = maxPdT = minPdC = maxPdC = NULL;
  workDOp = NULL;
  /* initialize times */
  sampFreq = f0DOp->sampFreq;
  frameShift = f0DOp->frameDur;
  begSmpNr = FRMNRtoSMPNR(gd->begFrameNr, frameShift);
  endSmpNr = FRMNRtoSMPNR(gd->endFrameNr, frameShift);
  ringBsn = ringEsn = begSmpNr;
  ringHead = 0;
  lastUsed = aliveIndex = -1;
  VOICED = FALSE;
  /*
   * set dependent global constants; determine buffer sizes
   * Note: ringLength must be an integral number of frames
   */
  minPrdLen = (int)FREQtoPERIOD(gd->maxF0, sampFreq);
  if(minPrdLen < 2)                     /* must be below Nyquist rate */
    minPrdLen = 2;
  maxPrdLen = (int)FREQtoPERIOD(gd->minF0, sampFreq);
  if(maxPrdLen <= minPrdLen)
    maxPrdLen = minPrdLen + 1;
  maxLenTwin = 2 * maxPrdLen;             /* maximum duration of twin */
  smpOverlap = maxLenTwin;        /* for AMV calculation in ksvTwin() */
  maxExtrema = maxLenTwin/minPrdLen + 2;   /* number of valid extrema */
  maxTwins = (int)PERIODtoFREQ(2*minPrdLen, sampFreq);   /* heuristic */
  minVoiced = (frameShift+1) / 2;    /* at least half of frame voiced */
  /*
   * ensure sufficient delay from ring to output buffer to install a
   * birth-ready chain plus a new twin.
   * NEW 1992: installation might be delayed by a reliable alive chain.
   */
  minDurChain = (int)TIMEtoSMPNR(KSV_MIN_DUR/1000.0, sampFreq);
  outputDelay = minDurChain;
  temp = KSV_MIN_TWIN * maxLenTwin;
  if(outputDelay < temp)
    outputDelay = (int)temp;
  temp = (long)ceil(KSV_MIN_PRD*(double)maxPrdLen);
  if(outputDelay < temp)
    outputDelay = (int)temp;
  temp = 3 * minDurChain;         /* NEW 1992: delayed by alive chain */
  if(outputDelay < temp)
    outputDelay = (int)temp;
  outputDelay += maxLenTwin;
  temp = SMPNRctFRMNR(outputDelay, frameShift); /* round up to frames */
  temp++;                 /* increment, so we can shift out one frame */
  outputDelay = (int)(temp * frameShift);  /* convert back to samples */
  temp = SMPNRctFRMNR(maxLenTwin, frameShift);
  if(temp < 4)       /* some extra space in ring for inhibited chains */
    temp = 4;
  ringLength = outputDelay + temp * frameShift;
  return(0);
}
/***********************************************************************
* verify validity of label object for period markers; if necessary,    *
* remove existing tags in analysis interval                            *
***********************************************************************/
LOCAL int checkTags(DOBJ *prdDOp, DOBJ *smpDOp)
{
  LABEL   *lPtr=NULL;
  XLBL_GD *gd=NULL;

  if(prdDOp->recordSize > 0 || prdDOp->fileData != FDF_ASC ||\
     (prdDOp->fileFormat != FF_IPDS_M && prdDOp->fileFormat != FF_XLABEL)) {
    setAsspMsg(AEF_ERR_FORM, "KSV: checkTags");
    return(-1);
  }
  if(prdDOp->sampFreq <= 0.0)
    prdDOp->sampFreq = smpDOp->sampFreq;
  else if(myrint(prdDOp->sampFreq) != myrint(smpDOp->sampFreq)) {
    if(prdDOp->fp == NULL && prdDOp->dataBuffer != NULL) {
      setAsspMsg(AEG_ERR_BUG, "KSV: sample rate mismatch in period markers");
      return(-1);
    }
    prdDOp->sampFreq = smpDOp->sampFreq;
  }
  if(prdDOp->fp != NULL) {
    gd = (XLBL_GD *)(prdDOp->generic);
    if(prdDOp->headerSize <= 0 ||\
       (prdDOp->fileFormat == FF_XLABEL &&\
	(gd == NULL ||\
	 (gd != NULL && strcmp(gd->ident, XLBL_GD_IDENT) != 0)))) {
      setAsspMsg(AEG_ERR_BUG, "KSV: faulty label object for period markers");
      return(-1);
    }
    if(fseek(prdDOp->fp, prdDOp->headerSize, SEEK_SET) < 0) {
      setAsspMsg(AEF_ERR_SEEK, prdDOp->filePath);
      return(-1);
    }
  }
  else if(prdDOp->dataBuffer != NULL) {
    lPtr = (LABEL *)(prdDOp->dataBuffer);
    while(lPtr != NULL) {
      if(lPtr->smpNr < 0 || (lPtr->smpNr == 0 && lPtr->time > 0.0))
	lPtr->smpNr = TIMEtoSMPNR(lPtr->time, prdDOp->sampFreq);
      if(lPtr->smpNr >= begSmpNr && lPtr->smpNr < endSmpNr)
	lPtr = delLabel(prdDOp, lPtr);
      else {
	lPtr->time = SMPNRtoTIME(lPtr->smpNr, prdDOp->sampFreq);
	lPtr = nextLabel(lPtr);
      }
    }
  }
  return(0);
}
/***********************************************************************
* allocate memory for the global buffers, tables and workspace         *
***********************************************************************/
LOCAL int allocBufs(DOBJ *smpDOp)
{
  int    i;
  long   numRecords;
  double dbli, minT, maxT, minC, maxC;
  DDESC *dd;

  extrBuf[0] = (EXTR *)calloc(2*(size_t)maxExtrema, sizeof(EXTR));
  twinBuf = (TWIN *)calloc((size_t)maxTwins, sizeof(TWIN));
  ringBuf = (PRDS *)calloc((size_t)ringLength, sizeof(PRDS));
  minPdT = (int *)calloc((size_t)(maxPrdLen+1), sizeof(int));
  maxPdT = (int *)calloc((size_t)(maxPrdLen+1), sizeof(int));
  minPdC = (int *)calloc((size_t)(maxPrdLen+1), sizeof(int));
  maxPdC = (int *)calloc((size_t)(maxPrdLen+1), sizeof(int));
  workDOp = allocDObj();
  if(extrBuf[0] == NULL || twinBuf == NULL || ringBuf == NULL ||
     minPdT == NULL || maxPdT == NULL || minPdC == NULL || maxPdC == NULL ||
     workDOp == NULL) {
    freeBufs();
    setAsspMsg(AEG_ERR_MEM, "KSV: allocBufs");
    return(-1);
  }
  extrBuf[1] = &extrBuf[0][maxExtrema]; /* set pointer for second buf */
  numExtr[0] = numExtr[1] = 0;   /* other buffers cleared by calloc() */
  /* initialize tolerance tables */
  for(i = 0; i < minPrdLen; i++)
    minPdT[i] = maxPdT[i] = minPdC[i] = maxPdC[i] = minPrdLen;
  minT = 1.0 - KSV_PDTOLT;
  maxT = 1.0 + KSV_PDTOLT;
  minC = 1.0 - KSV_PDTOLC;
  maxC = 1.0 + KSV_PDTOLC;
  for(i = minPrdLen; i <= maxPrdLen; i++) {
    dbli = (double)i;
    minPdT[i] = (int)(minT*dbli - 0.5);  /* shortest allowed duration */
    if(minPdT[i] < minPrdLen)
      minPdT[i] = minPrdLen;                                  /* clip */
    minPdC[i] = (int)(minC*dbli - 0.5);       /* same for alive chain */
    if(minPdC[i] < minPrdLen)
      minPdC[i] = minPrdLen;
    maxPdT[i] = (int)(maxT*dbli + 0.5);   /* longest allowed duration */
    if(maxPdT[i] > maxPrdLen)
      maxPdT[i] = maxPrdLen;                                  /* clip */
    maxPdC[i] = (int)(maxC*dbli + 0.5);
    if(maxPdC[i] > maxPrdLen)
      maxPdC[i] = maxPrdLen;
  }
  /* initialize work object */
  if(copyDObj(workDOp, smpDOp) < 0) {
    freeBufs();
    return(-1);
  }
  dd = &(workDOp->ddl);
  dd->coding = DC_LIN;
  dd->format = KSV_PFORMAT;
  dd->numFields = KSV_O_CHANS;
  setRecordSize(workDOp);
  if(smpDOp->fp != NULL)
    numRecords = smpDOp->maxBufRecs;       /* should be about optimal */
  else { /* MEMORY_IN */
    numRecords = ANA_BUF_BYTES / workDOp->recordSize;
    if(numRecords < 4 * smpOverlap)
      numRecords = 4 * smpOverlap;
  }
  if(numRecords > endSmpNr - begSmpNr)
    numRecords = endSmpNr - begSmpNr;
  if(allocDataBuf(workDOp, numRecords) == NULL) {
    freeBufs();
    return(-1);
  }
  return(0);
}
/***********************************************************************
* free memory allocated for the global buffers, tables and workspace   *
***********************************************************************/
LOCAL void freeBufs(void)
{
  if(extrBuf[0] != NULL) {
    free((void *)(extrBuf[0]));
    extrBuf[0] = extrBuf[1] = NULL;
  }
  if(twinBuf != NULL) {
    free((void *)twinBuf);
    twinBuf = NULL;
  }
  if(ringBuf != NULL) {
    free((void *)ringBuf);
    ringBuf = NULL;
  }
  if(minPdT != NULL) {
    free((void *)minPdT);
    minPdT = NULL;
  }
  if(maxPdT != NULL) {
    free((void *)maxPdT);
    maxPdT = NULL;
  }
  if(minPdC != NULL) {
    free((void *)minPdC);
    minPdC = NULL;
  }
  if(maxPdC != NULL) {
    free((void *)maxPdC);
    maxPdC = NULL;
  }
  if(workDOp != NULL) {
    workDOp = freeDObj(workDOp);
  }
  return;
}
/***********************************************************************
* Extremum detector: Searches until either a generalized maximum or a  *
* generalized minimum is found, or end-of-buffer is reached.           *
* Returns 1 if extremum found or 0 if end-of-buffer reached.           *
* Differences with respect to paper:                                   *
* - initialization if start = 0 (only occurs at first read because     *
*   of required overlap in next reads)                                 *
* - check on left side removed (rejects too many extrema in decaying   *
*   envelope)                                                          *
* - thresholding included in search (assumes that the speech signal    *
*   does not have a DC-offset)                                         *
***********************************************************************/
LOCAL int ksvExtr(long *start, register long end,\
		  long *sn, float *mag, int *type)
{
  static long  distMax, distMin;         /* keep distances to extrema */
  static float ampMax, ampMin;       /* and current maxima and minima */
  register long   i;
  register float *sPtr=(float *)(workDOp->dataBuffer);
  float   currSample, nextSample;
  KSV_GD *gd=(KSV_GD *)(workDOp->generic);  /* copy of f0DOp generics */

  if(*start == 0) {                       /* initialization condition */
    ampMax = (float)(gd->voiMag);                 /* set to threshold */
    ampMin = -ampMax;
    distMax = distMin = LONG_MIN;          /* long time in the future */
  }
  sPtr = &sPtr[*start];            /* initialize pointer to workspace */
  nextSample = *(sPtr++);                       /* fetch first sample */
  for(i = (*start) + 1; i < end; i++) {
    currSample = nextSample;
    nextSample = *(sPtr++);                      /* fetch next sample */
    distMax++;                                    /* update distances */
    distMin++;
    if(currSample >= ampMax && currSample > nextSample) {  /* MAXIMUM */
      ampMax = currSample;                          /* keep amplitude */
      distMax = 1;                          /* init distance i-to-max */
    }
    else if(currSample <= ampMin && currSample < nextSample) {
      ampMin = currSample;
      distMin = 1;
    }
    if(distMax >= minPrdLen) { /* generalized maximum */
      *start = i;                        /* continue search from here */
      *sn = workDOp->bufStartRec + i - distMax;      /* sample number */
      *mag = ampMax;                                     /* magnitude */
      *type = KSV_MAX;                               /* extremum type */
      ampMax = (float)(gd->voiMag);             /* reset to threshold */
      distMax = LONG_MIN;                       /* avoid false alarms */
      return(1);
    }
    if(distMin >= minPrdLen) { /* generalized minimum */
      *start = i;
      *sn = workDOp->bufStartRec + i - distMin;
      *mag = -ampMin;
      *type = KSV_MIN;
      ampMin = -((float)(gd->voiMag));
      distMin = LONG_MIN;
      return(1);
    }
  }
  return(0); /* end-of-buffer reached */
}
/***********************************************************************
* Attempts to group new extremum with two others of the same type to   *
* form a period twin. Calls chaining routine for each twin found.      *
* Differences with respect to paper:                                   *
* - thresholding already done in extremum detection routine            *
* - improbable extrema removed (HI-LO-HI condition)                    *
* - no minPrdLen-check (extrema of same type can never be that close)  *
* - fastest, simplest, twin tests performed first                      *
* - damped oscillation instead of sustaining columns test              *
* - added test on zero-crossing rate within period                     *
* - simpler AMV calculation on equal duration for both periods         *
***********************************************************************/
LOCAL int ksvTwin(long sn3, float a3, int type)
{
  register int   i, i1, i2, i3;
  register EXTR *ePtr;                                   /* for speed */
  long   age;                             /* for removing old extrema */
  long   sn1, sn2;                       /* sample numbers of extrema */
  int    dur12, dur23, n;                      /* duration of periods */
  int    minDur, maxDur;                       /* for tolerance check */
  float  a1, a2, amax;                         /* for amplitude tests */
  double amv12[KSV_NUM_AMV], amv23[KSV_NUM_AMV];       /* for AMV and */
  double sVar, dVar, zxRate;                            /* ZCR checks */
  KSV_GD *gd=(KSV_GD *)(workDOp->generic);  /* copy of f0DOp generics */

  /*
   * Remove too old and improbable extrema; store new one in buffer.
   */
  ePtr = &extrBuf[type][0];                            /* set pointer */
  i3 = numExtr[type];            /* initialize index for new extremum */
  if(i3 > 0) {
    if((int)(sn3-ePtr[i3-1].sn) <= maxPrdLen) { /* previous near enough */
      age = sn3 - maxLenTwin;                /* maximum valuable past */
      for(i = 0; i < i3; i++) {        /* check age of stored extrema */
        if(ePtr[i].sn >= age)                   /* keep from this one */
	  break;
      }
      if(i > 0) {                            /* extrema to be deleted */
        i1 = i3;                                    /* keep end index */
        for(i3 = 0; i < i1; i++, i3++) {
          ePtr[i3].mag = ePtr[i].mag;                 /* copy forward */
          ePtr[i3].sn = ePtr[i].sn;
        }
      }
      if(i3 > 1) {                              /* at least 3 extrema */
        a2 = ePtr[i3-1].mag;
        a1 = ePtr[i3-2].mag;
        if(a2 < a3 && a2 < a1 && a2 < (a3 + a1)/4.0 )     /* HI-LO-HI */
          i3--;                                          /* remove LO */
      }
    }
    else i3 = 0;                    /* gap too large; insert as first */
  }
  if(i3 >= maxExtrema) {
    asspMsgNum = AEG_ERR_BUG;
    sprintf(applMessage, "\nOverflow in extrema buffer %d at sample #%ld",\
	    type, sn3);
    return(-1);
  }
  ePtr[i3].sn = sn3;                            /* store new extremum */
  ePtr[i3].mag = a3;
  numExtr[type] = i3 + 1;   /* current number of extrema of this type */
  /*
   * Pair new extremum with all older ones of same type and perform twin
   * tests.
   * Note: Index of new extremum given by i3. End condition i2 > 0
   * because there must be a third, older, extremum to form a twin.
   */
  a2 = 0;                                    /* initialize for TEST 2 */
  /*
   * LOOP: younger period
   */
  for(i2 = i3-1; i2 > 0; i2--) {
    sn2 = ePtr[i2].sn;            /* sample number of centre extremum */
    dur23 = (int)(sn3 - sn2);           /* duration of younger period */
    /* TEST 1: Period duration not too large. */
    if(dur23 > maxPrdLen) break;     /* no more twins possible at all */
    /* TEST 2: Period is damped oscillation. */
    if(ePtr[i2].mag <= a2) continue;       /* younger period rejected */
    a2 = ePtr[i2].mag;                /* magnitude of centre extremum */
    /* TEST 3: Acceptable zero-crossing rate. */
    if(gd->voiZCR > gd->maxF0) {
      zxRate = ksvZCR(sn2, dur23);
      if(zxRate > gd->voiZCR) {                /* rejected: too noisy */
#ifndef WRASSP
	if(TRACE['R'])
	  fprintf(traceFP, "period %ld - %ld rejected on ZCR %.0f\n",\
		  sn2, sn3, zxRate);
#endif
	continue;
      }
    }
    minDur = minPdT[dur23];              /* shortest allowed duration */
    maxDur = maxPdT[dur23];               /* longest allowed duration */
    /*
     * LOOP: older period
     */
    for(i1 = i2-1; i1 >= 0; i1--) {
      sn1 = ePtr[i1].sn;          /* sample number of oldest extremum */
      dur12 = (int)(sn2 - sn1);           /* duration of older period */
      /* TEST 4: Similar period durations (also TEST 1 on older period). */
      if(dur12 < minDur) continue;          /* older period too short */
      if(dur12 > maxDur)
	break;             /* no more twins possible with i2 extremum */
      /* TEST 5: Limited variation in amplitude envelope (fixed threshold). */
      a1 = ePtr[i1].mag;              /* magnitude of oldest extremum */
      amax = a3 + a1;                  /* max = 2 * average magnitude */
      if(a2 > amax || a2 < amax/4.0)   /* min = average magnitude / 2 */
	continue;                             /* centre out of bounds */
      /* TEST 2 on older period */
      for(i = i1+1; i < i2; i++) {
        if(ePtr[i].mag >= a1) break;
      }
      if(i < i2) continue;                   /* older period rejected */
      /* TEST 6: Similar waveforms in both periods. */
      if(dur12 < dur23)    {                /* take shortest duration */
        n = ksvAMV(sn1, dur12, amv12);
        ksvAMV(sn2, dur12, amv23);
      }
      else {
        n = ksvAMV(sn1, dur23, amv12);
        ksvAMV(sn2, dur23, amv23);
      }
      for(sVar = dVar = 0, i = 0 ; i < n; i++) {
        sVar += (fabs(amv12[i]) + fabs(amv23[i]));
        dVar += fabs(amv12[i] - amv23[i]);
      }
      dVar += dVar;                    /* 2*dVar (inverse from paper) */
      if(dVar > sVar) {             /* rejected: AMV's too dissimilar */
#ifndef WRASSP
	if(TRACE['r']) {
	  fprintf(traceFP, "twin %ld - %ld - %ld rejected on AMV\n",\
		  sn1, sn2, sn3);
	}
#endif
	continue;
      }
      /*
       * Twin accepted! Now install in chain buffer.
       */
      if(ksvChain(sn1, sn2, sn3, type) < 0)
        return(-1);
    }
  }
  return(0);
}
/***********************************************************************
* NEW: 030610: Estimates the zero-crossing rate within a period.       *
***********************************************************************/
LOCAL double ksvZCR(long bsn, register int dur)
{
  register int    i, POS, numZX;
  register float *sPtr;
  
  i = (int)(bsn - workDOp->bufStartRec);
  sPtr = (float *)(workDOp->dataBuffer);
  sPtr = &sPtr[i];               /* pointer to first sample of period */
  POS = (*(sPtr++) >= 0);
  for(numZX = 0, i = 1; i < dur; i++) {
    if(*(sPtr++) >= 0) {
      if(!POS) {
	POS = TRUE;
	numZX++;
      }
    }
    else if(POS) {
      POS = FALSE;
      numZX++;
    }
  }
  if(numZX > 2)
    return(PERIODtoFREQ(2.0 * (double)dur/(double)(numZX-1),\
			workDOp->sampFreq));
  return(0.0);
}
/***********************************************************************
* Calculates short-term Average Magnitude Variation of the speech      *
* signal in 'KSV_NUM_AMV' equal-length segments of a period.           *
* Differences with respect to paper:                                   *
* - the AMV values are not divided by the segment length because that  *
*   is now the same for both periods                                   *
* - ToDo: set a minimum to the slot size, e.g. 3 samples and return    *
*         the number of slots used                                     *
***********************************************************************/
LOCAL int ksvAMV(long bsn, int dur, double amv[])
{
  register int     i, j, n, slot;
  register float  *sPtr;
  register double *amvPtr;
  double sum, mean;
  
  n = KSV_NUM_AMV;
  slot = dur / n;                             /* slot size in samples */
  i = (int)(bsn - workDOp->bufStartRec);
  sPtr = (float *)(workDOp->dataBuffer);
  sPtr = &sPtr[i];               /* pointer to first sample of period */
  for(amvPtr = amv, mean = 0.0, i = 0; i < n; i++) {
    for(sum = 0.0, j = 0; j < slot; j++)
      sum += fabs((double)*(sPtr++));
    *(amvPtr++) = sum;
    mean += sum;
  }
  mean /= n;                                              /* mean AMV */
  for(amvPtr = amv, i = 0; i < n; i++)
    *(amvPtr++) -= mean;                                 /* normalize */
  return(n);
}
/***********************************************************************
* Attempts to connect newly detected period twin with a previous one   *
* to form a continuous chain.                                          *
* Differences with respect to paper:                                   *
* - buffer time mark is sample number of last extremum                 *
* - best-match search for append and simpler perfect-match condition   *
* - birth-inhibiting limited to "outputDelay" duration                 *
* - overwrite of ringbuffer if new chain is preferred over alive one   *
*   delayed to avoid short erroneous chains disrupting valid chains    *
***********************************************************************/
LOCAL int ksvChain(long sn1, long sn2, long sn3, int type)
{
  register int i, j, k, dur12, dur23;
  register TWIN *tPtr;
  void *tmpPtr;
  int   APPEND, CURRENT, minDur, maxDur, diff, bestDiff, bestNdx;
  long  age;
  
  dur12 = (int)(sn2 - sn1);               /* duration of older period */
  dur23 = (int)(sn3 - sn2);             /* duration of younger period */
  minDur = minPdC[dur12];                /* shortest allowed duration */
  maxDur = maxPdC[dur12];                 /* longest allowed duration */
  /*
   * Check age of chains & release if too old; note last used element 
   * of twinBuf.
   */
/*   j = lastUsed; */
/*   for(tPtr = twinBuf, i = 0; i <= j; i++, tPtr++) { */
  lastUsed = -1;
  for(tPtr = twinBuf, i = 0; i < maxTwins; i++, tPtr++) {
    if(tPtr->flags & KSV_IN_USE) {                    /* used element */
      lastUsed = i;                     /* keep track of last element */
      if(tPtr->flags & KSV_EOC) {                   /* end of a chain */
	age = sn3 - tPtr->sn3;
        if(age > 5*(tPtr->dur23) ||
	   (i != aliveIndex && age > 3*(tPtr->dur23)) )
          clrChain(i);          /* release element i and predecessors */
      }
    }
  }
  /*
   * Verify whether new twin can be appended to alive chain. A gap 
   * between the twin and the chain (maximally one period) is hereby 
   * allowed to accomodate phase jumps e.g. on a vowel-nasal boundary.
   */
  APPEND = CURRENT = FALSE;
  if((i=aliveIndex) >= 0) {                /* there is an alive chain */
    j = twinBuf[i].dur23;                  /* duration of last period */
    if((int)(sn1-twinBuf[i].sn3) <= maxPdT[dur12]) {
      if(j >= minDur && j <= maxDur)       /* period within tolerance */
        APPEND = TRUE;
      else if((twinBuf[i].sn3-j) == sn2 && twinBuf[i].dur12 == dur12)
	APPEND = TRUE;               /* perfect match on elder period */
    }
  }
  /*
   * If unsuccessful, try to append to another chain.
   */
  if(!APPEND) {
    bestDiff = maxPrdLen + 1;     /* initialize for best-match search */
    bestNdx = -1;
    for(tPtr = twinBuf, i = 0; i <= lastUsed; i++, tPtr++) {
      if(!(tPtr->flags & KSV_EOC) ||              /* not end of chain */
	 i == aliveIndex ||                        /* already checked */
	 tPtr->sn3 < sn1)                          /* no gaps allowed */
	continue;
      j = tPtr->dur23;                  /* copy to register for speed */
      if(j < minDur || j > maxDur) continue;        /* too dissimilar */
      if((sn2 == tPtr->sn3 && dur12 == j) ||
	 (sn2 == (tPtr->sn3-j) && tPtr->dur12 == dur12)) {
        APPEND = TRUE;                               /* perfect match */
        break;                                         /* stop search */
      }
      diff = abs(dur12 - j);
      if(diff < bestDiff) {
        bestDiff = diff;                         /* search best match */
        bestNdx = i;
      }
      if(tPtr->type != type) {                 /* if different types: */
        j = tPtr->link;                        /* go through links in */
        while(j >= 0) {                    /* search of perfect match */
          if(twinBuf[j].sn3 < sn2) break;             /* too far back */
          if(sn2 == twinBuf[j].sn3 && dur12 == twinBuf[j].dur23) {
            APPEND = TRUE;                           /* perfect match */
            break;                                     /* stop search */
          }
          j = twinBuf[j].link;
        }
      }
      if(APPEND) break;                    /* perfect match overrules */
    }
    if(!APPEND && (bestNdx >= 0)) {               /* best match found */
      APPEND = TRUE;
      i = bestNdx;
    }
  }
  /*
   * Install twin: If previous linking test delivers APPEND = TRUE,
   * it also gives the element i in the twin buffer that contains 
   * the predecessor.
   */
  /* search an unused element */
  for(tPtr = twinBuf, j = 0; j < maxTwins; j++, tPtr++) {
    if(tPtr->flags == KSV_UNUSED) break;
  }
  if(j >= maxTwins) {                  /* should not but could happen */
    j = maxTwins;                                   /* just make sure */
#ifndef WRASSP
    if(TRACE['x']) {
      fprintf(traceFP, "extending twin buffer at sample #%ld\n",\
	      tPtr->sn3);
    }
#endif
    tmpPtr = realloc((void *)twinBuf, (size_t)(maxTwins+1)*sizeof(TWIN));
    if(tmpPtr == NULL) {
      setAsspMsg(AEG_ERR_MEM, "while trying to extend twin buffer");
      return(-1);
    }
    twinBuf = (TWIN *)tmpPtr; /* no need to clear: will be filled out */
    maxTwins++;
    tPtr = &twinBuf[maxTwins-1];        /* set pointer to new element */
  }
  if(j > lastUsed) lastUsed = j;
  tPtr->dur12 = dur12;                                /* install twin */
  tPtr->dur23 = dur23;
  tPtr->sn3 = sn3;
  tPtr->type = type;
  tPtr->flags = KSV_IN_USE + KSV_EOC;
  if(APPEND) {                            /* append to existing chain */
    tPtr->link = i;                       /* i identifies predecessor */
    tPtr->nTwins = twinBuf[i].nTwins + 1; /* number of twins in chain */
    tPtr->totDur = twinBuf[i].totDur + (sn3 - twinBuf[i].sn3); /* duration */
    twinBuf[i].flags &= ~KSV_EOC; /* predecessor no longer end of chain */
  }
  else {                                         /* install new chain */
    tPtr->link = -1;
    tPtr->nTwins = 1;
    tPtr->totDur = sn3 - sn1;
  }
  /*
   * Output of new twin if appended to alive chain, or if appending 
   * increases its chain beyond the birthsize limit. If twin not 
   * appended, no output. Index of the new twin is given by 'j', 
   * that of its predecessor by 'i'.
   */
  if(APPEND) {
    if(i == aliveIndex) { /* previous element was last of alive chain */
      if(putChain(j, FALSE) < 0)        /* update ring with element j */
	return(-1);                                     /* some error */
      /* Update and resets AFTER write, otherwise cannot detect gap! */
      aliveIndex = j;                  /* set alive index to new twin */
      tPtr->link = -1;                                  /* clear link */
      twinBuf[i].flags = KSV_UNUSED;           /* release predecessor */
      CURRENT = TRUE;                         /* alive chain extended */
    }
    else {                           /* check 'birthready' conditions */
      if((tPtr->totDur >= minDurChain) &&
	 (tPtr->nTwins >= KSV_MIN_TWIN) &&
	 (tPtr->totDur >= (long)(KSV_MIN_PRD * dur23)) ) {
	if(aliveIndex < 0) {                /* no current alive chain */
	  /*
	   * Check whether there exists a chain with a smaller end-period
	   * duration. If so, inhibit birth to avoid octave errors.
	   */
          for(k = 0; k <= lastUsed; k++) {
            if(k != j && (twinBuf[k].flags & KSV_EOC) &&
	       twinBuf[k].dur23 <= dur23)
              break;
          }
          if(k > lastUsed ||             /* no inhibiting chain found */
	    tPtr->totDur >= outputDelay) {             /* NEW: 280510 */
            if(putChain(j, FALSE) < 0)  /* update ring with new chain */
              return(-1);                          /* buffer overflow */
            clrChain(i);              /* release predecessors; keep j */
            tPtr->link = -1;                            /* clear link */
            aliveIndex = j;           /* set alive index to new chain */
            CURRENT = TRUE;                        /* new alive chain */
          }
        }
        else {
	  /*
	   * If there is an alive chain but this is not it, then select
	   * the one with the shortest period duration and remove the
	   * other.
	   * NEW 1992: Delay overwriting a reliable chain in the hope
	   *           that one of the two dies out in the mean time.
	   * NEW 060111: remove sub-sub-octave chains re. alive one.
	   */
          k = aliveIndex;
	  if(twinBuf[k].dur23 <= (dur23 / 3))          /* NEW: 060111 */
	    clrChain(j);        /* release element j and predecessors */
          else if(twinBuf[k].totDur < 2*(tPtr->totDur) || /* NEW 1992 */
		  tPtr->totDur >= 3*minDurChain) {
            if(twinBuf[k].dur23 <= dur23)
              clrChain(j);      /* release element j and predecessors */
            else {
              clrChain(k);                  /* throw alive chain away */
              if(putChain(j, TRUE) < 0)   /* overwrite with new chain */
                return(-1);                             /* some error */
              clrChain(i);      /* release i and predecessors, keep j */
              tPtr->link = -1;                          /* clear link */
              aliveIndex = j;         /* set alive index to new chain */
	      CURRENT = TRUE;                      /* new alive chain */
            }
          }
        }
      }
    }
  }
#ifndef WRASSP
  if(TRACE['c'] && CURRENT) {
    tPtr = &twinBuf[aliveIndex];
    fprintf(traceFP, "current: sn3 = %ld d23 = %d d12 = %d num = %ld "\
	    "dur = %ld\n", tPtr->sn3, tPtr->dur23, tPtr->dur12,\
	    tPtr->nTwins, tPtr->totDur);
  }
#endif
  return(0);
}
/***********************************************************************
* Write chain ending with element i to ring buffer. The ring buffer    *
* contains accumulated period durations at each sample moment.         *
* Note: at least for an alive chain, it is essential that the youngest *
* twin will be written first.                                          *
***********************************************************************/
LOCAL int putChain(int i, int OVERWRITE)
{
  register int j, k, dur12, dur23;
  register TWIN *tPtr;
  uint8_t tag;
  long    bsn, esn;

  tPtr = &twinBuf[i];                                  /* set pointer */
  if(OVERWRITE) {/* clear ring from begin of new chain to end of ring */
    bsn = tPtr->sn3 - tPtr->totDur;                 /* begin of chain */
    j = (int)(bsn - ringBsn);                /* offset in ring buffer */
    if(j < 0) j = 0;                           /* warning comes later */
    j += ringHead;                             /* start index in ring */
    dur12 = (int)(ringEsn - bsn);        /* number of values to clear */
    if(dur12 > ringLength)
      dur12 = ringLength;
    for(k = 0; k < dur12; k++, j++) {
      j %= ringLength;                    /* wrap around if necessary */
      memset((void *)&ringBuf[j], 0, sizeof(PRDS));  /* clear element */
    }
    ringEsn -= dur12;
  }
  bsn = -1;/* initialize begin sample number of twin for filling gaps */
  dur12 = 0;
  while(i >= 0) {                                     /* follow links */
    tPtr = &twinBuf[i];                                /* set pointer */
    esn = tPtr->sn3;                /* end sample number of next twin */
    if((int)(esn - ringBsn) > ringLength) {    /* should never happen */
      asspMsgNum = AEG_ERR_BUG;
      sprintf(applMessage, "\nOverflow in ring buffer at sample #%ld", esn);
      return(-1);
    }
    dur23 = tPtr->dur23;             /* duration of right-hand period */
    if(bsn > esn) {
      /*
       * Gap in chain; bsn gives the begin sample number, dur12 the
       * duration of the left-hand period of the previous twin.
       */
      dur12 = (dur12 + dur23 + 1)/2;    /* mean of olddur12, newdur23 */
      j = ringHead + (int)(esn - ringBsn);      /* index begin of gap */
      for(k = (int)(bsn-esn); k > 0; k--, j++) {          /* fill gap */
	j %= ringLength;                  /* wrap around if necessary */
        ringBuf[j].val += dur12;
        ringBuf[j].cnt++;
      }
    }
    /*
     * Note: in order to be able append to an alive chain, it is
     * necessary that its last twin is still in memory. That twin,
     * however, should not be written twice. Note also that this
     * implies that the update of 'aliveIndex' in putChain() has
     * to be done AFTER the call to this function.
     */
    if(i == aliveIndex) return(0);               /* don't write twice */
    if(tPtr->type == KSV_MAX)       /* can't use type because indexed */
      tag = TAG_MAX;
    else
      tag = TAG_MIN;
    dur12 = tPtr->dur12;    /* duration of left-hand period next twin */
    bsn = esn - dur23 - dur12;    /* begin sample number of next twin */
    j = (int)(bsn - ringBsn);                /* offset in ring buffer */
    if(j < 0) {
      asspMsgNum = AWG_WARN_BUG;
      sprintf(applMessage, "\nunderflow of ring buffer: bsn = %ld"\
	      " ringBsn = %ld", bsn, ringBsn);
      return(1);
    }
    j += ringHead;                             /* start index in ring */
    j %= ringLength;                      /* wrap around if necessary */
    ringBuf[j].tag = tag;                              /* mark period */
    for(k = 0; k < dur12; k++) {                /* write first period */
      ringBuf[j].val += dur12;
      ringBuf[j].cnt++;
      j++;
      j %= ringLength;                    /* wrap around if necessary */
    }
    ringBuf[j].tag = tag;                              /* mark period */
    for(k = 0; k < dur23; k++) {               /* write second period */
      ringBuf[j].val += dur23;
      ringBuf[j].cnt++;
      j++;
      j %= ringLength;                    /* wrap around if necessary */
    }
    ringBuf[j].tag = tag;                              /* mark period */
    /* BEWARE! this marker may be at an unvoiced sample */
    /*         and even past ringEsn */
    if(esn > ringEsn)             /* update end sample number of ring */
      ringEsn = esn;
    i = tPtr->link;                               /* get next element */
  }
  return(0);
}
/***********************************************************************
* Remove element i and all predecessors from twin buffer.              *
***********************************************************************/
LOCAL void clrChain(register int i)
{
  if(i == aliveIndex)
    aliveIndex = -1;                             /* clear alive index */
  while(i >= 0) {                     /* continue until no more links */
    twinBuf[i].flags = KSV_UNUSED;        /* element no longer in use */
    if(i == lastUsed)                   /* keep track of last element */
      lastUsed--;
    i = twinBuf[i].link;                             /* get next link */
  }
  return;
}
/***********************************************************************
* When enough data available in ring buffer: Decimate period data,     *
* convert to F0-values and store values in output data object.         *
* Special mode for finalizing.                                         *
* NEW 190410: always convert until smpNr is less than one frame from   *
* end of ring; no need to go through every element if ring empty;      *
* implies that this function is called BEFORE ksvTwin()                *
***********************************************************************/
LOCAL int ksvConvert(long smpNr, int FINISH, DOBJ *f0DOp, DOBJ *tagDOp)
{
  register int  j, vcnt;
  register long i, cnt, sum;
  int   CONVERT, TAGS_OUT;
  long  tagSn, frameNr, frameShift;
  float F0;
  
  frameShift = f0DOp->frameDur;
  frameNr = SMPNRtoFRMNR(ringBsn, frameShift);
  if(FINISH)
    CONVERT = (ringBsn < endSmpNr);
  else
    /* CONVERT = ((int)(smpNr - ringBsn) >= outputDelay) ? TRUE : FALSE; */
    /* NEW 190410: use ringLength rather than outputDelay */
    CONVERT = (smpNr > (ringBsn + ringLength - frameShift));
  TAGS_OUT = (tagDOp != NULL);
  tagSn = ringBsn;
  while(CONVERT) { /* convert and store one frame */
    if(ringBsn >= ringEsn) { /* NEW 190410: ring empty */
      F0 = 0.0;          /* no need to clear ring; set frame unvoiced */
      if(TAGS_OUT && VOICED) {                 /* seldom but possible */
	/* fix for bug #3066007: there will be a period marker left */
	j = ringHead % ringLength;
	if(ringBuf[j].tag != TAG_NONE) {
	  if(ringBuf[j].tag == TAG_MAX)
	    storeTag("max", tagSn, tagDOp);
	  else 
	    storeTag("min", tagSn, tagDOp);
	  ringBuf[j].tag = TAG_NONE;                    /* clear flag */
	}
	VOICED = FALSE;
	storeTag("EOV", tagSn, tagDOp);
	/* Note: this can only happen once so no need to update tagSn */
      }
    }
    else {
      vcnt = 0;
      sum = cnt = 0;
      for(j = ringHead, i = 0; i < frameShift; i++, j++) {
	j %= ringLength;                  /* wrap around if necessary */
	if(TAGS_OUT && ringBuf[j].tag != TAG_NONE) {
	  /* output period marker */
	  if(!VOICED) {                      /* mark begin of voicing */
	    VOICED = TRUE;
	    storeTag("BOV", tagSn, tagDOp);
	  }
	  if(ringBuf[j].tag == TAG_MAX)
	    storeTag("max", tagSn, tagDOp);
	  else 
	    storeTag("min", tagSn, tagDOp);
	}
	if(ringBuf[j].cnt > 0) {
	  vcnt++;                            /* another voiced sample */
	  sum += ringBuf[j].val;
	  cnt += ringBuf[j].cnt;
	}
	else if(TAGS_OUT && VOICED) {          /* mark end of voicing */
	  VOICED = FALSE;
	  storeTag("EOV", tagSn, tagDOp);
	}
	memset((void *)&ringBuf[j], 0, sizeof(PRDS));/* clear element */
	tagSn++;                              /* update sample number */
      }
      if(vcnt >= minVoiced) /* calculate F0 from mean period duration */
	F0 = (float)PERIODtoFREQ((double)sum/(double)cnt, f0DOp->sampFreq);
      else
	F0 = 0.0;                                   /* unvoiced frame */
    }
    if(storeF0(F0, frameNr, f0DOp) < 0)  /* transfer to output object */
      return(-1);
    frameNr++;
    ringBsn += frameShift;
    ringHead += frameShift;                    /* update ring pointer */
    ringHead %= ringLength;               /* wrap around if necessary */
    if(FINISH)                    /* repeat conditions for converting */
      CONVERT = (ringBsn < endSmpNr);
    else
      CONVERT = (smpNr > (ringBsn + ringLength - frameShift));
  }
  if(ringEsn < ringBsn)
    ringEsn = ringBsn;
  return(0);
}
/***********************************************************************
* copy F0 value to output buffer; handle data writes                   *
***********************************************************************/
LOCAL int storeF0(float F0, long frameNr, DOBJ *f0DOp)
{
  int     FILE_OUT;
  long    ndx;
  float  *fPtr;
  KSV_GD *gd=(KSV_GD *)(f0DOp->generic);

  FILE_OUT = (f0DOp->fp != NULL);
  if(f0DOp->bufNumRecs <= 0) {
    f0DOp->bufNumRecs = 0;
    f0DOp->bufStartRec = frameNr;
  }
  else if(frameNr >= (f0DOp->bufStartRec + f0DOp->maxBufRecs)) {
    if(FILE_OUT) {
      if(asspFFlush(f0DOp, gd->writeOpts) < 0)
	return(-1);
    }
    else {
      setAsspMsg(AEG_ERR_BUG, "storeF0: buffer overflow");
      return(-1);
    }
  }
  ndx = frameNr - f0DOp->bufStartRec;
  fPtr = (float *)(f0DOp->dataBuffer);
  fPtr[ndx] = F0;
  if(ndx >= f0DOp->bufNumRecs)
    f0DOp->bufNumRecs = ndx + 1;
  f0DOp->bufNeedsSave = TRUE;
  return(0);
}
/***********************************************************************
* write tag to file or add to list in the data buffer                  *
***********************************************************************/
LOCAL int storeTag(char *name, long smpNr, DOBJ *tagDOp)
{
  char     lineBuf[128];
  int      nd=0;
  double   time;
  LABEL   *tag=NULL;
  XLBL_GD *gd=NULL;

  time = SMPNRtoTIME(smpNr, tagDOp->sampFreq);
  if(tagDOp->fp != NULL) {
    nd = numDecim(1.0 / (tagDOp->sampFreq), 9);
    if(tagDOp->fileFormat == FF_IPDS_M)
      sprintf(lineBuf, MIX_LBL_STR_AP, smpNr + 1, name,\
	      (long)floor(time*100.0) + 1, nd, time);
    else { /* FF_XLABEL */
      gd = (XLBL_GD *)(tagDOp->generic);
      sprintf(lineBuf, XLBL_STR_AP, nd, time, gd->color, name);
    }
    strcat(lineBuf, tagDOp->eol);
    if(fwrite((void *)lineBuf, sizeof(char),\
	      strlen(lineBuf), tagDOp->fp) < 1) {
      setAsspMsg(AEF_ERR_WRIT, "(KSV: storeTag)");
      return(-1);
    }
  }
  else {
    tag = makeLabel(name, smpNr, time);
    if(tag == NULL)
      return(-1);
    if(addLabel(tagDOp, tag,\
		LBL_ADD_AS_LAST + LBL_ADD_AT_TIME, NULL) == NULL) {
      freeLabel((void *)tag);
      return(-1);
    }
    tagDOp->bufNeedsSave = TRUE;
  }
  return(0);
}
