"acfana" <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, 
	EndTime = 0.0, WindowShift = 5.0, WindowSize = 20.0, EffectiveLength = TRUE, 
	Window = "BLACKMAN", AnalysisOrder = 0, EnergyNormalization = FALSE, LengthNormalization = FALSE, ToFile = TRUE, ExplicitExt = NULL, fileCheck = TRUE) {

	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}

	if(!isAsspWindowType(Window)){
		stop("WindowFunction of type '", Window,"' is not supported!")
	}

	###########################
	# file check
	
	if(fileCheck){
		if(is.null(ExplicitExt)){
			#stop('asfdsfasdfasdf')
			hasDuplicateFiles(listOfFiles,'.acf')
		}
		print("DOING FILE CHECK!")
	}

	###########################
	# perform analysis
	
	if(length(listOfFiles)==1){
		pb <- NULL
	}else{
		pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}	

	invisible(.External("performAssp", listOfFiles, fname = "acfana", BeginTime = BeginTime, 
		CenterTime = CenterTime, EndTime = EndTime, WindowShift = WindowShift, WindowSize = WindowSize, EffectiveLength = EffectiveLength, Window = Window, AnalysisOrder = as.integer(AnalysisOrder), EnergyNormalization = EnergyNormalization, LengthNormalization = LengthNormalization, ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb))

	if(!(length(listOfFiles)==1)){ close(pb) }

}
