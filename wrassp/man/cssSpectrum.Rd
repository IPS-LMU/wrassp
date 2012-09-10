\name{cssSpectrum}
\alias{cssSpectrum}
\title{cssSpectrum}
\usage{
  cssSpectrum(listOfFiles = NULL, optLogFilePath = NULL,
    BeginTime = 0, CenterTime = FALSE, EndTime = 0,
    Resolution = 40, FftLength = 0, WindowSize = 0,
    WindowShift = 5, Window = "BLACKMAN", Bandwidth = 0,
    EffectiveLength = FALSE, Order = 0, Preemphasis = 0,
    Deemphasize = FALSE, NumCeps = 0, ToFile = TRUE,
    ExplicitExt = NULL)
}
\arguments{
  \item{listOfFiles}{vector of file paths to be processed
  by function}

  \item{optLogFilePath}{path to option log file}

  \item{BeginTime}{bla}

  \item{CenterTime}{bli}

  \item{EndTime}{blup}

  \item{Resolution}{bla}

  \item{FftLength}{bli}

  \item{WindowSize}{blup}

  \item{WindowShift}{bla}

  \item{Window}{bli}

  \item{Bandwidth}{blup}

  \item{EffectiveLength}{bla}

  \item{Order}{bli}

  \item{Preemphasis}{blup}

  \item{Deemphasize}{bla}

  \item{NumCeps}{bli}

  \item{ToFile}{write results to file (default extension is
  .css)}

  \item{ExplicitExt}{set if you wish to overwride the
  default extension}
}
\value{
  nrOfProcessedFiles or if only one file to process return
  dataObj of that file
}
\description{
  description
}
\details{
  details
}
\author{
  Raphael Winkelmann
}

