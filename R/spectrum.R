##' spectrum function adapted from libassp
##'
##' Short-term spectral analysis of the signal in <listOfFiles>
##' using the Fast Fourier Transform. The default is to
##' calculate an unsmoothed narrow-band spectrum with the
##' size of the analysis window equal to the length of the
##' FFT. The output from the FFT will be converted to a
##' power spectrum in dB from 0 Hz up to and including the
##' Nyquist rate.
##' Alternatively, the program can calculate smoothed
##' spectra or cepstra. In the latter case the number of
##' coefficients per output record will also equal the
##' FFT length / 2 + 1 (i.e. be non-mirrored).
##' Analysis results will be written to a file with the
##' base name of the input file and the spectrum type in
##' lower case as extension (e.g. '.dft').
##' Default output is in SSFF format with the
##' spectrum type in lower case as track name.
##' @title spectrum
##' @param listOfFiles vector of file paths to be processed by function 
##' @param optLogFilePath path to option log file
##' @param BeginTime = <time>: set begin of analysis interval to <time> seconds
##' (default: begin of data)
##' @param CenterTime = <time>: set single-frame analysis with the analysis
##' window centred at <time> seconds; overrules BeginTime, EndTime and
##' WindowShift options
##' @param EndTime = <time>: set end of analysis interval to <time> seconds
##' (default: end of data)
##' @param Resolution = <freq>: set FFT length to the smallest value which
##' results in a freqequency resolution of <freq> Hz or better (default: 40.0)
##' @param FftLength = <num>: set FFT length to <num> points (overrules default
##' and 'Resolution' option)
##' @param WindowSize = <dur>: set analysis window size to <dur> ms (overrules
##' 'Bandwidth' option and restriction by FFT length) additional option for
##' cepstrally smoothed spectrum:
##' @param WindowShift = <dur>: set analysis window shift to <dur> ms
##' (default: 5.0)
##' @param Window = <type>: set analysis window function to <type> (default:
##' BLACKMAN)
##' @param Bandwidth = <freq>: set the effective analysis bandwidth to <freq>
##' Hz (default: 0, yielding the smallest possible value given the length of
##' the FFT)
##' @param SpectrumType =<type>  set analysis type; <type> may be:
##' DFT : unsmoothed spectrum (default)
##' LPS : linear prediction smoothed spectrum
##' CSS : cepstrally smoothed spectrum
##' CEP : cepstral coefficients
##' @param EffectiveLength make window size effective rather than exact
##' @param Order = <num>: set prediction order to <num> (default: sampling
##' rate in kHz + 3)
##' @param Preemphasis = <val>: set pre-emphasis factor to <val> (default:
##' -0.95)
##' @param Deemphasize omit de-emphasis (default: undo spectral tilt due to
##' pre-emphasis used in LP analysis)
##' @param NumCeps = <num>: set number of cepstral coeffcients used to <num>
##' (default: sampling rate in kHz + 1; minimum: 2)
##' @param ToFile write results to file (default extension depends on )
##' @param ExplicitExt set if you wish to overwride the default extension
##' @param forceToLog is set by the global package variable useWrasspLogger. This is set
##' to FALSE by default and should be set to TRUE is logging is desired.
##' @return nrOfProcessedFiles or if only one file to process return
##' AsspDataObj of that file
##' @author Raphael Winkelmann
##' @useDynLib wrassp
##' @export
'spectrum' <- function(listOfFiles = NULL, optLogFilePath = NULL,
                       BeginTime = 0.0, CenterTime = FALSE,
                       EndTime = 0.0, Resolution = 40.0,
                       FftLength = 0, WindowSize = 20.0,
                       WindowShift = 5.0, Window = 'BLACKMAN',
                       Bandwidth = 0.0, SpectrumType = 'DFT',
                       EffectiveLength = FALSE, Order = 0,
                       Preemphasis = -0.95, Deemphasize = FALSE,
                       NumCeps = 0, ToFile = TRUE,
                       ExplicitExt = NULL, forceToLog = useWrasspLogger){
  
  ## ########################
  ## a few parameter checks and expand paths
  
  if (is.null(listOfFiles)) {
    stop(paste("listOfFiles is NULL! It has to be a string or vector of file",
               "paths (min length = 1) pointing to valid file(s) to perform",
               "the given analysis function."))
  }

  if (is.null(optLogFilePath) && forceToLog){
    stop("optLogFilePath is NULL! -> not logging!")
  }else{
    if(forceToLog){
      optLogFilePath = path.expand(optLogFilePath)  
    }
  }
  
  if(!isAsspWindowType(Window)){
    stop("WindowFunction of type '", Window,"' is not supported!")
  }
  
  if(!isAsspSpectType(SpectrumType)){
    stop("SpectrumType of type '", SpectrumType, "' is not supported!")
  }

  ## ########################
  ## remove file:// and expand listOfFiles (SIC)
  
  listOfFiles = gsub("^file://","", listOfFiles)
  listOfFiles = path.expand(listOfFiles)
  
  ## #######################
  ## perform analysis

  if(length(listOfFiles)==1){
    pb <- NULL
  }else{
    cat('\n  INFO: applying spectrum to', length(listOfFiles), 'files\n')
    pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
  }	
  
  externalRes = invisible(.External("performAssp", listOfFiles, 
    fname = "spectrum", BeginTime = BeginTime, 
    CenterTime = CenterTime, EndTime = EndTime, 
    SpectrumType = SpectrumType,
    Resolution = Resolution, 
    FftLength = as.integer(FftLength), WindowSize = WindowSize, 
    WindowShift = WindowShift, Window = Window, 
    Bandwidth = Bandwidth, EffectiveLength = EffectiveLength, 
    Order = as.integer(Order), Preemphasis = Preemphasis, 
    Deemphasize = Deemphasize, NumCeps = as.integer(NumCeps), 
    ToFile = ToFile, ExplicitExt = ExplicitExt, 
    ProgressBar = pb, PACKAGE = "wrassp"))


  ## #########################
  ## write options to options log file
  if (forceToLog){
    optionsGivenAsArgs = as.list(match.call(expand.dots = TRUE))
    wrassp.logger(optionsGivenAsArgs[[1]], optionsGivenAsArgs[-1],
                  optLogFilePath, listOfFiles)

  }
  
  ## #########################
  ## return dataObj if length only one file
  
  if(!(length(listOfFiles)==1)){
    close(pb)
  }else{
    return(externalRes)
  }
}
