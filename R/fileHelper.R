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
'prepareFiles' <- function(listOfFiles = NULL, header = NULL) {
	directory <- getCacheDirectory()
	for(i in 1:length(listOfFiles)) {
		if(substr(listOfFiles[i], 1, 7) == "http://") {
			tmpFilename <- paste(digest(listOfFiles[i], "md5"), ".", file_ext(listOfFiles[i]), sep="")
			if(file.exists(file.path(directory, tmpFilename))) {
				listOfFiles[i] <- file.path(directory, tmpFilename)
				next
			}
			if (is.null(header) || nchar(header) == 0) {
				header <- paste("X-API-KEY: ", getHCSVLABKey(), sep="")
			} else {
				# add hcsvlab api key if header doesn't already contain key
				if(length(grep("X-API-KEY", header)) == 0) {
					header <- c(header, paste("X-API-KEY: ", getHCSVLABKey(), sep=""))
				}
			}
			bFile <- getBinaryURL(listOfFiles[i], httpheader=header)
			writeCacheListing(listOfFiles[i], tmpFilename)
			writeBin(bFile, file.path(directory, tmpFilename))
			listOfFiles[i] <- file.path(directory, tmpFilename)
		}
	}
	listOfFiles
}


##' Records the uri to filename mapping to the cache contents file
##' @title writeCacheListing
'writeCacheListing' <- function(uri, filename) {
	directory <- getCacheDirectory()
	write(paste(uri, filename, sep=" -> "), file=file.path(directory, "cache_contents"), append=TRUE, sep="\n")
}


##' Opens the cache contents file to display list of files in the cache with their mappings
##' 
##' @return A file containing the list of files in the cache
##' @title viewCache
'viewCache' <- function() {
	directory <- getCacheDirectory()
	cache <- file.path(directory, "cache_contents")
	file.show(cache)
}


##' Deletes all the files currently in the cache directory
##' @title emptyCache
'emptyCache' <- function() {
	directory <- getCacheDirectory()
	file.remove(file.path(directory, list.files(directory)))
	file.create(file.path(directory, "cache_contents"))
	return(TRUE)
}


##' Gets HCS vLab API key from local config file if it exists
##' @title getHCSVLABKey
'getHCSVLABKey' <- function() {
	apiKey <- ""
	home <- getHomeDirectory()
	if(file.exists(file.path(home, "hcsvlab.config"))) {
		source(file.path(home, "hcsvlab.config"))
	}
	apiKey
}


##' Gets default directory for cache from local config file if it exists
##' @title getCacheDirectory
'getCacheDirectory' <- function() {
	home <- getHomeDirectory()
	if(file.exists(file.path(home, "hcsvlab.config"))) {
		source(file.path(home, "hcsvlab.config"))
	}
	if(file.exists(cacheDir)) {
		cacheDir
	}
	else {
		if(!file.exists(file.path(home, "wrassp_cache"))) {
			dir.create(file.path(home, "wrassp_cache"))
		}
		file.path(home, "wrassp_cache")
	}
}


##' Gets home directory for machine. Depending on OS this could be found 
##' in HOME or USERPROFILE environment variables
##' @title getHomeDirectory
'getHomeDirectory' <- function() {
	if(file.exists(Sys.getenv("HOME"))) {
		Sys.getenv("HOME")
	}
	else if(file.exists(Sys.getenv("USERPROFILE"))) {
		Sys.getenv("USERPROFILE")
	}
	else {
		stop("Make sure your $HOME environment variable is set")
	}
}