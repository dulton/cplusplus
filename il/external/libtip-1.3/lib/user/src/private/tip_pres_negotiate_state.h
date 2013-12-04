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

#ifndef TIP_PRES_NEG_STATE_H
#define TIP_PRES_NEG_STATE_H

#include "tip_system.h"

namespace LibTip {

    // pre-declare classes
    class CTipPresImpl;
    
    /**
     * Abstract base class for presentation states.  This class
     * provides the interface for all presentation states.
     */
    class CPresState {
    protected:
        /**
         * Protected constructor
         */
        CPresState();

        /**
         * Destructor
         */
        virtual ~CPresState();

    public:
        /**
         * Assert control over the presentation stream.
         */
        virtual Status LocalAssert(CTipPresImpl* am);

        /**
         * Remote has ack'ed our control.
         */
        virtual Status LocalAssertAck(CTipPresImpl* am, CRtcpAppReqToSendAckPacket& packet);

        /**
         * Time out waiting for remote to ack our control.
         */
        virtual Status LocalAssertTimeout(CTipPresImpl* am);
        
        /**
         * Release control of the the presentation stream.
         */
        virtual Status LocalRelease(CTipPresImpl* am);

        /**
         * Remote Tip has asserted control
         */
        virtual Status RemoteAssert(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet);

        /**
         * Remote Tip has released control
         */
        virtual Status RemoteRelease(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet);

        /**
         * Presentation negotiated parameter may have changed
         */
        virtual Status OptionUpdate(CTipPresImpl* am, CTipSystem::PresentationFrameRate oldRate);
        
        /**
         * Get the name of this state.
         */
        virtual const char* const GetName() = 0;
    };

    /**
     * No presentation state.  Neither local nor remote has asserted
     * control over the presentation.
     */
    class CNoPresState : public CPresState {
    public:
        CNoPresState();
        virtual ~CNoPresState();

        virtual Status LocalAssert(CTipPresImpl* am);
        virtual Status RemoteAssert(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet);

        virtual const char* const GetName();
    };

    /**
     * Local assert state.  The local Tip has asserted control of
     * the presentation but it has not be ACK'ed yet.
     */
    class CLocalAssertPresState : public CPresState {
    public:
        CLocalAssertPresState();
        virtual ~CLocalAssertPresState();

        virtual Status LocalRelease(CTipPresImpl* am);
        virtual Status LocalAssertAck(CTipPresImpl* am, CRtcpAppReqToSendAckPacket& packet);
        virtual Status LocalAssertTimeout(CTipPresImpl* am);
        virtual Status RemoteAssert(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet);
        virtual Status OptionUpdate(CTipPresImpl* am, CTipSystem::PresentationFrameRate oldRate);

        virtual const char* const GetName();
    };
    
    /**
     * Local control state.  The local Tip has control of the
     * presentation.
     */
    class CLocalPresState : public CPresState {
    public:
        CLocalPresState();
        virtual ~CLocalPresState();

        virtual Status LocalRelease(CTipPresImpl* am);
        virtual Status RemoteAssert(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet);
        virtual Status OptionUpdate(CTipPresImpl* am, CTipSystem::PresentationFrameRate oldRate);

        virtual const char* const GetName();
    };

    /**
     * Remote control state.  The remote Tip has control of the
     * presentation.
     */
    class CRemotePresState : public CPresState {
    public:
        CRemotePresState();
        virtual ~CRemotePresState();

        virtual Status LocalAssert(CTipPresImpl* am);
        virtual Status RemoteAssert(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet);
        virtual Status RemoteRelease(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet);
        virtual Status OptionUpdate(CTipPresImpl* am, CTipSystem::PresentationFrameRate oldRate);

        virtual const char* const GetName();
    };

    /**
     * Override state.  The local Tip had control of presentation
     * but was pre-empted by the remote Tip.
     */
    class COverridePresState : public CPresState {
    public:
        COverridePresState();
        virtual ~COverridePresState();

        virtual Status LocalRelease(CTipPresImpl* am);
        virtual Status RemoteAssert(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet);
        virtual Status RemoteRelease(CTipPresImpl* am, CRtcpAppReqToSendPacket& packet);
        virtual Status OptionUpdate(CTipPresImpl* am, CTipSystem::PresentationFrameRate oldRate);

        const char* const GetName();
    };

    /**
     * Global state objects.  As each class has no internal data we
     * can use globals instead of having multiple copies.
     */
    extern CNoPresState          gNoPresState;
    extern CLocalAssertPresState gLocalAssertPresState;
    extern CLocalPresState       gLocalPresState;
    extern CRemotePresState      gRemotePresState;
    extern COverridePresState    gOverridePresState;
};       
#endif
