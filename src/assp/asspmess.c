/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
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
*                                                                      *
* File:     asspmess.c                                                 *
* Contents: Functions implementing the ASSP message handler.           *
* Author:   Michel T.M. Scheffers                                      *
* Remarks:  For programs using a GUI, the function prtAsspMsg() may    *
*           serve as an example for constructing the message box.      *
*                                                                      *
***********************************************************************/
/* $Id: asspmess.c,v 1.12 2010/07/09 13:35:05 mtms Exp $ */

#include <errno.h>      /* errno */
#include <stdio.h>      /* FILE NULL stderr fprintf() */
#include <stddef.h>     /* size_t */
#include <string.h>     /* str...() */

#include <asspmess.h>   /* function prototypes, constants ... */
#include <trace.h>      /* TRACE[] traceFP */

/*
 * global variables
 */
short asspMsgNum = 0;
char  applMessage[MAX_MSG_LEN + 1] = {'\0'};
AMREC asspMessage[] = {
/* warnings */
  { AWG_WARN_BUG, "Programming error (please report)" },
  { AWG_WARN_MEM, "Out of memory" },
  { AWG_ERR_ROUND,"Rounding error" },
  { AWA_WARN_DEV, "No audio device available" },
  { AWA_WARN_SFR, "Audio device can't handle sample rate" },
  { AWD_NO_DATA,  "No data available" },
  { AWD_NO_AUDIO, "No audio signal" },
  { AWF_BAD_ITEM, "Bad header item" },
  { AWF_RAW_FORM, "Unknown file format; using RAW settings" },
  { AWF_EMPTY,    "Empty file" },
/* errors */
  { AEG_ERR_BUG,  "Programming error (please report)" },
  { AEG_ERR_MEM,  "Out of memory" },
  { AEG_ERR_ROUND,"Rounding error" },
  { AEG_NOT_YET,  "Not yet implemented" },
  { AEG_ERR_FATAL,"FATAL ERROR" },
  { AEA_ERR_DEV,  "No such audio device" },
  { AEA_ERR_OPEN, "Can't open audio device" },
  { AEA_ERR_BUSY, "Audio device busy" },
  { AEA_ERR_INIT, "Can't initialize audio device" },
  { AEA_ERR_BUF,  "Can't get audio buffer" },
  { AEA_ERR_FORM, "Audio device can't handle data format" },
  { AEA_ERR_RATE, "Audio device can't handle sample rate" },
  { AEA_ERR_CHAN, "Too many channels for audio device" },
  { AEA_ERR_PUT,  "Error in writing to audio device" },
  { AEA_ERR_GET,  "Error in reading from audio device" },
  { AEB_BAD_ARGS, "Invalid argument(s) in call to function" },
  { AEB_BAD_CALL, "Inappropriate call to function" },
  { AEB_TOO_SOON, "Attempt to access data before Begin-Of-File" },
  { AEB_TOO_LATE, "Attempt to access data behind End-Of-File" },
  { AEB_BUF_RANGE,"Attempt to access data not in buffer" },
  { AEB_BUF_SPACE,"Insufficient space in buffer" },
  { AEB_BAD_WIN,  "Unknown or invalid window function" },
  { AEB_ERR_EMPH, "Invalid preemphasis" },
  { AEB_ERR_GAIN, "Invalid gain factor" },
  { AEB_ERR_TRACK,"No track name available" },
  { AEC_BAD_OPT,  "Unknown option" },
  { AEC_BAD_VAL,  "Option value missing or invalid" },
  { AEC_ARG_MISS, "Argument(s) missing" },
  { AEC_ARG_MANY, "Too many arguments" },
  { AEC_IO_CLASH, "Output would overwrite input file" },
  { AED_NO_DATA,  "No data available" },
  { AED_BAD_TYPE, "Unknown data type" },
  { AED_BAD_FORM, "Unknown data format" },
  { AED_NOHANDLE, "Can't handle data format" },
  { AED_ERR_TYPE, "Incorrect data type" },
  { AED_ERR_FORM, "Incorrect data format" },
  { AED_ERR_RATE, "Incorrect data rate" },
  { AED_ERR_RANGE,"Empty or invalid data range" },
  { AED_ERR_SIZE, "Window size undefined or too small" },
  { AED_INCOMPAT, "Incompatible with existing data" },
  { AED_NO_AUDIO, "No audio signal" },
  { AEE_BAD_ERR,  "Unknown error code" },
  { AEF_NOT_OPEN, "File not open" },
  { AEF_MISSING,  "File does not exist" },
  { AEF_EXISTS,   "File already exists" },
  { AEF_EMPTY,    "Empty file" },
  { AEF_ERR_OPEN, "Can't open file" },
  { AEF_ERR_SEEK, "Can't seek in file" },
  { AEF_ERR_READ, "Can't read file" },
  { AEF_ERR_WRIT, "Can't write file" },
  { AEF_ERR_MOVE, "Can't rename file" },
  { AEF_ERR_COPY, "Can't copy file" },
  { AEF_ERR_EOF,  "Attempt to access data past end of file" },
  { AEF_BAD_FORM, "Unknown file format" },
  { AEF_BAD_HEAD, "Corrupted file header" },
  { AEF_ERR_FORM, "Incorrect file format" },
  { 0, NULL }         /* mark end of list */
};

/***********************************************************************
* Clears all warning/error variables.                                  *
***********************************************************************/
void clrAsspMsg(void)
{
  errno = 0;
  asspMsgNum = 0;
  applMessage[0] = '\0';
  return;
}

/***********************************************************************
* Returns pointer to string with message corresponding to code "num".  *
***********************************************************************/
char *getAsspMsg(short num)
{
  size_t i;

  if(num == 0)
    return(NULL);
  if(num == AWG_WARN_SYS || num == AEG_ERR_SYS) {
    if(errno == 0)
      return(NULL);
    else
      return(strerror(errno));
  }
  if(num == AWG_WARN_APPL || num == AEG_ERR_APPL)
    return(applMessage);
  for(i = 0; asspMessage[i].num != 0; i++) {        /* search message */
    if(asspMessage[i].num == num)
      return(asspMessage[i].str);
  }
/* not found in list */
  snprintf(applMessage, sizeof(applMessage), "\n%s: %04x", getAsspMsg(AEE_BAD_ERR), num);
  return(getAsspMsg(AWG_WARN_BUG));
}

/***********************************************************************
* Prints message corresponding to 'asspMsgNum' to the stream pointed   *
* at by "fp". Appends 'applMessage' and/or system error message when   *
* appropriate. If "fp" equals NULL, prints to 'stderr'.                *
* When bug tracing is enabled and trace output is directed to a file,  *
* the message will also be printed to that file.                       *
* When output is directed to 'stdout' or 'stderr', message variables   *
* will be cleared, otherwise they will NOT.                            *
* Returns 0 when no message; 1 when warning, -1 when error message.    *
***********************************************************************/
int prtAsspMsg(FILE *fp)
{
  char *msg, *bug, indent[16];
  int   funcVal;

#ifndef WRASSP
  if(fp == NULL) fp = stderr;
  if((TRACE['F'] || TRACE['f']) && traceFP != NULL &&
     traceFP != stderr && traceFP != stdout) { /* print to trace file */
    if(fp != traceFP)                          /* avoid endless loops */
      prtAsspMsg(traceFP);        /* will NOT clear message variables */
  }
  bug = NULL;
  funcVal = 0;
  msg = getAsspMsg(asspMsgNum);
  if(msg != NULL) {
    if(asspBug)                            /* get the general message */
      bug = getAsspMsg(AEG_ERR_BUG);
    if(asspWarning) {
      funcVal = 1;
      snprintf(indent, sizeof(indent), "%9s", " ");
      fprintf(fp, "WARNING: %s", msg);
    }
    else {
      funcVal = -1;
      snprintf(indent, sizeof(indent), "%7s", " ");
      if(bug != NULL) {
	fprintf(fp, "ERROR: %s\n", bug);
	fprintf(fp, "%s%s", indent, msg);         /* the real message */
      }
      else
	fprintf(fp, "ERROR: %s", msg);
    }
    fflush(fp);
    if(asspMsgNum != AWG_WARN_APPL && asspMsgNum != AEG_ERR_APPL &&
       strlen(applMessage) != 0) {          /* append additional info */
      msg = applMessage;
      if(*msg != ' ') {
	if(*msg == '\n') {
	  fprintf(fp, "\n");
	  msg++;
	  if(*msg != ' ')                         /* make nice indent */
	    fprintf(fp, "%s", indent);
	}
	else
	  fprintf(fp, " ");
	fflush(fp);
      }
      fprintf(fp, "%s", msg);
      fflush(fp);
    }
    if(errno != 0 && !(asspMsgNum == AWG_WARN_SYS ||
		       asspMsgNum == AEG_ERR_SYS)) {
      /* append system error message */
      if(strlen(applMessage) != 0)
	fprintf(fp, "\n%s(%s)", indent, strerror(errno));
      else
	fprintf(fp, " (%s)", strerror(errno));
      fflush(fp);
    }
    fprintf(fp, "\n");
  }
  if(fp == stderr || fp == stdout)
    clrAsspMsg();                    /* clear message/error variables */
  return(funcVal);
#else
  return(0);
#endif
}

/***********************************************************************
* Sets asspMsgNum to "num" and copies "txt" to applMessage. If "num"   *
* equals zero, applMessage will also be cleared but errno NOT.         *
* Returns 0 if successful or -1 if string "txt' is too long to fit in  *
* applMessage (fitting part will be copied though).                    *
***********************************************************************/
int setAsspMsg(short num, char *txt)
{
  size_t l;

  if(num == 0) {
    asspMsgNum = 0;
    applMessage[0] = '\0';
    return(0);
  }
  asspMsgNum = num;
  if(txt == NULL) {
    applMessage[0] = '\0';
    return(0);
  }
  l = strlen(txt);
  if(l > MAX_MSG_LEN) {
    strncpy(applMessage, txt, MAX_MSG_LEN);
    applMessage[MAX_MSG_LEN] = '\0';                  /* close string */
    return(-1);                          /* indicate string shortened */
  }
  strcpy(applMessage, txt);
  return(0);
}
