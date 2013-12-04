/// @file
/// @brief Server Connection Handler header file
///
///  Copyright (c) 2007 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _DATA_CONNECTION_HANDLER_H_
#define _DATA_CONNECTION_HANDLER_H_

#include <queue>

#include <ace/Time_Value.h>
#include <app/StreamSocketHandler.h>

#include <base/BaseCommon.h>

#include <boost/random/linear_congruential.hpp>

#include <Tr1Adapter.h>
#include <utils/BodySize.h>
#include <utils/MessageIterator.h>

#include "FtpCommon.h"
#include "FtpProtocol.h"

// Forward declarations (global)
class ACE_Message_Block;

namespace FTP_NS
{
    class DataTransactionManager ;
}

DECL_FTP_NS

///////////////////////////////////////////////////////////////////////////////

class DataConnectionHandler : public L4L7_APP_NS::StreamSocketHandler
{
public:
    typedef FTP_IDL_NS::BodyContentOptions bodyType_t;
    typedef fastdelegate::FastDelegate0 < uint32_t > responseLatencyGetter_t ;
    typedef fastdelegate::FastDelegate1 <uint32_t, void > statsUpdate_t ;

    explicit DataConnectionHandler(uint32_t serial = 0);
    ~DataConnectionHandler();

    /// @brief API to setup data body size parameters
    void SetTxBodyConfig(uint8_t &szType, 
                         uint32_t &fixedSz, 
                         uint32_t &randomSzMean, 
                         uint32_t &randomStdDev, 
                         uint8_t &contentType) ;

    /// @brief API to tell connection handler if its going to do Tx or Rx
    void SetDoTx(bool doTx) { mDoTx = doTx ; }

    /// @brief delegate to callback on additional tx bytes - switch to use StreamSocketHandler::RegisterUpdateTxCountDelegate()
    
    /// @brief delegate to callback on additional rx bytes
    void SetIncrementalRxCallback(const statsUpdate_t& rxUpdate) { mRxUpdate = rxUpdate ; }

protected:
    /// Socket open handler method (from StreamSocketHandler interface)
    int HandleOpenHook(void);

    /// @brief Socket input handler method (from StreamSocketHandler interface)
    int HandleInputHook(void);

    /// @brief Socket output handler method (from StreamSocketHandler interface)
    int HandleOutputHook(void);

private:
    /// Convenience typedefs
    typedef boost::minstd_rand randomEngine_t;

    uint32_t GetBodySize(void) { return mBodySize.Get(); }

    void NotifyRx(uint32_t bytes) 
    { 
        if (bytes && !mRxUpdate.empty()) 
            mRxUpdate(bytes) ; 
    }

    uint8_t           mContentType ;      ///< Body content type
    randomEngine_t    mRandomEngine;      ///< randomness engine
    L4L7_UTILS_NS::BodySize mBodySize;    ///< body size
    bool              mDoTx ;
    responseLatencyGetter_t mLatencyGetter ;

    // stats notification delegate
    statsUpdate_t mTxUpdate, 
                  mRxUpdate ;

};

///////////////////////////////////////////////////////////////////////////////

END_DECL_FTP_NS

#endif
