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
* File:     headers.c                                                  *
* Contents: Functions for handling headers of formatted speech/audio   *
*           related data files.                                        *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: headers.c,v 1.24 2010/07/01 14:19:32 mtms Exp $ */

#include <stdio.h>      /* FILE NULL */
#include <errno.h>      /* errno */
#include <stddef.h>     /* size_t */
#include <inttypes.h>   /* uint32_t etc. */
#include <stdlib.h>     /* strtol() strtod() malloc() free() */
#include <string.h>     /* str...() mem...() */
#include <ctype.h>      /* is...() */
#include <math.h>       /* */
#include <time.h>       /* time_t time() ctime() */

#include <misc.h>       /* EOS LOCAL */
#include <mylimits.h>   /* INT8_MIN NAME_MAX */
#include <ieee.h>       /* extended floating point */
#include <asspendian.h> /* byte-order stuff */
#include <asspmess.h>   /* message codes; reference to globals */
#include <assptime.h>   /* conversion macros */
#include <dataobj.h>    /* DOBJ DDESC fform_e fdata_e */
#include <labelobj.h>   /* LBLHDR */
#include <headers.h>    /* constants and structures */
#include <aucheck.h>    /* auCapsFF() checkSound() */
#include <ipds_lbl.h>   /* IPdS MIX and SAMPA formats */
#include <esps_lbl.h>   /* ESPS xlabel format */
#include <uwm_xrmb.h>   /* Wisconsin X-ray microbeam formats */

#include <R_ext/PrtUtil.h>
/*
 * macros
 */
#define PCM_BYTES(d) ((d->numBits + 7) / 8)

/*
 * prototypes of local functions
 */
LOCAL int  isASCII(FILE *fp);
LOCAL long getFileSize(DOBJ *dop);
LOCAL int  checkXRMB(DOBJ *dop);
LOCAL int  getAIFhdr(DOBJ *dop),   putAIFhdr(DOBJ *dop);
LOCAL int  getCSLhdr(DOBJ *dop),   putCSLhdr(DOBJ *dop);
LOCAL int  getADFhdr(DOBJ *dop),   putADFhdr(DOBJ *dop);
LOCAL int  getSNDhdr(DOBJ *dop),   putSNDhdr(DOBJ *dop);
LOCAL int  getWAVhdr(DOBJ *dop),   putWAVhdr(DOBJ *dop);
LOCAL int  getKTHhdr(DOBJ *dop),   putKTHhdr(DOBJ *dop);
LOCAL int  getNISThdr(DOBJ *dop),  putNISThdr(DOBJ *dop);
LOCAL int  getSSFFhdr(DOBJ *dop),  putSSFFhdr(DOBJ *dop);
LOCAL int  getXASSPhdr(DOBJ *dop), putXASSPhdr(DOBJ *dop);
LOCAL int  getMIXhdr(DOBJ *dop),   putMIXhdr(DOBJ *dop);
LOCAL int  getSAMhdr(DOBJ *dop),   putSAMhdr(DOBJ *dop);
LOCAL int  getXLBLhdr(DOBJ *dop),  putXLBLhdr(DOBJ *dop);
LOCAL int  adjustTiming(DOBJ *dop);

/* LOCAL uint8_t  getU8(void **ptr); */
LOCAL void    *putU8(uint8_t val, void **ptr);
/* LOCAL int8_t   getI8(void **ptr); */
LOCAL void    *putI8(int8_t val, void **ptr);
LOCAL uint16_t getU16(void **ptr, int SWAP);
LOCAL void    *putU16(uint16_t val, void **ptr, int SWAP);
LOCAL int16_t  getI16(void **ptr, int SWAP);
LOCAL void    *putI16(int16_t val, void **ptr, int SWAP);
LOCAL uint32_t getU32(void **ptr, int SWAP);
LOCAL void    *putU32(uint32_t val, void **ptr, int SWAP);
LOCAL int32_t  getI32(void **ptr, int SWAP);
LOCAL void    *putI32(int32_t val, void **ptr, int SWAP);
LOCAL float    getF32(void **ptr, int SWAP);
LOCAL void    *putF32(float val, void **ptr, int SWAP);

/*
 * Sizes and names used by SSFF to store generic variables
 */
SSFFST SSFF_TYPES[] = {
  {SSFF_CHAR, "CHAR", 1},
  {SSFF_BYTE, "BYTE", 1},
  {SSFF_SHORT, "SHORT", 2},
  {SSFF_LONG, "LONG", 4},
  {SSFF_FLOAT, "FLOAT", 4},
  {SSFF_DOUBLE, "DOUBLE", 8},
  {SSFF_UNDEF, NULL, 0}
};

/*
 * keyword <-> data type tables
 * Note: different keywords may be associated with the same data type 
 * but not the other way around. When different keywords exist for the 
 * same type, the first one in the list will be the default for ASSP.
 * Beware: in some cases an abbreviated keyword is used.
 */

/*
 * SSFF
 */
KDTAB KDT_SSFF[] = {
  {"samples", NULL, NULL, DT_SMP},
  {"audio"  , NULL, NULL, DT_SMP},
  {"F0"     , NULL, "Hz", DT_PIT},
  {"pit"    , NULL, "Hz", DT_PIT},
  {"rms"    , NULL, "dB", DT_RMS},
  {"zcr"    , NULL, "Hz", DT_ZCR},
  {"ac1"    , NULL, NULL, DT_AC1},
  {"lp1"    , NULL, NULL, DT_LP1},
  {"prob"   , NULL, NULL, DT_PROB},
  {"gain"   , NULL, "dB", DT_GAIN},
  {"acf"    , NULL, NULL, DT_ACF},
  {"lpc"    , NULL, NULL, DT_LPC},
  {"rfc"    , NULL, NULL, DT_RFC},
  {"arf"    , NULL, NULL, DT_ARF},
  {"lar"    , NULL, NULL, DT_LAR},
  {"fm"     , NULL, "Hz", DT_FFR},
  {"formant", NULL, "Hz", DT_FFR},
  {"bw"     , NULL, "Hz", DT_FBW},
  {"bandwid", NULL, "Hz", DT_FBW},
  {"fa"     , NULL, "dB", DT_FAM},
  {"amp"    , NULL, "dB", DT_FAM},
  {"fta"    , NULL, NULL, DT_FTAMP},
  {"fts"    , NULL, NULL, DT_FTSQR},
  {"dft"    , NULL, "dB", DT_FTPOW},
  {"lps"    , NULL, "dB", DT_FTLPS},
  {"css"    , NULL, "dB", DT_FTCSS},
  {"cep"    , NULL, NULL, DT_FTCEP},
  {"epg"    , NULL, NULL, DT_EPG},
  {NULL     , NULL, NULL, DT_UNDEF}
};

/*
 * XASSP
 */
KDTAB KDT_XASSP[] = {
  {"LABELS"    , NULL, NULL, DT_LBL},
  {"LABELS"    , NULL, NULL, DT_TAG},
  {"FZERO"     , NULL, "Hz", DT_PIT},
  {"F0"        , NULL, "Hz", DT_PIT},
  {"ENERGY"    , NULL, "dB", DT_RMS},
  {"RMS"       , NULL, NULL, DT_RMS},
  {"PALATOGRAM", NULL, NULL, DT_EPG},
  {"EPG"       , NULL, NULL, DT_EPG},
  {"DATALOG"   , NULL, NULL, DT_DATA_LOG},
  {NULL        , NULL, NULL, DT_UNDEF}
};

/*
 * public functions
 */

/*DOC

Sets items in data object to default values for raw sampled data.

DOC*/

void setRawSMP(DOBJ *dop)
{
  DDESC *dd;

  if(dop != NULL) {
    clrAsspMsg();
    initDObj(dop);    /* zero all items in data object and descriptor */
    dop->fileFormat = FF_RAW;
    dop->fileData = FDF_BIN;
    SETENDIAN(dop->fileEndian);
    dop->sampFreq = KTH_DEF_SFR;
    dop->frameDur = 1;
    dd = &(dop->ddl);
    dd->ident = strdup("audio");
    dd->type = DT_SMP;
    dd->format = DF_INT16;
    dd->coding = DC_PCM;
    dd->numBits = 16;
    dd->numFields = 1;
    setRecordSize(dop);
  }
  return;
}

/*DOC

Tries to identify the format of the data file specified by "fp" and 
"filePath".

DOC*/

fform_e guessFormat(FILE *fp, char *filePath, fdata_e *dataFormat)
{
  char    buf[ONEkBYTE], *field[MAX_HDR_FIELDS], ident[NAME_MAX+1];
  int     n, i, asc, err;
  size_t  numBytes;
  fform_e fileFormat;

  if(fp == NULL || filePath == NULL || dataFormat == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "guessFormat");
    return(FF_ERROR);
  }
  if(fp == stdin) {
    setAsspMsg(AEG_ERR_BUG, "guessFormat: can't handle 'stdin'");
    return(FF_ERROR);
  }
  rewind(fp);
  clrAsspMsg();
  fileFormat = FF_UNDEF;
  *dataFormat = FDF_UNDEF;
  numBytes = SIZEOF_FORM;     /* == 12, sufficient for identification */
  if(fread(buf, 1, numBytes, fp) != numBytes) {
    if(ferror(fp)) {
      setAsspMsg(AEF_ERR_READ, filePath);
      return(FF_ERROR);
    }
    if(numBytes == 0) {
      setAsspMsg(AEF_EMPTY, filePath);
      return(FF_ERROR);
    }
  }
  else {            /* first check the formats with pretty unique IDs */
    buf[numBytes] = EOS;                              /* close string */
    if(strncmp(buf, AIF_FORM_ID, 4) == 0) {
      if(strncmp(&buf[8], AIFF_MAGIC, 4) == 0) {
	fileFormat = FF_AIFF;
	*dataFormat = FDF_BIN;
      }
      else if(strncmp(&buf[8], AIFC_MAGIC, 4) == 0) {
	fileFormat = FF_AIFC;
	*dataFormat = FDF_BIN;
      }
      else if(strncmp(buf, CSL_MAGIC, strlen(CSL_MAGIC)) == 0) {
	fileFormat = FF_CSL;
	*dataFormat = FDF_BIN;
      }
    }
    else if(strncmp(buf, RIFF_MAGIC, 4) == 0) {
      if(strncmp(&buf[8], WAVE_MAGIC, 4) == 0) {
	fileFormat = FF_WAVE;
	*dataFormat = FDF_BIN;
      }
    }
    else if(strcmp(buf, ADF_MAGIC) == 0) {    /* NULL-byte terminated */
      fileFormat = FF_CSRE;
      *dataFormat = FDF_BIN;
    }
    /* the following formats have ID strings > numBytes */
    else if(strncmp(buf, NIST_MAGIC, numBytes) == 0) {
      fileFormat = FF_NIST;
      *dataFormat = FDF_BIN;
    }
    else if(strncmp(buf, SSFF_MAGIC, numBytes) == 0) {
      fileFormat = FF_SSFF;
      *dataFormat = FDF_BIN;
    }
  }
  if(fileFormat == FF_UNDEF) {                  /* not identified yet */
    if((asc=isASCII(fp)) < 0) {                       /* ASC or BIN ? */
      strcpy(applMessage, filePath);                    /* read error */
      return(FF_ERROR);
    }
    if(!asc) { /* contains non 7-bit ASCII data */
      *dataFormat = FDF_BIN;
      if(strncmp(buf, SND_MAGIC, strlen(SND_MAGIC)) == 0) {
	fileFormat = FF_SND;
      }
      else if(strncmp(buf, KTH_MAGIC, strlen(KTH_MAGIC)) == 0 ||
	      strncmp(buf, KTH_MAGIC_ERR, strlen(KTH_MAGIC_ERR)) == 0 ||
	      strncmp(buf, SNACK_MAGIC, strlen(SNACK_MAGIC)) == 0) {
	fileFormat = FF_KTH;
      }
    }
    else { /* in all probability ASCII data */
      *dataFormat = FDF_ASC;
      rewind(fp);
      n = fgetl(buf, sizeof(buf), fp, NULL);       /* read first line */
      if(n > 0) {
	if(strcmp(buf, MIX_ORT_ID) == 0 || strcmp(buf, MIX_TRF_ID) == 0 ||
	   strcmp(buf, MIX_TRF_ERR) == 0) {
	  fileFormat = FF_IPDS_M;
	}
	else {                           /* parse line on white space */
	  n = strparse(buf, NULL, field, MAX_HDR_FIELDS);
	  if(n == 1) {
	    strcpy(ident, mybarename(field[0]));
	    if(strcmp(ident, mybarename(filePath)) == 0)
	      fileFormat = FF_IPDS_S;
	  }
	  else if(n == 2 && strcmp(field[0], XLBL_REF_ID) == 0) {
	    strcpy(ident, mybarename(field[1]));
	    if(strcmp(field[1], XLBL_DEF_REF) == 0 ||
	       strcmp(ident, mybarename(filePath)) == 0)
	      fileFormat = FF_XLABEL;
	  }
	  else if(n >= 2 && strcmp(field[0], XASSP_MAGIC) == 0) {
	    fileFormat = FF_XASSP;
	  }
	  else if(n == TXY_FIELDS || n == XYD_FIELDS) {
	    i = 1;
	    if(n == TXY_FIELDS) i++;
	    for(err = FALSE; i < n; i++) {
	      if(strcmp(field[i-1], field[i]) != 0) {
		err = TRUE;
		break;
	      }
	    }
	    if(!err)
	      fileFormat = FF_UWM;      /* but needs further checking */
	  }
	}
      } /* else ignore */
    }
  }
  if(fileFormat == FF_UNDEF) {            /* still not recognized */
    asspMsgNum = AEF_BAD_FORM;
    sprintf(applMessage, "in file %s", filePath);
  }
  return(fileFormat);
}

/*DOC

Decodes the header of the file referred to by 'fp' in the data object 
pointed to by "dop" and fills out the items in the object structure.
If 'fileFormat' in "dop" has not been set, the format of the file will 
be guessed. In this case, the object structure will be re-initialised, 
therefore make sure it contains no pointers to allocated memory except 
'filePath' and 'fp'.
In general, it is good practice to initialise/clear the data object 
before setting 'filePath' and 'fp' and calling this function. 
The function returns -1 upon error, 1 when there are warnings and 0 
upon success.

DOC*/

int getHeader(DOBJ *dop)
{
  char *savePath;
  int   saveMode;
  FILE *saveFile;
  long  fileSize;

  clrAsspMsg();
  if(dop == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "getHeader");
    return(-1);
  }
  if(dop->fp == NULL || dop->filePath == NULL) {
    setAsspMsg(AEB_BAD_CALL, "getHeader");
    return(-1);
  }
  if(dop->fp == stdin) {
    setAsspMsg(AEG_ERR_BUG, "getHeader: can't handle 'stdin'");
    return(-1);
  }
  if(dop->fileFormat <= FF_UNDEF) {
    saveFile = dop->fp;
    savePath = dop->filePath;
    saveMode = dop->openMode;
    initDObj(dop);
    dop->fp = saveFile;
    dop->filePath = savePath;
    dop->openMode = saveMode;
    dop->fileFormat = guessFormat(dop->fp, dop->filePath, &(dop->fileData));
    if(dop->fileFormat <= FF_UNDEF)        /* not recognized or error */
      return(-1);
  }
  switch(dop->fileFormat) {             /* now decode header contents */
  case FF_RAW:
    if(dop->recordSize > 0) {       /* trust descriptor to be correct */
      fileSize = getFileSize(dop);
      if(fileSize < 0)
	return(-1);
      if(fileSize >= dop->headerSize)
	dop->numRecords = (fileSize - dop->headerSize)\
	                / (long)(dop->recordSize);
      else {
	asspMsgNum = AEG_ERR_APPL;
	sprintf(applMessage, "File size less than header size"\
		"\n(RAW format) for file %s", dop->filePath);
	return(-1);
      }
    }
    break;
  case FF_XASSP:
    return(getXASSPhdr(dop));
  case FF_IPDS_M:
    return(getMIXhdr(dop));
  case FF_IPDS_S:
    return(getSAMhdr(dop));
  case FF_XLABEL:
    return(getXLBLhdr(dop));
// commented out by Raphael Winkelmann 13. Nov. 2014 to throw error for AIFF/AIFC file due to copyright issues of ieee.c for CRAN
//  case FF_AIFF:
//  case FF_AIFC:
//    return(getAIFhdr(dop));
  case FF_CSL:
    return(getCSLhdr(dop));
  case FF_CSRE:
    return(getADFhdr(dop));
  case FF_KTH:
    return(getKTHhdr(dop));
  case FF_NIST:
    return(getNISThdr(dop));
  case FF_SND:
    return(getSNDhdr(dop));
  case FF_SSFF:
    return(getSSFFhdr(dop));
  case FF_WAVE:
  case FF_WAVE_X:
    return(getWAVhdr(dop));
  case FF_UWM:
    return(checkXRMB(dop)); /* no real header but identifyable */
  default:
    asspMsgNum = AEF_BAD_FORM;
    sprintf(applMessage, "in file %s", dop->filePath);
    return(-1);
  }
  return(0);
}

/*DOC

Generates a header for the format defined in the data object pointed to 
by "dop" and writes it to the specified file. The file must have been 
opened in write or update mode.
It is assumed that at least the data descriptors have been filled out 
correctly. This function will only set the format-specific items 
'fileData' 'fileEndian', 'headerSize', 'version' and 'sepChars'.
It will verify whether the specified format can handle the data.

DOC*/

int putHeader(DOBJ *dop)
{
  clrAsspMsg();
  if(dop == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "putHeader");
    return(-1);
  }
  if(dop->fp == NULL) {
    setAsspMsg(AEB_BAD_CALL, "putHeader");
    return(-1);
  }
  switch(dop->fileFormat) {
  case FF_RAW:                               /* don't know what to do */
    break;       /* application should decide, we just don't complain */
  case FF_XASSP:
    return(putXASSPhdr(dop));
  case FF_IPDS_M:
    return(putMIXhdr(dop));
  case FF_IPDS_S:
    return(putSAMhdr(dop));
  case FF_XLABEL:
    return(putXLBLhdr(dop));
  case FF_AIFF:
  case FF_AIFC:
    return(putAIFhdr(dop));
  case FF_CSL:
    return(putCSLhdr(dop));
  case FF_CSRE:
    return(putADFhdr(dop));
  case FF_KTH:
    return(putKTHhdr(dop));
  case FF_NIST:
    return(putNISThdr(dop));
  case FF_SND:
    return(putSNDhdr(dop));
  case FF_SSFF:
    return(putSSFFhdr(dop));
  case FF_WAVE:
  case FF_WAVE_X:
    return(putWAVhdr(dop));
  default:
    asspMsgNum = AEG_ERR_BUG;
    sprintf(applMessage, "putHeader: unsupported file format code %i",\
	    (int)(dop->fileFormat));
    return(-1);
  }
  return(0);
}

/*DOC

This function determines whether the data in the object pointed to by 
"dop" require an extensible rather than an old-style RIFF_WAVE header. 
It doesn't change items in the data object structure.
It returns 'TRUE' (non-zero) if this is the case, otherwise 'FALSE' (0).

DOC*/

int needsWAVE_X(DOBJ *dop)
{
  DDESC *dd;

  dd = &(dop->ddl);
  if(dd->format != DF_UINT8 && dd->format != DF_INT16)
    return(TRUE); /* restricted to what RIFF-WAVE can handle */
  if(dd->numFields > 2)
    return(TRUE);
  if(dd->coding != DC_PCM && dd->coding != DC_BINOFF)
    return(TRUE);
  return(FALSE);
}

/*DOC

This function allocates memory for a minimal RIFF-WAVE header (old 
style: 44 byte, extensible: 68 byte without and 80 byte with a 'fact'
chunk) and fills it out based on the information in the data object 
pointed to by "dop". Before creating the header, some items in the 
data object such as 'headerSize', 'fileEndian', 'startRecord', etc. 
will be adjusted. 
After usage, you should return the allocated memory using 'freeWAVhdr'.
The function returns a pointer to the header or NULL upon error.

Note:
 - Implemented as a separate function because, under Windows, you can 
   apparently sometimes set up the audio card by passing a WAV header 
   to the device driver.
 - This function doesn't enforce an extensible WAVE header even when 
   officially required ( > 16-bit integer, > 2 channels, compressed 
   or float data). You can use the function 'needsWAVE_X' to determine 
   whether this is necessary for MS programs to accept the header.

DOC*/

char *genWAVhdr(DOBJ *dop)
{
  char    *ptr;
  char    *header=NULL;
  int      SWAP, NEED_FACT;
  uint16_t dataFormat, bitsPerSample;
  uint32_t chunkSize, sampRate, byteRate, dataBytes;
  ENDIAN   sysEndian={MSB};
  DDESC   *dd;

/*
 * perform basic checks
 */
  if(dop == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "genWAVhdr");
    return(NULL);
  }
  if(dop->fileFormat != FF_WAVE && dop->fileFormat != FF_WAVE_X) {
    setAsspMsg(AEB_BAD_CALL, "genWAVhdr: not WAV format");
    return(NULL);
  }
/*
 * set format-dependent items
 */
  dd = &(dop->ddl);
  dop->fileData = FDF_BIN;                      /* always binary data */
  SETMSBLAST(dop->fileEndian);                     /* always MSB last */
  SWAP = DIFFENDIAN(sysEndian, dop->fileEndian);
  NEED_FACT = FALSE;
  if(dop->fileFormat == FF_WAVE) {
    dop->version = 1;
    dop->headerSize = WAVE_MIN_HDR;
  }
  else { /* extensible format */
    dop->version = 3;
    dop->headerSize = WAVE_MIN_HDR + WAVE_FMTX_MAX + 2;
    /* it is unclear whether the fact chunk is needed for float since */
    /* it contains no additional information; can't harm though ;-) */
    if(!(dd->coding == DC_PCM || dd->coding == DC_BINOFF) ||\
       dd->format == DF_REAL32 || dd->format == DF_REAL64) {
      NEED_FACT = TRUE;
      dop->headerSize += SIZEOF_WAVFACT;
    }
  }
  dop->sepChars[0] = EOS;
  dop->startRecord = 0;                                     /* always */
  dop->Start_Time = dop->Time_Zero = 0.0;
  if(checkSound(dop, auCapsFF(dop->fileFormat), 0) <= 0) {
    strcat(applMessage, " (WAV format)");
    return(NULL);
  }
  sampRate = (uint32_t)myrint(dop->sampFreq);
  byteRate = sampRate * (uint32_t)(dop->recordSize);
  dataBytes = (uint32_t)(dop->numRecords) * (dop->recordSize);
  if(dd->numFields < 1)
    dd->numFields = 1;
  bitsPerSample = (uint16_t)(8 * dop->recordSize / dd->numFields);
  switch(dd->coding) {
  case DC_PCM:
  case DC_BINOFF:
    if(dd->format == DF_REAL32 || dd->format == DF_REAL64)
      dataFormat = WAVE_FLOAT; /* THUSFAR DECLARED PCM */
    else
      dataFormat = WAVE_PCM;
    break;
  case DC_ALAW:
    dataFormat = WAVE_ALAW;
    break;
  case DC_uLAW:
    dataFormat = WAVE_MULAW;
    break;
  default:
    asspMsgNum = AEG_ERR_BUG;
    sprintf(applMessage, "genWAVhdr: %s", getAsspMsg(AED_BAD_FORM));
    return(NULL);
  }
/*
 * allocate memory
 */
  header = (char *)calloc(1, (size_t)(dop->headerSize));
  if(header == NULL) {
    setAsspMsg(AEG_ERR_MEM, "genWAVhdr");
    return(NULL);
  }
/*
 * construct header
 */
  strncpy(header, RIFF_MAGIC, 4);
  ptr = (void *)(&header[4]);
  chunkSize = (uint32_t)(dop->headerSize) - SIZEOF_CHUNK + dataBytes;
  putU32(chunkSize, &ptr, SWAP);
  strncpy((char *)ptr, WAVE_MAGIC, 4);
  ptr += 4;
  /* format chunk */
  strncpy((char *)ptr, WAVE_FORM_ID, 4);
  ptr += 4;
  if(dop->fileFormat == FF_WAVE) {
    chunkSize = (uint32_t)(SIZEOF_WAVFMT - SIZEOF_CHUNK);
    putU32(chunkSize, &ptr, SWAP);
    putU16(dataFormat, &ptr, SWAP);
  }
  else {
    chunkSize = (uint32_t)(SIZEOF_WAVFMTX - SIZEOF_CHUNK);
    putU32(chunkSize, &ptr, SWAP);
    putU16(WAVE_EXTS, &ptr, SWAP);
  }
  putU16((uint16_t)(dd->numFields), &ptr, SWAP);
  putU32(sampRate, &ptr, SWAP);
  putU32(byteRate, &ptr, SWAP);
  putU16((uint16_t)(dop->recordSize), &ptr, SWAP);
  if(dop->fileFormat == FF_WAVE)
    putU16((uint16_t)(dd->numBits), &ptr, SWAP);
  else {
    putU16(bitsPerSample, &ptr, SWAP);
    putU16(WAVE_FMTX_MAX, &ptr, SWAP);
    putU16((uint16_t)(dd->numBits), &ptr, SWAP);        /* valid bits */
    putU32(0, &ptr, SWAP);      /* speaker mapping: put where you can */
    putU16(dataFormat, &ptr, SWAP);
    memcpy((void *)ptr, (void *)WAVE_GUID_ID, WAVE_GUID_LEN);
    ptr += WAVE_GUID_LEN;
  }
  /* fact chunk */
  if(NEED_FACT) {
    strncpy((char *)ptr, WAVE_FACT_ID, 4);
    ptr += 4;
    chunkSize = (uint32_t)(SIZEOF_WAVFACT - SIZEOF_CHUNK);
    putU32(chunkSize, &ptr, SWAP);
    putU32((uint32_t)(dop->numRecords), &ptr, SWAP);
  }
  /* data chunk */
  strncpy((char *)ptr, WAVE_DATA_ID, 4);
  ptr += 4;
  putU32(dataBytes, &ptr, SWAP);
  return(header);
}

/*DOC

This function returns the memory for a RIFF-WAVE header allocated by 
the function 'genWAVhdr'. It always returns a NULL pointer.

DOC*/

char *freeWAVhdr(char *header)
{
  if(header != NULL)
    free((void *)header);
  return((char *)NULL);
}

/*DOC

These functions search a keyword - data type table for the entry 
matching a keyword or a data tpe. The keyword matching is case-
insensitive and will be partial if the length of 'keyword' is more 
than that of the table entry.
The functions return a pointer to the entry in the table, which will 
be the NULL, NULL, NULL, 0 entry if not found.

DOC*/

KDTAB *keyword2entry(char *keyword, KDTAB *table)
{
  KDTAB *entry;

  entry = table;
  while(entry->keyword != NULL) {
    if(strnxcmp(keyword, entry->keyword, strlen(entry->keyword)) == 0)
      break;
    entry++;
  }
  return(entry);
}
KDTAB *dtype2entry(dtype_e type, KDTAB *table)
{
  KDTAB *entry;

  entry = table;
  while(entry->keyword != NULL) {
    if(type == entry->dataType)
      break;
    entry++;
  }
  return(entry);
}

/*----------------------------------------------------------------------
| All following functions are local. It is therefore assumed that      |
| arguments have already been checked and that descriptors contain     |
| valid items values.                                                  |
----------------------------------------------------------------------*/

/***********************************************************************
* Verifies whether file contents are (in all probability) 7-bit ASCII. *
***********************************************************************/
LOCAL int isASCII(FILE *fp)
{
  int8_t buf[ONEkBYTE];
  size_t sumBytes, numBytes, i, n;
  
  clrAsspMsg();
  rewind(fp);
  sumBytes = 0;
  numBytes = ONEkBYTE;
  for(n = 0; n < 4 && numBytes == ONEkBYTE; n++) { /* check up to 4kB */
    numBytes = fread((void *)buf, 1, numBytes, fp); 
    if(ferror(fp)) {
      asspMsgNum = AEF_ERR_READ;
      return(-1);
    }
    sumBytes += numBytes;
    for(i = 0; i < numBytes; i++)
      if(buf[i] <= 0) return(0);                     /* binary format */
    if(feof(fp)) break;
  }
  clearerr(fp);                                     /* reset eof flag */
  if(sumBytes > 0) return(1);                         /* ASCII format */
  asspMsgNum = AEF_EMPTY;            /* nothing read; can't determine */
  return(-1);
}
/***********************************************************************
* Determine the length of a data file.                                 *
***********************************************************************/
LOCAL long getFileSize(DOBJ *dop)
{
  long fileSize;

  if(fseek(dop->fp, 0L, SEEK_END) != 0) {
    setAsspMsg(AEF_ERR_SEEK, dop->filePath);
    return(-1L);
  }
  fileSize = ftell(dop->fp);
  if(fileSize == 0)
    setAsspMsg(AEF_EMPTY, dop->filePath);
  rewind(dop->fp);                    /* also resets file error flags */
  return(fileSize);
}
/***********************************************************************
* Check whether file contains Wisconsin X-ray microbeam data in 'txy'  *
* or 'xyd' format. If so, set descriptor and return 0, else return -1. *
***********************************************************************/
LOCAL int checkXRMB(DOBJ *dop)
{
  char   buf[ONEkBYTE], *field[TXY_FIELDS], *rest, eolCode[4];
  int    i, n, OK;
  long   val;
  DDESC *dd;

  rewind(dop->fp);
  asspMsgNum = AEF_ERR_FORM;                          /* preset error */
  sprintf(applMessage, "(not XRMB) in file %s", dop->filePath);
  OK = TRUE;
  n = fgetl(buf, sizeof(buf), dop->fp, &rest);
  if(n <= 0)
    return(-1);
  strcpy(eolCode, rest);
  n = strparse(buf, NULL, field, TXY_FIELDS);
  if(n != TXY_FIELDS && n != XYD_FIELDS)
    return(-1);
  i = 0;
  if(n == TXY_FIELDS) {
    if(strcmp(field[0], "0") == 0)
      i++;
    else
      return(-1);
  }
  while(OK && i < n) {
    val = strtol(field[i++], &rest, 10);
    if(val != XRM_INFINITE || strlen(rest))
      OK = FALSE;
  }
  if(!OK)
    return(-1);

  freeDDList(dop);                   /* clear/remove data descriptors */
  dd = &(dop->ddl);
  if(n == TXY_FIELDS) {                          /* read second line */
    n = fgetl(buf, sizeof(buf), dop->fp, NULL);
    if(n <= 0)
      return(-1);
    n = strparse(buf, NULL, field, TXY_FIELDS);
    if(n != TXY_FIELDS)
      return(-1);
    val = strtol(field[0], &rest, 10);
    if(val <= 0 || strlen(rest))
      return(-1);
    dop->frameDur = val;
    dd->coding = DC_TXY;
    dd->numFields = TXY_FIELDS;
  }
  else {
    dop->frameDur = XRM_FRAMEDUR;      /* not specified; take default */
    dd->coding = DC_XYD;
    dd->numFields = XYD_FIELDS;
  }
/*
 * complete DOBJ and DDESC
 */
  clrAsspMsg();
  dop->fileFormat = FF_UWM;
  dop->fileData = FDF_ASC;
  CLRENDIAN(dop->fileEndian);
  dop->version = 0;
  dop->sampFreq = XRM_SAMPFREQ;               /* pseudo sampling rate */
  dop->recordSize = 0;                                    /* variable */
  dop->numRecords = 0;                             /* can't determine */
  dop->startRecord = 0;                                     /* always */
  strcpy(dop->sepChars, "\t");
  strcpy(dop->eol, eolCode);
  dd->ident = strdup("X-ray_microbeam");
  strcpy(dd->unit, "m");
  strcpy(dd->factor, "u");
  dd->type = DT_XRM;
  dd->format = DF_INT32;
  SETFACERIGHT(dd->orientation);
  setStart_Time(dop);
  return(0);
}

/*----------------------------------------------------------------------
| Binary headers for audio data                                        |
----------------------------------------------------------------------*/

/***********************************************************************
* Read AIFF/AIFC header.                                               *
***********************************************************************/
LOCAL int getAIFhdr(DOBJ *dop)
{
  char    *ptr;
  char     buf[ONEkBYTE];
  int      err, SWAP;
  size_t   numBytes;
  int32_t  chunkSize;
  uint32_t blockSize;
  long     headSize, fileSize, offset;
  ENDIAN   sysEndian={MSB};
  DDESC   *dd;

  fileSize = getFileSize(dop);
  if(fileSize <= 0)
    return(-1);
  numBytes = SIZEOF_FORM;             /* outer chunk is standard FORM */
  if(fread(buf, 1, numBytes, dop->fp) != numBytes) {
    setAsspMsg(AEF_ERR_READ, dop->filePath);
    return(-1);
  }
/*
 * verify format
 */
  err = FALSE;
  if(strncmp(buf, AIF_FORM_ID, 4) != 0)
    err = TRUE;
  else {
    if(strncmp(&buf[8], AIFF_MAGIC, 4) == 0) {
      dop->fileFormat = FF_AIFF;
      dop->version = 0;
    }
    else if(strncmp(&buf[8], AIFC_MAGIC, 4) == 0)
      dop->fileFormat = FF_AIFC;
    else
      err = TRUE;
  }
  if(err) {
    asspMsgNum = AEF_ERR_FORM;
    if(dop->fileFormat == FF_AIFF)
      sprintf(applMessage, "(not AIFF)");
    else
      sprintf(applMessage, "(not AIFC)");
    strcat(applMessage, "\nin file ");
    strcat(applMessage, dop->filePath);
    return(-1);
  }
  SETMSBFIRST(dop->fileEndian);                   /* always MSB first */
  SWAP = DIFFENDIAN(dop->fileEndian, sysEndian);
  ptr = (void *)&buf[4];
  chunkSize = getI32(&ptr, SWAP);               /* for debugging only */
  headSize = (long)numBytes;              /* keep track of bytes read */
  dop->headerSize = 0;                        /* use as flag for SSND */
  dop->numRecords = -1;                       /* use as flag for COMM */
  freeDDList(dop);                   /* clear/remove data descriptors */
  dd = &(dop->ddl);
/*
 * loop through chunks
 */
  while((fileSize-headSize) >= SIZEOF_CHUNK &&
	(dop->headerSize == 0 || dop->numRecords < 0) ) {
    numBytes = SIZEOF_CHUNK;
    if(fread(buf, 1, numBytes, dop->fp) != numBytes) {
      setAsspMsg(AEF_ERR_READ, dop->filePath);
      return(-1);
    }
    headSize += (long)numBytes;
    ptr = (void *)&buf[4];
    chunkSize = getI32(&ptr, SWAP);
    if(ODD(chunkSize))
      chunkSize++;                       /* zero-padding NOT included */
/*
 * decode VERSION chunk (AIFC only but not everybody seems to know that)
 */
    if(strncmp(buf, AIFC_VERSION_ID, 4) == 0) {
      numBytes = (size_t)chunkSize;     /* doesn't include CHUNK part */
      if(fread(buf, 1, numBytes, dop->fp) != numBytes) {
	asspMsgNum = AEF_BAD_HEAD;
	sprintf(applMessage, "(AIFC format) in file %s", dop->filePath);
	return(-1);
      }
      headSize += (long)numBytes;
      ptr = (void *)buf;
      dop->version = getI32(&ptr, SWAP);
    }
/*
 * decode COMM chunk
 */
    else if(strncmp(buf, AIF_COMM_ID, 4) == 0) {
      numBytes = (size_t)chunkSize;     /* doesn't include CHUNK part */
      if(fread(buf, 1, numBytes, dop->fp) != numBytes) {
	asspMsgNum = AEF_BAD_HEAD;
	sprintf(applMessage, "(AIF format) in file %s", dop->filePath);
	return(-1);
      }
      headSize += (long)numBytes;
      ptr = (void *)buf;
      dd->numFields = (size_t)getI16(&ptr, SWAP);
      dop->numRecords = (long)getU32(&ptr, SWAP);
      dd->numBits = (uint16_t)getI16(&ptr, SWAP);
      // commented out by Raphael Winkelmann 13. Nov. 2014 due to copyright issues of ieee.c for CRAN (ieee.c is missing)
      //dop->sampFreq = ConvertFromIeeeExtended((uint8_t *)ptr);
      /* needs no swapping: conversion assumes MSB-first */
      ptr += XFPSIZE;
      dop->fileData = FDF_BIN;                  /* always binary data */
      dop->frameDur = 1;                       /* always sampled data */
      dop->startRecord = 0;                         /* per definition */
      dd->type = DT_SMP;
      dd->zeroValue = 0;               /* apparently no unsigned data */
      dd->format = DF_UNDEF;                   /* need to check later */
      if(dop->fileFormat == FF_AIFC) {
	if(strnxcmp((char *)ptr, AIFC_NONE, 4) == 0) {
	  dd->coding = DC_PCM;
	}
	else if(strnxcmp((char *)ptr, AIFC_ACE2, 4) == 0) {
	  dd->coding = DC_ACE2;
	  dd->format = DF_BIT;   /* proprietary, don't know how coded */
	  dop->frameDur = 0;
	}
	else if(strnxcmp((char *)ptr, AIFC_ACE8, 4) == 0) {
	  dd->coding = DC_ACE8;
	  dd->format = DF_BIT;
	  dop->frameDur = 0;
	}
	else if(strnxcmp((char *)ptr, AIFC_MAC3, 4) == 0) {
	  dd->coding = DC_MAC3;
	  dd->format = DF_BIT;
	  dop->frameDur = 0;
	}
	else if(strnxcmp((char *)ptr, AIFC_MAC6, 4) == 0) {
	  dd->coding = DC_MAC6;
	  dd->format = DF_BIT;
	  dop->frameDur = 0;
	}
	else if(strnxcmp((char *)ptr, AIFC_ALAW, 4) == 0) {
	  dd->coding = DC_ALAW;
	  dd->format = DF_UINT8;
	  dd->numBits = 8;
	}
	else if(strnxcmp((char *)ptr, AIFC_uLAW, 4) == 0) {
	  dd->coding = DC_uLAW;
	  dd->format = DF_UINT8;
	  dd->numBits = 8;
	}
	else if(strnxcmp((char *)ptr, AIFC_FL32, 4) == 0) {
	  dd->coding = DC_PCM;
	  dd->format = DF_REAL32;
	  dd->numBits = 32;
	}
	else if(strnxcmp((char *)ptr, AIFC_FL64, 4) == 0) {
	  dd->coding = DC_PCM;
	  dd->format = DF_REAL64;
	  dd->numBits = 64;
	}
	else {
	  dd->coding = DC_ERROR;
	  asspMsgNum = AED_BAD_FORM;
	  sprintf(applMessage, "(AIFC format) in file %s", dop->filePath);
	  return(-1);
	}
	ptr += 4;/* in case we want to check for bugs like PRAAT does */
      }
      else              /* AIFF format */
	dd->coding = DC_PCM;
      if(dd->format == DF_UNDEF) {
	if(dd->numBits <= 8)
	  dd->format = DF_INT8;        /* apparently no unsigned data */
	else if(dd->numBits <= 16)
	  dd->format = DF_INT16;
	else if(dd->numBits <= 24)        /* apparently always packed */
	  dd->format = DF_INT24;
	else if(dd->numBits <= 32)
	  dd->format = DF_INT32;
	else {
	  dd->format = DF_ERROR;
	  asspMsgNum = AED_BAD_FORM;
	  sprintf(applMessage, "(AIF format) in file %s", dop->filePath);
	  return(-1);
	}
      }
      dd->ident = strdup("audio");
      setRecordSize(dop);
    }
/*
 * decode SSND chunk
 */
    else if(strncmp(buf, AIF_DATA_ID, 4) == 0) {
      numBytes = SIZEOF_SSND - SIZEOF_CHUNK;        /* ignore data !! */
      if(fread(buf, 1, numBytes, dop->fp) != numBytes) {
	asspMsgNum = AEF_BAD_HEAD;
	sprintf(applMessage, "(AIF format) in file %s", dop->filePath);
	return(-1);
      }
      headSize += (long)numBytes;
      ptr = (void *)buf;
      offset = (long)getU32(&ptr, SWAP);
      blockSize = getU32(&ptr, SWAP);           /* for debugging only */
      dop->headerSize = headSize + offset; /* data follow immediately */
      if(dop->numRecords >= 0)
	break;                   /* can't handle multiple data chunks */
      headSize += ((long)chunkSize - (long)numBytes);
      if((fileSize-headSize) >= SIZEOF_CHUNK)      /* chunk following */
	fseek(dop->fp, headSize, SEEK_SET);  /* position file pointer */
    }
/* skip uninteresting chunks */
    else {
      headSize += (long)chunkSize;
      if((fileSize-headSize) >= SIZEOF_CHUNK)      /* chunk following */
	fseek(dop->fp, headSize, SEEK_SET);  /* position file pointer */
    }
  }

  if(dop->headerSize <= 0 || dop->numRecords < 0) {
    asspMsgNum = AEF_BAD_HEAD;
    sprintf(applMessage, "(AIF format) in file %s", dop->filePath);
    return(-1);
  }
  setStart_Time(dop);
  return(0);
}
/***********************************************************************
* Write minimal AIFF/AIFC header (54/72 byte).                         *
***********************************************************************/
LOCAL int putAIFhdr(DOBJ *dop)
{
  char   *ptr;
  char    header[AIFC_MIN_HDR];
  int     SWAP;
  size_t  numBytes;
  int32_t commSize;
  long    dataBytes;
  ENDIAN  sysEndian={MSB};
  DDESC  *dd=&(dop->ddl);

/*
 * set format-dependent items
 */
  dop->fileData = FDF_BIN;                      /* always binary data */
  SETMSBFIRST(dop->fileEndian);                   /* always MSB first */
  SWAP = DIFFENDIAN(sysEndian, dop->fileEndian);
  if(dop->fileFormat == FF_AIFF) {
    dop->headerSize = AIFF_MIN_HDR;
    dop->version = 0;
    commSize = AIFF_COMMSIZE;
  }
  else {
    dop->headerSize = AIFC_MIN_HDR;
    dop->version = AIFC_VERSION;
    commSize = AIFC_COMMSIZE;
  }
  dop->sepChars[0] = EOS;
  dop->startRecord = 0;                                     /* always */
  setStart_Time(dop);
  if(checkSound(dop, auCapsFF(dop->fileFormat), 0) <= 0) {
    if(dop->fileFormat == FF_AIFF)
      strcat(applMessage, " (AIFF format)");
    else
      strcat(applMessage, " (AIFC format)");
    return(-1);
  }
  dataBytes = dop->numRecords * (long)(dop->recordSize);
/*
 * construct header in memory: FORM chunk
 */
  strncpy(header, AIF_FORM_ID, 4);
  ptr = (void *)&header[4];
  putI32((int32_t)(dop->headerSize - SIZEOF_CHUNK + dataBytes), &ptr, SWAP);
  if(dop->fileFormat == FF_AIFF)
    strncpy((char *)ptr, AIFF_MAGIC, 4);
  else
    strncpy((char *)ptr, AIFC_MAGIC, 4);
  ptr += 4;
/*
 * FVER chunk (AIFC only)
 */
  if(dop->fileFormat == FF_AIFC) {
    strncpy((char *)ptr, AIFC_VERSION_ID, 4);
    ptr += 4;
    putI32(SIZEOF_FVER - SIZEOF_CHUNK, &ptr, SWAP);         /* ckSize */
    putI32(AIFC_VERSION, &ptr, SWAP);
  }
/*
 * COMM chunk
 */
  strncpy((void *)ptr, AIF_COMM_ID, 4);
  ptr += 4;
  putI32(commSize - SIZEOF_CHUNK, &ptr, SWAP);              /* ckSize */
  putI16((int16_t)dd->numFields, &ptr, SWAP);            /* numTracks */
  putU32((uint32_t)dop->numRecords, &ptr, SWAP);        /* numSamples */
  putI16((int16_t)dd->numBits, &ptr, SWAP);                /* numBits */
  // commented out by Raphael Winkelmann 13. Nov. 2014 due to copyright issues of ieee.c for CRAN (ieee.c is missing)
  // ConvertToIeeeExtended(dop->sampFreq, (uint8_t *)ptr);
  /* needs no swapping: conversion produces MSB-first */
  ptr += XFPSIZE;
  if(dop->fileFormat == FF_AIFC) {
    switch(dd->coding) {
    case DC_PCM:
      switch(dd->format) {
      case DF_REAL32:
	strncpy((char *)ptr, AIFC_FL32, 4);
	break;
      case DF_REAL64:
	strncpy((char *)ptr, AIFC_FL64, 4);
	break;
      default:
	strncpy((char *)ptr, AIFC_NONE, 4);
	break;
      }
      break;
    case DC_ALAW:
      strncpy((char *)ptr, AIFC_ALAW, 4);
      break;
    case DC_uLAW:
      strncpy((char *)ptr, AIFC_uLAW, 4);
      break;
    default:
      asspMsgNum = AEG_ERR_BUG;
      sprintf(applMessage, "putAIFhdr: %s", getAsspMsg(AED_BAD_FORM));
      return(-1);
    }
    ptr += 4;
    putU8(0, &ptr);               /* length of compression descriptor */
    putI8(0, &ptr);            /* descriptor text (we leave it empty) */
  }
/*
 * SSND chunk
 */
  strncpy((char *)ptr, AIF_DATA_ID,4);
  ptr += 4;
  putI32((int32_t)(SIZEOF_SSND - SIZEOF_CHUNK + dataBytes), &ptr, SWAP);
  putU32(0, &ptr, FALSE);                                   /* offset */
  putU32(0, &ptr, FALSE);                                /* blockSize */
/*
 * write header
 */
#ifndef WRASSP
  if(dop->fp != stdout)
    rewind(dop->fp);
#else
  rewind(dop->fp);
#endif
  numBytes = (size_t)dop->headerSize;
  if(fwrite(header, 1, numBytes, dop->fp) != numBytes) {
    setAsspMsg(AEF_ERR_WRIT, dop->filePath);
    return(-1);
  }
  return(0);
}
/***********************************************************************
* Read CSL header (only sampled data with maximally 2 channels).       *
***********************************************************************/
LOCAL int getCSLhdr(DOBJ *dop)
{
  char    *ptr;
  char     buf[ONEkBYTE];
  int      err, SWAP;
  size_t   numBytes;
  int16_t  peakMag;
  uint32_t chunkSize;
  long     numRecords, headSize, fileSize;
  ENDIAN   sysEndian={MSB};
  DDESC   *dd;

  fileSize = getFileSize(dop);
  if(fileSize <= 0)
    return(-1);
  numBytes = SIZEOF_FORM;        /* different structure but same size */
  if(fread(buf, 1, numBytes, dop->fp) != numBytes) {
    setAsspMsg(AEF_ERR_READ, dop->filePath);
    return(-1);
  }
/*
 * verify format
 */
  if(strncmp(buf, CSL_MAGIC, strlen(CSL_MAGIC)) != 0) {
    asspMsgNum = AEF_ERR_FORM;
    sprintf(applMessage, "(not CSL) in file %s", dop->filePath);
    return(-1);
  }
  SETMSBLAST(dop->fileEndian);                     /* always MSB last */
  SWAP = DIFFENDIAN(dop->fileEndian, sysEndian);
  ptr = (void *)&buf[strlen(CSL_MAGIC)];
  chunkSize = getU32(&ptr, SWAP);               /* for debugging only */
  dop->headerSize = 0;                      /* use as flag for 'DATA' */
  dop->numRecords = -1;                     /* use as flag for 'HEDR' */
  numRecords = -1;                         /* for header verification */
  headSize = (long)numBytes;              /* keep track of bytes read */
  freeDDList(dop);                   /* clear/remove data descriptors */
  dd = &(dop->ddl);
  err = 0;
/*
 * loop through chunks
 */
  while((fileSize-headSize) >= SIZEOF_CHUNK &&
	(dop->headerSize == 0 || dop->numRecords < 0) ) {
    numBytes = SIZEOF_CHUNK;
    if(fread(buf, 1, numBytes, dop->fp) != numBytes) {
      asspMsgNum = AEF_BAD_HEAD;
      sprintf(applMessage, "(CSL format) in file %s", dop->filePath);
      return(-1);
    }
    headSize += (long)numBytes;
    ptr = (void *)&buf[4];
    chunkSize = getU32(&ptr, SWAP);
    if(ODD(chunkSize))
      chunkSize++;                       /* zero-padding NOT included */
/*
 * decode FORMAT chunk
 */
    if(strncmp(buf, CSL_HEAD_ID, 4) == 0) {
      if(chunkSize != (SIZEOF_CSLFMT - SIZEOF_CHUNK)) {
	asspMsgNum = AEF_BAD_HEAD;
	sprintf(applMessage, "(CSL format) in file %s", dop->filePath);
	return(-1);
      }
      numBytes = (size_t)chunkSize;
      if(fread(buf, 1, numBytes, dop->fp) != numBytes) {
	asspMsgNum = AEF_BAD_HEAD;
	sprintf(applMessage, "(CSL format) in file %s", dop->filePath);
	return(-1);
      }
      headSize += (long)numBytes;
      ptr = (void *)buf;
      ptr += CSL_DATESIZE;         /* skip date until we can store it */
      dop->sampFreq = (double)getU32(&ptr, SWAP);
      dop->numRecords = (long)getU32(&ptr, SWAP);
      peakMag = getI16(&ptr, SWAP);                   /* skip peakA */
      peakMag = getI16(&ptr, SWAP);                   /* skip peakB */
    }
/*
 * decode DATA chunk
 */
    else if(strncmp(buf, CSL_DATA_ID_L, 4) == 0 ||
	    strncmp(buf, CSL_DATA_ID_R, 4) == 0 ||
	    strncmp(buf, CSL_DATA_ID_S, 4) == 0) {
      if(dop->headerSize > 0) {   /* may contain multiple data blocks */
	asspMsgNum = AWG_WARN_APPL;
	sprintf(applMessage, "Multiple data blocks in file %s (CSL format)",\
		dop->filePath);
	err = 1;
	break;
      }
      dop->headerSize = headSize;          /* data follow immediately */
      dop->fileData = FDF_BIN;                  /* always binary data */
      dop->frameDur = 1;                       /* always sampled data */
      dop->startRecord = 0;                         /* per definition */
      dd->ident = strdup("audio");
      dd->type = DT_SMP;
      dd->format = DF_INT16;                     /* pretty restricted */
      dd->coding = DC_PCM;
      dd->numBits = 16;
      dd->zeroValue = 0;
      if(strncmp(buf, CSL_DATA_ID_S, 4) == 0)
	dd->numFields = 2;
      else
	dd->numFields = 1;
      setRecordSize(dop);
      numRecords = (long)(chunkSize / dop->recordSize);
      headSize += (long)chunkSize;
      if((fileSize-headSize) >= SIZEOF_CHUNK)      /* chunk following */
	fseek(dop->fp, headSize, SEEK_SET);  /* position file pointer */
    }
/* skip uninteresting chunks */
    else {
      headSize += (long)chunkSize;
      if((fileSize-headSize) >= SIZEOF_CHUNK)      /* chunk following */
	fseek(dop->fp, headSize, SEEK_SET);  /* position file pointer */
    }
  }

  if(dop->headerSize <= 0 || dop->numRecords < 0) {
    asspMsgNum = AEF_BAD_HEAD;
    sprintf(applMessage, "(CSL format) in file %s", dop->filePath);
    return(-1);
  }
  if(dop->numRecords > 0 && numRecords > 0) {
    if(dop->numRecords > numRecords) {
      asspMsgNum = AWG_WARN_APPL;
      sprintf(applMessage, "Multiple data blocks\n"\
	      "         (CSL format) in file %s", dop->filePath);
      dop->numRecords = numRecords;
      err = 1;
    }
    else if(dop->numRecords != numRecords) {
      asspMsgNum = AEF_BAD_HEAD;
      sprintf(applMessage, "(CSL format) in file %s", dop->filePath);
      return(-1);
    }
  }
  setStart_Time(dop);
  return(err);
}
/***********************************************************************
* Write minimal CSL header (60 byte).                                  *
***********************************************************************/
LOCAL int putCSLhdr(DOBJ *dop)
{
  char    *ptr;
  char    *date, header[CSL_MIN_HDR];
  int      SWAP;
  size_t   numBytes;
  int16_t  peakMag;
  uint32_t dataBytes;
  time_t   currTime;
  ENDIAN   sysEndian={MSB};
  DDESC   *dd=&(dop->ddl);

/*
 * set format-dependent items
 */
  dop->fileData = FDF_BIN;                      /* always binary data */
  SETMSBLAST(dop->fileEndian);                     /* always MSB last */
  SWAP = DIFFENDIAN(sysEndian, dop->fileEndian);
  dop->headerSize = CSL_MIN_HDR;
  dop->version = 0;
  dop->sepChars[0] = EOS;
  dop->startRecord = 0;                                     /* always */
  setStart_Time(dop);
  if(checkSound(dop, auCapsFF(dop->fileFormat), 0) <= 0) {
    strcat(applMessage, " (CSL format)");
    return(-1);
  }
/*
 * construct header in memory: FORM chunk
 */
  dataBytes = (uint32_t)(dop->numRecords * (long)(dop->recordSize));
  strncpy(header, CSL_MAGIC, strlen(CSL_MAGIC));
  ptr = (void *)&header[strlen(CSL_MAGIC)];
  putU32(SIZEOF_CSLFMT + SIZEOF_CSLDAT + dataBytes, &ptr, SWAP);
/*
 * HEDR and DATA chunk
 */
  strncpy((char *)ptr, CSL_HEAD_ID, 4);
  ptr += 4;
  putU32(SIZEOF_CSLFMT - SIZEOF_CHUNK, &ptr, SWAP);
  currTime = time(NULL);                          /* get current time */
  date = ctime(&currTime);                       /* convert to string */
  strncpy((char *)ptr, &date[4], CSL_DATESIZE);
  ptr += CSL_DATESIZE;
  putU32((uint32_t)myrint(dop->sampFreq), &ptr, SWAP);
  putU32((uint32_t)(dop->numRecords), &ptr, SWAP);
  peakMag = (1 << (dd->numBits -1)) -1;
  if(dd->numFields == 1) {
    putI16(peakMag, &ptr, SWAP);
    putI16(-1, &ptr, SWAP);
    strncpy((char *)ptr, CSL_DATA_ID_L, 4);
  }
  else {
    putI16(peakMag, &ptr, SWAP);
    putI16(peakMag, &ptr, SWAP);
    strncpy((char *)ptr, CSL_DATA_ID_S, 4);
  }
  ptr += 4;
  putU32(dataBytes, &ptr, SWAP);
/*
 * write header
 */
#ifndef WRASSP
  if(dop->fp != stdout)
    rewind(dop->fp);
#else
  rewind(dop->fp);
#endif
  numBytes = (size_t)dop->headerSize;
  if(fwrite(header, 1, numBytes, dop->fp) != numBytes) {
    setAsspMsg(AEF_ERR_WRIT, dop->filePath);
    return(-1);
  }
  return(0);
}
/***********************************************************************
* Read CSRE-ADF header (only sampled data in mono).                    *
***********************************************************************/
LOCAL int getADFhdr(DOBJ *dop)
{
  void    *ptr;
  char     buf[ADF_HDR_SIZE];
  int      SWAP;
  size_t   numBytes;
  uint16_t uInt16;
  long     fileSize, numRecords;
  ENDIAN   sysEndian={MSB};
  DDESC   *dd;

  fileSize = getFileSize(dop);
  if(fileSize <= 0)
    return(-1);
  numBytes = ADF_HDR_SIZE;                        /* load full header */
  if(fread(buf, 1, numBytes, dop->fp) != numBytes) {
    setAsspMsg(AEF_ERR_READ, dop->filePath);
    return(-1);
  }
  if(strcmp(buf, ADF_MAGIC) != 0) {                  /* verify format */
    asspMsgNum = AEF_ERR_FORM;
    sprintf(applMessage, "(not CSRE-ADF) in file %s", dop->filePath);
    return(-1);
  }
  SETMSBLAST(dop->fileEndian);                     /* always MSB last */
  SWAP = DIFFENDIAN(dop->fileEndian, sysEndian);
  freeDDList(dop);                   /* clear/remove data descriptors */
  dd = &(dop->ddl);
  ptr = (void *)&buf[8];                                /* past ident */
  dop->numRecords = (long)getI32(&ptr, SWAP);
  dd->zeroValue = (uint32_t)getI32(&ptr, SWAP);         /* centreLine */
  uInt16 = getU16(&ptr, SWAP); /* startTrack: debugging and ptr advance */
  dd->numBits = getU16(&ptr, SWAP);
  if(dd->numBits < 12 || dd->numBits > 16) {
    asspMsgNum = AEF_BAD_HEAD;
    sprintf(applMessage, "(CSRE-ADF format) in file %s", dop->filePath);
    return(-1);
  }
  uInt16 = getU16(&ptr, SWAP);                          /* dataFormat */
  if(dd->zeroValue == 0)                           /* bug in CSRE ??? */
    uInt16 = ADF_SLIN;
  switch(uInt16) {
  case ADF_SLIN:                                    /* 2's complement */
    dd->coding = DC_PCM;
    dd->format = DF_INT16;
    dd->zeroValue = 0;
    break;
  case ADF_ULIN:                                     /* binary offset */
   dd->coding = DC_BINOFF;
   dd->format = DF_UINT16;
    break;
  default:
    dd->format = DF_ERROR;
    asspMsgNum = AEF_BAD_HEAD;
    sprintf(applMessage, "(CSRE-ADF format) in file %s", dop->filePath);
    return(-1);
  }
  dop->sampFreq = (double)getF32(&ptr, SWAP);  /* sampRate in kHz !!! */
  dop->sampFreq = myrint((dop->sampFreq) * 1000.0);    /* round to Hz */
  getI32(&ptr, SWAP);     /* peakSample: debugging and to advance ptr */
/*
 * complete DOBJ and DDESC
 */
  dop->fileData = FDF_BIN;                      /* always binary data */
  dop->headerSize = ADF_HDR_SIZE;                            /* fixed */
  dop->version = 4;                                     /* apparently */
  dop->frameDur = 1;                           /* always sampled data */
  dop->startRecord = 0;                             /* per definition */
  dd->ident = strdup("audio");
  dd->type = DT_SMP;
  dd->numFields = 1;                /* apparently only mono supported */
  setRecordSize(dop);
  numRecords = (fileSize - dop->headerSize) / (long)(dop->recordSize);
  if(dop->numRecords < 0 || dop->numRecords > numRecords) {
    dop->numRecords = numRecords;
  }
  setStart_Time(dop);
  return(0);
}
/***********************************************************************
* Write CSRE-ADF header (512 byte).                                    *
***********************************************************************/
LOCAL int putADFhdr(DOBJ *dop)
{
  void   *ptr;
  char    header[ADF_HDR_SIZE];
  int     SWAP;
  size_t  numBytes;
  int32_t maxMag;
  ENDIAN  sysEndian={MSB};
  DDESC  *dd=&(dop->ddl);

/*
 * set format-dependent items
 */
  dop->fileData = FDF_BIN;                      /* always binary data */
  SETMSBLAST(dop->fileEndian);                     /* always MSB last */
  SWAP = DIFFENDIAN(sysEndian, dop->fileEndian);
  dop->headerSize = ADF_HDR_SIZE;                            /* fixed */
  dop->version = 4;                                     /* apparently */
  dop->sepChars[0] = EOS;
  dop->startRecord = 0;                                     /* always */
  setStart_Time(dop);
  maxMag = 1 << (dd->numBits-1);
  if(dd->format == DF_UINT16) {
    dd->coding = DC_BINOFF;
    if(dd->zeroValue == 0)
      dd->zeroValue = maxMag;
  }
  else {
    dd->coding = DC_PCM;
    dd->zeroValue = 0;
  }
  if(checkSound(dop, auCapsFF(dop->fileFormat), 0) <= 0) {
    strcat(applMessage, " (CSRE-ADF format)");
    return(-1);
  }
/*
 * construct header in memory
 */
  memset((void *)header, 0, ADF_HDR_SIZE);            /* clear header */
  strcpy(header, ADF_MAGIC);
  ptr = (void *)(&header[8]);
  putI32((int32_t)dop->numRecords, &ptr, SWAP);
  putI32((int32_t)dd->zeroValue, &ptr, SWAP);       /* centreLine ??? */
  putU16(1, &ptr, SWAP);                            /* startTrack ??? */
  putU16(dd->numBits, &ptr, SWAP);
  if(dd->coding == DC_BINOFF)
    putU16(ADF_ULIN, &ptr, SWAP);
  else
    putU16(ADF_SLIN, &ptr, SWAP);
  putF32((float)(dop->sampFreq / 1000.0), &ptr, SWAP);  /* in kHz !!! */
  putI32(maxMag-1, &ptr, SWAP);                         /* peakSample */
/*
 * write header
 */
#ifndef WRASSP
  if(dop->fp != stdout)
#endif
    rewind(dop->fp);
  numBytes = (size_t)dop->headerSize;
  if(fwrite(header, 1, numBytes, dop->fp) != numBytes) {
    setAsspMsg(AEF_ERR_WRIT, dop->filePath);
    return(-1);
  }
  return(0);
}
/***********************************************************************
* Read SND/AU header.                                                  *
***********************************************************************/
LOCAL int getSNDhdr(DOBJ *dop)
{
  void   *ptr;
  char    buf[SND_MIN_HDR];
  int     err, SWAP;
  size_t  numBytes;
  long    fileSize, format;
  ENDIAN  sysEndian={MSB};
  DDESC  *dd;

  fileSize = getFileSize(dop);
  if(fileSize <= 0)
    return(-1);
  numBytes = SND_MIN_HDR;                     /* load standard header */
  if(fread(buf, 1, numBytes, dop->fp) != numBytes) {
    setAsspMsg(AEF_ERR_READ, dop->filePath);
    return(-1);
  }
/*
 * verify format
 */
  if(strncmp(buf, SND_MAGIC, strlen(SND_MAGIC)) != 0) {
    asspMsgNum = AEF_ERR_FORM;
    sprintf(applMessage, "(not AU/SND) in file %s", dop->filePath);
    return(-1);
  }
  err = 0;
  SETMSBFIRST(dop->fileEndian);                   /* always MSB first */
  SWAP = DIFFENDIAN(dop->fileEndian, sysEndian);
  freeDDList(dop);                   /* clear/remove data descriptors */
  dd = &(dop->ddl);
  ptr = (void *)(&buf[strlen(SND_MAGIC)]);
  dop->headerSize = (long)getI32(&ptr, SWAP);
  dop->numRecords = (long)getI32(&ptr, SWAP);
  format = (long)getI32(&ptr, SWAP);
  dop->sampFreq = (double)getI32(&ptr, SWAP);
  dd->numFields = (size_t)getI32(&ptr, SWAP);
  if(format < 1 || dop->sampFreq <= 0.0 || dd->numFields < 1) {
    asspMsgNum = AEF_BAD_HEAD;
    sprintf(applMessage, "(AU/SND format) in file %s", dop->filePath);
    return(-1);
  }
  if(dop->headerSize < SND_MIN_HDR) {
    err = 1;
    asspMsgNum = AWF_BAD_ITEM;
    sprintf(applMessage, "(AU/SND format) in file %s\n"\
	    "         Incorrect data offset (%ld, using %d)",\
	    dop->filePath, dop->headerSize, SND_MIN_HDR);
    dop->headerSize = SND_MIN_HDR;
  }
/*
 * complete DOBJ and DDESC
 */
  dop->fileData = FDF_BIN;                      /* always binary data */
  dop->frameDur = 1;                           /* always sampled data */
  dop->startRecord = 0;                             /* per definition */
  dd->ident = strdup("audio");
  dd->type = DT_SMP;
  dd->coding  = DC_PCM; /* apparently no unsigned data except A/u-law */
  switch(format) {
  case SND_LINEAR_8:
    dd->format  = DF_INT8;
    dd->numBits = 8;
    break;
  case SND_LINEAR_16:
    dd->format  = DF_INT16;
    dd->numBits = 16;
    break;
  case SND_LINEAR_24:
    dd->format  = DF_INT24;
    dd->numBits = 24;
    break;
  case SND_LINEAR_32:
    dd->format  = DF_INT32;
    dd->numBits = 32;
    break;
  case SND_FLOAT:
    dd->format  = DF_REAL32;
    dd->numBits = 32;
    break;
  case SND_DOUBLE:
    dd->format  = DF_REAL64;
    dd->numBits = 64;
    break;
  case SND_ALAW_8:
    dd->format  = DF_UINT8;
    dd->coding  = DC_ALAW;
    dd->numBits = 8;
    break;
  case SND_MULAW_8:
    dd->format  = DF_UINT8;
    dd->coding  = DC_uLAW;
    dd->numBits = 8;
    break;
  case SND_G721:       /* following not supported but shown by ffinfo */
    dd->format  = DF_BIT;
    dd->coding  = DC_G721;
    dd->numBits = 4;
    break;
  case SND_G722:
    dd->format  = DF_BIT;
    dd->coding  = DC_G722;
    dd->numBits = 1;
    break;
  case SND_G723_3:
    dd->format  = DF_BIT;
    dd->coding  = DC_G723_3;
    dd->numBits = 3;
    break;
  case SND_G723_5:
    dd->format  = DF_BIT;
    dd->coding  = DC_G723_5;
    dd->numBits = 5;
    break;
  default:
    dd->format = DF_ERROR;
    asspMsgNum = AED_BAD_FORM;
    sprintf(applMessage, "(AU/SND format) in file %s", dop->filePath);
    return(-1);
  }
  setRecordSize(dop);
  if(dop->recordSize > 0) {
    if(dop->numRecords <= 0) /* may be 0, i.e. unspecified */
      dop->numRecords = (fileSize - dop->headerSize)\
	              / (long)(dop->recordSize);
    else
      dop->numRecords /= (long)(dop->recordSize);
  }
  setStart_Time(dop);
  return(err);
}
/***********************************************************************
* Write standard SND/AU header (28 byte).                               *
***********************************************************************/
LOCAL int putSNDhdr(DOBJ *dop)
{
  void   *ptr;
  char    header[SND_STD_HDR];
  int     SWAP;
  size_t  numBytes;
  ENDIAN  sysEndian={MSB};
  DDESC  *dd=&(dop->ddl);

/*
 * set format-dependent items
 */
  dop->fileData = FDF_BIN;                      /* always binary data */
  SETMSBFIRST(dop->fileEndian);                   /* always MSB first */
  SWAP = DIFFENDIAN(sysEndian, dop->fileEndian);
  dop->headerSize = SND_MIN_HDR;
  dop->version = 0;
  dop->sepChars[0] = EOS;
  dop->startRecord = 0;                                     /* always */
  setStart_Time(dop);
  if(checkSound(dop, auCapsFF(dop->fileFormat), 0) <= 0) {
    strcat(applMessage, " (AU/SND format)");
    return(-1);
  }
/*
 * construct header in memory
 */
  strcpy(header, SND_MAGIC);
  ptr = (void *)(&header[strlen(SND_MAGIC)]);
  putI32((int32_t)(dop->headerSize), &ptr, SWAP);
  putI32((int32_t)(dop->numRecords * (long)(dop->recordSize)), &ptr, SWAP);
  switch(dd->coding) {
  case DC_PCM:
    switch(dd->format) {
    case DF_INT8:
      putI32(SND_LINEAR_8, &ptr, SWAP);
      break;
    case DF_INT16:
      putI32(SND_LINEAR_16, &ptr, SWAP);
      break;
    case DF_INT24:
      putI32(SND_LINEAR_24, &ptr, SWAP);
      break;
    case DF_INT32:
      putI32(SND_LINEAR_32, &ptr, SWAP);
      break;
    case DF_REAL32:
      putI32(SND_FLOAT, &ptr, SWAP);
      break;
    case DF_REAL64:
      putI32(SND_DOUBLE, &ptr, SWAP);
      break;
    default:
      asspMsgNum = AEG_ERR_BUG;
      sprintf(applMessage, "putSNDhdr: %s", getAsspMsg(AED_BAD_FORM));
      return(-1);
    }
    break;
  case DC_ALAW:
    putI32(SND_ALAW_8, &ptr, SWAP);
    break;
  case DC_uLAW:
    putI32(SND_MULAW_8, &ptr, SWAP);
    break;
  default:
    asspMsgNum = AEG_ERR_BUG;
    sprintf(applMessage, "putSNDhdr: %s", getAsspMsg(AED_BAD_FORM));
    return(-1);
  }
  putI32((int32_t)myrint(dop->sampFreq), &ptr, SWAP);
  putI32((int32_t)(dd->numFields), &ptr, SWAP);
  putI32(0, &ptr, FALSE);                   /* 4 NULL bytes in 'info' */
/*
 * write header
 */
#ifndef WRASSP
  if(dop->fp != stdout)
#endif
    rewind(dop->fp);
  numBytes = (size_t)dop->headerSize;
  if(fwrite(header, 1, numBytes, dop->fp) != numBytes) {
    setAsspMsg(AEF_ERR_WRIT, dop->filePath);
    return(-1);
  }
  return(0);
}
/***********************************************************************
* Read RIFF-WAVE header.                                               *
***********************************************************************/
LOCAL int getWAVhdr(DOBJ *dop)
{
  void    *ptr;
  char     buf[ONEkBYTE];
  int      err, SWAP;
  size_t   numBytes;
  uint16_t format, blockSize, sampleSize, extSize;
  uint32_t chunkSize; /* FORM chunk NOT compliant with AE IFF */
  uint32_t byteRate, lspPosMask;
  long     headSize, fileSize, numRecords, lenE, lenC;
  ENDIAN   sysEndian={MSB};
  DDESC   *dd;

  fileSize = getFileSize(dop);
  if(fileSize <= 0)
    return(-1);
  numBytes = SIZEOF_FORM;
  if(fread(buf, 1, numBytes, dop->fp) != numBytes) {
    setAsspMsg(AEF_ERR_READ, dop->filePath);
    return(-1);
  }
/*
 * verify format
 */
  if(!(strncmp(buf, RIFF_MAGIC, 4) == 0 &&
       strncmp(&buf[8], WAVE_MAGIC, 4) == 0)) {
    asspMsgNum = AEF_ERR_FORM;
    sprintf(applMessage, "(not WAV) in file %s", dop->filePath);
    return(-1);
  }
  SETMSBLAST(dop->fileEndian);                     /* always MSB last */
  SWAP = DIFFENDIAN(dop->fileEndian, sysEndian);
  freeDDList(dop);                   /* clear/remove data descriptors */
  dd = &(dop->ddl);
  ptr = (void *)(&buf[4]);
  chunkSize = getU32(&ptr, SWAP);               /* for debugging only */
  format = 0;                                /* use as flag for 'fmt' */
  dop->headerSize = 0;                      /* use as flag for 'data' */
  dop->numRecords = -1;                     /* use as flag for 'fact' */
  headSize = numBytes;                    /* keep track of bytes read */
/*
 * loop through chunks
 */
  err = 0;
  while((fileSize-headSize) >= SIZEOF_CHUNK && dop->headerSize == 0) {
    numBytes = SIZEOF_CHUNK;
    if(fread(buf, 1, numBytes, dop->fp) != numBytes) {
      asspMsgNum = AEF_BAD_HEAD;
      sprintf(applMessage, "(WAV format) in file %s", dop->filePath);
      return(-1);
    }
    headSize += numBytes;
    ptr = (void *)(&buf[4]);              /* first 4 chars contain ID */
    chunkSize = getU32(&ptr, SWAP);
    /* can't check on ODD here because we need the value for the data */
/*
 * decode FORMAT chunk
 */
    if(strncmp(buf, WAVE_FORM_ID, 4) == 0) {
      if(chunkSize < (SIZEOF_WAVFMT - SIZEOF_CHUNK)) {
	asspMsgNum = AEF_BAD_HEAD;
	sprintf(applMessage, "(WAV format) in file %s", dop->filePath);
	return(-1);
      }
      if(ODD(chunkSize))
	chunkSize++;                     /* zero-padding NOT included */
      numBytes = (size_t)chunkSize;
      if(fread(buf, 1, numBytes, dop->fp) != numBytes) {
	asspMsgNum = AEF_BAD_HEAD;
	sprintf(applMessage, "(WAV format) in file %s", dop->filePath);
	return(-1);
      }
      headSize += numBytes;
      ptr = (void *)buf;
      format = getU16(&ptr, SWAP);
      if(format == WAVE_EXTS) {
	dop->fileFormat = FF_WAVE_X;
	dop->version = 3; /* Revision 3 */
      }
      else {
	dop->fileFormat = FF_WAVE;
	dop->version = 1; /* old style */
      }
      dd->numFields = (size_t)getU16(&ptr, SWAP);
      dop->sampFreq = (double)getU32(&ptr, SWAP);
      byteRate = getU32(&ptr, SWAP);    /* only of interest for ADPCM */
      blockSize = getU16(&ptr, SWAP);         /* problemetic if ADPCM */
      dd->numBits = getU16(&ptr, SWAP);
      if(chunkSize >= SIZEOF_WAVFMTX_M - SIZEOF_CHUNK) {
	extSize = getU16(&ptr, SWAP);     /* extensible part of chunk */
	if(extSize == WAVE_FMTX_MAX) {
	  dd->numBits = getU16(&ptr, SWAP);  /* valid bits per sample */
	  lspPosMask = getU32(&ptr, SWAP);   /* loudspeaker positions */
	  format = getU16(&ptr, SWAP);        /* first 2 Byte of GUID */
	}
	else if(dop->fileFormat == FF_WAVE_X) {
	  asspMsgNum = AEF_BAD_HEAD;
	  sprintf(applMessage, "(extensible WAV format) in file %s",\
		  dop->filePath);
	  return(-1);
	}
      }
      else if(dop->fileFormat == FF_WAVE_X) {
	asspMsgNum = AEF_BAD_HEAD;
	sprintf(applMessage, "(extended WAV format) in file %s",
		dop->filePath);
	return(-1);
      }
      if(dd->numFields < 1 || dop->sampFreq == 0.0) {
	asspMsgNum = AEF_BAD_HEAD;
	sprintf(applMessage, "(WAV format) in file %s", dop->filePath);
	return(-1);
      }
      dop->fileData = FDF_BIN;                  /* always binary data */
      dop->frameDur = 1;                       /* always sampled data */
      dop->startRecord = 0;                         /* per definition */
      dd->type = DT_SMP;
      switch(format) {
      case WAVE_PCM:
	dd->coding = DC_PCM;
	sampleSize = blockSize / dd->numFields;   /* bytes per sample */
	if(sampleSize * dd->numFields != blockSize) {
	  asspMsgNum = AEF_BAD_HEAD;
	  sprintf(applMessage, "(WAV format) in file %s", dop->filePath);
	  return(-1);
	}
	switch(sampleSize) {
	case 1:
	  dd->format = DF_UINT8;        /* apparently always unsigned */
	  dd->coding = DC_BINOFF;
	  dd->zeroValue = -INT8_MIN;
	  break;
	case 2:
	  dd->format = DF_INT16;          /* apparently always signed */
	  dd->zeroValue = 0;
	  break;
	case 3:
	  dd->format = DF_INT24;          /* apparently always signed */
	  dd->zeroValue = 0;
	  break;
	case 4:
	  dd->format = DF_INT32;          /* apparently always signed */
	  dd->zeroValue = 0;
	  break;
	default:
	  asspMsgNum = AEF_BAD_HEAD;
	  sprintf(applMessage, "(WAV format) in file %s", dop->filePath);
	  return(-1);
	}
	break;
      case WAVE_FLOAT:       /* IEEE 754 float; range (+1.0 ... -1.0] */
	dd->coding = DC_PCM; /* OUGHT TO DO SOMETHING ABOUT THIS */
	dd->zeroValue = 0;
	sampleSize = blockSize / dd->numFields;   /* bytes per sample */
	if(sampleSize * dd->numFields != blockSize) {
	  asspMsgNum = AEF_BAD_HEAD;
	  sprintf(applMessage, "(WAV format) in file %s", dop->filePath);
	  return(-1);
	}
	switch(sampleSize) {
	case 4:
	  dd->format = DF_REAL32;
	  dd->numBits = 32;
	  break;
	case 8:
	  dd->format = DF_REAL64;
	  dd->numBits = 64;
	  break;
	default:
	  asspMsgNum = AEF_BAD_HEAD;
	  sprintf(applMessage, "(WAV format) in file %s", dop->filePath);
	  return(-1);
	}
	break;
      case WAVE_ALAW:
      case IBM_ALAW:
	dd->format = DF_UINT8;
	dd->coding = DC_ALAW;
	dd->numBits = 8;
	break;
      case WAVE_MULAW:
      case IBM_MULAW:
	dd->format = DF_UINT8;
	dd->coding = DC_uLAW;
	dd->numBits = 8;
	break;
      case WAVE_ADPCM: /* following not supported but shown by ffinfo */
	dd->format = DF_BIT;
	dd->coding = DC_MS_ADPCM;
	break;
      case OKI_ADPCM:
	dd->format = DF_BIT;
	dd->coding = DC_OKI_ADPCM;
	break;
      case IBM_ADPCM:
	dd->format = DF_BIT;
	dd->coding = DC_IBM_ADPCM;
	break;
      case CL_ADPCM:
	dd->format = DF_BIT;
	dd->coding = DC_CL_ADPCM;
	break;
      case IDVI_ADPCM:
	dd->format = DF_BIT;
	dd->coding = DC_IDVI_ADPCM;
	break;
      case MS_ISO_MP3:
	dd->format = DF_BIT;
	dd->coding = DC_MPEG3;
	break;
      default:
	dd->coding = DC_ERROR;
	asspMsgNum = AED_BAD_FORM;
	sprintf(applMessage, "(WAV format) in file %s",	dop->filePath);
	return(-1);
      }
      dd->ident = strdup("audio");
      setRecordSize(dop);
    }
/*
 * decode FACT chunk
 */
    else if(strncmp(buf, WAVE_FACT_ID, 4) == 0) {
      if(ODD(chunkSize))
	chunkSize++;                     /* zero-padding NOT included */
      headSize += (long)chunkSize;
      if(fread(buf, 1, 4, dop->fp) != 4) {/* only read the first item */
	asspMsgNum = AEF_BAD_HEAD;
	sprintf(applMessage, "(WAV format) in file %s", dop->filePath);
	return(-1);
      }
      ptr = (void *)buf;
      dop->numRecords = (long)getU32(&ptr, SWAP);
      if((fileSize-headSize) >= SIZEOF_CHUNK)    /* chunk must follow */
	fseek(dop->fp, headSize, SEEK_SET); /* skip rest (may be big) */
      else {
	asspMsgNum = AEF_BAD_HEAD;
	sprintf(applMessage, "(WAV format) in file %s",	dop->filePath);
	return(-1);
      }
    }
/*
 * decode DATA chunk
 */
    else if(strncmp(buf, WAVE_DATA_ID, 4) == 0) {
      dop->headerSize = headSize;          /* data follow immediately */
      if(dop->recordSize > 0) {
	numRecords = (long)(chunkSize / dop->recordSize);
	if(dop->numRecords >= 0) {
	  if(dop->numRecords != numRecords) {
	    err = 1;
	    asspMsgNum = AWF_BAD_ITEM;
	    sprintf(applMessage, "(WAV format) in file %s\n"\
		    "         Incorrect number of records (%ld, using %ld)",\
		    dop->filePath, dop->numRecords, numRecords);
	    dop->numRecords = numRecords;
	  }
	}
	else
	  dop->numRecords = numRecords;
      }
      else if(dop->numRecords < 0)
	dop->numRecords = 0;
      break;                                /* found what we came for */
    }
/* skip uninteresting chunks */
    else {
      if(ODD(chunkSize))
	chunkSize++;                     /* zero-padding NOT included */
      headSize += (long)chunkSize;
      if((fileSize-headSize) >= SIZEOF_CHUNK)      /* chunk following */
	fseek(dop->fp, headSize, SEEK_SET);  /* position file pointer */
    }
  }

  if(format < 1 || dop->headerSize <= 0 || dop->numRecords < 0) {
    asspMsgNum = AEF_BAD_HEAD;
    sprintf(applMessage, "(WAV format) in file %s", dop->filePath);
    return(-1);
  }
  if(dop->recordSize > 0) {
    numRecords = (fileSize - dop->headerSize) / (long)(dop->recordSize);
    if(dop->numRecords > numRecords) {          /* fewer is possible */
      lenE = (dop->numRecords) * (long)(dop->recordSize);
      lenC = numRecords * (long)(dop->recordSize);
      asspMsgNum = AWF_BAD_ITEM;
      sprintf(applMessage, "(WAV format) in file %s\n"\
	      "         Incorrect data length (%ld, using %ld)",\
	      dop->filePath, lenE, lenC);
      dop->numRecords = numRecords;
      err = 1;
    }
  }
  setStart_Time(dop);
  return(err);
}

/***********************************************************************
* Write minimal RIFF-WAVE header (see 'genWAVhdr').                    *
***********************************************************************/
LOCAL int putWAVhdr(DOBJ *dop)
{
  char  *header=NULL;
  size_t numBytes;

/*
 * allocate and set the header
 */
  header = genWAVhdr(dop);
  if(header == NULL)
    return(-1);
/*
 * write header
 */
#ifndef WRASSP
  if(dop->fp != stdout)
#endif
    rewind(dop->fp);
  numBytes = (size_t)(dop->headerSize);
  if(fwrite(header, 1, numBytes, dop->fp) != numBytes) {
    header = freeWAVhdr(header);
    setAsspMsg(AEF_ERR_WRIT, dop->filePath);
    return(-1);
  }
  header = freeWAVhdr(header);
  return(0);
}

/*----------------------------------------------------------------------
| ASCII headers                                                        |
----------------------------------------------------------------------*/

/***********************************************************************
* Read KTH/SWELL header (only sampled data supported).                 *
***********************************************************************/
LOCAL int getKTHhdr(DOBJ *dop)
{
  char  *rest, buf[ONEkBYTE], *field[MAX_HDR_FIELDS];
  int    isKTH, n;
  long   fileSize, numRecords;
  DDESC *dd;

  fileSize = getFileSize(dop);
  if(fileSize <= 0)
    return(-1);
/*
 * set default values
 */
  dop->fileData = FDF_BIN;                      /* always binary data */
  SETMSBLAST(dop->fileEndian);
  dop->version = 0;
  dop->headerSize = KTH_DEF_HDR;
  dop->sampFreq = KTH_DEF_SFR;
  dop->frameDur = 1;                     /* only support sampled data */
  dop->numRecords = -1;                 /* possibly defined in header */
  dop->startRecord = 0;                             /* per definition */
  strcpy(dop->eol, KTH_EOL_STR);
  freeDDList(dop);                   /* clear/remove data descriptors */
  dd = &(dop->ddl);
  dd->type = DT_SMP;                         /* we only support audio */
  dd->coding = DC_PCM;  /* obligatory item 'data=' not set by snack ! */
  dd->format = DF_INT16;
  dd->numBits = 16;
  dd->numFields = 1;
  isKTH = FALSE; /* verify format while reading header */
  while((n=fgetl(buf, sizeof(buf), dop->fp, NULL)) > 0) {
    if(strcmp(buf, KTH_SEP_STR) == 0)        /* end of header reached */
      break;                    /* ignore possibly following comments */
    if(strncmp(buf, KTH_MAGIC, strlen(KTH_MAGIC)) == 0 ||
       strncmp(buf, KTH_MAGIC_ERR, strlen(KTH_MAGIC_ERR)) == 0 ||
       strncmp(buf, SNACK_MAGIC, strlen(SNACK_MAGIC)) == 0)
      isKTH = TRUE;
/*     n = strparse(buf, KTH_SEP_STR, field, MAX_HDR_FIELDS); */
/*     problems in PH90 because sometimes items not specified */
    n = strsplit(buf, KTH_SEP_CHAR, field, 2);
    if(n != 2) {
      if(isKTH) {
	asspMsgNum = AEF_BAD_HEAD;
	sprintf(applMessage, "(KTH format) in file %s", dop->filePath);
      }
      else {
	asspMsgNum = AEF_ERR_FORM;
	sprintf(applMessage, "(not KTH) in %s", dop->filePath);
      }
      return(-1);
    }
    if(strncmp(field[0], "head", 4) == 0) {
      dop->headerSize = strtol(field[1], &rest, 10);
      if(strlen(rest) > 0) {
	asspMsgNum = AEF_BAD_HEAD;
	sprintf(applMessage, "(KTH format) in file %s", dop->filePath);
	return(-1);
      }
    }
    else if(strcmp(field[0], "file") == 0) {
      if(strcmp(field[1], "samp") != 0)  /* only support sampled data */
	dd->type = DT_UNDEF;
    }
    else if(strcmp(field[0], "data") == 0) {
      if(strcmp(field[1], "int16") == 0) {
	dd->coding = DC_PCM;
	dd->format = DF_INT16;
	dd->numBits = 16;
      }
      else dd->coding = DC_UNDEF;    /* no other coding supported yet */
    }
    else if(strcmp(field[0], "msb") == 0) {
      if(strcmp(field[1], "first") == 0) {
	SETMSBFIRST(dop->fileEndian);
      }
      else if(strcmp(field[1], "last") == 0) {
	SETMSBLAST(dop->fileEndian);
      }
      else {
	asspMsgNum = AEF_BAD_HEAD;
	sprintf(applMessage, "(KTH format) in file %s", dop->filePath);
	return(-1);
      }
    }
    else if(strcmp(field[0], "nchans") == 0) {
      dd->numFields = (size_t)strtol(field[1], &rest, 10);
    }
    else if(strcmp(field[0], "sftot") == 0) {
      dop->sampFreq = strtod(field[1], &rest);
    }
    else if(strcmp(field[0], "length") == 0) {
      dop->numRecords = strtol(field[1], &rest, 10);
    } /* else ignore item */
  }
  if(n <= 0 || !isKTH) {
    asspMsgNum = AEF_ERR_FORM;
    sprintf(applMessage, "(not KTH) in %s", dop->filePath);
    return(-1);
  }
  if(dd->type != DT_SMP || dd->coding != DC_PCM) {
    asspMsgNum = AED_NOHANDLE;
    sprintf(applMessage, "(KTH format) in file %s", dop->filePath);
    return(-1);
  }
  dd->ident = strdup("audio");
  if(dd->numFields > 1) {
    dop->sampFreq /= (double)(dd->numFields);
    if(dop->numRecords > 0)
      dop->numRecords /= (long)(dd->numFields);
  }
  setStart_Time(dop);
  setRecordSize(dop);
  numRecords = (fileSize - dop->headerSize) / (long)(dop->recordSize);
  if(dop->numRecords < 0) {                  /* not defined in header */
    dop->numRecords = numRecords;
  }
  else if(dop->numRecords > numRecords) {        /* fewer is possible */
    asspMsgNum = AWF_BAD_ITEM;
    sprintf(applMessage, "(KTH format) in file %s\n"\
	    "         Incorrect number of records (%ld, using %ld)",\
	    dop->filePath, dop->numRecords, numRecords);
    dop->numRecords = numRecords;
    return(1);
  }
  return(0);
}
/***********************************************************************
* Write KTH/SWELL header (at present only for standard audio data)     *
***********************************************************************/
LOCAL int putKTHhdr(DOBJ *dop)
{
  char  *cPtr, header[KTH_DEF_HDR];
  DDESC *dd=&(dop->ddl);

/*
 * set format-dependent items
 */
  dop->fileData = FDF_BIN;                      /* always binary data */
  if(MSBUNDEF(dop->fileEndian))
    SETENDIAN(dop->fileEndian);                /* take machine endian */
  dop->headerSize = KTH_DEF_HDR;             /* default size suffices */
  dop->version = 0;
  dop->sepChars[0] = EOS;
  strcpy(dop->eol, KTH_EOL_STR);
  dop->startRecord = 0;                                     /* always */
  setStart_Time(dop);
  if(checkSound(dop, auCapsFF(dop->fileFormat), 0) <= 0) {
    strcat(applMessage, " (KTH format)");
    return(-1);
  }
/*
 * construct header in memory
 */
  memset((void *)header, 0, KTH_DEF_HDR);             /* clear header */
  sprintf(header, "head=%ld", dop->headerSize);         /* used as ID */
  strcat(header, dop->eol);
  strcat(header, "file=samp");
  strcat(header, dop->eol);
  strcat(header, "data=int16");
  strcat(header, dop->eol);
  strcat(header, "msb=");
  if(MSBFIRST(dop->fileEndian))
    strcat(header, "first");
  else
    strcat(header, "last");
  strcat(header, dop->eol);
  cPtr = &header[strlen(header)];                      /* set pointer */
  sprintf(cPtr, "nchans=%ld", (long)(dd->numFields));
  strcat(header, dop->eol);
/*   if(dop->numRecords > 0) { */
/*     cPtr = &header[strlen(header)]; */
/*     sprintf(cPtr, "length=%ld", dop->numRecords * dd->numFields); */
/*     strcat(header, dop->eol); */
/*   } */
  cPtr = &header[strlen(header)];                      /* set pointer */
  sprintf(cPtr, "sftot=%ld", (long)((dop->sampFreq * dd->numFields) + 0.5));
  strcat(cPtr, dop->eol);
  strcat(header, "=");                           /* set end of header */
  strcat(header, dop->eol);
  cPtr += (strlen(cPtr) + 1);    /* (comment) terminated by NULL-byte */
  *(cPtr++) = 0x04;                          /* end of comment (^D^Z) */
  *cPtr = 0x1A;
/*
 * write header
 */
#ifndef WRASSP
  if(dop->fp != stdout)
#endif
    rewind(dop->fp);
  if(fwrite(header, 1, KTH_DEF_HDR, dop->fp) != KTH_DEF_HDR) {
    setAsspMsg(AEF_ERR_WRIT, dop->filePath);
    return(-1);
  }
  return(0);
}
/***********************************************************************
* Read NIST-SPHERE header.                                             *
***********************************************************************/
LOCAL int getNISThdr(DOBJ *dop)
{
  char   buf[ONEkBYTE], *field[MAX_HDR_FIELDS], *rest;
  int    n, bytePerSample=0;
  long   fileSize;
  DDESC *dd;

  fileSize = getFileSize(dop);
  if(fileSize <= 0)
    return(-1);
/*
 * verify format
 */
  n = fgetl(buf, sizeof(buf), dop->fp, NULL);
  if(n <= 0 || strcmp(buf, "NIST_1A") != 0) {
    asspMsgNum = AEF_ERR_FORM;
    sprintf(applMessage, "(not NIST) in file %s", dop->filePath);
    return(-1);
  }
  n = fgetl(buf, sizeof(buf), dop->fp, NULL);/* line with header size */
  if(n <= 0 || n >= 8) {
    asspMsgNum = AEF_ERR_FORM;
    sprintf(applMessage, "(not NIST) in file %s", dop->filePath);
    return(-1);
  }
  dop->headerSize = strtol(buf, &rest, 10);   /* skips initial blanks */
  if(strlen(rest) > 0 || (dop->headerSize) % 1024 != 0) {
    asspMsgNum = AEF_BAD_HEAD;
    sprintf(applMessage, "(NIST format) in file %s", dop->filePath);
    return(-1);
  }
/*
 * set default values
 */
  dop->fileData = FDF_BIN;                      /* always binary data */
  SETENDIAN(dop->fileEndian);               /* default: system endian */
  dop->version = 1;
  dop->sampFreq = 16000.0;
  dop->frameDur = 1;                           /* always sampled data */
  dop->numRecords = -1;
  dop->startRecord = 0;                             /* per definition */
  strcpy(dop->eol, NIST_EOL_STR);
  freeDDList(dop);                   /* clear/remove data descriptors */
  dd = &(dop->ddl);
  dd->type = DT_SMP;
  dd->coding = DC_PCM;
  while((n=fgetl(buf, sizeof(buf), dop->fp, NULL)) >= 0) {
    if(n > 0) {
      if(strcmp(buf, NIST_EOH_STR) == 0)     /* end of header reached */
	break;
      if(ftell(dop->fp) >= dop->headerSize) {
	asspMsgNum = AEF_BAD_HEAD;
	sprintf(applMessage, "(NIST format) in file %s", dop->filePath);
	return(-1);
      }
      n = strparse(buf, NULL, field, MAX_HDR_FIELDS);
      if(n < 3) {
	asspMsgNum = AEF_BAD_HEAD;
	sprintf(applMessage, "(NIST format) in file %s", dop->filePath);
	return(-1);
      }
      if(n == 3) {                  /* keyword, format, value triplet */
	if(strcmp(field[0], "sample_sig_bits") == 0)
	  dd->numBits = (uint16_t)strtol(field[2], &rest, 10);
	else if(strcmp(field[0], "sample_n_bytes") == 0)
	  bytePerSample = strtol(field[2], &rest, 10);
	else if(strcmp(field[0], "channel_count") == 0)
	  dd->numFields = (size_t)strtol(field[2], &rest, 10);
	else if(strcmp(field[0], "sample_count") == 0)
	  dop->numRecords = strtol(field[2], &rest, 10);
	else if(strcmp(field[0], "sample_coding") == 0) {
	  if(strcmp(field[2], "pcm") == 0)
	    dd->coding = DC_PCM;
	  else if(strcmp(field[2], "ulaw") == 0) {
	    dd->coding = DC_uLAW;
	    dd->format = DF_UINT8;
	    dd->numBits = 8;
	  }
	}
	else if(strcmp(field[0], "sample_rate") == 0) {
	  if(strchr(field[1], (int)'i') != NULL)
	    dop->sampFreq = (double)strtol(field[2], &rest, 10);
	  else
	    dop->sampFreq = strtod(field[2], NULL);
	}
	else if(strcmp(field[0], "sample_byte_format") == 0) {
	  if(strcmp(field[2], "10") == 0)
	    SETMSBFIRST(dop->fileEndian);
	  else if(strcmp(field[2], "01") == 0)
	    SETMSBLAST(dop->fileEndian);
	}
      }                                           /* else ignore item */
    }
  }
  if(n < 0) {
    if(ferror(dop->fp))
      asspMsgNum = AEG_ERR_SYS;
    else {
      asspMsgNum = AEF_BAD_HEAD;
      sprintf(applMessage, "(NIST format) in file %s", dop->filePath);
    }
    return(-1);
  }
  if(dd->numBits < 1)
    dd->numBits = bytePerSample * 8;
  else
    bytePerSample = PCM_BYTES(dd);
  if(dd->coding == DC_PCM) {
    switch(bytePerSample) {
    case 1:
      dd->format = DF_INT8;            /* apparently no unsigned data */
      break;
    case 2:
      dd->format = DF_INT16;
      break;
    case 3:
      dd->format = DF_INT24;
      break;
    case 4:
      dd->format = DF_INT32;
      break;
    default:
      dd->format = DF_ERROR;
      asspMsgNum = AED_NOHANDLE;
      sprintf(applMessage, "(NIST format) in file %s", dop->filePath);
      return(-1);
    }
  }
  dd->ident = strdup("audio");
  setStart_Time(dop);
  setRecordSize(dop);
  if(dop->numRecords < 0)
    dop->numRecords = (fileSize - dop->headerSize) / (long)(dop->recordSize);
  return(0);
}
/***********************************************************************
* Write standard NIST-SPHERE header (1024 byte).                       *
***********************************************************************/
LOCAL int putNISThdr(DOBJ *dop)
{
  char  *cPtr, header[ONEkBYTE];
  size_t numBytes;
  DDESC *dd=&(dop->ddl);

/*
 * set format-dependent items
 */
  dop->fileData = FDF_BIN;                      /* always binary data */
  if(MSBUNDEF(dop->fileEndian))
    SETENDIAN(dop->fileEndian);                /* take machine endian */
  dop->headerSize = ONEkBYTE;                /* default size suffices */
  dop->version = 1;
  dop->sepChars[0] = EOS;
  strcpy(dop->eol, NIST_EOL_STR);
  dop->startRecord = 0;                                     /* always */
  setStart_Time(dop);
  if(checkSound(dop, auCapsFF(dop->fileFormat), 0) <= 0) {
    strcat(applMessage, " (NIST format)");
    return(-1);
  }
/*
 * construct header in memory
 */
  memset((void *)header, 0, ONEkBYTE);                /* clear header */
  strcpy(header, NIST_MAGIC);
  cPtr = &header[strlen(header)];                      /* set pointer */
  sprintf(cPtr, "channel_count -i %ld", (long)(dd->numFields));
  strcat(header, dop->eol);
  cPtr = &header[strlen(header)];
  sprintf(cPtr, "sample_count -i %ld", dop->numRecords);
  strcat(header, dop->eol);
  cPtr = &header[strlen(header)];
  sprintf(cPtr, "sample_rate -i %ld", (long)myrint(dop->sampFreq));
  strcat(header, dop->eol);
  cPtr = &header[strlen(header)];
  sprintf(cPtr, "sample_n_bytes -i %d", (int)PCM_BYTES(dd));
  strcat(header, dop->eol);
  cPtr = &header[strlen(header)];
  if(dd->numBits > 8) {
    sprintf(cPtr, "sample_byte_format -s2 ");
    if(MSBFIRST(dop->fileEndian))
      strcat(header, "10");
    else
      strcat(header, "01");
  }
  else
    sprintf(cPtr, "sample_byte_format -s1 1");
  strcat(header, dop->eol);
  cPtr = &header[strlen(header)];
  sprintf(cPtr, "sample_sig_bits -i %d", (int)(dd->numBits));
  strcat(header, dop->eol);
  cPtr = &header[strlen(header)];
  sprintf(cPtr, "sample_coding ");
  if(dd->coding == DC_uLAW)
    strcat(header, "-s4 ulaw");
  else
    strcat(header, "-s3 pcm");
  strcat(header, dop->eol);
  strcat(header, NIST_EOH_STR);
  strcat(header, dop->eol);
/*
 * write header
 */
#ifndef WRASSP
  if(dop->fp != stdout)
#endif
    rewind(dop->fp);
  numBytes = (size_t)dop->headerSize;
  if(fwrite(header, 1, numBytes, dop->fp) != numBytes) {
    setAsspMsg(AEF_ERR_WRIT, dop->filePath);
    return(-1);
  }
  return(0);
}
/***********************************************************************
* Read SSFF header.                                                    *
***********************************************************************/
LOCAL int getSSFFhdr(DOBJ *dop)
{
  char   buf[ONEkBYTE], bak[ONEkBYTE], *field[MAX_HDR_FIELDS], *rest;
  int    n, FIRST, FIRSTMETA;
  long   fileSize;
  KDTAB *entry;
  DDESC *dd;
  SSFFST *ssff_type;
  TSSFF_Generic *genVar;

  fileSize = getFileSize(dop);
  if(fileSize <= 0)
    return(-1);
/*
 * verify format
 */
  n = fgetl(buf, sizeof(buf), dop->fp, NULL);
  strncpy(bak, buf, strlen(buf));
  if(n <= 0 || strcmp(buf, SSFF_MAGIC) != 0) {
    asspMsgNum = AEF_ERR_FORM;
    sprintf(applMessage, "(not SSFF) in file %s", dop->filePath);
    return(-1);
  }
/*
 * set default values
 */
  dop->fileData = FDF_BIN;                /* apparently always binary */
  CLRENDIAN(dop->fileEndian);
  dop->version = 0;
  dop->dataRate = dop->sampFreq = 0.0;
  dop->Start_Time = 0.0;
  dop->frameDur = -1;            /* previously only data rate defined */
  dop->sepChars[0] = EOS;
  strcpy(dop->eol, SSFF_EOL_STR);
  freeDDList(dop);                   /* clear/remove data descriptors */
  dd = &(dop->ddl);
  dd->format = DF_ERROR;
  FIRST = TRUE;       /* first data descriptor needs not be allocated */
  FIRSTMETA = TRUE; /* first meta variable needs not be allocated */
  while(fgetl(buf, sizeof(buf), dop->fp, NULL) > 0) {
    strncpy(bak, buf, strlen(buf)); /* retain a copy of the line because strparse will alter it */
  	if(strcmp(buf, SSFF_EOH_STR) == 0) {     /* end of header reached */
      dop->headerSize = ftell(dop->fp);    /* data follow immediately */
      break;
    }
    n = strparse(buf, NULL, field, MAX_HDR_FIELDS);
    if(n > 1) {                                   /* otherwise ignore */
      if(strcmp(field[0], SSFF_SYS_ID) == 0) {
	if(n < 2) {
	  asspMsgNum = AEF_BAD_HEAD;
	  sprintf(applMessage, "(SSFF format) in file %s", dop->filePath);
	  return(-1);
	}
	if(strcmp(field[1], SSFF_MSB_LAST) == 0 ||
	   strcmp(field[1], SSFF_MSB_LAST2) == 0)
	  SETMSBLAST(dop->fileEndian);
	else
	  SETMSBFIRST(dop->fileEndian);
      }
      else if(strcmp(field[0], SSFF_RATE_ID) == 0) {
	if(n < 2) {
	  asspMsgNum = AEF_BAD_HEAD;
	  sprintf(applMessage, "(SSFF format) in file %s", dop->filePath);
	  return(-1);
	}
	dop->dataRate = strtod(field[1], &rest);
      }
      else if(strcmp(field[0], SSFF_TIME_ID) == 0) {
	if(n < 2) {
	  asspMsgNum = AEF_BAD_HEAD;
	  sprintf(applMessage, "(SSFF format) in file %s", dop->filePath);
	  return(-1);
	}
	dop->Start_Time = strtod(field[1], &rest); 
	if(dop->Start_Time < 0.0) {
	  asspMsgNum = AEF_BAD_HEAD;
	  sprintf(applMessage, "(negative start time) in file %s",\
		  dop->filePath);
	  return(-1);
	}
      }
      else if(strcmp(field[0], SSFF_DATA_ID) == 0) {
	if(n < 4) {
	  asspMsgNum = AEF_BAD_HEAD;
	  sprintf(applMessage, "(SSFF format) in file %s", dop->filePath);
	  return(-1);
	}
	if(!FIRST) {                          /* multi-parameter data */
	  dd = addDDesc(dop);
	  if(!dd)
	    return(-1);
	}
	FIRST = FALSE;           /* next descriptor must be allocated */
	dd->ident = strdup(field[1]);          /* copy identification */
	entry = keyword2entry(field[1], KDT_SSFF);    /* search entry */
	dd->type = entry->dataType;
	if(entry->factor)
	  strcpy(dd->factor, entry->factor);
	if(entry->unit)
	  strcpy(dd->unit, entry->unit);
	if(dd->type == DT_SMP)                   /* also in table ??? */
	  dd->coding = DC_PCM;
	else
	  dd->coding = DC_LIN;
	if(strcmp(field[2], "CHAR") == 0) {
	  dd->format = DF_CHAR;
	  dd->coding = DC_UNDEF;
	  dd->numBits = 8;
	}
	else if(strcmp(field[2], "BYTE") == 0) {
	  dd->format = DF_UINT8;
	  dd->numBits = 8;
	}
	else if(strcmp(field[2], "SHORT") == 0) {
	  dd->format = DF_INT16;
	  dd->numBits = 16;
	}
	else if(strcmp(field[2], "LONG") == 0) {
	  dd->format = DF_INT32;
	  dd->numBits = 32;
	}
	else if(strcmp(field[2], "FLOAT") == 0) {
	  dd->format = DF_REAL32;
	  dd->numBits = 32;
	}
	else if(strcmp(field[2], "DOUBLE") == 0) {
	  dd->format = DF_REAL64;
	  dd->numBits = 64;
	}
	else {
	  dd->format = DF_ERROR;
	  asspMsgNum = AEF_BAD_HEAD;
	  sprintf(applMessage, "(SSFF format) in file %s", dop->filePath);
	  return(-1);
	}
	dd->numFields = (size_t)strtol(field[3], &rest, 10);
      }
      else if(strcmp(field[0], SSFF_REF_RATE) == 0) {      /* NEW !!! */
	if(n < 3) {
	  asspMsgNum = AEF_BAD_HEAD;
	  sprintf(applMessage, "(SSFF format) in file %s", dop->filePath);
	  return(-1);
	}
	dop->sampFreq = strtod(field[2], &rest);
      } else if (n > 2) {
      	/* this is probably a generic variable 
      	 * if so than field[1] must be a valid SSFF size/type thingy as defined above 
      	 * in SSFF_TYPES
      	 */
      	for (ssff_type = SSFF_TYPES; ssff_type->type != SSFF_UNDEF; ssff_type++) {
      		if (strncmp(field[1], ssff_type->ident, strlen(ssff_type->ident)) == 0)
      			break;
      	}
      	if (ssff_type->type == SSFF_UNDEF) {
      		/* unable to find type, this is not a generic variable
      		   just ignore silently */
      	} else {
      		/* this is a generic variable */
      		if (!FIRSTMETA) {
      			genVar = addTSSFF_Generic(dop);
      			if (genVar == NULL)
      				return(-1);
      		} else {
      			genVar = &(dop->meta);
      		}
      		FIRSTMETA = FALSE;
      		genVar->type = ssff_type->type;
      		genVar->ident = strdup(field[0]);
      		rest = bak + (field[2] - buf);
      		genVar->data = strdup(rest);
      	}
      }
    } /* else ignore item */
  }

  if(ferror(dop->fp)) {
    setAsspMsg(AEF_ERR_READ, dop->filePath);
    return(-1);
  }
  if(feof(dop->fp) || MSBUNDEF(dop->fileEndian) || dd->format <= DF_UNDEF ||
     dop->headerSize == 0 || dop->dataRate == 0.0) {
    asspMsgNum = AEF_BAD_HEAD;
    sprintf(applMessage, "(SSFF format) in file %s", dop->filePath);
    return(-1);
  }
  if(dop->ddl.type == DT_SMP) {
    dop->sampFreq = dop->dataRate;
    dop->frameDur = 1;
  }
  if(checkRates(dop) < 0) {   /* sync sampFreq, dataRate and frameDur */
    sprintf(applMessage, "(SSFF format) in file %s", dop->filePath);
    return(-1);
  }
  adjustTiming(dop);                 /* set startRecord and Time_Zero */
  setRecordSize(dop);
  if(dop->recordSize > 0)
    dop->numRecords = (fileSize - dop->headerSize) / (long)(dop->recordSize); 
  return(0);
}
/***********************************************************************
* Write SSFF header.                                                   *
* NOTE: It is the caller's responsibility to set the items 'Start_Time'*
*       and 'Time_Zero' correctly before calling this function.        *
***********************************************************************/
LOCAL int putSSFFhdr(DOBJ *dop)
{
  char   header[ONEkBYTE], format[10], *cPtr, *ident;
  int    nd;
  size_t numBytes;
  double frameRate;
  KDTAB *entry;
  DDESC *dd=&(dop->ddl);
  TSSFF_Generic *genVar;
  SSFFST *ssff_types;

/*
 * set format-dependent items
 */
  if(checkRates(dop) < 0) {   /* sync sampFreq, dataRate and frameDur */
    asspMsgNum = AEG_ERR_BUG;
    sprintf(applMessage, "putSSFFhdr: %s", getAsspMsg(AED_ERR_RATE));
    return(-1);
  }
  dop->fileData = FDF_BIN;
  if(MSBUNDEF(dop->fileEndian))
    SETENDIAN(dop->fileEndian);
  dop->version = 0;
  dop->sepChars[0] = EOS;
  strcpy(dop->eol, SSFF_EOL_STR);
/*
 * construct header in memory
 */
  strcpy(header, SSFF_MAGIC);
  strcat(header, dop->eol);
  cPtr = &header[strlen(header)];                      /* set pointer */
  sprintf(cPtr, "%s %s", SSFF_SYS_ID,\
	  MSBFIRST(dop->fileEndian) ? SSFF_MSB_FIRST : SSFF_MSB_LAST);
  strcat(header, dop->eol);
  if(dop->dataRate > 0.0)
    frameRate = dop->dataRate;
  else
    frameRate = dop->sampFreq;
  nd = numDecim(frameRate, 12);
  if(nd <= 0) nd = 1;
  cPtr = &header[strlen(header)];
  sprintf(cPtr, "%s %.*f", SSFF_RATE_ID, nd, frameRate);
  strcat(header, dop->eol);
  nd = numDecim(dop->Start_Time, 12);
  if(nd <= 0) nd = 1;
  cPtr = &header[strlen(header)];
  sprintf(cPtr, "%s %.*f", SSFF_TIME_ID, nd, dop->Start_Time);
  strcat(header, dop->eol);

  while(dd != NULL) {
    ident = dd->ident;
    if(ident == NULL) {
      entry = dtype2entry(dd->type, KDT_SSFF);        /* search entry */
      if(entry->keyword == NULL) {
	setAsspMsg(AEG_ERR_BUG, "putSSFFhdr: track name missing");
	return(-1);
      }
      ident = entry->keyword;
      dd->ident = strdup(ident);
    }
    switch(dd->format) {
    case DF_CHAR:
      strcpy(format, "CHAR");
      break;
    case DF_UINT8:
      strcpy(format, "BYTE");
      break;
    case DF_INT16:
      strcpy(format, "SHORT");
      break;
    case DF_INT32:
      strcpy(format, "LONG");
      break;
    case DF_REAL32:
      strcpy(format, "FLOAT");
      break;
    case DF_REAL64:
      strcpy(format, "DOUBLE");
      break;
    default:
      asspMsgNum = AEG_ERR_BUG;
      sprintf(applMessage, "putSSFFhdr: %s", getAsspMsg(AED_ERR_FORM));
      return(-1);
    }
    if(dd->numFields < 1)
      dd->numFields = 1;
    cPtr = &header[strlen(header)];
    sprintf(cPtr, "%s %s %s %ld", SSFF_DATA_ID,\
	    ident, format, (long)(dd->numFields));
    strcat(header, dop->eol);
    dd = dd->next;
  }
  /* NEW !!! */
  if(dop->sampFreq > 0.0 && dop->ddl.type != DT_SMP && dop->frameDur != 0) {
    nd = numDecim(dop->sampFreq, 12);
    if(nd <= 0) nd = 1;
    cPtr = &header[strlen(header)];
    sprintf(cPtr, "%s DOUBLE %.*f", SSFF_REF_RATE, nd, dop->sampFreq);
    strcat(header, dop->eol);
  }
  /* 
   * NEW !!! 
   * insert generic variables
   */
  genVar = &(dop->meta);
  for (; genVar != NULL; genVar=genVar->next) {
    if (genVar->ident == NULL)
      break; /* fixed var is empty */
  	for (ssff_types = SSFF_TYPES; ssff_types->type!= SSFF_UNDEF; ssff_types++) {
  		if (ssff_types->type == genVar->type)
  			break;
  	}
  	if (ssff_types->type == SSFF_UNDEF) {
  		/* bad type */
  		setAsspMsg(AED_ERR_TYPE, "Badly defined generic variable.");
  		return(-1);
  	} else {
  		cPtr = &header[strlen(header)];
  		sprintf(cPtr, "%s %s %s", genVar->ident, ssff_types->ident, genVar->data);
  		strcat(header, dop->eol);
  	}

  } 

  strcat(header, SSFF_EOH_STR);
  strcat(header, dop->eol);
  dop->headerSize = (long)(numBytes=strlen(header));
/*
 * write header
 */
#ifndef WRASSP
  if(dop->fp != stdout)
#endif
    rewind(dop->fp);
  if(fwrite(header, 1, numBytes, dop->fp) != numBytes) {
    setAsspMsg(AEF_ERR_WRIT, dop->filePath);
    return(-1);
  }
  return(0);
}
/***********************************************************************
* Read XASSP-ASCII header.                                             *
***********************************************************************/
LOCAL int getXASSPhdr(DOBJ *dop)
{
  char   buf[ONEkBYTE], *field[MAX_HDR_FIELDS], *rest, eolCode[4];
  int    n, len;
  double time;
  KDTAB *entry;
  DDESC *dd;

  rewind(dop->fp);
/*
 * verify format
 */
  len = fgetl(buf, sizeof(buf), dop->fp, &rest);
  strcpy(eolCode, rest);
  if(len <= 0) {
    asspMsgNum = AEF_ERR_FORM;
    sprintf(applMessage, "(not XASSP) in file %s", dop->filePath);
    return(-1);
  }
  if(strncmp(buf, XASSP_MAGIC, strlen(XASSP_MAGIC)) != 0) {
    asspMsgNum = AEF_ERR_FORM;
    sprintf(applMessage, "(not XASSP) in file %s", dop->filePath);
    return(-1);
  }
  n = strparse(buf, NULL, field, MAX_HDR_FIELDS);
  if(n < 2) {
    asspMsgNum = AEF_BAD_HEAD;
    sprintf(applMessage, "(XASSP format) in file %s", dop->filePath);
    return(-1);
  }

  dop->fileData = FDF_ASC;                       /* always ASCII data */
  CLRENDIAN(dop->fileEndian);
  dop->headerSize = ftell(dop->fp);
  dop->version = 0;
  dop->startRecord = 0;
  dop->numRecords = 0;                             /* can't determine */
  dop->recordSize = 0;                                    /* variable */
  strcpy(dop->sepChars, XASSP_SEP_STR);
  strcpy(dop->eol, eolCode);
  freeDDList(dop);                   /* clear/remove data descriptors */
  dd = &(dop->ddl);
  dd->numFields = 1;                                   /* per default */
  if(n > 2) {
    dop->sampFreq = strtod(field[2], &rest);
    if(n > 3 && isdigit((int)(*field[3])))
      dop->frameDur = (int32_t)strtol(field[3], &rest, 10);
    else                        /* old version without frame duration */
      dop->frameDur = 1;
  }
  else /* data log */
    dop->sampFreq = dop->dataRate = 0.0;

  entry = keyword2entry(field[1], KDT_XASSP);         /* search entry */
  dd->type = entry->dataType;
  if(entry->keyword)
    dd->ident = strdup(field[1]);
  if(entry->factor)
    strcpy(dd->factor, entry->factor);
  if(entry->unit)
    strcpy(dd->unit, entry->unit);
  if(dd->type != DT_DATA_LOG)               /* search first data line */
    while((len=fgetl(buf, sizeof(buf), dop->fp, NULL)) == 0) NIX;
  switch(dd->type) {
  case DT_LBL:
    dop->frameDur = 0;
    break;
  case DT_EPG:
    SETTOPVIEW(dd->orientation);
    if(len > 0) {
      n = strparse(buf, dop->sepChars, field, MAX_HDR_FIELDS);
      if(n > 1) {
	dd->numFields = n - 1;                      /* number of rows */
	time = strtod(field[0], &rest);
	dop->startRecord = TIMEtoFRMNR(time, dop->sampFreq, dop->frameDur);
      }
    }
    break;
  case DT_DATA_LOG:
    dop->frameDur = 0;
    dd->numFields = 0;                                    /* variable */
    break;
  case DT_UNDEF:
  case DT_ERROR:
    dd->type = DT_ERROR;
    asspMsgNum = AED_BAD_TYPE;
    sprintf(applMessage, "\"%s\"\n       (XASSP format) in file %s",\
	    field[1], dop->filePath);
    return(-1);
  default:                      /* all others are OK or not supported */
    if(len > 0) {
      n = strparse(buf, dop->sepChars, field, MAX_HDR_FIELDS);
      if(n > 1) {
	time = strtod(field[0], &rest);
	dop->startRecord = TIMEtoFRMNR(time, dop->sampFreq, dop->frameDur);
      }
    }
    break;    
  }
  setStart_Time(dop);
  return(0);
}
/***********************************************************************
* Write XASSP-ASCII header.                                            *
***********************************************************************/
LOCAL int putXASSPhdr(DOBJ *dop)
{
  char   header[ONEkBYTE];
  int    nd;
  size_t numBytes;
  long   frameDur;
  double frameRate;
  DDESC *dd=&(dop->ddl);

  dop->fileData = FDF_ASC;
  CLRENDIAN(dop->fileEndian);
  dop->version = 0;
  strcpy(dop->sepChars, XASSP_SEP_STR);
  if(strlen(dop->eol) == 0)
    sprintf(dop->eol, "\n");
/*   dop->recordSize = 0; BEWARE! data in memory problably binary */
  checkRates(dop);            /* sync sampFreq, dataRate and frameDur */
  if(dop->sampFreq > 0.0 && dop->frameDur >= 1) { /* KLUDGE IN XASSP !!! */
    frameRate = dop->dataRate;
    frameDur = 1;
    nd = numDecim(frameRate, 12);
  }
  else {
    if(dop->sampFreq > 0.0)
      nd = numDecim(dop->sampFreq, 10);
    else {
      nd = 1;
      if(dop->sampFreq < 0.0)
	dop->sampFreq = 0.0;
    }
    frameRate = dop->sampFreq;
    frameDur = 1;
  }
  if(nd <= 0)
    nd = 1;

  switch(dd->type) {
  case DT_LBL:
  case DT_TAG:
    dop->frameDur = 0;
    sprintf(header, "XASSP\tLABELS\t%.*f\t%ld",\
	    nd, dop->sampFreq, dop->frameDur);
    break;
  case DT_PIT:
    sprintf(header, "XASSP\tFZERO\t%.*f\t%ld",\
	    nd, frameRate, frameDur);
    break;
  case DT_RMS:
    sprintf(header, "XASSP\tENERGY\t%.*f\t%ld",\
	    nd, frameRate, frameDur);
    break;
  case DT_EPG:
    if(dop->frameDur <= 0)
      dop->frameDur = 1;
    sprintf(header, "XASSP\tPALATOGRAM\t%.*f\t%ld",\
	    nd, dop->sampFreq, dop->frameDur);
    break;
  case DT_DATA_LOG:
    dop->frameDur = 0;
    sprintf(header, "XASSP\tDATALOG");
    break;
  default:
    asspMsgNum = AEG_ERR_BUG;
    sprintf(applMessage, "putXASSPhdr: %s", getAsspMsg(AED_BAD_FORM));
    return(-1);
  }
  strcat(header, dop->eol);
  dop->headerSize = (long)(numBytes=strlen(header));
/*
 * write header
 */
#ifndef WRASSP
  if(dop->fp != stdout)
#endif
    rewind(dop->fp);
  if(fwrite(header, 1, numBytes, dop->fp) != numBytes) {
    setAsspMsg(AEF_ERR_WRIT, dop->filePath);
    return(-1);
  }
  return(0);
}
/***********************************************************************
* Read IPdS/KTH MIX header.                                            *
***********************************************************************/
LOCAL int getMIXhdr(DOBJ *dop)
{
  char   buf[ONEkBYTE], *field[MAX_HDR_FIELDS], *eolCode;
  int    n, isMIX;
  long   headSize;
  DDESC *dd;

  rewind(dop->fp);
  headSize = 0;
/*
 * verify format while reading header
 */
  isMIX = FALSE;
  while((n=fgetl(buf, sizeof(buf), dop->fp, &eolCode)) >= 0) {
    headSize = ftell(dop->fp);            /* keep track of bytes read */
    if(n > 0) {
      if(strcmp(buf, MIX_TRF_ID) == 0 || strcmp(buf, MIX_TRF_ERR) == 0) {
	isMIX = TRUE;
	break;                    /* end of fixed header part reached */
      }
      if(strcmp(buf, MIX_ORT_ID) == 0)
	isMIX = TRUE;
    }
  }
  if(n < 0 || !isMIX) {
    asspMsgNum = AEF_ERR_FORM;
    sprintf(applMessage, "(not IPdS-MIX) in file %s", dop->filePath);
    return(-1);
  }

  dop->fileData = FDF_ASC;
  CLRENDIAN(dop->fileEndian);
  dop->version = 0;
  dop->frameDur = 0;
  dop->recordSize = 0;                                    /* variable */
  dop->startRecord = 0;                             /* per definition */
  dop->numRecords = 0;                             /* can't determine */
  dop->Start_Time = dop->Time_Zero = 0.0;
  strcpy(dop->sepChars, " ");
  strcpy(dop->eol, eolCode);
  freeDDList(dop);                   /* clear/remove data descriptors */
  dd = &(dop->ddl);
  dd->ident = strdup("labels");
  dd->type = DT_LBL;
  dd->format = DF_STR;
  dd->coding = DC_MIX;
  dd->numFields = 1;
  SETBEGIN(dd->orientation);
  while((n=fgetl(buf, sizeof(buf), dop->fp, NULL)) >= 0) {
    /* seek label line */
    if(strncmp(buf, MIX_LBL_ID, strlen(MIX_LBL_ID)) == 0)
      break;
    if(strncmp(buf, MIX_SFR_ID, strlen(MIX_SFR_ID)) == 0) {
      n = strparse(buf, NULL, field, MAX_HDR_FIELDS);
      if(n > 1)
	dop->sampFreq = strtod(field[1], NULL);
    }
    headSize = ftell(dop->fp);
  }
  dop->headerSize = headSize;
/*  if(dop->sampFreq <= 0.0)                   not specified in header */
/*    dop->sampFreq = MIX_SFR;  SHOULD BE DONE BY THE CALLING FUNCTION */
  return(0);
}
/***********************************************************************
* Write IPdS/KTH MIX header.                                           *
***********************************************************************/
LOCAL int putMIXhdr(DOBJ *dop)
{
  char   *ptr, headLine[MAXMIXLINE+2];
  size_t  numBytes;
  DDESC  *dd=&(dop->ddl);
  LBLHDR *lPtr;

  if((dd->type != DT_LBL && dd->type != DT_TAG) ||
     dd->coding != DC_MIX) {
    setAsspMsg(AEB_BAD_CALL, "putMIXhdr");
    return(-1);
  }
  dop->fileData = FDF_ASC;
  CLRENDIAN(dop->fileEndian);
  dop->version = 0;
  if(dop->sampFreq <= 0.0)
    dop->sampFreq = MIX_SFR;
  dop->frameDur = 0;
  dop->recordSize = 0;                                    /* variable */
  dop->startRecord = 0;                             /* per definition */
  dop->Start_Time = dop->Time_Zero = 0.0;
  if(strlen(dop->sepChars) == 0)
    strcpy(dop->sepChars, " ");
  if(strlen(dop->eol) == 0)
    strcpy(dop->eol, MIX_EOL_STR);
  if(MARKSUNDEF(dd->orientation))
    SETBEGIN(dd->orientation);

  lPtr = (LBLHDR *)(dop->generic);
  if(lPtr != NULL && strcmp(lPtr->ident, LBL_GD_IDENT) == 0 &&
     lPtr->headCopy && lPtr->copySize > 0) {
    ptr = (char *)(lPtr->headCopy);
    numBytes = (size_t)(lPtr->copySize);
  }
  else {
    strcpy(headLine, MIX_TRF_ID);                   /* minimal header */
    strcat(headLine, dop->eol);
    ptr = headLine;
    numBytes = strlen(ptr);
  }
#ifndef WRASSP
  if(dop->fp != stdout)
#endif
    rewind(dop->fp);
  if(fwrite(ptr, 1, numBytes, dop->fp) != numBytes) {
    setAsspMsg(AEF_ERR_WRIT, dop->filePath);
    return(-1);
  }
  dop->headerSize = (long)numBytes;
/* TO BE COMMENTED IN WHEN ALL PROGRAMS INTERPRET SAMPLE RATE LINE */
/*  fprintf(dop->fp, MIX_SFR_LINE, dop->sampFreq); */
/*  fflush(dop->fp); */
  if(ferror(dop->fp)) {
    setAsspMsg(AEF_ERR_WRIT, dop->filePath);
    return(-1);
  }
  return(0);
}
/***********************************************************************
* Read IPdS SAMPA header.                                              *
***********************************************************************/
LOCAL int getSAMhdr(DOBJ *dop)
{
  char   buf[ONEkBYTE], *field[MAX_HDR_FIELDS], *eolCode;
  int    n;
  long   headSize;
  DDESC *dd;

  rewind(dop->fp);
  n = fgetl(buf, sizeof(buf), dop->fp, NULL);      /* read first line */
  if(n <= 0) {
    asspMsgNum = AEF_ERR_FORM;
    sprintf(applMessage, "(not IPdS-SAM\nin file %s", dop->filePath);
    return(-1);
  }
  strcpy(buf, mybarename(buf));             /* should contain file name */
  if(strcmp(buf, mybarename(dop->filePath)) != 0) {
    asspMsgNum = AEF_ERR_FORM;
    sprintf(applMessage, "(not IPdS-SAM\nin file %s", dop->filePath);
    return(-1);
  }
  do {
    n = fgetl(buf, sizeof(buf), dop->fp, &eolCode);
    headSize = ftell(dop->fp);            /* keep track of bytes read */
  } while(n >= 0 && strcmp(buf, SAM_EOC_ID) != 0);
  if(n <= 0) {
    asspMsgNum = AEF_BAD_HEAD;
    sprintf(applMessage, "(IPdS-SAM format) in file %s", dop->filePath);
    return(-1);
  }

  dop->fileData = FDF_ASC;
  CLRENDIAN(dop->fileEndian);
  dop->version = 0;
  dop->frameDur = 0;
  dop->recordSize = 0;                                    /* variable */
  dop->startRecord = 0;                             /* per definition */
  dop->numRecords = 0;                             /* can't determine */
  dop->Start_Time = dop->Time_Zero = 0.0;
  strcpy(dop->sepChars, " ");
  strcpy(dop->eol, eolCode);
  freeDDList(dop);                   /* clear/remove data descriptors */
  dd = &(dop->ddl);
  dd->ident = strdup("labels");
  dd->type = DT_LBL;
  dd->format = DF_STR;
  dd->coding = DC_SAM;
  dd->numFields = 1;
  SETBEGIN(dd->orientation);
  while((n=fgetl(buf, sizeof(buf), dop->fp, NULL)) >= 0) {
    /* seek end of header */
    headSize = ftell(dop->fp);
    if(strcmp(buf, SAM_EOH_ID) == 0)
      break;
    if(strncmp(buf, SAM_SFR_ID, strlen(SAM_SFR_ID)) == 0) {
      n = strparse(buf, NULL, field, MAX_HDR_FIELDS);
      if(n > 1)
	dop->sampFreq = strtod(field[1], NULL);
    }
  }
  if(ferror(dop->fp) || strcmp(buf, SAM_EOH_ID) != 0) {
    asspMsgNum = AEF_BAD_HEAD;
    sprintf(applMessage, "(IPdS-SAM format) in file %s", dop->filePath);
    return(-1);
  }
  dop->headerSize = headSize;
/*  if(dop->sampFreq <= 0.0)                   not specified in header */
/*    dop->sampFreq = MIX_SFR;  SHOULD BE DONE BY THE CALLING FUNCTION */
  return(0);
}
/***********************************************************************
* Write IPdS SAMPA header.                                             *
***********************************************************************/
LOCAL int putSAMhdr(DOBJ *dop)
{
  char   *name;
  DDESC  *dd=&(dop->ddl);
  LBLHDR *lPtr;

  if((dd->type != DT_LBL && dd->type != DT_TAG ) ||
     dd->coding != DC_SAM) {
    setAsspMsg(AEB_BAD_CALL, "putSAMhdr");
    return(-1);
  }
  dop->fileData = FDF_ASC;
  CLRENDIAN(dop->fileEndian);
  dop->version = 0;
  if(dop->sampFreq <= 0.0)
    dop->sampFreq = MIX_SFR;
  dop->frameDur = 0;
  dop->recordSize = 0;                                    /* variable */
  dop->startRecord = 0;                             /* per definition */
  dop->Start_Time = dop->Time_Zero = 0.0;
  if(strlen(dop->sepChars) == 0)
    strcpy(dop->sepChars, " ");
  if(strlen(dop->eol) == 0)
    strcpy(dop->eol, SAM_EOL_STR);
  if(MARKSUNDEF(dd->orientation))
    SETBEGIN(dd->orientation);

#ifndef WRASSP
  if(dop->fp != stdout)
#endif
    rewind(dop->fp);
  name = myfilename(dop->filePath);          /* remove directory path */
  fprintf(dop->fp, "%s%s", name, dop->eol); /* print (bad) identification */
  lPtr = (LBLHDR *)(dop->generic);
  if(lPtr != NULL && strcmp(lPtr->ident, LBL_GD_IDENT) == 0 &&
     lPtr->headCopy != NULL && lPtr->copySize > 0) {
    if(fwrite(lPtr->headCopy, sizeof(char), lPtr->copySize, dop->fp) !=\
       lPtr->copySize) {
      setAsspMsg(AEF_ERR_WRIT, dop->filePath);
      return(-1);
    }
  }
  else {                            /* orthography and canonics empty */
    fprintf(dop->fp, "%s%s%s%s%s%s",\
	    SAM_EOT_ID, dop->eol, dop->eol, SAM_EOC_ID, dop->eol, dop->eol);
  }
/* TO BE COMMENTED IN WHEN ALL PROGRAMS INTERPRET SAMPLE RATE LINE */
/*  fprintf(dop->fp, SAM_SFR_LINE, dop->sampFreq); */
  fflush(dop->fp);
  if(ferror(dop->fp)) {
    setAsspMsg(AEF_ERR_WRIT, dop->filePath);
    return(-1);
  }
/*
 * NOTE: Header must be extended by [variant field and]
 *       the end-of-header code by the calling function.
 */
#ifndef WRASSP
  if(dop->fp != stdout)
#endif
    dop->headerSize = ftell(dop->fp);
  return(0);
}
/***********************************************************************
* Read ESPS xlabel header (virtually undefined).                       *
***********************************************************************/
LOCAL int getXLBLhdr(DOBJ *dop)
{
  char     buf[ONEkBYTE], *field[MAX_HDR_FIELDS], ident[NAME_MAX+1];
  char    *signal, *font;
  int      n, isXLBL, color=-1;
  DDESC   *dd;
  XLBL_GD *gd=NULL;

  rewind(dop->fp);
  isXLBL = FALSE;            /* identification can only be very crude */
  freeDDList(dop);                   /* clear/remove data descriptors */
  signal = font = NULL;
  dd = &(dop->ddl);
  while((n=fgetl(buf, sizeof(buf), dop->fp, NULL)) >= 0) {
    if(n > 0) {
      if(strcmp(buf, XLBL_EOH_STR) == 0) {
	isXLBL = TRUE;
	break;
      }
      n = strparse(buf, NULL, field, MAX_HDR_FIELDS);
      if(n == 2) {
	strcpy(ident, mybarename(field[1]));
	if(strcmp(field[0], XLBL_REF_ID) == 0 &&
	   (strcmp(field[1], XLBL_DEF_REF) == 0 ||
	    strcmp(ident, mybarename(dop->filePath)) == 0)) {
	  signal = strdup(field[1]);
	  isXLBL = TRUE;
	}
	else if(strcmp(field[0], XLBL_TIERS_ID) == 0) {
	  dd->numFields = (size_t)strtol(field[1], NULL, 10);
	  isXLBL = TRUE;
	}
	else if(strcmp(field[0], XLBL_SEP_ID) == 0) {
	  strcpy(dd->sepChars, field[1]);
	  isXLBL = TRUE;
	}
	else if(strcmp(field[0], XLBL_VERS_ID) == 0) {
	  dop->version = strtol(field[1], NULL, 10);
	  /* isXLBL = TRUE; */
	}
	else if(strcmp(field[0], XLBL_COLOR_ID) == 0) {
	  color = (int)strtol(field[1], NULL, 10);
	  isXLBL = TRUE;
	}
	else if(strcmp(field[0], XLBL_FONT_ID) == 0) {
	  font = strdup(field[1]);
	  /* isXLBL = TRUE; */
	}
      }
    }
  }
  if(ferror(dop->fp) || n < 0 || !isXLBL) {
    if(signal != NULL)
      free((void *)signal);
    if(font != NULL)
      free((void *)font);
    asspMsgNum = AEF_ERR_FORM;
    sprintf(applMessage, "(not ESPS-xlabel) in file %s", dop->filePath);
    return(-1);
  }
  dop->fileData = FDF_ASC;
  CLRENDIAN(dop->fileEndian);
  dop->headerSize = ftell(dop->fp);
  dop->frameDur = 0;
  dop->recordSize = 0;                                    /* variable */
  dop->startRecord = 0;                             /* per definition */
  dop->numRecords = 0;                             /* can't determine */
  dop->Start_Time = dop->Time_Zero = 0.0;
  strcpy(dop->sepChars, "");                        /* blanks or tabs */
  strcpy(dop->eol, XLBL_EOL_STR);
  dd->ident = strdup("labels");
  dd->type = DT_LBL;
  dd->format = DF_STR;
  dd->coding = DC_XLBL;
  if(dd->numFields < 1)
    dd->numFields  = 1;
  SETEND(dd->orientation);
  gd = (XLBL_GD *)dop->generic;
  if(gd == NULL) {
    if((gd=(XLBL_GD *)calloc(1, sizeof(XLBL_GD))) != NULL) {
      strcpy(gd->ident, XLBL_GD_IDENT);
      gd->signal = signal;
      gd->font = font;
      if(color < 0)
	gd->color = XLBL_DEF_COLOR;
      else
	gd->color = color;
      dop->generic = (void *)gd;
      dop->doFreeGeneric = (DOfreeFunc)freeXLBL_GD;
    }
    else { /* not so bad; we can do without */
      if(signal != NULL)
	free((void *)signal);
      if(font != NULL)
	free((void *)font);
    }
  }
  else if(strcmp(gd->ident, XLBL_GD_IDENT) == 0) {
    if(signal != NULL) {
      if(gd->signal != NULL)
	free((void*)gd->signal);
      gd->signal = signal;
    }
    if(font != NULL) {
      if(gd->font != NULL)
	free((void *)gd->font);
      gd->font = font;
    }
    if(color >= 0)
      gd->color = color;
  }

  return(0);
}
/***********************************************************************
* Write ESPS Xlabel header.                                            *
***********************************************************************/
LOCAL int putXLBLhdr(DOBJ *dop)
{
  char     header[ONEkBYTE], temp[32], *cPtr=NULL;
  size_t   numBytes;
  DDESC   *dd=&(dop->ddl);
  XLBL_GD *gd=NULL;

  if((dd->type != DT_LBL && dd->type != DT_TAG) ||
     dd->coding != DC_XLBL) {
    setAsspMsg(AEB_BAD_CALL, "putXLBLhdr");
    return(-1);
  }
  dop->fileData = FDF_ASC;
  CLRENDIAN(dop->fileEndian);
  dop->version = XLBL_DEF_VERS;
  dop->frameDur = 0;
  dop->recordSize = 0;                                    /* variable */
  dop->startRecord = 0;                             /* per definition */
  dop->Start_Time = dop->Time_Zero = 0.0;
  strcpy(dop->sepChars, " ");
  strcpy(dop->eol, XLBL_EOL_STR);
  dd->format = DF_STR;
  if(dd->numFields < 1)
    dd->numFields = 1;
  if(dd->numFields > 1) {
    if(strlen(dd->sepChars) == 0)
      strcpy(dd->sepChars, XLBL_DEF_SEP);
  }
  if(MARKSUNDEF(dd->orientation))
    SETEND(dd->orientation);
  gd = (XLBL_GD *)dop->generic;
/*
 * construct (less bad) header in memory
 */
  strcpy(header, XLBL_REF_ID);
  strcat(header, dop->sepChars);
  if(gd != NULL && strcmp(gd->ident, XLBL_GD_IDENT) == 0) {
    if(gd->signal != NULL)
      strcat(header, gd->signal);
    else {
      cPtr = mybarename(dop->filePath);
      strcat(header, cPtr);
      gd->signal = strdup(cPtr);
    }
  }
  else
    strcat(header, mybarename(dop->filePath));
  strcat(header, dop->eol);
  sprintf(temp, "%s %ld", XLBL_VERS_ID, dop->version);
  strcat(header, temp);
  strcat(header, dop->eol);
  sprintf(temp, "%s %ld", XLBL_TIERS_ID, (long)(dd->numFields));
  strcat(header, temp);
  strcat(header, dop->eol);
  if(dd->numFields > 1) {
    strcat(header, XLBL_SEP_ID);
    strcat(header, dd->sepChars);
    strcat(header, dop->eol);
  }
  if(gd != NULL && strcmp(gd->ident, XLBL_GD_IDENT) == 0) {
    if(gd->color >= 0) {
      sprintf(temp, "%s %i", XLBL_COLOR_ID, gd->color);
      strcat(header, temp);
      strcat(header, dop->eol);
    }
    if(gd->font != NULL) {
      strcat(header, XLBL_FONT_ID);
      strcat(header, dop->sepChars);
      strcat(header, gd->font);
      strcat(header, dop->eol);
    }
  }
  strcat(header, XLBL_EOH_STR);
  strcat(header, dop->eol);
  dop->headerSize = (long)(numBytes=strlen(header));
/*
 * write header
 */
#ifndef WRASSP
  if(dop->fp != stdout)
#endif
    rewind(dop->fp);
  if(fwrite(header, 1, numBytes, dop->fp) != numBytes) {
    setAsspMsg(AEF_ERR_WRIT, dop->filePath);
    return(-1);
  }
  return(0);
}

/*
 * The DSP package ESPS (and therefore some files in SSFF format) uses 
 * different (and not entirely consistent) conventions for assigning 
 * time to a sample (the begin time of the sample) and a frame (the 
 * centre time of the analysis window, whereby the first window is 
 * left-aligned with the begin of the analysis interval) than ASSP.
 * This function converts the item 'Start_Time' from the SSFF format 
 * to the ASSP standard 'startRecord' and computes from the difference 
 * between the time of the (rounded) record number and 'Start_Time' the 
 * value of the item 'Time_Zero' as the -possible- difference in the 
 * timing conventions. This function should ONLY be used when reading 
 * the header of an SSFF formatted file. checkRates() should be called 
 * before calling this function to synchronize 'sampFreq', 'frameDur' 
 * and 'dataRate'.
 * The function foreignTime() (see dataobj.c) can then be used to 
 * convert to and from the 'real' time for the data. 
 */
LOCAL int adjustTiming(DOBJ *dop)
{
  long   recNr=0;
  double start, zero=0.0;

  if(dop->fileFormat == FF_SSFF) {
    start = dop->Start_Time;
    if(dop->frameDur > 0) {
      if(dop->ddl.type != DT_SMP)
	start -= (FRMNRtoTIME(1, dop->sampFreq, dop->frameDur)/2.0);
      recNr = (long)myrint((start/(double)(dop->frameDur))\
	                   * (dop->sampFreq));
      if(recNr < 0) recNr = 0;
      if(dop->ddl.type != DT_SMP)
        zero = start - FRMNRtoTIME(recNr, dop->sampFreq, dop->frameDur);
      else {           /* round Start_Time to nearest sample boundary */
        dop->Start_Time = SMPNRtoTIME(recNr, dop->sampFreq);
        zero = 0.0;
      }
    }
    else if(dop->frameDur == 0) {
      recNr = 0; /* per definition */
      zero = 0.0;
    }
    else {                     /* frameDur < 0; only dataRate defined */
      start -= (0.5 / (dop->dataRate));
      recNr = (long)myrint(start * (dop->dataRate));
      if(recNr < 0) recNr = 0;
      zero = start - SMPNRtoTIME(recNr, dop->dataRate);
      if(fabs(zero) < 0.5E-12)                      /* rounding error */
	zero = 0.0;
    }
    dop->startRecord = recNr;
    dop->Time_Zero = zero;
  }
  return(0);
}

/*
 * These functions convert non-char variables to and from a memory
 * location given by "ptr". The location will automatically be updated 
 * to that of the following variable.
 */
/* LOCAL uint8_t getU8(void **ptr) */
/* { */
/*   uint8_t val; */

/*   memcpy(&val, *ptr, 1); */
/*   (*ptr) += 1; */
/*   return(val); */
/* } */
LOCAL void *putU8(uint8_t val, void **ptr)
{
  memcpy(*ptr, &val, 1);
  (*ptr) = (void *)((char *)(*ptr) + 1);
  return(*ptr);
}
/* LOCAL int8_t getI8(void **ptr) */
/* { */
/*   int8_t val; */

/*   memcpy(&val, *ptr, 1); */
/*   (*ptr) += 1; */
/*   return(val); */
/* } */
LOCAL void *putI8(int8_t val, void **ptr)
{
  memcpy(*ptr, &val, 1);
  (*ptr) = (void *)((char *)(*ptr) + 1);
  return(*ptr);
}
LOCAL uint16_t getU16(void **ptr, int SWAP)
{
  uint16_t val;

  if(SWAP) memswab(&val, *ptr, 2, 1);
  else memcpy(&val, *ptr, 2);
  (*ptr) = (void *)((char *)(*ptr) + 2);
  return(val);
}
LOCAL void *putU16(uint16_t val, void **ptr, int SWAP)
{
  if(SWAP) memswab(*ptr, &val, 2, 1);
  else memcpy(*ptr, &val, 2);
  (*ptr) = (void *)((char *)(*ptr) + 2);
  return(*ptr);
}
LOCAL int16_t getI16(void **ptr, int SWAP)
{
  int16_t val;

  if(SWAP) memswab(&val, *ptr, 2, 1);
  else memcpy(&val, *ptr, 2);
  (*ptr) = (void *)((char *)(*ptr) + 2);
  return(val);
}
LOCAL void *putI16(int16_t val, void **ptr, int SWAP)
{
  if(SWAP) memswab(*ptr, &val, 2, 1);
  else memcpy(*ptr, &val, 2);
  (*ptr) = (void *)((char *)(*ptr) + 2);
  return(*ptr);
}
LOCAL uint32_t getU32(void **ptr, int SWAP)
{
  uint32_t val;

  if(SWAP) memswab(&val, *ptr, 4, 1);
  else memcpy(&val, *ptr, 4);
  (*ptr) = (void *)((char *)(*ptr) + 4);
  return(val);
}
LOCAL void *putU32(uint32_t val, void **ptr, int SWAP)
{
  if(SWAP) memswab(*ptr, &val, 4, 1);
  else memcpy(*ptr, &val, 4);
  (*ptr) = (void *)((char *)(*ptr) + 4);
  return(*ptr);
}
LOCAL int32_t getI32(void **ptr, int SWAP)
{
  int32_t val;

  if(SWAP) memswab(&val, *ptr, 4, 1);
  else memcpy(&val, *ptr, 4);
  (*ptr) = (void *)((char *)(*ptr) + 4);
  return(val);
}
LOCAL void *putI32(int32_t val, void **ptr, int SWAP)
{
  if(SWAP) memswab(*ptr, &val, 4, 1);
  else memcpy(*ptr, &val, 4);
  (*ptr) = (void *)((char *)(*ptr) + 4);
  return(*ptr);
}
LOCAL float getF32(void **ptr, int SWAP)
{
  float val;

  if(SWAP) memswab(&val, *ptr, 4, 1);
  else memcpy(&val, *ptr, 4);
  (*ptr) = (void *)((char *)(*ptr) + 4);
  return(val);
}
LOCAL void *putF32(float val, void **ptr, int SWAP)
{
  if(SWAP) memswab(*ptr, &val, 4, 1);
  else memcpy(*ptr, &val, 4);
  (*ptr) = (void *)((char *)(*ptr) + 4);
  return(*ptr);
}
