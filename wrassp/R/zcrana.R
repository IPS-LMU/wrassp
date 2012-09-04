"zcrana" <- function(listOfFiles = NULL, BeginTime = 0.0, CenterTime = FALSE, EndTime = 0.0, WindowShift = 5.0, WindowSize = 25.0, ,ToFile = TRUE, ExplicitExt = NULL) {


	.External("performAssp", listOfFiles, fname = "zcrana", BeginTime = BeginTime, CenterTime = CenterTime, EndTime = EndTime, WindowShift = WindowShift, WindowSize = WindowSize, ToFile = ToFile, ExplicitExt = ExplicitExt)



}