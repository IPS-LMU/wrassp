"rfcana" <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, WindowShift = 5.0, WindowSize = 20.0, EffectiveLength = TRUE, Window = 'BLACKMAN', Order = 0, Preemphasis = -0.95, LpType = 'RFC', ToFile = TRUE, ExplicitExt = NULL) {
	
	
	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}
	
	if(!isAsspWindowType(Window)){
		stop("WindowFunction of type '", Window,"' is not supported!")
	}
	
	
	.External("performAssp", listOfFiles, fname = "rfcana", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, WindowShift = WindowShift, WindowSize = WindowSize, EffectiveLength = EffectiveLength, Window = Window, Order = Order, Preemphasis = Preemphasis, LpType = LpType, ToFile = ToFile, ExplicitExt = ExplicitExt)
	
}