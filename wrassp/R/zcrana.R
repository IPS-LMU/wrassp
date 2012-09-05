"zcrana" <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, WindowShift = 5.0, WindowSize = 25.0, ToFile = TRUE, ExplicitExt = NULL) {

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

	invisible(.External("performAssp", listOfFiles, fname = "zcrana", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, WindowShift = WindowShift, WindowSize = WindowSize, ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb))
	
	
	if(!(length(listOfFiles)==1)){ close(pb) }

}