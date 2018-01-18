## Test environments

* local OS X install, R 3.4.3
* ubuntu 14.04.5 LTS (on travis-ci), R 3.4.2
* Debian GNU/Linux (rocker/r-devel docker image), R Under development (unstable) (2017-12-12 r73891) -- "Unsuffered Consequences"
* win-builder (devel and release)

## R CMD check results

There were no ERRORs, WARNINGs or NOTEs.

## Downstream dependencies

There are currently no downstream dependencies for this package.

## Adressing CRAN maintainer comments about last submit:

* added single quotes to 'ASSP' and 'libassp' in both title and Description fields
* added web reference to 'libassp'
* unfortunately no citeable reference for the library is available
* fixed rchk issues using https://github.com/joshuaulrich/rchk-docker/ (both maacheck and bcheck files don't contain any issues any more)
