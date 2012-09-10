##' getDObj creates an object of class dobj from a signal or parameter file readable by the ASSP Library (WAVE, SSFF, AUP, ...)
##'
##' and these are the details
##' @title Get a dobj data object from a signal/parameter File
##' @param fname filename of the signal or parameter file
##' @param begin begin time (default is in ms) of segment to retrieve
##' @param end end time (default is in ms) of segment to retrieve
##' @param samples (BOOL) if set to false ms values of begin/end are sample numbers
##' @return list object containing file data 
##' @author Lasse Bombien
'getDObj' <- function(fname, begin=0, end=0, samples=FALSE) {
    fname <- path.expand(fname)
    .External("getDObj2", fname, begin=begin, end=end, samples=samples, PACKAGE="wrassp")
}


