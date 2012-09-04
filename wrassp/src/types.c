#include "wrassp.h"
#include <asspdsp.h> //windows
#include <spectra.h> //lp types, spect types


SEXP AsspWindowTypes () {
  WFLIST *wPtr = wfShortList; //defined in assp
  int n = 0, i = 0;
  SEXP elem, wlist;
  
  while (wPtr->code != NULL) 
    {
      n++;
      wPtr++;
    }
  wPtr = wfShortList;

  PROTECT(wlist = allocVector(STRSXP, n));
  while (wPtr->code != NULL) 
    {
      SET_STRING_ELT(wlist, i, mkChar(wPtr->code));
      wPtr++, i++;
    }
  
  UNPROTECT(1);
  return wlist;
}


SEXP AsspLpTypes()
{
    LP_TYPE *lPtr = lpType;
    SEXP result;
    int n = 0, i = 0;

    while (lPtr->ident != NULL) {
	lPtr++;
	n++;
    }
    
    PROTECT(result = allocVector(STRSXP, n));
    while (lPtr->ident != NULL) {
      SET_STRING_ELT(result, i, mkChar(lPtr->ident));
      lPtr++; i++;
    }
    UNPROTECT(1);
    return(result);
}

SEXP AsspSpectTypes ()
{
    SPECT_TYPE *sPtr = spectType;
    SEXP result;
    int i = 0, n = 0;
    
    while (sPtr->ident != NULL) {
	sPtr++;
	n++;
    }
    
    PROTECT(result = allocVector(STRSXP, n));
    while (lPtr->ident != NULL) {
      SET_STRING_ELT(result, i, mkChar(sPtr->ident));
      lPtr++; i++;
    }
    UNPROTECT(1);
    return(result);
}


/* int winfuncs_Info() */
/* { */
/*     // code mostly taken from listWFs() in libassp's winfuncs.c */
/*     Tcl_Obj *all = Tcl_NewListObj(0, NULL); */
/*     Tcl_Obj *wInfo; */
/*     WFLIST *wPtr; */
/*     WFDATA *specs; */

/*     for (wPtr = wfShortList; wPtr->code != NULL; wPtr++) { */
/* 	specs = wfSpecs(wPtr->type);	//get the specs */
/* 	if (specs != NULL) { */
/* 	    wInfo = Tcl_NewListObj(0, NULL); */
/* 	    if (Tcl_ListObjAppendElement(interp, wInfo, */
/* 					 Tcl_NewStringObj(wPtr->code, -1))) */
/* 	      return TCL_ERROR; */
/* 	    if (Tcl_ListObjAppendElement(interp, wInfo, */
/* 					 Tcl_NewStringObj(wPtr->desc, -1))) */
/* 		return TCL_ERROR; */
/* 	    if (Tcl_ListObjAppendElement(interp, wInfo, */
/* 					 Tcl_NewDoubleObj(1.0 / */
/* 							  specs->enbw))) */
/* 		return TCL_ERROR; */
/* 	    if (Tcl_ListObjAppendElement(interp, wInfo, */
/* 					 Tcl_NewDoubleObj(specs->hsll))) */
/* 		return TCL_ERROR; */
/* 	    if (Tcl_ListObjAppendElement(interp, wInfo, */
/* 					 Tcl_NewDoubleObj(specs->roff))) */
/* 		return TCL_ERROR; */
/* 	    if (Tcl_ListObjAppendElement(interp, all, wInfo) != TCL_OK) */
/* 		return TCL_ERROR; */
/* 	} */
/*     } */
/*     Tcl_SetObjResult(interp, all); */
/*     return TCL_OK; */
/* } */
