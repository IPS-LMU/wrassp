##' calculate cepstrally smoothed spectrum using libassp
##'
##' Short-term spectral analysis of the signal in <listOfFiles>
##' using the Fast Fourier Transform and cepstral smoothing.
##' Analysis results will be written to a file with the
##' base name of the input file and '.css.' as extension.
##' Default output is in SSFF format with the
##' 'css' in lower case as track name.
##' @title cssSpectrum
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
##' @param WindowShift = <dur>: set analysis window shift to <dur> ms
##' (default: 5.0)
##' @param Window = <type>: set analysis window function to <type> (default:
##' BLACKMAN)
##' @param NumCeps = <num>: set number of cepstral coeffcients used to <num>
##' (default: sampling rate in kHz + 1; minimum: 2)
##' @param ToFile write results to file (default extension depends on )
##' @param ExplicitExt set if you wish to overwride the default extension
##' @param OutputDirectory directory in which output files are stored. Defaults to NULL, i.e.
##' the directory of the input files
##' @param forceToLog is set by the global package variable useWrasspLogger. This is set
##' to FALSE by default and should be set to TRUE is logging is desired.
##' @return nrOfProcessedFiles or if only one file to process return
##' AsspDataObj of that file
##' @author Raphael Winkelmann
##' @author Lasse Bombien
##' @seealso \code{\link{dftSpectrum}}, \code{\link{lpsSpectrum}}, \code{\link{cepstrum}}; all derived from libassp's spectrum function.
##' @useDynLib wrassp
##' @export
'cssSpectrum' <- function(listOfFiles = NULL, optLogFilePath = NULL,
                          BeginTime = 0.0, CenterTime = FALSE,
                          EndTime = 0.0, Resolution = 40.0,
                          FftLength = 0, WindowShift = 5.0, 
                          Window = 'BLACKMAN', NumCeps = 0, 
                          ToFile = TRUE, ExplicitExt = NULL, 
                          OutputDirectory = NULL, forceToLog = useWrasspLogger,
                          Header = NULL){
  
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
  
  if (!is.null(OutputDirectory)) {
    OutputDirectory = path.expand(OutputDirectory)
    finfo  <- file.info(OutputDirectory)
    if (is.na(finfo$isdir))
      if (!dir.create(OutputDirectory, recursive=TRUE))
        stop('Unable to create output directory.')
    else if (!finfo$isdir)
      stop(paste(OutputDirectory, 'exists but is not a directory.'))
  }
  ## ########################
  ## remove file:// and expand listOfFiles (SIC)
  
  listOfFiles = gsub("^file://","", listOfFiles)
  listOfFiles = path.expand(listOfFiles)

  # Prepare analysis by downloading any URIs to the cache 
  listOfFiles <- prepareFiles(listOfFiles, Header)
  
  ## #######################
  ## perform analysis
  
  if(length(listOfFiles)==1){
    pb <- NULL
  }else{
    cat('\n  INFO: applying cssSpectrum to', length(listOfFiles), 'files\n')
    pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
  }	
  
  externalRes = invisible(.External("performAssp", listOfFiles, 
                                    fname = "spectrum", BeginTime = BeginTime, 
                                    CenterTime = CenterTime, EndTime = EndTime, 
                                    SpectrumType = 'CSS',
                                    Resolution = Resolution, 
                                    FftLength = as.integer(FftLength), 
                                    WindowShift = WindowShift, Window = Window, 
                                    NumCeps = as.integer(NumCeps), 
                                    ToFile = ToFile, ExplicitExt = ExplicitExt, 
                                    ProgressBar = pb, OutputDirectory = OutputDirectory,
                                    PACKAGE = "wrassp"))
  
  
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
