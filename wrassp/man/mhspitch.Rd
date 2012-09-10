\name{mhspitch}
\alias{mhspitch}
\title{mhspitch}
\usage{
  mhspitch(listOfFiles = NULL, optLogFilePath = NULL,
    BeginTime = 0, CenterTime = FALSE, EndTime = 0,
    WindowShift = 5, Gender = "u", MaxF = 600, MinF = 50,
    MinAmp = 50, MinAC1 = 0.25, MinRMS = 18, MaxZCR = 3000,
    MinProb = 0.52, PlainSpectrum = FALSE, ToFile = TRUE,
    ExplicitExt = NULL)
}
\arguments{
  \item{listOfFiles}{vector of file paths to be processed
  by function}

  \item{optLogFilePath}{path to option log file}

  \item{BeginTime}{start time (in ms) in file to perform
  function on}

  \item{CenterTime}{???}

  \item{EndTime}{end time (in ms) in file to perform
  function on}

  \item{WindowShift}{window shift of function (in ms)}

  \item{Gender}{???}

  \item{MaxF}{???}

  \item{MinF}{???}

  \item{MinAmp}{???}

  \item{MinAC1}{???}

  \item{MinRMS}{???}

  \item{MaxZCR}{???}

  \item{MinProb}{???}

  \item{PlainSpectrum}{???}

  \item{ToFile}{write results to file (default extension is
  .pit)}

  \item{ExplicitExt}{set if you wish to overwride the
  default extension}
}
\value{
  nrOfProcessedFiles or if only one file to process return
  dataObj of that file
}
\description{
  this is the description
}
\details{
  and these details
}
\author{
  Raphael Winkelmann
}

