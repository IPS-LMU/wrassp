\name{acfana}
\alias{acfana}
\title{acfana}
\usage{
  acfana(listOfFiles = NULL, BeginTime = 0,
    CenterTime = FALSE, EndTime = 0, WindowShift = 5,
    WindowSize = 20, EffectiveLength = TRUE,
    Window = "BLACKMAN", AnalysisOrder = 0,
    EnergyNormalization = FALSE,
    LengthNormalization = FALSE, ToFile = TRUE,
    ExplicitExt = NULL)
}
\arguments{
  \item{listOfFiles}{vector of file paths to be processed
  by function}

  \item{BeginTime}{start time (in ms) in file to perform
  function on}

  \item{CenterTime}{?????}

  \item{EndTime}{end time (in ms) in file to perform
  function on}

  \item{WindowShift}{window shift of function (in ms)}

  \item{WindowSize}{window size of function (in ms)}

  \item{EffectiveLength}{?????}

  \item{Window}{type of window see isAsspWindowType
  function}

  \item{AnalysisOrder}{????}

  \item{EnergyNormalization}{????}

  \item{LengthNormalization}{????}

  \item{ToFile}{write results to file (default extension is
  .acf)}

  \item{ExplicitExt}{set if you wish to overwride the
  default extension}
}
\value{
  nrOfProcessedFiles or if only one file to process return
  dataObj of that file
}
\description{
  acfana function adapted from assp library
}
\details{
  still have to write propper documentation
}
\author{
  Raphael Winkelmann
}

