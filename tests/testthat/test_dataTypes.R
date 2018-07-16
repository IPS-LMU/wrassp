##' testthat tests to check if correct types are returned by wrassp
##'
##' @author Raphael Winkelmann

context("type variable testing")

test_that("wrassp returns correct windows types", {
  # window types
  wrasspWins = AsspWindowTypes()
  WTs = c("RECTANGLE", "TRIANGLE", "PARABOLA", 
          "COS", "HANN", "COS_3", "COS_4", 
          "HAMMING", "BLACKMAN", "BLACK_X", 
          "BLACK_3", "BLACK_M3", "BLACK_4", 
          "BLACK_M4", "NUTTAL_3", "NUTTAL_4", 
          "GAUSS2_5", "GAUSS3_0", "GAUSS3_5", 
          "KAISER2_0", "KAISER2_5", "KAISER3_0", 
          "KAISER3_5", "KAISER4_0")
  
  expect_that(wrasspWins, equals(WTs))

})

test_that("wrassp returns correct LP types", {
  # LP types
  wrasspLps = AsspLpTypes()
  LPTs = c("ARF", "LAR", "LPC", "RFC")
  
  expect_that(wrasspLps, equals(LPTs))

})

test_that("wrassp returns correct spect types", {
  # spect types
  wrasspSpectTypes = AsspSpectTypes()
  spectTs = c("DFT", "LPS", "CSS", "CEP")

  expect_that(wrasspSpectTypes, equals(spectTs))
  
})
