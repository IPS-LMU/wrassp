##' afdiff adapted from assp
##'
##' still have to write propper manual entry
##' @title afdiff
##' @param listOfFiles vector of file paths to be processed by function
##' @param optLogFilePath path to option log file
##' @param ComputeBackwardDifference ???
##' @param ComputeCentralDifference ???
##' @param Channel channel of audio file to compute diff (default = 1)
##' @param ToFile write results to file (default extension is .d+(extensionsOfAudioFile))
##' @param ExplicitExt set if you wish to overwride the default extension 
##' @return nrOfProcessedFiles or if only one file to process return dataObj of that file
##' @author Raphael Winkelmann
'afdiff' <- function(listOfFiles = NULL, optLogFilePath = NULL,ComputeBackwardDifference = FALSE, ComputeCentralDifference = FALSE, Channel = 1, ToFile = TRUE, ExplicitExt=NULL) {

	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}

        if (is.null(optLogFilePath)){
          stop("optLogFilePath is NULL!")
        }
	
	###########################
	# perform analysis
	
	if(length(listOfFiles)==1){
		pb <- NULL
	}else{
		pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}
	
	invisible(.External("performAssp", listOfFiles, fname = "afdiff",ComputeBackwardDifference  = ComputeBackwardDifference, Channel = as.integer(Channel), ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar=pb, PACKAGE = "wrassp"))


        #############################
        # return dataObj if length only one file
        
	if(!(length(listOfFiles)==1)){
          close(pb)
        }else{
          resDataObj = getDObj(listOfFiles[1])
          return(resDataObj)
        }
}
