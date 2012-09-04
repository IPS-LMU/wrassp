'mhspitch' <-function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, WindowShift = 5.0, Gender = 'u', MaxF = 600.0, MinF = 50.0, MinAmp = 50.0, MinAC1 = 0.25, MinRMS = 18.0, MaxZCR = 3000.0, MinProb = 0.52, PlainSpectrum = FALSE, ToFile = TRUE, ExplicitExt = NULL) {
	
	.External("performAssp", listOfFiles, fname = "mhspitch", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, WindowShift = WindowShift, Gender = Gender, MaxF = MaxF, MinF = MinF, MinAmp = MinAmp, MinAC1 = MinAC1, MinRMS = MinRMS, MaxZCR = MaxZCR, MinProb = MinProb, PlainSpectrum = PlainSpectrum, ToFile = ToFile, ExplicitExt = ExplicitExt)
	
}