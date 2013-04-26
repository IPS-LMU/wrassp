/***********************************************************************
*                                                                      *
* This file is part of Michel Scheffers's miscellanous functions       *
* library.                                                             *
*                                                                      *
* Copyright (C) 1989 - 2010  Michel Scheffers                          *
*                            IPdS, CAU Kiel                            *
*                            Leibnizstr. 10                            *
*                            24118 Kiel, Germany                       *
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
* File:     trace.c                                                    *
* Contents: Global variables and functions implementing the standard   *
*           trace/debug handler.                                       *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: trace.c,v 1.4 2010/01/29 08:50:41 mtms Exp $ */

#include <stdio.h>  /* fopen() fclose() fprintf() stderr NULL */
#include <string.h> /* strlen() */
#include <time.h>   /* time() localtime() strftime() */

#include <trace.h>  /* TRACE_VAR_LEN TRACE_NAM_LEN */
#include <misc.h>   /* prototypes */

/*
 * global variables
 */
char  TRACE[TRACE_VAR_LEN]={'\0'};
char  traceFile[TRACE_NAM_LEN+1]={'t','r','a','c','e','\0'};
char *traceOpts=NULL;
FILE *traceFP=NULL;
/*
 * Note:
 *  o Trace options 'F', 'f' and 'A' have thusfar been reserved.
 *  o Check one of the command line interfaces to see how to work with 
 *    the trace handler.
 */

/***********************************************************************
* open file for trace output                                           *
***********************************************************************/
void openTrace(char *id)
{
  char   date[64];
  time_t current;

  if((TRACE['F'] || TRACE['f']) && strlen(traceFile) > 0) {
    if(TRACE['F']) {   /* open in append mode */
      traceFP = fopen(traceFile, "a+");
      if(traceFP != NULL)
	fseek(traceFP, 0L, SEEK_END);
    }
    else               /* create/truncate */
      traceFP = fopen(traceFile, "w+");
    if(traceFP == NULL) {
      perror("Could not open trace file");
#ifndef WRASSP
      traceFP = stderr;
#endif
    }
    else { 
      if(id != NULL) { /* head new block with identification */
	if(strlen(id) > 0)
	  fprintf(traceFP, "TRACE_ID: %s\n", id);
      }
      time(&current);  /* always record date and time */
      strftime(date, 64, "%a %d.%m.%Y %H:%M:%S", localtime(&current));
      fprintf(traceFP, "Date: %s\n", date);
      if(traceOpts != NULL) {
	if(strlen(traceOpts) > 0)
	  fprintf(traceFP, "Trace options: %s\n", traceOpts);
      }
    }
  }
  else
#ifdef WRASSP
     traceFP = NULL;
#else
    traceFP = stderr; /* default output */
#endif
  return;
}
/***********************************************************************
* close file for trace output if not stderr                            *
***********************************************************************/
void closeTrace(void)
{
#ifndef WRASSP
  if(traceFP != NULL && traceFP != stderr && traceFP != stdout)
#else
  if (traceFP != NULL)
#endif
    fclose(traceFP);
  traceFP = NULL;
  return;
}

