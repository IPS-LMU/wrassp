"acfana" <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, 
	EndTime = 0.0, WindowShift = 5.0, WindowSize = 20.0, EffectiveLength = TRUE, 
	WindowFunction = "BLACKMAN", AnalysisOrder = 0, EnergyNormalization = FALSE, LengthNormalization = FALSE, ToFile = TRUE, ExplicitExt = NULL) {
	if (is.null(x)) {
		stop("listOfFiles as the paths of the files to perform the analysis on has to be set. A string or a vektor of paths has to be given to the function")

	}
	###########################
	# a few parameter checks
if (round(AnalysisOrder) != AnalysisOrder) { #strange check if int number seeing as 
		stop("order parameter is not allowed to be a decimal value")
	}


	if(!checkAsspWindowType(WindowFunction)){
		stop("WindowFunction of type '", WindowFunction,"' is not supported!")
	}
	

	.External("performAssp", listOfFiles, fname = "acfana", BeginTime = BeginTime, 
		CenterTime = CenterTime, EndTime = EndTime, WindowShift = WindowShift, WindowSize = WindowSize, EffectiveLength = EffectiveLength, WindowFunction = WindowFunction, AnalysisOrder = AnalysisOrder, EnergyNormalization = EnergyNormalization, LengthNormalization = LengthNormalization, ToFile = ToFile, ExplicitExt = ExplicitExt)
}
