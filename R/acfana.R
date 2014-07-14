##' acfana function adapted from libassp
##'
##' Analysis of short-term autocorrelation function of
##' the signals in <listOFFiles>.
##' Analysis results will be written to a file with the
##' base name of the input file and extension '.acf'.
##' Default output is in SSFF binary format (track 'acf').
##' @title acfana
##' @param listOfFiles vector of file paths to be processed by function
##' @param optLogFilePath path to option log file
##' @param BeginTime = <time>: set begin of analysis interval to <time> seconds (default: 0 = beginning of file)
##' @param CenterTime = <time>: set single-frame analysis with the analysis window centred at <time> seconds; overrules BeginTime, EndTime and WindowShift options
##' @param EndTime = <time>: set end of analysis interval to <time> seconds (default: 0 = end of file)
##' @param WindowShift = <dur>: set analysis window shift to <dur> ms (default: 5.0)
##' @param WindowSize = <dur>: set analysis window size to <dur> ms; overrules EffectiveLength parameter
##' @param EffectiveLength make window size effective rather than exact
##' @param Window = <type>: set analysis window function to <type> (default: BLACKMAN)
##' @param AnalysisOrder = <num>: set analysis order to <num> (default: 0 = sample rate in kHz + 3)
##' @param EnergyNormalization calculate energy-normalized autocorrelation
##' @param LengthNormalization calculate length-normalized autocorrelation
##' @param ToFile write results to file (default extension is .acf)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @param OutputDirectory directory in which output files are stored. Defaults to NULL, i.e. 
##' the directory of the input files
##' @param forceToLog is set by the global package variable useWrasspLogger. This is set
##' to FALSE by default and should be set to TRUE is logging is desired.
##' @return nrOfProcessedFiles or if only one file to process return AsspDataObj of that file
##' @author Raphael Winkelmann
##' @useDynLib wrassp
##' @export
'acfana' <- function(listOfFiles = NULL, optLogFilePath = NULL, 
                     BeginTime = 0.0, CenterTime = FALSE, 
                     EndTime = 0.0, WindowShift = 5.0, 
                     WindowSize = 20.0, EffectiveLength = TRUE, 
                     Window = "BLACKMAN", AnalysisOrder = 0, 
                     EnergyNormalization = FALSE, LengthNormalization = FALSE, 
                     ToFile = TRUE, ExplicitExt = NULL, OutputDirectory = NULL,
                     forceToLog = useWrasspLogger){
  
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
  
  if(!isAsspWindowType(Window)){
    stop("WindowFunction of type '", Window,"' is not supported!")
  }
  
  if (!is.null(OutputDirectory)) {
    OutputDirectory = normalizePath(path.expand(OutputDirectory))
    finfo  <- file.info(OutputDirectory)
    if (is.na(finfo$isdir))
      if (!dir.create(OutputDirectory, recursive=TRUE))
        stop('Unable to create output directory.')
    else if (!finfo$isdir)
      stop(paste(OutputDirectory, 'exists but is not a directory.'))
  }
  ###########################
  # Pre-process file list
  listOfFiles <- prepareFiles(listOfFiles)
  
  ###########################
  # perform analysis
  
  if(length(listOfFiles)==1){
    pb <- NULL
  }else{
    if(ToFile==FALSE){
      stop("length(listOfFiles) is > 1 and ToFile=FALSE! ToFile=FALSE only permitted for single files.")
    }
    cat('\n  INFO: applying acfana to', length(listOfFiles), 'files\n')
    pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
  }
  
  externalRes = invisible(.External("performAssp", listOfFiles, 
                                    fname = "acfana", BeginTime = BeginTime, 
                                    CenterTime = CenterTime, EndTime = EndTime, 
                                    WindowShift = WindowShift, WindowSize = WindowSize, 
                                    EffectiveLength = EffectiveLength, Window = Window, 
                                    AnalysisOrder = as.integer(AnalysisOrder), EnergyNormalization = EnergyNormalization, 
                                    LengthNormalization = LengthNormalization, ToFile = ToFile, 
                                    ExplicitExt = ExplicitExt, ProgressBar = pb,
                                    OutputDirectory = OutputDirectory, PACKAGE = "wrassp"))
  
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
