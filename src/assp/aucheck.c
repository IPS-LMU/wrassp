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
*----------------------------------------------------------------------*
*                                                                      *
* File:     aucheck.c                                                  *
* Contents: Functions for summarizing the properties of audio data,    *
*           capacities of audio file formats and verification of       *
*           capacity for handling audio coding.                        *
* Author:   Michel T.M. Scheffers                                      *
*                                                                      *
***********************************************************************/
/* $Id: aucheck.c,v 1.6 2010/01/14 14:44:44 mtms Exp $ */

#include <stdio.h>      /* snprintf() */
#include <string.h>     /* strlen() */

#include <miscdefs.h>   /* TRUE FALSE */
#include <asspmess.h>   /* message handler */
#include <asspendian.h> /* byte order stuff */
#include <aucheck.h>    /* flags and function prototypes */
#include <dataobj.h>    /* DOBJ DDESC fform_e */

/*DOC

Function 'checkSound'

Verifies whether the data object pointed to by "dop" refers to an audio 
signal and whether its properties can be handled by the calling function/
file format/audio device whose audio capabilities are summarized in 
"auCaps" (see aucheck.h). 
If "channel" is greater than zero, it will also be verified whether the 
signal contains at least this channel (counting starts at 1).
The function returns 0 when the object does not refer to an audio signal 
and -1 when the format or coding can't be handled. Otherwise it returns 
the properties of the audio signal as a set of flags (see aucheck.h).

DOC*/

long checkSound(DOBJ *dop, long auCaps, int channel)
{
  int  isFile;
  long auProps, format, endian, numTracks, maxTracks;

  if(dop == NULL || auCaps <= 0) {
    setAsspMsg(AEB_BAD_ARGS, "checkSound");
    return(-1L);
  }
  
  isFile = FALSE;
  if(dop->filePath != NULL) {
    if(strlen(dop->filePath) != 0)
      isFile = TRUE;
  }
  auProps = auPropsDO(dop);
  if(auProps < 0) {
    if(strlen(applMessage) == 0 && isFile && !(auCaps & AUC_HEAD))
      snprintf(applMessage, sizeof(applMessage), "in file %s", dop->filePath);
    return(AUC_ERROR);
  }
  if(auProps == AUC_NONE) {
    setAsspMsg(AED_NO_AUDIO, NULL);
    if(isFile && !(auCaps & AUC_HEAD))
      snprintf(applMessage, sizeof(applMessage), "in file %s", dop->filePath);
    return(AUC_NONE);
  }
  format = auProps & AUC_FORM_MASK;
  if(!(auCaps & format)) {
    setAsspMsg(AED_NOHANDLE, NULL);
    if(auProps & AUC_FILE && !(auCaps & AUC_HEAD))
      snprintf(applMessage, sizeof(applMessage), "in file %s", dop->filePath);
    return(AUC_ERROR);
  }
  if(format & AUC_SWAP_MASK) {/* no endian check for single-byte data */
    endian = auProps & AUC_MSB_MASK;
    if(!(auCaps & endian)) {
      setAsspMsg(AED_NOHANDLE, NULL);
      if(auProps & AUC_FILE && !(auCaps & AUC_HEAD))
	snprintf(applMessage, sizeof(applMessage), "in file %s", dop->filePath);
      return(AUC_ERROR);
    }
  }
  numTracks = auProps & AUC_CHAN_MASK;
  if(numTracks == 0 || dop->frameDur != 1) {
    setAsspMsg(AED_ERR_FORM, NULL);
    if(auProps & AUC_FILE && !(auCaps & AUC_HEAD))
      snprintf(applMessage, sizeof(applMessage), "in file %s", dop->filePath);
    return(AUC_ERROR);
  }
  maxTracks = auCaps & AUC_CHAN_MASK;
  if(maxTracks > 0 && numTracks > maxTracks) {
    setAsspMsg(AEG_ERR_APPL, NULL);
    snprintf(applMessage, sizeof(applMessage), "Can't handle %ld-channel data", numTracks);
    if(auProps & AUC_FILE && !(auCaps & AUC_HEAD)) {
      strcat(applMessage, "\nin file ");
      strcat(applMessage, dop->filePath);
    }
    return(AUC_ERROR);
  }
  if(channel > 0 && (long)channel > numTracks) {
    setAsspMsg(AEG_ERR_APPL, NULL);
    snprintf(applMessage, sizeof(applMessage), "Channel %i not available", channel);
    if(auProps & AUC_FILE && !(auCaps & AUC_HEAD)) {
      strcat(applMessage, "\nin file ");
      strcat(applMessage, dop->filePath);
    }
    return(AUC_ERROR);
  }
  if(dop->sampFreq <= 0.0) {
    setAsspMsg(AEG_ERR_APPL, "Sampling frequency undefined");
    if(auProps & AUC_FILE && !(auCaps & AUC_HEAD)) {
      strcat(applMessage, "\nin file ");
      strcat(applMessage, dop->filePath);
    }
    return(AUC_ERROR);
  }
  return(auProps);
}

/*DOC

Function 'auPropsDO'

Verifies whether the data object pointed to by "dop" refers to an audio 
signal and returns the properties of its coding as flags (see aucheck.h).
The function returns 0 if the object does not refer to an audio signal  
and -1 when the format or coding can't be handled.

DOC*/

long auPropsDO(DOBJ *dop)
{
  long auProps;

  if(dop == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "auPropsDO");
    return(AUC_ERROR);
  }

  auProps = auPropsDD(&(dop->ddl));
  if(auProps > 0) { /* add endian flags */
    if(MSBFIRST(dop->fileEndian))
      auProps |= AUC_MSB_F;
    if(MSBLAST(dop->fileEndian))
      auProps |= AUC_MSB_L;
    if(dop->filePath != NULL) {
      if(strlen(dop->filePath) != 0)
	auProps |= AUC_FILE;
    }
  }
  return(auProps);
}

/*DOC

Function 'auPropsDD'

Verifies whether the data descriptor "dd" refers to an audio signal 
and returns the properties of its coding as flags (see aucheck.h).
The function returns 0 if the descriptor does not refer to an 
audio signal and -1 when the format or coding can't be handled.

DOC*/

long auPropsDD(DDESC *dd)
{
  long auProps;

  if(dd == NULL) {
    setAsspMsg(AEB_BAD_ARGS, "auPropsDD");
    return(AUC_ERROR);
  }

  clrAsspMsg();
  auProps = AUC_NONE;
  if(dd->type == DT_SMP && dd->next == NULL) {
    switch(dd->format) {
    case DF_UINT8:
      switch(dd->coding) {
      case DC_PCM:
      case DC_BINOFF:
	auProps = AUC_U8;
	break;
      case DC_ALAW:
	auProps = AUC_ALAW;
	break;
      case DC_uLAW:
	auProps = AUC_uLAW;
	break;
      default:
	setAsspMsg(AED_NOHANDLE, NULL);
	return(AUC_ERROR);
      }
      break;
    case DF_INT8:
      switch(dd->coding) {
      case DC_PCM:
	auProps = AUC_I8;
	break;
      case DC_ALAW:
	auProps = AUC_ALAW;
	break;
      case DC_uLAW:
	auProps = AUC_uLAW;
	break;
      default:
	setAsspMsg(AED_NOHANDLE, NULL);
	return(AUC_ERROR);
      }
      break;
    default:
      if(!(dd->coding == DC_PCM || dd->coding == DC_BINOFF) ) {
	setAsspMsg(AED_NOHANDLE, NULL);
	return(AUC_ERROR);
      }
      switch(dd->format) {
      case DF_UINT16:
	auProps = AUC_U16;
	break;
      case DF_INT16:
	auProps = AUC_I16;
	break;
      case DF_UINT24:
	auProps = AUC_U24;
	break;
      case DF_INT24:
	auProps = AUC_I24;
	break;
      case DF_UINT32:
	auProps = AUC_U32;
	break;
      case DF_INT32:
	auProps = AUC_I32;
	break;
      case DF_REAL32:
	auProps = AUC_F32;
	break;
      case DF_REAL64:
	auProps = AUC_F64;
	break;
      default:
	setAsspMsg(AED_NOHANDLE, NULL);
	return(AUC_ERROR);
      }
      break;
    }
    if((long)dd->numFields > AUC_CHAN_MASK)
      auProps |= AUC_CHAN_MASK;   /* very large */
    else
      auProps |= ((long)dd->numFields & AUC_CHAN_MASK);
  }
  return(auProps);
}

/*DOC

Function 'auCapsFF'

Returns the audio capabilities of a file format as a set of flags 
(see aucheck.h). Returns 0 when the file format doesn't support 
audio or isn't recognized.

DOC*/

long auCapsFF(fform_e fileFormat)
{
  long auCaps;

  auCaps = AUC_NONE;
  switch(fileFormat) {
  case FF_RAW:
    auCaps = AUC_ALAW | AUC_uLAW | AUC_U8 | AUC_I8 |\
             AUC_U16 | AUC_I16 | AUC_U24 | AUC_I24 |\
	     AUC_U32 | AUC_I32 | AUC_F32 | AUC_F64;
    auCaps |= AUC_MSB_X;
    auCaps |= AUC_CHAN_MASK;
    break;
  case FF_AIFF:
    auCaps = AUC_I8 | AUC_I16 | AUC_I24 | AUC_I32;
    auCaps |= AUC_MSB_F;
    auCaps |= AUC_CHAN_MASK;
    break;
  case FF_AIFC:
    auCaps = AUC_I8 | AUC_I16 | AUC_I24 | AUC_I32 |\
             AUC_ALAW | AUC_uLAW | AUC_F32 | AUC_F64;
    auCaps |= AUC_MSB_F;
    auCaps |= AUC_CHAN_MASK;
    break;
  case FF_CSL:
    auCaps = AUC_I16;
    auCaps |= AUC_MSB_L;
    auCaps |= 0x02;
    break;
  case FF_CSRE:
    auCaps = AUC_U16 | AUC_I16;
    auCaps |= AUC_MSB_L;
    auCaps |= 0x01;   /* ??? */
    break;
  case FF_ESPS:
    /* NOT SUPPORTED */
    break;
  case FF_ILS:
    /* NOT SUPPORTED */
    break;
  case FF_KTH:
    /* auCaps = AUC_I8 | AUC_I16 | AUC_I32 | AUC_F32 | AUC_F64; */
    auCaps = AUC_I16; /* only support 16-bit audio */
    auCaps |= AUC_MSB_X;
    auCaps |= AUC_CHAN_MASK;
    break;
  case FF_SFS:
    /* NOT SUPPORTED */
    break;
  case FF_SND:
    auCaps = AUC_ALAW | AUC_uLAW | AUC_I8 | AUC_I16 | AUC_I24 |\
             AUC_I32 | AUC_F32 | AUC_F64;
    auCaps |= AUC_MSB_F;
    auCaps |= AUC_CHAN_MASK;
    break;
  case FF_NIST:
    auCaps = AUC_uLAW | AUC_I8 | AUC_I16 | AUC_I24 | AUC_I32;
    auCaps |= AUC_MSB_X;
    auCaps |= AUC_CHAN_MASK;
    break;
  case FF_SSFF:
    /* auCaps = AUC_U8 | AUC_I16 | AUC_I32 | AUC_F32 | AUC_F64; */
    auCaps = AUC_I16; /* the only ones left? */
    auCaps |= AUC_MSB_X;
    auCaps |= AUC_CHAN_MASK;
    break;
  case FF_WAVE: /* old style; may cause problems with MS programs */
    auCaps = AUC_ALAW | AUC_uLAW | AUC_U8 | AUC_I16 | AUC_I24 |\
             AUC_I32 | AUC_F32 | AUC_F64;
    auCaps |= AUC_MSB_L;
    auCaps |= AUC_CHAN_MASK;
    break;
/*   case FF_WAVE: */ /* new style; may cause problems with non-MS programs */
/*     auCaps = AUC_U8 | AUC_I16; */
/*     auCaps |= AUC_MSB_L; */
/*     auCaps |= 0x02; */
/*     break; */
  case FF_WAVE_X: /* extended format; may need 'fact' chunk */
    auCaps = AUC_ALAW | AUC_uLAW | AUC_U8 | AUC_I16 | AUC_I24 |\
             AUC_I32 | AUC_F32 | AUC_F64;
    auCaps |= AUC_MSB_L;
    auCaps |= AUC_CHAN_MASK;
    break;
  default: /* no audio or not supported */
    auCaps = AUC_NONE;
    break;
  }
  if(auCaps > 0)
    auCaps |= AUC_HEAD;
  return(auCaps);
}

/*DOC

Function 'checkAuBits'

Verifies/corrects 'numBits' in the data descriptor "dd" for audio data.
This function assumes a more or less sensible data packing.

DOC*/

int checkAuBits(DDESC *dd)
{
  while(dd != NULL) {
    if(dd->type == DT_SMP) {
      switch(dd->coding) {
      case DC_PCM:
      case DC_BINOFF:
	switch(dd->format) {
	case DF_UINT8:
	case DF_INT8:
	  dd->numBits = 8;
	  break;
	case DF_UINT16:
	case DF_INT16:
	  if(dd->numBits <= 8 || dd->numBits > 16)
	    dd->numBits = 16;
	  break;
	case DF_UINT24:
	case DF_INT24:
	  if(dd->numBits <= 16 || dd->numBits > 24)
	    dd->numBits = 24;
	  break;
	case DF_UINT32:
	case DF_INT32: /* might have put 24-bit data in 4 bytes */
	  if(dd->numBits < 24 || dd->numBits > 32)
	    dd->numBits = 32;
	  break;
	case DF_REAL32:
	  dd->numBits = 32;
	  break;
	case DF_REAL64:
	  dd->numBits = 64;
	  break;
	default: /* just to appease the compiler */
	  break;
	}
	break;
      case DC_ALAW:
      case DC_uLAW:
	dd->numBits = 8;
	break;
      default: /* just to appease the compiler */
	break;
      }
    }
    dd = dd->next; /* just in case we have mixed data */
  }
  return(0);
}

