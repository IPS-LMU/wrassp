\name{getAsspDataObj}
\alias{getAsspDataObj}
\title{Get a dobj data object from a signal/parameter File}
\usage{
  getAsspDataObj(fname, begin = 0, end = 0,
    samples = FALSE)
}
\arguments{
  \item{fname}{filename of the signal or parameter file}

  \item{begin}{begin time (default is in ms) of segment to
  retrieve}

  \item{end}{end time (default is in ms) of segment to
  retrieve}

  \item{samples}{(BOOL) if set to false ms values of
  begin/end are sample numbers}
}
\value{
  list object containing file data
}
\description{
  getAsspDataObj creates an object of class dobj from a
  signal or parameter file readable by the ASSP Library
  (WAVE, SSFF, AUP, ...)
}
\details{
  and these are the details
}
\author{
  Lasse Bombien
}

