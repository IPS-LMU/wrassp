context("test signal processing functions")

# variable that has to be set manually before testing: 
# path2wavs = path to a folder containing wav files
# 
test_that("all signal processing functions run without errors on audio files", {
  # expect_true(exists('path2wavs'))
  
  # fL = list.files(path2wavs, "wav", full.names=T)
  
  # for (func in names(wrasspOutputInfos)){
  #   funcFormals = formals(func)
  #   funcFormals$listOfFiles = fL
  #   res = do.call(func,as.list(funcFormals))
  #   expect_that(res, equals(NULL))
    
  #   #clean up newly created files
  #   for (ex in wrasspOutputInfos[[func]]$ext){
  #     system(paste("rm ", path2wavs, "*", ex, sep=""))
  #   }

  # }
  
})