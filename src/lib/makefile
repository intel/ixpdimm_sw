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
# Makefile for Native Management Library
#


# ---- BUILD ENVIRONMENT ---------------------------------------------------------------------------
ROOT_DIR = ../..
# sets up standard build variables
include $(ROOT_DIR)/build.mk

# defines the location of this module's compiled objects, and the archive (static lib) of its objects
OBJECT_MODULE_DIR = $(OBJECT_DIR)/lib

# ---- COMPONENTS ----------------------------------------------------------------------------------
CREATE_CFG_DB = create_config_db
UNITTEST = unittest

# ---- FILES ---------------------------------------------------------------------------------------
# defines list of objects to create from source, with an exclusion list based on OS
ifdef BUILD_WINDOWS
	SRC_EXCLUDE = $(wildcard lnx_*.* esx_*.*)
else ifdef BUILD_LINUX
	SRC_EXCLUDE = $(wildcard win_*.* esx_*.*)
else ifdef BUILD_ESX
	SRC_EXCLUDE = $(wildcard win_*.* lnx_*.*)
else
	$(error Build target platform is not defined)
endif

# excludes based on simulator flag
ifdef BUILD_SIM
	SRC_EXCLUDE = $(wildcard lnx_*.*)
	SRC_EXCLUDE += $(wildcard win_*.*)
	SRC_EXCLUDE += $(wildcard esx*.*)
else
	SRC_EXCLUDE += $(wildcard sim_*.*)
endif

LIB_HEADER = nvm_management.h
EXTERNAL_HEADERS = $(LIB_HEADER) nvm_types.h 
IOCTL = device_fw.h fis_types.h
STYLE_IGNORE = $(IOCTL) libIntelNvmMgmt.h
SRC = $(filter-out $(SRC_EXCLUDE),$(wildcard *.c))
HEADERS = $(filter-out $(SRC_EXCLUDE),$(wildcard *.h))
OBJS = $(patsubst %.c,%.o,$(SRC)) 
#add the resource file on windows
ifdef BUILD_WINDOWS
	OBJS += lib_resources.o
endif

# defines the location of the object files
OBJNAMES = $(addprefix $(OBJECT_MODULE_DIR)/, $(OBJS))

# pull in any previously generated source dependency declarations (.d files)
# (hyphen preceeding the "include" keyword allows MAKE to continue if a .d file is not found) 
-include $(addprefix $(OBJECT_MODULE_DIR)/, $(patsubst %.c,%.d,$(SRC)))

CSTYLE_FILES = $(filter-out $(STYLE_IGNORE),$(HEADERS) $(SRC))

# Target library 'linker name'
TARGETBASE = $(API_LIB_SONAME).$(LIB_SUFFIX)
# Target library 'soname'
TARGETSO = $(TARGETBASE).$(VERSION_MAJOR)
# Target library 'real name'
ifdef BUILD_WINDOWS
	TARGET = $(TARGETBASE)
else
	TARGET = $(TARGETSO).$(VERSION_MINOR).0
endif 
# Target library 'real name', prefixed with its output location
# This library is packaged as its own library, therefore output to build directory 
TARGETNAME = $(addprefix $(BUILD_DIR)/, $(TARGET))

ALL_TARGETS = $(OBJECT_MODULE_DIR) $(TARGETNAME) create_config_db $(UNITTEST) 

# ---- COMPILER PARAMETERS -------------------------------------------------------------------------
INCS =  -I. \
		-I$(SRC_DIR)/common \
		-I$(EXTERN_DIR)/sqlite \
		-I$(EXTERN_DIR)/openssl/include \
		-I$(EXTERN_LIB_DIR)/openssl \
		-I$(EXTERN_DIR)/zlib \
		-I$(EXTERN_DIR)/fw_headers

LIBS =	-L$(OBJECT_DIR)/common -lcommon \
		-L$(BUILD_DIR) -lsqlite3 \
		-lz \
		-lcrypto

ifdef BUILD_WINDOWS
	INCS += -I$(EXTERN_DIR)/os_headers
	LIBS += -lws2_32 -lmswsock -ladvapi32 -lversion -lGdi32 -lShlwapi\
	 		-L$(BUILD_DIR) -lssp -lpthread -llibintl \
			-L$(EXTERN_LIB_DIR)/zlib \
			-L$(EXTERN_LIB_DIR)/openssl/openssl

	CFLAGS += -D_WIN32_WINNT=0x0601
else ifdef BUILD_LINUX
	INCS += -isystem$(EXTERN_DIR)/os_headers \
			-I$(EXTERN_DIR)/os_headers/linux
	
	LIBS += -lpthread -ldl -lm
	ifndef BUILD_SIM
		LIBS += -lndctl
	endif
	CFLAGS += -D_GNU_SOURCE
ifndef BUILD_SIM
	LIBS += -lnuma
endif
else ifdef BUILD_ESX
	INCS += -I$(EXTERN_DIR)/os_headers
	LIBS += -lpthread -ldl -lm
ifndef BUILD_SIM
	LIBS += -L/opt/vmware/nvm-mgmt-6.0.0-$(DEVKIT_BUILD_NUM)/samples/lib -lvmkuserlib
endif
	CFLAGS += -D_GNU_SOURCE 
	CFLAGS += -DVMK_DEVKIT_HAS_API_VMKAPI_BASE \
		-DVMK_DEVKIT_USES_PUBLIC_APIS \
		-DVMK_DEVKIT_USES_BINARY_COMPATIBLE_APIS \
		-D_DEVKIT_SRC_
ifndef BUILD_SIM
	INCS += -I/opt/vmware/nvm-mgmt-6.0.0-$(DEVKIT_BUILD_NUM)/samples/include 
endif
endif

# Building a DLL - control exports
CFLAGS += $(BUILD_DLL_FLAG)

ifdef INTEL_I18N
INCS += -I$(I18N_INCLUDE_DIR)
LIBS += -l$(I18N_LIB_NAME)
endif
# ---- RECIPES -------------------------------------------------------------------------------------
all : 
	$(MAKE) $(JOBCOUNT) $(OBJECT_MODULE_DIR)
	$(MAKE) $(JOBCOUNT) $(TARGETNAME)
	$(MAKE) $(CREATE_CFG_DB)
ifndef SKIP_UNITTESTS	
	$(MAKE) $(UNITTEST)
endif

$(TARGETNAME) : $(OBJNAMES) $(OBJECT_DIR)/common/libcommon.a
ifdef BUILD_WINDOWS
	$(CC) $(CFLAGS) -shared $^ $(LIBS) -o $@
else
	$(CC) $(CFLAGS) -shared $^ $(LIBS) -Wl,-soname,$(TARGETSO) -o $@ 
	cd $(BUILD_DIR); $(RM) $(TARGETSO); $(SOFTLINK) $(TARGET) $(TARGETSO)
	cd $(BUILD_DIR); $(RM) $(TARGETBASE); $(SOFTLINK) $(TARGET) $(TARGETBASE)
endif
	# copy the header to the build output
	$(foreach header,$(EXTERNAL_HEADERS),$(RM) $(BUILD_DIR)/$(header))
	$(COPY) $(EXTERNAL_HEADERS) $(BUILD_DIR)
	#update the version number in the released header file
	sed -i -e "s/\#define	NVM_VERSION_MAJOR   __VERSION_MAJOR__/\#define	NVM_VERSION_MAJOR	$(VERSION_MAJOR)/g" $(BUILD_DIR)/$(LIB_HEADER)
	sed -i -e "s/\#define	NVM_VERSION_MINOR   __VERSION_MINOR__/\#define	NVM_VERSION_MINOR	$(VERSION_MINOR)/g" $(BUILD_DIR)/$(LIB_HEADER)
	sed -i -e "s/\#define	NVM_VERSION_HOTFIX   __VERSION_HOTFIX__/\#define	NVM_VERSION_HOTFIX	$(VERSION_HOTFIX)/g" $(BUILD_DIR)/$(LIB_HEADER)
	sed -i -e "s/\#define	NVM_VERSION_BUILDNUM	__VERSION_BUILDNUM__/\#define	NVM_VERSION_BUILDNUM	$(VERSION_BUILDNUM)/g" $(BUILD_DIR)/$(LIB_HEADER)
	
$(OBJECT_MODULE_DIR) :
	$(MKDIR) $@

# suffix rule for .c -> .o
$(OBJECT_MODULE_DIR)/%.o : %.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@ $(LDFLAGS)

# suffix rule for .rc -> .o
$(OBJECT_MODULE_DIR)/%.o : %.rc
	$(RC) $(RCFLAGS) $(INCS) $< -o $@

cstyle:
	$(CSTYLE) $(CSTYLE_FILES)

unittest:
ifdef BUILD_SIM
	$(MAKE) -C $(UNITTEST) all
endif

test:
ifndef ESX_BUILD # can't run ESX tests on build system
ifdef BUILD_SIM
	$(MAKE) -C $(UNITTEST) test
endif
endif

create_config_db:
	$(MAKE) -C $(CREATE_CFG_DB) all

i18n:
	$(GETTEXT) support.c # translate error messages
	$(GETTEXT) monitor.c #translate event messages

clean :
	$(RM) $(OBJNAMES)
	$(MAKE) -C $(CREATE_CFG_DB) clean
	$(MAKE) -C $(UNITTEST) clean

clobber :
	$(RMDIR) $(OBJECT_MODULE_DIR)

sourcedrop: 
	$(MKDIR) $(SOURCEDROP_DIR)/src/lib
	$(MKDIR) $(SOURCEDROP_DIR)/src/lib/$(CREATE_CFG_DB)
	$(MKDIR) -p $(SOURCEDROP_DIR)/external/os_headers/linux
	$(COPY) $(EXTERN_DIR)/os_headers/linux/ndctl.h $(SOURCEDROP_DIR)/external/os_headers/linux
	$(COPY) $(HEADERS) $(SOURCEDROP_DIR)/src/lib/
	$(COPY) makefile $(SOURCEDROP_DIR)/src/lib/
	$(COPY) $(SRC) $(SOURCEDROP_DIR)/src/lib/
	$(COPY) $(CREATE_CFG_DB)/* $(SOURCEDROP_DIR)/src/lib/$(CREATE_CFG_DB)
	$(COPY) *.rc $(SOURCEDROP_DIR)/src/lib/

.PHONY : all dir cstyle unittest test create_config_db i18n clean clobber version_header sourcedrop
