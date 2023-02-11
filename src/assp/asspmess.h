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
* File:     asspmess.h                                                 *
* Contents: Public header file for programs and functions using the    *
*           ASSP message handler                                       *
* Author:   Michel T.M. Scheffers                                      *
* Remarks:  If this file is changed, adjust the corresponding warning/ *
*           error messages in asspmess.c.                              *
*                                                                      *
***********************************************************************/
/* $Id: asspmess.h,v 1.11 2010/01/19 09:34:37 mtms Exp $ */

#ifndef _ASSPMESS_H
#define _ASSPMESS_H

#include <stdio.h>    /* FILE */
#include <dlldef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * warning codes
 */
#define ASSP_WARNING  (short)0xa000
/* general warningss */
#define AWG_WARN_BUG  (short)0xa000  /* programming error */
#define AWG_WARN_SYS  (short)0xa001  /* report system message */
#define AWG_WARN_APPL (short)0xa002  /* report only applMessage */
#define AWG_WARN_MEM  (short)0xa003  /* out of memory */
#define AWG_ERR_ROUND (short)0xa004  /* rounding error */
/* audio warnings */
#define AWA_WARN_DEV  (short)0xa0a0  /* no audio device available */
#define AWA_WARN_SFR  (short)0xa0a1  /* audio device can't handle sample rate */
/* data warnings */
#define AWD_NO_DATA   (short)0xa0d0  /* no data available */
#define AWD_NO_AUDIO  (short)0xa0da  /* no audio signal */
/* file warnings */
#define AWF_BAD_ITEM  (short)0xa0f0  /* bad header item */
#define AWF_RAW_FORM  (short)0xa0f1  /* file format not recognized */
#define AWF_EMPTY     (short)0xa0f3  /* empty file */

/*
 * error codes
 */
#define ASSP_ERROR    (short)0xae00
/* general errors */
#define AEG_ERR_BUG   (short)0xae00  /* programming error */
#define AEG_ERR_SYS   (short)0xae01  /* report system error */
#define AEG_ERR_APPL  (short)0xae02  /* report only applMessage */
#define AEG_ERR_MEM   (short)0xae03  /* out of memory */
#define AEG_ERR_ROUND (short)0xae04  /* rounding error */
#define AEG_NOT_YET   (short)0xae0e  /* not yet implemented */
#define AEG_ERR_FATAL (short)0xae0f  /* FATAL ERROR */
/* audio errors */
#define AEA_ERR_DEV   (short)0xaea0  /* no such audio device */
#define AEA_ERR_OPEN  (short)0xaea1  /* can't open audio device */
#define AEA_ERR_BUSY  (short)0xaea2  /* audio device busy */
#define AEA_ERR_INIT  (short)0xaea3  /* can't initialize audio device */
#define AEA_ERR_BUF   (short)0xaea4  /* can't get audio buffer */
#define AEA_ERR_FORM  (short)0xaea5  /* audio device can't handle data format */
#define AEA_ERR_RATE  (short)0xaea6  /* audio device can't handle sample rate */
#define AEA_ERR_CHAN  (short)0xaea7  /* too many channels for audio device */
#define AEA_ERR_PUT   (short)0xaea8  /* error in writing to audio device */
#define AEA_ERR_GET   (short)0xaea9  /* error in reading from audio device */
/* typical bugs */
#define ASSP_BUG      (short)0xaeb0
#define AEB_BAD_ARGS  (short)0xaeb0  /* invalid arguments in function call */
#define AEB_BAD_CALL  (short)0xaeb1  /* invalid function call */
#define AEB_TOO_SOON  (short)0xaeb2  /* request to access data before BOF */
#define AEB_TOO_LATE  (short)0xaeb3  /* request to access data behind EOF */
#define AEB_BUF_RANGE (short)0xaeb4  /* request to access data not in buffer */
#define AEB_BUF_SPACE (short)0xaeb5  /* insufficient space in buffer */
#define AEB_BAD_WIN   (short)0xaeb6  /* unknown/invalid window function */
#define AEB_ERR_EMPH  (short)0xaeb7  /* invalid preemphasis */
#define AEB_ERR_GAIN  (short)0xaeb8  /* invalid gain factor */
#define AEB_ERR_TRACK (short)0xaeb9  /* no track name available */
/* command line errors */
#define AEC_BAD_OPT   (short)0xaec0  /* unknown option */
#define AEC_BAD_VAL   (short)0xaec1  /* bad option value */
#define AEC_ARG_MISS  (short)0xaec2  /* argument missing */
#define AEC_ARG_MANY  (short)0xaec3  /* too many arguments */
#define AEC_IO_CLASH  (short)0xaec4  /* output would overwrite input file */
/* data errors */
#define AED_NO_DATA   (short)0xaed0  /* no data available */
#define AED_BAD_TYPE  (short)0xaed1  /* unknown data type */
#define AED_BAD_FORM  (short)0xaed2  /* unknown data format */
#define AED_NOHANDLE  (short)0xaed3  /* can't handle data format */
#define AED_ERR_TYPE  (short)0xaed4  /* incorrect data type */
#define AED_ERR_FORM  (short)0xaed5  /* incorrect data format */
#define AED_ERR_RATE  (short)0xaed6  /* incorrect data rate */
#define AED_ERR_RANGE (short)0xaed7  /* empty/invalid data range */
#define AED_ERR_SIZE  (short)0xaed8  /* window size undefined/too small */
#define AED_INCOMPAT  (short)0xaed9  /* incompatible with existing data */
#define AED_NO_AUDIO  (short)0xaeda  /* no audio signal */
/* exceptions */
#define AEE_BAD_ERR   (short)0xaeee  /* unknown error code */
/* file errors */
#define AEF_NOT_OPEN  (short)0xaef0  /* file not open */
#define AEF_MISSING   (short)0xaef1  /* file does not exist */
#define AEF_EXISTS    (short)0xaef2  /* file already exists */
#define AEF_EMPTY     (short)0xaef3  /* empty file */
#define AEF_ERR_OPEN  (short)0xaef4  /* can't open file */
#define AEF_ERR_SEEK  (short)0xaef5  /* can't seek in file */
#define AEF_ERR_READ  (short)0xaef6  /* can't read file */
#define AEF_ERR_WRIT  (short)0xaef7  /* can't write file */
#define AEF_ERR_MOVE  (short)0xaef8  /* can't move file */
#define AEF_ERR_COPY  (short)0xaef9  /* can't copy file */
#define AEF_ERR_EOF   (short)0xaefa  /* trying to seek past end of file */
#define AEF_BAD_FORM  (short)0xaefb  /* unknown file format */
#define AEF_BAD_HEAD  (short)0xaefc  /* corrupted file header */
#define AEF_ERR_FORM  (short)0xaefd  /* incorrect file format */

/*
 * structures
 */
typedef struct assp_message_record {
  short  num;
  char  *str;
} AMREC;

/*
 * extern declaration of global variables in asspmess.c
 */
#define MAX_MSG_LEN (4095)
ASSP_EXTERN AMREC asspMessage[];
ASSP_EXTERN short asspMsgNum;
ASSP_EXTERN char  applMessage[MAX_MSG_LEN + 1];

/*
 * prototypes of functions in asspmess.c
 */
ASSP_EXTERN void  clrAsspMsg(void);
ASSP_EXTERN char *getAsspMsg(short num);
ASSP_EXTERN int   prtAsspMsg(FILE *fp);
ASSP_EXTERN int   setAsspMsg(short num, char *txt);

/*
 * macros
 */
#define asspWarning ((asspMsgNum & (short)0xff00) == ASSP_WARNING)
#define asspError ((asspMsgNum & (short)0xff00) == ASSP_ERROR)
#define asspBug ((asspMsgNum & (short)0xfff0) == ASSP_BUG)
#define asspFeof (asspMsgNum == ASSP_ERR_EOF)

// helper macro to make usage of snprintf easier; macro copied from https://stackoverflow.com/a/3553321
#define member_size(type, member) sizeof(((type *)0)->member)

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _ASSPMESS_H */
