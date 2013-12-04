/*
*******************************************************************************
*                                                                             *
* NOTICE:                                                                     *
* This document contains information that is confidential and proprietary to  *
* RADVision LTD.. No part of this publication may be reproduced in any form   *
* whatsoever without written prior approval by RADVision LTD..                *
*                                                                             *
* RADVision LTD. reserves the right to revise this publication and make       *
* changes without obligation to notify any person of such revisions or changes*
*******************************************************************************
*/

/******************************************************************************
 *                              <SecurityMgrObject.h>
 *
 * The SecurityMgrObject.h file defined the Security Manager object, object
 * inspector and modifier functions, object action functions.
 *
 *
 *    Author                        Date
 *    ------------                  ------------
 *    Igor                          Februar 2006
 *****************************************************************************/

#ifndef SECURITY_MANAGER_H
#define SECURITY_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                            INCLUDE HEADER FILES                           */
/*---------------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#include "RvSipSecurityTypes.h"
#include "rvlog.h"
#include "rvmutex.h"
#include "AdsRlist.h"

#ifdef RV_SIP_IMS_ON

/*---------------------------------------------------------------------------*/
/*                            TYPE DEFINITIONS                               */
/*---------------------------------------------------------------------------*/

/* SecMgr
 * ----------------------------------------------------------------------------
 * SecMgr represents a Manager of SIP Toolkit Security module.
 * Security module implements:
 * 1. Security Agreement mechanism, described in RFC3329
 * 2. SIP message protection, described in 3GPP TS33.203
 * 3. IPsec for messages, sent and received by Toolkit objects, e.g. SecObj
 *
 * pLogMgr         - Handle to the Log Manager
 * pLogSrc         - Handle to the Log Source
 * pSelectEngine   - Pointer to the Select Engine
 * hMsgMgr         - Handle to the Message Manager
 * hTransportMgr   - Handle to the Transport Manager
 * hStack          - Handle to the SIP Toolkit Stack instance
 * pAppSecMgr      - Application handle to the Security Manager
 * appEvHandlers   - Event Handlers, set by application
 * ownerEvHandlers - Event Handlers, set by internal module
 * bTcpEnabled     - RV_TRUE, if Application enabled TCP
 * appEvHandlers   - Union of application callbacks, called by Manager
 * maxSecObjects   - Size of pool of Security Objects
 * spiRangeStart   - lowest SPI, that can be used by Security Manager
 * spiRangeEnd  - highest SPI, that can be used by Security Manager
 * hSecObjPool     - Handle to the pool of Security Objects
 * hSecObjList     - Handle to the list of elements from Security Object Pool
 * maxIpsecSessions  - Size of pool of IPsec Sessions
 * hIpsecSessionPool - Handle to the pool of IPsec Sessions.
 *                   Each Security Object can handle 0 or 1 IPsec Session
 * hIpsecSessionList - Handle to the list of IPsec Sessions.
 * pSeed           - Is used to generate unique identifier for Security Objects
 * ccImsCfg        - IPsec configuration, used by Common Core
 * hMutex          - Lock, defending the Pools
 */
typedef struct
{
    RvLogMgr*               pLogMgr;
    RvLogSource*            pLogSrc;
    RvSelectEngine*         pSelectEngine;

    RvSipMsgMgrHandle       hMsgMgr;
    RvSipTransportMgrHandle hTransportMgr;
    void*                   hStack;

    void*                   pAppSecMgr;
    RvSipSecEvHandlers      appEvHandlers;

    RvSipSecEvHandlers      ownerEvHandlers;

    RvBool                  bTcpEnabled;
    RvInt32                 maxSecObjects;
    RvUint32                spiRangeStart;
    RvUint32                spiRangeEnd;
    RLIST_POOL_HANDLE       hSecObjPool;
    RLIST_HANDLE            hSecObjList;
    RvInt32                 maxIpsecSessions;
    RLIST_POOL_HANDLE       hIpsecSessionPool;
    RLIST_HANDLE            hIpsecSessionList;
    RvRandomGenerator*      pSeed;
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    RvImsCfgData            ccImsCfg;
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
    RvMutex                 hMutex;
#endif
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    void*                      (*getAlternativeSecObjFunc)(void* so,int isResp);
#endif
//SPIRENT_END
} SecMgr;

/*---------------------------------------------------------------------------*/
/*                            FUNCTIONS HEADERS                              */
/*---------------------------------------------------------------------------*/

/******************************************************************************
 * SecMgrCreateSecObj
 * ----------------------------------------------------------------------------
 * General: Allocates a Security Object.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr       - Pointer to the Security Manager
 *          hAppSecObj - Application handle to the allocated Security Object
 * Output:  phSecObj   - Pointer to the allocated Security Object
*****************************************************************************/
RvStatus SecMgrCreateSecObj(IN  SecMgr*              pMgr,
                            IN  RvSipAppSecObjHandle hAppSecObj,
                            OUT RvSipSecObjHandle*   phSecObj);

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
#define SecMgrLock(_m,_l)       RvMutexLock(_m,_l)
#define SecMgrUnlock(_m,_l)     RvMutexUnlock(_m,_l)
#else /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/
#define SecMgrLock(_m,_l)   (RV_OK)
#define SecMgrUnlock(_m,_l)
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) #else*/

#endif /* END OF: #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef SECURITY_MANAGER_H*/

