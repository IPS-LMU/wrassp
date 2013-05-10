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

  \item{BeginTime}{= <time>: set begin of analysis interval
  to <time> seconds (default = 0: begin of data)}

  \item{EndTime}{= <time>: set end of analysis interval to
  <time> seconds (default = 0: end of data)}

  \item{WindowShift}{= <dur>: set analysis window shift to
  <dur> ms (default: 5.0)}

  \item{WindowSize}{= <dur>: set analysis window size to
  <dur> ms (default: 30.0)}

  \item{EffectiveLength}{make window size effective rather
  than exact}

  \item{NominalF1}{= <freq>: set nominal F1 frequency to
  <freq> Hz (default: 500.0 Hz)}

  \item{Gender}{= <type>: set gender specific parameters
  where <type> can be f=female, m=male or u=unspecified
  (when <type>=f: eff. window length = 12.5 ms nominal F1 =
  560.0 Hz)}

  \item{Estimate}{insert rough frequency estimates of
  missing formants (default: frequency set to zero)}

  \item{Order}{decrease default order by 2 (one resonance
  less)}

  \item{IncrOrder}{increase default order by 2 (one
  resonance more)}

  \item{NumFormants}{= <num>: set number of formants to
  <num> (default: 4; maximum: 8 or half the LP order)}

  \item{Window}{= <type>: set analysis window function to
  <type> (default: BLACKMAN)}

  \item{Preemphasis}{= <val>: set pre-emphasis factor to
  <val> (-1 <= val <= 0) (default: dependent on sample rate
  and nominal F1)}

  \item{ToFile}{write results to file (default extension is
  .fms)}

  \item{ExplicitExt}{set if you wish to overwride the
  default extension}
}
\value{
  nrOfProcessedFiles or if only one file to process return
  AsspDataObj of that file
}
\description{
  forest function adapted from libassp
}
\details{
  Formant estimation of the signal(s) in <listOfFiles>. Raw
  resonance frequency and bandwidth values are obtained by
  root-solving of the Linear Prediction polynomial from the
  autocorrelation method and the Split-Levinson-Algorithm
  (SLA). Resonances are then classified as formants using
  the so-called Pisarenko frequencies (by-product of the
  SLA) and a formant frequeny range table derived from the
  nominal F1 frequency. The latter may have to be increased
  by about 12% for female voices (see NominalF1 and Gender
  options). Formant estimates will be written to a file
  with the base name of the input file and extension
  '.fms'. Default output is in SSFF binary format (tracks
  'fm' and ''bw)
}
\author{
  Raphael Winkelmann
}

