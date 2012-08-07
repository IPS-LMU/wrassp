/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
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
* File:     ipds_lbl.h                                                 *
* Contents: Properties and keywords for label files in IPdS MIX or     *
*           SAMPA format.                                              *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: ipds_lbl.h,v 1.4 2010/05/14 08:55:59 mtms Exp $ */

#ifndef _IPDS_LBL_H
#define _IPDS_LBL_H

#ifdef __cplusplus
extern "C" {
#endif

#define HEAP_SN (1600000L)   /* sample number of heap */
#define MIX_SFR (16000.0)    /* sampling frequency fixed in MIX ! */
/*
 * limitations of the original MIX programme (obsolete)
 */
#define MAXMIXLINE 126       /* maximum line length in MIX file */
#define MAXMIXLABELS 512     /* maximum number of labels in MIX file */
#define MAXMIXNAME 8         /* maximum length of MIX label name */
/*
 * keywords in MIX header
 */
#define MIX_ORT_ID "TEXT:"   /* begin of orthography */
#define MIX_CAN_ID "PHONET:" /* begin of canonical form */
#define MIX_TRF_ID "CT 1"    /* end of fixed part/begin of transcribed form */
#define MIX_TRF_ERR "CT 1 "  /* many files have this erroneous line */
#define MIX_SFR_ID "SAMPLE_RATE:" /* NEW! sampling frequency */
#define MIX_VAR_ID "LABELS:" /* begin of optional variant field */
#define MIX_LBL_ID "FR "     /* identification of label lines */
/*
 * MIX line formats
 */
#define MIX_SFR_LINE "SAMPLE_RATE: %.1f Hz\n"
/* original label line with insufficient time resolution
   the 3rd argument is the time in full centi-seconds 
   ('frames' with count starting at 1 as for the sample number)
  #define MIX_LBL_LINE "FR %9ld %-10s %7ld %11.5f sec\n"
*/
#define MIX_LBL_LINE "FR %9ld %-10s %7ld %12.7f sec\n"

/* This line format has adjustable precision for 'time'; the precision */
/* (int) should be before the argument 'time' in an sprintf. Defined as a */
/* string without an end-of-line so you can append the EOL string and then */
/* use a binary write to avoid EOL conversion. */
#define MIX_LBL_STR_AP "FR %9ld %-10s %7ld  %.*f sec"

#define MIX_MIN_FIELDS 3     /* fields in prototype label line */
#define MIX_MAX_FIELDS 6     /* fields in segmented label line */
#define MIX_EOL_STR "\x0A"   /* <LF> (traditionally) */
/*
 * keywords in SAMPA header
 * (first line contains file name)
 */
#define SAM_EOT_ID "oend"    /* end of orthography/begin of canonical form */
#define SAM_EOC_ID "kend"    /* end of canonical/begin of transcribed form */
#define SAM_SFR_ID "sample_rate" /* NEW! sampling frequency */
#define SAM_EOH_ID "hend"    /* end-of-header */
/*
 * SAMPA line formats
 */
#define SAM_SFR_LINE "sample_rate %.1f Hz\n"
/* original label line without time column
  #define SAM_LBL_LINE "%9ld %-10s\n"
*/
#define SAM_LBL_LINE "%9ld %-10s %12.7f\n"

/* as for the MIX format */
#define SAM_LBL_STR_AP "%9ld %-10s  %.*f sec"

#define SAM_MIN_FIELDS 2     /* fields in original label line */
#define SAM_MAX_FIELDS 3     /* fields in current label line */
#define SAM_EOL_STR "\x0A"   /* <LF> (traditionally) */
/*
 * maximum width of formatted header line
 */
#define HDR_LINE_WIDTH 72
/*
 * PhonDat '92 idiosyncrasies
 */
#define PH92_E_ID "SPRA"
#define PH92_S_ID "% SIEMENS"

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _IPDS_LBL_H */
