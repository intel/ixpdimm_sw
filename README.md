# ixpdimm_sw [deprecated]

### This project has been replaced by ipmctl located at https://github.com/intel/ipmctl. This repo is not active and pull requests are no longer being accepted.

IXPDIMM SW is application-level software for configuring and managing Intel DIMMs.
It supports functionality to:

Discover Intel DIMMs.
* View and update the firmware on Intel DIMMs.
* Provision the platform memory configuration.
* Create and delete namespaces.
* Configure data-at-rest security on Intel DIMMs.
* Monitor Intel DIMM health.
* Track performance of Intel DIMMs.
* Debug and troubleshoot Intel DIMMs.

IXPDIMM SW refers to the following interface components:

* An Application Programming Interface (API) library which provides programmatic access to the IXPDMIM SW functionality.
* A Command Line Interface (CLI)  application for configuring and managing Intel DIMMs from the command line.
* A monitor daemon/system service for monitoring the health and status of Intel DIMMs.

Packages are available on Fedora.

Fedora and Epel 7 packages can be found at: https://copr.fedorainfracloud.org/coprs/jhli/ixpdimm_sw/

For more information please visit our project home: https://01.org/intel-nvm-frameworks


## Build

### Linux

Kernel 4.12 or newer is suggested

libndctl is required to build, packages can be found at: https://copr.fedoraproject.org/coprs/djbw/ndctl/

The source can be found at: https://github.com/pmem/ndctl

invm-frameworks is required to build, packages can be found at: https://copr.fedorainfracloud.org/coprs/jhli/ixpdimm_sw/

The source can be found at: https://github.com/intel/invm-frameworks

All other dependencies are widely available.
This includes openssl sqlite zlib numactl kmod sblim-cmpi.

```
mkdir output && cd output
cmake -DRELEASE=ON -DCMAKE_INSTALL_PREFIX=/usr ..
make -j all
sudo make install
```
build artifacts can be found in output/build/linux/real/release

RPMs can also be built:
```
make rpm
```
The RPMs will be in output/rpmbuild/RPMS/

## Build Windows

WIP
