/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             MsgInterval.c                                  *
 *                                                                            *
 * This file implements MsgInterval object methods.                           *
 * The methods are construct,destruct, copy an object, and methods for        *
 * providing access (get and set) to it's components. It also implement the   *
 * encode and parse methods.                                                  *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             December 2001                                        *
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "MsgTypes.h"
#include "MsgUtils.h"
#include "_SipDateHeader.h"
#include "ParserProcess.h"
#include "_SipCommonUtils.h"
#include <string.h>
#include <stdlib.h>



/****************************************************/
/*        CONSTRUCTORS AND DESTRUCTORS                */
/****************************************************/
/***************************************************************************
 * IntervalConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone MsgInterval object.
 *          The objectr is constructed on a given page taken from a specified
 *          pool. The pointer to the new header object is returned.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  pMsgMgr - Pointer to the message manager.
 *         hPool -   Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: pInterval - Pointer to the newly constructed interval object.
 ***************************************************************************/
RvStatus RVCALLCONV IntervalConstruct( IN  MsgMgr*       pMsgMgr,
                                        IN  HRPOOL        hPool,
                                        IN  HPAGE         hPage,
                                        OUT MsgInterval** pInterval)
{
    MsgInterval* pTemp;

    if((hPage == NULL_PAGE)||(hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if ((pInterval == NULL)||(pMsgMgr == NULL))
        return RV_ERROR_NULLPTR;

    *pInterval = NULL;

    pTemp = (MsgInterval*)SipMsgUtilsAllocAlign(pMsgMgr,
                                               hPool,
                                               hPage,
                                               sizeof(MsgInterval),
                                               RV_TRUE);
    if(pTemp == NULL)
    {
        *pInterval = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                  "IntervalConstruct - Failed to construct interval. outOfResources. hPool 0x%p, hPage 0x%p",
                  hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pTemp->pMsgMgr          = pMsgMgr;
    pTemp->eFormat          = RVSIP_EXPIRES_FORMAT_UNDEFINED;
    pTemp->hPage            = hPage;
    pTemp->hPool            = hPool;
    pTemp->hDate            = NULL;
    pTemp->deltaSeconds     = 0;

    *pInterval = pTemp;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
              "IntervalConstruct - MsgInterval was constructed successfully. (hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
              hPool, hPage, *pInterval));

    return RV_OK;
}


/***************************************************************************
 * IntervalCopy
 * ------------------------------------------------------------------------
 * General:Copies all fields from a source Interval object to a
 *         destination Interval object.
 *         You must construct the destination object before using this function.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pDestination - Pointer to the destination Interval object.
 *    pSource      - Pointer to the source Interval object.
 ***************************************************************************/
RvStatus RVCALLCONV IntervalCopy(INOUT MsgInterval* pDestination,
                                     IN    MsgInterval* pSource)
{
    RvStatus  retStatus;

    if((pDestination == NULL) || (pSource == NULL))
        return RV_ERROR_BADPARAM;

    pDestination->eFormat       = pSource->eFormat;
    pDestination->deltaSeconds  = pSource->deltaSeconds;
    if (RVSIP_EXPIRES_FORMAT_DATE == pSource->eFormat)
    {
        if (NULL == pDestination->hDate)
        {
            retStatus = SipDateConstructInInterval(pDestination, &(pDestination->hDate));
            if (RV_OK != retStatus)
            {
                return retStatus;
            }
        }
        retStatus = RvSipDateHeaderCopy(pDestination->hDate, pSource->hDate);
    }
    else
    {
        pDestination->hDate = NULL;
        retStatus = RV_OK;
    }

    return retStatus;
}

/***************************************************************************
 * IntervalEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Interval object (as string) on it.
 *          format: (SIP-date|delta-seconds)
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPool are NULL or the
 *                                     header is not initialized.
 *               RV_ERROR_NULLPTR       - pLength is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: pInterval  - Pointer of the interval object.
 *        hPool      - Handle of the pool of pages
 *        hPage      - Handle of the page that will contain the encoded message.
 *        bIsParameter - True if the interval is of an expires that is a parameter
 *                       of a Contact header and False if this not. The
 *                       encoding is done appropriately.
 *          bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: pLength   - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV IntervalEncode(
                          IN    MsgInterval* pInterval,
                          IN    HRPOOL      hPool,
                          IN    HPAGE       hPage,
                          IN    RvBool     bIsParameter,
                          IN    RvBool     bInUrlHeaders,
                          INOUT RvUint32*  pLength)
{
    RvStatus          stat;
    RvChar            strHelper[16];

    if((pInterval == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;
    if (pLength == NULL)
        return RV_ERROR_NULLPTR;

    RvLogInfo(pInterval->pMsgMgr->pLogSrc,(pInterval->pMsgMgr->pLogSrc,
        "IntervalEncode - Encoding interval. pInterval 0x%p, hPool 0x%p, hPage 0x%p",
        pInterval, hPool, hPage));

    if (RVSIP_EXPIRES_FORMAT_UNDEFINED == pInterval->eFormat)
    {
        RvLogExcep(pInterval->pMsgMgr->pLogSrc,(pInterval->pMsgMgr->pLogSrc,
            "IntervalEncode - Encoding interval. pInterval 0x%p, hPool 0x%p, hPage 0x%p, Interval format is undefined",
            pInterval, hPool, hPage));
        return RV_ERROR_BADPARAM;
    }

    if (RVSIP_EXPIRES_FORMAT_DELTA_SECONDS == pInterval->eFormat)
    {
        MsgUtils_uitoa(pInterval->deltaSeconds , strHelper);
        stat = MsgUtilsEncodeExternalString(pInterval->pMsgMgr, hPool, hPage, strHelper, pLength);
        if (RV_OK != stat)
        {
            RvLogError(pInterval->pMsgMgr->pLogSrc,(pInterval->pMsgMgr->pLogSrc,
                      "IntervalEncode - Failed to encode interval object. stat is %d",
                      stat));
            return stat;
        }
    }
    else
    {
        /* date of an expire header's interval that is a parameter of a contact header
             need to be in "" */
        if (RV_TRUE == bIsParameter)
        {
            /* put " . The date should  have quotes around it */
            stat = MsgUtilsEncodeExternalString(pInterval->pMsgMgr, hPool, hPage,
                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), pLength);
            if (RV_OK != stat)
            {
                RvLogError(pInterval->pMsgMgr->pLogSrc,(pInterval->pMsgMgr->pLogSrc,
                    "IntervalEncode - Failed to encode expires header. stat is %d",
                    stat));
                return stat;
            }
        }

        stat = DateHeaderEncode(pInterval->hDate, hPool, hPage, RV_TRUE, bInUrlHeaders, pLength);
        if (RV_OK != stat)
        {
            RvLogError(pInterval->pMsgMgr->pLogSrc,(pInterval->pMsgMgr->pLogSrc,
                      "IntervalEncode - Failed to encode interval object. stat is %d",
                      stat));
            return stat;
        }
        if (RV_TRUE == bIsParameter)
        {
            /* put " . The date should  have quotes around it */
            stat = MsgUtilsEncodeExternalString(pInterval->pMsgMgr, hPool, hPage,
                                MsgUtilsGetQuotationMarkChar(bInUrlHeaders), pLength);
            if (RV_OK != stat)
            {
            RvLogError(pInterval->pMsgMgr->pLogSrc,(pInterval->pMsgMgr->pLogSrc,
                    "IntervalEncode - Failed to encode expires header. stat is %d",
                    stat));
                return stat;
            }
        }

    }
    return RV_OK;
}


/***************************************************************************
 * IntervalSetDate
 * ------------------------------------------------------------------------
 * General: Sets a new Date header in the Interval object and changes
 *          the Interval format to date. (The function allocates a date header,
 *          and copy the given hDate object to it).
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Retry-After header handle is invalid.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Retry-After header object.
 *         hDate - The date handle to be set to the Retry-After header.
 *                 If the date handle is NULL, the existing date header
 *                 is removed from the expires header.
 ***************************************************************************/
RvStatus RVCALLCONV IntervalSetDate(IN  MsgInterval*             pInterval,
                                     IN  RvSipDateHeaderHandle    hDate)
{
   RvStatus             retStatus;
    RvSipDateHeaderHandle hTempDate;

    if (NULL == pInterval)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if (NULL == hDate)
    {
        pInterval->hDate = NULL;
        pInterval->eFormat = RVSIP_EXPIRES_FORMAT_UNDEFINED;
        return RV_OK;
    }
    retStatus = SipDateConstructInInterval(pInterval, &hTempDate);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    retStatus = RvSipDateHeaderCopy(hTempDate, hDate);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    pInterval->eFormat = RVSIP_EXPIRES_FORMAT_DATE;
    pInterval->hDate    = hTempDate;
    return RV_OK;
}

#endif /*#ifndef RV_SIP_LIGHT*/
#ifdef __cplusplus
}
#endif
