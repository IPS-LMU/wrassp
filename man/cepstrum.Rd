\name{cepstrum}
\alias{cepstrum}
\title{cepstrum}
\usage{
  cepstrum(listOfFiles = NULL, optLogFilePath = NULL,
    BeginTime = 0, CenterTime = FALSE, EndTime = 0,
    Resolution = 40, FftLength = 0, WindowShift = 5,
    Window = "BLACKMAN", ToFile = TRUE, ExplicitExt = NULL,
    forceToLog = TRUE)
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
  smallest value which results in a frequency resolution of
  <freq> Hz or better (default: 40.0)}

  \item{FftLength}{= <num>: set FFT length to <num> points
  (overrules default and 'Resolution' option)}

  \item{WindowShift}{= <dur>: set analysis window shift to
  <dur> ms (default: 5.0)}

  \item{Window}{= <type>: set analysis window function to
  <type> (default: BLACKMAN)}

  \item{ToFile}{write results to file (default extension
  depends on )}

  \item{ExplicitExt}{set if you wish to overwride the
  default extension}

  \item{forceToLog}{option to override the package default.
  Option should be left alone by normal user}
}
\value{
  nrOfProcessedFiles or if only one file to process return
  AsspDataObj of that file
}
\description{
  calculate cepstral coeffisients using libassp
}
\details{
  Short-term cepstral analysis of the signal in
  <listOfFiles> using the Fast Fourier Transform. The
  number of coefficients per output record will also equal
  the FFT length / 2 + 1 (i.e. be non-mirrored). Analysis
  results will be written to a file with the base name of
  the input file and as extension '.cep'. Default output is
  in SSFF format with the 'cep' as track name.
}
\author{
  Raphael Winkelmann

  Lasse Bombien
}

