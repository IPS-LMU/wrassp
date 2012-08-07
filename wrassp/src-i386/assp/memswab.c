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
* File:     memswab.c                                                  *
* Contents: General purpose byte swapper.                              *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: memswab.c,v 1.2 2007/10/04 09:01:24 mtms Exp $ */

#include <stddef.h>    /* size_t */
#include <inttypes.h>  /* uint8_t */
#include <asspendian.h> /*memswab*/

/*DOC

"dst" and "src" may point to the same array
varSize: size of variable in bytes
numVars: number of variables to swap

DOC*/

void memswab(void *dst, void *src, size_t varSize, size_t numVars)
{
  register uint8_t *iPtr, *oPtr, *ePtr, save;
  register int      l, r;
  
  if(varSize < 1 || (varSize < 2 && dst == src))
    return;
  iPtr = src;                           /* set pointers */
  oPtr = dst;
  ePtr = iPtr + numVars*varSize;        /* end of array */
  while(iPtr < ePtr) {
    for(l = 0, r = varSize-1; l <= r; l++, r--) {
      save = iPtr[r];
      oPtr[r] = iPtr[l];
      oPtr[l] = save;
    }
    iPtr += varSize;                    /* next variable */
    oPtr += varSize;
  }
  return;
}
