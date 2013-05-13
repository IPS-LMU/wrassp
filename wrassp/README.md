# wrassp

wrassp is a wrapper to Michel Scheffer's [libassp](http://libassp.sourceforge.net/) (Advanced Speech Signal Processor). The libassp library aims at providing functionality for handling speech signal files in most common audio formats and for performing analyses common in phonetic science/speech science. This includes the calculation of formants, fundamental frequency, root mean square, auto correlation, a variety of spectral analyses, zero crossing rate, filtering etc. This wrapper library for R exposes a large subset of the signal processing functions to R in a (hopefully) user friendly manor.


## Quick start
 
* Download and install the package: `install.packages("wrassp")`

* load the library: `library("wrassp")`

* calculate formants from audio file: `res=forest("path/to/audio.wav", ToFile=FALSE)`

* plot the first 100 F1 values over time: `plot(res$fm[0:99,1],type='l')`

## Available signal processing functions

+ `acfana()` 
+ `afdiff()`
+ `affilter()` 
+ `f0_ksv()`
+ `f0_mhs()`
+ `forest()`
+ `rfcana()`
+ `rmsana()`
+ `spectrum()`
+ `zcrana()`

(see the R documentation of these functions for more details)

## Available file handling functions

+ `read.AsspDataObj()` which reads and existing SSFF file into a `AsspDataObj` which is its in-memory equivalent.
+ `write.AsspDataObj()` which writes a `AsspDataObj` out to a SSFF file.

## Authors

**Raphael Winkelmann**

+ [email](raphael@phonetik.uni-muenchen.de)
+ [github](http://github.com/raphywink)

**Lasse Bombien**

+ [email](lasse@phonetik.uni-muenchen.de)
+ [github](http://github.com/lassesGitUserName)


**Affiliations**

[INSTITUTE OF PHONETICS AND SPEECH PROCESSING](http://www.phonetik.uni-muenchen.de/)