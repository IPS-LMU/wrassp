## Test environments

* local Arch Linux install, gcc, R 4.2.2
* local Docker container rocker/r-devel, gcc, R Under development (unstable) (2023-02-05 r83767) -- "Unsuffered Consequences"
* R-hub builder:
  * Windows Server 2022, R-devel, 64 bit
  * Ubuntu Linux 20.04.1 LTS, R-release, GCC
  * Fedora Linux, R-devel, clang, gfortran
  * Debian Linux, R-devel, GCC ASAN/UBSAN
* win-builder devel

## R CMD check results (Arch Linux, R-hub Ubuntu, R-hub Debian)

There were no ERRORs, WARNINGs or NOTEs.

## R CMD check results (rocker/r-devel)

There was 1 NOTE mentioning the container’s use of non-portable compiler flags:

```
* checking compilation flags used ... NOTE
Compilation used the following non-portable flag(s):
  ‘-Wdate-time’ ‘-Werror=format-security’ ‘-Wformat’
```

## R CMD check results (R-hub Windows)

There was 1 NOTE mentioning left-over files in the temp directory:

```
* checking for detritus in the temp directory ... NOTE
Found the following files/directories:
  'lastMiKTeXException'
```

## R CMD check results (R-hub Fedora)

There was 1 NOTE:

```
* checking HTML version of manual ... NOTE
Skipping checking HTML validation: no command 'tidy' found
```

## R CMD check results (win-builder)

There was 1 NOTE that highlighted a possibly invalid URL:

https://support.posit.co/hc/en-us/articles/200486498-Package-Development-Prerequisites

This URL is valid and active, but win-builder gets a status 403 because the CDN
Cloudflare correctly thinks that win-builder is not a human.

## Downstream dependencies

The revdepcheck::revdep_check() function revealed no problems.
