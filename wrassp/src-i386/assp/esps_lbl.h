/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 2002 - 2010  Michel Scheffers                          *
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
* File:     esps_lbl.h                                                 *
* Contents: Properties and keywords for label files in ESPS xlabel     *
*           format taken from the waves+ Manual Release 5.3.           *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: esps_lbl.h,v 1.2 2010/05/14 08:55:59 mtms Exp $ */

#ifndef _ESPS_LBL_H
#define _ESPS_LBL_H

#ifdef __cplusplus
extern "C" {
#endif

#define XLBL_EOL_CHAR (0x0A) /* <LF> (Unix only) */
#define XLBL_EOL_STR  "\x0A"
#define XLBL_EOH_STR "#" /* end of header; the only mandatory header item! */

/*
 * Optional keywords in the header; there may be others but according to 
 * the manual, xlabel only interprets 'color', 'font' and 'separator'.
 * Keyword-value pairs are separated by a blank.
 */
#define XLBL_REF_ID    "signal"  /* name of reference audio file (string) */
#define XLBL_DEF_REF   "eddy"    /* default (often seen) */
#define XLBL_VERS_ID   "type"    /* format version (int) */
#define XLBL_DEF_VERS  (0)       /* apparently never got beyond version 0 */
#define XLBL_COLOR_ID  "color"   /* colormap entry for the label (int) */
#define XLBL_DEF_COLOR (121)     /* from the example */
#define XLBL_FONT_ID   "font"    /* font for displaying the label (string) */
#define XLBL_TIERS_ID  "nfields" /* maximum number of tiers in file (int) */
#define XLBL_DEF_TIERS (1)
#define XLBL_SEP_ID    "separator" /* tier separator in label string (char) */
#define XLBL_DEF_SEP   ";"       /* the default - must be non-whitespace */
#define XLBL_COMM_ID   "comment"

/*
 * Line format; the field separator may be spaces or tabs.
 * Common practice is to have the time specifying the end of a segment
 * and to preceed the labels with a generalized pause label.
 * For multi-tier label files the names are separated by the separator 
 * character specified in the header or ';' if not specified.
 * BEWARE: label lines need not be in chronological order!
 */
#define XLBL_NUM_FIELDS 3 /* time, color, name(s) */

/* This fixed line format has sufficient precision for 'time' to be */
/* rounded exactly to a sample number at a rate of 11025 Hz. */
#define XLBL_LINE "%.9f  %i  %s\n"

/* This line format has adjustable precision for 'time'; the precision */
/* (int) should be the first argument in a printf after the format. */
#define XLBL_LINE_AP "%.*f  %i  %s\n"

/* As above but as a string without an end-of-line so you can append the */
/* EOL string and then use a binary write to avoid EOL conversion. */
#define XLBL_STR_AP "%.*f  %i  %s"

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _ESPS_LBL_H */
