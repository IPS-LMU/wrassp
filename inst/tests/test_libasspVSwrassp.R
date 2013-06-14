context("compute libassp vs. wrassp comparison")

# variable that has to be set manually before testing: 
# path2wavs = path to a folder containing wav files
# 
test_that("wrassp does the same thing as libassp", {
  expect_true(exists('path2wavs'))
  
  logFile = paste(path2wavs, "log.rmMe", sep="")
  wrasspExt = ".wrasspOut.rmMe"
  libasspExt = ".libasspOut.rmMe"
  
  fL = list.files(path2wavs, "wav", full.names=T)
  
  funcList = list(rep("acfana", 2), 
                  rep("afdiff", 2),
                  rep("affilter", 2),
                  rep("f0_ksv", 2),
                  rep("f0_mhs", 2),
                  rep("forest", 2),               
                  rep("rfcana", 2), 
                  rep("rmsana", 2),
                  c("spectrum -t=DFT", "dftSpectrum"),
                  c("spectrum -t=CEP", "cepstrum"),
                  c("spectrum -t=CSS", "cssSpectrum"),
                  c("spectrum -t=LPS", "lpsSpectrum"),
                  rep("zcrana", 2))
  
  
    
  for(i in 1:length(funcList)){
    CURR <- funcList[[i]]
    cmdName  <- CURR[1]
    funcName <- CURR[2]
    print(funcName)
    if(funcName=="affilter" || funcName=="zcr"){ #SIC because of filter options
      next;
    }
    curFun = match.fun(funcName)
    curFun(fL, optLogFilePath=logFile, ExplicitExt=wrasspExt)
    odParm = paste("-ox=", libasspExt," -od=", path.expand(path2wavs), sep="")
    
   for (file in fL){
     libCmd = paste(cmdName, odParm, file)
    
     system(libCmd)
    
     wrFileP = paste(strsplit(file, ".", fixed=T)[[1]][1], wrasspExt, sep="")
     laFileP = paste(strsplit(file, ".", fixed=T)[[1]][1], libasspExt, sep="")
    
     diffCmd = paste("diff", wrFileP, laFileP)
    
     expect_that(system(diffCmd),equals(0))
    
   }  
}
  # clean up newly created files
  # system(paste("rm ", path2wavs, "*", wrasspExt, sep=""))
  # system(paste("rm ", path2wavs, "*", libasspExt, sep=""))
  # system(paste("rm ", path2wavs, "*.rmMe", sep=""))
})

# library(wrassp)
# 
# library(devtools)
# library(testthat)
# setwd("~/Developer//wrasspDevelFolder/wrassp/")
# path2wavs="~/Desktop//ae/signals/"
# test_file("inst//tests/test_libasspVSwrassp.R")