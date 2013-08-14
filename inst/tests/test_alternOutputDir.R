context("test alternative output dir")

test_that("all signal processing functions run without errors on audio files", {
  
  altDir = "/tmp/"
  
  wavFiles <- list.files(system.file("extdata", package = "wrassp"), pattern = glob2rx("*.wav"), full.names = TRUE)
  
  for (func in names(wrasspOutputInfos)){
    funcFormals = formals(func)
    funcFormals$listOfFiles = wavFiles
    funcFormals$OutputDirectory = altDir
    res = do.call(func,as.list(funcFormals))
    expect_that(res, equals(NULL))
    
    #clean up newly created files
    for (ex in wrasspOutputInfos[[func]]$ext){
      suppressWarnings(system(paste("rm ", altDir, "*", ex, sep="")))
    }
  }

})