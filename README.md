[![CI status](https://github.com/eic/EICrecon/actions/workflows/linux-eic-shell.yml/badge.svg)](https://github.com/eic/EICrecon/actions/workflows/linux-eic-shell.yml)
[![DOI](https://zenodo.org/badge/512187504.svg)](https://zenodo.org/badge/latestdoi/512187504)

# EICrecon
JANA based reconstruction for the EPIC detector

The repository contains reconstruction source code for the EPIC detector. It's function
is to take real or simulated data and reconstruct it into physical values
for later analysis.

- [Documentation website](https://eic.github.io/EICrecon/#/)
- [Getting started instruction with eic-shell](https://eic.github.io/EICrecon/#/get-started/eic-shell)


If you are new to EIC software then you should start with the tutorials which
can be found here:
[https://indico.bnl.gov/category/443/](https://indico.bnl.gov/category/443/)

A tutorial on JANA2 is available 
[here](https://eic.github.io/EICrecon/#/tutorial/01-introduction)

### Compilation

To configure, build, and install the geometry (to the `install` directory), use the following commands:
```bash
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=install
cmake --build build
cmake --install build
```
To load the geometry, you can use the scripts in the `install` directory:
```bash
source install/bin/eicrecon-this.sh
```
