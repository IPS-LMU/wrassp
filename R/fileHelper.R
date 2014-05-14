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
		if(substr(listOfFiles[i], 1, 4) == "http") {
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
			bFile <- getBinaryURL(listOfFiles[i], httpheader=header, .opts = list(ssl.verifypeer = FALSE))
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
	if(!file.exists(cache)) {
		file.create(cache)
	}
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


##' Removes requested file from the cache if it exists
##' @title removeItemFromCache
'removeItemFromCache' <- function(filename) {
	directory <- getCacheDirectory()
	if(file.exists(file.path(directory, filename))) {
		file.remove(file.path(directory, filename))
		con  <- file(file.path(directory, "cache_contents"), open = "r")
		files <- c()
		while (length(oneLine <- readLines(con, n = 1, warn = FALSE)) > 0) {
			if(length(grep(filename, oneLine)) == 0) {
				files <- c(files, oneLine)
			}
		}
		close(con)
		write(files, file=file.path(directory, "cache_contents"), sep="\n")
	}
	else {
		stop("File not found in cache")
	}
}


##' Gets HCS vLab API key from local config file if it exists
##' @title getHCSVLABKey
'getHCSVLABKey' <- function() {
	apikey <- ""
	home <- getHomeDirectory()
	if(file.exists(file.path(home, "hcsvlab.config"))) {
		config <- rjson::fromJSON(file = file.path(home, "hcsvlab.config"))
		return(config$apiKey)
	}
	apikey
}


##' Gets default directory for cache from local config file if it exists
##' @title getCacheDirectory
'getCacheDirectory' <- function() {
	home <- getHomeDirectory()
	current_dir <- getwd()
	setwd(home)
	cacheDir <- NULL
	if(file.exists(file.path(home, "hcsvlab.config"))) {
		config <- rjson::fromJSON(file = file.path(home, "hcsvlab.config"))
		cacheDir <- config$cacheDir
	}
	if(!is.null(cacheDir) && file.exists(cacheDir)) {
			cacheDir <- normalizePath(cacheDir)
	}
	else if(!is.null(cacheDir) && !file.exists(cacheDir)) {
		# R in Windows strangely can't handle directory paths with trailing slashes
		if(substr(cacheDir, nchar(cacheDir), nchar(cacheDir)+1) == "/") {
			cacheDir <- substr(cacheDir, 1, nchar(cacheDir)-1)
		}
		dir.create(cacheDir)
		cacheDir <- normalizePath(cacheDir)
	}
	else {
		if(!file.exists(file.path(home, "wrassp_cache"))) {
			dir.create(file.path(home, "wrassp_cache"))
		}
		cacheDir <- file.path(home, "wrassp_cache")
	}
	setwd(current_dir)
	return(cacheDir)
}	


##' Gets home directory for machine. Depending on OS this could be found 
##' in HOME or USERPROFILE environment variables
##' @title getHomeDirectory
'getHomeDirectory' <- function() {
	if(file.exists(Sys.getenv("HOME"))) {
		Sys.getenv("HOME")
	}
	else {
		stop("Make sure your $HOME environment variable is set")
	}
}