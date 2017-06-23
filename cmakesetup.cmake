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

if (NOT BUILDNUM)
	execute_process(COMMAND git describe --abbrev=0 OUTPUT_VARIABLE BUILDNUM)
	if ("${BUILDNUM}" MATCHES "^([a-zA-Z-]*)(.*)$")
		#replace

		string(REGEX REPLACE "^([a-zA-Z-]+)" "" BUILDNUM "${BUILDNUM}")
		string(REGEX REPLACE "\n$" "" BUILDNUM "${BUILDNUM}")
	else ()
		execute_process(COMMAND pwd OUTPUT_VARIABLE BUILDNUM)
		if ("NOT ${BUILDNUM}" MATCHES "^v([0-9]+)\\.([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
			set(BUILDNUM 99.99.99.9999)
		endif()
	endif()
endif()

string(REPLACE "." ";" VERSION_LIST "${BUILDNUM}")

#ToDo: windows needs list as "${Listname}"
list(GET VERSION_LIST 0 VERSION_MAJOR)
list(GET VERSION_LIST 1 VERSION_MINOR) 
list(GET VERSION_LIST 2 VERSION_HOTFIX) 
list(GET VERSION_LIST 3 VERSION_BUILDNUM) 

##install

#Target names
set(MARKETING_PRODUCT_NAME ixpdimm_sw)
set(API_LIB_SONAME libixpdimm)
set(API_LIB_NAME ixpdimm)
set(CORE_LIB_SONAME libixpdimm-core)
set(CORE_LIB_NAME ixpdimm-core)
set(CLI_LIB_SONAME libixpdimm-cli)
set(CIM_LIB_SONAME libixpdimm-cim)
set(CIM_LIB_NAME ixpdimm-cim)
set(CLI_NAME ixpdimm-cli)
set(MONITOR_NAME ixpdimm-monitor)
set(CIM_NAMESPACE intelwbem)

set(SCRIPTS_PATH ${ROOT}/scripts)

if (LNX_BUILD)
        #Change to conditionally set        
        set(LIB_BUILD_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.0)
		set(SO_BUILD_VERSION ${VERSION_MAJOR})
		set(LINUX_PRODUCT_NAME  ${MARKETING_PRODUCT_NAME})
		set(PRODUCT_DATADIR "/var/lib/${LINUX_PRODUCT_NAME}")
		
		set(MGMT_ENV_DIR "/opt/mgmt_env")
		# doxygen
		set(DOXYGEN "${MGMT_ENV_DIR}/doxygen/doxygen")
		
elseif (WIN_BUILD)
		set(MGMT_ENV_DIR "C:/mgmt_env")
		# doxygen
		set(DOXYGEN "${MGMT_ENV_DIR}/doxygen/doxygen.exe")
		set(WINDMC c:/mgmt_env/mingw_w64/bin/windmc.exe)
		set(WINDRES c:/mgmt_env/mingw_w64/bin/windres.exe)
		set(OBJECT_MODULE_DIR ${ROOT}/output/obj/${OS_TYPE}/${ADAPTER_TYPE}/${BUILD_TYPE})
elseif (ESX_BUILD)
		set(MGMT_ENV_DIR "/opt/mgmt_env")
endif()

#ToDo: Compiler setting ned to go in main makefile
# test coverage tool
if (CCOV)
	set(ENV{COVFILE} "${ROOT}/bste.cov")
		
	# Prevent conflicts with other copies of this project on the system
	set(COVBUILDZONE ${OUTPUT_DIR})
	set(BULLSEYE_DIR ${MGMT_ENV_DIR}/bullseye)
	#include(${BULLSEYE_DIR}/bullseye.mk)
	# force bullseye to update coverage metrics at termination of executables
	# instead of during execution
	set(COVAUTOSAVE 0)
	# Copy the coverage file into the build dir when it's done
	set(COVFILE_COPY ${OUTPUT_DIR}/bste.cov)
	
	# Blank coverage file for validation
	set(COVFILE_VAL_COPY ${OUTPUT_DIR}/val.cov)
	
	find_program(BULLSEYE_COV_ENABLE NAMES cov01 PATHS ${BULLSEYE_DIR}/bin NO_DEFAULT_PATH)
	set (RES 1)
	execute_process(COMMAND ${BULLSEYE_COV_ENABLE} -1 RESULT_VARIABLE RES)
	message(RES: ${RES})
else ()
	set(CMAKE_C_COMPILER gcc)
	set(CMAKE_CXX_COMPILER g++)
endif ()

