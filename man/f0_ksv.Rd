\name{f0_ksv}
\alias{f0_ksv}
\alias{f0ana}
\title{f0_ksv}
\usage{
  f0_ksv(listOfFiles = NULL, optLogFilePath = NULL,
    BeginTime = 0, EndTime = 0, WindowShift = 5,
    Gender = "u", MaxF = 600, MinF = 50, MinAmp = 50,
    MaxZCR = 3000, ToFile = TRUE, ExplicitExt = NULL,
    forceToLog = useWrasspLogger)
}
\arguments{
  \item{listOfFiles}{vector of file paths to be processed
  by function}

  \item{optLogFilePath}{path to option log file}

  \item{BeginTime}{= <time>: set begin of analysis interval
  to <time> seconds (default = 0: begin of data)}

  \item{EndTime}{set end of analysis interval to <time>
  seconds (default = 0: end of data)}

  \item{WindowShift}{= <dur>: set frame shift to <dur> ms
  (default: 5.0)}

  \item{Gender}{= <code> set gender-specific F0 ranges;
  <code> may be: "f[emale]" (80.0 - 640.0 Hz) "m[ale]"
  (50.0 - 400.0 Hz) "u[nknown]" (default; 50.0 - 600.0 Hz)}

  \item{MaxF}{= <freq>: set maximum F0 value to <freq> Hz
  (default: 500.0)}

  \item{MinF}{= <freq>: set minimum F0 value to <freq> Hz
  (default: 50.0)}

  \item{MinAmp}{= <amp>: set amplitude threshold for voiced
  samples to <amp> (default: 100)}

  \item{MaxZCR}{maximum zero crossing rate in Hz (for
  voicing detection)}

  \item{ToFile}{write results to file (default extension is
  .f0)}

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
  f0_ksv function adapted from libassp
}
\details{
  F0 analysis of the signal in <listOfFile> using the K.
  Schaefer-Vincent periodicity detection algortithm.
  Analysis results will be written to a file with the base
  name of the input file and extension '.f0'. Default
  output is in SSFF binary format (track 'F0'). Optionally,
  location and type of the signal extrema on which the F0
  data are based, may be stored in a label file. The name
  of this file will consist of the base name of the F0 file
  and the extension '.prd'.
}
\author{
  Raphael Winkelmann
}

