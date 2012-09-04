"afdiff" <- function(listOfFiles = NULL, ComputeBackwardDifference = FALSE, ComputeCentralDifference = FALSE, Channel = 1, ToFile = TRUE, ExplicitExt=NULL) {
		if (is.null(x)) {
		stop("listOfFiles as the paths of the files to perform the analysis on has to be set. A string or a vektor of paths has to be given to the function")}
	
	###########################
	# a few parameter checks
	
	
	.External("performAssp", listOfFiles, fname = "afdiff",ComputeBackwardDifference  = ComputeBackwardDifference, Channel = Channel, ToFile = ToFile, ExplicitExt = ExplicitExt)
		
}