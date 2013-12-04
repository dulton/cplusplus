#ifndef _TEST_UTILITIES_H_
#define _TEST_UTILITIES_H_

#include <memory>
#include <stdio.h>

#include "UserAgentConfig.h"

#define _CPPUNIT_ASSERT CPPUNIT_ASSERT
#define _CPPUNIT_ASSERT_NO_THROW CPPUNIT_ASSERT_NO_THROW
#define _CPPUNIT_ASSERT_THROW CPPUNIT_ASSERT_THROW

#define _CPPUNIT_TEST(A) CPPUNIT_TEST(A)

namespace TestUtilities
{
    std::auto_ptr<APP_NS::UserAgentConfig> MakeUserAgentConfigIpv4(void);
    std::auto_ptr<APP_NS::UserAgentConfig> MakeUserAgentConfigIpv6(void);
};

#endif
