#include "wrassp.h"
#include <asspana.h>
#include <dataobj.h>
#include <asspmess.h>


#include <R_ext/PrtUtil.h>
T_OPT acfanaOptions[] = {
  {"BeginTime", WO_BEGINTIME}
  ,
  {"CenterTime", WO_CENTRETIME}
  ,
  {"EndTime", WO_ENDTIME}
  ,
  {"WindowShift", WO_MSSHIFT}
  ,
  {"WindowSize", WO_MSSIZE}
  ,
  {"EffectiveLength", WO_MSEFFLEN}
  ,
  {"ExplicitExt", WO_OUTPUTEXT}
  ,				//DON'T FORGET EXTENSION!!!
  {"WindowFunction", WO_WINFUNC}
  ,
  {"AnalysisOrder", WO_ORDER}
  ,
  {"EnergyNormalization", WO_ENERGYNORM}
  ,
  {"LengthNormalization", WO_LENGTHNORM}
  ,
  {"ToFile", WO_TOFILE}
  ,
  {NULL, WO_NONE}
};

T_OPT afdiffOptions[] = {
  {"ComputeBackwardDifference", WO_DIFF_OPT_BACKWARD}
  ,
  {"ComputeCentralDifference", WO_DIFF_OPT_CENTRAL}
  ,
  {"Channel", WO_CHANNEL}
  ,
  {"ExplicitExt", WO_OUTPUTEXT}
  ,				//DON'T FORGET EXTENSION!!!
  {"ToFile", WO_TOFILE},    {NULL, WO_NONE}
};

T_OPT affilterOptions[] = {
  {"HighPass", WO_HPCUTOFF}
  ,
  {"LowPass", WO_LPCUTOFF}
  ,
  {"StopBand", WO_STOPDB}
  ,
  {"Transition", WO_TBWIDTH}
  ,
  {"UseIIR", WO_USEIIR}
  ,
  {"NumIIRsections", WO_NUMIIRSECS}
  ,
  {"ExplicitExt", WO_OUTPUTEXT}
  ,				//DON'T FORGET EXTENSION!!!
  {"ToFile", WO_TOFILE}
  ,    
  /* {"-channel"}, */
  /* {"-noDither"}, */
  /* {"-gain"}, */
  {NULL, WO_NONE}
};

T_OPT f0_ksvOptions[] = {
  {"BeginTime", WO_BEGINTIME}
  ,
  {"EndTime", WO_ENDTIME}
  ,
  {"WindowShift", WO_MSSHIFT}
  ,
  {"Gender", WO_GENDER}
  ,
  {"MaxF", WO_MAXF}
  ,
  {"MinF", WO_MINF}
  ,
  {"MinAmp", WO_VOIMAG}
  ,
  {"MaxZCR", WO_VOIZCR}
  ,
  {"ExplicitExt", WO_OUTPUTEXT}
  ,				//DON'T FORGET EXTENSION!!!
  {"ToFile", WO_TOFILE},    {NULL, WO_NONE}
};

T_OPT f0_mhsOptions[] = {
  {"BeginTime", WO_BEGINTIME}
  ,
  {"CenterTime", WO_CENTRETIME}
  ,
  {"EndTime", WO_ENDTIME}
  ,
  {"WindowShift", WO_MSSHIFT}
  ,
  {"Gender", WO_GENDER}
  ,
  {"MaxF", WO_MAXF}
  ,
  {"MinF", WO_MINF}
  ,
  {"MinAmp", WO_VOIMAG}
  ,
  {"MinAC1", WO_VOIAC1PP}
  ,
  {"MinRMS", WO_VOIRMS}
  ,
  {"MaxZCR", WO_VOIZCR}
  ,
  {"MinProb", WO_VOIPROB}
  ,
  {"PlainSpectrum", WO_MHS_OPT_POWER}
  ,
  {"ExplicitExt", WO_OUTPUTEXT}
  ,				//DON'T FORGET EXTENSION!!!
  {"ToFile", WO_TOFILE},    {NULL, WO_NONE}
};

/**
 * Options array for forest (formant estimation)
 * 
 */
T_OPT forestOptions[] = {
  {"BeginTime", WO_BEGINTIME}
  ,
  {"EndTime", WO_ENDTIME}
  ,
  {"WindowShift", WO_MSSHIFT}
  ,
  {"WindowSize", WO_MSSIZE}
  ,
  {"EffectiveLength", WO_MSEFFLEN}
  ,
  {"NominalF1", WO_NOMF1}
  ,
  {"Gender", WO_GENDER}
  ,
  {"Estimate", WO_INS_EST}
  ,
  {"Order", WO_ORDER}
  ,
  {"IncrOrder", WO_INCREMENT}
  ,
  {"NumFormants", WO_NUMFORMANTS}
  ,
  /* {}, */
  {"Window", WO_WINFUNC}
  ,
  {"Preemphasis", WO_PREEMPH}
  ,
  {"ExplicitExt", WO_OUTPUTEXT}
  ,				//DON'T FORGET EXTENSION!!!
  {"ToFile", WO_TOFILE},    {NULL, WO_NONE}
};


T_OPT rfcanaOptions[] = {
  {"BeginTime", WO_BEGINTIME}
  ,
  {"CenterTime", WO_CENTRETIME}
  ,
  {"EndTime", WO_ENDTIME}
  ,
  {"WindowShift", WO_MSSHIFT}
  ,
  {"WindowSize", WO_MSSIZE}
  ,
  {"EffectiveLength", WO_MSEFFLEN}
  ,
  {"ExplicitExt", WO_OUTPUTEXT}
  ,				//DON'T FORGET EXTENSION!!!
  {"Window", WO_WINFUNC}
  ,
  {"Order", WO_ORDER}
  ,
  {"Preemphasis", WO_PREEMPH}
  ,
  {"LpType", WO_TYPE}
  ,
  {"ToFile", WO_TOFILE},    {NULL, WO_NONE}
};

T_OPT rmsanaOptions[] = {
  {"BeginTime", WO_BEGINTIME}
  ,
  {"CenterTime", WO_CENTRETIME}
  ,
  {"EndTime", WO_ENDTIME}
  ,
  {"WindowShift", WO_MSSHIFT}
  ,
  {"WindowSize", WO_MSSIZE}
  ,
  {"EffectiveLength", WO_MSEFFLEN}
  ,
  {"Linear", WO_RMS_OPT_LINEAR}
  ,
  {"ExplicitExt", WO_OUTPUTEXT}
  ,				//DON'T FORGET EXTENSION!!!
  {"Window", WO_WINFUNC}
  ,
  {"ToFile", WO_TOFILE},    {NULL, WO_NONE}
};

T_OPT spectrumOptions[] = {
  {"BeginTime", WO_BEGINTIME}
  ,
  {"CenterTime", WO_CENTRETIME}
  ,
  {"EndTime", WO_ENDTIME}
  ,
  {"SpectrumType", WO_TYPE}
  ,
  {"Resolution", WO_RESOLUTION}
  ,
  {"FftLength", WO_FFTLEN}
  ,
  {"WindowSize", WO_MSSIZE}
  ,
  {"WindowShift", WO_MSSHIFT}
  ,
  {"Window", WO_WINFUNC}
  ,
  /* DFT spectrum */
  {"Bandwidth", WO_BANDWIDTH}
  ,
  /* LP smoothed spectrum */
  {"EffectiveLength", WO_MSEFFLEN}
  ,
  {"Order", WO_ORDER}
  ,
  {"Preemphasis", WO_PREEMPH}
  ,
  {"Deemphasize", WO_LPS_OPT_DEEMPH}
  ,
  /* Cepstrally smoothed spectrum */
  {"NumCeps", WO_ORDER}
  ,
  /* general stuff */
  {"ExplicitExt", WO_OUTPUTEXT}
  ,				//DON'T FORGET EXTENSION!!!
  {"ToFile", WO_TOFILE},    {NULL, WO_NONE}
};

T_OPT zcranaOptions[] = {
  {"BeginTime", WO_BEGINTIME}
  ,
  {"CenterTime", WO_CENTRETIME}
  ,
  {"EndTime", WO_ENDTIME}
  ,
  {"WindowShift", WO_MSSHIFT}
  ,
  {"WindowSize", WO_MSSIZE}
  ,
  {"ExplicitExt", WO_OUTPUTEXT}
  ,				//DON'T FORGET EXTENSION!!!
  /* {"Window", WO_WINFUNC} */
  /* , */
  {"ToFile", WO_TOFILE}
  ,
  {NULL, WO_NONE}
};

A_F_LIST funclist[] = {
  {"acfana", setACFdefaults, computeACF, ACF_MAJOR,
   ACF_MINOR,
   ACF_DEF_SUFFIX, TAF_ACFANA}
  ,
  {"afdiff", setDiffDefaults, diffSignal, DIFF_MAJOR,
   DIFF_MINOR, "AUTO", TAF_AFDIFF}
  ,
  {"affilter", setFILTdefaults, computeFilter,
   FILT_MAJOR, FILT_MINOR,
   "", TAF_AFFILTER}
  ,
  {"f0ana", setKSVdefaults, computeF0, KSV_MAJOR,
   KSV_MINOR,
   KSV_DEF_SUFFIX, TAF_KSV_PITCH}
  ,
  {"forest", setFMTdefaults, computeFMT, FMT_MAJOR,
   FMT_MINOR,
   FMT_DEF_SUFFIX, TAF_FOREST}
  ,
  {"mhspitch", setMHSdefaults, computeMHS, MHS_MAJOR,
   MHS_MINOR,
   MHS_DEF_SUFFIX, TAF_MHS_PITCH}
  ,
  {"rfcana", setLPdefaults, computeLP, RFC_MAJOR,
   RFC_MINOR,
   "", TAF_RFCANA}
  ,
  {"rmsana", setRMSdefaults, computeRMS, RMS_MAJOR,
   RMS_MINOR,
   RMS_DEF_SUFFIX, TAF_RMSANA}
  ,
  {"spectrum", setSPECTdefaults, computeSPECT,
   SPECT_MAJOR, SPECT_MINOR,
   "", TAF_SPECTRUM}
  ,
  {"zcrana", setZCRdefaults, computeZCR, ZCR_MAJOR,
   ZCR_MINOR,
   ZCR_DEF_SUFFIX, TAF_ZCRANA}
  ,
  {NULL, NULL, NULL, 0, 0, TAF_NONE}
};

SEXP performAssp(SEXP args) {
  SEXP el, inputs;
  const char * name;
  AOPTS * opts;
  A_F_LIST *anaFunc = funclist; 
  args = CDR(args); // skip function name
  
  /* First Element is file name or vector of file names */
  inputs = CAR(args);
  args = CDR(args);

  /* Second element must be assp function name */
  name = isNull(TAG(args)) ? "" : CHAR(PRINTNAME(TAG(args)));
  if (strcmp(name, 'fname') != 0)
    error("Second argument must be named 'fname'");
  
  el = CAR(args);
  while (anaFunc->funcNum != AF_NONE) {
    if (strcmp(CHAR(STRING_ELT(el, 0)), anaFunc->fName) == 0)
      break;
    anaFunc++;
  }
  if (anaFunc->funcNume == AF_NONE)
    error ("Invalid analysis function in performAssp.c");
  
  args= CDR(args);
  

  for (int i = 0; args != R_NilValue; i++, args = CDR(args)) {
    
  }
  return args;
}
