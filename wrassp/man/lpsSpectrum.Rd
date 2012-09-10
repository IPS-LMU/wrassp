\name{lpsSpectrum}
\alias{lpsSpectrum}
\title{lpsSpectrum}
\usage{
  lpsSpectrum(listOfFiles = NULL, BeginTime = 0,
    CenterTime = FALSE, EndTime = 0, Resolution = 40,
    FftLength = 0, WindowSize = 20, WindowShift = 5,
    Window = "BLACKMAN", Bandwidth = 0,
    EffectiveLength = FALSE, Order = 0,
    Preemphasis = -0.95, Deemphasize = FALSE, NumCeps = 0,
    ToFile = TRUE, ExplicitExt = NULL)
}
\arguments{
  \item{listOfFiles}{vector of file paths to be processed
  by function}

  \item{BeginTime}{bli}

  \item{CenterTime}{blup}

  \item{EndTime}{bla}

  \item{Resolution}{bli}

  \item{FftLength}{blup}

  \item{WindowSize}{bla}

  \item{WindowShift}{bli}

  \item{Window}{blup}

  \item{Bandwidth}{bla}

  \item{EffectiveLength}{bli}

  \item{Order}{blup}

  \item{Preemphasis}{bla}

  \item{Deemphasize}{bli}

  \item{NumCeps}{blup}

  \item{ToFile}{write results to file (default extension is
  .lps)}

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

