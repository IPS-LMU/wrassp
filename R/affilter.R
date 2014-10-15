##' affilter function adapted from libassp
##'
##' Filters the audio signal in <listOfFiles>.
##' By specifying the high-pass and/or low-pass cut-off
##' frequency one of four filter characteristics may be
##' selected as shown in the table below.\cr
##' hp     lp     filter characteristic         extension\cr
##' -------------------------------------------------------\cr
##' > 0     [0]    high-pass from hp             '.hpf'\cr
##'  [0]   > 0     low-pass up to lp             '.lpf'\cr
##' > 0    > hp    band-pass from hp to lp       '.bpf'\cr
##' > lp   > 0     band-stop between lp and hp   '.bsf'\cr
##' The Kaiser-window design method is used to compute the
##' coefficients of a linear-phase FIR filter with unity gain
##' in the pass-band. The cut-off frequencies (-6 dB points)
##' of the filters are in the middle of the transition band.
##' The filtered signal will be written to a file with the
##' base name of the input file and an extension corresponding
##' to the filter characteristic (see table). The format of
##' the output file will be the same as that of the input file.
##' @title affilter
##' @param listOfFiles vector of file paths to be processed by function
##' @param optLogFilePath path to option log file 
##' @param highPass = <num>: set the high-pass cut-off frequency to <num> Hz (default: 0, no high-pass filtering)
##' @param lowPass = <num>: set the low-pass cut-off frequency to <num> Hz (default: 0, no low-pass filtering)
##' @param stopBand = <num>: set the stop-band attenuation to <num> dB (default: 93.0 dB, minimum: 21.0 dB)
##' @param transition = <num>: set the width of the transition band to <num> Hz (default: 250.0 Hz)
##' @param useIIR switch from the default FIR to IIR filter 
##' @param numIIRsections = <num>: set the number of 2nd order sections to <num> (default: 4) where each section adds 12dB/oct to the slope of the filter 
##' @param toFile write results to file (for default extension see details section))
##' @param explicitExt set if you wish to overwride the default extension
##' @param outputDirectory directory in which output files are stored. Defaults to NULL, i.e. 
##' the directory of the input files
##' @param forceToLog is set by the global package variable useWrasspLogger. This is set
##' to FALSE by default and should be set to TRUE is logging is desired.
##' @return nrOfProcessedFiles or if only one file to process return AsspDataObj of that file
##' @author Raphael Winkelmann
##' @author Lasse Bombien
##' @useDynLib wrassp
##' @export
'affilter' <- function(listOfFiles = NULL, optLogFilePath = NULL, 
                       highPass = 4000, lowPass = 0, 
                       stopBand = 96, transition = 250, 
                       useIIR = FALSE, numIIRsections = 4, 
                       toFile = TRUE, explicitExt = NULL,
                       outputDirectory = NULL, forceToLog = useWrasspLogger){
  
  ###########################
  ### a few parameter checks and expand paths
  
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
  
  if (!is.null(outputDirectory)) {
    outputDirectory = normalizePath(path.expand(outputDirectory))
    finfo  <- file.info(outputDirectory)
    if (is.na(finfo$isdir))
      if (!dir.create(outputDirectory, recursive=TRUE))
        stop('Unable to create output directory.')
    else if (!finfo$isdir)
      stop(paste(outputDirectory, 'exists but is not a directory.'))
  }
  ###########################
  # Pre-process file list
  listOfFiles <- prepareFiles(listOfFiles)
  
  ###########################
  ### perform analysis
  
  if(length(listOfFiles)==1){
    pb <- NULL
  }else{
    if(toFile==FALSE){
      stop("length(listOfFiles) is > 1 and ToFile=FALSE! ToFile=FALSE only permitted for single files.")
    }
    cat('\n  INFO: applying affilter to', length(listOfFiles), 'files\n')
    pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
  }
  
  externalRes = invisible(.External("performAssp", listOfFiles, 
                                    fname = "affilter", highPass = highPass, 
                                    lowPass = lowPass, stopBand = stopBand, transition = transition, 
                                    useIIR = useIIR, numIIRsections = as.integer(numIIRsections),
                                    toFile = toFile, explicitExt = explicitExt, 
                                    progressBar = pb, outputDirectory = outputDirectory,
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

