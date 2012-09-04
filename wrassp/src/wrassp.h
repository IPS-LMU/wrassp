#ifndef _WRASSP
#define _WRASSP

#include <R.h>
#include <Rinternals.h>
#include <dataobj.h>
#include <asspana.h>


SEXP getDObj(SEXP fname);
SEXP getDObjTracks(SEXP dobj);
SEXP getDObjTrackData(DOBJ *data, DDESC *desc);
static void DObjFinalizer(SEXP dPtr);

SEXP performAssp(SEXP args);
SEXP showArgs(SEXP args);

SEXP AsspWindowList();
/* Option handling */

typedef enum TclAsspFuncs
{
  TAF_NONE,
  TAF_ACFANA,
  TAF_AFDIFF,
  TAF_AFFILTER,
  TAF_KSV_PITCH, // f0ana
  TAF_FOREST,
  TAF_MHS_PITCH,
  TAF_RFCANA,
  TAF_RMSANA,
  TAF_SPECTRUM,
  TAF_ZCRANA
} tclAsspFunc_e;

typedef enum AsspFuncs
{
  AF_NONE,
  AF_ACFANA,
  AF_AFDIFF,
  AF_AFFILTER,
  AF_KSV_PITCH, // f0ana
  AF_FOREST,
  AF_MHS_PITCH,
  AF_RFCANA,
  AF_RMSANA,
  AF_SPECTRUM,
  AF_ZCRANA
} AsspFunc_e;

typedef int (setDefProc) (AOPTS * opt);
typedef DOBJ *(computeProc) (DOBJ * smpDOp, AOPTS * aoPtr, DOBJ * lpDOp);

typedef struct anaopt_function_list
{
  char *fName;			/* symbolic (known) name of the function */
  setDefProc *setFunc;		/* name of the function to set default option values */
  computeProc *compProc;	/* name of the function to call for parameter
				 *  computation*/
  int major;			/*major version number */
  int minor;			/*minor version number */
  char defExt[16];		/*default extension */
  tclAsspFunc_e funcNum;	/*number of function */
} A_F_LIST;



#endif // _WRASSP
