/***********************************************************************
*                                                                      *
* This file is part of Michel Scheffers's miscellanous functions       *
* library.                                                             *
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
* File:     bitarray.c                                                 *
* Contents: Functions for handling a bitarray.                         *
*           Prototypes of these functions are in 'misc.h'.             *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: bitarray.c,v 1.3 2007/10/02 14:52:21 mtms Exp $ */

#include <stdio.h>     /* NULL */
#include <stddef.h>    /* size_t */
#include <inttypes.h>  /* uint8_t */

#include <misc.h>      /* prototypes */

/*DOC

Functions for handling a bit array.

bitVal() returns the value of the bit "bitNr" in the array pointed to 
         by "bitArray" or -1 upon error.
bitSet() sets the bit "bitNr" in the array pointed to by "bitArray" to 1
bitClr() sets the bit "bitNr" in the array pointed to by "bitArray" to 0

NOTE: Bit numbering is reversed from 'normal' C.

DOC*/

int bitVal(void *bitArray, register size_t bitNr)
{
  register uint8_t *bytePtr, mask;
  
  if(bitArray == NULL) return(-1);
  bytePtr = (uint8_t *)bitArray;
  bytePtr += (bitNr >> 3);         /* get byte that contains this bit */
  bitNr = 7 - (bitNr % 8);             /* reversed bit number in byte */
  if(bitNr == 0) mask = 1;
  else           mask = 1 << bitNr;             /* generate byte mask */
  if(*bytePtr & mask) return(1);
  return(0);
}

void bitSet(void *bitArray, register size_t bitNr)
{
  register uint8_t *bytePtr, mask;
  
  if(bitArray == NULL) return;
  bytePtr = (uint8_t *)bitArray;
  bytePtr += (bitNr >> 3);
  bitNr = 7 - (bitNr % 8);
  if(bitNr == 0) mask = 1;
  else           mask = 1 << bitNr;
  *bytePtr |= mask;
  return;
}

void bitClr(void *bitArray, register size_t bitNr)
{
  register uint8_t *bytePtr, mask;
  
  if(bitArray == NULL) return;
  bytePtr = (uint8_t *)bitArray;
  bytePtr += (bitNr >> 3);
  bitNr = 7 - (bitNr % 8);
  if(bitNr == 0) mask = 1;
  else           mask = 1 << bitNr;
  *bytePtr &= ~mask;
  return;
}

