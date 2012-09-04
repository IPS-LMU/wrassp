"affilter" <- function(listOfFiles = NULL, HighPass = 4000.0, LowPass = 0.0, StopBand = 96.0, Transition = 250.0, UseIIR = FALSE, NumIIRsections = 4, ToFile = TRUE, ExplicitExt = NULL){
	
	
		.External("performAssp", listOfFiles, fname = "affilter", HighPass = HighPass, LowPass = LowPass, StopBand = StopBand, Transition = Transition, UseIIR = UseIIR, NumIIRsections = NumIIRsections, ToFile = ToFile, ExplicitExt = ExplicitExt)
	
}