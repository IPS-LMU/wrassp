"f0ana" <- function(listOfFiles = NULL, BeginTime = 0.0, EndTime = 0.0, WindowShift = 5.0, Gender = 'u', MaxF = 600, MinF = 50, MinAmp = 50, MaxZCR = 3000.0, ToFile = TRUE, ExplicitExt = NULL){
	
	
	.External("performAssp", listOfFiles, fname = "f0ana", BeginTime = BeginTime, EndTime = EndTime, WindowShift = WindowShift, Gender = Gender, MaxF = MaxF, MinF = MinF, MinAmp = MinAmp, MaxZCR = MaxZCR, ExplicitExt = ExplicitExt, ToFile = ToFile)
	
}