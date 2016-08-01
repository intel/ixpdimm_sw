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
# Makefile for Wbem Library
#

# ---- BUILD ENVIRONMENT ---------------------------------------------------------------------------
ROOT_DIR = ../..
# sets up standard build variables
include $(ROOT_DIR)/build.mk

OBJECT_MODULE_DIR = $(OBJECT_DIR)/wbem

# ---- COMPONENTS ----------------------------------------------------------------------------------
ifdef BUILD_WINDOWS
	CIMOM = wmi
	CIMOMBUILD = wmi
endif
ifdef BUILD_LINUX
	# really open pegasus
	CIMOM = cmpi
	CIMOMBUILD = cmpi
endif
ifdef BUILD_ESX
	CIMOM = sfcb
	CIMOMBUILD = cmpi
endif

SUBMODULES = . framework_interface lib_interface exception server memory physical_asset profile \
	support software performance erasure mem_config pmem_config indication pegasus cimom/$(CIMOMBUILD)

UNITTEST = unittest
FRAMEWORK = $(BUILD_DIR)/$(CIM_FRAMEWORK_LIB_SONAME).$(LIB_SUFFIX)
# ---- FILES ---------------------------------------------------------------------------------------
SRC = $(foreach dir,$(SUBMODULES),$(wildcard $(dir)/*.cpp)) \
		../monitor/NvmMonitorBase.cpp \
        ../monitor/PerformanceMonitor.cpp \
        ../monitor/EventMonitor.cpp
HEADERS = $(foreach dir,$(SUBMODULES),$(wildcard $(dir)/*.h))

OBJS = $(patsubst %.cpp,%.o,$(SRC))
#add the resource file on windows
ifdef BUILD_WINDOWS
	OBJS += wbem_resources.o
endif
OBJNAMES = $(addprefix $(OBJECT_MODULE_DIR)/, $(OBJS))

# defines the location of each submodule's object files' path
SUBMODULE_DIRS = $(foreach dir,$(SUBMODULES),$(OBJECT_MODULE_DIR)/$(dir)) $(OBJECT_DIR)/monitor

# defines the location of each submodule's object files
SUBMODULE_OBJS = $(addprefix $(OBJECT_MODULE_DIR)/, $(patsubst %.cpp,%.o,$(SRC)))

# pull in any previously generated source dependency declarations (.d files)
# (hyphen preceeding the "include" keyword allows MAKE to continue if a .d file is not found) 
-include $(addprefix $(OBJECT_MODULE_DIR)/, $(patsubst %.cpp,%.d,$(SRC)))

# Target library 'linker name'
TARGETBASE = $(CIM_LIB_SONAME).$(LIB_SUFFIX)
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
INCS = 	-I$(SRC_DIR) \
		-I$(SRC_DIR)/common \
		-I$(SRC_DIR)/lib \
		-I$(SRC_DIR)/wbem \
		-I$(SRC_DIR) \
		-I$(EXTERN_DIR)/rapidxml-1.13 \
		-I$(CIM_FRAMEWORK_DIR)/include

ifndef BUILD_WINDOWS
	INCS += -I$(EXTERN_DIR)/cmpi/include
endif

LIBS = 	-L$(OBJECT_DIR)/common -lcommon \
		-L$(BUILD_DIR) -l$(API_LIB_NAME) \
		-L$(BUILD_DIR) -l$(CORE_LIB_NAME) \
		-lsqlite3 \
		-lssl -lcrypto \
		-l$(CIM_FRAMEWORK_LIB_NAME)

ifdef BUILD_WINDOWS
	LIBS += -lws2_32 -lmswsock -lShlwapi
	# these are needed to build the WMI COM stuff
	LIBS += -lkernel32 -luser32 -lgdi32 -lwinspool \
		-lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid \
		-lodbc32 -lodbccp32 -lwbemuuid -lwbemcore -lwbemupgd \
		-L$(BUILD_DIR) -llibintl \
		-L$(EXTERN_LIB_DIR)/openssl/openssl 
else ifdef BUILD_LINUX
	LIBS += -ldl -lm -lpthread
	ifndef BUILD_SIM
		LIBS += -lndctl -lkmod
	endif
else ifdef BUILD_ESX
	LIBS += -ldl -lm
endif

# Building a DLL - control exports
CPPFLAGS += $(BUILD_DLL_FLAG)
ifeq ($(CIMOMBUILD),cmpi)
	CPPFLAGS += -DCMPI_PLATFORM_LINUX_GENERIC_GNU=1
endif

ifdef INTEL_I18N
INCS += -I$(I18N_INCLUDE_DIR)
LIBS += -l$(I18N_LIB_NAME)
endif
# ---- RECIPES -------------------------------------------------------------------------------------
all :  
	$(MAKE) $(JOBCOUNT) $(OBJECT_MODULE_DIR)
	$(MAKE) $(JOBCOUNT) $(SUBMODULE_DIRS) 
	$(MAKE) $(JOBCOUNT) $(TARGETNAME)
	$(MAKE) create_mof
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
	
create_mof:
	## -----------------------------
	## Create registration mofs
	## -----------------------------
	$(RM) $(BUILD_DIR)/profile_registration.mof
	$(COPY) mof/profile_registration.mof  $(BUILD_DIR)/profile_registration.mof
	sed -i 's/<NVM_WBEM_PREFIX>/$(WBEM_PREFIX_INPUT)/g' $(BUILD_DIR)/profile_registration.mof
ifdef BUILD_WINDOWS
	$(RM) $(BUILD_DIR)/register.mof
	$(COPY) cimom/wmi/register.mof  $(BUILD_DIR)/register.mof
	sed -i 's/<NVM_WBEM_PREFIX>/$(WBEM_PREFIX_INPUT)/g' $(BUILD_DIR)/register.mof
else # Linux
	$(COPY) cimom/cmpi/pegasus_register.mof  $(BUILD_DIR)/pegasus_register.mof
	$(COPY) cimom/cmpi/sfcb_register.reg  $(BUILD_DIR)/INTEL_NVDIMM.reg
	sed -i 's/<NVM_WBEM_PREFIX>/$(WBEM_PREFIX_INPUT)/g' $(BUILD_DIR)/pegasus_register.mof
	sed -i 's/<NVM_WBEM_PREFIX>/$(WBEM_PREFIX_INPUT)/g' $(BUILD_DIR)/INTEL_NVDIMM.reg
endif
	## -----------------------------
	## Create class definition mofs
	## -----------------------------
	# DMTF CIM Schema
	$(RM) $(BUILD_DIR)/intelwbem.mof
	echo // DMTF CIM Schema 2.44.1 > $(BUILD_DIR)/intelwbem.mof
	cat mof/cim_schema_2.44.1_combined.mof >> $(BUILD_DIR)/intelwbem.mof
	echo >> $(BUILD_DIR)/intelwbem.mof
	# SNIA CIM Schema
	echo // SNIA CIM Schema 16Rev4-Updated >> $(BUILD_DIR)/intelwbem.mof	
	cat mof/snia_mofs_16Rev4-updated.mof >> $(BUILD_DIR)/intelwbem.mof
	echo >> $(BUILD_DIR)/intelwbem.mof
	# Intel CIM Schema
	echo // Intel CIM Schema >> $(BUILD_DIR)/intelwbem.mof
	cat mof/class_def.mof >> $(BUILD_DIR)/intelwbem.mof
	sed -i 's/<NVM_WBEM_PREFIX>/$(WBEM_PREFIX_INPUT)/g' $(BUILD_DIR)/intelwbem.mof
	# ---------------------------------------------------------------------------------------------
	# Change mof file to be CIMOM appropriate.
	# The following sed commands rely on the <WMI> and <CMPI> tags to indicate pieces that should
	# be removed or stay. The opening and closing WMI/CMPI tags must be on the same line.
	# Note: the ".bak" value to the -i option in sed seems to be required on Windows for some reason
	# ---------------------------------------------------------------------------------------------
ifdef BUILD_WINDOWS
	# get rid of CMPI Specifics
	sed -i".bak"  's/<CMPI>\(.*\)<\/CMPI>//' $(BUILD_DIR)/intelwbem.mof
	# Keep WMI Specifics
	sed -i".bak"  's/<WMI>\(.*\)<\/WMI>/\1/' $(BUILD_DIR)/intelwbem.mof
	$(RM) $(BUILD_DIR)/intelwbem.mof.bak
else
	# get rid of WMI Specifics
	sed -i  's/<WMI>\(.*\)<\/WMI>//' $(BUILD_DIR)/intelwbem.mof
	# Keep CMPI Specifics
	sed -i  's/<CMPI>\(.*\)<\/CMPI>/\1/' $(BUILD_DIR)/intelwbem.mof
endif
ifndef BUILD_WINDOWS
	#we need to remove "SubType" for cmpi which understands datetime intervals better than WMI
	cat $(BUILD_DIR)/intelwbem.mof | sed 's/SubType("interval"), //' > $(BUILD_DIR)/intelwbem_new.mof
	mv  $(BUILD_DIR)/intelwbem_new.mof $(BUILD_DIR)/intelwbem.mof

ifdef BUILD_ESX
	# sfcb on ESX complains about qualifier redefinitions, so use only SFCB-friendly qualifiers
	cat mof/sfcb_qualifiers.mof > $(BUILD_DIR)/sfcb_intelwbem.mof
else
	echo "" > $(BUILD_DIR)/sfcb_intelwbem.mof
endif
	cat $(BUILD_DIR)/intelwbem.mof >> $(BUILD_DIR)/sfcb_intelwbem.mof
	
	# profile registration doesn't need qualifiers for sfcb
	$(COPY) $(BUILD_DIR)/profile_registration.mof $(BUILD_DIR)/sfcb_profile_registration.mof
	sed -i".bak" '1,8d' $(BUILD_DIR)/sfcb_profile_registration.mof
	$(RM) $(BUILD_DIR)/sfcb_profile_registration.mof.bak
endif

	# add qualifiers for linux/windows
	cat mof/qualifiers.mof > $(BUILD_DIR)/intelwbem_with_qualifiers.mof
	cat $(BUILD_DIR)/intelwbem.mof >> $(BUILD_DIR)/intelwbem_with_qualifiers.mof
	mv $(BUILD_DIR)/intelwbem_with_qualifiers.mof $(BUILD_DIR)/intelwbem.mof
	
unittest:
	$(MAKE) -C $(UNITTEST) all
	
test:
ifndef ESX_BUILD # can't run ESX tests on build system
	$(MAKE) -C $(UNITTEST) test
endif
	
i18n:
	$(GETTEXT) NvmStrings.h # Translate error messages
	
clean:
	rm -f $(TARGET) $(OBJNAMES)
	$(MAKE) -C $(UNITTEST) clean
	
clobber :
	$(RMDIR) $(OBJECT_MODULE_DIR)

sourcedrop: 
	$(MKDIR) $(SOURCEDROP_DIR)/src/wbem
	$(COPY) makefile $(SOURCEDROP_DIR)/src/wbem
	for dir in $(SUBMODULES); do \
		$(MKDIR) $(SOURCEDROP_DIR)/src/wbem/$${dir}; \
		$(COPY) $${dir}/*.* $(SOURCEDROP_DIR)/src/wbem/$${dir}/; \
	done
	$(MKDIR) $(SOURCEDROP_DIR)/src/wbem/mof
	$(COPY) mof/*.mof $(SOURCEDROP_DIR)/src/wbem/mof/

ifndef BUILD_WINDOWS
	$(MKDIR) $(SOURCEDROP_DIR)/src/wbem/cimom/cmpi
	$(COPY) cimom/cmpi/* $(SOURCEDROP_DIR)/src/wbem/cimom/cmpi/
endif
	
.PHONY : all turncovon turncovoff unittest test i18n clean sourcedrop
