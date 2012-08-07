/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 2007  Michel Scheffers                          *
*                            IPdS, CAU Kiel                            *
*                            Leibnizstr. 10                            *
*                            24118 Kiel                                *
*                            Germany                                   *
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
* File:     auconv.h                                                   *
* Contents: Header file for conversion functions for non-standard      *
*           audio integer types.                                       *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: auconv.h,v 1.2 2007/09/21 09:53:31 mtms Exp $ */

#ifndef _AUCONV_H
#define _AUCONV_H

#include <inttypes.h>  /* int16_t etc. */
#include <dlldef.h>    /* ASSP_EXTERN */

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t  alaw_t;
typedef uint8_t  ulaw_t;
typedef uint8_t  binoff8_t;  /* unsigned encoded in binary offset */
typedef uint16_t binoff16_t;

ASSP_EXTERN alaw_t  int16_to_alaw(int16_t pcm_val);
ASSP_EXTERN int16_t alaw_to_int16(alaw_t a_val);
ASSP_EXTERN ulaw_t  int16_to_ulaw(int16_t pcm_val);
ASSP_EXTERN int16_t ulaw_to_int16(ulaw_t u_val);
ASSP_EXTERN ulaw_t  alaw_to_ulaw(alaw_t a_val);
ASSP_EXTERN alaw_t  ulaw_to_alaw(ulaw_t u_val);

ASSP_EXTERN int8_t  binoff8_to_int8(binoff8_t u8);
ASSP_EXTERN int16_t binoff8_to_int16(binoff8_t u8);
ASSP_EXTERN int16_t binoff16_to_int16(binoff16_t u16, uint16_t numBits);
ASSP_EXTERN int32_t int24_to_int32(uint8_t *i24Ptr);
ASSP_EXTERN int32_t binoff24_to_int32(uint8_t *i24Ptr, uint16_t numBits);
ASSP_EXTERN void    int32_to_int24(int32_t i32, uint8_t *i24Ptr);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _AUCONV_H */
