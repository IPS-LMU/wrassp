##' checks for any URIs in the list of files and downloads a temporary local file for
##' each so they can be used in the functions of the libassp library
##' @title downloadTempURIFiles
##' @param listOfFiles: list of input files
##' @param header: header to be used in the curl call to obtain URI content
##' @return list containing updated list of files and temporary local files
'downloadTempURIFiles' <- function(listOfFiles = NULL, header = NULL) {
	tmp <- c()
	for(i in 1:length(listOfFiles)) {
		if(substr(listOfFiles[i], 1, 7) == "http://") {
			tmpFilename <- createUniqueFilename(listOfFiles[i])
			if (is.null(header)) {
				bFile <- getBinaryURL(listOfFiles[i])
			} else {
				bFile <- getBinaryURL(listOfFiles[i], httpheader=header)
			}
			writeBin(bFile, tmpFilename)
			listOfFiles[i] <- tmpFilename
			tmp <- c(tmp, tmpFilename)
		}
	}
	list("listOfFiles" = listOfFiles, "tmpFiles" = tmp)
}


##' deletes all files in the given list if they exist
##' @title deleteTempFiles
##' @param tmpFiles: list of files to be deleted
'deleteTempFiles' <- function(tmpFiles) {
	for(i in 1:length(tmpFiles)) {
		if (file.exists(tmpFiles[i])) file.remove(tmpFiles[i])
	}
}


##' creates non-existing filename for given URI, by using basename of URI
##' and appending a number if already exists to ensure uniqueness 
##' @title createUniqueFilename
##' @param URI: URI to create filename for
##' @return filename
'createUniqueFilename' <- function(uri) {
	file <- basename(uri)
	if(regexpr("\\.[^\\.]*$", file) != -1) {
		filename <- substring(file, 1, regexpr("\\.[^\\.]*$", file)-1)
	} else {
		filename <- file
	}
	file <- filename
	count <- 2
	# append/increase number until filename is unique to working directory
	while(file.exists(paste(filename, ".fms", sep=""))) {
		filename <- paste(file, "(", toString(count), ")", sep="")
		count <- count + 1
	}
	filename
}
