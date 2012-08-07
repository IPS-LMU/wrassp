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
* File:     aucheck.h                                                  *
* Contents: Header file with constants and function prototypes related *
*           to properties of audio signals and capacities of functions *
*           to handle them.                                            *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: aucheck.h,v 1.4 2010/01/14 14:44:44 mtms Exp $ */

#ifndef _AUCHECK_H
#define _AUCHECK_H

#include <dataobj.h>  /* fform_e DOBJ DDESC */
#include <dlldef.h>   /* ASSP_EXTERN */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * flags for functions like checkSound() and play() which have to 
 * verify whether a given kind of audio coding can be handled
 */
#define AUC_NONE   (0L)
#define AUC_ERROR  (-1L)
#define AUC_CHAN_MASK 0x000000FF  /* lowest byte contains number of */
                                  /* channels (255 should be ample) */
#define AUC_ALAW      0x00000100  /* 8-bit A-Law encoded */
#define AUC_uLAW      0x00000200  /* 8-bit mu-Law encoded */
#define AUC_U8        0x00000400  /* unsigned 8-bit binary offset */
#define AUC_I8        0x00000800  /* signed 8-bit PCM */
#define AUC_BYTE_MASK 0x00000F00  /* mask for single byte format */

#define AUC_U16       0x00001000  /* unsigned 16-bit binary offset */
#define AUC_I16       0x00002000  /* signed 16-bit PCM */
#define AUC_U24       0x00004000  /* unsigned 24-bit binary offset */
#define AUC_I24       0x00008000  /* signed 24-bit PCM */
#define AUC_U32       0x00010000  /* unsigned 32-bit binary offset */
#define AUC_I32       0x00020000  /* signed 32-bit PCM */
#define AUC_F32       0x00040000  /* single precision floating point */
#define AUC_F64       0x00080000  /* double precision floating point */
#define AUC_SWAP_MASK 0x000FF000  /* mask for multi-byte format */

#define AUC_FORM_MASK 0x000FFF00  /* mask for format/coding bits */

#define AUC_RESERVED  0x00F00000  /* reserved for future enhancements */

#define AUC_DEV       0x01000000  /* bits to adjust messages to test */
#define AUC_FILE      0x02000000
#define AUC_FUNC      0x04000000
#define AUC_HEAD      0x08000000
#define AUC_TEST_MASK 0x0F000000  /* mask for testing bits */

#define AUC_MSB_L     0x10000000  /* MSB last (little endian) */
#define AUC_MSB_F     0x20000000  /* MSB first (big endian) */
#define AUC_MSB_X (AUC_MSB_F | AUC_MSB_L) /* don't care/can handle both */
#define AUC_MSB_MASK  AUC_MSB_X   /* mask for endian */

/*
 * prototypes of functions in aucheck.c
 */
ASSP_EXTERN long checkSound(DOBJ *dop, long auCaps, int channel);
ASSP_EXTERN long auPropsDO(DOBJ *dop);
ASSP_EXTERN long auPropsDD(DDESC *dd);
ASSP_EXTERN long auCapsFF(fform_e fileFormat);
ASSP_EXTERN int  checkAuBits(DDESC *dd);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _AUCHECK_H */
