\name{mhsF0}
\alias{f0_mhs}
\alias{mhsF0}
\alias{mhspitch}
\title{mhsF0}
\usage{
  mhsF0(listOfFiles = NULL, optLogFilePath = NULL,
    BeginTime = 0, CenterTime = FALSE, EndTime = 0,
    WindowShift = 5, Gender = "u", MaxF = 600, MinF = 50,
    MinAmp = 50, MinAC1 = 0.25, MinRMS = 18, MaxZCR = 3000,
    MinProb = 0.52, PlainSpectrum = FALSE, ToFile = TRUE,
    ExplicitExt = NULL, forceToLog = useWrasspLogger)
}
\arguments{
  \item{listOfFiles}{vector of file paths to be processed
  by function}

  \item{optLogFilePath}{path to option log file}

  \item{BeginTime}{= <time>: set begin of analysis interval
  to <time> seconds (default = 0: begin of file)}

  \item{CenterTime}{= <time>: set single-frame analysis
  with the analysis window centred at <time> seconds;
  overrules BeginTime, EndTime and WindowShift options}

  \item{EndTime}{= <time>: set end of analysis interval to
  <time> seconds (default = 0: end of file)}

  \item{WindowShift}{= <dur>: set analysis window shift to
  <dur> ms (default: 5.0)}

  \item{Gender}{= <code> set gender-specific pitch ranges;
  <code> may be: "f[emale]" (80.0 - 600.0 Hz) "m[ale]"
  (50.0 - 375.0 Hz) "u[nknown]" (default; 50.0 - 600.0 Hz)}

  \item{MaxF}{= <freq>: set maximum pitch value to <freq>
  Hz (default: 500.0)}

  \item{MinF}{= <freq>: set minimum pitch value to <freq>
  Hz (default: 50.0 minimum: 25.0)}

  \item{MinAmp}{= <amp>: minimum signal amplitude (default:
  50)}

  \item{MinAC1}{= <freq>: minimum 1st correlation
  coefficient (default: 0.250)}

  \item{MinRMS}{= <num>: minimum RMS amplitude in dB
  (default: 18.0)}

  \item{MaxZCR}{= <freq>: maximum zero crossing rate in Hz
  (default: 3000)}

  \item{MinProb}{= <num>: minimum quality value of F0 fit
  (default: 0.520)}

  \item{PlainSpectrum}{use plain rather than masked power
  spectrum}

  \item{ToFile}{write results to file (default extension is
  .pit)}

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
  mhsF0 function adapted from libassp
}
\details{
  Pitch analysis of the speech signal in <listOfFile> using
  Michel's/Modified Harmonic Sieve algorithm. Analysis
  results will be written to a file with the base name of
  the input file and extension '.pit'. Default output is in
  SSFF binary format (track 'pitch').
}
\author{
  Raphael Winkelmann
}
\seealso{
  \code{\link{ksvF0}} for an tracking the fundamental
  frequency
}

