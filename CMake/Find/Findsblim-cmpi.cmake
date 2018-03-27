#
# Copyright (c) 2018, Intel Corporation
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

# - Try to find sblim-cmpi
# Once done this will define
#  SBLIM-CMPI_FOUND - sblim-cmpi found
#  SBLIM-CMPI_INCLUDE_DIRS - sblim-cmpi include directories

find_path(SBLIM-CMPI_INCLUDE_DIR cmpidt.h
	HINTS ${SBLIM-CMPI_INCLUDE_PATH}
	PATH_SUFFIXES cmpi)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set SMLIM_CMPI_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(sblim-cmpi DEFAULT_MSG SBLIM-CMPI_INCLUDE_DIR)

mark_as_advanced(SBLIM-CMPI_INCLUDE_DIR)

set(SBLIM-CMPI_INCLUDE_DIRS ${SBLIM-CMPI_INCLUDE_DIR})
