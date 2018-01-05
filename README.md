# ixpdimm_sw

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
* A Common Information Model (CIM) provider library to expose the IXPDIMM SW functionality as standard CIM objects to plug-in to common information model object managers (CIMOMs).
* A Command Line Interface (CLI)  application for configuring and managing Intel DIMMs from the command line.
* A monitor daemon/system service for monitoring the health and status of Intel DIMMs.

Packages are availible on Fedora.

Fedora and Centos rpms can also be found: https://copr.fedorainfracloud.org/coprs/jhli/ixpdimm_sw/

For more information please visit our project home: https://01.org/intel-nvm-frameworks


## Build

### Linux

Kernel 4.12 or newer is suggested

libndctl is required to build, an rpm can be found at: https://copr.fedoraproject.org/coprs/djbw/ndctl/

The source can be found at: https://github.com/pmem/ndctl

invm-frameworks is required and included as a submodule.

The source can be found at: https://github.com/intel/invm-frameworks

All other dependencies are widely available. This includes ctemplate openssl sqlite and zlib.

```
git submodule init
git submodule update
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
