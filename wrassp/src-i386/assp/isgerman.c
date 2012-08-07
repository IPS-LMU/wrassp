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
* File:     isgerman.c                                                 *
* Contents: Functions for handling special german characters.          *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: isgerman.c,v 1.2 2007/10/02 14:20:42 mtms Exp $ */

#include <misc.h> /* ASCII, IBM and ISO codes for german characters */

/***********************************************************************
* Check whether 'c' contains a code for a special german character,    *
* if so, return simple letter, otherwise return the NULL character.    *
***********************************************************************/
char isgerman(char c)
{
  switch((int)(unsigned char)c) {
  case A_UML_ASC:
  case A_UML_IBM:
  case A_UML_ISO:
    return('A');
  case O_UML_ASC:
  case O_UML_IBM:
  case O_UML_ISO:
    return('O');
  case U_UML_ASC:
  case U_UML_IBM:
  case U_UML_ISO:
    return('U');
  case a_UML_ASC:
  case a_UML_IBM:
  case a_UML_ISO:
    return('a');
  case o_UML_ASC:
  case o_UML_IBM:
  case o_UML_ISO:
    return('o');
  case u_UML_ASC:
  case u_UML_IBM:
  case u_UML_ISO:
    return('u');
  case ESZET_ASC:
  case ESZET_IBM:
  case ESZET_ISO:
    return('s');
  default:
    return('\0');
  }
}

