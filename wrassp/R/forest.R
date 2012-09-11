##' this is the description
##'
##' and these are the details
##' @title forest
##' @param listOfFiles vector of file paths to be processed by function
##' @param optLogFilePath path to option log file 
##' @param BeginTime bla 
##' @param EndTime bli
##' @param WindowShift blup 
##' @param WindowSize bla
##' @param EffectiveLength bli 
##' @param NominalF1 blup
##' @param Gender bla
##' @param Estimate bli
##' @param Order blup
##' @param IncrOrder bla
##' @param NumFormants bli
##' @param Window blup
##' @param Preemphasis bla 
##' @param ToFile write results to file (default extension is .fms)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @return nrOfProcessedFiles or if only one file to process return dataObj of that file
##' @author Raphael Winkelmann
'forest' <- function(listOfFiles = NULL, optLogFilePath = NULL,BeginTime = 0.0, EndTime = 0.0, WindowShift = 5.0, WindowSize = 20.0, EffectiveLength = TRUE, NominalF1 = 500, Gender = 'm', Estimate = FALSE, Order = 0, IncrOrder = 0, NumFormants = 4, Window = 'BLACKMAN', Preemphasis = -0.8, ToFile = TRUE, ExplicitExt = NULL) {
	
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
	#perform analysis
	
	if(length(listOfFiles)==1){
		pb <- NULL
	}else{
		pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}	
	
	
	externalRes = invisible(.External("performAssp", listOfFiles, fname = "forest", BeginTime =  BeginTime, EndTime = EndTime, WindowShift = WindowShift, WindowSize = WindowSize, EffectiveLength = EffectiveLength, NominalF1 = NominalF1, Gender = Gender, Estimate = Estimate, Order = as.integer(Order), IncrOrder = as.integer(IncrOrder), NumFormants = as.integer(NumFormants), Window = Window, Preemphasis = Preemphasis, ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb, PACKAGE = "wrassp"))

        ############################
        # write options to options log file

        cat("\n##################################\n", file = optLogFilePath, append = T)
        cat("##################################\n", file = optLogFilePath, append = T)
        cat("######## forest performed ########\n", file = optLogFilePath, append = T)

        cat("Timestamp: ", paste(Sys.time()), '\n', file = optLogFilePath, append = T)
        cat("BeginTime: ", BeginTime, '\n', file = optLogFilePath, append = T)
        cat("EndTime: ", EndTime, '\n', file = optLogFilePath, append = T)
        cat("WindowShift: ", WindowShift, '\n', file = optLogFilePath, append = T)
        cat("WindowSize: ", WindowSize, '\n', file = optLogFilePath, append = T)
        cat("EffectiveLength: ", EffectiveLength, '\n', file = optLogFilePath, append = T)
        cat("NominalF1: ",  NominalF1, '\n', file = optLogFilePath, append = T)
        cat("Gender: ", Gender, '\n', file = optLogFilePath, append = T)
        cat("Estimate: ", Estimate, '\n', file = optLogFilePath, append = T)
        cat("Order: ", Order, '\n', file = optLogFilePath, append = T)
        cat("IncrOrder: ", IncrOrder, '\n', file = optLogFilePath, append = T)
        cat("NumFormants: ", NumFormants, '\n', file = optLogFilePath, append = T)
        cat("Window: ", Window, '\n', file = optLogFilePath, append = T)
        cat("Preemphasis: ", Preemphasis, '\n', file = optLogFilePath, append = T)



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


