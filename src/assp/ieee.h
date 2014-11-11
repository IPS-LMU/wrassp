/***********************************************************************
*                                                                      *
* This file is part of the "Advanced Speech Signal Processor" library  *
* 'libassp'.                                                           *
* Copyright (C) 1989 - 2011  Michael T.M. Scheffers                    *
*                            IPdS, CAU Kiel                            *
* Copyright (C) 2011 - 2014  Michael T.M. Scheffers                    *
*                            ISFAS-ASW, CAU Kiel                       *
*                            Leibnizstr. 10, R. 409                    *
*                            24118 Kiel, Germany                       *
*                            m.scheffers@isfas.uni-kiel.de             *
*                                                                      *
* This library is free software: you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation, either version 3 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
* This library is distributed in the hope that it will be useful,      *
* but WITHOUT ANY WARRANTY; without even the implied warranty of       *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                 *
* See the GNU General Public License for more details.                 *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this library. If not, see <http://www.gnu.org/licenses/>. *
*                                                                      *
*----------------------------------------------------------------------*
*                                                                      *
* File    : ieee.h                                                     *
* Contents:                                                            *
* Authors : Michel Scheffers                                           *
*                                                                      *
***********************************************************************/
/* $Id:  Exp $ */

#ifndef _IEEE_H
#define _IEEE_H

#include <inttypes.h>   /* uint8_t */

#include <dlldef.h>     /* ASSP_EXTERN */

#ifdef __cplusplus
extern "C" {
#endif

#define XFPSIZE 10 /* number of bytes in IEEE 754 extended floating point */

ASSP_EXTERN void ConvertToIeeeExtended(double num, uint8_t *bytes);
ASSP_EXTERN double ConvertFromIeeeExtended(uint8_t *bytes);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _IEEE_H */
