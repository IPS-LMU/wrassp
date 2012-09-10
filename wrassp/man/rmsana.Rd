\name{rmsana}
\alias{rmsana}
\title{rmsana}
\usage{
  rmsana(listOfFiles = NULL, BeginTime = 0,
    CenterTime = FALSE, EndTime = 0, WindowShift = 5,
    WindowSize = 20, EffectiveLength = TRUE,
    Linear = FALSE, Window = "HAMMING", ToFile = TRUE,
    ExplicitExt = NULL)
}
\arguments{
  \item{listOfFiles}{nrOfProcessedFiles or if only one file
  to process return dataObj of that file}

  \item{BeginTime}{start time (in ms) in file to perform
  function on}

  \item{CenterTime}{???}

  \item{EndTime}{end time (in ms) in file to perform
  function on}

  \item{WindowShift}{window shift of function (in ms)}

  \item{WindowSize}{window size of function (in ms)}

  \item{EffectiveLength}{???}

  \item{Linear}{???}

  \item{Window}{???}

  \item{ToFile}{write results to file (default extension is
  .rms)}

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

