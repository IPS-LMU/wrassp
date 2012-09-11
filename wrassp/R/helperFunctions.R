##' Get SSFFdataObj for a file with different extension
##'
##' Truncates the file extention of OrigFilePath and appends newExt to the resulting
##' path and reads the SSFF-File and returns it.
##' @title getDataObjForFileWithNewExt
##' @param OrigFilePath file path to the original file (e.g. .wav file)
##' @param newExt new file extension (e.g. .fms)
##' @return SSFFdataObj
##' @author Raphael Winkelmann
getDataObjForFileWithNewExt <- function(OrigFilePath = NULL, newExt = NULL){


  splitPath = unlist(strsplit(OrigFilePath, ".", fixed=T))
  path = paste(c(splitPath[1:length(splitPath)-1], newExt), collapse='')
  resDataObj = getDObj(path)

  return(resDataObj)

}
