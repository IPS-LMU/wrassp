/************************************************************* ieee.c **
* IEEE 754 extended precision floating point conversion                *
*                                                                      *
* BEWARE ! These functions assume the extended float variable to have  *
*          MSB first !                                                 *
***********************************************************************/

/*
 * Copyright (C) 1988-1991 Apple Computer, Inc.
 * All rights reserved.
 *
 * Machine-independent I/O routines for IEEE floating-point numbers.
 *
 * NaN's and infinities are converted to HUGE_VAL or HUGE, which
 * happens to be infinity on IEEE machines.  Unfortunately, it is
 * impossible to preserve NaN's in a machine-independent way.
 * Infinities are, however, preserved on IEEE machines.
 *
 * These routines have been tested on the following machines:
 *    Apple Macintosh, MPW 3.1 C compiler
 *    Apple Macintosh, THINK C compiler
 *    Silicon Graphics IRIS, MIPS compiler
 *    Cray X/MP and Y/MP
 *    Digital Equipment VAX
 *
 *
 * Implemented by Malcolm Slaney and Ken Turkowski.
 *
 * Malcolm Slaney contributions during 1988-1990 include big- and little-
 * endian file I/O, conversion to and from Motorola's extended 80-bit
 * floating-point format, and conversions to and from IEEE single-
 * precision floating-point format.
 *
 * In 1991, Ken Turkowski implemented the conversions to and from
 * IEEE double-precision format, added more precision to the extended
 * conversions, and accommodated conversions involving +/- infinity,
 * NaN's, and denormalized numbers.
 *
 */

/*
 * Modified Oct. 2007 by Michel Scheffers (ms@ipds.uni-kiel.de)
 * - used data model independent integral types
 */

#include <math.h>     /* HUGE frexp() ldexp() floor() */
#include <inttypes.h>

#include <ieee.h>

#ifndef HUGE_VAL
# define HUGE_VAL HUGE
#endif /*HUGE_VAL*/

#define FloatToUnsigned(f) ((uint32_t)(((int32_t)(f - 2147483648.0))\
                           + 2147483647L) + 1)

void ConvertToIeeeExtended(double num, uint8_t *bytes)
{
  int      argExp;
  int16_t  sign, expon;
  uint32_t hiMant, loMant;
  double   fMant, fsMant;

  if(num < 0.0) {
    sign = 0x8000;
    num = -num;
  }
  else {
    sign = 0;
  }

  if(num == 0.0) {
    expon = 0; 
    hiMant = loMant = 0;
  }
  else {
    fMant = frexp(num, &argExp);
    expon = (int16_t)argExp;
    if((expon > 16384) || !(fMant < 1)) {          /* infinity or NaN */
      expon = sign | 0x7FFF;                              /* infinity */
      hiMant = loMant = 0;
    }
    else {                                                  /* finite */
      expon += 16382;
      if(expon < 0) {                                 /* denormalized */
	fMant = ldexp(fMant, (int)expon);
	expon = 0;
      }
      expon |= sign;
      fMant = ldexp(fMant, 32);          
      fsMant = floor(fMant); 
      hiMant = FloatToUnsigned(fsMant);
      fMant = ldexp(fMant - fsMant, 32); 
      fsMant = floor(fMant); 
      loMant = FloatToUnsigned(fsMant);
    }
  }
  bytes[0] = (uint8_t)((expon >> 8) & 0x00FF);
  bytes[1] = (uint8_t)(expon & 0x00FF);
  bytes[2] = (uint8_t)((hiMant >> 24) & 0x00FF);
  bytes[3] = (uint8_t)((hiMant >> 16) & 0x00FF);
  bytes[4] = (uint8_t)((hiMant >> 8) & 0x00FF);
  bytes[5] = (uint8_t)(hiMant & 0x00FF);
  bytes[6] = (uint8_t)((loMant >> 24) & 0x00FF);
  bytes[7] = (uint8_t)((loMant >> 16) & 0x00FF);
  bytes[8] = (uint8_t)((loMant >> 8) & 0x00FF);
  bytes[9] = (uint8_t)(loMant & 0x00FF);
  return;
}

#define UnsignedToFloat(u) (((double)((int32_t)(u - 2147483647L - 1)))\
                           + 2147483648.0)

double ConvertFromIeeeExtended(uint8_t *bytes /* LCN */)
{
  int16_t  expon;
  uint32_t hiMant, loMant;
  double   f;
  
  expon = (((int16_t)bytes[0] & 0x007F) << 8) | ((int16_t)bytes[1] & 0x00FF);
  hiMant = (((uint32_t)bytes[2] & 0x00FF) << 24)\
         | (((uint32_t)bytes[3] & 0x00FF) << 16)\
         | (((uint32_t)bytes[4] & 0x00FF) << 8)\
         | ((uint32_t)bytes[5] & 0x00FF);
  loMant = (((uint32_t)bytes[6] & 0x00FF) << 24)\
         | (((uint32_t)bytes[7] & 0x00FF) << 16)\
         | (((uint32_t)bytes[8] & 0x00FF) << 8)\
         | ((uint32_t)bytes[9] & 0x00FF);
  
  if(expon == 0 && hiMant == 0 && loMant == 0) {
    f = 0;
  }
  else {
    if(expon == 0x7FFF) {                          /* infinity or NaN */
      f = HUGE_VAL;
    }
    else {
      expon -= 16383;
/*       f = ldexp(UnsignedToFloat(hiMant), expon-=31); */
/*       f += ldexp(UnsignedToFloat(loMant), expon-=32); */
      expon -= 31;
      f = ldexp(UnsignedToFloat(hiMant), (int)expon);
      expon -= 32;
      f += ldexp(UnsignedToFloat(loMant), (int)expon);
    }
  }
  if(bytes[0] & 0x80)
    return(-f);
  else
    return(f);
}

