/***********************************************************************
*                                                                      *
* This file is part of Michel Scheffers's miscellanous functions       *
* library.                                                             *
*                                                                      *
* Copyright (C) 2003 - 2007  Michel Scheffers                          *
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
* File:     myrint.c                                                   *
* Contents: consistent rounding function                               *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: myrint.c,v 1.2 2007/09/19 10:33:18 mtms Exp $ */

/*DOC
 *
 * Rounds a floating-point number to the nearest integer, returning that 
 * value in double precision floating point.
 * The return value of the rint() function (if available) depends on the 
 * current rounding mode of the system. This ANSI-C replacement always 
 * rounds a fraction >= 0.5 away from zero.
 *
 DOC*/

#include <math.h>    /* ceil() floor() */
#include <misc.h>    /* prototype */

double myrint(double x)
{
  if(x >= 0.0)
    return(floor(x + 0.50));
  else
    return(ceil(x - 0.50));
}

