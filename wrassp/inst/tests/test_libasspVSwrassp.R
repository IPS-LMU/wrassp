context("compute libassp vs. wrassp comparison")

# variable that has to be set manually before testing: 
# path2wavs = path to a folder containing wav files
# 
test_that("wrassp does the same thing as libassp", {
  expect_true(exists('path2wavs'))
  
  logFile = paste(path2wavs, "log.rmMe", sep="")
  wrasspExt = ".wrasspOut"
  libasspExt = ".libasspOut"
  
  fL = list.files(path2wavs, "wav", full.names=T)
  
  nrPrF = f0_ksv(fL, optLogFilePath=logFile, ExplicitExt=wrasspExt)
  
  for (f in fL){
    odParm = paste("-ox=", libasspExt," -od=", path2wavs, sep="")
    libCmd = paste("f0_ksv", odParm, f)
  
    system(libCmd)
  
    wrFileP = paste(strsplit(f, ".", fixed=T)[[1]][1], wrasspExt, sep="")
    laFileP = paste(strsplit(f, ".", fixed=T)[[1]][1], libasspExt, sep="")
  
    diffCmd = paste("diff", wrFileP, laFileP)
  
    expect_that(system(diffCmd),equals(0))
  
  }  
  #clean up newly created files
  system(paste("rm ", path2wavs, "*", wrasspExt, sep=""))
  system(paste("rm ", path2wavs, "*", libasspExt, sep=""))
  system(paste("rm ", path2wavs, "*.rmMe", sep=""))
})