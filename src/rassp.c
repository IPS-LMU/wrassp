#include "rassp.h"
#include <R_ext/Rdynload.h>

void R_init_rassp(DllInfo *info)
{
     R_CallMethodDef callMethods[] = {
	  {"getDObj", (DL_FUNC) &getDObj, 1},
	  {"getDObjTracks", (DL_FUNC) & getDObjTracks, 1},
	  {NULL, NULL, 0}
     };
     R_registerRoutines(info, NULL, callMethods, NULL, NULL);
}

void R_unload_rassp(DllInfo *info)
{
}

