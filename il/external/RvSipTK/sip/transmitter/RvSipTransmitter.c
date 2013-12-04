/*
*********************************************************************************
*                                                                               *
* NOTICE:                                                                       *
* This document contains information that is confidential and proprietary to    *
* RADVision LTD.. No part of this publication may be reproduced in any form     *
* whatsoever without written prior approval by RADVision LTD..                  *
*                                                                               *
* RADVision LTD. reserves the right to revise this publication and make changes *
* without obligation to notify any person of such revisions or changes.         *
*********************************************************************************
*/


/*********************************************************************************
 *                              <RvSipTransmitter.c>
 *
 * This file contains all API function of the transmitter module.
 * The Transmitter functions of the RADVISION SIP stack enable you to create and
 * manage Transmitter objects, and use them to send sip messages.
 * Each transmitter can be used for sending a single sip message.
 * The transmitter performs the address resolution according to the message
 * object and other parameters such as the transmitter outbound address.
 *
 * The Transmitter Manager API
 * ------------------------
 * The Transmitter manager is used mainly for creating
 * new Transmitters.
 *
 * The Transmitter API
 * -----------------
 * A Transmitter object is used for sending a single SIP message (request
 * or response). It will usually be used by proxies to send out of context
 * messages such as ACK or responses that does not match any Transmitter.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                January 2004
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipTransmitterTypes.h"
#include "RvSipTransmitter.h"
#include "RvSipTransport.h"
#include "_SipTransmitter.h"
#include "_SipTransport.h"
#include "TransmitterObject.h"
#include "TransmitterControl.h"
#include "_SipViaHeader.h"
#include "_SipCommonUtils.h"
#include "_SipResolverMgr.h"
#include "TransmitterDest.h"
#include "_SipCommonConversions.h"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV HandleTopViaInAppMsg(
                                     IN Transmitter*          pTrx,
                                     IN RvSipMsgHandle        hMsg);

static RvStatus RVCALLCONV UpdateBranchInAppMsg(
                                     IN Transmitter*          pTrx,
                                     IN RvSipMsgHandle        hMsg);


/*-----------------------------------------------------------------------*/
/*                     TRANSMITTER MANAGER API FUNCTIONS                 */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * RvSipTransmitterMgrCreateTransmitter
 * ------------------------------------------------------------------------
 * General: Creates a new Transmitter and exchange handles with the
 *          application.  The new Transmitter assumes the IDLE state.
 *          The new transmitter can be used for sending one message only.
 *          After creating the transmitter you can set different parameters
 *          using set function and then use the RvSipTransmitterSendMsg to
 *          send the message to the remote party.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrxMgr - Handle to the Transmitter manager
 *            hAppTrx - Application handle to the newly created Transmitter.
 *            pEvHanders  - Event handlers structure for this transmitter.
 *            sizeofEvHandlers - The size of the Event handler structure
 * Output:    phTrx   -   SIP stack handle to the Transmitter
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrCreateTransmitter(
                   IN  RvSipTransmitterMgrHandle   hTrxMgr,
                   IN  RvSipAppTransmitterHandle   hAppTrx,
                   IN  RvSipTransmitterEvHandlers* pEvHandlers,
                   IN  RvInt32                     sizeofEvHandlers,
                   OUT RvSipTransmitterHandle*     phTrx)
{

    RvStatus          rv   = RV_OK;
    Transmitter*      pTrx    = NULL;
    TransmitterMgr*   pTrxMgr = (TransmitterMgr*)hTrxMgr;

    if(phTrx == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phTrx = NULL;

    if(pTrxMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	
    if(pEvHandlers == NULL ||
	   pEvHandlers->pfnStateChangedEvHandler == NULL)
    {
        RvLogError(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
            "RvSipTransmitterMgrCreateTransmitter - Illegal action. pfnStateChangedEvHandler was not supplied"));
        return RV_ERROR_BADPARAM;
    }

    RvLogInfo(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
              "RvSipTransmitterMgrCreateTransmitter - Creating a new Transmitter"));

    /*create a new Transmitter. This will lock the Transmitter manager*/
    rv = TransmitterMgrCreateTransmitter(pTrxMgr,RV_TRUE,hAppTrx,NULL,NULL,NULL,phTrx);
    if(rv != RV_OK)
    {
        RvLogError(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
              "RvSipTransmitterMgrCreateTransmitter - Failed, TransmitterMgr failed to create a new transmitter (rv = %d)",rv));
        return rv;
    }
    pTrx = (Transmitter*)*phTrx;

    /*set the transmitter event handler*/
    memcpy(&pTrx->evHandlers,pEvHandlers,sizeofEvHandlers);

    RvLogInfo(pTrxMgr->pLogSrc,(pTrxMgr->pLogSrc,
              "RvSipTransmitterMgrCreateTransmitter - New Transmitter 0x%p was created successfully",*phTrx));
    return RV_OK;
}

/***************************************************************************
 * RvSipTransmitterMgrGetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: Returns the handle to the application Transmitter manger object.
 *          You set this handle in the stack using the
 *          RvSipTransmitterMgrSetAppMgrHandle() function.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrxMgr - Handle to the stack Transmitter manager.
 * Output:    pAppTrxMgr - The application Transmitter manager handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrGetAppMgrHandle(
                                   IN RvSipTransmitterMgrHandle hTrxMgr,
                                   OUT void**                   pAppTrxMgr)
{
    TransmitterMgr  *pTrxMgr;
    if(hTrxMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pTrxMgr = (TransmitterMgr *)hTrxMgr;

    RvMutexLock(pTrxMgr->pMutex,pTrxMgr->pLogMgr); /*lock the manager*/

    *pAppTrxMgr = pTrxMgr->pAppTrxMgr;

    RvMutexUnlock(pTrxMgr->pMutex,pTrxMgr->pLogMgr); /*unlock the manager*/
    return RV_OK;
}


/***************************************************************************
 * RvSipTransmitterMgrSetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: The application can have its own Transmitter manager handle.
 *          You can use the RvSipTransmitterMgrSetAppMgrHandle function to
 *          save this handle in the stack Transmitter manager object.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrxMgr - Handle to the stack Transmitter manager.
 *           pAppTransmitterMgr - The application Transmitter manager handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrSetAppMgrHandle(
                                   IN RvSipTransmitterMgrHandle hTrxMgr,
                                   IN void*                     pAppTrxMgr)
{
    TransmitterMgr  *pTrxMgr;
    if(hTrxMgr == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    pTrxMgr = (TransmitterMgr *)hTrxMgr;

    RvMutexLock(pTrxMgr->pMutex,pTrxMgr->pLogMgr); /*lock the manager*/

    pTrxMgr->pAppTrxMgr = pAppTrxMgr;

    RvMutexUnlock(pTrxMgr->pMutex,pTrxMgr->pLogMgr); /*unlock the manager*/
    return RV_OK;
}

/***************************************************************************
 * RvSipTransmitterMgrGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this Transmitter
 *          manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrxMgr     - Handle to the stack Transmitter manager.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrGetStackInstance(
                             IN   RvSipTransmitterMgrHandle   hTrxMgr,
                             OUT  void**                      phStackInstance)
{
    TransmitterMgr  *pTrxMgr;

    if (NULL == phStackInstance)
    {
        return RV_ERROR_NULLPTR;
    }
    *phStackInstance = NULL;
    if(hTrxMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pTrxMgr = (TransmitterMgr *)hTrxMgr;

    RvMutexLock(pTrxMgr->pMutex,pTrxMgr->pLogMgr); /*lock the manager*/

    *phStackInstance = pTrxMgr->pStack;

    RvMutexUnlock(pTrxMgr->pMutex,pTrxMgr->pLogMgr); /*unlock the manager*/
    return RV_OK;
}

/************************************************************************************
 * RvSipTransmitterMgrSetDnsServers
 * ----------------------------------------------------------------------------------
 * General: This function is DEPRECATED. The resolver manager is responsible
 * for holding the list of DNS servers. please use the RvSipResolverMgrSetDnsServers()
 * API function.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrxMgr          - The transmitter manager
 *          pDnsServers      - A list of addresses of DNS servers to set to the stack
 *          numOfDnsServers  - The number of DNS servers in the list
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrSetDnsServers(
                              IN  RvSipTransmitterMgrHandle hTrxMgr,
                              IN  RvSipTransportAddr*       pDnsServers,
                              IN  RvInt32                   numOfDnsServers
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
)
{
    RvStatus        rv   = RV_OK;
    TransmitterMgr* pMgr = (TransmitterMgr*)hTrxMgr;

    if (NULL == pMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pDnsServers)
    {
        return RV_ERROR_NULLPTR;
    }


    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
        "RvSipTransmitterMgrSetDnsServers - changing list of DNS servers. (new number of DNS servers is=%d)",numOfDnsServers));
#if defined(UPDATED_BY_SPIRENT)
    rv = SipResolverMgrSetDnsServers(pMgr->hRslvMgr,pDnsServers,numOfDnsServers,vdevblock);
#else
    rv = SipResolverMgrSetDnsServers(pMgr->hRslvMgr,pDnsServers,numOfDnsServers);
#endif
    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipTransmitterMgrSetDnsServers - Failed to change list of DNS servers. (rv=%d)",rv));
    }
    else
    {
        RvLogLeave(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipTransmitterMgrSetDnsServers - changing of DNS servers list done succesfully."));
    }

    return rv;
}

/************************************************************************************
 * RvSipTransmitterMgrGetDnsServers
 * ----------------------------------------------------------------------------------
 * General: This function is DEPRECATED. The resolver manager is responsible
 * for holding the list of DNS servers. please use the RvSipResolverMgrGetDnsServers()
 * API function.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrxMgr          - transmitter mgr
 *          pDomainList      - a list of DNS domain. (an array of NULL terminated strings
 *                             to set to the stack
 *          numOfDomains     - The number of domains in the list
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrGetDnsServers(
                              IN     RvSipTransmitterMgrHandle hTrxMgr,
                              INOUT  RvInt32*                  pNumOfDnsServers,
                              OUT    RvSipTransportAddr*       pDnsServers
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
)

{
    RvStatus        rv   = RV_OK;
    TransmitterMgr* pMgr = (TransmitterMgr*)hTrxMgr;

    if (NULL == pMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pDnsServers || NULL == pNumOfDnsServers)
    {
        return RV_ERROR_NULLPTR;
    }


    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
        "RvSipTransmitterMgrGetDnsServers - changing list of DNS servers. (max number of DNS servers is=%d)",*pNumOfDnsServers));
#if defined(UPDATED_BY_SPIRENT)
    rv = SipResolverMgrGetDnsServers(pMgr->hRslvMgr,pDnsServers,pNumOfDnsServers,vdevblock);
#else
    rv = SipResolverMgrGetDnsServers(pMgr->hRslvMgr,pDnsServers,pNumOfDnsServers);
#endif

    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipTransmitterMgrGetDnsServers - Failed to get list of DNS servers. (rv=%d)",rv));
    }
    else
    {
        RvLogLeave(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipTransmitterMgrGetDnsServers - got list of DNS servers succesfully."));
    }

    return rv;
}


/************************************************************************************
 * RvSipTransmitterMgrSetDnsDomains
 * ----------------------------------------------------------------------------------
 * General: This function is DEPRECATED. The resolver manager is responsible
 * for holding the list of DNS Domains. please use the RvSipResolverMgrSetDnsDomains()
 * API function.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrxMgr          - transmitter mgr
 *          pDomainList      - a list of DNS domain. (an array of NULL terminated strings
 *                             to set to the stack
 *          numOfDomains     - The number of domains in the list
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrSetDnsDomains(
                      IN  RvSipTransmitterMgrHandle   hTrxMgr,
                      IN  RvChar**                    pDomainList,
                      IN  RvInt32                     numOfDomains
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
)
{
    RvStatus        rv   = RV_OK;
    TransmitterMgr* pMgr = (TransmitterMgr*)hTrxMgr;

    if (NULL == pMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pDomainList)
    {
        return RV_ERROR_NULLPTR;
    }


    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
        "RvSipTransmitterMgrSetDnsDomains - changing list of DNS Domains"));

#if defined(UPDATED_BY_SPIRENT)
    rv = SipResolverMgrSetDnsDomains(pMgr->hRslvMgr,pDomainList,numOfDomains,vdevblock);
#else
    rv = SipResolverMgrSetDnsDomains(pMgr->hRslvMgr,pDomainList,numOfDomains);
#endif

    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipTransmitterMgrSetDnsDomains - Failed to change list of DNS Domains. (rv=%d)",rv));
    }
    else
    {
        RvLogLeave(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipTransmitterMgrSetDnsDomains - changing of DNS Domains list done succesfully."));
    }

    return rv;
}

/************************************************************************************
 * RvSipTransmitterMgrGetDnsDomains
 * ----------------------------------------------------------------------------------
 * General: This function is DEPRECATED. The resolver manager is responsible
 * for holding the list of DNS Domains. please use the RvSipResolverMgrGetDnsDomains()
 * API function.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrxMgr          - transmitter mgr
 * Output:  numOfDomains     - The size of the pDomainList array, will reteurn the number
 *                             of domains set to the stack
 *          pDomainList      - A list of DNS domains. (an array of char pointers).
 *                             that array will be filled with pointers to DNS domains.
 *                             The list of DNS domain is part of the stack memory, if
 *                             the application wishes to manipulate that list, It must copy
 *                             the strings to a different memory space.
 *                             The size of this array must be no smaller than numOfDomains
 *          
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrGetDnsDomains(
                      IN     RvSipTransmitterMgrHandle   hTrxMgr,
                      INOUT  RvInt32*                    pNumOfDomains,
                      OUT    RvChar**                    pDomainList
#if defined(UPDATED_BY_SPIRENT)
      , IN RvUint16 vdevblock
#endif
)
{
    RvStatus        rv   = RV_OK;
    TransmitterMgr* pMgr = (TransmitterMgr*)hTrxMgr;

    if (NULL == pMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pDomainList || NULL == pNumOfDomains)
    {
        return RV_ERROR_NULLPTR;
    }


    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
        "RvSipTransmitterMgrGetDnsDomains - Getting list of DNS Domains, max domains =%d",*pNumOfDomains));
#if defined(UPDATED_BY_SPIRENT)
    rv = SipResolverMgrGetDnsDomains(pMgr->hRslvMgr,pDomainList,pNumOfDomains,vdevblock);
#else
    rv = SipResolverMgrGetDnsDomains(pMgr->hRslvMgr,pDomainList,pNumOfDomains);
#endif

    if (RV_OK != rv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipTransmitterMgrGetDnsDomains - Failed to change list of DNS Domains. (rv=%d)",rv));
    }
    else
    {
        RvLogLeave(pMgr->pLogSrc,(pMgr->pLogSrc,
            "RvSipTransmitterMgrGetDnsDomains - changing of DNS Domains list done succesfully."));
    }

    return rv;
}


/*-----------------------------------------------------------------------
       T R A N S M I T T E R    A P I
 ------------------------------------------------------------------------*/


/************************************************************************************
 * RvSipTransmitterSendMessage
 * ----------------------------------------------------------------------------------
 * General: Sends a message to the remote party. The application should supply
 *          the message object that it wishes the transmitter to send.
 *          In order to send the message the transmitter has to resolve the destination
 *          address. The transmitter first moves to the RESOLVING_ADDR
 *          state and starts the address resolution process.
 *          The transmitter calculates the remote address of the message
 *          according to RFC 3261 and RFC 3263 and takes into account the existence of
 *          outbound proxy, Route headers and loose routing rules.
 *          Once address resolution is completed the transmitter moves
 *          to the FINAL_DEST_RESOLVED state. This state is the last chance for
 *          the application to modify the Via header. The transmitter will then 
 *          move to the READY_FOR_SENDING state and will try to send the message. 
 *          If the message was sent successfully the transmitter moves to the MSG_SENT 
 *          state. If the transmitter fails to send the message it will move to 
 *          the MSG_SEND_FAILURE state.
 *          The RvSipTransmitterSendMessage() function can be called in two states:
 *          The IDLS state for initial sending and the MSG_SEND_FAILURE state in order
 *          to send the message to the next address in the DNS list in case the previos
 *          address failed.
 *          
 *          Note1: The DNS procedure is a-synchronic.
 *          Note2: If you wish the transmitter to fix the top via header of the message
 *          object according to the remote party address and transport types you should
 *          first call the RvSipTransmitterSetFixViaFlag() function. Otherwise the
 *          transmitter will only fix the via transport and will add the rport parameter
 *          in case it was configured to the stack. The send-by parameter will remain
 *          untouched.
 *          
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx           - The transmiter handle
 *          hMsgToSend     - The message to send.
 *          bHandleTopVia  - Indicates whether the transmitter should
 *                           add a top Via header to request messages and
 *                           remove the top Via from response messages.
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSendMessage (
                                  IN RvSipTransmitterHandle    hTrx,
                                  IN RvSipMsgHandle            hMsgToSend,
                                  IN RvBool                    bHandleTopVia)
{
    Transmitter*   pTrx     = (Transmitter*)hTrx;
    RvStatus       rv       = RV_OK;
    RvSipMsgHandle hMsgCopy = NULL;

    rv = TransmitterLockAPI(pTrx);
    if (RV_OK != rv)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
#ifndef RV_SIP_JSR32_SUPPORT
    if(pTrx->eState != RVSIP_TRANSMITTER_STATE_IDLE
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
       &&
       pTrx->eState != RVSIP_TRANSMITTER_STATE_MSG_SEND_FAILURE
#endif
       )
#else
   if(pTrx->eState == RVSIP_TRANSMITTER_STATE_RESOLVING_ADDR ||
		   pTrx->eState == RVSIP_TRANSMITTER_STATE_FINAL_DEST_RESOLVED ||
		   pTrx->eState == RVSIP_TRANSMITTER_STATE_TERMINATED)
#endif /*#ifndef RV_SIP_JSR32_SUPPORT*/

    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "RvSipTransmitterSendMessage - transmitter=0x%p Cannot send message in state %s",
                   pTrx, TransmitterGetStateName(pTrx->eState)));
        TransmitterUnLockAPI(pTrx);
        return RV_ERROR_ILLEGAL_ACTION;
    }

   if(pTrx->bSendingBuffer == RV_TRUE)
   {
	   RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
		   "RvSipTransmitterSendMessage - transmitter=0x%p Cannot send message after trying to send a buffer",
		   pTrx));
	   TransmitterUnLockAPI(pTrx);
	   return RV_ERROR_ILLEGAL_ACTION;
   }
  
   
   RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
               "RvSipTransmitterSendMessage - transmitter=0x%p is about to send a message",pTrx));

    /*use the message that exists in the transmitter - only in msg send failure state*/
    if(hMsgToSend == NULL)
    {
        if(pTrx->hMsgToSend != NULL &&
           pTrx->eState == RVSIP_TRANSMITTER_STATE_MSG_SEND_FAILURE)
        {
            RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                       "RvSipTransmitterSendMessage - transmitter=0x%p sending existing msg 0x%p at state %s",
                        pTrx,pTrx->hMsgToSend,TransmitterGetStateName(pTrx->eState)))
            rv = SipTransmitterSendMessage(hTrx,pTrx->hMsgToSend,SIP_TRANSPORT_MSG_APP);
            if(rv != RV_OK)
            {
                RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                    "RvSipTransmitterSendMessage - Transmitter = 0x%p, Failed to send message (rv=%d)",
                    pTrx,rv));
                /* In error case SipTransmitterSendMessage() doesn't free the page*/
                RvSipMsgDestruct(pTrx->hMsgToSend);
                pTrx->hMsgToSend = NULL;
            }
        }
        else
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "RvSipTransmitterSendMessage - transmitter=0x%p cannot send. msg is NULL or state in not msg send failure",pTrx));
            rv = RV_ERROR_BADPARAM;
        }
        TransmitterUnLockAPI(pTrx);
        return rv;
    }

    /*if the transmitter has a message destruct it first*/
    if(pTrx->hMsgToSend != NULL)
    {
        RvSipMsgDestruct(pTrx->hMsgToSend);
        pTrx->hMsgToSend= NULL;
    }

    if(pTrx->bCopyAppMsg == RV_TRUE)
    {
        /*copy the message*/
        rv = RvSipMsgConstruct(pTrx->pTrxMgr->hMsgMgr, pTrx->pTrxMgr->hMessagePool, &hMsgCopy);
        if(RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                "RvSipTransmitterSendMessage - Transmitter = 0x%p, Fail to construct a copy of the given message (rv=%d)",
                pTrx,rv));
            TransmitterUnLockAPI(pTrx);
            return rv;
        }
        rv = RvSipMsgCopy(hMsgCopy, hMsgToSend);
        if(RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                "RvSipTransmitterSendMessage - Transmitter = 0x%p, Fail to Copy the given message (rv=%d)",
                pTrx,rv));
            RvSipMsgDestruct(hMsgCopy);
            TransmitterUnLockAPI(pTrx);
            return rv;
        }
    }
    else
    {
        hMsgCopy = hMsgToSend;
    }
    if(bHandleTopVia == RV_TRUE)
    {
        rv = HandleTopViaInAppMsg(pTrx,hMsgCopy);
        if(rv != RV_OK)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                "RvSipTransmitterSendMessage - Transmitter = 0x%p, Fail to handle top via in message  0x%p(rv=%d)",
                pTrx,hMsgCopy,rv));
            RvSipMsgDestruct(hMsgCopy);
            TransmitterUnLockAPI(pTrx);
            return rv;
        }
   }
   else 
   {
       rv = UpdateBranchInAppMsg(pTrx,hMsgCopy);
       if(rv != RV_OK)
       {
           RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
               "RvSipTransmitterSendMessage - Transmitter = 0x%p, Fail to set via branch in message  0x%p(rv=%d)",
               pTrx,hMsgCopy,rv));
           RvSipMsgDestruct(hMsgCopy);
           TransmitterUnLockAPI(pTrx);
           return rv;
       }
   }

   rv = SipTransmitterSendMessage(hTrx,hMsgCopy,SIP_TRANSPORT_MSG_APP);
   if(rv != RV_OK)
   {
       RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
           "RvSipTransmitterSendMessage - Transmitter = 0x%p, Failed to send message (rv=%d)",
           pTrx,rv));
       /* In error case SipTransmitterSendMessage() doesn't free the page*/
       RvSipMsgDestruct(hMsgCopy);
       TransmitterUnLockAPI(pTrx);
       return rv;
   }

   TransmitterUnLockAPI(pTrx);
   return RV_OK;
}

/************************************************************************************
 * RvSipTransmitterRetransmitMessage
 * ----------------------------------------------------------------------------------
 * General: Retransmits the transmitter internal message.
 *          After the transmitter has successfully sent its initial SIP message, the
 *          application can instruct the transmitter to retransmit the message to the
 *          same destination. The transmitter will send its internal encoded message.
 *          Note: If there is no encoded message in the transmitter, RV_OK will be returned
 *          and the transmitter will not send any message.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx           - The transmitter handle
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterRetransmitMessage (
                                  IN RvSipTransmitterHandle    hTrx)
{
    Transmitter*   pTrx     = (Transmitter*)hTrx;
    RvStatus       rv       = RV_OK;

    rv = TransmitterLockAPI(pTrx);
    if (RV_OK != rv)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	rv = SipTransmitterRetransmit(hTrx);
	if(rv != RV_OK)
	{
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
			"RvSipTransmitterRetransmitMessage - transmitter=0x%p, Failed to retransmit message",
			pTrx));
        TransmitterUnLockAPI(pTrx);
		return rv;
	}
	
	TransmitterUnLockAPI(pTrx);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTrx);
#endif

	return RV_OK;
}

#ifdef RV_SIP_JSR32_SUPPORT
/************************************************************************************
 * RvSipTransmitterSendMessageToDestAddr
 * ----------------------------------------------------------------------------------
 * General: Sends a message to the destination address exists in the transmitter.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx           - The transmitter handle
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSendMessageToDestAddr (
                                  IN RvSipTransmitterHandle    hTrx,
                                  IN RvSipMsgHandle            hMsgToSend)
{
	Transmitter*   pTrx     = (Transmitter*)hTrx;
    RvStatus       rv       = RV_OK;
    RvSipMsgHandle hMsgCopy = NULL;
	
    rv = TransmitterLockAPI(pTrx);
    if (RV_OK != rv)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    /*if the transmitter has a message destruct it first*/
    if(pTrx->hMsgToSend != NULL)
    {
        RvSipMsgDestruct(pTrx->hMsgToSend);
        pTrx->hMsgToSend= NULL;
    }
	
    if(pTrx->bCopyAppMsg == RV_TRUE)
    {
        /*copy the message*/
        rv = RvSipMsgConstruct(pTrx->pTrxMgr->hMsgMgr, pTrx->pTrxMgr->hMessagePool, &hMsgCopy);
        if(RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                "RvSipTransmitterSendMessageToDestAddr - Transmitter = 0x%p, Fail to construct a copy of the given message (rv=%d)",
                pTrx,rv));
            TransmitterUnLockAPI(pTrx);
            return rv;
        }
        rv = RvSipMsgCopy(hMsgCopy, hMsgToSend);
        if(RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                "RvSipTransmitterSendMessageToDestAddr - Transmitter = 0x%p, Fail to Copy the given message (rv=%d)",
                pTrx,rv));
            RvSipMsgDestruct(hMsgCopy);
            TransmitterUnLockAPI(pTrx);
            return rv;
        }
    }
    else
    {
        hMsgCopy = hMsgToSend;
    }

	rv = SipTransmitterSendMessageToPreDiscovered(hTrx,hMsgCopy,SIP_TRANSPORT_MSG_APP);
	if(rv != RV_OK)
	{
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
			"RvSipTransmitterSendMessageToDestAddr - transmitter=0x%p, Failed to retransmit message",
			pTrx));
        TransmitterUnLockAPI(pTrx);
		return rv;
	}
	
	TransmitterUnLockAPI(pTrx);
	return RV_OK;

}
#endif /*#ifdef RV_SIP_JSR32_SUPPORT*/

/************************************************************************************
 * RvSipTransmitterSendBuffer 
 * ----------------------------------------------------------------------------------
 * General: Sends a buffer to the destination.
 *          If the message was sent successfully the transmitter moves the MSG_SENT
 *          state. If the transmitter fails to send the message it will move to
 *          the MSG_SEND_FAILURE state.
 *          If this function returns with an error, the transmitter will not move
 *          to the MSG_SEND_FAILURE state.
 *          In all cases it is the application's responsibility to terminate the
 *          transmitter.
 *          Note: You must set the transmitter's destination address before calling
 *          this function using RvSipTransmitterSetDestAddress().
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx         - The transmitter handle
 *          strBuff      - The buffer to send.
 *          buffSize     - The buffer size
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSendBuffer (
                                  IN RvSipTransmitterHandle    hTrx,
                                  IN RvChar*                   strBuff,
                                  IN RvInt32                   buffSize)
{
    Transmitter*   pTrx     = (Transmitter*)hTrx;
    RvStatus       rv       = RV_OK;

    rv = TransmitterLockAPI(pTrx);
    if (RV_OK != rv)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	if(strBuff == NULL || buffSize == 0)
	{
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
			"RvSipTransmitterSendBuffer - transmitter=0x%p, Failed, no buffer supplied, buffSize = %d",
			pTrx,buffSize));
        TransmitterUnLockAPI(pTrx);
        return RV_ERROR_BADPARAM;
	}

	/*In order to send a buffer the application must supply a destination address
	  first and the state should be TRANSMITTER_RESOLUTION_STATE_RESOLVED*/
	if(pTrx->eResolutionState  != TRANSMITTER_RESOLUTION_STATE_RESOLVED)
	{
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
			"RvSipTransmitterSendBuffer - transmitter=0x%p, No dest address was supplied. Can't send the buffer",
			pTrx));
        TransmitterUnLockAPI(pTrx);
        return RV_ERROR_ILLEGAL_ACTION;

	}
    if(pTrx->eState != RVSIP_TRANSMITTER_STATE_IDLE)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "RvSipTransmitterSendBuffer - transmitter=0x%p Cannot send message in state %s",
                   pTrx, TransmitterGetStateName(pTrx->eState)));
        TransmitterUnLockAPI(pTrx);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
               "RvSipTransmitterSendBuffer - transmitter=0x%p is about to send a buffer",pTrx));

   
   rv = TransmitterControlSendBuffer(pTrx,strBuff,buffSize);
   if(rv != RV_OK)
   {
       RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
           "RvSipTransmitterSendBuffer - Transmitter = 0x%p, Failed to send buffer (rv=%d)",
           pTrx,rv));
       TransmitterUnLockAPI(pTrx);
       return rv;
   }
   
   pTrx->bSendingBuffer = RV_TRUE;

   TransmitterUnLockAPI(pTrx);
   return RV_OK;
}

/************************************************************************************
 * RvSipTransmitterHoldSending
 * ----------------------------------------------------------------------------------
 * General: Hold sending activity of the transmitter object and moves the transmitter
 *          to the ON_HOLD state.
 *          It is the responsibility of the application to resume
 *          the sending of the message using RvSipTransmitterResumeSending()
 *          The application can hold sending activities only in the
 *          FINAL_DEST_RESOLVED state of the transmitter.
 *
 *
 * Return Value: RvStatus -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx    - The transmiter handle
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterHoldSending (
                                  IN RvSipTransmitterHandle    hTrx)
{
    Transmitter* pTrx = (Transmitter*)hTrx;
    RvStatus     rv   = RV_OK;

    rv = TransmitterLockAPI(pTrx);
    if (RV_OK != rv)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pTrx->eState != RVSIP_TRANSMITTER_STATE_FINAL_DEST_RESOLVED)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "RvSipTransmitterHoldSending - transmitter=0x%p Cannot hold sending in state %s",
                   pTrx, TransmitterGetStateName(pTrx->eState)));
        TransmitterUnLockAPI(pTrx);
        return RV_ERROR_ILLEGAL_ACTION;
    }


    if (RVSIP_MSG_REQUEST != RvSipMsgGetMsgType(pTrx->hMsgToSend))
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "RvSipTransmitterHoldSending - transmitter=0x%p can not hold sending for responses",pTrx));
        TransmitterUnLockAPI(pTrx);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "RvSipTransmitterHoldSending - transmitter=0x%p will hold sending",pTrx));

    TransmitterChangeState(pTrx,RVSIP_TRANSMITTER_STATE_ON_HOLD,
                           RVSIP_TRANSMITTER_REASON_UNDEFINED,
                           pTrx->hMsgToSend,NULL);

    TransmitterUnLockAPI(pTrx);
    return rv;
}

/************************************************************************************
 * RvSipTransmitterResumeSending
 * ----------------------------------------------------------------------------------
 * General: Resume sending activity of the transmitter. This function can be used only
 *          in the ON_HOLD state of the transmitter.
 *          The transmitter will check if its DNS list was changed and if so will
 *          return to the RESOLVING_ADDR state and to the address resolution process.
 *          If there is no need for address resolution the transmitter will move to
 *          the READY_FOR_SENDING state and will send the message.
 *
 * Return Value: RvStatus -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:  hTrx        - transmitter, responsible for the DNS procedure
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterResumeSending (
                             IN RvSipTransmitterHandle    hTrx)
{
    Transmitter* pTrx = (Transmitter*)hTrx;
    RvStatus     rv   = RV_OK;

    rv = TransmitterLockAPI(pTrx);
    if (RV_OK != rv)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pTrx->eState != RVSIP_TRANSMITTER_STATE_ON_HOLD)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "RvSipTransmitterResumeSending - transmitter=0x%p Cannot resume sending in state %s",
                   pTrx, TransmitterGetStateName(pTrx->eState)));
        TransmitterUnLockAPI(pTrx);
        return RV_ERROR_ILLEGAL_ACTION;
    }
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "RvSipTransmitterResumeSending - transmitter=0x%p is about to send msg=0x%p",pTrx, pTrx->hMsgToSend));

    if (pTrx->hMsgToSend == NULL)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "RvSipTransmitterResumeSending - Transmitter 0x%p no message to send",pTrx));
        TransmitterUnLockAPI(pTrx);
        return RV_ERROR_INVALID_HANDLE;
    }
    
    /*we call with rv_false since this is an async call that must move the
      transmitter to msg send failure*/
    rv = TransmitterDestDiscovery(pTrx,RV_FALSE);
    if (rv != RV_OK && rv != RV_ERROR_TRY_AGAIN)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "RvSipTransmitterResumeSending - Transmitter 0x%p failed to send message 0x%p",pTrx, pTrx->hMsgToSend));
        TransmitterUnLockAPI(pTrx);
        return rv;
    }

    TransmitterUnLockAPI(pTrx);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransmitterTerminate
 * ------------------------------------------------------------------------
 * General: Terminates a Transmitter. The transmitter will assume the TERMINATED
 *          state.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx - Handle to the Transmitter the user wishes to terminate.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterTerminate (
                                       IN  RvSipTransmitterHandle   hTrx)
{
    RvStatus  rv;
    Transmitter    *pTrx = (Transmitter*)hTrx;

    if(pTrx == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = TransmitterLockAPI(pTrx); /*try to lock the Transmitter*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pTrx->bIsAppTrx == RV_FALSE)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "RvSipTransmitterTerminate - Failed, Transmitter 0x%p, The application cannot terminate a transmitter that does not belong to it",pTrx));
        TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "RvSipTransmitterTerminate - Terminating Transmitter... 0x%p",pTrx));

    if (RVSIP_TRANSMITTER_STATE_TERMINATED == pTrx->eState)
    {
        TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
        return RV_OK;
    }

    TransmitterTerminate(pTrx);
    TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
    return RV_OK;
}

/*-----------------------------------------------------------------------
          T R N S M I T T E R:  G E T   A N D   S E T    A P I
 ------------------------------------------------------------------------*/

/***************************************************************************
 * RvSipTransmitterSetLocalAddress
 * ------------------------------------------------------------------------
 * General:Sets the local address from which the transmitter will send
 * outgoing messages.
 * The SIP Stack can be configured to listen to many local addresses. Each local
 * address has a transport type (UDP/TCP/TLS) and an address type (IPv4/IPv6).
 * When the SIP Stack sends an outgoing message, the local address (from where
 * the message is sent) is chosen according to the characteristics of the remote
 * address. Both the local and remote addresses must have the same characteristics
 * (transport type and address type). If several configured local addresses match
 * the remote address characteristics, the first configured address is taken. You can
 * use RvSipTransmitterSetLocalAddress() to force the transmitter to choose a specific
 * local address for a specific transport and address type. For example, you can
 * force the transmitter to use the second configured UDP/ IPv4 local address. If the
 * transmitter sends a message to a UDP/IPv4 remote address, it will use the local
 * address that you set instead of the default first local address.
 * Note: The localAddress string you provide for this function must match exactly
 * with the local address that was inserted in the configuration structure in the
 * initialization of the SIP Stack. If you configured the SIP Stack to listen to a 0.0.0.0
 * local address, you must use the same notation here.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx            - Handle to the Transmitter
 *            eTransportType  - The transport type(UDP, TCP, SCTP, TLS).
 *            eAddressType    - The address type(ip or ip6).
 *            strLocalAddress - The local address to be set to this Transmitter.
 *            localPort       - The local port to be set to this Transmitter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetLocalAddress(
                            IN  RvSipTransmitterHandle    hTrx,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            IN  RvChar*                   strLocalAddress,
                            IN  RvUint16                  localPort
#if defined(UPDATED_BY_SPIRENT)
			    , IN  RvChar*                 if_name
			    , IN  RvUint16                vdevblock
#endif
			    )
{
    RvStatus         rv;
    Transmitter    *pTrx = (Transmitter*)hTrx;

    if ( pTrx == NULL || strLocalAddress==NULL || localPort==(RvUint16)UNDEFINED)
    {
        return RV_ERROR_BADPARAM;
    }

    /*try to lock the Transmitter*/
    rv = TransmitterLockAPI(pTrx);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    rv = SipTransmitterSetLocalAddress(hTrx,eTransportType,eAddressType,strLocalAddress,localPort
#if defined(UPDATED_BY_SPIRENT)
				       ,if_name
				       ,vdevblock
#endif
				       );

    TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
    return rv;
}

/***************************************************************************
 * RvSipTransmitterGetLocalAddress
 * ------------------------------------------------------------------------
 * General: Gets the local address which the transmitter will use to send
 * outgoing messages to a destination that listens to a specific transport
 * and address type.
 * This is the address the user set using the RvSipTransmitterSetLocalAddress()
 * function. If no address was set, the function will return the default, first
 * configured local address of the requested transport and address type.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx            - Handle to the Transmitter
 *          eTransportType  - The transport type (UDP, TCP, SCTP, TLS).
 *          eAddressType    - The address type (ip4 or ip6).
 * Output:  strLocalAddress - The local address this Transmitter is using(must be longer than 48).
 *          pLocalPort       - The local port this Transmitter is using.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetLocalAddress(
                            IN  RvSipTransmitterHandle    hTrx,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            OUT  RvChar*                  strLocalAddress,
                            OUT  RvUint16*                pLocalPort
#if defined(UPDATED_BY_SPIRENT)
			    , OUT  RvChar*                if_name
			    , OUT  RvUint16*              vdevblock
#endif
			    )
{
    Transmitter*   pTrx  = (Transmitter *)hTrx;
    RvStatus       rv    = RV_OK;

    if (NULL == pTrx || strLocalAddress==NULL || pLocalPort==NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransmitterLockAPI(pTrx))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    rv = SipTransmitterGetLocalAddress(hTrx,eTransportType,eAddressType,strLocalAddress,pLocalPort
#if defined(UPDATED_BY_SPIRENT)
				       ,if_name
				       ,vdevblock
#endif
				       );

    TransmitterUnLockAPI(pTrx);
    return rv;
}



/***************************************************************************
 * RvSipTransmitterSetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Sets all outbound proxy details to the Transmitter object. 
 *          All details are supplied in the RvSipTransportOutboundProxyCfg
 *          structure that includes parameter such as IP address or host name
 *          transport, port and compression type.
 *          Request sent by this object will use the outbound detail specifications
 *          as a remote address. The Request-URI will be ignored.
 *          However, the outbound proxy will be ignored if the message contains
 *          a Route header or if the RvSipTransmitterSetIgnoreOutboundProxyFlag() function
 *          was called.
 *
 *          NOTE: If you specify both IP address and a host name in the
 *                Configuration structure, either of them will be set BUT
 *                the IP address is preferably used.
 *                If you do not specify port or transport, both will be determined
 *                according to the DNS procedures specified in RFC 3263
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx          - Handle to the Transmitter.
 *            pOutboundCfg   - Pointer to the outbound proxy configuration
 *                             structure with all relevant details.
 *            sizeOfCfg      - The size of the outbound proxy configuration structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetOutboundDetails(
                            IN  RvSipTransmitterHandle          hTrx,
                            IN  RvSipTransportOutboundProxyCfg *pOutboundCfg,
                            IN  RvInt32                         cfgStructSize)
{
    RvStatus   rv = RV_OK;
    Transmitter    *pTrx;

    pTrx = (Transmitter *)hTrx;

    if (NULL == pTrx || NULL == pOutboundCfg)
    {
        return RV_ERROR_NULLPTR;
    }

    if (RV_OK != TransmitterLockAPI(pTrx))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    

    /* Setting the Host Name of the outbound proxy BEFORE the ip address  */
    /* in order to assure that is the IP address is set it is going to be */
    /* even if the host name is set too (due to internal preference)      */
    if (pOutboundCfg->strHostName            != NULL &&
        strcmp(pOutboundCfg->strHostName,"") != 0)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "RvSipTransmitterSetOutboundDetails - Transmitter 0x%p - Setting an outbound host name %s:%d ",
            pTrx, (pOutboundCfg->strHostName==NULL)?"NULL":pOutboundCfg->strHostName,pOutboundCfg->port));

        /*we set all info on the size an it will be copied to the Transmitter on success*/
        rv = SipTransportSetObjectOutboundHost(pTrx->pTrxMgr->hTransportMgr,
                                               &pTrx->outboundAddr,
                                               pOutboundCfg->strHostName,
                                               (RvUint16)pOutboundCfg->port,
                                               pTrx->pTrxMgr->hGeneralPool,
                                               pTrx->hPage);
        if(RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "RvSipTransmitterSetOutboundDetails - Transmitter 0x%p Failed (rv=%d)", pTrx,rv));
            TransmitterUnLockAPI(pTrx); /*unlock the Transmitter */;
            return rv;
        }
    }

	/*Setting the transport type here is done in order to use it when undefined port is set.*/
	pTrx->outboundAddr.ipAddress.eTransport = pOutboundCfg->eTransport;

    /* Setting the IP Address of the outbound proxy */
    if(pOutboundCfg->strIpAddress                            != NULL &&
       strcmp(pOutboundCfg->strIpAddress,"")                 != 0    &&
       strcmp(pOutboundCfg->strIpAddress,IPV4_LOCAL_ADDRESS) != 0    &&
       memcmp(pOutboundCfg->strIpAddress, IPV6_LOCAL_ADDRESS, IPV6_LOCAL_ADDRESS_SZ) != 0)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "RvSipTransmitterSetOutboundDetails - Transmitter 0x%p - Setting an outbound address %s:%d ",
            pTrx, (pOutboundCfg->strIpAddress==NULL)?"0":pOutboundCfg->strIpAddress,pOutboundCfg->port));

        rv = SipTransportSetObjectOutboundAddr(pTrx->pTrxMgr->hTransportMgr,
                                               &pTrx->outboundAddr,
                                               pOutboundCfg->strIpAddress,
                                               pOutboundCfg->port);
        if (RV_OK != rv)
        {
            RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "RvSipTransmitterSetOutboundDetails - Failed to set outbound address for Transmitter 0x%p (rv=%d)",
                pTrx,rv));
            TransmitterUnLockAPI(pTrx); /*unlock the Transmitter */
            return rv;
        }
    }

     /* Setting the Transport type of the outbound proxy */
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "RvSipTransmitterSetOutboundDetails - Transmitter 0x%p - Setting outbound transport to %s",
               pTrx, SipCommonConvertEnumToStrTransportType(pOutboundCfg->eTransport)));
    

#ifdef RV_SIGCOMP_ON
    /* Setting the Compression type of the outbound proxy */
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "RvSipTransmitterSetOutboundDetails - Transmitter 0x%p - Setting outbound compression to %s",
               pTrx, SipTransportGetCompTypeName(pOutboundCfg->eCompression)));
    pTrx->outboundAddr.eCompType = pOutboundCfg->eCompression;

	/* setting the sigcomp-id to the outbound proxy */
    if (pOutboundCfg->strSigcompId            != NULL &&
        strcmp(pOutboundCfg->strSigcompId,"") != 0)
	{
	    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
		          "RvSipTransmitterSetOutboundDetails - Transmitter 0x%p - Setting outbound sigcomp-id to %s",
				  pTrx, (pOutboundCfg->strSigcompId==NULL)?"0":pOutboundCfg->strSigcompId));

		rv = SipTransportSetObjectOutboundSigcompId(pTrx->pTrxMgr->hTransportMgr,
													&pTrx->outboundAddr,
													pOutboundCfg->strSigcompId,
													pTrx->pTrxMgr->hGeneralPool,
													pTrx->hPage);
		if (RV_OK != rv)
		{
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                "RvSipTransmitterSetOutboundDetails - Failed to set outbound sigcomp-id for Transmitter 0x%p (rv=%d)",
                pTrx,rv));
            TransmitterUnLockAPI(pTrx); /*unlock the Transmitter */
            return rv;
		}
	}
#endif /* RV_SIGCOMP_ON */

    TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
    RV_UNUSED_ARG(cfgStructSize);

    return rv;
}

/***************************************************************************
 * RvSipTransmitterGetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Gets all outbound proxy details that the Transmitter object uses. 
 *          The details are placed in the RvSipTransportOutboundProxyCfg
 *          structure that includes parameter such as IP address or host name
 *          transport, port and compression type.
 *          If the outbound details were not set to the specific 
 *          Register-Client object but outbound proxy was defined to the stack 
 *          on initialization, the stack parameters will be returned.
 *          If the Transmitter is not using an outbound address NULL/UNDEFINED
 *          values are returned.
 *          NOTE: You must supply a valid consecutive buffer in the 
 *                RvSipTransportOutboundProxyCfg structure to get the
 *                outbound strings (host name and ip address).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx           - Handle to the Transmitter.
 *            sizeOfCfg      - The size of the configuration structure.
 * Output:  pOutboundCfg   - Pointer to outbound proxy configuration
 *                           structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetOutboundDetails(
                            IN  RvSipTransmitterHandle          hTrx,
                            IN  RvInt32                         cfgStructSize,
                            OUT RvSipTransportOutboundProxyCfg* pOutboundCfg)
{
    Transmitter*    pTrx;
    RvStatus        rv;
    RvUint16        port;

    pTrx = (Transmitter *)hTrx;
    if (NULL == pTrx || NULL == pOutboundCfg)
    {
        return RV_ERROR_NULLPTR;
    }

    pOutboundCfg->port = UNDEFINED;

    if (RV_OK != TransmitterLockAPI(pTrx))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* Retrieving the outbound host name */
    rv = SipTransportGetObjectOutboundHost(pTrx->pTrxMgr->hTransportMgr,
                                           &pTrx->outboundAddr,
                                           pOutboundCfg->strHostName,
                                           &port);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                 "RvSipTransmitterGetOutboundDetails - Failed for Transmitter 0x%p (rv=%d)", pTrx,rv));
        TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
        return rv;
    }
    pOutboundCfg->port = ((RvUint16)UNDEFINED==port || port==0) ? UNDEFINED:port;

    /* Retrieving the outbound ip address */
    rv = SipTransportGetObjectOutboundAddr(pTrx->pTrxMgr->hTransportMgr,
                                           &pTrx->outboundAddr,
                                           pOutboundCfg->strIpAddress,
                                           &port);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "RvSipTransmitterGetOutboundDetails - Failed to get outbound address of Transmitter 0x%p,(rv=%d)",
                   pTrx, rv));
        TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
        return rv;
    }
    /* If port was not updated after SipTransportGetObjectOutboundHost(),
       update it now -         after SipTransportGetObjectOutboundAddr() */
    if (UNDEFINED == pOutboundCfg->port)
    {
        pOutboundCfg->port = ((RvUint16)UNDEFINED==port) ? UNDEFINED:port;
    }

    /* Retrieving the outbound transport type */
    rv = SipTransportGetObjectOutboundTransportType(pTrx->pTrxMgr->hTransportMgr,
                                                    &pTrx->outboundAddr,
                                                    &pOutboundCfg->eTransport);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "RvSipTransmitterGetOutboundDetails - Failed to get transport for Transmitter 0x%p (rv=%d)", pTrx,rv));
        TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
        return rv;
    }

#ifdef RV_SIGCOMP_ON
    /* Retrieving the outbound compression type */
    rv = SipTransportGetObjectOutboundCompressionType(pTrx->pTrxMgr->hTransportMgr,
                                                      &pTrx->outboundAddr,
                                                      &pOutboundCfg->eCompression);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "RvSipTransmitterGetOutboundDetails - Failed to get compression for Transmitter 0x%p (rv=%d)", pTrx,rv));
        TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
        return rv;
    }

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
             "RvSipTransmitterGetOutboundDetails - outbound details of Transmitter 0x%p: comp=%s",
              pTrx,SipTransportGetCompTypeName(pOutboundCfg->eCompression)));

	/* retrieving the outbound sigcomp-id */
	rv = SipTransportGetObjectOutboundSigcompId(pTrx->pTrxMgr->hTransportMgr,
												&pTrx->outboundAddr,
												pOutboundCfg->strSigcompId);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                  "RvSipTransmitterGetOutboundDetails - Failed to get sigcomp-id for Transmitter 0x%p (rv=%d)", pTrx,rv));
        TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
        return rv;
    }

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
             "RvSipTransmitterGetOutboundDetails - outbound details of Transmitter 0x%p: sigcomp-id=%s",
              pTrx,pOutboundCfg->strSigcompId));

#endif /* RV_SIGCOMP_ON */

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
             "RvSipTransmitterGetOutboundDetails - outbound details of Transmitter 0x%p: address=%s, host=%s, port=%d, trans=%s",
              pTrx, pOutboundCfg->strIpAddress,pOutboundCfg->strHostName, pOutboundCfg->port,
              SipCommonConvertEnumToStrTransportType(pOutboundCfg->eTransport)));

    TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
    RV_UNUSED_ARG(cfgStructSize);

    return RV_OK;
}

/***************************************************************************
 * RvSipTransmitterSetPersistency
 * ------------------------------------------------------------------------
 * General: Changes the transmitter persistency definition at runtime.
 * This function receives a Boolean value that indicates whether or not the
 * application wishes this transmitter to be persistent. A persistent transmitter object
 * will try to locate a suitable connection in the connection hash before opening a new
 * connection.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx - The Transmitter handle
 *          bIsPersistent - Determines whether the Transmitter will try to use
 *                          a persistent connection or not.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetPersistency(
                           IN RvSipTransmitterHandle  hTrx,
                           IN RvBool                  bIsPersistent)
{
    RvStatus      rv   = RV_OK;
    Transmitter*  pTrx = (Transmitter*)hTrx;
    if(pTrx == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransmitterLockAPI(pTrx); /*try to lock the Transmitter*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "RvSipTransmitterSetPersistency - Setting Transmitter 0x%p persistency to %d ",
              pTrx,bIsPersistent));

    if(pTrx->bIsPersistent == RV_TRUE && bIsPersistent == RV_FALSE &&
       pTrx->hConn != NULL)
    {
        rv = RvSipTransportConnectionDetachOwner(
                                           pTrx->hConn,
                                           (RvSipTransportConnectionOwnerHandle)hTrx);
         if(rv != RV_OK)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                 "RvSipTransmitterSetPersistency - Failed to detach Transmitter 0x%p from prev connection 0x%p",pTrx,pTrx->hConn));
            TransmitterUnLockAPI(pTrx);
            return rv;
        }
        pTrx->hConn = NULL;
    }
    pTrx->bIsPersistent = bIsPersistent;
    TransmitterUnLockAPI(pTrx);
    return RV_OK;
}
/***************************************************************************
 * RvSipTransmitterGetPersistency
 * ------------------------------------------------------------------------
 * General: Returns the transmitter persistency definition.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTrx - The Transmitter handle
 * Output:   pbIsPersistent - The transmitter persistency definition.
 *                            RV_TRUE indicates that the transmitter is
 *                            persistent. Otherwise, RV_FALSE.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransmitterGetPersistency(
                     IN  RvSipTransmitterHandle  hTrx,
                     OUT RvBool*                 pbIsPersistent)
{

    Transmitter        *pTrx= (Transmitter *)hTrx;

    if (NULL == pTrx || NULL == pbIsPersistent)
    {
        return RV_ERROR_NULLPTR;
    }

    *pbIsPersistent = RV_FALSE;
    if (RV_OK != TransmitterLockAPI(pTrx))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *pbIsPersistent = pTrx->bIsPersistent;
    TransmitterUnLockAPI(pTrx);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransmitterSetConnection
 * ------------------------------------------------------------------------
 * General: Sets a connection object to be used by the transmitter.
 * The transmitter object will hold this connection in its internal database.
 * When sending a message the transmitter will use the connection only if it fits the local and
 * remote addresses. Otherwise, the transmitter will either locate
 * a suitable connection in the connection hash or create a new connection. The
 * transmitter object will be informed its owner when usingn a different
 * connection then the one that was supplied.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrx  - Handle to the transmitter
 *          hConn - Handle to the connection.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetConnection(
                   IN  RvSipTransmitterHandle           hTrx,
                   IN  RvSipTransportConnectionHandle   hConn)
{
    Transmitter*    pTrx= (Transmitter *)hTrx;
    if (NULL == pTrx)
    {
        return RV_ERROR_BADPARAM;
    }

    if (RV_OK != TransmitterLockAPI(pTrx))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "RvSipTransmitterSetConnection - Transmitter 0x%p - Setting connection 0x%p", pTrx,hConn));

    if(pTrx->bIsPersistent == RV_FALSE)
    {
         RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
                   "RvSipTransmitterSetConnection - Transmitter 0x%p - Cannot set connection when the Transmitter is not persistent", pTrx));
         TransmitterUnLockAPI(pTrx);
         return RV_ERROR_ILLEGAL_ACTION;
    }
    SipTransmitterSetConnection(hTrx,hConn);
    TransmitterUnLockAPI(pTrx);
    return RV_OK;
}

/***************************************************************************
 * RvSipTransmitterGetConnection
 * ------------------------------------------------------------------------
 * General: Returns the connection that is currently beeing used by the
 *          Transmitter.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx   - Handle to the Transmitter.
 * Output:    phConn - Handle to the currently used connection
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetConnection(
                            IN  RvSipTransmitterHandle             hTrx,
                           OUT  RvSipTransportConnectionHandle *phConn)
{
    Transmitter*     pTrx = (Transmitter*)hTrx;

    if (NULL == pTrx || phConn==NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    *phConn = NULL;
    if (RV_OK != TransmitterLockAPI(pTrx))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "RvSipTransmitterGetConnection - pTrx 0x%p - Getting connection 0x%p",pTrx,pTrx->hConn));

    *phConn = pTrx->hConn;

    TransmitterUnLockAPI(pTrx);
    return RV_OK;

}

/***************************************************************************
 * RvSipTransmitterSetKeepMsgFlag
 * ------------------------------------------------------------------------
 * General: Indicates when he the transmitter should destruct the message.
 *          On termination or after encoding is completed.
 *          By default the message will be destructed immediately after
 *          it has beed encoded.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    - Handle to the Transmitter.
 *            bKeepMsg - RV_TRUE if the application wishes that the transmitter
 *                       will not destruct the message until termination.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetKeepMsgFlag (
                      IN  RvSipTransmitterHandle   hTrx,
                      IN  RvBool                   bKeepMsg)
{
    RvStatus         rv;
    Transmitter*     pTrx = (Transmitter*)hTrx;

    if(pTrx == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the Transmitter*/
    rv = TransmitterLockAPI(pTrx);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "RvSipTransmitterSetKeepMsgFlag - Setting Transmitter 0x%p bKeepMsg =  %d",
              pTrx,bKeepMsg));

    pTrx->bKeepMsg = bKeepMsg;

    TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
    return RV_OK;

}

/***************************************************************************
 * RvSipTransmitterSetFixViaFlag
 * ------------------------------------------------------------------------
 * General: Indicates that the transmitter should update the address of the
 *          top via header before sending a message according to the remote
 *          address transport and address type. The default value of this
 *          parameter is RV_FALSE. If the application sets the top via by
 *          itself this parameter should remain RV_FALSE. In any case
 *          the transmitter will update the transport of the top via header.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    - Handle to the Transmitter.
 *            bFixVia - RV_TRUE if the application wishes that the transmitter
 *                       will update the top via of the message according to the
 *                       destination transport and address type.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetFixViaFlag (
                      IN  RvSipTransmitterHandle   hTrx,
                      IN  RvBool                   bFixVia)
{
    RvStatus         rv;
    Transmitter*     pTrx = (Transmitter*)hTrx;

    if(pTrx == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the Transmitter*/
    rv = TransmitterLockAPI(pTrx);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "RvSipTransmitterSetFixViaFlag - Setting Transmitter 0x%p bFixVia =  %d",
              pTrx,bFixVia));

    pTrx->bFixVia = bFixVia;

    TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
    return RV_OK;

}

/***************************************************************************
 * RvSipTransmitterSetSkipViaProcessingFlag
 * ------------------------------------------------------------------------
 * General: Indicates that the transmitter should not do any via header
 *          processing even if it belongs to an internal stack object that
 *          is currently sending the message.
 *          This function is for internal use only. It is not documented.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    - Handle to the Transmitter.
 *            bSkipViaProcessing - RV_TRUE if the application wishes that the transmitter
 *                       will skip the via processing. RV_FALSE otherwise.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetSkipViaProcessingFlag (
                      IN  RvSipTransmitterHandle   hTrx,
                      IN  RvBool                   bSkipViaProcessing)
{
    RvStatus         rv;
    Transmitter*     pTrx = (Transmitter*)hTrx;

    if(pTrx == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the Transmitter*/
    rv = TransmitterLockAPI(pTrx);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "RvSipTransmitterSetSkipViaProcessingFlag - Setting Transmitter 0x%p bSkipViaProcessing =  %d",
              pTrx,bSkipViaProcessing));

    pTrx->bSkipViaProcessing = bSkipViaProcessing;

    TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
    return RV_OK;

}

/***************************************************************************
 * RvSipTransmitterSetForceOutboundAddrFlag
 * ------------------------------------------------------------------------
 * General: Sets the force outbound addr flag. This flag forces the transmitter
 *          to send request messages to the outbound address regardless of the
 *          message content.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    - Handle to the Transmitter.
 *     	    bForceOutboundAddr - The flag value to set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetForceOutboundAddrFlag (
                      IN  RvSipTransmitterHandle   hTrx,
                      IN  RvBool                   bForceOutboundAddr)
{
    RvStatus         rv;
    Transmitter*     pTrx = (Transmitter*)hTrx;

    if(pTrx == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the Transmitter*/
    rv = TransmitterLockAPI(pTrx);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "RvSipTransmitterSetForceOutboundAddrFlag - Setting Transmitter 0x%p bForceOutboundAddr =  %d",
              pTrx,bForceOutboundAddr));

    pTrx->bForceOutboundAddr = bForceOutboundAddr;

    TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
    return RV_OK;

}


/***************************************************************************
 * RvSipTransmitterSetAppHandle
 * ------------------------------------------------------------------------
 * General: Sets the Transmitter application handle.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx    - Handle to the Transmitter.
 *            hAppTrx - A new application handle to the Transmitter
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetAppHandle (
                                      IN  RvSipTransmitterHandle       hTrx,
                                      IN  RvSipAppTransmitterHandle    hAppTrx)
{
    RvStatus         rv;
    Transmitter*     pTrx = (Transmitter*)hTrx;

    if(pTrx == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the Transmitter*/
    rv = TransmitterLockAPI(pTrx);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "RvSipTransmitterSetAppHandle - Setting Transmitter 0x%p Application handle 0x%p",
              pTrx,hAppTrx));

    pTrx->hAppTrx = hAppTrx;

    TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
    return RV_OK;

}

/***************************************************************************
 * RvSipTransmitterGetAppHandle
 * ------------------------------------------------------------------------
 * General: Returns the application handle of this Transmitter.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx      - Handle to the Transmitter.
 * Output:     phAppTrx - The application handle of the Transmitter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetAppHandle (
                                      IN    RvSipTransmitterHandle        hTrx,
                                      OUT   RvSipAppTransmitterHandle     *phAppTrx)
{
    RvStatus        rv   = RV_OK;
    Transmitter*    pTrx = (Transmitter*)hTrx;

    if(pTrx == NULL || phAppTrx == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the Transmitter*/
    rv = TransmitterLockAPI(pTrx);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phAppTrx = pTrx->hAppTrx;

    TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
    return rv;

}

/***************************************************************************
 * RvSipTransmitterGetCurrentState
 * ------------------------------------------------------------------------
 * General: Gets the Transmitter current state
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx - The Transmitter handle.
 * Output:    peState -  The Transmitter current state.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetCurrentState (
                                      IN  RvSipTransmitterHandle   hTrx,
                                      OUT RvSipTransmitterState*   peState)
{
    RvStatus      rv   = RV_OK;
    Transmitter*  pTrx = (Transmitter*)hTrx;

    if(pTrx == NULL || peState == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /*try to lock the Transmitter*/
    rv = TransmitterLockAPI(pTrx);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *peState = pTrx->eState;
    TransmitterUnLockAPI(pTrx); /*unlock the Transmitter*/
    return rv;
}
/***************************************************************************
 * RvSipTransmitterGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this Transmitter
 *          belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx        - Handle to the Transmitter object.
 * Output:    phStackInstance - A void pointer which will be updated with a
 *                             handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetStackInstance(
                                   IN   RvSipTransmitterHandle   hTrx,
                                   OUT  void**                   phStackInstance)
{
    Transmitter*          pTrx;
    RvStatus              rv;

    if (NULL == phStackInstance)
    {
        return RV_ERROR_NULLPTR;
    }
    *phStackInstance = NULL;

    pTrx = (Transmitter *)hTrx;
    if(pTrx == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    rv = TransmitterLockAPI(pTrx);
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phStackInstance = pTrx->pTrxMgr->pStack;

    TransmitterUnLockAPI(pTrx);
    return RV_OK;
}
/***************************************************************************
 * RvSipTransmitterSetCompartment
 * ------------------------------------------------------------------------
 * General: Associates the Transmitter to a SigComp Compartment.
 *          The transmitter will use this compartment in the message compression
 *          process.
 *          Note: The message will be compresses only if the remote URI
 *                includes the comp=sigcomp parameter.
 * Return Value: RvStatus
 *
 * NOTE: Function deprecated
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx         - The Transmitter handle.
 *            hCompartment - Handle to the SigComp Compartment.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetCompartment(
                            IN RvSipTransmitterHandle hTrx,
                            IN RvSipCompartmentHandle hCompartment)
{

    RV_UNUSED_ARG(hCompartment);
    RV_UNUSED_ARG(hTrx);

    return RV_ERROR_ILLEGAL_ACTION;
}


/***************************************************************************
 * RvSipTransmitterDisableCompression
 * ------------------------------------------------------------------------
 * General: Disables the compression mechanism in a Transmitter.
 *          This means that even if the message indicates compression
 *          it will not be used.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTrx    - The transmitter handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterDisableCompression(
                                IN  RvSipTransmitterHandle hTrx)
{
#ifdef RV_SIGCOMP_ON
    RvStatus   rv       = RV_OK;
    Transmitter *pTrx = (Transmitter*)hTrx;

    if(pTrx == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = TransmitterLockAPI(pTrx); /*try to lock the transmitter*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
              "RvSipTransmitterDisableCompression - disabling the compression in Transmitter 0x%p",
              pTrx));
    rv = SipTransmitterDisableCompression(hTrx);
    TransmitterUnLockAPI(pTrx); /*unlock the transmitter*/

    return rv;
#else /* RV_SIGCOMP_ON */
    RV_UNUSED_ARG(hTrx);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* RV_SIGCOMP_ON */
}

/***************************************************************************
 * RvSipTransmitterGetCompartment
 * ------------------------------------------------------------------------
 * General: Retrieves the associated the SigComp Compartment of a Transmitter
 * Return Value: RvStatus
 *
 * NOTE: Function deprecated
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTrx          - The Transmitter handle.
 *         phCompartment - Pointer of the handle to the SigComp Compartment.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetCompartment(
                            IN  RvSipTransmitterHandle  hTrx,
                            OUT RvSipCompartmentHandle* phCompartment)
{
    RV_UNUSED_ARG(hTrx);
    if (NULL != phCompartment)
    {
        *phCompartment = NULL;
    }

    return RV_ERROR_ILLEGAL_ACTION;
}


/***************************************************************************
 * RvSipTransmitterDNSGetList
 * ------------------------------------------------------------------------
 * General: Retrieves the DNS list object from the transmitter object.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx      - The Transmitter handle
 * Output     phDnsList - Handle to the DNS list.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterDNSGetList(
                                      IN  RvSipTransmitterHandle       hTrx,
                                      OUT RvSipTransportDNSListHandle* phDnsList)
{
    Transmitter*       pTrx;

    if((hTrx == NULL) || (phDnsList == NULL))
    {
        return RV_ERROR_BADPARAM;
    }

    pTrx = (Transmitter *)hTrx;

    if (RV_OK != TransmitterLockAPI(pTrx))
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    *phDnsList = pTrx->hDnsList;

    TransmitterUnLockAPI(pTrx);

    return RV_OK;
}


/***************************************************************************
 * RvSipTransmitterGetCurrentLocalAddress
 * ------------------------------------------------------------------------
 * General: Get the local address the Transmitter will use to send
 *          the next message this function returns the actual address from
 *          the 6 addresses that was used or going to be used.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx - Handle to the transmitter.
 * Output:    eTransporType - The transport type(UDP, TCP, SCTP, TLS).
 *            eAddressType  - The address type(IP4 or IP6).
 *            localAddress - The local address this transmitter is using(must be longer than 48).
 *            localPort - The local port this transmitter is using.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetCurrentLocalAddress(
                            IN  RvSipTransmitterHandle     hTrx,
                            OUT RvSipTransport*            eTransportType,
                            OUT RvSipTransportAddressType* eAddressType,
                            OUT RvChar*                    localAddress,
                            OUT RvUint16*                  localPort
#if defined(UPDATED_BY_SPIRENT)
			    , OUT  RvChar*                 if_name
			    , OUT  RvUint16*               vdevblock
#endif
			    )
{

    Transmitter*       pTrx;
    RvStatus           rv = RV_OK;
    if(hTrx == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    pTrx = (Transmitter *)hTrx;

    if (RV_OK != TransmitterLockAPI(pTrx))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = SipTransmitterGetCurrentLocalAddress(hTrx,eTransportType,eAddressType,
                                              localAddress,localPort
#if defined(UPDATED_BY_SPIRENT)
					      ,if_name
					      ,vdevblock
#endif
					      );
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
               "RvSipTransmitterGetCurrentLocalAddress - Transmitter 0x%p failed to get current local address (rv=%d)",pTrx,rv));
        TransmitterUnLockAPI(pTrx);
        return rv;
    }
    TransmitterUnLockAPI(pTrx);
    return RV_OK;
}

#if defined(UPDATED_BY_SPIRENT)

RvSipTransportLocalAddrHandle RvSipTransmitterGetCurrentLocalAddressHandle(IN  RvSipTransmitterHandle     hTrx)
{
  RvSipTransportLocalAddrHandle handle=NULL;

  Transmitter*       pTrx = NULL;
  RvStatus           rv = RV_OK;

  if(hTrx == NULL) {
    return NULL;
  }

  pTrx = (Transmitter *)hTrx;

  if (RV_OK != TransmitterLockAPI(pTrx)) {
    return NULL;
  }

  handle = SipTransmitterGetCurrentLocalAddressHandle(hTrx);
  if(!handle) {
    RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
				       "RvSipTransmitterGetCurrentLocalAddress - Transmitter 0x%p failed to get current local address",pTrx));
    TransmitterUnLockAPI(pTrx);
    return NULL;
  }

  TransmitterUnLockAPI(pTrx);

  return handle;
}

#endif
//SPIRENT_END

/***************************************************************************
 * RvSipTransmitterGetDestAddress
 * ------------------------------------------------------------------------
 * General: Return the destination address the transmitter will use.
 *
 * Return Value: RvStatus. The function will return RV_ERROR_NOT_FOUND if the
 *               dest address was not yet defined.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx              - The Transmitter handle
 *            addrStructSize    - The size of the pDestAddr struct.
 *            optionsStructSize - The size of the pOptions struct.
 * Output     pDestAddr         - The destination address the transmitter will use.
 *            pOptions          - Advanced instructions for the message sending such
 *                                as using compression
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetDestAddress(
                           IN  RvSipTransmitterHandle      hTrx,
                           IN  RvInt32                     addrStructSize,
                           IN  RvInt32                     optionsStructSize,
                           OUT RvSipTransportAddr*         pDestAddr,
                           OUT RvSipTransmitterExtOptions* pOptions)
{

    Transmitter*       pTrx;
    RvSipTransportAddressType eTrxDestAddrType = RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED;

    if((hTrx == NULL) || (pDestAddr == NULL))
    {
        return RV_ERROR_NULLPTR;
    }

    pTrx = (Transmitter *)hTrx;

    if (RV_OK != TransmitterLockAPI(pTrx))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    eTrxDestAddrType = TransportMgrConvertCoreAddrTypeToSipAddrType(RvAddressGetType(&pTrx->destAddr.addr));

    if(pTrx->destAddr.eTransport == RVSIP_TRANSPORT_UNDEFINED ||
       eTrxDestAddrType == RVSIP_TRANSPORT_ADDRESS_TYPE_UNDEFINED)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
               "RvSipTransmitterGetDestAddress - Transmitter 0x%p dest address was not defined yet",
               hTrx));
       TransmitterUnLockAPI(pTrx);
        return RV_ERROR_NOT_FOUND;
    }
    pDestAddr->eAddrType      = eTrxDestAddrType;
    pDestAddr->eTransportType = pTrx->destAddr.eTransport;
    pDestAddr->port           = RvAddressGetIpPort(&pTrx->destAddr.addr);
    RvAddressGetString(&pTrx->destAddr.addr,RVSIP_TRANSPORT_LEN_STRING_IP,pDestAddr->strIP);
    if(pOptions != NULL)
    {
#ifdef RV_SIGCOMP_ON
        pOptions->eMsgCompType = pTrx->eOutboundMsgCompType;
#endif
    }
    RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
               "RvSipTransmitterGetDestAddress - Transmitter 0x%p dest address is:%s:%u %s",
               pTrx, pDestAddr->strIP,pDestAddr->port,
               SipCommonConvertEnumToStrTransportType(pDestAddr->eTransportType)));
    TransmitterUnLockAPI(pTrx);
    RV_UNUSED_ARG(addrStructSize);
    RV_UNUSED_ARG(optionsStructSize);

    return RV_OK;
}

/***************************************************************************
 * RvSipTransmitterSetDestAddress
 * ------------------------------------------------------------------------
 * General: Sets the destination address the transmitter will use.
 *          Use this function when you want the transmitter to use
 *          a specific address regardless of the message content.
 *          If NULL was set as the dest address, the stack will continue
 *          the resolution process as if the dest IP was not received and
 *          the dest was not resolved yet.
 *
 *          NOTE: Sending the transmitter for another "round" with the
 *                DNS can take time as DNS requests and responses are exchanged
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx              - The Transmitter handle
 *            pDestAddr         - The destination address the transmitter will use.
 *                                if set to NULL, the transmitter will
 *                                continue the DNS procedure as if the dest ip was
 *                                never retrieved.
 *            addrStructSize    - The size of the pDestAddr struct.
 *            pOptions          - Advanced instructions for the message sending such
 *                                as using compression
 *            optionsStructSize - The size of the pOptions struct.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetDestAddress(
                          IN RvSipTransmitterHandle        hTrx,
                          IN RvSipTransportAddr*           pDestAddr,
                          IN RvInt32                       addrStructSize,
                          IN RvSipTransmitterExtOptions*   pOptions,
                          IN RvInt32                       optionsStructSize)
{
    Transmitter*         pTrx;
    SipTransportAddress  dest;
    RvStatus             rv = RV_OK;

    if(hTrx == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    pTrx = (Transmitter *)hTrx;

    /* if the dest address is NULL, we wont be using it's internals */
    if (NULL != pDestAddr)
    {
        /*check the address type*/
        if(pDestAddr->eAddrType != RVSIP_TRANSPORT_ADDRESS_TYPE_IP &&
           pDestAddr->eAddrType != RVSIP_TRANSPORT_ADDRESS_TYPE_IP6)
        {
            return RV_ERROR_BADPARAM;
        }

        /*check the transport type*/
        if(pDestAddr->eTransportType != RVSIP_TRANSPORT_UDP &&
           pDestAddr->eTransportType != RVSIP_TRANSPORT_TCP &&
           pDestAddr->eTransportType != RVSIP_TRANSPORT_TLS &&
           pDestAddr->eTransportType != RVSIP_TRANSPORT_SCTP)
        {
            return RV_ERROR_BADPARAM;
        }

        if((strcmp(pDestAddr->strIP,"") == 0) ||
           pDestAddr->port == 0)
        {
            return RV_ERROR_BADPARAM;
        }
    }

    if (RV_OK != TransmitterLockAPI(pTrx))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (NULL == pDestAddr)
    {
        RvSipTransportDNSSRVElement      srvElement;
        RvSipTransportDNSHostNameElement hostElement;

        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "RvSipTransmitterSetDestAddress - hTrx=0x%p: Application has decided to reject the DNS decision. %s",
            pTrx,"Will try to continue dns resolution"));
        memset(&pTrx->destAddr,0,sizeof(pTrx->destAddr));
        memset(&srvElement,0,sizeof(srvElement));
        memset(&hostElement,0,sizeof(hostElement));
        
        pTrx->destAddr.eTransport = RVSIP_TRANSPORT_UNDEFINED;
        RvSipTransportDNSListSetUsedSRVElement(pTrx->pTrxMgr->hTransportMgr,pTrx->hDnsList,&srvElement);
        RvSipTransportDNSListSetUsedHostElement(pTrx->pTrxMgr->hTransportMgr,pTrx->hDnsList,&hostElement);
        pTrx->bAddrManuallySet = RV_FALSE;
        pTrx->eResolutionState = TRANSMITTER_RESOLUTION_STATE_UNDEFINED;
        TransmitterUnLockAPI(pTrx);
        return RV_OK;
    }

    dest.eTransport = pDestAddr->eTransportType;
    rv = SipTransportConvertApiTransportAddrToCoreAddress(
            pTrx->pTrxMgr->hTransportMgr, pDestAddr, &dest.addr);
    if (RV_OK != rv)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "RvSipTransmitterSetDestAddress - hTrx=0x%p: Bad address string: %s",
            pTrx,(NULL == pDestAddr->strIP) ? "(NULL)":pDestAddr->strIP ));
        TransmitterUnLockAPI(pTrx);
        return rv;
    }
#if (RV_NET_TYPE & RV_NET_IPV6)
    RvAddressSetIpv6Scope(&dest.addr,0);
#endif /*#if (RV_NET_TYPE & RV_NET_IPV6)*/
    TransmitterSetDestAddr(pTrx,&dest,pOptions);
    pTrx->bAddrManuallySet = RV_TRUE;

    RvAddressDestruct(&dest.addr);
    TransmitterUnLockAPI(pTrx);
    RV_UNUSED_ARG(addrStructSize);
    RV_UNUSED_ARG(optionsStructSize);

    return RV_OK;
}



/***************************************************************************
 * RvSipTransmitterSetIgnoreOutboundProxyFlag
 * ------------------------------------------------------------------------
 * General:  Instructs the Transmitter to ignore its outbound proxy when sending
 *           requests.
 *           In some cases the application will want the transmitter to
 *           ignore its outbound proxy event if it is configured to use one.
 *           An example is when the request uri was calculated from a Route
 *           header that was found in the message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   Trx                  - The Transmitter handle.
 *          bIgnoreOutboundProxy - RV_TRUE if you wish the Transmitter to ignored
 *          its configured outbound proxy, RV_FALSE otherwise.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetIgnoreOutboundProxyFlag(
                                 IN  RvSipTransmitterHandle hTrx,
                                 IN  RvBool                 bIgnoreOutboundProxy)
{
    RvStatus  rv;

    Transmitter    *pTrx = (Transmitter*)hTrx;
    if(pTrx == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    rv = TransmitterLockAPI(pTrx); /*try to lock the Transmitter*/
    if(rv != RV_OK)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pTrx->bIgnoreOutboundProxy = bIgnoreOutboundProxy;
    TransmitterUnLockAPI(pTrx);
    return RV_OK;
}


/***************************************************************************
 * RvSipTransmitterSetUseFirstRouteFlag
 * ------------------------------------------------------------------------
 * General: Indicates that a message should be sent to the first Route header
 *          and not according to the request uri. This will be the case when
 *          the next hop is a loose router.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTrx      - The transmitter to set the bSendToFirstRouteHeader in.
 *            bSendToFirstRoute - RV_TRUE if the message should be sent to the
 *                                first route. RV_FALSE otherwise.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetUseFirstRouteFlag(
                                  IN RvSipTransmitterHandle hTrx,
                                  IN RvBool                 bSendToFirstRoute)
{
    Transmitter  *pTrx;

    if(hTrx == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTrx = (Transmitter *)hTrx;

    if (RV_OK != TransmitterLockAPI(pTrx))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pTrx->bSendToFirstRoute = bSendToFirstRoute;

    TransmitterUnLockAPI(pTrx);

    return RV_OK;
}


/***************************************************************************
 * RvSipTransmitterSetViaBranch
 * ------------------------------------------------------------------------
 * General: Sets the branch parameter to the transmitter.
 *          The transmitter will add the branch to the top Via header
 *          of outgoing messages.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTrx          - The transmitter to set the bSendToFirstRouteHeader in.
 *         strViaBranch  - The Via branch to add to the top via header.
 *                          This parameter is ignored for response messages or when
 *                          the handleTopVia parameter is RV_FALSE.
 *         pRpoolViaBranch - The branch supplied on a page. You should set this
 *                          parameter to NULL if the branch was supplied as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetViaBranch(
                                  IN RvSipTransmitterHandle hTrx,
                                  IN RvChar*                strViaBranch,
                                  IN RPOOL_Ptr*             pRpoolViaBranch)
{
    RvStatus     rv     = RV_OK;
    Transmitter  *pTrx;

    if(hTrx == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    if(strViaBranch == NULL && pRpoolViaBranch==NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    pTrx = (Transmitter *)hTrx;

    if (RV_OK != TransmitterLockAPI(pTrx))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = TransmitterSetBranch(pTrx,strViaBranch,pRpoolViaBranch);
    if(rv != RV_OK)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "RvSipTransmitterSetViaBranch - Failed to set branch for transmitter 0x%p (rv=%d)",
            pTrx,rv));

        TransmitterUnLockAPI(pTrx);
        return rv;
    }
    TransmitterUnLockAPI(pTrx);

    return RV_OK;
}

#if (RV_NET_TYPE & RV_NET_SCTP)
/******************************************************************************
 * RvSipTransmitterSetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Sets parameters that will be used while sending message
 *          by the Transmitter, over SCTP.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hTrx       - Handle to the Transmitter object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams    - Pointer to the struct contains all the parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetSctpMsgSendingParams(
                    IN  RvSipTransmitterHandle             hTrx,
                    IN  RvInt32                            sizeOfParamsStruct,
                    IN  RvSipTransportSctpMsgSendingParams *pParams)
{
    RvStatus rv;
    Transmitter  *pTrx = (Transmitter*)hTrx;
    RvSipTransportSctpMsgSendingParams params;
    RvUint32 minStructSize = RvMin(((RvUint)sizeOfParamsStruct),sizeof(RvSipTransportSctpMsgSendingParams));

    if(pTrx == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    memset(&params,0,sizeof(RvSipTransportSctpMsgSendingParams));
    memcpy(&params,pParams,minStructSize);

    rv = TransmitterLockAPI(pTrx);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "RvSipTransmitterSetSctpMsgSendingParams - transmitter 0x%p - failed to lock the Transmitter",
            pTrx));
        return RV_ERROR_INVALID_HANDLE;
    }

    SipTransmitterSetSctpMsgSendingParams(hTrx, &params);

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "RvSipTransmitterSetSctpMsgSendingParams - transmitter 0x%p - SCTP params(stream=%d, flag=0x%x) were set",
        pTrx, pParams->sctpStream, pParams->sctpMsgSendingFlags));

    TransmitterUnLockAPI(pTrx);
    return RV_OK;
}

/******************************************************************************
 * RvSipTransmitterGetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Gets parameters that are used while sending message
 *          by the Transmitter, over SCTP.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hTrx       - Handle to the Transmitter object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams    - Pointer to the struct contains all the parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetSctpMsgSendingParams(
                    IN  RvSipTransmitterHandle             hTrx,
                    IN  RvInt32                            sizeOfParamsStruct,
                    OUT RvSipTransportSctpMsgSendingParams *pParams)
{
    RvStatus rv;
    Transmitter  *pTrx = (Transmitter*)hTrx;
    RvSipTransportSctpMsgSendingParams params;
    RvUint32 minStructSize = RvMin(((RvUint)sizeOfParamsStruct),sizeof(RvSipTransportSctpMsgSendingParams));

    if(pTrx == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = TransmitterLockAPI(pTrx);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "RvSipTransmitterGetSctpMsgSendingParams - transmitter 0x%p - failed to lock the Transmitter",
            pTrx));
        return RV_ERROR_INVALID_HANDLE;
    }

    memset(&params,0,sizeof(RvSipTransportSctpMsgSendingParams));
    SipTransmitterGetSctpMsgSendingParams(hTrx, &params);
    memcpy(pParams,&params,minStructSize);

    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "RvSipTransmitterGetSctpMsgSendingParams - transmitter 0x%p - SCTP params(stream=%d, flag=0x%x) were get",
        pTrx, pParams->sctpStream, pParams->sctpMsgSendingFlags));

    TransmitterUnLockAPI(pTrx);
    return RV_OK;
}
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipTransmitterSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Transmitter.
 *          As a result of this operation, all messages, sent by this
 *          Transmitter, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hTrx - Handle to the Transmitter object.
 *          hSecObj - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterSetSecObj(
                                        IN  RvSipTransmitterHandle    hTrx,
                                        IN  RvSipSecObjHandle         hSecObj)
{
    RvStatus     rv;
    Transmitter* pTrx = (Transmitter*)hTrx;

    if(NULL == pTrx)
    {
        return RV_ERROR_BADPARAM;
    }
    
    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "RvSipTransmitterSetSecObj - Transmitter 0x%p - Set Security Object %p",
        pTrx, hSecObj));
    
    /*try to lock the Transmitter*/
    rv = TransmitterLockAPI(pTrx);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "RvSipTransmitterSetSecObj - Transmitter 0x%p - failed to lock the Trx",
            pTrx));
        return RV_ERROR_INVALID_HANDLE;
    }
    
    rv = TransmitterSetSecObj(pTrx, hSecObj);
    if (RV_OK != rv)
    {
        TransmitterUnLockAPI(pTrx);
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "RvSipTransmitterSetSecObj - Transmitter 0x%p - failed to set Security Object %p(rv=%d:%s)",
            pTrx, hSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }
    
    TransmitterUnLockAPI(pTrx);
    return RV_OK;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipTransmitterGetSecObj
 * ----------------------------------------------------------------------------
 * General: Gets Security Object set into the Transmitter.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hTrx     - Handle to the Transmitter object.
 *  Output: phSecObj - Handle to the Security Object.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterGetSecObj(
                                        IN  RvSipTransmitterHandle    hTrx,
                                        OUT RvSipSecObjHandle*        phSecObj)
{
    RvStatus     rv;
    Transmitter* pTrx = (Transmitter*)hTrx;
    
    if(NULL==pTrx || NULL==phSecObj)
    {
        return RV_ERROR_BADPARAM;
    }
    
    RvLogInfo(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
        "RvSipTransmitterGetSecObj - Transmitter 0x%p - Get Security Object",
        pTrx));
    
    /*try to lock the Transmitter*/
    rv = TransmitterLockAPI(pTrx);
    if(rv != RV_OK)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc,
            "RvSipTransmitterGetSecObj - Transmitter 0x%p - failed to lock the Trx",
            pTrx));
        return RV_ERROR_INVALID_HANDLE;
    }
    
    *phSecObj = pTrx->hSecObj;
    
    TransmitterUnLockAPI(pTrx);
    return RV_OK;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * HandleTopViaInAppMsg
 * ------------------------------------------------------------------------
 * General: The function will remove top via from responses and
 *                    will add top via to requests.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx  -   The transmitter handle
 *          hMsg  -   The Msg handle
 ***************************************************************************/
static RvStatus RVCALLCONV HandleTopViaInAppMsg(
                                     IN Transmitter*          pTrx,
                                     IN RvSipMsgHandle        hMsg)
{
    RvStatus rv = RV_OK;

    if(RvSipMsgGetMsgType(hMsg) == RVSIP_MSG_RESPONSE)
    {
        RvSipHeaderListElemHandle   hListElem;
        RvSipViaHeaderHandle        hVia;
        /*get the top via element*/
        hVia = (RvSipViaHeaderHandle)RvSipMsgGetHeaderByType(
                                           hMsg,RVSIP_HEADERTYPE_VIA,
                                           RVSIP_FIRST_HEADER,&hListElem);
        if(hVia == NULL || hListElem == NULL)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                      "HandleTopViaInAppMsg - Transmitter 0x%p Failed: No top via header in message 0x%p can't remove",
                       pTrx,hMsg));
            return RV_ERROR_NOT_FOUND;
        }
        /*remove the element*/
        rv = RvSipMsgRemoveHeaderAt(hMsg,hListElem);
        if(rv != RV_OK)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                      "HandleTopViaInAppMsg - Transmitter 0x%p Failed to remove top Via from message 0x%p (rv=%d)",
                       pTrx, hMsg, rv));
            return rv;
        }
        else
        {
            RvLogDebug(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                "HandleTopViaInAppMsg - Transmitter 0x%p Successfully removed Top Via from response message 0x%p",
                pTrx,hMsg));
            return RV_OK;
        }
    }
    else if(RvSipMsgGetMsgType(hMsg) == RVSIP_MSG_REQUEST)
    {
        rv = TransmitterAddNewTopViaToMsg(pTrx,hMsg,NULL,RV_FALSE);
        if(rv != RV_OK)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc,(pTrx->pTrxMgr->pLogSrc ,
                      "HandleTopViaInAppMsg - Transmitter 0x%p Failed to add top Via to message 0x%p (rv=%d)",
                       pTrx, hMsg, rv));
            return rv;
        }
        return RV_OK;
    }
    else
    {
         RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
            "HandleTopViaInAppMsg - Transmitter 0x%p Failed, message 0x%p has no type",
            pTrx, hMsg));
         return RV_ERROR_BADPARAM;
    }
}

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
RvStatus RVCALLCONV  RvSipTransmitterAddNewTopViaToMsg(IN  RvSipTransmitterHandle          hTrx, IN RvSipMsgHandle hMsg) {
  return TransmitterAddNewTopViaToMsg((Transmitter*)hTrx,hMsg,NULL,RV_FALSE);
}
#endif
/* SPIRENT_END */

/***************************************************************************
 * UpdateBranchInAppMsg
 * ------------------------------------------------------------------------
 * General: The function will set a new branch to the top via header of
 *          the message. (only for requests).
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTrx  -   The transmitter handle
 *          hMsg  -   The Msg handle
 ***************************************************************************/
static RvStatus RVCALLCONV UpdateBranchInAppMsg(
                                     IN Transmitter*          pTrx,
                                     IN RvSipMsgHandle        hMsg)

{
    RvSipViaHeaderHandle        hVia                  = NULL;
    RvSipHeaderListElemHandle   hListElem             = NULL;
    RvStatus                    rv                    = RV_OK;
    RvInt32                     currentBranchOffset   = UNDEFINED;
    RvChar                      strBranch[SIP_COMMON_ID_SIZE];
    RvSipMsgType                eMsgType;

    eMsgType = RvSipMsgGetMsgType(hMsg);
    if(eMsgType == RVSIP_MSG_RESPONSE)
    {
        return RV_OK;
    }

    hVia = (RvSipViaHeaderHandle)RvSipMsgGetHeaderByType(
                                            hMsg,
                                            RVSIP_HEADERTYPE_VIA,
                                            RVSIP_FIRST_HEADER,
                                            &hListElem);
    if (hVia == NULL)
    {
        RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
            "UpdateBranchInAppMsg - trx=0x%p: Failed to find via header in msg 0x%p, will try to send the message any way (rv=%d)",
            pTrx,hMsg,rv));
        return RV_OK; /*we will try to send the message any way*/
    }


    currentBranchOffset = SipViaHeaderGetBranchParam(hVia);
    /*if there is no branch in the message or in the trx, generate one.*/
    if(pTrx->viaBranchOffset == UNDEFINED && currentBranchOffset == UNDEFINED)
    {
        RvLogDebug(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                "UpdateBranchInAppMsg - trx=0x%p: no branch in via header. generating one...",
                pTrx));
        TransmitterGenerateBranchStr(pTrx,strBranch);
        rv = TransmitterSetBranch(pTrx,strBranch,NULL);
        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                "UpdateBranchInAppMsg - trx=0x%p: Failed to set branch in via header (rv=%d)",
                pTrx, rv));
            return rv;
        }
    }
    /*set the branch only if it exists*/
    if(pTrx->viaBranchOffset != UNDEFINED)
    {
        rv = SipViaHeaderSetBranchParam(hVia,NULL,pTrx->hPool,pTrx->hPage,pTrx->viaBranchOffset);
        if (RV_OK != rv)
        {
            RvLogError(pTrx->pTrxMgr->pLogSrc ,(pTrx->pTrxMgr->pLogSrc ,
                "UpdateBranchInAppMsg - trx=0x%p: Failed to set branch in via header (rv=%d)",
                pTrx, rv));
            return rv;
        }
    }
    return RV_OK;
}
/***************************************************************************
 * RvSipTransmitterMgrSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets event handlers for the transmitter manager events.
 *          The application has to supply a structure that contains
 *          function pointers to all the transmitter manager events that it
 *          wishes to listen to.
 *          Note: As apposed to the transmitter object events where you
 *          can supply a different set of events for each transmitter
 *          object, the event structure you supply to this function will
 *          be used for all the events called by the transmitter manager.
 *          
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTrxMgr - A handle to the Transmitter manager.
 *          pEvHandlers  - A pointer to the structure containing application 
 *                       event handler pointers.
 *          evHandlerStructSize - The size of the pEvHandlers structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransmitterMgrSetEvHandlers(
                        IN RvSipTransmitterMgrHandle        hTrxMgr,
                        IN RvSipTransmitterMgrEvHandlers      *pEvHandlers,
                        IN RvUint32                        evHandlerStructSize)
{
    TransmitterMgr      *pTrxMgr = (TransmitterMgr *)hTrxMgr;

    RvInt32 minStructSize = (evHandlerStructSize < sizeof(RvSipTransmitterMgrEvHandlers)) ?
                              evHandlerStructSize : sizeof(RvSipTransmitterMgrEvHandlers);

    if ((NULL == hTrxMgr) || (NULL == pEvHandlers) )
    {
        return RV_ERROR_NULLPTR;
    }

    RvMutexLock(pTrxMgr->pMutex,pTrxMgr->pLogMgr);

    if (evHandlerStructSize == 0)
    {
        RvMutexUnlock(pTrxMgr->pMutex,pTrxMgr->pLogMgr);
        return RV_ERROR_BADPARAM;
    }

    /* The manager signs in is the application manager:
       Set application's calbacks*/
    memset(&(pTrxMgr->mgrEvents), 0, sizeof(RvSipTransmitterMgrEvHandlers));
    memcpy(&(pTrxMgr->mgrEvents), pEvHandlers, minStructSize);

    RvLogDebug(pTrxMgr->pLogSrc ,(pTrxMgr->pLogSrc ,
              "RvSipTransmitterMgrSetEvHandlers - application event handlers were successfully set to the Transmitter manager"));
    RvMutexUnlock(pTrxMgr->pMutex,pTrxMgr->pLogMgr);
    return RV_OK;
}



