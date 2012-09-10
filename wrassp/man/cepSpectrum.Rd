\name{cepSpectrum}
\alias{cepSpectrum}
\title{cep}
\usage{
  cepSpectrum(listOfFiles = NULL, optLogFilePath = NULL,
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

  \item{BeginTime}{start time (in ms) in file to perform
  function on}

  \item{CenterTime}{???}

  \item{EndTime}{end time (in ms) in file to perform
  function on}

  \item{Resolution}{???}

  \item{FftLength}{???}

  \item{WindowSize}{window size of function (in ms)}

  \item{WindowShift}{window shift of function (in ms)}

  \item{Window}{type of window (see AsspWindowTypes()
  function)}

  \item{Bandwidth}{???}

  \item{EffectiveLength}{???}

  \item{Order}{???}

  \item{Preemphasis}{???}

  \item{Deemphasize}{???}

  \item{NumCeps}{???}

  \item{ToFile}{write results to file (default extension is
  .cep)}

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

