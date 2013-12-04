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
 *                              <SipTransactionTypes.h>
 *
 * This file defines handles of the Transaction and transaction manager objects.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                  Nov 2000
 *********************************************************************************/


#ifndef SIP_TRANSACTION_TYPES_H
#define SIP_TRANSACTION_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
/* Timers - the values are here (before the #ifndef RV_SIP_LIGHT) because 
   the stack mgr uses it on SIP_LIGHT mode too. */
#define DEFAULT_T1                     500
#define DEFAULT_T2                     4000
#define DEFAULT_T3                     16000
#define DEFAULT_T4                     5000
#define DEFAULT_PROVISIONAL_TIMER      180000
#define DEFAULT_INVITE_LINGER_TIMER    32000



#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "RvSipMsg.h"
#include "RvSipPartyHeader.h"
#include "RvSipTransactionTypes.h"
#include "AdsRpool.h"
#include "AdsRlist.h"
#include "_SipTransportTypes.h"
#include "RvSipAuthenticator.h"
#include "_SipCommonTypes.h"
#include "RvSipCompartmentTypes.h"
#include "rvlog.h"
#include "RvSipTransmitterTypes.h"

#ifdef RV_SIP_IMS_ON
#include "RvSipSecurityTypes.h"
#include "RvSipSecAgreeTypes.h"
#endif /* #ifdef RV_SIP_IMS_ON */

/*-----------------------------------------------------------------------*/
/*                            MACRO DEFINITIONS                          */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                            TYPE DEFINITIONS                           */
/*-----------------------------------------------------------------------*/

/* SipTransactionOwner
 * ---------------------------------
 * A transaction object can be part of a call, part of a registration
 * process, and also can function as an independent entity. This
 * enumeration will indicate the owner of the transaction.
 */
typedef enum
{
    SIP_TRANSACTION_OWNER_UNDEFINED = -1,
    SIP_TRANSACTION_OWNER_NON,
    SIP_TRANSACTION_OWNER_CALL,
    SIP_TRANSACTION_OWNER_TRANSC_CLIENT,
    SIP_TRANSACTION_OWNER_APP

} SipTransactionOwner;

/* SipTransactionMsgSentOption
 * ---------------------------------
 * extra information for SipTransactionInviteTranscMsgSentEv
 */
typedef enum
{
    SIP_TRANSACTION_MSG_SENT_OPTION_UNDEFINED = -1,
    SIP_TRANSACTION_MSG_SENT_OPTION_2XX,
    SIP_TRANSACTION_MSG_SENT_OPTION_ACK
}SipTransactionMsgSentOption;


/* SipTransactionKey
 * ---------------------------------
 * A There is a 1-1 match between a transaction and its key.
 * This match is done under the rules of comparison as defined for From
 * and To headers.
 */
typedef struct
{
    RvSipPartyHeaderHandle       hFromHeader; /* The transaction's From header.*/
    RvSipPartyHeaderHandle       hToHeader;   /* The transaction's To header.*/
    RvInt32                     cseqStep;    /* The transaction's CSeq-Step value.*/
    RPOOL_Ptr                    strCallId;   /* The transaction's Call-Id value.*/
    RvBool                      bIgnoreLocalAddr; /*indicates whether to use the key local addr*/
    SipTransportObjLocalAddresses        localAddr;   /* The local address to send messages from */

} SipTransactionKey;




#ifdef SIP_DEBUG
/* TranscationMgrStatistics
 * ------------------------
 * This structure manages the statiscs of sent and received messages
 */
typedef struct
{
    /* Total INVITE messages received (including retransmission) */
    RvUint32 rcvdINVITE;
    /* INVITE retransmissions received */
    RvUint32 rcvdINVITERetrans;
    /* Total non-INVITE requests received (including retransmission) */
    RvUint32 rcvdNonInviteReq;
    /* non-INVITE retransmissions received */
    RvUint32 rcvdNonInviteReqRetrans;
    /* Total responses received */
    RvUint32 rcvdResponse;
    /* responses retransmissions received */
    RvUint32 rcvdResponseRetrans;
    /* Total INVITE messages sent (including retransmission) */
    RvUint32 sentINVITE;
    /* INVITE retransmissions sent */
    RvUint32 sentINVITERetrans;
    /* Total non-INVITE requests sent (including retransmission) */
    RvUint32 sentNonInviteReq;
    /* non-INVITE request retransmissions sent */
    RvUint32 sentNonInviteReqRetrans;
    /* Total responses sent */
    RvUint32 sentResponse;
    /* Response retransmissions sent */
    RvUint32 sentResponseRetrans;

} SipTransactionMgrStatistics;
#endif


/* SipTransactionMethod
 * ---------------------------
 * A transaction method defines the method that will be used in the
 * transaction messages. The  RVSIP_TRANSACTION_METHOD_OTHER method
 * is used for methods not found in this enumeration.
 * The methods are enumerated bellow.
 */
typedef enum
{
    SIP_TRANSACTION_METHOD_UNDEFINED = -1,
    SIP_TRANSACTION_METHOD_INVITE,
    SIP_TRANSACTION_METHOD_BYE,
    SIP_TRANSACTION_METHOD_REGISTER,
#ifdef RV_SIP_SUBS_ON
    SIP_TRANSACTION_METHOD_REFER,
    SIP_TRANSACTION_METHOD_NOTIFY,
	SIP_TRANSACTION_METHOD_SUBSCRIBE,
#endif /* RV_SIP_SUBS_ON */
    SIP_TRANSACTION_METHOD_CANCEL,
#ifndef RV_SIP_PRIMITIVES
    SIP_TRANSACTION_METHOD_PRACK,
#endif /* #ifndef RV_SIP_PRIMITIVES */
    SIP_TRANSACTION_METHOD_OTHER

} SipTransactionMethod;


/* SipTransactionMgrTranscCreatedEv
 * ---------------------------------------
 * Called when a new transaction is created by the transaction's manager.
 * The Input parameter is the new transaction created - hTransc.
 * The Output parameters are:
 * pOwner - The new owner of the transaction, NULL if there
 *                         is no owner.
 * hLock: - A pointer to the lock of the new owner.
 * pKey: - The transactions key (To, From, CSeq-Step and Call-Id).
 */
typedef void (RVCALLCONV *SipTransactionMgrTranscCreatedEv)
                    (IN    void               *pMgr,
                     IN    RvSipTranscHandle  hTransc,
                     INOUT SipTransactionKey  *pKey,
                     OUT   void               **pOwner,
                     OUT   SipTripleLock      **tripleLock,
                     OUT   RvUint16           *statusCode);


/* SipTransactionInternalClientCreatedEv
 * ---------------------------------------
 * Called when a new client transaction is internally created. For example
 * when a new CANCEL or PRACK transactions are created.
 * The Input parameter is the new transaction created. Such a transaction
 * has the owner of the transaction that triggered the transaction creation.
 * hTransc:      - A pointer to new transaction.
 * hTranscOwner: - The transactions owner.
 */
typedef RvStatus (RVCALLCONV *SipTransactionMgrInternalClientCreatedEv)
                     (IN RvSipTranscHandle      hTransc,
                      IN RvSipTranscOwnerHandle hTranscOwner);

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * SipTransactionRelProvRespRcvdEv
 * ----------------------------------
 * General: Called when a reliable provisional response is received.
 *          Gives call-leg layer the 1xx message, so call-leg will send
 *          the PRACK request.
 * ------------------------------------------------------------------------
 * Arguments:
 * hTransc:      - A pointer to new transaction.
 * hTranscOwner: - The transactions owner.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *SipTransactionRelProvRespRcvdEv)
                     (IN RvSipTranscHandle      hTransc,
                      IN RvSipTranscOwnerHandle hTranscOwner,
                      IN RvUint32               rseqStep);

/***************************************************************************
 * SipTransactionInviteTranscMsgSentEv
 * ----------------------------------
 * General: Notifies the call-leg that the invite transaction sent a msg.
 *          1. On first 2xx sending, when working with new-invite handling. 
 *          2. On non-2xx Ack sending, when working with new-invite handling.
 * ------------------------------------------------------------------------
 * Arguments:
 * hTransc:      - A pointer to new transaction.
 * hTranscOwner: - The transactions owner.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV* SipTransactionInviteTranscMsgSentEv)
                        (IN RvSipTranscHandle      hTransc,
                         IN RvSipTranscOwnerHandle hTranscOwner,
                         IN SipTransactionMsgSentOption   eOption);

/***************************************************************************
 * SipTransactionIgnoreRelProvRespEv
 * ----------------------------------
 * General: Called when a reliable provisional response is received.
 *          the transaction need to consult the call-leg if the rseq value
 *          in the 1xx message is legal. if not, this 1xx request should be
 *          ignored.
 * ------------------------------------------------------------------------
 * Arguments:
 * hTransc:      - A pointer to new transaction.
 * hTranscOwner: - The transactions owner.
 ***************************************************************************/
typedef void (RVCALLCONV *SipTransactionIgnoreRelProvRespEv)
                     (IN RvSipTranscHandle      hTransc,
                      IN RvSipTranscOwnerHandle hTranscOwner,
                      IN RvUint32               rseqStep,
                      OUT RvBool               *pbIgnore);

/***************************************************************************
 * SipTransactionMgrForkedInviteResponseEv
 * ------------------------------------------------------------------------
 * General: Called when the transaction manager receives a new 1xx/2xx response
 *          for INVITE, and the response is not related to any transaction.
 *          (This is usually the case when INVITE request was forked by proxy
 *          to several UAS, and each one of them returns 1xx and 2xx).
 * Return Value: RvStatus - ignored in this version.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr   - The transaction manager that received the response.
 *          hResponseMsg - The handle to the received response message.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *SipTransactionMgrForkedInviteResponseEv)
                    (IN  void*               hCallLegMgr,
                     IN  RvSipMsgHandle      hResponseMsg,
                     OUT RvBool*             pbWasHandled);



/***************************************************************************
 * SipTransactionMgrInviteResponseNoTranscEv
 * ------------------------------------------------------------------------
 * General: Called when the transaction manager receives a new 1xx/2xx response
 *          for INVITE, and the response is not related to any transaction.
 *          The callback will be called in the following cases:
 *          1. The INVITE request was forked by a forking proxy
 *          to several UAS, and each one of them returns 1xx and 2xx responses.
 *          2. For 2xx retransmissions since the first 2xx terminates the
 *          transaction.
 * Return Value: RvStatus - ignored in this version.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr    - The transaction manager handle.
 *          hResponseMsg  - The handle to the received response message.
 *          pRcvdFromAddr - The address that the response was received from
 * Output:  pbWasHandled  - Indicates if the message was handled in the
 *                          callback.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *SipTransactionMgrInviteResponseNoTranscEv)
                    (IN   void*                   hCallLegMgr,
                     IN   RvSipMsgHandle          hResponseMsg,
					 IN   SipTransportAddress*    pRcvdFromAddr,
                     OUT  RvBool*                 pbWasHandled);

/***************************************************************************
 * SipTransactionMgrAckNoTranscEv
 * ------------------------------------------------------------------------
 * General: Called when the transaction manager receives an ACK request
 *          that does not match any transaction. This will be the case
 *          when the INVITE was responded with 2xx response and was terminated
 *          immediately after that.
 * Return Value: RV_Status - ignored in this version.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTranscMgr    - The transaction manager handle.
 *          hAckMsg       - The handle to the received ACK message.
 *          pRcvdFromAddr - The address that the ACK was received from
 * Output:  pbWasHandled  - Indicates if the message was handled in the
 *                          callback.
 ***************************************************************************/
typedef RvStatus (RVCALLCONV *SipTransactionMgrAckNoTranscEv)
                    (IN   void*                   hCallLegMgr,
                     IN   RvSipMsgHandle          hAckMsg,
					 IN   SipTransportAddress*    pRcvdFromAddr,
                     OUT  RvBool*                 pbWasHandled);

#endif /*RV_SIP_PRIMITIVES */

/* SipTranscMgrConfigParams
 * --------------------------------------
 * The transaction manager requires several configuration parameters:
 * MaxTransactionNumber : The maximum number of transactions this
 *                          manager will need to hold at once.
 * T1 : The shorter timer interval as was defined in the protocol.
 *        Has a default of 500ms.
 * T2 : The longer timer interval as was defined in the protocol.
 *        Has a default of 4s.
 * T4 : A timer defined in the protocol.
 *      Has a default of 5000ms.
 */
typedef struct
{
    RvLogMgr*              pLogMgr;
    void*                  hStack;
    HRPOOL                   hMessagePool;
    HRPOOL                   hGeneralPool;
    RvLogSource*           pLogSrc;
    RvSipTransportMgrHandle hTransportHandle;
    RvSipMsgMgrHandle       hMsgMgr;
    RvInt32               maxTransactionNumber;
    RvUint32              maxPageNumber;
    RvUint32              T1;
    RvUint32              T2;
    RvUint32              T4;
    RvInt32               genLinger;
    RvInt32               inviteLinger;
    RvInt32               provisional;
    RvInt32               cancelInviteNoResponseTimer;
    RvInt32               cancelGeneralNoResponseTimer;
    RvInt32               generalRequestTimeoutTimer;
    RvBool                enableInviteProceedingTimeoutState;
#ifndef RV_SIP_PRIMITIVES
    RvSipTransaction100RelStatus status100rel;
    RvBool                rejectUnsupportedExtensions;
    RvBool                addSupportedListToMsg;
#endif /*#ifndef RV_SIP_PRIMITIVES*/
    RvBool                isProxy;
    RvInt32               proxy2xxSentTimer;
    RvInt32               proxy2xxRcvdTimer;
#ifdef RV_SIP_AUTH_ON
    RvSipAuthenticatorHandle hAuthMgr;
    RvBool                   enableServerAuth;
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifndef RV_SIP_PRIMITIVES
    RvInt32               maxSubscriptions;
    RvChar                **supportedExtensionList;
    RvInt32               extensionListSize;
    RvBool                manualPrack;
#endif /* #ifndef RV_SIP_PRIMITIVES*/
    RvBool                manualBehavior; /* manual 487 on cancelled */
    RvBool                bIsPersistent;
    RvBool                bDynamicInviteHandling;
    RvRandomGenerator     *seed;

    /*new parameters for version 3.1*/
#ifdef RV_SIGCOMP_ON
    RvInt32                     sigCompTcpTimer;
    RvSipCompartmentMgrHandle   hCompartmentMgr;
#endif /* RV_SIGCOMP_ON */
    RvSipTransmitterMgrHandle   hTrxMgr;
    RvBool                      bDisableMerging;
    RvBool                      bEnableForking;
    RvBool                      bOldInviteHandling;
#ifdef RV_SIP_IMS_ON
    RvSipSecMgrHandle           hSecMgr;
#endif

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
    RvSipMessageRetransmitEv    pfnRetransmitEvHandler;
#else
    RvSipMessageRetransmitEv        pfnRetransmitEvHandler;
    RvSipMessageRetransmitRcvdEv    pfnRetransmitRcvdEvHandler;
#endif    
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

 
} SipTranscMgrConfigParams;


/* SipTransactionMgrEvHandlers
 * --------------------------------------
 * When a Call manager or any other manager who is working with
 * the transaction manager, has to attach to it, both itself and
 * its implementation for the transaction created callback, using
 * this struct.
 */
typedef struct
{
    SipTransactionMgrTranscCreatedEv           pfnEvTransactionCreated;
#ifndef RV_SIP_PRIMITIVES
    SipTransactionRelProvRespRcvdEv            pfnRelProvRespRcvd;
    SipTransactionIgnoreRelProvRespEv          pfnIgnoreRelProvRespRcvd;
    SipTransactionMgrInviteResponseNoTranscEv  pfnInviteResponseNoTranscRcvd;
    SipTransactionMgrAckNoTranscEv             pfnAckNoTranscRcvd;
    SipTransactionInviteTranscMsgSentEv        pfnInviteTranscMsgSent;
#endif /* RV_SIP_PRIMITIVES */
    void                                    *pMgr;

} SipTransactionMgrEvHandlers;

#endif /*#ifndef RV_SIP_LIGHT*/


#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef SIP_TRANSACTION_TYPES_H */


