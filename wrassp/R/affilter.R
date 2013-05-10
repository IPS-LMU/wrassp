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
##' @param HighPass = <num>: set the high-pass cut-off frequency to <num> Hz (default: 0, no high-pass filtering)
##' @param LowPass = <num>: set the low-pass cut-off frequency to <num> Hz (default: 0, no low-pass filtering)
##' @param StopBand = <num>: set the stop-band attenuation to <num> dB (default: 93.0 dB, minimum: 21.0 dB)
##' @param Transition = <num>: set the width of the transition band to <num> Hz (default: 250.0 Hz)
##' @param UseIIR switch from the default FIR to IIR filter 
##' @param NumIIRsections = <num>: set the number of 2nd order sections to <num> (default: 4) where each section adds 12dB/oct to the slope of the filter 
##' @param ToFile write results to file (for default extension see details section))
##' @param ExplicitExt set if you wish to overwride the default extension 
##' @return nrOfProcessedFiles or if only one file to process return AsspDataObj of that file
##' @author Raphael Winkelmann
##' @useDynLib wrassp
'affilter' <- function(listOfFiles = NULL, optLogFilePath = NULL, 
                       HighPass = 4000, LowPass = 0, 
                       StopBand = 96, Transition = 250, 
                       UseIIR = FALSE, NumIIRsections = 4, 
                       ToFile = TRUE, ExplicitExt = NULL) {

  ###########################
  ### a few parameter checks and expand paths

  if (is.null(listOfFiles)) {
    stop(paste("listOfFiles is NULL! It has to be a string or vector of file",
               "paths (min length = 1) pointing to valid file(s) to perform",
               "the given analysis function."))
  }
     
  if (is.null(optLogFilePath)){
    warning("optLogFilePath is NULL! -> not logging!")
  }else{     
    optLogFilePath = path.expand(optLogFilePath)
  }

  listOfFiles = path.expand(listOfFiles)

  ###########################
  ### perform analysis
    
  if(length(listOfFiles)==1){
    pb <- NULL
  }else{
    cat('\n  INFO: applying affilter to', length(listOfFiles), 'files\n')

    pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
  }
    
  externalRes = invisible(.External("performAssp", listOfFiles, 
                                    fname = "affilter", HighPass = HighPass, 
                                    LowPass = LowPass, StopBand = StopBand, Transition = Transition, 
                                    UseIIR = UseIIR, NumIIRsections = as.integer(NumIIRsections),
                                    ToFile = ToFile, ExplicitExt = ExplicitExt, 
                                    ProgressBar = pb, PACKAGE = "wrassp"))

  ############################
  # write options to options log file
  
  if (!is.null(optLogFilePath)){
    cat("\n##################################\n", file = optLogFilePath, append = T)
    cat("##################################\n", file = optLogFilePath, append = T)
    cat("####### affilter performed #######\n", file = optLogFilePath, append = T)
    
    cat("Timestamp: ", paste(Sys.time()), '\n', file = optLogFilePath, append = T)
    cat("HighPass", HighPass, "\n", file = optLogFilePath, append = T)
    cat("LowPass", LowPass, "\n", file = optLogFilePath, append = T)
    cat("StopBand", StopBand, "\n", file = optLogFilePath, append = T)
    cat("Transition", Transition, "\n", file = optLogFilePath, append = T) 
    cat("UseIIR", UseIIR, "\n", file = optLogFilePath, append = T)
    cat("NumIIRsections: ", NumIIRsections, "\n", file = optLogFilePath, append = T)
    
    cat("ToFile: ", ToFile, "\n", file = optLogFilePath, append = T)
    cat("ExplicitExt: ", ExplicitExt, "\n", file = optLogFilePath, append = T)
    
    cat(" => on files:\n\t", file = optLogFilePath, append = T)
    cat(paste(listOfFiles, collapse="\n\t"), file = optLogFilePath, append = T) 
  
  }
  
  #############################
  # return dataObj if length only one file
        
  if(!(length(listOfFiles)==1)){
    close(pb)
  }else{
    return(externalRes)
  }
}

