##' testthat test to see if writing to then reading from disc
##' changes anything 
##'
##' @author Raphael Winkelmann
context("test fileIO")

test_that("read things that are written to disc are the same as origs", {
  
  altDir = tempdir()
  
  wavFiles <- list.files(system.file("extdata", package = "wrassp"), pattern = glob2rx("*.wav"), full.names = TRUE)
  
  for (func in names(wrasspOutputInfos)){
    for(wavFile in wavFiles){
      funcFormals = formals(func)
      funcFormals$listOfFiles = wavFile
      funcFormals$outputDirectory = altDir
      funcFormals$explicitExt = "testthat"
      res = do.call(func,as.list(funcFormals))
      path2new = paste(altDir, basename(wavFile), sep="/")
      sp=unlist(strsplit(path2new, ".", fixed = T))
      sp[length(sp)] = funcFormals$explicitExt
      fromFile = read.AsspDataObj(paste(sp, collapse="."))
      funcFormals$toFile = FALSE
      funcFormals$verbose = FALSE
      inMem = do.call(func,as.list(funcFormals))
      # test attributes if they are the same 
      for (at in names(attributes(fromFile))){
        if(at != "filePath"){
          expect_that(length(attr(fromFile, at)), equals(sum(attr(fromFile, at)==attr(inMem, at))))
        }
      }
      # test if data is the same
      expect_that(sum(unlist(inMem[attr(inMem,"names")]) == unlist(fromFile[attr(fromFile,"names")]))
                  , equals(length(unlist(inMem[attr(inMem,"names")]))))
    }
  }
  
  # clean up
  files <- list.files(altDir, "testthat$", full.names=T)
  
  for (file in files){
    unlink(file)
  }
  
})

test_that("files containing utf-8 symbols in file names work (was an issue under windows)", {
  
  altDir = tempdir()
  
  wavFiles <- list.files(system.file("extdata", package = "wrassp"), pattern = glob2rx("*.wav"), full.names = TRUE)
  
  utf8filePath = enc2utf8(file.path(tempdir(), "bål.wav"))
  file.copy(wavFiles[1], utf8filePath)
  # check that reading works
  ado = read.AsspDataObj(utf8filePath)
  expect_equal(attr(ado, "sampleRate"), 16000)
  # is encoding of filePath preserved?
  expect_equal(Encoding(attr(ado, "filePath")), "UTF-8")
  # check that writing works
  write.AsspDataObj(ado, paste0(utf8filePath, "_new"))
  expect_true(file.exists(paste0(utf8filePath, "_new")))
  # function work 
  zcrana(listOfFiles = utf8filePath)
  expect_true(file.exists(paste0(tools::file_path_sans_ext(utf8filePath), ".zcr")))
  unlink(utf8filePath)
  unlink(paste0(utf8filePath, "_new"))
})