##' f0ana function adapted from libassp
##'
##' F0 analysis of the signal in <listOfFile> using the Schaefer-
##' Vincent periodicity detector (Phonetica 40, 1983).
##' Analysis results will be written to a file with the
##' base name of the input file and extension '.f0'.
##' Default output is in single precision floating point
##' binary in SSFF format (keyword 'F0').
##' Optionally, location and type of the signal extrema on
##' which the F0 data are based, may be stored in a label
##' file. The name of this file will consist of the base
##' name of the F0 file and the extension '.prd'.
##' @title f0ana
##' @param listOfFiles vector of file paths to be processed by function 
##' @param optLogFilePath path to option log file
##' @param BeginTime = <time>: set begin of analysis interval to <time> seconds (default = 0: begin of data)
##' @param EndTime set end of analysis interval to <time> seconds (default = 0: end of data)
##' @param WindowShift = <dur>: set frame shift to <dur> ms (default: 5.0)
##' @param Gender ???
##' @param MaxF = <freq>: set maximum F0 value to <freq> Hz (default: 500.0)
##' @param MinF = <freq>: set minimum F0 value to <freq> Hz (default: 50.0)
##' @param MinAmp = <amp>: set amplitude threshold for voiced samples to <amp> (default: 100)
##' @param MaxZCR ???
##' @param ToFile write results to file (default extension is .f0)
##' @param ExplicitExt set if you wish to overwride the default extension
##' @return  nrOfProcessedFiles or if only one file to process return AsspDataObj of that file
##' @author Raphael Winkelmann
'f0ana' <- function(listOfFiles = NULL, optLogFilePath = NULL, BeginTime = 0.0, EndTime = 0.0, WindowShift = 5.0, Gender = 'u', MaxF = 600, MinF = 50, MinAmp = 50, MaxZCR = 3000.0, ToFile = TRUE, ExplicitExt = NULL){
	
	###########################
	# a few parameter checks and expand paths
	
	if (is.null(listOfFiles)) {
		stop("listOfFiles is NULL! It has to be a string or vector of file paths (min length = 1) pointing to valid file(s) to perform the given analysis function.")
	}

        if (is.null(optLogFilePath)){
          stop("optLogFilePath is NULL!")
        }

        listOfFiles = path.expand(listOfFiles)
        optLogFilePath = path.expand(optLogFilePath)
	
	###########################
	# perform analysis
	
	if(length(listOfFiles)==1){
          pb <- NULL
	}else{
          cat('\n  INFO: applying f0ana to', length(listOfFiles), 'files\n')
          pb <- txtProgressBar(min = 0, max = length(listOfFiles), style = 3)
	}	
	
	externalRes = invisible(.External("performAssp", listOfFiles, fname = "f0ana", BeginTime = BeginTime, EndTime = EndTime, WindowShift = WindowShift, Gender = Gender, MaxF = MaxF, MinF = MinF, MinAmp = MinAmp, MaxZCR = MaxZCR, ExplicitExt = ExplicitExt, ToFile = ToFile, ProgressBar = pb, PACKAGE = "wrassp"))

        ############################
        # write options to options log file

        cat("\n##################################\n", file = optLogFilePath, append = T)
        cat("##################################\n", file = optLogFilePath, append = T)
        cat("######## f0ana performed #########\n", file = optLogFilePath, append = T)

        cat("Timestamp: ", paste(Sys.time()), '\n', file = optLogFilePath, append = T)

        cat("BeginTime: ", BeginTime, '\n',file = optLogFilePath, append = T)
        cat("EndTime: ", EndTime, '\n',file = optLogFilePath, append = T)
        cat("WindowShift: ", WindowShift, '\n',file = optLogFilePath, append = T)
        cat("Gender: ", Gender, '\n',file = optLogFilePath, append = T)
        cat("MaxF: ", MaxF, '\n',file = optLogFilePath, append = T)
        cat("MinF: ", MinF, '\n',file = optLogFilePath, append = T)
        cat("MinAmp: ", MinAmp, '\n',file = optLogFilePath, append = T)
        cat("MaxZCR: ", MaxZCR, '\n',file = optLogFilePath, append = T)


        
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
