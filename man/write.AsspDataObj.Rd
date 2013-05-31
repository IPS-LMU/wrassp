\name{write.AsspDataObj}
\alias{write.AsspDataObj}
\title{write.AsspDataObj to file}
\usage{
  write.AsspDataObj(dobj, file = attr(dobj, "filePath"))
}
\arguments{
  \item{dobj}{an object of class AsspDataObj}

  \item{file}{file name as a character string, defaults to
  the \code{filePath} attribute of the AsspDataObj}
}
\value{
  NULL
}
\description{
  Writes an object of class AsspDataObj to a file given the
  meta information contained in the object.
}
\author{
  Lasse Bombien
}

