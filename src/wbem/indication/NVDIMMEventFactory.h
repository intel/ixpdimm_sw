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
 * This file contains the definition of the NVMDIMMEvent provider.
 */


#ifndef _WBEM_INDICATION_NVDIMMEVENTFACTORY_H
#define	_WBEM_INDICATION_NVDIMMEVENTFACTORY_H

#include <string>
#include <libinvm-cim/Types.h>
#include <libinvm-cim/Instance.h>
#include <NvmStrings.h>
#include "NvmIndicationFactory.h"

namespace wbem
{
namespace indication
{

// Per the CIM_AlertIndication PerceivedSeverity value map
static const int NVDIMMEVENT_INFO = 2;
static const int NVDIMMEVENT_WARN = 3;
static const int NVDIMMEVENT_CRITICAL = 6;
static const int NVDIMMEVENT_FATAL = 7;

static const std::string NVDIMMEVENT_CLASSNAME = std::string(NVM_WBEM_PREFIX) + "NVDIMMEvent";

class NVDIMMEventFactory : public NvmIndicationFactory
{
public:

	virtual framework::Instance *createIndication(struct event *pEvent)
			throw (framework::Exception);

private:
	NVM_UINT16 getPerceivedSeverity(enum event_severity severity);
};


} // indication
} // wbem
#endif  // _WBEM_INDICATION_NVDIMMEVENTFACTORY_H
