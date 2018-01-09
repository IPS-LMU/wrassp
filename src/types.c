#include "wrassp.h"
#include <asspdsp.h>            // windows
#include <spectra.h>            // lp types, spect types


/*
 * This function makes the names of window functions as defined in ASSP
 * available to R 
 */
SEXP AsspWindowTypes_()
{
    WFLIST         *wPtr = wfShortList; // defined in assp
    int             n = 0,
        i = 0;
    SEXP            wlist;

    while (wPtr->code != NULL) {
        n++;
        wPtr++;
    }
    wPtr = wfShortList;

    PROTECT(wlist = allocVector(STRSXP, n));
    while (wPtr->code != NULL) {
        SET_STRING_ELT(wlist, i, mkChar(wPtr->code));
        wPtr++, i++;
    }

    UNPROTECT(1);
    return wlist;
}

/*
 * This function makes the names of the types of linear prediction used in 
 * ASSP available to R 
 */
SEXP
AsspLpTypes_()
{
    LP_TYPE        *lPtr = lpType;
    SEXP            result;
    int             n = 0,
        i = 0;

    while (lPtr->ident != NULL) {
        lPtr++;
        n++;
    }

    lPtr = lpType;
    PROTECT(result = allocVector(STRSXP, n));
    while (lPtr->ident != NULL) {
        SET_STRING_ELT(result, i, mkChar(lPtr->ident));
        lPtr++;
        i++;
    }
    UNPROTECT(1);
    return (result);
}

/*
 * This function makes the names of the types of spectral analysis used in 
 * ASSP available to R 
 */
SEXP AsspSpectTypes_()
{
    SPECT_TYPE     *sPtr = spectType;
    SEXP            result;
    int             i = 0,
        n = 0;

    while (sPtr->ident != NULL) {
        sPtr++;
        n++;
    }

    sPtr = spectType;
    PROTECT(result = allocVector(STRSXP, n));
    while (sPtr->ident != NULL) {
        SET_STRING_ELT(result, i, mkChar(sPtr->ident));
        sPtr++;
        i++;
    }
    UNPROTECT(1);
    return (result);
}
