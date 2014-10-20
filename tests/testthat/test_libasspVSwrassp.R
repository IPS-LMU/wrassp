##' testthat test to see if output of libassp is the same as 
##' wrassp output. This test will only 
##' work on a system where the libassp is installed and the path 
##' variable is set to point to the binaries
##'
##' @author Raphael Winkelmann
context("compute libassp vs. wrassp comparison")

test_that("wrassp does the same thing as libassp", {
  
  require(compare)
  
  altDir = tempdir()
  
  fromWrasspDir = file.path(altDir, 'fromWrassp')
  fromLibasspDir = file.path(altDir, 'fromLibassp')
  
  if(!file.exists(fromWrasspDir)){
    dir.create(fromWrasspDir)
  }
  if(!file.exists(fromLibasspDir)){
    dir.create(fromLibasspDir)
  }
  
  wavFiles <- list.files(system.file("extdata", package = "wrassp"), pattern = glob2rx("*.wav"), full.names = TRUE)
  # only run tests on my machine because I know that libassp is installed on it
  if(system('whoami', intern = T) == 'raphaelwinkelmann'){
    
    for (func in names(wrasspOutputInfos)){
      funcFormals = formals(func)
      funcFormals$listOfFiles = wavFiles[1]
      funcFormals$outputDirectory = fromWrasspDir
      
      res = do.call(func,as.list(funcFormals))
      
      sBaseName = unlist(strsplit(basename(wavFiles[1]), '.', fixed = T))
      ssffFileName  = paste(sBaseName[1], wrasspOutputInfos[[func]]$ext[1], sep = '.')
      
      # all functions that don't need additional handling
      if(func == 'acfana' ||
           func == 'afdiff' ||
           func == 'forest' ||
           func == 'rfcana' ||
           func == 'rmsana' ||
           func == 'zcrana'){
        
        system(paste(func, wavFiles[1], paste('-od=', normalizePath(fromLibasspDir), sep = '')), intern=T)
      }else if(func == 'affilter'){
        system(paste(func, wavFiles[1], '-hp=4000', paste('-od=', normalizePath(fromLibasspDir), sep = '')))
      }else if(func == 'cepstrum'){
        system(paste('spectrum', wavFiles[1], '-t=CEP', paste('-od=', normalizePath(fromLibasspDir), sep = '')))
      }else if(func == 'cssSpectrum'){
        system(paste('spectrum', wavFiles[1], '-t=CSS', paste('-od=', normalizePath(fromLibasspDir), sep = '')))
      }else if(func == 'dftSpectrum'){
        system(paste('spectrum', wavFiles[1], '-t=DFT', paste('-od=', normalizePath(fromLibasspDir), sep = '')))
      }else if(func == 'ksvF0'){
        system(paste('f0_ksv', wavFiles[1], paste('-od=', normalizePath(fromLibasspDir), sep = '')))
      }else if(func == 'mhsF0'){
        system(paste('f0_mhs', wavFiles[1], paste('-od=', normalizePath(fromLibasspDir), sep = '')))
      }else if(func == 'lpsSpectrum'){
        system(paste('spectrum', wavFiles[1], '-t=LPS', paste('-od=', normalizePath(fromLibasspDir), sep = '')))
      }else{
        stop('No test case defined for function name: ', func)
      }
      fromLibassp = read.AsspDataObj(paste(fromLibasspDir, ssffFileName, sep='/'))
      fromWrassp = read.AsspDataObj(paste(fromWrasspDir, ssffFileName, sep='/'))
      compRes = compare(fromLibassp, fromWrassp, allowAll = T)
      expect_true(compRes$result)
    }
  }
})
