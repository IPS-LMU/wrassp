##' forest function adapted from libassp
##'
##' Formant estimation of the signal(s) in <listOfFiles>.
##' Raw resonance frequency and bandwidth values are
##' obtained by root-solving of the Linear Prediction
##' polynomial from the autocorrelation method and the
##' Split-Levinson-Algorithm (SLA). Resonances are then
##' classified as formants using the so-called Pisarenko
##' frequencies (by-product of the SLA) and a formant
##' frequeny range table derived from the nominal F1
##' frequency. The latter may have to be increased by
##' about 12% for female voices (see NominalF1 and Gender options).
##' Formant estimates will be written to a file with the
##' base name of the input file and extension '.fms'.
##' Default output is in SSFF binary format (tracks 'fm'
##' and ''bw)
##' @title forest
##' @param listOfFiles vector of file paths to be processed by function
##' @param optLogFilePath path to option log file
##' @param BeginTime = <time>: set begin of analysis interval to <time> seconds (default = 0: begin of data)
##' @param EndTime = <time>:  set end of analysis interval to <time> seconds (default = 0: end of data)
##' @param WindowShift = <dur>: set analysis window shift to <dur> ms (default: 5.0)
##' @param WindowSize  = <dur>: set analysis window size to <dur> ms (default: 30.0)
##' @param EffectiveLength make window size effective rather than exact
##' @param NominalF1 = <freq>: set nominal F1 frequency to <freq> Hz (default: 500.0 Hz)
##' @param Gender = <code>: set gender specific parameters where <code> = f[emale], m[ale] or u[nknown] (when <code>=f: eff. window length = 12.5 ms nominal F1 = 560.0 Hz)
##' @param Estimate insert rough frequency estimates of missing formants (default: frequency set to zero)
##' @param Order decrease default order by 2 (one resonance less)
##' @param IncrOrder increase default order by 2 (one resonance more)
##' @param NumFormants = <num>: set number of formants to <num> (default: 4;  maximum: 8 or half the LP order)
##' @param Window = <type>: set analysis window function to <type> (default: BLACKMAN)
##' @param Preemphasis = <val>: set pre-emphasis factor to <val> (-1 <= val <= 0) (default: dependent on sample rate and nominal F1)
##' @param ToFile write results to file (default extension is .fms)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @param forceToLog is set by the global package variable useWrasspLogger. This is set
##' to FALSE by default and should be set to TRUE is logging is desired.
##' @return nrOfProcessedFiles or if only one file to process return AsspDataObj of that file
##' @author Raphael Winkelmann
##' @useDynLib wrassp
##' @export
'forest' <- function(listOfFiles = NULL, optLogFilePath = NULL,
                     BeginTime = 0.0, EndTime = 0.0, 
                     WindowShift = 5.0, WindowSize = 20.0, 
                     EffectiveLength = TRUE, NominalF1 = 500, 
                     Gender = 'm', Estimate = FALSE, 
                     Order = 0, IncrOrder = 0, 
                     NumFormants = 4, Window = 'BLACKMAN', 
                     Preemphasis = -0.8, ToFile = TRUE, 
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
	
	if(!isAsspWindowType(Window)){
		stop("WindowFunction of type '", Window,"' is not supported!")
	}

	###########################
	# remove file:// and expand listOfFiles (SIC)
	
	listOfFiles = gsub("^file://","", listOfFiles)
	listOfFiles = path.expand(listOfFiles)
	
	###########################
	#perform analysis
	
	if(length(listOfFiles)==1){
    pb <- NULL
	}else{
    cat('\n  INFO: applying forest to', length(listOfFiles), 'files\n')
    pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}	
	
	
	externalRes = invisible(.External("performAssp", listOfFiles, 
                                    fname = "forest", BeginTime =  BeginTime, 
                                    EndTime = EndTime, WindowShift = WindowShift, 
                                    WindowSize = WindowSize, EffectiveLength = EffectiveLength, 
                                    NominalF1 = NominalF1, Gender = Gender, 
                                    Estimate = Estimate, Order = as.integer(Order), 
                                    IncrOrder = as.integer(IncrOrder), NumFormants = as.integer(NumFormants), 
                                    Window = Window, Preemphasis = Preemphasis, 
                                    ToFile = ToFile, ExplicitExt = ExplicitExt, 
                                    ProgressBar = pb, PACKAGE = "wrassp"))
	
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
