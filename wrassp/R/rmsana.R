'rmsana' <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, WindowShift = 5.0, WindowSize = 20.0, EffectiveLength = TRUE, Linear = FALSE, Window = 'HAMMING', ToFile = TRUE, ExplicitExt = NULL) {


	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}
	
	if(!isAsspWindowType(Window)){
		stop("WindowFunction of type '", Window,"' is not supported!")
	}
	
	
	
	.External("performAssp", listOfFiles, fname = "rmsana", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, WindowShift = WindowShift, WindowSize = WindowSize, EffectiveLength = EffectiveLength, Linear = Linear, Window = Window, ToFile = ToFile, ExplicitExt = ExplicitExt)
	
}