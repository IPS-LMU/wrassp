##' .. content for \description{} (no empty lines) ..
##'
##' .. content for \details{} ..
##' @title f0ana
##' @param listOfFiles vector of file paths to be processed by function 
##' @param BeginTime bla  
##' @param EndTime bli
##' @param WindowShift blup 
##' @param Gender bla
##' @param MaxF bli
##' @param MinF blup
##' @param MinAmp bla
##' @param MaxZCR bli
##' @param ToFile write results to file (default extension is .f0)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @return  nrOfProcessedFiles or if only one file to process return dataObj of that file
##' @author Raphael Winkelmann
'f0ana' <- function(listOfFiles = NULL, BeginTime = 0.0, EndTime = 0.0, WindowShift = 5.0, Gender = 'u', MaxF = 600, MinF = 50, MinAmp = 50, MaxZCR = 3000.0, ToFile = TRUE, ExplicitExt = NULL){
	
	###########################
	# a few parameter checks
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}	
	
	###########################
	# perform analysis
	
	if(length(listOfFiles)==1){
		pb <- NULL
	}else{
		pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}	
	
	invisible(.External("performAssp", listOfFiles, fname = "f0ana", BeginTime = BeginTime, EndTime = EndTime, WindowShift = WindowShift, Gender = Gender, MaxF = MaxF, MinF = MinF, MinAmp = MinAmp, MaxZCR = MaxZCR, ExplicitExt = ExplicitExt, ToFile = ToFile, ProgressBar = pb))

        #############################
        # return dataObj if length only one file
        
	if(!(length(listOfFiles)==1)){
          close(pb)
        }else{
          resDataObj = getDObj(listOfFiles[1])
          return(resDataObj)
        }

	
}
