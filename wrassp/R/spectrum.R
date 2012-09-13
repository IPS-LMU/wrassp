##' descritopns
##'
##' details
##' @title dftspectrum
##' @param listOfFiles vector of file paths to be processed by function 
##' @param optLogFilePath path to option log file
##' @param BeginTime bla
##' @param CenterTime bli
##' @param EndTime blup
##' @param Resolution bla
##' @param FftLength bli
##' @param WindowSize blup
##' @param WindowShift bla
##' @param Window bli
##' @param Bandwidth blup
##' @param EffectiveLength bla
##' @param Order bli
##' @param Preemphasis blup
##' @param Deemphasize bla
##' @param NumCeps bli
##' @param ToFile write results to file (default extension is .acf)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @return nrOfProcessedFiles or if only one file to process return dataObj of that file
##' @author Raphael Winkelmann
'dftSpectrum' <- function(listOfFiles = NULL, optLogFilePath = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, Resolution = 40.0, FftLength = 0, WindowSize = 20.0, WindowShift = 5.0, Window = 'BLACKMAN', Bandwidth = 0.0, EffectiveLength = FALSE, Order = 0, Preemphasis = 0.0, Deemphasize = FALSE, NumCeps = 0, ToFile = TRUE, ExplicitExt = NULL) {
	
	SpectrumType = 'DFT'
	
	###########################
	# a few parameter checks and expand paths
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}

        if (is.null(optLogFilePath)){
          stop("optLogFilePath is NULL!")
        }
        
	if(!isAsspWindowType(Window)){
		stop("WindowFunction of type '", Window,"' is not supported!")
	}

	listOfFiles = path.expand(listOfFiles)
        optLogFilePath = path.expand(optLogFilePath)
	
	###########################
	# perform analysis

	if(length(listOfFiles)==1){
		pb <- NULL
	}else{
		pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}	
	
	externalRes = invisible(.External("performAssp", listOfFiles, fname = "spectrum", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, SpectrumType = SpectrumType, Resolution = Resolution, FftLength = as.integer(FftLength), WindowSize = WindowSize, WindowShift = WindowShift,  Window = Window, Bandwidth = Bandwidth, EffectiveLength = EffectiveLength, Order = as.integer(Order), Preemphasis = Preemphasis, Deemphasize = Deemphasize, NumCeps = as.integer(NumCeps), ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb, PACKAGE = "wrassp"))


        ############################
        # write options to options log file

        cat("\n##################################\n", file = optLogFilePath, append = T)
        cat("##################################\n", file = optLogFilePath, append = T)
        cat("##### dftSpectrum performed ######\n", file = optLogFilePath, append = T)

        cat("Timestamp: ", paste(Sys.time()), '\n', file = optLogFilePath, append = T)

        cat("BeginTime: ", BeginTime, '\n', file = optLogFilePath, append = T)
        cat("CenterTime: ", CenterTime, '\n', file = optLogFilePath, append = T)
        cat("EndTime: ", EndTime, '\n', file = optLogFilePath, append = T)
        cat("SpectrumType: ", SpectrumType, '\n', file = optLogFilePath, append = T)
        cat("Resolution: ", Resolution, '\n', file = optLogFilePath, append = T)
        cat("FftLength: ", FftLength, '\n', file = optLogFilePath, append = T)
        cat("WindowSize: ", WindowSize, '\n', file = optLogFilePath, append = T)
        cat("WindowShift: ", WindowShift, '\n', file = optLogFilePath, append = T)
        cat("Window: ", Window, '\n', file = optLogFilePath, append = T)
        cat("Bandwidth: ", Bandwidth, '\n', file = optLogFilePath, append = T)
        cat("EffectiveLength: ", EffectiveLength, '\n', file = optLogFilePath, append = T)
        cat("Order: ", Order, '\n', file = optLogFilePath, append = T)
        cat("Preemphasis: ", Preemphasis, '\n', file = optLogFilePath, append = T)
        cat("Deemphasize: ", Deemphasize, '\n', file = optLogFilePath, append = T)
        cat("NumCeps: ", NumCeps, '\n', file = optLogFilePath, append = T)
        

        cat("ToFile: ", ToFile, "\n", file = optLogFilePath, append = T)
        cat("ExplicitExt: ", ExplicitExt, "\n", file = optLogFilePath, append = T)

        cat(" => on files:\n\t", file = optLogFilePath, append = T)
        cat(paste(listOfFiles, collapse="\n\t"), file = optLogFilePath, append = T)
        
        #############################
        # return dataObj if length only one file
        
	if(!(length(listOfFiles)==1)){
          close(pb)
        }else{
          return(externalRes)
        }
        
}


##' description
##'
##' details
##' @title lpsSpectrum
##' @param listOfFiles vector of file paths to be processed by function
##' @param optLogFilePath path to option log file
##' @param BeginTime bli
##' @param CenterTime blup
##' @param EndTime bla
##' @param Resolution bli
##' @param FftLength blup
##' @param WindowSize bla
##' @param WindowShift bli
##' @param Window blup
##' @param Bandwidth bla
##' @param EffectiveLength bli
##' @param Order blup
##' @param Preemphasis bla 
##' @param Deemphasize bli
##' @param NumCeps blup
##' @param ToFile write results to file (default extension is .lps)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @return nrOfProcessedFiles or if only one file to process return dataObj of that file
##' @author Raphael Winkelmann
'lpsSpectrum' <- function(listOfFiles = NULL, optLogFilePath = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, Resolution = 40.0, FftLength = 0, WindowSize = 20.0, WindowShift = 5.0, Window = 'BLACKMAN', Bandwidth = 0.0, EffectiveLength = FALSE, Order = 0, Preemphasis = -0.95, Deemphasize = FALSE, NumCeps = 0, ToFile = TRUE, ExplicitExt = NULL) {
	
	stop("DEFAULT VALUES WRONG! NOT IMPLEMENTED YET!")
	
	SpectrumType = 'LPS'
	
	###########################
	# a few parameter checks and expand paths
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}

        if (is.null(optLogFilePath)){
          stop("optLogFilePath is NULL!")
        }
        
	if(!isAsspWindowType(Window)){
		stop("WindowFunction of type '", Window,"' is not supported!")
	}

        listOfFiles = path.expand(listOfFiles)
        optLogFilePath = path.expand(optLogFilePath)
	
	###########################
	# perform analysis

	if(length(listOfFiles)==1){
		pb <- NULL
	}else{
		pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}
	
	externalRes = invisible(.External("performAssp", listOfFiles, fname = "spectrum", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, SpectrumType = SpectrumType, Resolution = Resolution, FftLength = as.integer(FftLength), WindowSize = WindowSize, WindowShift = WindowShift,  Window = Window, Bandwidth = Bandwidth, EffectiveLength = EffectiveLength, Order = as.integer(Order), Preemphasis = Preemphasis, Deemphasize = Deemphasize, NumCeps = as.integer(NumCeps), ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb, PACKAGE = "wrassp"))


        ############################
        # write options to options log file

        cat("\n##################################\n", file = optLogFilePath, append = T)
        cat("##################################\n", file = optLogFilePath, append = T)
        cat("##### lpsSpectrum performed ######\n", file = optLogFilePath, append = T)

        cat("Timestamp: ", paste(Sys.time()), '\n', file = optLogFilePath, append = T)

        cat("BeginTime: ", BeginTime, '\n', file = optLogFilePath, append = T)
        cat("CenterTime: ", CenterTime, '\n', file = optLogFilePath, append = T)
        cat("EndTime: ", EndTime, '\n', file = optLogFilePath, append = T)
        cat("SpectrumType: ", SpectrumType, '\n', file = optLogFilePath, append = T)
        cat("Resolution: ", Resolution, '\n', file = optLogFilePath, append = T)
        cat("FftLength: ", FftLength, '\n', file = optLogFilePath, append = T)
        cat("WindowSize: ", WindowSize, '\n', file = optLogFilePath, append = T)
        cat("WindowShift: ", WindowShift, '\n', file = optLogFilePath, append = T)
        cat("Window: ", Window, '\n', file = optLogFilePath, append = T)
        cat("Bandwidth: ", Bandwidth, '\n', file = optLogFilePath, append = T)
        cat("EffectiveLength: ", EffectiveLength, '\n', file = optLogFilePath, append = T)
        cat("Order: ", Order, '\n', file = optLogFilePath, append = T)
        cat("Preemphasis: ", Preemphasis, '\n', file = optLogFilePath, append = T)
        cat("Deemphasize: ", Deemphasize, '\n', file = optLogFilePath, append = T)
        cat("NumCeps: ", NumCeps,  '\n', file = optLogFilePath, append = T)


        
        cat("ToFile: ", ToFile, "\n", file = optLogFilePath, append = T)
        cat("ExplicitExt: ", ExplicitExt, "\n", file = optLogFilePath, append = T)

        cat(" => on files:\n\t", file = optLogFilePath, append = T)
        cat(paste(listOfFiles, collapse="\n\t"), file = optLogFilePath, append = T)
        
        
        
        #############################
        # return dataObj if length only one file
        
	if(!(length(listOfFiles)==1)){
          close(pb)
        }else{
          return(externalRes)
        }
}


##' description
##'
##' details
##' @title cssSpectrum 
##' @param listOfFiles vector of file paths to be processed by function 
##' @param optLogFilePath path to option log file
##' @param BeginTime bla
##' @param CenterTime bli
##' @param EndTime blup
##' @param Resolution bla
##' @param FftLength bli
##' @param WindowSize blup
##' @param WindowShift bla
##' @param Window bli
##' @param Bandwidth blup
##' @param EffectiveLength bla
##' @param Order bli
##' @param Preemphasis blup
##' @param Deemphasize bla
##' @param NumCeps bli
##' @param ToFile write results to file (default extension is .css)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @return nrOfProcessedFiles or if only one file to process return dataObj of that file
##' @author Raphael Winkelmann
'cssSpectrum' <- function(listOfFiles = NULL, optLogFilePath = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, Resolution = 40.0, FftLength = 0, WindowSize = 0.0, WindowShift = 5.0, Window = 'BLACKMAN', Bandwidth = 0.0, EffectiveLength = FALSE, Order = 0, Preemphasis = 0.0, Deemphasize = FALSE, NumCeps = 0, ToFile = TRUE, ExplicitExt = NULL) {
	
	stop("DEFAULT VALUES WRONG! NOT IMPLEMENTED YET!")
	
	SpectrumType = 'CSS'
	
	###########################
	# a few parameter checks and expand paths
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}

        if (is.null(optLogFilePath)){
          stop("optLogFilePath is NULL!")
        }
	
	if(!isAsspWindowType(Window)){
		stop("WindowFunction of type '", Window,"' is not supported!")
	}

        listOfFiles = path.expand(listOfFiles)
        optLogFilePath = path.expand(optLogFilePath)
	
	###########################
	# perform analysis

	if(length(listOfFiles)==1){
		pb <- NULL
	}else{
		pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}	
	
	
	externalRes = invisible(.External("performAssp", listOfFiles, fname = "spectrum", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, SpectrumType = SpectrumType, Resolution = Resolution, FftLength = as.integer(FftLength), WindowSize = WindowSize, WindowShift = WindowShift,  Window = Window, Bandwidth = Bandwidth, EffectiveLength = EffectiveLength, Order = as.integer(Order), Preemphasis = Preemphasis, Deemphasize = Deemphasize, NumCeps = as.integer(NumCeps), ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb, PACKAGE = "wrassp"))

        ############################
        # write options to options log file

        cat("\n##################################\n", file = optLogFilePath, append = T)
        cat("##################################\n", file = optLogFilePath, append = T)
        cat("###### cssSpectrum performed #####\n", file = optLogFilePath, append = T)
        cat("Timestamp: ", paste(Sys.time()), '\n', file = optLogFilePath, append = T)
        cat("BeginTime: ", BeginTime, '\n', file = optLogFilePath, append = T)
        cat("CenterTime: ", CenterTime, '\n', file = optLogFilePath, append = T)
        cat("EndTime: ", EndTime, '\n', file = optLogFilePath, append = T)
        cat("SpectrumType: ", SpectrumType, '\n', file = optLogFilePath, append = T)
        cat("Resolution: ", Resolution, '\n', file = optLogFilePath, append = T)
        cat("FftLength: ", FftLength, '\n', file = optLogFilePath, append = T)
        cat("WindowSize: ", WindowSize, '\n', file = optLogFilePath, append = T)
        cat("WindowShift: ", WindowShift, '\n', file = optLogFilePath, append = T)
        cat("Window: ", Window, '\n', file = optLogFilePath, append = T)
        cat("Bandwidth: ", Bandwidth, '\n', file = optLogFilePath, append = T)
        cat("EffectiveLength: ", EffectiveLength, '\n', file = optLogFilePath, append = T)
        cat("Order: ", Order, '\n', file = optLogFilePath, append = T)
        cat("Preemphasis: ", Preemphasis, '\n', file = optLogFilePath, append = T)
        cat("Deemphasize: ", Deemphasize, '\n', file = optLogFilePath, append = T)
        cat("NumCeps: ", NumCeps, '\n', file = optLogFilePath, append = T)


        
        cat("ToFile: ", ToFile, "\n", file = optLogFilePath, append = T)
        cat("ExplicitExt: ", ExplicitExt, "\n", file = optLogFilePath, append = T)

        cat(" => on files:\n\t", file = optLogFilePath, append = T)
        cat(paste(listOfFiles, collapse="\n\t"), file = optLogFilePath, append = T)
        
        
        #############################
        # return dataObj if length only one file
        
	if(!(length(listOfFiles)==1)){
          close(pb)
        }else{
          return(externalRes)
        }
		
}

##' description
##'
##' details
##' @title cep 
##' @param listOfFiles vector of file paths to be processed by function 
##' @param optLogFilePath path to option log file
##' @param BeginTime start time (in ms) in file to perform function on 
##' @param CenterTime ???
##' @param EndTime end time (in ms) in file to perform function on
##' @param Resolution ???
##' @param FftLength ???
##' @param WindowSize window size of function (in ms)
##' @param WindowShift window shift of function (in ms) 
##' @param Window type of window (see AsspWindowTypes() function)
##' @param Bandwidth ???
##' @param EffectiveLength ??? 
##' @param Order ???
##' @param Preemphasis ??? 
##' @param Deemphasize ??? 
##' @param NumCeps ???
##' @param ToFile write results to file (default extension is .cep)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @return nrOfProcessedFiles or if only one file to process return dataObj of that file
##' @author Raphael Winkelmann
'cepSpectrum' <- function(listOfFiles = NULL, optLogFilePath = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, Resolution = 40.0, FftLength = 0, WindowSize = 0.0, WindowShift = 5.0, Window = 'BLACKMAN', Bandwidth = 0.0, EffectiveLength = FALSE, Order = 0, Preemphasis = 0.0, Deemphasize = FALSE, NumCeps = 0, ToFile = TRUE, ExplicitExt = NULL) {
	
	stop("DEFAULT VALUES WRONG! NOT IMPLEMENTED YET!")
	
	SpectrumType = 'CEP'
	
	###########################
	# a few parameter checks and expand paths
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}
        if (is.null(optLogFilePath)){
          stop("optLogFilePath is NULL!")
        }
	
	if(!isAsspWindowType(Window)){
		stop("WindowFunction of type '", Window,"' is not supported!")
	}

        listOfFiles = path.expand(listOfFiles)
        optLogFilePath = path.expand(optLogFilePath)
	
	###########################
	# perform analysis

	if(length(listOfFiles)==1){
		pb <- NULL
	}else{
		pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}	

	
	
	externalRes = invisible(.External("performAssp", listOfFiles, fname = "spectrum", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, SpectrumType = SpectrumType, Resolution = Resolution, FftLength = FftLength, WindowSize = WindowSize, WindowShift = WindowShift,  Window = Window, Bandwidth = Bandwidth, EffectiveLength = EffectiveLength, Order = Order, Preemphasis = Preemphasis, Deemphasize = Deemphasize, NumCeps = NumCeps, ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb, PACKAGE = "wrassp"))


        ############################
        # write options to options log file

        cat("\n##################################\n", file = optLogFilePath, append = T)
        cat("##################################\n", file = optLogFilePath, append = T)
        cat("##### cepSpectrum performed ######\n", file = optLogFilePath, append = T)

        cat("Timestamp: ", paste(Sys.time()), '\n', file = optLogFilePath, append = T)
        cat("BeginTime: ", BeginTime, '\n', file = optLogFilePath, append = T)
        cat("CenterTime: ", CenterTime, '\n', file = optLogFilePath, append = T)
        cat("EndTime: ", EndTime, '\n', file = optLogFilePath, append = T)
        cat("SpectrumType: ", SpectrumType, '\n', file = optLogFilePath, append = T)
        cat("Resolution: ", Resolution, '\n', file = optLogFilePath, append = T)
        cat("FftLength: ", FftLength, '\n', file = optLogFilePath, append = T)
        cat("WindowSize: ", WindowSize, '\n', file = optLogFilePath, append = T)
        cat("WindowShift: ", WindowShift, '\n', file = optLogFilePath, append = T)
        cat("Window: ", Window, '\n', file = optLogFilePath, append = T)
        cat("Bandwidth: ", Bandwidth, '\n', file = optLogFilePath, append = T)
        cat("EffectiveLength: ", EffectiveLength, '\n', file = optLogFilePath, append = T)
        cat("Order: ", Order, '\n', file = optLogFilePath, append = T)
        cat("Preemphasis: ", Preemphasis, '\n', file = optLogFilePath, append = T)
        cat("Deemphasize: ", Deemphasize, '\n', file = optLogFilePath, append = T)
        cat("NumCeps: ", NumCeps, '\n', file = optLogFilePath, append = T)


        cat("ToFile: ", ToFile, "\n", file = optLogFilePath, append = T)
        cat("ExplicitExt: ", ExplicitExt, "\n", file = optLogFilePath, append = T)

        cat(" => on files:\n\t", file = optLogFilePath, append = T)
        cat(paste(listOfFiles, collapse="\n\t"), file = optLogFilePath, append = T)
        

        

        #############################
        # return dataObj if length only one file
        
	if(!(length(listOfFiles)==1)){
          close(pb)
        }else{
          return(externalRes)
        }
}
