#ifndef _WRASSP
#define _WRASSP

#include <R.h>
#include <Rinternals.h>
#include <dataobj.h>
#include <asspana.h>


#define WRASSP_CLASS "AsspDataObj"


/*
 * An enumerator over all ASSP functions implemented in wrassp 
 */
typedef enum AsspFuncs {
    AF_NONE,
    AF_ACFANA,
    AF_AFDIFF,
    AF_AFFILTER,
    AF_KSV_PITCH,               /* f0ana */
    AF_FOREST,
    AF_MHS_PITCH,
    AF_RFCANA,
    AF_RMSANA,
    AF_SPECTRUM,
    AF_ZCRANA
} AsspFunc_e;

/*
 * Enable function pointers for setting the analysis defaults 
 */
typedef int     (setDefProc) (AOPTS * opt);

/*
 * Enable function pointers for using the analyses 
 */
typedef DOBJ   *(computeProc) (DOBJ * smpDOp, AOPTS * aoPtr, DOBJ * lpDOp);



/*
 * An enumerator over possible options to be implemented in ASSP functions  
 */
typedef enum wrassp_option_number {
    WO_NONE = -1,
    WO_OPTIONS,                 /* for bit flags (upper byte reserved) */
    WO_BEGINTIME,
    WO_ENDTIME,
    WO_CENTRETIME,
    WO_MSSIZE,
    WO_MSSHIFT,
    WO_MSSMOOTH,
    WO_BANDWIDTH,
    WO_RESOLUTION,
    WO_GAIN,
    WO_RANGE,
    WO_PREEMPH,
    WO_FFTLEN,
    WO_CHANNEL,
    WO_GENDER,
    WO_ORDER,
    WO_INCREMENT,
    WO_NUMLEVELS,
    WO_NUMFORMANTS,
    WO_PRECISION,
    WO_ACCURACY,
    WO_ALPHA,
    WO_THRESHOLD,
    WO_MAXF,
    WO_MINF,
    WO_NOMF1,                   /* e.g. for formant analysis */
    WO_INS_EST,
    WO_VOIAC1PP,                /* VOICING thresholds */
    WO_VOIMAG,
    WO_VOIPROB,
    WO_VOIRMS,
    WO_VOIZCR,
    WO_HPCUTOFF,                /* filter parameters */
    WO_LPCUTOFF,
    WO_STOPDB,
    WO_TBWIDTH,
    WO_USEIIR,                  /* use IIR filter instead of FIR */
    WO_NUMIIRSECS,              /* number of IIR sections, default 4 */
    WO_TYPE,                    /* hold-all */
    WO_FORMAT,
    WO_WINFUNC,

    /*
     * These are not in libassp, only in tclassp 
     */
    WO_MSEFFLEN,                /* plain power spectrum in mhs pitch */
    WO_MHS_OPT_POWER,           /* normation for acfana */
    WO_ENERGYNORM,
    WO_LENGTHNORM,

    /*
     * options specific to afdiff 
     */
    WO_DIFF_OPT_BACKWARD,       /* backwards difference (as opposed to
                                 * forward) */
    WO_DIFF_OPT_CENTRAL,        /* compute central/interpolated/3-point
                                 * difference */

    /*
     * options specific to rmsana 
     */
    WO_RMS_OPT_LINEAR,          /* linear RMS amplitude */

    /*
     * options specific to spectrum 
     */
    WO_LPS_OPT_DEEMPH,          /* omit de-emphasis */

    /*
     * general wrassp options 
     */
    WO_OUTPUTDIR,
    WO_OUTPUTEXT,
    WO_TOFILE,
    WO_PBAR                     /* R Textual Progress Bar */
} ASSP_OPT_NUM;

/*
 * Enable mapping from R strings to option enumerator 
 */
typedef struct wrassp_option {
    char           *name;       /* name of option as used in R */
    ASSP_OPT_NUM    optNum;
} W_OPT;

/*
 * This structure is a descriptor for an assp analysis function as used in 
 * R 
 */
typedef struct anaopt_function_list {
    char           *fName;      /* symbolic (known) name of the function */
    setDefProc     *setFunc;    /* name of the function to set default
                                 * option values */
    computeProc    *compProc;   /* name of the function to call for
                                 * parameter computation */
    W_OPT          *options;    /* pointer to options table */
    int             major;      /* major version number */
    int             minor;      /* minor version number */
    char            defExt[16]; /* default extension */
    AsspFunc_e      funcNum;    /* number of function */
} A_F_LIST;

/*
 * enumerator for the vast number of genders assp tries to account for 
 */
typedef enum wrassp_gender_type {
    TG_NONE = -1,
    TG_FEMALE,
    TG_MALE,
    TG_UNKNOWN,
} W_GENDER_TYPE;

/*
 * Enable mapping between gender specifiers passed from R to the
 * corresponding enumerator and the character used in ASSP 
 */
typedef struct wrassp_gender {
    char           *ident;
    W_GENDER_TYPE   num;
    char            code;
} W_GENDER;

/*
 * Each analysis function has an array of legal/available wrassp_options 
 */
extern W_OPT    acfanaOptions[];
extern W_OPT    afdiffOptions[];
extern W_OPT    affilterOptions[];
extern W_OPT    f0_ksvOptions[];
extern W_OPT    f0_mhsOptions[];
extern W_OPT    forestOptions[];
extern W_OPT    rmsanaOptions[];
extern W_OPT    rfcanaOptions[];
extern W_OPT    spectrumOptions[];
extern W_OPT    zcranaOptions[];


/*
 * Function prototypes 
 */
SEXP            getDObj(SEXP fname);
SEXP            getDObj2(SEXP fname);
SEXP            dobj2AsspDataObj(DOBJ * data);
SEXP            getDObjTracks(SEXP dobj);
SEXP            getDObjTrackData(DOBJ * data, DDESC * desc);
SEXP            getGenericVars(DOBJ * dop);

SEXP            performAssp(SEXP args);
SEXP            showArgs(SEXP args);

SEXP            AsspWindowList();
char           *asspDF2ssffString(int df);
SEXP            writeDObj_(SEXP, SEXP);
int             addTrackData(DOBJ * dop, DDESC * ddl, SEXP rdobj);
DOBJ           *computeFilter(DOBJ * inpDOp, AOPTS * anaopts,
                              DOBJ * outDOp);
DOBJ           *computeF0(DOBJ * inpDOp, AOPTS * anaOpts, DOBJ * outDOp);
DOBJ           *sexp2dobj(SEXP rdobj);


#endif                          // _WRASSP
