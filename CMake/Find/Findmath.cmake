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

# - Try to find math
# Once done this will define
#  MATH_FOUND - math found
#  MATH_INCLUDE_DIRS - math include directories
#  MATH_LIBRARIES - libraries needed to use math

find_path(MATH_INCLUDE_DIR math.h)

find_library(MATH_LIBRARY NAMES m)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set MATH_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(math DEFAULT_MSG
                                  MATH_LIBRARY MATH_INCLUDE_DIR)

mark_as_advanced(MATH_INCLUDE_DIR MATH_LIBRARY)

set(MATH_LIBRARIES ${MATH_LIBRARY})
set(MATH_INCLUDE_DIRS ${MATH_INCLUDE_DIR})
