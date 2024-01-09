## Test environments

* win-builder:
  * R Under development (unstable) (2024-01-07 r85787 ucrt)
  * R version 4.3.2 (2023-10-31 ucrt)
* R-hub builder:
  * Windows Server 2022, R-devel, 64 bit
  * Ubuntu Linux 20.04.1 LTS, R-release, GCC
  * Fedora Linux, R-devel, clang, gfortran
  * FIXME Debian Linux, R-devel, GCC ASAN/UBSAN


## R CMD check results

Across all environments, there were no ERRORs or WARNINGs, but some NOTEs.

The NOTEs were the following:

Two NOTEs mentioning left-over files:

```
* checking for detritus in the temp directory ... NOTE
Found the following files/directories:
  'lastMiKTeXException'

* checking for non-standard things in the check directory ... NOTE
Found the following files/directories:
  ''NULL''
```

One NOTE about a possibly invalid URL. This NOTE is a false positive, the URL is
valid and online when opened in a browser:

```
Found the following (possibly) invalid URLs:
  URL: https://support.posit.co/hc/en-us/articles/200486498-Package-Development-Prerequisites
    From: README.md
    Status: 403
    Message: Forbidden
```

One NOTE about a missing tidy command:

```
* checking HTML version of manual ... NOTE
Skipping checking HTML validation: no command 'tidy' found
```
