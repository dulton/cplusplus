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
 *                              <SipTransaction.h>
 *
 * This file defines the transaction object, interface
 * functions. A transaction object handle is called RvSipTranscHandle.
 * The interface functions defined here implement methods to
 * construct and destruct SipTransaction object; to attach an owner to the
 * transaction object, and a set of callbacks belonging to this owner; to attach
 * a lock to the inner lock handle of the object; and to get all the key fields
 * of a transaction. A few functions are defined to manipulate transactions.
 * These functions are also defined here and they are: Terminate a transaction
 * object, Request, Respond and Acknowledge.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                  Nov 2000
 *********************************************************************************/


#ifndef SIP_TRANSACTION_H
#define SIP_TRANSACTION_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "_SipTransactionTypes.h"
#include "RvSipTransactionTypes.h"
#include "RvSipTransaction.h"
#include "_SipCommonTypes.h"
#include "RvSipCommonList.h"
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctpTypes.h"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    
/*************************************************************************
 * Construct and Destruct functions
 *************************************************************************/


/***************************************************************************
 * SipTransactionCreate
 * ------------------------------------------------------------------------
 * General: Called when a new Uac transaction is created.
 * The transaction key values are received as parameters
 * and set to the transaction's inner parameters.
 * The transaction's state will be set to "Initial" state.
 * The default response-code will be set to 0.
 * The Transaction's callbacks will be set with a set of callbacks that
 * implement a default behavior.
 * The transaction will sign itself to the hash manager, in other words the
 * transaction will call the ManageTransaction function of the transactions
 * manager with a handle to itself.
 * Return Value: RV_OK - The out parameter pTransc now points
 *                            to a valid transaction, initialized by the
 *                              received key.
 *                 RV_ERROR_BADPARAM - At least one of the transaction key
 *                                     values received as a parameter is not
 *                                     legal.
 *                                     All key fields must have legal values.
 *                 RV_ERROR_NULLPTR - At least one of the pointers to the
 *                                 transaction key values is a NULL pointer,
 *                                 or the pointer to the transaction pointer is a
 *                                 NULL pointer.
 *               RV_ERROR_OUTOFRESOURCES - Couldn't allocate memory for the
 *                                     transaction object.
 *                                     Another possibility is that the transaction
 *                                     manager is full. In that case the
 *                                     transaction will not be created and this
 *                                     value is returned.
 *               RV_ERROR_UNKNOWN - A transaction with the given key already exists.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr    - The transaction manager to which this transaction will
 *                          belong.
 *            pKey        -  The transaction's KEY.
 *          pOwner        -  The transaction's owner.
 *          eOwnerType    -  The transaction's owner type.
 *          tripleLock    -  The transaction's lock, processing lock, API lock
 *                            and API counter.
 * Output:  phTransaction - The new transaction created.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionCreate(
                                    IN  RvSipTranscMgrHandle   hTranscMgr,
                                    IN  SipTransactionKey     *pKey,
                                    IN  void                  *pOwner,
                                    IN  SipTransactionOwner    eOwnerType,
                                    IN  SipTripleLock           *tripleLock,
                                    OUT RvSipTranscHandle     *phTransaction);

/***************************************************************************
 * SipTransactionTerminate
 * ------------------------------------------------------------------------
 *  General: Causes immediate shut-down of the transaction:
 *  Changes the transaction's state to "terminated" state.
 *  Calls Destruct on the transaction being
 *  terminated. The terminated state will not be notified to the owner.
 *  This function is used only by the call-leg.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTransc - The transaction to terminate.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionTerminate(
                                            IN RvSipTranscHandle hTransc);

/***************************************************************************
 * SipTransactionRequest
 * ------------------------------------------------------------------------
 * General: Creates and sends a request message according to the given method,
 *          with the given address as a Request-Uri.
 * Return Value: RV_OK - The message was created and sent successfully.
 *                 RV_ERROR_NULLPTR - The Request-Uri pointer is a NULL pointer,
 *                                 or the transaction pointer is a NULL pointer.
 *                 RV_ERROR_ILLEGAL_ACTION - The Request was called in a state where
 *                                    request can not be executed.
 *                 RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -   The transaction this request refers to.
 *            eRequestMethod - The enumeration value of the request method.
 *            pRequestUri -    The Request-Uri to be used in the request
 *                           message.
 *                           The message's Request-Uri will be the address
 *                           given here, and the message will be sent
 * Output:(-)
***************************************************************************/
RvStatus RVCALLCONV SipTransactionRequest(
                                           IN  RvSipTranscHandle           hTransc,
                                           IN  SipTransactionMethod        eRequestMethod,
                                           IN  RvSipAddressHandle          hRequestUri);

/***************************************************************************
 * SipTransactionAck
 * ------------------------------------------------------------------------
 * General:  Creates and sends an acknowledgment message with the given
 *           address as the Request-Uri.
 * Return Value: RV_OK - The message was created and sent successfully.
 *                 RV_ERROR_NULLPTR - The Request-Uri pointer is a NULL pointer,
 *                                 or the transaction pointer is a NULL pointer.
 *                 RV_ERROR_ILLEGAL_ACTION - The Ack was called in a state where
 *                                    acknowledgment can not be executed.
 *                 RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTransc - The transaction this request refers to.
 *            pRequestUri -  The Request-Uri to be used in the ACK message. The
 *                           message's Request-Uri will be the address given
 *                         here, and the message will be sent accordingly.
 *                         If the request URI is NULL then the request URI of the
 *                         transaction is used as the request URI. The request URI
 *                         can be NULL when the Ack is to a non 2xx response, and then
 *                         the request URI must be the same as the invite request URI
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionAck(
                                       IN  RvSipTranscHandle  hTransc,
                                       IN  RvSipAddressHandle hRequestUri);


/***************************************************************************
 * SipTransactionSendPrackResponse
 * ------------------------------------------------------------------------
 * General:  Creates and sends a response to a PRACK message.
 * Return Value: RV_OK - The message was created and sent successfully.
 *                 RV_ERROR_NULLPTR - The Request-Uri pointer is a NULL pointer,
 *                                 or the transaction pointer is a NULL pointer.
 *                 RV_ERROR_ILLEGAL_ACTION - The Ack was called in a state where
 *                                    acknowledgment can not be executed.
 *                 RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTransc - The INVITE transaction handle.
 *            responseCode - The response code of the response.
 * Output:  (-)
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionSendPrackResponse(
                                       IN  RvSipTranscHandle  hTransc,
                                       IN  RvUint16          responseCode);

/***************************************************************************
 * SipTransactionGetPrackParent
 * ------------------------------------------------------------------------
 * General:  Gets the handle to the PRACK parent transaction.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hPrackTransc - The PRACK transaction handle.
 * Output:  hTransc      - Handle to the PRACK parent transaction.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGetPrackParent(
                                       IN  RvSipTranscHandle   hPrackTransc,
                                       OUT RvSipTranscHandle  *hTransc);


/*************************************************************************
 * Get transaction key values
 *************************************************************************/
/***************************************************************************
 * SipTransactionGetTransport
 * ------------------------------------------------------------------------
 * General: Returns the transport type the transaction is using.
 * Return Value: the transport type
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to get method.
 * Output:
 ***************************************************************************/
RvSipTransport RVCALLCONV SipTransactionGetTransport(
                                          IN RvSipTranscHandle        hTransc);


/***************************************************************************
 * SipTransactionWasAckSent
 * ------------------------------------------------------------------------
 * General: Ack was actually sent over the network.
 * Return Value: RV_TRUE - if ack was actually sent.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle
 ***************************************************************************/
RvBool RVCALLCONV SipTransactionWasAckSent(IN RvSipTranscHandle        hTransc);


/***************************************************************************
 * SipTransactionGetCallId
 * ------------------------------------------------------------------------
 * General: Returns the transaction's Call-Id value. This value is indicated
 *          by the offset within a page of a memory pool, all received
 *          in the returned rpool pointer.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The handle to the transaction is invalid.
 *               RV_ERROR_NULLPTR - The pointer to the rpool pointer is invalid.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction from which to get Call-Id value.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGetCallId(
                                            IN  RvSipTranscHandle hTransc,
                                            OUT RPOOL_Ptr      *pstrCallId);

/***************************************************************************
 * SipTransactionGetMethod
 * ------------------------------------------------------------------------
 * General: Returns the transaction's method type.If the transaction method
 *               is kept as a string RVSIP_TRANSACTION_METHOD_OTHER will be
 *               returned.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to get method.
 * Output:  peMethod - The transaction's method type.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGetMethod(
                                          IN RvSipTranscHandle        hTransc,
                                          OUT SipTransactionMethod *peMethod);

/***************************************************************************
 * SipTransactionSetMethodStr
 * ------------------------------------------------------------------------
 * General: Copy the method sting on to the transaction page.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc - The transaction handle.
 *            strMethod - The transaction string method.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionSetMethodStr(
                                    IN RvSipTranscHandle       hTransc,
                                    IN RvSipMethodType         eMethod,
                                    IN  RvChar               *strMethod);

/***************************************************************************
 * SipTransactionIsEqualBranchToMsgBranch
 * ------------------------------------------------------------------------
 * General: Compare the transaction branch to a branch with in a given message
 * Return Value: RV_TRUE if the branches are equal or if there was no branch in
 *               the transaction and the message. RV_FALSE otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to get method.
 *              Msg - A message to take the branch from
 ***************************************************************************/
RvBool RVCALLCONV SipTransactionIsEqualBranchToMsgBranch(
                                          IN RvSipTranscHandle        hTransc,
                                          IN RvSipMsgHandle           Msg);

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * SipTransactionSetIsReferNotifyTransaction
 * ------------------------------------------------------------------------
 * General: Should be used in a NOTIFY client transaction to mark that this
 *          transaction relates to a former REFER transaction.
 * Return Value: RvBool
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The handle to the transaction.
 ***************************************************************************/
/*RvStatus RVCALLCONV SipTransactionSetIsReferNotifyTransaction(
                                          IN  RvSipTranscHandle       hTransc);
*/
/***************************************************************************
 * SipTransactionGetIsReferNotifyTransaction
 * ------------------------------------------------------------------------
 * General: Returns true if this transaction is a NOTIFY transaction that is
 *          used to NOTIFY to a former REFER request.
 * Return Value: RV_TRUE if the transaction is a NOTIFY transaction
 *               which corresponds to a REFER request.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The handle to the transaction.
 ***************************************************************************/
/*RvBool RVCALLCONV SipTransactionGetIsReferNotifyTransaction(
                                          IN  RvSipTranscHandle       hTransc);
*/
#endif
/***************************************************************************
 * SipTransactionGetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Return the transaction outbound message handle
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - handle to transaction to which the message should be set
 * Output: phMsg - The outbound message of the transaction.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGetOutboundMsg(
                                                IN RvSipTranscHandle hTransc,
                                                OUT RvSipMsgHandle   *phMsg);

/***************************************************************************
 * SipTransactionSetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Sets a handle to the outbound message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hTransc - handle to transaction to which the message should be set
 *        hReceivedMsg - Handle to the received message.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionSetOutboundMsg(
                                              IN RvSipTranscHandle hTransc,
                                               IN RvSipMsgHandle    hOutboundMsg);


/***************************************************************************
 * SipTransactionSetOutboundProxy
 * ------------------------------------------------------------------------
 * General: Set the outbound address of the transaction. Any requests sent
 *          by this transaction will be sent according to the transaction
 *          outbound proxy address, unless the transaction is using a
 *          Record-Route path.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction
 *            outboundAddr  - The outbound proxy to set.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionSetOutboundAddress(
                            IN  RvSipTranscHandle           hTransc,
                            IN  SipTransportOutboundAddress *outboundAddr);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)

/***************************************************************************
 * SipTransactionGetStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV SipTransactionGetStateName (
                                      IN  RvSipTransactionState  eState);

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * SipTransactionGetMsgCompTypeName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given transaction compression message type
 * Return Value: The string with the message type.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eMsgCompType - The compression type as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV SipTransactionGetMsgCompTypeName (
                             IN  RvSipTransmitterMsgCompType eTranscMsgType);
#endif /* RV_SIGCOMP_ON */
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/



/***************************************************************************
 * SipTransactionGetLastState
 * ------------------------------------------------------------------------
 * General: returns the lastState enumeration
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle.
 ***************************************************************************/
RvSipTransactionState RVCALLCONV SipTransactionGetLastState(
                                                    IN  RvSipTranscHandle hTransc);

/***************************************************************************
 * SipTransactionGetResponseCode
 * ------------------------------------------------------------------------
 * General: Get the transaction response code.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle.
 *          responseCode - The response code  of the transaction.
 ***************************************************************************/
void RVCALLCONV SipTransactionGetResponseCode(IN  RvSipTranscHandle hTransc,
                                              OUT RvUint16         *responseCode);

/***************************************************************************
 * SipTransactionSetResponseCode
 * ------------------------------------------------------------------------
 * General: Set the transaction response code.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle.
 *          responseCode - The response code  of the transaction.
 ***************************************************************************/
void RVCALLCONV SipTransactionSetResponseCode(IN  RvSipTranscHandle hTransc,
                                              IN  RvUint16          responseCode);

/***************************************************************************
 * SipTransactionGetContext
 * ------------------------------------------------------------------------
 * General: Get pointer to the context memory saved by the transaction
 *          for the call-leg.
 * Return Value: The context memory.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction.
 ***************************************************************************/
void *RVCALLCONV SipTransactionGetContext(
                                IN  RvSipTranscHandle           hTransc);

/***************************************************************************
 * SipTransactionAllocateOwnerMemory
 * ------------------------------------------------------------------------
 * General: Append a consecutive memory to the transaction memory to be filled
 *          with owner information. The transaction holds the information
 *          for its owner. The pointer to this memory will be saved by
 *          the transaction and may be retrieved using SipTransactionGetContext().
 * Return Value: A pointer to the consecutive memory.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction.
 *          size - Size of memory to append.
 ***************************************************************************/
void *RVCALLCONV SipTransactionAllocateOwnerMemory(
                                IN  RvSipTranscHandle      hTransc,
                                IN  RvInt32               size);


/***************************************************************************
 * SipTransactionGetMsgMethodFromTranscMethod
 * ------------------------------------------------------------------------
 * General: Returns the equivalent message method from the transaction method.
 * Return Value: the equivalent message method from the transaction method.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction handle
 ***************************************************************************/
RvSipMethodType SipTransactionGetMsgMethodFromTranscMethod(
                            IN  RvSipTranscHandle    hTransc);

/*************************************************************************
 * Owner function
 *************************************************************************/


/***************************************************************************
 * SipTransactionDetachOwner
 * ------------------------------------------------------------------------
 * General: Called when the transaction's old owner is not it's owner any
 *          more. The transaction will begin acting as a stand alone transaction.
 * Return Value: RV_ERROR_NULLPTR - The hTransc is a NULL pointer.
 *               RV_ERROR_OUTOFRESOURCES - There wasn't enough memory for the
 *                                   transaction's To, From and Call-Id headers.
 *                                   (at this point the transaction allocates its
 *                                   own place for these).
 *               RV_OK - The detaching was successfully finished.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to detach owner.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionDetachOwner(
                                          IN RvSipTranscHandle hTransc);

/***************************************************************************
 * SipTransactionDetachFromOwnerKey
 * ------------------------------------------------------------------------
 * General: Called when the transaction's owner wishes to detach from the transaction
 *          key, i.e., to have its own independent key. After calling this function,
 *          changes made by the transaction on its key will not be reflected at the
 *          owner. For example, if the transaction will copy the To tag from a 
 *          received message, the owner will not be updated with this To tag.
 *          NOTE: If the function fails during the copying of the key, the old
 *                key (that is the owners key) will be restored in the transaction.
 * Return Value: RV_ERROR_NULLPTR - The hTransc is a NULL pointer.
 *               RV_ERROR_OUTOFRESOURCES - There wasn't enough memory for the
 *                                   transaction's To, From and Call-Id headers.
 *                                   (at this point the transaction allocates its
 *                                   own place for these).
 *               RV_OK - The detaching was successfully finished.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to detach owner key.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionDetachFromOwnerKey(
                                             IN RvSipTranscHandle hTransc);

/***************************************************************************
 * SipTransactionSetAppInitiator
 * ------------------------------------------------------------------------
 * General: Set the application as the initiator of the transaction.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle.
 ***************************************************************************/
void RVCALLCONV SipTransactionSetAppInitiator(IN RvSipTranscHandle hTransc);

/***************************************************************************
 * SipTransactionIsAppInitiator
 * ------------------------------------------------------------------------
 * General: Returns weather the initiator of the transaction is the
 *          application.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle.
 ***************************************************************************/
RvBool RVCALLCONV SipTransactionIsAppInitiator(IN RvSipTranscHandle hTransc);

/***************************************************************************
 * SipTransactionSetCancelTranscPair
 * ------------------------------------------------------------------------
 * General: Sets the handle to the cancel pair transaction.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 *          hCancelPair -  The cancel pair transaction handle.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionSetCancelTranscPair(
                                        IN RvSipTranscHandle    hTransc,
                                        IN RvSipTranscHandle    hCancelPair);

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * SipTransactionSetSubsInfo
 * ------------------------------------------------------------------------
 * General: Set the subscription info in the transaction.
 *          (Subscription info is the related object handle - notification or
 *          subscription).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle.
 *          pSubsInfo - the info to set.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionSetSubsInfo(IN RvSipTranscHandle hTransc,
                                          IN void*             pSubsInfo);

/***************************************************************************
 * SipTransactionGetSubsInfo
 * ------------------------------------------------------------------------
 * General: Get the subscription info from the transaction.
 *          (Subscription info is the related object handle - notification or
 *          subscription).
 * Return Value: pointer to the subsInfo.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle.
 *          pSubsInfo - the info to set.
 ***************************************************************************/
void* RVCALLCONV SipTransactionGetSubsInfo(IN RvSipTranscHandle hTransc);

/***************************************************************************
 * SipTransactionAllocateSessionTimerMemory
 * ------------------------------------------------------------------------
 * General: Append a consecutive memory to the transaction memory to be filled
 *          with a call-leg information. The transaction holds the session
 *          timer information for the call-leg.
 * Return Value: A pointer to the session timer pointer.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction.
 *          size - Size of memory to append.
 ***************************************************************************/
void *RVCALLCONV SipTransactionAllocateSessionTimerMemory(
                                IN  RvSipTranscHandle      hTransc,
                                IN  RvInt32               size);
/***************************************************************************
 * SipTransactionGetSessionTimerMemory
 * ------------------------------------------------------------------------
 * General: Get pointer to the context memory saved by the transaction
 *          for the call-leg.
 * Return Value: The context memory.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction.
 ***************************************************************************/
void *RVCALLCONV SipTransactionGetSessionTimerMemory(
                                IN  RvSipTranscHandle           hTransc);
#endif /*#ifndef RV_SIP_PRIMITIVES */

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/***************************************************************************
 * SipTransactionDNSContinue
 * ------------------------------------------------------------------------
 * General: Creates new transaction and copies there major transaction
 *          parameters and terminates the original transaction.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hOrigTransaction   - The source transaction handle.
 *          owner              - pointer to the new transaction owner info.
 *          bRemoveToTag       - Remove the to tag from transaction or not.
 *                             (This parameter should be TRUE for a first
 *                              INVITE transaction, or for a non-callLeg
 *                              transaction.
 * Output:  hNewTransaction    - Pointer to the new transaction handler.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionDNSContinue(
                               IN  RvSipTranscHandle        hOrigTrans,
                               IN  RvSipTranscOwnerHandle   hOwner,
                               IN  RvBool                  bRemoveToTag,
                               OUT RvSipTranscHandle        *hCloneTrans);
#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/

/***************************************************************************
 * SipTransactionGetNewAddressHandle
 * ------------------------------------------------------------------------
 * General: Allocates a new Address on the transaction page and returns its handle.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction.
 *            eAddrType - Type of address the application wishes to create.
 * Output:     phAddr    - Handle to the newly created address header.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGetNewAddressHandle (
                                      IN   RvSipTranscHandle      hTransc,
                                      IN   RvSipAddressType       eAddrType,
                                      OUT  RvSipAddressHandle    *phAddr);

/***************************************************************************
 * SipTransactionGetNewPartyHeaderHandle
 * ------------------------------------------------------------------------
 * General: Allocates a new party header on the transaction page and
 *          returns its handle.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction.
 * Output:     phHeader    - Handle to the newly created header.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGetNewPartyHeaderHandle (
                                      IN   RvSipTranscHandle          hTransc,
                                      OUT  RvSipPartyHeaderHandle    *phHeader);

/***************************************************************************
 * SipTransactionSetReqURI
 * ------------------------------------------------------------------------
 * General: Sets the given request uri address in the transaction object.
 *          Before setting check if the given address was allocated on the
 *          transaction page, and if so do not copy it
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
  * Input:     hTransc   - Handle to the transaction.
 *            eAddrType - Type of address.
 *          phAddr    - Handle to the req-uri address.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionSetReqURI (
                                      IN  RvSipTranscHandle     hTransc,
                                      IN  RvSipAddressType      eAddrType,
                                      IN  RvSipAddressHandle    hAddr);
#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * SipTransactionConstructAndSetReferredByHeader
 * ------------------------------------------------------------------------
 * General: Allocate and Sets the given referred-by header in the transaction object.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionConstructAndSetReferredByHeader (
                                      IN  RvSipTranscHandle              hTransc,
                                      IN  RvSipReferredByHeaderHandle    hReferredBy);

#endif /*#ifndef RV_SIP_PRIMITIVES */
/***************************************************************************
 * SipTransactionGetStringMethodFromEnum
 * ------------------------------------------------------------------------
 * General: return a string with the method of the given enumeration.
 * Return Value: The method as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eMethod - the method as enum.
 ***************************************************************************/
RvChar*  RVCALLCONV SipTransactionGetStringMethodFromEnum(
                                            IN SipTransactionMethod eMethod);

/***************************************************************************
 * SipTransactionGenerateIDStr
 * ------------------------------------------------------------------------
 * General: Generate ID to be used for callID or tag.
 * Return Value: RV_OK - Call-Id was generated successfully
 *               o/w -      - failed to get local address
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransport - transportMgr handle.
 *          pseed      - pointer to random seed
 *          hObj       - pointer to the object that the ID is generated for.
 *          logId      - the logger to use for errors
 * Output:  strID      - a generated string that contains the random ID.
 *                       the string must be of size SIP_COMMON_ID_SIZE
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGenerateIDStr(IN RvSipTransportMgrHandle     hTransport,
                                         IN RvRandomGenerator*         pseed,
                                         IN void*                       hObj,
                                         IN RvLogSource*                logId,
                                         OUT RvChar*                   strID);

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * SipTransactionIs1xxLegalRel
 * ------------------------------------------------------------------------
 * General: Checks if a given 1xx message is a reliable provisional response,
 *          and if has all mandatory parameters.
 * Return Value: RV_TRUE - reliable and legal.
 *               RV_FALSE - not.
 * ------------------------------------------------------------------------
 * Input:
 *      hMsg - The reliable 1xx message.
 * Output:
 *      pRseqStep - the step in the rseq header.
 *      pbLegal1xx - is the rel 1xx message legal (contains legal rseq header)
 *                   or not.
 ***************************************************************************/
void RVCALLCONV SipTransactionIs1xxLegalRel(IN  RvSipMsgHandle hMsg,
                                            OUT RvUint32      *pRseqStep,
                                            OUT RvBool        *pbLegal1xx);

#endif /*#ifndef RV_SIP_PRIMITIVES*/

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * SipCallLegSetHeadersListToSetInInitialRequest
 * ------------------------------------------------------------------------
 * General: Sets the HeadersListToSetInInitialRequest to the call-leg,
 *          the list headers will be set to the outgoing INVITE request.
 * Return Value: RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate.
 *               RV_OK - Referred-By header was set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLeg     - Handle to the call-leg.
 *           hHeadersList - Handle to the headers list.
 ***************************************************************************/
RvStatus RVCALLCONV SipTranscSetHeadersListToSetInInitialRequest (
                            IN  RvSipTranscHandle           hTransc,
                            IN  RvSipCommonListHandle       hHeadersList);
#endif /*#ifndef RV_SIP_PRIMITIVES*/


/***************************************************************************
 * SipTransactionSetTimers
 * ------------------------------------------------------------------------
 * General: Sets timeout values for the transaction timers.
 *          If some of the fields in pTimers are not set (UNDEFINED), this 
 *          function will calculate it, or take the values from configuration.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hTransc - Handle to the transaction object.
 *           pTimers - Pointer to the struct contains all the timeout values.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionSetTimers(
                            IN  RvSipTranscHandle    hTransc,
                            IN  RvSipTimers*         pNewTimers);

/***************************************************************************
 * SipTransactionGetTrxControl
 * ------------------------------------------------------------------------
 * General: Gives the transmitter, and delete it from transaction object.
 *          Note that the trx, still has the transaction as an owner.
 *          Whoever took control on the transmitter must change it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc    - pointer to Transaction
 *            hAppTrx     - new application handle to the transmitter.
 *            pEvHanders  - new event handlers structure for this transmitter.
 *            sizeofEvHandlers - size of the event handler structure.
 ***************************************************************************/
RvStatus SipTransactionGetTrxControl(
                  IN  RvSipTranscHandle           hTransc,
                  IN  RvSipAppTransmitterHandle   hAppTrx,
                  IN  RvSipTransmitterEvHandlers* pEvHandlers,
                  IN  RvInt32                     sizeofEvHandlers,
                  OUT RvSipTransmitterHandle     *phTrx);

/***************************************************************************
 * SipTransactionGetClientResponseConnBackUp
 * ------------------------------------------------------------------------
 * General: Returns hClientResponseConnBackUp. call-leg invite object will hold
 *          this connection, after transaction termination, because 2xx
 *          retransmissions might be sent on this connection.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction.
 * Output:    phConn - Handle to the hClientResponseConnBackUp connection
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGetClientResponseConnBackUp(
                            IN  RvSipTranscHandle               hTransc,
                           OUT  RvSipTransportConnectionHandle *phConn);

/***************************************************************************
 * SipTransactionFillBasicRequestMessage
 * ------------------------------------------------------------------------
 * General: Fill a request message with basic parameters.
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *                 RV_OutOfResource - Couldn't allocate memory for the new
 *                                    message, and the associated headers.
 *                 RV_OK - The Request message was successfully created.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hRequestUri - The request Uri
 *         hFrom       - The From header
 *         hTo         - The To header.
 *         pStrCallId  - Pointer to the call-Id rpool string
 *         eMethod     - The request method.
 *         strMethod   - An other method string.
 *         pCseq       - Pointer to the cseq value.
 *         bAddMaxForwards - Add Max-Forward header to the message or not.
 * Output: phMsg      - The filled message.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionFillBasicRequestMessage(
                                        IN  RvSipAddressHandle hRequestUri,
                                        IN  RvSipPartyHeaderHandle hFrom,
                                        IN  RvSipPartyHeaderHandle hTo,
                                        IN  RPOOL_Ptr         *pStrCallId,
                                        IN  RvSipMethodType    eMethod,
                                        IN  RvChar            *strMethod,
                                        IN  SipCSeq            *pCseq,
                                        IN  RvBool             bAddMaxForwards,
                                        INOUT RvSipMsgHandle   hMsg);

#if (RV_NET_TYPE & RV_NET_SCTP)
/***************************************************************************
 * SipTransactionSetSctpMsgSendingParams
 * ------------------------------------------------------------------------
 * General: sets SCTP message sending parameters, such as stream number,
 *          into the Transaction object.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc - Handle to the Transaction object.
 *          pParams - Parameters to be get.
 * Output:  none.
 ***************************************************************************/
void RVCALLCONV SipTransactionSetSctpMsgSendingParams(
                    IN  RvSipTranscHandle                  hTransc,
                    IN  RvSipTransportSctpMsgSendingParams *pParams);

/***************************************************************************
 * SipTransactionGetSctpMsgSendingParams
 * ------------------------------------------------------------------------
 * General: gets SCTP message sending parameters, such as stream number,
 *          set for the Transaction object.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc - Handle to the Transaction object.
 * Output:  pParams - Parameters to be get.
 ***************************************************************************/
void RVCALLCONV SipTransactionGetSctpMsgSendingParams(
                    IN  RvSipTranscHandle                  hTransc,
                    OUT RvSipTransportSctpMsgSendingParams *pParams);
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * SipTransactionAuthProceed
 * ------------------------------------------------------------------------
 * General: The function order the stack to proceed authentication procedure.
 *          Actions options are:
 *          RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD
 *          - Check the given authorization header, with the given password
 *
 *          RVSIP_TRANSC_AUTH_ACTION_SUCCESS
 *          - user had checked the authorization header by himself, and it is
 *            correct (will cause AuthCompletedEv to be called, with status
 *            success)
 *
 *          RVSIP_TRANSC_AUTH_ACTION_FAILURE
 *          - user wants to stop the loop on authorization headers.
 *            (will cause AuthCompletedEv to be called, with status failure)
 *
 *          RVSIP_TRANSC_AUTH_ACTION_SKIP
 *          - order to skip the given header, and continue the loop with next
 *            header (if exists).
 *            (will cause AuthCredentialsFoundEv to be called, or
 *            AuthCompletedEv(failure) if there are no more authorization
 *            headers)
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc        - The transaction handle.
 *          action         - Which action to take (see above)
 *          hAuthorization - Handle of the header to be authenticated.
 *                           Can be NULL.
 *                           Should be provided if action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD.
 *          password       - The password for the realm+userName in the header.
 *                           Can be NULL.
 *                           Should be provided if action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD.
 *          hObject        - handle to the object to be authenticated.
 *                           Can be Subscription, CallLeg, RegClient or
 *                           Transaction itself, if it is outstanding.
 *          eObjectType    - type of the object to be authenticated.
 *          pObjTripleLock - lock of the object. Can be NULL. If it is
 *                           supplied, it will be unlocked before callbacks
 *                           to application and locked again after them.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionAuthProceed(
                    IN  RvSipTranscHandle              hTransc,
                    IN  RvSipTransactionAuthAction     action,
                    IN  RvSipAuthorizationHeaderHandle hAuthorization,
                    IN  RvChar*                        password,
                    IN  void*                          hObject,
                    IN  RvSipCommonStackObjectType     eObjectType,
                    IN  SipTripleLock*                 pObjTripleLock);

#endif /*#ifndef RV_SIP_AUTH_ON*/

/***************************************************************************
* SipTransactionGetReceivedFromAddress
* ------------------------------------------------------------------------
* General: Gets the address from which the transaction received it's last message.
* Return Value: RV_Status.
* ------------------------------------------------------------------------
* Arguments:
* Input:
* hTransaction - Handle to the transaction
* Output:
* pAddr     - basic details about the received from address
* pOptions  - Options about the the address details. If NULL, then only basic
*             data will be retrieved
***************************************************************************/
SipTransportAddress* RVCALLCONV SipTransactionGetReceivedFromAddress(
										IN  RvSipTranscHandle     hTransaction);
#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT 
/***************************************************************************
 * SipTransactionGetKeyFromTransc
 * ------------------------------------------------------------------------
 * General: Fill the Key structure with transaction/s parameters.
 * Return Value: RV_ERROR_BADPARAM - no cseq in transc.
 *                 RV_OK - All the values were updated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc - The transaction.
 * Output:  key     - A structure of the key to fill.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransactionGetKeyFromTransc(
                                    IN  RvSipTranscHandle  hTransc,
                                    OUT SipTransactionKey *pKey);
#endif /*#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * SipTransactionIsInActiveState
 * ------------------------------------------------------------------------
 * General: The function checks if a transaction is in active state.
 * Return Value: TRUE - if the transaction is active.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCallLeg - Pointer to the call-leg.
 *        hTransc  - Handle to the transaction found in the list.
 ***************************************************************************/
RvBool RVCALLCONV SipTransactionIsInActiveState(IN RvSipTranscHandle hTransc);
#endif /*#ifndef RV_SIP_PRIMITIVES*/


#endif /*#ifndef RV_SIP_LIGHT*/

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_TRANSACTION_H */

