##' .. content for \description{} (no empty lines) ..
##'
##' .. content for \details{} ..
##' @title rmsana
##' @param listOfFiles nrOfProcessedFiles or if only one file to process return dataObj of that file 
##' @param BeginTime bla
##' @param CenterTime bli
##' @param EndTime blup
##' @param WindowShift bla
##' @param WindowSize bli
##' @param EffectiveLength blup
##' @param Linear bla
##' @param Window bli
##' @param ToFile blup
##' @param ExplicitExt 
##' @return  nrOfProcessedFiles or if only one file to process return dataObj of that file 
##' @author Raphael Winkelmann
'rmsana' <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, WindowShift = 5.0, WindowSize = 20.0, EffectiveLength = TRUE, Linear = FALSE, Window = 'HAMMING', ToFile = TRUE, ExplicitExt = NULL) {


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
	
	invisible(.External("performAssp", listOfFiles, fname = "rmsana", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, WindowShift = WindowShift, WindowSize = WindowSize, EffectiveLength = EffectiveLength, Linear = Linear, Window = Window, ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb))

        #############################
        # return dataObj if length only one file
                
	if(!(length(listOfFiles)==1)){
          close(pb)
        }else{
          resDataObj = getDObj(listOfFiles[1])
          return(resDataObj)
        }
	
}
