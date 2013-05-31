//
// C++ Interface: dlldef
//
// Description:
//
//
// Author: Lasse Bombien,,, <lasse@cordulo>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
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
