/*
 * Copyright (c) 2015, Intel Corporation
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

#ifndef    _WBEM_PHYSICALASSET_NEW_NVDIMM_VIEW_FACTORY_H_
#define    _WBEM_PHYSICALASSET_NEW_NVDIMM_VIEW_FACTORY_H_

#include <string>
#include <nvm_management.h>
#include <framework_interface/NvmInstanceFactory.h>
#include <core/device/DeviceService.h>
#include <core/system/SystemService.h>


namespace wbem
{
namespace physical_asset
{
static const std::string NVDIMMVIEW_CREATIONCLASSNAME = std::string(NVM_WBEM_PREFIX) + "NVDIMMView";

class NVM_API NVDIMMViewFactory : public framework_interface::NvmInstanceFactory
{
public:
	NVDIMMViewFactory(
		core::device::DeviceService &deviceService = core::device::DeviceService::getService(),
		core::system::SystemService &systemService = core::system::SystemService::getService()) :
		m_deviceService(deviceService),
		m_systemService(systemService)
	{

	}
	~NVDIMMViewFactory() { }

	framework::Instance *getInstance(framework::ObjectPath &path,
		framework::attribute_names_t &attributes);
	framework::instance_names_t *getInstanceNames();

	virtual framework::instances_t *getInstances(framework::attribute_names_t &attributes);

	static void toInstance(core::device::Device &nvdimm, framework::Instance &instance,
		wbem::framework::attribute_names_t attributes);

private:
	core::device::DeviceService &m_deviceService;
	core::system::SystemService &m_systemService;
	std::string m_hostName;
	void populateAttributeList(framework::attribute_names_t &attributes)
		throw(framework::Exception);
	framework::ObjectPath createPath(const std::string &guid);
	std::string getHostName();


	static std::string getMemoryModeString(core::device::Device &nvdimm);

	static std::string getDimmId(core::device::Device &nvdimm);

};

} // physical_asset
} // wbem
#endif  // #ifndef _WBEM_PHYSICALASSET_NEW_NVDIMM_VIEW_FACTORY_H_
