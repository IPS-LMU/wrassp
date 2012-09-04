'rmsana' <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, WindowShift = 5.0, WindowSize = 20.0, EffectiveLength = TRUE, Linear = FALSE, Window = 'HAMMING', ToFile = TRUE, ExplicitExt = NULL) {
	
	.External("performAssp", listOfFiles, fname = "rmsana", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, WindowShift = WindowShift, WindowSize = WindowSize, EffectiveLength = EffectiveLength, Linear = Linear, Window = Window, ToFile = ToFile, ExplicitExt = ExplicitExt)
	
}