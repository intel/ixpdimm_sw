#
# Copyright (c) 2015 2016 2017 2018, Intel Corporation
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

cmake_minimum_required(VERSION 2.8.11)
project(cr_mgmt)

set(CMAKE_VERBOSE_MAKEFILE on)

SET(CMAKE_SKIP_BUILD_RPATH TRUE)

if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION} GREATER 2.8.12)
	cmake_policy(SET CMP0040 OLD)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake/Find")

get_filename_component(ROOT ./ ABSOLUTE)

if(ESX_BUILD)
	set(OS_TYPE esx)
	set(FILE_PREFIX esx)
	set(ADAPTER_PREFIX esx)
elseif(UNIX)
	set(OS_TYPE linux)
	set(FILE_PREFIX lnx)
	set(ADAPTER_PREFIX lnx)
	set(LNX_BUILD 1)

	find_package(OpenSSL REQUIRED)
	find_package(ZLIB REQUIRED)
	find_package(numactl REQUIRED)
	find_package(sblim-cmpi REQUIRED)
	find_package(Threads REQUIRED)
	find_package(math REQUIRED)

	find_package(PkgConfig)
	pkg_check_modules(INVM-I18N REQUIRED libinvm-i18n>=01.01)
	pkg_check_modules(INVM-CIM REQUIRED libinvm-cim>=01.01)
	pkg_check_modules(INVM-CLI REQUIRED libinvm-cli>=01.01)
	pkg_check_modules(NDCTL REQUIRED libndctl>=58.2)
	pkg_check_modules(SQLITE3 REQUIRED sqlite3)
	pkg_check_modules(LIBKMOD REQUIRED libkmod)
	pkg_check_modules(SYSTEMD systemd)
elseif(MINGW OR MSVC)
	set(WIN_BUILD 1)
	set(OS_TYPE windows)
	set(FILE_PREFIX win)
	set(ADAPTER_PREFIX win)

	set(EXTERN_DIR ${ROOT}/external_libs) #TODO:MSVC
	set(USE_INSTALLED_FRAMEWORK BOOL ON)
endif()

option(BUILD_STATIC "Build libraries as STATIC. This will override LIB_TYPE" OFF)
if(BUILD_STATIC)
	add_definitions(-DBUILD_STATIC)
	set(LIB_TYPE STATIC)
else()
	set(LIB_TYPE SHARED)
endif()

option(RELEASE "Release build" OFF)
if(RELEASE)
	set(BUILD_TYPE release)
else()
	set(BUILD_TYPE debug)
endif()

option(BUILD_SIM "Build in simulator mode." OFF)
if(BUILD_SIM)
	set(SIM_BUILD ON)
	set(ADAPTER_TYPE simulator)
	set(ADAPTER_PREFIX sim)
else()
	set(SIM_BUILD OFF)
	set(ADAPTER_TYPE real)
endif()

option(CCOV "Build in code coverage mode." OFF)
if(CCOV)
	set(OUTPUT_DIR ${ROOT}/output/cov/build/${OS_TYPE}/${ADAPTER_TYPE}/${BUILD_TYPE}
		CACHE PATH "Build Path" FORCE)
else()
	set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/output/build/${OS_TYPE}/${ADAPTER_TYPE}/${BUILD_TYPE}
		CACHE PATH "Build Path" FORCE)
endif()

message(ADAPTER_TYPE: ${ADAPTER_TYPE})
message(BUILD_TYPE: ${BUILD_TYPE})
message(OS_TYPE: ${OS_TYPE})
message(LIB_TYPE: ${LIB_TYPE})

set(CMAKE_BINARY_DIR ${OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(SCRIPTS_PATH ${ROOT}/scripts)

include(CMake/setup.cmake)
if(WIN_BUILD)
	include(CMake/windows_setup.cmake)
elseif(ESX_BUILD)
	include(CMake/esx_setup.cmake)
endif()

if(EXISTS ${ROOT}/CMake/internal.cmake AND NOT EXTERNAL)
	set(BUILD_INTERNAL ON)
	include(CMake/internal.cmake)
endif()

# --------------------------------------------------------------------------------------------------
# Project wide defines and flags
# --------------------------------------------------------------------------------------------------
add_definitions(
	-D__WBEM_PREFIX__=Intel_
	-D__ADD_MANUFACTURING__=1
	-D_GNU_SOURCE
	-D__VERSION_MAJOR__=${VERSION_MAJOR}
	-D__VERSION_MINOR__=${VERSION_MINOR}
	-D__VERSION_HOTFIX__=${VERSION_HOTFIX}
	-D__VERSION_BUILDNUM__=${VERSION_BUILDNUM}
	-D__VERSION_NUMBER__=${BUILDNUM}
	-D__GET_LARGE_PCD_OS_PARTITION__=0
	-D__GET_LARGE_PCD_NS_LABEL__=1
	-D__SET_LARGE_FW_UPDATE__=1
	-D__LARGE_PAYLOAD_NOT_SUPPORTED__=0  # there are few commands that works only with large payload
	)

if(BUILD_STATIC)
	add_definitions(
		-DBUILD_STATIC
		)
endif()

if(RELEASE)
	add_definitions(
		-DRELEASE
		)
	if(MSVC)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /O2")
	else()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -fno-strict-aliasing")
	endif()
else()
	if(MSVC)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Od")
	else()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1 -ggdb")
	endif()
endif()

if(LNX_BUILD)
	add_definitions(
		-D_GNU_SOURCE
		-D__LINUX__
		-D__NVM_DLL__
		-DCMPI_PLATFORM_LINUX_GENERIC_GNU=1
		-D__PRODUCT_DATADIR__="${PRODUCT_DATADIR}/"
		)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wformat-security -D_FORTIFY_SOURCE=2 -fstack-protector-all -D_XOPEN_SOURCE=500 -Wall -Wfatal-errors -MMD")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fstack-protector-all -D_XOPEN_SOURCE=500 -Wall -Wfatal-errors -MMD")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -z noexecstack -z relro -z now -pie -fPIC")
endif()

if(WIN_BUILD)
	add_definitions(
		-D_WIN32_WINNT=0x0601
		-DWINVER=0x0600
		-D__EARLY_HW__=0
		-D__INTEL_I18N__
		-D__WINDOWS__
		)

	if(MSVC)
		add_definitions(
			-D__INVM_CLI_DLL__
			-DWIN32_LEAN_AND_MEAN
			-D_CRT_SECURE_NO_WARNINGS #FIXME
		)
		#FIXME: temporarily hide some warnings
		set(MSVC_WARN_DISABLES "/wd4251 /wd4290")

		#FIXME: https://github.com/google/googletest/issues/1111
		set(MSVC_GTEST_TR1_DISABLE_WARN "/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MSVC_WARN_DISABLES} ${MSVC_GTEST_TR1_DISABLE_WARN}")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSVC_WARN_DISABLES} ${MSVC_GTEST_TR1_DISABLE_WARN}")

		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__MSVC__")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__MSVC__")
		set(USE_INSTALLED_FRAMEWORK BOOL ON)
		set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll" ".a")
	else()
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64 -mno-ms-bitfields -static-libstdgcc")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
	endif()
endif()

# --------------------------------------------------------------------------------------------------
# Driver Interface
# --------------------------------------------------------------------------------------------------
file(GLOB DRIVER_INTERFACE_FILES
	src/driver_interface/${ADAPTER_PREFIX}_passthrough.c
	src/driver_interface/passthrough.c
	)

if(WIN_BUILD)
	file(GLOB WIN_DRIVER_INTERFACE_FILES src/lib/win_scm2*.c)
	list(APPEND DRIVER_INTERFACE_FILES ${WIN_DRIVER_INTERFACE_FILES})
endif()

add_library(driver_interface STATIC ${DRIVER_INTERFACE_FILES})

target_include_directories(driver_interface PUBLIC
	src
	src/common
	)

if(LNX_BUILD)
	target_link_libraries(driver_interface ${NDCTL_LIBRARIES})
elseif(WIN_BUILD)
	target_include_directories(driver_interface PUBLIC
		src/lib
		${INVM_I18N_INCLUDE_DIRS}
		)
endif()

# --------------------------------------------------------------------------------------------------
# Firmware Interface
# --------------------------------------------------------------------------------------------------
file(GLOB FIRMWARE_INTERFACE_FILES
	src/firmware_interface/fis_commands.c
	src/firmware_interface/fis_parser.c
	src/firmware_interface/fw_commands.c
	src/firmware_interface/fw_command_printer.c
	src/firmware_interface/fw_command_controller.c
	src/firmware_interface/fw_command_dump.c
	src/firmware_interface/fwcmd_args.c
	)

add_library(firmware_interface STATIC ${FIRMWARE_INTERFACE_FILES})

target_link_libraries(firmware_interface driver_interface)
target_include_directories(firmware_interface PUBLIC
	src/common
	)

#---------------------------------------------------------------------------------------------------
# Common Library
#---------------------------------------------------------------------------------------------------
file(GLOB_RECURSE COMMON_SOURCE_FILES
	src/common/encrypt/*.c
	src/common/file_ops/file_ops.c
	src/common/guid/*.c
	src/common/uid/*.c
	src/common/persistence/*.c
	src/common/smbios/*.c
	src/common/string/*.c
	src/common/system/*.c
	src/common/time/*.c
	src/acpi/acpi.c
	src/acpi/${FILE_PREFIX}_acpi.c
	)

list(APPEND COMMON_SOURCE_FILES
	src/common/file_ops/${FILE_PREFIX}_file_ops.c
	src/common/os/${FILE_PREFIX}_os.c
	)

add_library(${COMMON_LIB_NAME} SHARED
	${COMMON_SOURCE_FILES}
	${schema_file}
	)

if(MSVC)
	# force declspec export when building
	target_compile_definitions(${COMMON_LIB_NAME}
		PRIVATE ${COMMON_LIB_EXPORT_DEF} ${NVM_DLL_DEF})
endif()

target_link_libraries(${COMMON_LIB_NAME}
	${CMAKE_DL_LIBS}
	${CMAKE_THREAD_LIBS_INIT}
	${MATH_LIBRARIES}
	${OPENSSL_CRYPTO_LIBRARY}
	${OPENSSL_SSL_LIBRARY}
	${SQLITE3_LIBRARIES}
	${ZLIB_LIBRARIES}
	)

target_include_directories(${COMMON_LIB_NAME} PUBLIC
	${EXT_INC_OS_HEADERS}
	${INVM-I18N_INCLUDE_DIRS}
	${OPENSSL_INCLUDE_DIR}
	${SQLITE3_INCLUDE_DIRS}
	${ZLIB_INCLUDE_DIRS}
	src/common
	src/lib
	)

set_target_properties(${COMMON_LIB_NAME}
	PROPERTIES
	VERSION ${LIB_BUILD_VERSION}
	SOVERSION ${SO_BUILD_VERSION}
)

if(WIN_BUILD)
	set(win_msgs_header ${ROOT}/src/common/os/win_msgs.h)
	set(win_msgs_rc ${ROOT}/src/common/os/win_msgs.rc)
	set(win_msgs_res ${ROOT}/src/common/os/win_msgs.res)
	set(win_msgs_depends ${ROOT}/src/common/os/win_msgs.mc)
	set(win_msgs_dll ${OUTPUT_DIR}/win_msgs.dll)

	set_source_files_properties(${win_msgs_header} ${win_msgs_rc} ${win_msgs_res}
		PROPERTIES GENERATED TRUE
		)

	add_custom_target(win_msgs_header_generate ALL
		DEPENDS ${win_msgs_header}
		)

	if(MSVC)
		add_custom_command(OUTPUT ${win_msgs_header} ${win_msgs_rc}
			DEPENDS ${win_msgs_depends}
			COMMAND cd ${ROOT}/src/common/os && ${CMAKE_MC_COMPILER} -U win_msgs.mc
			VERBATIM
			COMMENT "Generating win_msgs.h ..."
			)

		add_custom_target(win_msgs_dll_generate ALL
			DEPENDS ${win_msgs_dll})

		add_custom_command(OUTPUT ${win_msgs_dll}
			DEPENDS ${win_msgs_rc}
			COMMAND ${CMAKE_COMMAND} -E make_directory ${OBJECT_MODULE_DIR}/common/os
			COMMAND ${CMAKE_RES_COMPILER} ${win_msgs_rc}
			COMMAND ${VS_LINK} ${win_msgs_res} /MACHINE:AMD64 /NOENTRY /DLL /OUT:${OUTPUT_DIR}/win_msgs.dll
			VERBATIM
			COMMENT "Generating win_msgs.res ..."
			)
	else()
		set_source_files_properties(${win_msgs_er}
			PROPERTIES GENERATED TRUE
			)

		add_custom_command(
			OUTPUT ${win_msgs_header}
			DEPENDS ${win_msgs_depends}
			COMMAND ${CMAKE_COMMAND} -E make_directory ${OBJECT_MODULE_DIR}/common/os
			COMMAND cd ${ROOT}/src/common/os && ${WINDMC} -U win_msgs.mc -r ${OBJECT_MODULE_DIR}/common/os
			COMMAND ${WINDRES} ${OBJECT_MODULE_DIR}/common/os/win_msgs.rc -o ${OBJECT_MODULE_DIR}/common/os/win_msgs.o
			COMMAND ${CMAKE_C_COMPILER} -shared ${OBJECT_MODULE_DIR}/common/os/win_msgs.o -o ${OUTPUT_DIR}/win_msgs.dll
			VERBATIM
			COMMENT "Generating win_msgs.h ..."
			)
	endif()

	target_link_libraries(${COMMON_LIB_NAME}
			ws2_32
			Shlwapi
			setupapi
			version
			)

	if(NOT MSVC)
		target_link_libraries(${COMMON_LIB_NAME}
			iconv
			mswsock
			advapi32
			version
			Gdi32
		 )
	endif()

	add_dependencies(${COMMON_LIB_NAME}
		win_msgs_header_generate
	)
endif()

# --------------------------------------------------------------------------------------------------
# ACPI Library
# --------------------------------------------------------------------------------------------------
file(GLOB ACPI_SRC_FILES
	src/acpi/acpi.c
	src/acpi/${FILE_PREFIX}_acpi.c
	src/acpi/nfit_parser.c
	src/acpi/nfit_dimm.c
	src/acpi/nfit_tables.h
	src/acpi/nfit_printer.c
	src/acpi/nfit_interleave.c
	src/acpi/nfit.c
	src/common/guid/guid.c
	)

add_library(acpi STATIC ${ACPI_SRC_FILES})

target_include_directories(acpi PUBLIC
	${OPENSSL_INCLUDE_DIR}
	src/common
	)

target_link_libraries(acpi
	${OPENSSL_CRYPTO_LIBRARY}
	${OPENSSL_SSL_LIBRARY}
	${ZLIB_LIBRARIES}
	)

# --------------------------------------------------------------------------------------------------
# Native Library
# --------------------------------------------------------------------------------------------------
file(GLOB LIB_SOURCE_FILES
	src/lib/*.c
	)

# remove all adapter files TODO: Why...
file(GLOB_RECURSE TO_REMOVE
	src/lib/lnx_*
	src/lib/win_*
	src/lib/esx_*
	src/lib/sim_*
	)
list(REMOVE_ITEM LIB_SOURCE_FILES ${TO_REMOVE})
# add appropriate adapter files back in
if(SIM_BUILD)
	file(GLOB ADAPTER_FILES src/lib/sim*)
else()
	file(GLOB ADAPTER_FILES src/lib/${FILE_PREFIX}*)
endif()
list(APPEND LIB_SOURCE_FILES ${ADAPTER_FILES})

if(WIN_BUILD AND MSVC)
    list(APPEND LIB_SOURCE_FILES ${ROOT}/src/lib/lib_resources.rc)
endif()

add_library(${API_LIB_NAME} ${LIB_TYPE} ${LIB_SOURCE_FILES})

set_target_properties(${API_LIB_NAME}
	PROPERTIES
	VERSION ${LIB_BUILD_VERSION}
	SOVERSION ${SO_BUILD_VERSION}
	)

target_include_directories(${API_LIB_NAME} PUBLIC
	src
	src/lib
	src/acpi
	)

target_link_libraries(${API_LIB_NAME}
	${COMMON_LIB_NAME}
	${OPENSSL_CRYPTO_LIBRARY}
	${OPENSSL_SSL_LIBRARY}
	${SQLITE3_LIBRARIES}
	${INVM-I18N_LIBRARIES}
	acpi
	)

if(WIN_BUILD)
	if(MSVC)
		include_directories(${API_LIB_NAME} SYSTEM PUBLIC
		${EXT_INC_OS_HEADERS}
		)

	target_include_directories(${API_LIB_NAME} PUBLIC
		${EXT_INC_FW_HEADERS}
		${EXT_INC_OS_HEADERS}
		)

		# force declspec export when building
		target_compile_definitions(${API_LIB_NAME}
			PUBLIC ${NVM_DLL_DEF}
			PRIVATE ${API_LIB_EXPORT_DEF})
	else()
		target_link_libraries(${API_LIB_NAME}
			ws2_32
			mswsock
			advapi32
			version
			Gdi32
			Shlwapi
			iconv
			)
		add_custom_command(
			OUTPUT lib_resources.o
			COMMAND ${CMAKE_COMMAND} -E make_directory ${OBJECT_MODULE_DIR}/lib
			COMMAND cd ${ROOT}/src/lib && ${WINDRES} lib_resources.rc -o ${OBJECT_MODULE_DIR}/lib/lib_resources.o
			VERBATIM
			COMMENT "Generating lib_resources.o ..."
			)
	endif()
elseif(LNX_BUILD)
	target_link_libraries(${API_LIB_NAME}
		${NDCTL_LIBRARIES}
		${NUMACTL_LIBRARIES}
		${CMAKE_THREAD_LIBS_INIT}
		)

	target_include_directories(${API_LIB_NAME} PUBLIC
		external/fw_headers
		)
endif()

# ---------------------------------------------------------------------------------------
# LIB database creator
# ---------------------------------------------------------------------------------------
add_executable(db_creator src/lib/create_config_db/main.c)

target_link_libraries(db_creator
	${CMAKE_THREAD_LIBS_INIT}
	${COMMON_LIB_NAME}
	${MATH_LIBRARIES}
	${OPENSSL_SSL_LIBRARY}
	${SQLITE3_LIBRARIES}
	)

target_include_directories(db_creator PUBLIC
	src/common/persistence
	)

if(LNX_BUILD)
	add_custom_command(TARGET db_creator POST_BUILD
		COMMAND export LD_LIBRARY_PATH=${OUTPUT_DIR} && cd ${OUTPUT_DIR} && $<TARGET_FILE:db_creator>)
elseif(WIN_BUILD)
	add_custom_command(TARGET db_creator POST_BUILD
		COMMAND cd ${OUTPUT_DIR} && $<TARGET_FILE:db_creator>)
endif()

if(WIN_BUILD)
	if(NOT MSVC)
	target_link_libraries(db_creator
		Shlwapi
		ws2_32
		mswsock
		advapi32
		version
		Gdi32
		ssp
		libintl
		)
	endif()
endif()

# --------------------------------------------------------------------------------------------------
# Core Library
# --------------------------------------------------------------------------------------------------
file(GLOB core_SRC
	src/core/*.cpp
	src/core/configuration/*.cpp
	src/core/device/*.cpp
	src/core/exceptions/*
	src/core/firmware_interface/*.cpp
	src/core/logs/*.cpp
	src/core/memory_allocator/*.cpp
	src/core/system/*.cpp
	)

if(WIN_BUILD AND MSVC)
	list(APPEND core_SRC ${ROOT}/src/core/core_resources.rc)
endif()

add_library(${CORE_LIB_NAME} ${LIB_TYPE}
		${core_SRC}
		)

if(MSVC)
	# force declspec export when building
	target_compile_definitions(${CORE_LIB_NAME}
		PRIVATE ${CORE_LIB_EXPORT_DEF} ${NVM_DLL_DEF})
endif()

set_target_properties(${CORE_LIB_NAME}
	   PROPERTIES
	   VERSION ${LIB_BUILD_VERSION}
	   SOVERSION ${SO_BUILD_VERSION}
	   )

target_link_libraries(${CORE_LIB_NAME}
		firmware_interface
		${API_LIB_NAME}
		)

if(WIN_BUILD AND NOT MSVC)
	add_custom_command(
		OUTPUT core_resources.o
		COMMAND ${CMAKE_COMMAND} -E make_directory ${OBJECT_MODULE_DIR}/core
		COMMAND cd ${ROOT}/src/core && ${WINDRES} core_resources.rc -o ${OBJECT_MODULE_DIR}/core/core_resources.o
		VERBATIM
		COMMENT "Generating core_resources.o ..."
		)
endif()

# ---------------------------------------------------------------------------------------
# CIM Library / WBEM
# ---------------------------------------------------------------------------------------
file(GLOB WBEM_SOURCE_FILES
	src/wbem/*.cpp
	src/wbem/erasure/*.cpp
	src/wbem/framework_interface/*.cpp
	src/wbem/indication/*.cpp
	src/wbem/exception/*.cpp
	src/wbem/lib_interface/*.cpp
	src/wbem/mem_config/*.cpp
	src/wbem/memory/*.cpp
	src/wbem/pegasus/*.cpp
	src/wbem/performance/*.cpp
	src/wbem/physical_asset/*.cpp
	src/wbem/pmem_config/*.cpp
	src/wbem/profile/*.cpp
	src/wbem/server/*.cpp
	src/wbem/software/*.cpp
	src/wbem/support/*.cpp
	src/monitor/EventMonitor.cpp
	src/monitor/NvmMonitorBase.cpp
	src/monitor/PerformanceMonitor.cpp
	src/monitor/AcpiEventMonitor.cpp
	)

if(LNX_BUILD)
	file(GLOB WBEM_CIMOM_FILES src/wbem/cimom/cmpi/*)
elseif(WIN_BUILD)
	file(GLOB WBEM_CIMOM_FILES src/wbem/cimom/wmi/*)
endif()
list(APPEND WBEM_SOURCE_FILES ${WBEM_CIMOM_FILES})

if(MSVC)
	list(APPEND WBEM_COMPILE_FILES ${ROOT}/src/wbem/wbem_resources.rc)
endif()

add_library(${CIM_LIB_NAME} ${LIB_TYPE} ${WBEM_SOURCE_FILES})

SET_TARGET_PROPERTIES(${CIM_LIB_NAME}
	PROPERTIES
	VERSION ${LIB_BUILD_VERSION}
	SOVERSION ${SO_BUILD_VERSION}
	)

if(MSVC)
	# force declspec export when building
	target_compile_definitions(${CIM_LIB_NAME}
		 PRIVATE ${CIM_LIB_EXPORT_DEF} -D__INVM_CIM_DLL__
		 PUBLIC ${NVM_DLL_DEF})
endif()

target_include_directories(${CIM_LIB_NAME} PUBLIC
	src/common
	src/lib
	src/wbem
	external/rapidxml-1.13 #TODO:rapidxml-devel exists but is a .h file..
	${INVM-CIM_INCLUDE_DIRS}
	${SBLIM-CMPI_INCLUDE_DIRS}
	)

target_link_libraries(${CIM_LIB_NAME}
	${API_LIB_NAME}
	${CORE_LIB_NAME}
	${INVM-CIM_LIBRARIES}
	)

if(WIN_BUILD AND NOT MSVC)
	add_custom_command(
		OUTPUT wbem_resources.o
		COMMAND ${CMAKE_COMMAND} -E make_directory ${OBJECT_MODULE_DIR}/wbem
		COMMAND cd ${ROOT}/src/wbem && ${WINDRES} wbem_resources.rc -o ${OBJECT_MODULE_DIR}/wbem/wbem_resources.o
		VERBATIM
		COMMENT "Generating wbem_resources.o ..."
		)
endif()

# ---------------------------------------------------------------------------------------
# Generating mofs
# ---------------------------------------------------------------------------------------
if(WIN_BUILD)
	set(MOF_FILES
		"${CMAKE_BINARY_DIR}/intelwbem.mof"
		"${CMAKE_BINARY_DIR}/profile_registration.mof"
		"${CMAKE_BINARY_DIR}/register.mof"
		)
else()
	set(MOF_FILES
		"${CMAKE_BINARY_DIR}/intelwbem.mof"
		"${CMAKE_BINARY_DIR}/profile_registration.mof"
		"${CMAKE_BINARY_DIR}/pegasus_register.mof"
		"${CMAKE_BINARY_DIR}/sfcb_intelwbem.mof"
		"${CMAKE_BINARY_DIR}/sfcb_profile_registration.mof"
		)
endif()

set(MOF_DEPENDS_FILES
	"${PROJECT_SOURCE_DIR}/src/wbem/cimom/cmpi/pegasus_register.mof"
	"${PROJECT_SOURCE_DIR}/src/wbem/cimom/wmi/register.mof"
	"${PROJECT_SOURCE_DIR}/src/wbem/mof/cim_schema_2.29.1_combined.mof"
	"${PROJECT_SOURCE_DIR}/src/wbem/mof/cim_schema_2.44.1_combined.mof"
	"${PROJECT_SOURCE_DIR}/src/wbem/mof/class_def.mof"
	"${PROJECT_SOURCE_DIR}/src/wbem/mof/profile_registration.mof"
	"${PROJECT_SOURCE_DIR}/src/wbem/mof/qualifiers.mof"
	"${PROJECT_SOURCE_DIR}/src/wbem/mof/sfcb_qualifiers.mof"
	"${PROJECT_SOURCE_DIR}/src/wbem/mof/snia_mofs_16Rev4-updated.mof"
)

add_custom_target(generate_mofs ALL
	DEPENDS ${MOFS_FILES}
	)

if(NOT WIN_BUILD)
	add_custom_command(TARGET generate_mofs POST_BUILD
		COMMAND cd ${ROOT} &&
		/bin/bash ${ROOT}/src/wbem/mof/createmof_linux.sh ${ROOT} ${OUTPUT_DIR} ${ESX_BUILD}
		DEPENDS ${MOF_DEPENDS_FILES}
		COMMENT "Generating mofs.."
		)
elseif(WIN_BUILD)
	add_custom_command(TARGET generate_mofs POST_BUILD
		WORKING_DIRECTORY ${ROOT}/src/wbem/mof
		COMMAND cmd \"/\"/C createmof_windows.cmd ${ROOT} ${OUTPUT_DIR}
		DEPENDS ${MOF_DEPENDS_FILES}
		COMMENT "Generating mofs.."
		)
endif()

# ---------------------------------------------------------------------------------------
# CIM Provider
# ---------------------------------------------------------------------------------------
add_custom_target(cimprovider DEPENDS ${CIM_LIB_NAME} generate_mofs)

# --------------------------------------------------------------------------------------------------
# CLI Library Source
# --------------------------------------------------------------------------------------------------
file(GLOB CLI_SOURCE_FILES
	src/cli/features/*.cpp
	src/cli/features/core/*.cpp
	src/cli/features/core/framework/*.cpp
	)

if(MSVC)
	list(APPEND ${CLI_SOURCE_FILES} ${ROOT}/src/cli/features/core/nvmcli_core_features_resources.rc)
endif()

add_library(${CLI_LIB_NAME} ${LIB_TYPE} ${CLI_SOURCE_FILES})

if(MSVC)
	# force declspec export when building
	target_compile_definitions(${CLI_LIB_NAME}
		PRIVATE ${CLI_LIB_EXPORT_DEF}
		PUBLIC ${NVM_DLL_DEF})
endif()

if(NOT MSVC)
	SET_TARGET_PROPERTIES(${CLI_LIB_NAME}
		PROPERTIES
		OUTPUT_NAME ixpdimm-cli)
endif()

SET_TARGET_PROPERTIES(${CLI_LIB_NAME}
	PROPERTIES
	VERSION ${LIB_BUILD_VERSION}
	SOVERSION ${SO_BUILD_VERSION}
	)

target_include_directories(${CLI_LIB_NAME} PUBLIC
	src
	src/common
	src/lib
	src/wbem
	src/acpi
	${INVM-CLI_INCLUDE_DIRS}
	${INVM-CIM_INCLUDE_DIRS}
	)

target_link_libraries(${CLI_LIB_NAME}
	acpi
	${API_LIB_NAME}
	${CIM_LIB_NAME}
	${CORE_LIB_NAME}
	${INVM-CLI_LIBRARIES}
	${INVM-CIM_LIBRARIES}
	${INVM-I18N_LIBRARIES}
	)

if(WIN_BUILD AND NOT MSVC)
	add_custom_command(
		OUTPUT cli_resources.o ALL
		COMMAND ${CMAKE_COMMAND} -E make_directory ${OBJECT_MODULE_DIR}/cli
		COMMAND cd ${ROOT}/src/cli/features/core && ${WINDRES} nvmcli_core_features_resources.rc -o ${OBJECT_MODULE_DIR}/cli/cli_resources.o
		VERBATIM
		COMMENT "Generating cli_resources.o ..."
		)
endif()

# --------------------------------------------------------------------------------------------------
# CLI Executable
# --------------------------------------------------------------------------------------------------
if(MSVC)
	set(CLI_RESOURCE_FILE ${ROOT}/src/cli/cli_resources.rc)
endif()

add_executable(${CLI_NAME} src/cli/nvmcli.cpp ${CLI_RESOURCE_FILE})

target_include_directories(${CLI_NAME} PUBLIC
	src
	src/common
	${INVM-CLI_INCLUDE_DIRS}
	${INVM-CIM_INCLUDE_DIRS}
	)

target_link_libraries(${CLI_NAME}
	${INVM-CLI_LIBRARIES}
	${INVM-I18N_LIBRARIES}
	${CIM_LIB_NAME}
	${CLI_LIB_NAME}
	${API_LIB_NAME}
	)

add_dependencies(${CLI_NAME}
	db_creator
	)

# generating man pages
if(LNX_BUILD)
	add_custom_command(TARGET ${CLI_NAME} POST_BUILD
		COMMAND cd ${SCRIPTS_PATH} && ./create_${CLI_NAME}_manpage.py dataPath=${PRODUCT_DATADIR} outputPath=${OUTPUT_DIR}
		COMMAND cd ${SCRIPTS_PATH} && ./create_${MONITOR_NAME}_manpage.py dataPath=${PRODUCT_DATADIR}
		COMMAND mv ${SCRIPTS_PATH}/${CLI_NAME}.8.gz ${SCRIPTS_PATH}/${MONITOR_NAME}.8.gz ${OUTPUT_DIR}
		COMMAND ${CMAKE_COMMAND} -E touch ${ROOT}/src/cli/nvmcli.cpp
		)
endif()

# --------------------------------------------------------------------------------------------------
# Monitor
# --------------------------------------------------------------------------------------------------
if(WIN_BUILD)
	set(EXTRA_WINDOWS ./src/monitor/win_service.cpp)
endif()

file(GLOB MONITOR_SOURCE_FILES
	src/monitor/${FILE_PREFIX}_main.cpp
	${EXTRA_WINDOWS}
	src/monitor/EventMonitor.cpp
	src/monitor/NvmMonitorBase.cpp
	src/monitor/PerformanceMonitor.cpp
	src/monitor/AcpiEventMonitor.cpp
	)
if(MSVC)
    list(APPEND MONITOR_SOURCE_FILES ${ROOT}/src/monitor/monitor_resources.rc)
endif()

add_executable(${MONITOR_NAME} ${MONITOR_SOURCE_FILES})

target_include_directories(${MONITOR_NAME} PUBLIC
	src
	src/lib
	src/monitor
	)

target_link_libraries(ixpdimm-monitor ${API_LIB_NAME} ${CORE_LIB_NAME})

if(WIN_BUILD AND NOT MSVC)
	add_custom_command(
		OUTPUT monitor_resources.o
		COMMAND ${CMAKE_COMMAND} -E make_directory ${OBJECT_MODULE_DIR}/monitor
		COMMAND cd ${ROOT}/src/monitor && ${WINDRES} wbem_resources.rc -o ${OBJECT_MODULE_DIR}/monitor/monitor_resources.o
		VERBATIM
		COMMENT "Generating monitor_resources.o ..."
		)
endif()

# --------------------------------------------------------------------------------------------------
# Install
# --------------------------------------------------------------------------------------------------
if(LNX_BUILD)
	include(GNUInstallDirs)

	if(DEFINED INSTALL_UNITDIR)
		set(INSTALL_SYSTEMDSYSTEMUNITDIR ${INSTALL_UNITDIR})
	else()
		if(SYSTEMD_FOUND)
			if(CMAKE_VERSION VERSION_LESS 3.4.3)
				set(INSTALL_SYSTEMDSYSTEMUNITDIR  "/usr/lib/systemd/system")
			else()
				pkg_get_variable(INSTALL_SYSTEMDSYSTEMUNITDIR systemd systemdsystemunitdir)
			endif()
		endif()
	endif()

	if(CMAKE_VERSION VERSION_LESS 3.4.3)
		set(APSS_DAT_DIR "/var/lib/ixpdimm_sw")
	else()
		set(APSS_DAT_DIR ${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/ixpdimm_sw)
	endif()

	install (TARGETS ${API_LIB_NAME} ${CORE_LIB_NAME} ${CLI_LIB_NAME} ${CIM_LIB_NAME} ${CLI_NAME} ${MONITOR_NAME} ${COMMON_LIB_NAME}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
	install (FILES ${OUTPUT_DIR}/apss.dat ${OUTPUT_DIR}/public.rev0.pem
		DESTINATION ${APSS_DAT_DIR})
	install (FILES ${OUTPUT_DIR}/pegasus_register.mof ${OUTPUT_DIR}/intelwbem.mof ${OUTPUT_DIR}/profile_registration.mof
		DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/ixpdimm_sw/Pegasus/mof)
	install (FILES ${OUTPUT_DIR}/sfcb_intelwbem.mof ${OUTPUT_DIR}/INTEL_NVDIMM.reg
		DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/ixpdimm_sw/sfcb)
	install (FILES ${OUTPUT_DIR}/nvm_types.h ${OUTPUT_DIR}/nvm_management.h ${OUTPUT_DIR}/export_api.h
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
	install (FILES ${OUTPUT_DIR}/ixpdimm-cli.8.gz ${OUTPUT_DIR}/ixpdimm-monitor.8.gz
		DESTINATION ${CMAKE_INSTALL_MANDIR}/man8)
	if(DEFINED INSTALL_SYSTEMDSYSTEMUNITDIR)
		install (FILES ${OUTPUT_DIR}/${MONITOR_NAME}.service
			DESTINATION ${INSTALL_SYSTEMDSYSTEMUNITDIR})
	endif()
elseif(WIN_BUILD)
	install (CODE "execute_process(COMMAND cmd /c ${ROOT}/install/windows/install.cmd ${OUTPUT_DIR} ${SIM_BUILD})")
endif()

# --------------------------------------------------------------------------------------------------
# Uninstall
# --------------------------------------------------------------------------------------------------
if(WIN_BUILD)
	add_custom_target(uninstall
		COMMAND cmd /c ${ROOT}/install/windows/uninstall.cmd ${OUTPUT_DIR} ${SIM_BUILD})
endif()

# --------------------------------------------------------------------------------------------------
# Pegasus Register
# --------------------------------------------------------------------------------------------------
add_custom_target(pegasus_register COMMAND /bin/sh ${ROOT}/install/linux/pegasus_register.sh)
add_custom_target(pegasus_deregister COMMAND /bin/sh ${ROOT}/install/linux/pegasus_deregister.sh)

# --------------------------------------------------------------------------------------------------
# Sfcb Register
# --------------------------------------------------------------------------------------------------
add_custom_target(sfcb_register COMMAND /bin/sh ${ROOT}/install/linux/sfcb_register.sh)
add_custom_target(sfcb_deregister COMMAND /bin/sh ${ROOT}/install/linux/sfcb_deregister.sh)

# --------------------------------------------------------------------------------------------------
# RPM
# --------------------------------------------------------------------------------------------------
add_custom_target(rpm
	COMMAND ${CMAKE_COMMAND} -E echo "Entering make rpm..."
	# Make directories
	COMMAND ${CMAKE_COMMAND} -E make_directory ${RPMBUILD_DIR}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${RPMBUILD_DIR}/BUILDROOT
	COMMAND ${CMAKE_COMMAND} -E make_directory ${RPMBUILD_DIR}/SOURCES
	COMMAND ${CMAKE_COMMAND} -E make_directory ${RPMBUILD_DIR}/RPMS
	COMMAND ${CMAKE_COMMAND} -E make_directory ${RPMBUILD_DIR}/SRPMS
	COMMAND ${CMAKE_COMMAND} -E make_directory ${RPMBUILD_DIR}/SPECS
	COMMAND ${CMAKE_COMMAND} -E make_directory ${RPMBUILD_DIR}/BUILD/${MARKETING_PRODUCT_NAME}-${BUILDNUM}

	# Copy spec file
	COMMAND ${CMAKE_COMMAND} -E copy
	${ROOT}/install/linux/${LINUX_DIST}-release/*.spec ${RPMBUILD_DIR}/SPECS/${MARKETING_PRODUCT_NAME}.spec

	# Update the spec file with build version
	COMMAND sed -i "s/^%define build_version .*/%define build_version ${BUILDNUM}/g" ${RPMBUILD_DIR}/SPECS/${MARKETING_PRODUCT_NAME}.spec

	# Archive the directory
	COMMAND cd ${ROOT} && tar --exclude-vcs --exclude="*output" --exclude="*.swp*" --transform="s,^.,${MARKETING_PRODUCT_NAME}-${BUILDNUM}," -zcf ${RPMBUILD_DIR}/SOURCES/${MARKETING_PRODUCT_NAME}-${BUILDNUM}.tar.gz .

	# rpmbuild
	COMMAND cd ${ROOT} && rpmbuild -ba ${RPMBUILD_DIR}/SPECS/${MARKETING_PRODUCT_NAME}.spec --define "_topdir ${RPMBUILD_DIR}"
	)

# --------------------------------------------------------------------------------------------------
# Internal Includes
# --------------------------------------------------------------------------------------------------
if(BUILD_INTERNAL)
	if(SIM_BUILD)
		include(CMake/sim.cmake)
	endif()

	if(NOT ESX_BUILD)
		include(CMake/unit_test.cmake)
	endif()

	if(ESX_BUILD)
		include(CMake/esx.cmake)
	endif()
endif()
