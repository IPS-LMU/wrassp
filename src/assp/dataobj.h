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
* File:     dataobj.h                                                  *
* Contents: Structures and constants defining an ASSP data object.     *
*           Prototypes of basic data handling functions.               *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: dataobj.h,v 1.11 2012/01/24 15:48:10 lasselasse Exp $ */

#ifndef _DATAOBJ_H
#define _DATAOBJ_H

#include <stdio.h>      /* FILE NULL */
#include <stddef.h>     /* size_t */
#include <inttypes.h>   /* int16_t etc. */

#include <dlldef.h>     /* ASSP_EXTERN */
#include <asspendian.h> /* ENDIAN */


#ifdef __cplusplus
extern "C" {
#endif

/*
 * enumerator for file formats
 * Note: not all file formats listed are actually supported
 */
typedef enum fileFormat {
  FF_ERROR = -1,
  FF_UNDEF     ,
  FF_RAW       ,     /* headerless or unsupported format */
  FF_ASP_A     ,     /* ASP with ASCII data */
  FF_ASP_B     ,     /* ASP with binary data */
  FF_XASSP     ,     /* xassp ASCII */
  FF_IPDS_M    ,     /* labels in IPdS `MIX' format */
  FF_IPDS_S    ,     /* labels in IPdS `SAMPA' format */
  FF_AIFF      ,     /* Apple Audio Interchange File Format */
  FF_AIFC      ,     /*   AIFF extended for compressed data */
  FF_CSL       ,     /* Kay Elemetrics Computerized Speech Lab */
  FF_CSRE      ,     /* Computerized Speech Research Environment */
  FF_ESPS      ,     /* Entropic Signal Processing System */
  FF_ILS       ,     /* */
  FF_KTH       ,     /* Kungliga Tekniska Hoegskolan Stockholm */
  FF_SWELL = FF_KTH, /*   commercial version of KTH */
  FF_SNACK = FF_KTH, /*   as Tcl extension */
  FF_SFS       ,     /* University College London Speech Filing System */
  FF_SND       ,     /* NeXT version of `SND' format */
  FF_AU = FF_SND,    /* Sun version of `SND' format */
  FF_NIST      ,     /* National Institute of Standards and Technology */
  FF_SPHERE = FF_NIST,/*   SPeech HEader REsources */
  FF_PRAAT_S   ,     /* UvA praat 'short' text file */
  FF_PRAAT_L   ,     /* UvA praat 'long' text file */
  FF_PRAAT_B   ,     /* UvA praat binary file */
  FF_SSFF      ,     /* Simple Signal File Format */
  FF_WAVE      ,     /* IBM/Microsoft RIFF-WAVE */
  FF_WAVE_X    ,     /*   RIFF-WAVE extended format (Revision 3) */
  FF_XLABEL    ,     /* ESPS xlabel */
  FF_YORK      ,     /* University of York (Klatt'80 parameters) */
  FF_UWM       ,     /* University of Wisconsin at Madison (microbeam data) */
  NUM_FILE_FORMATS
} fform_e;

typedef enum fileDataFormat { /* basic data format in file */
  FDF_ERROR = -1,
  FDF_UNDEF     ,
  FDF_ASC       ,    /* ASCII */
  FDF_BIN       ,    /* binary */
  NUM_FILE_DATA_FORMATS
} fdata_e;

/*
 * enumerators for data/parameter definitions and properties
 */
typedef enum dataType {
  DT_ERROR = -1,
  DT_UNDEF     ,
  DT_TIME      ,     /* time in seconds */
  DT_RECNR     ,     /* record/sample/frame number */

  DT_SMP       ,     /* sampled data (audio/sound/speech) */
  DT_MAG       ,     /* peak magnitude */
  DT_NRG       ,     /* energy (sum of squares) */
  DT_PWR       ,     /* power (mean energy) */
  DT_RMS       ,     /* RMS amplitude (dB) */
  DT_ZCR       ,     /* zero-crossing rate */
  DT_PIT       ,     /* pitch/F0 */
  DT_AC1       ,     /* normalized 1st autocorrelation coefficient */
  DT_LP1       ,     /* 1st order LP coefficient (= -R[1]/R[0]) */
  DT_PROB      ,     /* probability */

  DT_ACF       ,     /* auto-correlation function */
  DT_CCF       ,     /* cross-correlation function */
  DT_LPC       ,     /* LP filter (A-) coefficients */
  DT_RFC       ,     /* reflection coefficients */
  DT_ARF       ,     /* area function */
  DT_LAR       ,     /* log area ratios */
  DT_LPCEP     ,     /* cepstral coefficients from LP */
  DT_GAIN      ,     /* filter gain (dB) */

  DT_PQP       ,     /* PQ parameters (2nd order filters) */
  DT_FFB       ,     /* formant frequencies and bandwidths */
  DT_FBA       ,     /* formant frequencies, bandwidths and amplitudes */
  DT_FFR       ,     /* formant frequencies */
  DT_FBW       ,     /* formant bandwidths */
  DT_FAM       ,     /* formant amplitudes */

  DT_DFT       ,     /* complex spectrum */
  DT_FTAMP     ,     /* linear amplitude spectrum */
  DT_FTSQR     ,     /* linear power spectrum */
  DT_FTPOW     ,     /* power spectrum in dB */
  DT_FTPHI     ,     /* phase spectrum */
  DT_FTFTS     ,     /* smoothed spectrum */
  DT_FTLPS     ,     /* LP smoothed spectrum */
  DT_FTCSS     ,     /* cepstral smoothed spectrum */
  DT_FTCEP     ,     /* cepstrum */
  DT_MFCC      ,     /* mel frequency cepstral coefficients */

  DT_FILTER    ,     /* digital filter (coefficients in 'generic', */
                     /* processing buffer in 'dataBuffer') */

  DT_TAG       ,     /* general for annotation */
  DT_MRK       ,     /* markers (unique name for point in time) */
  DT_LBL       ,     /* labels (descriptive name for stretch) */
  DT_EPO       ,     /* epochs (leading amplitude of period) */
  DT_PRD       ,     /* unspecified period markers */

  DT_AMP       ,     /* amplitude factor */
  DT_DUR       ,     /* duration factor */

  DT_EGG       ,     /* electro-glotto(laryngo)graph data */
  DT_EPG       ,     /* electro-palatograph data */
  DT_EMA       ,     /* electro-magnetic articulograph data */
  DT_XRM       ,     /* X-ray microbeam data */

  DT_DATA_LOG  ,     /* xassp data logs */
  NUM_DATA_TYPES
} dtype_e;

typedef enum dataFormat {
  DF_ERROR = -1,
  DF_UNDEF     ,
  DF_BIT       ,     /* bit array */
  DF_STR       ,     /* variable length character array */
  DF_CHAR      ,     /* character */
  DF_UINT8     ,     /* unsigned 1 byte integer */
  DF_INT8      ,     /* signed 1 byte integer */
  DF_UINT16    ,     /* unsigned 2 byte integer */
  DF_INT16     ,     /* signed 2 byte integer */
  DF_UINT24    ,     /* unsigned 3 byte integer (packed) */
  DF_INT24     ,     /* signed 3 byte integer (packed) */
  DF_UINT32    ,     /* unsigned 4 byte integer */
  DF_INT32     ,     /* signed 4 byte integer */
  DF_UINT64    ,     /* unsigned 8 byte integer */
  DF_INT64     ,     /* signed 8 byte integer */
  DF_REAL32    ,     /* IEEE 754 single precision floating point */
  DF_REAL64    ,     /* IEEE 754 double precision floating point */
  /*  DF_REAL80    ,    */ /* IEEE 754 extended precision floating point */
  /*  DF_REAL128   ,    */ /* IEEE 754 quadruple precision floating point */
  NUM_DATA_FORMATS
} dform_e;

typedef enum dataCoding {
  DC_ERROR = -1,
  DC_UNDEF     ,
  DC_LIN       ,     /* linear/standard (plain numbers in 2's complement) */
  /* audio */
  DC_PCM = DC_LIN,   /* Pulse Code Modulation (2's complement / IEEE 754) */
  DC_BINOFF    ,     /* integer in binary offset */
  DC_FNORM1    ,     /* IEEE float normalized to [-1:1] (PRELIMINARY) */
  DC_ALAW      ,     /* CCITT G711 A-law compression */
  DC_uLAW      ,     /* CCITT G711 mu-law compression */
  DC_ACE2      ,     /* Apple IIGS 2:1 compression */
  DC_ACE8      ,     /* Apple IIGS 8:3 compression */
  DC_MAC3      ,     /* Macintosh 3:1 compression */
  DC_MAC6      ,     /* Macintosh 6:1 compression */
  DC_DELTA     ,     /* delta modulation */
  DC_ADPCM     ,     /* ? ?-bit ? kb/s ADPCM */
  DC_G721      ,     /* CCITT 4-bit 32 kb/s ADPCM */
  DC_G722      ,     /* CCITT 8-bit 64/48 kb/s ADPCM */
  DC_G723_3    ,     /* CCITT 3-bit 24 kb/s ADPCM */
  DC_G723_5    ,     /* CCITT 5-bit 40 kb/s ADPCM */
  DC_MS_ADPCM  ,     /* Microsoft ?-bit ? kb/s ADPCM */
  DC_CL_ADPCM  ,     /* Creative Labs ?-bit ? kb/s ADPCM */
  DC_IDVI_ADPCM,     /* Intel/DVI ?-bit 32 kb/s ADPCM */
  DC_OKI_ADPCM ,     /* OKI ?-bit ? kb/s ADPCM */
  DC_IBM_ADPCM ,     /* IBM ?-bit ? kb/s ADPCM */
  DC_MPEG3     ,     /* ISO/MPEG Layer 3 */
  /* labels */
  DC_MIX       ,     /* IPdS 'MIX' format */
  DC_SAM       ,     /* IPdS 'SAMPA' format */
  DC_XLBL      ,     /* ESPS 'xlabel' format */
  /* X-ray microbeam */
  DC_TXY       ,     /* time & pellet positions in um */
  DC_XYD       ,     /* pellet displacements in um/frame */
  NUM_DATA_CODINGS
} dcode_e;

/*
 * SSFF generic variables 
 */
#define NAME_SIZE 256
#define DATA_SIZE 1024
typedef enum SSFF_TYPE_ENUM
{
  SSFF_CHAR,
  SSFF_BYTE,
  SSFF_SHORT,
  SSFF_LONG,
  SSFF_FLOAT,
  SSFF_DOUBLE,
  SSFF_UNDEF
} SSFF_TYPE_ENUM_e;

typedef struct SSFF_TYPES_S {
  SSFF_TYPE_ENUM_e type;
  const char       *ident;
  size_t           bytesize;
} SSFFST;

ASSP_EXTERN SSFFST SSFF_TYPES[];

typedef struct SSFF_Generic TSSFF_Generic;
struct SSFF_Generic
{
  TSSFF_Generic *next;
  
  SSFF_TYPE_ENUM_e type;           /* data is held in string form,
                         and converted by the user program */
  /* the data */
  char *ident;
  char *data;
};

/*
 * data/parameter descriptor structure
 */
typedef struct dataDescriptor {
  char    *ident;       /* identification string (ALLOCATED) */
  char     unit[8];     /* unit, e.g. "Hz" */
  char     factor[4];   /* factor/SI prefix, e.g. "k" */
  dtype_e  type;        /* type of data */
  dform_e  format;      /* format of data */
  dcode_e  coding;      /* coding of data */
  ENDIAN   orientation; /* for e.g. EPG, EMA but also labels */
  uint16_t numBits;     /* bits per sample for audio / bits in bitarray */
  uint32_t zeroValue;   /* value representing 0 in unsigned PCM */
  size_t   offset;      /* byte offset to data within record when binary */
  size_t   numFields;   /* number of fields/audio tracks */
  char     ascFormat[8];/* format string for ASCII output (see below) */
  char     sepChars[4]; /* field separator(s) for ASCII data */
  struct dataDescriptor *next;/* linked list for multi-parameter (ALLOCATED) */
} DDESC;

  /* Note on 'ascFormat': Integral data will always be converted to 'long'  */
  /* and printed in "ld" format. This string is only evaluated for floating */
  /* point data (after conversion to 'double'). The string refers to the    */
  /* format of a single value, If it is not specified "%f" will be used.    */

/*
 * typedef of funtion for returning special allocated memory in a data 
 * object such as the data buffer.
 * NOTE - If these functions are not specified in the data object you 
 *        will have to return the memory by hand before destroying the 
 *        data object.
 */
typedef void (*DOfreeFunc)(void *);

/*
 * data object structure
 */
typedef struct dataObject {
  char   *filePath;    /* pointer to string with path to file */
  FILE   *fp;          /* pointer to file descriptor from fopen() */
  int     openMode;    /* 'mode' flags used in asspFOpen() */
  fform_e fileFormat;  /* format of file */
  fdata_e fileData;    /* basic data format in file (details in 'ddl') */
  ENDIAN  fileEndian;  /* byte order of file header and/or data */
  long    version;     /* version number of format/header */
  long    headerSize;  /* size of header in byte (offset to data) */
  double  sampFreq;    /* (reference) sampling frequency in Hz */
  double  dataRate;    /* actual data rate in Hz (valid if frameDur < 0) */
  long    frameDur;    /* frame duration/window shift in samples */
  size_t  recordSize;  /* number of bytes per record (0 = variable) */
  long    startRecord; /* (absolute) number of first record in file */
  long    numRecords;  /* number of records/frames/samples-per-track in file */
       /* the following two items are needed to map timing */
       /* conventions other than ASSP (e.g. ESPS/SSFF) */ 
  double  Time_Zero;   /* time in foreign format corresponding to 0 in ASSP */
  double  Start_Time;  /* reference time of the first record in the file */
  char    sepChars[4]; /* block separator(s) for ASCII data */
  char    eol[4];      /* end-of-line character(s) in header and/or data */
  DDESC   ddl;         /* data/parameter descriptor list */
  TSSFF_Generic meta;   /* store list of generic variables here */
  void   *generic;     /* pointer to generic data (ALLOCATED) */
  DOfreeFunc doFreeGeneric; /* pointer to freeing function */
  void   *dataBuffer;  /* pointer to (part of) the data (ALLOCATED) */
  DOfreeFunc doFreeDataBuf; /* pointer to freeing function */
  long    maxBufRecs;  /* size of buffer in records */
  long    bufStartRec; /* (absolute) number of first record in buffer */
  long    bufNumRecs;  /* number of valid records in buffer */
  int8_t  bufNeedsSave;/* should be maintained by user; the library */
                       /* functions only do the most obvious/harmless */
  /* still needed: int8_t locked, DOBJ *refDObj & LINK *depDObjs */
  void    *userData;    /* let the user store something at his/her own risk*/
} DOBJ;

/*
 * generic data should contain an identification string as first item
 * this is the suggested maximum length of that string
 */
#define GD_MAX_ID_LEN 31
 
/*
 * prototypes of functions in dataobj.c
 */
ASSP_EXTERN DOBJ  *allocDObj(void);
ASSP_EXTERN DOBJ  *freeDObj(DOBJ *dop);
ASSP_EXTERN void   clearDObj(DOBJ *dop);
ASSP_EXTERN void   freeDDList(DOBJ *dop);
ASSP_EXTERN void   freeMeta(DOBJ *dop);
ASSP_EXTERN void   initDObj(DOBJ *dop);
ASSP_EXTERN int    copyDObj(DOBJ *dst, DOBJ *src);
ASSP_EXTERN DDESC *addDDesc(DOBJ *dop);
ASSP_EXTERN DDESC *freeDDesc(DDESC *ddp);
ASSP_EXTERN DDESC *clearDDesc(DDESC *ddp);
ASSP_EXTERN void   initDDesc(DDESC *ddp);
ASSP_EXTERN int    copyDDesc(DDESC *dst, DDESC *src);
ASSP_EXTERN DDESC *findDDesc(DOBJ *dop, dtype_e type, char *ident);
ASSP_EXTERN TSSFF_Generic *addTSSFF_Generic(DOBJ *dop);
ASSP_EXTERN TSSFF_Generic *freeTSSFF_Generic(TSSFF_Generic *genVar);
ASSP_EXTERN TSSFF_Generic *clearTSSFF_Generic(TSSFF_Generic *genVar);
ASSP_EXTERN void   initTSSFF_Generic(TSSFF_Generic *genVar);
ASSP_EXTERN int    copyTSSFF_Generic(TSSFF_Generic *dst, TSSFF_Generic *src);
ASSP_EXTERN TSSFF_Generic *findTSSFF_Generic(DOBJ *dop, char *ident);
ASSP_EXTERN int    setRecordSize(DOBJ *dop);
ASSP_EXTERN int    checkRates(DOBJ *dop);
ASSP_EXTERN int    setStart_Time(DOBJ *dop);
ASSP_EXTERN double foreignTime(double t, DOBJ *dop, int TO_ASSP);
ASSP_EXTERN void   freeGeneric(DOBJ *dop);
ASSP_EXTERN void  *allocDataBuf(DOBJ *dop, long numRecords);
ASSP_EXTERN void   clearDataBuf(DOBJ *dop);
ASSP_EXTERN void   freeDataBuf(DOBJ *dop);
ASSP_EXTERN int    swapDataBuf(DOBJ *dop);
ASSP_EXTERN int    swapRecord(DOBJ *dop, void *record);
ASSP_EXTERN int    blockSwap(DOBJ *dop, size_t *numUnits);
ASSP_EXTERN long   mapRecord(DOBJ *dst, DOBJ *src, long recordNr);
ASSP_EXTERN long   getSmpCaps(dform_e format);
ASSP_EXTERN int    getSmpFrame(DOBJ *smpDOp, long nr, long size, long shift,\
			       long head, long tail, int channel, void *frame,\
			       dform_e format);
ASSP_EXTERN void  *getSmpPtr(DOBJ *smpDOp, long smpNr, long head, long tail,\
			     int channel, DOBJ *workDOp);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _DATAOBJ_H */
