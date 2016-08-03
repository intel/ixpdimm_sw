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
# Makefile for the NVMCLI core features of the CLI
#

# ---- BUILD ENVIRONMENT ---------------------------------------------------------------------------
ROOT_DIR = ../../../..
# sets up standard build variables
include $(ROOT_DIR)/build.mk

OBJECT_MODULE_DIR = $(OBJECT_DIR)/cli/features/core
TEMP_OBJECT_MODULE_DIR = $(OBJECT_DIR)/cli/features/core/framework

# ---- COMPONENTS ----------------------------------------------------------------------------------
UNITTEST = unittest

# ---- FILES ---------------------------------------------------------------------------------------
SRC = $(wildcard *.cpp) $(wildcard framework/*.cpp)
HEADERS = $(wildcard *.h)

OBJS = $(patsubst %.cpp,%.o,$(SRC))
#add the resource file on windows
ifdef BUILD_WINDOWS
	OBJS += nvmcli_core_features_resources.o
endif
OBJNAMES = $(addprefix $(OBJECT_MODULE_DIR)/, $(OBJS))

# pull in any previously generated source dependency declarations (.d files)
# (hyphen preceeding the "include" keyword allows MAKE to continue if a .d file is not found) 
-include $(addprefix $(OBJECT_MODULE_DIR)/, $(patsubst %.cpp,%.d,$(SRC)))

# Target library 'linker name'
TARGETBASE = $(CLI_LIB_SONAME).$(LIB_SUFFIX)
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

# ---- COMPILER PARAMETERS -------------------------------------------------------------------------
INCS = 	-I$(SRC_DIR)/cli \
		-I$(SRC_DIR)/lib \
		-I$(SRC_DIR)/common \
		-I$(SRC_DIR)/wbem \
		-I$(SRC_DIR)/ \
		-I$(CLI_FRAMEWORK_DIR)/include \
		-I$(CIM_FRAMEWORK_DIR)/include

LIBS = 	-L$(OBJECT_DIR)/common -lcommon \
		-L$(BUILD_DIR) \
			-l$(CLI_FRAMEWORK_LIB_NAME) \
			-l$(CIM_FRAMEWORK_LIB_NAME) \
			-l$(API_LIB_NAME) \
			-l$(CORE_LIB_NAME) \
			-l$(CIM_LIB_NAME) \
			-lsqlite3 \
			-lcrypto 

ifdef BUILD_WINDOWS
	LIBS += -lws2_32 -lmswsock -lShlwapi \
			-L$(BUILD_DIR) -llibintl \
			-L$(EXTERN_LIB_DIR)/openssl/openssl 
else
	LIBS += -ldl -lm
endif

# Building a DLL - control exports
CPPFLAGS += $(BUILD_DLL_FLAG)

ifdef INTEL_I18N
INCS += -I$(I18N_INCLUDE_DIR)
LIBS += -l$(I18N_LIB_NAME)
endif
# ---- RECIPES -------------------------------------------------------------------------------------
all :
	$(MAKE) $(JOBCOUNT) $(OBJECT_MODULE_DIR) $(TEMP_OBJECT_MODULE_DIR)
	$(MAKE) $(JOBCOUNT) $(TARGETNAME)
ifndef SKIP_UNITTESTS
	$(MAKE) $(UNITTEST)
endif

$(OBJECT_MODULE_DIR) $(TEMP_OBJECT_MODULE_DIR):
	$(MKDIR) $@
	
$(TARGETNAME) : $(OBJNAMES)
ifdef BUILD_WINDOWS
	$(CPP) $(CPPFLAGS) -shared $^ $(LIBS) -o $@ 
else
	$(CPP) $(CPPFLAGS) -shared $^ $(LIBS) -Wl,-soname,$(TARGETSO) -o $@
	cd $(BUILD_DIR); $(RM) $(TARGETSO); $(SOFTLINK) $(TARGET) $(TARGETSO)
	cd $(BUILD_DIR); $(RM) $(TARGETBASE); $(SOFTLINK) $(TARGET) $(TARGETBASE)
endif
	
# suffix rule for .cpp -> .o
$(OBJECT_MODULE_DIR)/%.o : %.cpp
	$(CPP) $(CPPFLAGS) $(INCS) -c $< -o $@ $(LDFLAGS)

# suffix rule for .rc -> .o
$(OBJECT_MODULE_DIR)/%.o : %.rc
	$(RC) $(RCFLAGS) $(INCS) $< -o $@
	
$(UNITTEST):
ifndef BUILD_ESX
	$(MAKE) -C $(UNITTEST) all
endif

test:
ifndef BUILD_ESX
	$(MAKE) -C $(UNITTEST) test
endif

i18n: 
	$(GETTEXT) *.cpp
	$(GETTEXT) *.h

sourcedrop: 
	$(MKDIR) $(SOURCEDROP_DIR)/src/cli/features/core
	$(MKDIR) $(SOURCEDROP_DIR)/src/cli/features/core/framework
	$(COPY) *.* $(SOURCEDROP_DIR)/src/cli/features/core/
	$(COPY) framework/*.* $(SOURCEDROP_DIR)/src/cli/features/core/framework
	$(COPY) makefile $(SOURCEDROP_DIR)/src/cli/features/core/
	$(RM) $(SOURCEDROP_DIR)/src/cli/features/core/Example*.*
	
clean:
	rm -f $(TARGET) $(OBJNAMES)
	$(MAKE) -C $(UNITTEST) clean

clobber :
	$(RMDIR) $(OBJECT_MODULE_DIR)

.PHONY : all unittest test clean clobber sourcedrop i18n
