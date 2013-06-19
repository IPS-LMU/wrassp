context("test signal processing functions")

# variable that has to be set manually before testing: 
# path2wavs = path to a folder containing wav files
# 
test_that("all signal processing functions run without errors on audio files", {
  expect_true(exists('path2wavs'))
  
  fL = list.files(path2wavs, "wav", full.names=T)
  
  inMemObj = read.AsspDataObj(fL[1])
  newPath = paste(fL[1], ".rmMe", sep="")
  write.AsspDataObj(inMemObj, newPath)
  
  diffCmd = paste("diff", fL[1], newPath)
  expect_that(system(diffCmd),equals(0))
})