##' calculate cepstral coeffisients using libassp
##'
##' Short-term cepstral analysis of the signal in <listOfFiles>
##' using the Fast Fourier Transform. The number of
##' coefficients per output record will also equal the
##' FFT length / 2 + 1 (i.e. be non-mirrored).
##' Analysis results will be written to a file with the
##' base name of the input file and as extension '.cep'.
##' Default output is in SSFF format with the
##' 'cep' as track name.
##' @title cepstrum
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
##' results in a frequency resolution of <freq> Hz or better (default: 40.0)
##' @param FftLength = <num>: set FFT length to <num> points (overrules default
##' and 'Resolution' option)
##' @param WindowShift = <dur>: set analysis window shift to <dur> ms
##' (default: 5.0)
##' @param Window = <type>: set analysis window function to <type> (default:
##' BLACKMAN)
##' @param ToFile write results to file (default extension depends on )
##' @param ExplicitExt set if you wish to overwride the default extension
##' @param forceToLog is set by the global package variable useWrasspLogger. This is set
##' to FALSE by default and should be set to TRUE is logging is desired.
##' @return nrOfProcessedFiles or if only one file to process return
##' AsspDataObj of that file
##' @author Raphael Winkelmann
##' @author Lasse Bombien
##' @useDynLib wrassp
##' @export
'cepstrum' <- function(listOfFiles = NULL, optLogFilePath = NULL,
                       BeginTime = 0.0, CenterTime = FALSE,
                       EndTime = 0.0, Resolution = 40.0,
                       FftLength = 0,
                       WindowShift = 5.0, Window = 'BLACKMAN',
                       ToFile = TRUE, 
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
  

  ## ########################
  ## remove file:// and expand listOfFiles (SIC)
  
  listOfFiles = gsub("^file://","", listOfFiles)
  listOfFiles = path.expand(listOfFiles)
  
  ## #######################
  ## perform analysis

  if(length(listOfFiles)==1){
    pb <- NULL
  }else{
    cat('\n  INFO: applying cepstrum to', length(listOfFiles), 'files\n')
    pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
  }	
  
  externalRes = invisible(.External("performAssp", listOfFiles, 
    fname = "spectrum", BeginTime = BeginTime, 
    CenterTime = CenterTime, EndTime = EndTime, 
    SpectrumType = 'CEP',
    Resolution = Resolution, 
    FftLength = as.integer(FftLength),
    WindowShift = WindowShift, Window = Window, 
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
