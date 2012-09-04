"acfana" <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, 
	EndTime = 0.0, WindowShift = 5.0, WindowSize = 20.0, EffectiveLength = TRUE, 
	Window = "BLACKMAN", AnalysisOrder = 0, EnergyNormalization = FALSE, LengthNormalization = FALSE, ToFile = TRUE, ExplicitExt = NULL) {

	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}

	#DON'T FORGET TO RENAME WindowFunction to window in performAssp.c!!!!

	if(!isAsspWindowType(Window)){
		stop("WindowFunction of type '", Window,"' is not supported!")
	}
	

	.External("performAssp", listOfFiles, fname = "acfana", BeginTime = BeginTime, 
		CenterTime = CenterTime, EndTime = EndTime, WindowShift = WindowShift, WindowSize = WindowSize, EffectiveLength = EffectiveLength, WindowFunction = WindowFunction, AnalysisOrder = AnalysisOrder, EnergyNormalization = EnergyNormalization, LengthNormalization = LengthNormalization, ToFile = ToFile, ExplicitExt = ExplicitExt)

}
