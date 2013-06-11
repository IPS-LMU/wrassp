# wrassp

wrassp is a wrapper to Michel Scheffer's [libassp](http://libassp.sourceforge.net/) (Advanced Speech Signal Processor). The libassp library aims at providing functionality for handling speech signal files in most common audio formats and for performing analyses common in phonetic science/speech science. This includes the calculation of formants, fundamental frequency, root mean square, auto correlation, a variety of spectral analyses, zero crossing rate, filtering etc. This wrapper library for R exposes a large subset of the signal processing functions to R in a (hopefully) user friendly manner.


## Quick start
 
* Download then install the package with: `install.packages("path/to/wrassp", repos = NULL, type="source")`

* load the library: `library("wrassp")`

* calculate formants from audio file: `res=forest("path/to/audio.wav", ToFile=FALSE)`

* plot the first 100 F1 values over time: `plot(res$fm[0:99,1],type='l')`

## Available signal processing functions

+ `acfana()` = Analysis of short-term autocorrelation function
+ `afdiff()` = Computes the first difference of the signal
+ `affilter()` = Filters the audio signal (see docs for types)
+ `f0_ksv()` = F0 analysis of the signal
+ `f0_mhs()` = Pitch analysis of the speech signal using Michel's/Modified Harmonic Sieve algorithm
+ `forest()` = Formant estimation
+ `rfcana()` = Linear Prediction analysis
+ `rmsana()` = Analysis of short-term Root Mean Square amplitude
+ `spectrum()` = Short-term spectral analysis (see docs for types)
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