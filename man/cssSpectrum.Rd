\name{cssSpectrum}
\alias{cssSpectrum}
\title{cssSpectrum}
\usage{
  cssSpectrum(listOfFiles = NULL, optLogFilePath = NULL,
    BeginTime = 0, CenterTime = FALSE, EndTime = 0,
    Resolution = 40, FftLength = 0, WindowShift = 5,
    Window = "BLACKMAN", NumCeps = 0, ToFile = TRUE,
    ExplicitExt = NULL, forceToLog = useWrasspLogger)
}
\arguments{
  \item{listOfFiles}{vector of file paths to be processed
  by function}

  \item{optLogFilePath}{path to option log file}

  \item{BeginTime}{= <time>: set begin of analysis interval
  to <time> seconds (default: begin of data)}

  \item{CenterTime}{= <time>: set single-frame analysis
  with the analysis window centred at <time> seconds;
  overrules BeginTime, EndTime and WindowShift options}

  \item{EndTime}{= <time>: set end of analysis interval to
  <time> seconds (default: end of data)}

  \item{Resolution}{= <freq>: set FFT length to the
  smallest value which results in a freqequency resolution
  of <freq> Hz or better (default: 40.0)}

  \item{FftLength}{= <num>: set FFT length to <num> points
  (overrules default and 'Resolution' option)}

  \item{WindowShift}{= <dur>: set analysis window shift to
  <dur> ms (default: 5.0)}

  \item{Window}{= <type>: set analysis window function to
  <type> (default: BLACKMAN)}

  \item{NumCeps}{= <num>: set number of cepstral
  coeffcients used to <num> (default: sampling rate in kHz
  + 1; minimum: 2)}

  \item{ToFile}{write results to file (default extension
  depends on )}

  \item{ExplicitExt}{set if you wish to overwride the
  default extension}

  \item{forceToLog}{is set by the global package variable
  useWrasspLogger. This is set to FALSE by default and
  should be set to TRUE is logging is desired.}
}
\value{
  nrOfProcessedFiles or if only one file to process return
  AsspDataObj of that file
}
\description{
  calculate cepstrally smoothed spectrum using libassp
}
\details{
  Short-term spectral analysis of the signal in
  <listOfFiles> using the Fast Fourier Transform and
  cepstral smoothing. Analysis results will be written to a
  file with the base name of the input file and '.css.' as
  extension. Default output is in SSFF format with the
  'css' in lower case as track name.
}
\author{
  Raphael Winkelmann

  Lasse Bombien
}
\seealso{
  \code{\link{dftSpectrum}}, \code{\link{lpsSpectrum}},
  \code{\link{cepstrum}}; all derived from libassp's
  spectrum function.
}

