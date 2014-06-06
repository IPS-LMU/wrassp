##' Takes any files that are URIs and downloads them to a local cache
##' 
##' @param listOfFiles The list of files to look at and download to cache if a URI
##' @return A list of files
##' @author Matt Hillman
##' @examples
##' 
##' 
##' #   listOfFiles <- prepareFiles(listOfFiles)
##' 
##' 
'prepareFiles' <- function(listOfFiles = NULL) {
    
	listOfFiles = gsub("^file://","", listOfFiles)
	listOfFiles = path.expand(listOfFiles)
    
	return(listOfFiles)
}

