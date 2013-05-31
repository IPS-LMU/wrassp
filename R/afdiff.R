##' afdiff function adapted from libassp
##'
##' Computes the first difference of the signal in the audio-
##' formatted file(s) <listOfFiles>. The differentiated signal will
##' be written to a file with the base name of the input file
##' and an extension consisting of '.d', followed by the
##' extension of the input file. The format of the output file
##' will be the same as that of the input file.
##' Differentiation can improve results an F0 analysis of e.g.
##' EGG signals because it removes a DC offset, attenuates
##' very low frequency components - and in the case of central
##' differentiation also very high ones - and enhances the
##' moment of glottal closure.
##' @title afdiff
##' @param listOfFiles vector of file paths to be processed by function
##' @param optLogFilePath path to option log file
##' @param ComputeBackwardDifference compute backward difference (s'[n] = s[n] - s[n-1]) (default: forward difference s'[n] = s[n+1] - s[n])
##' @param ComputeCentralDifference compute central/interpolated/3-point difference
##' @param Channel = <num>: for multi-channel input files: extract and differentiate channel <num> (1 <= <num> <= 8  default: channel 1)
##' @param ToFile write results to file (default extension is .d+(extensionsOfAudioFile))
##' @param ExplicitExt set if you wish to overwride the default extension 
##' @param forceToLog option to override the package default. Option should be left alone by normal user
##' @return nrOfProcessedFiles or if only one file to process return AsspDataObj of that file
##' @author Raphael Winkelmann
##' @useDynLib wrassp
##' @export
'afdiff' <- function(listOfFiles = NULL, optLogFilePath = NULL, 
                     ComputeBackwardDifference = FALSE, ComputeCentralDifference = FALSE, 
                     Channel = 1, ToFile = TRUE, 
                     ExplicitExt=NULL, forceToLog = forceToLogDefault){

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

  listOfFiles = path.expand(listOfFiles)
        
	###########################
	# perform analysis
	
	if(length(listOfFiles)==1){
    pb <- NULL
	}else{
    cat('\n  INFO: applying afdiff to', length(listOfFiles), 'files\n')
    pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}
	
	externalRes = invisible(.External("performAssp", listOfFiles, 
                                    fname = "afdiff", ComputeBackwardDifference = ComputeBackwardDifference, 
                                    Channel = as.integer(Channel), ToFile = ToFile, 
                                    ExplicitExt = ExplicitExt, ProgressBar=pb, 
                                    PACKAGE = "wrassp"))
	
  
  ############################
	# write options to options log file

	if (forceToLog){
	  cat("\n##################################\n", file = optLogFilePath, append = T)
	  cat("##################################\n", file = optLogFilePath, append = T)
	  cat("######## afdiff performed ########\n", file = optLogFilePath, append = T)
	  
	  cat("Timestamp: ", paste(Sys.time()), '\n', file = optLogFilePath, append = T)
	  
	  cat("ComputeBackwardDifference: ", ComputeBackwardDifference, '\n', file = optLogFilePath, append = T)
	  cat("Channel: ", Channel, '\n', file = optLogFilePath, append = T)
	  cat("ToFile: ", ToFile, '\n', file = optLogFilePath, append = T)
	  cat("ExplicitExt: ", ExplicitExt, '\n', file = optLogFilePath, append = T)
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
