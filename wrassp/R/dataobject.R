##' read.AsspDataObj creates an object of class dobj from a signal or parameter file readable by the ASSP Library (WAVE, SSFF, AUP, ...)
##'
##' and these are the details
##' @title Get a dobj data object from a signal/parameter File
##' @param fname filename of the signal or parameter file
##' @param begin begin time (default is in ms) of segment to retrieve
##' @param end end time (default is in ms) of segment to retrieve
##' @param samples (BOOL) if set to false ms values of begin/end are sample numbers
##' @return list object containing file data
##' @author Lasse Bombien
'read.AsspDataObj' <- 'getAsspDataObj' <- function(fname, begin=0, end=0, samples=FALSE) {
  fname <- path.expand(fname)
  .External("getDObj2", fname, begin=begin, end=end, samples=samples, PACKAGE="wrassp")
}

##' Prints an overview of ASSP Data Objects
##'
##' nothing really
##' @title print aa summary of an AsspDataObj
##' @param x an object of class AsspDataObj
##'
##' @param ... other arguments that might be passed on to other functions 
##' @author Lasse Bombien
##' @seealso \code{\link{getDObj}}
"print.AsspDataObj" <- function(x, ...)
{
    temp <- attr(x, "filePath")
    if (is.null(temp)) {
        cat("In-memory Assp Data Object\n")
    }
    else {
        cat(paste("Assp Data Object of file ", temp, ".\n", sep=""))
    }

    cat(paste(as.integer(attr(x, 'end_record') -
                         attr(x, 'start_record') + 1),
              "records at", attr(x, 'samplerate'), "Hz.\n"))
    cat(paste("Number of tracks:", length(names(x)), "\n"))
    for (track in names(x)) {
        cat('\t', track)
        cat(paste(" (", ncol(x[[track]]), " fields)\n", sep=''))
    }
}

##' Writes an object of class ASspDataObj to a file given the meta information
##' contained in the object.
##'
##' Details
##' @title write
##' @param dobj an object of class AsspDataObj
##' @param file file name as a character string, defaults to the
##' \code{filePath} attribute of the AsspDataObj
##' @return NULL
##' @author Lasse Bombien
"write.AsspDataObj" <- function (dobj, file=attr(dobj, 'filePath'))
  {
    file <- path.expand(file)
    .Call("writeDObj", dobj, file, PACKAGE="wrassp")
  }

##' .. content for \description{} (no empty lines) ..
##'
##' .. content for \details{} ..
##' @title Checks whether x is a valid AsspDataObj
##' @param x 
##' @param ... 
##' @return TRUE or FALSE
##' @author Lasse Bombien
is.AsspDataObj <- function (x, ...)
  {
    if (class (x) != "AsspDataObj")
      return (FALSE)
    return (TRUE)
  }


##' Remove a track from an
##' AsspDataObj object
##'
##' .. content for \details{} ..
##' @title Remove track from an AsspDataObj
##' @param dobj An object of class AsspDataObj
##' @param trackname the name of a track in this object
##' @return The object without the track named trackname
##' @author Lasse Bombien
delTrack <- function (dobj, trackname)
  {
    if (!is.AsspDataObj (dobj))
      stop ('First argument must be a AsspDataObj.')

    w <- which (names (dobj) == trackname)
    if (length (w) != 1)
      stop ('Invalid trackname')

    ## remove track
    dobj[[trackname]] <- NULL
    ## remove
    attr(dobj, 'trackformats') <- attr(dobj, 'trackformats')[-w]
    
    return (dobj)
  }

##' Add a track to an AsspDataObj
##'
##' The specified data object is extended by a new track named \code{trackname}.
##' If there already is a track with the same name and \code{deleteExisiting}
##' is \code{FALSE} the function does nothing but returns with an error. If
##' \code{deleteExisting} is \code{TRUE} the existing track will be removed
##' (see \code{\link{delTrack}}.
##' \code{data} to be added is a numeric matrix (or will be coerced to one).
##' It must have
##' the same number of rows as the tracks that already exist in the object
##' (if any). TODO add \code{format} information.
##' @title Add a track to an AsspDataObj
##' @param dobj The data object to which the data is to be added
##' @param trackname The name of the new track
##' @param data a matrix with values
##' @param format format for binary writing to file (defaults to 'INT16') 
##' @param deleteExisting Delete existing track with the same (default: FALSE)
##' @return the object including the new track
##' @author Lasse Bombien
##' @seealso \code{\link{delTrack}}
addTrack <- function (dobj, trackname, data, format = 'INT16',
                      deleteExisting=FALSE) {
  if (!is.AsspDataObj(dobj))
    stop('dobj must be an AsspDataObj.')
  
  if (!is.numeric(data))
    stop('data must be a numeric matrix')
  
  if (!is.character(trackname) | length(trackname) != 1)
    stop('trackname must be an atomic string.')
  
  data <- as.matrix(data)
  
  tracks <- names(dobj)
  w <- tracks  == trackname
  if (any(w) & !deleteExisting)
    stop(paste('Track', trackname,
                'exists and will not be deleted',
                '("deleteExisting" argument)'))
  if (length(tracks) == 1 & any(w)){
      ## this is fine: the only track will be replaced
  } else if (length(tracks) > 0) {
    if (nrow(data) != nrow(dobj[[1]]))
      stop(paste("number of rows in data must match number of rows in",
                  "existing tracks."))
  }

  dobj[[trackname]] <- data
  if (any(w))
    attr(dobj, 'trackformats')[w] <- format
  else
    append(attr(dobj, 'trackformats'), format)

  return(dobj)
}
    
