
##' Write single column vektor of floats to SSFF-file format
##'
##' Simple function that writes an R vektor to a SSFF file. The file header is written to file using the write() function then reopened in binary append mode and the column data is added using the writeBin() function.
##' @title rVekSingleColSsff
##' @param OneDimFloatVek R vektor (a simple example of such a vektor would be c(0.2, 0.1, 0.4))
##' @param Path2newSsffFile full path to file that will be created (caution if file exist ist will be overwritten) 
##' @param RecordFreq time (in ms) between samples in OneDimFloatVek
##' @param StartTime time of first sample (in ms) 
##' @param ColName name of column under which the data is saved in the SSFF-File
##' @return NULL
##' @author Raphael Winkelmann
rVek2SingleColSsff <- function(OneDimFloatVek, Path2newSsffFile, RecordFreq, StartTime, ColName){
  ssffFile <- file(Path2newSsffFile, 'w')
  
  headStr = "SSFF -- (c) SHLRC"

  machineType = "Machine IBM-PC"
  
  rFreq = paste("Record_Freq", RecordFreq)
  sT = paste("Start_Time", StartTime)
  c1 = paste("Column", ColName, "FLOAT 1")

  endStr = "-----------------"

  if (.Platform$endian != "little"){
    stop("plattform is not little endian! -> not supported yet sorry!")
  }

  write(headStr, ssffFile)
  write(machineType, ssffFile)
  write(rFreq, ssffFile)
  write(sT, ssffFile)
  write(c1, ssffFile)
  write(endStr, ssffFile)

  flush(ssffFile)
  close(ssffFile)

  ssffFile <- file(Path2newSsffFile, 'ab')
  
  writeBin(OneDimFloatVek, ssffFile, size=4)

  close(ssffFile)
  
}

