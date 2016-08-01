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
# Makefile for the common code library
#

# ---- BUILD ENVIRONMENT ---------------------------------------------------------------------------
ROOT_DIR = ../..
# sets up standard build variables
include $(ROOT_DIR)/build.mk

# defines the location of this module's compiled objects, and the archive (static lib) of its objects
OBJECT_MODULE_DIR = $(OBJECT_DIR)/common

# ---- COMPONENTS ----------------------------------------------------------------------------------
FILEOPS = file_ops
OS = os
UNITTEST = unittest

# defines the submodules compiled via this makefile (note encrypt not included intentionally)
SUBMODULES = string time guid uid system persistence smbios

# defines the submodules compiled independent of this makefile
INDEPENDENT_SUBMODULES = file_ops os

SUBDIRS = . encrypt $(SUBMODULES) $(INDEPENDENT_SUBMODULES) 

# ---- FILES ---------------------------------------------------------------------------------------
# defines the header files
HDRS := $(wildcard *.h)
HDRS += $(foreach dir,$(SUBMODULES),$(wildcard $(dir)/*.h))
HDRS += $(wildcard encrypt/*.h)
# defines the source files to be compiled by this makefile
SCHEMA = schema.c schema.h
SRC := persistence/schema.c $(foreach dir,$(SUBMODULES),$(wildcard $(dir)/*.c)) 

# defines the location of each submodule's object files' path
SUBMODULE_DIRS = $(foreach dir,$(SUBMODULES),$(OBJECT_MODULE_DIR)/$(dir))
# defines the location of each submodule's object files
SUBMODULE_OBJS := $(addprefix $(OBJECT_MODULE_DIR)/, $(patsubst %.c,%.o,$(SRC)))

EXCLUDES = 
# find all submodule elements (.c, .rc, .mc) that get turned into .o files
INDEPENDENT_SUBMODULE_SRC = $(foreach dir,$(INDEPENDENT_SUBMODULES),$(wildcard $(dir)/*.*c))
ifdef BUILD_LINUX
	EXCLUDES = esx*.* private.rev0.pem
	INDEPENDENT_SUBMODULE_SRC_EXCLUDE = $(foreach dir,$(INDEPENDENT_SUBMODULES),\
		$(wildcard $(dir)/win*.*c $(dir)/esx*.*c))
else ifdef BUILD_WINDOWS
	EXCLUDES = esx*.* private.rev0.pem
	INDEPENDENT_SUBMODULE_SRC_EXCLUDE = $(foreach dir,$(INDEPENDENT_SUBMODULES),\
		$(wildcard $(dir)/lnx*.*c $(dir)/esx*.*c))
else ifdef BUILD_ESX
	EXCLUDES = lnx*.* win*.* private.rev0.pem
	INDEPENDENT_SUBMODULE_SRC_EXCLUDE = $(foreach dir,$(INDEPENDENT_SUBMODULES),\
		$(wildcard $(dir)/lnx*.*c $(dir)/win*.*c))
endif

INDEPENDENT_SUBMODULE_TMP = $(patsubst %.rc,%.o,$(patsubst %.mc,%.o,$(patsubst %.c,%.o,$(filter-out $(INDEPENDENT_SUBMODULE_SRC_EXCLUDE),$(INDEPENDENT_SUBMODULE_SRC)))))
INDEPENDENT_SUBMODULE_OBJS = $(foreach obj,$(INDEPENDENT_SUBMODULE_TMP),$(OBJECT_MODULE_DIR)/$(obj))

# pull in any previously generated source dependency declarations (.d files)
# (hyphen preceeding the "include" keyword allows MAKE to continue if a .d file is not found) 
-include $(addprefix $(OBJECT_MODULE_DIR)/, $(patsubst %.c,%.d,$(SRC) encrypt/encrypt.c))

# Target archive 'linker name'
TARGET = libcommon.a
# Target archive 'real name', prefixed with its output location
# This archive is packaged into other libraries, therefore output to object directory 
TARGETNAME = $(addprefix $(OBJECT_MODULE_DIR)/, $(TARGET))

STYLE_IGNORE = persistence/schema.c persistence/schema.h
CSTYLE_FILES = $(filter-out $(STYLE_IGNORE),$(HEADERS) $(SRC))

# ---- COMPILER PARAMETERS -------------------------------------------------------------------------
INCS = 	-I. \
		-I$(EXTERN_DIR)/sqlite \
		-I$(EXTERN_DIR)/openssl/include \
		-I$(EXTERN_LIB_DIR)/openssl \
		-I$(EXTERN_DIR)/zlib \
		-I$(SRC_DIR)/lib
LIBS =

ifdef BUILD_WINDOWS
	ENCRYPT_CFLAGS=$(CFLAGS)
else ifdef BUILD_LINUX
	ENCRYPT_CFLAGS=$(CFLAGS)
else ifdef BUILD_ESX
	ENCRYPT_CFLAGS=-fstack-protector-all -D_FORTIFY_SOURCE=2 -D_XOPEN_SOURCE=500 -std=c99 \
	-fPIC -MMD -D__VERSION_MAJOR__=$(VERSION_MAJOR) -D__VERSION_MINOR__=$(VERSION_MINOR) \
	-D__VERSION_HOTFIX__=$(VERSION_HOTFIX) -D__VERSION_BUILDNUM__=$(VERSION_BUILDNUM) \
	-D__VERSION_NUMBER__=$(VERSION_NUMBER)
endif

ifdef INTEL_I18N
INCS += -I$(I18N_INCLUDE_DIR)
endif

# public key file
PUBLIC_KEY_NAME = public.rev0.pem

# ---- RECIPES -------------------------------------------------------------------------------------
all :
	$(MAKE) $(SUBMODULE_DIRS)
	$(MAKE) $(JOBCOUNT) dependencies
	$(MAKE) $(JOBCOUNT) $(TARGETNAME)
ifndef SKIP_UNITTESTS
	$(MAKE) unittest
endif 

dependencies: 
	$(MAKE) persistence/schema.c
	$(MAKE) $(INDEPENDENT_SUBMODULES)    
	$(MAKE) encryptdir

$(TARGETNAME) : $(SUBMODULE_OBJS) $(OBJECT_MODULE_DIR)/encrypt/encrypt.o $(INDEPENDENT_SUBMODULE_OBJS)
	$(AR) rcs $@ $^

$(SUBMODULE_DIRS) : $(BUILD_DIR)/$(PUBLIC_KEY_NAME)
	$(MKDIR) $@
	$(MKDIR) $(OBJECT_MODULE_DIR)/encrypt

$(BUILD_DIR)/$(PUBLIC_KEY_NAME) :
	$(COPY) $(SRC_DIR)/common/encrypt/$(PUBLIC_KEY_NAME) $(BUILD_DIR)

# suffix rule for .c -> .o
$(OBJECT_MODULE_DIR)/%.o : %.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@ $(LDFLAGS)

file_ops :
	$(MAKE) -C $(FILEOPS) all

os :
	$(MAKE) -C $(OS) all
	
# this is separate because I need to turn off warnings on ESX
encryptdir:
	$(CC) $(ENCRYPT_CFLAGS) $(INCS) -c encrypt/encrypt.c -o $(OBJECT_MODULE_DIR)/encrypt/encrypt.o $(LDFLAGS)

unittest :
	$(MAKE) -C $(UNITTEST) all

test :
ifndef ESX_BUILD # can't run ESX tests on build system
	$(MAKE) -C $(UNITTEST) test
endif

cstyle :
	$(CSTYLE) $(CSTYLE_FILES)
	$(MAKE) -C $(OS) cstyle
	$(MAKE) -C $(FILEOPS) cstyle

i18n:
	$(GETTEXT) persistence/event.c #translate event messages

clean :
	$(RM) $(TARGETNAME)
	$(RM) $(SUBMODULE_OBJS)
	$(RM) $(OBJECT_MODULE_DIR)/encrypt/encrypt.o
	$(RM) persistence/schema.c persistence/schema.h
	$(RM) $(BUILD_DIR)/$(PUBLIC_KEY_NAME)
	$(MAKE) -C $(OS) clean
	$(MAKE) -C $(FILEOPS) clean
	$(MAKE) -C $(UNITTEST) clean

clobber :
	$(RMDIR) $(OBJECT_MODULE_DIR)
	$(RM) $(BUILD_DIR)/$(PUBLIC_KEY_NAME)

sourcedrop: 
	$(MKDIR) $(SOURCEDROP_DIR)/src/common
	for dir in $(SUBDIRS); do \
		$(MKDIR) $(SOURCEDROP_DIR)/src/common/$${dir}; \
		$(COPY) $${dir}/*.* $(SOURCEDROP_DIR)/src/common/$${dir}/; \
		$(COPY) $${dir}/makefile $(SOURCEDROP_DIR)/src/common/$${dir}/; \
		for exclude in $(EXCLUDES); do \
			$(RM) $(SOURCEDROP_DIR)/src/common/$${dir}/$${exclude}; \
		done \
	done 

.PHONY : all file_ops os dependencies unittest test cstyle clean clobber sourcedrop
