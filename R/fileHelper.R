##' Normalise a list of filenames so that they can be passed to a signal processing function
##' 
##' @param listOfFiles The list of file names to process
##' @return A normalised list of filenames
##' @author Matt Hillman
##' @examples
##' 
##' 
##' #   listOfFiles <- prepareFiles(listOfFiles)
##' 
##' 
'prepareFiles' <- function(listOfFiles) {
    
	listOfFiles = gsub("^file://","", listOfFiles)
	listOfFiles = path.expand(listOfFiles)
    
	return(listOfFiles)
}
