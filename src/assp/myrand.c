/***********************************************************************
*                                                                      *
* This file is part of Michel Scheffers's miscellanous functions       *
* library.                                                             *
*                                                                      *
* Copyright (C) 1998 - 2007  Michel Scheffers                          *
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
* File:     myrand.c                                                   *
* Contents: Fast but simple pseudo-random number generator.            *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: myrand.c,v 1.2 2007/09/20 13:04:42 mtms Exp $ */

#include <inttypes.h>  /* uint32_t */
#include <misc.h>      /* prototype */

/*DOC
 *
 * Calculates pseudo-random number with a uniform distribution in the 
 * interval [-1.0 , 1.0]. "seedPtr" points to the start value for the 
 * computation and will be updated to produce a new random number at 
 * the next call.
 *
 * NOTE: This generator is derived from the classical C-library function.
 *       For comments see Presser et al. (2002) "Numerical Recipes in C", 
 *       2nd Edition, Cambridge University Press, p. 276 ff.
 *
 DOC*/

double myrand(uint32_t *seedPtr)
{
  double r;

  *seedPtr *= 1103515245UL;
  *seedPtr += 12345UL;
  r = (double)(*seedPtr);                         /* 0 ... 4294967295 */
  return((r / 2147483647.5) - 1.0);                   /* -1.0 ... 1.0 */
}
