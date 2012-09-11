\name{getDataObjForFileWithNewExt}
\alias{getDataObjForFileWithNewExt}
\title{getDataObjForFileWithNewExt}
\usage{
  getDataObjForFileWithNewExt(OrigFilePath = NULL,
    newExt = NULL)
}
\arguments{
  \item{OrigFilePath}{file path to the original file (e.g.
  .wav file)}

  \item{newExt}{new file extension (e.g. .fms)}
}
\value{
  SSFFdataObj
}
\description{
  Get SSFFdataObj for a file with different extension
}
\details{
  Truncates the file extention of OrigFilePath and appends
  newExt to the resulting path and reads the SSFF-File and
  returns it.
}
\author{
  Raphael Winkelmann
}

