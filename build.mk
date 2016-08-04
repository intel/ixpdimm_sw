#
# Copyright (c) 2015 2016, Intel Corporation
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#   * Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#   * Neither the name of Intel Corporation nor the names of its contributors
#     may be used to endorse or promote products derived from this software
#     without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#
# Standard make declarations. Handles common and OS specific settings
#


# require that we know the root of the repo, or error
ifndef ROOT_DIR
	ROOT_ERR := $(error ROOT_DIR variable not set)
endif

# define common OS operations
RM = rm -f
COPY = cp -av
MOVE = mv
MKDIR = mkdir -p
RMDIR = rm -rf
SOFTLINK = ln -s -f 
SED = sed
TAR = tar
RPMBUILD = rpmbuild

# useful makefile debugging tools
# Its a hack, but it does the job! To unpause, simply hit the [ENTER] key.
PAUSE = read

# helper variables to define certain characters
BLDMK_PERIOD = .
BLDMK_COMMA = ,
BLDMK_EMPTY =
BLDMK_COLON = :
BLDMK_SLASH = /
BLDMK_SPACE = $(BLDMK_EMPTY) $(BLDMK_EMPTY)

# HW specific compile flags
LARGE_PAYLOAD = 1 #Whether or not large payloads are available 
EARLY_HW = 0 #Temporary compile option for PO HW with no media

#version number passed in from the build server
ifndef BUILDNUM
	BUILDNUM= $(shell git describe --abbrev=0 | sed -e 's/\([a-zA-Z_-]*\)\(.*\)/\2/g')
	ifeq ($(strip $(BUILDNUM)),)
		BUILDNUM=99.99.99.9999
	endif
endif

#parse into individual pieces
VERSION_MAJOR = $(word 1,$(subst ., ,$(BUILDNUM)))
VERSION_MINOR = $(word 2,$(subst ., ,$(BUILDNUM)))
VERSION_HOTFIX = $(word 3,$(subst ., ,$(BUILDNUM)))
VERSION_BUILDNUM = $(word 4,$(subst ., ,$(BUILDNUM)))

#version number passed in from the build server
ifndef WBEM_PREFIX_INPUT
	WBEM_PREFIX_INPUT = Intel_
endif

# additional manufacturing type commands
ADD_MANUFACTURING ?= 1

#Uncomment this line if you wish to use Intel I18N
INTEL_I18N=-D__INTEL_I18N__

# initialize/define compilation flag helper variables
C_CPP_FLAGS_CMN = -Wall -Werror -Wfatal-errors -Wformat -Wformat-security -fstack-protector-all -D_FORTIFY_SOURCE=2 -D_XOPEN_SOURCE=500 
C_CPP_FLAGS_SRC = -MMD -D__VERSION_MAJOR__=$(VERSION_MAJOR) -D__VERSION_MINOR__=$(VERSION_MINOR) -D__VERSION_HOTFIX__=$(VERSION_HOTFIX) -D__VERSION_BUILDNUM__=$(VERSION_BUILDNUM) -D__VERSION_NUMBER__=$(BUILDNUM) -D__ADD_MANUFACTURING__=$(ADD_MANUFACTURING)  -D__WBEM_PREFIX__='$(WBEM_PREFIX_INPUT)' $(INTEL_I18N)
LDFLAGS = -z noexecstack -z relro -z now -pie

CFLAGS_CMN = -std=c99
CPPFLAGS_CMN =
CPPFLAGS_SRC =
CFLAGS_EXTERNAL ?=
CPPFLAGS_EXTERNAL ?=

MARKETING_PRODUCT_NAME=ixpdimm_sw
API_LIB_SONAME=libixpdimm
API_LIB_NAME=ixpdimm
CORE_LIB_SONAME=libixpdimm-core
CORE_LIB_NAME=ixpdimm-core
CLI_LIB_SONAME=libixpdimm-cli
CLI_LIB_NAME=ixpdimm-cli
CIM_LIB_SONAME=libixpdimm-cim
CIM_LIB_NAME=ixpdimm-cim

CLI_NAME=ixpdimm-cli
MONITOR_NAME=ixpdimm-monitor

CIM_FRAMEWORK_LIB_SONAME=libinvm-cim
CIM_FRAMEWORK_LIB_NAME=invm-cim
I18N_LIB_SONAME=libinvm-i18n
I18N_LIB_NAME=invm-i18n
CLI_FRAMEWORK_LIB_SONAME=libinvm-cli
CLI_FRAMEWORK_LIB_NAME=invm-cli

# OS specific settings
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
	CORE_COUNT = $(shell nproc)
		
	# ESX builds occur on Linux but the environment will include this variable
	ifdef ESXBUILD
		# ESX path for CIM libraries - needs to be defined here so can be used as rpath in other makefiles
		ESX_SUPPORT_DIR := /opt/intel/bin
		
		ifndef GLIBC_HASH
			HASH_ERR := $(error GLIBC_HASH not set, see /opt/vmware/toolchain/cayman_esx_glibc* for hash)
		endif
		
		ifndef DEVKIT_BUILD_NUM
			DEVKIT_ERR := $(error DEVKIT_BUILD_NUM not set, see /opt/vmware/nvm-mgmt-6.0.0-* for Number)
		endif		
		
		GLIBC_DIR = /opt/vmware/toolchain/cayman_esx_glibc-$(GLIBC_HASH)/sysroot
		
		# the current devkit version
		MGMT_ENV_DIR = /opt/mgmt_env
		MGMT_SYSROOT =  $(MGMT_ENV_DIR)/4.5-32
		BUILD_ESX = 1
		OS_TYPE = esx
		LIB_SUFFIX = so

		# GNU toolchain provided by VMware
		AR = /build/toolchain/lin32/gcc-4.4.3-1/x86_64-linux5.0/bin/ar
		CC = /build/toolchain/lin32/gcc-4.6.3-1/bin/i686-linux5.0-gcc \
			--sysroot=$(GLIBC_DIR)
		C_CPP_FLAGS_CMN += -D_GNU_SOURCE -m32
		# VMware doesn't directly support C++ for ESX 3rd party applications.
		# Provide own C++ compiler and development environment.
		CPP = export LD_LIBRARY_PATH=$(MGMT_SYSROOT)/usr/lib;\
			$(MGMT_SYSROOT)/usr/bin/g++ -m32 --sysroot=$(MGMT_SYSROOT)
		CPP_RUNTIME = $(MGMT_SYSROOT)/usr/lib/libstdc++.so*

		C_CPP_FLAGS_CMN += -fPIE -fPIC
		C_CPP_FLAGS_SRC += -D__ESX__
		
		PRODUCT_DATADIR = $(ESX_SUPPORT_DIR)
	else
		BUILD_LINUX = 1
		OS_TYPE = linux
		LIB_SUFFIX = so
		HW_ARCH = $(shell uname -i)
		# get the Mgmt Build Environment
		MGMT_ENV_DIR ?= /opt/mgmt_env

		# doxygen
		DOXYGEN ?= $(MGMT_ENV_DIR)/doxygen/doxygen

		# GNU toolchain
		CC = gcc
		CPP = g++
		AR = ar

		#Linux Product Names
		LINUX_PRODUCT_NAME = $(MARKETING_PRODUCT_NAME)
		CIM_NAMESPACE = intelwbem
		
		# Linux Install Directories
		uname_m = $(shell uname -m)
		ifneq (,$(findstring i686,$(uname_m)))
			LIB_DIR ?= /usr/lib
			CIM_LIB_DIR = /usr/lib/cmpi
		else
			LIB_DIR ?= /usr/lib64
			CIM_LIB_DIR = /usr/lib64/cmpi
		endif
		INCLUDE_DIR ?= /usr/include
		BIN_DIR ?= /usr/bin
		DATADIR ?= /var/lib
		LOCALSTATE_DIR ?= /var
		PRODUCT_DATADIR = $(DATADIR)/$(LINUX_PRODUCT_NAME)
		PEGASUS_MOFDIR = $(PRODUCT_DATADIR)/Pegasus/mof
		UNIT_DIR ?=  /usr/lib/systemd/system
		SFCB_DIR = $(PRODUCT_DATADIR)/sfcb
		SYSCONF_DIR ?= /etc
		MANPAGE_DIR ?= /usr/share/man
		MAN8_DIR ?= $(MANPAGE_DIR)/man8

		# Linux Install Files
		LIB_FILES = $(API_LIB_SONAME).so* $(CORE_LIB_SONAME).so* $(CLI_LIB_SONAME).so*
		CIM_LIB_FILES = $(CIM_LIB_SONAME).so*
		INCLUDE_FILES = nvm_management.h nvm_types.h
		BIN_FILES = $(CLI_NAME) $(MONITOR_NAME)
		PEGASUS_MOF_FILES = pegasus_register.mof profile_registration.mof intelwbem.mof
		SFCB_MOF_FILES = sfcb_intelwbem.mof
		SFCB_REG_FILE = INTEL_NVDIMM.reg 
		DATADIR_FILES = apss.dat* public.rev0.pem
		INIT_FILES = $(MONITOR_NAME).service
		MANPAGE_GZ_FILES = $(CLI_NAME).8.gz $(MONITOR_NAME).8.gz
		MANPAGE_SCRIPT_FILES = create_$(CLI_NAME)_manpage.py create_$(MONITOR_NAME)_manpage.py $(CLI_NAME).manpage.footer $(CLI_NAME).manpage.header $(MONITOR_NAME).manpage.text $(CLI_NAME).sed manpage_helper.py

		C_CPP_FLAGS_SRC += -D__LINUX__ 
		
		ifneq ("$(wildcard /etc/redhat-release)","")
			LINUX_DIST := rel
		else ifneq ("$(wildcard /etc/SuSE-release)","")
			LINUX_DIST := sle
		else
			LINUX_DIST := $(warning Unrecognized Linux distribution)
		endif
	endif
	C_CPP_FLAGS_CMN += -fPIE -fPIC -D__PRODUCT_DATADIR__=\"$(PRODUCT_DATADIR)/\"		
else
	CORE_COUNT = $(NUMBER_OF_PROCESSORS)
	BUILD_WINDOWS = 1
	OS_TYPE = windows
	LIB_SUFFIX = dll

	# get the Mgmt Build Environment
	MGMT_ENV_DIR ?= C:/mgmt_env

	# doxygen (runs in windows only)
	DOXYGEN ?= $(MGMT_ENV_DIR)/doxygen/doxygen.exe

	# MinGW_w64 toolchain
	MINGW_DIR ?= $(MGMT_ENV_DIR)/mingw_w64
	include $(MINGW_DIR)/mingw.mk

	# note: -mno-ms-bitfields is a workaround for a gcc (4.7.0)+ byte-packing bug
	#		This may cause GCC packed structs to present differences with MSVC packed structs
	C_CPP_FLAGS_CMN += -m64 -mno-ms-bitfields
	C_CPP_FLAGS_SRC += -D__WINDOWS__
	CPPFLAGS_CMN += -static-libgcc -static-libstdc++
endif

# the number of make jobs to run in parallel = 2 * number of processors + 1
# can be overriden by setting an environment variable MAX_JOBS = whatever
MAX_JOBS ?= $(shell echo $$((2*$(CORE_COUNT)+1)))
JOBCOUNT ?= -j $(MAX_JOBS) 

# simulator/real build specific settings
BUILD_DLL_FLAG = -D__NVM_DLL_EXPORTS__
ifdef BUILD_SIM
	ADAPTER_TYPE = simulator
	C_CPP_FLAGS_SRC += -D__BUILD_SIM__=$(BUILD_SIM)
	CFLAGS_CMN += -D__LARGE_PAYLOAD__=1
	CFLAGS_CMN += -D__EARLY_HW__=0
else
	ADAPTER_TYPE = real
	CFLAGS_CMN += -D__LARGE_PAYLOAD__=$(LARGE_PAYLOAD)
	CFLAGS_CMN += -D__EARLY_HW__=$(EARLY_HW)
	# only export public APIs for non-simulated builds 
	ifdef BUILD_WINDOWS
		C_CPP_FLAGS_CMN += -D__NVM_DLL__
	else 
		C_CPP_FLAGS_CMN += -D__NVM_DLL__ -fvisibility=hidden
	endif
endif

ifdef LEAK_CHECK
	CPPFLAGS_SRC += -D__LEAK_CHECK__=$(LEAK_CHECK)
endif

# release/debug build specific settings
ifdef RELEASE
	BUILD_TYPE = release
	C_CPP_FLAGS_CMN += -O2
	CPPFLAGS_CMN += -fno-strict-aliasing
else
	BUILD_TYPE = debug
	C_CPP_FLAGS_CMN += -O
	C_CPP_FLAGS_CMN += -ggdb
endif

# define compilation flags used within $(ROOT_DIR)/external & $(ROOT_DIR)/src
CFLAGS_EXT = $(C_CPP_FLAGS_CMN) $(CFLAGS_CMN)
CPPFLAGS_EXT = $(C_CPP_FLAGS_CMN) $(CPPFLAGS_CMN)
CFLAGS = $(C_CPP_FLAGS_CMN) $(CFLAGS_CMN) $(C_CPP_FLAGS_SRC) $(CFLAGS_EXTERNAL)
CPPFLAGS = $(C_CPP_FLAGS_CMN) $(CPPFLAGS_CMN) $(C_CPP_FLAGS_SRC) $(CPPFLAGS_SRC) $(CPPFLAGS_EXTERNAL)
RCFLAGS = -D__VERSION_MAJOR__=$(VERSION_MAJOR) -D__VERSION_MINOR__=$(VERSION_MINOR) -D__VERSION_HOTFIX__=$(VERSION_HOTFIX) -D__VERSION_BUILDNUM__=$(VERSION_BUILDNUM) -D__VERSION_NUMBER__=$(BUILDNUM)

# define top-level directories
EXTERN_DIR = $(ROOT_DIR)/external
TOOLS_DIR = $(ROOT_DIR)/tools
SRC_DIR = $(ROOT_DIR)/src
ifdef CCOV
	OUTPUT_DIR ?= $(ROOT_DIR)/output/cov
else 
	OUTPUT_DIR ?= $(ROOT_DIR)/output
endif

# define OS-specific build output directories
EXTERN_LIB_DIR = $(EXTERN_DIR)/precompiled_libs/$(OS_TYPE)
BUILT_TOOLS_DIR = $(OUTPUT_DIR)/tools/$(OS_TYPE)/$(ADAPTER_TYPE)/$(BUILD_TYPE)
OBJECT_DIR = $(OUTPUT_DIR)/obj/$(OS_TYPE)/$(ADAPTER_TYPE)/$(BUILD_TYPE)
BUILD_DIR = $(OUTPUT_DIR)/build/$(OS_TYPE)/$(ADAPTER_TYPE)/$(BUILD_TYPE)
DOCS_DIR = $(OUTPUT_DIR)/docs
SOURCEDROP_WORKSPACE ?= $(OUTPUT_DIR)/workspace
SOURCEDROP_DIR ?= $(OUTPUT_DIR)/workspace/$(MARKETING_PRODUCT_NAME)
CLI_FRAMEWORK_DIR = $(ROOT_DIR)/external/intel_cli_framework
CIM_FRAMEWORK_DIR = $(ROOT_DIR)/external/intel_cim_framework
I18N_DIR = $(ROOT_DIR)/external/intel_i18n
I18N_INCLUDE_DIR = $(EXTERN_DIR)/intel_i18n/include/
RPMBUILD_DIR ?= $(shell pwd)/$(OUTPUT_DIR)/rpmbuild

# unit test execution directories
COMMON_TEST_DIR = $(BUILD_DIR)/common_test
LIB_TEST_DIR = $(BUILD_DIR)/lib_test
CORE_TEST_DIR = $(BUILD_DIR)/core_test
WBEM_TEST_DIR = $(BUILD_DIR)/wbem_test
CLI_TEST_DIR = $(BUILD_DIR)/cli_test
MONITOR_TEST_DIR = $(BUILD_DIR)/monitor_test

# Tools
CSTYLE = $(TOOLS_DIR)/cstyle/cstyle -o doxygen -pP
SCHEMA_GEN = $(BUILT_TOOLS_DIR)/SchemaGen.exe

# test coverage tool
ifdef CCOV
	export COVFILE := $(ROOT_DIR_PATH)/bste.cov
	JOBCOUNT := -j 1 # don't run coverage compile in parallel
	# Prevent conflicts with other copies of this project on the system
	export COVBUILDZONE := $(abspath $(BUILD_DIR))
	
	BULLSEYE_DIR = $(MGMT_ENV_DIR)/bullseye
	include $(BULLSEYE_DIR)/bullseye.mk
	# force bullseye to update coverage metrics at termination of executables
	# instead of during execution
	export COVAUTOSAVE := 0
endif

#get text package name - used in file name and set as the text domain
LOCALE_DOMAIN = $(CLI_NAME)
LOCALE_DIR = $(abspath $(BUILD_DIR))/lang

# output file for gettext 
GETTEXT_OUTPUTFILE = $(BUILD_DIR)/$(LOCALE_DOMAIN).pot

# gettext (http://www.gnu.org/software/gettext/manual/gettext.html)
# keyword option defines what is used to "mark" text that should be translated.
# "-j" option joins existing output file. This is needed because otherwise each xgettext call
#		would overwrite output file. It won't duplicate entries if run over and over, however
#		the file must already exist.
# example: "this should be translated" becomes TR("this should be translated")
# TODO: After initial translations have been made an additional utility (msgmerge) will be
#		needed to merge changes into existing translated files. The gettext manual describes this
#		process in detail.
GETTEXT = xgettext
GETTEXT_DIR = $(ROOT_DIR)/tools/gettext
MSGMERGE = msgmerge
MSGFMT = msgfmt
ifdef BUILD_WINDOWS
	GETTEXT = $(ROOT_DIR)/tools/gettext/xgettext
	MSGMERGE = $(ROOT_DIR)/tools/gettext/msgmerge
	MSGFMT = $(ROOT_DIR)/tools/gettext/msgfmt
endif
ifdef BUILD_ESX
	GETTEXT = $(MGMT_ENV_DIR)/gettext/xgettext
	MSGMERGE = $(MGMT_ENV_DIR)/gettext/msgmerge
endif
GETTEXT :=  touch $(GETTEXT_OUTPUTFILE); $(GETTEXT) --keyword=N_TR --keyword=TR -j -o $(GETTEXT_OUTPUTFILE)
