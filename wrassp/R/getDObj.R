##' .. content for \description{} (no empty lines) ..
##'
##' .. content for \details{} ..
##' @title getDObj
##' @param fname file name to retrieve
##' @param begin begin time (default is in ms) of segment to retrieve
##' @param end end time (default is in ms) of segment to retrieve
##' @param samples (BOOL) if set to false ms values of begin/end are sample numbers
##' @return list object containing file data 
##' @author Raphael Winkelmann
'getDObj' <- function(fname, begin=0, end=0, samples=FALSE) {
    fname <- path.expand(fname)
    .External("getDObj2", fname, begin=begin, end=end, samples=samples, PACKAGE="wrassp")
}


