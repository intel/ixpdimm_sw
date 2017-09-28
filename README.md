# ixpdimm_sw

ixpdimm_sw contains the following interface components:

An Application Program Interface (API) library which provides programatic access to
the IXPDIMM SW functionality.

A Command Line Interface (CLI) application for configuring and managing IXPDIMMs from the
command line.

A Common Information Model (CIM) provider library to expose the
IXPDIMM SW functionality as standard CIM objects to plug-in to
common information model object managers (CIMOMs).

A monitor daemon/system service for monitoring the health and status of IXPDIMMs.

For more information please visit our project home. https://01.org/ixpdimm-sw/

## Build Linux

Kernel 4.12 or newer is suggested

libndctl is required to build, an rpm can be found at:

https://copr.fedoraproject.org/coprs/djbw/ndctl/

and the source can be found at:

https://github.com/pmem/ndctl

libinvm-cim, libinvm-cli, libinvm-il8n libraries are required and included as a submodule.
The source can be found at:

https://github.com/01org/invm-frameworks

All other dependencies are widely available. This includes ctemplate openssl sqlite and zlib.

```
git submodule init
git submodule update
mkdir output && cd output
cmake .. -DRELEASE=ON
make -j all
sudo make install
```
build artifacts can be found in output/build/linux/real/release

RPMs can also be built:
```
cmake ..
make rpm
```
The RPMs will be in output/rpmbuild/RPMS/

## Build Windows

WIP
