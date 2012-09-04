##########################################################
'dftSpectrum' <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, Resolution = 40.0, FftLength = 0, WindowSize = 0.0, WindowShift = 5.0, Window = 'BLACKMAN', Bandwidth = 0.0, EffectiveLength = FALSE, Order = 0, Preemphasis = 0.0, Deemphasize = FALSE, NumCeps = 0, ToFile = TRUE, ExplicitExt = NULL) {
	
	SpectrumType = 'DFT'
	
	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}
	
	if(!isAsspWindowType(Window)){
		stop("WindowFunction of type '", Window,"' is not supported!")
	}
	
	
	
	.External("performAssp", listOfFiles, fname = "spectrum", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, SpectrumType = SpectrumType, Resolution = Resolution, FftLength = FftLength, WindowSize = WindowSize, WindowShift = WindowShift,  Window = Window, Bandwidth = Bandwidth, EffectiveLength = EffectiveLength, Order = Order, Preemphasis = Preemphasis, Deemphasize = Deemphasize, NumCeps = NumCeps, ToFile = ToFile, ExplicitExt = ExplicitExt)
		
}

###########################################################

'lpsSpectrum' <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, Resolution = 40.0, FftLength = 0, WindowSize = 20.0, WindowShift = 5.0, Window = 'BLACKMAN', Bandwidth = 0.0, EffectiveLength = FALSE, Order = 0, Preemphasis = -0.95, Deemphasize = FALSE, NumCeps = 0, ToFile = TRUE, ExplicitExt = NULL) {
	
	SpectrumType = 'LPS'
	
	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}

	if(!isAsspWindowType(Window)){
		stop("WindowFunction of type '", Window,"' is not supported!")
	}
	
	.External("performAssp", listOfFiles, fname = "spectrum", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, SpectrumType = SpectrumType, Resolution = Resolution, FftLength = FftLength, WindowSize = WindowSize, WindowShift = WindowShift,  Window = Window, Bandwidth = Bandwidth, EffectiveLength = EffectiveLength, Order = Order, Preemphasis = Preemphasis, Deemphasize = Deemphasize, NumCeps = NumCeps, ToFile = ToFile, ExplicitExt = ExplicitExt)
		
}

###########################################################

'cssSpectrum' <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, Resolution = 40.0, FftLength = 0, WindowSize = 0.0, WindowShift = 5.0, Window = 'BLACKMAN', Bandwidth = 0.0, EffectiveLength = FALSE, Order = 0, Preemphasis = 0.0, Deemphasize = FALSE, NumCeps = 0, ToFile = TRUE, ExplicitExt = NULL) {
	
	SpectrumType = 'css'
	
	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}
	
	if(!isAsspWindowType(Window)){
		stop("WindowFunction of type '", Window,"' is not supported!")
	}
	
	.External("performAssp", listOfFiles, fname = "spectrum", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, SpectrumType = SpectrumType, Resolution = Resolution, FftLength = FftLength, WindowSize = WindowSize, WindowShift = WindowShift,  Window = Window, Bandwidth = Bandwidth, EffectiveLength = EffectiveLength, Order = Order, Preemphasis = Preemphasis, Deemphasize = Deemphasize, NumCeps = NumCeps, ToFile = ToFile, ExplicitExt = ExplicitExt)
		
}

###########################################################

'cepSpectrum' <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, Resolution = 40.0, FftLength = 0, WindowSize = 0.0, WindowShift = 5.0, Window = 'BLACKMAN', Bandwidth = 0.0, EffectiveLength = FALSE, Order = 0, Preemphasis = 0.0, Deemphasize = FALSE, NumCeps = 0, ToFile = TRUE, ExplicitExt = NULL) {
	
	SpectrumType = 'cep'
	
	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}
	
	if(!isAsspWindowType(Window)){
		stop("WindowFunction of type '", Window,"' is not supported!")
	}
	
	
	.External("performAssp", listOfFiles, fname = "spectrum", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, SpectrumType = SpectrumType, Resolution = Resolution, FftLength = FftLength, WindowSize = WindowSize, WindowShift = WindowShift,  Window = Window, Bandwidth = Bandwidth, EffectiveLength = EffectiveLength, Order = Order, Preemphasis = Preemphasis, Deemphasize = Deemphasize, NumCeps = NumCeps, ToFile = ToFile, ExplicitExt = ExplicitExt)
		
}
