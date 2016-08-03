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
# Makefile for the NVMCLI application
#

# ---- BUILD ENVIRONMENT ---------------------------------------------------------------------------
ROOT_DIR = ../..

SCRIPTS_PATH := $(dir $(realpath $(lastword $(realpath $(CURDIR)))))
SCRIPTS_PATH := $(dir $(realpath $(lastword $(SCRIPTS_PATH))))scripts
CLI_BUILD_PATH = $(realpath $(BUILD_DIR)) 

# sets up standard build variables
include $(ROOT_DIR)/build.mk

OBJECT_MODULE_DIR = $(OBJECT_DIR)/cli

C_CPP_FLAGS_SRC += -DLOCALE_DOMAIN='"$(LOCALE_DOMAIN)"' -DLOCALE_DIR='"$(LOCALE_DIR)"'

# ---- COMPONENTS ----------------------------------------------------------------------------------
FRAMEWORK = $(BUILD_DIR)/$(CLI_FRAMEWORK_LIB_SONAME).$(LIB_SUFFIX)
FEATURES = $(BUILD_DIR)/$(CLI_LIB_SONAME).$(LIB_SUFFIX)

# ---- FILES ---------------------------------------------------------------------------------------
SRC = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp, %.o, $(SRC))
#add the resource file on windows
ifdef BUILD_WINDOWS
	OBJS += cli_resources.o
endif
OBJNAMES = $(addprefix $(OBJECT_MODULE_DIR)/, $(OBJS))

# pull in any previously generated source dependency declarations (.d files)
# (hyphen preceeding the "include" keyword allows MAKE to continue if a .d file is not found) 
-include $(addprefix $(OBJECT_MODULE_DIR)/, $(patsubst %.cpp,%.d,$(SRC)))

TARGETBASE=$(CLI_NAME)
ifdef BUILD_WINDOWS
	TARGET = $(TARGETBASE).exe
else
	TARGET = $(TARGETBASE)
endif 
TARGET_NAME = $(addprefix $(BUILD_DIR)/, $(TARGET))

# ---- COMPILER PARAMETERS -------------------------------------------------------------------------
INCS = 	-I$(SRC_DIR)/common \
		-I$(EXTERN_DIR)/sqlite \
		-I$(EXTERN_DIR)/openssl/include \
		-I$(EXTERN_DIR)/zlib \
		-I$(SRC_DIR)/cli \
		-I$(SRC_DIR)/lib \
		-I$(SRC_DIR)/wbem \
		-I$(CLI_FRAMEWORK_DIR)/include \
		-I$(CIM_FRAMEWORK_DIR)/include

LIBS =	-L$(OBJECT_DIR)/common -lcommon \
		-L$(BUILD_DIR) \
			-l$(CLI_FRAMEWORK_LIB_NAME) \
			-l$(CIM_FRAMEWORK_LIB_NAME) \
			-l$(API_LIB_NAME) \
			-l$(CORE_LIB_NAME) \
			-l$(CIM_LIB_NAME) \
			-lsqlite3 \
			-lz \
			-lcrypto		
		
ifdef BUILD_WINDOWS
	LIBS += -lws2_32 -lmswsock -ladvapi32 -lversion -lGdi32 -lShlwapi \
	 		-L$(BUILD_DIR) -lssp -lpthread -llibintl \
			-L$(EXTERN_LIB_DIR)/zlib \
			-L$(EXTERN_LIB_DIR)/openssl/openssl 
else ifdef BUILD_LINUX
	LIBS += -lpthread -ldl -lm
	ifndef BUILD_SIM
		LIBS += -lndctl -lkmod
	endif
else ifdef BUILD_ESX
	LIBS += -lpthread -ldl -lm
ifndef BUILD_SIM
	LIBS += -L/opt/vmware/nvm-mgmt-6.0.0-$(DEVKIT_BUILD_NUM)/samples/lib -lvmkuserlib
endif
endif

ifdef INTEL_I18N
INCS += -I$(I18N_INCLUDE_DIR)
LIBS += -l$(I18N_LIB_NAME)
endif
# ---- RECIPES -------------------------------------------------------------------------------------
all :   
	$(MAKE) $(JOBCOUNT) $(BUILD_DIR)
	$(MAKE) features
	$(MAKE) $(JOBCOUNT) $(TARGET_NAME)
	$(MAKE) doc
	
$(TARGET_NAME): $(FEATURES) $(OBJNAMES) 
ifdef BUILD_ESX
	$(CPP) -Wl,-rpath,$(ESX_SUPPORT_DIR) $(CPPFLAGS) $(OBJNAMES) $(LIBS) -o $@
else
	$(CPP) $(CPPFLAGS) $(OBJNAMES) $(LIBS) -o $@
endif

$(BUILD_DIR):
	$(MKDIR) $@

features:
	$(MAKE) -C features all ADD_MANUFACTURING=$(ADD_MANUFACTURING)
	
doc :
ifdef BUILD_LINUX
	cd $(SCRIPTS_PATH); \
	python $(SCRIPTS_PATH)/create_$(CLI_NAME)_manpage.py dataPath=$(PRODUCT_DATADIR) outputPath=$(CLI_BUILD_PATH); \
	python $(SCRIPTS_PATH)/create_$(MONITOR_NAME)_manpage.py dataPath=$(PRODUCT_DATADIR); 
	$(eval MANPAGE_GZ_FILES := $(addprefix $(SCRIPTS_PATH)/, $(MANPAGE_GZ_FILES)))
	$(MOVE) $(MANPAGE_GZ_FILES) $(BUILD_DIR)
endif
	
test:
ifndef ESX_BUILD # can't run ESX tests on build system
	$(MAKE) -C features test
endif

# suffix rule for .cpp -> .o
$(OBJECT_MODULE_DIR)/%.o : %.cpp
	$(CPP) $(CPPFLAGS) $(INCS) -c $< -o $@ $(LDFLAGS)

# suffix rule for .rc -> .o
$(OBJECT_MODULE_DIR)/%.o : %.rc
	$(RC) $(RCFLAGS) $(INCS) $< -o $@

i18n:
	$(GETTEXT) *.cpp
	make -C features i18n

clean : 
	rm -f $(OBJNAMES)
	$(MAKE) -C features clean

clobber :
	rm -f $(TARGET)
	$(MAKE) -C features clobber

sourcedrop: 
	$(MKDIR) $(SOURCEDROP_DIR)/src/cli
	$(COPY) *.* $(SOURCEDROP_DIR)/src/cli
	$(COPY) makefile $(SOURCEDROP_DIR)/src/cli
	$(MAKE) -C features sourcedrop
ifdef BUILD_ESX
	$(MKDIR) $(SOURCEDROP_DIR)/src/cli/esx
	$(COPY) esx/esxcli-nvmcli.xml $(SOURCEDROP_DIR)/src/cli/esx
endif
		
.PHONY : all clean clobber features i18n sourcedrop
