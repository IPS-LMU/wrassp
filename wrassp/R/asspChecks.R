##############################################
"isAsspWindowType" <- function(windowName = NULL) {
	if (is.null(windowName)) {
		stop("No windowName given!")
	}

	winTypes = .Call("AsspWindowTypes")

	isValidWindow = FALSE

	for (type in winTypes) {
		if (windowName == type) {
			isValidWindow = TRUE
			break
		}
	}
	return(isValidWindow)
}

############################################
"isAsspLpType" <- function(lpName = NULL) {
	if (is.null(lpName)) {
		stop("No lpName given!")
	}

	lpTypes = .Call("AsspLpTypes")

	isValidLp = FALSE

	for (type in lpTypes) {
		if (lpName == type) {
			isValidLp = TRUE
			break
		}
	}
	return(isValidLp)
}


###########################################
"checkForExistingFiles" <- function(listOfFilePaths, newExt) {

	problemFiles <- NULL

	for (file in listOfFilePaths) {
		basePath = unlist(strsplit(file, ".", fixed = T))[1]
		newPath = paste(basePath, newExt, sep = "")
		if (!file.exists(newPath)) {
			print(newPath)
			problemFiles <- c(problemFiles, newPath)
		}
	}

	userInput = "xxx"
	if (!is.null(problemFiles)) {
		while (!((userInput == "y") | (userInput == "n"))) {
			cat("######################\n")
			print(problemFiles)
			cat("\n", "Following files exist with the extension '", newExt, 
				"'! Do you wish to overwrite them? (enter 'y' for yes or 'n' for no )", 
				"\n")
			userInput <- readline("enter a positive integer: ")
			if (nchar(userInput) == 0) {
				userInput = "xxx"
			}
		}
	} else {
		userInput = "y"
	}

	if (userInput == "y") {
		return(TRUE)
	} else {
		return(FALSE)
	}

}

checkForExistingFiles(dir("~/tmp/kielread/signals/wav", ".wav$", full.name = T), 
	".fms")
