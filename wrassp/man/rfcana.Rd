\name{rfcana}
\alias{rfcana}
\title{rfcana}
\usage{
  rfcana(listOfFiles = NULL, BeginTime = 0,
    CenterTime = FALSE, EndTime = 0, WindowShift = 5,
    WindowSize = 20, EffectiveLength = TRUE,
    Window = "BLACKMAN", Order = 0, Preemphasis = -0.95,
    LpType = "RFC", ToFile = TRUE, ExplicitExt = NULL)
}
\arguments{
  \item{listOfFiles}{vector of file paths to be processed
  by function}

  \item{BeginTime}{bla}

  \item{CenterTime}{bli}

  \item{EndTime}{blup}

  \item{WindowShift}{bla}

  \item{WindowSize}{bli}

  \item{EffectiveLength}{blup}

  \item{Window}{bla}

  \item{Order}{bli}

  \item{Preemphasis}{blup}

  \item{LpType}{bla}

  \item{ToFile}{bli}

  \item{ExplicitExt}{blup}
}
\value{
  nrOfProcessedFiles or if only one file to process return
  dataObj of that file
}
\description{
  .. content for \description{} (no empty lines) ..
}
\details{
  .. content for \details{} ..
}
\author{
  Raphael Winkelmann
}

