/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                          RvSipBodyPart.c                                   *
 *                                                                            *
 * The file defines the methods of the body part object:                      *
 * construct, destruct, copy, encode, parse and the ability to access         *
 * and change it's body and headers.                                          *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza    Aug 2001                                              *
 ******************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "RvSipBodyPart.h"
#include "RvSipBody.h"
#include "RvSipMsgTypes.h"
#include "MsgUtils.h"
#include "MsgTypes.h"
#include "RvSipContentDispositionHeader.h"
#include "RvSipOtherHeader.h"
#include "_SipMsg.h"
#include "_SipParserManager.h"
#include "_SipCommonUtils.h"
#include <string.h>

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

#define MSG_BODYPART_SP   0x20
#define MSG_BODYPART_HT   0x09

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus EncodeHeadersList(
                             IN    BodyPart                 *pBodyPart,
                             IN    HRPOOL                       hPool,
                             IN    HPAGE                        hPage,
                             INOUT RvUint32*                   length);

static RvStatus ParseBody(IN    RvSipBodyPartHandle       hBodyPart,
                           IN    RvChar                     *strBody,
                           IN    RvUint                      index,
                           IN    RvUint                      length);

static RvStatus FindBody(IN    RvSipBodyPartHandle       hBodyPart,
                          IN    RvChar                     *strBody,
                          IN    RvUint                      length,
                          INOUT RvUint                     *offset);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipBodyPartConstructInBody
 * ------------------------------------------------------------------------
 * General: Constructs a body-part object inside a given body object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hBody              - Handle to the body related to the new body
 *                              part object.
 *         pushBodyPartAtHead - Boolean value indicating whether the body
 *                              part should be pushed to the head of the
 *                              list-RV_TRUE-or to the tail-RV_FALSE.
 * output: hBodyPart - Handle to the new body-part object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyPartConstructInBody(
                               IN  RvSipBodyHandle       hBody,
                               IN  RvBool               pushBodyPartAtHead,
                               OUT RvSipBodyPartHandle  *hBodyPart)
{
    BodyPart              *pBodyPart;
    Body                  *pBody = (Body *)hBody;
    RvSipListLocation      location;
    RvStatus              stat;

    if(hBody == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hBodyPart == NULL)
        return RV_ERROR_NULLPTR;

    *hBodyPart = NULL;

    if(pushBodyPartAtHead == RV_TRUE)
        location = RVSIP_FIRST_ELEMENT;
    else
        location = RVSIP_LAST_ELEMENT;

    /* attach the body part in the body */
    stat = BodyPushBodyPart(hBody,
                           location,
                           NULL,
                           hBodyPart);
    if (RV_OK != stat)
    {
        *hBodyPart = NULL;
        RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                  "RvSipBodyPartConstructInBody - Failed to construct message body part."));
        return stat;
    }

    pBodyPart = (BodyPart *)*hBodyPart;

    pBodyPart->hHeadersList        = NULL;
    pBodyPart->pBody               = NULL;
    pBodyPart->pContentDisposition = NULL;
    pBodyPart->pMsgMgr             = pBody->pMsgMgr;
    pBodyPart->hPool               = pBody->hPool;
    pBodyPart->hPage               = pBody->hPage;

    RvLogInfo(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
              "RvSipBodyPartConstructInBody - message body part was constructed successfully. (hBodyPart=0x%p)",
              *hBodyPart));

    return RV_OK;
}

/***************************************************************************
 * RvSipBodyPartConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone body-part
 *          object. The body-part object is constructed on a given page
 *          taken from a specified pool. The handle to the new body-part
 *          object is returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr  - Handle to the message manager.
 *         hPool    - Handle to the memory pool that the object will use.
 *         hPage    - Handle to the memory page that the object will use.
 * output: hBody    - Handle to the newly constructed body-part object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyPartConstruct(
                                    IN  RvSipMsgMgrHandle    hMsgMgr,
                                    IN  HRPOOL               hPool,
                                    IN  HPAGE                hPage,
                                    OUT RvSipBodyPartHandle *hBodyPart)
{
    BodyPart*  pBodyPart;
    MsgMgr*    pMsgMgr = (MsgMgr*)hMsgMgr;

    if((NULL == hBodyPart) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    *hBodyPart = NULL;

    /* allocate the message body part */
    pBodyPart = (BodyPart*)SipMsgUtilsAllocAlign(pMsgMgr,
                                                 hPool,
                                                 hPage,
                                                 sizeof(BodyPart),
                                                 RV_TRUE);
    if(pBodyPart == NULL)
    {
        *hBodyPart = NULL;
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                  "RvSipBodyPartConstruct - Allocation failed, hPool 0x%p, hPage 0x%p",
                  hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pBodyPart->hHeadersList        = NULL;
    pBodyPart->pBody               = NULL;
    pBodyPart->pContentDisposition = NULL;
    pBodyPart->pMsgMgr             = pMsgMgr;
    pBodyPart->hPool               = hPool;
    pBodyPart->hPage               = hPage;

    *hBodyPart = (RvSipBodyPartHandle)pBodyPart;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
              "RvSipBodyPartConstruct - message body part was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
              hMsgMgr, hPool, hPage, *hBodyPart));


    return RV_OK;
}


/***************************************************************************
 * RvSipBodyPartCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source body-part object to a
 *          destination body-part object.
 *          You must construct the destination object before using this
 *          function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination body-part object.
 *    hSource      - Handle to the source body-part object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyPartCopy(
                                  INOUT  RvSipBodyPartHandle hDestination,
                                  IN     RvSipBodyPartHandle hSource)
{
    BodyPart*     source;
    RvStatus     stat;
    RvUint32     safeCounter;
    RvUint32     maxLoops;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source = (BodyPart*)hSource;

    safeCounter = 0;
    maxLoops    = 10000;

    if (NULL != source->hHeadersList)
    {
        /* Copy the list of headers */
        RvSipOtherHeaderHandle hSourceHeader;
        RvSipOtherHeaderHandle hDestinationHeader = NULL;

        stat = RvSipBodyPartGetHeader(hSource, RVSIP_FIRST_HEADER,
                                         NULL, &hSourceHeader);
        while ((RV_OK == stat) &&
               (NULL != hSourceHeader))
        {
            stat = RvSipBodyPartPushHeader(hDestination, RVSIP_LAST_HEADER,
                                              hSourceHeader, NULL,
                                              &hDestinationHeader);
            if (RV_OK != stat)
            {
                RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                          "RvSipBodyPartCopy - Failed to copy message body-part. stat = %d",
                          stat));
                return stat;
            }
            stat = RvSipBodyPartGetHeader(hSource, RVSIP_NEXT_HEADER,
                                             hSourceHeader, &hSourceHeader);

            safeCounter++;
            if (safeCounter > maxLoops)
            {
                RvLogExcep(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                          "RvSipBodyPartCopy - Execption in loop. Too many rounds."));
                return RV_ERROR_UNKNOWN;
            }
        }
        if (RV_OK != stat)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipBodyPartCopy - Failed to copy message body-part. stat = %d",
                      stat));
            return stat;
        }

    }

    /* Set body object */
    stat = RvSipBodyPartSetBodyObject(hDestination,
                                   (RvSipBodyHandle)source->pBody);

    if (RV_OK != stat)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                  "RvSipBodyPartCopy - Failed to copy message body-part. stat = %d",
                  stat));
        return stat;
    }
    /* Set Content-Disposition */
    stat = RvSipBodyPartSetContentDisposition(hDestination,
              (RvSipContentDispositionHeaderHandle)source->pContentDisposition);
    if (RV_OK != stat)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                  "RvSipBodyPartCopy - Failed to copy message body-part. stat = %d",
                  stat));
        return stat;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipBodyPartEncode
 * ------------------------------------------------------------------------
 * General: Encodes a body-part object to a textual body-part.
 *          The headers of the body-part are encoded, the
 *          body of the body part is encoded according to the definition of
 *          RvSipBodyEncode(), and they are concatinated seperated by CRLF.
 *          The textual body-part is placed on a page taken from a specified
 *          pool. In order to copy the textual body-part from the
 *          page to a consecutive buffer, use RPOOL_CopyToExternal().
 *          The application must free the allocated page, using
 *          RPOOL_FreePage(). The Allocated page must be freed only if this
 *          function returns RV_OK.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *         hBodyPart    - Handle to the body-part object.
 *         hPool        - Handle to the specified memory pool.
 * Output:
 *         hPage        - The memory page allocated to contain the encoded
 *                        object.
 *         length       - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyPartEncode(
                                  IN    RvSipBodyPartHandle   hBodyPart,
                                  IN    HRPOOL                hPool,
                                  OUT   HPAGE*                hPage,
                                  OUT   RvUint32*            length)
{
    RvStatus stat;
    BodyPart* pBodyPart;

    pBodyPart = (BodyPart*)hBodyPart;
    if(hPage == NULL || length == NULL)
        return RV_ERROR_NULLPTR;

    *hPage = NULL_PAGE;
    *length = 0;

    if ((hPool == NULL) || (hBodyPart == NULL))
        return RV_ERROR_INVALID_HANDLE;

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pBodyPart->pMsgMgr->pLogSrc,(pBodyPart->pMsgMgr->pLogSrc,
                  "RvSipBodyPartEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hBodyPart is 0x%p",
                  hPool, hBodyPart));
        return stat;
    }
    else
    {
        RvLogInfo(pBodyPart->pMsgMgr->pLogSrc,(pBodyPart->pMsgMgr->pLogSrc,
                  "RvSipBodyPartEncode - Got a new page 0x%p on pool 0x%p. hBodyPart is 0x%p",
                  *hPage, hPool, hBodyPart));
    }

    stat = BodyPartEncode(hBodyPart, hPool, *hPage, length);
    if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pBodyPart->pMsgMgr->pLogSrc,(pBodyPart->pMsgMgr->pLogSrc,
                  "RvSipBodyPartEncode - Failed. Free page 0x%p on pool 0x%p. BodyEncode fail",
                  *hPage, hPool));
    }

    return stat;
}

/***************************************************************************
 * BodyPartEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            message body-part object (as string) on it.
 *          Encodes all headers separated by CRLF and encodes the body of
 *          the body part.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no
 *                                     method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hBodyPart  - Handle of the message body-part object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV BodyPartEncode(
                         IN    RvSipBodyPartHandle          hBodyPart,
                         IN    HRPOOL                       hPool,
                         IN    HPAGE                        hPage,
                         INOUT RvUint32*                   length)
{
    BodyPart*    pBodyPart;
    RvStatus       stat;

    if((hBodyPart == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    pBodyPart = (BodyPart*)hBodyPart;

    RvLogInfo(pBodyPart->pMsgMgr->pLogSrc,(pBodyPart->pMsgMgr->pLogSrc,
              "BodyPartEncode - Encoding message body-part. hBodyPart 0x%p, hPool 0x%p, hPage 0x%p",
              hBodyPart, hPool, hPage));

    if (NULL != pBodyPart->pBody)
    {
        stat = BodyUpdateContentType((RvSipBodyHandle)pBodyPart->pBody);
        if (RV_OK != stat)
        {
            RvLogError(pBodyPart->pMsgMgr->pLogSrc,(pBodyPart->pMsgMgr->pLogSrc,
                      "BodyPartEncode - Failed to encode message body-part. stat = %d",
                      stat));
            return stat;
        }
    }

    stat = EncodeHeadersList(pBodyPart, hPool, hPage, length);
    if (RV_OK != stat)
    {
        RvLogError(pBodyPart->pMsgMgr->pLogSrc,(pBodyPart->pMsgMgr->pLogSrc,
                  "BodyPartEncode - Failed to encode message body-part. stat = %d",
                  stat));
        return stat;
    }

    if (NULL != pBodyPart->pBody)
    {
        /* There is a non empty body in the body part. Encode the body after
           encoding CRLF */
        /* Encode CRLF */
        stat = MsgUtilsEncodeCRLF(pBodyPart->pMsgMgr, hPool, hPage, length);
        if (RV_OK != stat)
        {
            RvLogError(pBodyPart->pMsgMgr->pLogSrc,(pBodyPart->pMsgMgr->pLogSrc,
                      "BodyPartEncode - Failed to encode message body-part. stat = %d",
                      stat));
            return stat;
        }
        if ((NULL != pBodyPart->pBody->hBodyPartsList) ||
             (((UNDEFINED != pBodyPart->pBody->strBody) &&
               (0 != pBodyPart->pBody->length))))
        {
            stat = BodyEncode((RvSipBodyHandle)pBodyPart->pBody, hPool,
                              hPage, length);
            if (RV_OK != stat)
            {
                RvLogError(pBodyPart->pMsgMgr->pLogSrc,(pBodyPart->pMsgMgr->pLogSrc,
                          "BodyPartEncode - Failed to encode message body-part. stat = %d",
                          stat));
                return stat;
            }
        }

    }

    return RV_OK;
}

/***************************************************************************
 * RvSipBodyPartParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual body-part into a body-part object.
 *         An example of a textual body-part is:
 *         "Content-Type: application/SDP; charset=ISO-10646
 *
 *          v=0
 *          o=audet 2890844526 2890842807 5 IN IP4 134.177.64.4
 *          s=SDP seminar
 *          c=IN IP4 MG141.nortelnetworks.com
 *          t= 2873397496 2873404696
 *          m=audio 9092 RTP/AVP 0 3 4"
 *         The headers are parsed and set to the body-part object.
 *         The body is identified and set as a string to the body part
 *         object.
 *         If the body is of type multipart, you can parse it using
 *         RvSipBodyMultipartParse() using the following instructions:
 *         1. Get the body object from the body-part object using
 *         RvSipBodyPartGetBodyObject().
 *         2. Get the body string from the body object using
 *         RvSipBodyGetBodyStr().
 *         3. Parse the body string you received in 2 to the body object
 *         you received in 1, using RvSipBodyMultipartParse().
 *
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *        hBodyPart       - A handle to the body-part object.
 *        strBodyPart     - A buffer containning the body-part string.
 *      length          - The length of the buffer.
 *
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyPartParse(
                               IN      RvSipBodyPartHandle    hBodyPart,
                               IN      RvChar               *strBodyPart,
                               IN      RvUint                length)
{
    BodyPart           *pBodyPart;
    RvUint             index = 0;
    RvStatus           stat;
    RvChar             crOrLf;


    if ((NULL == hBodyPart) || (NULL == strBodyPart))
        return RV_ERROR_NULLPTR;

    pBodyPart = (BodyPart *)hBodyPart;

    /* Destruct old lists of this body part object */
    pBodyPart->hHeadersList = NULL;
    pBodyPart->pBody        = NULL;
    pBodyPart->pContentDisposition = NULL;

    if (0 == length)
    {
        pBodyPart->hHeadersList        = NULL;
        pBodyPart->pBody               = NULL;
        pBodyPart->pContentDisposition = NULL;
        return RV_OK;
    }

    if ((CR == strBodyPart[0]) ||
        (LF == strBodyPart[0]))
    {
        /* The headers list is empty */
        stat = ParseBody(hBodyPart, strBodyPart, index, length);
        if (RV_OK != stat)
        {
            RvLogError(pBodyPart->pMsgMgr->pLogSrc,(pBodyPart->pMsgMgr->pLogSrc,
                      "RvSipBodyPartParse - Failed parse message body part. stat = %d",
                      stat));
            return stat;
        }
        return RV_OK;
    }

    stat = FindBody(hBodyPart, strBodyPart, length, &index);

    if ((RV_OK != stat) &&
        (RV_ERROR_NOT_FOUND != stat))
    {
        RvLogError(pBodyPart->pMsgMgr->pLogSrc,(pBodyPart->pMsgMgr->pLogSrc,
                  "RvSipBodyPartParse - Failed parse message body part. stat = %d",
                  stat));
        return stat;
    }

    /* Put '\0' at the last CRLF (or CR or LF of the headers list) */
    index -= 1;

    if (((RvInt)index < 0) ||
        ((LF != strBodyPart[index]) &&
         (CR != strBodyPart[index])))
    {
        /* We expect the headers list not to be empty at this point, and to
           end with CRLF or CR or LF. */
        RvLogError(pBodyPart->pMsgMgr->pLogSrc,(pBodyPart->pMsgMgr->pLogSrc,
                  "RvSipBodyPartParse - Failed parse message body part. stat = %d",
                  stat));
        return RV_ERROR_UNKNOWN;
    }

    while (((RvInt)index >= 0) &&
           ((LF == strBodyPart[index]) ||
            (CR == strBodyPart[index])))
    {
        index -= 1;
    }

    index  += 1;
    crOrLf = strBodyPart[index];
    strBodyPart[index] = '\0';

    stat = MsgParserParse(pBodyPart->pMsgMgr, 
                          (void*)hBodyPart, 
                          SIP_PARSETYPE_MIME_BODY_PART, 
                          strBodyPart,
                          index+1);

    strBodyPart[index] = crOrLf;

    if (RV_OK != stat)
    {
        RvLogError(pBodyPart->pMsgMgr->pLogSrc,(pBodyPart->pMsgMgr->pLogSrc,
                  "RvSipBodyPartParse - Failed parse message body part. stat = %d",
                  stat));
        return stat;
    }

    return RV_OK;
}

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * SipBodyPartGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the message body-part object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hBodyPart - The body-part to take the page from.
 ***************************************************************************/
HRPOOL SipBodyPartGetPool(RvSipBodyPartHandle hBodyPart)
{
    return ((BodyPart *)hBodyPart)->hPool;
}

/***************************************************************************
 * SipBodyPartGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the message body-part object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hBodyPart - The body-part to take the page from.
 ***************************************************************************/
HPAGE SipBodyPartGetPage(IN RvSipBodyPartHandle hBodyPart)
{
    return ((BodyPart*)hBodyPart)->hPage;
}

/***************************************************************************
 * RvSipBodyPartGetContentDisposition
 * ------------------------------------------------------------------------
 * General: Get the Content-Disposition header from the body-part object.
 * Return Value: Returns the Content-Disposition header handle, or NULL if
 *               there is no Content-Disposition set for this body-part.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *        hBodyPart - Handle to the message body-part object.
 ***************************************************************************/
RVAPI RvSipContentDispositionHeaderHandle RVCALLCONV
                                RvSipBodyPartGetContentDisposition(
                                     IN RvSipBodyPartHandle hBodyPart)
{
    MsgContentDispositionHeader *pContentDisp;

    if (NULL == hBodyPart)
    {
        return NULL;
    }
    pContentDisp = ((BodyPart *)hBodyPart)->pContentDisposition;
    return (RvSipContentDispositionHeaderHandle)pContentDisp;
}

/***************************************************************************
 * RvSipBodyPartSetContentDisposition
 * ------------------------------------------------------------------------
 * General:Set Content-Disposition header to the body-part object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *        hBodyPart     - Handle to the body-part object.
 *        hContentDisp  - Content-Disposition header to be set in the
 *                      body-part object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyPartSetContentDisposition(
                        IN  RvSipBodyPartHandle              hBodyPart,
                        IN  RvSipContentDispositionHeaderHandle hContentDisp)
{
    BodyPart                         *pBodyPart;
    RvStatus                            stat;
    RvSipContentDispositionHeaderHandle  hTempContentDisp;

    if (NULL == hBodyPart)
        return RV_ERROR_NULLPTR;
    pBodyPart = (BodyPart *)hBodyPart;

    if (NULL == hContentDisp)
    {
        pBodyPart->pContentDisposition = NULL;
        return RV_OK;
    }
    if (NULL == pBodyPart->pContentDisposition)
    {
        stat = RvSipContentDispositionHeaderConstructInBodyPart(
                                            hBodyPart, &hTempContentDisp);
        if (RV_OK != stat)
            return stat;
    }
    stat = RvSipContentDispositionHeaderCopy(
        (RvSipContentDispositionHeaderHandle)pBodyPart->pContentDisposition,
        hContentDisp);
    return stat;
}

/***************************************************************************
 * RvSipBodyPartGetBodyObject
 * ------------------------------------------------------------------------
 * General: Get the body of the body-part object.
 * Return Value: Returns the body handle, or NULL if the body is not set.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *        hBodyPart - Handle to the body-part object.
 ***************************************************************************/
RVAPI RvSipBodyHandle RVCALLCONV RvSipBodyPartGetBodyObject(
                                     IN RvSipBodyPartHandle hBodyPart)
{
    BodyPart *pBodyPart;

    if (NULL == hBodyPart)
    {
        return NULL;
    }
    pBodyPart = (BodyPart *)hBodyPart;
    return (RvSipBodyHandle)pBodyPart->pBody;
}

/***************************************************************************
 * RvSipBodyPartSetBodyObject
 * ------------------------------------------------------------------------
 * General:Set the body of the body-part object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hBodyPart    - Handle to the body-part object.
 *    hBody        - Body object to be set to the body-part object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyPartSetBodyObject(
                                  IN  RvSipBodyPartHandle    hBodyPart,
                                  IN  RvSipBodyHandle        hBody)
{
    BodyPart           *pBodyPart;
    RvStatus              stat;
    RvSipBodyHandle     hTempBody;

    if (NULL == hBodyPart)
        return RV_ERROR_NULLPTR;
    pBodyPart = (BodyPart *)hBodyPart;

    if (NULL == hBody)
    {
        pBodyPart->pBody = NULL;
        return RV_OK;
    }
    if (NULL == pBodyPart->pBody)
    {
        stat = RvSipBodyConstructInBodyPart(hBodyPart, &hTempBody);
        if (RV_OK != stat)
            return stat;
    }
    stat = RvSipBodyCopy((RvSipBodyHandle)pBodyPart->pBody, hBody);
    return stat;
}

/***************************************************************************
 * RvSipBodyPartGetHeader
 * ------------------------------------------------------------------------
 * General: Gets a header from the headers list. The body-part object
 *          holds all headers other than Content-Disposition and
 *          Content-Type in a sequential list. All the headers in the list
 *          are other-headers (represented by RvSipOtherHeaderHandle).
 *          Use the location parameter to define the location of the header
 *          you request, and the relative parameter to supply a valid
 *          relative header when you request a next or previos location.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *        hBodyPart    - Handle of the body-part object.
 *      location     - The location in list - next, previous, first or last.
 *      hRelative    - Handle to the current position in the list (a relative
 *                     header from the list). Supply this value if you choose
 *                     next or previous in the location parameter.
 *  Output:
 *      hHeader      - Handle to the requested header.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyPartGetHeader(
                                 IN  RvSipBodyPartHandle     hBodyPart,
                                 IN  RvSipHeadersLocation       location,
                                 IN  RvSipOtherHeaderHandle     hRelative,
                                 OUT RvSipOtherHeaderHandle    *hHeader)
{
    BodyPart*        pBodyPart;
    RLIST_ITEM_HANDLE   listElem;

    if((hBodyPart == NULL) || (hHeader == NULL))
        return RV_ERROR_NULLPTR;

    pBodyPart = (BodyPart*)hBodyPart;

    *hHeader = NULL;

    if (pBodyPart->hHeadersList == NULL)
        return RV_OK;

    switch(location)
    {
        case RVSIP_FIRST_HEADER:
        {
            RLIST_GetHead (NULL,
                           pBodyPart->hHeadersList, &listElem);
            break;
        }
        case RVSIP_LAST_HEADER:
        {
            RLIST_GetTail (NULL,
                           pBodyPart->hHeadersList, &listElem);
            break;
        }
        case RVSIP_NEXT_HEADER:
        {
            RLIST_GetNext (NULL,
                           pBodyPart->hHeadersList,
                           (RLIST_ITEM_HANDLE)hRelative,
                           &listElem);
            break;

        }
        case RVSIP_PREV_HEADER:
        {
            RLIST_GetPrev (NULL,
                           pBodyPart->hHeadersList,
                           (RLIST_ITEM_HANDLE)hRelative,
                           &listElem);
            break;
        }
        default:
            return RV_ERROR_UNKNOWN;
    }

    if(listElem == NULL)
        return RV_OK;

    *hHeader = (RvSipOtherHeaderHandle)listElem;
    return RV_OK;
}

/***************************************************************************
 * RvSipBodyPartPushHeader
 * ------------------------------------------------------------------------
 * General: Inserts a given header into the header list based on a given
 *          location - first, last, before or after a given relative
 *          element. The body-part object holds all headers other than
 *          Content-Disposition and Content-Type in a sequential list. All
 *          the headers in the list are other-headers (represented by
 *          RvSipOtherHeaderHandle).
 *          The header you supply is copied before it is inserted into the list.
 *          The function returnes the handle to the header object that was
 *          actually inserted to the list, which can be used in following
 *          calls to this function as the relative header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hBodyPart    - Handle of the body-part object.
 *            location     - The location in list - next, previous, first or
 *                         last.
 *            hHeader      - Handle to the header to be pushed into the list.
 *          hRelative    - Handle to the current position in the list (a
 *                         relative header from the list). Supply this
 *                         value if you choose next or previous in the
 *                         location parameter.
 *  Output:
 *          hNewHeader   - Handle to a copy of hHeader that was inserted
 *                         to the headers list. Can be used in further
 *                         calls for this function as hRelative.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyPartPushHeader(
                              IN   RvSipBodyPartHandle       hBodyPart,
                              IN   RvSipHeadersLocation         location,
                              IN   RvSipOtherHeaderHandle       hHeader,
                              IN   RvSipOtherHeaderHandle       hRelative,
                              OUT  RvSipOtherHeaderHandle      *hNewHeader)
{
    BodyPart*        pBodyPart;
    MsgOtherHeader  *pOtherHeader;
    RvStatus        stat;

    if ((hBodyPart == NULL) || (hNewHeader == NULL) || (hHeader == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pBodyPart = (BodyPart*)hBodyPart;

    *hNewHeader = NULL;

    /* Allocate new header according to the given location */
    stat = BodyPartPushHeader(hBodyPart, location, hRelative, hNewHeader);
    if(stat != RV_OK)
    {
        *hNewHeader = NULL;
        return stat;
    }

    pOtherHeader = (MsgOtherHeader *)*hNewHeader;

    /* Initialize header */
    pOtherHeader->hPage            = pBodyPart->hPage;
    pOtherHeader->hPool            = pBodyPart->hPool;
    pOtherHeader->pMsgMgr          = pBodyPart->pMsgMgr;
    pOtherHeader->strHeaderName    = UNDEFINED;
    pOtherHeader->strHeaderVal     = UNDEFINED;
#ifdef SIP_DEBUG
    pOtherHeader->pHeaderName      = NULL;
    pOtherHeader->pHeaderVal       = NULL;
#endif

    stat = RvSipOtherHeaderCopy(*hNewHeader, hHeader);
    if(stat != RV_OK)
    {
        RvSipBodyPartRemoveHeader(hBodyPart, *hNewHeader);
        *hNewHeader = NULL;
        return stat;
    }

    return RV_OK;
}

/***************************************************************************
 * BodyPartPushHeader
 * ------------------------------------------------------------------------
 * General: Inserts a new header into the other headers list based on a
 *          given location - first, last, before or after a given relative
 *          element. You don't supply a header for this function.
 *          The function returnes the handle to the other header object that
 *          was actually inserted to the list, and you can then initialize
 *          or set values to this header.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hBodyPart - Handle of the message body-part object.
 *            location     - The location in list - next, previous, first or
 *                         last.
 *          hRelative    - Handle to the current position in the list (a
 *                         relative header from the list). Supply this
 *                         value if you choose next or previous in the
 *                         location parameter.
 *  Output:
 *          hNewHeader   - Handle to a copy of hHeader that was inserted
 *                         to the headers list. Can be used in further
 *                         calls for this function as hRelative.
 ***************************************************************************/
RvStatus RVCALLCONV BodyPartPushHeader(
                              IN   RvSipBodyPartHandle       hBodyPart,
                              IN   RvSipHeadersLocation         location,
                              IN   RvSipOtherHeaderHandle       hRelative,
                              OUT  RvSipOtherHeaderHandle      *hNewHeader)
{
    RLIST_HANDLE       list;
    RLIST_ITEM_HANDLE  allocatedItem = NULL;
    RLIST_ITEM_HANDLE  insertLocation;
    BodyPart*       pBodyPart;
    RvStatus          stat;

    if ((hBodyPart == NULL) || (hNewHeader == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pBodyPart = (BodyPart*)hBodyPart;

    *hNewHeader = NULL;

    /* if the headers list is NULL we will create a new list */
    if(pBodyPart->hHeadersList == NULL)
    {
         list = RLIST_RPoolListConstruct(pBodyPart->hPool, pBodyPart->hPage,
                                         sizeof(MsgOtherHeader), pBodyPart->pMsgMgr->pLogSrc);
         if(list == NULL)
            return RV_ERROR_OUTOFRESOURCES;
         else
            pBodyPart->hHeadersList = list;
    }
    else
    {
        list = pBodyPart->hHeadersList;
    }

    insertLocation = (RLIST_ITEM_HANDLE)hRelative;

    /* pushing */
    switch (location)
    {
        case RVSIP_FIRST_HEADER:
        {
            stat = RLIST_InsertHead (NULL, list,
                                    &allocatedItem);
            break;
        }
        case RVSIP_LAST_HEADER:
        {
            stat = RLIST_InsertTail (NULL, list,
                                     &allocatedItem);
            break;
        }
        case RVSIP_NEXT_HEADER:
        {
            stat = RLIST_InsertAfter (NULL, list,
                                      insertLocation, &allocatedItem);
            break;
        }
        case RVSIP_PREV_HEADER:
        {
            stat = RLIST_InsertBefore (NULL, list,
                                       insertLocation, &allocatedItem);
            break;
        }
        default:
            stat = RV_ERROR_BADPARAM;
            break;
    }
    if(stat != RV_OK)
        return stat;

    /* copy the given header to the list */
    *hNewHeader = (RvSipOtherHeaderHandle)allocatedItem;

    return RV_OK;
}

/***************************************************************************
 * RvSipBodyPartRemoveHeader
 * ------------------------------------------------------------------------
 * General: Removes a header from the headers list. You should supply this
 *          function the handle to the header you wish to remove.
 * Return Value: returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hBodyPart  - Handle to the body-part object.
 *           hHeader    - Handle to the header to be removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyPartRemoveHeader(
                                     IN  RvSipBodyPartHandle hBodyPart,
                                     IN  RvSipOtherHeaderHandle hHeader)
{
    BodyPart*          pBodyPart;
    RLIST_ITEM_HANDLE  listElem;

    if((hBodyPart == NULL) || (hHeader == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pBodyPart = (BodyPart*)hBodyPart;

    /* removing */
    RLIST_Remove (NULL, pBodyPart->hHeadersList,
                  (RLIST_ITEM_HANDLE)hHeader);

    /* Seting the list to NULL when empty */
    RLIST_GetHead(NULL, pBodyPart->hHeadersList, &listElem);

    if (NULL == listElem)
    {
        pBodyPart->hHeadersList = NULL;
    }

    return RV_OK;
}


/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * EncodeHeadersList
 * General: Accepts pool and page for allocating memory, and encode the
 *            headers list on it, including the Content-Type and
 *          Content-Disposition headers of the body part..
 *          Encodes all body parts separated by a boundary delimiter line.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no
 *                                     method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: pBodyPart  - Pointer to the message body-part object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
static RvStatus EncodeHeadersList(
                             IN    BodyPart                 *pBodyPart,
                             IN    HRPOOL                       hPool,
                             IN    HPAGE                        hPage,
                             INOUT RvUint32*                   length)
{
    RvStatus                    stat;
    RvSipOtherHeaderHandle       hTempOtherHeader;
    RvUint32                    safeCounter;
    RvUint32                    maxLoops;

    if ((NULL != pBodyPart->pBody) &&
        (NULL != pBodyPart->pBody->pContentType))
    {
        /* Encode Content-Type header */
        stat = ContentTypeHeaderEncode(
                   (RvSipContentTypeHeaderHandle)pBodyPart->pBody->pContentType,
                    hPool, hPage, RV_FALSE, RV_FALSE, length);
        if (RV_OK != stat)
            return stat;
        /* Encode CRLF */
        stat = MsgUtilsEncodeCRLF(pBodyPart->pMsgMgr, hPool, hPage, length);
        if (RV_OK != stat)
            return stat;
    }
	
	if ((NULL != pBodyPart->pBody) &&
        (NULL != pBodyPart->pBody->pContentID))
    {
        /* Encode Content-ID header */
        stat = ContentIDHeaderEncode(
                   (RvSipContentIDHeaderHandle)pBodyPart->pBody->pContentID,
                    hPool, hPage, RV_FALSE, length);
        if (RV_OK != stat)
            return stat;
        /* Encode CRLF */
        stat = MsgUtilsEncodeCRLF(pBodyPart->pMsgMgr, hPool, hPage, length);
        if (RV_OK != stat)
            return stat;
    }

    if (NULL != pBodyPart->pContentDisposition)
    {
        /* Encode Content-Disposition header */
        stat = ContentDispositionHeaderEncode(
             (RvSipContentDispositionHeaderHandle)pBodyPart->pContentDisposition,
             hPool, hPage, RV_FALSE, length);
        if (RV_OK != stat)
            return stat;
        /* Encode CRLF */
        stat = MsgUtilsEncodeCRLF(pBodyPart->pMsgMgr, hPool, hPage, length);
        if (RV_OK != stat)
            return stat;
    }

    safeCounter = 0;
    maxLoops    = 10000;

    /* Encode other headers */
    stat = RvSipBodyPartGetHeader((RvSipBodyPartHandle)pBodyPart,
                                     RVSIP_FIRST_HEADER,
                                     NULL, &hTempOtherHeader);
    while ((RV_OK == stat) &&
           (NULL != hTempOtherHeader))
    {
        /* Encode header */
        stat = OtherHeaderEncode(hTempOtherHeader, hPool, hPage, RV_TRUE, RV_FALSE, RV_FALSE, length);
        if (RV_OK != stat)
            return stat;
        /* Encode CRLF */
        stat = MsgUtilsEncodeCRLF(pBodyPart->pMsgMgr, hPool, hPage, length);
        if (RV_OK != stat)
            return stat;
        /* Get next header */
        stat = RvSipBodyPartGetHeader((RvSipBodyPartHandle)pBodyPart,
                                         RVSIP_NEXT_HEADER,
                                         hTempOtherHeader, &hTempOtherHeader);

        safeCounter++;
        if (safeCounter > maxLoops)
        {
            RvLogExcep(pBodyPart->pMsgMgr->pLogSrc,(pBodyPart->pMsgMgr->pLogSrc,
                      "EncodeHeadersList - Execption in loop. Too many rounds."));
            return RV_ERROR_UNKNOWN;
        }

    }
    if (RV_OK != stat)
        return stat;

    return RV_OK;
}

/***************************************************************************
 * ParseBody
 * General: The buffer offset indicates the begining of a body, including
 *          the previos CRLF. Construct a new body object for this body
 *          part and set it the appropriate values.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no
 *                                     method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hBodyPart  - Handle to the message body-part object.
 *        strBody       - An empty buffer to use for parsing.
 *        index      - The offset of the body within the buffer.
 *        length     - The length of the buffer to be parsed.
 ***************************************************************************/
static RvStatus ParseBody(IN    RvSipBodyPartHandle       hBodyPart,
                           IN    RvChar                     *strBody,
                           IN    RvUint                      index,
                           IN    RvUint                      length)
{
    RvStatus              stat;
    RvSipBodyHandle     hTempBody;

    if (index + 2 <= length)
    {
        if ((CR == strBody[index]) &&
            (LF == strBody[index+1]))
        {
            /* The body is preceded by CRLF */
            index += 2;
        }
        else
        {
            /* The body is preceded by CR/LF */
            index += 1;
        }
    }
    else
    {
        /* The body is preceded by CR/LF */
        index += 1;
    }
    stat = RvSipBodyConstructInBodyPart(hBodyPart, &hTempBody);
    if (RV_OK != stat)
        return stat;

    if (index < length)
    {
        stat = RvSipBodySetBodyStr(hTempBody, strBody+index, length-index);
        if (RV_OK != stat)
            return stat;
    }
    return RV_OK;
}

/***************************************************************************
 * FindBody
 * General: Find two following CRLF (also allowes CR CR or LF LF). If there
 *          are non, returnes the end of the buffer.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no
 *                                     method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hBodyPart  - Handle to the message body-part object.
 *        strBody       - An empty buffer to use for parsing.
 *        length     - The length of the buffer to be parsed.
 * output:
 *        offset      - The offset of the body within the buffer.
 ***************************************************************************/
static RvStatus FindBody(IN    RvSipBodyPartHandle       hBodyPart,
                          IN    RvChar                  *strBody,
                          IN    RvUint                   length,
                          INOUT RvUint                  *offset)
{
    RvStatus              stat;
    RvSipBodyHandle        hTempBody;
    RvBool                bIsCr      = RV_FALSE;
    RvBool                bIsLf      = RV_FALSE;
    RvBool                bIsCrLf    = RV_FALSE;
    RvBool                bIsCrLfCr  = RV_FALSE;
    RvUint                index = *offset;

    while (index < length)
    {

        if (strBody[index] == CR)
        {
            if (RV_TRUE == bIsCr)
            {
                /* There were CR CR in the body string beginning in index-1 */
                *offset = index;
                break;
            }
            else if (RV_TRUE == bIsCrLf)
            {
                /* There were CR LF CR in the body string up to this point. */
                bIsCrLfCr = RV_TRUE;
                bIsCr     = RV_TRUE;
                bIsLf     = RV_FALSE;
                bIsCrLf   = RV_FALSE;
            }
            else
            {
                /* There was CR in the body string up to this point */
                bIsCr     = RV_TRUE;
                bIsLf     = RV_FALSE;
                bIsCrLf   = RV_FALSE;
                bIsCrLfCr = RV_FALSE;
            }
        }
        else if (strBody[index] == LF)
        {
            if (RV_TRUE == bIsLf)
            {
                /* There were LF LF in the body string beginning in index-1 */
                *offset = index;
                break;
            }
            else if (RV_TRUE == bIsCrLfCr)
            {
                /* There were CR LF CR LF in the body string beginning in index-3 */
                *offset = index-1;
                break;
            }
            else if (RV_TRUE == bIsCr)
            {
                /* There were CR LF in the body string up to this point. */
                bIsCrLf   = RV_TRUE;
                bIsLf     = RV_TRUE;
                bIsCr     = RV_FALSE;
                bIsCrLfCr = RV_FALSE;
            }
            else
            {
                /* There was LF in the body string up to this point. */
                bIsLf     = RV_TRUE;
                bIsCrLf   = RV_FALSE;
                bIsCr     = RV_FALSE;
                bIsCrLfCr = RV_FALSE;
            }
        }
        else if ((strBody[index] == MSG_BODYPART_SP) ||
                 (strBody[index] == MSG_BODYPART_HT))
        {
            /* A space or tab were found. If they are preceded by a line break this
               line break is covered by spaces, to be removed from the headers
               list */
            if (RV_TRUE == bIsCrLf)
            {
                /* following CRLF */
                strBody[index-1] = MSG_BODYPART_SP;
                strBody[index-2] = MSG_BODYPART_SP;
            }
            else if ((RV_TRUE == bIsCr) ||
                     (RV_TRUE == bIsLf))
            {
                /* following CR/LF */
                strBody[index-1] = MSG_BODYPART_SP;
            }
            bIsLf     = RV_FALSE;
            bIsCrLf   = RV_FALSE;
            bIsCr     = RV_FALSE;
            bIsCrLfCr = RV_FALSE;
        }
        else
        {
            bIsLf     = RV_FALSE;
            bIsCrLf   = RV_FALSE;
            bIsCr     = RV_FALSE;
            bIsCrLfCr = RV_FALSE;
        }
        index++;
    }

    if (index == length)
    {
        *offset = index;
        return RV_ERROR_NOT_FOUND;
    }

    stat = RvSipBodyConstructInBodyPart(hBodyPart, &hTempBody);
    if (RV_OK != stat)
        return stat;

    /* index + 1 is the end of the line terminators that were found in the
       above loop */
    if (index+1 != length)
    {
        stat = RvSipBodySetBodyStr(hTempBody, strBody + index + 1,
                                   (length - index) - 1);
        if (RV_OK != stat)
            return stat;
    }

    return RV_OK;
}


#endif /*RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif
