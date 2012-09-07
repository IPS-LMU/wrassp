##' .. content for \description{} (no empty lines) ..
##'
##' .. content for \details{} ..
##' @title 
##' @param fname 
##' @param begin 
##' @param end 
##' @param samples 
##' @return 
##' @author Raphael Winkelmann
'getDObj' <- function(fname, begin=0, end=0, samples=FALSE) {
    fname <- path.expand(fname)
    .External("getDObj2", fname, begin=begin, end=end, samples=samples, PACKAGE="wrassp")
}


