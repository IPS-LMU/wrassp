##' .. content for \description{} (no empty lines) ..
##'
##' .. content for \details{} ..
##' @title AsspWindowTypes
##' @return vector containing window types
##' @author Raphael Winkelmann
'AsspWindowTypes' <-function(){
	
	return(.Call("AsspWindowTypes"))
}

##' .. content for \description{} (no empty lines) ..
##'
##' .. content for \details{} ..
##' @title AsspLpTypes
##' @return vector containing lp types
##' @author Raphael Winkelmann
'AsspLpTypes' <-function(){
	
	return(.Call("AsspLpTypes"))
}

##' .. content for \description{} (no empty lines) ..
##'
##' .. content for \details{} ..
##' @title AsspSpectTypes
##' @return vector containing spectrogram types
##' @author Raphael Winkelmann
'AsspSpectTypes' <-function(){
	
	return(.Call("AsspSpectTypes"))
}
