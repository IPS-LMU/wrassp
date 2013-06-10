##' rfcana function adapted from libassp 
##'
##' Linear Prediction analysis of <listOfFiles> using the
##' autocorrelation method and the Durbin recursion.
##' This program calculates the RMS amplitudes of the input
##' and residual signal in dB and, per default, reflection
##' coefficients (see '-t' option).
##' Analysis results will be written to a file with the
##' base name of the input file and the parameter type in
##' lower case as extension (e.g. '.rfc').
##' Default output is in SSFF binary format (tracks 'rms',
##' 'gain' and the LP type in lower case).
##' @title rfcana
##' @param listOfFiles vector of file paths to be processed by function 
##' @param optLogFilePath path to option log file
##' @param BeginTime = <time>: set begin of analysis interval to <time> seconds (default = 0: begin of file)
##' @param CenterTime set single-frame analysis with the analysis window centred at <time> seconds; overrules BeginTime, EndTime and WindowShift options
##' @param EndTime = <time>: set end of analysis interval to <time> seconds (default = 0: end of file)
##' @param WindowShift = <dur>: set analysis window shift to <dur> ms (default: 5.0)
##' @param WindowSize = <dur>: set analysis window size to <dur> ms; overrules EffectiveLength option
##' @param EffectiveLength make window size effective rather than exact
##' @param Window = <type>: set analysis window function to <type> (default: BLACKMAN)
##' @param Order = <num>: set prediction order to <num> (default: sample rate in kHz + 3)
##' @param Preemphasis = <val>: set pre-emphasis factor to <val> (default: -0.95)
##' @param LpType = <type>: calculate <type> LP parameters; <type> may be:
##' "ARF": area function
##' "LAR": log area ratios
##' "LPC": linear prediction filter coefficients
##' "RFC": reflection coefficients (default)
##' @param ToFile  write results to file (default extension dependent on LpType .arf/.lar/.lpc/.rfc)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @param forceToLog option to override the package default. Option should be left alone by normal user
##' @return nrOfProcessedFiles or if only one file to process return AsspDataObj of that file
##' @author Raphael Winkelmann
##' @useDynLib wrassp
##' @export
'rfcana' <- function(listOfFiles = NULL, optLogFilePath = NULL, 
                     BeginTime = 0.0, CenterTime = FALSE, 
                     EndTime = 0.0, WindowShift = 5.0, 
                     WindowSize = 20.0, EffectiveLength = TRUE, 
                     Window = 'BLACKMAN', Order = 0, 
                     Preemphasis = -0.95, LpType = 'RFC', 
                     ToFile = TRUE, ExplicitExt = NULL,
                     forceToLog = TRUE){
	
	
	###########################
	# a few parameter checks and expand files
	
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
	
	if(!isAsspLpType(LpType)){
		stop("LpType of type '", LpType,"' is not supported!")
	}
        
  listOfFiles = path.expand(listOfFiles)
  
	
	###########################
	# perform analysis

	if(length(listOfFiles)==1){
    pb <- NULL
	}else{
    cat('\n  INFO: applying rfcana to', length(listOfFiles), 'files\n')
    pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}
	
	
	externalRes = invisible(.External("performAssp", listOfFiles, 
                                    fname = "rfcana", BeginTime = BeginTime, 
                                    CenterTime = CenterTime, EndTime = EndTime, 
                                    WindowShift = WindowShift, WindowSize = WindowSize, 
                                    EffectiveLength = EffectiveLength, Window = Window, 
                                    Order = as.integer(Order), 
                                    Preemphasis = Preemphasis, LpType = LpType, 
                                    ToFile = ToFile, ExplicitExt = ExplicitExt, 
                                    ProgressBar = pb, PACKAGE = "wrassp"))

  ############################
  # write options to options log file
  
	if (forceToLog){
    cat("\n##################################\n", file = optLogFilePath, append = T)
    cat("##################################\n", file = optLogFilePath, append = T)
    cat("######## rfcana performed ########\n", file = optLogFilePath, append = T)

    cat("Timestamp: ", paste(Sys.time()), '\n', file = optLogFilePath, append = T)

    cat("BeginTime: ", BeginTime, '\n', file = optLogFilePath, append = T)
    cat("CenterTime: ", CenterTime, '\n', file = optLogFilePath, append = T)
    cat("EndTime: ", EndTime, '\n', file = optLogFilePath, append = T)
    cat("WindowShift: ", WindowShift, '\n', file = optLogFilePath, append = T)
    cat("WindowSize: ", WindowSize, '\n', file = optLogFilePath, append = T)
    cat("EffectiveLength: ", EffectiveLength, '\n', file = optLogFilePath, append = T)
    cat("Window: ", Window, '\n', file = optLogFilePath, append = T)
    cat("Order: ", Order, '\n', file = optLogFilePath, append = T)
    cat("Preemphasis: ", Preemphasis, '\n', file = optLogFilePath, append = T)
    cat("LpType: ", LpType, '\n', file = optLogFilePath, append = T)
        

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
