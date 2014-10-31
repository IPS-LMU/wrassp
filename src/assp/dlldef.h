/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 2007         Lasse Bombien                             *
*                            lasse@phonetik.uni-muenchen.de            *
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
* File:     dlldef.h                                                   *
* Author:   Lasse Bombien                                              *
*                                                                      *
***********************************************************************/
#ifndef _ASSP_DLLDEF_H
#define _ASSP_DLLDEF_H

#if (defined(WIN32) || defined(_WIN32)) && defined(DLL_EXPORT) 
#if defined(BUILDING_ASSP)
#define ASSP_EXTERN  __declspec(dllexport)
#else
#define ASSP_EXTERN  extern __declspec(dllimport)
#endif
#else
#define ASSP_EXTERN extern
#endif

#endif /*_ASSP_DLLDEF*/
