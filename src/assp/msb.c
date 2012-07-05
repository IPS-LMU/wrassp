/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor program    *
* package.                                                             *
*                                                                      *
* Copyright (C) 1989 - 2010  Michel Scheffers                          *
*                            IPdS, CAU Kiel                            *
*                            Leibnizstr. 10                            *
*                            24118 Kiel                                *
*                            Germany                                   *
*                            ms@ipds.uni-kiel.de                       *
*                                                                      *
* This package is free software: you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation, either version 3 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
* This package is distributed in the hope that it will be useful,      *
* but WITHOUT ANY WARRANTY; without even the implied warranty of       *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
* GNU General Public License for more details.                         *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this library. If not, see <http://www.gnu.org/licenses/>. *
*                                                                      *
*----------------------------------------------------------------------*
*                                                                      *
* File:     msb.c                                                      *
* Contents: Extremely simple program which determines the byte-        *
*           orientation of the system and prints it to "stdout".       *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: msb.c,v 1.2 2010/01/22 14:00:55 mtms Exp $ */

#include <stdio.h>
#include <stdlib.h>     /* exit() */
#include <asspendian.h>

int main(int argc, char *argv[])
{
  ENDIAN sysEndian={MSB};
  
  if(MSBFIRST(sysEndian))
    printf("   This system has MSB first (big endian).\n");
  else if(MSBLAST(sysEndian))
    printf("   This system has MSB last (little endian).\n");
  else
    printf("   This system is disoriented.\n");
  exit(0);
}
