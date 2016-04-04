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
# Makefile for the DB creator
#

# ---- BUILD ENVIRONMENT ---------------------------------------------------------------------------
ROOT_DIR = ../../..
# sets up standard build variables
include $(ROOT_DIR)/build.mk

OBJECT_MODULE_LIB_DIR = $(OBJECT_DIR)/lib
OBJECT_MODULE_DIR = $(OBJECT_MODULE_LIB_DIR)/create_config_db

# ---- FILES ---------------------------------------------------------------------------------------
# This makefile doesn't follow the format of the others because it only needs a small subset of the 
# functionality of the native library. It can't link to the whole library, because the library
# requires the apss.dat DB to exist, which is what this utilty does. Chicken? Egg? 
# It's a tool used during the build and not released. 
OBJNAMES  = $(OBJECT_MODULE_DIR)/main.o 

# pull in any previously generated source dependency declarations (.d files)
# (hyphen preceeding the "include" keyword allows MAKE to continue if a .d file is not found) 
-include $(OBJECT_MODULE_DIR)/main.d

TARGET = $(BUILD_DIR)/create_db.exe

# ---- COMPILER PARAMETERS -------------------------------------------------------------------------
INCS = 	-I$(SRC_DIR)/common \
		-I$(SRC_DIR)/lib \
		-I$(EXTERN_DIR)/sqlite
		
LIBS = 	-L$(OBJECT_DIR)/common -lcommon \
	-L$(BUILD_DIR) -lsqlite3
		
ifdef BUILD_WINDOWS
	LIBS += -lws2_32 -lmswsock -ladvapi32 -lversion -lGdi32 -lShlwapi\
	 		-L$(BUILD_DIR) -lssp -lpthread -llibintl
	CFLAGS += -D_WIN32_WINNT=0x0601
else ifdef BUILD_LINUX
	LIBS += -lpthread -ldl -lm
	CFLAGS += -D_GNU_SOURCE
ifndef BUILD_SIM
	LIBS += -lnuma
endif
else ifdef BUILD_ESX
	LIBS += -lpthread -ldl -lm
	CFLAGS += -D_GNU_SOURCE
endif
		
		
ifdef BUILD_WINDOWS
	LIBS += -lws2_32 -lmswsock -lShlwapi
else
	LIBS += -ldl -lpthread
endif
		
# ---- RECIPES -------------------------------------------------------------------------------------
all :
	$(MAKE) $(JOBCOUNT) $(OBJECT_MODULE_DIR)
	$(MAKE) $(JOBCOUNT) $(TARGET)
	$(MAKE) create_db 

$(TARGET) : $(OBJNAMES)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

create_db :
	export LD_LIBRARY_PATH=$(BUILD_DIR) && $(TARGET) $(abspath $(BUILD_DIR)) 

$(OBJECT_MODULE_DIR) :
	$(MKDIR) $@
	
# suffix rule for .c -> .o
$(OBJECT_MODULE_DIR)/%.o : %.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@ $(LDFLAGS)
	
clean :
	$(RM) $(OBJNAMES)
	
clobber : clean
	$(RM) $(TARGET)

.PHONY : all create_db clean clobber
