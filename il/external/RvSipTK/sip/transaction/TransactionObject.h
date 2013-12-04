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
 *                              <Transaction.h>
 *
 *  This file organizes and uses the transaction fields that are used in receiving
 *  and sending messages. These fields are To, From, Call-Id, CSeq-Step, Via list.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                  Nov 2000
 *********************************************************************************/


#ifndef TRANSACTION_OBJECT_H
#define TRANSACTION_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "RvSipMsg.h"
#include "_SipTransactionTypes.h"
#include "RvSipPartyHeader.h"
#include "AdsRlist.h"
#include "AdsRpool.h"
#include "_SipTransaction.h"
#include "_SipTransactionTypes.h"
#include "TransactionMgrObject.h"
#include "_SipCommonTypes.h"
#include "RvSipCompartmentTypes.h"
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctpTypes.h"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
#ifdef RV_SIP_IMS_ON
#include "RvSipSecAgreeTypes.h"
#endif /* #ifdef RV_SIP_IMS_ON */

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

#define MAX_LOOP(pcall) (pCallLeg->pMgr->maxNumOfTransc*10)

/* callLegCbName - internal enumeration of the transaction callbacks.
   We use the enumeration value as the matching bit in the callbacks bitmap.
   the allocated bitmap is currently 32 bits */
typedef enum
{
    CB_TRANSC_UNDEFINED = 0,
    CB_TRANSC_CREATED = 1,
    CB_TRANSC_STATE_CHANGED,
    CB_TRANSC_MSG_TO_SEND,
    CB_TRANSC_MSG_RECEIVED,
    CB_TRANSC_APP_INT_CREATED,
    CB_TRANSC_CANCELLED,
    CB_TRANSC_CALL_CREATED,
    CB_TRANSC_IGNORE_REL_PROV,
    CB_TRANSC_INVITE_TRANSC_MSG_SENT,
    CB_TRANSC_SUPPLY_PARAMS,
    CB_TRANSC_AUTH_CREDENTIALS_FOUND,
    CB_TRANSC_AUTH_COMPLETED,
    CB_TRANSC_OPEN_CALL_LEG,
    CB_TRANSC_OTHER_URL_FOUND,
    CB_TRANSC_FINAL_DEST_RESOLVED,
    CB_TRANSC_NEW_CONN_IN_USE,
    CB_TRANSC_SIGCOMP_NOT_RESPONDED
}TranscCbName;


/* Transaction
 * ----------------
 * The transaction object comprises all messages from the first request sent
 * from the client to the server up to a final (non-1xx) response sent from
 * the server to the client. An Invite transaction comprises also the ACK
 * requests sent from the client to the server. A transaction is identified by
 * the CSeq sequence number within a single call-leg.
 * The attributes defined for the transaction object are:
 * The current state of the transaction, Call-Id header, From header, To
 * header, CSeq-Step value, The request method, A retransmissions counter, A
 * pointer to a lock, Transaction timer, A timer used for the delay between
 * retransmissions, The last message sent in encoded format, An address to
 * which messages will be sent by the transaction, The owner object and
 * Pointers to the callback functions.
 */
typedef struct
{
    RvSipTransportObjEventInfo  terminationInfo;
    HPAGE                       memoryPage; /* The personal memory page of the transaction */
    RvBool                      bSending;  /*the transaction is currently sending a message so no other send
                                           function is allowd.*/
    TransactionMgr*             pMgr;  /*The manager this transaction belongs to*/
    RvSipTransactionState       eState;  /* The current state of the transaction.*/
    RvSipPartyHeaderHandle      hFromHeader; /* The transaction's From header.*/
    RvSipPartyHeaderHandle      hToHeader; /* The transaction's To header.*/
    RvSipAddressHandle          hRequestUri; /*The transaction request uri. incase of
                                             invite transaction this is the request uri of the invite*/
    RvSipMsgHandle              hReceivedMsg; /* The transaction received message.*/
    RvSipMsgHandle              hOutboundMsg;
    SipCSeq                     cseq;		 /* The transaction's CSeq-Step value.*/
    RPOOL_Ptr                   strCallId; /* The transaction's Call-Id value.*/
    RvBool                      bIsUac; /*Is the transaction a Uac or a server transaction*/
    SipTransactionMethod        eMethod; /* The request method.*/
    RvChar*                     strMethod; /*A string of the request method. Used when the
                                           method is other than BYE ACK and INVITE*/
    RvInt32                     retransmissionsCount; /* A retransmissions counter.*/
    RvSipTimers*               pTimers;   /* Pointer to the timers timeouts values.
                                           points to the transacMgr.pTimers structure or
                                           to the transaction private structure allocated on it's page */

    SipTripleLock*              tripleLock;      /* Processing/API and object lock of the transaction */
    SipTripleLock               transactionTripleLock; /* transaction own triple lock */
    SipTimer                    hMainTimer;/* Transaction timer - used for most of the transaction timers*/
    SipTimer                    hRequestLongTimer;/* Timer that is only once for requests only.*/
    RvSipAppTranscHandle        hAppHandle;
    void*                       pOwner; /* The owner object.*/
    SipTransactionOwner         eOwnerType; /*the type of owner*/
    RvUint16                    responseCode;/* A default response code to be used when the
                                                transaction has no owner.*/
    RLIST_HANDLE                pViaHeadersList;/* A list of Via headers.*/
    /* a set of callbacks the transaction will use when defined for it*/
    RvSipTransactionEvHandlers* pEvHandlers;
#ifdef SUPPORT_EV_HANDLER_PER_OBJ
	RvSipTransactionEvHandlers	objEvHandlers;
#endif /*SUPPORT_EV_HANDLER_PER_OBJ*/
    void*                       hashElementPtr; /* The pointer of the transaction within the hash table*/
    RvBool                      bTagInTo;  /* The request message of this UAC transaction was sent
                                                  with tag in the To header*/
    RvBool                      bAppInitiate;  /*did the application initiate the transaction*/
    RvSipTranscHandle           hCancelTranscPair; /*for regular transaction this is the cancel transaction. for cancel
                                                     transaction this is the cancelled transcaction*/
    RvSipViaHeaderHandle        hTopViaHeader;   /*used to match the transaction to a cacnel transaction*/
    RvSipOtherHeaderHandle      hTimeStamp;
    RvInt32                     transactionUniqueIdentifier;
                                   /* A unique identifier for this transaction.
                                      It is generated randomly on transaction
                                      creation. If it is 0, the transaction
                                      object is vacant. */
    SipTransportAddress         receivedFrom;               /* stores the last address that the transaction recied a message from
                                                              TCP/UDP and remote IP address */

#ifdef SIP_DEBUG
    RvChar*                     pCallId;
#endif

    RvSipTransmitterHandle      hTrx; /* Transmitter object used to manage asynchronous DNS procedure
                                                 when sending the transaction's SIP message */
    RvSipTransportConnectionHandle hActiveConn;   /* handle to a connection.       */
    RvSipTransportConnectionHandle hClientResponseConnBackUp;/* We save the Invite connection
                                                         since we might receive 2xx retransmissions on it
                                                         it will be closed when the transaction terminates.*/

    SipTransportMsgType         msgType;
    RvSipTransport              eTranspType;

#ifndef RV_SIP_PRIMITIVES
	SipRSeq						localRSeq;
    RvSipTranscHandle           hPrackParent; /* The transc that received the provisional response and created the PRACK*/
    RvSipTransaction100RelStatus relStatus;
    RvInt32                     unsupportedList;
#endif /*#ifndef RV_SIP_PRIMITIVES*/
    RvBool                      bIsProxy;     /* Indicate whether the transaction is a proxy transaction*/
    RvBool                      bIsProxySend2xxTimer; /* Indicate whether the proxy transaction already send
                                                         2xx response message. */
    RvSipTransactionState       eLastState; /* saving the last state of the transaction */

#ifdef RV_SIP_AUTH_ON
    RLIST_HANDLE                pAuthorizationHeadersList; /* A list of authorization headers, for
                                                      server authentication procedure */
    RLIST_ITEM_HANDLE           lastAuthorizGotFromList; /* Handle of to the last item
                                                          that we got from  authorizationList.*/
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON
    void*                       pSubsInfo;    /* holds the subscription or notification object
                                                handle. this parameter is used only for subscribe and notify
                                                transactions. */

    RvSipReferredByHeaderHandle hReferredBy; /* referred-By header to be set in request message,
                                              in case of tranaction created becuase of refer accept */
#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /*#ifndef RV_SIP_PRIMITIVES*/
#ifdef SIP_DEBUG
    RvChar      *strUnsupportedList;
#endif

    void        *pContext; /* A call-leg information associated with this transaction.
                              The transaction holds the information for the call-leg. */

    /* If the value of this pointer is not NULL, then when terminating the transaction
       this parameter will be set to point on RV_TRUE.
       The purpose of this pointer is to tell the transaction if the application
       terminated it from the evStateChange CB. If the application terminated
       the transaction from the evStateChange CB then the transaction stops its activity. */
    RvBool*    pbIsTransactionTerminated;
#ifndef RV_SIP_PRIMITIVES
    void       *pSessionTimer; /*pointer to the session timer params of a
                                 general transaction (UPDATE) before
                                 this parameters become active in the call*/
#endif /*#ifndef RV_SIP_PRIMITIVES*/
    RvInt32     postConnectionAssertionNameOffset; /* name saved for post connection assertion */
    RvBool      bAllowCancellation;  /* This flag indicates that the transaction already associated
                                        locks with its call-leg in case it's owned by any  */

    RvBool      bAllowAckHandling;   /* This flag indicates that the transaction sent 2xx on invite
                                        and therefore should not handle the ACK request (new invite)*/
    /* this parameter saves the to-header and from-header of the incoming request.
       when we will send the response we will verify that the tags and addresses in the response
       message are the same as in those headers. If not, we will set those to the message.
       those parameters are kept and used to be on the safe side... */
    RvSipPartyHeaderHandle reqMsgToHeader;
    RvSipPartyHeaderHandle reqMsgFromHeader;
    /* this parameter saves the to-header of the incoming non-2xx response.
       when we will send the ACK we will verify that the to header in the ACK
       message is the same as this header. If not, we will set it to the message.
       this parameter is useful when call-leg retrieved 1xx with to-tag=x, and then
       final non-2xx response with to-tag=y. we don't want to change the call-leg to-
       tag, because we might have 2 calls with same identifiers in the hash, but the
       ACK must have the same to-tag.*/
    RvSipPartyHeaderHandle        non2xxRespMsgToHeader;
    RvSipPartyHeaderHandle        hOrigToForCancel;
    RvBool         bAckSent;  /*ack was actually sent*/
    RvBool         bInitialRequest; /* is this an initial request or not */

#ifdef RV_SIGCOMP_ON
    RvBool                           bSigCompEnabled;     /* Indication if the application has chosen
                                                             to stop the SigComp processing */
    RvSipCompartmentHandle           hSigCompCompartment; /* Handle to the SigComp compartment
                                                             related to the Transaction */
    RvSipCompType                    eIncomingMsgComp;    /* The incoming message compression type*/
#endif /* RV_SIGCOMP_ON */

#ifndef RV_SIP_PRIMITIVES
    RvSipCommonListHandle           hHeadersListToSetInInitialRequest; /*list of headers to set in a
                                           transaction request, that was created as result of refer */
#endif /*#ifndef RV_SIP_PRIMITIVES*/
#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT
	RvSipMsgHandle                  hInitialRequestForReprocessing; /* The first message that was received by the transaction.
																	It will be used when reprocessing invokes the creation
																	and processing of a new transaction. */
	RvSipTransportObjEventInfo      reprocessingInfo;          /* Will be used to insert a reprocessing request to the
															   event queue. */
#endif /* #ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */

	/* new parameters - version 4.5 */
#ifdef RV_SIP_IMS_ON
    RvSipSecAgreeHandle hSecAgree;/* Handle to the security-agreement object */
    RvSipSecObjHandle   hSecObj;  /* Handle to the Security Object */
#endif /* #ifdef RV_SIP_IMS_ON */

    /* new parameters - version 5.0 */
    RvInt32               cbBitmap;            /* Bitmap that saves the callback information.
                                                  When we enter a callback, we turn the callback matching bit on.
                                                  When we leave a callback, we turn the callback matching bit off
                                                  This information can be used to block APIs that are forbidden to be
                                                  used in specific callbacks.  */
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvBool          bMsgSentFailureInQueue;     /* Boolean that indicates if the transaction already received
                                                   message-sent-failure event. If so, and the event is in the queue,
                                                   we do not want to handle any incoming response for this transaction.
                                                   It prevent the following case: msg-sent-fail event is in the event
                                                   queue, an incoming response is received and handled --> update of the state machine,
                                                   then the msg-sent-failure is out of termination quque, and the
                                                   state machine is incorrect for this case */
#endif /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/

    //SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    int cctContext;
    RvSipTransportAddr  localAddr;
#define URI_CFG_PORT       1
#define URI_CFG_TRANSPORT  2
    int URI_Cfg_Flag;
#endif
    //SPIRENT_END
} Transaction;




/***************************************************************************
 * TransactionInitiateOwner
 * ------------------------------------------------------------------------
 * General: Attaches the owner and it's lock to the transaction.
 *          The transaction's callbacks will be set with a set of callbacks
 *          that are defined by this owner.
 * Return Value: RV_OK - The initialization have succeeded
 *                 RV_InvaildParameter - The owner type is unexpected, or the
 *                                     lock is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc - The new trasnaction created.
 *         pOwner - The owner of the transaction.
 *         eOwnerType - The owner's type.
 *         tripleLock - The owner's lock.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionInitiateOwner(
                                    IN  Transaction           *pTransc,
                                    IN  void                  *pOwner,
                                    IN  SipTransactionOwner    eOwnerType,
                                    IN  SipTripleLock            *tripleLock);

/***************************************************************************
 * TransactionInitialize
 * ------------------------------------------------------------------------
 * General: Initialize the transaction object. allocate all needed resources
 *          and set parameters to initial value.
 * The transaction's state will be set to "Initial" state.
 * The default response-code will be set to 0.
 * Return Value: RV_OK - The transaction was successfully initialized.
 *                 RV_ERROR_BADPARAM - At least one of the transaction key
 *                                     values received as a parameter is not
 *                                     legal.
 *                                     All key fields must have legal values.
 *               RV_InvaildParameter - The owner type is unexpected, or the
 *                                     lock is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr - The transaction manager to which this transaction
 *                       will belong.
 *          pTransc    - The new trasnaction.
 *          b_isUac    - RV_TRUE if the transaction is UAC. RV_FALSE for UAS.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionInitialize(
                                    IN  RvSipTranscMgrHandle   hTranscMgr,
                                    IN  Transaction            *pTransc,
                                    IN  RvBool                b_isUac);

/***************************************************************************
 * TransactionAttachKey
 * ------------------------------------------------------------------------
 * General: Attach the values of a given key to the transaction object.
 *          The key can belong to the transaction owner or the a message.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc - The transaction handle.
 *            pKey - The transaction's key.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAttachKey(
                                    IN  Transaction           *pTransc,
                                    IN  SipTransactionKey     *pKey);

/***************************************************************************
 * TransactionCopyKeyToInternalPage
 * ------------------------------------------------------------------------
 * General: In many cases the transaction key span on the transaction owner
 *          page. This function takes the transaction key on copy it to
 *          the transaction page.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc    - The transaction handle
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCopyKeyToInternalPage(
                                    IN  Transaction      *pTransc);


/***************************************************************************
 * TransactionCreate
 * ------------------------------------------------------------------------
 * General: Called when a server transaction is created by the transaction
 * manager. The transaction key values will be received as parameters
 * and will be set to the transaction's inner parameters.
 * The transaction's state will be set to "Initial" state.
 * The default response-code will be set to 0.
 * The Transaction's callbacks will be set with a set of callbacks that
 * are defined by it's owner and the owner and it's lock will be attached to \
 * the transaction.
 * The transaction will sign itself to the hash manager.
 * Return Value: RV_OK - The out parameter pTransc now points
 *                            to a valid transaction, initialized by the
 *                              received key.
 *                 RV_ERROR_BADPARAM - At least one of the transaction key values
 *                                       received as a parameter is not legal.
 *                                     All key fields must have legal values.
 *               RV_InvaildParameter - The owner type is unexpected, or the
 *                                     lock is NULL.
 *                 RV_ERROR_NULLPTR - At least one of the pointers to the
 *                                 transaction key values is a NULL pointer,
 *                                 or the pointer to the transaction pointer is a
 *                                 NULL pointer.
 *               RV_ERROR_OUTOFRESOURCES - Couldn't allocate memory for the
 *                                     transaction object.
 *                                     Another posibility is that the transaction
 *                                     manager is full. In that case the
 *                                     transaction will not be created and this
 *                                     value is returned.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr    - The transaction manager to which this transaction will
 *                          belong.
 *            pKey        - The transaction's key.
 *          pOwner        - The transaction's owner.
 *          eOwnerType    - The transaction's owner type.
 *          tripleLock    - The transaction's locks.
 *          b_isUac        - is client flag
 * Output:  phTransaction - The new trasnaction created.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCreate(
                                    IN  RvSipTranscMgrHandle    hTranscMgr,
                                    IN  SipTransactionKey        *pKey,
                                    IN  void                    *pOwner,
                                    IN  SipTransactionOwner        eOwnerType,
                                    IN  SipTripleLock            *tripleLock,
                                    IN  RvBool                    b_isUac,
                                    OUT RvSipTranscHandle        *phTransaction);


/***************************************************************************
 * TransactionTerminate
 * ------------------------------------------------------------------------
 * General: Causes immediate shut-down of the transaction:
 *  Changes the transaction's state to "terminated" state.
 *  Calls EvStateChanged callback with "terminated" state, and the reason
 *  received as a parameter. Calls Destruct on the transaction being
 *  terminated.
 * Return Value: RV_OK - The transaction was terminated successfully.
 *                 RV_ERROR_NULLPTR - The pointer to the transaction is a NULL
 *                                 pointer.
 *                 RV_ERROR_UNKNOWN - The transaction was not found in the transaction
 *                              manager, in other words the UnManageTransaction
 *                              call has returned the RV_ERROR_NOT_FOUND return value.
 *                              In practice the Destruct function call returns
 *                              RV_ERROR_UNKNOWN (that way it is detected here).
 *                              Note : The transaction was destructed and can
 *                              not be used any more. The RV_ERROR_UNKNOWN return is
 *                              for indication only.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTransc - The transaction to terminate.
 *            eReason -      The reason for terminating the transaction.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionTerminate(
                           IN RvSipTransactionStateChangeReason   eReason,
                           IN Transaction                      *pTransc);

/***************************************************************************
 * TransactionDestruct
 * ------------------------------------------------------------------------
 * General: Destruct a transaction object.
 * Call the UnManageTransaction function of the transactions manager
 * with this transaction handle, in other words delete this transaction from
 * the transactions manager data - structure.
 * Recycle personal used memory, including the transaction object itself
 * Return Value: RV_OK - The transaction has been destructed successfully
 *               RV_ERROR_NULLPTR - The pointer to the transaction to be deleted
 *                                 (received as a parameter) is a NULL pointer.
 *                 RV_ERROR_UNKNOWN - The UnManageTransaction call has returned the
 *                              RV_ERROR_NOT_FOUND return value. This is an error since
 *                              the transaction object is the only entity that
 *                              can call the UnManageTransaction function of the
 *                              transaction manager.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hTransc - A handle to the transaction that will be destructed.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionDestruct(Transaction *pTransc);

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/************************************************************************************
 * TransactionLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks Transaction according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the Transaction.
***********************************************************************************/
RvStatus RVCALLCONV TransactionLockAPI(IN  Transaction *pTransc);


/************************************************************************************
 * TransactionUnLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks Transaction according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the Transaction.
***********************************************************************************/
void RVCALLCONV TransactionUnLockAPI(IN  Transaction *pTransc);

/************************************************************************************
 * TransactionLockMsg
 * ----------------------------------------------------------------------------------
 * General: Locks Transaction according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the Transaction.
***********************************************************************************/
RvStatus RVCALLCONV TransactionLockMsg(IN  Transaction *pTransc);

/************************************************************************************
 * TransactionUnLockMsg
 * ----------------------------------------------------------------------------------
 * General: UnLocks Transaction according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - pointer to the Transaction.
***********************************************************************************/
void RVCALLCONV TransactionUnLockMsg(IN  Transaction *pTransc);
#else
#define TransactionLockAPI(a) (RV_OK)
#define TransactionUnLockAPI(a)
#define TransactionLockMsg(a) (RV_OK)
#define TransactionUnLockMsg(a)
#endif /* #if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE) */
/***************************************************************************
 * TransactionSetDefaultResponseCodes
 * ------------------------------------------------------------------------
 * General: Set default response-code to the transaction.
 * A server transaction that was created but doesn't belong to a
 * call-leg will have a default final response code, and it will be
 * responsible for transmitting this response as any other final response
 * code. The defaults are: Invite and Bye transactions: 481, other: 501.
 * Return Value: RV_OK - The new response-code was successfully set.
 *                 RV_ERROR_NULLPTR - The transaction handle received as a
 *                                 parameter is a NULL pointer.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc - The transaction to set the response-code to.
 *          hMsg - The message that have created this transaction.
 ***************************************************************************/
void RVCALLCONV TransactionSetDefaultResponseCodes(
                                              IN RvSipTranscHandle hTransc,
                                              IN RvSipMsgHandle  hMsg);


/***************************************************************************
 * TransactionSetMethod
 * ------------------------------------------------------------------------
 * General: Called when a new server transaction is created. Sets the
 *          transaction's method type according to the type that was received
 *          in the message.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc - The transaction to set the method to.
 *          eMethodType - The transaction's method.
 ***************************************************************************/
void RVCALLCONV TransactionSetMethod(
                                     IN RvSipTranscHandle hTransc,
                                     IN RvSipMethodType eMethodType);

/***************************************************************************
 * TransactionSetMethodStr
 * ------------------------------------------------------------------------
 * General: Copy the method sting on to the transacion page.
 * Return Value: RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc - The transaction handle.
 *            strMethod - The transaction string method.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSetMethodStr(
                                    IN  Transaction           *pTransc,
                                    IN  RvChar               *strMethod);

/***************************************************************************
 * TransactionSetMethodStrFromMsg
 * ------------------------------------------------------------------------
 * General: Called when a new server transaction is created. Sets the
 *          transaction's method string according to the string that was
 *          received in the message.
 * Return Value: RV_OK - The method was successfully set.
 *               RV_ERROR_OUTOFRESOURCES - Not enough memory to allocate method.
 *               RV_ERROR_NULLPTR - Bad pointer was received as parameter.
 *               RV_ERROR_BADPARAM - The message does not contain a
 *                                     method string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc - The transaction to set the method to.
 *          hMsg - The message to take the method from.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSetMethodStrFromMsg(
                                            IN RvSipTranscHandle hTransc,
                                            IN RvSipMsgHandle  hMsg);



/***************************************************************************
 * TransactionUpdatePartyHeaders
 * ------------------------------------------------------------------------
 * General: A user agent should strip invalid paramters from the party headers.
 *          In this case the party headers of a response can differ from the
 *          ones in the transaction. In this case we need to update the transaction.
 *          This can happen only for initial requests of a dialog: e.g. - one
 *          of the tags is missing. If this is the case we
 *          Delete the existing Party headers, and create a new ones similar to
 *            the ones received in the message.
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *                 RV_ERROR_BADPARAM - The message received has no To header
 *                                       in it.
 *                 RV_OutOfResource - Couldn't allocate memory for the new To
 *                                    header in the transaction objetc.
 *                 RV_OK - The To header was updated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: pTransc - The transaction in which to set To header.
 *           pMsg - The message from which the new To header is copied.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionUpdatePartyHeaders(
                                                  IN Transaction   *pTransc,
                                                  IN RvSipMsgHandle hMsg,
                                                 IN RvUint16 responseCode);
/***************************************************************************
 * TransactionCorrectToHeaderUrl
 * ------------------------------------------------------------------------
 * General: The SIP-URL of the To header MUST NOT contain the "transport-param"
 *          ,"maddr-param", "ttl-param", or "headers" elements. A server that
 *          receives a SIP-URL with these elements removes them before further
 *          processing. This function removes those parameters.
 * Return Value: RV_ERROR_NULLPTR - hMsg is a NULL pointers.
 *                 RV_ERROR_BADPARAM - The To header of hMsg or the Url
 *                                     of the To header are NULL.
 *                 RV_OK - The parameters were successfully removed.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hMsg - The message to correct the To Sip-Url in.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCorrectToHeaderUrl(IN RvSipMsgHandle hMsg);


/***************************************************************************
 * TransactionUpDateViaList
 * ------------------------------------------------------------------------
 * General: Copies the list of Via headers that were received in teh
 *            hMsg message, which is a request message. These headers
 *            will be set, by their order, to the responses sent by this
 *            transaction. This function will copy the Via headers and save
 *            them within the transaction object.
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *                 RV_OutOfResource - Couldn't allocate the place needed for
 *                                    the Via headers list in the transaction
 *                                    object.
 *                 RV_OK - The Via headers list was created successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction to which the Via header list belongs
 *                         to.
 *          pMsg - The message from which the Via headers should be copied.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionUpDateViaList(IN Transaction   *pTransc,
                                              IN RvSipMsgHandle hMsg);

/***************************************************************************
 * TransactionAddViaToMsg
 * ------------------------------------------------------------------------
 * General: Adds the local IP address as a Via header to the received message.
 * Return Value: RV_ERROR_NULLPTR - pTransc or hMsg are NULL pointers.
 *                 RV_OutOfResource - No memory for the allocations required.
 *               RV_ERROR_UNKNOWN - An error occured.
 *                 RV_OK - The Via header was successfully created and
 *                            inserted.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction to which the Via header list belongs
 *                         to.
 *          pMsg - The message from which the Via headers should be copied.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAddViaToMsg(IN Transaction   *pTransc,
                                            IN RvSipMsgHandle hMsg);

/***************************************************************************
 * TransactionRemoveTopViaFromMsg
 * ------------------------------------------------------------------------
 * General: Adds the local IP address as a Via header to the received message.
 * Return Value: RV_ERROR_NOT_FOUND - no via header was found in the message.
 *                 RV_OK - The Via header was successfully removed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pMgr - The transaction manager for log printing
 *                         to.
 *          hMsg - The message from which the Via headers should be removed.
 ***************************************************************************/
RvStatus TransactionRemoveTopViaFromMsg(IN TransactionMgr *pMgr,
                                         IN RvSipMsgHandle hMsg);

/***************************************************************************
 * TransactionAddTag
 * ------------------------------------------------------------------------
 * General: Generate and add a Tag to the transaction's To header.
 * Return Value: RV_ERROR_NULLPTR - pTransc is a NULL pointers.
 *               RV_ERROR_BADPARAM - The To header of pTransc is a NULL
 *                                     pointer.
 *                 RV_OutOfResource - Couldn't allocate memory for the new Tag
 *                                    string, in the transaction object.
 *                 RV_OK - The To tag was created and set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction in which new Tag will be set.
 *        bIsTo - RV_TRUE if the tag should be added to the To header.
 *                RV_FALSE if it should be added to the From header.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAddTag(IN Transaction *pTransc,
                                         IN RvBool      bIsTo);

/***************************************************************************
 * TransactionCopyToTag
 * ------------------------------------------------------------------------
 * General: Copy the To Tag from the message to the To header of the
 *          transaction. The tag is copied only if there is no tag found
 *          in the transaction object.
 * Return Value: RV_OutOfResource - Couldn't allocate memory for the new To
 *                                  header in the transaction objetc.
 *               RV_OK - The To header was updated successfully.
 *               RV_ERROR_UNKNOWN - The message is illegal.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: pTransc - The transaction in which to update To header.
 *           pMsg - The message from which the To tag is copied.
 *           bIgnoreTranscTag - true: copy to-tag even if transaction to-tag exists.
 *                              false: do not copy if transaction to-tag exists.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCopyToTag(
                                           IN Transaction   *pTransc,
                                           IN RvSipMsgHandle hMsg,
                                           IN RvBool         bIgnoreTranscTag);

/***************************************************************************
 * TransactionCreateRequestMessage
 * ------------------------------------------------------------------------
 * General: Create a request message. The request message will contain the
 *            To, From, CSeq, Call-Id values assosiated with the transaction
 *            and the Method and Request-Uri that were received as parameters.
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *                 RV_OutOfResource - Couldn't allocate memory for the new
 *                                    message, and the assosiated headers.
 *                 RV_OK - The Request message was successfully created.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc - The transaction for which the request message is
 *                          created.
 *           eMethod - The request method.
 *           pRequestUri - The Request-Uri of the request message.
 * Output: phMsg - The new message created.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCreateRequestMessage(
                                          IN  Transaction       *pTransc,
                                          IN  RvSipAddressHandle hRequestUri,
                                          INOUT RvSipMsgHandle   phMsg);


/***************************************************************************
 * TransactionAddViaToResponseMessage
 * ------------------------------------------------------------------------
 * General: Inserts to the message the list of Via headers assosiated with
 *          this transaction (a server transaction).
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *                 RV_OutOfResource - Couldn't allocate memory for the new
 *                                    message, and the assosiated headers.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction for which the response message is
 *                          created.
 *          hMsg - The message to insert the Via headers to.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAddViaToResponseMessage(
                                               IN Transaction   *pTransc,
                                               IN RvSipMsgHandle hMsg);


/***************************************************************************
 * TransactionCreateResponseMessage
 * ------------------------------------------------------------------------
 * General: Create a response message. The response message will contain the
 *            To, From, CSeq, Call-Id values assosiated with the transaction.
 *            The list of Via headers assosiated with the transaction (was
 *            received in the request message) will also be inserted to the
 *            message. The Method received as a parameters will be used in the
 *            CSeq header. The Response-Code and Reason-Phrase received as
 *            parameters will be the used in this response. The Reason-Prase is
 *            optional - if the parameter is NULL no Response-code will be set
 *            to the response.
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *                 RV_OutOfResource - Couldn't allocate memory for the new
 *                                    message, and the assosiated headers.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction for which the response message is
 *                          created.
 *          responseCode - The Response-Code of this message.
 *          strReasonPhrase - The Reason-Phrase of this message (optional).
 *        b_isReliable    - Is this a reliable provisional response.
 *          pMsg - The new message created.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCreateResponseMessage(
                                         IN  Transaction    *pTransc,
                                         IN  RvUint16       responseCode,
                                         IN  RvChar        *strReasonPhrase,
                                         IN RvBool          b_isReliable,
                                         INOUT RvSipMsgHandle  phMsg);

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/***************************************************************************
* TransactionClone
* ------------------------------------------------------------------------
* General: Creates new transaction and copies there major transaction
*          parameters.
*          It is important to note: we do not copy the via, as new via
*          will have to generated. the reason for that is that new dest
*          protocol or local address might be in use for this transaction
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:   pOrigTrans        - The source transaction structure pointer.
*          owner            - pointer to the new transaction owner info.
*          bRemoveToTag     - Remove the to tag from transaction or not.
*                             (This parameter should be TRUE for a first
*                              INVITE transaction, or for a non-callLeg
*                              transaction.
* Output:  phCloneTrans        - Pointer to the new transaction handler.
***************************************************************************/
RvStatus RVCALLCONV TransactionClone(IN  Transaction              *pOrigTrans,
                                      IN  RvSipTranscOwnerHandle hOwner,
                                      IN  RvBool                bRemoveToTag,
                                      OUT RvSipTranscHandle         *phCloneTrans);

#endif /*#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT*/

/***************************************************************************
 * TransactionNoOwner
 * ------------------------------------------------------------------------
 * General: Called when attaching NULL owner. This function allocates new
 *            From, To and Call-Id fields and updates them with the values
 *            she is assosiated with.
 * Return Value: RV_OK - The detaching was successfully made.
 *                 RV_OutOfResource - Couldn't allocate memory for the
 *                                    assosiated headers.
 *                 RV_ERROR_NULLPTR - pTransc is a NULL pointer.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction for which the Attach was called with
 *                         NULL.
 ****************************************************************************/
RvStatus RVCALLCONV TransactionNoOwner(IN Transaction *pTransc);

RvStatus RVCALLCONV TransactionAttachApplicationOwner(
                                    IN RvSipTranscHandle    hTransc,
                                    IN SipTransactionKey    *pKey,
                                    IN void                 *pOwner);

/***************************************************************************
 * TransactionAttachOwner
 * ------------------------------------------------------------------------
 * General: Attach a new owner to the transaction. The owner should attach it's
 * callback implementations to the transaction as well, so that the
 * callbacks will be addressed to this owner (the handle to the
 * owner will be sent as a parameter to the callback). The owner should also
 * attach its lock to the transaction, so that the transaction could lock it
 * when synchronization is needed.
 * Return Value: RV_OK - The new owner is attached to the transaction.
 *                              (Attaching NULL is equivalent to detach. The
 *                               transaction will no longer has an owner, and the
 *                               callbacks use a default implementation).
 *                 RV_ERROR_NULLPTR - The transaction handle parameter is a NULL
 *                                 pointer.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransc - The transaction to attach the owner to.
 *            pOwner -       The new owner of the transaction.
 *          eOwnerType -   The owner's type: Call, Registrar ...
 *          bIsLast -      Is this the last owner we will try to attach this
 *                         transaction. If yes than in case this owner is NULL
 *                         the transaction is a stand alone transaction.
 *            tripleLock     - locks of the transaction
 *          bCopyTripleLock- Indication if to copy the input tripleLock to
 *                           the local pTransc->tripleLock, since there
 *                           might be a situation that the lock was already
 *                           copied.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAttachOwner(
                                    IN RvSipTranscHandle        hTransc,
                                    IN void                        *pOwner,
                                    IN SipTransactionKey        *pKey,
                                    IN SipTripleLock            *tripleLock,
                                    IN SipTransactionOwner        eOwnerType,
                                    IN RvBool                  bCopyTripleLock);


/***************************************************************************
 * TransactionCopyToHeader
 * ------------------------------------------------------------------------
 * General: Allocate new memory for the transaction's To header, and copy
 *          it's value to this new memory.
 * Return Value: RV_OK - The allocation and copying were successfully
 *                            made.
 *                 RV_OutOfResource - Couldn't allocate memory for the copying.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc - The transaction to copy the To header in.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCopyToHeader(IN Transaction *pTransc);


/***************************************************************************
 * TransactionCopyFromHeader
 * ------------------------------------------------------------------------
 * General: Allocate new memory for the transaction's From header, and copy
 *          it's value to this new memory.
 * Return Value: RV_OK - The allocation and copying were successfully
 *                            made.
 *                 RV_OutOfResource - Couldn't allocate memory for the copying.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc - The transaction to copy the From header in.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCopyFromHeader(IN Transaction *pTransc);


/***************************************************************************
 * TransactionCopyCallId
 * ------------------------------------------------------------------------
 * General: Allocate new memory for the transaction's Call-Id , and copy
 *          it's value to this new memory.
 * Return Value: RV_OK - The allocation and copying were successfully
 *                            made.
 *                 RV_OutOfResource - Couldn't allocate memory for the copying.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc - The transaction to copy the Call-Id in.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCopyCallId(IN Transaction *pTransc);

/***************************************************************************
 * TransactionSetRequestUri
 * ------------------------------------------------------------------------
 * General: Set the request URI on the transcation page.
 * Return Value: RV_OK - The allocation and copying were successfully
 *                            made.
 *                 RV_OutOfResource - Couldn't allocate memory for the copying.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pTransc - The transaction to copy the request uri in.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSetRequestUri(
                                   IN Transaction             *pTransc,
                                   IN RvSipAddressHandle      hRequestUri);

/***************************************************************************
 * TransactionGenerateCallId
 * ------------------------------------------------------------------------
 * General: Generate a CallId to the given Transctaion object.
 *          The call-id is kept on the transaction memory page.
 *          We use this function if the request was called and no
 *          call-id was supplied.
 * Return Value: RV_ERROR_OUTOFRESOURCES - The Call-leg page was full.
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - Pointer to the transaction object
 ***************************************************************************/
RvStatus RVCALLCONV TransactionGenerateCallId(IN  Transaction*  pTransc);


/***************************************************************************
 * TransactionInitializeCancelTransc
 * ------------------------------------------------------------------------
 * General: Initialize a cancel transaction according to the transaction
 *          that is about to be canceled.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCancelTransc - The cancel transaction
 *        pCancelledTransc - The tranasction to be cancelled
 ***************************************************************************/
RvStatus RVCALLCONV  TransactionInitializeCancelTransc(
                                            IN  Transaction*  pCancelTransc,
                                            IN  Transaction*  pCancelledTransc);

/***************************************************************************
 * TransactionCopyTimestampToMsg
 * ------------------------------------------------------------------------
 * General: Copy time stamp header from the transaction page to the
 *           message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction handle
 *          hMsg - handle to the message.
 ***************************************************************************/
RvStatus TransactionCopyTimestampToMsg(IN Transaction   *pTransc,
                                        IN RvSipMsgHandle hMsg);

/***************************************************************************
 * TransactionCopyTimestampFromMsg
 * ------------------------------------------------------------------------
 * General: Copy time stamp header from the request message to the transaction
 *          page.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction which received the request messgae.
 *          hMsg - the request message.
 ***************************************************************************/
RvStatus TransactionCopyTimestampFromMsg(IN Transaction   *pTransc,
                                                 IN RvSipMsgHandle hMsg);



/*-----------------------------------------------------------------
                     DNS SUPPORT
  -----------------------------------------------------------------*/

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/***************************************************************************
 * TransactionHandleMsgSentFailureEvent
 * ------------------------------------------------------------------------
 * General: Sends INTERNAL_OBJECT_EVENT of msg sent failure to the processing queue event.
 * Return Value: RV_OK        - The event was sent successfully.
 *                 RV_ERROR_NULLPTR    - The pointer to the transaction is a NULL
 *                                  pointer.
 *                 RV_ERROR_OUTOFRESOURCES
 *                                - The function failed to allocate event structure
 *                                  due to no enough resources.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hTransc - The transaction handle
 *            eReason -      The reason for the event
 ***************************************************************************/
RvStatus RVCALLCONV TransactionHandleMsgSentFailureEvent(
                        IN Transaction*                         pTransc,
                        IN RvSipTransactionStateChangeReason    eReason);

#endif

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * TransactionUpdateReliableStatus
 * ------------------------------------------------------------------------
 * General: Update the reliable status of an Invite transaction from an
 *          INVITE message received.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction handle.
 *          pMsg - The INVITE received message.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionUpdateReliableStatus(
                                            IN Transaction   *pTransc,
                                            IN RvSipMsgHandle hMsg);

/***************************************************************************
 * TransactionSetUnsupportedList
 * ------------------------------------------------------------------------
 * General: Go over the message require headers and add any unsupported
 *          option to the unsupported list in the transaction.If the list
 *          is not empty the transaction will respond with 420 and will add
 *          all unsupported headers in the response message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction handle.
 *          pMsg - The INVITE received message.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSetUnsupportedList(
                                            IN Transaction   *pTransc,
                                            IN RvSipMsgHandle hMsg);

#endif /*#ifndef RV_SIP_PRIMITIVES*/



/***************************************************************************
 * TransactionRand
 * ------------------------------------------------------------------------
 * General: returns a random number smaller then the size of RvInt32.
 * Return Value: random RvInt32.
 * ------------------------------------------------------------------------
 * Arguments: (-)
 ***************************************************************************/
RvInt32 RVCALLCONV TransactionRand(IN Transaction         *pTransc);


/***************************************************************************
 * TransactionMethodStricmp
 * ------------------------------------------------------------------------
 * General: The function does insensitive comparsion between 2 strings..
 * Return Value: RV_TRUE - if equal, RV_FALSE - if not equal.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: number - The integer value.
 *        buffer - The buffer that will contain the string of the integer.
 ***************************************************************************/
RvBool RVCALLCONV TransactionMethodStricmp(IN RvChar* firstStr, IN RvChar* secStr);

/***************************************************************************
 * TransactionGetMsgMethodFromTranscMethod
 * ------------------------------------------------------------------------
 * General: Returns the aquivalent message method from the transaction method.
 * Return Value: the aquivalent message method from the transaction method.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction handle
 ***************************************************************************/
RvSipMethodType TransactionGetMsgMethodFromTranscMethod(
                                    IN  Transaction    *pTransc);

/***************************************************************************
 * TransactionGetEnumMethodFromString
 * ------------------------------------------------------------------------
 * General: returns the enumeration that a string represents. If the string
 *          is invalid such as begins with spaces or too long the method
 *          returned is SIP_TRANSACTION_METHOD_UNDEFINED.
 * Return Value: The method as emum.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: strRequestMethod - the method as string.
 ***************************************************************************/
SipTransactionMethod RVCALLCONV TransactionGetEnumMethodFromString(
                                IN RvChar   *strRequestMethod);

/***************************************************************************
 * TransactionGetStringMethodFromEnum
 * ------------------------------------------------------------------------
 * General: return a string with the method of the given enumeration.
 * Return Value: The method as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: eMethod - the method as enum.
 ***************************************************************************/
RvChar*  RVCALLCONV TransactionGetStringMethodFromEnum(
                                            IN SipTransactionMethod eMethod);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)

/***************************************************************************
 * TransactionGetStateName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given state
 * Return Value: The string with the state name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eState - The state as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV TransactionGetStateName(
                                      IN  RvSipTransactionState  eState);

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * TransactionGetMsgCompTypeName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given transaction compression message type
 * Return Value: The string with the message type.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     eMsgCompType - The compression type as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV TransactionGetMsgCompTypeName(
                             IN  RvSipTransmitterMsgCompType eMsgCompType);
#endif /* RV_SIGCOMP_ON */
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE) */


/***************************************************************************
 * TransactionSetReceivedMsg
 * ------------------------------------------------------------------------
 * General: Sets a handle to the received message.
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction to which the mesasge should be set
 *        hReceivedMsg - Handle to the received message.
 ***************************************************************************/
void  RVCALLCONV TransactionSetReceivedMsg(IN Transaction* pTransc,
                                           IN RvSipMsgHandle    hReceivedMsg);


/***************************************************************************
 * TransactionGetReceivedMsg
 * ------------------------------------------------------------------------
 * General: Return the transaction received message handle
 * Return Value: Handle to the transaction received message.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction to which the mesasge should be set
 ***************************************************************************/
RvSipMsgHandle  RVCALLCONV TransactionGetReceivedMsg(IN Transaction* pTransc);


/***************************************************************************
 * IsValidMessage
 * ------------------------------------------------------------------------
 * General: Return Indication whether the message (request/response) is a
 *          message.
 * Return Value: The function return RV_TRUE if the message is valid,
 *                otherwise the function return RV_FALSE
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - pointer to transaction to print the log on.
 *        hMsg - pointer to validate message.
 *        bIsRequest - indication whether to check the message as request or
 *                     as response.
 ***************************************************************************/
RvBool RVCALLCONV IsValidMessage(IN Transaction* pTransc,
                                  IN RvSipMsgHandle    hMsg,
                                  IN RvBool        bIsRequest);

/***************************************************************************
 * FinalResponseReason
 * ------------------------------------------------------------------------
 * General: Returnes the enumeration value defined for the class of the
 *          received response code
 * Return Value: The response code class inumeration.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: responseCode - The response code.
 ***************************************************************************/
RvSipTransactionStateChangeReason RVCALLCONV
                        FinalResponseReason(
                                               IN RvUint16 responseCode);

/***************************************************************************
 * TransactionCopyStrToTranscPage
 * ------------------------------------------------------------------------
 * General: Copy a given string to the transaction
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - targt transaction to copy the string to
 *          strData - The strgin to be set in the object.
 *                           If Null, the exist string in the
 *                           object will be removed.
 *          strOffset      - Offset of a string on the same page to use in an attach case.
 *                           (else - UNDEFINED)
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          strOffset - the offset of the string.
 * Out:     pReturnOffset - traget offset.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionCopyStrToTranscPage(IN  Transaction       *pTransc,
                                                 OUT RvInt32           *pReturnOffset,
                                                 OUT RvChar           **pDebugInf,
                                                 IN  RvChar*          strData,
                                                 IN  HRPOOL            hPool,
                                                 IN  HPAGE             hPage,
                                                 IN  RvInt32          strOffset);


/***************************************************************************
 * TransactionUpdateTopViaFromMsg
 * ------------------------------------------------------------------------
 * General: Update the transaction top via header from the message which about
 *          to be send.
 *          this function should be called after calling EvMsgToSend.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pTransc    - pointer to transaction to update its top via header.
 *          hMsg       - Source message to update the top via from.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionUpdateTopViaFromMsg(
                                            IN Transaction        *pTransc,
                                            IN RvSipMsgHandle   hMsg);



/***************************************************************************
 * TransactionChangeState
 * ------------------------------------------------------------------------
 * General: Change the transaction state accordingly. Call the EvStateChanged
 *            callback with the new state.
 * Return Value: (-).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc              - The transaction in which the state has changed.
 *        eState                    - The new state.
 *        eReason                   - The reason for the state change.
 * Output:(-)
***************************************************************************/
void RVCALLCONV TransactionChangeState(IN Transaction                      *pTransc,
                                       IN RvSipTransactionState             eState,
                                       IN RvSipTransactionStateChangeReason eReason);


/***************************************************************************
* TransactionLockTranscMgr
* ------------------------------------------------------------------------
* General: Locks the transaction mgr. This function is used when changing parameters
*          inside the transaction that influence the hash key of the transaction.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTransc    - Pointer to the transaction structure
***************************************************************************/
void TransactionLockTranscMgr(Transaction *pTransc);

/***************************************************************************
* TransactionUnlockTranscMgr
* ------------------------------------------------------------------------
* General: Unlocks the transaction mgr. This function is used when changing parameters
*          inside the transaction that influence the hash key of the transaction.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTransc    - Pointer to the transaction structure
***************************************************************************/
void TransactionUnlockTranscMgr(Transaction *pTransc);

/***************************************************************************
* TransactionSetCallID
* ------------------------------------------------------------------------
* General: Sets the Call ID in the transaction. The function locks the transaction
*          manager when doing this.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTransc    - Pointer to the transaction structure
*          hPool      - The pool of the call ID
*          hPage      - The page of the call ID
*          offset     - The call ID offset
***************************************************************************/
void TransactionSetCallID(Transaction *pTransc,
                          HRPOOL       hPool,
                          HPAGE        hPage,
                          RvInt32     offset);

/***************************************************************************
* TransactionSetResponseCode
* ------------------------------------------------------------------------
* General: Sets the response code in the transaction. The function locks the transaction
*          manager when doing this.
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTransc      - Pointer to the transaction structure
*          responseCode - The response code to set in the transaction
***************************************************************************/
void TransactionSetResponseCode(Transaction *pTransc,
                                RvUint16    responseCode);

/***************************************************************************
* TransactionSetTag
* ------------------------------------------------------------------------
* General: Sets the tag of the to/from header in the transaction. The function locks the
*          transaction manager when doing this.
* Return Value: - RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:   pTransc   - Pointer to the transaction structure
*          strTag    - The string to set in the tag
*          bIsToTag  - RV_TRUE if the tag is the to-tag, RV_FALSE if the tag is the from tag
***************************************************************************/
RvStatus TransactionSetTag(Transaction *pTransc,
                            RvChar     *strTag,
                            RvBool      bIsToTag);

/***************************************************************************
 * TransactionGenerateTag
 * ------------------------------------------------------------------------
 * General: Generate and add a Tag to the transaction's To header.
 * Return Value: RV_ERROR_NULLPTR - pTransc is a NULL pointers.
 *               RV_ERROR_BADPARAM - The To header of pTransc is a NULL
 *                                     pointer.
 *                 RV_OutOfResource - Couldn't allocate memory for the new Tag
 *                                    string, in the transaction object.
 *                 RV_OK - The To tag was created and set successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction in which new Tag will be set.
 *        bIsTo - RV_TRUE if the tag should be added to the To header.
 *                RV_FALSE if it should be added to the From header.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionGenerateTag(IN TransactionMgr         *pMgr,
                                           IN Transaction            *pTransc,
                                           OUT RvChar*               strTag);

/***************************************************************************
 * TransactionSaveReceivedFromAddr
 * ------------------------------------------------------------------------
 * General: saves the address from which the last address was received.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   -- Handle to the transaction.
 *            pRecvFrom -- the address on which a message was received
 ***************************************************************************/
void RVCALLCONV TransactionSaveReceivedFromAddr (
                                      IN  Transaction*         pTransc,
                                      IN  SipTransportAddress* pRecvFrom);

/***************************************************************************
 * TransactionSetTripleLock
 * ------------------------------------------------------------------------
 * General: replaces transaction and objects under the transaction
 *        triple locks
 *
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    ownerTripleLock  - new triple lock.
 *          pTransc     - The transaction
 ***************************************************************************/
void RVCALLCONV TransactionSetTripleLock(
    IN SipTripleLock *ownerTripleLock,
    IN Transaction   *pTransc);

#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * TransactionConstructAndSetReferredByHeader
 * ------------------------------------------------------------------------
 * General: Allocate and Sets the given referred-by header in the transaction object.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc   - Handle to the transaction.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionConstructAndSetReferredByHeader (
                                      IN  Transaction                   *pTransc,
                                      IN  RvSipReferredByHeaderHandle    hReferredBy);
#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /*#ifndef RV_SIP_PRIMITIVES */

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * TransactionHandleIncomingSigCompMsgIfNeeded
 * ------------------------------------------------------------------------
 * General: The function should be called when a SigComp-SIP message is
 *          received. According to the SigComp model each SigComp-SIP
 *          message MUST be related to any SigComp compartment. Thus, the
 *          function verifies that the Transaction is currently associated
 *          with a SigComp Compartment. In case that the Transaction is not
 *          part of any compartment, then new one is created for it.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc    - The transaction handle.
 *          hConn      - The connection on which the message was received.
 *          pRecvFrom  - The address from which the message was received.
 *          hLocalAddr - The address on which the message was received
 *          hMsg       - Handle to the incoming message, to be related to a
 *                       Compartment object
 *          pSigCompMsgInfo -
 *                      SigComp info related to the received message.
 *                      The information contains indication if the UDVM
 *                      waits for a compartment ID for the given unique
 *                      ID that was related to message by the UDVM.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionHandleIncomingSigCompMsgIfNeeded(
                             IN    Transaction                     *pTransc,
                             IN    RvSipTransportConnectionHandle   hConn,
                             IN    SipTransportAddress             *pRecvFrom,
                             IN    RvSipTransportLocalAddrHandle    hLocalAddr,
                             IN    RvSipMsgHandle                   hMsg,
                             INOUT SipTransportSigCompMsgInfo      *pSigCompMsgInfo);


/***************************************************************************
 * TransactionGetOutboundMsgCompType
 * ------------------------------------------------------------------------
 * General: Retrieves the compression type of the Transaction outbound
 *          message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc - The transaction handle.
 ***************************************************************************/
RvSipCompType RVCALLCONV TransactionGetOutboundMsgCompType(
                                         IN Transaction   *pTransc);
#endif /* RV_SIGCOMP_ON */


/***************************************************************************
 * TransactionAttachToConnection
 * ------------------------------------------------------------------------
 * General: Attach the Transaction as the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc       - pointer to Transaction
 *            hConn         - The connection handle
 *            bUpdateActiveConn - update the transaction active connection with
 *                                the connection we just attached to.
 ***************************************************************************/
RvStatus TransactionAttachToConnection(
                        IN Transaction                    *pTransc,
                        IN RvSipTransportConnectionHandle  hConn,
                        IN RvBool                          bUpdateActiveConn);


/***************************************************************************
 * TransactionDetachFromConnection
 * ------------------------------------------------------------------------
 * General: detach the Transaction from the connection owner
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransc    - pointer to Transaction
 *            hConn   - the connection to detach from.
 *            bUpdateActiveConn - if we detached from the active conn, set it
 *                                to NULL.
 ***************************************************************************/
void TransactionDetachFromConnection(
                  IN Transaction                    *pTransc,
                  IN RvSipTransportConnectionHandle  hConn,
                  IN RvBool                          bUpdateActiveConn);

/***************************************************************************
 * TransactionSetTimers
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
RvStatus TransactionSetTimers(IN  Transaction*  pTransc,
                              IN  RvSipTimers*  pTimers);

/***************************************************************************
 * TransactionFillBasicRequestMessage
 * ------------------------------------------------------------------------
 * General: Fill a request message with basic parameters.
 * Return Value: RV_ERROR_NULLPTR - pTransc or pMsg are NULL pointers.
 *                 RV_OutOfResource - Couldn't allocate memory for the new
 *                                    message, and the assosiated headers.
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
RvStatus RVCALLCONV TransactionFillBasicRequestMessage(
                                        IN  RvSipAddressHandle hRequestUri,
                                        IN  RvSipPartyHeaderHandle hFrom,
                                        IN  RvSipPartyHeaderHandle hTo,
                                        IN  RPOOL_Ptr         *pStrCallId,
                                        IN  RvSipMethodType    eMethod,
                                        IN  RvChar            *strMethod,
                                        IN  SipCSeq            *pCseq,
                                        IN  RvBool             bAddMaxForwards,
                                        INOUT RvSipMsgHandle   hMsg);

/***************************************************************************
 * TransactionGetKeyFromTransc
 * ------------------------------------------------------------------------
 * General: Fill the Key structure with transaction/s parameters.
 * Return Value: RV_ERROR_BADPARAM - no cseq in transc.
 *                 RV_OK - All the values were updated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc - The transaction.
 * Output:  key     - A structure of the key to fill.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionGetKeyFromTransc(
                                    IN  Transaction       *pTransc,
                                    OUT SipTransactionKey *pKey);

#endif /*#ifndef RV_SIP_LIGHT*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * TransactionSetSecObj
 * ----------------------------------------------------------------------------
 * General: Sets Security Object into the Transaction.
 *          As a result of this operation, all messages, sent by this
 *          Transaction, will be protected with the security mechanism,
 *          provided by the Security Object.
 * Return Value: RV_OK on error,
 *          error code, defined in RV_SIP_DEF.h or rverror.h, otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  pTransc - Pointer to the Transaction object.
 *          hSecObj - Handle to the Security Object. Can be NULL.
 *****************************************************************************/
RvStatus TransactionSetSecObj(IN  Transaction*      pTransc,
                              IN  RvSipSecObjHandle hSecObj);
#endif /*#ifndef RV_SIP_IMS_ON*/

/******************************************************************************
 * TranscGetCbName
 * ----------------------------------------------------------------------------
 * General: Print the name of callback represented by the bit turned on in the
 *          mask results.
 * Return Value: (-)
 * ----------------------------------------------------------------------------
 * Arguments:
 *  Input:  MaskResults - the bitmap holding the callback representing bit.
 *****************************************************************************/
RvChar* TranscGetCbName(RvInt32 MaskResults);


#ifdef __cplusplus
}
#endif

#endif /*END OF: #ifndef TRANSACTION_OBJECT_H */
