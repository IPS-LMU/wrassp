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
	
	###########################
	# perform analysis

	if(length(listOfFiles)==1){
		pb <- NULL
	}else{
		pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}	
	
	invisible(.External("performAssp", listOfFiles, fname = "spectrum", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, SpectrumType = SpectrumType, Resolution = Resolution, FftLength = as.integer(FftLength), WindowSize = WindowSize, WindowShift = WindowShift,  Window = Window, Bandwidth = Bandwidth, EffectiveLength = EffectiveLength, Order = as.integer(Order), Preemphasis = Preemphasis, Deemphasize = Deemphasize, NumCeps = as.integer(NumCeps), ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb))
	
	if(!(length(listOfFiles)==1)){ close(pb) }
		
}

###########################################################

'lpsSpectrum' <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, Resolution = 40.0, FftLength = 0, WindowSize = 20.0, WindowShift = 5.0, Window = 'BLACKMAN', Bandwidth = 0.0, EffectiveLength = FALSE, Order = 0, Preemphasis = -0.95, Deemphasize = FALSE, NumCeps = 0, ToFile = TRUE, ExplicitExt = NULL) {
	
	stop("DEFAULT VALUES WRONG! NOT IMPLEMENTED YET!")
	
	SpectrumType = 'LPS'
	
	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}

	if(!isAsspWindowType(Window)){
		stop("WindowFunction of type '", Window,"' is not supported!")
	}
	
	###########################
	# perform analysis

	if(length(listOfFiles)==1){
		pb <- NULL
	}else{
		pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}
	
	invisible(.External("performAssp", listOfFiles, fname = "spectrum", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, SpectrumType = SpectrumType, Resolution = Resolution, FftLength = as.integer(FftLength), WindowSize = WindowSize, WindowShift = WindowShift,  Window = Window, Bandwidth = Bandwidth, EffectiveLength = EffectiveLength, Order = as.integer(Order), Preemphasis = Preemphasis, Deemphasize = Deemphasize, NumCeps = as.integer(NumCeps), ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb))
	
	if(!(length(listOfFiles)==1)){ close(pb) }
		
}

###########################################################

'cssSpectrum' <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, Resolution = 40.0, FftLength = 0, WindowSize = 0.0, WindowShift = 5.0, Window = 'BLACKMAN', Bandwidth = 0.0, EffectiveLength = FALSE, Order = 0, Preemphasis = 0.0, Deemphasize = FALSE, NumCeps = 0, ToFile = TRUE, ExplicitExt = NULL) {
	
	stop("DEFAULT VALUES WRONG! NOT IMPLEMENTED YET!")
	
	SpectrumType = 'css'
	
	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}
	
	if(!isAsspWindowType(Window)){
		stop("WindowFunction of type '", Window,"' is not supported!")
	}
	
	###########################
	# perform analysis

	if(length(listOfFiles)==1){
		pb <- NULL
	}else{
		pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}	
	
	
	invisible(.External("performAssp", listOfFiles, fname = "spectrum", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, SpectrumType = SpectrumType, Resolution = Resolution, FftLength = as.integer(FftLength), WindowSize = WindowSize, WindowShift = WindowShift,  Window = Window, Bandwidth = Bandwidth, EffectiveLength = EffectiveLength, Order = as.integer(Order), Preemphasis = Preemphasis, Deemphasize = Deemphasize, NumCeps = as.integer(NumCeps), ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb))
	
	if(!(length(listOfFiles)==1)){ close(pb) }
		
}

###########################################################

'cepSpectrum' <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, Resolution = 40.0, FftLength = 0, WindowSize = 0.0, WindowShift = 5.0, Window = 'BLACKMAN', Bandwidth = 0.0, EffectiveLength = FALSE, Order = 0, Preemphasis = 0.0, Deemphasize = FALSE, NumCeps = 0, ToFile = TRUE, ExplicitExt = NULL) {
	
	stop("DEFAULT VALUES WRONG! NOT IMPLEMENTED YET!")
	
	SpectrumType = 'cep'
	
	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}
	
	if(!isAsspWindowType(Window)){
		stop("WindowFunction of type '", Window,"' is not supported!")
	}
	
	###########################
	# perform analysis

	if(length(listOfFiles)==1){
		pb <- NULL
	}else{
		pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}	

	
	
	invisible(.External("performAssp", listOfFiles, fname = "spectrum", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, SpectrumType = SpectrumType, Resolution = Resolution, FftLength = FftLength, WindowSize = WindowSize, WindowShift = WindowShift,  Window = Window, Bandwidth = Bandwidth, EffectiveLength = EffectiveLength, Order = Order, Preemphasis = Preemphasis, Deemphasize = Deemphasize, NumCeps = NumCeps, ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb))
		
	if(!(length(listOfFiles)==1)){ close(pb) }
}
