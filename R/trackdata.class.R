#############################################################################
#                                                                           #
#   copyright            : (C) 2000 SHLRC, Macquarie University             #
#   email                : Steve.Cassidy@mq.edu.au			    #
#   url			 : http://www.shlrc.mq.edu.au/emu		    #
#									    #
#   This program is free software; you can redistribute it and/or modify    #
#   it under the terms of the GNU General Public License as published by    #
#   the Free Software Foundation; either version 2 of the License, or       #
#   (at your option) any later version.                                     #
#									    #
#############################################################################

##########################################################################
#modified by Raphael Winkelmann <Aug. 2012>

## Methods that define operations on the class "trackdata"
## see also track and frames

"print.trackdata"<- function(x, ...)
{
  if(is.null(x$trackname)) 
    cat("trackdata from unknown track.\n")
  else
    cat("trackdata from track:", x$trackname,"\n")

  cat("index:\n")
  print(x$index, ...)
  cat("ftime:\n")
  print(x$ftime, ...)
  cat("data:\n")
  print(x$data, ...)
}




"[.trackdata" <-
  function (dataset, i, j, ...) 
{


  if (missing(i)) {
    i <- 1:nrow(dataset$index)
  }


  ftime <- dataset$ftime[i, , drop = FALSE]
  index <- dataset$index[i, , drop = FALSE]


  datarows <- NULL
  for (ind in 1:nrow(index)) {
    datarows <- c(datarows, seq(from = index[ind, 1], to = index[ind, 
                                                        2]))
  }
  if (is.matrix(dataset$data)) {
    if (missing(j)) 
      data <- dataset$data[datarows, , drop = FALSE]
    else data <- dataset$data[datarows, j, drop = FALSE]
  } else {
    data <- dataset$data[datarows, drop = FALSE]
  }
  lval <- index[, 2] - index[, 1] + 1
  right <- cumsum(lval)
  left <- right + 1
  left <- left[-length(left)]
  left <- c(1, left)
  nindex <- cbind(left, right)
  dataset$index <- nindex
  dataset$ftime <- ftime
  dataset$data <- data
  return(dataset)
}








"summary.trackdata" <- function(object, ...)
{
  if( is.matrix(object$data)){
    dimens <- ncol(object$data)
    len <- nrow(object$data)
  }
  else {
    dimens <- 1
    len <- length(object$data)
  }
  cat("Emu track data from", nrow(object$index), "segments\n\n")
  cat("Data is ", dimens, "dimensional from track", 
      object$trackname,"\n")
  cat("Mean data length is ", len/nrow(object$index), " samples\n")
  invisible()
}


"as.trackdata" <- function( data, index, ftime, trackname="" )
{
  mat <- list( data=as.matrix(data), 
	      index=index, 
	      ftime=ftime,
	      trackname=trackname)
  if( version$major >= 5 ) {
    oldClass(mat) <- "trackdata"
  } else {
    class(mat) <- "trackdata"
  }
  mat
}

"is.trackdata" <-
  function (object) 
{
  return(inherits(object, "trackdata"))
}

`plot.trackdata` <-
  function (x, timestart = NULL, xlim = NULL, ylim = NULL, labels = NULL, 
            col = TRUE, lty = FALSE, type="p", pch=NULL, contig = TRUE, ...) 
{
  trackdata <- x
  N <- nrow(trackdata$data)
  if(is.logical(col))
    {
      if (col) 
	col <- 1:ncol(trackdata)
      else
        col <- rep(1, ncol(trackdata))
    }
  else
    {
      if(length(col)!=ncol(trackdata))
        col <- rep(col[1], ncol(trackdata))
    }

  if(is.logical(lty))
    {
      if (lty) 
	lty <- 1:ncol(trackdata)
      else
        lty <- rep(1, ncol(trackdata))
    }
  else
    {
      if(length(lty)!=ncol(trackdata))
        lty <- rep(lty[1], ncol(trackdata))
    }
  if(is.null(pch))
    pch <- rep(1, ncol(trackdata))
  else
    {
      if(length(pch)!=ncol(trackdata))
        pch <- rep(pch[1], ncol(trackdata))
    }

  
  n <- nrow(trackdata)
  if (!is.null(xlim)) 
    labels <- NULL
  if (!is.null(labels)) {
    if (length(labels) != nrow(trackdata)) 
      stop("if labels are supplied, there must be one label per segment")
    label.times <- apply(trackdata$ftime, 1, mean)
    boundary.times <- c(trackdata$ftime[, 1], trackdata$ftime[n])
  }
  if (n > 1 & contig) {
    inds <- cbind(1, N)
    ftime <- cbind(trackdata$ftime[1, 1], trackdata$ftime[n, 
                                                          2])
    trackdata <- as.trackdata(trackdata$data, inds, ftime)
  }
  if (!is.null(xlim)) {
    if (nrow(trackdata) != 1) 
      stop("can't specify xlim if there's more than one segment")
  }
  left <- trackdata$ftime[1]
  right <- trackdata$ftime[2]
  times <- seq(left, right, length = nrow(trackdata$data))
  if (!is.null(timestart)) {
    times <- times - left + timestart
    if (!is.null(labels)) {
      label.times <- label.times - left + timestart
      boundary.times <- boundary.times - left + timestart
    }
  }
  data <- trackdata$data
  if (nrow(trackdata) == 1) {
    if (is.null(xlim)) 
      xlim <- range(times)
    if (is.null(ylim)) 
      ylim <- range(data)
    for (k in 1:ncol(data)) {
      if(k==ncol(data))
        plot(times, data[, k], xlim = xlim, ylim = ylim, 
             col = col[k], lty = lty[k], pch=pch[k], type=type, ...)
      else
        plot(times, data[, k], xlim = xlim, ylim = ylim, 
             col = col[k], lty = lty[k], pch=pch[k], xlab="", ylab="", main="", axes=FALSE, bty="n", type=type)
      par(new = TRUE)
    }
    par(new = FALSE)
    if (!is.null(labels)) {
      if (length(boundary.times) > 2) 
        abline(v = boundary.times)
      mtext(labels, at = label.times)
    }
  }
  else {
    if (is.null(labels)) 
      labels <- rep("", nrow(trackdata))
    for (j in 1:nrow(trackdata)) {
      plot(trackdata[j, ], timestart = timestart, xlim = xlim, 
           ylim = ylim, labels = labels[j], col=col, lty=lty, type=type, pch=pch,contig = TRUE, ...)
    }
  }
}




"bark.trackdata" <-
  function(f, ...)
{
  trackdata = f
  if(is.spectral(trackdata$data))
    return(bark.spectral(trackdata))
  else
    {
      trackdata$data <- bark(trackdata$data)
      return(trackdata)
    }
}



"mel.trackdata" <-
  function(a)
{
  trackdata = a
  if(is.spectral(trackdata$data))
    return(mel.spectral(trackdata))
  else
    {
      trackdata$data <- mel(trackdata$data)
      return(trackdata)
    }
}





"trackfreq" <-
  function(specdata){
    if(is.trackdata(specdata))
      return(attr(specdata$data, "fs"))
    else
      return(attr(specdata, "fs"))
  }


"get.trackkeywrd" <-
  function (fname) 
{
  line <- readLines(fname, n = 2)
  if (length(line) < 2) {
    return(NULL)
  }
  
  line <- splitstring(line[2], " ")
  if ((length(line) == 3) && (line[2] == "Trackname")) {
    trackname <- line[3]
  }
  else {
    return(NULL)
  }
  if (trackname  !=  "") {
    return(trackname)
  }
  else {
    return(NULL)
  }
}


"dur.trackdata" <-
  function (x) 
{
  x$ftime[,2] - x$ftime[,1]
}

"frames" <- function(trackdata)
{
  if(!(is.trackdata(trackdata)))
    stop ("Object must be of class trackdata")
  trackdata$data
}



# Local Variables:
# mode:S
# S-temp-buffer-p:t
# End:
