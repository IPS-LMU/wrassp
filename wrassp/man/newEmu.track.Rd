\name{newEmu.track}
\alias{newEmu.track}
%- Also NEED an '\alias' for EACH other topic documented here.
\title{
Extracts relavant data from a given database (Signalfiles+SSFF-Files)
}
\description{
Reads time relavent data from a given seglist and extracts the specified
trackdata and places it into a trackdata object (same as in emu/R). The 
seglist has to contain valid paths to the signal files and the according 
SSFF files have to be in the same directory.}

\usage{
newEmu.track(seglist = NULL, trackname = NULL, fext = NULL)
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{seglist}{seglist obtained by emu.query (or from new package seglist)}
  \item{trackname}{name of track wanted in trackdata returned (for valid options see below)}
  \item{fext}{extension of file if NULL default values of tracknames are expected}
}
\details{
%%  ~~ If necessary, more details than the description above ~~
}
\value{an object of type trackdata is returned
%%  ~Describe the value returned
%%  If it is a LIST, use
%%  \item{comp1 }{Description of 'comp1'}
%%  \item{comp2 }{Description of 'comp2'}
%% ...
}
\references{
%% ~put references to the literature/web site here ~
}
\author{R. Winkelmann <raphael@phoentik.uni-muenchen.de>
}
\note{
%fugly dude!
default extension list (if nr of cols=1
then the according trackname is equal to the file
extension ommiting the preceeding dot. if nr of cols>1
then the according trackname is equal to the file
extension ommiting the preceeding dot + ':' + the required
colum name e.g. "fms:bw" as trackname will return
the bandwidths of formants):

 - acfana -> .acf (nr of cols=1)

 - affilter:
     high-pass -> .hpf (NO SSFF! SOUND FILE!) ???
     low-pass -> .lpf (NO SSFF! SOUND FILE!) ???
     band-pass -> .bpf (NO SSFF! SOUND FILE!) ???
     band-stop -> .bsf (NO SSFF! SOUND FILE!) ???

 - mdiffsig -> .dwav  ??? other audio formats? .daiff??? (NO SSFF! SOUND FILE!)

 - f0ana -> .f0 (nr of cols=1)

 - forest -> .fms (nr of cols=4 -> rms+gain+fm+bw)
 
 - mhspitch -> .pit (nr of cols=1)

 - rfcana:
     LP Type=ARF -> .arf (nr of cols=3 -> rms+gain+arf)
     LP Type=LAR -> .lar (nr of cols=3 -> rms+gain+lar)
     LP Type=LPC -> .lpc (nr of cols=3 -> rms+gain+lpc)
     LP Type=RFC -> .rfc (nr of cols=3 -> rms+gain+rfc)

 rmsana -> .rms (nr of cols=1)

 - spectrum:
     LP Type=DFT -> .dft (nr of cols=1)
     LP Type=LPS -> .lps (nr of cols=1)
     LP Type=CSS -> .css (nr of cols=1)
     LP Type=CEP -> .cep (nr of cols=1)

 zcrana -> .zcr (nr of cols=1)

}

%% ~Make other sections like Warning with \section{Warning }{....} ~

\seealso{
}
\examples{
##---- Should be DIRECTLY executable !! ----
##-- ==>  Define data, use random,
##--	or do  help(data=index)  for the standard data sets.
\dontrun{sl = read.TextGrid(nam="FileName.TextGrid",layername="MAU",database="ALC")}
}
% Add one or more standard keywords, see file 'KEYWORDS' in the
% R documentation directory.
\keyword{emu}
\keyword{trackdata}
