\name{rfcana}
\alias{rfcana}
\title{rfcana}
\usage{
  rfcana(listOfFiles = NULL, optLogFilePath = NULL,
    BeginTime = 0, CenterTime = FALSE, EndTime = 0,
    WindowShift = 5, WindowSize = 20,
    EffectiveLength = TRUE, Window = "BLACKMAN", Order = 0,
    Preemphasis = -0.95, LpType = "RFC", ToFile = TRUE,
    ExplicitExt = NULL, forceToLog = forceToLogDefault)
}
\arguments{
  \item{listOfFiles}{vector of file paths to be processed
  by function}

  \item{optLogFilePath}{path to option log file}

  \item{BeginTime}{= <time>: set begin of analysis interval
  to <time> seconds (default = 0: begin of file)}

  \item{CenterTime}{set single-frame analysis with the
  analysis window centred at <time> seconds; overrules
  BeginTime, EndTime and WindowShift options}

  \item{EndTime}{= <time>: set end of analysis interval to
  <time> seconds (default = 0: end of file)}

  \item{WindowShift}{= <dur>: set analysis window shift to
  <dur> ms (default: 5.0)}

  \item{WindowSize}{= <dur>: set analysis window size to
  <dur> ms; overrules EffectiveLength option}

  \item{EffectiveLength}{make window size effective rather
  than exact}

  \item{Window}{= <type>: set analysis window function to
  <type> (default: BLACKMAN)}

  \item{Order}{= <num>: set prediction order to <num>
  (default: sample rate in kHz + 3)}

  \item{Preemphasis}{= <val>: set pre-emphasis factor to
  <val> (default: -0.95)}

  \item{LpType}{= <type>: calculate <type> LP parameters;
  <type> may be: "ARF": area function "LAR": log area
  ratios "LPC": linear prediction filter coefficients
  "RFC": reflection coefficients (default)}

  \item{ToFile}{write results to file (default extension
  dependent on LpType .arf/.lar/.lpc/.rfc)}

  \item{ExplicitExt}{set if you wish to overwride the
  default extension}
}
\value{
  nrOfProcessedFiles or if only one file to process return
  AsspDataObj of that file
}
\description{
  rfcana function adapted from libassp
}
\details{
  Linear Prediction analysis of <listOfFiles> using the
  autocorrelation method and the Durbin recursion. This
  program calculates the RMS amplitudes of the input and
  residual signal in dB and, per default, reflection
  coefficients (see '-t' option). Analysis results will be
  written to a file with the base name of the input file
  and the parameter type in lower case as extension (e.g.
  '.rfc'). Default output is in SSFF binary format (tracks
  'rms', 'gain' and the LP type in lower case).
}
\author{
  Raphael Winkelmann
}

