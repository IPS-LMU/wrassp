# wrassp

[![Build Status](https://travis-ci.org/IPS-LMU/wrassp.svg?branch=master)](https://travis-ci.org/IPS-LMU/wrassp)

`wrassp` is a wrapper for R around Michel Scheffers's [libassp](http://libassp.sourceforge.net/)
(Advanced Speech Signal Processor). The libassp library aims at providing functionality for handling speech signal files in most common audio formats and for performing analyses common in phonetic science/speech science. This includes the calculation of formants, fundamental frequency, root mean square, auto correlation, a variety of spectral analyses, zero crossing rate, filtering etc. This wrapper provides R with a large subset of libassp's signal processing functions and provides them to the user in a (hopefully) user-friendly manner.


## Installation

* as large parts of `wrassp` are written in `C` make sure your system fulfills the requirements for package development (see [here](http://www.rstudio.com/ide/docs/packages/prerequisites))

* Download & extract the package from GitHub. Then install it with the following command:
```r
install.packages("path/to/wrassp", repos = NULL, type="source")
```

* or install the latest development version from GitHub (**preferred method**):
```r
library(devtools)
install_github("IPS-LMU/wrassp")
```

## Quick start

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
res=forest(path2wav, toFile=FALSE)
```

* plot the first 100 F1 values over time: 
```r
plot(res$fm[1:100,1],type='l')
```

* for more information see the `An introduction to the wraspp package` vignette: 
```r
vignette('wrassp_intro')
```


## Available signal processing functions

+ `acfana()` = Analysis of short-term autocorrelation function
+ `afdiff()` = Computes the first difference of the signal
+ `affilter()` = Filters the audio signal (see docs for types)
+ `cepstrum()` = Short-term cepstral analysis
+ `cssSpectrum()` = Cepstral smoothed version of `dftSpectrum()`
+ `dftSpectrum()` = Short-term DFT spectral analysis
+ `forest()` = Formant estimation
+ `ksvF0()` = F0 analysis of the signal
+ `lpsSpectrum()` = Linear Predictive smoothed version of `dftSpectrum()`
+ `mhsF0()` = Pitch analysis of the speech signal using Michel's/Modified Harmonic Sieve algorithm
+ `rfcana()` = Linear Prediction analysis
+ `rmsana()` = Analysis of short-term Root Mean Square amplitude
+ `zcrana()` = Analysis of the averages of the short-term positive and negative zero-crossing rates

(see the respective R documentation for more details on all of these functions)

## Available file handling functions

+ `read.AsspDataObj()` which reads an existing SSFF file into a `AsspDataObj` which is its in-memory equivalent.
+ `write.AsspDataObj()` which writes a `AsspDataObj` out to a SSFF file.

## Authors

**Raphael Winkelmann**

+ [github](http://github.com/raphywink)

**Lasse Bombien**

+ [github](http://github.com/quabolasse)


**Affiliations**

[INSTITUTE OF PHONETICS AND SPEECH PROCESSING](http://www.en.phonetik.uni-muenchen.de/)
