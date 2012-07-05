/*********************************************************** auconv.c **
*                                                                      *
* Conversion functions for non-standard audio integer types.           *
*                                                                      *
*-- Modifications -----------------------------------------------------*
* Replaced ANSI-C integral types by data model independent ones.       *
* Michel Scheffers, Feb. 21 2006                                       *
*                                                                      *
***********************************************************************/


#include <math.h>       /* ldexp() frexp() */
#include <inttypes.h>   /* uint8_t etc. */

#include <auconv.h>     /* alaw_t ulaw_t binoff8_t binoff16_t */
#include <asspendian.h>     /* nifty byte-order stuff */

#define SIGN_8BIT (0x80) /* sign bit in byte */

/***********************************************************************
* CCITT G.711  u-law and A-law conversion                              *
***********************************************************************/

/*
 * This source code is a product of Sun Microsystems, Inc. and is provided
 * for unrestricted use.  Users may copy or modify this source code without
 * charge.
 *
 * SUN SOURCE CODE IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING
 * THE WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun source code is provided with no support and without any obligation on
 * the part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THIS SOFTWARE
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

/*
 * Functions Snack_Lin2Alaw, Snack_Lin2Mulaw have been updated to correctly
 * convert unquantized 16 bit values.
 * Tables for direct u- to A-law and A- to u-law conversions have been
 * corrected.
 * Borge Lindberg, Center for PersonKommunikation, Aalborg University.
 * bli@cpk.auc.dk
 *
 */
 
/*
 * This version has been adapted from the snack sources:
 * http://www.speech.kth.se/snack/
 * Modified by: Michel Scheffers  Dec. 17 2004
 *              Institut fuer Phonetik und digitale Sprachverarbeitung
 *              Christian-Albrechts University of Kiel, 24098 Germany
 */

#define LAW_QUANT_MASK (0x0F) /* quantization field mask */
#define LAW_NUM_SEGS   (8)    /* number of segments */
#define LAW_SEG_SHIFT  (4)    /* shift for segment number */
#define LAW_SEG_MASK   (0x70) /* segment field mask */

static int16_t seg_aend[LAW_NUM_SEGS] = { 0x1F,  0x3F,  0x7F,  0xFF,
					  0x1FF, 0x3FF, 0x7FF, 0xFFF };
static int16_t seg_uend[LAW_NUM_SEGS] = { 0x3F,  0x7F,  0xFF,  0x1FF,
					  0x3FF, 0x7FF, 0xFFF, 0x1FFF };

/* copy from CCITT G.711 specifications */
static alaw_t _u2a[128] = {         /* u- to A-law conversions */
	1,	1,	2,	2,	3,	3,	4,	4,
	5,	5,	6,	6,	7,	7,	8,	8,
	9,	10,	11,	12,	13,	14,	15,	16,
	17,	18,	19,	20,	21,	22,	23,	24,
	25,	27,	29,	31,	33,	34,	35,	36,
	37,	38,	39,	40,	41,	42,	43,	44,
	46,	48,	49,	50,	51,	52,	53,	54,
	55,	56,	57,	58,	59,	60,	61,	62,
	64,	65,	66,	67,	68,	69,	70,	71,
	72,	73,	74,	75,	76,	77,	78,	79,
/* corrected:
	81,	82,	83,	84,	85,	86,	87,	88, 
   should be: */
	80,	82,	83,	84,	85,	86,	87,	88,
	89,	90,	91,	92,	93,	94,	95,	96,
	97,	98,	99,	100,	101,	102,	103,	104,
	105,	106,	107,	108,	109,	110,	111,	112,
	113,	114,	115,	116,	117,	118,	119,	120,
	121,	122,	123,	124,	125,	126,	127,	128
};

static ulaw_t _a2u[128] = {	   /* A- to u-law conversions */
	1,	3,	5,	7,	9,	11,	13,	15,
	16,	17,	18,	19,	20,	21,	22,	23,
	24,	25,	26,	27,	28,	29,	30,	31,
	32,	32,	33,	33,	34,	34,	35,	35,
	36,	37,	38,	39,	40,	41,	42,	43,
	44,	45,	46,	47,	48,	48,	49,	49,
	50,	51,	52,	53,	54,	55,	56,	57,
	58,	59,	60,	61,	62,	63,	64,	64,
	65,	66,	67,	68,	69,	70,	71,	72,
/* corrected:
	73,	74,	75,	76,	77,	78,	79,	79,
   should be: */
	73,	74,	75,	76,	77,	78,	79,	80,
	80,	81,	82,	83,	84,	85,	86,	87,
	88,	89,	90,	91,	92,	93,	94,	95,
	96,	97,	98,	99,	100,	101,	102,	103,
	104,	105,	106,	107,	108,	109,	110,	111,
	112,	113,	114,	115,	116,	117,	118,	119,
	120,	121,	122,	123,	124,	125,	126,	127
};

static uint16_t getLawSeg(register int16_t val, 
			  register int16_t *table, 
			  register uint16_t size)
{
  register uint16_t i;
  
  for(i = 0; i < size; i++) {
    if(val <= *(table++))
      break;
  }
  return(i);
}

/*
 * int16_to_alaw() - Convert a 16-bit linear PCM value to 8-bit A-law.
 *
 *    Linear Input Code    Compressed Code
 *    -----------------    ---------------
 *    0000000wxyza         000wxyz
 *    0000001wxyza         001wxyz
 *    000001wxyzab         010wxyz
 *    00001wxyzabc         011wxyz
 *    0001wxyzabcd         100wxyz
 *    001wxyzabcde         101wxyz
 *    01wxyzabcdef         110wxyz
 *    1wxyzabcdefg         111wxyz
 *
 * For further information see John C. Bellamy's Digital Telephony, 1982,
 * John Wiley & Sons, pps 98-111 and 472-476.
 */

alaw_t int16_to_alaw(register int16_t pcm_val)
/* input value in 2's complement, 16-bit range */
{
  register alaw_t   a_val, mask;
  register uint16_t seg;

  pcm_val /= 8;     /* shift to 13-bit range */
  if(pcm_val >= 0) {
    mask = 0xD5;    /* sign (7th) bit = 1 */
  }
  else {
    mask = 0x55;    /* sign bit = 0 */
    pcm_val = -pcm_val - 1;
  }
  /* Convert the scaled magnitude to segment number. */
  seg = getLawSeg(pcm_val, seg_aend, LAW_NUM_SEGS);
  /* Combine the sign, segment, and quantization bits. */
  if(seg >= LAW_NUM_SEGS) /* out of range, take maximum value */
    a_val = 0x7F;
  else {
    a_val = (alaw_t)(seg << LAW_SEG_SHIFT);
    if(seg < 2)
      a_val |= ((pcm_val >> 1) & LAW_QUANT_MASK);
    else
      a_val |= ((pcm_val >> seg) & LAW_QUANT_MASK);
  }
  return(a_val ^ mask);
}

/*
 * alaw_to_int16() - Convert an 8-bit A-law value to 16-bit linear PCM.
 */

int16_t alaw_to_int16(register alaw_t a_val)
{
  register uint16_t seg;
  register int16_t  pcm_val;

  a_val ^= 0x55;
  pcm_val = (int16_t)(a_val & LAW_QUANT_MASK) << LAW_SEG_SHIFT;
  seg = ((uint16_t)a_val & LAW_SEG_MASK) >> LAW_SEG_SHIFT;
  switch (seg) {
  case 0:
    pcm_val += 8;
    break;
  case 1:
    pcm_val += 0x108;
    break;
  default:
    pcm_val += 0x108;
    pcm_val <<= (seg - 1);
  }
  return((a_val & SIGN_8BIT) ? pcm_val : -pcm_val);
}

/*
 * int16_to_ulaw() - Convert a 16-bit linear PCM value to 8-bit u-law.
 *
 * In order to simplify the encoding process, the original linear magnitude
 * is biased by adding 33 which shifts the encoding range from (0 - 8158) to
 * (33 - 8191). The result can be seen in the following encoding table:
 *
 *    Biased Linear Input Code    Compressed Code
 *    ------------------------    ---------------
 *    00000001wxyza               000wxyz
 *    0000001wxyzab               001wxyz
 *    000001wxyzabc               010wxyz
 *    00001wxyzabcd               011wxyz
 *    0001wxyzabcde               100wxyz
 *    001wxyzabcdef               101wxyz
 *    01wxyzabcdefg               110wxyz
 *    1wxyzabcdefgh               111wxyz
 *
 * Each biased linear code has a leading 1 which identifies the segment
 * number. The value of the segment number is equal to 7 minus the number
 * of leading 0's. The quantization interval is directly available as the
 * four bits wxyz.  The trailing bits (a - h) are ignored.
 *
 * Ordinarily the complement of the resulting code word is used for
 * transmission, and so the code word is complemented before it is returned.
 *
 * For further information see John C. Bellamy's Digital Telephony, 1982,
 * John Wiley & Sons, pps 98-111 and 472-476.
 */

#define ULAW_BIAS (0x84)  /* bias for linear code */
#define ULAW_CLIP (8159)

ulaw_t int16_to_ulaw(register int16_t pcm_val)
/* input value in 2's complement, 16-bit range */
{
  register ulaw_t   u_val, mask;
  register uint16_t seg;

  /* Get the sign and the magnitude of the value. */
  pcm_val /= 4;            /* clip to range -8192 ... +8191 */
  if(pcm_val < 0) {
    pcm_val = -pcm_val;
    mask = 0x7F;
  }
  else {
    mask = 0xFF;
  }
  if(pcm_val > ULAW_CLIP )
    pcm_val = ULAW_CLIP;   /* clip magnitude */
  pcm_val += (ULAW_BIAS >> 2);
  /* Convert the scaled magnitude to segment number. */
  seg = getLawSeg(pcm_val, seg_uend, LAW_NUM_SEGS);
  /*
   * Combine the sign, segment, quantization bits;
   * and complement the code word.
   */
  if(seg >= LAW_NUM_SEGS)  /* out of range, take maximum value */
    u_val = 0x7F;
  else
    u_val = (ulaw_t)(seg << LAW_SEG_SHIFT)\
          | ((pcm_val >> (seg + 1)) & LAW_QUANT_MASK);
  return(u_val ^ mask);
}

/*
 * ulaw_to_int16() - Convert an 8-bit u-law value to 16-bit linear PCM.
 *
 * First, a biased linear code is derived from the code word. An unbiased
 * output can then be obtained by subtracting 33 from the biased code.
 *
 * Note that this function expects to be passed the complement of the
 * original code word. This is in keeping with ISDN conventions.
 */
int16_t ulaw_to_int16(register ulaw_t u_val)
{
  register int16_t pcm_val;

  /* Complement to obtain normal u-law value. */
  u_val = ~u_val;
  /*
   * Extract and bias the quantization bits. Then
   * shift up by the segment number and subtract out the bias.
   */
  pcm_val = ((uint16_t)(u_val & LAW_QUANT_MASK) << 3) + ULAW_BIAS;
  pcm_val <<= ((u_val & LAW_SEG_MASK) >> LAW_SEG_SHIFT);
  if(u_val & SIGN_8BIT)
    pcm_val = ULAW_BIAS - pcm_val;
  else
    pcm_val -= ULAW_BIAS;
  return(pcm_val);
}

/*
 * A-law to u-law conversion
 */
ulaw_t alaw_to_ulaw(register alaw_t a_val)
{
  register ulaw_t u_val;

  /* a_val &= 0xFF;     pretty useless unless char is more than 8 bit */
  if(a_val & SIGN_8BIT)
    u_val = 0xFF ^ _a2u[a_val ^ 0xD5];
  else
    u_val = 0x7F ^ _a2u[a_val ^ 0x55];
  return(u_val);
}

/*
 * u-law to A-law conversion
 */
alaw_t ulaw_to_alaw(register ulaw_t u_val)
{
  register alaw_t a_val;

  /* u_val &= 0xFF;     pretty useless unless char is more than 8 bit */
  if(u_val & SIGN_8BIT)
    a_val = 0xD5 ^ (_u2a[0xFF ^ u_val] - 1);
  else
    a_val = 0x55 ^ (_u2a[0x7F ^ u_val] - 1);
  return(a_val);
}

/***********************************************************************
*                                                                      *
* The following functions are part of the Advanced Speech Signal       *
* Processor library.                                                   *
*                                                                      *
* Copyright (C) 2004 - 2008  Michel Scheffers                          *
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
* Conversion functions for 8, 9-16 and 17-24 bit audio data which      *
* cannot be converted using type casts because unsigned values are     *
* taken to be encoded in binary offset, signed ones in 2's complement. *
* The argument 'numBits' for the functions converting unsigned 2- and  *
* 3-byte data give the number of the lower bits used (typically the    *
* resolution of the A/D coverter).                                     *
*                                                                      *
* NOTE: These functions do NOT test the validity of their arguments    *
* nor do they rescale values. The calling function must ensure that    *
* input values are within the range of the output type and, if desired,*
* upscale the output values.                                           *
*                                                                      *
* Author: Michel T.M. Scheffers                                        *
*                                                                      *
***********************************************************************/

int8_t binoff8_to_int8(register binoff8_t u8)
{
  return((int8_t)binoff8_to_int16(u8));
}

int16_t binoff8_to_int16(register binoff8_t u8)
{
  register int16_t offset, i16;

  offset = 1 << 7; /* binary offset -> 2's complement */
  i16 = u8;
  return(i16 - offset);
}

int16_t binoff16_to_int16(register binoff16_t u16,
			  register uint16_t numBits)
{
  register int32_t offset, i32;

  if(numBits < 1 || numBits > 15)
    offset = 0;
  else
    offset = 1L << (numBits - 1);
  i32 = u16;
  return((int16_t)(i32 - offset));
}

int32_t int24_to_int32(register uint8_t *i24Ptr)
{
  register uint8_t *bPtr;
  register int      i;
  int32_t i32=0; /* make sure all bytes are set to zero */
  ENDIAN  sysEndian={MSB};

  bPtr = (uint8_t *)&i32;
  if(MSBFIRST(sysEndian)) {
    if(*i24Ptr & SIGN_8BIT) /* extend sign */
      *bPtr = 0xFF;
    bPtr++;
    for(i = 0; i < 3; i++)
      *(bPtr++) = *(i24Ptr++);
  }
  else {
    for(i = 0; i < 3; i++)
      *(bPtr++) = i24Ptr[i];
    if(i24Ptr[2] & SIGN_8BIT)
      *bPtr = 0xFF;
  }
  return(i32);
}

int32_t binoff24_to_int32(register uint8_t *i24Ptr,\
			  register uint16_t numBits)
{
  register uint8_t *bPtr;
  register int      i;
  register int32_t  offset;
  int32_t i32=0; /* make sure all bytes are set to zero */
  ENDIAN  sysEndian={MSB};

  if(numBits < 1 || numBits > 23)
    offset = 0;
  else
    offset = 1L << (numBits - 1);
  bPtr = (uint8_t *)&i32;
  if(MSBFIRST(sysEndian))
    bPtr++;                /* skip MSB */
  for(i = 0; i < 3; i++)
    *(bPtr++) = *(i24Ptr++);
  return(i32 - offset);
}

void int32_to_int24(int32_t i32, register uint8_t *i24Ptr)
/* input value MUST be in 24-bit range */
{
  register uint8_t *bPtr;
  register int      i;
  ENDIAN sysEndian={MSB};

  bPtr = (uint8_t *)&i32;
  if(MSBFIRST(sysEndian))
    bPtr++;
  for(i = 0; i < 3; i++)
    *(i24Ptr++) = *(bPtr++);
  return;
}
