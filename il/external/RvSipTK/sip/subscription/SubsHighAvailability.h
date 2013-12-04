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
 *                              <SubsHighAvailability.c>
 *
 *  The following functions are for High Availability use. It means that
 *  the application can store all the nessecary details of an active subscription
 *  and restore it later to another list of subscriptions.
 *
 *    Author                         Date
 *    ------                        ------
 *  Dikla Dror                      Mar 2004
 *********************************************************************************/

#ifndef SUBS_HIGH_AVAILABILITY_H
#define SUBS_HIGH_AVAILABILITY_H

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifndef RV_SIP_PRIMITIVES
#if defined(RV_SIP_SUBS_ON) && defined(RV_SIP_HIGHAVAL_ON)

#include "RvSipCallLeg.h"
#include "SubsObject.h"

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SubsGetActiveSubsStorageSize
 * ------------------------------------------------------------------------
 * General: Gets the size of buffer needed to store all parameters of active subs.
 *          (The size of buffer, that should be supply in RvSipSubsStoreActiveSubs()).
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the pSubs is invalid.
 *              RV_ERROR_NULLPTR        - pSubs or len is a bad pointer.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems - didn't manage to encode.
 *              RV_OK                   - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSubs          - Pointer to the subscription.
 *          bStandAlone    - Indicates if the Subs is independent.
 *                           This parameter should be RV_FALSE in case
 *                           that a CallLeg object retrieves its subscription
 *                           storage size, in order to avoid infinity recursion.
 * Output:  pLen           - size of buffer. will be -1 in case of failure.
 ***************************************************************************/
RvStatus SubsGetActiveSubsStorageSize(IN  RvSipSubsHandle hSubs,
                                      IN  RvBool          bStandAlone,
                                      OUT RvInt32        *pLen);

/***************************************************************************
 * SubsStoreActiveSubs
 * ------------------------------------------------------------------------
 * General: Gets all the important parameters of an already active subs.
 *          The subs must be in state active.
 *          User must give an empty page that will be fullfill with the information.
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the pSubs is invalid.
 *              RV_ERROR_NULLPTR        - pSubs or len is a bad pointer.
 *              RV_ERROR_ILLEGAL_ACTION - If the state is not Active.
 *              RV_ERROR_INSUFFICIENT_BUFFER - The buffer is too small.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems.
 *              RV_OK                   - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSubs          - Pointer to the subscription.
 *          bStandAlone    - Indicates if the Subs is independent.
 *                           This parameter should be RV_FALSE if
 *                           a CallLeg object asks for its subscription
 *                           storage, in order to avoid infinity recursion.
 *          buffLen        - The length of the given buffer.
 * Output:  memBuff        - The buffer that will store the Subs' parameters.
 *          pStoredLen     - The length of the stored data in the membuff.
 ***************************************************************************/
RvStatus SubsStoreActiveSubs(IN  RvSipSubsHandle  hSubs,
                             IN  RvBool           bStandAlone,
                             IN  RvUint32         buffLen,
                             OUT void            *memBuff,
                             OUT RvUint32        *pStoredLen);

/***************************************************************************
 * SubsRestoreActiveSubs
 * ------------------------------------------------------------------------
 * General: Create a new Subs on state Active, and fill all it's parameters,
 *          which are strored on the given page. (the page was updated in the
 *          SubsStoreActiveSubs function).
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the pSubs is invalid.
 *              RV_ERROR_NULLPTR    - Bad pointer to the memPool or params structure.
 *              RV_ERROR_ILLEGAL_ACTION - If the state is not Active.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems.
 *              RV_OK       - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: memBuff        - The buffer that stores all the Subs' parameters.
 *        buffLen        - The buffer length.
 *        bStandAlone    - Indicates if the Subs is independent.
 *                         This parameter should be RV_FALSE if
 *                         a CallLeg object asks for its subscription
 *                         restoration, in order to avoid infinity recursion.
 * Input/Output:
 *        pSubs   - Pointer to the restored subscription.
 ***************************************************************************/
RvStatus SubsRestoreActiveSubs(IN    void           *memBuff,
                               IN    RvUint32        buffLen,
                               IN    RvBool          bStandAlone,
                               INOUT Subscription   *pSubs);

#endif /* #if defined(RV_SIP_SUBS_ON) && defined(RV_SIP_HIGHAVAL_ON) */
#endif /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SUBS_HIGH_AVAILABILITY_H*/


