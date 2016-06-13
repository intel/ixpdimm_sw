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
# Makefile for IXPDIMM Monitor service
#

# ---- BUILD ENVIRONMENT ---------------------------------------------------------------------------
ROOT_DIR = ../..
# sets up standard build variables
include $(ROOT_DIR)/build.mk

OBJECT_MODULE_DIR = $(OBJECT_DIR)/monitor

# ---- COMPONENTS ----------------------------------------------------------------------------------
ifdef BUILD_WINDOWS
    TARGET_EXTENSION=.exe
endif

UNITTEST = unittest

# ---- FILES ---------------------------------------------------------------------------------------
SRC = $(wildcard *.cpp)
HEADERS = $(wildcard *.h)

ifdef BUILD_WINDOWS
	SRC_EXCLUDE = $(wildcard lnx_*.cpp esx_*.cpp)
	HEADER_EXCLUDE= = $(wildcard lnx_*.h esx_*.h)
else ifdef BUILD_LINUX
	SRC_EXCLUDE = $(wildcard win_*.cpp esx_*.cpp)
	HEADER_EXCLUDE= = $(wildcard win_*.h esx_*.h)
else ifdef BUILD_ESX
	# TODO: need to test on Linux. If lnx requires
	# own implementation, then the first one is correct
	# SRC_EXCLUDE = $(wildcard win_*.cpp lnx_*.cpp)
	SRC_EXCLUDE = $(wildcard win_*.cpp)
	HEADER_EXCLUDE= = $(wildcard win_*.h)
else
	$(error Build target platform is not defined)
endif

SRC = $(filter-out $(SRC_EXCLUDE),$(wildcard *.cpp))
HEADERS = $(filter-out $(HEADER_EXCLUDE),$(wildcard *.h))

OBJS = $(patsubst %.cpp, %.o, $(SRC))
#add the resource file on windows

OBJNAMES = $(addprefix $(OBJECT_MODULE_DIR)/, $(OBJS))

# pull in any previously generated source dependency declarations (.d files)
# (hyphen preceeding the "include" keyword allows MAKE to continue if a .d file is not found) 
-include $(addprefix $(OBJECT_MODULE_DIR)/, $(patsubst %.cpp,%.d,$(SRC)))

# Target executable
TARGET = $(MONITOR_NAME)$(TARGET_EXTENSION)

# Target library 'real name', prefixed with its output location
# This library is packaged as its own library, therefore output to build directory 
TARGETNAME = $(addprefix $(BUILD_DIR)/, $(TARGET))

INIT_SCRIPT = $(MONITOR_NAME).service

# Control exports
CPPFLAGS += $(BUILD_DLL_FLAG)


# ---- COMPILER PARAMETERS -------------------------------------------------------------------------
INCS = 	-I$(SRC_DIR)/common \
		-I$(SRC_DIR)/lib

LIBS = 	-L$(OBJECT_DIR)/common -lcommon \
		-L$(BUILD_DIR) -l$(API_LIB_NAME) \
			-lsqlite3 -lssl -lcrypto -lz

ifdef BUILD_WINDOWS
	LIBS += -lws2_32 -lmswsock -ladvapi32 -lversion -lGdi32 -lShlwapi \
	 		-L$(BUILD_DIR) -lssp -lpthread -llibintl \
			-L$(EXTERN_LIB_DIR)/zlib \
			-L$(EXTERN_LIB_DIR)/openssl/openssl
else ifdef BUILD_LINUX
	INCS += -I$(EXTERN_DIR)/os_headers/linux \
	
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
	$(MAKE) $(JOBCOUNT) $(OBJECT_MODULE_DIR)
	$(MAKE) $(JOBCOUNT) $(TARGETNAME)
	$(MAKE) initd
ifndef SKIP_UNITTESTS
	$(MAKE) $(UNITTEST)
endif

$(OBJECT_MODULE_DIR) :
	$(MKDIR) $@

$(TARGETNAME) : $(OBJNAMES)
ifdef BUILD_ESX
	$(CPP) -Wl,-rpath,$(ESX_SUPPORT_DIR) $(CPPFLAGS) $(OBJNAMES) $(LIBS) -o $@
else
	$(CPP) $(CPPFLAGS) $(OBJNAMES) $(LIBS) -o $@
endif
ifdef BUILD_WINDOWS
	# Set System account security for service
	# (NOTE: QuickBuild fails with this command, so commenting out. It also appears that the files built by QB
	# have the appropriate permissions
	# cd $(BUILD_DIR); icacls . //grant SYSTEM:F
	# cd $(BUILD_DIR); icacls \* //grant SYSTEM:F
endif

# suffix rule for .cpp -> .o
$(OBJECT_MODULE_DIR)/%.o : %.cpp
	$(CPP) $(CPPFLAGS) $(INCS) -c $< -o $@ $(LDFLAGS)
	
# suffix rule for .rc -> .o
$(OBJECT_MODULE_DIR)/%.o : %.rc
	$(RC) $(RCFLAGS) $(INCS) $< -o $@

initd:
ifndef BUILD_WINDOWS
	$(COPY) $(INIT_SCRIPT) $(BUILD_DIR)
endif

ifdef BUILD_SIM
$(UNITTEST):
	$(MAKE) -C $(UNITTEST) all
	
test:
ifndef ESX_BUILD # can't run ESX tests on build system
	$(MAKE) -C $(UNITTEST) test
endif
endif
	
i18n:
	$(GETTEXT) framework/NvmStrings.h # Translate error messages
	
sourcedrop: 
	$(MKDIR) $(SOURCEDROP_DIR)/src/monitor
	$(COPY) $(SRC) $(SOURCEDROP_DIR)/src/monitor/
	$(COPY) $(HEADERS) $(SOURCEDROP_DIR)/src/monitor/
	$(COPY) *.rc $(SOURCEDROP_DIR)/src/monitor/
	$(COPY) *.service $(SOURCEDROP_DIR)/src/monitor/
	$(COPY) makefile $(SOURCEDROP_DIR)/src/monitor/
	
clean:
	rm -f $(TARGET) $(OBJNAMES)
	$(MAKE) -C $(UNITTEST) clean
	
clobber :
	$(RMDIR) $(OBJECT_MODULE_DIR)
	
.PHONY : all turncovon turncovoff unittest test clean sourcedrop
