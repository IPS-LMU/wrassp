context("test fileIO")

# variable that has to be set manually before testing: 
# path2wavs = path to a folder containing wav files
# 
test_that("read things that are written to disc are the same as origs", {
  expect_true(exists('path2wavs'))
  
  funcList = c("acfana", "afdiff",
               "affilter", "f0_ksv",
               "f0_mhs", "forest",
               "rfcana", "rmsana",
               "spectrum", "zcrana")
  
  fL = list.files(path2wavs, "wav", full.names=T)
  
  inMemObj = read.AsspDataObj(fL[1])
  newPath = paste(fL[1], ".rmMe", sep="")
  write.AsspDataObj(inMemObj, newPath)
  
  diffCmd = paste("diff", fL[1], newPath)
  expect_that(system(diffCmd),equals(0))
})