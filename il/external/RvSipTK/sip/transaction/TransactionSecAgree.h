
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
 *                              <TransactionSecAgree.h>
 *
 * In this file the transaction attaches to a security agreement-object and uses it 
 * to obtain and maintain a security-agreement according to RFC3329.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Apr 2001
 *********************************************************************************/

#ifndef TRANSACTION_SEC_AGREE_H
#define TRANSACTION_SEC_AGREE_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIP_IMS_ON
#include "RvSipTransactionTypes.h"
#include "TransactionObject.h"
#include "RvSipSecAgreeTypes.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TransactionSecAgreeAttachSecAgree
 * ------------------------------------------------------------------------
 * General: Attaches a security-agreement object to this transaction. 
 *          If this security-agreement object represents an agreement that
 *          was obtained with the remote party, the transaction will exploit this 
 *          agreement and the data it brings. If not, the transaction will use this 
 *          security-agreement to negotiate with the remote party.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransaction   - Pointer to the transaction.
 *          hSecAgree  - Handle to the security-agreement.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSecAgreeAttachSecAgree(
								IN  Transaction                 *pTransaction,
								IN  RvSipSecAgreeHandle          hSecAgree);

/***************************************************************************
 * TransactionSecAgreeDetachSecAgree
 * ------------------------------------------------------------------------
 * General: Detaches the security-agreement object of this transaction. 
 *          This security-agreement is no longer used.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransaction         - Pointer to the transaction.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSecAgreeDetachSecAgree(
										IN  Transaction            *pTransaction);

/***************************************************************************
 * TransactionSecAgreeMsgToSend
 * ------------------------------------------------------------------------
 * General: Processes a message that is about to be sent by the transaction:
 *          adds the required security information.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransaction         - Pointer to the transaction.
 *          hMsg             - Handle to the message about to be sent.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSecAgreeMsgToSend(
											IN  Transaction                 *pTransaction,
											IN  RvSipMsgHandle               hMsg);

/***************************************************************************
 * TransactionSecAgreeDestResolved
 * ------------------------------------------------------------------------
 * General: Notified the security-agreement object that destination was 
 *          resolved. The security-agreement will modify the message and 
 *          its inner state accordingly.
 *          
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransaction        - Pointer to the transaction.
 *          hMsg            - Handle to the message
 *          hTrx            - The transmitter that notified destination resolution
 ***************************************************************************/
void RVCALLCONV TransactionSecAgreeDestResolved(
								IN  Transaction                  *pTransaction,
						        IN  RvSipMsgHandle                hMsg,
							    IN  RvSipTransmitterHandle        hTrx);

/***************************************************************************
 * TransactionSecAgreeMsgSendFailure
 * ------------------------------------------------------------------------
 * General: Processes a message send failure in the transaction:
 *          invalidates security-agreement.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransc      - Pointer to the transaction.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSecAgreeMsgSendFailure(
											IN  Transaction                 *pTransc);

/***************************************************************************
 * TransactionSecAgreeMsgRcvd
 * ------------------------------------------------------------------------
 * General: Processes a message being received by the transaction:
 *          loads the required security information and verifies the security
 *          agreement validity.
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransaction         - Pointer to the transaction.
 *          hMsg             - Handle to the message received.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSecAgreeMsgRcvd(
										IN  Transaction                 *pTransaction,
										IN  RvSipMsgHandle               hMsg);


/*-----------------------------------------------------------------------*/
/*                          CALLBACKS                                    */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * TransactionSecAgreeAttachSecObjEv
 * ------------------------------------------------------------------------
 * General: Attaches a security object to this transaction. 
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hOwner          - Handle to the transaction.
 *         hSecAgree       - Handle to the security-agreement object.
 *         hSecObj         - Handle to the security object
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSecAgreeAttachSecObjEv(
										IN  void*                      hOwner,
										IN  RvSipSecAgreeHandle        hSecAgree,
										IN  RvSipSecObjHandle          hSecObj);

/***************************************************************************
 * TransactionSecAgreeDetachSecAgreeEv
 * ------------------------------------------------------------------------
 * General: Detaches a security-agreement object from this transaction. 
 *          
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hOwner          - Handle to the transaction.
 *         hSecAgree       - Handle to the security-agreement object.
 ***************************************************************************/
RvStatus RVCALLCONV TransactionSecAgreeDetachSecAgreeEv(
										IN  void*                      hOwner,
										IN  RvSipSecAgreeHandle        hSecAgree);

#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif


#endif /* END OF: ifdef TRANSACTION_SEC_AGREE_H */


