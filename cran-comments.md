## Test environments
* local OS X install, R 3.4.0 Under development (unstable) (2016-05-01 r70566) -- "Unsuffered Consequences"
* ubuntu 12.04 (on travis-ci), R 3.2.5
* win-builder (devel and release)

## R CMD check results
There were no ERRORs or WARNINGs.

There was 1 NOTE:

* checking CRAN incoming feasibility ... NOTE
Maintainer: 'Raphael Winkelmann <raphael@phonetik.uni-muenchen.de>'

Possibly mis-spelled words in DESCRIPTION:
  ASSP (3:25) -> acronym for (A)dvanced (S)peech (S)ignal (P)rocessor
  Scheffers's (8:38) -> authors name
  formants (12:33) -> https://en.wikipedia.org/wiki/Formant
  libassp (8:50, 9:35) -> C lib name
  libassp's (15:5) -> C lib name

@ Possibly mis-spelled words: see comments after -> above

## Downstream dependencies
There are currently no downstream dependencies for this package.