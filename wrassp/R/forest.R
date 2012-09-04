"forest" <- function(listOfFiles = NULL, BeginTime = 0.0, EndTime = 0.0, WindowShift = 5.0, WindowSize = 20.0, EffectiveLength = TRUE, NominalF1 = 500, Gender = 'm', Estimate = FALSE, Order = 0, IncrOrder = 0, NumFormants = 4, Window = 'BLACKMAN', Preemphasis = -0.8, ToFile = TRUE, ExplicitExt = NULL) {
	
	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}
	
	
	.External("performAssp", listOfFiles, fname = "forest", BeginTime =  BeginTime, EndTime = EndTime, WindowShift = WindowShift, WindowSize = WindowSize, EffectiveLength = EffectiveLength, NominalF1 = NominalF1, Gender = Gender, Estimate = Estimate, Order = Order, IncrOrder = IncrOrder, NumFormants = NumFormants, Window = Window, Preemphasis = Preemphasis, ToFile = ToFile, ExplicitExt = ExplicitExt)
	
}