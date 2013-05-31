/***********************************************************************
*                                                                      *
* This file is part of Michel Scheffers's miscellanous functions       *
* library.                                                             *
*                                                                      *
* Copyright (C) 1995 - 2010  Michel Scheffers                          *
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
* File:     parsepath.c                                                *
* Contents: Functions to extract parts of the path to a file.          *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: parsepath.c,v 1.6 2010/01/27 10:59:24 mtms Exp $ */

#include <stddef.h>   /* size_t */
#include <string.h>   /* strchr() strlen() strrchr() strcpy() strcat() */

#include <misc.h>     /* prototypes PATH/NAME/SUFF_MAX EOS */

/*DOC
 *
 * Strips directory path from full path to a file.
 *
 *-- synopsis --
 *
 * char *myfilename(fullPath)
 *
 * char *fullPath   null-terminated string with full path to file
 *
 *-- returns --
 *
 * Pointer to string with file name or NULL upon error.
 *
 *-- remarks --
 *
 * - The return value points to the beginning of the file name in 
 *   "fullPath". That part should therefore be copied before a change 
 *   in "fullPath" renders the pointer invalid.
 * - If "fullPath" is in fact a directory path, the directory name will 
 *   be returned, i.e. it is not verified whether the result refers to 
 *   a file or a directory.
 *
 DOC*/

char *myfilename(char *fullPath)
{
  char *cPtr;
  
  if(fullPath == NULL)
    return(NULL);
  cPtr = strrchr(fullPath, DIR_SEP_CHR);  /* last directory separator */
  if(cPtr != NULL)
    cPtr++;
  else                               /* only file name: reset pointer */
    cPtr = fullPath;
  return(cPtr);
}

/*DOC
 *
 * Strips directory path and last extension from full path to a file.
 *
 *-- synopsis --
 *
 * char *mybasename(fullPath)
 *
 * char *fullPath   null-terminated string with full path to file
 *
 *-- returns --
 *
 * Pointer to string with base name. The string will be empty upon error
 * ("fullPath" is NULL or base name too long).
 *
 *-- remarks --
 *
 * - The return value points at static memory. Its contents must 
 *   therefore be copied before a next call to this function modifies 
 *   them.
 * - Leading periods in a file name will be interpreted correctly.
 *
 DOC*/

char *mybasename(char *fullPath)
{
  static char result[NAME_MAX+1];
  register char *cPtr;
  
  strcpy(result, "");
  cPtr = myfilename(fullPath);            /* get pointer to file name */
  if(cPtr == NULL)
    return(result);          /* prefer empty string over NULL-pointer */
       /* because many strcpy()s crash on being passed a NULL-pointer */
  if(strlen(cPtr) > NAME_MAX)
    return(result);
  strcpy(result, cPtr);
  cPtr = result;
  while(*cPtr == '.')
    cPtr++;                                   /* skip leading periods */
  cPtr = strrchr(cPtr, '.');                 /* search last extension */
  if(cPtr != NULL)
    *cPtr = EOS;                                    /* and cut it off */
  return(result);
}

/*DOC
 *
 * Strips directory path and all extensions from full path to a file.
 *
 *-- synopsis --
 *
 * char *mybarename(fullPath)
 *
 * char *fullPath   null-terminated string with full path to file
 *
 *-- returns --
 *
 * Pointer to string with bare name. The string will be empty upon error
 * ("fullPath" is NULL or bare name too long).
 *
 *-- remarks --
 *
 * - The return value points at static memory. Its contents must 
 *   therefore be copied before a next call to this function modifies 
 *   them.
 * - Leading periods in a file name will be interpreted correctly.
 *
 DOC*/

char *mybarename(char *fullPath)
{
  static char result[NAME_MAX+1];
  register char *cPtr;
  
  strcpy(result, "");
  cPtr = myfilename(fullPath);            /* get pointer to file name */
  if(cPtr == NULL)
    return(result);          /* prefer empty string over NULL-pointer */
       /* because many strcpy()s crash on being passed a NULL-pointer */
  if(strlen(cPtr) > NAME_MAX)
    return(result);
  strcpy(result, cPtr);
  cPtr = result;
  while(*cPtr == '.')
    cPtr++;                                    /* skip leading periods */
  cPtr = strchr(cPtr, '.');                  /* search first extension */
  if(cPtr != NULL)
    *cPtr = EOS;                                     /* and cut it off */
  return(result);
}

/*DOC
 *
 * Parses the full path to a file into the directory path (including 
 * trailing separator symbol), the base name of the file and the last 
 * extension (incl. period) of the file name.
 *
 *-- synopsis --
 *
 * int parsepath(fullPath, dirPath, baseName, extension)
 *
 * char  *fullPath   null-terminated string with full path to file
 *               (each of the following pointers may be NULL)
 * char **dirPath    pointer to pointer for directory path
 * char **baseName   pointer to pointer for base name of file
 * char **extension  pointer to pointer for extension of file name
 *
 *-- returns --
 *
 *  -1 if fullPath NULL, empty or too long; otherwise 0
 *
 *-- remarks --
 *
 * - The pointers "dirPath", "baseName" and "extension" will be set to 
 *   point at static memory. Their contents must therefore be copied 
 *   before a next call to this function modifies them.
 * - Leading periods in a file name will be interpreted correctly.
 *
 DOC*/

int parsepath(char *fullPath, char **dirPath,\
	      char **baseName, char **extension)
{
  static char path[PATH_MAX+1], base[NAME_MAX+1], ext[SUFF_MAX+1];
  register char  *cPtr;
  size_t len;

  path[0] = base[0] = ext[0] = EOS;                 /* clear strings */
  if(dirPath != NULL)          /* set pointers for requested elements */
    *dirPath = path;
  if(baseName != NULL)
    *baseName = base;
  if(extension != NULL)
    *extension = ext;

  if(fullPath == NULL)
    return(-1);
  len = strlen(fullPath);
  if(len == 0 || len > PATH_MAX)
    return(-1);

  strcpy(path, fullPath);
  cPtr = strrchr(path, DIR_SEP_CHR);  /* search last separator symbol */
  if(cPtr != NULL) {
    cPtr++;                                  /* skip separator symbol */
    if(strlen(cPtr) > NAME_MAX)
      return(-1);
    strcpy(base, cPtr);    /* copy remainder to string with base name */
    *cPtr = EOS;                  /* close string with directory path */ 
  }
  else {
    if(len > NAME_MAX)
      return(-1);
    strcpy(base, fullPath);          /* path contained only file name */
    path[0] = EOS;                            /* clear directory path */
  }
  cPtr = base;
  while(*cPtr == '.')
    cPtr++;                                   /* skip leading periods */
  cPtr = strrchr(cPtr, '.');          /* search last extension symbol */
  if(cPtr != NULL) {
    if(strlen(cPtr) > SUFF_MAX)
      return(-1);
    strcpy(ext, cPtr);             /* copy extension including period */
    *cPtr = EOS;                       /* close string with base name */
  }
  return(0);
}
