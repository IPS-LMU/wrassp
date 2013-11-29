##' mhsF0 function adapted from libassp
##'
##' Pitch analysis of the speech signal in <listOfFile> using
##' Michel's/Modified Harmonic Sieve algorithm.
##' Analysis results will be written to a file with the
##' base name of the input file and extension '.pit'.
##' Default output is in SSFF binary format (track 'pitch').
##' @title mhsF0
##' @param listOfFiles vector of file paths to be processed by function
##' @param optLogFilePath path to option log file
##' @param BeginTime = <time>: set begin of analysis interval to <time> seconds (default = 0: begin of file) 
##' @param CenterTime = <time>:  set single-frame analysis with the analysis window centred at <time> seconds; overrules BeginTime, EndTime and WindowShift options
##' @param EndTime = <time>: set end of analysis interval to <time> seconds (default = 0: end of file)
##' @param WindowShift = <dur>: set analysis window shift to <dur> ms (default: 5.0)
##' @param Gender = <code>  set gender-specific pitch ranges; <code> may be:
##' "f[emale]" (80.0 - 600.0 Hz)
##' "m[ale]" (50.0 - 375.0 Hz)
##' "u[nknown]" (default; 50.0 - 600.0 Hz)
##' @param MaxF = <freq>: set maximum pitch value to <freq> Hz (default: 500.0)
##' @param MinF = <freq>:  set minimum pitch value to <freq> Hz (default: 50.0  minimum: 25.0)
##' @param MinAmp = <amp>:  minimum signal amplitude (default: 50)
##' @param MinAC1 = <freq>: minimum 1st correlation coefficient (default: 0.250)
##' @param MinRMS = <num>:  minimum RMS amplitude in dB (default: 18.0)
##' @param MaxZCR = <freq>: maximum zero crossing rate in Hz (default: 3000)
##' @param MinProb = <num>: minimum quality value of F0 fit (default: 0.520)
##' @param PlainSpectrum use plain rather than masked power spectrum
##' @param ToFile write results to file (default extension is .pit)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @param OutputDirectory directory in which output files are stored. Defaults to NULL, i.e.
##' the directory of the input files
##' @param forceToLog is set by the global package variable useWrasspLogger. This is set
##' to FALSE by default and should be set to TRUE is logging is desired.
##' @return nrOfProcessedFiles or if only one file to process return AsspDataObj of that file
##' @author Raphael Winkelmann
##' @aliases mhspitch f0_mhs
##' @seealso \code{\link{ksvF0}} for an tracking the fundamental frequency
##' @useDynLib wrassp
##' @export
'mhsF0' <- 'mhspitch' <- 'f0_mhs' <-function(listOfFiles = NULL, optLogFilePath = NULL,
                                             BeginTime = 0.0, CenterTime = FALSE, 
                                             EndTime = 0.0, WindowShift = 5.0, 
                                             Gender = 'u', MaxF = 600.0, 
                                             MinF = 50.0, MinAmp = 50.0, 
                                             MinAC1 = 0.25, MinRMS = 18.0, 
                                             MaxZCR = 3000.0, MinProb = 0.52, 
                                             PlainSpectrum = FALSE, ToFile = TRUE, 
                                             ExplicitExt = NULL,  OutputDirectory = NULL,
                                             forceToLog = useWrasspLogger, Header = NULL){
  
  ###########################
  # a few parameter checks and expand paths
  
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
  
  if (!is.null(OutputDirectory)) {
    OutputDirectory = path.expand(OutputDirectory)
    finfo  <- file.info(OutputDirectory)
    if (is.na(finfo$isdir))
      if (!dir.create(OutputDirectory, recursive=TRUE))
        stop('Unable to create output directory.')
    else if (!finfo$isdir)
      stop(paste(OutputDirectory, 'exists but is not a directory.'))
  }
  
  ###########################
  # remove file:// and expand listOfFiles (SIC)
  
  listOfFiles = gsub("^file://","", listOfFiles)
  listOfFiles = path.expand(listOfFiles)

  # Prepare analysis by downloading any URIs to the cache 
  listOfFiles <- prepareFiles(listOfFiles, Header)
  
  ###########################
  # perform analysis
  
  if(length(listOfFiles)==1){
    pb <- NULL
  }else{
    cat('\n  INFO: applying mhspitch to', length(listOfFiles), 'files\n')
    pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
  }		
  
  externalRes = invisible(.External("performAssp", listOfFiles, 
                                    fname = "mhspitch", BeginTime = BeginTime, 
                                    CenterTime = CenterTime, EndTime = EndTime, 
                                    WindowShift = WindowShift, Gender = Gender, 
                                    MaxF = MaxF, MinF = MinF, 
                                    MinAmp = MinAmp, MinAC1 = MinAC1, 
                                    MinRMS = MinRMS, MaxZCR = MaxZCR, 
                                    MinProb = MinProb, PlainSpectrum = PlainSpectrum, 
                                    ToFile = ToFile, ExplicitExt = ExplicitExt, 
                                    ProgressBar = pb, OutputDirectory = OutputDirectory,
                                    PACKAGE = "wrassp"))
  
  ############################
  # write options to options log file
  
  if (forceToLog){
    optionsGivenAsArgs = as.list(match.call(expand.dots = TRUE))
    wrassp.logger(optionsGivenAsArgs[[1]], optionsGivenAsArgs[-1],
                  optLogFilePath, listOfFiles)
    
  }
  
  #############################
  # return dataObj if length only one file
  
  if(!(length(listOfFiles)==1)){
    close(pb)
  }else{
    return(externalRes)
  }
}
