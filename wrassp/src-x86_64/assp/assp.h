/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 2008  Michel Scheffers                          *
*                            IPdS, CAU Kiel                            *
*                            Leibnizstr. 10                            *
*                            24118 Kiel, Germany                       *
*                            ms@ipds.uni-kiel.de                       *
*                                                                      *
* This library is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 3 of the License, or    *
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
* File:     assp.h                                                     *
* Contents: Private header file for ASSP programs.                     *
*           - defines version and identification                       *
*           - includes the ASSP message handler                        *
*           - includes the trace/debug handler                         *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: assp.h,v 1.6 2008/04/25 13:08:02 mtms Exp $ */

#ifndef _ASSP_H
#define _ASSP_H

#define ASSP_MAJOR 2
#define ASSP_MINOR 0
#define ASSP_PROG

#include <asspmess.h>  /* message handler */
#include <trace.h>     /* trace/debug handler */
#include <assptime.h>  /* standard conversion macros */

#endif /* _ASSP_H */
