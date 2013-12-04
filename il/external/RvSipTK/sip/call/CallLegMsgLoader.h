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
 *                              <CallLegMsgLoader.h>
 *
 *  The message loader supply functions to load message information into
 *  the call-leg object.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Jun 2001
 *********************************************************************************/

#ifndef CALL_LEG_MSG_LOADER_H
#define CALL_LEG_MSG_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
#include "CallLegObject.h"
#include "RvSipMsg.h"



/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/




/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CallLegMsgLoaderLoadRequestUriToLocalContact
 * ------------------------------------------------------------------------
 * General: This function loads the request URI from the message to the
 *          call-leg local contact address field.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Poiner to the call-leg
 *            hMsg     - Handle to the message.
 ***************************************************************************/
void RVCALLCONV CallLegMsgLoaderLoadRequestUriToLocalContact(
                                 IN  CallLeg                *pCallLeg,
                                 IN  RvSipMsgHandle          hMsg);



/***************************************************************************
 * CallLegMsgLoaderLoadContactToRedirectAddress
 * ------------------------------------------------------------------------
 * General: Load the contact address from a 3xx message to the redirect addr
 *          field in the call-leg. This address will be used as the request
 *          URI in the invite if the user wishes to redirect the call.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            hMsg     - Handle to the message.
 ***************************************************************************/
void RVCALLCONV CallLegMsgLoaderLoadContactToRedirectAddress(
                                  IN  CallLeg                 *pCallLeg,
                                  IN  RvSipMsgHandle          hMsg);


/***************************************************************************
 * CallLegMsgLoaderLoadContactToRemoteContact
 * ------------------------------------------------------------------------
 * General: Load the contact address from a message to the remote contact
 *          field in the call-leg. This address will be used as the request
 *          URI in later requests from this call-leg.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call-leg.
 *            hMsg     - Handle to the message.
 ***************************************************************************/
void RVCALLCONV CallLegMsgLoaderLoadContactToRemoteContact(
                                  IN  CallLeg                 *pCallLeg,
                                  IN  RvSipMsgHandle          hMsg);


/***************************************************************************
* CallLegMsgLoaderLoadDialogInfo
* ------------------------------------------------------------------------
* General: Load the dialog information from message.
*          the information is: remote contact and reoute-list.
*          The function is used when a message that creates a dialog is
*          received (invite, refer, subscribe).
* Return Value: (-)
* ------------------------------------------------------------------------
* Arguments:
* Input:     pCallLeg - Pointer to the call-leg.
*            hMsg     - Handle to the message.
***************************************************************************/
void RVCALLCONV CallLegMsgLoaderLoadDialogInfo(IN  CallLeg                 *pCallLeg,
                                               IN  RvSipMsgHandle          hMsg);


#endif  /* RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif
#endif /*CALL_LEG_MSG_LOADER_H*/

