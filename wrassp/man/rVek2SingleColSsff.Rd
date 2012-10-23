\name{rVek2SingleColSsff}
\alias{rVek2SingleColSsff}
\title{rVekSingleColSsff}
\usage{
  rVek2SingleColSsff(OneDimFloatVek, Path2newSsffFile,
    RecordFreq, StartTime, ColName)
}
\arguments{
  \item{OneDimFloatVek}{R vektor (a simple example of such
  a vektor would be c(0.2, 0.1, 0.4))}

  \item{Path2newSsffFile}{full path to file that will be
  created (caution if file exist ist will be overwritten)}

  \item{RecordFreq}{time (in ms) between samples in
  OneDimFloatVek}

  \item{StartTime}{time of first sample (in ms)}

  \item{ColName}{name of column under which the data is
  saved in the SSFF-File}
}
\value{
  NULL
}
\description{
  Write single column vektor of floats to SSFF-file format
}
\details{
  Simple function that writes an R vektor to a SSFF file.
  The file header is written to file using the write()
  function then reopened in binary append mode and the
  column data is added using the writeBin() function.
}
\author{
  Raphael Winkelmann
}

