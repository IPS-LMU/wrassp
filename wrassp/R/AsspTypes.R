##' this is the description
##'
##' and these are the details
##' @title AsspWindowTypes
##' @return vector containing window types
##' @author Raphael Winkelmann
'AsspWindowTypes' <-function(){
	
	return(.Call("AsspWindowTypes", PACKAGE = "wrassp"))
}

##' this is the description
##'
##' an these are the details
##' @title AsspLpTypes
##' @return vector containing lp types
##' @author Raphael Winkelmann
'AsspLpTypes' <-function(){
	
	return(.Call("AsspLpTypes", PACKAGE = "wrassp"))
}

##' this is the description
##'
##' and these are the details
##' @title AsspSpectTypes
##' @return vector containing spectrogram types
##' @author Raphael Winkelmann
'AsspSpectTypes' <-function(){
	
	return(.Call("AsspSpectTypes", PACKAGE = "wrassp"))
}
