# wrassp

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

Prerequisite: Docker is installed on your machine

### Build and check package using Docker image rocker/r-devel

- Pull current r-devel image: `docker pull rocker/r-devel`
- Check if pull worked: `docker images`
- Check R version in image: `docker run --rm rocker/r-devel:latest R --version`
- Check R-devel version in image: `docker run --rm rocker/r-devel:latest RD --version`
- Run a container with an interactive shell, mounting the wrassp project folder (that is, the current directory) and a named docker volume for the output tarball:
  `docker run --name wrassp-checks -ti -v $(pwd):/source -v r_packages:/output rocker/r-devel:latest bash`

In the interactive shell you just started:

- Manually install OS dependencies (this might need a bit of tweaking): `apt update && apt install --yes pandoc tidy qpdf`
- Manually install R dependencies (this might also need a bit of tweaking): `RD -e "install.packages(c('tibble','compare', 'rmarkdown', 'knitr', 'testthat'))"`
- Build: `RD CMD build --resave-data source/`
- Check: `RD CMD check --as-cran wrassp_*.tar.gz`
- Copy the built package to the named docker volume, so it can be shared with other containers that might, for example, run additional checks: `cp wrassp_*.tar.gz /output`


## Authors

Development of the R package and ongoing maintenance of the ASSP library in this repository:

[Institute for Phonetics and Speech Processing, Ludwig-Maximilians-Universität München](https://www.en.phonetik.uni-muenchen.de)

- **Markus Jochim** (current maintainer) [[GitHub](https://github.com/MJochim)]
- **Raphael Winkelmann** [[GitHub](https://github.com/raphywink)]
- **Lasse Bombien** [[GitHub](https://github.com/quabolasse)]

Development of the original ASSP library:

[Christian-Albrechts-Universität zu Kiel](https://www.isfas.uni-kiel.de/en/general-linguistics-and-phonetics?set_language=en)

- **Michel Scheffers**
