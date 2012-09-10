##' affilter function adapted from assp
##'
##' still have to write propper man pages
##' @title affilter
##' @param listOfFiles vector of file paths to be processed by function
##' @param HighPass bli
##' @param LowPass blup
##' @param StopBand bla
##' @param Transition bli
##' @param UseIIR blup
##' @param NumIIRsections bla
##' @param ToFile bla
##' @param ExplicitExt bli
##' @return number of files processed
##' @author Raphael Winkelmann
'affilter' <- function(listOfFiles = NULL, HighPass = 4000, LowPass = 0, StopBand = 96, 
                       Transition = 250, UseIIR = FALSE, NumIIRsections = 4, ToFile = TRUE,
                       ExplicitExt = NULL) {

###########################
### a few parameter checks

    if (is.null(listOfFiles)) {
        stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
    }

###########################
### perform analysis
    
    if(length(listOfFiles)==1){
        pb <- NULL
    }else{
        pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
    }
    

                                        #setTxtProgressBar(pb, i,)
    invisible(.External("performAssp", listOfFiles, fname = "affilter", HighPass = HighPass, 
			LowPass = LowPass, StopBand = StopBand, Transition = Transition, 
			UseIIR = UseIIR, NumIIRsections = as.integer(NumIIRsections),
                        ToFile = ToFile, 
			ExplicitExt = ExplicitExt, ProgressBar = pb))

    #############################
    # return dataObj if length only one file

    if(!(length(listOfFiles)==1)){
      close(pb)
    }else{
      resDataObj = getDObj(listOfFiles[1])
      return(resDataObj)
    }



}
