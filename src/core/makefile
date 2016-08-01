#
# Copyright (c) 2016, Intel Corporation
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
# Makefile for Core Library
#

# ---- BUILD ENVIRONMENT ---------------------------------------------------------------------------
ROOT_DIR = ../..
# sets up standard build variables
include $(ROOT_DIR)/build.mk

OBJECT_MODULE_DIR = $(OBJECT_DIR)/core

# ---- COMPONENTS ----------------------------------------------------------------------------------
SUBMODULES = . device system exceptions memory_allocator

UNITTEST = unittest
# ---- FILES ---------------------------------------------------------------------------------------
SRC = $(foreach dir,$(SUBMODULES),$(wildcard $(dir)/*.cpp))
HEADERS = $(foreach dir,$(SUBMODULES),$(wildcard $(dir)/*.h))

OBJS = $(patsubst %.cpp,%.o,$(SRC))
#add the resource file on windows
ifdef BUILD_WINDOWS
	OBJS += core_resources.o
endif
OBJNAMES = $(addprefix $(OBJECT_MODULE_DIR)/, $(OBJS))

# defines the location of each submodule's object files' path
SUBMODULE_DIRS = $(foreach dir,$(SUBMODULES),$(OBJECT_MODULE_DIR)/$(dir))

# defines the location of each submodule's object files
SUBMODULE_OBJS = $(addprefix $(OBJECT_MODULE_DIR)/, $(patsubst %.cpp,%.o,$(SRC)))

# pull in any previously generated source dependency declarations (.d files)
# (hyphen preceeding the "include" keyword allows MAKE to continue if a .d file is not found) 
-include $(addprefix $(OBJECT_MODULE_DIR)/, $(patsubst %.cpp,%.d,$(SRC)))

# Target library 'linker name'
TARGETBASE = $(CORE_LIB_SONAME).$(LIB_SUFFIX)
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
INCS = 	-I$(SRC_DIR) -I$(SRC_DIR)/core \
		-I$(SRC_DIR)/common \
		-I$(SRC_DIR)/lib \
		-I$(EXTERN_DIR)/intel_i18n/include

LIBS = 	-L$(OBJECT_DIR)/common -lcommon \
		-L$(BUILD_DIR) -l$(API_LIB_NAME) \
			-lsqlite3 \
			-lcrypto

ifdef BUILD_WINDOWS
	LIBS += -lws2_32 -lmswsock -lShlwapi -L$(EXTERN_LIB_DIR)/openssl/openssl
else ifdef BUILD_LINUX
	LIBS += -ldl -lm -lpthread
else ifdef BUILD_ESX
	LIBS += -ldl -lm
endif

# Building a DLL - control exports
CPPFLAGS += $(BUILD_DLL_FLAG)

# ---- RECIPES -------------------------------------------------------------------------------------
all :  
	$(MAKE) $(JOBCOUNT) $(OBJECT_MODULE_DIR)
	$(MAKE) $(JOBCOUNT) $(SUBMODULE_DIRS) 
	$(MAKE) $(JOBCOUNT) $(TARGETNAME)
ifndef SKIP_UNITTESTS
	$(MAKE) $(UNITTEST)
endif
	
$(OBJECT_MODULE_DIR) $(SUBMODULE_DIRS) :
	$(MKDIR) $@

$(TARGETNAME) : $(OBJNAMES)
ifdef BUILD_WINDOWS
	$(CPP) $(CPPFLAGS) -shared $^ $(LIBS) -o $@ 
else ifdef BUILD_ESX
	$(CPP) $(CPPFLAGS) -Wl,-rpath,$(ESX_SUPPORT_DIR) -shared $^ $(LIBS) -Wl,-soname,$(TARGETSO) -o $@
	cd $(BUILD_DIR); $(RM) $(TARGETSO); $(SOFTLINK) $(TARGET) $(TARGETSO)
	cd $(BUILD_DIR); $(RM) $(TARGETBASE); $(SOFTLINK) $(TARGET) $(TARGETBASE)
else ifdef BUILD_LINUX
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
	
unittest:
	$(MAKE) -C $(UNITTEST) all
	
test:
ifndef ESX_BUILD # can't run ESX tests on build system
	$(MAKE) -C $(UNITTEST) test
endif
	
clean:
	rm -f $(TARGET) $(OBJNAMES)
	$(MAKE) -C $(UNITTEST) clean
	
clobber :
	$(RMDIR) $(OBJECT_MODULE_DIR)

sourcedrop: 
	$(MKDIR) $(SOURCEDROP_DIR)/src/core
	$(COPY) makefile $(SOURCEDROP_DIR)/src/core
	for dir in $(SUBMODULES); do \
		$(MKDIR) $(SOURCEDROP_DIR)/src/core/$${dir}; \
		$(COPY) $${dir}/*.* $(SOURCEDROP_DIR)/src/core/$${dir}/; \
	done
	
.PHONY : all unittest test i18n clean sourcedrop
