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
 *                              <TransactionCallBacks.h>
 * Transaciton layer callback function wrappers
 * The file also includes default callbacks for transactions with no owner.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Nov 2000
 *********************************************************************************/


#ifndef TRANSACTION_CALL_BACKS_H
#define TRANSACTION_CALL_BACKS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RV_SIP_LIGHT
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RvSipMsg.h"
#include "_SipTransactionTypes.h"
#include "TransactionObject.h"
/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          Application Callbacks                        */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * TransactionCallbackAppTranscCreatedEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that a new server transaction was
 *          created. The newly created transaction always assumes the Idle
 *          state. The application should decide whether to handle this transaction.
 *          If so, the application can exchange handles with the SIP Stack
 *          using this callback.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc     - The new SIP Stack transaction handle.
 * Output:  pbAppHandleTransc - Indicated wether the application wishes to handle
 *                           the transaction - RV_TRUE.
 *                           if set tp RV_FALSE the stack will handle the
 *                           transaction by itself. The normal behavior will
 *                           be returning of 501 not implemented.
 ***************************************************************************/
void RVCALLCONV TransactionCallbackAppTranscCreatedEv(
                      IN    Transaction              *pTransc,
                       OUT   RvSipTranscOwnerHandle   *phAppTransc,
                      OUT   RvBool                  *pbAppHandleTransc);



/***************************************************************************
 * TransactionCallbackStateChangedEv
 * ------------------------------------------------------------------------
 * General: Notifies of a transaction state change and the
 *          associated state change reason.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc      - The transaction pointer.
 *            eState       - The new transaction state.
 *            eReason      - The reason for the state change.
 ***************************************************************************/
void RVCALLCONV  TransactionCallbackStateChangedEv(
                            IN Transaction                      *pTransc,
                              IN RvSipTransactionState             eState,
                            IN RvSipTransactionStateChangeReason eReason);





/***************************************************************************
 * TransactionCallbackMsgToSendEv
 * ------------------------------------------------------------------------
 * General: Indicates that a transaction-related outgoing message is about
 *          to be sent.
 * Return Value: RV_OK (the returned status is ignored in the current
 *                           stack version)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc      - The transaction pointer.
 *            hMsgToSend   - The handle to the outgoing message.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackMsgToSendEv(
                            IN Transaction        *pTransc,
                            IN RvSipMsgHandle      hMsgToSend);

/***************************************************************************
 * TransactionCallBackMsgReceivedEv
 * ------------------------------------------------------------------------
 * General: Indicates that a transaction-related incoming message has been
 *          received.
 * Return Value: RV_OK
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc      - The transaction handle.
 *            hMsgReceived - Handle to the incoming message.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallBackMsgReceivedEv (
                            IN Transaction            *pTransc,
                            IN RvSipMsgHandle         hMsgReceived);



/***************************************************************************
 * TransactionCallbackAppInternalClientCreatedEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that a new
 *          client transaction has been created by the SIP Stack. The newly
 *          created transaction always assumes the Idle state.
 *          This callback is called for CANCEL and PRACK transactions that are
 *          created automatically by the SIP Stack.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc     - The new SIP Stack transaction pointer.
 ***************************************************************************/
void RVCALLCONV TransactionCallbackAppInternalClientCreatedEv(
                      IN    Transaction             *pTransc);



/***************************************************************************
 * TransactionCallbackCancelledEv
 * ------------------------------------------------------------------------
 * General: Notifies the application when a CANCEL request is received on a
 *          transaction.
 *          Note: This is not the CANCEL transaction but the cancelled
 *          transaction. For example an INVITE request that was cancelled.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc      - The transaction pointer
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackCancelledEv (
                      IN Transaction            *pTransc);


/***************************************************************************
* TransactionCallbackCallTranscCreatedEv
* ------------------------------------------------------------------------
* General: Call the pfnEvTransactionCreated callback.
* Return Value: (-).
* ------------------------------------------------------------------------
* Arguments:
* Input: pTranscMgr   - Handle to the transaction manager.
*         hTransc - Handle to the newly created transaction.
* In-Out pKey         - pointer to a temporary transaction key. The key is
*                       replaced with the call-leg key (on the call-leg memory
*                       page).
* Output:ppOwner      - pointer to the new transaction owner.
*        tripleLock      - pointer to the new locks structure
***************************************************************************/
void RVCALLCONV TransactionCallbackCallTranscCreatedEv(
                                     IN     TransactionMgr           *pTranscMgr,
                                     IN     RvSipTranscHandle         hTransc,
                                     INOUT  SipTransactionKey        *pKey,
                                     OUT    void                     **ppOwner,
                                     OUT    SipTripleLock             **tripleLock,
                                     OUT    RvUint16                *statusCode);

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * TransactionCallbackCallRelProvRespRcvdEv
 * ------------------------------------------------------------------------
 * General: Notifies the call-leg that a reliable provisional response has
 *            received. This callback is called only if the manualPrack flag is
 *            set to RV_TRUE.
 * Return Value: RV_OK - success.
 *               RV_ERROR_OUTOFRESOURCES - No resources to add personal information
 *                                   to the message.
 *               RV_ERROR_UNKNOWN - failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction.
 *       rseqStep - the rseq step that was received in this 1xx message.
 * Output:(-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackCallRelProvRespRcvdEv(
                                    IN  Transaction    *pTransc,
                                    IN  RvUint32        rseqStep);

/***************************************************************************
 * TransactionCallbackCallIgnoreRelProvRespEv
 * ------------------------------------------------------------------------
 * General: Notifies the call-leg that a reliable provisional response has
 *            received. This callback is called only if the manualPrack flag is
 *            set to RV_TRUE.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:pTransc - The transaction.
 *       rseqStep - the rseq step that was received in this 1xx message.
 * Output:pbIgnore - ignore the 1xx or not.
 ***************************************************************************/
void RVCALLCONV TransactionCallbackCallIgnoreRelProvRespEv(
                                    IN  Transaction    *pTransc,
                                    IN  RvUint32         rseqStep,
                                    OUT RvBool*         pbIgnore);
#endif /* RV_SIP_PRIMITIVES */

/***************************************************************************
 * TransactionCallbackSupplyTranscParamsEv
 * ------------------------------------------------------------------------
 * General: Notified the application that an internally created client
 *          transaction needs a CSeq step and a Request URI from its owner.
 *          The application should supply these parameters to the transaction.
 *          In this version, this callback is called only for PRACK client
 *          transactions.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc      - Pointer to the transaction structure
 * Output:  pCSeqStep    - CSeq step for the transaction
 *          phRequestUri - Request URI for the transaction.
 ***************************************************************************/
void RVCALLCONV TransactionCallbackSupplyTranscParamsEv(
                            IN    Transaction               *pTransc,
                            OUT   RvInt32                 *pCSeqStep,
                            OUT   RvSipAddressHandle       *phRequestUri);

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * TransactionCallbackAuthCredentialsFoundEv
 * ------------------------------------------------------------------------
 * General:  Supply an authorization header, to pass it to the user,
 *           that will continue the authentication procedure, according to
 *           the realm and username parameters in it.
 *           in order to coninue the procedure, user shall use
 *           RvSipTransactionAuthProceedExt().
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc   - Pointer to the transaction structure .
 *         hAuthorization - Handle to the authorization header that was found.
 *         isSupported    - TRUE if supported, FALSE elsewhere.
 ***************************************************************************/
void RVCALLCONV TransactionCallbackAuthCredentialsFoundEv(
                                    IN Transaction                      *pTransc,
                                    IN RvSipAuthorizationHeaderHandle hAuthorization,
                                    IN RvBool                        isSupported);


/***************************************************************************
 * TransactionCallbackAuthCompletedEv
 * ------------------------------------------------------------------------
 * General:  Notify that authentication procedure is completed.
 *           If it is completed because a correct authorization was found,
 *           bAuthSucceed is RV_TRUE.
 *           If it is completed because there are no more authorization headers
 *           to check, or because user ordered to stop the searching for
 *           correct header, bAuthSucceed is RV_FALSE.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - Pointer to the transaction structure .
 *        bAuthSucceed - RV_TRUE if we found correct authorization header,
 *                       RV_FALSE if we did not.
 ***************************************************************************/
void RVCALLCONV TransactionCallbackAuthCompletedEv(
                                           IN Transaction* pTransc,
                                           IN  RvBool      bAuthSucceed);
#endif /* #ifdef RV_SIP_AUTH_ON */

/***************************************************************************
 * TransactionCallbackOpenCallLegEv
 * ------------------------------------------------------------------------
 * General: When a request that is suitable for opening a dialog is received
 *          (INVITE/ REFER/SUBSCRIBE-with no To tag), the Transaction layer
 *          asks the application whether to open a call-leg for this transaction.
 *          For a proxy application, the callback is called for INVITE/REFER/
 *          SUBSCRIBE methods.
 *          For UA applications, the callback is called only for initial
 *          REFER/SUBSCRIBE methods.
 *          An application that does not want the Stack implementation that
 *          opens a new dialog for REFER and SUBSCRIBE should implement
 *          this callback.
 *          This function can be used by proxies that wish to handle specific
 *          requests in a call-leg context by themselves.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc    - Pointer to the transaction structure
 * Output:  pbNotifyCallLegMgr - RV_TRUE if we should notify the call-leg manager
 *                       of the new transaction. RV_FALSE otherwise.
 ***************************************************************************/
void RVCALLCONV TransactionCallbackOpenCallLegEv(
                                         IN  Transaction            *pTransc,
                                         OUT RvBool                *pbNotifyCallLegMgr);


/***************************************************************************
 * TransactionCallbackOtherURLAddressFoundEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that other URL address (URL that is
 *            currently not supported by the sip stack) was found and has
 *            to be converted to known SIP URL address.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - The transaction handle
 *            hAppTransc     - The application handle for this transaction.
 *            hMsg           - The message that includes the other URL address.
 *            hAddress       - Handle to unsupport address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackOtherURLAddressFoundEv (
                     IN  Transaction           *pTransc,
                     IN  RvSipMsgHandle         hMsg,
                     IN  RvSipAddressHandle        hAddress,
                     OUT RvSipAddressHandle        hSipURLAddress,
                     OUT RvBool                *pbAddressResolved);


/***************************************************************************
 * TransactionCallbackMgrOutOfContextMsgEv
 * ------------------------------------------------------------------------
 * General: Called when the transaction manager receives a message the does
 *          not match any existing transaction.
 *          The callback is called in the following cases:
 *          1. For a response message that does not match any client transaction.
 *          2. When ACK for 2xx response is received.
 *          3. When CANCEL is received and the canclled transaction is not found.
 *          This callback is used only for proxy applications. For user agent
 *          applications the ACK for 2xx is handles in the context of the invite
 *          transaction, responses that cannot be matched to any existing transaction
 *          are ignored and a CANCEL is responded with 481 automatically.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscMgr - The transaction manager which received the message
 *            hMsg       - The received Out of Context message.
 *            hLocalAddr - indicates the local address on which the message
 *                         was received.
 *            pRcvdFromAddr - The address from where the message
 *                         was received.
 *            pOptions -   Other identifiers of the message such as compression type.
 *            hConn      - If the message was received on a specific connection,
 *                         the connection handle is supplied. Otherwise this parameter
 *                         is set to NULL.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackMgrOutOfContextMsgEv (
                     IN TransactionMgr*                pTranscMgr,
                     IN RvSipMsgHandle                 hReceivedMsg,
                     IN RvSipTransportLocalAddrHandle  hLocalAddr,
                     IN SipTransportAddress*           pRcvdFromAddr,
                     IN RvSipTransportAddrOptions*     pOptions,
                     IN RvSipTransportConnectionHandle hConn);




/***************************************************************************
 * TransactionCallbackMgrNewRequestRcvdEv
 * ------------------------------------------------------------------------
 * General: Called when the transaction manager receives a new request that
 *          is not a retransmission and not an ACK request.
 *          The application should instruct the stack whether it should
 *          create a new transaction for the request or not.
 *          If you do not implement this callback, a new transaction
 *          will be created by default. Stateless proxies will usually want
 *          to prevent the stack from creating a new transaction.
 * Return Value: RvStatus - ignored in this version.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     0TranscMgr - The transaction manager that received the request.
 *          hMsg       - The handle to the new request message.
 * Output:  pbCreateTransc - Indicates whether the stack should handle the
 *                       new request by creating a new transaction.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackMgrNewRequestRcvdEv (
                        IN TransactionMgr*         pTranscMgr,
                        IN  RvSipMsgHandle         hMsg,
                        OUT RvBool               *pbCreateTransc);


/***************************************************************************
 * TransactionCallbackMgrOtherURLAddressFoundEv
 * ------------------------------------------------------------------------
 * General: Called when the transaction manager sends an out of context message.
 *            Notifies the application that other URL address (URL that is
 *            currently not supported by the RvSip stack) was found and has
 *            to be converted to known SIP URL address.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscMgr     - The transaction manager that received the request.
 *            hMsg           - The handle to the new request message.
 *            hAddress       - Handle to unsupport address to be converted.
 * Output:    hSipURLAddress - Handle to the known SIP URL address.
 *            bAddressResolved-Indication of a successful/failed address
 *                             resolving.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackMgrOtherURLAddressFoundEv(
                        IN TransactionMgr         *pTranscMgr,
                        IN  RvSipMsgHandle        hMsg,
                        IN  RvSipAddressHandle      hAddress,
                        OUT RvSipAddressHandle      hSipURLAddress,
                        OUT RvBool                   *bAddressResolved);


/***************************************************************************
 * TransactionCallbackMgrFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: Call the transaction manager pfnEvFinalDestResolved callback.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTranscMgr     - The transaction manager pointer.
 *            hMsg           - The handle to the message that is about to be sent.
 *            hTrx           - The transmitter that is going to send the message.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackMgrFinalDestResolvedEv(
                          IN TransactionMgr          *pTranscMgr,
                          IN RvSipMsgHandle           hMsg,
                          IN RvSipTransmitterHandle   hTrx);




/***************************************************************************
 * TransactionCallbackFinalDestResolvedEv
 * ------------------------------------------------------------------------
 * General: This callback indicates that the transaction is about to send
 *          a message after the destination address was resolved.
 *          The callback supplies the final message.
 *          Changes in the message at this stage will
 *          not effect the destination address.
 *          When this callback is called the application can query the
 *          transaction about the destination address using the
 *          RvSipTransactionGetCurrentDestAddr() API function.
 *          If the application wishes it can update the sent-by part of
 *          the top most via. The application must not update the branch parameter.
 * Return Value: RV_OK (the returned status is ignored in the current
 *                           stack version)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc      - The transaction pointer.
 *            hMsgToSend   - The handle to the outgoing message.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackFinalDestResolvedEv(
                            IN Transaction        *pTransc,
                            IN RvSipMsgHandle      hMsgToSend);


/***************************************************************************
 * TransactionCallbackNewConnInUseEv
 * ------------------------------------------------------------------------
 * General: Calls the pfnNewConnInUseEvHandler callback
 * Return Value: RV_OK (the returned status is ignored in the current
 *                      stack version)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc  - The transaction pointer.
 *            hConn    - The handle to the connection.
 *            bNewConnCreated - The connection is also a newly created connection
 *                              (Not a connection that was found in the hash).
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackNewConnInUseEv(
                     IN  Transaction*                   pTransc,
                     IN  RvSipTransportConnectionHandle hConn,
                     IN  RvBool                         bNewConnCreated);


#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * TransactionCallbackSigCompMsgNotRespondedEv
 * ------------------------------------------------------------------------
 * General: Notifies the application that no response to the last sent
 *          SigComp message was received.
 * Return Value: RvStatus (RV_OK on success execution or RV_ERROR_UNKNOWN
 *                 otherwise).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc       - The transaction handle
 *          retransNo     - The number of retransmissions of the request
 *                          until now.
 *          ePrevMsgComp  - The type of the previous not responded request
 * Output:  peNextMsgComp - The type of the next request to retransmit (
 *                          RVSIP_TRANSMITTER_MSG_COMP_TYPE_UNDEFINED means that the
 *                          application wishes to stop sending requests).
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackSigCompMsgNotRespondedEv (
                     IN  Transaction                 *pTransc,
                     IN  RvInt32                      retransNo,
                     IN  RvSipTransmitterMsgCompType  ePrevMsgComp,
                     OUT RvSipTransmitterMsgCompType *peNextMsgComp);
#endif /* RV_SIGCOMP_ON */


/***************************************************************************
 * TransactionCallbackMgrAckNoTranscEv
 * ------------------------------------------------------------------------
 * General: Call the transaction manager pfnAckNoTranscRcvd callback.
 *
 * Return Value: RV_Status
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr    - The transaction manager handle.
 *          hAckMsg       - The handle to the received ACK message.
 *          pRcvdFromAddr - The address that the response was received from
 * Output:  pbWasHandled  - Indicates if the message was handled in the
 *                          callback.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackMgrAckNoTranscEv(
                     IN    TransactionMgr*			pTranscMgr,
                     IN    RvSipMsgHandle			hAckMsg,
                     IN    SipTransportAddress*     pRcvdFromAddr,
                     OUT   RvBool*					pbWasHandled);

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * TransactionCallbackMgrInviteResponseNoTranscEv
 * ------------------------------------------------------------------------
 * General: Call the transaction manager pfnInviteResponseNoTranscRcvd callback.
 *
 * Return Value: RV_Status
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr    - The transaction manager handle.
 *          hResponseMsg  - The handle to the received response message.
 *          pRcvdFromAddr - The address that the response was received from
 * Output:  pbWasHandled  - Indicates if the message was handled in the
 *                          callback.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCallbackMgrInviteResponseNoTranscEv(
                     IN    TransactionMgr*			pTranscMgr,
                     IN    RvSipMsgHandle			hMsg,
                     IN    SipTransportAddress*     pRcvdFromAddr,
                     OUT   RvBool*					pbWasHandled);

/***************************************************************************
 * TransactionCallbackCallInviteTranscMsgSentEv
 * ------------------------------------------------------------------------
 * General: Notifies the call-leg that the invite transaction is going to
 *          be terminated. this happens on first 2xx sending, when working 
 *          with new-invite handling.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:pTransc - The transaction.
 ***************************************************************************/
void RVCALLCONV TransactionCallbackCallInviteTranscMsgSentEv(
                               IN  Transaction                  *pTransc,
                               IN SipTransactionMsgSentOption   eOption);

#else /*ifndef RV_SIP_PRIMITIVES*/
/***************************************************************************
 * TransactionCallbackMgrAppForkedInviteRespEv
 * ------------------------------------------------------------------------
 * General: Notifies of a transaction state change and the
 *          associated state change reason.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr     - The transaction manager pointer.
 *          hMsg           - The handle to the message that is about to be sent.
 ***************************************************************************/
void RVCALLCONV  TransactionCallbackMgrAppForkedInviteRespEv(
                            IN TransactionMgr  *pTranscMgr,
                            IN RvSipMsgHandle  hMsg);

#endif /*#ifndef RV_SIP_PRIMITIVES*/

/***************************************************************************
 * TransactionCallBacksNoOwnerEvStateChanged
 * ------------------------------------------------------------------------
 * General: Called when a state changes in one of the transactions.
 * This is a default implementation that will be set to the callbacks
 * in the construct function, and after detaching the transaction's owner.
 * Return Value: -.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pOwner -       The owner of the transaction in which the "event
 *                         state changed" occured.
 *            hTransc - The transaction in which the "event state changed"
 *                         occured.
 *            eNewState -    The new state of the transaction.
 *            eReason -      The reason for the state change.
 ***************************************************************************/
void RVCALLCONV TransactionCallBacksNoOwnerEvStateChanged(
                            IN RvSipTranscHandle                   hTransc,
                            IN RvSipTranscOwnerHandle              hOwner,
                            IN RvSipTransactionState               eNewState,
                            IN RvSipTransactionStateChangeReason   eReason);



#endif /*#ifndef RV_SIP_LIGHT*/

#ifdef __cplusplus
}
#endif

#endif /*END OF #ifndef TRANSACTION_CALL_BACKS_H */

/*nl for linux */

