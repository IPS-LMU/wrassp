##' force user to use logging (BOOL)
##'
##' package global to force user to log function calls and parameters (done automatically)
##' @title forceToLog
##' @author Raphael Winkelmann
##' @useDynLib wrassp
##' @export
'forceToLog' <- TRUE;

##' bla
##'
##' bli
##' @title toggleForceToLog
##' @author Raphael Winkelmann
##' @useDynLib wrassp
##' @export
'toggleForceToLog' <- function(){
  
  if(forceToLog){
    cat("is true -> swtiching to false")
    forceToLog <<- FALSE
  }else{
    cat("is false -> switching to true")
    forceToLog <<- TRUE
  }
 
}
