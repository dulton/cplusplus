#ifdef __cplusplus
extern "C" {
#endif

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
* Copyright RADVision 1996.                                                     *
* Last Revision:                                                       *
*********************************************************************************
*/


/*********************************************************************************
 *                              TransportCallbacks.c
 *
 * From this c file all transport layer callbacks are called.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                1.4.2003
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#include "TransportCallbacks.h"
#include "_SipTransport.h"
#include "TransportTLS.h"
#include "_SipCommonUtils.h"
#include "_SipCommonConversions.h"
    

/*-------------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                              */
/*-------------------------------------------------------------------------*/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static const RvChar* GetBSActionName(RvSipTransportBsAction bsAction);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
/*-------------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                              */
/*-------------------------------------------------------------------------*/
/***************************************************************************
 * TransportCallbackMsgToSendEv
 * ------------------------------------------------------------------------
 * General:  notifies the application that a new message is about to be sent.
 *           The application can decide whether the transport layer
 *           should not transmit the message to its destination.
 * Return Value: RV_TURE to transmit the message. RV_FALSE to discard the
 *               message without transmiting it to destination.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTrasportMgr - A pointer to the transport manager object.
 *          msgBuffer - The about to be sent message (ggiven in a consecutive
 *                      buffer).
 *          bufferLen - The length of the message buffer.
 ***************************************************************************/
RvBool RVCALLCONV  TransportCallbackMsgToSendEv(
                      IN    TransportMgr               *pTransportMgr,
                      IN    RvChar                    *msgBuffer,
                      IN    RvUint                    bufferLen)
{
    /*default value incase the application did not register to the callback*/
    RvBool bSendMsg = RV_TRUE;
    if (NULL != pTransportMgr->appEvHandlers.pfnEvMsgToSend)

    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportCallbackMsgToSendEv: Before callback"));

        bSendMsg =  pTransportMgr->appEvHandlers.pfnEvMsgToSend(
                                (RvSipTransportMgrHandle)pTransportMgr,
                                pTransportMgr->hAppTransportMgr,
                                msgBuffer, bufferLen);
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportCallbackMsgToSendEv: After callback, bSendMsg = %d",bSendMsg));

    }
    else
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportCallbackMsgToSendEv: Application did not register to call back, default bSendMsg = %d",bSendMsg));
    }
    return bSendMsg;
}

/***************************************************************************
 * TransportCallbackMsgReceivedEv
 * ------------------------------------------------------------------------
 * General:  Notifies the application that a new message was received.
 *           The application can decide whether the transport layer
 *           should discard the message, in which case the stack will not
 *           process this message.
 * Return Value: RV_TURE to accept the receipt of the message and process it
 *               in the stack. RV_FALSE to discard the message without
 *               further processing.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr - A pointer to the transport manager object.
 *          hMsgReveived - The received message.
 ***************************************************************************/
RvBool RVCALLCONV TransportCallbackMsgReceivedEv(
                      IN    TransportMgr              *pTransportMgr,
                      IN    RvSipMsgHandle            hMsgReceived)

{
    /*default value incase the application did not registered to the callback*/
    RvBool bProcessMsg = RV_TRUE;

    if (NULL != pTransportMgr->appEvHandlers.pfnEvMsgRecvd)
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportCallbackMsgReceivedEv - hMsg=0x%p: Before callback", hMsgReceived));

        bProcessMsg = pTransportMgr->appEvHandlers.pfnEvMsgRecvd(
                        (RvSipTransportMgrHandle)pTransportMgr,
                        pTransportMgr->hAppTransportMgr,
                        hMsgReceived);
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportCallbackMsgReceivedEv - hMsg=0x%p: After callback, bProcessMsg = %d",hMsgReceived,bProcessMsg));

    }
    else
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportCallbackMsgReceivedEv - hMsg=0x%p: Application did not register to call back, default bProcessMsg = %d",hMsgReceived,bProcessMsg));
    }

    return bProcessMsg;
}


/******************************************************************************
 * TransportCallbackMsgReceivedExtEv
 * ----------------------------------------------------------------------------
 * General: Notifies the transaction manager that a message was received.
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr	- Pointer to the transport manager object.
 *          hMsgReceived	- Handle to the received message.
 *          pRcvdMsgDetails - Pointer to received message details struct
 *          pRecvFromAddr   - Pointer to Received From Address in SIPTK format
 *          pSecOwner       - Pointer to Security Object, owning the Local
 *                            Address or Connection, on which the message was
 *                            received. Can be NULL.
 * Output:  pbProcessMsg    - The application can use this parameter for
 *							  asking the SIP Stack to ignore the incoming
 *							  message by setting this out parameter to
 *							  RV_FALSE (otherwise its value should be set
 *							  to RV_TRUE).
 *****************************************************************************/
void RVCALLCONV TransportCallbackMsgReceivedExtEv (
                         IN  TransportMgr                 *pTransportMgr,
                         IN  RvSipMsgHandle				   hMsgReceived,
                         IN  RvSipTransportRcvdMsgDetails *pRcvdMsgDetails,
                         IN  SipTransportAddress          *pRecvFromAddr,
                         IN  void                         *pSecOwner,
						 OUT RvBool						  *pbProcessMsg)
{
	/*Default value incase the application did not registered to the callback*/
    *pbProcessMsg = RV_TRUE;

    /* If there is Security Module, the message should be approved by it before
    it is exposed to the application */
#ifdef RV_SIP_IMS_ON
    if (NULL != pTransportMgr->secEvHandlers.pfnMsgRcvdHandler &&
        NULL != pSecOwner)
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackMsgReceivedExtEv: Msg 0x%p before Security Module callback",
            hMsgReceived));

        pTransportMgr->secEvHandlers.pfnMsgRcvdHandler(
            hMsgReceived, pRecvFromAddr, pSecOwner, pbProcessMsg);
        
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackMsgReceivedExtEv: Msg 0x%p after Security Module callback. bProcessMsg=%s",
            hMsgReceived, (*pbProcessMsg==RV_TRUE)?"TRUE":"FALSE"));
    }
    if (RV_FALSE == *pbProcessMsg)
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackMsgReceivedExtEv: hMsg=0x%p was disapproved by the Security Module.Return.",
            hMsgReceived));
        return;
    }
#else
    RV_UNUSED_ARG(pSecOwner);
    RV_UNUSED_ARG(pRecvFromAddr);
#endif /*#ifdef RV_SIP_IMS_ON*/

    if(pTransportMgr->appEvHandlers.pfnEvMsgRecvdExt != NULL)
    {
		
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
               "TransportCallbackMsgReceivedExtEv: Msg 0x%p Before callback, appHandle=0%x",
               hMsgReceived,pTransportMgr->hAppTransportMgr));

        pTransportMgr->appEvHandlers.pfnEvMsgRecvdExt(
					(RvSipTransportMgrHandle)pTransportMgr,
					pTransportMgr->hAppTransportMgr,
					hMsgReceived,
					pRcvdMsgDetails,
					pbProcessMsg);

        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                 "TransportCallbackMsgReceivedExtEv: Msg 0x%p After callback",hMsgReceived));

    }
	else
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
			"TransportCallbackMsgReceivedExtEv - hMsg=0x%p: Application did not register to call back, calling to the deprecated",hMsgReceived));
		/* If the callback function is not registered call to the equivalent deprecated */
		*pbProcessMsg = TransportCallbackMsgReceivedEv(pTransportMgr,hMsgReceived);	
    }
}

/******************************************************************************
 * TransportCallbackTcpMsgReceivedExtEv
 * ----------------------------------------------------------------------------
 * General: Notifies the transaction manager that a message was received on TCP
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsgReceived	- Handle to the received message. 
 *		    pRcvdMsgDetails - Pointer to received message details struct
 *          pRecvFromAddr   - Pointer to Received From Address in SIPTK format
 *          pSecOwner       - Pointer to Security Object, owning the Local
 *                            Address or Connection, on which the message was
 *                            received. Can be NULL.
 * Output:  pbProcessMsg    - The application can use this parameter for
 *							  asking the SIP Stack to ignore the incoming
 *							  message by setting this out parameter to
 *							  RV_FALSE (otherwise its value should be set
 *							  to RV_TRUE).
 *****************************************************************************/
void RVCALLCONV TransportCallbackTcpMsgReceivedExtEv(
                         IN    RvSipMsgHandle                hMsgReceived,
						 IN    RvSipTransportRcvdMsgDetails *pRcvdMsgDetails,
                         IN    SipTransportAddress          *pRecvFromAddr,
                         IN    void                         *pSecOwner,
						 OUT   RvBool						*pbProcessMsg)
{
    /* IMPORTANT!!!
       Assume that the Connection object is not locked.
       If you access the connection fields, lock connection Event Lock !
       Locking of persistent connection while parsing received buffer
       have bad impact on performance, since it locks Main (Select) Thread. */

	TransportConnection *pConn		   = (TransportConnection*) pRcvdMsgDetails->hConnInfo;
	TransportMgr        *pTransportMgr = pConn->pTransportMgr;
    
	/*default value incase the application did not registered to the callback*/
    *pbProcessMsg = RV_TRUE;

    /* If there is Security Module, the message should be approved by it before
    it is exposed to the application */
#ifdef RV_SIP_IMS_ON
    if (NULL != pTransportMgr->secEvHandlers.pfnMsgRcvdHandler &&
        NULL != pSecOwner)
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackTcpMsgReceivedExtEv: Conn 0x%p,Msg 0x%p Before Security Module callback",
            pConn,hMsgReceived));

        pTransportMgr->secEvHandlers.pfnMsgRcvdHandler(
            hMsgReceived, pRecvFromAddr, pSecOwner, pbProcessMsg);

        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackTcpMsgReceivedExtEv: Conn 0x%p, Msg 0x%p after Security Module callback. bProcessMsg=%s",
            pConn, hMsgReceived, (*pbProcessMsg==RV_TRUE)?"TRUE":"FALSE"));
    }
    if (RV_FALSE == *pbProcessMsg)
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackTcpMsgReceivedExtEv: Conn 0x%p, hMsg=0x%p was disapproved by the Security Module.Return.",
            pConn, hMsgReceived));
        return;
    }
#else
    RV_UNUSED_ARG(pSecOwner);
    RV_UNUSED_ARG(pRecvFromAddr);
#endif /*#ifdef RV_SIP_IMS_ON*/
    
    if(pTransportMgr->appEvHandlers.pfnEvMsgRecvdExt != NULL)
    {

        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                 "TransportCallbackTcpMsgReceivedExtEv: Conn 0x%p,Msg 0x%p Before callback",
                 pConn,hMsgReceived));

        pTransportMgr->appEvHandlers.pfnEvMsgRecvdExt(
						(RvSipTransportMgrHandle)pTransportMgr,
						pTransportMgr->hAppTransportMgr,
						hMsgReceived,
						pRcvdMsgDetails,
						pbProcessMsg);

        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                 "TransportCallbackTcpMsgReceivedExtEv: Conn 0x%p,Msg 0x%p After callback",pConn,hMsgReceived));

    }
	else
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
			"TransportCallbackTcpMsgReceivedExtEv - hMsg=0x%p: Application did not register to call back, calling to the deprecated",hMsgReceived));
		/* If the callback function is not registered call to the equivalent deprecated */
		*pbProcessMsg = TransportCallbackMsgReceivedEv(pTransportMgr,hMsgReceived);	
    }

    return;
}

/***************************************************************************
 * TransportCallbackMsgThreadErr
 * ------------------------------------------------------------------------
 * General:  Notify the application on an Ubnormal thread termination.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr - A pointer to the transport manager object.
 *           error         - RV_ERROR_NUM_OF_THREADS_DECREASED - There is one less thread
 *                          in the system since one thread terminated ubnormally.
 ***************************************************************************/
void RVCALLCONV TransportCallbackMsgThreadErr (
                     IN TransportMgr             *pTransportMgr,
                     IN RvStatus                  error)
{
    if (pTransportMgr->appEvHandlers.pfnThreadError != NULL)
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportCallbackMsgThreadErr: Before callback"));

        /* Notifies application according to the thread problem */
        pTransportMgr->appEvHandlers.pfnThreadError((RvSipTransportMgrHandle)pTransportMgr,
                                                    RV_ERROR_NUM_OF_THREADS_DECREASED);
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportCallbackMsgThreadErr: After callback"));

    }
    else
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportCallbackMsgThreadErr: Application did not registered to call back"));
    }

    RV_UNUSED_ARG(error);
}

/***************************************************************************
 * TransportCallbackBadSyntaxStartLineMsgEv
 * ------------------------------------------------------------------------
 * General:  Notifies the application
 *           that a new message is received, with bad-syntax start-line.
 *           The application can fix the message in this callback.
 *           The application should use the eAction parameter to decide how the
 *           transport layer will handle this message: discard it, continue
 *           with message processing, or send '400 Bad Syntax' response (in case
 *           of request message).
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr    - A pointer to the transport manager object.
 *          hMsgReveived     - The received message, with bad-syntax info.
 * Output:  peAction         - User decision of stack way of handling this message.
 ***************************************************************************/
RvStatus RVCALLCONV TransportCallbackBadSyntaxStartLineMsgEv(
                      IN    TransportMgr             *pTransportMgr,
                      IN    RvSipMsgHandle            hMsgReceived,
                      OUT   RvSipTransportBsAction    *peAction)
{
    RvStatus rv = RV_OK;
    /*default value incase the application did not registered to the callback*/
    *peAction = RVSIP_TRANSPORT_BS_ACTION_DISCARD_MSG;

    if (NULL != pTransportMgr->appEvHandlers.pfnEvBadSyntaxStartLineMsg)
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportCallbackBadSyntaxStartLineMsgEv: Before callback"));
        rv = pTransportMgr->appEvHandlers.pfnEvBadSyntaxStartLineMsg(
                                    (RvSipTransportMgrHandle)pTransportMgr,
                                    pTransportMgr->hAppTransportMgr,
                                    hMsgReceived,
                                    peAction);
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportCallbackBadSyntaxStartLineMsgEv: After callback, Action = %s",
                  GetBSActionName(*peAction)));

    }
    else
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportCallbackBadSyntaxStartLineMsgEv: Application did not registered to call back,default action = %d",*peAction));
    }
    return rv;
}


/***************************************************************************
 * TransportCallbackBadSyntaxMsgEv
 * ------------------------------------------------------------------------
 * General:  Notifies the application
 *           that a new message was received, with bad-syntax.
 *           The application can fix the message in this callback.
 *           The application should use the eAction parameter to decide how the
 *           transport layer will handle this message: discard it, continue
 *           with message processing, or send '400 Bad Syntax' response (in case
 *           of request message).
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr    - A pointer to the transport manager object.
 *          hMsgReveived     - The received message, with bad-syntax info.
 * Output:  peAction         - User decision of stack way of handling this message.
 ***************************************************************************/
RvStatus RVCALLCONV TransportCallbackBadSyntaxMsgEv(
                      IN    TransportMgr             *pTransportMgr,
                      IN    RvSipMsgHandle            hMsgReceived,
                      OUT   RvSipTransportBsAction    *peAction)
{
    RvStatus rv = RV_OK;
    /*default value incase the application did not registered to the callback*/
    *peAction = RVSIP_TRANSPORT_BS_ACTION_DISCARD_MSG;
    if (NULL != pTransportMgr->appEvHandlers.pfnEvBadSyntaxMsg)
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportCallbackBadSyntaxMsgEv: Before callback"));

        rv = pTransportMgr->appEvHandlers.pfnEvBadSyntaxMsg(
                                (RvSipTransportMgrHandle)pTransportMgr,
                                pTransportMgr->hAppTransportMgr,
                                hMsgReceived,
                                peAction);
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportCallbackBadSyntaxMsgEv: After callback, Action = %s",
                  GetBSActionName(*peAction)));

    }
    else
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                  "TransportCallbackBadSyntaxMsgEv: Application did not register to call back, default action = %d",*peAction));
    }
    return rv;

}

/************************************************************************************
 * TransportCallbackMsgRcvdEv
 * ----------------------------------------------------------------------------------
 * General: Notifies the transaction manager that a message was received.
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - Pointer to the transport manager object.
 *          hReceivedMsg  - Handle to the received message.
 *          pConn         - Pointer to the relevant connection.
 *          pLocalAddr   -  The local address that the message was received on.
 *          pRecvFromAddr - The address from which the message was received.
 *          eBSAction    -  Bad-syntax action, that was given by application in
 *                          bad-syntax callbacks. continue processing, reject, discard.
 *          pSigCompMsgInfo - SigComp info related to the received message.
 *                            The information contains indication if the UDVM
 *                            waits for a compartment ID for the given unique
 *                            ID that was related to message by the UDVM.
 ***********************************************************************************/
void RVCALLCONV TransportCallbackMsgRcvdEv (
                         IN    TransportMgr*                  pTransportMgr,
                         IN    RvSipMsgHandle                 hReceivedMsg,
                         IN    RvSipTransportConnectionHandle hConn,
                         IN    RvSipTransportLocalAddrHandle  hLocalAddr,
                         IN    SipTransportAddress           *pRecvFromAddr,
                         IN    RvSipTransportBsAction         eBSAction,
                         INOUT SipTransportSigCompMsgInfo    *pSigCompMsgInfo)
{
    RvStatus rv;
    if(pTransportMgr->eventHandlers.pfnMsgRcvdEvHandler != NULL)
    {
        RvLogDebug(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
               "TransportCallbackMsgRcvdEv: Msg 0x%p Before callback (bsAction=%s)",
               hReceivedMsg, GetBSActionName(eBSAction)));

        rv = pTransportMgr->eventHandlers.pfnMsgRcvdEvHandler(
                                        pTransportMgr->hCallbackContext,
                                        hReceivedMsg,
                                        hConn,
                                        hLocalAddr,
                                        pRecvFromAddr,
                                        eBSAction,
                                        pSigCompMsgInfo);

        RvLogDebug(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                 "TransportCallbackMsgRcvdEv: Msg 0x%p After callback(rv=%d)",
                 hReceivedMsg, rv));

    }
    else
    {
        RvLogError(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                 "TransportCallbackMsgRcvdEv: Msg=0x%p hConn=0x%p: callback was not implemented",
                 hConn, hReceivedMsg));
    }
    return;
}

/******************************************************************************
 * TransportCallbackConnectionMsgRcvdEv
 * ----------------------------------------------------------------------------
 * General: Notifies the transaction manager that a message was received on
 *          connection (e.g. TCP, SCTP).
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - the Transport Manager object.
 *          hReceivedMsg    - the Message object
 *          pConn           - Pointer to the relevant connection.
 *          hLocalAddr      - the Local Address object, used by the Connection.
 *          pRecvFromAddr   - the remote connection address.
 *          eBSAction       - Bad-syntax action, that was given by application
 *                            in bad-syntax callbacks(continue|reject|discard).
 *          pSigCompMsgInfo - SigComp info related to the received message.
 *                            The information contains indication if the UDVM
 *                            waits for a compartment ID for the given unique
 *                            ID that was related to message by the UDVM.
 *****************************************************************************/
void RVCALLCONV TransportCallbackConnectionMsgRcvdEv (
                         IN    TransportMgr*                  pTransportMgr,
                         IN    RvSipMsgHandle                 hReceivedMsg,
                         IN    TransportConnection*           pConn,
                         IN    RvSipTransportLocalAddrHandle  hLocalAddr,
                         IN    SipTransportAddress*           pRecvFromAddr,
                         IN    RvSipTransportBsAction         eBSAction,
                         INOUT SipTransportSigCompMsgInfo*    pSigCompMsgInfo)
{
    /* IMPORTANT!!!
       Assume that the Connection object is not locked.
       If you access the connection fields, lock connection Event Lock !
       Locking of persistent connection while parsing received buffer
       have bad impact on performance, since it locks Main (Select) Thread. */

    RvStatus rv;

    if(pTransportMgr->eventHandlers.pfnMsgRcvdEvHandler != NULL)
    {
        RvLogDebug(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                 "TransportCallbackConnectionMsgRcvdEv: Conn 0x%p,Msg 0x%p Before callback (baAction=%s)",
                 pConn, hReceivedMsg, GetBSActionName(eBSAction)));

        rv = pTransportMgr->eventHandlers.pfnMsgRcvdEvHandler(
                                        pTransportMgr->hCallbackContext,
                                        hReceivedMsg,
                                        (RvSipTransportConnectionHandle) pConn,
                                        hLocalAddr,
                                        pRecvFromAddr,
                                        eBSAction,
                                        pSigCompMsgInfo);

        RvLogDebug(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                 "TransportCallbackConnectionMsgRcvdEv: Conn 0x%p,Msg 0x%p After callback(rv=%d)",
                 pConn, hReceivedMsg, rv));
    }
    return;
}

#if (RV_TLS_TYPE != RV_TLS_NONE)
/************************************************************************************
 * TransportCallbackTLSAppPostConnectionAssertionEv
 * ----------------------------------------------------------------------------------
 * General: notifies the application on TLS state changes
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 *          strSipAddress - the address to which the assertion was made.
 *          hMsg          - the entire msg that the assertion was made against.
 *                          This parameter will be NULL if assertion was made
 *                          against an outgoing message or predefined string.
 * Output:  pbAssertion - is the connection asserted.
 ***********************************************************************************/
void RVCALLCONV TransportCallbackTLSAppPostConnectionAssertionEv(IN  TransportConnection *pConn,
                                                       IN RvChar              *strSipAddress,
                                                       IN RvSipMsgHandle        hMsg,
                                                       OUT RvBool             *pbAssertion)
{
    if (NULL != pConn->pTransportMgr->appEvHandlers.pfnEvTlsPostConnectionAssertion)
    {
        RvInt32        nestedLock  = 0;
        SipTripleLock *pTripleLock;
        RvThreadId     currThreadId; RvInt32 threadIdLockCount;
        RvSipTransportMgrEvHandlers* appEvHandlers  = &(pConn->pTransportMgr->appEvHandlers);

        pTripleLock = pConn->pTripleLock;
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                 "TransportCallbackTLSAppPostConnectionAssertionEv - Connection 0x%p: asserting post connection with application. host name=%s, msg=0x%p",
                 pConn,(NULL==strSipAddress)?"NULL":strSipAddress,hMsg));

        SipCommonUnlockBeforeCallback(pConn->pTransportMgr->pLogMgr,pConn->pTransportMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        
        (*appEvHandlers).pfnEvTlsPostConnectionAssertion((RvSipTransportConnectionHandle)pConn,
                                                        pConn->hAppHandle,
                                                        strSipAddress,
                                                        hMsg,
                                                        pbAssertion);
        
        SipCommonLockAfterCallback(pConn->pTransportMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                 "TransportCallbackTLSAppPostConnectionAssertionEv - Connection 0x%p: returning from post connection assertion CB application decision=%d",
                 pConn,*pbAssertion));

    }
    else
    {
       RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                 "TransportCallbackTLSAppPostConnectionAssertionEv - Connection 0x%p: callback not implemented",
                 pConn));
    }
}

/************************************************************************************
 * TransportCallbackTLSStateChange
 * ----------------------------------------------------------------------------------
 * General: notifies the application on tls state changes
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 *          eState - Tls State to assume
 *          eReason - reason for state change
 *          bSendToQueue - send event though queue
 *          bChangeInternalState - whether to change internal state
 * Output: RV_OK on success, error code - otherwise.
 ***********************************************************************************/
RvStatus RVCALLCONV TransportCallbackTLSStateChange(
                    IN TransportConnection*              pConn,
                    IN RvSipTransportConnectionTlsState  eState,
                    IN RvSipTransportConnectionStateChangedReason   eReason,
                    IN RvBool                            bSendToQueue,
                    IN RvBool                            bChangeInternalState)
{
    RvStatus rv;
    RvSipTransportMgrEvHandlers* appEvHandlers  = &(pConn->pTransportMgr->appEvHandlers);

    if (RV_TRUE == bChangeInternalState)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportCallbackTLSStateChange - Connection 0x%p changing internal Tls state to %s",pConn,
            TransportConnectionGetTlsStateName(eState)));
        pConn->eTlsState = eState;
    }

    if(bSendToQueue == RV_TRUE)
    {
        pConn->eTlsReason = eReason;

        rv = SipTransportSendStateObjectEvent(
                    (RvSipTransportMgrHandle)pConn->pTransportMgr,
                    (void*)pConn,
                    pConn->connUniqueIdentifier,
                    (RvInt32)eState,
                    UNDEFINED,
                    TransportTlsStateChangeQueueEventHandler,
                    RV_TRUE,
                    TransportConnectionGetTlsStateName(eState),
                    CONNECTION_OBJECT_NAME_STR);

		if (rv != RV_OK)
		{
			RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
				"TransportCallbackTLSStateChange - Connection 0x%p was unable to send state change event for changing internal Tls state to %s, closing connection.",pConn,
				TransportConnectionGetTlsStateName(eState)));
			TransportTlsClose(pConn);
		}
        return rv;
    }

    if (NULL != (*appEvHandlers).pfnEvTlsStateChanged)
    {
        RvInt32                    nestedLock  = 0;
        SipTripleLock             *pTripleLock;
        RvThreadId                 currThreadId; RvInt32 threadIdLockCount;

        pTripleLock = pConn->pTripleLock;
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                     "TransportCallbackTLSStateChange - Connection 0x%p notifing the application of Tls state %s",pConn,
                      TransportConnectionGetTlsStateName(eState)));

        SipCommonUnlockBeforeCallback(pConn->pTransportMgr->pLogMgr,pConn->pTransportMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*appEvHandlers).pfnEvTlsStateChanged((RvSipTransportConnectionHandle)pConn,
                                             pConn->hAppHandle,
                                             eState,
                                             eReason);

        SipCommonLockAfterCallback(pConn->pTransportMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportCallbackTLSStateChange - Connection 0x%p after callback for Tls state %s (rv=%d)",
            pConn, TransportConnectionGetTlsStateName(eState), rv));

        pConn->eTlsReason =   RVSIP_TRANSPORT_CONN_REASON_UNDEFINED;
        if (RVSIP_TRANSPORT_CONN_TLS_STATE_HANDSHAKE_FAILED ==eState)
        {

            TransportConnectionNotifyStatus(
                           pConn,
                           NULL,
                           NULL,
                           RVSIP_TRANSPORT_CONN_STATUS_ERROR,
                           RV_FALSE);
        }
    }
    return RV_OK;
}

/************************************************************************************
 * TransportCallbackConnectionTlsStatus
 * ----------------------------------------------------------------------------------
 * General: notifies the application on security events
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:    hConn           -   The connection handle
 *           eStatus         -   The connection security status
 * Output    peAction        -   application instructions.
  ***********************************************************************************/
void RVCALLCONV TransportCallbackConnectionTlsStatus(IN TransportConnection*      pConn,
                                           IN  RvSipTransportConnectionTlsStatus  eStatus,
                                           OUT RvBool*                            pbContinue)
{
#if defined(RV_SSL_SESSION_STATUS)
    RvSipTransportMgrEvHandlers* appEvHandlers  = &(pConn->pTransportMgr->appEvHandlers);

    if (NULL != (*appEvHandlers).pfnEvConnTlsStatus)
    {
        RvInt32                    nestedLock  = 0;
        SipTripleLock             *pTripleLock;
        RvThreadId                 currThreadId; RvInt32 threadIdLockCount;

        pTripleLock = pConn->pTripleLock;
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                     "TransportCallbackConnectionTlsStatus - Connection 0x%p notifing the application security event %s",
                     pConn,TransportConnectionGetSecurityNotificationName(eStatus)));

        SipCommonUnlockBeforeCallback(pConn->pTransportMgr->pLogMgr,pConn->pTransportMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*appEvHandlers).pfnEvConnTlsStatus((RvSipTransportConnectionHandle)pConn,
                                             pConn->hAppHandle,
                                             eStatus,
                                             pbContinue);

        SipCommonLockAfterCallback(pConn->pTransportMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportCallbackConnectionTlsStatus - Connection 0x%p after security notification %s, action=%d",pConn,
            TransportConnectionGetSecurityNotificationName(eStatus),
            *pbContinue));
    }
#else
    RV_UNUSED_ARG(pConn);
    RV_UNUSED_ARG(eStatus);
    RV_UNUSED_ARG(pbContinue);
#endif /*#if defined(RV_SSL_SESSION_STATUS)*/
    return;
}

/************************************************************************************
 * TransportCallbackTLSSeqStarted
 * ----------------------------------------------------------------------------------
 * General: notifies the application on tls seq start.
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 * Output:
 ***********************************************************************************/
void RVCALLCONV TransportCallbackTLSSeqStarted(IN TransportConnection* pConn)
{
    RvSipTransportMgrEvHandlers* appEvHandlers = &(pConn->pTransportMgr->appEvHandlers);


    if (NULL != (*appEvHandlers).pfnEvTlsSeqStarted)
    {
        RvInt32          nestedLock = 0;
        SipTripleLock   *pTripleLock;
        RvThreadId       currThreadId; RvInt32 threadIdLockCount;
        RvSipTransportConnectionAppHandle hAppHandle = pConn->hAppHandle;

        pTripleLock = pConn->pTripleLock;
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportCallbackTLSSeqStarted - Connection 0x%p Tls seq started",pConn));

        SipCommonUnlockBeforeCallback(pConn->pTransportMgr->pLogMgr,pConn->pTransportMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        
        (*appEvHandlers).pfnEvTlsSeqStarted((RvSipTransportConnectionHandle)pConn,
                                             &hAppHandle);
        
        SipCommonLockAfterCallback(pConn->pTransportMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        /* update the stack object only when it is locked!!! */
        pConn->hAppHandle = hAppHandle;
        return;
    }
    else
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportCallbackTLSSeqStarted- Connection 0x%p Tls seq started but no callback is provided",pConn));
        return;
    }
}

#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

/***************************************************************************
 * TransportCallbackConnCreated
 * ------------------------------------------------------------------------
 * General: Notifies an application about COnnection object creation for
 *          incoming TCP connection.
 *          The callback is called immediately after accept.
 *          Application can order Stack to close the connection by means of
 *          parameter 'bDrop'.
 *          In this case the connection will be closed immediately after
 *          return from callback. It resources will be freed.
 *          No data will be received or sent on it.
 *          If application didn't register to this callback,
 *          the connection will not be closed, and will be used for data
 *          sending and reception.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - A pointer to the transport manager object.
 *          pConn            - pointer to the created connection.
 * Output:  pbDrop           - If set to RV_TRUE by application, the connection
 *                             will be dropped immediately after return from
 *                             callback.
 *                             Otherwise the connection will be not dropped and
 *                             will be used for data receiption and sending.
 ***************************************************************************/
void RVCALLCONV TransportCallbackConnCreated(
                                    IN  TransportMgr            *pTransportMgr,
                                    IN  TransportConnection     *pConn,
                                    OUT RvBool                  *pbDrop)
{
    *pbDrop = RV_FALSE;

    if(pTransportMgr->appEvHandlers.pfnEvConnCreated != NULL)
    {
        RvInt32        nestedLock = 0;
        SipTripleLock *pTripleLock;
        RvThreadId     currThreadId; RvInt32 threadIdLockCount;
        RvSipTransportMgrEvHandlers* appEvHandlers  = &(pTransportMgr->appEvHandlers);
        RvSipTransportConnectionAppHandle hAppHandle = NULL;

        pTripleLock = pConn->pTripleLock;
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackConnCreated: Connection 0x%p Before callback",
            pConn));

        SipCommonUnlockBeforeCallback(pConn->pTransportMgr->pLogMgr,pConn->pTransportMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);
        
        (*appEvHandlers).pfnEvConnCreated(
                        (RvSipTransportMgrHandle)pTransportMgr,
                         pTransportMgr->hAppTransportMgr,
                        (RvSipTransportConnectionHandle)pConn,
                        &hAppHandle,
                        pbDrop);

        SipCommonLockAfterCallback(pConn->pTransportMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);
        pConn->hAppHandle = hAppHandle;
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackConnCreated: Connection 0x%p After callback - bDrop=%s",
            pConn,(RV_TRUE==*pbDrop)?"TRUE":"FALSE"));
    }
    else
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackConnCreated: Application did not register to call back, default bDrop=FALSE"));
    }
    return;
}

/***************************************************************************
 * TransportCallbackBufferReceived
 * ------------------------------------------------------------------------
 * General: Exposes to an application row data buffer, containing exactly
 *          one SIP message, received on TCP/UDP layer.
 *          Application can dump the data by means of this callback.
 *          Also application can order Stack to discard the buffer, and not
 *          to parse it, by means of pbDiscardBuffer parameter.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - A pointer to the transport manager object.
 *          pLocalAddr      - pointer to the local address object,corresponding
 *                            to address, on which the message was received.
 *          pSenderAddr     - A pointer to the SIP Transport Address structure,
 *                            which contains details of the address,
 *                            from which the message was sent.
 *          pConn           - A pointer to the connection object, on which
 *                            the buffer was received. NULL in case of UDP.
 *          buffer          - pointer to the buffer,which contains the message
 *          buffLen         - length of the message in the buffer (in bytes)
 * Output:  bDiscardBuffer  - if set to RV_TRUE, the buffer will be not
 *                            processed, the resources will be freed.
 *****************************************************************************/
void RVCALLCONV TransportCallbackBufferReceived(
                                IN  TransportMgr            *pTransportMgr,
                                IN  TransportMgrLocalAddr   *pLocalAddr,
                                IN  SipTransportAddress     *pSenderAddr,
                                IN  TransportConnection     *pConn,
                                IN  RvChar                  *buffer,
                                IN  RvUint32                buffLen,
                                OUT RvBool                  *pbDiscardBuffer)
{
    /* IMPORTANT!!!
       Assume that the Connection object is not locked.
       If you access the connection fields, lock connection Event Lock !
       Locking of persistent connection while parsing received buffer
       have bad impact on performance, since it locks Main (Select) Thread. */

    RvSipTransportMgrEvHandlers* appEvHandlers  = &(pTransportMgr->appEvHandlers);
    
    *pbDiscardBuffer = RV_FALSE;
    
    if((*appEvHandlers).pfnEvBufferReceived != NULL)
    {
        RvStatus rv;
        RvSipTransportAddr  localAddrDetails;
        RvSipTransportAddr  remoteAddrDetails;
        RvInt32             nestedLockLocalAddr = 0;
        RvSipTransportConnectionAppHandle hAppConn = NULL;

        memset(&localAddrDetails,0,sizeof(RvSipTransportAddr));
        memset(&remoteAddrDetails,0,sizeof(RvSipTransportAddr));

        rv = RvSipTransportMgrLocalAddressGetDetails(
                            (RvSipTransportLocalAddrHandle)pLocalAddr,
                            sizeof(RvSipTransportAddr),&localAddrDetails);
        if (RV_OK != rv)
        {
            RvLogError(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                "TransportCallbackBufferReceived: RvSipTransportMgrLocalAddressGetDetails(hLocalAddr=0x%p) failed",
                pLocalAddr));
            return;
        }

        /* Get Connection local port */
        if (NULL != pConn)
        {
            rv = TransportConnectionLockEvent(pConn);
            if (rv != RV_OK)
            {
                RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "TransportCallbackBufferReceived - Conn %p was terminated. Return", pConn));
                return;
            }
            if (UNDEFINED != pConn->localPort)
            {
                localAddrDetails.port = (RvUint16)pConn->localPort;
            }
            hAppConn = pConn->hAppHandle;
            TransportConnectionUnLockEvent(pConn);
        }

        TransportMgrSipAddrGetDetails(pSenderAddr,&remoteAddrDetails);

        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackBufferReceived: Local Addrs (%s:%d,%s), Remote Addrs (%s:%d,%s), hConn=0x%p, buffLen=%d Before callback",
            localAddrDetails.strIP,localAddrDetails.port,SipCommonConvertEnumToStrTransportType(localAddrDetails.eTransportType),
            remoteAddrDetails.strIP,remoteAddrDetails.port,SipCommonConvertEnumToStrTransportType(remoteAddrDetails.eTransportType),
            pConn, buffLen));

        /* before releasing the lock, make sure that no other thread had locked it... */
        rv = TransportMgrLocalAddrLock(pLocalAddr);
        if (RV_OK != rv)
        {
            RvLogError(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                "TransportCallbackBufferReceived: (hLocalAddr=0x%p) failed to lock locaAddressMgr",
                pLocalAddr));
            return;
        }
        if (RV_OK != TransportMgrLocalAddrLockRelease(pLocalAddr,&nestedLockLocalAddr))
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportCallbackBufferReceived: Failed to release nested locks on Local Address 0x%p",pLocalAddr));
        }

        (*appEvHandlers).pfnEvBufferReceived(
            (RvSipTransportMgrHandle)pTransportMgr, pTransportMgr->hAppTransportMgr,
            (RvSipTransportLocalAddrHandle)pLocalAddr, &remoteAddrDetails,
            (RvSipTransportConnectionHandle)pConn, hAppConn,
            buffer, buffLen, pbDiscardBuffer);

        TransportMgrLocalAddrLockRestore(pLocalAddr,nestedLockLocalAddr);
        TransportMgrLocalAddrUnlock(pLocalAddr);

        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackBufferReceived: bDiscardBuffer=%s, Local Addrs (%s:%d,%s), Remote Addrs (%s:%d,%s), hConn=0x%p, buffLen=%d After callback",
            (RV_TRUE==*pbDiscardBuffer)?"TRUE":"FALSE",
            localAddrDetails.strIP,localAddrDetails.port,SipCommonConvertEnumToStrTransportType(localAddrDetails.eTransportType),
            remoteAddrDetails.strIP,remoteAddrDetails.port,SipCommonConvertEnumToStrTransportType(remoteAddrDetails.eTransportType),
            pConn, buffLen));
    }
    else
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackBufferReceived: Application did not register to call back. By default bDiscardBuffer=FALSE"));
    }
    return;
}

/***************************************************************************
 * TransportCallbackBufferToSend
 * ------------------------------------------------------------------------
 * General: Exposes to an application row data buffer, containing exactly
 *          one SIP message, that is going to be sent on TCP/UDP layer.
 *          Application can dump the data by means of this callback.
 *          Also the application can decide whether the transport layer
 *          should not transmit the message to its destination.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the Transport Manager object.
 *          pLocalAddr       - pointer to the local address object,corresponding
 *                             to address, from which the message is sent.
 *          pDestinationAddr - pointer to the Transport Address structure,
 *                             which contains details of the address,
 *                             to which the message is going to be sent.
 *          pConn            - A pointer to the connection object, on which
 *                             the buffer is going to be sent. NULL for UDP.
 *          buffer           - pointer to the buffer,which contains the message
 *          buffLen          - length of the message in the buffer (in bytes)
 * Output:  bDiscardBuffer   - if set to RV_TRUE, the buffer will be not sent,
 *                             the resources will be freed.
 ***************************************************************************/
void RVCALLCONV TransportCallbackBufferToSend(
                                IN  TransportMgr            *pTransportMgr,
                                IN  TransportMgrLocalAddr   *pLocalAddr,
                                IN  SipTransportAddress     *pDestinationAddr,
                                IN  TransportConnection     *pConn,
                                IN  RvChar                  *buffer,
                                IN  RvUint32                buffLen,
                                OUT RvBool                  *pbDiscardBuffer)
{
    *pbDiscardBuffer = RV_FALSE;

    if(pTransportMgr->appEvHandlers.pfnEvBufferToSend != NULL)
    {
        RvSipTransportAddr  localAddrDetails;
        RvSipTransportAddr  remoteAddrDetails;
        RvInt32    nestedLockLocalAddr = 0;
        RvStatus   rv;
        RvSipTransportConnectionAppHandle hAppConnHandle;

        memset(&localAddrDetails,0,sizeof(RvSipTransportAddr));
        memset(&remoteAddrDetails,0,sizeof(RvSipTransportAddr));

        rv = RvSipTransportMgrLocalAddressGetDetails(
                                (RvSipTransportLocalAddrHandle)pLocalAddr,
                                sizeof(RvSipTransportAddr),&localAddrDetails);
        if (RV_OK != rv)
        {
            RvLogError(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                "TransportCallbackBufferToSend: RvSipTransportMgrLocalAddressGetDetails(hLocalAddr=0x%p) failed",
                pLocalAddr));
            return;
        }
        TransportMgrSipAddrGetDetails(pDestinationAddr,&remoteAddrDetails);

        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackBufferToSend: Local Addrs (%s:%d,%s), Remote Addrs (%s:%d,%s), buffLen=%d Before callback",
            localAddrDetails.strIP,localAddrDetails.port,SipCommonConvertEnumToStrTransportType(localAddrDetails.eTransportType),
            remoteAddrDetails.strIP,remoteAddrDetails.port,SipCommonConvertEnumToStrTransportType(remoteAddrDetails.eTransportType),
            buffLen));

        /* before releasing the lock, make sure that no other thread had locked it... */
        rv = TransportMgrLocalAddrLock(pLocalAddr);
        if (RV_OK != rv)
        {
            RvLogError(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                "TransportCallbackBufferToSend: (hLocalAddr=0x%p) failed to lock locaAddressMgr",
                pLocalAddr));
            return;
        }
        if (RV_OK != TransportMgrLocalAddrLockRelease(pLocalAddr,&nestedLockLocalAddr))
        {
            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "TransportCallbackBufferToSend: Failed to release nested locks on Local Address 0x%p",pLocalAddr));
        }

        hAppConnHandle = (NULL==pConn) ? NULL:pConn->hAppHandle;
        pTransportMgr->appEvHandlers.pfnEvBufferToSend(
            (RvSipTransportMgrHandle)pTransportMgr,pTransportMgr->hAppTransportMgr,
            (RvSipTransportLocalAddrHandle)pLocalAddr, &remoteAddrDetails,
            (RvSipTransportConnectionHandle)pConn, hAppConnHandle,
            buffer, buffLen,pbDiscardBuffer);

        TransportMgrLocalAddrLockRestore(pLocalAddr,nestedLockLocalAddr);
        TransportMgrLocalAddrUnlock(pLocalAddr);
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackBufferToSend: bDiscardBuffer=%s, Local Addrs (%s:%d,%s), Remote Addrs ((%s:%d,%s)), buffLen=%d After callback",
            (RV_TRUE==*pbDiscardBuffer) ? "TRUE" : "FALSE",
            localAddrDetails.strIP,localAddrDetails.port,SipCommonConvertEnumToStrTransportType(localAddrDetails.eTransportType),
            remoteAddrDetails.strIP,remoteAddrDetails.port,SipCommonConvertEnumToStrTransportType(remoteAddrDetails.eTransportType),
            buffLen));

    }
    else
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackBufferToSend: Application did not register to call back, default *bDiscardBuffer=FALSE"));
    }
    return;
}

/******************************************************************************
 * TransportCallbackConnParserResult
 * ----------------------------------------------------------------------------
 * General: Notifies an application about result of parsing of incoming
 *          over TCP/SCTP connection message.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - A pointer to the transport manager object.
 *          hMsg          - Handle to the message, which was parsed.
 *          pConn         - pointer to the connection, on which the parsed
 *                          message arrived.
 *          hAppConn      - Handle stored by the Application in the connection.
 *          bLegalSyntax  - If RV_FALSE, parser encountered into bad syntax.
 *                          Otherwise - RV_FALSE.
 * Output:  none.
 *****************************************************************************/
void RVCALLCONV TransportCallbackConnParserResult(
                            IN TransportMgr*                     pTransportMgr,
                            IN RvSipMsgHandle                    hMsg,
                            IN TransportConnection*              pConn,
                            IN RvSipTransportConnectionAppHandle hAppConn,
                            IN RvBool                            bLegalSyntax)
{
    /* IMPORTANT!!!
       Assume that the Connection object is not locked.
       If you access the connection fields, lock connection Event Lock !
       Locking of persistent connection while parsing received buffer
       have bad impact on performance, since it locks Main (Select) Thread. */

    RvSipTransportMgrEvHandlers* appEvHandlers  = &(pTransportMgr->appEvHandlers);
    if((*appEvHandlers).pfnEvConnParserResult != NULL)
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackConnParserResult: hMsg=0x%p, hConn=0x%p, bLegalSyntax=%s Before callback",
            hMsg,pConn,(RV_TRUE==bLegalSyntax)?"TRUE":"FALSE"));

        (*appEvHandlers).pfnEvConnParserResult(
                                        (RvSipTransportMgrHandle)pTransportMgr,
                                        pTransportMgr->hAppTransportMgr,
                                        hMsg,
                                        (RvSipTransportConnectionHandle)pConn,
                                        hAppConn,
                                        bLegalSyntax);

        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackConnParserResult: hMsg=0x%p, hConn=0x%p, bLegalSyntax=%s After callback",
            hMsg,pConn,(RV_TRUE==bLegalSyntax)?"TRUE":"FALSE"));
    }
    else
    {
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportCallbackConnParserResult: Application did not register to call back"));
    }
    return;
}

/******************************************************************************
 * TransportCallbackIncomingConnStateChange
 * ----------------------------------------------------------------------------
 * General: notifies the application on incoming connection state changes
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the relevant connection.
 *          eState - state to notify
 *          eReason - reason for state change
 * Output:
 *****************************************************************************/
void RVCALLCONV TransportCallbackIncomingConnStateChange(
                        IN TransportConnection*                       pConn,
                        IN RvSipTransportConnectionState              eState,
                        IN RvSipTransportConnectionStateChangedReason eReason)
{
    RvSipTransportMgrEvHandlers* appEvHandlers  = &(pConn->pTransportMgr->appEvHandlers);

    if (NULL != (*appEvHandlers).pfnEvConnStateChanged)
    {
        RvInt32         nestedLock = 0;
        SipTripleLock  *pTripleLock;
        RvThreadId      currThreadId; RvInt32 threadIdLockCount;
        RvSipTransportConnectionAppHandle hAppHandle = pConn->hAppHandle;
        RvStatus rv;
        

        pTripleLock = pConn->pTripleLock;
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportCallbackIncomingConnStateChange - Connection 0x%p: state %s. Before callback",
            pConn, SipCommonConvertEnumToStrConnectionState(eState)));

        SipCommonUnlockBeforeCallback(pConn->pTransportMgr->pLogMgr,pConn->pTransportMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        rv = (*appEvHandlers).pfnEvConnStateChanged((RvSipTransportConnectionHandle)pConn,
                                              (void*)hAppHandle, eState, eReason);

        SipCommonLockAfterCallback(pConn->pTransportMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportCallbackIncomingConnStateChange - Connection 0x%p: state %s. After callback(rv=%d)",
            pConn, SipCommonConvertEnumToStrConnectionState(eState), rv));
    }
    else
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportCallbackIncomingConnStateChange: Application did not register to call back"));
    }
    return;
}

/***************************************************************************
* TransportCallbackConnDataReceived
* ------------------------------------------------------------------------
* General: Notifies an application that data was read form the socket
*          of the incoming connection.
* Return Value: none.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTransportMgr - A pointer to the transport manager object.
*          pConn         - pointer to the created connection.
*          buff          - A buffer, containing read data.
*          buffSize      - Size of the read data in bytes.
* Output:  pbDiscard     - If is set to RV_TRUE by the Application,
*                          the received data will be discarded.
***************************************************************************/
void RVCALLCONV TransportCallbackConnDataReceived(
                                    IN  TransportMgr            *pTransportMgr,
                                    IN  TransportConnection     *pConn,
                                    IN  RvChar                  *buff,
                                    IN  RvInt32                 buffSize,
                                    OUT RvBool                  *pbDiscard)
{
    RvSipTransportMgrEvHandlers* appEvHandlers  = &(pTransportMgr->appEvHandlers);
    if((*appEvHandlers).pfnEvConnDataReceived != NULL)
    {
        
		RvInt32         nestedLock = 0;
        SipTripleLock  *pTripleLock;
        RvThreadId      currThreadId; RvInt32 threadIdLockCount;
        RvSipTransportConnectionAppHandle hAppHandle = pConn->hAppHandle;
        
        *pbDiscard = RV_FALSE;
        pTripleLock = pConn->pTripleLock;
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportCallbackConnDataReceived: Connection 0x%p Before callback",
            pConn));
        
        SipCommonUnlockBeforeCallback(pConn->pTransportMgr->pLogMgr,pConn->pTransportMgr->pLogSrc,
                pTripleLock,&nestedLock,&currThreadId,&threadIdLockCount);

        (*appEvHandlers).pfnEvConnDataReceived(
                                (RvSipTransportMgrHandle)pTransportMgr,
                                pTransportMgr->hAppTransportMgr,
                                (RvSipTransportConnectionHandle)pConn,
                                hAppHandle,
                                buff,
                                buffSize, 
                                pbDiscard);
        
        SipCommonLockAfterCallback(pConn->pTransportMgr->pLogMgr,pTripleLock,nestedLock,currThreadId,threadIdLockCount);

        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportCallbackConnDataReceived: Connection 0x%p After callback, pbDiscard=%d",
            pConn,*pbDiscard));
    }
    else
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportCallbackConnDataReceived: Application did not register to call back"));
    }
    return;
}

/***************************************************************************
* TransportCallbackConnServerReuse
* ------------------------------------------------------------------------
* General: This callback function informs application that a server TLS connection
*          should be reuse, and it has to be authorized first.
*          In this callback application should authenticate the connection, as 
*          described in draft-ietf-sip-connect-reuse-03. 
*          If the connection was authorized,Application should call to 
*          RvSipTransportConnectionUseTlsConnByAlias() function.
* Return Value: none.
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTransportMgr - A pointer to the transport manager object.
*          pConn         - pointer to the connection.
*          hAppConn      - handle stored by the application in the connection.
* Output:  none.
***************************************************************************/
void RVCALLCONV TransportCallbackConnServerReuse(
                            IN TransportMgr*                     pTransportMgr,
                            IN TransportConnection*              pConn,
                            IN RvSipTransportConnectionAppHandle hAppConn)
{
    /* IMPORTANT!!!
       Assume that the Connection object is not locked.
       If you access the connection fields, lock connection Event Lock !
       Locking of persistent connection while parsing received buffer
       have bad impact on performance, since it locks Main (Select) Thread. */

    RvSipTransportMgrEvHandlers* appEvHandlers  = &(pTransportMgr->appEvHandlers);
    if((*appEvHandlers).pfnEvConnServerReuse != NULL)
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportCallbackConnServerReuse: Connection 0x%p Before callback",
            pConn));
        
        (*appEvHandlers).pfnEvConnServerReuse((RvSipTransportMgrHandle)pTransportMgr,
                                              pTransportMgr->hAppTransportMgr,
                                              (RvSipTransportConnectionHandle)pConn,
                                              hAppConn);

        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportCallbackConnServerReuse: Connection 0x%p After callback",
            pConn));
    }
    else
    {
        RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportCallbackConnServerReuse: Application did not register to call back"));
    }
    return;
}

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * TransportCallbackMsgSendFailureEv
 * ----------------------------------------------------------------------------
 * General: Is called when a protected message sending was failed due to error,
 *          returned by the system call, which sends buffers.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - Pointer to the Transport Manager
 *          hMsgPage   - Handle to the page,where the encoded message is stored
 *          pLocalAddr - Pointer to the local address, if the message was being
 *                       sent over UDP
 *          pConn      - Pointer to the connection, if the message was being
 *                       sent over TCP / TLS / SCTP
 *          pSecOwner  - Pointer to the Security Object, owning the local
 *                       address or connection, on which the message was
 *                       being sent.
 *****************************************************************************/
void RVCALLCONV TransportCallbackMsgSendFailureEv(
                                    IN TransportMgr*          pTransportMgr,
                                    IN HPAGE                  hMsgPage,
                                    IN TransportMgrLocalAddr* pLocalAddr,
                                    IN TransportConnection*   pConn,
                                    IN void*                  pSecOwner)
{
    RvInt32         nestedLock        = 0;
    SipTripleLock*  pTripleLock       = NULL;
    RvThreadId      currThreadId      = 0;
    RvInt32         threadIdLockCount = 0;
    SipTransportSecEvHandlers *psecEvHandlers = &pTransportMgr->secEvHandlers;

    if(NULL == (*psecEvHandlers).pfnMsgSendFailureHandler)
        return;

    RvLogDebug(pTransportMgr->pLogSrc, (pTransportMgr->pLogSrc,
        "TransportCallbackMsgSendFailureEv: hMsgPage %p, pLocalAddr %p, Connection %p, pSecOwner %p Before callback",
        hMsgPage, pLocalAddr, pConn, pSecOwner));

    if (NULL != pConn)
    {
        pTripleLock = pConn->pTripleLock;
        SipCommonUnlockBeforeCallback(pTransportMgr->pLogMgr, pTransportMgr->pLogSrc,
            pTripleLock, &nestedLock, &currThreadId, &threadIdLockCount);
    }

    (*psecEvHandlers).pfnMsgSendFailureHandler(
                        hMsgPage, (RvSipTransportLocalAddrHandle)pLocalAddr,
                        (RvSipTransportConnectionHandle)pConn, pSecOwner);
    if (NULL != pTripleLock)
    {
        SipCommonLockAfterCallback(pTransportMgr->pLogMgr,
            pTripleLock, nestedLock, currThreadId, threadIdLockCount);
    }

    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
        "TransportCallbackMsgSendFailureEv:  hMsgPage %p, pLocalAddr %p, Connection %p, pSecOwner %p After callback",
        hMsgPage, pLocalAddr, pConn, pSecOwner));
    return;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

/*-------------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                              */
/*-------------------------------------------------------------------------*/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static const RvChar* GetBSActionName(RvSipTransportBsAction bsAction)
{
    switch (bsAction)
    {
    case RVSIP_TRANSPORT_BS_ACTION_DISCARD_MSG:
        return "Discard";
    case RVSIP_TRANSPORT_BS_ACTION_REJECT_MSG:
        return "Reject";
    case RVSIP_TRANSPORT_BS_ACTION_CONTINUE_PROCESS:
        return "Continue";
    case RVSIP_TRANSPORT_BS_ACTION_UNDEFINED:
    default:
        return "Undefined";
    }
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

#ifdef __cplusplus
}
#endif

