##' acfana function adapted from assp library
##'
##' still have to write propper documentation
##' @title acfana
##' @param listOfFiles vector of file paths to be processed by function
##' @param BeginTime start time (in ms) in file to perform function on 
##' @param CenterTime ?????
##' @param EndTime end time (in ms) in file to perform function on
##' @param WindowShift window shift of function (in ms) 
##' @param WindowSize window size of function (in ms)
##' @param EffectiveLength ?????
##' @param Window type of window see isAsspWindowType function  
##' @param AnalysisOrder ????
##' @param EnergyNormalization ????
##' @param LengthNormalization ????
##' @param ToFile write results to file (default extension is .acf)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @return nrOfProcessedFiles or if only one file to process return dataObj of that file
##' @author Raphael Winkelmann
'acfana' <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, 
	EndTime = 0.0, WindowShift = 5.0, WindowSize = 20.0, EffectiveLength = TRUE, 
	Window = "BLACKMAN", AnalysisOrder = 0, EnergyNormalization = FALSE, LengthNormalization = FALSE, ToFile = TRUE, ExplicitExt = NULL) {

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

	invisible(.External("performAssp", listOfFiles, fname = "acfana", BeginTime = BeginTime, 
		CenterTime = CenterTime, EndTime = EndTime, WindowShift = WindowShift, WindowSize = WindowSize, EffectiveLength = EffectiveLength, Window = Window, AnalysisOrder = as.integer(AnalysisOrder), EnergyNormalization = EnergyNormalization, LengthNormalization = LengthNormalization, ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb))

        #############################
        # return dataObj if length only one file
        
	if(!(length(listOfFiles)==1)){
          close(pb)
        }else{
          resDataObj = getDObj(listOfFiles[1])
          return(resDataObj)
        }

}
