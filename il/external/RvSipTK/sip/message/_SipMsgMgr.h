/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           SipMsgMgr.h                                      *
 *                                                                            *
 * The file defines the 'internal APIs' of the message manager.               *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             DEC.2000                                             *
 ******************************************************************************/

#ifndef SIP_MSG_MGR_H
#define SIP_MSG_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipParserManager.h"
#include "AdsRlist.h"
#include "_SipCommonUtils.h"
#include "RvSipTransportTypes.h"
#include "rvlog.h"
#include "rvrandomgenerator.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

typedef struct
{
    RvLogMgr*            pLogMgr;
    RvLogSource*   logId;
    RvRandomGenerator  *seed;
    SipParserMgrHandle   hParserMgr;
} SipMessageMgrCfg;

/***************************************************************************
 * SipMessageMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct a new MsgMgr.
 * Return Value: RV_OK
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: msgMgrCfg - The message manager configuration parameters.
 * Output: hMsgMgr - Handle of the constructed msg manager.
 ***************************************************************************/
RvStatus SipMessageMgrConstruct(IN  SipMessageMgrCfg *msgMgrCfg,
                                 OUT RvSipMsgMgrHandle *hMsgMgr);

/***************************************************************************
 * SipMsgMgrSetTransportHandle
 * ------------------------------------------------------------------------
 * General: Sets the transport manager in message manager.
 * Return Value: RV_OK
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hMsgMgr       - The message manager.
 *        hTransportMgr - The transport manager handle.
 ***************************************************************************/
RvStatus SipMsgMgrSetTransportHandle(IN RvSipMsgMgrHandle       hMsgMgr,
                                            IN RvSipTransportMgrHandle hTransportMgr);

/***************************************************************************
 * SipMessageMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destruct all paramaters of message module.
 *          (hHeadersPool
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsgMgr    - Handle of the message manager.
 ***************************************************************************/
void SipMessageMgrDestruct (IN RvSipMsgMgrHandle hMsgMgr);


#ifdef __cplusplus
}
#endif

#endif /*SIP_MSG_MGR_H*/

