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
* File:     fgetl.c                                                    *
* Contents: An attempt to combine the better properties of the ANSI-C  *
*           gets() and fgets() functions.                              *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: fgetl.c,v 1.3 2009/04/27 09:12:05 mtms Exp $ */

#include <stdio.h>      /* FILE EOF */

#include <misc.h>       /* prototype */

#define EOL_LF 0x0A     /* line feed / new line */
#define EOL_CR 0x0D     /* carriage return */

/*DOC

Reads at most one less than "size" characters from the stream pointed 
to by "fp", and copies them into "buffer". "fp" may hereby be 'stdin'.
Reading stops upon encountering End-Of-File, an End-Of-Line code or 
when the number of characters copied equals "size" -1. "buffer" will 
always be terminated with a NULL-byte.
The End-Of-Line code may either be <LF> (UNIX), <CR> (MAC/OS-9) or 
<CR><LF> (DOS) and will NOT be copied.
The function returns the number of characters copied into "buffer" or 
'EOF' when End-Of-File has been encountered without any characters 
having been copied. A return value of 0 thus indicates an empty line.
If "eolPtr" does not equal NULL, it will be set to point at a string 
which will normally contain the End-Of-Line code encountered.
The string will be empty when "fp" refers to 'stdin' or when End-Of-File 
has been reached (even when the function does not return EOF).
Its first character will equal EOF when the length of the line exceeds 
"size" -1. The string is static memory; its contents should therefore be 
evaluated/copied before another call to "fgetl" modifies them.

DOC*/

int fgetl(char *buffer, int size, FILE *fp, char **eolPtr)
{
  static char eolStr[4];
  int i, cnt, chr, nxt;

  for(i = 0; i < 4; i++)
    eolStr[i] = '\0';
  if(eolPtr != NULL)
    *eolPtr = eolStr;
  if(buffer == NULL || size < 1 || fp == NULL) {
    if(buffer != NULL)
      *buffer = '\0';
    return(-1);
  }
  i = cnt = 0;
  while((chr=fgetc(fp)) != EOF) {
    if(chr == EOL_LF) {                                       /* UNIX */
      eolStr[i++] = EOL_LF;
      break;
    }
    if(chr == EOL_CR) {                           /* DOS, MAC or OS-9 */
      eolStr[i++] = EOL_CR;
      nxt = fgetc(fp);                              /* test next char */
      if(nxt == EOL_LF)                                        /* DOS */
	eolStr[i++] = EOL_LF;
      else if(nxt != EOF)
	ungetc(nxt, fp);                    /* MAC or OS-9: push back */
      break;
    }
/*     if(chr == 0x0C || chr == 0x1A) */
/*      continue;                                 skip FF and <Ctrl>Z */
    if(cnt >= (size - 1)) {                            /* buffer full */
      ungetc(chr, fp);                      /* push the last one back */
      eolStr[0] = EOF;                            /* signal to caller */
      break;
    }
    buffer[cnt++] = chr;
  }
  buffer[cnt] = '\0';                                 /* close string */
  if(chr == EOF && cnt == 0)      /* attempt to read past End-Of-File */
    return(EOF);
  return(cnt);
}

