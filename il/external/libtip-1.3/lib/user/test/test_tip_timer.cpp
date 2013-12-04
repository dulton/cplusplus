/*
 *   Copyright 2010 IMTC, Inc.  All rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "private/tip_timer.h"
using namespace LibTip;

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class CTipTimerTest : public CppUnit::TestFixture {
private:
    CTipTimer* mt;

public:
    void setUp() {
        mt = new CTipTimer();
        CPPUNIT_ASSERT( mt != NULL );
    }

    void tearDown() {
        delete mt;
    }

    void testInit() {
        CTipTimer::TimerType type;
        uint32_t data;

        CPPUNIT_ASSERT_EQUAL( mt->GetNextExpiredTime(), (uint64_t) -1 );
        CPPUNIT_ASSERT_EQUAL( mt->GetExpired(type, data), TIP_ERROR );
    }

    void testRegister() {
        CPPUNIT_ASSERT_EQUAL( mt->Register(CTipTimer::AMT_TIP_NEGOTIATE, 0, 0), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( mt->GetNextExpiredTime(), (uint64_t) 0 );
    }

    void testRegister2() {
        CPPUNIT_ASSERT_EQUAL( mt->Register(CTipTimer::AMT_TIP_NEGOTIATE, 1, 0), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( mt->Register(CTipTimer::AMT_TIP_NEGOTIATE, 2, 0), (uint32_t) 1 );
        CPPUNIT_ASSERT_EQUAL( mt->GetNextExpiredTime(), (uint64_t) 1 );
    }

    void testRegister3() {
        CPPUNIT_ASSERT_EQUAL( mt->Register(CTipTimer::AMT_TIP_NEGOTIATE, 2, 0), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( mt->Register(CTipTimer::AMT_TIP_NEGOTIATE, 1, 0), (uint32_t) 1 );
        CPPUNIT_ASSERT_EQUAL( mt->GetNextExpiredTime(), (uint64_t) 1 );
    }

    void testCancel() {
        CPPUNIT_ASSERT_EQUAL( mt->Register(CTipTimer::AMT_TIP_NEGOTIATE, 0, 0), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( mt->Cancel(0), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( mt->GetNextExpiredTime(), (uint64_t) -1 );
    }

    void testCancel2() {
        CPPUNIT_ASSERT_EQUAL( mt->Register(CTipTimer::AMT_TIP_NEGOTIATE, 0, 0), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( mt->Register(CTipTimer::AMT_TIP_NEGOTIATE, 100, 0), (uint32_t) 1 );
        CPPUNIT_ASSERT_EQUAL( mt->Cancel(0), TIP_OK );
        CPPUNIT_ASSERT( mt->GetNextExpiredTime() <= 100 );
        CPPUNIT_ASSERT( mt->GetNextExpiredTime() > 75 );
    }

    void testCancel3() {
        CPPUNIT_ASSERT_EQUAL( mt->Register(CTipTimer::AMT_TIP_NEGOTIATE, 0, 0), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( mt->Register(CTipTimer::AMT_DELAYED_ACK, 0, 0), (uint32_t) 1 );
        
        CPPUNIT_ASSERT_EQUAL( mt->Cancel(0), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( mt->GetNextExpiredTime(), (uint64_t) 0 );
    }
    
    void testCancelInvalid() {
        CPPUNIT_ASSERT_EQUAL( mt->Cancel(0), TIP_ERROR );
    }
    
    void testExpired() {
        CTipTimer::TimerType type;
        uint32_t data;
        
        CPPUNIT_ASSERT_EQUAL( mt->Register(CTipTimer::AMT_TIP_NEGOTIATE, 0, 0), (uint32_t) 0 );
        
        CPPUNIT_ASSERT_EQUAL( mt->GetExpired(type, data), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( type, CTipTimer::AMT_TIP_NEGOTIATE );
        CPPUNIT_ASSERT_EQUAL( data, (uint32_t) 0 );
        
        CPPUNIT_ASSERT_EQUAL( mt->GetExpired(type, data), TIP_ERROR );
        CPPUNIT_ASSERT_EQUAL( mt->GetNextExpiredTime(), (uint64_t) -1 );
    }
    
    void testExpired2() {
        CTipTimer::TimerType type;
        uint32_t data;
        
        CPPUNIT_ASSERT_EQUAL( mt->Register(CTipTimer::AMT_TIP_NEGOTIATE, 1000, 0), (uint32_t) 0 );
        CPPUNIT_ASSERT_EQUAL( mt->GetNextExpiredTime(), (uint64_t) 1000 );
        
        CPPUNIT_ASSERT_EQUAL( mt->Register(CTipTimer::AMT_TIP_NEGOTIATE, 0, 1), (uint32_t) 1 );
        CPPUNIT_ASSERT_EQUAL( mt->GetNextExpiredTime(), (uint64_t) 0 );
        
        CPPUNIT_ASSERT_EQUAL( mt->GetExpired(type, data), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( type, CTipTimer::AMT_TIP_NEGOTIATE );
        CPPUNIT_ASSERT_EQUAL( data, (uint32_t) 1 );
        
        CPPUNIT_ASSERT_EQUAL( mt->GetExpired(type, data), TIP_ERROR );
        CPPUNIT_ASSERT( mt->GetNextExpiredTime() <= (uint64_t) 1000 );
        CPPUNIT_ASSERT( mt->GetNextExpiredTime() > (uint64_t) 750 );

        sleep(1);
        
        CPPUNIT_ASSERT_EQUAL( mt->GetExpired(type, data), TIP_OK );
        CPPUNIT_ASSERT_EQUAL( type, CTipTimer::AMT_TIP_NEGOTIATE );
        CPPUNIT_ASSERT_EQUAL( data, (uint32_t) 0 );
        
        CPPUNIT_ASSERT_EQUAL( mt->GetExpired(type, data), TIP_ERROR );
        CPPUNIT_ASSERT_EQUAL( mt->GetNextExpiredTime(), (uint64_t) -1 );
    }

    CPPUNIT_TEST_SUITE( CTipTimerTest );
    CPPUNIT_TEST( testInit );
    CPPUNIT_TEST( testRegister );
    CPPUNIT_TEST( testRegister2 );
    CPPUNIT_TEST( testRegister3 );
    CPPUNIT_TEST( testCancel );
    CPPUNIT_TEST( testCancel2 );
    CPPUNIT_TEST( testCancel3 );
    CPPUNIT_TEST( testCancelInvalid );
    CPPUNIT_TEST( testExpired );
    CPPUNIT_TEST( testExpired2 );
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION( CTipTimerTest );
