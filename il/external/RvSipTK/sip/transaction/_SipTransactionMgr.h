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
 *                              <SipTransactionMgr.h>
 *
 * This file defines the transactions manager object, attributes and interface
 * functions. A transaction object is implemented by the name
 * SipTransactionMgr. The interface functions defined here implement methods
 * to construct and destruct SipTransactionMgr object; and functions to
 * control the elements of the data-structure: methods to insert, delete and
 * find an element within the data-structure.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                  Nov 2000
 *********************************************************************************/

#ifndef SIP_TRANSACTION_MGR_H
#define SIP_TRANSACTION_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "RvSipMsg.h"
#include "_SipTransaction.h"
#include "_SipTransactionTypes.h"
#include "RvSipResourcesTypes.h"
#include "TransactionMgrObject.h"
#include "TransactionObject.h"
#include "rvlog.h"
#ifdef RV_SIP_IMS_ON
#include "RvSipSecAgreeTypes.h"
#endif /* #ifdef RV_SIP_IMS_ON */


#ifdef RV_SIGCOMP_ON
#include "RvSigComp.h"
#endif /* RV_SIGCOMP_ON */

/*************************************************************************
 * Construct and Destruct functions
 *************************************************************************/


/***************************************************************************
 * SipTransactionMgrConstruct
 * ------------------------------------------------------------------------
 * General: Called in the Stack initialization process.
 * Initiates data-structures. Sets global memory-pools, timer-pool,
 * log Id number to the manager. Signs transaction default callbacks to the
 * manager.
 * Return Value: RV_OK - The out parameter pTranscMgr now points
 *                            to a valid transaction manager object.
 *                 RV_ERROR_OUTOFRESOURCES - Couldn't allocate memory for the
 *                                     transaction manager object.
 *                 RV_ERROR_NULLPTR - pConfiguration or phTranscMgr are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pConfiguration - Managers parameters as were defined in the
 *                             configuration.
 * Output:    phTranscMgr          - The new transaction manager object created.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrConstruct(
                              INOUT  SipTranscMgrConfigParams  *pConfiguration,
                              OUT    RvSipTranscMgrHandle      *phTranscMgr);

/***************************************************************************
 * SipTransacMgrSetEnableNestedInitialRequest
 * ------------------------------------------------------------------------
 * General: Sets the bEnableNestedInitialRequestHandling parameter to RV_TRUE.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr - A handle to the transaction manager
 ***************************************************************************/
void RVCALLCONV SipTransacMgrSetEnableNestedInitialRequest(
                                   IN RvSipTranscMgrHandle hTranscMgr);

/***************************************************************************
 * SipTransactionMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the transactions data - structure.
 * Return Value: RV_OK - The transaction manager has been successfully
 *                            destructed.
 *               RV_ERROR_NULLPTR - The pointer to the transaction manager to be
 *                               deleted (received as a parameter) is a NULL
 *                                 pointer.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscMgr - A handle to the transaction manager that will be
 *                       destructed.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrDestruct(
                                        IN RvSipTranscMgrHandle hTranscMgr);


/***************************************************************************
 * SipTransactionMgrSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets a set of event handlers identified with the entity received
 * in the eOwnerType parameter.
 * Each of the entities which can own a transaction must attach a
 * set of implementations to the transaction's callbacks.
 * Return Value: RV_ERROR_NULLPTR - The pointer to the hanlers struct is NULL.
 *                 RV_OK - The event handlers were successfully set.
 *                 RV_ERROR_BADPARAM - Size of struct is negativ,
 *                                       or owner enumeration is UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscMgr - A handle to the transaction manager to set the
 *                       handlers to.
 *            eOwnerType - The type of the owner to which the event handlers
 *                       belong.
 *          pHandlers  - The struct of the event handlers.
 *          evHandlerStructSize - The size of the pHandlers struct.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrSetEvHandlers(
                        IN RvSipTranscMgrHandle           hTranscMgr,
                        IN SipTransactionOwner            eOwnerType,
                        IN SipTransactionMgrEvHandlers   *pMgrDetails,
                        IN RvUint32                      evMgrHandlerStructSize,
                        IN RvSipTransactionEvHandlers    *pHandlers,
                        IN RvUint32                      evHandlerStructSize);

/***************************************************************************
* SipTransactionMgrMessageReceived
* ------------------------------------------------------------------------
* General: Searches the data - structure for a transaction
* uniquely defined by the key values that were received in the message.
* If the transaction exist it will receive the
* new message. If it is doesn't exist, and a new transaction should be
* created (when the message is a request message, except for an ACK request),
* a new transaction is created. A new transaction created will also
* be given the new message received but before that the event that indicates a
* new transaction will be called.
* Return Value: RV_OK - The message has been received by a compatible
*                              transaction.
*                 RV_ERROR_NULLPTR - The transaction manager pointer or the message
*                               pointer are NULL pointers
*                 RV_OutOfResource - Couldn't allocate a new transaction when
*                                    required.
*               RV_ERROR_BADPARAM - The message is invalid.
* ------------------------------------------------------------------------
* Arguments:
* Input:  pReceivedMsg -   A message belonging to the transaction this
*                          function is searching for. There can be only
*                          one transaction this message belongs to.
*          hTranscMgr  -   The transactions manager.
*          pLocalAddr  -   The local address to which the request message was
*                          received.
*          pRecvFromAddr - The address from which the message was received.
*          eBSAction    -  Bad-syntax action, that was given by application in
*                          bad-syntax callbacks. continue processing, reject, discard.
*          pSigCompMsgInfo - SigComp info related to the received message.
*                            The information contains indication if the UDVM
*                            waits for a compartment ID for the given unique
*                            ID that was related to message by the UDVM.
***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrMessageReceivedEv(
                         IN    SipTransportCallbackContextHandle msgContext,
                         IN    RvSipMsgHandle                    hReceivedMsg,
                         IN    RvSipTransportConnectionHandle    hConn,
                         IN    RvSipTransportLocalAddrHandle     hLocalAddr,
                         IN    SipTransportAddress*              pRecvFromAddr,
                         IN    RvSipTransportBsAction            eBSAction,
                         IN    SipTransportSigCompMsgInfo       *pSigCompMsgInfo);

#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT
/***************************************************************************
 * SipTransactionMgrReprocessTransactionEv
 * ------------------------------------------------------------------------
 * General: This function is called from the event queue due to its insertion
 *          by RvSipTransactionMgrReprocessTransaction().
 *          Reprocess a transaction: The transaction manager will terminate 
 *          the given transaction, and will reprocess the request that was 
 *          received by it via a new transaction. 
 *          Notice: The transaction you supply here must be in state 
 *          SERVER_GEN_REQUEST_RCVD or in state SERVER_INVITE_REQUEST_RCVD, 
 *          and it must have a valid hReceivedMsg.
 *          Notice: The TransactionTerminated() callback will notify the 
 *          termination of the old transaction. The CallLegTransactionCreated() 
 *          callback will notify the creation of the new transaction. The new 
 *          transaction will notify the processing of the request via callbacks 
 *          MsgRcvd(), StateChanged(), etc.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc     - The transaction to be reprocessed.
 *          
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrReprocessTransactionEv(IN  void        *hTransc,
															IN  RvInt32      notUsed);
#endif /* #ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */

/***************************************************************************
* SipTransactionMgrCreateServerTransactionFromMsg
* ------------------------------------------------------------------------
* General: Called to create a new Server transaction from a request message that
 *         the application supplies. This function creates a new transaction in the
 *         Request-Receaved state.
* Return Value: RV_OK - The message has been received by a compatible
*                              transaction.
*                 RV_ERROR_NULLPTR - The transaction manager pointer or the message
*                               pointer are NULL pointers
*                 RV_OutOfResource - Couldn't allocate a new transaction when
*                                    required.
*               RV_ERROR_BADPARAM - The message is invalid.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hTranscMgr   -   The transactions manager.
*          hTranscOwner -   The application owner of the transaction.
*          hMsg         -   The handle to the request message that the application
*                           supplies to create the transaction
* Output:  pTransc      -   A pointer to the address of the new trasnaction created.
***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrCreateServerTransactionFromMsg(
                                              IN  TransactionMgr          *pTranscMgr,
                                              IN  RvSipTranscOwnerHandle   hTranscOwner,
                                              IN  RvSipMsgHandle           hMsg,
                                              OUT Transaction             **pTransc);


/***************************************************************************
 * SipTransactionMgrGetKeyFromMessage
 * ------------------------------------------------------------------------
 * General: Gets the To, From, Call-Id and CSeq-Step values from the received
 *          message
 * Return Value: RV_ERROR_BADPARAM - The message is missing one of these
 *                                     values.
 *                 RV_OK - All the values were updated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr - The transaction manager.
 *          hMsg - the received message.
 * Output:    key - a structure with the key taken from the message.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrGetKeyFromMessage(
                                    IN  RvSipTranscMgrHandle    hTranscMgr,
                                    IN  RvSipMsgHandle          hMsg,
                                    OUT SipTransactionKey      *pKey);

/***************************************************************************
 * SipTransactionMgrFindTransaction
 * ------------------------------------------------------------------------
 * General: Finds a transaction acording to the received key. If the transaction
 *          existes give the message to the transaction.
 * Return Value: RV_OK - The transaction was found and the message was
 *                            given successfully.
 *                 RV_ERROR_OUTOFRESOURCES - Not enough resources.
 *                 RV_ERROR_NULLPTR - Bad parameters.
 *               RV_ERROR_NOT_FOUND - The transaction was not found.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr - The transaction manager.
 *          hReceivedMsg - the received message.
 *          pKey - The transaction key taken from the message
 *          bIsUac - is the transaction should be client or server.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrFindTransaction(
                                    IN TransactionMgr        *pTranscMgr,
                                    IN RvSipMsgHandle         hReceivedMsg,
                                    IN SipTransactionKey     *pKey,
                                    IN RvBool                bIsUac,
                                    OUT Transaction           **pTransc);

/***************************************************************************
 * SipTransactionMgrGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: Get the status of the transaction manager's resources. The
 *          transaction manager's resources are the transaction's pool list
 *          and the Via headers' pool list.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTranscMgr - The transaction manager.
 * Output:  pResources - The transaction's resources structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrGetResourcesStatus (
                                 IN  RvSipTranscMgrHandle  hTranscMgr,
                                 OUT RvSipTranscResources *pResources);

/***************************************************************************
 * SipTransactionMgrResetMaxUsageResourcesStatus
 * ------------------------------------------------------------------------
 * General: Reset the counter that counts the max number of transactions that
 *          were used ( at one time ) until the call to this routine.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTranscMgr - The transaction manager.
 ***************************************************************************/
void RVCALLCONV SipTransactionMgrResetMaxUsageResourcesStatus (
                                 IN  RvSipTranscMgrHandle  hTranscMgr);

#ifdef SIP_DEBUG

/***************************************************************************
 * SipTransactionMgrGetStatistics
 * ------------------------------------------------------------------------
 * General: Get the statistics of received and sent messages. This statistic
 *          is managed by the transaction manager.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTranscMgr - The transaction manager.
 * Output:  pStatistics - The transaction's manager statistics structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrGetStatistics(
                                 IN  RvSipTranscMgrHandle         hTranscMgr,
                                 OUT SipTransactionMgrStatistics *pStatistics);
#endif

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * SipTransactionMgrGet100relStatus
 * ------------------------------------------------------------------------
 * General: returns the 100 rel status kept in the transaction manager
 * Return Value: 100 rel status of the transaction manager
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr - The transaction manager.
 *
 ***************************************************************************/
RvSipTransaction100RelStatus RVCALLCONV SipTransactionMgrGet100relStatus(
                                    IN RvSipTranscMgrHandle hTranscMgr);

/***************************************************************************
 * SipTransactionMgrGetT1
 * ------------------------------------------------------------------------
 * General: returns the T1 timeout value kept in the transaction manager
 * Return Value: 100 rel status of the transaction manager
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr - The transaction manager.
 ***************************************************************************/
RvInt32 RVCALLCONV SipTransactionMgrGetT1(
                                    IN RvSipTranscMgrHandle hTranscMgr);
/***************************************************************************
 * SipTransactionMgrGetT2
 * ------------------------------------------------------------------------
 * General: returns the T2 timeout value kept in the transaction manager
 * Return Value: 100 rel status of the transaction manager
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr - The transaction manager.
 ***************************************************************************/
RvInt32 RVCALLCONV SipTransactionMgrGetT2(
                                    IN RvSipTranscMgrHandle hTranscMgr);

#endif /* RV_SIP_PRIMITIVES */

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipTransactionMgrSetSecAgreeMgrHandle
 * ------------------------------------------------------------------------
 * General: Sets the security-agreement manager handle in transaction mgr.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr         - Handle to the transaction manager.
 *            hSecAgreeMgr - Handle to the security-agreement manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionMgrSetSecAgreeMgrHandle(
                                        IN RvSipTranscMgrHandle     hMgr,
                                        IN RvSipSecAgreeMgrHandle   hSecAgreeMgr);
#endif /* #ifdef RV_SIP_IMS_ON */

#endif /*#ifndef RV_SIP_LIGHT*/
#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef SIP_TRANSACTION_MGR_H */
