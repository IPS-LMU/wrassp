## Change of maintainership

Maintainership of this package is being handed over from Raphael Winkelmann to
Markus Jochim. Raphael is still reachable at the below email address and he will
confirm the mutual intention.

* Raphael Winkelmann <raphael@phonetik.uni-muenchen.de>
* Markus Jochim <markusjochim@phonetik.uni-muenchen.de>

## Test environments

* local Arch Linux install, gcc, R 4.2.2
* local Docker container rocker/r-devel, clang, R Under development (unstable) (2022-11-06 r83294) -- "Unsuffered Consequences"
* R-hub builder:
  * Windows Server 2022, R-devel, 64 bit
  * Ubuntu Linux 20.04.1 LTS, R-release, GCC
  * Fedora Linux, R-devel, clang, gfortran
  * Debian Linux, R-devel, GCC ASAN/UBSAN
* win-builder (devel), R Under development (unstable) (2022-10-11 r83083 ucrt) -- "Unsuffered Consequences"

## R CMD check results (Arch Linux, rocker/r-devel, R-Hub Windows, R-Hub Debian)

There were no ERRORs, WARNINGs or NOTEs.

## R CMD check results (R-hub Ubuntu)

There was 1 NOTE that highlighted the change of maintainership.

## R CMD check results (R-hub Fedora)

There were 2 NOTEs. One that highlighted the change of maintainership and this one:

* checking HTML version of manual ... NOTE
Skipping checking HTML validation: no command 'tidy' found

### win-builder

There was 1 NOTE that highlighted both the change of maintainership and one
possibly invalid URL:

https://support.posit.co/hc/en-us/articles/200486498-Package-Development-Prerequisites

This URL is valid and active, but win-builder gets a status 403 because the CDN
Cloudflare correctly thinks that win-builder is not a human.

## Downstream dependencies

The revdepcheck::revdep_check() function revealed no problems.
