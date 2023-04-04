## Test environments

* local Arch Linux install, gcc, R 4.2.3
* local Docker container rocker/r-devel, gcc, R Under development (unstable) (2023-04-02 r84146) -- "Unsuffered Consequences"
  * compiling wrassp with gcc
  * compiling wrassp with clang
* R-hub builder:
  * Windows Server 2022, R-devel, 64 bit
  * Ubuntu Linux 20.04.1 LTS, R-release, GCC
  * Fedora Linux, R-devel, clang, gfortran
  * Debian Linux, R-devel, GCC ASAN/UBSAN

## R CMD check results (Arch Linux, rocker/r-devel, R-hub Ubuntu, R-hub Debian)

There were no ERRORs, WARNINGs or NOTEs.

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

## Downstream dependencies (revdepcheck results)

We checked 3 reverse dependencies, comparing R CMD check results across CRAN and dev versions of this package.

 * We saw 0 new problems
 * We failed to check 0 packages
