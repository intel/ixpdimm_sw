#
# Copyright (c) 2015 2017, Intel Corporation
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
# Makefile for the ACPI library code. 
#

# ---- BUILD ENVIRONMENT ---------------------------------------------------------------------------
ROOT_DIR = ../..
# sets up standard build variables
include $(ROOT_DIR)/build.mk

# defines the location of this module's compiled objects
OBJECT_MODULE_DIR = $(OBJECT_DIR)/acpi

# ---- FILES ---------------------------------------------------------------------------------------
# defines list of objects to create from source
ifdef BUILD_WINDOWS
	SRC_EXCLUDE = $(wildcard lnx_*.c esx_*.c)
else ifdef BUILD_LINUX
	SRC_EXCLUDE = $(wildcard win_*.c esx_*.c)
else ifdef BUILD_ESX
	SRC_EXCLUDE = $(wildcard win_*.c lnx_*.c)
endif

SRC = $(filter-out $(SRC_EXCLUDE),$(wildcard *.c))
OBJS = $(patsubst %.c,%.o,$(SRC))
COMMON_OBJS += $(wildcard $(OBJECT_DIR)/common/string/s_str.o)
COMMON_OBJS += $(wildcard $(OBJECT_DIR)/common/guid/guid.o)

# defines the location of the object files
OBJNAMES = $(addprefix $(OBJECT_MODULE_DIR)/, $(OBJS))

# pull in any previously generated source dependency declarations (.d files)
# (hyphen preceeding the "include" keyword allows MAKE to continue if a .d file is not found)
-include $(addprefix $(OBJECT_MODULE_DIR)/, $(patsubst %.c,%.d,$(SRC)))

# Target archive 'linker name'
TARGET = libacpi.a
# Target archive 'real name', prefixed with its output location
# This archive is packaged into other libraries, therefore output to object directory 
TARGETNAME = $(addprefix $(OBJECT_MODULE_DIR)/, $(TARGET))
 
# ---- COMPILER PARAMETERS -------------------------------------------------------------------------
INCS = -I$(SRC_DIR)/acpi \
	   -I$(SRC_DIR)/common
LIBS = 

ifdef BUILD_LINUX
	CFLAGS += -D_GNU_SOURCE
else ifdef BUILD_ESX
	CFLAGS += -D_GNU_SOURCE
	LIBS += -lpthread -ldl -lm
	INCS += -I$(EXTERN_DIR)/os_headers/esx
	INCS += -I/opt/vmware/$(ESX_DEVKIT_PREFIX)-$(DEVKIT_BUILD_NUM)/samples/include
	LIBS += -L/opt/vmware/$(ESX_DEVKIT_PREFIX)-$(DEVKIT_BUILD_NUM)/samples/lib -lvmkuser
	CFLAGS += -DVMK_DEVKIT_HAS_API_VMKAPI_BASE \
		-DVMK_DEVKIT_USES_PUBLIC_APIS \
		-DVMK_DEVKIT_USES_BINARY_COMPATIBLE_APIS \
		-D_DEVKIT_SRC_
endif 

# ---- RECIPES -------------------------------------------------------------------------------------
all :
	$(MAKE) $(JOBCOUNT) $(OBJECT_MODULE_DIR) 
	$(MAKE) $(JOBCOUNT) $(TARGETNAME)
ifndef SKIP_UNITTESTS	
	$(MAKE) -C unittest all
endif
	
$(OBJECT_MODULE_DIR) :
	$(MKDIR) $@

$(TARGETNAME) : $(OBJNAMES) $(COMMON_OBJS)
	$(AR) rcs $@ $^ 
	
# suffix rule for .c -> .o
$(OBJECT_MODULE_DIR)/%.o : %.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@ $(LDFLAGS)

test:
ifndef ESX_BUILD # can't run ESX tests on build system
	$(MAKE) -C unittest test
endif

cstyle :
	$(CSTYLE) $(SRC)

clean :
	$(RM) $(OBJNAMES)

clobber : clean

sourcedrop: 
	$(MKDIR) $(SOURCEDROP_DIR)/src/acpi
	$(COPY) *.h $(SOURCEDROP_DIR)/src/acpi
	$(COPY) *.c $(SOURCEDROP_DIR)/src/acpi
	$(COPY) makefile $(SOURCEDROP_DIR)/src/acpi

.PHONY : all clean test cstyle clobber sourcedrop