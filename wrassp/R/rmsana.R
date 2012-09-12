##' this is the description
##'
##' and these are the details
##' @title rmsana
##' @param listOfFiles nrOfProcessedFiles or if only one file to process return dataObj of that file 
##' @param optLogFilePath path to option log file
##' @param BeginTime start time (in ms) in file to perform function on 
##' @param CenterTime ???
##' @param EndTime end time (in ms) in file to perform function on
##' @param WindowShift window shift of function (in ms) 
##' @param WindowSize window size of function (in ms)
##' @param EffectiveLength ???
##' @param Linear ???
##' @param Window ???
##' @param ToFile write results to file (default extension is .rms)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @return  nrOfProcessedFiles or if only one file to process return dataObj of that file 
##' @author Raphael Winkelmann
'rmsana' <- function(listOfFiles = NULL, optLogFilePath = NULL,BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, WindowShift = 5.0, WindowSize = 20.0, EffectiveLength = TRUE, Linear = FALSE, Window = 'HAMMING', ToFile = TRUE, ExplicitExt = NULL) {


	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}

        if (is.null(optLogFilePath)){
          stop("optLogFilePath is NULL!")
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
	
	externalRes = invisible(.External("performAssp", listOfFiles, fname = "rmsana", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, WindowShift = WindowShift, WindowSize = WindowSize, EffectiveLength = EffectiveLength, Linear = Linear, Window = Window, ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb, PACKAGE = "wrassp"))


        ############################
        # write options to options log file

        cat("\n##################################\n", file = optLogFilePath, append = T)
        cat("##################################\n", file = optLogFilePath, append = T)
        cat("######## rmsana performed ########\n", file = optLogFilePath, append = T)

        cat("Timestamp: ", paste(Sys.time()), '\n', file = optLogFilePath, append = T)
        cat("BeginTime: ", BeginTime, '\n', file = optLogFilePath, append = T)
        cat("CenterTime: ", CenterTime, '\n', file = optLogFilePath, append = T)
        cat("EndTime: ", EndTime, '\n', file = optLogFilePath, append = T)
        cat("WindowShift: ", WindowShift, '\n', file = optLogFilePath, append = T)
        cat("WindowSize: ", WindowSize, '\n', file = optLogFilePath, append = T)
        cat("EffectiveLength: ", EffectiveLength, '\n', file = optLogFilePath, append = T)
        cat("Linear: ", Linear, '\n', file = optLogFilePath, append = T)
        cat("Window: ", Window, '\n', file = optLogFilePath, append = T)


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

