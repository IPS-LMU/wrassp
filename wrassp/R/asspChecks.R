##############################################
'checkAsspWindowType' <-function(windowName){
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

###################TODO#########################
'checkAsspLPType' <-function(lpName){}



	