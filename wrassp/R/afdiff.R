##' afdiff adapted from assp
##'
##' still have to write propper manual entry
##' @title afdiff
##' @param listOfFiles 
##' @param ComputeBackwardDifference 
##' @param ComputeCentralDifference 
##' @param Channel 
##' @param ToFile 
##' @param ExplicitExt 
##' @return nr of files processed
##' @author Raphael Winkelmann
"afdiff" <- function(listOfFiles = NULL, ComputeBackwardDifference = FALSE, ComputeCentralDifference = FALSE, Channel = 1, ToFile = TRUE, ExplicitExt=NULL) {

	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}
	
	###########################
	# perform analysis
	
	if(length(listOfFiles)==1){
		pb <- NULL
	}else{
		pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}
	
	invisible(.External("performAssp", listOfFiles, fname = "afdiff",ComputeBackwardDifference  = ComputeBackwardDifference, Channel = as.integer(Channel), ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar=pb))


	if(!(length(listOfFiles)==1)){ close(pb) }
}
