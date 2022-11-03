# wrassp

[![Build Status](https://travis-ci.org/IPS-LMU/wrassp.svg?branch=master)](https://travis-ci.org/IPS-LMU/wrassp)
[![Coverage Status](https://coveralls.io/repos/IPS-LMU/wrassp/badge.svg)](https://coveralls.io/github/IPS-LMU/wrassp)
[![CRAN_Status_Badge](https://www.r-pkg.org/badges/version/wrassp)](https://CRAN.R-project.org/package=wrassp)

## Out of funding

Unfortunately, the EMU-SDMS is currently out of funding.

We at the IPS will do what we can to fix bugs, security issues or necessary adjustments to new versions of R; but we cannot currently work on new features or performance improvements.

We would be very glad if funding in academia allowed for more technical staff to maintain software used by the research community.

## Introduction

`wrassp` is a wrapper for R around Michel Scheffers's [libassp](https://libassp.sourceforge.net/)
(Advanced Speech Signal Processor). The libassp library aims at providing functionality for handling speech signal files in most common audio formats and for performing analyses common in phonetic science/speech science. This includes the calculation of formants, fundamental frequency, root mean square, auto correlation, a variety of spectral analyses, zero crossing rate, filtering etc. This wrapper provides R with a large subset of libassp's signal processing functions and provides them to the user in a (hopefully) user-friendly manner.

This package is part of the next iteration of the EMU Speech Database Management System which aims to be as close to an all-in-one solution for generating, manipulating, querying, analyzing and managing speech databases as possible. For an overview of the system please visit this URL: [https://ips-lmu.github.io/EMU.html](https://ips-lmu.github.io/EMU.html).

## Installation

* install the current [CRAN release](https://CRAN.R-project.org/package=wrassp):
```r
install.packages("wrassp")
```

* or install the latest development version from GitHub (as large parts of `wrassp` are written in `C` make sure your system fulfills the requirements for package development (see [here](https://support.posit.co/hc/en-us/articles/200486498-Package-Development-Prerequisites))):
```r
library(devtools)
install_github("IPS-LMU/wrassp", build_vignettes = TRUE)
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

+ `acfana()`: Analysis of short-term autocorrelation function
+ `afdiff()`: Computes the first difference of the signal
+ `affilter()`: Filters the audio signal (see docs for types)
+ `cepstrum()`: Short-term cepstral analysis
+ `cssSpectrum()`: Cepstral smoothed version of `dftSpectrum()`
+ `dftSpectrum()`: Short-term DFT spectral analysis
+ `forest()`: Formant estimation
+ `ksvF0()`: F0 analysis of the signal
+ `lpsSpectrum()`: Linear Predictive smoothed version of `dftSpectrum()`
+ `mhsF0()`: Pitch analysis of the speech signal using Michel's/Modified Harmonic Sieve algorithm
+ `rfcana()`: Linear Prediction analysis
+ `rmsana()`: Analysis of short-term Root Mean Square amplitude
+ `zcrana()`: Analysis of the averages of the short-term positive and negative zero-crossing rates

(see the respective R documentation for more details on all of these functions)

## Available file handling functions

+ `read.AsspDataObj()`: read an existing SSFF file into a `AsspDataObj` which is its in-memory equivalent.
+ `write.AsspDataObj()`: write a `AsspDataObj` out to a SSFF file.

## For Developers

Prerequisite: docker is installed on your machine

### Build and check package using rocker/r-devel docker image

- pull current r-devel image: `docker pull rocker/r-devel`
- check if pull worked: `docker images`
- check R version in image: `docker run --rm rocker/r-devel:latest R --version`
- run the container with an interactive shell, mounting wrassp project folder (==current directory) and a named docker volume for the output tarball:
  `docker run --rm -ti -v $(pwd):/wrassp -v wrassp_packages:/output rocker/r-devel:latest bash`

In the interactive shell you just started:

- manually install OS deps (this might need a bit of tweaking): `apt update && apt install --yes pandoc tidy qpdf`
- manually install R deps (this might need a bit of tweaking): `RD -e 'install.packages(c("tibble","compare", "rmarkdown", "knitr", "testthat"))'`
- build: `RD CMD build --resave-data wrassp`
- check: `RD CMD check --as-cran wrassp_*.tar.gz`
- copy built package to the named docker volume so it can be retrieved from outside this container: `cp wrassp_*.tar.gz /output`

### Additional checks using kalibera/rchk

- pull current rchk image: `docker pull kalibera/rchk`
- run the checks:
  `docker run --rm -v wrassp_packages:/rchk/packages kalibera/rchk:latest /rchk/packages/wrassp_x.y.z.tar.gz`
- rchk results are printed to stdout and also stored in `libsonly/wrassp/libs/wrassp.so{maa|b|ffi}check` on the named docker volume
- see also: https://github.com/kalibera/rchk/blob/master/doc/DOCKER.md (Checking a package from a tarball)


## Authors

**Raphael Winkelmann**

+ [github](https://github.com/raphywink)

**Lasse Bombien**

+ [github](https://github.com/quabolasse)

**Markus Jochim** (current maintainer)

+ [github](https://github.com/MJochim)

**Affiliations**

[INSTITUTE OF PHONETICS AND SPEECH PROCESSING](https://www.en.phonetik.uni-muenchen.de)
