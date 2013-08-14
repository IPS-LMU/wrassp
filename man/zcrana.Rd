\name{zcrana}
\alias{zcrana}
\title{zcrana}
\usage{
  zcrana(listOfFiles = NULL, optLogFilePath = NULL,
    BeginTime = 0, CenterTime = FALSE, EndTime = 0,
    WindowShift = 5, WindowSize = 25, ToFile = TRUE,
    ExplicitExt = NULL, OutputDirectory = NULL,
    forceToLog = useWrasspLogger)
}
\arguments{
  \item{listOfFiles}{vector of file paths to be processed
  by function}

  \item{optLogFilePath}{path to option log file}

  \item{BeginTime}{= <time>: set begin of analysis interval
  to <time> seconds (default: begin of file)}

  \item{CenterTime}{= <time> set single-frame analysis with
  the analysis window centred at <time> seconds; overrules
  BeginTime, EndTime and WindowShift options}

  \item{EndTime}{= <time>: set end of analysis interval to
  <time> seconds (default: end of file)}

  \item{WindowShift}{= <dur>: set analysis window shift to
  <dur> ms (default: 5.0)}

  \item{WindowSize}{= <dur>: set analysis window size to
  <dur> ms (default: 25.0)}

  \item{ToFile}{write results to file (default extension is
  .zcr)}

  \item{ExplicitExt}{set if you wish to overwride the
  default extension}

  \item{OutputDirectory}{directory in which output files
  are stored. Defaults to NULL, i.e. the directory of the
  input files}

  \item{forceToLog}{is set by the global package variable
  useWrasspLogger. This is set to FALSE by default and
  should be set to TRUE is logging is desired.}
}
\value{
  nrOfProcessedFiles or if only one file to process return
  AsspDataObj of that file
}
\description{
  zcrana function adapted from libassp
}
\details{
  Analysis of the averages of the short-term positive and
  negative zero-crossing rates of the signal in
  <listOfFiles>. Analysis results will be written to a file
  with the base name of the input file and extension
  '.zcr'. Default output is in SSFF binary format (track
  'zcr').
}
\author{
  Raphael Winkelmann
}

