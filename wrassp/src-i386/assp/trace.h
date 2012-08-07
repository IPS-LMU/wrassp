/***********************************************************************
*                                                                      *
* This file is part of Michel Scheffers's miscellanous functions       *
* library.                                                             *
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
* File:     trace.h                                                    *
* Contents:  Public header file for standard trace/debug handler.      *
*          - definition of constants                                   *
*          - extern declaration of global variables and prototypes of  *
*            functions in trace.c                                      *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: trace.h,v 1.5 2010/01/29 08:50:41 mtms Exp $ */

#ifndef _TRACE_H
#define _TRACE_H

#include <stdio.h>      /* FILE */

#include <misc.h>       /* MISC_EXTERN */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * constants
 */
#define TRACE_SUFFIX ".why"
#define TRACE_VAR_LEN 128
#define TRACE_NAM_LEN 255

/*
 * extern declaration of global variables
 */
MISC_EXTERN char  TRACE[];
MISC_EXTERN char  traceFile[];
MISC_EXTERN char *traceOpts;
MISC_EXTERN FILE *traceFP;

/*
 * function prototypes
 */
MISC_EXTERN void openTrace(char *id);
MISC_EXTERN void closeTrace(void);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _TRACE_H */
