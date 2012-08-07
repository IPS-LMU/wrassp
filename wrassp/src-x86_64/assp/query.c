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
* File:     query.c                                                    *
* Contents: Functions for interactive requests to user, including      *
*           verification.                                              *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: query.c,v 1.2 2007/10/04 11:35:50 mtms Exp $ */

#include <stdio.h>
#include <errno.h>    /* for perror() */
#include <stdlib.h>   /* strtol() strtod() */
#include <ctype.h>
#include <string.h>
#include <unistd.h>   /* access() */

#include <misc.h>     /* QFLAG_xx prototypes */


/*
 * "allow" is a string of the form "aBc" whereby the capital letter 
 * defines the default value. The 'reply' character returned will 
 * always be in lower case.
 */
int queryChar(char *quest, char *allow, long flags, char *reply)
{
  char  dflt;
  int   DONE, n, num, response, temp;

  if(!quest || !allow || !reply)
    return(-2);
  *reply = dflt = '\0';
  num = strlen(allow);
  if(num < 1)
    return(-2);
  for(n = 0; n < num; n++) {
    if(isupper((int)allow[n])) {
      dflt = (char)tolower((int)allow[n]);
      break;
    }
  }

  printf("%s (%c", quest, (char)tolower((int)allow[0]));
  for(n = 1; n < num; n++)
    printf("/%c", (char)tolower((int)allow[n]));
  if(dflt)
    printf(")[%c]: ", dflt);
  else
    printf("): ");
  fflush(stdout);

  DONE = FALSE;
  while(!DONE) {
    response = getchar();
    if(response == EOF) {                  /* handle exceptions first */
      return(-1);
    }
    temp = response;
    while(temp != '\n') /* haven't found a way yet to get one char !! */
      temp = getchar();
    if(flags & QFLAG_EVAL) {                              /* evaluate */
      if(dflt && response == '\n') {
	*reply = dflt;            /* must be one of the allowed chars */
	return(0);
      }
      response = tolower(response);          /* compare in lower case */
      for(n = 0; n < num; n++) {
	if(response == tolower((int)allow[n])) {
	  DONE = TRUE;
	  break;
	}
      }
      if(!DONE) {
	dflt = '\0';
	printf("Please answer %c", (char)tolower(allow[0]));
	for(n = 1; n < num; n++) {
	  if(n == (num-1))
	    printf(" or %c", (char)tolower(allow[n]));
	  else
	    printf(", %c", (char)tolower(allow[n]));
	}
	printf(" (<Ctrl>c to stop): ");
	fflush(stdout);
      }
    }
    else
      DONE = TRUE;
  }
  *reply = (char)response;
  return(0);
}

int queryFile(char *quest, char *dflt, long flags,\
	      char *reply, int length)
{
  int n, mode, fval;

  if(!reply || length < 2)
    return(-2);
  strcpy(reply, "");
  if(dflt && length <= strlen(dflt))
    return(-2);
  if(dflt) {
    if(!strlen(dflt))
      dflt = NULL;
  }

  printf("%s", quest);
  if(dflt)
    printf(" [%s]", dflt);
  else
    printf(" (<Ctrl>c to stop)");
  printf(": ");
  fflush(stdout);

  fval = 0;                                         /* function value */
  n = fgetl(reply, length, stdin, NULL);
  if(n < 0)
    return(-1);
  if(n == 0) {
    if(dflt)
      strcpy(reply, dflt);
    else
      return(queryFile(quest, NULL, flags, reply, length));
  }
  if(flags & QFLAG_EVAL) {                                /* evaluate */
    if(flags & QFLAG_FNEW || flags & QFLAG_FWRITE) {
      mode = F_OK;
      if(access(reply, mode) == 0) {                   /* file exists */
	if(!(flags & QFLAG_FWRITE)) {
	  perror(reply);                     /* recursive interaction */
	  fval = queryFile(quest, dflt, flags, reply, length);
	}
	else {
	  mode = W_OK;
	  if(flags & QFLAG_FREAD)
	    mode |= R_OK;
	  if(access(reply, mode) != 0) {
	    perror(reply);
	    fval = queryFile(quest, dflt, flags, reply, length);
	  }
	}
      }
    }
    if(flags & QFLAG_FOLD || flags & QFLAG_FREAD) {
      mode = F_OK;
      if(flags & QFLAG_FREAD)
	mode |= R_OK;
      if(flags & QFLAG_FWRITE)
	mode |= W_OK;
      if(access(reply, mode) != 0) {
	perror(reply);
	fval = queryFile(quest, dflt, flags, reply, length);
      }
    }
  }
  return(fval);
}

int queryLong(char *quest, long *dflt, long flags,\
	      long *reply, long min, long max)
{
  char buf[16];
  int  n, fval;

  if(!reply)
    return(-2);
  *reply = -1;                                 /* have to set a value */

  printf("%s", quest);
  if(dflt)
    printf(" [%ld]", *dflt);
  else
    printf(" (<Ctrl>c to stop)");
  printf(": ");
  fflush(stdout);

  n = fgetl(buf, 16, stdin, NULL);
  if(n < 0)
    return(-1);
  if(n == 0) {
    if(dflt) {                 /* should already have been verified */
      *reply = *dflt;
      return(0);
    }
    return(queryLong(quest, NULL, flags, reply, min, max));
  }
  else /* n > 0 */
    *reply = strtol(buf, NULL, 10);
  fval = 0;                                         /* function value */
  if(flags & QFLAG_EVAL) {                                /* evaluate */
    if(flags & QFLAG_VMIN) {
      if(*reply < min) {                     /* recursive interaction */
	printf("ERROR: number too small (minimally %ld)\n", min);
	fval = queryLong(quest, NULL, flags, reply, min, max);
      }
    }
    if(flags & QFLAG_VMAX) {
      if(*reply > max) {
	printf("ERROR: number too large (maximally %ld)\n", max);
	fval = queryLong(quest, NULL, flags, reply, min, max);
      }
    }
  }
  return(fval);
}

int queryDouble(char *quest, double *dflt, long flags,\
		double *reply, double min, double max)
{
  char buf[32];
  int  n, fval;

  if(!reply)
    return(-2);
  *reply = -1;                                 /* have to set a value */

  printf("%s", quest);
  if(dflt)
    printf(" [%f]", *dflt);
  else
    printf(" (<Ctrl>c to stop)");
  printf(": ");
  fflush(stdout);

  n = fgetl(buf, 32, stdin, NULL);
  if(n < 0)
    return(-1);
  if(n == 0) {
    if(dflt) {                   /* should already have been verified */
      *reply = *dflt;
      return(0);
    }
    return(queryDouble(quest, NULL, flags, reply, min, max));
  }
  else /* n > 0 */
    *reply = strtod(buf, NULL);
  fval = 0;                                         /* function value */
  if(flags & QFLAG_EVAL) {                                /* evaluate */
    if(flags & QFLAG_VMIN) {
      if(*reply < min) {                     /* recursive interaction */
	printf("ERROR: number too small (minimally %f)\n", min);
	fval = queryDouble(quest, NULL, flags, reply, min, max);
      }
    }
    if(flags & QFLAG_VMAX) {
      if(*reply > max) {
	printf("ERROR: number too large (maximally %f)\n", max);
	fval = queryDouble(quest, NULL, flags, reply, min, max);
      }
    }
  }
  return(fval);
}

