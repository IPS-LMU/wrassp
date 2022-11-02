#include <R.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include <R_ext/Rdynload.h>

/*
The following name(s) appear with different usages
e.g., with different numbers of arguments:

performAssp

This needs to be resolved in the tables and any declarations.
*/

/* FIXME: 
Check these declarations against the C/Fortran source code.
*/

/* .Call calls */
extern SEXP AsspLpTypes_(void);
extern SEXP AsspSpectTypes_(void);
extern SEXP AsspWindowTypes_(void);
extern SEXP writeDObj_(SEXP, SEXP);

/* .External calls */
extern SEXP getDObj2(SEXP);
extern SEXP performAssp(SEXP);
//extern SEXP performAssp(SEXP);
//extern SEXP performAssp(SEXP);
//extern SEXP performAssp(SEXP);
//extern SEXP performAssp(SEXP);
//extern SEXP performAssp(SEXP);
//extern SEXP performAssp(SEXP);

static const R_CallMethodDef CallEntries[] = {
  {"AsspLpTypes_",     (DL_FUNC) &AsspLpTypes_,     0},
  {"AsspSpectTypes_",  (DL_FUNC) &AsspSpectTypes_,  0},
  {"AsspWindowTypes_", (DL_FUNC) &AsspWindowTypes_, 0},
  {"writeDObj_",       (DL_FUNC) &writeDObj_,       2},
  {NULL, NULL, 0}
};

static const R_ExternalMethodDef ExternalEntries[] = {
  {"getDObj2",    (DL_FUNC) &getDObj2,     4},
  {"performAssp", (DL_FUNC) &performAssp,  -1}, // -1 specifies a variable number of argumetns
  //{"performAssp", (DL_FUNC) &performAssp,  8},
  //{"performAssp", (DL_FUNC) &performAssp, 11},
  //{"performAssp", (DL_FUNC) &performAssp, 12},
  //{"performAssp", (DL_FUNC) &performAssp, 14},
  //{"performAssp", (DL_FUNC) &performAssp, 15},
  //{"performAssp", (DL_FUNC) &performAssp, 16},
  //{"performAssp", (DL_FUNC) &performAssp, 19},
  {NULL, NULL, 0}
};

void R_init_wrassp(DllInfo *dll)
{
  R_registerRoutines(dll, NULL, CallEntries, NULL, ExternalEntries);
  R_useDynamicSymbols(dll, FALSE);
}
