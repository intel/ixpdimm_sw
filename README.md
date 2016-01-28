#IXPDIMM_SW

IXPDIMM_SW contains the following

A command line interface (CLI) application for configuring and 
managing IXPDIMM. Including commands for basic inventory,
capacity provisioning, health monitoring, and troubleshooting.

A daemon for monitoring the health and status of IXPDIMM

A common information model (CIM) provider that exposes
IXPDIMM_SW as standard CIM objects in order to plug-in to various
common information model object managers (CIMOMS).

An application program interface (API) for configuring and managing
IXPDIMM. Including basic inventory, capacity provisioning,
health monitoring, and troubleshooting. 

#Documentation

https://01.org/ixpdimm-sw/

#Build
GNU Make is used to build for Linux and Windows.
As such we recommend mingw_w64 and msys be used to build in a windows
environment. Both of which can be found at mingw.org

#Build Windows

#The following variables must be set to find the required dependencies necessary
to build.

The default path to these dependencies for windows is: 
	workspace/external/
where the makefile is located in: 
	workspace/ixpdimm_sw/makefile

* MINGW_DIR - On Windows, the path to the MinGW 64-bit tools.
* CTEMPLATE_INCLUDE_DIR - The path to the ctemplate headers.
* CTEMPLATE_LIB_DIR - The path to the ctemplate libraries.  
* OPENSSL_INCLUDE_DIR - The path to the openssl headers. 
* OPENSSL_LIB_DIR - The path to openssl libraries.
* SQLITE_INCLUDE_DIR - The path to the sqlite headers. 
* SQLITE_LIB_DIR - The path to the sqlite libraries.
* ZLIB_INCLUDE_DIR - The path to the zlib headers. 
* ZLIB_LIB_DIR - The path to the zlib libraries.
* OS_INCLUDE_DIR - The path to any OS specific driver headers. 
* OS_LIB_DIR - The path to any OS specific driver libraries.
* RAPIDXML_INCLUDE_DIR - The path to the rapidxml headers. 
* GETTEXT_EXE_DIR - The path to the xgettext.exe (for string translations). 
* INTEL_CLI_FRAMEWORK_DIR - The path to Intel NVM CLI libraries
* INTEL_CIM_FRAMEWORK_DIR - The path to Intel NVM CIM libraries

Assuming default paths have been used execute 'make RELEASE=1'

build artifacts can then be found in output/build/windows/release

#Build Linux

Kernel 4.3 or newer is suggested, a prebuilt fedora 4.3 kernel can be found at

https://kojipkgs.fedoraproject.org/packages/kernel/4.3.0/1.fc24/x86_64/

libndctl is required to build, an rpm can be found at. 

https://copr.fedoraproject.org/coprs/djbw/ndctl/

and the source can be found at 

https://github.com/pmem/ndctl

Intel NVM CIM library is required, the source can be found at

https://github.com/01org/intelnvmcimlibrary

Intel NVM CLI library is required, the source can be found at

https://github.com/01org/intelnvmclilibrary

All other dependencies are widely available. This includes ctemplate openssl
sqlite and zlib.

Once dependencies have been resolved execute 'make RELEASE=1'

build artifacts can be found in output/build/linux/release

