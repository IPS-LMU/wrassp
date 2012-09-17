\name{f0ana}
\alias{f0ana}
\title{f0ana}
\usage{
  f0ana(listOfFiles = NULL, optLogFilePath = NULL,
    BeginTime = 0, EndTime = 0, WindowShift = 5,
    Gender = "u", MaxF = 600, MinF = 50, MinAmp = 50,
    MaxZCR = 3000, ToFile = TRUE, ExplicitExt = NULL)
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

  \item{Gender}{???}

  \item{MaxF}{= <freq>: set maximum F0 value to <freq> Hz
  (default: 500.0)}

  \item{MinF}{= <freq>: set minimum F0 value to <freq> Hz
  (default: 50.0)}

  \item{MinAmp}{= <amp>: set amplitude threshold for voiced
  samples to <amp> (default: 100)}

  \item{MaxZCR}{???}

  \item{ToFile}{write results to file (default extension is
  .f0)}

  \item{ExplicitExt}{set if you wish to overwride the
  default extension}
}
\value{
  nrOfProcessedFiles or if only one file to process return
  AsspDataObj of that file
}
\description{
  f0ana function adapted from libassp
}
\details{
  F0 analysis of the signal in <listOfFile> using the
  Schaefer- Vincent periodicity detector (Phonetica 40,
  1983). Analysis results will be written to a file with
  the base name of the input file and extension '.f0'.
  Default output is in single precision floating point
  binary in SSFF format (keyword 'F0'). Optionally,
  location and type of the signal extrema on which the F0
  data are based, may be stored in a label file. The name
  of this file will consist of the base name of the F0 file
  and the extension '.prd'.
}
\author{
  Raphael Winkelmann
}

