\name{prepareFiles}
\alias{prepareFiles}
\title{prepareFiles}
\usage{
  prepareFiles(listOfFiles = NULL, header = NULL)
}
\arguments{
  \item{listOfFiles}{list of input files}

  \item{header}{header to be used in the curl call to
  obtain URI content}
}
\value{
  list containing updated list of files and local files
}
\description{
  checks for any URIs in the list of files and downloads 
  the file to a local cache so they can be used in the
  functions of the libassp library. Adds in the HCS vLab
  api key to the header if stored in local config file
}

