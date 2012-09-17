\name{rmsana}
\alias{rmsana}
\title{rmsana}
\usage{
  rmsana(listOfFiles = NULL, optLogFilePath = NULL,
    BeginTime = 0, CenterTime = FALSE, EndTime = 0,
    WindowShift = 5, WindowSize = 20,
    EffectiveLength = TRUE, Linear = FALSE,
    Window = "HAMMING", ToFile = TRUE, ExplicitExt = NULL)
}
\arguments{
  \item{listOfFiles}{vector of file paths to be processed
  by function}

  \item{optLogFilePath}{path to option log file}

  \item{BeginTime}{= <time>: set begin of analysis interval
  to <time> seconds (default = 0: begin of file)}

  \item{CenterTime}{= <time>: set single-frame analysis
  with the analysis window centred at <time> seconds;
  overrules BeginTime, EndTime and WindowShift options}

  \item{EndTime}{= <time>: set end of analysis interval to
  <time> seconds (default: end of file)}

  \item{WindowShift}{= <dur>: set analysis window shift to
  <dur> ms (default: 5.0)}

  \item{WindowSize}{= <dur>: set analysis window size to
  <dur> ms; overrules EffectiveLength option}

  \item{EffectiveLength}{= <dur>: set effective length of
  analysis window to <dur> ms (default: 20.0)}

  \item{Linear}{calculate linear RMS values (default:
  values in dB)}

  \item{Window}{= <type>: set analysis window function to
  <type> (default: HAMMING)}

  \item{ToFile}{write results to file (default extension is
  .rms)}

  \item{ExplicitExt}{set if you wish to overwride the
  default extension}
}
\value{
  nrOfProcessedFiles or if only one file to process return
  AsspDataObj of that file
}
\description{
  rmsana function adapted from libassp
}
\details{
  Analysis of short-term Root Mean Square amplitude of the
  signal in <listOfFiles>. Per default, the RMS values are
  expressed in decibel (dB) so that they correspond to the
  short-term power of the signal. Analysis results will be
  written to a file with the base name of the input file
  and extension '.rms'. Default output is in SSFF binary
  format (track 'rms').
}
\author{
  Raphael Winkelmann
}

