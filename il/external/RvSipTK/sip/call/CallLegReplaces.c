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
 *                              <CallLegReplaces.c>
 *
 *  Handles CallLegs with Replaces header
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Apr 2001
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "CallLegReplaces.h"
#include "_SipCommonUtils.h"
#include "RvSipMsg.h"
#include "RvSipReplacesHeader.h"
#include "_SipPartyHeader.h"
#include "_SipReplacesHeader.h"
#include "RvSipCallLeg.h"

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           REPLACES FUNCTIONS                          */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CallLegReplacesAddToMsg
 * ------------------------------------------------------------------------
 * General: Sets Replaces header in an outgoing INVITE.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg that sends the INVITE message.
 *          hMsg     - Handle to the message to add the Replaces header to.
 *          eMethod  - The method of the message. If the method is not INVITE then
 *                     the Replaces will not be added to the message.
 ***************************************************************************/
RvStatus CallLegReplacesAddToMsg(IN  CallLeg                *pCallLeg,
                                  IN  RvSipMsgHandle          hMsg,
                                  IN  SipTransactionMethod    eMethod)
{
    RvStatus                 rv = RV_OK;
    RvSipReplacesHeaderHandle hReplacesHeader;

    if(RvSipMsgGetMsgType(hMsg) != RVSIP_MSG_REQUEST)
    {
        return RV_OK;
    }
    /*replaces should be added only to initial invite
      of outgoing calls
      this will prevent adding the replaces header to invite of an incoming call-leg
      that received replaces in its initial invite*/
    if(pCallLeg->eState == RVSIP_CALL_LEG_STATE_CONNECTED ||
       pCallLeg->eDirection == RVSIP_CALL_LEG_DIRECTION_INCOMING)
    {
        return RV_OK;
    }
    if(eMethod != SIP_TRANSACTION_METHOD_INVITE)
    {
        return RV_ERROR_ILLEGAL_ACTION;
    }
    if(pCallLeg->hReplacesHeader == NULL)
    {
        return RV_OK;
    }
    rv = RvSipReplacesHeaderConstructInMsg(hMsg, RV_TRUE, &hReplacesHeader);
    if(rv == RV_OK)
    {
        rv = RvSipReplacesHeaderCopy(hReplacesHeader, pCallLeg->hReplacesHeader);
    }
    return rv;
    /*Note: we reset the replaces header when 2xx is received*/
}


/***************************************************************************
 * CallLegReplacesGetMatchedCall
 * ------------------------------------------------------------------------
 * General: This function is called when the call leg is in the OFFERING state.
 *          This function searches for the call leg that has the same Call-ID,
 *          to tag and from tag as the Replaces header in the original call leg.
 *          If a matched call leg is found then this call leg is returned as
 *          the function output, otherwise this pointer will be NULL.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg - Pointer to the call leg that received an INVITE with
 *                     Replaces header.
 * Output:  hMatchedCallLeg - Pointer to the handle to the call leg with he same
 *                     identifiers as the Replaces header. If no call leg was found
 *                     this parameter will be NULL.
 *          peReason - If we found a dialog with same dialog identifiers,
 *                     but still does not match the replaces header, this
 *                     parameter indicates why the dialog doesn't fit.
 *                     application should use this parameter to decide how to
 *                     respond (401/481/486/501) to the INVITE with the Replaces.
 ***************************************************************************/
RvStatus CallLegReplacesGetMatchedCall(
                            IN  CallLeg                   *pCallLeg,
                            OUT RvSipCallLegHandle        *hMatchedCallLeg,
                            OUT RvSipCallLegReplacesReason *peReason)
{
    CallLeg *pMatchedCallLeg;

    CallLegMgrHashFindReplaces(pCallLeg->pMgr, pCallLeg->hReplacesHeader,
                               (RvSipCallLegHandle**)&pMatchedCallLeg,
                               peReason);

    *hMatchedCallLeg = (RvSipCallLegHandle)pMatchedCallLeg;

    return RV_OK;
}

/***************************************************************************
 * CallLegReplacesLoadReplacesHeaderToCallLeg
 * ------------------------------------------------------------------------
 * General: Loads the Replaces header from the INVITE request to the Call leg. If the
 *          INVITE has more then one Replaces header then the message is not legal
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg        - Pointer to the call leg that received the INVITE.
 *          hMsg            - Handle to the message to load the Replaces header from.
 *          pblegalReplaces - If the INVITE has more then one replaces header then
 *                            the message is not legal, and this parameter will be false.
 ***************************************************************************/
RvStatus CallLegReplacesLoadReplacesHeaderToCallLeg(IN  CallLeg          *pCallLeg,
                                                     IN  RvSipMsgHandle    hMsg,
                                                     OUT RvBool          *pblegalReplaces)
{
    RvSipHeaderListElemHandle hListElem;
    RvSipReplacesHeaderHandle hReplacesHeader = NULL;

    *pblegalReplaces = RV_TRUE;

    hReplacesHeader = (RvSipReplacesHeaderHandle)RvSipMsgGetHeaderByType(hMsg,
                                     RVSIP_HEADERTYPE_REPLACES,
                                     RVSIP_FIRST_HEADER,
                                     &hListElem);

    if(hReplacesHeader != NULL)
    {
        RvSipCallLegSetReplacesHeader((RvSipCallLegHandle)pCallLeg, hReplacesHeader);
    }
    hReplacesHeader =  (RvSipReplacesHeaderHandle)RvSipMsgGetHeaderByType(hMsg,
                                      RVSIP_HEADERTYPE_REPLACES,
                                      RVSIP_NEXT_HEADER,
                                      &hListElem);

    if(hReplacesHeader != NULL)
    {
        RvLogDebug(pCallLeg->pMgr->pLogSrc, (pCallLeg->pMgr->pLogSrc,
            "CallLegReplacesLoadReplacesHeaderToCallLeg: CallLeg 0x%p - more than one replaces header.", pCallLeg));
        *pblegalReplaces = RV_FALSE;
    }
    return RV_OK;
}

/***************************************************************************
 * CallLegReplacesPrepareReplacesHeaderFromCallLeg
 * ------------------------------------------------------------------------
 * General: This function prepares a Replaces header from the Call-ID, from-tag and
 *          to-tag of a Call-Leg
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg        - The call leg to make the Replaces header from.
 *          hReplacesHeader - Handle to a Constructed Replaces header.
 * Output:  hReplacesHeader - The Replaces header with hCallLeg identifiers
 ***************************************************************************/
RvStatus CallLegReplacesPrepareReplacesHeaderFromCallLeg(IN       CallLeg                    *pCallLeg,
                                                          INOUT    RvSipReplacesHeaderHandle   hReplacesHeader)
{
    RvStatus rv = RV_OK;
    RvInt32  strFromTag;
    RvInt32  strToTag;

    strFromTag = SipPartyHeaderGetTag(pCallLeg->hFromHeader);

    rv = SipReplacesHeaderSetFromTag(hReplacesHeader, NULL,
                            pCallLeg->pMgr->hGeneralPool, pCallLeg->hPage, strFromTag);

    if(rv != RV_OK)
    {
        return rv;
    }

    strToTag = SipPartyHeaderGetTag(pCallLeg->hToHeader);

    rv = SipReplacesHeaderSetToTag(hReplacesHeader, NULL,
                            pCallLeg->pMgr->hGeneralPool, pCallLeg->hPage, strToTag);

    if(rv != RV_OK)
    {
        return rv;
    }
    rv = SipReplacesHeaderSetCallID(hReplacesHeader, NULL,
                 pCallLeg->pMgr->hGeneralPool, pCallLeg->hPage, pCallLeg->strCallId);

    return rv;
}

/***************************************************************************
 * CallLegReplacesCompareReplacesToCallLeg
 * ------------------------------------------------------------------------
 * General: This function compares a Call leg to a Replaces header. The Call leg and Replaces
 *          header are equal if the Call-ID, from-tag and to-tag are equal.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCallLeg        - The call leg to compare with the Replaces header.
 *          hReplacesHeader - Handle to a Replaces header.
 * Output:  pbIsEqual       - The result of the comparison. RV_TRUE if the Call-leg and
 *                            Replaces header are equal, RV_FALSE - otherwise.
 ***************************************************************************/
RvStatus CallLegReplacesCompareReplacesToCallLeg(IN  CallLeg                  *pCallLeg,
                                                  IN  RvSipReplacesHeaderHandle hReplacesHeader,
                                                  OUT RvBool                  *pbIsEqual)
{
    HRPOOL hReplacesPool;
    HPAGE  hReplacesPage;

    hReplacesPool = SipReplacesHeaderGetPool(hReplacesHeader);
    hReplacesPage = SipReplacesHeaderGetPage(hReplacesHeader);
    *pbIsEqual = RV_TRUE;

    /* compares Call-ID*/
    *pbIsEqual = RPOOL_Strcmp(hReplacesPool, hReplacesPage, SipReplacesHeaderGetCallID(hReplacesHeader),
                    pCallLeg->pMgr->hGeneralPool, pCallLeg->hPage, pCallLeg->strCallId);

    if(*pbIsEqual == RV_FALSE)
    {
        return RV_OK;
    }
    /* compares to-tag */
    *pbIsEqual = RPOOL_Strcmp(hReplacesPool, hReplacesPage, SipReplacesHeaderGetToTag(hReplacesHeader),
                    pCallLeg->pMgr->hGeneralPool, pCallLeg->hPage, SipPartyHeaderGetTag(pCallLeg->hToHeader));

    if(*pbIsEqual == RV_FALSE)
    {
        return RV_OK;
    }
    /* compares from-tag */
    *pbIsEqual = RPOOL_Strcmp(hReplacesPool, hReplacesPage, SipReplacesHeaderGetFromTag(hReplacesHeader),
                    pCallLeg->pMgr->hGeneralPool, pCallLeg->hPage, SipPartyHeaderGetTag(pCallLeg->hFromHeader));

    if(*pbIsEqual == RV_FALSE)
    {
        return RV_OK;
    }

    return RV_OK;
}

#endif /* #ifndef RV_SIP_PRIMITIVES */
