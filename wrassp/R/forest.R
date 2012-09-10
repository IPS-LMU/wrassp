##' .. content for \description{} (no empty lines) ..
##'
##' .. content for \details{} ..
##' @title 
##' @param listOfFiles vector of file paths to be processed by function
##' @param BeginTime 
##' @param EndTime 
##' @param WindowShift 
##' @param WindowSize 
##' @param EffectiveLength 
##' @param NominalF1 
##' @param Gender 
##' @param Estimate 
##' @param Order 
##' @param IncrOrder 
##' @param NumFormants 
##' @param Window 
##' @param Preemphasis 
##' @param ToFile 
##' @param ExplicitExt 
##' @return 
##' @author Raphael Winkelmann
'forest' <- function(listOfFiles = NULL, BeginTime = 0.0, EndTime = 0.0, WindowShift = 5.0, WindowSize = 20.0, EffectiveLength = TRUE, NominalF1 = 500, Gender = 'm', Estimate = FALSE, Order = 0, IncrOrder = 0, NumFormants = 4, Window = 'BLACKMAN', Preemphasis = -0.8, ToFile = TRUE, ExplicitExt = NULL) {
	
	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
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
	
	
	invisible(.External("performAssp", listOfFiles, fname = "forest", BeginTime =  BeginTime, EndTime = EndTime, WindowShift = WindowShift, WindowSize = WindowSize, EffectiveLength = EffectiveLength, NominalF1 = NominalF1, Gender = Gender, Estimate = Estimate, Order = as.integer(Order), IncrOrder = as.integer(IncrOrder), NumFormants = as.integer(NumFormants), Window = Window, Preemphasis = Preemphasis, ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb))

        #############################
        # return dataObj if length only one file
        
	if(!(length(listOfFiles)==1)){
          close(pb)
        }else{
          resDataObj = getDObj(listOfFiles[1])
          return(resDataObj)
        }
}
