#include "wrassp.h"
#include <asspdsp.h> //windows

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
