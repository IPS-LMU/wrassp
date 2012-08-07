/***********************************************************************
*                                                                      *
* This file is part of Michel Scheffers's miscellanous functions       *
* library.                                                             *
*                                                                      *
* Copyright (C) 1989 - 2010  Michel Scheffers                          *
*                            IPdS, CAU Kiel                            *
*                            Leibnizstr. 10                            *
*                            24118 Kiel                                *
*                            Germany                                   *
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
* File:     misc.h                                                     *
* Contents: Header file for programs and functions using the library   *
*           libmisc.                                                   *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: misc.h,v 1.11 2010/01/27 10:57:29 mtms Exp $ */

#ifndef _MISC_H
#define _MISC_H

#if (defined(WIN32) || defined(_WIN32)) && defined(DLL_EXPORT) 
#if defined(BUILDING_MISC)
#define MISC_EXTERN  __declspec(dllexport)
#else
#define MISC_EXTERN  extern __declspec(dllimport)
#endif
#else
#define MISC_EXTERN extern
#endif

#include <stdio.h>      /* FILE */
#include <stddef.h>     /* size_t */
#include <inttypes.h>   /* uint32_t */

#include <miscdefs.h>
#include <mylimits.h>
#include <trace.h>

#ifdef __cplusplus
extern "C" {
#endif

MISC_EXTERN int fgetl(char *buffer, int size, FILE *fp, char **eolPtr);
MISC_EXTERN int numDecim(double x, int maxDecim);
MISC_EXTERN double myrint(double x);
MISC_EXTERN double myrand(uint32_t *seedPtr);

/*
 * Constants and prototypes of functions in parsepath.c
 */
MISC_EXTERN char *myfilename(char *fullPath);
MISC_EXTERN char *mybasename(char *fullPath);
MISC_EXTERN char *mybarename(char *fullPath);
MISC_EXTERN int   parsepath(char *fullPath, char **dirPath,\
			    char **baseName, char **extension);

/*
 * Constants and prototypes of interactive functions in query.c
 */
#define QFLAG_NONE     0x00000000
#define QFLAG_EVAL     0x00000001  /* function should evaluate */

#define QFLAG_FOLD     0x00000100  /* file should exist */
#define QFLAG_FNEW     0x00000200  /* file should not exist */
#define QFLAG_FREAD    0x00000400  /* file should exist and be readable */
#define QFLAG_FWRITE   0x00000800  /* file should not exist or be writeable */

#define QFLAG_VMIN     0x00010000  /* verify >= minimum */
#define QFLAG_VMAX     0x00020000  /* verify <= maximum */

MISC_EXTERN int queryChar(char *quest, char *allow, long flags, char *reply);
MISC_EXTERN int queryFile(char *quest, char *deflt, long flags,\
			  char *reply, int length);
MISC_EXTERN int queryLong(char *quest, long *deflt, long flags,\
			  long *reply, long min, long max);
MISC_EXTERN int queryDouble(char *quest, double *deflt, long flags,\
			    double *reply, double min, double max);

/*
 * Prototypes of functions in miscstring.c
 */
MISC_EXTERN char *strccpy(char *dst, char *src, char u_or_l);
MISC_EXTERN char *strnccpy(char *dst, char *src, size_t n, char u_or_l);
MISC_EXTERN int   strxcmp(char *s1, char *s2);
MISC_EXTERN int   strnxcmp(char *s1, char *s2, size_t n);
MISC_EXTERN int   strparse(char *str, char *sep, char *part[], int maxParts);
MISC_EXTERN int   strsplit(char *str, char sep, char *part[], int maxParts);
MISC_EXTERN char *strmove(char *dst, char *src);
MISC_EXTERN char *strshft(char *str, int nchar);
MISC_EXTERN char *strsubs(char *str, char *pat, char *sub);
MISC_EXTERN int   strsuba(char *str, char *pat, char *sub);

/*
 * Prototypes of functions in bitarry.c
 */
MISC_EXTERN int  bitVal(void *bitArray, size_t bitNr);
MISC_EXTERN void bitSet(void *bitArray, size_t bitNr);
MISC_EXTERN void bitClr(void *bitArray, size_t bitNr);

/*
 * Structure for double-linked lists
 */
typedef struct link_in_chain {
  struct link_in_chain *prev; /* toward head of chain (first link) */
  struct link_in_chain *next; /* toward tail of chain (last link) */
} LINK;

/*
 * Function for freeing allocated memory in a link and the link itself
 */
typedef void (*freeLinkFunc)(void *);

/*
 * Prototypes of functions in chain.c
 */
MISC_EXTERN LINK *firstLink(LINK *chain);
MISC_EXTERN LINK *lastLink(LINK *chain);
MISC_EXTERN long  numLinks(LINK *chain);
MISC_EXTERN LINK *appendChain(LINK **head, LINK *chain);
MISC_EXTERN LINK *appendLink(LINK **head, LINK *link);
MISC_EXTERN LINK *insChainBefore(LINK **head, LINK *pos, LINK *chain);
MISC_EXTERN LINK *insLinkBefore(LINK **head, LINK *pos, LINK *link);
MISC_EXTERN LINK *insChainBehind(LINK **head, LINK *pos, LINK *chain);
MISC_EXTERN LINK *insLinkBehind(LINK **head, LINK *pos, LINK *link);
MISC_EXTERN LINK *detachChain(LINK **head, LINK *first, LINK *last);
MISC_EXTERN LINK *detachLink(LINK **head, LINK *link);
MISC_EXTERN LINK *deleteLink(LINK **head, LINK *link, freeLinkFunc freeLink);
MISC_EXTERN void  deleteChain(LINK **head, freeLinkFunc freeLink);

/*
 * Constants and prototype of function for handling german characters.
 */
/* German characters in 7-bit ASCII */
#define A_UML_ASC  91
#define O_UML_ASC  92
#define U_UML_ASC  93
#define a_UML_ASC 123
#define o_UML_ASC 124
#define u_UML_ASC 125
#define ESZET_ASC 126
/* German characters in IBM code pages 437 und 850 */
#define A_UML_IBM 142
#define O_UML_IBM 153
#define U_UML_IBM 154
#define a_UML_IBM 132
#define o_UML_IBM 148
#define u_UML_IBM 129
#define ESZET_IBM 225
/* German characters in ISO Latin 1 */
#define A_UML_ISO 196
#define O_UML_ISO 214
#define U_UML_ISO 220
#define a_UML_ISO 228
#define o_UML_ISO 246
#define u_UML_ISO 252
#define ESZET_ISO 223

MISC_EXTERN char isgerman(char c);

/*
 * Constants and structures for functions in statistics.c
 */
/* error numbers */
#define STATERR_NONE     0
#define STATERR_NO_MEM   1
#define STATERR_NO_DATA  2
#define STATERR_NO_PAIR  3
#define STATERR_BAD_ARG  4
/* structures */
typedef struct statistical_variables {
  size_t  numX;
  double  maxX;
  double  minX;
  double  sumX;
  double  sumXX;
  double *maBuf;    /* values for moving average */
  size_t  maLen;    /* length of moving average */
  size_t *histBuf;  /* histogram counts */
  double  histMin;  /* minimum value of histogram */
  double  barWidth; /* bar width of histogram */
  size_t  numBars;  /* number of bars in histogram */
  size_t  histNum;  /* number of values within histogram range */
  size_t  numBelow; /* number of values below histogram range */
  size_t  numAbove; /* number of values above histogram range */
  size_t  numY;
  double  maxY;
  double  minY;
  double  sumY;
  double  sumYY;
  double  sumXY;
  int     error;
} STAT;
/*
 * Prototypes of functions in statistics.c
 */
MISC_EXTERN void   statInit(STAT *statPtr);
MISC_EXTERN int    statInclMovAvr(STAT *statPtr, size_t length);
MISC_EXTERN int    statInclHist(STAT *statPtr, double minimum,\
				double barWidth, size_t numBars);
MISC_EXTERN void   statClear(STAT *statPtr);
MISC_EXTERN void   statFree(STAT *statPtr);
/* single variable statistics */
MISC_EXTERN void   statAddVal(STAT *statPtr, double val);
MISC_EXTERN double statGetMovAvr(STAT *statPtr);
MISC_EXTERN size_t statGetNum(STAT *s);
MISC_EXTERN double statGetMean(STAT *statPtr);
MISC_EXTERN double statGetSD(STAT *statPtr);
MISC_EXTERN double statGetSigma(STAT *statPtr);
MISC_EXTERN double statEstQuantile(STAT *statPtr, int percent);
MISC_EXTERN int    statPrintHist(STAT *statPtr, FILE *fp);
/* dual variable statistics */
MISC_EXTERN void   statAddX(STAT *statPtr, double X);
MISC_EXTERN void   statAddY(STAT *statPtr, double Y);
MISC_EXTERN size_t statGetNumX(STAT *s);
MISC_EXTERN size_t statGetNumY(STAT *s);
MISC_EXTERN double statGetMeanX(STAT *statPtr);
MISC_EXTERN double statGetMeanY(STAT *statPtr);
MISC_EXTERN double statGetSDX(STAT *statPtr);
MISC_EXTERN double statGetSDY(STAT *statPtr);
MISC_EXTERN double statGetSigmaX(STAT *statPtr);
MISC_EXTERN double statGetSigmaY(STAT *statPtr);
/*linear regression */
MISC_EXTERN int    statAddXY(STAT *statPtr, double X, double Y);
MISC_EXTERN double statGetSlope(STAT *statPtr);
MISC_EXTERN double statGetIntercept(STAT *statPtr);
MISC_EXTERN double statGetCorrCoeff(STAT *statPtr);
MISC_EXTERN double statGetXestimate(STAT *statPtr, double Y);
MISC_EXTERN double statGetYestimate(STAT *statPtr, double X);

/*MISC_EXTERN ;
MISC_EXTERN ;
MISC_EXTERN ;
MISC_EXTERN ;
MISC_EXTERN ;*/

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /*_MISC_H*/
