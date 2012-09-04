##############################################
'isAsspWindowType' <-function(windowName=NULL){
	if(is.null(windowName)){stop("No windowName given!")}
	
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
'isAsspLpType' <-function(lpName=NULL){
	if(is.null(lpName)){stop("No lpName given!")}
	
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



	