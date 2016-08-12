/*
 * INTEL CONFIDENTIAL
 *
 * Copyright 2016 Intel Corporation All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to the
 * source code ("Material") are owned by Intel Corporation or its suppliers or
 * licensors. Title to the Material remains with Intel Corporation or its
 * suppliers and licensors. The Material may contain trade secrets and proprietary
 * and confidential information of Intel Corporation and its suppliers and licensors,
 * and is protected by worldwide copyright and trade secret laws and treaty
 * provisions. No part of the Material may be used, copied, reproduced, modified,
 * published, uploaded, posted, transmitted, distributed, or disclosed in any way
 * without Intel's prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery
 * of the Materials, either expressly, by implication, inducement, estoppel or
 * otherwise. Any license under such intellectual property rights must be express
 * and approved by Intel in writing.
 *
 * Unless otherwise agreed by Intel in writing, you may not remove or alter this
 * notice or any other notice embedded in Materials by Intel or Intel's suppliers
 * or licensors in any way.
 */

/*
 * Encapsulates the logic to select a DIMM to be reserved for storage
 * from a list of DIMMs.
 */

#ifndef RESERVEDIMMSELECTOR_H_
#define RESERVEDIMMSELECTOR_H_

#include <exception>
#include <vector>
#include <map>
#include <string>
#include <nvm_types.h>
#include <core/memory_allocator/MemoryAllocationTypes.h>

namespace core
{
namespace memory_allocator
{

class NVM_API ReserveDimmSelector
{
	public:
		ReserveDimmSelector(const std::vector<Dimm> &dimms);
		virtual ~ReserveDimmSelector();

		class NoDimmsException : public std::exception {};

		std::string getReservedDimm();

	protected:
		bool dimmSelected() const;
		void selectDimm(const Dimm &dimm);

		void sortDimmsBySocket(const std::vector<Dimm> &dimms);

		void selectDimmToReserve();

		void trySelectDimmAloneOnMemoryController();
		void selectDimmAloneOnMemoryControllerFromSocket(const std::vector<Dimm> &socketDimms);
		std::map<NVM_UINT16, std::vector<Dimm> > getDimmsSortedByMemoryController(
				const std::vector<Dimm> &dimms);
		bool atLeastOneImcFullyPopulated(std::map<NVM_UINT16, std::vector<Dimm> > sortedByImc);
		std::vector<Dimm> getDimmsAloneOnTheirImc(std::map<NVM_UINT16, std::vector<Dimm> > sortedByImc);

		void trySelectDimmWithoutPartnerOnOtherMemoryController();
		void selectDimmWithoutPartnerOnOtherMemoryControllerFromSocket(
				const std::vector<Dimm> &socketDimms);
		std::map<NVM_UINT16, std::vector<Dimm> > getDimmsSortedByChannelPartnership(
				const std::vector<Dimm> &dimms);
		std::vector<Dimm> getUnpartneredDimms(std::map<NVM_UINT16, std::vector<Dimm> > sortedByChannel);

		void trySelectDifferentSizedDimm();
		void selectDifferentSizedDimmFromSocket(const std::vector<Dimm> &socketDimms);
		std::map<NVM_UINT64, std::vector<Dimm> > getDimmsSortedByCapacity(const std::vector<Dimm> &dimms);
		std::vector<Dimm> getUniquelySizedDimms(std::map<NVM_UINT64, std::vector<Dimm> > dimmsByCapacity);

		void trySelectSmallestSizedDimm();
		void selectSmallestSizedDimmFromSocket(const std::vector<Dimm> &socketDimms);
		std::vector<Dimm> getSmallestCapacityDimms(
				const std::map<NVM_UINT64, std::vector<Dimm> > &sortedByCapacity);

		void trySelectFirstDimm();
		void selectFirstDimm();

		std::map<NVM_UINT16, std::vector<Dimm> > m_sockets;
		std::string m_selectedDimmUid;
};

} /* namespace memory_allocator */
} /* namespace core */

#endif /* RESERVEDIMMSELECTOR_H_ */
