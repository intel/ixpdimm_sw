/*
 * Copyright (c) 2015 2016, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Intel Corporation nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This file contains common definitions used by the performance factories.
 */

#ifndef _WBEM_PERFORMANCE_METRIC_COMMON_H
#define	_WBEM_PERFORMANCE_METRIC_COMMON_H

#include <string>
#include <libinvm-cim/Exception.h>
#include <libinvm-cim/ExceptionBadParameter.h>
#include <libinvm-cim/Types.h>
#include <exception/NvmExceptionLibError.h>
#include <NvmStrings.h>

namespace wbem
{
namespace performance
{

/*
 * Shared string constant for Metric definitions
 */
static const std::string METRICDEF_DEFINITION_STR = "Metric Definition ";
static const std::string METRICDEF_BYTES_READ_STR = "Bytes Read";
static const std::string METRICDEF_BYTES_WRITTEN_STR = "Bytes Written";
static const std::string METRICDEF_HOST_READS_STR = "Host Reads";
static const std::string METRICDEF_HOST_WRITES_STR = "Host Writes";
static const std::string METRICDEF_BLOCK_READS_STR = "Block Reads";
static const std::string METRICDEF_BLOCK_WRITES_STR = "Block Writes";
static const std::string METRICDEF_BYTES_STR = "Bytes";
static const std::string METRICDEF_COUNT_STR = "Count";

/*
 * Shared Metric Value string constants
 */
static const std::string METRICVAL_BYTES_READ_STR = "bytesread";
static const std::string METRICVAL_BYTES_WRITTEN_STR = "byteswritten";
static const std::string METRICVAL_HOST_READS_STR = "hostreads";
static const std::string METRICVAL_HOST_WRITES_STR = "hostwrites";
static const std::string METRICVAL_BLOCK_READS_STR = "blockreads";
static const std::string METRICVAL_BLOCK_WRITES_STR = "blockwrites";

/*!
 * Shared Metric Name string constants
 */
static const std::string METRICNAME_BYTES_READ_STR = "bytes read for DIMM";
static const std::string METRICNAME_BYTES_WRITTEN_STR = "bytes written for DIMM";
static const std::string METRICNAME_HOST_READS_STR = "host reads for DIMM";
static const std::string METRICNAME_HOST_WRITES_STR = "host writes for DIMM";
static const std::string METRICNAME_BLOCK_READS_STR = "block reads for DIMM";
static const std::string METRICNAME_BLOCK_WRITES_STR = "block writes for DIMM";

static const std::string METRIC_DIMM_STR = "DIMM ";

/*!
 * Enumerated list of supported metric types.
 * This list must always be sequential (no gaps)
 * It is used to avoid constant string parsing.
 */
enum metric_type
{
	METRIC_UNDEFINED = 0, // Used as default value for an uninitialized metric type.
	METRIC_BYTES_READ = 1,
	METRIC_BYTES_WRITTEN = 2,
	METRIC_HOST_READS = 3,
	METRIC_HOST_WRITES = 4,
	METRIC_BLOCK_READS = 5,
	METRIC_BLOCK_WRITES = 6,
	METRIC_FIRST_TYPE = METRIC_BYTES_READ, // Marks first valid enum value.
	METRIC_LAST_TYPE = METRIC_BLOCK_WRITES // Marks last valid enum value
};

/*!
 * CIM standard metric data type code.
 */
enum metric_data_type
{
	METRICDATATYPE_UINT64	= 13 //!< Intel NVDIMMs use only 64 bit metric values.
};
} // performance
} // wbem
#endif  // _WBEM_PERFORMANCE_METRIC_COMMON_H
