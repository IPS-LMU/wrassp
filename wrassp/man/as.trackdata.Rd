\name{as.trackdata}
\alias{as.trackdata}
\title{ Create an Emu trackdata object }
\description{
  Create an Emu trackdata object from a raw data matrix.
}
\usage{
  as.trackdata(data, index, ftime, trackname="")
}
\arguments{
 \item{data}{ A two dimensional matrix of numerical data. }
 \item{index}{ Segment index, one row per segment, two columns give the
   start and end rows in the \code{data} matrix for each segment.  }
 \item{ftime}{ A two column matrix with one row per segment, gives the
   start and end times in milliseconds for each segment. }
 \item{trackname}{ The name of the track. }
}
\details{
   Emu trackdata objects contain possibly multi-column numerical data
   corresponding to a set of segments from a database.  Data for each
   segment takes up a number of rows in the main \code{data} matrix, the 
   start and end rows are stored in the \code{index} component.  The
   \code{ftime} component contains the start and end times of the
   segment data.

   Trackdata objects are returned by the \code{\link{emu.track}} function.
}
\value{
  The components are bound into a trackdata object.
}

\seealso{ \code{\link{emu.track}} \code{\link{dplot}} }

\examples{

# make a trackdata object of two data segments
data1 <- matrix( 1:10, ncol=2 )
data2 <- matrix( 11:20, ncol=2 )

nd1 <- nrow(data1)
nd2 <- nrow(data2)
index <- rbind( c( 1, nd1 ), c(nd1+1,nd1+nd2) )

times <- rbind( c( 100.0, 110.0 ), c( 200.0, 210.0 ) )

tdata <- as.trackdata( rbind( data1, data2 ), index, times, trackname="fake")

# describe the data
summary(tdata)
# get the data for the first segment
tdata[1]
# and the second
tdata[2]

}


\keyword{misc}
