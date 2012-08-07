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
* File:     headers.h                                                  *
* Contents: Header file for functions, handling formatted speech/audio *
*           related data files.                                        *
* Author:   Michel T.M. Scheffers                                      *
* Note:     A good overview of the most common audio formats with      *
*           references can be found at:                                *
*           <http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats>     *
*                                                                      *
***********************************************************************/
/* $Id: headers.h,v 1.11 2010/03/25 09:30:45 mtms Exp $ */

#ifndef _HEADERS_H
#define _HEADERS_H

#include <stdio.h>      /* FILE */
#include <stddef.h>     /* size_t */
#include <inttypes.h>   /* uint16_t etc. */

#include <dlldef.h>     /* ASSP_EXTERN */
#include <ieee.h>       /* XFPSIZE */
#include <asspendian.h> /* ENDIAN */
#include <dataobj.h>    /* DOBJ fformat_e dtype_e */

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_HDR_FIELDS 32 /* ample (including UWXRMB) */

/*
 * Structure for keyword tables.
 * Note: Different keywords may be associated with the same data type 
 * but not the other way around. When different keywords exist for the 
 * same number, the first one in the list will be the default for ASSP.
 */
typedef struct keyword_data_type_table {
  char   *keyword;
  char   *factor;
  char   *unit;
  dtype_e dataType;
} KDTAB;

/*---------------------------------------------------------------------
|  Binary headers                                                      |
|                                                                      |
| WARNING !!!                                                          |
| 32-bit and especially 64-bit compilers do not pack structures like   |
| the old 16-bit ones did (all multi-byte types one the nearest even   |
| address) but rather align elements on adresses which are an integral |
| multiple of the size of the element or of the system's word size.    |
| This means that the structures below can usually NOT be used to map  |
| elements on a data block, but rather serve to show which bytes       |
| correspond to which element and (where appropriate) the chunked      |
| structure of the header.                                             |
 ---------------------------------------------------------------------*/

/*
 * Electronic Arts Interchange Format Files (EA IFF '85)
 * Chunk structure used in AIFF[-C] and RIFF-WAVE format.
 * NOTE:
 *  o  IFF specifies MSB-first.
 *  o  A chunk must ALWAYS be aligned at an even address. 'ckSize' gives 
 *     the valid size in bytes, if odd, a NULL byte therefore has to be 
 *     appended to the data.
 */
#define SIZEOF_CHUNK 8 /* can't rely on sizeof() any more */
typedef struct basicChunk {
  char    ckID[4];     /* identification string */
  int32_t ckSize;      /* length of ckData in byte */
/* uint8_t ckData[];    variable length data array */
} CHUNK;

/*
 * Audio Interchange File Format (Apple AIFF/AIFF-C)
 *
 * Both header and data have MSB first (big endian).
 *
 */
#define AIFF_MAGIC "AIFF"
#define AIFC_MAGIC "AIFC"
#define AIF_FORM_ID "FORM"
#define AIF_COMM_ID "COMM"
#define AIF_DATA_ID "SSND"
#define AIFF_MIN_HDR  54   /* minimum header size */
#define AIFC_MIN_HDR  72
#define AIFC_VERSION_ID "FVER"
#define AIFC_VERSION 0xA2805140  /* version number/timestamp */
/*
 * AIFF-C compression types
 */
#define AIFC_NONE "NONE" /* "not compressed" */
#define AIFC_ACE2 "ACE2" /* "ACE 2-to-1" */
#define AIFC_ACE8 "ACE8" /* "ACE 8-to-3" */
#define AIFC_MAC3 "MAC3" /* "MACE 3-to-1" */
#define AIFC_MAC6 "MAC6" /* "MACE 6-to-1" */
/*
 * according to 'sndlib', 'audiofile', etc. now also the following
 * (Apple prefers lower case IDs; SGI and other upper case)
 */
#define AIFC_ALAW "alaw" /* "ALaw 2-to-1" */
#define AIFC_uLAW "ulaw" /* "uLaw 2-to-1" */
#define AIFC_FL32 "fl32" /* "32-bit floating point" (compression ;-) */
#define AIFC_FL64 "fl64" /* "64-bit floating point" */

#define SIZEOF_FORM 12   /* this FORM structure is also used by others */
typedef struct formChunk {    /* the outer chunk */
  char    formID[4];     /* == "FORM" */
  int32_t ckSize;        /* file length in byte - 8 */
  char    formType[4];   /* == "AIFF"/"AIFC" */
/* char chunks[];          list of sub chunks */
} FORM;

#define SIZEOF_FVER 12
typedef struct versionChunk { /* format version chunk (AIFC only) */
  char    fverID[4];     /* == "FVER" */
  int32_t ckSize;        /* == 4 */
  int32_t timeStamp;     /* == AIFC_VERSION */
} FVER;
/*
 * The COMMON structures are incompatible with 32-bit compilers.
 */
#define AIFF_COMMSIZE 26 /* total number of bytes in AIFF COMMON chunk */
/* typedef struct commonChunk {   the data format description (AIFF) */
/*   char     commID[4];     == "COMM" */
/*   int32_t  ckSize;        == 18 */
/*   int16_t  numTracks;     number of sound tracks */
/*   uint32_t numSamples;    number of samples per track */
/*   int16_t  numBits;       number of bits per sample (1 ... 32) */
/*   uint8_t  sampRate[XFPSIZE];  sampling rate in extended float */
/* } COMM; */
#define AIFC_COMMSIZE 32 /* minimum number of bytes in AIFC COMMON chunk */
/* typedef struct commonCChunk {  the data format description (AIFC) */
/*   char     commID[4];     == "COMM" */
/*   int32_t  ckSize;        == 22 + len cmprName (at least 2) */
/*   int16_t  numTracks;     number of sound tracks */
/*   uint32_t numSamples;    number of samples per track */
/*   int16_t  numBits;       number of bits per sample (1 ... 64) */
/*   uint8_t  sampRate[XFPSIZE];  sampling rate in extended float */
/*   char     cmprType[4];   type of compression */
/*   uint8_t  cmprLength;    length of cmprName[] */
/*   char     cmprName[1];   compression name (always odd length) */
/* } COMMC; */

#define SIZEOF_SSND 16
typedef struct soundDataChunk { /* the data chunk */
  char     ssndID[4];    /* == "SSND" */
  int32_t  ckSize;       /* 8 + numSoundDateBytes + offset */
  uint32_t offset;       /* offset in soundData[] (normally 0) */
  uint32_t blockSize;    /* block align in soundData[] (normally 0) */
/* uint8_t soundData[];    actual samples */
} SSND;

/*
 * Resource Interchange File Format (IBM/Microsoft RIFF)
 * Restricted to 'WAV' format.
 *
 * Both header and data have MSB last (little endian).
 *
 */
#define RIFF_MAGIC "RIFF"
/* #define RIFX_MAGIC "RIFX" as RIFF but with MSB first (never seen one) */
#define WAVE_MAGIC "WAVE"
#define WAVE_FORM_ID "fmt "
#define WAVE_FACT_ID "fact"
#define WAVE_DATA_ID "data"
#define WAVE_JUNK_ID "junk" /* may contain obsolete data */
#define WAVE_PAD_ID  "pad " /* padding e.g. to increase header size to 512 */
#define WAVE_MIN_HDR 44   /* minimum header size */
#define WAVE_PCM   0x0001 /* some of the zillion format codes */
#define WAVE_ADPCM 0x0002
#define WAVE_FLOAT 0x0003 /* IEEE 754 single or double precision */
#define WAVE_ALAW  0x0006 /* ~ G711 */
#define WAVE_MULAW 0x0007 /* ~ G711 */
#define OKI_ADPCM  0x0010
#define IDVI_ADPCM 0x0011
#define MS_ISO_MP3 0x0055
#define IBM_MULAW  0x0101
#define IBM_ALAW   0x0102
#define IBM_ADPCM  0x0103
#define CL_ADPCM   0x0200
#define WAVE_EXTS  0xFFFE /* extensible format; may require 'fact' chunk */
/*
 * The following structure might directly be used to write the header
 * provided that the data are 8 or 16-bit linear PCM in mono or stereo.
 */
typedef struct waveHeader {
  char     formID[4];    /* == "RIFF" */
  uint32_t ckSize;       /* file length in byte - 8 */
  char     formType[4];  /* == "WAVE" */
  char     formatID[4];  /* data format: == "fmt " */
  uint32_t fckSize;      /* == 16 */
  uint16_t wFormat;      /* see codes above */
  uint16_t numTracks;    /* number of sound tracks */
  uint32_t sampRate;     /* sampling rate in Hz */
  uint32_t byteRate;     /* = nr.tracks * nr.bytes/sample * sampRate */
  uint16_t blockSize;    /* = nr.tracks * nr.bytes/sample */
  uint16_t numBits;      /* number of bits per sample (8 ... 16) */
  char     dataID[4];    /* == "data" */
  uint32_t dckSize;      /* number of bytes in data chunk */
} WAVHDR;
/*
 * The following structures describe the inner chunks of interest.
 */
/* the old style format chunk */
#define SIZEOF_WAVFMT 24 /* full size */
typedef struct waveFormatChunk { /* the data format description */
  char     formatID[4];  /* data format: == "fmt " */
  uint32_t fckSize;      /* 16 */
  uint16_t wFormat;      /* see format codes above */
  uint16_t numTracks;    /* number of sound tracks */
  uint32_t sampRate;     /* sampling rate in Hz */
  uint32_t byteRate;     /* = no.tracks * no.bytes/sample * sampRate */
  uint16_t blockSize;    /* = no.tracks * no.bytes/sample */
  uint16_t numBits;      /* number of bits per sample (8 ... 16) */
} WAVFMT;

/* the extensible version should officially be used for all non-PCM */
/* formats and PCM with > 2 bytes/sample or > 2 tracks */
#define SIZEOF_WAVFMTX 48   /* full size */
#define SIZEOF_WAVFMTX_M 26 /* minimum size */
#define WAVE_FMTX_MAX 22    /* maximum size of extended part */
#define WAVE_GUID_ID "\x00\x00\x00\x00\x10\x00\x80\x00\x00\xAA\x00\x38\x9B\x71"
#define WAVE_GUID_LEN 14
typedef struct waveFormatExtChunk {
  char     formatID[4];  /* data format: == "fmt " */
  uint32_t fckSize;      /* 18 or 40 */
  uint16_t wFormat;      /* WAVE_PCM or WAVE_EXTS */
  uint16_t numTracks;    /* number of sound tracks */
  uint32_t sampRate;     /* sampling rate in Hz */
  uint32_t byteRate;     /* = no.tracks * no.bytes/sample * sampRate */
  uint16_t blockSize;    /* = no.tracks * no.bytes/sample */
  uint16_t numBits;      /* container size (multiple of 8 bits) */
  uint16_t extSize;      /* 0 (for WAVE_PCM ?) or 22 */
  uint16_t validBits;    /* valid bits per sample */
  uint32_t lspMapMask;   /* loudspeaker mapping info (can use 0) */
  uint16_t subFormat;    /* initial part of GUID; as wFormat old style */
  uint8_t  guidID[14];   /* media identification code (see above) */
} WAVFMTX;

/* the 'fact' chunk should be present for compressed data */
/* it is unclear whether it should also be used for float data */
#define SIZEOF_WAVFACT 12 /* minimum size */
typedef struct waveFactChunk {
  char     factID[4];    /* == "fact" */
  uint32_t fckSize;      /* at least 4 */
  uint32_t numRecords;   /* number of samples per track */
/* uint8_t more[];    ???? */
} WAVFACT;

typedef struct waveDataChunk { /* the data chunk */
  char      dataID[4];   /* == "data" */
  uint32_t  dckSize;     /* number of bytes in data chunk */
/* uint8_t soundData[];    actual samples */
} WAVDAT;

/*
 * 'snd' (NeXt) and 'au' (Sun) format
 *
 * Both header and data have MSB first (big endian).
 * It is unclear how exactly the sampling rate is coded.
 * At least 3 rates are defined:
 *       CODEC:  8012.821 (8000, 8012, 8013 in header ?-)
 *       LOW  : 22050.0
 *       HIGH : 44100.0
 *
 */
#define SND_MAGIC ".snd"
#define SND_STD_HDR 28  /* standard header size */
#define SND_MIN_HDR 24  /* minimum header size */
#define SND_UNSPECIFIED            0
#define SND_MULAW_8                1
#define SND_LINEAR_8               2
#define SND_LINEAR_16              3
#define SND_LINEAR_24              4
#define SND_LINEAR_32              5
#define SND_FLOAT                  6
#define SND_DOUBLE                 7
#define SND_INDIRECT               8
#define SND_NESTED                 9
#define SND_DSP_CORE              10
#define SND_DSP_DATA_8            11
#define SND_DSP_DATA_16           12
#define SND_DSP_DATA_24           13
#define SND_DSP_DATA_32           14
#define SND_DSP_UNKNOWN           15
#define SND_DISPLAY               16
#define SND_MULAW_SQUELCH         17
#define SND_EMPHASIZED            18
#define SND_COMPRESSED            19
#define SND_COMPRESSED_EMPHASIZED 20
#define SND_DSP_COMMANDS          21
#define SND_DSP_COMMANDS_SAMPLES  22
#define SND_G721                  23
#define SND_G722                  24
#define SND_G723_3                25
#define SND_G723_5                26
#define SND_ALAW_8                27
/*
 * The following structure might directly be used to write the header.
 */
typedef struct sndHeader {
  char    formID[4];     /* == ".snd" */
  int32_t dataOffset;    /* offset to data (typically sizeof(SNDHDR)) */
  int32_t dataLength;    /* number of data bytes (may be 0/unspecified) */
  int32_t dataFormat;    /* see code numbers above */
  int32_t sampRate;      /* sampling frequency in Hz */
  int32_t numTracks;     /* number of tracks */
  /* the next item is specified by NeXt but is sometimes missing */
  char    info[4];       /* NULL-byte terminated string (may be longer) */
} SNDHDR;

/*
 * Computerized Speech Lab (Kay Elemetrics CSL)
 *
 * Both header and data have MSB last (little endian).
 *
 */
#define CSL_MAGIC "FORMDS16"
#define CSL_HEAD_ID "HEDR"
#define CSL_DATA_ID_L "SDA_" /* mono, left channel */
#define CSL_DATA_ID_R "SD_B" /* mono, right channel */
#define CSL_DATA_ID_S "SDAB" /* stereo */
#define CSL_MIN_HDR  60  /* minimum header size */
#define CSL_DATESIZE 20  /* length of creation date string */
/*
 * The following structure might directly be used to write the header.
 */
typedef struct cslHeader {
  char     formID[4];    /* == "FORM" */
  char     blockID[4];   /* == "DS16" */
  uint32_t blockSize;    /* size of main block in byte */
  char     headID[4];    /* == "HEDR" */
  uint32_t headSize;     /* == 32 */
  char     date[CSL_DATESIZE]; /* creation date */
  uint32_t sampRate;     /* sampling frequency in Hz */
  uint32_t numSamples;   /* samples per track */
  int16_t  peakA;        /* peak magnitude track A (-1 if absent) */
  int16_t  peakB;        /* peak magnitude track B (-1 if absent) */
  char     trackID[4];   /* == "SDA_" / "SD_B" / "SDAB" */
  uint32_t trackSize;    /* number of bytes in track */
} CSLHDR;
/*
 * The following chunk structures provide the surest way to read the 
 * header. There may be other chunks (such as "NOTE") for which the 
 * basic chunk structure can be used.
 */
typedef struct cslForm {
  char     formID[4];    /* == "FORM" */
  char     blockID[4];   /* == "DS16" */
  uint32_t blockSize;    /* total size of main block in byte */
} CSLFRM;
#define SIZEOF_CSLFMT 40
typedef struct cslFormatChunk {
  char     headID[4];    /* == "HEDR" */
  uint32_t headSize;     /* == 32 */
  char     date[CSL_DATESIZE]; /* creation date */
  uint32_t sampRate;     /* sampling frequency in Hz */
  uint32_t numSamples;   /* samples per track */
  int16_t  peakA;        /* peak magnitude track A (-1 if absent) */
  int16_t  peakB;        /* peak magnitude track B (-1 if absent) */
} CSLFMT;
#define SIZEOF_CSLDAT 8
typedef struct cslDataChunk {
  char     trackID[4];   /* == "SDA_" / "SD_B" / "SDAB" */
  uint32_t trackSize;    /* number of bytes in track */
} CSLDAT;

/*
 * Computerized Speech Research Environment (CSRE)
 *
 * Both header and data have MSB last (little endian).
 * Only the header for Audio Data Files (ADF) is described.
 *
 */
#define ADF_MAGIC "CSRE40" /* followed by two NULL-bytes */
#define ADF_HDR_SIZE 512   /* header size in byte (fixed) */
/* The following two values are defined in the manual but it looks like 
   CSRE itself does't use them !!! */
#define ADF_SLIN 0 /* 2's complement */
#define ADF_ULIN 1 /* binary offset */
/*
 * The following structure CANNOT be used to read or write the header.
 */
/* typedef struct adfHeader { */
/*   char     headID[8];     == "CSRE40" */
/*   int32_t  numSamples;    samples in file */
/*   int32_t  centreLine;    offset for binary offset coding ??? */
/*   uint16_t startTrack;    tracks not multiplexed ??? */
/*   uint16_t numBits;       bits per sample (12 or 16) */
/*   uint16_t dataFormat;    see codes above */
/*   float    sampRate;      sampling frequency in >>> kHz <<< */
/*   int32_t  peakSample;    peak sample in file (signed ???)*/
/*   char     fill[482];     extend to 512 byte */
/* } ADFHDR; */

/*---------------------------------------------------------------------
|  ASCII headers                                                       |
 ---------------------------------------------------------------------*/

/*
 * Kungliga Tekniska Hoegskolan Stockholm (KTH), also used in 'snack'.
 *
 * Only audio files are supported.
 */
#define KTH_EOL_STR "\x0D\x0A"  /* <CR><LF> */
#define KTH_SEP_STR "="         /* separator in header lines */
#define KTH_SEP_CHAR '='
#define KTH_MAGIC "head="       /* IPdS definition for identification */
                /* because - if defined - this must be the first item */
#define KTH_MAGIC_ERR "header=" /* some files still have this erroneous line */
#define SNACK_MAGIC "file=samp" /* somewhere in the header */
#define KTH_EOH_STR "\x04\x1A"  /* end of header/comment (^D^Z) */
#define KTH_DEF_HDR 1024        /* default header size in byte */
#define KTH_DEF_SFR 16000.0     /* default sampling frequency */

/* 
 * National Institute of Standards and Technology (NIST)
 * SPeech HEader REsources (SPHERE)
 */
#define NIST_EOL_CHAR 0x0A      /* <LF> */
#define NIST_EOL_STR "\x0A"
#define NIST_MAGIC "NIST_1A\x0A   1024\x0A"
#define NIST_EOH_STR "end_head"
#define NIST_HDR_SIZE 1024

/*
 * Simple Signal File Format (SSFF)
 *
 * BEWARE !!! Some audio files have a Start_Time value unequal zero !!!
 */
#define SSFF_EOL_CHAR 0x0A      /* <LF> */
#define SSFF_EOL_STR "\x0A"
#define SSFF_MAGIC "SSFF -- (c) SHLRC"
#define SSFF_SYS_ID "Machine"
#define SSFF_MSB_FIRST "SPARC"
#define SSFF_MSB_LAST "IBM-PC"
#define SSFF_MSB_LAST2 "VAX"
#define SSFF_RATE_ID "Record_Freq"
#define SSFF_TIME_ID "Start_Time"
#define SSFF_COMM_ID "Comment"
#define SSFF_DATA_ID "Column"
#define SSFF_EOH_STR "-----------------"
/* Reference sample rate now stored as 'generic' header item */
#define SSFF_REF_RATE "Original_Freq"
#define ESPS_REF_RATE "src_sf" /* e.g. in down-sampled file */

ASSP_EXTERN KDTAB KDT_SSFF[];

/*
 * XASSP
 *
 * Header: "XASSP dataType dataRate"          KLUDGE !!!
 *     or: "XASSP dataType sampFreq frameDur"
 * Data are in ASCII.
 */
#define XASSP_MAGIC "XASSP"
#define XASSP_SEP_STR "\t"

ASSP_EXTERN KDTAB KDT_XASSP[];

/*
 * prototypes of public functions in headers.c
 */
ASSP_EXTERN void    setRawSMP(DOBJ *dop);
ASSP_EXTERN fform_e guessFormat(FILE *fp, char *filePath, fdata_e *dataFormat);
ASSP_EXTERN int     getHeader(DOBJ *dop);
ASSP_EXTERN int     putHeader(DOBJ *dop);
ASSP_EXTERN int     needsWAVE_X(DOBJ *dop);
ASSP_EXTERN char   *genWAVhdr(DOBJ *dop);
ASSP_EXTERN char   *freeWAVhdr(char *header);
ASSP_EXTERN KDTAB  *keyword2entry(char *keyword, KDTAB *table);
ASSP_EXTERN KDTAB  *dtype2entry(dtype_e type, KDTAB *table);


#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _HEADERS_H */
