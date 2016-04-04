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
# Makefile for the schema generator
#

# ---- BUILD ENVIRONMENT ---------------------------------------------------------------------------
ROOT_DIR = ../..
# sets up standard build variables
include $(ROOT_DIR)/build.mk

OBJECT_MODULE_DIR = $(OBJECT_DIR)/schema_generator

# ---- FILES ---------------------------------------------------------------------------------------
SRC_EXCLUDE = $(wildcard lnx_*.cpp)
ifdef BUILD_LINUX
	SRC_EXCLUDE = $(wildcard win_*.cpp)
endif
# excludes based on simulator flag
ifdef BUILD_SIM
	SRC_EXCLUDE = $(wildcard real_*.cpp)
else
	SRC_EXCLUDE += $(wildcard sim_*.cpp)
endif

SRC = $(filter-out $(SRC_EXCLUDE),$(wildcard *.cpp))
OBJS = $(patsubst %.cpp,%.o,$(SRC))
OBJNAMES = $(addprefix $(OBJECT_MODULE_DIR)/, $(OBJS))

# pull in any previously generated source dependency declarations (.d files)
# (hyphen preceeding the "include" keyword allows MAKE to continue if a .d file is not found) 
-include $(addprefix $(OBJECT_MODULE_DIR)/, $(patsubst %.o,%.d,$(OBJS)))

# specify appropriate ctemplate library
ifndef BUILD_WINDOWS
	LIBCTEMPLATE = -L$(BUILT_TOOLS_DIR) -lctemplate
else
	LIBCTEMPLATE = -L$(BUILT_TOOLS_DIR) -lctemplate-2
endif

LIBCTEMPLATE_FILES = $(addprefix $(BUILT_TOOLS_DIR)/, $(notdir $(wildcard $(EXTERN_LIB_DIR)/ctemplate/*)))
LIBSQLITE_FILES = $(addprefix $(BUILT_TOOLS_DIR)/, $(notdir $(wildcard $(EXTERN_LIB_DIR)/sqlite/*)))

TARGET = $(SCHEMA_GEN)

# ---- COMPILER PARAMETERS -------------------------------------------------------------------------
#LIBS = -L$(BUILT_TOOLS_DIR) -lsqlite3 $(LIBCTEMPLATE)
LIBS = $(LIBCTEMPLATE) -L$(OBJECT_DIR)/db_schema_gen -ldb_schema_gen -L$(BUILD_DIR) -lsqlite3  


ifndef BUILD_WINDOWS
	LIBS += -ldl
endif

INCS = 	-I$(EXTERN_DIR) \
		-I$(EXTERN_DIR)/sqlite3 \
		-I$(TOOLS_DIR)/db_schema_gen

# ---- RECIPES -------------------------------------------------------------------------------------
all:
	$(MAKE) $(JOBCOUNT) $(BUILT_TOOLS_DIR)
	$(MAKE) $(JOBCOUNT) $(OBJECT_MODULE_DIR)
	$(MAKE) $(TARGET) 

$(TARGET): $(LIBCTEMPLATE_FILES) $(LIBSQLITE_FILES) $(OBJNAMES)
	$(CPP) $(CPPFLAGS) $(OBJNAMES) $(LIBS) -o $@

$(OBJECT_MODULE_DIR) $(BUILT_TOOLS_DIR) :
	$(MKDIR) $@

# suffix rule for .cpp -> .o
$(OBJECT_MODULE_DIR)/%.o : %.cpp
	$(CPP) $(CPPFLAGS) $(INCS) -c $< -o $@ $(LDFLAGS)

$(LIBCTEMPLATE_FILES) :
	$(COPY) $(EXTERN_LIB_DIR)/ctemplate/* $(BUILT_TOOLS_DIR)/
ifndef BUILD_WINDOWS
	cd $(BUILT_TOOLS_DIR); $(RM) libctemplate.so.2; $(SOFTLINK) libctemplate.so.2.0.1 libctemplate.so.2
	cd $(BUILT_TOOLS_DIR); $(RM) libctemplate.so; $(SOFTLINK) libctemplate.so.2.0.1 libctemplate.so
endif

$(LIBSQLITE_FILES) :
	$(COPY) $(EXTERN_LIB_DIR)/sqlite/* $(BUILT_TOOLS_DIR)/
ifndef BUILD_WINDOWS
	cd $(BUILT_TOOLS_DIR); $(RM) libsqlite3.so.0; $(SOFTLINK) libsqlite3.so.0.8.6 libsqlite3.so.0
	cd $(BUILT_TOOLS_DIR); $(RM) libsqlite3.so; $(SOFTLINK) libsqlite3.so.0.8.6  libsqlite3.so
endif

clean:
	rm -f $(TARGET) $(OBJNAMES)

sourcedrop: 
	$(MKDIR) $(SOURCEDROP_DIR)/src/schema_generator
	$(COPY) * $(SOURCEDROP_DIR)/src/schema_generator/

.PHONY : all clean run sourcedrop
