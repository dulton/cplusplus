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
 *                              <RvSipTransaction.h>
 *
 * The Transaction functions of the RADVISION SIP stack enable you to create and
 * manage Transactions that are not related to a call-leg but to the user agent.
 * An example for such a transaction is OPTIONS.
 * Using the API you can create a transaction and send the request. You can receive
 * the response using callbacks.
 * Incoming transactions that are not related to a call-leg will be also handled
 * by the transaction layer and API. You will be informed of such transaction
 * in the callback functions.
 *
 * Transaction API functions are grouped as follows:
 * The Transaction Manager API
 * ------------------------
 * The transaction manager is in charge of all the transactions. It is used
 * to set the event handlers of the transaction module and to create
 * new transaction.
 *
 * The Transaction API
 * -----------------
 * Enables to create new transactions, send receive and respond to transactions
 * and access the Transaction object parameters.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                  Nov 2000
 *********************************************************************************/


#ifndef RVSIP_TRANSACTION_H
#define RVSIP_TRANSACTION_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "RvSipTransactionTypes.h"
#include "RvSipTransport.h"
#include "RvSipCompartmentTypes.h"
#include "RvSipTransmitterTypes.h"
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctpTypes.h"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
#ifdef RV_SIP_IMS_ON
#include "RvSipSecAgreeTypes.h"
#include "RvSipSecurityTypes.h"
#endif /* RV_SIP_IMS_ON */
 

/***************************************************************************
 * RvSipTransactionMgrSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Set event handlers for the transaction events.
 * Return Value: RV_ERROR_NULLPTR - The pointer to the handlers struct is NULL.
 *                 RV_OK - The event handlers were successfully set.
 *                 RV_ERROR_BADPARAM - Size of struct is negative,
 *                                       or owner enumeration is UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscMgr  - Handle to the transaction manager.
 *            pAppContext - An application context. This context will be supplied
 *                        with the callback informing of the creation of a
 *                        new transaction.
 *          pHandlers   - Pointer to structure containing application event
 *                        handler pointers.
 *          evHandlerStructSize - The size of the event handler structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionMgrSetEvHandlers(
                        IN RvSipTranscMgrHandle            hTranscMgr,
                        IN void                            *pAppContext,
                        IN RvSipTransactionEvHandlers      *pHandlers,
                        IN RvUint32                        evHandlerStructSize);


/***************************************************************************
 * RvSipTransactionMgrSetMgrEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets a set of event handlers the transaction manager will use.
 *          Transaction manager events do not belong to a specific transaction
 *          but to the manager itself.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscMgr - A handle to the transaction manager.
 *            pAppContext - An application context. This context will be supplied
 *                        when the manager callbacks will be called.
 *            pMgrHandlers  - A pointer to a structure of event handlers.
 *            evHandlerStructSize - The size of the pMgrHandlers structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionMgrSetMgrEvHandlers(
                        IN RvSipTranscMgrHandle            hTranscMgr,
                        IN void                            *pAppContext,
                        IN RvSipTransactionMgrEvHandlers   *pMgrHandlers,
                        IN RvUint32                        evHandlerStructSize);




/***************************************************************************
 * RvSipTransactionMgrSetProxyEvHandlers
 * ------------------------------------------------------------------------
 * General: This function is deprecated. All needed events can be found
 *          in the RvSipTransactionMgrEvHandlers structure.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr - A handle to the transaction manager.
 *          pHandlers  - Pointer to structure containing application event
 *                        handler pointers.
 *          evHandlerStructSize - The size of the event handler structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionMgrSetProxyEvHandlers(
                        IN RvSipTranscMgrHandle            hTranscMgr,
                        IN RvSipTransactionProxyEvHandlers *pHandlers,
                        IN RvUint32                        evHandlerStructSize);



/***************************************************************************
 * RvSipTranscMgrCreateTransaction
 * ------------------------------------------------------------------------
 * General: Creates a new Client transaction and exchange handles with the
 *          application.  The new transaction assumes the Initial state.
 *
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
 * Input:   hTranscMgr - Handle to the transaction manager.
 *            hTranscOwner - The owner of the transaction. This is the application
 *                         handle for this transaction. We use the term owner since
 *                         a transaction an be owned also by a call-leg or a
 *                         register client and not by the application.
 * Output:  hTransc - SIP stack handle to the newly created transaction.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscMgrCreateTransaction(
                                  IN  RvSipTranscMgrHandle     hTranscMgr,
                                  IN  RvSipTranscOwnerHandle   hTranscOwner,
                                  OUT RvSipTranscHandle        *hTransc);


/***************************************************************************
 * RvSipTranscMgrCreateServerTransactionFromMsg
 * ------------------------------------------------------------------------
 * General: Called to create a new Server transaction from a request message that
 *          the application supplies. This function creates a new transaction in the
 *          Request-Received state.
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
 * Input:   hTranscMgr - The transaction manager to which this transaction will
 *                         belong.
 *          hTranscOwner - The application owner of the transaction.
 *          hMsg - The handle to the request message that the application
 *                 supplies to create the transaction
 * Output:  hTransc - The new transaction created. If the function failed hTransc = NULL
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscMgrCreateServerTransactionFromMsg(
                                              IN  RvSipTranscMgrHandle     hTranscMgr,
                                              IN  RvSipTranscOwnerHandle   hTranscOwner,
                                              IN  RvSipMsgHandle           hMsg,
                                              OUT RvSipTranscHandle        *hTransc);


/***************************************************************************
 * RvSipTranscMgrSetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: Sets a handle to the application transc manager in the stack
 *          transaction manager. The application can use this handle to
 *          save a context in the transaction manager. This handle is given in
 *          transaction created callback.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr - The SIP stack transaction manager handle
 *            hAppMgr - The application manager handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscMgrSetAppMgrHandle(
                                  IN  RvSipTranscMgrHandle     hTranscMgr,
                                  IN  void*                    hAppMgr);

/***************************************************************************
 * RvSipTranscMgrGetAppMgrHandle
 * ------------------------------------------------------------------------
 * General: Gets the handle to the application transaction manager from the stack
 *          transaction manager.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr - The SIP stack transaction manager handle
 * Output:  hAppMgr - The application manager handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscMgrGetAppMgrHandle(
                                  IN   RvSipTranscMgrHandle     hTranscMgr,
                                  OUT  void**                   hAppMgr);

/***************************************************************************
 * RvSipTranscMgrSendOutOfContextMsg
 * ------------------------------------------------------------------------
 * General: This function is deprecated. For sending messages that are not
 *          related to a specific transaction, please use the Transmitter API.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTranscMgr              - The transaction manager handle.
 *          hMsgToSend              - Handle to the message to send.
 *          handleTopVia            - Indicate whether the transaction manager should
 *                                    add a top Via header to request messages and
 *                                    remove the top via from response messages.
 *          strViaBranch            - The Via branch to add to the top via header.
 *                                    This parameter is ignored for response messages or when
 *                                    the handleTopVia parameter is RV_FALSE.
 *          localAddress            - The local address to use when sending the message. If
 *                                    NULL is given the default local address will be used.
 *          localPort               - The local port to use when sending the message.
 *          bSendToFirstRouteHeader       - Determines weather to send a request to to a loose router
 *                                    (to the first in the route list) or to a strict router
 *                                    (to the request URI). When the message sent is a response
 *                                    this parameter is ignored.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscMgrSendOutOfContextMsg(
                                    IN RvSipTranscMgrHandle            hTranscMgr,
                                    IN RvSipMsgHandle                  hMsgToSend,
                                    IN RvBool                         handleTopVia,
                                    IN RvChar                        *strViaBranch,
                                    IN RvChar                        *localAddress,
                                    IN RvUint16                       localPort,
#if defined(UPDATED_BY_SPIRENT)
				    IN    RvChar                     *if_name,
				    IN    RvUint16                    vdevblock,
#endif
                                    IN RvBool                         bIgnoreOutboundProxy,
                                    IN RvBool                         bSendToFirstRouteHeader);


/***************************************************************************
 * RvSipTranscMgrGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this transaction
 *          manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTranscMgr      - Handle to the transaction manager.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscMgrGetStackInstance(
                                   IN   RvSipTranscMgrHandle   hTranscMgr,
                                   OUT  void**      phStackInstance);

#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT
/***************************************************************************
 * RvSipTranscMgrReprocessTransaction
 * ------------------------------------------------------------------------
 * General: Reprocess a transaction: The transaction manager will terminate 
 *          the given transaction, and will reprocess the request that was 
 *          received by it via a new transaction. 
 *          Notice: The transaction you supply here must be in state 
 *          SERVER_GEN_REQUEST_RCVD or in state SERVER_INVITE_REQUEST_RCVD, 
 *          and it must have a valid hReceivedMsg. You must have called
 *          RvSipTransactionPrepareForReprocessing() for this transaction 
 *          before hand.
 *          Notice: The TransactionTerminated() callback will notify the 
 *          termination of the old transaction. The CallLegTransactionCreated() 
 *          callback will notify the creation of the new transaction. The new 
 *          transaction will notify the processing of the request via callbacks 
 *          MsgRcvd(), StateChanged(), etc.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr - Handle to the transaction manager.
 *          hTransc    - Handle to the transaction to be reprocessed.
 *          
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscMgrReprocessTransaction(
											IN   RvSipTranscMgrHandle   hTranscMgr,
											IN   RvSipTranscHandle      hTransc);
#endif /* #ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */

/*----------------------------------------------------------------------------
        TRANSACTION CONTROL
  ----------------------------------------------------------------------------*/

#ifdef SUPPORT_EV_HANDLER_PER_OBJ
/***************************************************************************
 * RvSipTransactionSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets a set of event handlers identified with the application for a certain transaction.
 * Return Value:   RV_OK - The event handlers were successfully set otherwise error occurred.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:		hTransc    - A handle to the transaction to set the handlers to.
 *				pHandlers  - The struct of the event handlers.
 *				evHandlerStructSize - The size of the pHandlers struct.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetEvHandlers(
                        IN RvSipTranscHandle	            hTransc,
                        IN RvSipTransactionEvHandlers      *pHandlers,
                        IN RvUint32                        evHandlerStructSize);

#endif /*SUPPORT_EV_HANDLER_PER_OBJ*/


/***************************************************************************
 * RvSipTransactionRequest
 * ------------------------------------------------------------------------
 * General: Sends a request message. The method of the request and the
 *          request URI are given as parameters to this function.
 * Return Value: RV_OK - The message was created and sent successfully.
 *                 RV_ERROR_NULLPTR - The Request-Uri pointer is a NULL pointer,
 *                                 or the transaction pointer is a NULL pointer.
 *                 RV_ERROR_ILLEGAL_ACTION - The Request was called in a state where
 *                                    request can not be executed, or with
 *                                  invalid string method.
 *                 RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                              message (Couldn't send a message to the given
 *                            Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 *            strRequestMethod - The request method as a string.
 *            hRequestUri -    The Request-Uri to be used in the request
 *                           message.
 *                           The message's Request-Uri will be the address
 *                           given here, and the message will be send
 *                           accordingly.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionRequest(
                           IN RvSipTranscHandle           hTransc,
                           IN RvChar                     *strRequestMethod,
                           IN RvSipAddressHandle          hRequestUri);

/***************************************************************************
 * RvSipTransactionRespond
 * ------------------------------------------------------------------------
 * General: Sends a response message with the given response code and a
 *          reason phrase. The reason phrase is optional and can be set to
 *          NULL. In this case the response will have a default reason phrase
 *          according to the RFC.
 * Return Value: RV_OK - The response was successfully created and sent.
 *                 RV_ERROR_NULLPTR - The pointer to the transaction is a NULL
 *                                 pointer.
 *                 RV_ERROR_ILLEGAL_ACTION - The Respond was called in a state where
 *                                    respond can not be executed.
 *                 RV_ERROR_UNKNOWN - an error occurred while trying to send the
 *                              response message.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -    The transaction handle.
 *            responseCode -    The response-code to respond with.
 *            strReasonPhrase - The reason phrase to describe the response code.
 *                            If set to NULL a default reason phrase will be
 *                            supplied.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionRespond(
                                        IN RvSipTranscHandle   hTransc,
                                        IN RvUint16         responseCode,
                                        IN RvChar          *strReasonPhrase);

/***************************************************************************
 * RvSipTransactionSendPUBLISH
 * ------------------------------------------------------------------------
 * General: Sends a PUBLISH request over a given transaction.
 *          The function may use given parameters in a string format, or application 
 *          may supply NULL as the parameters struct, so the function will use 
 *          parameters that were set to the transaction in advance.
 * Return Value: RV_OK - The message was created and sent successfully.
 *               RV_ERROR_NULLPTR - The Request-Uri pointer is a NULL pointer,
 *                                  or the transaction pointer is a NULL pointer.
 *               RV_ERROR_ILLEGAL_ACTION - The Request was called in a state where
 *                                  request can not be executed, or with
 *                                  invalid string method.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                                  message (Couldn't send a message to the given
 *                                  Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc       - The transaction this request refers to.
 *            pTranscParams - Structure containing the transaction parameters
 *                        (Request-Uri, To, From, Cseq val) in a string format,
 *                        to be used in the request message.
 *                        if NULL - the function will use parameters that were 
 *                        set to the transaction in advance.
 *            sizeofCfg     - The size of the given parameters structure.
 *            hRequestUri   - In case the given parameters structure is NULL,
 *                        the application need to supply the request-uri handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSendPUBLISH(
                           IN RvSipTranscHandle              hTransc,
                           IN RvSipTransactionStrParamsCfg*  pTranscParams,
                           IN RvInt32                        sizeofCfg,
                           IN RvSipAddressHandle             hRequestUri);

/***************************************************************************
 * RvSipTransactionSendMESSAGE
 * ------------------------------------------------------------------------
 * General: Sends a MESSAGE request over a given transaction.
 *          The function may use given parameters in a string format, or application 
 *          may supply NULL as the parameters struct, so the function will use 
 *          parameters that were set to the transaction in advance.
 * Return Value: RV_OK - The message was created and sent successfully.
 *               RV_ERROR_NULLPTR - The Request-Uri pointer is a NULL pointer,
 *                                  or the transaction pointer is a NULL pointer.
 *               RV_ERROR_ILLEGAL_ACTION - The Request was called in a state where
 *                                  request can not be executed, or with
 *                                  invalid string method.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                                  message (Couldn't send a message to the given
 *                                  Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc       - The transaction this request refers to.
 *            pTranscParams - Structure containing the transaction parameters
 *                        (Request-Uri, To, From, Cseq val) in a string format,
 *                        to be used in the request message.
 *                        if NULL - the function will use parameters that were 
 *                        set to the transaction in advance.
 *            sizeofCfg     - The size of the given parameters structure.
 *            hRequestUri   - In case the given parameters structure is NULL,
 *                        the application need to supply the request-uri handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSendMESSAGE(
                           IN RvSipTranscHandle              hTransc,
                           IN RvSipTransactionStrParamsCfg*  pTranscParams,
                           IN RvInt32                        sizeofCfg,
                           IN RvSipAddressHandle             hRequestUri);

/***************************************************************************
 * RvSipTransactionSendOPTIONS
 * ------------------------------------------------------------------------
 * General: Sends a OPTIONS request over a given transaction.
 *          The function may use given parameters in a string format, or application 
 *          may supply NULL as the parameters struct, so the function will use 
 *          parameters that were set to the transaction in advance.
 * Return Value: RV_OK - The message was created and sent successfully.
 *               RV_ERROR_NULLPTR - The Request-Uri pointer is a NULL pointer,
 *                                  or the transaction pointer is a NULL pointer.
 *               RV_ERROR_ILLEGAL_ACTION - The Request was called in a state where
 *                                  request can not be executed, or with
 *                                  invalid string method.
 *               RV_ERROR_UNKNOWN - An error occurred while trying to send the
 *                                  message (Couldn't send a message to the given
 *                                  Request-Uri).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc       - The transaction this request refers to.
 *            pTranscParams - Structure containing the transaction parameters
 *                        (Request-Uri, To, From, Cseq val) in a string format,
 *                        to be used in the request message.
 *                        if NULL - the function will use parameters that were 
 *                        set to the transaction in advance.
 *            sizeofCfg     - The size of the given parameters structure.
 *            hRequestUri   - In case the given parameters structure is NULL,
 *                        the application need to supply the request-uri handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSendOPTIONS(
                           IN RvSipTranscHandle              hTransc,
                           IN RvSipTransactionStrParamsCfg*  pTranscParams,
                           IN RvInt32                        sizeofCfg,
                           IN RvSipAddressHandle             hRequestUri);
#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * RvSipTransactionRespondReliable
 * ------------------------------------------------------------------------
 * General: Creates and sends a reliable provisional response message with the given
 * response code and reason phrase. This function can be called only for
 * response codes between 101 and 199. The provisional response will contain the
 * Require=100rel and the RSeq headers.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -    The transaction handle.
 *            responseCode -    The provisional response-code (between 101 and 199).
 *            strReasonPhrase - The reason phrase that describes the response code.
 *                            If NULL is supplied a default reason phrase will be
 *                            used.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionRespondReliable(
                                        IN RvSipTranscHandle   hTransc,
                                        IN RvUint16         responseCode,
                                        IN RvChar          *strReasonPhrase);




#endif

/***************************************************************************
 * RvSipTransactionMake
 * ------------------------------------------------------------------------
 * General: Initialize a transaction and sends a request. before calling
 *          this function you should call RvSipTranscMgrCreateTransaction
 *          to create a new transaction object. A call-id will
 *          be created automatically by the transaction layer. You can
 *          use RvSipTransactionSetCallId before calling this function
 *          to specify a different Call-ID.
 * Return Value: RV_OK - The out parameter pTransc now points
 *                            to a valid transaction, initialized by the
 *                              received key.
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
 * Input:   hTransc   - The transaction handle.
 *          strFrom   - A string with the from party header for example:
 *                     "From:sip:172.20.1.1:5060"
 *          strTo     - A string with the to party header for example:
 *                     "To:sip:172.20.5.5:5060"
 *          strRequestURI - A string with the request uri address. for example:
 *                     "sip:sara@172.20.1.1:5060"
 *          cseqStep  - The CSeq step to use in this transaction.
 *          strMethod - The request method as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionMake(
                                  IN  RvSipTranscHandle       hTransc,
                                  IN  RvChar*                strFrom,
                                  IN  RvChar*                strTo,
                                  IN  RvChar*                strRequestURI,
                                  IN  RvInt32                cseqStep,
                                  IN  RvChar*                strMethod);


/***************************************************************************
 * RvSipTransactionCancel
 * ------------------------------------------------------------------------
 * General: Cancels the transaction given as a parameter.
 *          calling RvSipTransactionCancel() will cause A client CANCEL
 *          transaction to be created with transaction key
 *          taken from the transaction that is about to be canceled.
 *          The CANCEL request will then be sent to the remote party.
 *          If the transaction to cancel is an Invite transaction it will
 *          assume the Invite Canceled state. A general transaction will assume
 *          the general canceled state.
 *          The newly created CANCEL transaction will assume the Cancel sent
 *          state.
 *          You can cancel only transactions that have received a provisional
 *          response and are in the PROCEEDING or PROCEEDING_TIMEOUT state.
 *          canceling a transaction that has already received a 
 *          final response has no affect.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTransc - The transaction to cancel.
 ***************************************************************************/
RVAPI RvStatus  RVCALLCONV RvSipTransactionCancel(
                                    IN  RvSipTranscHandle   hTransc);

/***************************************************************************
 * RvSipTransactionAck
 * ------------------------------------------------------------------------
 * General: Sends an ACK request on a given Invite transaction.
 *          This function can only be called on Invite client transaction in state
 *          RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD when the
 *          response is a non 2xx response.
 *          According to RFC 3261, ACK request for an Invite transaction that 
 *          has received a 2xx response is not part of the transaction. This
 *          ACK should be sent using the transmitter API.
 *          Note: When the bOldInviteHandling configuration parameter is set
 *          to RV_TRUE, The ACK request is part of the transaction even for 2xx
 *          responses. In that case you will be able to use this function
 *          for sending the ACK request for all transaction response codes. This
 *          mode of action however is not recommended.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 *          hRequestUri -    The Request-Uri to use in the ACK message.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionAck(
                                             IN  RvSipTranscHandle     hTransc,
                                             IN  RvSipAddressHandle hRequestUri);


/***************************************************************************
 * RvSipTransactionRequestMsg
 * ------------------------------------------------------------------------
 * General: Sends a request message to the remote party using a given
 *          prepared message object. This function will normally be used by
 *          proxy implementations in order to proxy a received request to the remote
 *          party. The request will be sent according to the Request URI
 *          found in the  message object.
 *          The user is responsible for setting the correct Request-URI in the
 *          given message object and for applying the Record-Route
 *          rules if necessary.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 *            hMsg -           Handle of the request message object. This message
 *                           will be sent to the remote party.
 *          bAddTopVia -      Indicates whether the stack should add a top Via  header
 *                           to the request message before the message is sent.
 *                           You should set this parameter to RV_TRUE if you wish
 *                           the stack to handle the Via header for you. If you
 *                           have added the Via header by your self, set this
 *                           parameter to RV_FALSE.
 *                           Note: If you wish the stack to add a branch
 *                           parameter to the top Via header you should first use the
 *                           RvSipTransactionSetViaBranch() API function and set
 *                           the branch in the transaction.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionRequestMsg(
                                                   IN  RvSipTranscHandle     hTransc,
                                                   IN  RvSipMsgHandle         hMsg,
                                                   IN  RvBool               bAddTopVia);


/***************************************************************************
 * RvSipTransactionRespondMsg
 * ------------------------------------------------------------------------
 * General: Sends a response message to the remote party using a given
 *          prepared message object. This function will normally be used by
 *          proxy implementations in order to proxy a received response.
 *          The response will be sent according to the top Via header
 *          found in the  message object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 *            hMsg -           Handle of response message object.
 *          bRemoveTopVia -   Indicate whether the stack should remove the
 *                           top Via header from the message object before the
 *                           destination is resolved and the message is sent.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionRespondMsg(
                                                   IN  RvSipTranscHandle     hTransc,
                                                   IN  RvSipMsgHandle         hMsg,
                                                   IN  RvBool               bRemoveTopVia);

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * RvSipTransactionAuthBegin
 * ------------------------------------------------------------------------
 * General: Begin the server authentication process.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionAuthBegin(
                                                   IN  RvSipTranscHandle     hTransc);

/***************************************************************************
 * RvSipTransactionAuthProceed
 * ------------------------------------------------------------------------
 * General: The function order the stack to proceed in authentication procedure.
 *          actions options are:
 *          RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD
 *          - Check the given authorization header, with the given password.
 *
 *          RVSIP_TRANSC_AUTH_ACTION_SUCCESS
 *          - user had checked the authorization header by himself, and it is
 *            correct. (will cause AuthCompletedEv to be called, with status success)..
 *
 *          RVSIP_TRANSC_AUTH_ACTION_FAILURE
 *          - user wants to stop the loop that searches for authorization headers.
 *            (will cause AuthCompletedEv to be called, with status failure).
 *
 *          RVSIP_TRANSC_AUTH_ACTION_SKIP
 *          - order to skip the given header, and continue the authentication procedure
 *            with next header (if exists).
 *            (will cause AuthCredentialsFoundEv to be called, or
 *            AuthCompletedEv(failure) if there are no more authorization headers.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 *          action -         In which action to proceed. (see above)
 *          hAuthorization - Handle of the authorization header that the function
 *                           will check authentication for. (needed if action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD, else NULL.)
 *          password -       The password for the realm+userName in the header.
 *                           (needed if action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD, else NULL.)
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionAuthProceed(
                                   IN  RvSipTranscHandle              hTransc,
                                   IN  RvSipTransactionAuthAction     action,
                                   IN  RvSipAuthorizationHeaderHandle hAuthorization,
                                   IN  RvChar                        *password);

/***************************************************************************
 * RvSipTransactionRespondUnauthenticated
 * ------------------------------------------------------------------------
 * General: Sends 401/407 response.
 *          Add a copy of the given authentication header to the response message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 *          responseCode -   401 or 407
 *          headerType -     The type of the given header
 *          hHeader -        Pointer to the header to be set in the msg.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionRespondUnauthenticated(
                                   IN  RvSipTranscHandle    hTransc,
                                   IN  RvUint16            responseCode,
                                   IN  RvChar              *strReasonPhrase,
                                   IN  RvSipHeaderType      headerType,
                                   IN  void*                hHeader);

/***************************************************************************
 * RvSipTransactionRespondUnauthenticatedDigest
 * ------------------------------------------------------------------------
 * General: Sends 401/407 response.
 *          Build an authentication header with all the given parameters,
 *          and add it to the response message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 *          responseCode -   401 or 407
 *          strReasonPhrase - The reason phrase for this response code.
 *          strRealm -       mandatory.
 *          strDomain -      Optional string. may be NULL.
 *          strNonce -       Optional string. may be NULL.
 *          strOpaque -      Optional string. may be NULL.
 *          bStale -         TRUE or FALSE
 *          eAlgorithm -     Enumeration of algorithm. if RVSIP_AUTH_ALGORITHM_OTHER
 *                           the algorithm value is taken from the the next argument.
 *          strAlgorithm -   String of algorithm. this parameters will be set only if
 *                           eAlgorithm parameter is set to be RVSIP_AUTH_ALGORITHM_OTHER.
 *          eQop -           Enumeration of qop. if RVSIP_AUTH_QOP_OTHER, the qop value
 *                           will be taken from the next argument.
 *          strQop -         String of qop. this parameter will be set only if eQop
 *                           argument is set to be RVSIP_AUTH_QOP_OTHER.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionRespondUnauthenticatedDigest(
                                   IN  RvSipTranscHandle hTransc,
                                   IN  RvUint16         responseCode,
                                   IN  RvChar           *strReasonPhrase,
                                   IN  RvChar           *strRealm,
                                   IN  RvChar           *strDomain,
                                   IN  RvChar           *strNonce,
                                   IN  RvChar           *strOpaque,
                                   IN  RvBool           bStale,
                                   IN  RvSipAuthAlgorithm eAlgorithm,
                                   IN  RvChar            *strAlgorithm,
                                   IN  RvSipAuthQopOption eQop,
                                   IN  RvChar            *strQop);
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT
/***************************************************************************
 * RvSipTransactionPrepareForReprocessing
 * ------------------------------------------------------------------------
 * General: Prepares the transaction for reprocessing. 
 *          Notice: The latest possible point to call this function is at 
 *          the StateChanged() that reports a receipt of a request. 
 *          Notice: You must call this function before you call 
 *          RvSipTranscMgrReprocessTransactionEv() for this transaction. 
 *          However, you may not call RvSipTranscMgrReprocessTransactionEv() 
 *          at all even if this function was called earlier. In this case you
 *          can respond or terminate this transaction normally.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc -        The transaction handle.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionPrepareForReprocessing(
											IN  RvSipTranscHandle    hTransc);
#endif /* #ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */

/***************************************************************************
 * RvSipTransactionTerminate
 * ------------------------------------------------------------------------
 * General: Causes immediate shut-down of the transaction:
 *          Changes the transaction's state to "terminated" state.
 *          Calls Destruct on the transaction being terminated.
 *  
 *          You may not use this function when the stack is in the
 *          middle of processing one of the following callbacks:
 *          pfnEvTransactionCreated, pfnEvInternalTranscCreated,
 *          pfnEvTranscCancelled, pfnEvSupplyParams, pfnEvOpenCallLeg
 *          pfnEvSigCompMsgNotResponded, pfnEvNewConnInUse,
 *          In this case the function returns RV_ERROR_TRY_AGAIN.
 *
 * Return Value: RV_OK - The transaction was terminated successfully.
 *               RV_ERROR_NULLPTR - The pointer to the transaction is a NULL
 *                        pointer.
 *               RV_ERROR_INVALID_HANDLE - The stack object failed to be locked.
 *               RV_ERROR_TRY_AGAIN - The object can not be terminated at this
 *                        stage, because it is in the middle of a stack callback.
 *               RV_ERROR_UNKNOWN - The transaction was not found in the transaction
 *                        manager, in other words the UnManageTransaction
 *                        call has returned the RV_ERROR_NOT_FOUND return value.
 *                        In practice the Destruct function call returns
 *                        RV_ERROR_UNKNOWN (that way it is detected here).
 *                        Note: The transaction was destructed and can
 *                        not be used any more. The RV_ERROR_UNKNOWN return is
 *                        for indication only.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTransc - The transaction to terminate.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionTerminate(
                                          IN RvSipTranscHandle hTransc);


/*-----------------------------------------------------------------------
       T R A N S A C T I O N:  G E T   A N D   S E T    A P I
 ------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipTransactionGetNewMsgElementHandle
 * ------------------------------------------------------------------------
 * General:Allocates a new message element on the transaction page, and returns the new
 *         element handle.
 *         The application may use this function to allocate a message header or a message
 *         address. It should then fill the element information,
 *         and set it back to the transaction using the relevant 'set' function.
 *
 *         The function supports the following elements:
 *         To, From - you should set these headers back with 
 *                    RvSipTransactionSetToHeader() or RvSipTransactionSetFromHeader().
 *         Address of any type - you should supply the address to the 
 *                               RvSipTransactionRequest() function
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction.
 *            eHeaderType - The type of header to allocate. RVSIP_HEADERTYPE_UNDEFINED
 *                        should be supplied when allocating an address.
 *            eAddrType - The type of the address to allocate. RVSIP_ADDRTYPE_UNDEFINED
 *                        should be supplied when allocating a header.
 * Output:    phMsgElement - Handle to the newly created header or address.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetNewMsgElementHandle (
									  IN   RvSipTranscHandle    hTransc,
                                      IN   RvSipHeaderType      eHeaderType,
                                      IN   RvSipAddressType     eAddrType,
                                      OUT  void*                *phMsgElement);

/***************************************************************************
 * RvSipTransactionGetNewPartyHeaderHandle
 * ------------------------------------------------------------------------
 * General:DEPRECATED!!! use RvSipTransactionGetNewMsgElementHandle() instead.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction.
 * Output:     phParty - Handle to the newly created party header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetNewPartyHeaderHandle (
                                      IN   RvSipTranscHandle      hTransc,
                                      OUT  RvSipPartyHeaderHandle  *phParty);


/***************************************************************************
 * RvSipTransactionGetNewAddressHandle
 * ------------------------------------------------------------------------
 * General:DEPRECATED!!! use RvSipTransactionGetNewMsgElementHandle() instead.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc  - Handle to the transaction.
 *            eAddrType - The type of address the application wishes to create.
 * Output:     phAddr    - Handle to the newly created address header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetNewAddressHandle (
                                      IN   RvSipTranscHandle      hTransc,
                                      IN   RvSipAddressType        eAddrType,
                                      OUT  RvSipAddressHandle      *phAddr);

/*----------------------------------------------------------------------------
        TRANSACTION KEY VALUES
  ----------------------------------------------------------------------------*/
/***************************************************************************
 * RvSipTransactionSetCallId
 * ------------------------------------------------------------------------
 * General: Sets the transaction's Call-Id.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle
 *          strCallId - Null terminating string with the new call Id.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetCallId(
                                     IN  RvSipTranscHandle hTransc,
                                     IN  RvChar           *strCallId);


/***************************************************************************
 * RvSipTransactionGetCallId
 * ------------------------------------------------------------------------
 * General:Copies the transaction Call-Id into a given buffer.
 *         If the buffer allocated by the application is insufficient
 *         an RV_ERROR_INSUFFICIENT_BUFFER status is returned and the actualSize
 *         parameter will be set to the actual size of the Call-ID string in
 *         the transaction.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The handle to the transaction is invalid.
 *               RV_ERROR_NULLPTR - The pointer to the strCallId buffer is
 *               invalid.
 *               RV_InsuffitientBuffer - The given buffer is not large enough.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction from which to get Call-Id value.
 * Output     strCallId - the call id.
 *            pActualSize - if present, will be filled with the actual size 
 *                          of the buffer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetCallId(
                                            IN  RvSipTranscHandle hTransc,
                                            IN  RvInt32          bufSize,
                                            OUT RvChar           *strCallId,
                                            OUT RvInt32          *pActualSize);

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
int RvSipTransactionGetCct(IN  RvSipTranscHandle hTransc);
void RvSipTransactionSetCct(IN  RvSipTranscHandle hTransc,int cct);
int RvSipTransactionGetURIFlag(IN  RvSipTranscHandle hTransc);
void RvSipTransactionSetURIFlag(IN  RvSipTranscHandle hTransc,int uri_flag);
RvSipTransportAddr* RvSipTransactionGetLocalAddr(IN RvSipTranscHandle hTransc);
const RvChar* RvSipTransactionGetLocalIpStr(IN RvSipTranscHandle hTransc);
RvInt32 RvSipTransactionGetLocalPort(IN RvSipTranscHandle hTransc);
RvStatus RvSipTransactionSetLocalAddr(IN RvSipTranscHandle hTransc,RvSipTransportAddr  *localAddr);
#endif
//SPIRENT_END

/***************************************************************************
 * RvSipTransactionSendToFirstRoute
 * ------------------------------------------------------------------------
 * General: Sets the loose route parameter for sending a request. If the transaction
 *          sends the message to a loose route proxy (the next hop proxy) the bSendToFirstRouteHeader
 *          will set to RV_TRUE. If the transaction sends the message to a strict route
 *          proxy (the next hop proxy) the bSendToFirstRouteHeader will set to RV_FALSE.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The handle to the transaction is invalid.
 *               RV_ERROR_NULLPTR - The pointer to the strCallId buffer is
 *               invalid.
 *               RV_InsuffitientBuffer - The given buffer is not large enough.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc      - The transaction to set the bSendToFirstRouteHeader in.
 *          bSendToFirstRouteHeader - The bSendToFirstRouteHeader value to set in the transaction
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSendToFirstRoute(IN RvSipTranscHandle hTransc,
                                                         IN RvBool           bSendToFirstRouteHeader);


/***************************************************************************
 * RvSipTransactionSetFromHeader
 * ------------------------------------------------------------------------
 * General: Sets the transaction from header.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle
 *          hFrom        - Handle to the from header to set in the transaction
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetFromHeader(
                                          IN RvSipTranscHandle      hTransc,
                                          IN RvSipPartyHeaderHandle hFrom);


/***************************************************************************
 * RvSipTransactionGetFromHeader
 * ------------------------------------------------------------------------
 * General: Returns the from header associated with the transaction.
 *         Attempting to alter the from header after
 *         transaction left the Initial state might cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction handle is invalid.
 *               RV_OK        - From header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -  Handle to the transaction.
 * Output:     phFrom  -  Pointer to the transaction From header handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetFromHeader(
                                          IN RvSipTranscHandle        hTransc,
                                          OUT RvSipPartyHeaderHandle *phFrom);


/***************************************************************************
 * RvSipTransactionSetToHeader
 * ------------------------------------------------------------------------
 * General: Sets the transaction To header.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle
 *          hFrom        - Handle to the To header to set in the transaction
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetToHeader(
                                          IN RvSipTranscHandle      hTransc,
                                          IN RvSipPartyHeaderHandle hTo);


/***************************************************************************
 * RvSipTransactionGetToHeader
 * ------------------------------------------------------------------------
 * General: Returns the To address associated with the transaction.
 *          Attempting to alter the To address after
 *          transaction left the Initial state might cause unexpected results.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - To header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction.
 * Output:     phTo     - Pointer to the transaction To header handle.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetToHeader(
                                          IN  RvSipTranscHandle       hTransc,
                                          OUT RvSipPartyHeaderHandle  *phTo);





/***************************************************************************
 * RvSipTransactionSetCSeqStep
 * ------------------------------------------------------------------------
 * General: sets the transaction's CSeq-Step number.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle
 *          CSeqStep - The CSeq step to be set in the transaction.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetCSeqStep(
                                            IN RvSipTranscHandle hTransc,
                                            IN RvInt32          CSeqStep);

/***************************************************************************
 * RvSipTransactionGetCSeqStep
 * ------------------------------------------------------------------------
 * General: Gets the transaction's CSeq-Step number.
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to get CSeq-Step value.
 * Output:  pCSeqStep - The transaction's CSeq step number.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetCSeqStep(
                                            IN  RvSipTranscHandle hTransc,
                                            OUT RvInt32          *pCSeqStep);



/***************************************************************************
 * RvSipTransactionGetMethodStr
 * ------------------------------------------------------------------------
 * General: Returns the transaction method.
 * Return Value: RV_OK - the method was successfuly copied to the buffer.
 *               RV_InsuffitientBuffer - The given buffer is not long enough.
 *               RV_ERROR_INVALID_HANDLE - The transaction handle is invalid.
 *               RV_ERROR_NULLPTR - The pointer to the buffer is invalid.
 *               RV_ERROR_NOT_FOUND - The method is enumerated thus no string can be
 *                             found.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to get method.
 *          bufferSize - The buffer's size.
 * Output:  strMethodBuffer - The buffer to be filled with the method.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetMethodStr(
                                          IN RvSipTranscHandle  hTransc,
                                          IN RvUint32          bufferSize,
                                          OUT RvChar           *strMethodBuffer);


/***************************************************************************
 * RvSipTransactionGetResponseCode
 * ------------------------------------------------------------------------
 * General: Gets the transaction's response code. 0 if there is no relevant
 *          response code (If a response hasn't yet been sent/received).
 * Return Value: RV_ERROR_INVALID_HANDLE  - The transaction handle is invalid.
 *               RV_ERROR_NULLPTR     - Bad pointer was given by the application.
 *               RV_OK        - To header was returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction from which to get response code.
 * Output:  pResponseCode - The transaction response code.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetResponseCode(
                                         IN  RvSipTranscHandle hTransc,
                                           OUT RvUint16         *pResponseCode);

/***************************************************************************
 * RvSipTransactionSetViaBranch
 * ------------------------------------------------------------------------
 * General: Sets the top Via branch of the transaction. The transaction will add
 *          the branch to the top Via header of outgoing requests.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc    - The transaction handle.
 *          strBranch    - The Via branch as a string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetViaBranch(
                                   IN RvSipTranscHandle       hTransc,
                                   IN RvChar                 *strBranch);

/***************************************************************************
 * RvSipTransactionGetCurrentState
 * ------------------------------------------------------------------------
 * General: Gets the transaction current state
 * Return Value: RV_ERROR_INVALID_HANDLE - if the given transaction leg handle is invalid
 *               RV_ERROR_NULLPTR    - Bad pointer was given by the application.
 *               RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 * Output:     peState -  The transaction current state.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetCurrentState(
                                        IN  RvSipTranscHandle     hTransc,
                                        OUT RvSipTransactionState *peState);

/***************************************************************************
 * RvSipTransactionGetCancelTransc
 * ------------------------------------------------------------------------
 * General: Gets the handle to the transaction that cancels the transaction
 *          given as a parameter. When calling the RvSipTransactionCancel()
 *          function a new cancel transaction is created. The
 *          RvSipTransactionGetCancelTransc() returns the handle to this
 *          transaction.
 *          If the transaction given as a parameter was never cancelled NULL
 *          is returned.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 * Output:     phCancelTransc -  The cancel transaction handle.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetCancelTransc(
                                        IN  RvSipTranscHandle     hTransc,
                                        OUT RvSipTranscHandle    *phCancelTransc);


/***************************************************************************
 * RvSipTransactionGetCancelledTransc
 * ------------------------------------------------------------------------
 * General: Gets the handle to the transaction that a cancel transaction
 *          cancelled. You can call this function when cancel is received
 *          to get the handle to the cancelled transaction.
 *          If there is not cancelled transaction NULL is returned.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The cancel transaction handle.
 * Output:     phCancelTransc -  The transaction cancelled by the cancel transaction.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetCancelledTransc(
                                        IN  RvSipTranscHandle     hTransc,
                                        OUT RvSipTranscHandle    *phCancelledTransc);

/***************************************************************************
 * RvSipTransactionSetKeyFromMsg
 * ------------------------------------------------------------------------
 * General: Initialize the transaction key from a given message.
 *          The following fields will be set in the transaction:
 *          To and From (including tags), Call-Id and CSeq step.
 *          before calling this function you should call
 *            RvSipTranscMgrCreateTransaction to create a new transaction object.
 *          This function will normally be used by proxy implementations. Before a
 *          proxy server can proxy a request it should:
 *          1. Create a new client transaction object with RvSipTranscMgrCreateTransaction().
 *          2. Get the received message from the server
 *             transaction using the RvSipTransactionGetReceivedMsg() function.
 *          3. Set the transaction key using the RvSipTransactionSetKeyFromMsg() function.
 *          Then the proxy can use the initialized client transaction to proxy the
 *          request.
 *
 * Return Value:RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc - The transaction handle.
 *          hMsg    - Handle to the message from which the transaction key will be set.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionSetKeyFromMsg(
                                                      IN  RvSipTranscHandle     hTransc,
                                                      IN  RvSipMsgHandle         hMsg);

/***************************************************************************
 * RvSipTransactionSetLocalAddress
 * ------------------------------------------------------------------------
 * General: Sets the local address from which the transaction will send outgoing requests.
 * The SIP Stack can be configured to listen to many local addresses. Each local
 * address has a transport type (UDP/TCP/TLS) and an address type (IPv4/IPv6).
 * When the SIP Stack sends an outgoing request, the local address (from where
 * the request is sent) is chosen according to the characteristics of the remote
 * address. Both the local and remote addresses must have the same characteristics,
 * such as the same transport type and address type. If several configured local
 * addresses match the remote address characteristics, the first configured address
 * is taken.
 * You can use RvSipTransactionSetLocalAddress() to force the transaction to
 * choose a specific local address for a specific transport and address type. For
 * example, you can force the transaction to use the second configured UDP/
 * IPv4 local address. If the transaction sends a request to a UDP/IPv4 remote
 * address, it will use the local address that you set instead of the default first local
 * address.
 * Note: The localAddress string you provide for this function must match exactly
 * with the local address that was inserted in the configuration structure in the
 * initialization of the SIP Stack. If you configured the SIP Stack to listen to a 0.0.0.0
 * local address, you must use the same notation here.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction
 *          eTransportType - The transport type(UDP, TCP, SCTP, TLS).
 *          eAddressType - The address type(ip or ip6).
 *            localAddress - The local address to be set to this transaction.
 *          localPort - The local port to be set to this transaction.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetLocalAddress(
                            IN  RvSipTranscHandle         hTransc,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            IN  RvChar                   *localAddress,
                            IN  RvUint16                 localPort
#if defined(UPDATED_BY_SPIRENT)
                            , IN  RvChar                *if_name
			    , IN  RvUint16               vdevblock
#endif
			    );

/***************************************************************************
 * RvSipTransactionGetLocalAddress
 * ------------------------------------------------------------------------
 * General: Get the local address the Transaction will use to send outgoing
 *          requests. This is the address the user set using the
 *          RvSipCallLegSetLocalAddress function. If no address was set the
 *          function will return the default UDP address.
 *          The user can use the transport type and the address type to indicate
 *          which kind of local address he wishes to get.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction.
 *          eTransportType - The transport type(UDP, TCP, SCTP, TLS).
 *          eAddressType - The address type(ip or ip6).
 * Output:    localAddress - The local address this transaction is using.
 *          localPort - The local port this transaction is using.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetLocalAddress(
                            IN  RvSipTranscHandle         hTransc,
                            IN  RvSipTransport            eTransportType,
                            IN  RvSipTransportAddressType eAddressType,
                            OUT  RvChar                  *localAddress,
                            OUT  RvUint16                *localPort
#if defined(UPDATED_BY_SPIRENT)
                            , OUT  RvChar                *if_name
			    , OUT  RvUint16              *vdevblock
#endif
			    );

/***************************************************************************
 * RvSipTransactionGetCurrentLocalAddress
 * ------------------------------------------------------------------------
 * General: Get the local address the Transaction will use to send outgoing
 *          requests. The transaction holds 4 types of local address:
 *          UDP address on IP4, UDP address on IP6, TCP address on IP4 and TCP address
 *          on IP6. This function returns the actual address from the 4 addresses
 *          that was used or going to be used.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction.
 * Output:    eTransporType - The transport type(UDP, TCP, SCTP, TLS).
 *          eAddressType  - The address type(IP4 or IP6).
 *          localAddress - The local address this transaction is using.
 *          localPort - The local port this transaction is using.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetCurrentLocalAddress(
                            IN  RvSipTranscHandle         hTransc,
                            OUT RvSipTransport            *eTransportType,
                            OUT RvSipTransportAddressType *eAddressType,
                            OUT RvChar                   *localAddress,
                            OUT RvUint16                 *localPort
#if defined(UPDATED_BY_SPIRENT)
                            , OUT  RvChar                *if_name
			    , OUT  RvUint16              *vdevblock
#endif
			    );
#if defined(UPDATED_BY_SPIRENT)

RvSipTransportLocalAddrHandle RvSipTransactionGetCurrentLocalAddressHandle(IN  RvSipTranscHandle         hTransc);

#endif

/***************************************************************************
 * RvSipTransactionGetCurrentDestAddress
 * ------------------------------------------------------------------------
 * General: Gets the destination address the Transaction will use to send the
 *          next outgoing message. This address is available only in the
 *          context of the RvSipTransactionFinalDestResolvedEv callback.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction.
 * Output:    peTransportType - The transport type(UDP, TCP, SCTP, TLS).
 *            peAddressType  - The address type(IP4 or IP6).
 *            strDestAddress - The destination ip address. (must be of RVSIP_TRANSPORT_LEN_STRING_IP size)
 *            pDestPort - The destination port.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetCurrentDestAddress(
                            IN  RvSipTranscHandle         hTransc,
                            OUT RvSipTransport            *peTransportType,
                            OUT RvSipTransportAddressType *peAddressType,
                            OUT RvChar                   *strDestAddress,
                            OUT RvUint16                 *pDestPort);

/***************************************************************************
 * RvSipTransactionSetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Sets all outbound proxy details to the transmitter object.
 *          All details are supplied in the RvSipTransportOutboundProxyCfg structure that
 *           includes parameters such as the IP address or host name, transport, port and
 *           compression type. Requests sent by this object will use the outbound detail
 *           specifications as a remote address. The Request-URI will be ignored. However,
 *           the outbound proxy will be ignored if the message contains a Route header or if
 *           the RvSipTransactionIgnoreOutboundProxy() function was called.
 *
 *          Note: If you specify both the IP address and a host name in the configuration
 *          structure, either of them will be set BUT the IP address will be used.
 *          If you do not specify port or transport, both will be determined according
 *          to the DNS procedures specified in RFC 3263.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc        - Handle to the transaction
 *            pOutboundCfg   - Pointer to the outbound proxy configuration
 *                             structure with all relevant details.
 *            sizeOfCfg      - The size of the outbound proxy configuration structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetOutboundDetails(
                            IN  RvSipTranscHandle               hTransc,
                            IN  RvSipTransportOutboundProxyCfg *pOutboundCfg,
                            IN  RvInt32                         sizeOfCfg);

/***************************************************************************
 * RvSipTransactionGetOutboundDetails
 * ------------------------------------------------------------------------
 * General: Gets all the outbound proxy details that the transaction object uses.
 *          The details are placed in the RvSipTransportOutboundProxyCfg structure that
 *          includes parameters such as the IP address or host name, transport, port and
 *          compression type.
 *          If the outbound details were not set to the specific transaction object but the
 *          outbound proxy was defined to the SIP Stack on initialization, the SIP Stack
 *          parameters will be returned. If the transaction is not using an outbound address,
 *          NULL/UNDEFINED values are returned.
 *          Note You must supply a valid consecutive buffer in the
 *          RvSipTransportOutboundProxyCfg structure to get the outbound strings (host
 *          name and IP address).
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc      - Handle to the transaction
 *            sizeOfCfg      - The size of the configuration structure.
 * Output:  pOutboundCfg   - Pointer to outbound proxy configuration
 *                           structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetOutboundDetails(
                            IN  RvSipTranscHandle               hTransc,
                            IN  RvInt32                         sizeOfCfg,
                            OUT RvSipTransportOutboundProxyCfg *pOutboundCfg);

/***************************************************************************
 * RvSipTransactionSetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Sets the outbound address the transaction will use as the request
 *          destination address. A requests sent
 *          by this transaction will be sent according to the transaction
 *          outbound address and not according to the Request-URI.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc         - Handle to the transaction
 *            strOutboundAddrIp    - The outbound ip to be set to this
 *                                 transaction.
 *          outboundPort         - The outbound port to be set to this
 *                                 transaction.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetOutboundAddress(
                            IN  RvSipTranscHandle           hTransc,
                            IN  RvChar                     *strOutboundAddrIp,
                            IN  RvUint16                   outboundPort);

/***************************************************************************
 * RvSipTransactionSetOutboundTransport
 * ------------------------------------------------------------------------
 * General: Sets the outbound transport of the transaction outbound proxy.
 * This transport will be used for the outbound proxy that you set using the
 * RvSipTransactionSetOutboundAddress() function or the
 * RvSipTransactionSetOutboundHostName() function. If you do not set an
 * outbound transport, the transport will be determined using the DNS procedures.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc           - Handle to the transaction
 *          eOutboundTransport - The outbound transport to be set
 *                               to this transaction.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetOutboundTransport(
                            IN  RvSipTranscHandle     hTransc,
                            IN  RvSipTransport         eOutboundTransport);

/***************************************************************************
 * RvSipTransactionGetOutboundAddress
 * ------------------------------------------------------------------------
 * General: Get the outbound address the transaction is using. If an
 *          outbound address was set to the transaction it is returned. If
 *          no outbound address was set to the transaction, but an outbound
 *          proxy was configured for the stack the configured address is
 *          returned. '\0' is returned no address was defined for
 *          the transaction or the stack.
 *          NOTE: you must supply a valid consecutive buffer of size 20 for the
 *          strOutboundIp parameter.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTransc      - Handle to the transaction
 * Output:
 *            strOutboundIp   - A buffer with the outbound IP address the transaction.
 *                               is using
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetOutboundAddress(
                            IN   RvSipTranscHandle    hTransc,
                            OUT  RvChar             *strOutboundIp,
                            OUT  RvUint16           *pOutboundPort);

/***************************************************************************
 * RvSipTransactionGetOutboundTransport
 * ------------------------------------------------------------------------
 * General: Gets the transport of the outbound proxy that the transaction is using.
 * If an outbound transport was set to the transaction, this transport will be
 * returned. If no outbound transport was set to the transaction but an outbound
 * proxy was configured for the SIP Stack, the configured transport is returned.
 * RVSIP_TRANSPORT_UNDEFINED is returned if the transaction is not
 * using an outbound proxy.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTransc           - Handle to the transaction
 * Output: eOutboundTransport - The outbound proxy transport the transaction is using.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetOutboundTransport(
                            IN   RvSipTranscHandle    hTransc,
                            OUT  RvSipTransport      *eOutboundProxyTransport);



/***************************************************************************
 * RvSipTransactionSetOutboundHostName
 * ------------------------------------------------------------------------
 * General: Sets the outbound proxy host name of the transaction object.
 * The outbound host name will be resolved each time a request is sent to this host.
 * the request will be sent to the resolved IP address.
 * Note: To set a specific IP address, use RvSipTransactionSetOutboundAddress().
 * If you configure a transaction with both an outbound IP address and an
 * outbound host name, the transaction will ignore the outbound host name and
 * will use the outbound IP address.
 * When using an outbound host all outgoing requests will be sent directly to the transaction
 * outbound proxy host unless the application specifically ordered to ignore the outbound host.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc           - Handle to the transaction
 *            strOutboundHostName    - The outbound proxy host to be set to this
 *                               transaction.
 *          outboundPort  - The outbound proxy port to be set to this
 *                               transaction. If you set the port to zero it
 *                               will be determined using the DNS procedure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetOutboundHostName(
                            IN  RvSipTranscHandle     hTransc,
                            IN  RvChar                *strOutboundHost,
                            IN  RvUint16              outboundPort);

/***************************************************************************
 * RvSipTransactionGetOutboundHostName
 * ------------------------------------------------------------------------
 * General: Gets the host name of the outbound proxy that the transaction is using.
 * If an outbound host was set to the transaction this host will be returned. If no
 * outbound host was set to the transaction but an outbound host was configured
 * for the SIP Stack, the configured host is returned. If the transaction is not
 * using an outbound host, '\0' is returned.
 * Note: You must supply a valid consecutive buffer to get the strOutboundHost host
 *       name.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTransc      - Handle to the transaction
 * Output:
 *            strOutboundHost   -  A buffer with the outbound host name
 *          pOutboundPort - The outbound host port.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetOutboundHostName(
                            IN   RvSipTranscHandle    hTransc,
                            OUT  RvChar              *strOutboundHostName,
                            OUT  RvUint16            *pOutboundPort);


/***************************************************************************
 * RvSipTransactionSetPersistency
 * ------------------------------------------------------------------------
 * General: Changes the transaction persistency definition at runtime.
 * This function receives a Boolean value that indicates whether or not the
 * application wishes this transaction to be persistent. A persistent
 * transaction will try to locate a suitable connection in the connection hash before
 * opening a new connection. A connection that was constructed by a persistent
 * transaction will automatically inserted to the connection hash.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle
 *          bIsPersistent - Determines the transaction persistency definition.
 *                          RV_TRUE indicates that the transaction is persistent.
 *                          Otherwise, RV_FALSE.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetPersistency(
                           IN RvSipTranscHandle       hTransc,
                           IN RvBool                 bIsPersistent);


/***************************************************************************
 * RvSipTransactionGetPersistency
 * ------------------------------------------------------------------------
 * General: Returns the transaction persistency definition.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Input:    hTransc - The transaction handle
 * Output:  pbIsPersistent - The transaction persistency definition.
 *                           RV_TRUE indicates that the transaction is persistent. Otherwise,
 *                           RV_FALSE.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetPersistency(
        IN  RvSipTranscHandle                   hTransc,
        OUT RvBool                            *pbIsPersistent);



/***************************************************************************
 * RvSipTransactionSetConnection
 * ------------------------------------------------------------------------
 * General: Sets a connection object to be used by the transaction.
 * The supplied connection will be used only if it fits the local and remote
 * addresses of the transaction. Otherwise, the
 * transaction will either locate a suitable connection in the connection hash or
 * create a new connection.
 * Note: this function can be used only if the transaction is persistent.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction
 *          hConn - Handle to the connection.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetConnection(
                   IN  RvSipTranscHandle                hTransc,
                   IN  RvSipTransportConnectionHandle   hConn);

/***************************************************************************
 * RvSipTransactionGetConnection
 * ------------------------------------------------------------------------
 * General: Returns the connection that the transaction is currently using.
 * Note: Only persistent transactions keep the currently-used connection. NULL
 * will be returned if the transaction is not persistent.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - Handle to the transaction.
 * Output:    phConn - Handle to the currently used connection
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetConnection(
                            IN  RvSipTranscHandle               hTransc,
                           OUT  RvSipTransportConnectionHandle *phConn);

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * RvSipTransactionGet100RelStatus
 * ------------------------------------------------------------------------
 * General: Return the 100rel option tag status of a received Invite request.
 *          A transaction can get a request message with a Supported=100rel
 *          or Require=10rel headers. These headers indicate whether a
 *          a reliable provisional response should be sent.
 *          The function can be used only for server invite transactions.
 *          For any other transaction RVSIP_TRANSC_100_REL_UNDEFINED will be
 *          returned.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -    The transaction handle.
 * Output:  relStatus - The reliable status received in the INVITE request.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGet100RelStatus(
                                    IN RvSipTranscHandle              hTransc,
                                    OUT RvSipTransaction100RelStatus  *relStatus);

/***************************************************************************
 * RvSipTransactionIsUnsupportedExtRequired
 * ------------------------------------------------------------------------
 * General: Returns whether the transaction require an extensions that is
 *          not supported. You can call this function in states that indicates
 *          that a request was receives.
 *          You can use this function only if the rejectUnsupportedExtensions
 *          configuration parameter is set to RV_TRUE.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle.
 * Output:  bUnsupportedReq - RV_TRUE if the request requires unsupported
 *                            extension. RV_FALSE otherwise.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionIsUnsupportedExtRequired(
                                 IN  RvSipTranscHandle hTransc,
                                 OUT RvBool           *bUnsupportedReq);


#endif /*#ifndef RV_SIP_PRIMITIVES*/


/***************************************************************************
 * RvSipTransactionIgnoreOutboundProxy
 * ------------------------------------------------------------------------
 * General:  Indicate the transaction to ignore its outbound proxy when sending
 *           requests.
 *           In some cases you will want the transaction to ignore its outbound
 *           proxy event if it is configured to use one.
 *           An example is when the request uri was calculated from a Route
 *           header that was found in the message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle.
 *          bIgnoreOutboundProxy - RV_TRUE if you wish the transaction to ignored
 *          its configured outbound proxy, RV_FALSE otherwise.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionIgnoreOutboundProxy(
                                 IN  RvSipTranscHandle hTransc,
                                 IN  RvBool           bIgnoreOutboundProxy);

/***************************************************************************
 * RvSipTransactionSetOwner
 * ------------------------------------------------------------------------
 * General: Sets the owner of a given transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc      - The transaction handle.
 *          hOwnerHandle - The transaction owner handle
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionSetOwner(
                                        IN  RvSipTranscHandle      hTransc,
                                        IN  RvSipTranscOwnerHandle hOwnerHandle);

/***************************************************************************
 * RvSipTransactionSetIsProxyFlag
 * ------------------------------------------------------------------------
 * General: Sets the isProxy flag of the transaction.
 *          This flag defines if the transaction should behave as a proxy
 *          transaction.
 *          The transaction isProxy flag is set when the transaction is created
 *          according to the stack configuration. (If the stack is configured
 *          as a proxy, the isProxy flag will be set to RV_TRUE automatically.
 *          For server transaction you must not call this function from the
 *          transcCreated callback. (You can use the stateChanged callback).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc  - The transaction handle.
 *          bIsProxy - The value to set in the is proxy flag.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionSetIsProxyFlag(
                                        IN  RvSipTranscHandle   hTransc,
                                        IN  RvBool              bIsProxy);


/***************************************************************************
 * RvSipTransactionGetOwner
 * ------------------------------------------------------------------------
 * General: returns the handle to the owner of a given transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The cancel transaction handle.
 * Output:     phCancelTransc -  The transaction cancelled by the cancel transaction.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetOwner(
                                        IN  RvSipTranscHandle      hTransc,
                                        OUT RvSipTranscOwnerHandle *phOwnerHandle);

/***************************************************************************
 * RvSipTransactionIsStackOwner
 * ------------------------------------------------------------------------
 * General: Indicates if the owner of the transaction is one of the SIP stack
 *          objects or not.
 *          This is an internal function, written for the SipServer.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 * Output:    pbStackOwner -  RV_TRUE if it is a stack owner.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionIsStackOwner(
                                        IN  RvSipTranscHandle      hTransc,
                                        OUT RvBool*                pbStackOwner);

/***************************************************************************
 * RvSipTransactionSetAppHandle
 * ------------------------------------------------------------------------
 * General: Sets the application handle to the given transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 *          hAppHandle -  The application handle
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionSetAppHandle(
                                        IN  RvSipTranscHandle      hTransc,
                                        IN RvSipAppTranscHandle    hAppHandle);

/***************************************************************************
 * RvSipTransactionGetAppHandle
 * ------------------------------------------------------------------------
 * General: returns the application handle to the given transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 * Output:     phAppHandle -  The application handle
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetAppHandle(
                                        IN  RvSipTranscHandle      hTransc,
                                        OUT RvSipAppTranscHandle *phAppHandle);


/***************************************************************************
 * RvSipTransactionGetReceivedMsg
 * ------------------------------------------------------------------------
 * General: Gets the message that was received by the transaction. You can
 *          call this function from the state changed call back function
 *          when the new state indicates that a message was received.
 *          If there is no valid received message, NULL will be returned.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 * Output:     phMsg -  pointer to the received message.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetReceivedMsg(
                                                      IN  RvSipTranscHandle     hTransc,
                                                      OUT RvSipMsgHandle       *phMsg);


/***************************************************************************
 * RvSipTransactionGetOutboundMsg
 * ------------------------------------------------------------------------
 * General: Gets the message that is going to be sent by the transaction.
 * You can call this function before calling an API function that causes a message to be sent
 * such as RvSipTransactionRequest(), RvSipTransactionRespond() and RvSipTransactionAck() and in one of
 * the following states:
 * RVSIP_TRANSC_STATE_IDLE,
 * RVSIP_TRANSC_STATE_SERVER_INVITE_REQUEST_RCVD,
 * RVSIP_TRANSC_STATE_CLIENT_INVITE_FINAL_RESPONSE_RCVD,
 * RVSIP_TRANSC_STATE_SERVER_INVITE_REL_PROV_RESPONSE_SENT,
 * RVSIP_TRANSC_STATE_SERVER_GEN_REQUEST_RCVD,
 * RVSIP_TRANSC_STATE_SERVER_CANCEL_REQUEST_RCVD.
 * The function returns an empty message object. You can fill the message object with headers and
 * body and then call the API. The transaction will add more headers to the same message object and
 * the message will be sent.
 * Note 1: You must not use this function to add headers that are part of the transaction
 * key such as To, From, Call-Id and Via. Doing so will cause an unexpected behavior of the transaction.
 * Note 2: If your application is multi-threaded you must lock your application object before calling
 * the RvSipTransactionGetOutboundMsg() function and unlock it only after calling the API that sends the
 * message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - The transaction handle.
 * Output:     phMsg   -  pointer to the message.
 ***************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionGetOutboundMsg(
                                     IN  RvSipTranscHandle     hTransc,
                                     OUT RvSipMsgHandle       *phMsg);


/***************************************************************************
 * RvSipTransactionGetRequestUri
 * ------------------------------------------------------------------------
 * General: Return the Request URI the transaction is using.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc    - The transaction handle.
 * Output:  hReqUri    - The Request URI.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetRequestUri(
                                   IN  RvSipTranscHandle     hTransc,
                                   OUT RvSipAddressHandle    *hReqUri);


/***************************************************************************
 * RvSipTransactionDNSContinue
 * ------------------------------------------------------------------------
 * General: Creates new transaction and copies all parameters from the
 *          original transaction to the new transaction.
 *          This functions also terminates the original transaction unless
 *          the transaction received 503 response on INVITE.
 *          If the function failed the original transaction will not be
 *          terminated.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hOrigTransaction    - The source transaction handle.
 *          owner               - pointer to the new transaction owner.
 * Output:  hNewTransaction     - Pointer to the new transaction handler.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionDNSContinue(IN  RvSipTranscHandle        hOrigTransaction,
                                                       IN  RvSipTranscOwnerHandle    hOwner,
                                                       OUT RvSipTranscHandle        *hNewTransaction);


/***************************************************************************
 * RvSipTransactionDNSGetList
 * ------------------------------------------------------------------------
 * General: retrieves the DNS list object from the transaction object.
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc    - Transaction Handle
 * Output   phDnsList        - DNS list handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionDNSGetList(IN  RvSipTranscHandle               hTransc,
                                                OUT RvSipTransportDNSListHandle *phDnsList);



/***************************************************************************
 * RvSipTransactionIsUAC
 * ------------------------------------------------------------------------
 * General: Return RV_TRUE if the transaction is a client transaction, and RV_FALSE
 *          if the transaction is a server transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc    - The transaction handle.
 * Output:  bIsUAC     - The type of the transaction (RV_TRUE if the transaction is
 *                       a client transaction, and RV_FALSE if the transaction is a
 *                       server transaction
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionIsUAC(IN  RvSipTranscHandle hTransc,
                                                 OUT RvBool          *bIsUAC);


/***************************************************************************
 * RvSipTransactionDetachOwner
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
RVAPI RvStatus RVCALLCONV RvSipTransactionDetachOwner(
                                             IN RvSipTranscHandle hTransc);

/***************************************************************************
 * RvSipTransactionSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the transaction
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hTransc - Handle to the transaction object.
 *           eStringName   - The string the user wish to set
 *         pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetRpoolString(
                              IN  RvSipTranscHandle     hTransc,
                              IN  RvSipTransactionStringName  eStringName,
                              IN  RPOOL_Ptr                 *pRpoolPtr);


/***************************************************************************
 * RvSipTranscMgrSendOutOfContextMsgExt
 * ------------------------------------------------------------------------
 * General: This function is deprecated. For sending messages that are not
 *          related to a specific transaction, please use the Transmitter API.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTranscMgr           - The transaction manager handle.
 *          hMsgToSend           - Handle to the message to send.
 *          handleTopVia         - Indicate whether the transaction manager should
 *                                 add a top Via header to request messages and
 *                                 remove the top via from response messages.
 *          strViaBranch         - The Via branch to add to the top via header.
 *                                 This parameter is ignored for response messages or when
 *                                 the handleTopVia parameter is RV_FALSE.
 *          localAddress         - The local address to use when sending the message. If
 *                                 NULL is given the default local address will be used.
 *          localPort            - The local port to use when sending the message.
 *          bIgnoreOutboundProxy - Indicate the proxy whether to ignore an
 *                                 outbound proxy.Set this parameter to RV_TRUE only
 *                                 If a Route Header was used as a request uri.
 *          bSendToFirstRouteHeader    - Determines weather to send a request to to a loose router
 *                                 (to the first in the route list) or to a strict router
 *                                 (to the request URI). When the message sent is a response
 *                                 this parameter is ignored.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscMgrSendOutOfContextMsgExt(
                                    IN RvSipTranscMgrHandle            hTranscMgr,
                                    IN RvSipMsgHandle                  hMsgToSend,
                                    IN RvBool                         handleTopVia,
                                    IN RPOOL_Ptr                      *pViaBranch,
                                    IN RvChar                        *localAddress,
                                    IN RvUint16                       localPort,
#if defined(UPDATED_BY_SPIRENT)
				    IN    RvChar                     *if_name,
				    IN    RvUint16                    vdevblock,
#endif
                                    IN RvBool                         bIgnoreOutboundProxy,
                                    IN RvBool                         bSendToFirstRouteHeader);


/***************************************************************************
 * RvSipTranscGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this transaction
 *          belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc         - Handle to the transaction.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                            handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTranscGetStackInstance(
                                   IN   RvSipTranscHandle      hTransc,
                                   OUT  void*      *phStackInstance);


/***************************************************************************
 * RvSipTransactionSetCompartment
 * ------------------------------------------------------------------------
 * General: Associates the transaction to a SigComp compartment. The transaction will use
 *          this compartment in the message compression process.
 *          Note: The message will be compressed only if the remote URI includes the
 *          comp=sigcomp parameter.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc      - The transaction handle.
 *            hCompartment - The handle to the SigComp compartment.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetCompartment(
                            IN RvSipTranscHandle      hTransc,
                            IN RvSipCompartmentHandle hCompartment);

/***************************************************************************
 * RvSipTransactionGetCompartment
 * ------------------------------------------------------------------------
 * General: Retrieves the SigComp compartment associated with the transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTransc       - The transaction handle.
 *         phCompartment - The handle to the SigComp compartment.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetCompartment(
                            IN  RvSipTranscHandle       hTransc,
                            OUT RvSipCompartmentHandle *phCompartment);

/***************************************************************************
 * RvSipTransactionDisableCompression
 * ------------------------------------------------------------------------
 * General: Disables the compression mechanism in a transaction. This means
 *          that even if the message indicates compression, it will not be
 *          compressed.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTransc       - The transaction handle.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionDisableCompression(
                                IN  RvSipTranscHandle hTransc);

/***************************************************************************
 * RvSipTransactionGetOutboundMsgCompType
 * ------------------------------------------------------------------------
 * General: Retrieves the compression type of the Transaction outbound
 *          message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - The transaction handle.
 * Output:    peCompType - The compression type.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetOutboundMsgCompType(
                                         IN  RvSipTranscHandle  hTransc,
                                         OUT RvSipCompType     *peCompType);

/***************************************************************************
 * RvSipTransactionIsCompartmentRequired
 * ------------------------------------------------------------------------
 * General: Checks if the transaction has to be related to a compartment.
 *          The function should be used by applications that would
 *          like to manage the stack compartments manually. For instance,
 *          if you wish to relate an incoming transaction object, which handles
 *          a sigComp message, to a specific compartment, you can call this
 *          function in the RvSipTransactionCreatedEv() callback and attach
 *          your chosen compartment to the transaction is needed.
 *          When the callback returns the stack will automatically attach
 *          the transaction object to compartment if it does not have one yet.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc     - The transaction handle.
 * Output:    pbRequired  - Indication if a compartment is required.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionIsCompartmentRequired(
                                         IN  RvSipTranscHandle  hTransc,
                                         OUT RvBool            *pbRequired);

/***************************************************************************
* RvSipTransactionGetReceivedFromAddress
* ------------------------------------------------------------------------
* General: Get the address from which the transaction received it's last message from
* Return Value: RvStatus.
* ------------------------------------------------------------------------
* Arguments:
* Input:
* hTransaction - Handle to the transaction
* Output:
* pAddr     - basic details about the received from address
* pOptions  - Options about the the address details. If NULL, then only basic
*             data will be retrieved
***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetReceivedFromAddress(
                            IN RvSipTranscHandle hTransaction,
                            OUT RvSipTransportAddr *pAddr,
                            OUT RvSipTransportAddrOptions *pOptions);

/***************************************************************************
 * RvSipTransactionGetTransmitter
 * ------------------------------------------------------------------------
 * General: Returns the transmitter object that the transaction is using.
 *          The application can use the transmitter object to hold and
 *          resume message sending.
 *          After address resolution is completed, the transaction calls
 *          the RvSipTransactionFinalDestResolvedEv() callback.
 *          At this stage the application can get the transmitter object from
 *          the transaction and call the RvSipTransmitterHoldSending()
 *          function. As a result, the message will not be sent to the
 *          remote party. The application can then manipulate the DNS list
 *          and the destination address. It is the application responsibility
 *          to call the RvSipTransmitterResumeSending() function so that
 *          the message will be sent to the updated destination.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransc - The transaction handle
 * Output:   phTrx - The transmitter handle
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetTransmitter(
                                          IN RvSipTranscHandle        hTransc,
                                          OUT RvSipTransmitterHandle *phTrx);

/***************************************************************************
 * RvSipTransactionSetTimers
 * ------------------------------------------------------------------------
 * General: Sets new timer values to the transaction.
 *          According to RFC3261, the transaction has to set different timers 
 *          during its life cycle and perform different actions when the timers 
 *          expire. For example, after sending a final response, the transaction 
 *          has to set a timer to the value of 32,000 MSec. 
 *          When this timer expires the transaction must terminate.
 *          The values of the transaction timers are taken from
 *			the stack configuration and are defined upon initialization.
 *			The application can use the RvSipTransactionSetTimers() API function 
 *          to change the different timer values of the transaction.
 *			The RvSipTimers structure received by this function contains all the 
 *          transaction configurable timers. 
 *          You can set values to the timers you want to configure. You can set
 *          UNDEFINED to timers you wish the stack to calculate.
 *          If you set a timer value to UNDEFINED, and this timer cannot be
 *          calculated from other timers in the structure, the timer value will
 *          be taken from the stack configuration.
 *          The RvSipTimers structure also enables you to change the number
 *          of retransmissions performed by the transaction. The default number
 *          of retransmissions is calculated using the different transaction
 *          timers. However, the application can set a different number and change
 *          the retransmission count.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hTransc - Handle to the transaction object.
 *           pTimers - Pointer to the struct contains all the timeout values.
 *           sizeOfTimersStruct - Size of the RvSipTimers structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetTimers(
                              IN  RvSipTranscHandle     hTransc,
                              IN  RvSipTimers           *pTimers,
                              IN  RvInt32               sizeOfTimersStruct);

/***************************************************************************
 * RvSipTransactionSetRejectStatusCodeOnCreation
 * ------------------------------------------------------------------------
 * General: This function can be used synchronously from the 
 *          RvSipTransactionCreatedEv when the application decides not to handle
 *          the transaction. A transaction that is not handled by the application
 *          is handled automatically by the stack and the stack rejects the request
 *          that created the transaction with a default response code.   
 *          The application can change the default response code by calling the
 *          function RvSipTransactionSetRejectStatusCodeOnCreation() and suppling
 *          a new response code.
 *          Note that after the application decides not to handle the transaction,
 *          it will not get any further callbacks
 *          that relate to this transaction. The application will not get the
 *          RvSipTransactionMsgToSendEv for the reject response message or the 
 *          Terminated state for the transaction object.
 *          Note: When this function is used to reject a request, the application
 *          cannot use the outbound message mechanism to add information
 *          to the outgoing response message. If you wish to change the response
 *          message you must use the regular respond mechanism.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hTransc - Handle to the transaction object.
 *           rejectStatusCode - The reject status code for rejecting the request
 *                              that created this object. The value must be
 *                              between 300 to 699.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetRejectStatusCodeOnCreation(
                              IN  RvSipTranscHandle      hTransc,
                              IN  RvUint16               rejectStatusCode);

/***************************************************************************
 * RvSipTransactionSetForceOutboundAddrFlag 
 * ------------------------------------------------------------------------
 * General: Sets the force outbound addr flag. This flag forces the transaction
 *          to send request messages to the outbound address regardless of the
 *          message content.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: 	hTransc           - The transaction handle.
 *     	    bForceOutboundAddr - The flag value to set.
 **************************************************************************/
RVAPI  RvStatus RVCALLCONV RvSipTransactionSetForceOutboundAddrFlag(
                                IN  RvSipTranscHandle   hTransc,
			                    IN  RvBool              bForceOutboundAddr);

#if (RV_NET_TYPE & RV_NET_SCTP)
/******************************************************************************
 * RvSipTransactionSetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Sets SCTP transport parameters to the transaction object.
 *          All possible SCTP parameters are located in the 
 *          RvSipTransportSctpMsgSendingParams structure and will be used 
 *          only if the transaction will use SCTP as the destination transport.
 *          Note: This function is optional. If specific SCTP parameters are
 *          not set, the default SCTP parameters will be used.
 *          Note: Calling this function does not affect the chosen transport.
 *          The transport will be chosen in the address resolution process.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hTransc    - Handle to the Transaction object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams    - Pointer to the structure that contains all SCTP parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetSctpMsgSendingParams(
                    IN  RvSipTranscHandle                  hTransc,
                    IN  RvInt32                            sizeOfParamsStruct,
                    IN  RvSipTransportSctpMsgSendingParams *pParams);

/******************************************************************************
 * RvSipTransactionGetSctpMsgSendingParams
 * ----------------------------------------------------------------------------
 * General: Gets SCTP parameters that are used by the transaction
 *          while sending a message over SCTP.
 *          Note: Only SCTP parameters that were set with the 
 *          RvSipTransactionSetSctpMsgSendingParams() will be returned when
 *          calling this function.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------
 * Arguments:
 *    Input: hTransc    - Handle to the Transaction object.
 *           sizeOfStruct - Size of the RvSipTransportSctpMsgSendingParams
 *                      structure.
 *           pParams      - Pointer to the struct contains all the parameters.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetSctpMsgSendingParams(
                    IN  RvSipTranscHandle                  hTransc,
                    IN  RvInt32                            sizeOfParamsStruct,
                    OUT RvSipTransportSctpMsgSendingParams *pParams);
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipTransactionSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Transaction.
 *          As a result of this operation, all messages, sent by this
 *          Transaction, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hTransc - Handle to the Transaction object.
 *          hSecObj - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetSecObj(
                                        IN  RvSipTranscHandle    hTransc,
                                        IN  RvSipSecObjHandle    hSecObj);
#endif /* RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipTransactionGetSecObj
 * ----------------------------------------------------------------------------
 * General: Gets Security Object that was set into the Transaction.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h o rrverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  hTransc  - Handle to the Transaction object.
 *  Output: phSecObj - Handle to the Security Object.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetSecObj(
                                        IN  RvSipTranscHandle    hTransc,
                                        OUT RvSipSecObjHandle*   phSecObj);
#endif /* RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * RvSipTransactionSetSecAgree
 * ------------------------------------------------------------------------
 * General: Sets a security-agreement object to this transaction. If this 
 *          security-agreement object maintains an existing agreement with the 
 *          remote party, the transaction will exploit this agreement and the data 
 *          it brings. If not, the transaction will use this security-agreement 
 *          object to negotiate an agreement with the remote party.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransaction   - Handle to the transaction.
 *          hSecAgree      - Handle to the security-agreement.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionSetSecAgree(
                    IN  RvSipTranscHandle           hTransaction,
                    IN  RvSipSecAgreeHandle         hSecAgree);

/***************************************************************************
 * RvSipTransactionGetSecAgree
 * ------------------------------------------------------------------------
 * General: Gets the security-agreement object associated with the transaction. 
 *          The security-agreement object captures the security-agreement with 
 *          the remote party as defined in RFC3329.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransaction  - Handle to the transaction.
 *          hSecAgree     - Handle to the security-agreement.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipTransactionGetSecAgree(
									IN   RvSipTranscHandle       hTransaction,
									OUT  RvSipSecAgreeHandle    *phSecAgree);
#endif /* RV_SIP_IMS_ON */

#endif /*#ifndef RV_SIP_LIGHT*/

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
RvStatus RVCALLCONV RvSipTransactionAddViaToMsg(RvSipTranscHandle   hTransc,
                                                RvSipMsgHandle hMsg);

RvStatus RVCALLCONV RvTransactionGetIncomingSigCompUrn(IN    RvSipTranscHandle   pTransc, char* purn);
RvStatus RVCALLCONV RvTransactionIsIncomingSigComp(IN    RvSipTranscHandle   pTransc, int* EComp);
RvStatus RVCALLCONV RvSipTransactionGetFromAddr(RvSipTranscHandle   hTransc, 
                                                RvAddress           *addr,
                                                RvSipTransport      *eTransport);

#endif
//SPIRENT_END

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef RVSIP_TRANSACTION_H */



