"afdiff" <- function(listOfFiles = NULL, ComputeBackwardDifference = FALSE, ComputeCentralDifference = FALSE, Channel = 1, ToFile = TRUE, ExplicitExt=NULL) {

	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}
	
	.External("performAssp", listOfFiles, fname = "afdiff",ComputeBackwardDifference  = ComputeBackwardDifference, Channel = as.integer(Channel), ToFile = ToFile, ExplicitExt = ExplicitExt)
		
}