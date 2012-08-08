
newEmu.track <- function(seglist=NULL, trackname=NULL, fext=NULL){
  
  if( is.null(seglist) || is.null(trackname)) {
    cat("newEmu.track: usage: newEmu.track(seglist=seglistFromQuery, trackname=trackNameToBeReturned, ext=extenstionOfFile\n")
    cat("               replacement of the old emu.track function using the wrassp package\n")
    stop("newEmu.track: Argument names=NamesOfTextGridFileToRead and trackname=trackNameToBeReturned are required.\n")
  }
  
  #generate new file extension
  if (trackname=="acf"){
      print("acf")
      newFileEx <- ".acf"
    }else if (trackname=="hpf"){
      print("hpf")
      newFileEx <- ".hpf"      
    }else if (trackname=="lpf"){
      print("lpf")
      newFileEx <- ".lpf"
    }else if (trackname=="bpf"){
      print("bpf")
      newFileEx <- ".bpf"
    }else if (trackname=="bsf"){
      newFileEx <- ".bsf"
    }else if (trackname=="f0"){
      print("f0")
      newFileEx <- ".f0"
    }else if (trackname=="fms:rms"){
      print("fms:rms")
      newFileEx <- ".fms"
      colName <- "rms"
    }else if (trackname=="fms:gain"){
      print("fms:gain")
      newFileEx <- ".fms"
      colName <- "gain"      
    }else if (trackname=="fms:fm"){
      print("fms:fm")
      newFileEx <- ".fms"
      colName <- "fm"
    }else if (trackname=="fms:bw"){
      print("fms:bw")
      newFileEx <- ".fms"
      colName <- "bw"    
    }else if (trackname=="pit"){
      print("pit")
      newFileEx <- ".pit"
    }else if (trackname=="arf:rms"){
      print("arf:rms")
      newFileEx <- ".arf"
      colName <- "rms"
    }else if (trackname=="arf:gain"){
      print("arf:gain")
      newFileEx <- ".arf"
      colName <- "gain"
    }else if (trackname=="arf:arf"){
      print("arf:arf")
      newFileEx <- ".arf"
      colName <- "arf"
    }else if (trackname=="lar:rms"){
      print("lar:rms")
      newFileEx <- ".lar"
      colName <- "rms"
    }else if (trackname=="lar:gain"){
      print("lar:gain")
      newFileEx <- ".lar"
      colName <- "gain"
    }else if (trackname=="lar:lar"){
      print("lar:lar")
      newFileEx <- ".lar"
      colName <- "lar"
    }else if (trackname=="lpc:rms"){
      print("lpc:rms")
      newFileEx <- ".lpc"
      colName <- "rms"
    }else if (trackname=="lpc:gain"){
      print("lpc:gain")
      newFileEx <- ".lpc"
      colName <- "gain"
    }else if (trackname=="lpc:lpc"){
      print("lpc:arf")
      newFileEx <- ".lpc"
      colName <- "lpc"
    }else if (trackname=="rfc:rms"){
      print("rfc:rms")
      newFileEx <- ".rfc"
      colName <- "rms"
    }else if (trackname=="rfc:gain"){
      print("rfc:gain")
      newFileEx <- ".rfc"
      colName <- "gain"
    }else if (trackname=="rfc:rfc"){
      print("rfc:rfc")
      newFileEx <- ".rfc"
      colName <- "rfc"
    }else if (trackname=="rms"){  
      print("rms")
      newFileEx <- ".fms"
    }else if (trackname=="dft"){  
      print("dft")
      newFileEx <- ".dft"
    }else if (trackname=="lps"){  
      print("lps")
      newFileEx <- ".lps"
    }else if (trackname=="css"){  
      print("css")
      newFileEx <- ".css"
    }else if (trackname=="cep"){  
      print("cep")
      newFileEx <- ".cep"      
    }else if (trackname=="zcr"){  
      print("zcr")
      newFileEx <- ".zcr"
   }else{
     stop("unknown trackname...")
   }
  
  #create empty index, ftime, data matrices
  index <- matrix(ncol=2, nrow=length(seglist$utts))
  colnames(index) <- c("start","end")
  
  ftime <- matrix(ncol=2, nrow=length(seglist$utts))
  colnames(ftime) <- c("start","end")
  
  data <- matrix(ncol=4, nrow=0)
  
  #####LOOP OVER UTTS######
  curIndexStart = 1
  for (i in 1:length(seglist$utts)){
    #split at "." char (fixed=T to turn off regex matching)
    dotSplitFilePath = unlist(strsplit(seglist$utts[i], ".",fixed=T ))  
    dotSplitFilePath[length(dotSplitFilePath)] <- newFileEx
  
    fname = paste(dotSplitFilePath, collapse="")
    
    #get data object
    curDObj <- getDObj(fname)
  
    curStart <- seglist$start[i]
    curEnd <- seglist$end[i]

    fSampleRateInMS <- (1/attr(curDObj, "samplerate"))*1000
    fStartTime <- 0.00246875*1000 #SIC! get from object
  
    timeStampSeq = seq(fStartTime, curEnd, fSampleRateInMS)
  
    #search for first element larger than start time
    breakVal = -1
    for (j in 1:length(timeStampSeq)){
      if (timeStampSeq[j] >= curStart){
        breakVal = j
        break
      }
    }
    print(breakVal)
    curStartDataIdx = breakVal
    curEndDataIdx = length(timeStampSeq)
  
    #set index and ftime
    curIndexEnd = curIndexStart+curEndDataIdx-curStartDataIdx
    index[i,] <- c(curIndexStart, curIndexEnd)
    ftime[i,] <- c(timeStampSeq[curStartDataIdx], timeStampSeq[curEndDataIdx])
  
    #calculate size of and create new data matrix
    rowSeq <- seq(timeStampSeq[curStartDataIdx],timeStampSeq[curEndDataIdx], fSampleRateInMS) 
    curData <- matrix(ncol=4, nrow=length(rowSeq))
    colnames(curData) <- paste("T", 1:ncol(curData), sep="")
    rownames(curData) <- rowSeq
    curData[,] <- curDObj$fm[curStartDataIdx:curEndDataIdx,] 

  
    #Append to global data matrix app
    nrow(data)
    data <- rbind(data, curData)
  
    curIndexStart = curIndexEnd+1
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
