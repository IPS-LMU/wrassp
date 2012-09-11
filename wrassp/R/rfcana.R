##' this is the description
##'
##' and these are details
##' @title rfcana
##' @param listOfFiles vector of file paths to be processed by function 
##' @param optLogFilePath path to option log file
##' @param BeginTime bla
##' @param CenterTime bli
##' @param EndTime blup
##' @param WindowShift bla
##' @param WindowSize bli
##' @param EffectiveLength blup
##' @param Window bla
##' @param Order bli
##' @param Preemphasis blup
##' @param LpType bla
##' @param ToFile bli
##' @param ExplicitExt blup
##' @return nrOfProcessedFiles or if only one file to process return dataObj of that file
##' @author Raphael Winkelmann
'rfcana' <- function(listOfFiles = NULL, optLogFilePath = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, WindowShift = 5.0, WindowSize = 20.0, EffectiveLength = TRUE, Window = 'BLACKMAN', Order = 0, Preemphasis = -0.95, LpType = 'RFC', ToFile = TRUE, ExplicitExt = NULL) {
	
	
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
	
	if(!isAsspLpType(LpType)){
		stop("LpType of type '", LpType,"' is not supported!")
	}
	
	###########################
	# perform analysis

	if(length(listOfFiles)==1){
		pb <- NULL
	}else{
		pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}	
	
	
	externalRes = invisible(.External("performAssp", listOfFiles, fname = "rfcana", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, WindowShift = WindowShift, WindowSize = WindowSize, EffectiveLength = EffectiveLength, Window = Window, Order = as.integer(Order), Preemphasis = Preemphasis, LpType = LpType, ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb, PACKAGE = "wrassp"))

        ############################
        # write options to options log file

        cat("\n##################################\n", file = optLogFilePath, append = T)
        cat("##################################\n", file = optLogFilePath, append = T)
        cat("######## rfcana performed ########\n", file = optLogFilePath, append = T)

        cat("Timestamp: ", paste(Sys.time()), '\n', file = optLogFilePath, append = T)

        cat("BeginTime: ", BeginTime, '\n', file = optLogFilePath, append = T)
        cat("CenterTime: ", CenterTime, '\n', file = optLogFilePath, append = T)
        cat("EndTime: ", EndTime, '\n', file = optLogFilePath, append = T)
        cat("WindowShift: ", WindowShift, '\n', file = optLogFilePath, append = T)
        cat("WindowSize: ", WindowSize, '\n', file = optLogFilePath, append = T)
        cat("EffectiveLength: ", EffectiveLength, '\n', file = optLogFilePath, append = T)
        cat("Window: ", Window, '\n', file = optLogFilePath, append = T)
        cat("Order: ", Order, '\n', file = optLogFilePath, append = T)
        cat("Preemphasis: ", Preemphasis, '\n', file = optLogFilePath, append = T)
        cat("LpType: ", LpType, '\n', file = optLogFilePath, append = T)
        

        cat("ToFile: ", ToFile, "\n", file = optLogFilePath, append = T)
        cat("ExplicitExt: ", ExplicitExt, "\n", file = optLogFilePath, append = T)

        cat(" => on files:\n\t", file = optLogFilePath, append = T)
        cat(paste(listOfFiles, collapse="\n\t"), file = optLogFilePath, append = T)
                

        
        #############################
        # return dataObj if length only one file
        
	if(!(length(listOfFiles)==1)){
          close(pb)
        }else{
          resDataObj = getDObj(externalRes)
          return(resDataObj)
        }
}
