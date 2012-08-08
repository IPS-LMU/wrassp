
newEmu.track <- function(seglist=NULL, trackname=NULL){
  
  if( is.null(seglist) || is.null(trackname)) {
    cat("newEmu.track: usage: newEmu.track(seglist=seglistFromQuery, trackname=trackNameToBeReturned, ext=extenstionOfFile\n")
    cat("               replacement of the old emu.track function using the wrassp package\n")
    stop("newEmu.track: Argument names=NamesOfTextGridFileToRead and trackname=trackNameToBeReturned are required.\n")
  }
  
  #generate new file extension
  if (trackname=="acf"){
      newFileEx <- ".acf"
      colName <- "acf"
    }else if (trackname=="hpf"){
      newFileEx <- ".hpf"
      colName <- "hpf"
    }else if (trackname=="lpf"){
      newFileEx <- ".lpf"
      colName <- "hpf"
    }else if (trackname=="bpf"){
      newFileEx <- ".bpf"
      colName <- "bpf"
    }else if (trackname=="bsf"){
      newFileEx <- ".bsf"
      colName <- "bsf"
    }else if (trackname=="f0"){
      newFileEx <- ".f0"
      colName <- "f0"
    }else if (trackname=="fms:rms"){
      newFileEx <- ".fms"
      colName <- "rms"
    }else if (trackname=="fms:gain"){
      newFileEx <- ".fms"
      colName <- "gain"      
    }else if (trackname=="fms:fm"){
      newFileEx <- ".fms"
      colName <- "fm"
    }else if (trackname=="fms:bw"){
      newFileEx <- ".fms"
      colName <- "bw"    
    }else if (trackname=="pit"){
      newFileEx <- ".pit"
      colName <- "pit"
    }else if (trackname=="arf:rms"){
      newFileEx <- ".arf"
      colName <- "rms"
    }else if (trackname=="arf:gain"){
      newFileEx <- ".arf"
      colName <- "gain"
    }else if (trackname=="arf:arf"){
      newFileEx <- ".arf"
      colName <- "arf"
    }else if (trackname=="lar:rms"){
      newFileEx <- ".lar"
      colName <- "rms"
    }else if (trackname=="lar:gain"){
      newFileEx <- ".lar"
      colName <- "gain"
    }else if (trackname=="lar:lar"){
      newFileEx <- ".lar"
      colName <- "lar"
    }else if (trackname=="lpc:rms"){
      newFileEx <- ".lpc"
      colName <- "rms"
    }else if (trackname=="lpc:gain"){
      newFileEx <- ".lpc"
      colName <- "gain"
    }else if (trackname=="lpc:lpc"){
      newFileEx <- ".lpc"
      colName <- "lpc"
    }else if (trackname=="rfc:rms"){
      newFileEx <- ".rfc"
      colName <- "rms"
    }else if (trackname=="rfc:gain"){
      newFileEx <- ".rfc"
      colName <- "gain"
    }else if (trackname=="rfc:rfc"){
      newFileEx <- ".rfc"
      colName <- "rfc"
    }else if (trackname=="rms"){  
      newFileEx <- ".fms"
      colName <- "rms"
    }else if (trackname=="dft"){
      newFileEx <- ".dft"
      colName <- "dft"
    }else if (trackname=="lps"){  
      newFileEx <- ".lps"
      colName <- "lps"
    }else if (trackname=="css"){  
      newFileEx <- ".css"
      colName <- "css"
    }else if (trackname=="cep"){  
      newFileEx <- ".cep"
      colName <- "cep"
    }else if (trackname=="zcr"){  
      newFileEx <- ".zcr"
      colName <- "zcr"
   }else{
     stop("unknown trackname...")
   }
  
  #create empty index, ftime matrices
  index <- matrix(ncol=2, nrow=length(seglist$utts))
  colnames(index) <- c("start","end")
  
  ftime <- matrix(ncol=2, nrow=length(seglist$utts))
  colnames(ftime) <- c("start","end")
  
  data <- NULL
  
  #####LOOP OVER UTTS######
  curIndexStart = 1
  for (i in 1:length(seglist$utts)){
    #split at "." char (fixed=T to turn off regex matching)
    dotSplitFilePath <- unlist(strsplit(seglist$utts[i], ".",fixed=T ))  
    dotSplitFilePath[length(dotSplitFilePath)] <- newFileEx
  
    fname <- paste(dotSplitFilePath, collapse="")
    cat("current file:\n")
    cat("\t - ", fname, "\n")
    
    #get data object
    curDObj <- getDObj(fname)
    
    if(is.null(data)){
      tmpData <- eval(parse(text=paste("curDObj$",colName,sep="")))
      data <- matrix(ncol=ncol(tmpData), nrow=0)
      tmpData <- NULL
    }
         
    curStart <- seglist$start[i]
    curEnd <- seglist$end[i]

    fSampleRateInMS <- (1/attr(curDObj, "samplerate"))*1000
    fStartTime <- attr(curDObj,"start_time")*1000
  
    timeStampSeq <- seq(fStartTime, curEnd, fSampleRateInMS)
  
    #search for first element larger than start time
    breakVal <- -1
    for (j in 1:length(timeStampSeq)){
      if (timeStampSeq[j] >= curStart){
        breakVal <- j
        break
      }
    }
    curStartDataIdx <- breakVal
    curEndDataIdx <- length(timeStampSeq)
  
    #set index and ftime
    curIndexEnd <- curIndexStart+curEndDataIdx-curStartDataIdx
    index[i,] <- c(curIndexStart, curIndexEnd)
    ftime[i,] <- c(timeStampSeq[curStartDataIdx], timeStampSeq[curEndDataIdx])
  
    #calculate size of and create new data matrix
    tmpData <- eval(parse(text=paste("curDObj$",colName,sep="")))
    
    rowSeq <- seq(timeStampSeq[curStartDataIdx],timeStampSeq[curEndDataIdx], fSampleRateInMS) 
    curData <- matrix(ncol=ncol(tmpData), nrow=length(rowSeq))
    colnames(curData) <- paste("T", 1:ncol(curData), sep="")
    rownames(curData) <- rowSeq
    curData[,] <- tmpData[curStartDataIdx:curEndDataIdx,] 

  
    #Append to global data matrix app
    data <- rbind(data, curData)
  
    curIndexStart <- curIndexEnd+1
    #to be safe
    curDObj = NULL
  }  
  #convert data, index, ftime to trackdata
  myTrackData <- as.trackdata(data, index=index, ftime, trackname)
  
  
  return(myTrackData)
  
}#newEmu.track

####################################
#test func

#make seglist from query
#seglist <- emu.query("stops", "*", "Phonetic=u:")

#convert to valid file path (in local dir)
#for (i in 1:length(seglist$utts)){
#  curFile <- paste(seglist$utts[i],".wav", sep="")
#  curFile <-unlist(strsplit(curFile, ":",fixed=T ))[2]
#  seglist$utts[i] <- curFile
#}

#res = newEmu.track(seglist,'fms:fm')
