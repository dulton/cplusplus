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
 *                              <TransactionMgr.h>
 *
 *  This files defines an interface between the transaction and the transactions
 *  manager.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                  Nov 2000
 *********************************************************************************/



#ifndef TRANSACTION_MGR_OBJECT_H
#define TRANSACTION_MGR_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RV_SIP_LIGHT

#include "RV_SIP_DEF.h"
#include "rvmutex.h"
#include "rvselect.h"
#include "RvSipPartyHeader.h"
#include "RvSipAddress.h"
#include "RvSipMsg.h"
#include "RvSipAuthenticator.h"
#include "AdsRlist.h"
#include "AdsRpool.h"
#include "_SipTransaction.h"
#include "_SipTransactionTypes.h"
#include "_SipTransportTypes.h"
#include "AdsHash.h"
#include "RvSipCompartmentTypes.h"

#ifdef SIP_DEBUG
#include "RvSipResourcesTypes.h"
#endif
#include "rvlog.h"
#ifdef RV_SIGCOMP_ON
#include "RvSigComp.h"
#endif /* RV_SIGCOMP_ON */
#ifdef RV_SIP_IMS_ON
#include "RvSipSecAgreeTypes.h"
#endif

#define DEFAULT_MAX_TRANSACTION_NUMBER 10
#define DEFAULT_MAX_TOTAL_VIA_HEADERS_NUMBER 50

#define MAX_INVITE_RETRANSMISSIONS   7


/* TransactionMgr
 * -------------------
 * The transaction manager will manipulate a data-structure of transactions.
 * This data - structure will supply methods to insert, delete and find a
 * transaction within it.
 */
typedef struct
{
    RvLogMgr*            pLogMgr;

    /*the transactions data-structure pool:*/
    RLIST_POOL_HANDLE hTranasactionsListPool;
    /*A transactions data-structure:*/
    RLIST_HANDLE hTranasactionsList;
    /* A lock: */
    RvMutex hLock;
	
	/*This lock is used when processing incoming msgs for switching the order of the locks
	  between the transaction and its mgr.
	  Note: 
	  1. The transactionMgr lock should not lock under this mutex.
	  2. This lock should not be locked under transaction lock.*/
	RvMutex hSwitchSafeLock;

    /*The default event handlers*/
    RvSipTransactionEvHandlers pDefaultEvHandlers;
#ifndef RV_SIP_PRIMITIVES
    /*The call leg event handlers*/
    RvSipTransactionEvHandlers pCallLegEvHandlers;
    /* The transaction-client event handlers */
    RvSipTransactionEvHandlers pTranscClientEvHandlers;
#endif /* #ifndef RV_SIP_PRIMITIVES */
    /* The application event handlers */
    RvSipTransactionEvHandlers pAppEvHandlers;
    /* a callback defined by the Call-Manager for the transaction manager */
    SipTransactionMgrEvHandlers    pCallMgrDetail;
    RvSipTransactionMgrEvHandlers  pMgrEvHandlers;
    /* a callback defined by the application-Manager for the transaction manager */
    void* pAppMgr;
    /* A hash table that manages the transaction list */
    HASH_HANDLE hHashTable;
    /* The size of the hash table */
    RvInt32 hashSize;
    /* The global memory pools */
    HRPOOL hMessagePool;
    HRPOOL hGeneralPool;
    /* The log handle */
    RvLogSource* pLogSrc;
    /* The transport handle */
    RvSipTransportMgrHandle hTransport;
    /* The message manager handle */
    RvSipMsgMgrHandle hMsgMgr;
    /* Configuration parameters */
    RvInt32 maxTransactionNumber;
    RvInt32  cancelGeneralNoResponseTimeout;
    RvSipTimers  timers;
    RvSipTimers* pTimers;
#ifndef RV_SIP_PRIMITIVES
    RvSipTransaction100RelStatus status100rel;
    RvChar**         supportedExtensionList;
    RvInt32          extensionListSize;
    RvBool           rejectUnsupportedExtensions;
    RvBool           addSupportedListToMsg;
#endif /* #ifndef RV_SIP_PRIMITIVES*/
    RvInt32          proxy2xxSentTimer;
    RvInt32          proxy2xxRcvdTimer;
    RvBool           bIgnoreOutboundProxy;
    RvSipTransactionProxyEvHandlers proxyEvHandlers;
    HPAGE              hTransportAddr; /* used for saving a consecutive address
                                                      of a destination address of the out of context send message*/
    RvBool                      isProxy;
#ifdef RV_SIP_AUTH_ON
    RvSipAuthenticatorHandle    hAuthenticator;
    RvBool                      enableServerAuth;
#endif /* #ifdef RV_SIP_AUTH_ON */
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON
    RvInt32                     maxSubscriptions;
#endif /* #ifdef RV_SIP_SUBS_ON */
    RvBool                      manualPrack;
#endif /* #ifndef RV_SIP_PRIMITIVES*/
    RvBool                      manualBehavior; /* 487 on cancelled */
    RvBool                      bIsPersistent;
    RvBool                      bDynamicInviteHandling;
    /* patch for Customer */
    RvBool                     bEnableNestedInitialRequestHandling;
    RvBool                      bDisableMerging;
    RvBool                      bEnableForking;
    RvRandomGenerator*          seed;
    RvBool                      enableInviteProceedingTimeoutState;
#ifdef SIP_DEBUG
    /* The stack's statistics of sending and receiving messages */
    SipTransactionMgrStatistics msgStatistics;
#endif
    void*                       hStack;
#ifdef RV_SIGCOMP_ON
    RvInt32                     sigCompTcpTimer; /* SigComp TCP Timeout duration */
    RvSipCompartmentMgrHandle   hCompartmentMgr;
#endif /* RV_SIGCOMP_ON */
    RvSelectEngine*             pSelect;
    RvSipTransmitterMgrHandle   hTrxMgr;
    RvSipTransmitterEvHandlers  transcTrxEvHandlers; /*event handler for a transmitter that belongs to a transaction*/
    RvSipTransmitterEvHandlers  mgrTrxEvHandlers; /*event handler for a transmitter that belongs to the manager*/
    RvSipTransportConnectionEvHandlers connEvHandlers;
    RvBool                      bOldInviteHandling; /*Ack on 2xx is part of the tranasction*/
#ifdef RV_SIP_IMS_ON
    RvSipSecMgrHandle           hSecMgr;
    RvSipSecAgreeMgrHandle      hSecAgreeMgr;  /* Handle to the security-agreement manager */
#endif /* RV_SIP_IMS_ON */
    /* new parameters for version 5.0 */
    RvInt32                    terminateMask; /* Mask that defines in which callbacks we block the usage 
                                                 of RvSipTransactionTerminate() API.*/

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    /* maxRetransmitINVITENumber:  This parameter defines the maximum number of times that stack will RE-SEND an INVITE command if
     *        it does not receive the Ringing status signal within a configured timeout period. Therefore, in a worst case, the 
     *        total number of "INVITE" message equals to (maxRetransmitINVITECount+1).
     */
    RvInt32                 maxRetransmitINVITENumber;
    /*
     * initialGeneralRequestTimeout:  This is the initial waiting period used by the stack to wait for an acknowledgement 
     *      message for its previous sent "INVITE" message.   Please note that this value is used only once because 
     *      a subsequence waiting period (for a resending INVITE message) is calculated using the exponential back-off 
     *      formula with this initial value.
     */
    RvUint32                initialGeneralRequestTimeout; 
#endif
#if defined(UPDATED_BY_SPIRENT_ABACUS)    
    // For detailed SIP statistics
    RvSipMessageRetransmitEv                pfnRetransmitEvHandler;
#else
    RvSipMessageRetransmitEv                pfnRetransmitEvHandler;
    RvSipMessageRetransmitRcvdEv            pfnRetransmitRcvdEvHandler;
#endif    
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */


} TransactionMgr;


/* TransactionMgrHashKey
 * ---------------------------------------------------------------------------
 */

typedef struct {
    RPOOL_Ptr              *pCallId;
    RvSipPartyHeaderHandle *hFrom;
    RvSipPartyHeaderHandle *hTo;
    RvInt32               cseqStep;
    RvBool                bIsUac;
    RvBool                bSendRequest; /* This parameter is TRUE if the hash key is of
                                            a client transaction that sends a request
                                        (before inserting the transaction to the hash) */
    SipTransactionMethod   eMethod;
    RvChar               *strMethod;
    TransactionMgr        *pMgr;
    RvSipAddressHandle     hRequestUri;
    RvSipViaHeaderHandle  *hTopVia;
    RvUint16             *pRespCode;
#ifndef RV_SIP_PRIMITIVES
    RvUint32             *rseqVal;   /*used to match prack transaction only*/
#endif/* #ifndef RV_SIP_PRIMITIVES*/
    RvBool               *bIsProxy;   /*used to match proxy transaction only*/
    RvBool               *bAllowCancellation; /* This flag indicates that the transc  */
                                               /* already associated locks with its    */
                                               /* call-leg in case it's owned by any   */
    RvBool               *bAllowAckHandling; /* This flag indicates that the transaction
                                             sent 2xx (on new invite handling) and remained
                                             alive only for catching the request retransmissions.
                                             This transaction should not handle the ACK request */
} TransactionMgrHashKey;

/***************************************************************************
 * TransactionMgrInitialize
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
 * Input:   pTranscMgr      - The new transaction manager object.
 *          pCfg            - Managers parameters as were defined in the
 *                             configuration.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionMgrInitialize(
                              IN     TransactionMgr            *pTranscMgr,
                              INOUT  SipTranscMgrConfigParams  *pCfg);


/***************************************************************************
 * TransactionMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destruct the transaction manager freeing all resources.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr          - The transaction manager object.
 ***************************************************************************/
void RVCALLCONV TransactionMgrDestruct(
                              IN     TransactionMgr            *pTranscMgr);


/***************************************************************************
 * TransactionMgrHashInsert
 * ------------------------------------------------------------------------
 * General: Insert a transaction into the hash table.
 *          The key is generated from the transaction information and the
 *          transaction handle is the data.
 * Return Value: RV_ERROR_OUTOFRESOURCES - No more resources.
 *               RV_OK - transaction handle was inserted into hash
 *                            successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - A handle to the transaction to insert to the hash
 ***************************************************************************/
RvStatus RVCALLCONV TransactionMgrHashInsert(RvSipTranscHandle hTransc);

/***************************************************************************
 * TransactionMgrHashRemove
 * ------------------------------------------------------------------------
 * General: Removes a transaction from the hash table.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc - A handle to the transaction to remove from the hash
 ***************************************************************************/
void RVCALLCONV TransactionMgrHashRemove(RvSipTranscHandle hTransc);


/***************************************************************************
 * TransactionMgrHashFind
 * ------------------------------------------------------------------------
 * General: find a transaction in the hash table acording to the transaction
 *          hash key.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr -       Handle of the transaction manager
 *          pKey -       The transaction hash key
 *          eMethod-     Method for calling the right comparison function.
 * Output:  phTransaction - Handle to the transaction found in the hash or null
 *                          if the transaction was not found.
 ***************************************************************************/
void RVCALLCONV TransactionMgrHashFind(
                           IN  TransactionMgr         *pMgr,
                           IN  TransactionMgrHashKey  *pKey,
                           OUT RvSipTranscHandle      *phTransaction,
                           RvSipMethodType             eMethod);

/***************************************************************************
 * TransactionMgrHashFindCancelledTransc
 * ------------------------------------------------------------------------
 * General: find a transaction that a cancel transaction is cancelling.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr -       Handle of the transaction manager
 *          pCancelTransc - The cancel transaction.
 * Output:  phTransaction - Handle to the transaction found in the hash or null
 *                          if the transaction was not found.
 ***************************************************************************/
void RVCALLCONV TransactionMgrHashFindCancelledTransc(
                           IN  TransactionMgr        *pMgr,
                           IN  RvSipTranscHandle     hCancelTransc,
                           OUT RvSipTranscHandle     *phTransaction);

/***************************************************************************
 * TransactionMgrHashFindCancelledTranscByMsg
 * ------------------------------------------------------------------------
 * General: find a transaction that a cancel msg is supposed to cancel.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr -       Handle of the transaction manager
 *          hCancelMsg - The cancel message
 *          pTranscKey - A transaction key retrieved from the message
 * Output:  phTransaction - Handle to the transaction found in the hash or null
 *                          if the transaction was not found.
 ***************************************************************************/
void RVCALLCONV TransactionMgrHashFindCancelledTranscByMsg(
                           IN  TransactionMgr        *pMgr,
                           IN  RvSipMsgHandle         hCancelMsg,
                           IN  SipTransactionKey     *pTranscKey,
                           OUT RvSipTranscHandle     *phTransaction);
/***************************************************************************
 * TransactionMgrHashFindMergedTransc
 * ------------------------------------------------------------------------
 * General: find a transaction with the same request identifiers:
 *          from-tag, call-id, and cseq, and the transaction is on ongoing state.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr -       Handle of the transaction manager
 *          pTranscKey - A transaction key retrieved from the message
 * Output:  phTransaction - Handle to the transaction found in the hash or null
 *                          if the transaction was not found.
 ***************************************************************************/
void RVCALLCONV TransactionMgrHashFindMergedTransc(
                           IN  TransactionMgr        *pMgr,
                           IN  RvSipMsgHandle        hReqMsg,
                           IN  SipTransactionKey     *pTranscKey,
                           OUT RvSipTranscHandle     *phTransaction);


#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * TransactionMgrHashFindMatchPrackTransc
 * ------------------------------------------------------------------------
 * General: find a transaction that a prack transaction related to.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr -       Handle of the transaction manager
 *          hPrackTransc - The cancel transaction.
 *          hPrackMsg - The prack message
 * Output:  phPrackMatch - Handle to the transaction found in the hash or null
 *                          if the transaction was not found.
 ***************************************************************************/
void RVCALLCONV TransactionMgrHashFindMatchPrackTransc(
                           IN  TransactionMgr        *pMgr,
                           IN  RvSipTranscHandle     hPrackTransc,
                           IN RvSipMsgHandle         hPrackMsg,
                           OUT RvSipTranscHandle     *phPrackMatch);
#endif /*#ifndef RV_SIP_PRIMITIVES*/

/***************************************************************************
 * TransactionMgrCreateTransaction
 * ------------------------------------------------------------------------
 * General: Creates and Initialize a new transaction object.
 * Return Value: RV_OK - The transaction was successfully inserted to
 *                              the data-structure.
 *                 RV_ERROR_NULLPTR - The pointer to the transaction that needs
 *                                 to be managed is a NULL pointer, or the
 *                                 pointer to the transaction manager is a
 *                                 NULL pointer.
 *                 RV_ERROR_OUTOFRESOURCES - The data-structure is full.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pTransc - The transaction to be managed by the transaction
 *                         manager, in other words the transaction that will
 *                         be inserted to the data-structure.
 *          b_isUac    - RV_TRUE if the transaction is UAC. RV_FALSE for UAS.
 *          pTranscMgr -   The transactions manager.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionMgrCreateTransaction(
                                     IN  TransactionMgr    *pTranscMgr,
                                     IN  RvBool           b_isUac,
                                     OUT RvSipTranscHandle *hTransc);

/***************************************************************************
 * TransactionMgrFreeTransaction
 * ------------------------------------------------------------------------
 * General: Deletes the transaction from the transactions data - structure.
 * (This interface function will be used only by the transaction object).
 * Return Value: RV_OK - The transaction was successfully removed from
 *                              the data-structure.
 *                 RV_ERROR_NULLPTR - The pointer to the transaction that needs
 *                                 to be unmanaged is a NULL pointer, or the
 *                                 pointer to the transaction manager is a
 *                                 NULL pointer.
 *                 RV_ERROR_NOT_FOUND - The transaction to be removed was not found
 *                               in the data structure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pTransc - The transaction to be unmanaged by the transaction
 *                         manager, in other words the transaction that will
 *                           be deleted from the data-structure.
 *          pTranscMgr -   The transactions manager.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionMgrFreeTransaction(
                                             IN TransactionMgr  *pTranscMgr,
                                             IN RvSipTranscHandle  hTransc);



/***************************************************************************
 * TransactionMgrCheckIfExists
 * ------------------------------------------------------------------------
 * General: Checks if there exist a transaction that has the same key as the
 *          received key.
 * Return Value: RV_TRUE - The transaction exists.
 *               RV_FALSE - The transaction does not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    pTranscMgr - The transaction manager in which to look for the
 *                       transaction.
 *          pKey - The transaction's key.
 *          eMethod - The transaction enumeration method.
 *          strMethod - The transaction string method.
 *          bIsUac - Is the transaction Uac.
 *          hMsg - message handle to retrive request method and top via.
 ***************************************************************************/
RvBool RVCALLCONV TransactionMgrCheckIfExists(
                                        IN TransactionMgr        *pTranscMgr,
                                        IN SipTransactionKey     *pKey,
                                        IN SipTransactionMethod eMethod,
                                        IN RvChar               *strMethod,
                                        IN RvBool                bIsUac,
                                        IN RvSipMsgHandle        hMsg);

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * TransactionMgrFreeUnusedSigCompResources
 * ------------------------------------------------------------------------
 * General: Frees unused SigComp resources.
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTranscMgr     - The transaction manager.
 *          pSigCompInfo   - SigComp information structure.
 ***************************************************************************/
void RVCALLCONV TransactionMgrFreeUnusedSigCompResources(
                           IN TransactionMgr             *pTranscMgr,
                           IN SipTransportSigCompMsgInfo *pSigCompInfo);
#endif /* RV_SIGCOMP_ON */

#endif /*#ifndef RV_SIP_LIGHT*/
#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef TRANSACTION_MGR_OBJECT_H */
