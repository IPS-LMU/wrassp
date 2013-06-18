##' zcrana function adapted from libassp
##'
##' Analysis of the averages of the short-term positive and
##' negative zero-crossing rates of the signal in <listOfFiles>.
##' Analysis results will be written to a file with the
##' base name of the input file and extension '.zcr'.
##' Default output is in SSFF binary format (track 'zcr').
##' @title zcrana
##' @param listOfFiles vector of file paths to be processed by function 
##' @param optLogFilePath path to option log file
##' @param BeginTime = <time>: set begin of analysis interval to <time> seconds (default: begin of file)
##' @param CenterTime = <time>  set single-frame analysis with the analysis window centred at <time> seconds; overrules BeginTime, EndTime and WindowShift options
##' @param EndTime = <time>: set end of analysis interval to <time> seconds (default: end of file)
##' @param WindowShift = <dur>: set analysis window shift to <dur> ms (default: 5.0)
##' @param WindowSize = <dur>:  set analysis window size to <dur> ms (default: 25.0)
##' @param ToFile write results to file (default extension is .zcr)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @param forceToLog is set by the global package variable useWrasspLogger. This is set
##' to FALSE by default and should be set to TRUE is logging is desired.
##' @return nrOfProcessedFiles or if only one file to process return AsspDataObj of that file
##' @author Raphael Winkelmann
##' @useDynLib wrassp
##' @export
'zcrana' <- function(listOfFiles = NULL, optLogFilePath = NULL, 
                     BeginTime = 0.0, CenterTime = FALSE, 
                     EndTime = 0.0, WindowShift = 5.0, 
                     WindowSize = 25.0, ToFile = TRUE, 
                     ExplicitExt = NULL, forceToLog = useWrasspLogger){

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

	###########################
	# remove file:// and expand listOfFiles (SIC)
	
	listOfFiles = gsub("^file://","", listOfFiles)
	listOfFiles = path.expand(listOfFiles)
        
	###########################
	# perform analysis

	if(length(listOfFiles)==1){
    pb <- NULL
	}else{
    cat('\n  INFO: applying zcrana to', length(listOfFiles), 'files\n')
    pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}	

	externalRes = invisible(.External("performAssp", PACKAGE = "wrassp", 
                                    listOfFiles, fname = "zcrana", 
                                    BeginTime = BeginTime, CenterTime = CenterTime, 
                                    EndTime = EndTime, WindowShift = WindowShift, 
                                    WindowSize = WindowSize, 
                                    ToFile = ToFile, ExplicitExt = ExplicitExt, 
                                    ProgressBar = pb))


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

