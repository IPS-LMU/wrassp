\name{zcrana}
\alias{zcrana}
\title{zcrana}
\usage{
  zcrana(listOfFiles = NULL, optLogFilePath = NULL,
    BeginTime = 0, CenterTime = FALSE, EndTime = 0,
    WindowShift = 5, WindowSize = 25, ToFile = TRUE,
    ExplicitExt = NULL)
}
\arguments{
  \item{listOfFiles}{vector of file paths to be processed
  by function}

  \item{optLogFilePath}{path to option log file}

  \item{BeginTime}{start time (in ms) in file to perform
  function on}

  \item{CenterTime}{???}

  \item{EndTime}{end time (in ms) in file to perform
  function on}

  \item{WindowShift}{window shift of function (in ms)}

  \item{WindowSize}{window size of function (in ms)}

  \item{ToFile}{write results to file (default extension is
  .zcr)}

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

