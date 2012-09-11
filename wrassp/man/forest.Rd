\name{forest}
\alias{forest}
\title{forest}
\usage{
  forest(listOfFiles = NULL, optLogFilePath = NULL,
    BeginTime = 0, EndTime = 0, WindowShift = 5,
    WindowSize = 20, EffectiveLength = TRUE,
    NominalF1 = 500, Gender = "m", Estimate = FALSE,
    Order = 0, IncrOrder = 0, NumFormants = 4,
    Window = "BLACKMAN", Preemphasis = -0.8, ToFile = TRUE,
    ExplicitExt = NULL)
}
\arguments{
  \item{listOfFiles}{vector of file paths to be processed
  by function}

  \item{optLogFilePath}{path to option log file}

  \item{BeginTime}{bla}

  \item{EndTime}{bli}

  \item{WindowShift}{blup}

  \item{WindowSize}{bla}

  \item{EffectiveLength}{bli}

  \item{NominalF1}{blup}

  \item{Gender}{bla}

  \item{Estimate}{bli}

  \item{Order}{blup}

  \item{IncrOrder}{bla}

  \item{NumFormants}{bli}

  \item{Window}{blup}

  \item{Preemphasis}{bla}

  \item{ToFile}{write results to file (default extension is
  .fms)}

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
  and these are the details
}
\author{
  Raphael Winkelmann
}

