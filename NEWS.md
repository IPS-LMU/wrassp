# wrassp 1.0.0

## new features / performance tweaks / improvements

## bug fixes

* fixed broken UTF-8 support on in windows paths

# wrassp 0.1.9.9000

## new features / performance tweaks / improvements

* replaced ms with (the correct) seconds in `read.AsspDataObj` function doc.
* implemented `as_tibble` S3 method for AsspDataObjects

## bug fixes

* using return value of `fread` in labelobj.c
* removed `*(c++) == 0.0;` line that was causing [-Wunused-value] warning

# wrassp 0.1.8

## new features / performance tweaks / improvements

* exposing/using long instead of short windows function list
* added deprecation warning to the `wrassp_intro` vignette

## bug fixes

# wrassp 0.1.7

## bug fixes

* fixed GCC >= 8.0 [-Wformat-overflow=] significant warning in numdecim.c

# wrassp 0.1.6

## bug fixes

* fixed rchk issues
* adressed comments by cran maintainer about description files

# wrassp 0.1.5

## bug fixes

* added explicit (void **) type casts for `get` and `put` functions
* increased char array sizes for `sprintf` to combat compiler warnings of potential buffer overflow

# wrassp 0.1.4

## new features / performance tweaks / improvements

* added verbose parameter to sig. proc. functions
* added new package docs (?wrassp)
* updated wrassp_intro vignette to include ?wrassp reference
* improved documentation