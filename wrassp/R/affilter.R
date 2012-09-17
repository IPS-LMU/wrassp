##' affilter function adapted from assp
##'
##' still have to write propper man pages
##' @title affilter
##' @param listOfFiles vector of file paths to be processed by function
##' @param optLogFilePath path to option log file 
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
'affilter' <- function(listOfFiles = NULL, optLogFilePath = NULL, HighPass = 4000, LowPass = 0, StopBand = 96, 
                       Transition = 250, UseIIR = FALSE, NumIIRsections = 4, ToFile = TRUE,
                       ExplicitExt = NULL) {

     ###########################
     ### a few parameter checks and expand paths

     if (is.null(listOfFiles)) {
       stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
     }
     
     if (is.null(optLogFilePath)){
       stop("optLogFilePath is NULL!")
     }

     listOfFiles = path.expand(listOfFiles)
     optLogFilePath = path.expand(optLogFilePath)

     
    ###########################
    ### perform analysis
    
    if(length(listOfFiles)==1){
        pb <- NULL
    }else{
      cat('\n  INFO: applying affilter to', length(listOfFiles), 'files\n')

      pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
    }
    
    externalRes = invisible(.External("performAssp", listOfFiles, fname = "affilter", HighPass = HighPass, 
			LowPass = LowPass, StopBand = StopBand, Transition = Transition, 
			UseIIR = UseIIR, NumIIRsections = as.integer(NumIIRsections),
                        ToFile = ToFile, 
			ExplicitExt = ExplicitExt, ProgressBar = pb, PACKAGE = "wrassp"))

     ############################
     # write options to options log file

     cat("\n##################################\n", file = optLogFilePath, append = T)
     cat("##################################\n", file = optLogFilePath, append = T)
     cat("####### affilter performed #######\n", file = optLogFilePath, append = T)

     cat("Timestamp: ", paste(Sys.time()), '\n', file = optLogFilePath, append = T)
     cat("HighPass", HighPass, "\n", file = optLogFilePath, append = T)
     cat("LowPass", LowPass, "\n", file = optLogFilePath, append = T)
     cat("StopBand", StopBand, "\n", file = optLogFilePath, append = T)
     cat("Transition", Transition, "\n", file = optLogFilePath, append = T) 
     cat("UseIIR", UseIIR, "\n", file = optLogFilePath, append = T)
     cat("NumIIRsections: ", NumIIRsections, "\n", file = optLogFilePath, append = T)

     cat("ToFile: ", ToFile, "\n", file = optLogFilePath, append = T)
     cat("ExplicitExt: ", ExplicitExt, "\n", file = optLogFilePath, append = T)

     cat(" => on files:\n\t", file = optLogFilePath, append = T)
     cat(paste(listOfFiles, collapse="\n\t"), file = optLogFilePath, append = T)
        

     
     #############################
     # return dataObj if length only one file
        
     if(!(length(listOfFiles)==1)){
       close(pb)
     }else{
       return(externalRes)
     }

}

