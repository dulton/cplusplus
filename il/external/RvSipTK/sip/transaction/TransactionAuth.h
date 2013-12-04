
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
 *                              <TransactionAuth.h>
 *  Handles all server authentication functions
 *
 *    Author                         Date
 *    ------                        ------
 *   Ofra Wachsman                  2.02
 *********************************************************************************/


#ifndef TRANSACTION_AUTH_H
#define TRANSACTION_AUTH_H

#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"

#ifdef RV_SIP_AUTH_ON

#include "RvSipTransactionTypes.h"
#include "TransactionObject.h"


/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TransactionAuthSetAuthorizationList
 * ------------------------------------------------------------------------
 * General: Go over the message authorization headers and add them
 *          to the authorization headers list in the transaction.
 *          We assume that the list was not set yet, because this function
 *          should be called only once, for every transaction.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pTransc - The transaction handle.
 *          pMsg - The INVITE received message.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAuthSetAuthorizationList(
                                            IN Transaction   *pTransc,
                                            IN RvSipMsgHandle hMsg);

/***************************************************************************
 * TransactionAuthBegin
 * ------------------------------------------------------------------------
 * General: Start the server authentication proccess.
 *          if there are authorization headers in the transaction list,
 *          we will supply it to application, so it can call to authenticateHeader
 *          function.
 *          if there are none, we can call to the authCompletedEv, notifing that
 *          the authentication procedure had finished with failure.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransc -        The transaction handle.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAuthBegin(IN RvSipTranscHandle hTransc);

/***************************************************************************
 * TransactionAuthProceed
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
 *          - user wants to stop the loop that searchs for authorization headers.
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
 * Input:   hTransc -        The transaction handle.
 *          action -         In which action to proceed. (see above)
 *          hAuthorization - Handle of the authorization header that the function
 *                           will check authentication for. (needed if action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD, else NULL.)
 *          password -       The password for the realm+userName in the header.
 *                           (needed if action is
 *                           RVSIP_TRANSC_AUTH_ACTION_USE_PASSWORD, else NULL.)
 *          eObjectType    - type of the object to be authenticated.
 *          pObjTripleLock - lock of the object. Can be NULL. If it is
 *                           supplied, it will be unlocked before callbacks
 *                           to application and locked again after them.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAuthProceed(
                            IN  RvSipTranscHandle              hTransc,
                            IN  RvSipTransactionAuthAction     action,
                            IN  RvSipAuthorizationHeaderHandle hAuthorization,
                            IN  RvChar*                        password,
                            IN  void*                          hObject,
                            IN  RvSipCommonStackObjectType     eObjectType,
                            IN  SipTripleLock*                 pObjTripleLock);

/***************************************************************************
 * TransactionAuthRespondUnauthenticated
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
 * output:   (-)
 ***************************************************************************/
RvStatus RVCALLCONV TransactionAuthRespondUnauthenticated(
                                                           IN  Transaction*         pTransc,
                                                           IN  RvUint16            responseCode,
                                                           IN  RvChar              *strReasonPhrase,
                                                           IN  RvSipHeaderType      headerType,
                                                           IN  void*                hHeader,
                                                           IN  RvChar              *realm,
                                                           IN  RvChar              *domain,
                                                           IN  RvChar              *nonce,
                                                           IN  RvChar              *opaque,
                                                           IN  RvBool              stale,
                                                           IN  RvSipAuthAlgorithm   eAlgorithm,
                                                           IN  RvChar              *strAlgorithm,
                                                           IN  RvSipAuthQopOption   eQop,
                                                           IN  RvChar              *strQop);


#endif /* #ifdef RV_SIP_AUTH_ON */
#ifdef __cplusplus
}
#endif

#endif /*END OF: #ifndef TRANSACTION_AUTH_H*/



