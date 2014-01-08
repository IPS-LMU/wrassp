\name{viewCache}
\alias{viewCache}
\title{viewCache}
\usage{
  viewCache()
}
\description{
  opens up the cache contents file to display all the
  files stored in the local cache and the URIs they are
  mapped with. The local filenames are computated by
  taking an MD5 hash of the entire URI it corresponds
  to and appending the appropriate file extension.
}
\format{
  <URI> -> <MD5 has of URI + file extension>

  e.g.
  http://www.phonetik.uni-muenchen.de/~raphael/data/msajc003.wav -> 40586c2ce57890eb058efd6a4d00cf3f.wav
}
