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
                     ToFile = TRUE, ExplicitExt = NULL,
                     forceToLog = forceToLogDefault){

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
        
  listOfFiles = path.expand(listOfFiles)
        
	###########################
	# perform analysis

	if(length(listOfFiles)==1){
    pb <- NULL
	}else{
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
                                    PACKAGE = "wrassp"))

	############################
	# write options to options log file
	if (forceToLog){
	  
	  cat("\n##################################\n", file = optLogFilePath, append = T)
	  cat("##################################\n", file = optLogFilePath, append = T)
	  cat("######## acfana performed ########\n", file = optLogFilePath, append = T)
	  
	  cat("Timestamp: ", paste(Sys.time()), '\n', file = optLogFilePath, append = T)
	  cat("BeginTime: ", BeginTime, "\n", file = optLogFilePath, append = T)
	  cat("CenterTime: ", CenterTime, "\n", file = optLogFilePath, append = T) 
	  cat("EndTime: ", EndTime, "\n", file = optLogFilePath, append = T)
	  cat("WindowShift: ", WindowShift, "\n", file = optLogFilePath, append = T)
	  cat("WindowSize: ", WindowSize, "\n", file = optLogFilePath, append = T)
	  cat("EffectiveLength: ", EffectiveLength, "\n", file = optLogFilePath, append = T)
	  cat("Window: ", Window, "\n", file = optLogFilePath, append = T)
	  cat("AnalysisOrder: ", AnalysisOrder, "\n", file = optLogFilePath, append = T)
	  cat("EnergyNormalization: ", EnergyNormalization, "\n", file = optLogFilePath, append = T) 
	  cat("LengthNormalization: ", LengthNormalization, "\n", file = optLogFilePath, append = T)
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
