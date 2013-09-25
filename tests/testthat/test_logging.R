context("test logging capabilities")

test_that("wrong parameters combi for logging causes error", {
  
#   funcList = c("acfana", "afdiff",
#                    "affilter", "f0_ksv",
#                    "f0_mhs", "forest",
#                    "rfcana", "rmsana",
#                    "spectrum", "zcrana")
  
#   for(f in funcList){
#     curFun = match.fun(f)
#     expect_error(curFun("~/someFile.doesNotExist"), "optLogFilePath is NULL! -> not logging!")
#     expect_error(curFun("~/someFile.doesNotExist",forceToLog=F), "Can't open file")
#   }
# })

# variable that has to be set manually before testing: 
# path2wavs = path to a folder containing wav files
# 
test_that("logging output is created", {
  # expect_true(exists('path2wavs'))
  
  # fL = list.files(path2wavs, "wav", full.names=T)
  # acfana(fL[1], paste(path2wavs, "log.tmp", sep=""), ExplicitExt=".rmMe")
  
  # expContentHead = paste("\n##################################\n", 
  #                    "##################################\n", 
  #                    "######## acfana performed ########\n", sep="") 
  
  # logContent=suppressWarnings(readLines(paste(path2wavs, "log.tmp",sep="")))
  
  # logContent = paste(logContent,collapse="\n")
  
  # expect_match(logContent, expContentHead)
  
  # #clean up newly created files
  # system(paste("rm ", path2wavs, "log.tmp", sep=""))
  # system(paste("rm ", path2wavs, "*.rmMe", sep=""))
  
})