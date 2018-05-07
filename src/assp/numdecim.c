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
* File:     numdecim.c                                                 *
* Contents: Support function for formatted output of floating point    *
*           numbers.                                                   *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: numdecim.c,v 1.2 2007/10/01 15:00:01 mtms Exp $ */

#include <stdio.h>   /* sprintf() */
#include <string.h>  /* strlen() */

#include <misc.h>    /* prototype */

/*DOC

Returns the number of relevant decimals places of the floating point 
number "x". If the number has an integral value, 0 is returned.
"maxDecim" gives the maximum number of places to be evaluated,

DOC*/

int numDecim(double x, int maxDecim)
{
  char format[16], string[256]; /* change char format from size 8 to 16 for max %d range */
  int  nd, i;
  
  if(maxDecim <= 0)
    return(0);                                      /* integral value */
  sprintf(format,"%%.%df", maxDecim);         /* create format string */
  sprintf(string, format, x);             /* convert number to string */
  i = strlen(string);
  nd = maxDecim;
  while(nd > 0) {	       /* scan string back to front for zeros */
    i--;
    if(string[i] != '0')
      break;
    nd--;
  }
  return(nd);
}

