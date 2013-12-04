/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                          RvSipBody.c                                       *
 *                                                                            *
 * The file defines the methods of the body object:                           *
 * construct, destruct, copy, encode, parse and the ability to access, add    *
 * and change it's body parts and Content-Type.                               *
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

#include "RvSipBody.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "_SipContentTypeHeader.h"
#include "RvSipContentTypeHeader.h"
#include "_SipContentIDHeader.h"
#include "RvSipContentIDHeader.h"
#include "RvSipBodyPart.h"
#include "_SipCommonCore.h"
#include "rvansi.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

#define MSG_BODY_CR   0x0D
#define MSG_BODY_LF   0x0A
#define MSG_BODY_SP   0x20
#define MSG_BODY_HT   0x09

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus EncodeBoundary(
                         IN    MsgMgr*                        pMsgMgr,
                         IN    HRPOOL                         hBoundaryPool,
                         IN    HPAGE                          hBoundaryPage,
                         IN    RvInt32                       strBoundary,
                         IN    RvUint32                      boundaryLength,
                         IN    HRPOOL                         hPool,
                         IN    HPAGE                          hPage,
                         INOUT RvUint32*                     length);

static RvStatus EncodeBodyPartsList(
                             IN    Body                         *pBody,
                             IN    HRPOOL                           hPool,
                             IN    HPAGE                            hPage,
                             INOUT RvUint32*                       length);

static RvStatus EncodeBodyString(
                             IN    Body                         *pBody,
                             IN    HRPOOL                           hPool,
                             IN    HPAGE                            hPage,
                             INOUT RvUint32*                       length);

static RvStatus FindNextBoundaryLine(
                             IN    RvChar                  *strBodyString,
                             IN    RvUint                   bufLength,
                             INOUT RvUint*                  offset,
                             OUT   RvUint                  *beginOffset);

static RvStatus FindEndOfLine(
                             IN    RvChar                  *strBodyString,
                             IN    RvUint                   bufLength,
                             INOUT RvUint*                  offset,
                             OUT   RvUint                  *beginOffset);

static RvStatus CompareBoundary(
                             IN    HRPOOL                    hPool,
                             IN    HPAGE                     hPage,
                             IN    RvInt32                  boundary,
                             IN    RvUint32                 boundaryLength,
                             IN    RvChar                  *strBodyString,
                             IN    RvUint                   bufLength,
                             INOUT RvUint                  *offset,
                             OUT   RvBool                  *bAreEqual,
                             OUT   RvBool                  *bIsLast);

static RvStatus FindNextBoundary(
                             IN    Body                  *pBody,
                             IN    RvInt32                  boundary,
                             IN    RvUint32                 boundaryLength,
                             IN    RvChar                  *strBodyString,
                             IN    RvUint                   bufLength,
                             IN    RvBool                   bIsFirst,
                             INOUT RvUint                  *offset,
                             OUT   RvUint                  *beginOffset,
                             OUT   RvBool                  *bIsLast);

static RvStatus IsThisLastBoundary(
                             IN    RvChar                  *strBodyString,
                             IN    RvUint                   bufLength,
                             IN    RvBool                   bIsFirst,
                             INOUT RvUint                  *offset,
                             OUT   RvBool                  *bIsLast);

/*-----------------------------------------------------------------------*/
/*                   CONSTRUCTORS AND DESTRUCTORS                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipBodyConstructInMsg
 * ------------------------------------------------------------------------
 * General: Constructs a body object inside a given message object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle to the message related to the new body object.
 * output: hBody - Handle to the body object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyConstructInMsg(
                                           IN  RvSipMsgHandle    hSipMsg,
                                           OUT RvSipBodyHandle   *hBody)
{
    MsgMessage*        msg;
    RvStatus          stat;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;
    if(hBody == NULL)
        return RV_ERROR_NULLPTR;

    *hBody = NULL;


    msg = (MsgMessage*)hSipMsg;

    /* Construct the message body object of the message pool and page */
    stat = RvSipBodyConstruct((RvSipMsgMgrHandle)msg->pMsgMgr, msg->hPool, msg->hPage, hBody);
    if (RV_OK != stat)
        return stat;

   /* Attach the body object to the msg */
    msg->pBodyObject = (Body *)*hBody;

    return RV_OK;
}

/***************************************************************************
 * RvSipBodyConstructInBodyPart
 * ------------------------------------------------------------------------
 * General: Constructs a body object inside a given body-part object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hBodyPart - Handle to the body-part related to the new body object.
 * output: hBody - Handle to the body object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyConstructInBodyPart(
                                    IN  RvSipBodyPartHandle  hBodyPart,
                                    OUT RvSipBodyHandle     *hBody)
{
    BodyPart*       bodyPart;
    RvStatus       stat;

    if(hBodyPart == NULL)
        return RV_ERROR_INVALID_HANDLE;
     if(hBody == NULL)
        return RV_ERROR_NULLPTR;

    *hBody = NULL;

    bodyPart = (BodyPart*)hBodyPart;

    /* Construct the message body object on the body-part pool and page */
    stat = RvSipBodyConstruct((RvSipMsgMgrHandle)bodyPart->pMsgMgr, bodyPart->hPool, bodyPart->hPage, hBody);
    if (RV_OK != stat)
        return stat;

   /* Attach the body to the body-part */
    bodyPart->pBody = (Body *)*hBody;

    return RV_OK;
}

/***************************************************************************
 * RvSipBodyConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone body object.
 *          The body object is constructed on a given page taken
 *          from a specified pool. The handle to the new body object is
 *          returned.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr   - Handle to the message manager.
 *         hPool   - Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: hBody - Handle to the newly constructed body object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyConstruct(
                                           IN  RvSipMsgMgrHandle   hMsgMgr,
                                           IN  HRPOOL              hPool,
                                           IN  HPAGE               hPage,
                                           OUT RvSipBodyHandle    *hBody)
{
    Body*  pBody;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((NULL == hMsgMgr) || (NULL == hBody) || (hPage == NULL_PAGE) || (hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    *hBody = NULL;

    /* Allocate the message body object */
    pBody = (Body*)SipMsgUtilsAllocAlign(pMsgMgr,
                                         hPool,
                                         hPage,
                                         sizeof(Body),
                                         RV_TRUE);
    if(pBody == NULL)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                  "RvSipBodyConstruct - Allocation failed, hPool 0x%p, hPage 0x%p",
                  hPool, hPage));
        return RV_ERROR_OUTOFRESOURCES;
    }

    /* Initialize message body fields */
    pBody->pMsgMgr        = pMsgMgr;
    pBody->hBodyPartsList = NULL;
    pBody->hPool          = hPool;
    pBody->hPage          = hPage;
    pBody->pContentType   = NULL;
    pBody->pContentID     = NULL;
    pBody->length         = 0;
    pBody->strBody        = UNDEFINED;
#ifdef SIP_DEBUG
    pBody->pBody          = NULL;
#endif

    *hBody = (RvSipBodyHandle)pBody;

    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
              "RvSipBodyConstruct - Message body object was constructed successfully. (hMsgMgr=0x%p, hPool=0x%p, hPage=0x%p, hHeader=0x%p)",
              pMsgMgr, hPool, hPage, *hBody));

    return RV_OK;
}


/***************************************************************************
 * RvSipBodyCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source body object to a destination
 *          body object.
 *          You must construct the destination object before using this
 *          function.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle to the destination body object.
 *    hSource      - Handle to the source body object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyCopy(
                                     INOUT  RvSipBodyHandle hDestination,
                                     IN     RvSipBodyHandle hSource)
{
    Body*            source;
    Body*            dest;
    RvStatus        stat;
    RvUint32        safeCounter;
    RvUint32        maxLoops;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    source      = (Body*)hSource;
    dest        = (Body*)hDestination;

    safeCounter = 0;
    maxLoops    = 10000;

    if (NULL != source->hBodyPartsList)
    {
        /* Copy the list of body parts */
        RvSipBodyPartHandle hSourceBodyPart;
        RvSipBodyPartHandle hDestinationBodyPart = NULL;

        /* Get source body-part */
        stat = RvSipBodyGetBodyPart(hSource, RVSIP_FIRST_ELEMENT,
                                       NULL, &hSourceBodyPart);
        while ((RV_OK == stat) &&
               (NULL != hSourceBodyPart))
        {
            /* Push source body part into the destination body object */
            stat = RvSipBodyPushBodyPart(hDestination, RVSIP_LAST_ELEMENT,
                                            hSourceBodyPart, NULL,
                                            &hDestinationBodyPart);
            if (RV_OK != stat)
            {
                RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                          "RvSipBodyCopy - Failed to copy message body. stat = %d",
                          stat));
                return stat;
            }
            /* Get next source body-part */
            stat = RvSipBodyGetBodyPart(hSource, RVSIP_NEXT_ELEMENT,
                                           hSourceBodyPart, &hSourceBodyPart);
            safeCounter++;
            if (safeCounter > maxLoops)
            {
                RvLogExcep(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                          "RvSipBodyCopy - Execption in loop. Too many rounds."));
                return RV_ERROR_UNKNOWN;
            }
        }
        if (RV_OK != stat)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipBodyCopy - Failed to copy message body. stat = %d",
                      stat));
            return stat;
        }

    }
    /* Copy the body string */
    dest->length = source->length;
    if (UNDEFINED != source->strBody)
    {
        stat = RPOOL_Append(dest->hPool, dest->hPage,
                            source->length,
                            RV_FALSE,
                            &(dest->strBody));
        if (RV_OK != stat)
        {
            dest->strBody = UNDEFINED;
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipBodyCopy - Failed to copy message body. stat = %d",
                      stat));
            return stat;
        }
        stat = RPOOL_CopyFrom(dest->hPool, dest->hPage, dest->strBody,
                              source->hPool, source->hPage, source->strBody,
                              source->length);
        if (RV_OK != stat)
        {
            dest->strBody = UNDEFINED;
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipBodyCopy - Failed to copy message body. stat = %d",
                      stat));
            return stat;
        }
#ifdef SIP_DEBUG
        dest->pBody = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage, dest->strBody);
#endif
    }
    else
    {
        dest->strBody = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pBody = NULL;
#endif
    }

    /* Copy the Content-Type header */
    stat = RvSipBodySetContentType(hDestination,
                              (RvSipContentTypeHeaderHandle)source->pContentType);
    if (RV_OK != stat)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                  "RvSipBodyCopy - Failed to copy message body. stat = %d",
                  stat));
        return stat;
    }

    /* Copy the Content-ID header */
    stat = RvSipBodySetContentID(hDestination,
                              (RvSipContentIDHeaderHandle)source->pContentID);
    if (RV_OK != stat)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                  "RvSipBodyCopy - Failed to copy message body. stat = %d",
                  stat));
        return stat;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipBodyEncode
 * ------------------------------------------------------------------------
 * General: Encodes a body object to a textual body.
 *          1. If the body is of type multipart (represented as a list
 *          of body parts), each body part is encoded and concatenated to
 *          the textual body. The body parts are separated by a boundary
 *          delimiter line. The boundary delimiter line is created using
 *          the boundary parameter of the Content-Type of the body object.
 *          If no boundary is defined for this body object, a unique
 *          boundary is generated and the boundary delimiter line is
 *          created accordingly.
 *          2. If the body type is other than multipart it is copied, as
 *          is, to the textual message body.
 *          The textual body is placed on a page taken from a
 *          specified pool. In order to copy the textual body from
 *          the page to a consecutive buffer, use RPOOL_CopyToExternal().
 *          The application must free the allocated page, using
 *          RPOOL_FreePage(). The Allocated page must be freed if and only
 *          if this function returns RV_OK.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *         hBody    - Handle to the body object.
 *         hPool    - Handle to the specified memory pool.
 * Output:
 *         hPage    - The memory page allocated to contain the encoded object.
 *         length   - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyEncode(
                                          IN    RvSipBodyHandle   hBody,
                                          IN    HRPOOL            hPool,
                                          OUT   HPAGE*            hPage,
                                          OUT   RvUint32*        length)
{
    RvStatus stat;
    Body*     pBody;

    pBody = (Body*)hBody;
    if(hPage == NULL || length == NULL)
        return RV_ERROR_NULLPTR;

    *hPage = NULL_PAGE;
    *length = 0;

    if ((hPool == NULL) || (hBody == NULL))
        return RV_ERROR_INVALID_HANDLE;

    /* Get memory page for the encoded string */
    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                  "RvSipBodyEncode - Failed. RPOOL_GetPage failed, hPool is 0x%p, hBody is 0x%p",
                  hPool, hBody));
        return stat;
    }
    else
    {
        RvLogInfo(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                  "RvSipBodyEncode - Got a new page 0x%p on pool 0x%p. hBody is 0x%p",
                  *hPage, hPool, hBody));
    }

    /* Encode message body */
    stat = BodyEncode(hBody, hPool, *hPage, length);
    if(stat != RV_OK)
    {
        /* Free the page when error occures */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                  "RvSipBodyEncode - Failed. Free page 0x%p on pool 0x%p. BodyEncode fail",
                  *hPage, hPool));
    }

    return stat;
}

/***************************************************************************
 * BodyEncode
 * General: Accepts pool and page for allocating memory, and encode the
 *            message body object (as string) on it.
 *          Encodes all body parts separated by a boundary delimiter line.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no
 *                                     method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hBody  - Handle of the message body object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV BodyEncode(
                     IN    RvSipBodyHandle                  hBody,
                     IN    HRPOOL                           hPool,
                     IN    HPAGE                            hPage,
                     INOUT RvUint32*                       length)
{
    Body*        pBody = (Body*)hBody;
    RvInt32     boundary;
    RvStatus    stat;

    if((hBody == NULL) || (hPage == NULL_PAGE))
        return RV_ERROR_BADPARAM;

    RvLogInfo(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
              "BodyEncode - Encoding message body. hBody 0x%p, hPool 0x%p, hPage 0x%p",
              hBody, hPool, hPage));


    if (NULL == pBody->hBodyPartsList)
    {
        /* The body is a string. Copy the string to the given memory page */
        stat = EncodeBodyString(pBody, hPool, hPage, length);
        if (RV_OK != stat)
        {
            RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                      "BodyEncode - Failed to encode message body. stat = %d",
                      stat));
            return stat;
        }

        return RV_OK;
    }

    /* The body is a list of body parts. Encode the body parts separated by a
       delimiter boundary line */
    stat = BodyUpdateContentType(hBody);
    if (RV_OK != stat)
    {
        /* Couldn't update Content-Type */
        RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                  "BodyEncode - Failed to encode message body. Missing Content-Type."));
        return RV_ERROR_UNKNOWN;
    }

    if (NULL == pBody->pContentType)
    {
        /* Missing Content-Type */
        RvLogExcep(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                  "BodyEncode - Failed to encode message body. Missing Content-Type."));
        return RV_ERROR_UNKNOWN;
    }
    boundary = SipContentTypeHeaderGetBoundary(
                            (RvSipContentTypeHeaderHandle)pBody->pContentType);
    if (UNDEFINED == boundary)
    {
        /* Missing boundary */
        RvLogExcep(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                  "BodyEncode - Failed to encode message body. Missing boundary."));
        return RV_ERROR_UNKNOWN;
    }

    stat = EncodeBodyPartsList(pBody, hPool, hPage, length);
    if (RV_OK != stat)
    {
        RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                  "BodyEncode - Failed to encode message body. stat = %d",
                  stat));
        return stat;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipBodyMultipartParse
 * ------------------------------------------------------------------------
 * General:Parses a SIP textual body of type multipart into a body object.
 *         The given body object must have a valid Content-Type header,
 *         which contains the boundary that was used to create the boundary
 *         delimiter lines that separated the different parts of the body.
 *         The parsing will be made according to this boundary.
 *         - for example, to parse the following multipart body correctly,
 *         "boundary=unique-boundary-1" must be a parameter of the
 *         Content-Type of the given body object.
 *         "--unique-boundary-1
 *          Content-Type: application/SDP; charset=ISO-10646
 *
 *          v=0
 *          o=audet 2890844526 2890842807 5 IN IP4 134.177.64.4
 *          s=SDP seminar
 *          c=IN IP4 MG141.nortelnetworks.com
 *          t= 2873397496 2873404696
 *          m=audio 9092 RTP/AVP 0 3 4
 *
 *          --unique-boundary-1
 *          Content-type:application/QSIG; version=iso
 *
 *          08 02 55 55 05 04 02 90 90 18 03 a1 83 01
 *          70 0a 89 31 34 30 38 34 39 35 35 30 37 32
 *          --unique-boundary-1--"
 *         The body parts are separated into a list of body parts within the
 *         body object. The headers list of each body part is parsed within
 *         the appropriate body part object.
 *
 *         To Parse a body object you can either supply a string of your own,
 *         or you can use the body string within the body object, if exists.
 *         The second option can be used when the body was received via the
 *         network. The body object will contain the body string. You can
 *         receive this string using RvSipBodyGetBodyStr. To receive the
 *         length of this string in advance, use RvSipBodyGetBodyStrLength().
 *         After you retrieve the body string you can use
 *         RvSipBodyMultipartParse() to parse it.
 *
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *        hBody        - A handle to the body object.
 *        strBody      - The string of the body.
 *      bufLength    - The buffer length.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyMultipartParse(
                                 IN    RvSipBodyHandle      hBody,
                                 IN    RvChar                *strBody,
                                 IN    RvUint                 bufLength)
{
    Body                    *pBody;
    RvInt32                 boundary;
    RvUint                  index = 0;
    RvUint                  tempIndex = 0;
    RvUint                  beginIndex;
    RvUint                  tempBeginIndex;
    RvSipBodyPartHandle      hTempBodyPart;
    RvUint                  tempLength;
    RvBool                  bIsLast;
    RvUint32                boundaryLength;
    RvStatus                stat;

    if ((NULL == hBody) || (NULL == strBody))
        return RV_ERROR_NULLPTR;

    pBody = (Body *)hBody;

    /* The body must contain a Content-Type which contains a boundary in order
       to parse the body string */
    if (NULL == pBody->pContentType)
    {
        RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                  "RvSipBodyMultipartParse - Failed to parse message body. Missing Content-Type"));
        return RV_ERROR_BADPARAM;
    }
    /* Only parse multipart body */
    if (RVSIP_MEDIATYPE_MULTIPART != RvSipContentTypeHeaderGetMediaType(
                          (RvSipContentTypeHeaderHandle)pBody->pContentType))
    {
        RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                  "RvSipBodyMultipartParse - Failed to parse message body. The body type is differet than multipart"));
        return RV_ERROR_BADPARAM;
    }
    boundary = SipContentTypeHeaderGetBoundary(
                        (RvSipContentTypeHeaderHandle)pBody->pContentType);
    if (UNDEFINED == boundary)
    {
        RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                  "RvSipBodyMultipartParse - Failed to parse message body. Missing boundary"));
        return RV_ERROR_BADPARAM;
    }
    boundaryLength = RPOOL_Strlen(pBody->hPool, pBody->hPage, boundary);
    if (0 == boundaryLength)
    {
        RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                  "RvSipBodyMultipartParse - Failed to parse message body. Missing boundary"));
        return RV_ERROR_BADPARAM;
    }

    /* Destruct the old list of body parts */
    pBody->hBodyPartsList = NULL;
    pBody->strBody        = UNDEFINED;

    /* Find the first boundary. Skip preamble. */
    stat = FindNextBoundary(pBody, boundary, boundaryLength,
                            strBody, bufLength,
                            RV_TRUE, &index,
                            &beginIndex, &bIsLast);
    if ((RV_OK != stat) ||
        ((index == bufLength) &&
         (RV_TRUE == bIsLast)))
    {
        RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                  "RvSipBodyMultipartParse - Failed to parse message body. No boundary was found in message"));
        return RV_ERROR_UNKNOWN;
    }

    while (index < bufLength)
    {
        tempIndex = index;
        stat = FindNextBoundary(pBody, boundary, boundaryLength,
                                strBody, bufLength,
                                RV_FALSE, &tempIndex,
                                &tempBeginIndex, &bIsLast);
        if ((RV_OK != stat) &&
            (RV_ERROR_NOT_FOUND != stat))
        {
            RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                      "RvSipBodyMultipartParse - Failed to parse message body. "));
            return stat;
        }
        tempLength = tempBeginIndex - index;
        stat = RvSipBodyPartConstructInBody(hBody, RV_FALSE, &hTempBodyPart);
        if (RV_OK != stat)
        {
            RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                      "RvSipBodyMultipartParse - Failed to parse message body. "));
            return stat;
        }
        if (tempLength > 0)
        {
             stat = RvSipBodyPartParse(hTempBodyPart, strBody+index, tempLength);
            if (RV_OK != stat)
            {
                RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                          "RvSipBodyMultipartParse - Failed to parse message body. "));
                return stat;
            }
        }
        if (RV_TRUE == bIsLast)
            return RV_OK;
        else
            index = tempIndex;

    }

    return RV_OK;
}

/*-----------------------------------------------------------------------
                         G E T  A N D  S E T  M E T H O D S
 ------------------------------------------------------------------------*/

/***************************************************************************
 * SipBodyGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the message body object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hBody - The message body to take the page from.
 ***************************************************************************/
HRPOOL SipBodyGetPool(IN RvSipBodyHandle hBody)
{
    return ((Body*)hBody)->hPool;
}

/***************************************************************************
 * SipBodyGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the message body object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hContentType - The Content-Type header to take the page from.
 ***************************************************************************/
HPAGE SipBodyGetPage(IN RvSipBodyHandle hBody)
{
    return ((Body*)hBody)->hPage;
}

/***************************************************************************
 * BodyUpdateContentType
 * ------------------------------------------------------------------------
 * General: Updates default values for the Content-Type header of the message
 *          body object. If the body is represented as a list of body parts,
 *          the default Content-Type is multipart/mixed. If there is no
 *          boundary, a unique one must be generated.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments: hContentType - The Content-Type header to take the page from.
 ***************************************************************************/
RvStatus RVCALLCONV BodyUpdateContentType(IN RvSipBodyHandle hBody)
{
    Body                       *pBody;
    RvSipContentTypeHeaderHandle   hTempContentType;
    RvStatus                      stat;

    pBody = (Body *)hBody;

    if (NULL == pBody->hBodyPartsList)
        return RV_OK;

    if ((NULL == pBody->pContentType) ||
        (RVSIP_MEDIATYPE_MULTIPART !=
        RvSipContentTypeHeaderGetMediaType((RvSipContentTypeHeaderHandle)pBody->pContentType)))
    {
        stat = RvSipContentTypeHeaderConstructInBody(hBody,
                                                     &hTempContentType);
        if (RV_OK != stat)
            return stat;
        stat = RvSipContentTypeHeaderSetMediaType(hTempContentType,
                                                  RVSIP_MEDIATYPE_MULTIPART,
                                                  NULL);
        if (RV_OK != stat)
            return stat;
        stat = RvSipContentTypeHeaderSetMediaSubType(hTempContentType,
                                                     RVSIP_MEDIASUBTYPE_MIXED,
                                                     NULL);
        if (RV_OK != stat)
            return stat;
    }
    else
    {
        hTempContentType = (RvSipContentTypeHeaderHandle)pBody->pContentType;
    }

    if (UNDEFINED == SipContentTypeHeaderGetBoundary(hTempContentType))
    {
        RvUint32 timeInSec  = SipCommonCoreRvTimestampGet(IN_SEC);
        RvUint32 timeInMili = SipCommonCoreRvTimestampGet(IN_MSEC);
        RvInt32  r;
        RvChar   boundary[72]; /* The boundary is at most 70 chars */
        RvRandomGeneratorGetInRange(pBody->pMsgMgr->seed,RV_INT32_MAX,(RvRandom*)&r);

        RvSprintf(boundary, "\"Boundary%x_%x_%x_%lx\"", timeInSec, timeInMili, r, (RvUlong)RvPtrToUlong(pBody));

        stat = RvSipContentTypeHeaderSetBoundary(hTempContentType, boundary);
        if (RV_OK != stat)
            return stat;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipBodyGetContentType
 * ------------------------------------------------------------------------
 * General: Gets the Content-Type header from the body object.
 * Return Value: Returns the Content-Type header handle, or NULL if there
 *               is no Content-Type for this body object.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *        hBody - Handle to the body object.
 ***************************************************************************/
RVAPI RvSipContentTypeHeaderHandle RVCALLCONV RvSipBodyGetContentType(
                                             IN RvSipBodyHandle hBody)
{
    if (NULL == hBody)
    {
        return NULL;
    }
    return (RvSipContentTypeHeaderHandle)(((Body *)hBody)->pContentType);
}

/***************************************************************************
 * RvSipBodySetContentType
 * ------------------------------------------------------------------------
 * General:Sets the Content-Type header in the body object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *        hBody         - Handle to the body object.
 *        hContentType  - Content-Type header to be set in the body object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodySetContentType(
                              IN  RvSipBodyHandle           hBody,
                              IN  RvSipContentTypeHeaderHandle hContentType)
{
    Body                      *pBody;
    RvStatus                     stat;
    RvSipContentTypeHeaderHandle  hTempContentType;

    if (NULL == hBody)
        return RV_ERROR_NULLPTR;
    pBody = (Body *)hBody;
    if (NULL == hContentType)
    {
        pBody->pContentType = NULL;
        return RV_OK;
    }
    if (NULL == pBody->pContentType)
    {
        stat = RvSipContentTypeHeaderConstructInBody(hBody, &hTempContentType);
        if (RV_OK != stat)
            return stat;
    }
    stat = RvSipContentTypeHeaderCopy(
                    (RvSipContentTypeHeaderHandle)pBody->pContentType,
                    hContentType);
    return stat;
}

/***************************************************************************
 * RvSipBodyGetContentID
 * ------------------------------------------------------------------------
 * General: Gets the Content-ID header from the body object.
 * Return Value: Returns the Content-ID header handle, or NULL if there
 *               is no Content-ID for this body object.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *        hBody - Handle to the body object.
 ***************************************************************************/
RVAPI RvSipContentIDHeaderHandle RVCALLCONV RvSipBodyGetContentID(
                                             IN RvSipBodyHandle hBody)
{
    if (NULL == hBody)
    {
        return NULL;
    }
    return (RvSipContentIDHeaderHandle)(((Body *)hBody)->pContentID);
}

/***************************************************************************
 * RvSipBodySetContentID
 * ------------------------------------------------------------------------
 * General:Sets the Content-ID header in the body object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *        hBody         - Handle to the body object.
 *        hContentID  - Content-ID header to be set in the body object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodySetContentID(
                              IN  RvSipBodyHandle               hBody,
                              IN  RvSipContentIDHeaderHandle    hContentID)
{
    Body                      *pBody;
    RvStatus                     stat;
    RvSipContentIDHeaderHandle  hTempContentID;

    if (NULL == hBody)
        return RV_ERROR_NULLPTR;
    pBody = (Body *)hBody;
    if (NULL == hContentID)
    {
        pBody->pContentID = NULL;
        return RV_OK;
    }
    if (NULL == pBody->pContentID)
    {
        stat = RvSipContentIDHeaderConstructInBody(hBody, &hTempContentID);
        if (RV_OK != stat)
            return stat;
    }
    stat = RvSipContentIDHeaderCopy(
                    (RvSipContentIDHeaderHandle)pBody->pContentID,
                    hContentID);
    return stat;
}

/***************************************************************************
 * RvSipBodyGetBodyStrLength
 * ------------------------------------------------------------------------
 * General: Get the length of the body string.
 *          NOTE: If the body is of type multipart, it might contain a list
 *                of body parts. The body string and the list of body parts
 *                may not represent the same body, since each of the body
 *                parts can be independently manipulated. You can use this
 *                function only when the body object does not contain a list
 *                of body parts. If the body object contains a list of body
 *                parts this function will return 0. In that case you can use
 *                RvSipBodyEncode(). RvSipBodyEncode() will encode the
 *                list of body parts and will return the length of the encoded
 *                string, which represents the length of the body string.
 * Return Value: The length of the body string.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *        hBody      - Handle to the body object.
 ***************************************************************************/
RVAPI RvUint32 RVCALLCONV RvSipBodyGetBodyStrLength(
                                      IN     RvSipBodyHandle    hBody)
{
    Body *pBody;

    if (NULL == hBody)
        return 0;

    pBody = (Body *)hBody;

    if (NULL != pBody->hBodyPartsList)
        return 0;
    return pBody->length;
}

/***************************************************************************
 * RvSipBodyGetBodyStr
 * ------------------------------------------------------------------------
 * General: Get the string of the body.
 *          NOTE: If the body is of type multipart, it might contain a list
 *                of body parts. The body string and the list of body parts
 *                may not represent the same body, since each of the body
 *                parts can be independently manipulated. If the body object
 *                contains a list of body parts this function will return
 *                RV_ERROR_UNKNOWN. You can only use this function when the body
 *                does not contain a list of body parts. If you would like
 *                to get the body string of a body object in the format of
 *                body parts list, use RvSipBodyEncode() and than
 *                RPOOL_CopyToExternal() method.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *       hBody         - Handle to the body object.
 *       rawBuffer     - The body string of the body object. Note that the
 *                       body string does not necessarily end with '\0'. The
 *                       body string might contain '\0' as part of the body
 *                       string.
 *       bufferLen     - The length of the given buffer.
 * Output:
 *       actualLen     - The actual length of the body string.
 *                       This is also relevant when the buffer was insufficient.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyGetBodyStr(
                                      IN     RvSipBodyHandle     hBody,
                                      IN     RvChar            *rawBuffer,
                                      IN     RvUint32           bufferLen,
                                      OUT    RvUint32          *actualLen)
{
    Body    *pBody;
    RvStatus   stat;

    if ((NULL == rawBuffer) || (NULL == actualLen) || (NULL == hBody))
        return RV_ERROR_NULLPTR;

    pBody = (Body *)hBody;

    *actualLen = pBody->length;

    /* The body length is indicated by pBody->length */
    if (bufferLen < pBody->length)
    {
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }

    /* Check that the list of body parts is empty */
    if (NULL != pBody->hBodyPartsList)
    {
        return RV_ERROR_UNKNOWN;
    }

    if ((UNDEFINED == pBody->strBody) ||
        (0 == *actualLen))
    {
        /* The body is empty */
        return RV_ERROR_NOT_FOUND;
    }

    /* Copy the body string to the buffer */
    stat = RPOOL_CopyToExternal(pBody->hPool, pBody->hPage, pBody->strBody,
                                rawBuffer, *actualLen);
    return stat;
}

/***************************************************************************
 * RvSipBodySetBodyStr
 * ------------------------------------------------------------------------
 * General: Sets a textual body to the body object. The textual body is
 *          copied and saved in the body object.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *       hBody         - Handle to the body object.
 *       rawBuffer     - The buffer to set to the body object.
 *       length        - The buffer's length.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodySetBodyStr(
                                        IN  RvSipBodyHandle     hBody,
                                        IN  RvChar            *rawBuffer,
                                        IN  RvUint32           length)
{
    Body     *pBody;
    RvStatus    stat;
    RvInt32     offset;

    if (NULL == hBody)
        return RV_ERROR_NULLPTR;
    pBody = (Body *)hBody;

    /* Destruct list of body parts since the body is now represented as a string */
    if (NULL != pBody->hBodyPartsList)
    {
        pBody->hBodyPartsList = NULL;
    }
    if ((NULL == rawBuffer) ||
        (0 == length))
    {
        pBody->length  = 0;
        pBody->strBody = UNDEFINED;
#ifdef SIP_DEBUG
        pBody->pBody   = NULL;
#endif
        return RV_OK;
    }
    stat = RPOOL_Append(pBody->hPool, pBody->hPage,
                        length, RV_FALSE, &offset);
    if (RV_OK != stat)
        return stat;
    stat = RPOOL_CopyFromExternal(pBody->hPool, pBody->hPage, offset,
                                  rawBuffer, length);
    if (RV_OK != stat)
        return stat;
    pBody->length = length;
    pBody->strBody = offset;
#ifdef SIP_DEBUG
    pBody->pBody = (RvChar *)RPOOL_GetPtr(pBody->hPool, pBody->hPage, offset);
#endif

    return RV_OK;
}

/***************************************************************************
 * SipBodySetRawBuffer
 * ------------------------------------------------------------------------
 * General: This method sets the raw buffer of the message body.
 *          The raw buffer string is given on a memory page, indicated
 *          by it's offset.
 * Return Value:  RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hBody - Handle of a message body object.
 *         bufferOffset - The offset of the raw buffer string.
 *         hPool - The pool on which the string lays (if relevant)
 *         hPage - The page on which the string lays (if relevant)
 *         length - The length of the raw buffer string.
 ***************************************************************************/
RvStatus SipBodySetRawBuffer(IN    RvSipBodyHandle     hBody,
                              IN    HRPOOL              hPool,
                              IN    HPAGE               hPage,
                              IN    RvInt32            bufferOffset,
                                 IN    RvUint32           length)
{
    Body     *pBody;
    RvStatus    stat;
    RvInt32     offset;

    pBody = (Body *)hBody;

    /* Destruct list of body parts since the body is now represented as a string */
    if (NULL != pBody->hBodyPartsList)
    {
        pBody->hBodyPartsList = NULL;
    }
    if (UNDEFINED == bufferOffset)
    {
        pBody->length  = 0;
        pBody->strBody = UNDEFINED;
#ifdef SIP_DEBUG
        pBody->pBody   = NULL;
#endif
        return RV_OK;
    }
    stat = RPOOL_Append(pBody->hPool, pBody->hPage,
                        length, RV_FALSE, &offset);
    if (RV_OK != stat)
        return stat;
    stat = RPOOL_CopyFrom(pBody->hPool, pBody->hPage, offset,
                          hPool, hPage, bufferOffset, length);
    if (RV_OK != stat)
        return stat;
    pBody->length = length;
    pBody->strBody = offset;
#ifdef SIP_DEBUG
    pBody->pBody = (RvChar *)RPOOL_GetPtr(pBody->hPool, pBody->hPage, offset);
#endif

    return stat;
}

/***************************************************************************
 * RvSipBodyGetBodyPart
 * ------------------------------------------------------------------------
 * General: Gets a body part from the body parts list. The body object
 *          holds the body part objects in a sequential list. You can
 *          use this function to receive the first and last body parts of
 *          the list, and also the following body part or previous body part
 *          using a relative body part from the list. Use the location
 *          parameter to define the location of the body part you request,
 *          and the relative parameter to supply a valid relative body
 *          part in the cases you request a next or previous location.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *        hBody     - Handle of the body object.
 *      location  - The location in list - next, previous, first or last.
 *      hRelative - Handle to the current position in the list (a relative
 *                  body part from the list). Supply this value if you choose
 *                  next or previous in the location parameter.
 *  Output:
 *      hBodyPart - Handle to the requested body part.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyGetBodyPart(
                                 IN  RvSipBodyHandle         hBody,
                                 IN  RvSipListLocation          location,
                                 IN  RvSipBodyPartHandle     hRelative,
                                 OUT RvSipBodyPartHandle    *hBodyPart)
{
    Body*            pBody;
    RLIST_ITEM_HANDLE   listElem;

    if((hBody == NULL) || (hBodyPart == NULL))
        return RV_ERROR_NULLPTR;

    pBody = (Body*)hBody;

    *hBodyPart = NULL;

    if (pBody->hBodyPartsList == NULL)
        return RV_OK;

    switch(location)
    {
        case RVSIP_FIRST_ELEMENT:
        {
            RLIST_GetHead (NULL, pBody->hBodyPartsList,
                           &listElem);
            break;
        }
        case RVSIP_LAST_ELEMENT:
        {
            RLIST_GetTail (NULL, pBody->hBodyPartsList,
                           &listElem);
            break;
        }
        case RVSIP_NEXT_ELEMENT:
        {
            RLIST_GetNext (NULL,
                           pBody->hBodyPartsList,
                           (RLIST_ITEM_HANDLE)hRelative,
                           &listElem);
            break;

        }
        case RVSIP_PREV_ELEMENT:
        {
            RLIST_GetPrev (NULL,
                           pBody->hBodyPartsList,
                           (RLIST_ITEM_HANDLE)hRelative,
                           &listElem);
            break;
        }
        default:
            return RV_ERROR_UNKNOWN;
    }

    if(listElem == NULL)
        return RV_OK;

    *hBodyPart = (RvSipBodyPartHandle)listElem;
    return RV_OK;
}

/***************************************************************************
 * RvSipBodyPushBodyPart
 * ------------------------------------------------------------------------
 * General: Inserts a given body part into the body parts list based on a
 *          given location - first, last, before or after a given relative
 *          element. The body part you supply is copied before it
 *          is inserted into the list. The function returns the handle to
 *          the body part object that was actually inserted to the list,
 *          which can be used in a following call to this function as the
 *          relative body part.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hBody        - Handle of the body object.
 *            location     - The location in list - next, previous, first or
 *                         last.
 *            hBodyPart    - Handle to the body part to be pushed into the
 *                         list.
 *          hRelative    - Handle to the current position in the list (a
 *                         relative body part from the list). Supply this
 *                         value if you chose next or previous in the
 *                         location parameter.
 *  Output:
 *          hNewBodyPart - Handle to a copy of hBodyPart that was inserted
 *                         to the body parts list. Can be used in further
 *                         calls for this function as hRelative.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyPushBodyPart(
                                IN   RvSipBodyHandle            hBody,
                                IN   RvSipListLocation          location,
                                IN   RvSipBodyPartHandle        hBodyPart,
                                IN   RvSipBodyPartHandle        hRelative,
                                OUT  RvSipBodyPartHandle       *hNewBodyPart)
{
    Body*              pBody;
    BodyPart*          pBodyPart;
    RvStatus          stat;

    if ((hBody == NULL) || (hNewBodyPart == NULL) || (hBodyPart == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pBody = (Body*)hBody;

    *hNewBodyPart = NULL;

    /* Allocate body part according to the given location */
    stat = BodyPushBodyPart(hBody, location, hRelative, hNewBodyPart);
    if (RV_OK != stat)
    {
        *hNewBodyPart = NULL;
        return stat;
    }

    pBodyPart     = (BodyPart*)*hNewBodyPart;

    /* Initialize The new body part */
    pBodyPart->hHeadersList        = NULL;
    pBodyPart->pBody               = NULL;
    pBodyPart->pContentDisposition = NULL;
    pBodyPart->hPool               = pBody->hPool;
    pBodyPart->hPage               = pBody->hPage;
    pBodyPart->pMsgMgr             = pBody->pMsgMgr;

    stat = RvSipBodyPartCopy(*hNewBodyPart, hBodyPart);
    if(stat != RV_OK)
    {
        RvSipBodyRemoveBodyPart(hBody, *hNewBodyPart);
        *hNewBodyPart = NULL;
        return stat;
    }

    return RV_OK;
}

/***************************************************************************
 * BodyPushBodyPart
 * ------------------------------------------------------------------------
 * General: Inserts a new body part into the body parts list based on a
 *          given location - first, last, before or after a given relative
 *          element. You don't supply a body part for this function.
 *          The function returns the handle to the body part object that
 *          was actually inserted to the list, and you can then initialize
 *          or set values to this body part.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hBody     - Handle of the message body object.
 *            location     - The location in list - next, previous, first or
 *                         last.
 *          hRelative    - Handle to the current position in the list (a
 *                         relative body part from the list). Supply this
 *                         value if you chose next or previous in the
 *                         location parameter.
 *  Output:
 *          hNewBodyPart - Handle to a copy of hBodyPart that was inserted
 *                         to the body parts list. Can be used in further
 *                         calls for this function as hRelative.
 ***************************************************************************/
RvStatus RVCALLCONV BodyPushBodyPart(
                                IN   RvSipBodyHandle            hBody,
                                IN   RvSipListLocation             location,
                                IN   RvSipBodyPartHandle        hRelative,
                                OUT  RvSipBodyPartHandle       *hNewBodyPart)
{
    RLIST_HANDLE       list;
    RLIST_ITEM_HANDLE  allocatedItem = NULL;
    RLIST_ITEM_HANDLE  insertLocation;
    Body*              pBody;
    RvStatus          stat;

    if ((hBody == NULL) || (hNewBodyPart == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pBody = (Body*)hBody;

    *hNewBodyPart = NULL;

    /* if body parts is NULL we will create a new list */
    if(pBody->hBodyPartsList == NULL)
    {
        list = RLIST_RPoolListConstruct(pBody->hPool, pBody->hPage,
                                        sizeof(BodyPart), pBody->pMsgMgr->pLogSrc);
        if(list == NULL)
            return RV_ERROR_OUTOFRESOURCES;
        else
            pBody->hBodyPartsList = list;
    }
    else
    {
        list = pBody->hBodyPartsList;
    }

    insertLocation = (RLIST_ITEM_HANDLE)hRelative;

    /* pushing */
    switch (location)
    {
        case RVSIP_FIRST_ELEMENT:
        {
            stat = RLIST_InsertHead (NULL, list,
                                    &allocatedItem);
            break;
        }
        case RVSIP_LAST_ELEMENT:
        {
            stat = RLIST_InsertTail (NULL, list,
                                     &allocatedItem);
            break;
        }
        case RVSIP_NEXT_ELEMENT:
        {
            stat = RLIST_InsertAfter (NULL, list,
                                      insertLocation, &allocatedItem);
            break;
        }
        case RVSIP_PREV_ELEMENT:
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

    /* copy the given body part to the list */
    *hNewBodyPart = (RvSipBodyPartHandle)allocatedItem;

    return RV_OK;
}

/***************************************************************************
 * RvSipBodyRemoveBodyPart
 * ------------------------------------------------------------------------
 * General: Removes a body part from the body parts list.
 *          You should supply this function the handle to the body part you
 *          wish to remove.
 * Return Value: returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hBody     - Handle to the body object.
 *           hBodyPart - Handle to the body part to be removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyRemoveBodyPart(
                                      IN    RvSipBodyHandle     hBody,
                                      IN    RvSipBodyPartHandle hBodyPart)
{
    Body*           pBody;
    RLIST_ITEM_HANDLE  listElem;

    if((hBody == NULL) || (hBodyPart == NULL))
        return RV_ERROR_INVALID_HANDLE;

    pBody = (Body*)hBody;

    if (NULL == pBody->hBodyPartsList)
        return RV_ERROR_BADPARAM;

    /* Removing */
    RLIST_Remove (NULL, pBody->hBodyPartsList,
                  (RLIST_ITEM_HANDLE)hBodyPart);

    /* Setting the list to NULL when empty */
    RLIST_GetHead(NULL, pBody->hBodyPartsList, &listElem);

    if (NULL == listElem)
    {
        pBody->hBodyPartsList = NULL;
    }

    return RV_OK;
}



/***************************************************************************
 * RvSipBodyGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy the body string from the body object into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input: hBody     - Handle to the body object.
 *  Input/Output:
 *         pRpoolPtr -  Pointer to a location inside a RPOOL memory object.
 *                      You need to supply only the pool and page. The offset
 *                      where the returned string was located will be returned
 *                      as an output parameter. If the function set the returned 
 *                      offset to UNDEFINED is means that the parameter was 
 *                      not set in the RPOOL memory.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodyGetRpoolString(
                             IN    RvSipBodyHandle     hBody,
                             INOUT RPOOL_Ptr          *pRpoolPtr)
{


    Body*           pBody;

    if(hBody == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    pBody = (Body*)hBody;

    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                  "RvSipBodyGetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    /* the parameter does not exit in the header - return UNDEFINED*/
    if(pBody->strBody == UNDEFINED)
    {
        pRpoolPtr->offset = UNDEFINED;
        return RV_OK;
    }

    pRpoolPtr->offset = MsgUtilsAllocCopyRpoolString(
                                     pBody->pMsgMgr,
                                     pRpoolPtr->hPool,
                                     pRpoolPtr->hPage,
                                     pBody->hPool,
                                     pBody->hPage,
                                     pBody->strBody);

    if (pRpoolPtr->offset == UNDEFINED)
    {
        RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                  "RvSipBodyGetRpoolString - Failed to copy the string into the supplied page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;

}

/***************************************************************************
 * RvSipBodySetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a  body string into a body
 *          object. The given string is located on an RPOOL memory and is
 *          not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hBody     - Handle to the body object.
 *           pRpoolPtr - Pointer to a location inside an rpool where the
 *                       new string is located.
 *           length    - The body length +1 for the terminating '\0' char. 
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipBodySetRpoolString(
                             IN    RvSipBodyHandle     hBody,
                             IN    RPOOL_Ptr          *pRpoolPtr,
                             IN    RvUint32            length)
{
    Body* pBody = (Body*)hBody;
    RvStatus rv;
    if(pBody == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE ||
       pRpoolPtr->offset == UNDEFINED)
    {
        RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                 "RvSipBodySetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    rv = SipBodySetRawBuffer(hBody,pRpoolPtr->hPool,pRpoolPtr->hPage,pRpoolPtr->offset,
                             length);
    if(rv != RV_OK)
    {
        RvLogError(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                 "RvSipBodySetRpoolString - Failed to set string"));
        return rv;

    }
    return RV_OK;

}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * EncodeBoundary
 * General: Accepts pool and page for allocating memory, and encode the
 *            boundary delimiter line on it.
 *          The delimiter line is in the format of "--boundary".
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no
 *                                     method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hBoundaryPool  - Handle to the source pool.
 *        hBoundaryPage  - Handle to the source page.
 *        strBoundary    - The boundary offset.
 *        boundaryLength - The boundary string length.
 *        hPool          - Handle of the destination pool
 *        hPage          - Handle of the destination page.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
static RvStatus EncodeBoundary(
                         IN    MsgMgr*              pMsgMgr,
                         IN    HRPOOL               hBoundaryPool,
                         IN    HPAGE                          hBoundaryPage,
                         IN    RvInt32                       strBoundary,
                         IN    RvUint32                      boundaryLength,
                         IN    HRPOOL                         hPool,
                         IN    HPAGE                          hPage,
                         INOUT RvUint32*                     length)
{
    RvStatus   stat;
    RvChar    *firstChar;
    RvInt32    offset;

    /* Encode "--boundary" */
    /* set "--" */
    stat = MsgUtilsEncodeExternalString(pMsgMgr, hPool, hPage, "--", length);
    if (RV_OK != stat)
        return stat;

    firstChar = (RvChar *)RPOOL_GetPtr(hBoundaryPool, hBoundaryPage, strBoundary);

    if (*firstChar == '"')
    {
        /* The boundary is quoted */
        boundaryLength -= 2;
        strBoundary    += 1;
    }

    /* set boundary */
    *length += boundaryLength;

    stat = RPOOL_Append(hPool, hPage, boundaryLength, RV_FALSE, &offset);
    if(stat != RV_OK)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                  "EncodeBoundary - Failed, RPOOL_Append failed. hPool 0x%p, hPage 0x%p, RvStatus: %d",
                  hPool, hPage, stat));
        return stat;
    }

    stat = RPOOL_CopyFrom(hPool,
                          hPage,
                          offset,
                          hBoundaryPool,
                          hBoundaryPage,
                          strBoundary,
                          boundaryLength);
    if(stat != RV_OK)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                  "EncodeBoundary - Failed, RPOOL_CopyFrom failed. hPool 0x%p, hPage 0x%p, destPool 0x%p, destPage 0x%p, RvStatus: %d",
                  hBoundaryPool, hBoundaryPage, hPool, hPage, stat));
    }
    return stat;
}

/***************************************************************************
 * EncodeBodyPartsList
 * General: Accepts pool and page for allocating memory, and encode the
 *            body parts list on it.
 *          Encodes all body parts separated by a boundary delimiter line.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no
 *                                     method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: pBody  - Pointer to the message body object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
static RvStatus EncodeBodyPartsList(
                             IN    Body                         *pBody,
                             IN    HRPOOL                           hPool,
                             IN    HPAGE                            hPage,
                             INOUT RvUint32*                       length)
{
    RvSipBodyPartHandle          hTempBodyPart = NULL;
    RvStatus                    stat;
    RvInt32                     boundary;
    RvUint32                    boundaryLength;
    HRPOOL                       hBoundaryPool;
    HPAGE                        hBoundaryPage;
    RvUint32                    safeCounter;
    RvUint32                    maxLoops;

    boundary = SipContentTypeHeaderGetBoundary(
                            (RvSipContentTypeHeaderHandle)pBody->pContentType);
    hBoundaryPool = SipContentTypeHeaderGetPool(
                            (RvSipContentTypeHeaderHandle)pBody->pContentType);
    hBoundaryPage = SipContentTypeHeaderGetPage(
                            (RvSipContentTypeHeaderHandle)pBody->pContentType);
    boundaryLength = RPOOL_Strlen(hBoundaryPool, hBoundaryPage, boundary);
    if (0 == boundaryLength)
        return RV_ERROR_UNKNOWN;

    safeCounter = 0;
    maxLoops    = 10000;

    /* Encode body parts */
    stat = RvSipBodyGetBodyPart((RvSipBodyHandle)pBody,
                                   RVSIP_FIRST_ELEMENT,
                                   hTempBodyPart, &hTempBodyPart);
    while ((RV_OK == stat) &&
           (NULL != hTempBodyPart))
    {
        /* Encode boundary line */
        stat = EncodeBoundary(pBody->pMsgMgr, pBody->hPool, pBody->hPage, boundary,
                              boundaryLength, hPool, hPage, length);
        if (RV_OK != stat)
            return stat;

        /* Encode CRLF after delimiter boundary*/
        stat = MsgUtilsEncodeCRLF(pBody->pMsgMgr, hPool, hPage, length);
        if (RV_OK != stat)
            return stat;

        /* Encode body part */
        stat = BodyPartEncode(hTempBodyPart, hPool, hPage, length);
        if (RV_OK != stat)
            return stat;

        /* Encode CRLF previous to boundary delimiter*/
        stat = MsgUtilsEncodeCRLF(pBody->pMsgMgr,hPool, hPage, length);
        if (RV_OK != stat)
            return stat;

        /* Get next body part */
        stat = RvSipBodyGetBodyPart((RvSipBodyHandle)pBody,
                                       RVSIP_NEXT_ELEMENT,
                                       hTempBodyPart, &hTempBodyPart);

        safeCounter++;
        if (safeCounter > maxLoops)
        {
            RvLogExcep(pBody->pMsgMgr->pLogSrc,(pBody->pMsgMgr->pLogSrc,
                      "EncodeBodyPartsList - Execption in loop. Too many rounds."));
            return RV_ERROR_UNKNOWN;
        }
    }
    if (RV_OK != stat)
        return stat;

    /* Encode the closing boundary which is "--boundary--" */
    stat = EncodeBoundary(pBody->pMsgMgr, pBody->hPool, pBody->hPage, boundary,
                          boundaryLength, hPool, hPage,length);
    if (RV_OK != stat)
        return stat;

    stat = MsgUtilsEncodeExternalString(pBody->pMsgMgr, hPool, hPage, "--", length);
    if (RV_OK != stat)
        return stat;

	/* Encode CRLF after delimiter boundary according to RFC2046 (syntax section) */
	stat = MsgUtilsEncodeCRLF(pBody->pMsgMgr, hPool, hPage, length);
	if (RV_OK != stat)
		return stat;
	
    return RV_OK;
}

/***************************************************************************
 * EncodeBodyString
 * General: Accepts pool and page for allocating memory, and copies the
 *            body string to it, if needed.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no
 *                                     method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: pBody  - Pointer to the message body object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
static RvStatus EncodeBodyString(
                             IN    Body                         *pBody,
                             IN    HRPOOL                           hPool,
                             IN    HPAGE                            hPage,
                             INOUT RvUint32*                       length)
{
    RvStatus stat;

    if ((UNDEFINED == pBody->strBody) ||
        (0 == pBody->length))
    {
        /* The body string is empty */
        return RV_OK;
    }
    else
    {
        /* Copy the body string to the memory page */
        RvInt32 offset;

        stat = RPOOL_Append(hPool, hPage, pBody->length, RV_FALSE, &offset);
        if (RV_OK != stat)
            return stat;
        stat = RPOOL_CopyFrom(hPool, hPage, offset,
                              pBody->hPool,
                              pBody->hPage,
                              pBody->strBody,
                              pBody->length);
        if (RV_OK != stat)
            return stat;
        /* Set the body length to the length parameter */
        *length += pBody->length;
    }
    return RV_OK;
}

/***************************************************************************
 * FindNextBoundaryLine
 * General: Finds the next "CRLF--" in the given body string. Also allows
 *          "CR--" or "LF--"
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: pBody  - Pointer to the message body object.
 *        bufLength - The length of the given buffer.
 * output: offset  - The offset to start looking from. When function returned
 *                   it indicates the offset where the "CRLF--" ends.
 *         beginOffset - The offset where the "CRLF--boundary" begins.
 ***************************************************************************/
static RvStatus FindNextBoundaryLine(
                             IN    RvChar                  *strBodyString,
                             IN    RvUint                   bufLength,
                             INOUT RvUint*                  offset,
                             OUT   RvUint                  *beginOffset)
{
    RvUint   index = *offset;
    RvUint   tempBeginOffset;
    RvStatus stat;

    while (index < bufLength)
    {
        /* Move to the next CR/LF character */
        while ((index < bufLength) &&
               (strBodyString[index] != MSG_BODY_CR) &&
               (strBodyString[index] != MSG_BODY_LF))
        {
            index++;
        }
        /* Find the end of line. It is in one character if CR or LF are alone,
           and in two characters if there is a CRLF at this point */
        stat = FindEndOfLine(strBodyString, bufLength, &index, &tempBeginOffset);
        if (RV_OK != stat)
        {
            return stat;
        }
        *offset = index;
        *beginOffset = tempBeginOffset;
        if ((index + 1 < bufLength) &&
            (strBodyString[index] == '-') &&
            (strBodyString[index+1] == '-'))
        {
            /* This line begins in a format of a boundary line */
            *offset = index+2;
            return RV_OK;
        }
    }
    return RV_ERROR_NOT_FOUND;
}

/***************************************************************************
 * FindNextBoundary
 * General: Finds the next "CRLF--boundary" in the given body string. Also
 *          allows "CR--boundary" or "LF--boundary"
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: pBody - Pointer to the message body object.
 *        boundary - The boundary that was found in the Content-Type header.
 *        boundaryLength - The length of the boundary string.
 *        strBodyString  - The body string buffer.
 *        bufLength - The length of the given buffer.
 *        bIsFirst - Is the boundary the first boundary to find.
 * output: offset  - The offset to start looking from. When function returned
 *                   it indicates the offset where the "CRLF--boundary" ends.
 *         beginOffset - The offset where the "CRLF--boundary" begins.
 *         bIsLast - Is the boundary a closing boundary (is it followed by "--")
 ***************************************************************************/
static RvStatus FindNextBoundary(
                             IN    Body                  *pBody,
                             IN    RvInt32                  boundary,
                             IN    RvUint32                 boundaryLength,
                             IN    RvChar                  *strBodyString,
                             IN    RvUint                   bufLength,
                             IN    RvBool                   bIsFirst,
                             INOUT RvUint                  *offset,
                             OUT   RvUint                  *beginOffset,
                             OUT   RvBool                  *bIsLast)
{
    RvUint   index = *offset;
    RvBool   bTemp = RV_FALSE;
    RvUint   tempBeginOffset;
    RvStatus stat;

    *bIsLast = RV_FALSE;
    if (RV_TRUE == bIsFirst)
    {
        /* The boundary does not have to be preceded by CRLF, since this is
           the opening boundary. */
        if (bufLength <= index+2)
            return RV_ERROR_NOT_FOUND;

        if ((strBodyString[index] == '-') &&
            (strBodyString[index+1] == '-'))
        {
            /* The boundary is not preceded by a preamble */
            *beginOffset = index;
            index += 2;
        }
        else
        {
            /* There is a preamable before the boundary. The boundary is
               precede by a CRLF. Find the next "CRLF--" */
            stat = FindNextBoundaryLine(strBodyString, bufLength, &index, &tempBeginOffset);
            if (RV_OK != stat)
                return stat;
            *offset = index;
            *beginOffset = tempBeginOffset;
        }
    }
    else
    {
        /* Find the next "CRLF--" */
        stat = FindNextBoundaryLine(strBodyString, bufLength, &index, &tempBeginOffset);
        if (RV_OK != stat)
        {
            if (RV_ERROR_NOT_FOUND == stat)
            {
                /* We allow the message to encd without an ending boundary, since
                   the RFC instructs that a part of the boundary may be lost
                   in the network. If there is no boundary line, The boundary
                   is marked as last, and both the begin offset and the offset
                   point to the end of the buffer. */
                *bIsLast = RV_TRUE;
                *beginOffset = bufLength;
                *offset = bufLength;
                return RV_OK;
            }
            return stat;
        }
        *offset = index;
        *beginOffset = tempBeginOffset;
    }
    /* The boundary can't be empty */
    if (bufLength == index)
        return RV_ERROR_NOT_FOUND;
    while (index < bufLength)
    {
        stat = CompareBoundary(pBody->hPool, pBody->hPage, boundary,
                               boundaryLength, strBodyString,
                               bufLength, &index, &bTemp, bIsLast);
        if (RV_OK != stat)
            return stat;
        if (RV_TRUE == bTemp)
        {
            /* Check if this is the closing delimiter line */
            stat = IsThisLastBoundary(strBodyString, bufLength, bIsFirst,
                                      &index, bIsLast);
            if (RV_OK != stat)
                return stat;
            /* Go to the end of the delimiter line. The CRLF following a boundary
               delimiter line is considered a part of the delimiter line */
            stat = FindEndOfLine(strBodyString, bufLength, &index, &tempBeginOffset);
            if (RV_OK != stat)
                return stat;
            *offset = index;
            return RV_OK;
        }
        else if (RV_TRUE == *bIsLast)
        {
            /* The boundaries did not match but this is the last boundary in the
               body. This means that there is no closing boundary of the
               boundary we are looking for, and this is permitted by the stack. */
            return RV_OK;
        }
        /* Find nex "CRLF--" */
        stat = FindNextBoundaryLine(strBodyString, bufLength, &index, &tempBeginOffset);
        if (RV_OK != stat)
        {
            if (RV_ERROR_NOT_FOUND == stat)
            {
                /* We allow the message to encd without an ending boundary, since
                   the RFC instructs that a part of the boundary may be lost
                   in the network. If there is no boundary line, The boundary
                   is marked as last, and both the begin offset and the offset
                   point to the end of the buffer. */
                *bIsLast = RV_TRUE;
                *beginOffset = bufLength;
                *offset = bufLength;
                return RV_OK;
            }
            return stat;
        }
        *offset = index;
        *beginOffset = tempBeginOffset;
    }


    return RV_ERROR_NOT_FOUND;
}

/***************************************************************************
 * IsThisLastBoundary
 * General: Is this the last boundary of the body.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: strBodyString  - The body string buffer.
 *        bufLength - The length of the given buffer.
 *        bIsFirst - Is the boundary the first boundary to find.
 * output: offset  - The offset to start looking from. When function returned
 *                   it indicates the offset where the "CRLF--boundary--" ends.
 *         bIsLast - Is the boundary a closing boundary (is it followed by "--")
 ***************************************************************************/
static RvStatus IsThisLastBoundary(
                             IN    RvChar                  *strBodyString,
                             IN    RvUint                   bufLength,
                             IN    RvBool                   bIsFirst,
                             INOUT RvUint                  *offset,
                             OUT   RvBool                  *bIsLast)
{
    RvUint   index = *offset;

    if ((index + 1 < bufLength) &&
        (strBodyString[index] == '-') &&
        (strBodyString[index+1] == '-'))
    {
        /* This is a closing delimiter line.*/
        /* It can't be the opening delimiter line. */
        if (RV_TRUE == bIsFirst)
            return RV_ERROR_UNKNOWN;
        /* Mark this boundary as last in the body */
        *bIsLast = RV_TRUE;
        /* Skip "--" */
        index += 2;
        while ((index < bufLength) &&
               ((strBodyString[index] == MSG_BODY_SP) ||
               (strBodyString[index] == MSG_BODY_HT)))
        {
            /* Skip transport padding */
            index++;
        }
        *offset = index;
    }
    else if ((index  < bufLength) &&
             (strBodyString[index] == '-'))
    {
        /* This is a closing delimiter line missing one -.*/
        /* It can't be the opening delimiter line. */
        if (RV_TRUE == bIsFirst)
            return RV_ERROR_UNKNOWN;
        /* Mark this boundary as last in the body */
        *bIsLast = RV_TRUE;
        /* Skip "-\-" */
        index += 1;
        while (index < bufLength)
        {
            /* Skip transport padding */
            if ((strBodyString[index] != MSG_BODY_SP) &&
                (strBodyString[index] != MSG_BODY_HT) &&
                (strBodyString[index] != MSG_BODY_CR) &&
                (strBodyString[index] != MSG_BODY_LF))
            {
                return RV_ERROR_UNKNOWN;
            }
            index++;
        }
        *offset = index;
    }
    return RV_OK;
}

/***************************************************************************
 * FindEndOfLine
 * General: Finds the next "CRLF" in the given body string. Also allows
 *          "CR" or "LF"
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: strBodyString  - The body string buffer.
 *        bufLength - The length of the given buffer.
 * output: offset  - The offset to start looking from. When function returned
 *                   it indicates the offset where the "CRLF/CR/LF" ends.
 *         beginOffset - The offset where the end of line begins.
 ***************************************************************************/
static RvStatus FindEndOfLine(
                             IN    RvChar                  *strBodyString,
                             IN    RvUint                   bufLength,
                             INOUT RvUint*                  offset,
                             OUT   RvUint                  *beginOffset)
{
    RvUint   index = *offset;

    /* Find next CR/LF */
    while ((index < bufLength) &&
           (strBodyString[index] != MSG_BODY_CR) &&
           (strBodyString[index] != MSG_BODY_LF))
    {
        if ((strBodyString[index] != MSG_BODY_SP) &&
            (strBodyString[index] != MSG_BODY_HT))
        {
            return RV_ERROR_UNKNOWN;
        }
        index++;
    }
    if (index == bufLength)
    {
        return RV_ERROR_NOT_FOUND;
    }
    *beginOffset = index;
    /* The buffer ends with one CR/LF */
    if (index + 1 == bufLength)
    {
        *offset = index+1;
        return RV_OK;
    }
    /* Found CRLF */
    if ((strBodyString[index] == MSG_BODY_CR) &&
        (strBodyString[index+1] == MSG_BODY_LF))
    {
        *offset = index+2;
        return RV_OK;
    }
    /* Found CR or LF */
    *offset = index+1;
    return RV_OK;
}

/***************************************************************************
 * CompareBoundary
 * General: Compares the two boundaries. One boundary is on a memory pool and
 *          is ended with  '\0'. The other boundary is on the consecutive
 *          buffer. We will need to end it with '\0' before comparing.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hPool - The pool on which the boundary is.
 *        hPage - The page on which the boundary is.
 *        boundary - The boundary offset.
 *        boundaryLength - The length of the boundary string.
 *        bufLength - The length of the given buffer.
 * output: offset  - The offset of the boundary.
 *         bAreEqual - Are the two boundaries equal.
 *         bIsLast - Is this the last optional boundary in the body string.
 ***************************************************************************/
static RvStatus CompareBoundary(
                             IN    HRPOOL                    hPool,
                             IN    HPAGE                     hPage,
                             IN    RvInt32                  boundary,
                             IN    RvUint32                 boundaryLength,
                             IN    RvChar                  *strBodyString,
                             IN    RvUint                   bufLength,
                             INOUT RvUint                  *offset,
                             OUT   RvBool                  *bAreEqual,
                             OUT   RvBool                  *bIsLast)
{
    RvUint   index = *offset;
    RvChar   temp;
    RvChar  *firstChar;
    RvChar  *lastChar =NULL;
    RvChar   tempChar = '\0';

    /* Initialize boolean parameters to FALSE */
    *bAreEqual = RV_FALSE;
    *bIsLast = RV_FALSE;

    if (0 == boundaryLength)
        return RV_ERROR_UNKNOWN;

    /* If the boundary parameter is quoted we need to compare the string within
       the quotes with the boundary that was received in the delimiter line */

    /* Get the first character in the boundary parameter */
    firstChar = (RvChar *)RPOOL_GetPtr(hPool, hPage, boundary);

    if (*firstChar == '"')
    {
        /* The boundary is quoted. The length of the boundary is considered 2
           characters shorter than it's actual length. The boundary offset is
           forwarded by one */
        boundary       += 1;
        boundaryLength -= 2;
    }

    while ((index < bufLength) &&
           (index - *offset < boundaryLength) &&
           (strBodyString[index] != MSG_BODY_CR) &&
           (strBodyString[index] != MSG_BODY_LF) &&
           (strBodyString[index] != MSG_BODY_HT) &&
           (strBodyString[index] != MSG_BODY_SP))
    {
        /* Move throw the boundary string until you find a LWS or CRLF character,
           or until you reach the length of the original boundary */
        index++;
    }
    /* The buffer can't end right after the boundary. The final boundary line is
       followed by "--". The other boundary lines are followed by CRLF */
    if (index == bufLength)
    {
        *bAreEqual = RV_FALSE;
        /* We allow the last boundary to be cut off the message body */
        *bIsLast   = RV_TRUE;
        return RV_OK;
    }

    if (*firstChar == '"')
    {
        /* The boundary parameter is quoted. Run the ending '"' by '\0' in order
           to compare the string within the quotes with the received boundary. */
        lastChar  = (RvChar *)RPOOL_GetPtr(hPool, hPage, boundary+boundaryLength);
        tempChar  = *lastChar;
        *lastChar = '\0';
    }

    /* End the boundary string with '\0' */
    temp = strBodyString[index];
    strBodyString[index] = '\0';

    /* Compare two strings */
    *bAreEqual = RPOOL_CmpToExternal(hPool, hPage, boundary, strBodyString+*offset,RV_FALSE);

    /* Return all ran over characters to their original values */
    strBodyString[index] = temp;
    if (*firstChar == '"')
    {
        /*lint -e613*/
        *lastChar = tempChar;
    }

    if (RV_TRUE == *bAreEqual)
    {
        /* Update the offset when the boundaries are equal */
        *offset = index;
    }

    return RV_OK;
}





#endif /*RV_SIP_PRIMITIVES */

#ifdef __cplusplus
}
#endif
