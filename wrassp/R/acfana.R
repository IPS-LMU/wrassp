'acfana' <- function(listOfFiles=NULL,
					startTime = -1.0,
					endTime = -1.0,
					msSize = 20.0,
					msShift = 5.0,
					order = 0,
					channel = 1,
					accuracy = 14,
					winFunc = "BLACKMAN",
					ACF_OPT_MEAN = FALSE, # "0x0001" 
					ACF_OPT_NORM = FALSE, #"0x0002"
					toFile = TRUE
				)
{
	if(is.null(x)){
		stop("listOfFiles as the paths of the files to perform the analysis on has to be set. A string or a vektor of paths has to be given to the function")
		
	}
	
	# a few parameter checks
	if (round(order) != order){ #strange check if int number seeing as 
		stop("order parameter is not allowed to be a decimal value")
	}
	
	winT = c("RECTANGE","PARABOLA","COS","HANN","COS_4","HAMMING")# AsspWindowTypes
	
	isValidWindow = FALSE
	
	for(type in winT){
		if (winFunc==type){
			isValidWindow = TRUE
			break
		}
	}
	
	if(!isValidWindow){stop("winfunc parameter ist not a valid window type! Call function AsspWindowTypes for a vector of valid window types.")}
	
	
	.External("performAssp", 
			  listOfFiles, 
			  fname="acfana", 
			  startTime=startTime, 
			  endTime=endTime, 
			  msSize=msSize, 
			  msShift=msShift, 
			  order=order,
			  channel=channel,
			  accuracy=accuracy,
			  winFunc=winFunc,
			  ACF_OPT_MEAN = ACF_OPT_MEAN,
			  ACF_OPT_NORM = ACF_OPT_NORM,
			  toFile=toFile)



		
					
}