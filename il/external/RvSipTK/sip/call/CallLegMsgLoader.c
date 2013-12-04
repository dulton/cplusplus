#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES
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
 *                              <CallLegMsgLoader.c>
 *
 *  The message loader supply functions to load message information into
 *  the call-leg object.
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Jun 2001
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#include "CallLegMsgLoader.h"
#include "RvSipContactHeader.h"
#include "RvSipCSeqHeader.h"
#include "CallLegRouteList.h"
#include "_SipCommonUtils.h"


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
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
                                 IN  RvSipMsgHandle          hMsg)

{
    RvSipAddressHandle hLocalContact = NULL;
    hLocalContact = RvSipMsgGetRequestUri(hMsg);
    if(hLocalContact != NULL)
    {
        CallLegSetLocalContactAddress(pCallLeg,hLocalContact);
    }
    return;
}



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
                                  IN  RvSipMsgHandle          hMsg)
{

    RvSipHeaderListElemHandle   hPos = NULL;
    RvSipContactHeaderHandle    hContact;
    RvSipAddressHandle          hRedirectAddress;
    /*get the contact header from the message*/
    hContact = (RvSipContactHeaderHandle)RvSipMsgGetHeaderByType(
                                             hMsg,
                                             RVSIP_HEADERTYPE_CONTACT,
                                             RVSIP_FIRST_HEADER, &hPos);
    if(hContact == NULL)
    {
        return;
    }
    /*get the address spec from the contact header and set it in the call-leg*/
    hRedirectAddress = RvSipContactHeaderGetAddrSpec(hContact);
    if(hRedirectAddress != NULL)
    {
        CallLegSetRedirectAddress(pCallLeg,hRedirectAddress);
    }
}


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
                                  IN  RvSipMsgHandle          hMsg)
{

    RvSipHeaderListElemHandle   hPos = NULL;
    RvSipContactHeaderHandle    hContact;
    RvSipAddressHandle          hContactAddress;
    /*get the contact header from the message*/
    hContact = (RvSipContactHeaderHandle)RvSipMsgGetHeaderByType(
                                             hMsg,
                                             RVSIP_HEADERTYPE_CONTACT,
                                             RVSIP_FIRST_HEADER, &hPos);
    if(hContact == NULL)
    {
        return;
    }
    /*get the address spec from the contact header and set it in the call-leg*/
    hContactAddress = RvSipContactHeaderGetAddrSpec(hContact);
    if(hContactAddress != NULL)
    {
        CallLegSetRemoteContactAddress(pCallLeg,hContactAddress);
    }
}

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
                                               IN  RvSipMsgHandle          hMsg)
{
    RvStatus rv;

    CallLegMsgLoaderLoadContactToRemoteContact(pCallLeg,hMsg);

    /*-------------------------------------------------------
    if the call-leg has no local contact load the request URI to
    the local contact
    -------------------------------------------------------------*/
    if (pCallLeg->hLocalContact == NULL)
    {
        CallLegMsgLoaderLoadRequestUriToLocalContact(pCallLeg,hMsg);
    }
    /*-------------------------------------------------------------------
    build the route list from the record-route headers in the message
    we do it only if there is no list.
    ---------------------------------------------------------------------*/
    if(CallLegRouteListIsEmpty(pCallLeg))
    {
        rv = CallLegRouteListInitialize(pCallLeg,hMsg);
        if(rv != RV_OK)
        {
            RvLogError(pCallLeg->pMgr->pLogSrc,(pCallLeg->pMgr->pLogSrc,
                "CallLegMsgLoaderLoadDialogInfo - Call-Leg 0x%p - Failed build route list - Record-Route mechanism will not be supported for this call-leg",pCallLeg));
        }
    }
}

#endif /*RV_SIP_PRIMITIVES */

