##' rmsana function adapted from libassp
##'
##' Analysis of short-term Root Mean Square amplitude of
##' the signal in <listOfFiles>. Per default, the RMS values are
##' expressed in decibel (dB) so that they correspond to
##' the short-term power of the signal.
##' Analysis results will be written to a file with the
##' base name of the input file and extension '.rms'.
##' Default output is in SSFF binary format (track 'rms').
##' @title rmsana
##' @param listOfFiles vector of file paths to be processed by function
##' @param optLogFilePath path to option log file
##' @param BeginTime = <time>:  set begin of analysis interval to <time> seconds (default = 0: begin of file)
##' @param CenterTime = <time>: set single-frame analysis with the analysis window centred at <time> seconds; overrules BeginTime, EndTime and WindowShift options
##' @param EndTime = <time>: set end of analysis interval to <time> seconds (default: end of file)
##' @param WindowShift = <dur>: set analysis window shift to <dur> ms (default: 5.0)
##' @param WindowSize = <dur>: set analysis window size to <dur> ms; overrules EffectiveLength option
##' @param EffectiveLength make window size effective rather than exact
##' @param Linear calculate linear RMS values (default: values in dB)
##' @param Window = <type>: set analysis window function to <type> (default: HAMMING)
##' @param ToFile write results to file (default extension is .rms)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @return nrOfProcessedFiles or if only one file to process return AsspDataObj of that file
##' @author Raphael Winkelmann
'rmsana' <- function(listOfFiles = NULL, optLogFilePath = NULL,
                     BeginTime = 0.0, CenterTime = FALSE, 
                     EndTime = 0.0, WindowShift = 5.0, 
                     WindowSize = 20.0, EffectiveLength = TRUE, 
                     Linear = FALSE, Window = 'HAMMING', 
                     ToFile = TRUE, ExplicitExt = NULL) {


	###########################
	# a few parameter checks and expand paths
	
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
	
	if(!isAsspWindowType(Window)){
		stop("WindowFunction of type '", Window,"' is not supported!")
	}


  listOfFiles = path.expand(listOfFiles)
  
	###########################
	# perform analysis

	if(length(listOfFiles)==1){
          pb <- NULL
	}else{
          cat('\n  INFO: applying rmsana to', length(listOfFiles), 'files\n')
          pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}	
	
	externalRes = invisible(.External("performAssp", listOfFiles, 
                                    fname = "rmsana", BeginTime = BeginTime, 
                                    CenterTime = CenterTime, EndTime = EndTime, 
                                    WindowShift = WindowShift, WindowSize = WindowSize, 
                                    EffectiveLength = EffectiveLength, Linear = Linear, 
                                    Window = Window, ToFile = ToFile, 
                                    ExplicitExt = ExplicitExt, 
                                    ProgressBar = pb, PACKAGE = "wrassp"))
	
  ############################
	# write options to options log file
  
	if (!is.null(optLogFilePath)){
    cat("\n##################################\n", file = optLogFilePath, append = T)
    cat("##################################\n", file = optLogFilePath, append = T)
    cat("######## rmsana performed ########\n", file = optLogFilePath, append = T)

    cat("Timestamp: ", paste(Sys.time()), '\n', file = optLogFilePath, append = T)
    cat("BeginTime: ", BeginTime, '\n', file = optLogFilePath, append = T)
    cat("CenterTime: ", CenterTime, '\n', file = optLogFilePath, append = T)
    cat("EndTime: ", EndTime, '\n', file = optLogFilePath, append = T)
    cat("WindowShift: ", WindowShift, '\n', file = optLogFilePath, append = T)
    cat("WindowSize: ", WindowSize, '\n', file = optLogFilePath, append = T)
    cat("EffectiveLength: ", EffectiveLength, '\n', file = optLogFilePath, append = T)
    cat("Linear: ", Linear, '\n', file = optLogFilePath, append = T)
    cat("Window: ", Window, '\n', file = optLogFilePath, append = T)

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

