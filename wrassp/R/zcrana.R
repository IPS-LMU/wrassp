##' this is the description
##'
##' and these are the details
##' @title zcrana
##' @param listOfFiles vector of file paths to be processed by function 
##' @param optLogFilePath path to option log file
##' @param BeginTime start time (in ms) in file to perform function on
##' @param CenterTime ???
##' @param EndTime end time (in ms) in file to perform function on
##' @param WindowShift window shift of function (in ms) 
##' @param WindowSize window size of function (in ms)
##' @param ToFile write results to file (default extension is .zcr)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @return nrOfProcessedFiles or if only one file to process return dataObj of that file
##' @author Raphael Winkelmann
'zcrana' <- function(listOfFiles = NULL, optLogFilePath = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, WindowShift = 5.0, WindowSize = 25.0, ToFile = TRUE, ExplicitExt = NULL) {

	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}
        
        if (is.null(optLogFilePath)){
          stop("optLogFilePath is NULL!")
        }

        
	###########################
	# perform analysis

	if(length(listOfFiles)==1){
		pb <- NULL
	}else{
		pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}	

	invisible(.External("performAssp", PACKAGE = "wrassp", listOfFiles, fname = "zcrana", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, WindowShift = WindowShift, WindowSize = WindowSize, ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb))

        
        #############################
        # return dataObj if length only one file
	
	if(!(length(listOfFiles)==1)){
          close(pb)
        }else{
          resDataObj = getDObj(listOfFiles[1])
          return(resDataObj)
        }


}
