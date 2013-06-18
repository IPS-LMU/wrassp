\name{afdiff}
\alias{afdiff}
\title{afdiff}
\usage{
  afdiff(listOfFiles = NULL, optLogFilePath = NULL,
    ComputeBackwardDifference = FALSE,
    ComputeCentralDifference = FALSE, Channel = 1,
    ToFile = TRUE, ExplicitExt = NULL,
    forceToLog = useWrasspLogger)
}
\arguments{
  \item{listOfFiles}{vector of file paths to be processed
  by function}

  \item{optLogFilePath}{path to option log file}

  \item{ComputeBackwardDifference}{compute backward
  difference (s'[n] = s[n] - s[n-1]) (default: forward
  difference s'[n] = s[n+1] - s[n])}

  \item{ComputeCentralDifference}{compute
  central/interpolated/3-point difference}

  \item{Channel}{= <num>: for multi-channel input files:
  extract and differentiate channel <num> (1 <= <num> <= 8
  default: channel 1)}

  \item{ToFile}{write results to file (default extension is
  .d+(extensionsOfAudioFile))}

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
  afdiff function adapted from libassp
}
\details{
  Computes the first difference of the signal in the audio-
  formatted file(s) <listOfFiles>. The differentiated
  signal will be written to a file with the base name of
  the input file and an extension consisting of '.d',
  followed by the extension of the input file. The format
  of the output file will be the same as that of the input
  file. Differentiation can improve results an F0 analysis
  of e.g. EGG signals because it removes a DC offset,
  attenuates very low frequency components - and in the
  case of central differentiation also very high ones - and
  enhances the moment of glottal closure.
}
\author{
  Raphael Winkelmann
}

