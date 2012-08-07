/***********************************************************************
*                                                                      *
* This file is part of Michel Scheffers's miscellanous functions       *
* library.                                                             *
*                                                                      *
* Copyright (C) 1992 - 2009  Michel Scheffers                          *
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
* File:     miscstring.c                                               *
* Contents: - strict ANSI-C versions of string functions which are not *
*             available in all compilers                               *
*           - functions to split a string into sub-strings             *
*           - AWK-like string substitution functions                   *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: miscstring.c,v 1.3 2009/01/23 10:26:40 mtms Exp $ */

#include <stdio.h>   /* NULL*/
#include <ctype.h>   /* islower() isupper() tolower() toupper() */
#include <stddef.h>  /* size_t */
#include <string.h>  /* strstr() strlen() */
#include <misc.h>    /* prototypes LOCAL EOS TRUE FALSE */

/*DOC
 *
 * Copies the string pointed to by "src" to the one pointed to by "dst" 
 * with case conversion as specified by "u_or_l". "u_or_l" should be 
 * 'u' or 'U' for conversion to upper case or 'l' or 'L' for lower case.
 * "src" and "dst" may be identical but may not partly overlap. "src" 
 * must end with a NULL byte and "dst" should have sufficient length.
 * This function returns the pointer "dst" or NULL upon error.
 *
 DOC*/

char *strccpy(register char *dst, register char *src, char u_or_l)
{
  register int c;
  char *keep;
  
  if(dst == NULL || src == NULL)
    return(NULL);
  keep = dst;
  if(u_or_l == 'U' || u_or_l == 'u') {       /* convert to upper case */
    do {
      c = *(src++);
      if(c != 0 && islower(c))
	c = toupper(c);
      *(dst++) = (char)c;
    } while(c != 0);
  }
  else if(u_or_l == 'L' || u_or_l == 'l') {  /* convert to lower case */
    do {
      c = *(src++);
      if(c != 0 && isupper(c))
	c = tolower(c);
      *(dst++) = (char)c;
    } while(c != 0);
  }
  else return(NULL);
  return(keep);
}

/*DOC
 *
 * Copy string with case conversion. 'src' and 'dst' may be identical
 * but may not partly overlap. 'src' must end with a NULL byte.
 * This function copies exactly 'n' bytes, truncating 'src' or adding
 * NULL characters to 'dst' if necessary. The result will not be NULL-
 * terminated if the length of 'src' is 'n' or more.
 * Returns pointer to 'dst' or NULL upon error.
 *
 DOC*/

char *strnccpy(register char *dst, register char *src,\
	       register size_t n, char u_or_l)
{
  register int    c;
  register size_t i;
  char *keep;
  
  if(dst == NULL || src == NULL)
    return(NULL);
  keep = dst;
  if(u_or_l == 'U' || u_or_l == 'u') {       /* convert to upper case */
    for(i = 0; i < n; i++) {
      c = *(src++);
      if(c == 0) break;
      if(islower(c))
	c = toupper(c);
      *(dst++) = (char)c;
    }
    while(i < n) {
      *(dst++) = '\0';
      i++;
    }
  }
  else if(u_or_l == 'L' || u_or_l == 'l') {  /* convert to lower case */
    for(i = 0; i < n; i++) {
      c = *(src++);
      if(c == 0) break;
      if(isupper(c))
	c = tolower(c);
      *(dst++) = (char)c;
    }
    while(i < n) {
      *(dst++) = '\0';
      i++;
    }
  }
  else return(NULL);
  return(keep);
}

/*DOC
 *
 * Case-insensitive versions of the ANSI-C functions strcmp() and strncmp().
 * The returned value is based on comparisons made in lower case.
 *
 DOC*/

int strxcmp(register char *s1, register char *s2)
{
  register int c1, c2;
  
  if(s1 == NULL) {
    if(s2 == NULL) return(0);
    return(-(int)*s2);
  }
  if(s2 == NULL) return((int)*s1);

  do {
    c1 = (int)*(s1++);
    if(c1 && isupper(c1))
       c1 = tolower(c1);
    c2 = (int)*(s2++);
    if(c2 && isupper(c2))
       c2 = tolower(c2);
  } while(c1 && c2 && c1 == c2);

  return(c1 - c2);
}

int strnxcmp(register char *s1, register char *s2, size_t n)
{
  register int c1, c2;

  if(s1 == NULL) {
    if(s2 == NULL) return(0);
    return(-(int)*s2);
  }
  if(s2 == NULL) return((int)*s1);
  if(n == 0) return(0);

  do {
    n--;
    c1 = (int)*(s1++);
    if(c1 && isupper(c1))
       c1 = tolower(c1);
    c2 = (int)*(s2++);
    if(c2 && isupper(c2))
       c2 = tolower(c2);
  } while(n > 0 && c1 && c2 && c1 == c2);

  return(c1 - c2);
}

/*DOC
 *
 * Support function for strparse(): Returns 'TRUE' (1) if the character 
 * 'c' matches one of the characters in the string pointed to by "sep",
 * otherwise it returns 'FALSE' (0).
 * If "sep" is a NULL pointer "c" will be tested on being a white space 
 * character.
 *
 DOC*/

LOCAL int isSep(register char c, register char *sep)
{
  if(sep == NULL)
    return(isspace((int)c));
  while(*sep != EOS) {
    if(c == *sep)
      return(TRUE);
    sep++;
  }
  return(FALSE);
}

/*DOC
 *
 * Parses the string pointed to by "str" into a set of substrings, 
 * separated by one or more of the characters in the string pointed to 
 * by "sep". If "sep" is a NULL pointer or points to an empty string, 
 * white spaces will be used as separator characters. Initial separator 
 * characters will be ignored.
 * Pointers to the first character of each substring will be stored in 
 * the character pointer array "part" whose size must be at least 
 * "maxParts". Each substring will be terminated by a NULL byte.
 * This function returns the number of substrings found or -1 if more 
 * than "maxParts" substrings were found. In that case, the last 
 * element in "part" contains a pointer to the remaining (unparsed) 
 * part of the string.
 *
 * BEWARE: This function always modifies the string pointed to by "str".
 *
 DOC*/

int strparse(register char *str, register char *sep,\
	     char *part[], int maxParts)
{
  int num;
  
  num = 0;
  if(str != NULL) {
    if(sep != NULL && *sep == EOS)
      sep = NULL;
    while(*str != EOS) {
      while(isSep(*str, sep))
	str++;                                /* search begin of part */
      if(*str != EOS) {
	if(num >= maxParts)
	  return(-1);
	part[num] = str;                               /* set pointer */
	num++;
	while(*str != EOS && !isSep(*str, sep))
	  str++;                                /* search end of part */
	if(*str != EOS) {            /* must be a separator character */
	  *str = EOS;                                 /* close string */
	  str++;                       /* advance to search next part */
	}
      }
    }
  }
  return(num);
}

/*DOC
 *
 * This function acts as strparse() except that only a single character 
 * may be given as separator and that each occurrence of "sep" produces
 * a substring. Thus if "str" contains N separators, N + 1 substrings 
 * (any of which may be empty) will be returned.
 *
 DOC*/

int strsplit(register char *str, register char sep,\
	     char *part[], int maxParts)
{
  int num;
  
  if(str == NULL || *str == EOS)
    return(0);
  if(maxParts < 1)
    return(-1);

  part[0] = str;
  num = 1;
  do {
    while(*str != EOS && *str != sep)
      str++;
    if(*str == sep) {
      if(num >= maxParts)
	return(-1);
      *str = EOS;
      str++;
      part[num] = str;
      num++;
    }
  } while(*str != EOS);

  return(num);
}

/*DOC
 *
 * Copies the characters in the string pointed to by "src" to the string 
 * pointed to by "dst". The closing NULL-byte in "str" will NOT be copied!
 * Returns a pointer to the character position past the last one copied.
 *
 * BEWARE: The caller must ensure sufficient space in "dst".
 *
 DOC*/

char *strmove(register char *dst, register char *src)
{
  if(dst == NULL || src == NULL)
    return(dst);
  while(*src != EOS)
    *(dst++) = *(src++);
  return(dst);
}

/*DOC
 *
 * Shifts the string pointed to by "str" by "nchar" character positions. 
 * If "nchar" is positive the string is shifted to the right and the 
 * characters in the resulting gap will be set to blanks. Otherwise, 
 * the shift will be to the left. The new string is always terminated 
 * by a NULL-byte.
 * This function returns a pointer to the new position of the string 
 * (i.e. "str" + "nchar").
 *
 * BEWARE: It is the responsibility of the calling function to ensure 
 *         that no underflow or overflow occurs in the array of which 
 *         the string is a part.
 *
 DOC*/

char *strshft(register char *str, int nchar)
{ 
  register char *rPtr, *wPtr;

  if(str == NULL || nchar == 0)
    return(str);
  rPtr = str;                                     /* set read pointer */
  if(nchar > 0) { /* copy back to front */
    while(*rPtr) rPtr++;                       /* go to end of string */
    for(wPtr = rPtr+nchar; rPtr >= str; rPtr--, wPtr--)
      *wPtr = *rPtr;                     /* include null-byte in copy */
    while(wPtr >= str)
      *(wPtr--) = ' ';                        /* fill gap with blanks */
  }
  else {          /* copy front to back */
    for(wPtr = rPtr+nchar; *rPtr != EOS; rPtr++, wPtr++)
      *wPtr = *rPtr;
    *wPtr = EOS;                                 /* set end of string */
  }
  return(str+nchar);
}

/*DOC
 *
 * Substitutes the first occurrence of the string pointed to by "pat" in 
 * the string pointed to by "str" by the string pointed to by "sub".
 * This function returns a pointer past the substitution or NULL if "pat" 
 * does not occur in "str".
 *
 * BEWARE: It is the responsibility of the calling function to ensure 
 *         that there is sufficient space in "str" if "sub" is longer 
 *         than "pat".
 *
 DOC*/

char *strsubs(char *str, char *pat, char *sub)
{
  size_t l, n;

  if(str == NULL || pat == NULL)
    return(NULL);
  str = strstr(str, pat);
  if(str != NULL) {
    l = strlen(pat);
    if(sub != NULL)
      n = strlen(sub);
    else
      n = 0;
    strshft(&str[l], n-l);                           /* adjust length */
    str = strmove(str, sub);           /* overwrite with substitution */
  }
  return(str);
}

/*DOC
 *
 * Substitutes all occurrences of the string pointed to by "pat" in the 
 * string pointed to by "str" by the string pointed to by "sub".
 * This function returns the number of substitutions made.
 *
 * BEWARE: It is the responsibility of the calling function to ensure 
 *         that there is sufficient space in "str" if "sub" is longer 
 *         than "pat".
 *
 DOC*/

int strsuba(char *str, char *pat, char *sub)
{
  int n;

  n = 0;
  while(str != NULL) {
    str = strsubs(str, pat, sub);
    if(str != NULL)
      n++;
  }
  return(n);
}
