##' .. content for \description{} (no empty lines) ..
##'
##' .. content for \details{} ..
##' @title mhspitch
##' @param listOfFiles vector of file paths to be processed by function
##' @param BeginTime bla
##' @param CenterTime bli
##' @param EndTime blup
##' @param WindowShift bla
##' @param Gender bli
##' @param MaxF blup
##' @param MinF bla
##' @param MinAmp bli
##' @param MinAC1 blup
##' @param MinRMS bla
##' @param MaxZCR bli
##' @param MinProb blup
##' @param PlainSpectrum bla
##' @param ToFile bli
##' @param ExplicitExt blup
##' @return nrOfProcessedFiles or if only one file to process return dataObj of that file
##' @author Raphael Winkelmann
'mhspitch' <-function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, WindowShift = 5.0, Gender = 'u', MaxF = 600.0, MinF = 50.0, MinAmp = 50.0, MinAC1 = 0.25, MinRMS = 18.0, MaxZCR = 3000.0, MinProb = 0.52, PlainSpectrum = FALSE, ToFile = TRUE, ExplicitExt = NULL) {
	
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
	
	invisible(.External("performAssp", listOfFiles, fname = "mhspitch", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, WindowShift = WindowShift, Gender = Gender, MaxF = MaxF, MinF = MinF, MinAmp = MinAmp, MinAC1 = MinAC1, MinRMS = MinRMS, MaxZCR = MaxZCR, MinProb = MinProb, PlainSpectrum = PlainSpectrum, ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb))

        #############################
        # return dataObj if length only one file
        
	if(!(length(listOfFiles)==1)){
          close(pb)
        }else{
          resDataObj = getDObj(listOfFiles[1])
          return(resDataObj)
        }
	
}
