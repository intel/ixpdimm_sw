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
# Makefile the Intel NVM-DIMM management software

ROOT_DIR := .
include build.mk

# Generate an absolute path for the root dir
ROOT_DIR_PATH := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
# Remove the trailing slash
export ROOT_DIR_PATH := $(patsubst %/,%,$(ROOT_DIR_PATH))

#settings 
FLAGS := SKIP_UNITTESTS=1 ADD_MANUFACTURING=0
I18N_TARGET := 
ifdef INTERNATIONALIZE #perform string translations
I18N_TARGET := i18n
endif

#external dependencies
CMPI_INCLUDE_DIR ?= ../external/cmpi/include
FW_INCLUDE_DIR ?= ../external/fw/include
OPENSSL_INCLUDE_DIR ?= ../external/openssl/include
OPENSSL_LIB_DIR ?= ../external/openssl/lib
SQLITE_INCLUDE_DIR ?= ../external/sqlite/include
SQLITE_LIB_DIR ?= ../external/sqlite/lib
ZLIB_INCLUDE_DIR ?= ../external/zlib/include
ZLIB_LIB_DIR ?= ../external/zlib/lib
OS_INCLUDE_DIR ?= ../external/os/include
OS_LIB_DIR ?= ../external/os/lib
RAPIDXML_INCLUDE_DIR ?= ../external/rapidxml/include
GETTEXT_EXE_DIR ?= ../external/gettext
ESX_DIR ?= ../external/esx
INTEL_CLI_FRAMEWORK_DIR ?= ../external/intel_cli_framework
INTEL_CIM_FRAMEWORK_DIR ?= ../external/intel_cim_framework
INTEL_I18N_DIR ?= ../external/intelnvmi18n

#build targets
all : extern libcopy 
	$(MAKE) -C src/common $(I18N_TARGET) all $(FLAGS)
	$(MAKE) -C src/lib $(I18N_TARGET) all $(FLAGS)
	$(MAKE) -C src/core $(I18N_TARGET) all $(FLAGS)
	$(MAKE) -C src/wbem $(I18N_TARGET) all $(FLAGS)
	$(MAKE) -C src/cli $(I18N_TARGET) all $(FLAGS)
	$(MAKE) -C src/monitor all $(FLAGS)

extern: 
ifndef BUILD_LINUX
	$(MKDIR) $(EXTERN_DIR)
	$(MKDIR) $(EXTERN_LIB_DIR)
	
ifndef BUILD_WINDOWS
	$(MKDIR) $(EXTERN_DIR)/cmpi/include/cmpi
	$(COPY) $(CMPI_INCLUDE_DIR)/* $(EXTERN_DIR)/cmpi/include/cmpi
endif
	
	$(MKDIR) $(EXTERN_DIR)/os_headers/$(OS_TYPE)
	$(COPY) $(OS_INCLUDE_DIR)/* $(EXTERN_DIR)/os_headers/$(OS_TYPE)/
	-$(COPY) $(OS_LIB_DIR)/* $(EXTERN_LIB_DIR)/
	
	$(MKDIR) $(EXTERN_DIR)/openssl/include/openssl
	$(COPY) $(OPENSSL_INCLUDE_DIR)/* $(EXTERN_DIR)/openssl/include/openssl/
	$(MKDIR) $(EXTERN_LIB_DIR)/openssl/openssl
	$(COPY) $(OPENSSL_LIB_DIR)/* $(EXTERN_LIB_DIR)/openssl/openssl/
	
	$(MKDIR) $(EXTERN_DIR)/sqlite
	$(COPY) $(SQLITE_INCLUDE_DIR)/* $(EXTERN_DIR)/sqlite/
	$(MKDIR) $(EXTERN_LIB_DIR)/sqlite
	$(COPY) $(SQLITE_LIB_DIR)/* $(EXTERN_LIB_DIR)/sqlite/
	
	$(MKDIR) $(EXTERN_DIR)/zlib
	$(COPY) $(ZLIB_INCLUDE_DIR)/* $(EXTERN_DIR)/zlib/
	$(MKDIR) $(EXTERN_LIB_DIR)/zlib
	$(COPY) $(ZLIB_LIB_DIR)/* $(EXTERN_LIB_DIR)/zlib/
	
	$(MKDIR) $(EXTERN_DIR)/intel_cli_framework/$(OS_TYPE)_$(BUILD_TYPE) 
	$(COPY) $(INTEL_CLI_FRAMEWORK_DIR)/lib/lib* $(EXTERN_DIR)/intel_cli_framework/$(OS_TYPE)_$(BUILD_TYPE)/
	$(MKDIR) $(EXTERN_DIR)/intel_cli_framework/include 
	$(COPY) $(INTEL_CLI_FRAMEWORK_DIR)/include/* $(EXTERN_DIR)/intel_cli_framework/include/
	
	$(MKDIR) $(EXTERN_DIR)/intel_cim_framework/$(OS_TYPE)_$(BUILD_TYPE) 
	$(COPY) $(INTEL_CIM_FRAMEWORK_DIR)/lib/lib* $(EXTERN_DIR)/intel_cim_framework/$(OS_TYPE)_$(BUILD_TYPE)/
	$(MKDIR) $(EXTERN_DIR)/intel_cim_framework/include 
	$(COPY) $(INTEL_CIM_FRAMEWORK_DIR)/include/* $(EXTERN_DIR)/intel_cim_framework/include/
	
	$(MKDIR) $(EXTERN_DIR)/intel_i18n/$(OS_TYPE)_$(BUILD_TYPE) 
	$(COPY) $(INTEL_I18N_DIR)/lib/lib* $(EXTERN_DIR)/intel_i18n/$(OS_TYPE)_$(BUILD_TYPE)/
	$(MKDIR) $(EXTERN_DIR)/intel_i18n/include 
	$(COPY) $(INTEL_I18N_DIR)/include/* $(EXTERN_DIR)/intel_i18n/include/

endif	
ifdef $(INTERNATIONALIZE)
	$(MKDIR) tools/gettext
	$(COPY) $(GETTEXT_EXE_DIR)/* tools/gettext/
endif 

ifdef BUILD_ESX
	$(MKDIR) $(EXTERN_DIR)/esx
	$(COPY) $(ESX_DIR)/* $(EXTERN_DIR)/esx/
endif	
		
libcopy:
	$(MKDIR) $(BUILD_DIR)
	$(MKDIR) $(BUILT_TOOLS_DIR)
ifndef BUILD_LINUX
	$(COPY) $(EXTERN_LIB_DIR)/sqlite/lib* $(BUILD_DIR)/
	$(COPY) $(EXTERN_LIB_DIR)/openssl/openssl/lib* $(BUILD_DIR)/
	$(COPY) $(EXTERN_LIB_DIR)/zlib/lib* $(BUILD_DIR)/
	$(COPY) $(EXTERN_DIR)/intel_cli_framework/$(OS_TYPE)_$(BUILD_TYPE)/lib* $(BUILD_DIR)
	$(COPY) $(EXTERN_DIR)/intel_cim_framework/$(OS_TYPE)_$(BUILD_TYPE)/lib* $(BUILD_DIR)
	-$(COPY) $(EXTERN_LIB_DIR)/*.* $(BUILD_DIR)/
endif
ifdef BUILD_ESX #make soft links
	cd $(BUILD_DIR); $(RM) libsqlite3.so.1; $(SOFTLINK) libsqlite3.so.1.0.0 libsqlite3.so.1
	cd $(BUILD_DIR); $(RM) libsqlite3.so; $(SOFTLINK) libsqlite3.so.1.0.0  libsqlite3.so
	cd $(BUILD_DIR); $(SOFTLINK) libssl.so.1.0.0 libssl.so
	cd $(BUILD_DIR); $(SOFTLINK) libcrypto.so.1.0.0 libcrypto.so
	cd $(BUILD_DIR); $(SOFTLINK) libz.so.1.2.5 libz.so
endif
	
install:
ifdef BUILD_LINUX
	# complete the paths for the files to be installed
	$(eval LIB_FILES := $(addprefix $(BUILD_DIR)/, $(LIB_FILES)))
	$(eval CIM_LIB_FILES := $(addprefix $(BUILD_DIR)/, $(CIM_LIB_FILES)))
	$(eval INCLUDE_FILES := $(addprefix $(BUILD_DIR)/, $(INCLUDE_FILES)))
	$(eval BIN_FILES := $(addprefix $(BUILD_DIR)/, $(BIN_FILES)))
	$(eval INIT_FILES := $(addprefix $(BUILD_DIR)/, $(INIT_FILES)))
	$(eval DATADIR_FILES := $(addprefix $(BUILD_DIR)/, $(DATADIR_FILES)))
	$(eval PEGASUS_MOF_FILES := $(addprefix $(BUILD_DIR)/, $(PEGASUS_MOF_FILES)))
	$(eval SFCB_MOF_FILES := $(addprefix $(BUILD_DIR)/, $(SFCB_MOF_FILES)))
	$(eval SFCB_REG_FILE := $(addprefix $(BUILD_DIR)/, $(SFCB_REG_FILE)))
	$(eval MANPAGE_GZ_FILES := $(addprefix $(BUILD_DIR)/, $(MANPAGE_GZ_FILES)))
	
	# install files into LIB_DIR
	$(MKDIR) $(RPM_ROOT)$(LIB_DIR)
	$(COPY) $(LIB_FILES) $(RPM_ROOT)$(LIB_DIR)

	# install files into CIM_LIB_DIR
	$(MKDIR) $(RPM_ROOT)$(CIM_LIB_DIR)
	$(COPY) $(CIM_LIB_FILES) $(RPM_ROOT)$(CIM_LIB_DIR)
	
	# install files into INCLUDE_DIR
	$(MKDIR) $(RPM_ROOT)$(INCLUDE_DIR)
	$(COPY) $(INCLUDE_FILES) $(RPM_ROOT)$(INCLUDE_DIR)

	# install files into BIN_DIR
	$(MKDIR) $(RPM_ROOT)$(BIN_DIR)
	$(COPY) $(BIN_FILES) $(RPM_ROOT)$(BIN_DIR)
	
	#install files into UNIT_DIR
	$(MKDIR) $(RPM_ROOT)$(UNIT_DIR)
	$(COPY) $(INIT_FILES) $(RPM_ROOT)$(UNIT_DIR)
	
	#install files into DATADIR
	$(MKDIR) $(RPM_ROOT)$(PRODUCT_DATADIR)
	$(COPY) $(DATADIR_FILES) $(RPM_ROOT)$(PRODUCT_DATADIR)

	#install manpage files into MAN8_DIR
	$(MKDIR) $(RPM_ROOT)$(MAN8_DIR)
	$(MOVE) $(MANPAGE_GZ_FILES) $(RPM_ROOT)$(MAN8_DIR)
	
	#install Pegasus Files
	$(MKDIR) $(RPM_ROOT)$(PEGASUS_MOFDIR)
	$(COPY) $(PEGASUS_MOF_FILES) $(RPM_ROOT)$(PEGASUS_MOFDIR)
	
	#install SFCB Files
	$(MKDIR) $(RPM_ROOT)$(SFCB_DIR)
	$(COPY) $(SFCB_MOF_FILES) $(RPM_ROOT)$(SFCB_DIR)
	$(COPY) $(SFCB_REG_FILE) $(RPM_ROOT)$(SFCB_DIR)
	
	-$(MKDIR) $(RPM_ROOT)$(SYSCONF_DIR)/ld.so.conf.d
	echo $(CIM_LIB_DIR) > $(RPM_ROOT)$(SYSCONF_DIR)/ld.so.conf.d/$(LINUX_PRODUCT_NAME)-$(HW_ARCH).conf
else ifdef BUILD_ESX
	#
	# directory structure for installing our stuff on ESX
	#
	# the main staging area for building the vib
	$(eval INSTALL_DIR := $(BUILD_DIR)/vib_stage)
	# where all supporting libraries goes
	$(eval ESX_SUPPORT_DIR := $(INSTALL_DIR)/$(ESX_SUPPORT_DIR))
	
	# esx specific directories ...
	# esxcli plug in
	$(eval CLI_PLUGIN_DIR := $(INSTALL_DIR)/usr/lib/vmware/esxcli/ext)
	# esx vm-support plug in
	$(eval SUPPORT_DIR := $(INSTALL_DIR)/etc/vmware/vm-support)
	# CIM Provider library
	$(eval CIM_LIB_DIR := $(INSTALL_DIR)/usr/lib/cim)
	# dmtf base mofs go here
	$(eval CIM_BASE_MOF_DIR := $(INSTALL_DIR)/share/sfcb/CIM)
	
	# where the mofs go to be processed by sfcbrepos	
	$(eval CIM_STAGE := $(INSTALL_DIR)/var/lib/sfcb/stage)
	$(eval CIM_STAGE_NAMESPACE := $(CIM_STAGE)/mofs/intel-0-0/root/intelwbem)
	$(eval CIM_STAGE_REGISTRATION := $(CIM_STAGE)/regs)
	# after processing by sfcbrepos
	$(eval CIM_REGISTRATION := $(INSTALL_DIR)/var/lib/sfcb/registration)
	
	# create the directory structures
	$(MKDIR) $(SUPPORT_DIR)
	$(MKDIR) $(CIM_LIB_DIR)
	$(MKDIR) $(CIM_STAGE_NAMESPACE)
	$(MKDIR) $(CIM_STAGE_REGISTRATION)
	$(MKDIR) $(CIM_BASE_MOF_DIR)
	$(MKDIR) $(CIM_REGISTRATION)
	$(MKDIR) $(ESX_SUPPORT_DIR)
	$(MKDIR) $(CLI_PLUGIN_DIR)

	#
	# copy files to the right spots
	#
	# move the mof related stuff
	$(COPY) $(ROOT_DIR)/external/esx/esx_mof.tgz $(CIM_BASE_MOF_DIR)
	cd $(CIM_BASE_MOF_DIR); tar -xvf esx_mof.tgz
	$(COPY) $(ROOT_DIR)/external/esx/default.reg $(CIM_STAGE)
	$(COPY) $(BUILD_DIR)/sfcb_intelwbem.mof $(CIM_STAGE_NAMESPACE)
	chmod 644 $(CIM_STAGE_NAMESPACE)/sfcb_intelwbem.mof
	$(COPY) $(BUILD_DIR)/INTEL_NVDIMM.reg $(CIM_STAGE_REGISTRATION)/intelwbem.reg
	chmod 644 $(CIM_STAGE_REGISTRATION)/intelwbem.reg
	$(COPY) $(BUILD_DIR)/$(CLI_NAME) $(ESX_SUPPORT_DIR)
	$(COPY) $(BUILD_DIR)/$(MONITOR_NAME) $(ESX_SUPPORT_DIR)
	$(COPY) $(BUILD_DIR)/$(API_LIB_SONAME).so* $(ESX_SUPPORT_DIR)
	$(COPY) $(BUILD_DIR)/$(CORE_LIB_SONAME).so* $(ESX_SUPPORT_DIR)
	$(COPY) $(BUILD_DIR)/$(CLI_LIB_SONAME).so* $(ESX_SUPPORT_DIR)
	$(COPY) $(BUILD_DIR)/$(CIM_LIB_SONAME).so* $(ESX_SUPPORT_DIR) # for CLI
	$(COPY) $(BUILD_DIR)/$(CIM_FRAMEWORK_LIB_SONAME).so* $(ESX_SUPPORT_DIR) # for CLI
	$(COPY) $(BUILD_DIR)/$(CIM_LIB_SONAME).so* $(CIM_LIB_DIR) # for SFCB
	$(COPY) $(BUILD_DIR)/$(CIM_FRAMEWORK_LIB_SONAME).so* $(CIM_LIB_DIR) # for SFCB
	$(COPY) $(BUILD_DIR)/$(CLI_FRAMEWORK_LIB_SONAME).so* $(ESX_SUPPORT_DIR)
	$(COPY) $(BUILD_DIR)/libsqlite3.so* $(ESX_SUPPORT_DIR)
	$(COPY) $(BUILD_DIR)/libssl.so* $(ESX_SUPPORT_DIR)
	$(COPY) $(BUILD_DIR)/libcrypto.so* $(ESX_SUPPORT_DIR)
	$(COPY) $(BUILD_DIR)/libz.so* $(ESX_SUPPORT_DIR)
	$(COPY) $(BUILD_DIR)/$(I18N_LIB_SONAME).so* $(ESX_SUPPORT_DIR)
	$(COPY) $(BUILD_DIR)/public.rev0.pem $(ESX_SUPPORT_DIR)
	# must provide own C++ runtime - CPP_RUNTIME defined in build.mk
	$(COPY) $(CPP_RUNTIME) $(ESX_SUPPORT_DIR)

	# the vm-support plug-in descriptor
	$(COPY) esx_gather-support-plugin.mfx $(SUPPORT_DIR)/intel_pmem_support.mfx

	# the ESXCLI plug-in descriptor
	$(COPY) $(ROOT_DIR)/src/cli/esx/esxcli-nvmcli.xml $(CLI_PLUGIN_DIR)
	
	#
	# modify the descriptor file to include the version number
	# 
	sed "s|<version>0.1.0-000000</version>|<version>$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_HOTFIX)-$(VERSION_BUILDNUM)</version>|" \
		<esx_install_descriptor.xml >esx_install_descriptor_tmp.xml
	#
	# process SFCB MOF repository
	#
	export LD_LIBRARY_PATH=$(ROOT_DIR)/external/esx; sfcbrepos -f -s $(CIM_STAGE) \
		-r $(CIM_REGISTRATION) \
		-c $(CIM_BASE_MOF_DIR)/build
	# temporary, don't put files here
	$(RMDIR) $(INSTALL_DIR)/share
	$(RMDIR) $(CIM_STAGE)
	$(MOVE) $(CIM_REGISTRATION)/providerRegister \
		$(CIM_REGISTRATION)/vmw_intelnvm-providerRegister
	#
	# create the VIB
	#
	vibauthor --create --force --descriptor=esx_install_descriptor_tmp.xml --stage-dir=$(INSTALL_DIR)
	$(MOVE) intel_nvmdimm_mgmt*.vib $(BUILD_DIR)
	$(RM) esx_install_descriptor_tmp.xml
	$(RMDIR) $(INSTALL_DIR)
endif

uninstall:
ifdef BUILD_LINUX
	# complete the paths for the files to be uninstalled
	$(eval LIB_FILES := $(addprefix $(RPM_ROOT)$(LIB_DIR)/, $(LIB_FILES)))
	$(eval CIM_LIB_FILES := $(addprefix $(RPM_ROOT)$(CIM_LIB_DIR)/, $(CIM_LIB_FILES)))
	$(eval INCLUDE_FILES := $(addprefix $(RPM_ROOT)$(INCLUDE_DIR)/, $(INCLUDE_FILES)))
	$(eval BIN_FILES := $(addprefix $(RPM_ROOT)$(BIN_DIR)/, $(BIN_FILES)))
	$(eval DATADIR_FILES := $(addprefix $(RPM_ROOT)$(PRODUCT_DATADIR)/, $(DATADIR_FILES)))
	$(eval PEGASUS_MOF_FILES := $(addprefix $(RPM_ROOT)$(PEGASUS_MOFDIR)/, $(PEGASUS_MOF_FILES)))
	$(eval SFCB_MOF_FILES := $(addprefix $(RPM_ROOT)$(SFCB_DIR)/, $(SFCB_MOF_FILES)))
	$(eval SFCB_REG_FILE := $(addprefix $(RPM_ROOT)$(SFCB_DIR)/, $(SFCB_REG_FILE)))
	$(eval MANPAGE_GZ_FILES := $(addprefix $(RPM_ROOT)$(MAN8_DIR)/, $(MANPAGE_GZ_FILES)))
	
	# uninstall files from LIB_DIR	
	$(RM) $(LIB_FILES)

	# uninstall files from CIM_LIB_DIR	
	$(RM) $(CIM_LIB_FILES)
	
	# uninstall files from INCLUDE_DIR
	$(RM) $(INCLUDE_FILES)
		
	# uninstall files from BIN_DIR
	$(RM) $(BIN_FILES)
	
	# uninstall monitor service
	$(RM) $(RPM_ROOT)$(INITD_DIR)/$(MONITOR_NAME)	
	
	#uninstall Pegasus files
	$(RM) $(PEGASUS_MOF_FILES)
	-rmdir $(RPM_ROOT)$(PEGASUS_MOFDIR)
	-rmdir $(RPM_ROOT)$(PRODUCT_DATADIR)/Pegasus
	
	#uninstall SFCB files
	$(RM) $(SFCB_MOF_FILES)
	$(RM) $(SFCB_REG_FILE)
	-rmdir $(RPM_ROOT)$(SFCB_DIR)
	
	$(RM) $(RPM_ROOT)$(SYSCONF_DIR)/ld.so.conf.d/$(LINUX_PRODUCT_NAME)-$(HW_ARCH).conf
	
	# uninstall shared files
	$(RM) $(DATADIR_FILES)
	-rmdir $(RPM_ROOT)$(PRODUCT_DATADIR)
	
	# uninstall man pages
	$(RM) $(MANPAGE_GZ_FILES)
	-rmdir $(RPM_ROOT)$(MAN8_DIR)
endif

clean : 
	$(RMDIR) $(OUTPUT_DIR)

rpm :
	#Make the Directories
	$(MKDIR) $(RPMBUILD_DIR) $(RPMBUILD_DIR)/BUILD $(RPMBUILD_DIR)/SOURCES $(RPMBUILD_DIR)/RPMS \
				$(RPMBUILD_DIR)/SRPMS $(RPMBUILD_DIR)/SPECS $(RPMBUILD_DIR)/BUILDROOT \
				$(RPMBUILD_DIR)/BUILD/$(MARKETING_PRODUCT_NAME)-$(BUILDNUM)

	#Copy Spec File
	$(COPY) install/linux/$(LINUX_DIST)-release/*.spec $(RPMBUILD_DIR)/SPECS/$(MARKETING_PRODUCT_NAME).spec
	#Update the Spec file
	$(SED) -i 's/^%define build_version .*/%define build_version $(BUILDNUM)/g' $(RPMBUILD_DIR)/SPECS/$(MARKETING_PRODUCT_NAME).spec

	#Archive the directory
	git archive --format=tar --prefix="$(MARKETING_PRODUCT_NAME)-$(BUILDNUM)/" HEAD | gzip -c > $(RPMBUILD_DIR)/SOURCES/$(MARKETING_PRODUCT_NAME)-$(BUILDNUM).tar.gz
	#rpmbuild
	$(RPMBUILD) -ba $(RPMBUILD_DIR)/SPECS/$(MARKETING_PRODUCT_NAME).spec --define "_topdir $(RPMBUILD_DIR)" --define "cflag $(CFLAGS_EXTERNAL)"

.PHONY : all clean install rpm
