##' this is the description
##'
##' and these details
##' @title mhspitch
##' @param listOfFiles vector of file paths to be processed by function
##' @param optLogFilePath path to option log file
##' @param BeginTime start time (in ms) in file to perform function on 
##' @param CenterTime ???
##' @param EndTime end time (in ms) in file to perform function on
##' @param WindowShift window shift of function (in ms) 
##' @param Gender ???
##' @param MaxF ???
##' @param MinF ???
##' @param MinAmp ???
##' @param MinAC1 ???
##' @param MinRMS ???
##' @param MaxZCR ???
##' @param MinProb ???
##' @param PlainSpectrum ???
##' @param ToFile write results to file (default extension is .pit)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @return nrOfProcessedFiles or if only one file to process return dataObj of that file
##' @author Raphael Winkelmann
'mhspitch' <-function(listOfFiles = NULL, optLogFilePath = NULL,BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, WindowShift = 5.0, Gender = 'u', MaxF = 600.0, MinF = 50.0, MinAmp = 50.0, MinAC1 = 0.25, MinRMS = 18.0, MaxZCR = 3000.0, MinProb = 0.52, PlainSpectrum = FALSE, ToFile = TRUE, ExplicitExt = NULL) {
	
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
	
	externalRes = invisible(.External("performAssp", listOfFiles, fname = "mhspitch", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, WindowShift = WindowShift, Gender = Gender, MaxF = MaxF, MinF = MinF, MinAmp = MinAmp, MinAC1 = MinAC1, MinRMS = MinRMS, MaxZCR = MaxZCR, MinProb = MinProb, PlainSpectrum = PlainSpectrum, ToFile = ToFile, ExplicitExt = ExplicitExt, ProgressBar = pb, PACKAGE = "wrassp"))

        #############################
        # return dataObj if length only one file
        
	if(!(length(listOfFiles)==1)){
          close(pb)
        }else{
          resDataObj = getDObj(externalRes)
          return(resDataObj)
        }

        
}
