/***********************************************************************
*                                                                      *
* This file is part of the Advanced Speech Signal Processor library.   *
*                                                                      *
* Copyright (C) 1989 - 2010  Michel Scheffers                          *
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
* File:     labelobj.c                                                 *
* Contents: Basic functions for handling label objects.                *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: labelobj.c,v 1.9 2010/05/28 07:29:30 mtms Exp $ */

#include <stdio.h>    /* NULL */
#include <stddef.h>   /* size_t */
#include <stdlib.h>   /* malloc() calloc() stttod() */
#include <string.h>   /* str...() */
#include <ctype.h>    /* isdigit() */

#include <misc.h>     /* ONEkBYTE myrint() LINK link functions */
#include <asspmess.h> /* message codes; reference to globals */
#include <assptime.h> /* standard conversion macros */
#include <dataobj.h>  /* DOBJ DDESC and prototypes */
#include <labelobj.h> /* LBLHDR GD_LBL_IDENT IPdS & ESPS formats */

/*DOC

Function 'freeLabel'

Returns all memory allocated for the label link pointed to by "ptr". 
This function is used in the deleteLink() and deleteChain() functions 
for double-linked lists hence the use of pointer type 'void'.

DOC*/

void freeLabel(void *ptr)
{
  LABEL *lPtr;

  lPtr = (LABEL *)ptr;
  if(lPtr != NULL) {
    if(lPtr->name != NULL)
      free((void *)(lPtr->name));
    free(ptr);
  }
  return;
}

/*DOC

Function 'freeLabelList'

Returns all memory allocated for a double-linked list of labels pointed 
to by "ptr". This function should be set as the 'doFreeDataBuf' function 
in a label object so that the functions 'freeDataBuf' and 'asspFClose' 
can be used as with the other data objects.

DOC*/

void freeLabelList(void *ptr)
{
  LINK **temp=NULL;

  if(ptr != NULL) {
    *temp = (LINK *)ptr;
    deleteChain(temp, (freeLinkFunc)freeLabel);
  }
  return;
}

/*DOC
 
Function 'getLabelHead'

Copies the fixed header part (orthography and citation form) of a label 
file in IPdS MIX or SAMPA format including the end-of-citation line 
('CT 1' and 'kend' respectively) into a generic data structure. 
This structure is then stored in the label object pointed to by "dop".

DOC*/

int getLabelHead(DOBJ *dop)
{
  char    buf[ONEkBYTE], *eolCode;
  int     n, OK;
  long    offset, headSize;
  LBLHDR *ptr=NULL;

  clrAsspMsg();
  if(!(dop->fileFormat == FF_IPDS_M || dop->fileFormat == FF_IPDS_S) ) {
    asspMsgNum = AEG_ERR_BUG;
    snprintf(applMessage, sizeof(applMessage), "File %s is not in MIX or SAMPA format",\
	    dop->filePath);
    return(-1);
  }
  if(dop->generic != NULL) {
    asspMsgNum = AEG_ERR_BUG;
    snprintf(applMessage, sizeof(applMessage), "Data object for file %s\n"\
	    " already contains generic data", dop->filePath);
    return(-1);
  }
  rewind(dop->fp);
  headSize = offset = 0;
  if(dop->fileFormat == FF_IPDS_S) {
    n = fgetl(buf, sizeof(buf), dop->fp, NULL);/* line with file name */
    offset = ftell(dop->fp);              /* NOT included in copy !!! */
  }
  OK = FALSE;
  do {                               /* seek end of fixed header part */
    n = fgetl(buf, sizeof(buf), dop->fp, &eolCode);
    headSize = ftell(dop->fp);            /* keep track of bytes read */
    if(n > 0) {
      if( (dop->fileFormat == FF_IPDS_M &&\
	   strncmp(buf, MIX_TRF_ID, strlen(MIX_TRF_ID)) == 0) ||
	  (dop->fileFormat == FF_IPDS_S && strcmp(buf, SAM_EOC_ID) == 0) ) {
	OK = TRUE;
	break;
      }
    }
  } while(n >= 0);
  if(!OK || n < 0) {
    asspMsgNum = AEF_BAD_HEAD;
    snprintf(applMessage, sizeof(applMessage), "(IPdS-%s format)\nin file %s",
	    (dop->fileFormat == FF_IPDS_M) ? "MIX": "SAM", dop->filePath);
    return(-1);
  }

  if((dop->generic=malloc(sizeof(LBLHDR))) != NULL) {
    ptr = (LBLHDR *)(dop->generic);
    strcpy(ptr->ident, LBL_GD_IDENT);
    headSize -= offset;
    ptr->headCopy = malloc((size_t)headSize);
    if(ptr->headCopy == NULL) {
      free(dop->generic);
      dop->generic = NULL;
    }
  }
  if(dop->generic == NULL) {
    asspMsgNum = AEG_ERR_MEM;
    snprintf(applMessage, sizeof(applMessage), "\n(can't copy header of file %s)",\
	    dop->filePath);
    return(-1);
  }
  fseek(dop->fp, offset, SEEK_SET);
  if(fread(ptr->headCopy, sizeof(char), (size_t)headSize, dop->fp) != (size_t)headSize){
    setAsspMsg(AEF_ERR_READ, dop->filePath);
    return(-1);
  }
  ptr->copySize = headSize;
  dop->doFreeGeneric = (DOfreeFunc)freeHeadCopy;
  if(strlen(dop->eol) == 0) {
    if(strlen(eolCode) > 0 && eolCode[0] != EOF)
      strcpy(dop->eol, eolCode);
    else if(dop->fileFormat == FF_IPDS_M)
      strcpy(dop->eol, MIX_EOL_STR);
    else
      strcpy(dop->eol, SAM_EOL_STR);
  }
  return(0);
}

/*DOC

Function 'freeHeadCopy'

Frees the memory allocated for a copy of an IPdS label header.

DOC*/

void freeHeadCopy(void *generic)
{
  LBLHDR *ptr;

  if(generic != NULL) {
    ptr = (LBLHDR *)generic;
    if(ptr->headCopy != NULL) 
      free(ptr->headCopy);
    free(generic);
  }
  return;
}

/*DOC

Function 'freeXLBL_GD'

Frees the memory allocated for the generic data structure of an 
ESPS xlabel file.

DOC*/

void freeXLBL_GD(void *generic)
{
  XLBL_GD *ptr;

  if(generic != NULL) {
    ptr = (XLBL_GD *)generic;
    if(ptr->signal != NULL) 
      free((void *)ptr->signal);
    if(ptr->font != NULL) 
      free((void *)ptr->font);
    free(generic);
  }
  return;
}

/*DOC

Function 'loadLabels'

Reads all labels from the file referred to by the data object pointed to 
by "dop" and stores them in a double-linked 'LABEL' list in the data 
buffer of that object. If the data buffer is not empty its contents will 
be destroyed.

Note:
 - The item 'headerSize' in "dop" is assumed to be correctly set to 
   the offset of the first label line in the file.
 - At present, this function can only handle label files in IPdS-MIX, 
   IPdS-SAMPA and ESPS-xlabel format.

DOC*/

long loadLabels(DOBJ *dop)
{
  char     lineBuf[ONEkBYTE];
  char    *eolCode, *name, *field[MIX_MAX_FIELDS+1];
  int      n, pos, color=-1;
  long     smpNr;
  double   time;
  LABEL   *lPtr;
  XLBL_GD *gd=NULL;

  if(dop == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "loadLabels");
    return(-1);
  }
  if(dop->fp == NULL || dop->recordSize > 0 || dop->fileData != FDF_ASC ||\
     (dop->fileFormat != FF_IPDS_M && dop->fileFormat != FF_IPDS_S &&\
      dop->fileFormat != FF_XLABEL)) {
    setAsspMsg(AEB_BAD_CALL, "loadLabels");
    return(-1);
  }
  if(dop->dataBuffer != NULL) {
    if(dop->doFreeDataBuf == NULL) {              /* apparently fixed */
      setAsspMsg(AEG_ERR_BUG, "loadLabels: can't free data buffer");
      return(-1);
    }
    freeDataBuf(dop);
  }
  dop->startRecord = dop->numRecords = 0;
  dop->Time_Zero = dop->Start_Time = 0.0;
  dop->bufStartRec = dop->bufNumRecs = dop->maxBufRecs = 0;
  dop->doFreeDataBuf = (DOfreeFunc)freeLabelList;
  if(fseek(dop->fp, dop->headerSize, SEEK_SET) < 0) {
    setAsspMsg(AEF_ERR_SEEK, dop->filePath);
    return(-1);
  }
  while((n=fgetl(lineBuf, ONEkBYTE, dop->fp, &eolCode)) >= 0) {
    if(n > 0) {                                   /* skip empty lines */
      if(dop->fileFormat == FF_IPDS_M) {
	if(strncmp(lineBuf, MIX_LBL_ID, strlen(MIX_LBL_ID)) != 0)
	  continue;                               /* not a label line */
	n = strparse(lineBuf, NULL, field, MIX_MAX_FIELDS+1);
	if(n < 3)
	  continue;                               /* not a label line */
	smpNr = strtol(field[1], NULL, 10) - 1;  /* count starts at 1 */
	name = field[2];
	if(n > 4)
	  time = strtod(field[4], NULL);
	else
	  time = LBL_TIME_UNDEF;
      }
      else if(dop->fileFormat == FF_IPDS_S) {
	n = strparse(lineBuf, NULL, field, MIX_MAX_FIELDS+1);
	if(n < 2 || !isdigit((int)field[0][0]))
	  continue;                               /* not a label line */
	smpNr = strtol(field[0], NULL, 10) - 1;  /* count starts at 1 */
	name = field[1];
	if(n > 2)
	  time = strtod(field[2], NULL);
	else
	  time = LBL_TIME_UNDEF;
      }
      else { /* FF_XLABEL */
	n = strparse(lineBuf, NULL, field, MIX_MAX_FIELDS+1);
	if(n < 3 || !(isdigit((int)field[0][0]) || field[0][0] == '.'))
	  continue;                               /* not a label line */
	smpNr = LBL_TIME_UNDEF;
	time = strtod(field[0], NULL);
	color = (int)strtol(field[1], NULL, 10);
	name = field[2];
      }
      if((lPtr=makeLabel(name, smpNr, time)) == NULL) {
	freeDataBuf(dop);
	return(-1);
      }
      pos = LBL_ADD_AS_LAST;
      if(dop->fileFormat == FF_XLABEL) {
	/* ESPS labels need not be in chronological order */
	pos |= LBL_ADD_AT_TIME;
      }
      if(addLabel(dop, lPtr, pos, NULL) == NULL) {
	freeLabel((void *)lPtr);
	freeDataBuf(dop);
	return(-1);
      }
    }
  }
  if(dop->fileFormat == FF_XLABEL) {
    gd = (XLBL_GD *)(dop->generic);
    if(gd != NULL) {
      if(color < 0)
	color = XLBL_DEF_COLOR;
      gd->color = color;
    }
  }
  dop->numRecords = dop->bufNumRecs;             /* set in addLabel() */
  dop->bufNeedsSave = FALSE;
  return(dop->numRecords);
}

/*DOC

Function 'saveLabels'

Writes the labels stored in a double-linked 'LABEL' list in the data 
buffer in the label object pointed to by "dop" to the file referred to 
in that object. The file must have been opened in write or update mode 
and the (full) header should be present. If writing was successful, 
the item 'numRecords' will be set to the number of labels written and 
the flag 'bufNeedsSave' will be cleared. 
This function returns the number of labels written or -1 upon error.

Note:
 - The item 'headerSize' in "dop" is assumed to be correctly set to 
   the offset of the first label line in the file. Writing will start 
   from this position.
 - If this function is used to update a label file, the file may not 
   contain lines past the header. Preferably make a backup of the file, 
   (re-)open it in write/truncate mode, copy the header from the backup 
   into the file (possibly extend it with the variant field) and set 
   'headerSize' to the correct value.
 - At present, this function can only handle label files in IPdS-MIX, 
   IPdS-SAMPA and ESPS-xlabel format.

DOC*/

long saveLabels(DOBJ *dop)
{
  char     lineBuf[ONEkBYTE];
  int      nd, color;
  long     numWrite;
  LABEL   *lPtr;
  XLBL_GD *gd;

  if(dop == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "saveLabels");
    return(-1);
  }
  if(dop->fp == NULL || dop->recordSize > 0 || dop->fileData != FDF_ASC ||\
     (dop->fileFormat != FF_IPDS_M && dop->fileFormat != FF_IPDS_S &&\
      dop->fileFormat != FF_XLABEL)) {
    setAsspMsg(AEB_BAD_CALL, "saveLabels");
    return(-1);
  }

  if(dop->dataBuffer == NULL) {                    /* got nothing to do */
    dop->numRecords = dop->bufNumRecs = 0;
    dop->bufNeedsSave = FALSE;
    return(0);
  }
  if(fseek(dop->fp, dop->headerSize, SEEK_SET) < 0) {
    setAsspMsg(AEF_ERR_SEEK, dop->filePath);
    return(-1);
  }
  if(dop->fileFormat == FF_XLABEL) {
    gd = (XLBL_GD *)(dop->generic);
    if(gd != NULL)
      color = gd->color;
    else
      color = XLBL_DEF_COLOR;
  }
  else
    color = 0;
  if(dop->sampFreq > 0.0)
    nd = numDecim(1.0 / (dop->sampFreq), 9);
  else
    nd = 8; /* should be ample */
  if(strlen(dop->eol) == 0)
    strcpy(dop->eol, "\x0A");
  numWrite = 0;
  lPtr = (LABEL *)(dop->dataBuffer);
  while(lPtr != NULL) {
    if(dop->fileFormat == FF_IPDS_M) {
      if(lPtr->smpNr < 0 || lPtr->time < 0.0) {
	if(dop->sampFreq <= 0.0) {
	  setAsspMsg(AEG_ERR_BUG, "saveLabels: sample rate unknown");
	  return(-1);
	}
	if(lPtr->smpNr < 0 && lPtr->time < 0.0) {
	  setAsspMsg(AEG_ERR_BUG, "saveLabels: label time unknown");
	  return(-1);
	}
	if(lPtr->smpNr < 0)
	  lPtr->smpNr = TIMEtoSMPNR(lPtr->time, dop->sampFreq);
	else
	  lPtr->time = SMPNRtoTIME(lPtr->smpNr, dop->sampFreq);
      }
      snprintf(lineBuf, sizeof(lineBuf), MIX_LBL_STR_AP, lPtr->smpNr+1, lPtr->name,\
	      (long)floor((lPtr->time)*100.0)+1, nd, lPtr->time);
    }
    else if(dop->fileFormat == FF_IPDS_S) {
      if(lPtr->smpNr < 0 || lPtr->time < 0.0) {
	if(dop->sampFreq <= 0.0) {
	  setAsspMsg(AEG_ERR_BUG, "saveLabels: sample rate unknown");
	  return(-1);
	}
	if(lPtr->smpNr < 0 && lPtr->time < 0.0) {
	  setAsspMsg(AEG_ERR_BUG, "saveLabels: label time unknown");
	  return(-1);
	}
	if(lPtr->smpNr < 0)
	  lPtr->smpNr = TIMEtoSMPNR(lPtr->time, dop->sampFreq);
	else
	  lPtr->time = SMPNRtoTIME(lPtr->smpNr, dop->sampFreq);
      }
      snprintf(lineBuf, sizeof(lineBuf), SAM_LBL_STR_AP, lPtr->smpNr+1, lPtr->name,\
	      nd, lPtr->time);
    }
    else { /* FF_XLABEL */
      if(lPtr->time < 0.0) {
	if(dop->sampFreq <= 0.0) {
	  setAsspMsg(AEG_ERR_BUG, "saveLabels: sample rate unknown");
	  return(-1);
	}
	if(lPtr->smpNr < 0) {
	  setAsspMsg(AEG_ERR_BUG, "saveLabels: label time unknown");
	  return(-1);
	}
	lPtr->time = SMPNRtoTIME(lPtr->smpNr, dop->sampFreq);
      }
      snprintf(lineBuf, sizeof(lineBuf), XLBL_STR_AP, nd, lPtr->time, color, lPtr->name);
    }
    strcat(lineBuf, dop->eol);
    if(fwrite((void *)lineBuf, sizeof(char), strlen(lineBuf), dop->fp) < 1) {
      setAsspMsg(AEF_ERR_WRIT, "(saveLabels)");
      return(-1);
    }
    numWrite++;
    lPtr = nextLabel(lPtr);
  }
  dop->numRecords = dop->bufNumRecs = numWrite;
  dop->bufNeedsSave = FALSE;
  return(numWrite);
}
/*DOC

Function 'makeLabel'

Allocates memory for a 'LABEL' structure and sets its members. Either 
"smpNr" or "time" may be < 0 (e.g. 'LBL_TIME_UNDEF') but not both.
Returns a pointer to the new structure or NULL upon error.

DOC*/

LABEL *makeLabel(char *name, long smpNr, double time)
{
  LABEL *new;

  if(smpNr < 0 && time < 0.0) {
    setAsspMsg(AEB_BAD_ARGS, "makeLabel");
    return(NULL);
  }
  new = (LABEL *)calloc(1, sizeof(LABEL));
  if(new == NULL) {
    setAsspMsg(AEG_ERR_MEM, "(makeLabel)");
    return(NULL);
  }
  if(name != NULL) {
    new->name = strdup(name);
    if(new->name == NULL) {
      free((void *)new);
      setAsspMsg(AEG_ERR_MEM, "(makeLabel)");
      return(NULL);
    }
  }
  if(smpNr >= 0)
    new->smpNr = smpNr;
  else
    new->smpNr = LBL_TIME_UNDEF;
  if(time >= 0.0)
    new->time = time;
  else
    new->time = LBL_TIME_UNDEF;
  return(new);
}

/*DOC

Function 'firstLabel'

Returns a pointer to the first label of the list in the label object 
pointed to by "dop". Returns NULL if the list is empty.

DOC*/

LABEL *firstLabel(DOBJ *dop)
{
  LABEL *lPtr=NULL;

  if(dop != NULL) {
    if(dop->dataBuffer != NULL)
      lPtr = (LABEL *)(dop->dataBuffer);
  }
  return(lPtr);
}

/*DOC

Function 'lastLabel'

Returns a pointer to the last label of the list in the label object 
pointed to by "dop". Returns NULL if the list is empty.

DOC*/

LABEL *lastLabel(DOBJ *dop)
{
  LABEL *lPtr=NULL;

  if(dop != NULL) {
    if(dop->dataBuffer != NULL)
      lPtr = (LABEL *)lastLink((LINK *)(dop->dataBuffer));
  }
  return(lPtr);
}

/*DOC

Function 'prevLabel'

Returns a pointer to the label preceding the one pointed to by "lPtr".
Returns NULL if "lPtr" referred to the first label in the list.

DOC*/

LABEL *prevLabel(LABEL *lPtr)
{
  if(lPtr != NULL)
    lPtr = (LABEL *)(lPtr->prev);
  return(lPtr);
}

/*DOC

Function 'nextLabel'

Returns a pointer to the label following the one pointed to by "lPtr".
Returns NULL if "lPtr" referred to the last label in the list.

DOC*/

LABEL *nextLabel(LABEL *lPtr)
{
  if(lPtr != NULL)
    lPtr = (LABEL *)(lPtr->next);
  return(lPtr);
}

/*DOC

Function 'addLabel'

Adds the label pointed to by "new" to the list in the label object 
pointed to by "dop". "pos" indicates where the label should be 
inserted, it may be: 
LBL_ADD_AS_FIRST  Insert the label at the beginning of the list.
LBL_ADD_AS_LAST   Append the label to the list.
LBL_ADD_AT_TIME   Search the list for the last label whose time/smpNr 
                  is before that of "new" and insert "new" behind that 
		  label; when combined with 'LBL_ADD_AS_LAST' the list 
		  will be searched for the first label whose time/smpNr 
		  is behind that of "new" and "new" will be inserted 
		  before that label - this may make a difference if 
		  multiple labels are allowed at the same point in time.
LBL_ADD_BEFORE    Insert the label before the one pointed to by "ref"
LBL_ADD_BEHIND    Insert the label behind the one pointed to by "ref"
Returns the pointer "new" or NULL upon error.

Note:
 - It is considered to be an error to insert a label behind one that is 
   later or before one that is earlier in time.

DOC*/

LABEL *addLabel(DOBJ *dop, LABEL *new, int pos, LABEL *ref)
{
  int    USE_TIME=FALSE;   /* prefer using sample numbers if possible */
  LABEL *next;
  LINK  *head;

  clrAsspMsg();
  if(dop == NULL || new == NULL || pos <= 0) {
    setAsspMsg(AEB_BAD_ARGS, "addLabel");
    return(NULL);
  }
  new->prev = new->next = NULL;                  /* just to make sure */
  if(dop->dataBuffer == NULL)                    /* the simplest case */
    dop->dataBuffer = (void *)new;
  else {
    head = (LINK *)(dop->dataBuffer);
    if(new->smpNr < 0)
      USE_TIME = TRUE;
    if(pos == LBL_ADD_BEFORE || pos == LBL_ADD_BEHIND) {
      if(ref == NULL) {
	setAsspMsg(AEB_BAD_ARGS, "addLabel");
	return(NULL);
      }
      if((USE_TIME && ref->time < 0.0) ||
	 (!USE_TIME && ref->smpNr < 0)) {
	setAsspMsg(AEB_BAD_CALL, "addLabel: time/smpNr mismatch");
	return(NULL);
      }
      if(pos == LBL_ADD_BEFORE) {
	if((USE_TIME && new->time > ref->time) ||
	   (!USE_TIME && new->smpNr > ref->smpNr)) {
	  setAsspMsg(AEB_BAD_CALL, "addLabel: incorrect insertion point");
	  return(NULL);
	}
	new = (LABEL *)insLinkBefore(&head, (LINK *)ref, (LINK *)new);
      }
      else {
	if((USE_TIME && new->time < ref->time) ||
	   (!USE_TIME && new->smpNr < ref->smpNr)) {
	  setAsspMsg(AEB_BAD_CALL, "addLabel: incorrect insertion point");
	  return(NULL);
	}
	new = (LABEL *)insLinkBehind(&head, (LINK *)ref, (LINK *)new);
      }
    }
    else if(pos & LBL_ADD_AT_TIME) {
      next = (LABEL *)(dop->dataBuffer);
      while(next != NULL) {
	if((USE_TIME && next->time < 0.0) ||
	   (!USE_TIME && next->smpNr < 0)) {
	  setAsspMsg(AEG_ERR_BUG, "addLabel: time/smpNr mismatch");
	  return(NULL);
	}
	if((USE_TIME && next->time >= new->time) ||
	   (!USE_TIME && next->smpNr >= new->smpNr))
	  break;
	next = nextLabel(next);
      }
      if(next != NULL && pos & LBL_ADD_AS_LAST) {
	while(next != NULL) {
	  if((USE_TIME && next->time < 0.0) ||
	     (!USE_TIME && next->smpNr < 0)) {
	    setAsspMsg(AEG_ERR_BUG, "addLabel: time/smpNr mismatch");
	    return(NULL);
	  }
	  if((USE_TIME && next->time > new->time) ||
	     (!USE_TIME && next->smpNr > new->smpNr))
	    break;
	  next = nextLabel(next);
	}
      }
      if(next == NULL)
	new = (LABEL *)appendLink(&head, (LINK *)new);
      else 
	new = (LABEL *)insLinkBefore(&head, (LINK *)next, (LINK *)new);
    }
    else if(pos & LBL_ADD_AS_FIRST) {
      new = (LABEL *)insLinkBefore(&head, head, (LINK *)new);
    }
    else if(pos & LBL_ADD_AS_LAST) {
      new = (LABEL *)appendLink(&head, (LINK *)new);
    }
    else {
      setAsspMsg(AEG_ERR_BUG, "addLabel: invalid position specification");
      return(NULL);
    }
  }
  if(new == NULL)
    setAsspMsg(AEG_ERR_BUG, "addLabel: corrupted label list");
  dop->maxBufRecs = dop->bufNumRecs = numLinks((LINK *)(dop->dataBuffer));
  dop->bufNeedsSave = TRUE;
  return(new);
}

/*DOC

Function 'delLabel'

Removes the label pointed to by "lPtr" from the list in the label object 
pointed to by "dop" and deletes it. Returns a pointer to the next label 
in the list which will be NULL if "lPtr" referred to the last label. 
Since this function also returns NULL upon error, the macro 'asspError' 
should be used to detect the latter.

DOC*/

LABEL *delLabel(DOBJ *dop, LABEL *lPtr)
{
  clrAsspMsg();
  if(dop == NULL || lPtr == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "delLabel");
    return(NULL);
  }
  if(dop->dataBuffer == NULL) {
    setAsspMsg(AED_NO_DATA, "delLabel");
    return(NULL);
  }
  lPtr = (LABEL *)deleteLink((LINK **)&(dop->dataBuffer), (LINK *)lPtr,\
			    (freeLinkFunc)freeLabel);
  dop->maxBufRecs = dop->bufNumRecs = numLinks((LINK *)(dop->dataBuffer));
  dop->bufNeedsSave = TRUE;
  return(lPtr);
}

/*DOC

Function 'estRefRate'

The IPdS label formats 'MIX' and 'SAMPA' do not (yet) contain a header 
line with the sampling rate of the audio file they refer to.
This function may be used to estimate that rate on the basis of the 
sample number - time pair given by "smpNr" and "time". The rate will be 
rounded to the nearest "round" Hz. If you can be sure that the sample 
rate is one commonly used for audio data, "round" can be set to 25. A 
value of "round" less than 1 will simply return the quotient of "smpNr" 
and "time". The function returns -1 upon error.

Note:
 - "smpNr" should confirm ASSP conventions (0 corresponding to T = 0)
 - Most reliable estimates are obtained if "smpNr" is a prime. It should 
   at least be an odd number.

DOC*/

double estRefRate(long smpNr, double time, int round)
{
  long rate;

  if(smpNr <= 0 || time <= 0.0) {
    setAsspMsg(AEB_BAD_ARGS, "delLabel");
    return(-1.0);
  }
  if(round < 1) {
    return((double)smpNr / time);
  }
  if(round == 1) {
    rate = (long)myrint((double)smpNr / time);
  }
  else {
    rate = (long)myrint((double)smpNr / ((double)round * time));
    rate *= round;
  }
  return((double)rate);
}
