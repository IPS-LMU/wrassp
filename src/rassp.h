#ifndef _RASSP
#define _RASSP

#include <R.h>
#include <Rinternals.h>
#include <dataobj.h>


SEXP getDObj(SEXP fname);
SEXP getDObjTracks(SEXP dobj);
SEXP getDObjTrackData(DOBJ *data, DDESC *desc);
static void DObjFinalizer(SEXP dPtr);


#endif // _RASSP
