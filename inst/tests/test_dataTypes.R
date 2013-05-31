context("type testing")

test_that("wrassp returns correct types", {
  # window types
  wrasspWins = AsspWindowTypes()
  WTs = c("RECTANGLE", "PARABOLA", "COS", "HANN", 
    "COS_4", "HAMMING", "BLACKMAN", "BLACK_X",
    "BLACK_M3", "BLACK_M4", "NUTTAL_3", "NUTTAL_4",
    "KAISER2_0", "KAISER3_0", "KAISER4_0")
  
  expect_that(wrasspWins, equals(WTs))
  
  # window types
  wrasspLps = AsspLpTypes()
  LPTs = c("ARF", "LAR", "LPC", "RFC")
  
  expect_that(wrasspLps, equals(LPTs))
  
  # spect types
  wrasspSpectTypes = AsspSpectTypes()
  spectTs = c("DFT", "LPS", "CSS", "CEP")

  expect_that(wrasspSpectTypes, equals(spectTs))
  
})