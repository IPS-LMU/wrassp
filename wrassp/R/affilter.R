"affilter" <- function(listOfFiles = NULL, HighPass = 4000, LowPass = 0, StopBand = 96, 
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
    

    if(!(length(listOfFiles)==1)){ close(pb) }



}
