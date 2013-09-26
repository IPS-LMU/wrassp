##' checks for any URIs in the list of files and downloads a temporary local file for
##' each so they can be used in the functions of the libassp library.
##' Adds in the HCS vLab api key to the header if stored in local config file
##' @title downloadTempURIFiles
##' @param listOfFiles list of input files
##' @param header header to be used in the curl call to obtain URI content
##' @return list containing updated list of files and temporary local files
'downloadTempURIFiles' <- function(listOfFiles = NULL, header = NULL) {
	tmp <- c()
	for(i in 1:length(listOfFiles)) {
		if(substr(listOfFiles[i], 1, 7) == "http://") {
			tmpFilename <- createUniqueFilename(listOfFiles[i])
			if (is.null(header) || nchar(header) == 0) {
				header = paste("X-API-KEY: ", getHCSVLABKey(), sep="")
			} else {
				# add hcsvlab api key if header doesn't already contain key
				if(length(grep("X-API-KEY", header)) == 0) {
					header = c(header, paste("X-API-KEY: ", getHCSVLABKey(), sep=""))
				}
			}
			bFile <- getBinaryURL(listOfFiles[i], httpheader=header)
			writeBin(bFile, tmpFilename)
			listOfFiles[i] <- tmpFilename
			tmp <- c(tmp, tmpFilename)
		}
	}
	list("listOfFiles" = listOfFiles, "tmpFiles" = tmp)
}


##' deletes all files in the given list if they exist
##' @title deleteTempFiles
##' @param tmpFiles list of files to be deleted
'deleteTempFiles' <- function(tmpFiles) {
	for(i in 1:length(tmpFiles)) {
		if (file.exists(tmpFiles[i])) file.remove(tmpFiles[i])
	}
}


##' creates non-existing filename for given URI, by using basename of URI
##' and appending a number if already exists to ensure uniqueness 
##' @title createUniqueFilename
##' @param URI URI to create filename for
##' @return filename
'createUniqueFilename' <- function(URI) {
	file <- basename(URI)
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


##' Gets HCS vLab API key from local config file if it exists
##' @title getHCSVLABKey
'getHCSVLABKey' <- function() {
	key <- ""
	if(file.exists(file.path(Sys.getenv("HOME"), "hcsvlab.config"))) {
		key <- scan(file=file.path(Sys.getenv("HOME"), "hcsvlab.config"), what="character", quiet=TRUE)
	}
	if(file.exists(file.path(Sys.getenv("USERPROFILE"), "hcsvlab.config"))) {
		key <- scan(file=file.path(Sys.getenv("USERPROFILE"), "hcsvlab.config"), what="character", quiet=TRUE)
	}
	key
}
