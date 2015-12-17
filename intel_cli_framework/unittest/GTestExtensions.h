/*
 * INTEL CONFIDENTIAL
 *
 * Copyright 2013 2014 Intel Corporation All Rights Reserved.
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
 *
 *
 * GTestExtensions.h
 *
 * Additional common macros for GTEST
 */

#include <intel_cli_framework/NotImplementedErrorResult.h>
/*
 * Asserts that the syntax (list of strings) is valid but the command is not implemented yet
 */

#define	ASSERT_NOT_IMPLEMENTED(expectedId, expectedType, tokens) \
{ \
	cli::framework::Framework *pFrameworkInst = cli::framework::Framework::getFramework(); \
	cli::framework::ResultBase *pResult = pFrameworkInst->execute(tokens); \
	cli::framework::NotImplementedErrorResult *pErrorResult = dynamic_cast<cli::framework::NotImplementedErrorResult *>(pResult); \
	ASSERT_NE((cli::framework::NotImplementedErrorResult *)NULL, pErrorResult) << pResult->output(); \
	EXPECT_EQ(cli::framework::ErrorResult::ERRORCODE_NOTSUPPORTED, pErrorResult->getErrorCode()); \
	EXPECT_EQ(expectedType::Name, pErrorResult->getFeatureName()); \
	EXPECT_EQ( expectedId, pErrorResult->getCommandSpecId()); \
	delete pResult;\
}

#define	EXPECT_STR_CONTAINS(expected, stringToSearch) \
{	std::string expectedStr(expected); \
	std::string l_stringToSearch(stringToSearch);\
	size_t pos = l_stringToSearch.find(expectedStr); \
	EXPECT_NE((size_t)std::string::npos, (size_t)pos) << "\"" << expected << "\" not found in \"" << stringToSearch << "\""; \
}

#define	EXPECT_STR_NOT_CONTAINS(expected, stringToSearch) \
{	std::string expectedStr(expected); \
	std::string l_stringToSearch(stringToSearch);\
	size_t pos = l_stringToSearch.find(expectedStr); \
	EXPECT_EQ((size_t)std::string::npos, (size_t)pos) << "\"" << expected << "\" not found in \"" << stringToSearch << "\""; \
}

#define ASSERT_NOT_NULL(type, pointer) ASSERT_NE((type)NULL, (pointer))
#define ASSERT_NULL(type, pointer) ASSERT_EQ((type)NULL, (pointer))
#define ASSERT_TYPE(type, pointer) ASSERT_NE((type)NULL, dynamic_cast<type>(pointer))
#define ASSERT_NOT_TYPE(type, pointer) ASSERT_EQ((type)NULL, dynamic_cast<type>(pointer))
#define EXPECT_NOT_NULL(type, pointer) EXPECT_NE((type)NULL, pointer)
#define EXPECT_NULL(type, pointer) EXPECT_EQ((type)NULL, pointer)
#define EXPECT_TYPE(type, pointer) EXPECT_NE((type)NULL, dynamic_cast<type>(pointer))
#define EXPECT_NOT_TYPE(type, pointer) EXPECT_EQ((type)NULL, dynamic_cast<type>(pointer))

#define EXPECT_OBJECTLIST_CONTAINS_KEY(expect, collection) \
		{cli::framework::propertyObjects_t::iterator iter = collection->objectsBegin();     \
		bool found = false;                                                                   \
		for (; iter != collection->objectsEnd() && !found; iter++)                       \
		{                                                                                     \
			if (iter->first == expect)                                                      \
				found = true;                                                                 \
		}\
		EXPECT_TRUE(found) << expect << " not found in collection";}
