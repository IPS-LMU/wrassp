\name{createUniqueFilename}
\alias{createUniqueFilename}
\title{createUniqueFilename}
\usage{
  createUniqueFilename(URI)
}
\arguments{
  \item{URI}{URI to create filename for}
}
\value{
  filename
}
\description{
  creates non-existing filename for given URI, by using
  basename of URI and appending a number if already exists
  to ensure uniqueness
}

