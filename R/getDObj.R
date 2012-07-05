##' getDObj creates an object of class dobj from a signal or parameter
##' file readable by the ASSP Library (WAVE, SSFF, AUP, ...)
##'
##' .. content for details section ..
##' @title Get a dobj data object from a signal/parameter File
##' @param fname the path to a signal or paraneter file
##' @return an object of class dobj
##' @author Lasse Bombien
getDObj <- function(fname) {
    fname <- path.expand(fname)
    .Call("getDObj", fname, PACKAGE="wrassp")
}


