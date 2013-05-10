\name{spectrum}
\alias{spectrum}
\title{spectrum}
\usage{
  spectrum(listOfFiles = NULL, optLogFilePath = NULL,
    BeginTime = 0, CenterTime = FALSE, EndTime = 0,
    Resolution = 40, FftLength = 0, WindowSize = 20,
    WindowShift = 5, Window = "BLACKMAN", Bandwidth = 0,
    SpectrumType = "DFT", EffectiveLength = FALSE,
    Order = 0, Preemphasis = 0, Deemphasize = FALSE,
    NumCeps = 0, ToFile = TRUE, ExplicitExt = NULL)
}
\arguments{
  \item{listOfFiles}{vector of file paths to be processed
  by function}

  \item{optLogFilePath}{path to option log file}

  \item{BeginTime}{= <time>: set begin of analysis interval
  to <time> seconds (default: begin of data)}

  \item{CenterTime}{= <time>: set single-frame analysis
  with the analysis window centred at <time> seconds;
  overrules BeginTime, EndTime and WindowShift options}

  \item{EndTime}{= <time>: set end of analysis interval to
  <time> seconds (default: end of data)}

  \item{Resolution}{= <freq>: set FFT length to the
  smallest value which results in a frequency resolution of
  <freq> Hz or better (default: 40.0)}

  \item{FftLength}{= <num>: set FFT length to <num> points
  (overrules default and 'Resolution' option)}

  \item{WindowSize}{= <dur>: set analysis window size to
  <dur> ms (overrules 'Bandwidth' option and restriction by
  FFT length) additional option for cepstrally smoothed
  spectrum:}

  \item{WindowShift}{= <dur>: set analysis window shift to
  <dur> ms (default: 5.0)}

  \item{Window}{= <type>: set analysis window function to
  <type> (default: BLACKMAN)}

  \item{Bandwidth}{= <freq>: set the effective analysis
  bandwidth to <freq> Hz (default: 0, yielding the smallest
  possible value given the length of the FFT)}

  \item{SpectrumType}{=<type> set analysis type; <type> may
  be: DFT : unsmoothed spectrum (default) LPS : linear
  prediction smoothed spectrum CSS : cepstrally smoothed
  spectrum CEP : cepstral coefficients}

  \item{EffectiveLength}{make window size effective rather
  than exact}

  \item{Order}{= <num>: set prediction order to <num>
  (default: sampling rate in kHz + 3)}

  \item{Preemphasis}{= <val>: set pre-emphasis factor to
  <val> (default: -0.95)}

  \item{Deemphasize}{omit de-emphasis (default: undo
  spectral tilt due to pre-emphasis used in LP analysis)}

  \item{NumCeps}{= <num>: set number of cepstral
  coeffcients used to <num> (default: sampling rate in kHz
  + 1; minimum: 2)}

  \item{ToFile}{write results to file (default extension
  depends on )}

  \item{ExplicitExt}{set if you wish to overwride the
  default extension}
}
\value{
  nrOfProcessedFiles or if only one file to process return
  AsspDataObj of that file
}
\description{
  spectrum function adapted from libassp
}
\details{
  Short-term spectral analysis of the signal in
  <listOfFiles> using the Fast Fourier Transform. The
  default is to calculate an unsmoothed narrow-band
  spectrum with the size of the analysis window equal to
  the length of the FFT. The output from the FFT will be
  converted to a power spectrum in dB from 0 Hz up to and
  including the Nyquist rate. Alternatively, the program
  can calculate smoothed spectra or cepstra. In the latter
  case the number of coefficients per output record will
  also equal the FFT length / 2 + 1 (i.e. be non-mirrored).
  Analysis results will be written to a file with the base
  name of the input file and the spectrum type in lower
  case as extension (e.g. '.dft'). Default output is in
  SSFF format with the spectrum type in lower case as track
  name.
}
\author{
  Raphael Winkelmann
}

