##' .. content for \description{} (no empty lines) ..
##'
##' .. content for \details{} ..
##' @title rfcana
##' @param listOfFiles vector of file paths to be processed by function 
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
'rfcana' <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, WindowShift = 5.0, WindowSize = 20.0, EffectiveLength = TRUE, Window = 'BLACKMAN', Order = 0, Preemphasis = -0.95, LpType = 'RFC', ToFile = TRUE, ExplicitExt = NULL) {
	
	
	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
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
	
	
	invisible(.External("performAssp", listOfFiles, fname = "rfcana", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, WindowShift = WindowShift, WindowSize = WindowSize, EffectiveLength = EffectiveLength, Window = Window, Order = as.integer(Order), Preemphasis = Preemphasis, LpType = LpType, ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb))

        #############################
        # return dataObj if length only one file
        
	if(!(length(listOfFiles)==1)){
          close(pb)
        }else{
          resDataObj = getDObj(listOfFiles[1])
          return(resDataObj)
        }	
}
