\name{acfana}
\alias{acfana}
\title{acfana}
\usage{
  acfana(listOfFiles = NULL, optLogFilePath = NULL,
    BeginTime = 0, CenterTime = FALSE, EndTime = 0,
    WindowShift = 5, WindowSize = 20,
    EffectiveLength = TRUE, Window = "BLACKMAN",
    AnalysisOrder = 0, EnergyNormalization = FALSE,
    LengthNormalization = FALSE, ToFile = TRUE,
    ExplicitExt = NULL)
}
\arguments{
  \item{listOfFiles}{vector of file paths to be processed
  by function}

  \item{optLogFilePath}{path to option log file}

  \item{BeginTime}{= <time>: set begin of analysis interval
  to <time> seconds (default: 0 = beginning of file)}

  \item{CenterTime}{= <time>: set single-frame analysis
  with the analysis window centred at <time> seconds;
  overrules BeginTime, EndTime and WindowShift options}

  \item{EndTime}{= <time>: set end of analysis interval to
  <time> seconds (default: 0 = end of file)}

  \item{WindowShift}{= <dur>: set analysis window shift to
  <dur> ms (default: 5.0)}

  \item{WindowSize}{= <dur>: set analysis window size to
  <dur> ms; overrules EffectiveLength parameter}

  \item{EffectiveLength}{make window size effective rather
  than exact}

  \item{Window}{= <type>: set analysis window function to
  <type> (default: BLACKMAN)}

  \item{AnalysisOrder}{= <num>: set analysis order to <num>
  (default: 0 = sample rate in kHz + 3)}

  \item{EnergyNormalization}{calculate energy-normalized
  autocorrelation}

  \item{LengthNormalization}{calculate length-normalized
  autocorrelation}

  \item{ToFile}{write results to file (default extension is
  .acf)}

  \item{ExplicitExt}{set if you wish to overwride the
  default extension}
}
\value{
  nrOfProcessedFiles or if only one file to process return
  AsspDataObj of that file
}
\description{
  acfana function adapted from libassp
}
\details{
  Analysis of short-term autocorrelation function of the
  signals in <listOFFiles>. Analysis results will be
  written to a file with the base name of the input file
  and extension '.acf'. Default output is in SSFF binary
  format (track 'acf').
}
\author{
  Raphael Winkelmann
}

