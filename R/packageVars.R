##' package variable to force the usage of the logger
##' set to FALSE by defaul
##' @author Raphael Winkelmann
##' @export
useWrasspLogger <- FALSE



##' list of default output extensions,
##' track names and output type 
##' for each signal processing function in wrassp
##' @author Raphael Winkelmann
##' @export
wrasspOutputInfos = list("acfana" = list("ext"= c("acf"), "tracks"=c("acf"), "outputType"="SSFF"),
                         "afdiff" = list("ext"= c("dwav"), "tracks"=c(""), "outputType"="wav"),
                         "affilter" = list("ext"= c("hpf", "lpf", "bpf", "bsf"), "tracks"=c(""), "outputType"="wav"),
                         "cepstrum" = list("ext"= c("cep"), "tracks"=c("cep"), "outputType"="SSFF"),
                         "cssSpectrum" = list("ext"= c("css"), "tracks"=c("css"), "outputType"="SSFF"),
                         "dftSpectrum" = list("ext"= c("dft"), "tracks"=c("dft"), "outputType"="SSFF"),
                         "f0_ksv" = list("ext"= c("f0"), "tracks"=c("F0"), "outputType"="SSFF"),
                         "f0_mhs" = list("ext"= c("pit"), "tracks"=c("pitch"), "outputType"="SSFF"),
                         "forest" = list("ext"= c("fms"), "tracks"=c("fm", "bw"), "outputType"="SSFF"),
                         "lpsSpectrum" = list("ext"= c("lps"), "tracks"=c("lps"), "outputType"="SSFF"),
                         "rfcana" = list("ext"= c("arf", "lar", "lpc", "rfc"), "tracks"=c("rms", "gain", "arf|lar|lpc|rfc"), "outputType"="SSFF"),
                         "rmsana" = list("ext"= c("rms"), "tracks"=c("rms"), "outputType"="SSFF"),
                         "zcrana" = list("ext"= c("zcr"), "tracks"=c("zcr"), "outputType"="SSFF")
                         )
