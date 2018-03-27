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

# - Try to find numactl
# Once done this will define
#  NUMACTL_FOUND - numactl found
#  NUMACTL_INCLUDE_DIRS - numactl include directories
#  NUMACTL_LIBRARIES - libraries needed to use numactl

find_path(NUMACTL_INCLUDE_DIR numa.h
	HINTS ${NUMACTL_INCLUDE_PATH})

find_library(NUMACTL_LIBRARY NAMES numa
	HINTS ${NUMACTL_LIBRARY_PATH})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set NUMACTL_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(numactl DEFAULT_MSG
                                  NUMACTL_LIBRARY NUMACTL_INCLUDE_DIR)

mark_as_advanced(NUMACTL_INCLUDE_DIR NUMACTL_LIBRARY)

set(NUMACTL_LIBRARIES ${NUMACTL_LIBRARY})
set(NUMACTL_INCLUDE_DIRS ${NUMACTL_INCLUDE_DIR})
