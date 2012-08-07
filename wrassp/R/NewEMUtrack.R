
newEmu.track <- function(seglist=NULL, trackname=NULL, fext=NULL){
  
  if( is.null(seglist) || is.null(trackname)) {
    cat("newEmu.track: usage: newEmu.track(seglist=seglistFromQuery, trackname=trackNameToBeReturned, ext=extenstionOfFile\n")
    cat("               replacement of the old emu.track function using the wrassp package\n")
    stop("newEmu.track: Argument names=NamesOfTextGridFileToRead and trackname=trackNameToBeReturned are required.\n")
  }

  #maybe put in textfile then parse...???
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
  
  #####TEST WITH first element######
  
  #split at "." char (fixed=T to turn off regex matching)
  dotSplitFilePath = unlist(strsplit(seglist$utts[1], ".",fixed=T ))  
  dotSplitFilePath[length(dotSplitFilePath)] <- newFieEx
  
  fname = paste(dotSplitFilePath[1], dotSplitFilePath[2], sep="")
  
  fname <- path.expand(fname)
  
#  for (i in 1:length(seglist$utts)){

    #split at "." char (fixed=T to turn off regex matching)
#    dotSplitFilePath = unlist(strsplit(seglist$utts[i], ".",fixed=T ))

#    print(dotSplitFilePath[length(dotSplitFilePath)]) = newFieEx
    
#  }
  

}#newEmu.track

####################################
#test func

#make seglist from query
#segs <- emu.query("stops", "*", "Phonetic=g")

#convert to valid file path (in local dir)
#for (i in 1:length(segs$utts)){
#  segs$utts[i] <- paste(segs$utts[i],".wav", sep="")
#}

#newEmu.track(segs,'fm')



