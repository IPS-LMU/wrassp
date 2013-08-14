\name{downloadTempURIFiles}
\alias{downloadTempURIFiles}
\title{downloadTempURIFiles}
\usage{
  downloadTempURIFiles(listOfFiles = NULL, header = NULL)
}
\arguments{
  \item{listOfFiles:}{list of input files}

  \item{header:}{header to be used in the curl call to
  obtain URI content}
}
\value{
  list containing updated list of files and temporary local
  files
}
\description{
  checks for any URIs in the list of files and downloads a
  temporary local file for each so they can be used in the
  functions of the libassp library
}

