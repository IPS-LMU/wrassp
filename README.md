# wrassp

wrassp is a wrapper to Michel Scheffer's [libassp](http://libassp.sourceforge.net/) (Advanced Speech Signal Processor). The libassp library aims at providing functionality for handling speech signal files in most common audio formats and for performing analyses common in phonetic science/speech science. This includes the calculation of formants, fundamental frequency, root mean square, auto correlation, a variety of spectral analyses, zero crossing rate, filtering etc. This wrapper library for R exposes a large subset of the signal processing functions to R in a (hopefully) user friendly manner.


## Quick start

* as large parts of `wrassp` are written in `C` make sure your system fulfills the requirements for package development (see [here](http://www.rstudio.com/ide/docs/packages/prerequisites))

* Download then install the package with: 
```r
install.packages("path/to/wrassp", repos = NULL, type="source")
```

* or install the latest development version from GitHub with:
```r
library(devtools)
install_github("wrassp", "IPS-LMU")
```

* load the library: 
```r
library("wrassp")
```

* get path to an audio file: 
```r
path2wav <- list.files(system.file("extdata", package = "wrassp"), pattern = glob2rx("*.wav"), full.names = TRUE)[1]
```

* calculate formants from audio file: 
```r
res=forest(path2wav, ToFile=FALSE)
```

* plot the first 100 F1 values over time: 
```r
plot(res$fm[0:99,1],type='l')
```

## Available signal processing functions

+ `acfana()` = Analysis of short-term autocorrelation function
+ `afdiff()` = Computes the first difference of the signal
+ `affilter()` = Filters the audio signal (see docs for types)
+ `ksvF0()` = F0 analysis of the signal
+ `mhsF0()` = Pitch analysis of the speech signal using Michel's/Modified Harmonic Sieve algorithm
+ `forest()` = Formant estimation
+ `rfcana()` = Linear Prediction analysis
+ `rmsana()` = Analysis of short-term Root Mean Square amplitude
+ `dftSpectrum()` = Short-term DFT spectral analysis
+ `lpsSpectrum()` = Linear Predictive smoothed version of `dftSpectrum()`
+ `cssSpectrum()` = Cepstral smoothed version of `dftSpectrum()`
+ `cepstrum()` = Short-term cepstral analysis
+ `zcrana()` = Analysis of the averages of the short-term positive and negative zero-crossing rates

(see the respective R documentation for more details on all of these functions)

## Available file handling functions

+ `read.AsspDataObj()` which reads and existing SSFF file into a `AsspDataObj` which is its in-memory equivalent.
+ `write.AsspDataObj()` which writes a `AsspDataObj` out to a SSFF file.

## Authors

**Raphael Winkelmann**

+ [github](http://github.com/raphywink)

**Lasse Bombien**

+ [github](http://github.com/quabolasse)


**Affiliations**

[INSTITUTE OF PHONETICS AND SPEECH PROCESSING](http://www.phonetik.uni-muenchen.de/)