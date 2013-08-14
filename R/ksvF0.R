##' ksvF0 function adapted from libassp
##'
##' F0 analysis of the signal in <listOfFile> using the 
##' K. Schaefer-Vincent periodicity detection algortithm.
##' Analysis results will be written to a file with the
##' base name of the input file and extension '.f0'.
##' Default output is in SSFF binary format (track 'F0').
##' Optionally, location and type of the signal extrema on
##' which the F0 data are based, may be stored in a label
##' file. The name of this file will consist of the base
##' name of the F0 file and the extension '.prd'.
##' @title ksvF0
##' @param listOfFiles vector of file paths to be processed by function 
##' @param optLogFilePath path to option log file
##' @param BeginTime = <time>: set begin of analysis interval to <time> seconds (default = 0: begin of data)
##' @param EndTime set end of analysis interval to <time> seconds (default = 0: end of data)
##' @param WindowShift = <dur>: set frame shift to <dur> ms (default: 5.0)
##' @param Gender = <code>  set gender-specific F0 ranges; <code> may be:
##' "f[emale]" (80.0 - 640.0 Hz)
##' "m[ale]" (50.0 - 400.0 Hz)
##' "u[nknown]" (default; 50.0 - 600.0 Hz)
##' @param MaxF = <freq>: set maximum F0 value to <freq> Hz (default: 500.0)
##' @param MinF = <freq>: set minimum F0 value to <freq> Hz (default: 50.0)
##' @param MinAmp = <amp>: set amplitude threshold for voiced samples to <amp> (default: 100)
##' @param MaxZCR maximum zero crossing rate in Hz (for voicing detection)
##' @param ToFile write results to file (default extension is .f0)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @param OutputDirectory directory in which output files are stored. Defaults to NULL, i.e.
##' the directory of the input files
##' @param forceToLog is set by the global package variable useWrasspLogger. This is set
##' to FALSE by default and should be set to TRUE is logging is desired.
##' @return nrOfProcessedFiles or if only one file to process return AsspDataObj of that file
##' @author Raphael Winkelmann
##' @aliases f0ana f0_ksv
##' @seealso \code{\link{mhsF0}} for an alternative pitch tracker
##' @useDynLib wrassp
##' @export
'ksvF0' <- 'f0ana' <- 'f0_ksv' <- function(listOfFiles = NULL, optLogFilePath = NULL, 
                                           BeginTime = 0.0, EndTime = 0.0, 
                                           WindowShift = 5.0, Gender = 'u',
                                           MaxF = 600, MinF = 50, 
                                           MinAmp = 50, MaxZCR = 3000.0, 
                                           ToFile = TRUE, ExplicitExt = NULL,
                                           OutputDirectory = NULL, forceToLog = useWrasspLogger){
  
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
        error('Unable to create output directory.')
    else if (!finfo$isdir)
      error(paste(OutputDirectory, 'exists but is not a directory.'))
  }
  
  ###########################
  # remove file:// and expand listOfFiles (SIC)
  
  listOfFiles = gsub("^file://","", listOfFiles)
  listOfFiles = path.expand(listOfFiles)
  
  ###########################
  # perform analysis
  
  if(length(listOfFiles)==1){
    pb <- NULL
  }else{
    cat('\n  INFO: applying f0ana to', length(listOfFiles), 'files\n')
    pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
  }	
  
  externalRes = invisible(.External("performAssp", listOfFiles, 
                                    fname = "f0ana", BeginTime = BeginTime, 
                                    EndTime = EndTime, WindowShift = WindowShift, 
                                    Gender = Gender, MaxF = MaxF, 
                                    MinF = MinF, MinAmp = MinAmp, 
                                    MaxZCR = MaxZCR, ExplicitExt = ExplicitExt, 
                                    ToFile = ToFile, ProgressBar = pb, 
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
