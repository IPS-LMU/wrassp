\name{afdiff}
\alias{afdiff}
\title{afdiff}
\usage{
  afdiff(listOfFiles = NULL, optLogFilePath = NULL,
    ComputeBackwardDifference = FALSE,
    ComputeCentralDifference = FALSE, Channel = 1,
    ToFile = TRUE, ExplicitExt = NULL)
}
\arguments{
  \item{listOfFiles}{vector of file paths to be processed
  by function}

  \item{optLogFilePath}{path to option log file}

  \item{ComputeBackwardDifference}{???}

  \item{ComputeCentralDifference}{???}

  \item{Channel}{channel of audio file to compute diff
  (default = 1)}

  \item{ToFile}{write results to file (default extension is
  .d+(extensionsOfAudioFile))}

  \item{ExplicitExt}{set if you wish to overwride the
  default extension}
}
\value{
  nrOfProcessedFiles or if only one file to process return
  dataObj of that file
}
\description{
  afdiff adapted from assp
}
\details{
  still have to write propper manual entry
}
\author{
  Raphael Winkelmann
}

