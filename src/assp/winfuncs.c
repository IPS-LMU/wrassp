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
* File:      winfuncs.c                                                *
* Contents:  Support functions for windowing signals. Prototypes of    *
*            these functions, codes of the window functions and the    *
*            structure 'WFDATA' are in 'asspdsp.h'.                    *
* Author:    Michel T.M. Scheffers                                     *
* References:                                                          *
*   Harris, F.J. (1978), "On the Use of Windows for Harmonic Analysis  *
*     with the Discrete Fourier Transform," Proc. IEEE 66, 51-83.      *
*   Nuttal, A.H. (1981), "Some Windows with Very Good Sidelobe         *
*     Behavior," IEEE Trans. ASSP 29, 84-91.                           *
*   Harris, F.J. (2004 ?), "Finite Aperture Effects and Applications   *
*     in Signal Processing," http:\\www.signumconcepts.com             *
*                                                                      *
***********************************************************************/
/* $Id: winfuncs.c,v 1.10 2010/07/19 08:53:37 mtms Exp $ */

#include <stdio.h>     /* FILE NULL */
#include <stddef.h>    /* size_t */
#include <stdlib.h>    /* calloc() free() */
#include <string.h>    /* strncmp() strcpy() */
#include <ctype.h>     /* islower() toupper() */
#include <math.h>      /* sin() cos() exp() pow() ceil() floor() */

#include <miscdefs.h>  /* LOCAL EOS PI EVEN() */
#include <misc.h>      /* strxcmp() */
#include <asspdsp.h>   /* wfunc_e WFDATA WFLIST WF_XYZ bessi0() */
#include <asspmess.h>  /* message codes setAsspMsg() */

/*
 * public lists
 */
WFLIST wfShortList[] = { /* most appropriate window functions */
  {"RECTANGLE", "rectangle"                  , WF_RECTANGLE},
  {"PARABOLA" , "parabola/Riesz/Welch"       , WF_PARABOLA},
  {"COS"      , "cosine"                     , WF_COSINE},
  {"HANN"     , "Hann/hanning/cosine^2"      , WF_HANN},
  {"COS_4"    , "cosine^4"                   , WF_COS_4},
  {"HAMMING"  , "Hamming"                    , WF_HAMMING},
  {"BLACKMAN" , "Blackman"                   , WF_BLACKMAN},
  {"BLACK_X"  , "exact Blackman"             , WF_BLACK_X},
  {"BLACK_M3" , "min. 3-term Blackman-Nuttal", WF_BLACK_M3},
  {"BLACK_M4" , "min. 4-term Blackman-Nuttal", WF_BLACK_M4},
  {"NUTTAL_3" , "3-term Nuttal (-18 dB/oct)" , WF_NUTTAL_3},
  {"NUTTAL_4" , "4-term Nuttal (-18 dB/oct)" , WF_NUTTAL_4},
  {"KAISER2_0", "Kaiser-Bessel (alpha = 2.0)", WF_KAISER2_0},
  {"KAISER3_0", "Kaiser-Bessel (alpha = 3.0)", WF_KAISER3_0},
  {"KAISER4_0", "Kaiser-Bessel (alpha = 4.0)", WF_KAISER4_0},
  {NULL, NULL, WF_NONE}
};

WFLIST wfLongList[] = { /* all non-parametric window functions */
  {"RECTANGLE", "rectangle"                  , WF_RECTANGLE},
  {"TRIANGLE" , "triangle/Bartlett/Fejer"    , WF_TRIANGLE},
  {"PARABOLA" , "parabola/Riesz/Welch"       , WF_PARABOLA},
  {"COS"      , "cosine"                     , WF_COSINE},
  {"HANN"     , "Hann/hanning/cosine^2"      , WF_HANN},
  {"COS_3"    , "cosine^3"                   , WF_COS_3},
  {"COS_4"    , "cosine^4"                   , WF_COS_4},
  {"HAMMING"  , "Hamming"                    , WF_HAMMING},
  {"BLACKMAN" , "Blackman"                   , WF_BLACKMAN},
  {"BLACK_X"  , "exact Blackman"             , WF_BLACK_X},
  {"BLACK_3"  , "3-term Blackman-Harris"     , WF_BLACK_3},
  {"BLACK_M3" , "min. 3-term Blackman-Nuttal", WF_BLACK_M3},
  {"BLACK_4"  , "4-term Blackman-Harris"     , WF_BLACK_4},
  {"BLACK_M4" , "min. 4-term Blackman-Nuttal", WF_BLACK_M4},
  {"NUTTAL_3" , "3-term Nuttal (-18 dB/oct)" , WF_NUTTAL_3},
  {"NUTTAL_4" , "4-term Nuttal (-18 dB/oct)" , WF_NUTTAL_4},
  {"GAUSS2_5" , "Gaussian (alpha = 2.5)"     , WF_GAUSS2_5},
  {"GAUSS3_0" , "Gaussian (alpha = 3.0)"     , WF_GAUSS3_0},
  {"GAUSS3_5" , "Gaussian (alpha = 3.5)"     , WF_GAUSS3_5},
  {"KAISER2_0", "Kaiser-Bessel (alpha = 2.0)", WF_KAISER2_0},
  {"KAISER2_5", "Kaiser-Bessel (alpha = 2.5)", WF_KAISER2_5},
  {"KAISER3_0", "Kaiser-Bessel (alpha = 3.0)", WF_KAISER3_0},
  {"KAISER3_5", "Kaiser-Bessel (alpha = 3.5)", WF_KAISER3_5},
  {"KAISER4_0", "Kaiser-Bessel (alpha = 4.0)", WF_KAISER4_0},
  {NULL, NULL, WF_NONE}
};

WFLIST wfAlphaList[] = { /* parametric window functions */
  {"COS_A"    , "cosine^alpha"                , WF_COS_A},
  {"GEN_HAMM" , "generalised Hamming(alpha)"  , WF_GEN_HAMM},
  {"GAUSS_A"  , "Gaussian(alpha)"             , WF_GAUSS_A},
  {"KAISER_A" , "Kaiser-Bessel(alpha)"        , WF_KAISER_A},
  {"KAISER_B" , "Kaiser-Bessel(beta)"         , WF_KAISER_B},
  {"KBD_A"    , "Kaiser-Bessel-derived(alpha)", WF_KBD_A},
  {NULL, NULL, WF_NONE}
};

/*DOC

Function 'wfType'

Returns the type number of a window function, given by name. For the 
names, case-insensitive string equivalents of the enum's in 'asspdsp.h' 
are used. 
The function returns WF_ERROR (-1) for an unknown name.

DOC*/

wfunc_e wfType(char *str)
{
  char    name[16];
  int     i;
  wfunc_e type;

  for(i = 0; i < 15 && *str != EOS; i++, str++) {
    if(islower((int)*str)) /* convert to upper case */
      name[i] = (char)toupper((int)*str);
    else
      name[i] = *str;
  }
  name[i] = EOS;           /* close string */
  type = WF_ERROR;         /* init unknown */
  switch(name[0]) {
  case 'B':
    if(strncmp(name, "BAR", 3) == 0)
      type = WF_BARTLETT;
    else if(strncmp(name, "BLACKM", 6) == 0)
      type = WF_BLACKMAN;
    else if(strncmp(name, "BLACK_X", 7) == 0)
      type = WF_BLACK_X;
    else if(strncmp(name, "BLACK_3", 7) == 0)
      type = WF_BLACK_3;
    else if(strncmp(name, "BLACK_M3", 8) == 0)
      type = WF_BLACK_M3;
    else if(strncmp(name, "BLACK_4", 7) == 0)
      type = WF_BLACK_4;
    else if(strncmp(name, "BLACK_M4", 8) == 0)
      type = WF_BLACK_M4;
    break;
  case 'C':
    if(strcmp(name, "COS") == 0 || strncmp(name, "COSI", 4) == 0)
      type = WF_COSINE;
    else if(strncmp(name, "COS_2", 5) == 0)
      type = WF_COS_2;
    else if(strncmp(name, "COS_3", 5) == 0)
      type = WF_COS_3;
    else if(strncmp(name, "COS_4", 5) == 0)
      type = WF_COS_4;
    else if(strncmp(name, "COS_A", 5) == 0)
      type = WF_COS_A;
    break;
  case 'F':
    if(strncmp(name, "FEJ", 3) == 0)
      type = WF_FEJER;
    break;
  case 'G':
    if(strncmp(name, "GAUSS2_5", 8) == 0)
      type = WF_GAUSS2_5;
    else if(strncmp(name, "GAUSS3_0", 8) == 0)
      type = WF_GAUSS3_0;
    else if(strncmp(name, "GAUSS3_5", 8) == 0)
      type = WF_GAUSS3_5;
    else if(strncmp(name, "GAUSS_A", 7) == 0)
      type = WF_GAUSS_A;
    else if(strncmp(name, "GEN", 3) == 0)
      type = WF_GEN_HAMM;
    break;
  case 'H':
    if(strncmp(name, "HAM", 3) == 0)
      type = WF_HAMMING;
    else if(strncmp(name, "HAN", 3) == 0)
      type = WF_HANN;
    break;
  case 'K':
    if(strncmp(name, "KAISER2_0", 9) == 0)
      type = WF_KAISER2_0;
    else if(strncmp(name, "KAISER2_5", 9) == 0)
      type = WF_KAISER2_5;
    else if(strncmp(name, "KAISER3_0", 9) == 0)
      type = WF_KAISER3_0;
    else if(strncmp(name, "KAISER3_5", 9) == 0)
      type = WF_KAISER3_5;
    else if(strncmp(name, "KAISER4_0", 9) == 0)
      type = WF_KAISER4_0;
    else if(strncmp(name, "KAISER_A", 8) == 0)
      type = WF_KAISER_A;
    else if(strncmp(name, "KAISER_B", 8) == 0)
      type = WF_KAISER_B;
    else if(strncmp(name, "KBD", 3) == 0)
      type = WF_KBD_A;
    break;
  case 'N':
    if(strncmp(name, "NON", 3) == 0)
      type = WF_NONE;
    else if(strncmp(name, "NUTTAL_3", 8) == 0)
      type = WF_NUTTAL_3;
    else if(strncmp(name, "NUTTAL_4", 8) == 0)
      type = WF_NUTTAL_4;
    break;
  case 'P':
    if(strncmp(name, "PAR", 3) == 0)
      type = WF_PARABOLA;
    break;
  case 'R':
    if(strncmp(name, "REC", 3) == 0)
      type = WF_RECTANGLE;
    else if(strncmp(name, "RIE", 3) == 0)
      type = WF_RIESZ;
    break;
  case 'T':
    if(strncmp(name, "TRI", 3) == 0)
      type = WF_TRIANGLE;
    break;
  case 'W':
    if(strncmp(name, "WEL", 3) == 0)
      type = WF_WELCH;
    break;
  }
  return(type);
}

/*DOC

Function 'wfListEntry'

Searches the entry in the window function list pointed to by "list" 
matching its code name pointed to by "code" or, if "code" is a NULL-
pointer, its description pointed to by "desc" or , "desc" is also a 
NULL-pointer, its type number given in "type".
The function returns a pointer to the entry in "list" or NULL if no 
matchning window function has been found.

NOTE: If a descriptor string is used it must exactly match, should 
      therefore best be taken from the "desc" entry in the list.

DOC*/

WFLIST *wfListEntry(WFLIST *list, char *code, char *desc, wfunc_e type)
{
  WFLIST *wPtr=NULL;

  if(list != NULL) {
    if(code != NULL)
      type = wfType(code); /* more flexible */
    for(wPtr = list; wPtr->code != NULL; wPtr++) {
      if(desc != NULL) {
	if(strxcmp(desc, wPtr->desc) == 0)
	  break;
      }
      else if(type == wPtr->type)
	break;
    }
    if(wPtr->code == NULL) /* not found */
      wPtr = NULL;
  }
  return(wPtr);
}

/*DOC

Function 'wfSpecs'

Returns a pointer to a 'WFDATA' structure containing the name and the 
main specifications of a window function, given by its type number. 
The structure is static and needs to be copied if the data are to be 
retained.
The function returns a NULL pointer for an unknown type number or for 
type numbers referring to parametric windows.

NOTE: In Harris(1978) the highest side lobe level is often very roughly 
      rounded or even incorrect (see Nuttal, 1981). We therefore list 
      measured values instead.

DOC*/

WFDATA *wfSpecs(wfunc_e type)
{
  static WFDATA specs;
  WFLIST *wPtr;

  wPtr = wfListEntry(wfLongList, NULL, NULL, type);
  if(wPtr == NULL) {
    setAsspMsg(AEG_ERR_BUG, "wfSpecs: incorrect window function");
    return(NULL);
  }
  specs.entry = wPtr;
  switch(type) {
  case WF_RECTANGLE:
    specs.hsll = -13.26;
    specs.roff = -6;
    specs.gain = 1.0;
    specs.msqr = 1.0;
    specs.dB_3 = 0.89;
    specs.enbw = 1.0;
    specs.dB_6 = 1.21;
    specs.mlbw = 2.0;
    break;
  case WF_TRIANGLE:
    specs.hsll = -26.52;
    specs.roff = -12;
    specs.gain = 0.50;
    specs.msqr = 1.0 / 3.0; /* 0.333... */
    specs.dB_3 = 1.28;
    specs.enbw = 4.0 / 3.0; /* 1.333... */
    specs.dB_6 = 1.77;
    specs.mlbw = 4.0;
    break;
  case WF_PARABOLA:
    specs.hsll = -21.29;
    specs.roff = -12;
    specs.gain = 2.0 / 3.0;  /* 0.666... */
    specs.msqr = 1.60 / 3.0; /* 0.5333... */
    specs.dB_3 = 1.16;
    specs.enbw = 1.20;
    specs.dB_6 = 1.59;
    specs.mlbw = 2.86;
    break;
  case WF_COSINE:
    specs.hsll = -23.00;
    specs.roff = -12;
    specs.gain = 2.0 / PI;
    specs.msqr = 0.50;
    specs.dB_3 = 1.19;
    specs.enbw = 1.23370055;
    specs.dB_6 = 1.64;
    specs.mlbw = 3.0;
    break;
  case WF_HANN:
    specs.hsll = -31.47;
    specs.roff = -18;
    specs.gain = 0.50;
    specs.msqr = 0.3750;
    specs.dB_3 = 1.44;
    specs.enbw = 1.50;
    specs.dB_6 = 2.00;
    specs.mlbw = 4.0;
    break;
  case WF_COS_3:
    specs.hsll = -39.30;
    specs.roff = -24;
    specs.gain = 4.0 / (3.0 * PI);
    specs.msqr = 0.31250;
    specs.dB_3 = 1.66;
    specs.enbw = 1.73489140;
    specs.dB_6 = 2.31;
    specs.mlbw = 5.0;
    break;
  case WF_COS_4:
    specs.hsll = -46.74;
    specs.roff = -30;
    specs.gain = 0.3750;
    specs.msqr = 0.27343750;
    specs.dB_3 = 1.85;
    specs.enbw = 1.94444444;
    specs.dB_6 = 2.59;
    specs.mlbw = 6.0;
    break;
  case WF_HAMMING:
    specs.hsll = -42.67;
    specs.roff = -6;
    specs.gain = 0.540;
    specs.msqr = 0.39740;
    specs.dB_3 = 1.30;
    specs.enbw = 1.36282579;
    specs.dB_6 = 1.82;
    specs.mlbw = 4.0;
    break;
  case WF_BLACKMAN:
    specs.hsll = -58.11;
    specs.roff = -18;
    specs.gain = 0.420;
    specs.msqr = 0.30460;
    specs.dB_3 = 1.64;
    specs.enbw = 1.72675737;
    specs.dB_6 = 2.30;
    specs.mlbw = 6.0;
    break;
  case WF_BLACK_X:    /* table values in Harris(1978) incorrect */
    specs.hsll = -68.23; /* Harris(1978) -51 vs. Harris(2004) -68 */
    specs.roff = -6;
    specs.gain = 7938.0 / 18608.0;
    specs.msqr = 0.30821872;
    specs.dB_3 = 1.61;
    specs.enbw = 1.69369895;
    specs.dB_6 = 2.25;
    specs.mlbw = 6.0;
    break;
  case WF_BLACK_3:
    specs.hsll = -62.04; /* Harris(1978): -61 */
    specs.roff = -6;
    specs.gain = 0.449590;
    specs.msqr = 0.32558281;
    specs.dB_3 = 1.53; /* Harris(1978): 1.56 */
    specs.enbw = 1.61075015;
    specs.dB_6 = 2.14; /* Harris(1978): 2.19 */
    specs.mlbw = 5.58;
    break;
/*   case WF_BLACK_M3: */
/*     specs.hsll = -70.83; */ /* Harris(1978) -67 vs. Harris(2004) -72 */
/*     specs.roff = -6; */
/*     specs.gain = 0.423230; */
/*     specs.msqr = 0.30603954; */
/*     specs.dB_3 = 1.62; */ /* Harris(1978): 1.66 */
/*     specs.enbw = 1.70853803; */
/*     specs.dB_6 = 2.27; */ /* Harris(1978): 1.81 */
/*     specs.mlbw = 6.0; */
/*     break; */
  case WF_BLACK_M3:  /* with Nuttal improvement */
    specs.hsll = -71.47;
    specs.roff = -6;
    specs.gain = 0.42438010;
    specs.msqr = 0.30683613;
    specs.dB_3 = 1.62;
    specs.enbw = 1.70371315;
    specs.dB_6 = 2.27;
    specs.mlbw = 6.0;
    break;
  case WF_BLACK_4:
    /* !!! BLACK MAGIC !!! Window doesn't meet specifications at all  */
    /* when constructed with the constants given in Harris(1978) (see */
    /* makeWF() below). By judiciously replacing 3's by 8's, one gets */
    /* the following values which come very much closer.              */
    specs.hsll = -74.39;
    specs.roff = -6;
    specs.gain = 0.402170;
    specs.msqr = 0.29015447;
    specs.dB_3 = 1.70;
    specs.enbw = 1.79394830;
    specs.dB_6 = 2.39;
    specs.mlbw = 6.53;
    break;
/*   case WF_BLACK_M4: */
/*     specs.hsll = -92.01; */
/*     specs.roff = -6; */
/*     specs.gain = 0.358750; */
/*     specs.msqr = 0.25796335; */
/*     specs.dB_3 = 1.90; */
/*     specs.enbw = 2.00435294; */
/*     specs.dB_6 = 2.67; */ /* Harris(1978): 2.72 */
/*     specs.mlbw = 8.0; */
/*     break; */
  case WF_BLACK_M4:  /* with Nuttal improvement */
    specs.hsll = -98.14;
    specs.roff = -6;
    specs.gain = 0.36358190;
    specs.msqr = 0.26122544;
    specs.dB_3 = 1.87;
    specs.enbw = 1.97619285;
    specs.dB_6 = 2.63;
    specs.mlbw = 8.0;
    break;
  case WF_NUTTAL_3:
    specs.hsll = -64.19;
    specs.roff = -18;
    specs.gain = 0.408970;
    specs.msqr = 0.29639969;
    specs.dB_3 = 1.69;
    specs.enbw = 1.77212701;
    specs.dB_6 = 2.36;
    specs.mlbw = 6.0;
    break;
  case WF_NUTTAL_4:
    specs.hsll = -93.32;
    specs.roff = -18;
    specs.gain = 0.35557680;
    specs.msqr = 0.25582917;
    specs.dB_3 = 1.92;
    specs.enbw = 2.02123258;
    specs.dB_6 = 2.69;
    specs.mlbw = 8.0;
    break;
/* The table values for the Gaussian windows in Harris(1978) often    */
/* deviate from computed/estimated ones. Since there are some clearly */
/* erroneous values, the computed/estimated ones are given here.      */
  case WF_GAUSS2_5:
    specs.hsll = -43.25; /* Harris(1978): -42 */
    specs.roff = -6;
    specs.gain = 0.49509953;
    specs.msqr = 0.35434651;
    specs.dB_3 = 1.37;
    specs.enbw = 1.44558335;
    specs.dB_6 = 1.92;
    specs.mlbw = 6.41;
    break;
  case WF_GAUSS3_0:
    specs.hsll = -56.07; /* Harris(1978): -55 */
    specs.roff = -6;
    specs.gain = 0.41664348;
    specs.msqr = 0.29540245;
    specs.dB_3 = 1.60;
    specs.enbw = 1.70170748;
    specs.dB_6 = 2.26;
    specs.mlbw = 6.95;
    break;
  case WF_GAUSS3_5:
    specs.hsll = -70.99; /* Harris(1978): -69 */
    specs.roff = -6;
    specs.gain = 0.35792315;
    specs.msqr = 0.25320750;
    specs.dB_3 = 1.86;
    specs.enbw = 1.97650081;
    specs.dB_6 = 2.62;
    specs.mlbw = 10.45; /* Note: weird sudden increase */
    break;
/*   case WF_GAUSS4_0: */   /* omit (too wide) */
/*     specs.hsll = -87.61; */
/*     specs.roff = -6; */
/*     specs.gain = 0.31330869; */
/*     specs.msqr = 0.22155673; */
/*     specs.dB_3 = 2.12; */
/*     specs.enbw = 2.25704422; */
/*     specs.dB_6 = 3.00; */
/*     specs.mlbw = 11.41; */
/*     break; */
  case WF_KAISER2_0:
    specs.hsll = -45.85;
    specs.roff = -6;
    specs.gain = 0.48919357;
    specs.msqr = 0.35809013;
    specs.dB_3 = 1.43;
    specs.enbw = 1.49634200;
    specs.dB_6 = 1.99;
    specs.mlbw = 4.47;
    break;
  case WF_KAISER2_5:
    specs.hsll = -57.56; /* Harris(1978): -57 */
    specs.roff = -6;
    specs.gain = 0.43963077;
    specs.msqr = 0.31927529;
    specs.dB_3 = 1.57;
    specs.enbw = 1.65192049;
    specs.dB_6 = 2.20;
    specs.mlbw = 5.39;
    break;
  case WF_KAISER3_0:
    specs.hsll = -69.62; /* Harris(1978): -69 */
    specs.roff = -6;
    specs.gain = 0.40254800;
    specs.msqr = 0.29090868;
    specs.dB_3 = 1.71;
    specs.enbw = 1.79523508;
    specs.dB_6 = 2.39;
    specs.mlbw = 6.33;
    break;
  case WF_KAISER3_5:
    specs.hsll = -81.92;
    specs.roff = -6;
    specs.gain = 0.37747737;
    specs.msqr = 0.26899017;
    specs.dB_3 = 1.83;
    specs.enbw = 1.92844758;
    specs.dB_6 = 2.57;
    specs.mlbw = 7.28;
    break;
  case WF_KAISER4_0:
    specs.hsll = -94.41;
    specs.roff = -6;
    specs.gain = 0.34990224;
    specs.msqr = 0.25139004;
    specs.dB_3 = 1.95;
    specs.enbw = 2.05331043;
    specs.dB_6 = 2.73;
    specs.mlbw = 8.250;
    break;
  default: /* parametric windows not specified */
    return(NULL);
  }
  return(&specs);
}

/*DOC

Function 'makeWF'

Allocates memory for "N" window coefficients and sets them according to 
the window function, specified by its type number "type".
The bits in "flags" may be used to further specify the coefficients: 
When no bits are set (aka 'WF_FULL_SIZE') a full-length (i.e. including 
both endpoints) symmetric function will be computed. In many cases, such 
as in proper DFT applications, this is undesired because the function 
should be periodic. This can be ensured by setting "flags" to 
'WF_PERIODIC' causing the endpoints to be shifted half a sample inwards. 
Furthermore, in short-term analyses, one might wish to align the centre 
of the window exactly with the centre of the frame. If the frame/window 
shift is an odd number of samples but the window length an even one or 
vise versa, "flags" should hereto be set to 'WF_ASYMMETRIC', otherwise 
to 'WF_PERIODIC'. In all cases the macros 'FRMNRtoSMPNR' and 'FRAMEHEAD' 
from 'assptime.h' should be used to determine the exact start point of 
the windowing.

The function returns a pointer to the array with the coefficients. 
It returns a NULL pointer if no memory can be allocated, if an invalid 
type number is given or if the number of coefficients is less than 
'WF_MIN_SIZE'.

NOTE: If "flags" equals 'WF_ASYMMETRIC' the peak value of '1' is reached 
      for even values of "N" at index N/2, otherwise for odd values of 
      "N" at (N-1)/2.

DOC*/

double *makeWF(wfunc_e type, long N, int flags)
{
  register long    n;
  register double *lPtr, *rPtr;
  int     SHIFT;
  double *wf, a0, a1, a2, a3;
  double  arg, step;
  
  if(N < WF_MIN_SIZE) {
    setAsspMsg(AEB_BAD_ARGS, "makeWF: N too small");
    return(NULL);
  }
  clrAsspMsg();
  switch(type) {
  case WF_GAUSS2_5:
    return(makeWF_A(WF_GAUSS_A, 2.5, N, flags));
    break;
  case WF_GAUSS3_0:
    return(makeWF_A(WF_GAUSS_A, 3.0, N, flags));
    break;
  case WF_GAUSS3_5:
    return(makeWF_A(WF_GAUSS_A, 3.5, N, flags));
    break;
  case WF_KAISER2_0:
    return(makeWF_A(WF_KAISER_A, 2.0, N, flags));
    break;
  case WF_KAISER2_5:
    return(makeWF_A(WF_KAISER_A, 2.5, N, flags));
    break;
  case WF_KAISER3_0:
    return(makeWF_A(WF_KAISER_A, 3.0, N, flags));
    break;
  case WF_KAISER3_5:
    return(makeWF_A(WF_KAISER_A, 3.5, N, flags));
    break;
  case WF_KAISER4_0:
    return(makeWF_A(WF_KAISER_A, 4.0, N, flags));
    break;
  default: /* non-parametric windows */
    SHIFT = (flags == WF_PERIODIC);
    if(flags == WF_ASYMMETRIC)
      N++;         /* make the window symmetric for simplicity's sake */
    wf = (double *)calloc((size_t)N, sizeof(double));
    if(wf == NULL) {
      setAsspMsg(AEG_ERR_MEM, "(makeWF)");
      return(NULL);
    }
    lPtr = wf;
    rPtr = &wf[N-1];
    switch(type) {
    case WF_RECTANGLE: /* for dumb/lazy programmers */
      for(n = 0; n < N; n++)
	*(lPtr++) = 1.0;
      break;
    case WF_TRIANGLE:
      if(SHIFT) {
	arg = 1.0/(double)N;
	step = 2.0/(double)N;
      }
      else {
	arg = 0.0;
	step = 2.0/(double)(N-1);
      }
      while(lPtr < rPtr) {
	*(lPtr++) = *(rPtr--) = arg;
	arg += step;
      }
      if(lPtr == rPtr)
	*lPtr = 1.0;
      break;
    case WF_PARABOLA:
      if(SHIFT) {
	arg = 0.5/(double)N;
	step = 1.0/(double)N;
      }
      else {
	arg = 0.0;
	step = 1.0/(double)(N-1);
      }
      while(lPtr < rPtr) {
	*(lPtr++) = *(rPtr--) = 4.0 * arg * (1.0 - arg);
	arg += step;
      }
      if(lPtr == rPtr)
	*lPtr = 1.0;
      break;
    case WF_COSINE:
      if(SHIFT) {
	arg = 0.5*PI/(double)N;
	step = PI/(double)N;
      }
      else {
	arg = 0.0;
	step = PI/(double)(N-1);
      }
      while(lPtr < rPtr) {
	*(lPtr++) = *(rPtr--) = sin(arg); /* surprised ? */
	arg += step;
      }
      if(lPtr == rPtr)
	*lPtr = 1.0;
      break;
    case WF_HANN:
      if(SHIFT) {
	arg = PI/(double)N;
	step = 2.0*PI/(double)N;
      }
      else {
	arg = 0.0;
	step = 2.0*PI/(double)(N-1);
      }
      while(lPtr < rPtr) {
	*(lPtr++) = *(rPtr--) = (1.0 - cos(arg)) / 2.0;
	arg += step;
      }
      if(lPtr == rPtr)
	*lPtr = 1.0;
      break;
    case WF_COS_3:
      if(SHIFT) {
	arg = 0.5*PI/(double)N;
	step = PI/(double)N;
      }
      else {
	arg = 0.0;
	step = PI/(double)(N-1);
      }
      while(lPtr < rPtr) {
	*(lPtr++) = *(rPtr--) = (3.0*sin(arg) - sin(3.0*arg)) / 4.0;
	arg += step;
      }
      if(lPtr == rPtr)
	*lPtr = 1.0;
      break;
    case WF_COS_4:
      if(SHIFT) {
	arg = PI/(double)N;
	step = 2.0*PI/(double)N;
      }
      else {
	arg = 0.0;
	step = 2.0*PI/(double)(N-1);
      }
      while(lPtr < rPtr) {
	*(lPtr++) = *(rPtr--) = (3.0 - 4.0*cos(arg) + cos(2.0*arg)) / 8.0;
	arg += step;
      }
      if(lPtr == rPtr)
	*lPtr = 1.0;
      break;
    case WF_HAMMING:
      a0 = 0.540; /* exact Hamming has 25/46 and 21/46 */
      a1 = 0.460; /* is narrower but has higher sidelobes */
                  /* minimum Hamming for a0 = 0.53856 */
      if(SHIFT) {
	arg = PI/(double)N;
	step = 2.0*PI/(double)N;
      }
      else {
	arg = 0.0;
	step = 2.0*PI/(double)(N-1);
      }
      while(lPtr < rPtr) {
	*(lPtr++) = *(rPtr--) = a0 - a1*cos(arg);
	arg += step;
      }
      if(lPtr == rPtr)
	*lPtr = 1.0;
      break;
    case WF_BLACKMAN: /* summarize the 3-term windows */
    case WF_BLACK_X:
    case WF_BLACK_3:
    case WF_BLACK_M3:
    case WF_NUTTAL_3:
      switch(type) { /* differentiate */
      case WF_BLACKMAN:
	a0 = 0.420;
	a1 = 0.50;
	a2 = 0.080;
	break;
      case WF_BLACK_X:
	a0 = 7938.0 / 18608.0;
	a1 = 9240.0 / 18608.0;
	a2 = 1430.0 / 18608.0;
	break;
      case WF_BLACK_3:
	a0 = 0.44959;
	a1 = 0.49364;
	a2 = 0.05677;
	break;
/*       case WF_BLACK_M3: */
/* 	a0 = 0.42323; */
/* 	a1 = 0.49755; */
/* 	a2 = 0.07922; */
/* 	break; */
      case WF_BLACK_M3: /* Nuttal improvement */
	a0 = 0.4243801;
	a1 = 0.4973406;
	a2 = 0.0782793;
	break;
      case WF_NUTTAL_3:
	a0 = 0.408970;
	a1 = 0.50;
	a2 = 0.091030;
	break;
      default:
	freeWF(wf);
	setAsspMsg(AEG_ERR_BUG, "makeWF: incorrect window function");
	return(NULL);
      }
      if(SHIFT) {
	arg = PI/(double)N;
	step = 2.0*PI/(double)N;
      }
      else {
	arg = 0.0;
	step = 2.0*PI/(double)(N-1);
      }
      while(lPtr < rPtr) {
	*(lPtr++) = *(rPtr--) = a0 - a1*cos(arg) + a2*cos(2.0*arg);
	arg += step;
      }
      if(lPtr == rPtr)
	*lPtr = 1.0;
      break;
    case WF_BLACK_4: /* summarize the 4-term windows */
    case WF_BLACK_M4:
    case WF_NUTTAL_4:
      switch(type) { /* differentiate */
      case WF_BLACK_4:
/*      a0 = 0.40217;    these values are taken from Harris(1978) */
/*      a1 = 0.49703;    there must some error here because they */
/*      a2 = 0.09392;    don't add up to 1 (we're missing 0.00505) */
/*      a3 = 0.00183;     */
/* The following coefficients yield the computed values:
 * ENBW 1.794101384392  -3dB 1.702928  -6dB 2.387407  mlbw 6.500  hsll -72.89
 */
/*      a0 = 0.40217; */
/*      a1 = 0.49708; */ /* replaced 3 by 8 */
/*      a2 = 0.09892; */ /* replaced 3 by 8 */
/*      a3 = 0.00183; */
/* The following coefficients yield the computed -best- values:
 * ENBW 1.793948299864  -3dB 1.702747  -6dB 2.387185  mlbw 6.531  hsll -74.39
 *
 * Found out in 2009 that Nuttal had reached the same conclusion in 1981 !
 */
	a0 = 0.40217;
	a1 = 0.49703;
	a2 = 0.09892;  /* replaced 3 by 8 */
	a3 = 0.00188;  /* replaced 3 by 8 */
	break;
/*       case WF_BLACK_M4: */
/* 	a0 = 0.35875; */
/* 	a1 = 0.48829; */
/* 	a2 = 0.14128; */
/* 	a3 = 0.01168; */
/* 	break; */
      case WF_BLACK_M4: /* Nuttal improvement */
	a0 = 0.3635819;
	a1 = 0.4891775;
	a2 = 0.1365995;
	a3 = 0.0106411;
	break;
      case WF_NUTTAL_4:
	a0 = 0.355768;
	a1 = 0.487396;
	a2 = 0.144232;
	a3 = 0.012604;
	break;
      default:
	freeWF(wf);
	setAsspMsg(AEG_ERR_BUG, "makeWF: incorrect window function");
	return(NULL);
      }
      if(SHIFT) {
	arg = PI/(double)N;
	step = 2.0*PI/(double)N;
      }
      else {
	arg = 0.0;
	step = 2.0*PI/(double)(N-1);
      }
      while(lPtr < rPtr) {
	*(lPtr++) = *(rPtr--) = a0 - a1*cos(arg)\
	                           + a2*cos(2.0*arg)\
	                           - a3*cos(3.0*arg);
	arg += step;
      }
      if(lPtr == rPtr)
	*lPtr = 1.0;
      break;
    default:
      freeWF(wf);
      setAsspMsg(AEG_ERR_BUG, "makeWF: incorrect window function");
      wf = NULL;
    }
  }
  return(wf);
}

/*DOC

Function 'makeWF_A'

Allocates memory for "N" window coefficients and sets them according to 
the parametric window function, specified by its type number "type" and 
parameter value "alpha". See 'makeWF()' for the settings of "flags".
The function returns a pointer to the array with the coefficients. 
It returns a NULL pointer if the number of coefficients is less than 
'WF_MIN_SIZE', if no memory can be allocated or if an invalid parameter 
value or type number is given.

DOC*/

double *makeWF_A(wfunc_e type, double alpha, long N, int flags)
{
  register long    n, HN;
  register double *lPtr, *rPtr;
  int     SHIFT;
  double *wf, arg, step, x, denom, sum;
  
  if(alpha < 0.0) {
    setAsspMsg(AEB_BAD_ARGS, "makeWF_A: alpha < 0");
    return(NULL);
  }
  SHIFT = (flags == WF_PERIODIC);
  if((SHIFT && N < 2) || (!SHIFT && N < 3)) { /* not really windowing */
    setAsspMsg(AEB_BAD_ARGS, "makeWF_A: N too small");
    return(NULL);
  }
  if(flags == WF_ASYMMETRIC)
    N++;           /* make the window symmetric for simplicity's sake */
  if(type == WF_KAISER_A) {
    /* Harris uses the parameter alpha which differs by a factor of */
    /* PI from those found in filtering applications. We therefore */
    /* replace the parameter value by beta, equaling PI * alpha. */
    alpha *= PI;
    type = WF_KAISER_B;
  }
  wf = (double *)calloc((size_t)N, sizeof(double));
  if(wf == NULL) {
    setAsspMsg(AEG_ERR_MEM, "(makeWF_A)");
    return(NULL);
  }
  clrAsspMsg();
  lPtr = wf;
  rPtr = &wf[N-1];
  switch(type) {
  case WF_COS_A:  /* cos**alpha */
    if(alpha <= 0.0) {
      freeWF(wf);
      setAsspMsg(AEB_BAD_ARGS, "makeWF_A: alpha = 0");
      return(NULL);
    }
    if(SHIFT) {
      arg = 0.5*PI/(double)N;
      step = PI/(double)N;
    }
    else {
      arg = 0.0;
      step = PI/(double)(N-1);
    }
    while(lPtr < rPtr) {
      *(lPtr++) = *(rPtr--) = pow(sin(arg), alpha);
      arg += step;
    }
    if(lPtr == rPtr)
      *lPtr = 1.0;
    break;
  case WF_GEN_HAMM: /* generalised hamming */
    if(alpha <= 0.0 || alpha >= 1.0) {
      freeWF(wf);
      setAsspMsg(AEB_BAD_ARGS, "makeWF_A: alpha invalid");
      return(NULL);
    }
    if(SHIFT) {
      arg = PI/(double)N;
      step = 2.0*PI/(double)N;
    }
    else {
      arg = 0.0;
      step = 2.0*PI/(double)(N-1);
    }
    while(lPtr < rPtr) {
      *(lPtr++) = *(rPtr--) = alpha - (1.0 - alpha)*cos(arg);
      arg += step;
    }
    if(lPtr == rPtr)
      *lPtr = 1.0;
    break;
  case WF_GAUSS_A: /* Gauss(alpha) */
    if(alpha <= 0.0) {
      freeWF(wf);
      setAsspMsg(AEB_BAD_ARGS, "makeWF_A: alpha = 0");
      return(NULL);
    }
    x = alpha * alpha / 2.0;
    if(SHIFT) {
      arg = 1.0/(double)N;
      step = 2.0/(double)N;
    }
    else {
      arg = 0.0;
      step = 2.0/(double)(N-1);
    }
    while(lPtr < rPtr) {
      *(lPtr++) = *(rPtr--) = exp(-x * pow(arg - 1.0, 2.0));
      arg += step;
    }
    if(lPtr == rPtr)
      *lPtr = 1.0;
    break;
  case WF_KAISER_B: /* 'alpha' is in reality 'beta' */
/*    denom = bessi0(alpha); */
/*    for(n = 0; lPtr < rPtr; n++, lPtr++, rPtr--) { */
/*	x = (2.0 * (double)n / (double)(N-1)) - 1.0; */
/*	*lPtr = *rPtr = bessi0(alpha * sqrt(1.0 - pow(x, 2.0))) / denom; */
/*    } */
    denom = bessi0(alpha);
    if(SHIFT) {
      arg = 1.0/(double)N;
      step = 2.0/(double)N;
    }
    else {
      arg = 0.0;
      step = 2.0/(double)(N-1);
    }
    while(lPtr < rPtr) {
      *(lPtr++) = *(rPtr--) = bessi0(alpha * sqrt(arg*(2.0-arg))) / denom;
      arg += step;
    }
    if(lPtr == rPtr)
      *lPtr = 1.0;
    break;
  case WF_KBD_A:
    /* Nice window for smoothing power spectra (alpha >> 1) */
    /* provided you can live with a discrete length. */
    /* Otherwise, the cosine window might be a good alternative. */
    /* Both windows have the property that their squares add up */
    /* to 1 when overlapping by exactly 50% (N even). */
    /* The KBD window has gaussian ramps and a flattish top; for */
    /* large alpha it tends to a rectangle with half the length. */
    if(alpha <= 0.0 || flags != WF_FULL_SIZE) {
      freeWF(wf);
      setAsspMsg(AEB_BAD_ARGS, "makeWF_A");
      return(NULL);
    }
    HN = N / 2;
    alpha *= PI;
    /* integrate an asymmetrical KB window of half length */
    /* denominator discarded since it will be divided out */
    for(sum = 0.0, n = 0; n < HN; n++) {
      x = (4.0 * (double)n / (double)N) - 1.0;
      sum += bessi0(alpha * sqrt(1.0 - x*x));
      wf[n] = sum;
    }
    /* need to add the endpoint of a symmetrical KB window */
    sum += 1.0;
    /* normalize and set right-hand part */
    for(NIX; lPtr < rPtr; lPtr++, rPtr--) {
      *lPtr = *rPtr = sqrt((*lPtr) / sum);
    }
    if(lPtr == rPtr) /* allow odd length */
      *lPtr = 1.0;
    break;
  default:
    freeWF(wf);
    setAsspMsg(AEG_ERR_BUG, "makeWF_A: incorrect window function");
    wf = NULL;
  }
  return(wf);
}

/*DOC

Function 'freeWF'

Frees the memory allocated for window coefficients by "makeWF()".

DOC*/

void freeWF(double *w)
{
  if(w != NULL) free((void *)w);
  return;
}

/*DOC

Function 'mulSigWF'

Performs an in-place multiplication of the signal in "s" with the "N" window 
coefficients in "w".

DOC*/

void mulSigWF(register double *s, register double *w, register long N)
{
  register long n;
  
  if(s != NULL && w != NULL) {
    for(n = 0; n < N; n++)
      *(s++) *= *(w++);
  }
  return;
}

/*DOC

Function 'listWFs'

Prints the code name, description and main specifications of the window 
functions in the list pointed to by "list" to the stream pointed to by 
"fp". If "fp" is a NULL-pointer, printing will be to 'stdout'.

DOC*/

void listWFs(WFLIST *list, FILE *fp)
{
  int     l, m;
  WFLIST *wPtr;
  WFDATA *specs;

#ifndef WRASSP
  if(fp == NULL)
    fp = stdout;
  for(m = 0, wPtr = list; wPtr->code != NULL; wPtr++) {
    if((l=strlen(wPtr->desc)) > m)       /* search longest descriptor */
      m = l;
  }
  fprintf(fp, "\n");
  fprintf(fp, "%-10s  %-*s  eff. length  side lobes   roll-off\n\n",\
	  "code name", m, "common name");
  for(wPtr = list; wPtr->code != NULL; wPtr++) {
    specs = wfSpecs(wPtr->type);            /* get the specifications */
    if(specs != NULL)                            /* valid type number */
      fprintf(fp, "%-10s  %-*s  %10.7f   < %5.1f dB  %3.0f dB/oct\n",\
	      wPtr->code, m, wPtr->desc,\
	      1.0/(specs->enbw), specs->hsll, specs->roff);
  }
  fprintf(fp, "\n");
#endif
  return;
}

/*DOC

Function 'wfCohGain'

Computes the coherent gain (mean) of the window function of length "N" 
pointed to by "w".

DOC*/

double wfCohGain(register double *w, register long N)
{
  register long n;
  double sum;

  if(w == NULL || N < 1)
    return(0.0);
  for(sum = 0.0, n = 0; n < N; n++)
    sum += *(w++);
  return(sum/(double)N);
}

/*DOC

Function 'wfIncGain'

Computes the incoherent gain (power) of the window function of length 
"N" pointed to by "w".

DOC*/

double wfIncGain(register double *w, register long N)
{
  register long n;
  double sum;

  if(w == NULL || N < 1)
    return(0.0);
  for(sum = 0.0, n = 0; n < N; n++, w++)
    sum += ((*w) * (*w));
  return(sum/(double)N);
}

/*DOC

Function 'wfENBW'

Computes the (normalized) Equivalent Noise BandWidth of the window 
function of length "N" pointed to by "w".

DOC*/

double wfENBW(register double *w, register long N)
{
  register long n;
  double sum, sqr;

  if(w == NULL || N < 1)
    return(0.0);
  for(sqr = sum = 0.0, n = 0; n < N; n++) {
    sqr += ((*w) * (*w));
    sum += *(w++);
  }
  return((double)N * sqr/(sum*sum));
}

/*DOC

Function 'kaiserBeta'

Estimates the parameter "beta" of a Kaiser-Bessel window for an
attenuation (> 0) of at least "att" dB in FIR design applications.

From: IEEE Press "Programs for Digital Signal Processing", Section 5.2

DOC*/

double kaiserBeta(double att)
{
  double beta;

  if(att > 50.0) {
    beta = 0.1102 * (att - 8.7);
  }
  else if(att > 20.96) {
    beta = 0.58417 * pow(att - 20.96, 0.4) + 0.07886 * (att - 20.96);
  }
  else {
    beta = 0.0;
  }
  return(beta);
}

/*DOC

Function 'kaiserLength'

Estimates the length of a Kaiser-Bessel window for a transition band of 
"trb" Hz at "sfr" sampling rate and an attenuation of at least "att" dB 
in FIR design applications. The length will be rounded upwards to an odd 
value.

From: IEEE Press "Programs for Digital Signal Processing", Section 5.2

DOC*/

long kaiserLength(double sfr, double trb, double att)
{
  long L;

  if(att > 20.96) {
    L = (long)ceil(sfr * (att - 7.95) / (14.36 * trb));
  }
  else { /* beta = 0 --> rectangular window */
    L = (long)ceil(sfr * 0.9215 / trb);         /* occasionally found */
  }
  if(EVEN(L)) L++;
  return(L);
}

/*DOC

Function 'bandwidth2frameSize'

Converts "bandwidth" in Hz to the corresponding frame size in samples, 
taking  the bandwidth increase due to the window function given by its 
number "type" into account. If the number of FFT points "nFFT" is 
greater than zero this value will determine the maximum size, whereas 
the minimum size is then defined by the smallest length of an FFT 
(MIN_FFT as defined in "asspdsp.h").
The function returns -1 upon an argument error.

DOC*/

long bandwidth2frameSize(double bandwidth, wfunc_e type,\
			 double sampFreq, long nFFT)
{
  long    frameSize;
  double  enbw;
  WFDATA *specs;
  
  if(sampFreq <= 0.0 || (bandwidth <= 0.0 && nFFT <= 0))
    return(-1);
  if(bandwidth <= 0.0)
    return(nFFT);
  if((specs=wfSpecs(type)) != NULL)
    enbw = specs->enbw;
  else
    enbw = 1.0;
  frameSize = (long)floor(enbw * sampFreq / bandwidth + 0.5);
  if(nFFT > 0) {
    if(frameSize > nFFT)
      frameSize = nFFT;
    if(frameSize < MIN_NFFT)
      frameSize = MIN_NFFT;
  }
  return(frameSize);
}

/*DOC

Function 'frameSize2bandwidth'

Converts "frameSize" in samples to the corresponding bandwidth in Hz, 
taking  the bandwidth increase due to the window function given by 
its number "type" into account. If the number of FFT points "nFFT" is 
greater than zero this value will determine the maximum size, whereas 
the minimum size is then defined by the smallest length of an FFT 
(MIN_FFT as defined in "asspdsp.h").
The function returns -1 upon an argument error.

DOC*/

double frameSize2bandwidth(long frameSize, wfunc_e type,\
			   double sampFreq, long nFFT)
{
  double  bandwidth, enbw;
  WFDATA *specs;
  
  if(sampFreq <= 0.0 || frameSize <= 0)
    return(-1.0);
  if((specs=wfSpecs(type)) != NULL)
    enbw = specs->enbw;
  else
    enbw = 1.0;
  if(nFFT > 0) {
    if(frameSize > nFFT)
      frameSize = nFFT;
    if(frameSize < MIN_NFFT)
      frameSize = MIN_NFFT;
  }
  bandwidth = enbw * sampFreq / (double)frameSize;
  return(bandwidth);
}
