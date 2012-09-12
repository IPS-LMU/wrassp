
##' Prints an overview of ASSP Data Objects
##'
##' nothing really
##' @title print.AsspDataObj
##' @param dobj an object of class AsspDataObj
##'
##' @author Lasse Bombien
##' @seealso \code{\link{getDObj}}
"print.AsspDataObj" <- function(dobj)
{
    temp <- attr(dobj, "filePath")
    if (is.null(temp)) {
        cat("In-memory Assp Data Object\n")
    }
    else {
        cat(paste("Assp Data Object of file ", temp, ".\n", sep=""))
    }

    cat(paste(as.integer(attr(dobj, 'end_record') -
                         attr(dobj, 'start_record') + 1),
              "records at", attr(dobj, 'samplerate'), "Hz.\n"))
    cat(paste("Number of tracks:", length(names(dobj)), "\n"))
    for (track in names(dobj)) {
        cat('\t', track)
        cat(paste(" (", ncol(dobj[[track]]), " fields)\n", sep=''))
    }
}
