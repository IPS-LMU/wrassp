## Test environments

* local OS X install, R 3.5.1
* ubuntu 14.04.5 LTS (on travis-ci), R 3.5.0
* win-builder (devel and release)

## R CMD check results

There were no ERRORs, WARNINGs or NOTEs.

## Downstream dependencies

There are currently no downstream dependencies for this package.

## Additional comments

* Usage of `#ifdef __unix__` was replaced by `#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))` as requested by CRAN maintainer