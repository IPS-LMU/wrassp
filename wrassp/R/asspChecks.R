##' this is the description
##'
##' and these are the details
##' @title isAsspWindowType
##' @param windowName name of window
##' @return (BOOL) true if windowName is valid 
##' @author Raphael Winkelmann
"isAsspWindowType" <- function(windowName = NULL) {
	if (is.null(windowName)) {
		stop("No windowName given!")
	}

	winTypes = AsspWindowTypes()

	isValidWindow = FALSE

	for (type in winTypes) {
		if (windowName == type) {
			isValidWindow = TRUE
			break
		}
	}
	return(isValidWindow)
}

##' this is the description
##'
##' and these are the details
##' @title isAsspLpType
##' @param lpName name of lp type
##' @return (BOOL) true if lpName is valid
##' @author Raphael Winkelmann
"isAsspLpType" <- function(lpName = NULL) {
	if (is.null(lpName)) {
		stop("No lpName given!")
	}

	lpTypes = AsspLpTypes()

	isValidLp = FALSE

	for (type in lpTypes) {
		if (lpName == type) {
			isValidLp = TRUE
			break
		}
	}
	return(isValidLp)
}


##' this is the description
##'
##' and these are the details
##' @title hasDupicateFiles
##' @param listOfFilePaths 
##' @param newExt 
##' @return (BOOL)
##' @author Raphael Winkelmann
#"hasDuplicateFiles" <- function(listOfFilePaths, newExt) {

#	problemFiles <- NULL

#	for (file in listOfFilePaths) {
#		basePath = unlist(strsplit(file, ".", fixed = T))[1]
#		newPath = paste(basePath, newExt, sep = "")
#		if (!file.exists(newPath)) {
#			print(newPath)
#			problemFiles <- c(problemFiles, newPath)
#		}
#	}
#
#	userInput = "xxx"
#	if (!is.null(problemFiles)) {
#		while (!((userInput == "y") | (userInput == "n"))) {
#			cat("######################\n")
#			print(problemFiles)
##			cat("\n", "Following files exist with the extension '", newExt, 
#				"'! Do you wish to overwrite them?", 
#				"\n")
#			userInput <- readline("type 'y' for yes or 'n' for no: ")
#			if (nchar(userInput) == 0) {
#				userInput = "xxx"
#			}
#		}
#	} else {
#		userInput = "y"
#	}
#
#	if (userInput == "y") {
#		return(TRUE)
#	} else {
#		return(FALSE)
#	}
#
#}
