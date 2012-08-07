/* Copyright (C) 1988-1991 Apple Computer, Inc.
 * All rights reserved.
 *
 * Machine-independent I/O routines for IEEE floating-point numbers.
 *
 */

/*
 * Modified Oct. 2007 by Michel Scheffers (ms@ipds.uni-kiel.de)
 * - used data model independent integral types
 */

#ifndef _IEEE_H
#define _IEEE_H

#include <inttypes.h>   /* uint8_t */

#include <dlldef.h>     /* ASSP_EXTERN */

#ifdef __cplusplus
extern "C" {
#endif

#define XFPSIZE 10 /* number of bytes in IEEE 754 extended floating point */

ASSP_EXTERN void ConvertToIeeeExtended(double num, uint8_t *bytes);
ASSP_EXTERN double ConvertFromIeeeExtended(uint8_t *bytes);

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* _IEEE_H */
