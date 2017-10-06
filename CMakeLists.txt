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

cmake_minimum_required(VERSION 2.8.4)
project(cr_mgmt)

if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION} GREATER 2.8.12)
	cmake_policy(SET CMP0040 OLD)
endif ()

#set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_SKIP_RPATH TRUE)
set(SIM_BUILD 0)
set(ADAPTER_TYPE real)

# LIB Type
set(LIB_TYPE SHARED)
option(BUILD_STATIC "Build libraries as STATIC. This will override LIB_TYPE" OFF)
if (BUILD_STATIC)
    set(LIB_TYPE STATIC)
	set(FRAMEWORK_OPTIONS "${FRAMEWORK_OPTIONS}-DBUILD_STATIC=ON ")
endif()

option(RELEASE "Build in simulator mode." OFF)
if (RELEASE)
    set(BUILD_TYPE release)
    set(FRAMEWORK_OPTIONS "${FRAMEWORK_OPTIONS}-DRELEASE=ON ")
else ()
    set(BUILD_TYPE debug)
endif ()

if (UNIX)
    set(LNX_BUILD 1)
elseif (MINGW)
    set(WIN_BUILD 1)
endif ()

if (LNX_BUILD)
    set(OS_TYPE linux)
    set(FILE_PREFIX lnx)
    set(ADAPTER_PREFIX lnx)
elseif (WIN_BUILD)
    set(OS_TYPE windows)
    set(FILE_PREFIX win)
    set(ADAPTER_PREFIX win)
endif ()

get_filename_component(ROOT ./ ABSOLUTE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98")
set(OUTPUT_DIR ${ROOT}/output/build/${OS_TYPE}/${ADAPTER_TYPE}/${BUILD_TYPE} CACHE PATH "Build Path")

add_subdirectory(${ROOT}/invm-frameworks/invm-i18n)
add_subdirectory(${ROOT}/invm-frameworks/invm-cim)
add_subdirectory(${ROOT}/invm-frameworks/invm-cli)
set(invm-i18n_include ${OUTPUT_DIR}/include)
set(invm-cim_include ${OUTPUT_DIR}/include)
set(invm-cli_include ${OUTPUT_DIR}/include)

set(CMAKE_BINARY_DIR ${OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(SCRIPTS_PATH ${ROOT}/scripts)

include(cmakesetup.cmake)

# --------------------------------------------------------------------------------------------------
# DRIVER INTERFACE
# --------------------------------------------------------------------------------------------------
file(GLOB DRIVER_INTERFACE_FILES
        src/driver_interface/${ADAPTER_PREFIX}_passthrough.c
        src/driver_interface/passthrough.c
		src/common/string/s_str.c
        )
if (WIN_BUILD)
	file(GLOB WIN_DRIVER_INTERFACE_FILES src/lib/win_scm2*.c)
	list(APPEND DRIVER_INTERFACE_FILES ${WIN_DRIVER_INTERFACE_FILES})
endif()

add_library(driver_interface STATIC ${DRIVER_INTERFACE_FILES})
target_include_directories(driver_interface PUBLIC
       		src/
            )
if (LNX_BUILD)
    target_link_libraries(driver_interface ndctl)
elseif (WIN_BUILD)
    target_include_directories(driver_interface PUBLIC
        	src/common
            ${invm-i18n_include}
            )
endif ()

# --------------------------------------------------------------------------------------------------
# FIRMWARE INTERFACE
# --------------------------------------------------------------------------------------------------
file(GLOB FIRMWARE_INTERFACE_FILES
        src/firmware_interface/fis_commands.c
        src/firmware_interface/fis_parser.c
        src/firmware_interface/fw_commands.c
        src/firmware_interface/fw_command_printer.c
        src/firmware_interface/fw_command_controller.c
        src/firmware_interface/fw_command_dump.c
        src/firmware_interface/fwcmd_args.c
        src/common/string/s_str.c
        )

add_library(firmware_interface STATIC ${FIRMWARE_INTERFACE_FILES})

target_link_libraries(firmware_interface driver_interface)
target_include_directories(firmware_interface PUBLIC
        ../
        ./
        )

#---------------------------------------------------------------------------------------------------
# common library
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
        src/common/acpi/acpi.c
        src/common/acpi/${FILE_PREFIX}_acpi.c
        )

if (LNX_BUILD)
    list(APPEND COMMON_SOURCE_FILES
            ${ROOT}/src/common/file_ops/lnx_file_ops.c
            ${ROOT}/src/common/os/lnx_os.c
            )
elseif (WIN_BUILD)
    list(APPEND COMMON_SOURCE_FILES
            src/common/file_ops/win_file_ops.c
            src/common/os/win_os.c
            )
endif ()

add_library(common STATIC
        ${COMMON_SOURCE_FILES}
        )

target_link_libraries(common
        sqlite3
        ssl
        crypto
        m
        z
       	)
target_include_directories(common PUBLIC
        src/common/
        src/lib
        )
if (WIN_BUILD)
	set(win_msgs_header ${ROOT}/src/common/os/win_msgs.h)

	# win_msgs.h generation
    add_custom_command(
            OUTPUT ${win_msgs_header}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${OBJECT_MODULE_DIR}/common/os
            COMMAND cd ${ROOT}/src/common/os && ${WINDMC} -U win_msgs.mc -r ${OBJECT_MODULE_DIR}/common/os
            COMMAND ${WINDRES} ${OBJECT_MODULE_DIR}/common/os/win_msgs.rc -o ${OBJECT_MODULE_DIR}/common/os/win_msgs.o
            DEPENDS src/common/os/win_msgs.mc
            VERBATIM
   			)
    add_custom_target(
            win_msgs_header
            DEPENDS ${win_msgs_header}
            )
    set_source_files_properties(${win_msgs_header}
        	PROPERTIES GENERATED TRUE
        	)
    target_link_libraries(common
            ws2_32
            mswsock
            advapi32
            version
            Gdi32
            Shlwapi
            iconv
            z
            )
    add_dependencies(common
       		win_msgs_header
      		)
endif ()

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
        src/common/guid/guid.c)
add_library(acpi STATIC ${ACPI_SRC_FILES})
target_include_directories(acpi PUBLIC src/common)
target_link_libraries(acpi
        ssl
        crypto
        z
        )

# --------------------------------------------------------------------------------------------------
# Native Library
# --------------------------------------------------------------------------------------------------
file(GLOB LIB_SOURCE_FILES
        src/lib/*.c)
# remove all adapter files
file(GLOB_RECURSE TO_REMOVE
        src/lib/lnx_*
        src/lib/win_*
        src/lib/esx_*
        src/lib/sim_*
        )
list(REMOVE_ITEM LIB_SOURCE_FILES ${TO_REMOVE})

# add appropriate adapter files back in
if (WIN_BUILD)
    file(GLOB ADAPTER_FILES src/lib/win*)
elseif (LNX_BUILD)
    file(GLOB ADAPTER_FILES src/lib/lnx*)
endif ()
list(APPEND LIB_SOURCE_FILES ${ADAPTER_FILES})

add_library(${API_LIB_NAME} ${LIB_TYPE} ${LIB_SOURCE_FILES})
# this will create the symlinks on Linux
set_target_properties(${API_LIB_NAME}
       PROPERTIES
       VERSION ${LIB_BUILD_VERSION}
       SOVERSION ${SO_BUILD_VERSION}
       )
include_directories(${API_LIB_NAME} SYSTEM PUBLIC
        external/os_headers
        )
target_include_directories(${API_LIB_NAME} PUBLIC
        src
        src/acpi
        external/fw_headers
        external/os_headers
        )
target_link_libraries(${API_LIB_NAME}
        common
        acpi
        sqlite3
        ssl
        crypto
        invm-i18n
        z
        pthread
        )

if (WIN_BUILD)
    target_include_directories(${API_LIB_NAME} PUBLIC
            src/acpi)
    target_link_libraries(${API_LIB_NAME}
            ws2_32
            mswsock
            advapi32
            version
            Gdi32
            Shlwapi
            iconv
            )
elseif (LNX_BUILD)
    target_link_libraries(${API_LIB_NAME}
            dl
            numa
            m
            ndctl
            )
endif ()

# ---------------------------------------------------------------------------------------
# LIB database creator
# ---------------------------------------------------------------------------------------
add_executable(db_creator src/lib/create_config_db/main.c)
target_link_libraries(db_creator
        common
        sqlite3
        pthread
	    )
target_include_directories(db_creator PUBLIC
       		src/common
            )
add_custom_command(TARGET db_creator POST_BUILD
       COMMAND cd ${OUTPUT_DIR} && $<TARGET_FILE:db_creator>)

target_link_libraries(db_creator dl ssl)

# --------------------------------------------------------------------------------------------------
# core
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
add_library(${CORE_LIB_NAME} ${LIB_TYPE}
        ${core_SRC}
        )
set_target_properties(${CORE_LIB_NAME}
       PROPERTIES
       VERSION ${LIB_BUILD_VERSION}
       SOVERSION ${SO_BUILD_VERSION}
       )

target_link_libraries(${CORE_LIB_NAME}
        ${API_LIB_NAME}
        firmware_interface
        )

# ---------------------------------------------------------------------------------------
# WBEM
# ---------------------------------------------------------------------------------------
file(GLOB WBEM_SOURCE_FILES
		src/wbem/*
        src/wbem/erasure/*
        src/wbem/framework_interface/*
        src/wbem/indication/*
        src/wbem/exception/*
        src/wbem/lib_interface/*
        src/wbem/mem_config/*
        src/wbem/memory/*
        src/wbem/pegasus/*
        src/wbem/performance/*
        src/wbem/physical_asset/*
        src/wbem/pmem_config/*
        src/wbem/profile/*
        src/wbem/server/*
        src/wbem/software/*
        src/wbem/support/*
        src/monitor/EventMonitor.cpp
        src/monitor/NvmMonitorBase.cpp
        src/monitor/PerformanceMonitor.cpp
        )
if (LNX_BUILD)
	file(GLOB WBEM_CIMOM_FILES src/wbem/cimom/cmpi/*)
elseif (WIN_BUILD)
	file(GLOB WBEM_CIMOM_FILES src/wbem/cimom/wmi/*)
endif ()
list(APPEND WBEM_SOURCE_FILES ${WBEM_CIMOM_FILES})

add_library(${CIM_LIB_NAME} ${LIB_TYPE} ${WBEM_SOURCE_FILES})
SET_TARGET_PROPERTIES(${CIM_LIB_NAME}
        PROPERTIES
      	VERSION ${LIB_BUILD_VERSION}
        SOVERSION ${SO_BUILD_VERSION}
        )

target_include_directories(${CIM_LIB_NAME} PUBLIC
        src/lib/
        src/wbem/
        external/rapidxml-1.13
        ${invm-cim_include}
        )
target_link_libraries(${CIM_LIB_NAME}
        ${API_LIB_NAME}
        ${CORE_LIB_NAME}
        invm-cim
        )

# ---------------------------------------------------------------------------------------
# Generating mofs
# ---------------------------------------------------------------------------------------
add_custom_target(generate_mofs ALL)
if (LNX_BUILD)
  add_custom_command(TARGET generate_mofs POST_BUILD
  				COMMAND cd ${ROOT} &&
  				/bin/sh ${ROOT}/src/wbem/mof/createmof_linux.sh ${ROOT} ${OUTPUT_DIR}
  				)
elseif (WIN_BUILD)
  add_custom_command(TARGET generate_mofs POST_BUILD
				WORKING_DIRECTORY ${ROOT}/src/wbem/mof
  				COMMAND cmd \"/\"/C createmof_windows.cmd ${ROOT} ${OUTPUT_DIR}
  				)
endif ()

# ---------------------------------------------------------------------------------------
# Cim provider
# ---------------------------------------------------------------------------------------
add_custom_target(cimprovider DEPENDS ${CIM_LIB_NAME} generate_mofs)

# --------------------------------------------------------------------------------------------------
# CLI Library Source
# --------------------------------------------------------------------------------------------------
file(GLOB CLI_SOURCE_FILES
        src/cli/features/*
        src/cli/features/core/*
        src/cli/features/core/framework/*
        )
add_library(${CLI_LIB_SONAME} ${LIB_TYPE} ${CLI_SOURCE_FILES})
SET_TARGET_PROPERTIES(${CLI_LIB_SONAME}
        PROPERTIES
      	VERSION ${LIB_BUILD_VERSION}
        SOVERSION ${SO_BUILD_VERSION}
        OUTPUT_NAME ixpdimm-cli
        )

target_include_directories(${CLI_LIB_SONAME} PUBLIC
		src/
		src/lib/
		src/wbem/
		src/acpi/
		${invm-cli_include}
		${invm-i18n_include}
		)
target_link_libraries(${CLI_LIB_SONAME}
        acpi
        ${CORE_LIB_NAME}
        ${CIM_LIB_NAME}
        ${API_LIB_NAME}
        invm-cim
        invm-cli
        invm-i18n
        )

# --------------------------------------------------------------------------------------------------
# CLI Executable
# --------------------------------------------------------------------------------------------------
add_executable(${CLI_NAME} src/cli/nvmcli.cpp)

target_include_directories(${CLI_NAME} PUBLIC
        src/common
        ${invm-cli_include}
		${invm-i18n_include}
		src/
        )
target_link_libraries(${CLI_NAME}
        invm-cli
        invm-i18n
        ${CIM_LIB_NAME}
        ${CLI_LIB_SONAME}
        ${API_LIB_NAME}
        )
add_dependencies(${CLI_NAME}
        db_creator
        )
# generating man pages
if (LNX_BUILD)
		add_custom_command(TARGET ${CLI_NAME} POST_BUILD
					COMMAND cd ${SCRIPTS_PATH} && ./create_${CLI_NAME}_manpage.py dataPath=${PRODUCT_DATADIR} outputPath=${OUTPUT_DIR}
					COMMAND cd ${SCRIPTS_PATH} && ./create_${MONITOR_NAME}_manpage.py dataPath=${PRODUCT_DATADIR}
					COMMAND mv ${SCRIPTS_PATH}/${CLI_NAME}.8.gz ${SCRIPTS_PATH}/${MONITOR_NAME}.8.gz ${OUTPUT_DIR}
					COMMAND ${CMAKE_COMMAND} -E touch ${ROOT}/src/cli/nvmcli.cpp
					)
endif ()

# --------------------------------------------------------------------------------------------------
# Monitor
# --------------------------------------------------------------------------------------------------
if (WIN_BUILD)
    set(EXTRA_WINDOWS ./src/monitor/win_service.cpp)
endif ()

file(GLOB MONITOR_SOURCE_FILES
		src/monitor/${FILE_PREFIX}_main.cpp
        ${EXTRA_WINDOWS}
        src/monitor/EventMonitor.cpp
        src/monitor/NvmMonitorBase.cpp
        src/monitor/PerformanceMonitor.cpp)

add_executable(${MONITOR_NAME} ${MONITOR_SOURCE_FILES})
target_include_directories(${MONITOR_NAME} PUBLIC
        src/
        src/lib/
        src/monitor
        )
target_link_libraries(ixpdimm-monitor ${API_LIB_NAME} ${CORE_LIB_NAME})

# --------------------------------------------------------------------------------------------------
# Install
# --------------------------------------------------------------------------------------------------
if (LNX_BUILD)
	install (CODE "execute_process(COMMAND /bin/sh ${ROOT}/install/linux/install.sh ${OUTPUT_DIR} ${SIM_BUILD})")
elseif (WIN_BUILD)
	install (CODE "execute_process(COMMAND cmd /c ${ROOT}/install/windows/install.cmd ${OUTPUT_DIR} ${SIM_BUILD})")
endif ()

# --------------------------------------------------------------------------------------------------
# Uninstall
# --------------------------------------------------------------------------------------------------
if (LNX_BUILD)
		add_custom_target(uninstall
			COMMAND /bin/sh ${ROOT}/install/linux/uninstall.sh ${OUTPUT_DIR} ${SIM_BUILD})
elseif (WIN_BUILD)
		add_custom_target(uninstall
			COMMAND cmd /c ${ROOT}/install/windows/uninstall.cmd ${OUTPUT_DIR} ${SIM_BUILD})
endif ()

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

        COMMAND sed -i "s/^%define invm_framework_build_version .*/%define invm_framework_build_version ${BUILDNUM_FRAMEWORKS}/g" ${RPMBUILD_DIR}/SPECS/${MARKETING_PRODUCT_NAME}.spec

        # Archive the directory
        COMMAND cd ${ROOT} && tar --exclude-vcs --exclude="*output" --exclude="*.swp*" --transform="s,^.,${MARKETING_PRODUCT_NAME}-${BUILDNUM}," -zcf ${RPMBUILD_DIR}/SOURCES/${MARKETING_PRODUCT_NAME}-${BUILDNUM}.tar.gz .

        # rpmbuild
        COMMAND cd ${ROOT} && rpmbuild -ba ${RPMBUILD_DIR}/SPECS/${MARKETING_PRODUCT_NAME}.spec --define "_topdir ${RPMBUILD_DIR}" --define "cflag ${CFLAGS_EXTERNAL}"
        )

# --------------------------------------------------------------------------------------------------
# Project wide defines and flags
# --------------------------------------------------------------------------------------------------

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -D_XOPEN_SOURCE=500 -Wall -Werror -Wfatal-errors -MMD")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wformat -Wformat-security -D_FORTIFY_SOURCE=2 -ggdb -fstack-protector-all -D_XOPEN_SOURCE=500 -Wall -Werror -Wfatal-errors -MMD")

add_definitions(
        -D__WBEM_PREFIX__=Intel_
        -D__ADD_MANUFACTURING__=1
        -D_GNU_SOURCE
        -D__VERSION_MAJOR__=${VERSION_MAJOR}
        -D__VERSION_MINOR__=${VERSION_MINOR}
        -D__VERSION_HOTFIX__=${VERSION_HOTFIX}
        -D__VERSION_BUILDNUM__=${VERSION_BUILDNUM}
        -D__VERSION_NUMBER__=${BUILDNUM}
)

if (BUILD_STATIC)
	add_definitions(
            -DBUILD_STATIC
    )
endif ()

if (RELEASE)
    add_definitions(
            -DRELEASE
    )
    add_definitions(-O2
    -fno-strict-aliasing
    )
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -fno-strict-aliasing")
else ()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O")
endif ()

if (LNX_BUILD)
    add_definitions(
            -D_GNU_SOURCE
            -D__LINUX__
            -D__LARGE_PAYLOAD__=1
            -D__NVM_DLL__
            -fPIC
            -DCMPI_PLATFORM_LINUX_GENERIC_GNU=1
            -D__PRODUCT_DATADIR__="${PRODUCT_DATADIR}/"
    )
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -z noexecstack -z relro -z now -pie")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFLAGS_EXTERNAL}")
endif ()

# Windows specific
if (WIN_BUILD)
     add_definitions(
            -D_WIN32_WINNT=0x0601
            -D__NVM_DLL_EXPORTS__
            -D__EARLY_HW__=0
            -D__LARGE_PAYLOAD__=1
            -D__INTEL_I18N__
            -D__WINDOWS__
    )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64 -mno-ms-bitfields -static-libgcc -static-libstdc++")
endif ()
