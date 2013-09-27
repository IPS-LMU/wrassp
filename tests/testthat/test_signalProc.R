##' testthat test to check multiple varying parameters
##' on all of the functions of wrassp somehow breaks the
##' functions 
##'
##' @author Raphael Winkelmann
context("test signal processing functions")

###################################
# acfana
test_that("acfana doesn't brake due to varying parameters", {

  nrOfRandomCalls = 10

  wavFiles <- list.files(system.file("extdata", package = "wrassp"), pattern = glob2rx("*.wav"), full.names = TRUE)

  posValsBeginTime = list(0) #list(0, 0.0001, 0.1)
  posValsCenterTime = list(FALSE) #list(TRUE, FALSE)
  posValsEndTime = list(0) #list(0, 0.1001, 0.2, 1, 1.2)
  posValsWindowShift = list(5) #list(1, 5, 10, 7)
  posValsWindowSize = list(20) #???
  posValsEffectiveLength = list(TRUE) #list(TRUE, FALSE)
  posValsWindow = as.list(AsspWindowTypes())
  posValsAnalysisOrder = list(0) #???
  posValsEnergyNormalization = list(FALSE) #list(TRUE, FALSE)
  posValsLengthNormalization = list(FALSE) #list(TRUE, FALSE)

  for(i in 1:nrOfRandomCalls){
    params = list(listOfFiles=sample(wavFiles, 1)[[1]], optLogFilePath=NULL, 
                  BeginTime=sample(posValsBeginTime, 1)[[1]], CenterTime=sample(posValsCenterTime, 1)[[1]],
                  EndTime=sample(posValsEndTime, 1)[[1]], WindowShift=sample(posValsWindowShift, 1)[[1]], 
                  WindowSize=sample(posValsWindowSize, 1)[[1]], EffectiveLength=sample(posValsEffectiveLength, 1)[[1]], 
                  Window=sample(posValsWindow, 1)[[1]], AnalysisOrder=sample(posValsAnalysisOrder, 1)[[1]], 
                  EnergyNormalization=sample(posValsEnergyNormalization, 1)[[1]], LengthNormalization=sample(posValsLengthNormalization, 1)[[1]], 
                  ToFile=FALSE, forceToLog=useWrasspLogger)
    # print(params)
    res = do.call(acfana,as.list(params))

    expect_that(class(res), equals("AsspDataObj"))
  }

})

##################################
# afdiff
test_that("afdiff doesn't brake due to varying parameters", {

  nrOfRandomCalls = 10

  wavFiles <- list.files(system.file("extdata", package = "wrassp"), pattern = glob2rx("*.wav"), full.names = TRUE)

  posValsComputeBackwardDifference = list(FALSE) #list(TRUE, FALSE)
  posValsComputeCentralDifference = list(FALSE) #list(TRUE, FALSE)
  posValsChannel = list(1)

  for(i in 1:nrOfRandomCalls){
    params = list(listOfFiles=sample(wavFiles, 1)[[1]], optLogFilePath=NULL, 
                  ComputeBackwardDifferenc=sample(posValsComputeBackwardDifference,1)[[1]], ComputeCentralDifference=sample(posValsComputeCentralDifference,1)[[1]],
                  Channel=sample(posValsChannel,1)[[1]], ToFile=FALSE, 
                  forceToLog=useWrasspLogger)
    # print(params)
    res = do.call(afdiff,as.list(params))
  }

})

test_that("affilter doesn't brake due to varying parameters", {
  # TODO
  
})

test_that("cepstrum doesn't brake due to varying parameters", {
  # TODO
  
})

test_that("cssSpectrum doesn't brake due to varying parameters", {
  # TODO
  
})

test_that("dftSpectrum doesn't brake due to varying parameters", {
  # TODO
  
})

test_that("forest doesn't brake due to varying parameters", {
  # TODO
  
})

test_that("ksvF0 doesn't brake due to varying parameters", {
  # TODO
  
})

test_that("lpsSpectrum doesn't brake due to varying parameters", {
  # TODO
  
})

test_that("mhsF0 doesn't brake due to varying parameters", {
  # TODO
  
})

test_that("rfcana doesn't brake due to varying parameters", {
  # TODO
  
})

test_that("rmsana doesn't brake due to varying parameters", {
  # TODO
  
})

test_that("zcrana doesn't brake due to varying parameters", {
  # TODO
  
})