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
  AsspFunc_e funcNum;	/*number of function */
} A_F_LIST;

typedef enum assp_option_number
{
  TO_NONE = -1,
  TO_OPTIONS,			/* for bit flags (upper byte reserved) */
  TO_BEGINTIME,
  TO_ENDTIME,
  TO_CENTRETIME,
  TO_MSSIZE,
  TO_MSSHIFT,
  TO_MSSMOOTH,
  TO_BANDWIDTH,
  TO_RESOLUTION,
  TO_GAIN,
  TO_RANGE,
  TO_PREEMPH,
  TO_FFTLEN,
  TO_CHANNEL,
  TO_GENDER,
  TO_ORDER,
  TO_INCREMENT,
  TO_NUMLEVELS,
  TO_NUMFORMANTS,
  TO_PRECISION,
  TO_ACCURACY,
  TO_ALPHA,
  TO_THRESHOLD,
  TO_MAXF,
  TO_MINF,
  TO_NOMF1,			/* e.g. for formant analysis */
  TO_INS_EST,
  TO_VOIAC1PP,			/* VOICING thresholds */
  TO_VOIMAG,
  TO_VOIPROB,
  TO_VOIRMS,
  TO_VOIZCR,
  TO_HPCUTOFF,			/* filter parameters */
  TO_LPCUTOFF,
  TO_STOPDB,
  TO_TBWIDTH,
  TO_USEIIR,      /* use IIR filter instead of FIR */
  TO_NUMIIRSECS,  /* number of IIR sections, default 4 */
  TO_TYPE,			/* hold-all */
  TO_FORMAT,
  TO_WINFUNC,
  /* These are not in libassp, only in tclassp */
  TO_MSEFFLEN,
  /* plain power spectrum in mhs pitch */
  TO_MHS_OPT_POWER,
  /* normation for acfana */
  TO_ENERGYNORM,
  TO_LENGTHNORM,
  /* options specific to afdiff */
  TO_DIFF_OPT_BACKWARD,		/* backwards difference (as opposed to forward) */
  TO_DIFF_OPT_CENTRAL,		/* compute central/interpolated/3-point difference */
  /* options specific to rmsana*/
  TO_RMS_OPT_LINEAR,     /* linear RMS amplitude  */
  /* options specific to spectrum */
  TO_LPS_OPT_DEEMPH, /* omit de-emphasis */
  /* general tclassp options */
  TO_OUTPUTDIR,
  TO_OUTPUTEXT,
  TO_FILE
} ASSP_OPT_NUM;



#endif // _WRASSP
