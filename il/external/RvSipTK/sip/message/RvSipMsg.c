/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        RvSipMsg.c                                          *
 *                                                                            *
 * The file defines the methods of the Message object.                        *
 * The message object represents a SIP message and holds all SIP message      *
 * information including start-line, headers and SDP body.                    *
 * Message object functions enable you to construct, destruct, copy, encode,  *
 * parse, access and change Message object parameters and manage the list     *
 * of headers in message objects.                                             *
 * list of headers.                                                           *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             Nov.2000                                             *
 ******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "RV_SIP_DEF.h"

#include "RvSipMsg.h"
#include "RvSipHeader.h"
#include "RvSipDateHeader.h"
#include "_SipMsg.h"
#include "_SipMsgMgr.h"
#include "_SipOtherHeader.h"
#include "_SipAddress.h"
#include "MsgTypes.h"
#include "MsgUtils.h"
#include "RvSipAddress.h"
#include "RvSipPartyHeader.h"
#include "RvSipCSeqHeader.h"
#include "RvSipAllowHeader.h"
#include "RvSipContactHeader.h"
#include "RvSipViaHeader.h"
#include "RvSipOtherHeader.h"
#include "RvSipAuthenticationHeader.h"
#include "RvSipAuthorizationHeader.h"
#include "RvSipExpiresHeader.h"
#include "RvSipReferToHeader.h"
#include "RvSipReferredByHeader.h"
#include "RvSipBody.h"
#include "RvSipContentTypeHeader.h"
#include "RvSipContentIDHeader.h"
#include "RvSipRAckHeader.h"
#include "RvSipRSeqHeader.h"
#include "RvSipContentDispositionHeader.h"
#include "RvSipRetryAfterHeader.h"
#include "RvSipReplacesHeader.h"
#include "RvSipEventHeader.h"
#include "RvSipAllowEventsHeader.h"
#include "RvSipSubscriptionStateHeader.h"
#include "RvSipSessionExpiresHeader.h"
#include "RvSipMinSEHeader.h"
#include "_SipReplacesHeader.h"
#include "_SipTransport.h"
#include "RvSipCommonList.h"
#include "RvSipAuthenticationInfoHeader.h"

#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
#include "RvSipReasonHeader.h"
#include "RvSipWarningHeader.h"
#include "RvSipTimestampHeader.h"
#include "RvSipInfoHeader.h"
#include "RvSipAcceptHeader.h"
#include "RvSipAcceptEncodingHeader.h"
#include "RvSipAcceptLanguageHeader.h"
#include "RvSipReplyToHeader.h"
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */

#ifdef RV_SIP_IMS_HEADER_SUPPORT
#include "RvSipPUriHeader.h"
#include "RvSipPVisitedNetworkIDHeader.h"
#include "RvSipPAccessNetworkInfoHeader.h"
#include "RvSipPChargingFunctionAddressesHeader.h"
#include "RvSipPChargingVectorHeader.h"
#include "RvSipPMediaAuthorizationHeader.h"
#include "RvSipSecurityHeader.h"
#include "RvSipPProfileKeyHeader.h"
#include "RvSipPUserDatabaseHeader.h"
#include "RvSipPAnswerStateHeader.h"
#include "RvSipPServedUserHeader.h"
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
#include "RvSipPDCSTracePartyIDHeader.h"		
#include "RvSipPDCSOSPSHeader.h"
#include "RvSipPDCSBillingInfoHeader.h"
#include "RvSipPDCSLAESHeader.h"
#include "RvSipPDCSRedirectHeader.h"
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
	/* XXX */

#define MAX_HEADERS_PER_MESSAGE 200
#define MAX_HEADER_NAME_LEN 50
#define MSG_SP           0x20
#define MSG_HT           0x09

    /*-----------------------------------------------------------------------*/
    /*                          TYPE DEFINITIONS                             */
    /*-----------------------------------------------------------------------*/


/* The struct is for sorting the other headers in the header list */
typedef struct
{
    RvChar                      name[MAX_HEADER_NAME_LEN];     /* header name */
    RvSipHeaderListElemHandle hListElement; /* header handle */
} listElem;

#define BS_HEADER_FROM          45
#define BS_HEADER_TO            46
#define BS_HEADER_CSEQ          47
#define BS_HEADER_CONTENT_TYPE  48
#define BS_HEADER_LIST          49

/* SPIRENT_BEGIN */
/*
* COMMENTS:
* Modified by Armyakov
* Counting raw messages 
*/
#if defined(UPDATED_BY_SPIRENT)
static RvSipRawMessageCounterHandlers SIP_SipRawMsgCntrHandlers = {NULL,NULL,NULL,NULL};
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV MsgEncodeHeaderList(IN   RvSipMsgHandle   hMsg,
                                               IN    RvSipMsgHeadersOption eOption,
                                               IN    HRPOOL          hPool,
                                               IN    HPAGE           hPage,
                                               INOUT RvUint32*      length);

static RvStatus RVCALLCONV MsgEncode(IN  RvSipMsgHandle          hSipMsg,
                                     IN  RvSipMsgHeadersOption   eOption,
                                     IN  HRPOOL                  hPool,
                                     IN  HPAGE                   hPage,
                                     OUT RvUint32*              length);

static RvStatus RVCALLCONV MsgEncodeBody(IN  MsgMessage*             msg,
                                          IN  RvSipMsgHeadersOption   eOption,
                                          IN  HRPOOL                  hPool,
                                          IN  HPAGE                   hPage,
                                          OUT RvUint32*              length);

#ifndef RV_SIP_PRIMITIVES
static RvUint GetContentTypeStringLength(IN MsgMessage *pSipMsg);
static RvUint GetBodyStringLength(IN MsgMessage *pSipMsg);
#endif /*#ifndef RV_SIP_PRIMITIVES*/
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
RVAPI  RvStatus sortHeaderList(IN MsgMessage* pMsg);
#else
static RvStatus sortHeaderList(IN MsgMessage* pMsg);
#endif
/* SPIRENT_END */

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
void  HSTPOOL_Release_TRACE ( void * buff, char* file, int line );
#define HSTPOOL_Release(buff) HSTPOOL_Release_TRACE(buff,__FILE__,__LINE__)
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

static RvStatus sortOtherHeadersList(
                  IN RvSipMsgHandle               hMsg,
                  IN RvSipHeaderListElemHandle hOtherFirstHeader,
                  IN RvSipHeaderListElemHandle hOtherLastHeader);


#ifndef RV_SIP_PRIMITIVES
static RvStatus StatusLineEncode(IN    MsgMessage*     pMsg,
                                  IN    MsgStatusLine*  statLine,
                                  IN    HRPOOL          hPool,
                                  IN    HPAGE           hPage,
                                  INOUT RvUint32*      length);
static RvStatus RequestLineEncode(IN    MsgMessage*     pMsg,
                                   IN    MsgRequestLine* reqLine,
                                   IN    HRPOOL          hPool,
                                   IN    HPAGE           hPage,
                                   INOUT RvUint32*      length);
#endif /*#ifndef RV_SIP_PRIMITIVES*/

static void* RVCALLCONV MsgGetHeaderFromList(
                                     IN    RvSipMsgHandle                hMsg,
                                     IN    RvSipHeadersLocation          location,
                                     INOUT RvSipHeaderListElemHandle*    hListElem,
                                     OUT   RvSipHeaderType*              pHeaderType);

static void* RVCALLCONV MsgGetHeaderByTypeFromList(
                                         IN    RvSipMsgHandle             hSipMsg,
                                         IN    RvSipHeaderType            eHeaderType,
                                         IN    RvSipHeadersLocation       location,
                                         INOUT RvSipHeaderListElemHandle* hListElem);

static RvSipOtherHeaderHandle RVCALLCONV MsgGetHeaderByNameFromList
                                            (IN    RvSipMsgHandle             hSipMsg,
                                             IN    RvChar*                   strName,
                                             IN    RvSipHeadersLocation       location,
                                             INOUT RvSipHeaderListElemHandle* hListElem);

static void MsgClean( IN MsgMessage* pMsg,
                      IN RvBool      bCleanBS);

#ifndef RV_SIP_PRIMITIVES
static RvStatus MsgSetRpoolBody(IN MsgMessage *pMsg,
                                IN RPOOL_Ptr  *pRpoolPtr);
#endif /*#ifndef RV_SIP_PRIMITIVES*/

#ifdef RV_SIP_JSR32_SUPPORT
static RvStatus RVCALLCONV MsgGetNumOfHeaders(
											  IN  RvSipMsgHandle     hMsg,
											  OUT RvUint32*          pNumOfHeaders);
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */											  
											  
/****************************************************/
/*        CONSTRUCTORS AND DESTRUCTORS                */
/****************************************************/


/***************************************************************************
 * RvSipMsgConstruct
 * ------------------------------------------------------------------------
 * General: Constructs SIP message object. The object is constructed on a
 *          page taken from a given memory pool.
 * Return Value: RV_OK or RV_ERROR_OUTOFRESOURCES.
 * ------------------------------------------------------------------------
 * Arguments:
 *  input:
 *      hMsgMgr - Handle to the message manager.
 *      hPool   - Handle to the memory pool. The Construct() function uses a page
 *                from this pool for the newly created message object.
 *  output:
 *        hSipMsg - Handle to the new message object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgConstruct(IN  RvSipMsgMgrHandle hMsgMgr,
                                             IN  HRPOOL            hPool,
                                             OUT RvSipMsgHandle*   hSipMsg)
{

    HPAGE     hPage;
    RvStatus stat;
    MsgMessage* pNewMsg;
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if((hPool == NULL)||(pMsgMgr == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if(hSipMsg == NULL)
        return RV_ERROR_NULLPTR;

    *hSipMsg = NULL;

    stat = RPOOL_GetPage(hPool, 0, &hPage);
    if(stat != RV_OK)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipMsgConstruct - Failed to construct new message. RPOOL_GetPage failed on pool 0x%p. status id %d",
            hPool, stat));
        return stat;
    }
    else
    {
        RvLogDebug(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "RvSipMsgConstruct - Got a new page. hPage is 0x%p.",
            hPage));
    }

    pNewMsg = (MsgMessage*)SipMsgUtilsAllocAlign(pMsgMgr, hPool, hPage, sizeof(MsgMessage), RV_TRUE);
    if(pNewMsg == NULL)
    {
        RPOOL_FreePage(hPool, hPage);
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "RvSipMsgConstruct - Failed to construct message, free the new page. hPage 0x%p, hPool 0x%p.",
                hPage, hPool));
        return RV_ERROR_OUTOFRESOURCES;
    }
    pNewMsg->pMsgMgr           = (MsgMgr*)hMsgMgr;
    pNewMsg->hPage             = hPage;
    pNewMsg->hPool             = hPool;

    MsgClean(pNewMsg, RV_TRUE);
	MsgConstructStatusLine(pNewMsg);
	MsgConstructRequestLine(pNewMsg);
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    pNewMsg->f_Encoder = NULL;
    pNewMsg->Encoder_cfg.cfg = NULL;
    pNewMsg->SPIRENT_msgBody   = NULL;
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */


    *hSipMsg = (RvSipMsgHandle)pNewMsg;
    
    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "RvSipMsgConstruct - Message was constructed successfully. The new message handle is 0x%p.",
                pNewMsg));
    return RV_OK;
}

/***************************************************************************
 * RvSipMsgDestruct
 * ------------------------------------------------------------------------
 * General: Destructor. Destroys the message object and frees all the object
 *          resources.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSipMsg - Handle to the message object to destruct.
 ***************************************************************************/
RVAPI void RVCALLCONV RvSipMsgDestruct(IN  RvSipMsgHandle  hSipMsg)
{
    MsgMessage* msg = (MsgMessage*)hSipMsg;
    HRPOOL      hPool;
    HPAGE       hPage;


    if(msg == NULL)
        return;
    if(msg->pMsgMgr == NULL)
        return;
#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT
	if (msg->terminationCounter > 0)
	{
		msg->terminationCounter--;
		return;
	}
#endif /* #ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */

    /* making it impossible to use destructed messages */
    hPool   = msg->hPool;
    hPage   = msg->hPage;
    msg->hPage = NULL_PAGE;
    msg->hPool = NULL;

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    if ( msg->SPIRENT_msgBody )
    {
        HSTPOOL_Release ( msg->SPIRENT_msgBody );
        msg->SPIRENT_msgBody = NULL;
    }
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

    RvLogInfo(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "RvSipMsgDestruct - Destructing message 0x%p ,free page 0x%p on pool 0x%p",
                hSipMsg, hPage, hPool));
    RPOOL_FreePage(hPool, hPage);
}


/***************************************************************************
 * RvSipMsgCopy
 * ------------------------------------------------------------------------
 * General: Copies all fields from a source message object to a destination
 *          message object.You should construct the destination message object
 *          before using this function.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hDestination - Handle to the destination message to which values
 *                         are copied.
 *          hSource      - Handle of the source msg.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgCopy(INOUT RvSipMsgHandle hDestination,
                                        IN    RvSipMsgHandle hSource)
{
    RvStatus                   stat;
    MsgMessage*                 dest;
    MsgMessage*                 source;
    RvSipHeaderListElemHandle   sourceElem, destElem;
    RvSipHeaderType             sourceType;
    void*                       returnedHeader;

    if((hDestination == NULL) || (hSource == NULL))
        return RV_ERROR_INVALID_HANDLE;

    dest   = (MsgMessage*)hDestination;
    source = (MsgMessage*)hSource;

    RvLogInfo(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
              "RvSipMsgCopy - Copy message 0x%p to message 0x%p", source, dest));

    /*dest->bIsRequest = UNDEFINED;*/

    /* coping startLine */
    if(source->bIsRequest == RV_TRUE) /* request */
    {
        MsgConstructRequestLine(dest);

        dest->bIsRequest = RV_TRUE;

        if(source->strBadSyntaxStartLine > UNDEFINED) /* bad syntax request line */
        {
            dest->strBadSyntaxStartLine =
                MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                dest->hPool,
                                                dest->hPage,
                                                source->hPool,
                                                source->hPage,
                                                source->strBadSyntaxStartLine);
            if(dest->strBadSyntaxStartLine == UNDEFINED)
            {
                RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                    "RvSipMsgCopy Failed for message 0x%p (bad-syntax start line) - no more resources", dest));
                return RV_ERROR_OUTOFRESOURCES;
            }
        }
        else /* legal request line */
        {
            /* Method */
            dest->startLine.requestLine.eMethod = source->startLine.requestLine.eMethod;
            if(source->startLine.requestLine.strMethod > UNDEFINED)
            {
                dest->startLine.requestLine.strMethod =
                                        MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                        dest->hPool,
                                        dest->hPage,
                                        source->hPool,
                                        source->hPage,
                                        source->startLine.requestLine.strMethod);
                if(dest->startLine.requestLine.strMethod == UNDEFINED)
                {
                    RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                        "RvSipMsgCopy Failed for message 0x%p - no more resources", dest));
                    return RV_ERROR_OUTOFRESOURCES;
                }

#ifdef SIP_DEBUG
                dest->startLine.requestLine.pMethod =
                    (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                    dest->startLine.requestLine.strMethod);
#endif /* SIP_DEBUG */
            }
            else
            {
                dest->startLine.requestLine.strMethod = UNDEFINED;
#ifdef SIP_DEBUG
                dest->startLine.requestLine.pMethod = NULL;
#endif /* SIP_DEBUG */
            }

            /* RequestUri */
            stat = RvSipMsgSetRequestUri(hDestination, source->startLine.requestLine.hRequestUri);
            if(stat != RV_OK)
            {
                RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                    "RvSipMsgCopy Failed for message 0x%p - failed to set Request-Uri, stat = %d",
                    dest, stat));
                return stat;
            }
        }

    }
    else if (source->bIsRequest == RV_FALSE) /* response */
    {
        MsgConstructStatusLine(dest);

        dest->bIsRequest = RV_FALSE;

        /* statusCode */
        dest->startLine.statusLine.statusCode = source->startLine.statusLine.statusCode;

        /*pReasonPharse */
        if(source->startLine.statusLine.strReasonePhrase > UNDEFINED)
        {
            dest->startLine.statusLine.strReasonePhrase =
                   MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                dest->hPool,
                                                dest->hPage,
                                                source->hPool,
                                                source->hPage,
                                                source->startLine.statusLine.strReasonePhrase);
            if(dest->startLine.statusLine.strReasonePhrase == UNDEFINED)
            {
                RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                          "RvSipMsgCopy Failed for message 0x%p - no more resources", dest));
                return RV_ERROR_OUTOFRESOURCES;
            }
#ifdef SIP_DEBUG
            dest->startLine.statusLine.pReasonePhrase =
                        (RvChar *)RPOOL_GetPtr(
                                  dest->hPool, dest->hPage,
                                  dest->startLine.statusLine.strReasonePhrase);
#endif /* SIP_DEBUG */
        }
        else
        {
            dest->startLine.statusLine.strReasonePhrase = UNDEFINED;
#ifdef SIP_DEBUG
            dest->startLine.statusLine.pReasonePhrase = NULL;
#endif /* SIP_DEBUG */
        }
    }


#ifndef RV_SIP_PRIMITIVES
    /* Copy the message body */
    stat = RvSipMsgSetBodyObject(hDestination,
                                 (RvSipBodyHandle)source->pBodyObject);
    if (stat != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                  "RvSipMsgCopy Failed for message 0x%p - failed to set body, stat = %d",
                  dest, stat));
        return stat;
    }
#else /* #ifndef RV_SIP_PRIMITIVES */
    /* contentType */
    if(source->strContentType > UNDEFINED)
    {
        dest->strContentType =
            MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                            dest->hPool,
                                            dest->hPage,
                                            source->hPool,
                                            source->hPage,
                                            source->strContentType);
        if(dest->strContentType == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipMsgCopy Failed for message 0x%p - no more resources", dest));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pContentType = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                          dest->strContentType);
#endif /*  SIP_DEBUG */
    }
    else
    {
        dest->strContentType = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pContentType = NULL;
#endif /*  SIP_DEBUG */
    }

    /* strBody */
    if(source->strBody > UNDEFINED)
    {
        dest->strBody = MsgUtilsAllocCopyRpoolString(source->pMsgMgr,
                                                       dest->hPool,
                                                       dest->hPage,
                                                       source->hPool,
                                                       source->hPage,
                                                       source->strBody);

        if(dest->strBody == UNDEFINED)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                      "RvSipMsgCopy Failed for message 0x%p - no more resources", dest));
            return RV_ERROR_OUTOFRESOURCES;
        }
#ifdef SIP_DEBUG
        dest->pBody = (RvChar *)RPOOL_GetPtr(dest->hPool, dest->hPage,
                                          dest->strBody);
#endif
    }
    else
    {
        dest->strBody = UNDEFINED;
#ifdef SIP_DEBUG
        dest->pBody = NULL;
#endif
    }
#endif /* #ifndef RV_SIP_PRIMITIVES */

    /* contentLength */
    dest->contentLength   = source->contentLength;
    dest->bBadSyntaxContentLength = source->bBadSyntaxContentLength;
    dest->bCompContentLen = source->bCompContentLen;
    dest->bForceCompactForm  = source->bForceCompactForm;

    /* coping the headers */
    /* From header */
#ifndef RV_SIP_LIGHT
    stat = RvSipMsgSetFromHeader(hDestination, source->hFromHeader);
    if (stat != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                  "RvSipMsgCopy Failed for message 0x%p - failed to set From header, stat = %d",
                  dest, stat));
        return stat;
    }

    /* To header */
    stat = RvSipMsgSetToHeader(hDestination, source->hToHeader);
    if (stat != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                  "RvSipMsgCopy Failed for message 0x%p - failed to set To header, stat = %d",
                  dest, stat));
        return stat;
    }
    /*if there is a call-id, copy it*/
    if(source->hCallIdHeader != NULL)
    {

        /* Call-ID header */
        /* First set the header name (that might be compact) */
        stat = SipMsgSetCallIdHeaderName(hDestination, NULL, source->hPool, source->hPage,
            SipMsgGetCallIdHeaderName(hSource));
        if (stat != RV_OK)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipMsgCopy Failed for message 0x%p - failed to set Call-ID name, stat = %d",
                dest, stat));
            return stat;
        }
        /* Now the Call-ID header value is set */
        stat = SipMsgSetCallIdHeader(hDestination, NULL, source->hPool, source->hPage,
            SipMsgGetCallIdHeader(hSource));
        if (stat != RV_OK)
        {
            RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                "RvSipMsgCopy Failed for message 0x%p - failed to set Call-ID, stat = %d",
                dest, stat));
            return stat;
        }
    }

    /* CSeq header */
    stat = RvSipMsgSetCSeqHeader(hDestination, source->hCSeqHeader);
    if (stat != RV_OK)
    {
        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                  "RvSipMsgCopy Failed for message 0x%p - failed to set CSeq header, stat = %d",
                  dest, stat));
        return stat;
    }
#endif /*#ifndef RV_SIP_LIGHT*/
    /* headerList */
    /* if there is already header list in the dest msg, we should free it,
    and construct a new one... */
    if(dest->headerList != NULL)
    {
        /* deleting the exist list*/
        dest->headerList = NULL;
        /* the creation of the new list, will be done in the first push function. */
    }
    if (source->headerList != NULL)
    {

        /* getting first header from source list */
        returnedHeader = MsgGetHeaderFromList(hSource,
                                           RVSIP_FIRST_HEADER,
                                           &sourceElem,
                                           &sourceType);
        if(returnedHeader == NULL)
        {
            dest->headerList = NULL;
        }
        else
        {
            RvInt  safeCounter = 0;

            /* setting the first element in dest list */

            stat = RvSipMsgPushHeader(hDestination,
                                      RVSIP_FIRST_HEADER,
                                      returnedHeader,
                                      sourceType,
                                      &destElem,
                                      &returnedHeader);
            if (stat != RV_OK)
            {
                RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                          "RvSipMsgCopy Failed for message 0x%p - failed to copy headers list, stat = %d",
                           dest, stat));
                return stat;
            }

            /* passing the source list, and push all to the dest list */
            while((returnedHeader != NULL) && (safeCounter < MAX_HEADERS_PER_MESSAGE))
            {
                returnedHeader = MsgGetHeaderFromList(hSource,
                                                  RVSIP_NEXT_HEADER,
                                                  &sourceElem,
                                                  &sourceType);
                ++ safeCounter;

                if(returnedHeader == NULL)
                {
                    break;
                }
                else
                {
                    stat = RvSipMsgPushHeader(hDestination,
                                              RVSIP_NEXT_HEADER,
                                              returnedHeader,
                                              sourceType,
                                              &destElem,
                                              &returnedHeader);
                    if(stat != RV_OK)
                    {
                        RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                                  "RvSipMsgCopy Failed for message 0x%p - failed to copy headers list, stat = %d",
                                  dest, stat));
                        return stat;
                    }
                }
            } /* while */
            if(safeCounter >= MAX_HEADERS_PER_MESSAGE)
            {
                RvLogError(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
                    "RvSipMsgCopy: msg 0x%p - too many headers in list (more than %d) - failed to copy all", 
                    source, MAX_HEADERS_PER_MESSAGE));
            }
        } /* else */
    } /* header list */
    /*else
    {
        dest->headerList = NULL;
    }*/

    RvLogDebug(source->pMsgMgr->pLogSrc,(source->pMsgMgr->pLogSrc,
              "RvSipMsgCopy finished succesfully for message 0x%p",
              dest));
    return RV_OK;
}

/***************************************************************************
 * RvSipMsgEncode
 * ------------------------------------------------------------------------
 * General: Encodes a message object to a textual SIP message.
 *          The textual SIP message is placed on a page taken from a given
 *          memory pool. In order to copy the message from the page to a
 *          consecutive buffer, use RPOOL_CopyToExternal().
 *          The application must free the allocated page, using RPOOL_FreePage().
 *          The allocated page must be freed only if this function
 *          returns RV_OK.
 * Return Value: RV_OK, RV_ERROR_UNKNOWN, RV_ERROR_BADPARAM, RV_ERROR_OUTOFRESOURCES.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSipMsg - Handle to the message to encode.
 *          hPool   - Handle to a pool.
 * Output:  hPage   - The memory page allocated to hold the encoded message.
 *          length  - The length of the encoded message.
 ***************************************************************************/
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
RVAPI RvStatus RVCALLCONV RADVISION_RvSipMsgEncode(IN  RvSipMsgHandle hSipMsg,
                                                   IN  HRPOOL         hPool,
                                                   OUT HPAGE*         hPage,
                                                   OUT RvUint32*     length)
#else
RVAPI RvStatus RVCALLCONV RvSipMsgEncode(IN  RvSipMsgHandle hSipMsg,
                                         IN  HRPOOL         hPool,
                                         OUT HPAGE*         hPage,
                                         OUT RvUint32*      length)
#endif
/* SPIRENT_END */
{
    RvStatus rv;
    MsgMessage* msg = (MsgMessage*)hSipMsg;

    if(hPage == NULL || length == NULL)
        return RV_ERROR_NULLPTR;

    *hPage = NULL_PAGE;
    *length = 0;

    if((msg == NULL) ||(hPool == NULL))
        return RV_ERROR_INVALID_HANDLE;

    if(msg->hPool == NULL)
    {
        /* message was already destructed */
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = RPOOL_GetPage(hPool, 0, hPage);
    if(rv != RV_OK)
    {
        RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "RvSipMsgEncode - RPOOL_GetPage failed, hPool is 0x%p, hSipMsg is 0x%p",
                hPool, hSipMsg));
        return rv;
    }
    else
    {
        RvLogDebug(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "RvSipMsgEncode - Got new page 0x%p on pool 0x%p. hSipMsg is 0x%p",
                *hPage, hPool, hSipMsg));
    }

    *length = 0;
    rv =  MsgEncode(hSipMsg, RVSIP_MSG_HEADERS_OPTION_ALL, hPool, *hPage, length);

    if(rv != RV_OK)
    {
        RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "RvSipMsgEncode - hMsg 0x%p - MsgEncode failed, free page 0x%p, hPool is 0x%p",
                 hSipMsg, *hPage, hPool));
        RPOOL_FreePage(hPool, *hPage);
        return rv;
    }

    RvLogInfo(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "RvSipMsgEncode - hMsg 0x%p - MsgEncode Succeed, page 0x%p, hPool 0x%p",
                 hSipMsg, *hPage, hPool));
    return RV_OK;
}


/***************************************************************************
 * RvSipMsgGetStringLength
 * ------------------------------------------------------------------------
 * General: Some of the Message fields are kept in a string format-for
 *          example, the reason phrase of a response message. In order to get
 *          such a field from the Message object, your application should
 *          supply an adequate buffer to where the string will be copied.
 *          This function provides you with the length of the string to enable
 *          you to allocate an appropriate buffer size before calling the
 *          Get function.
 *          NOTE:
 *                If the message body is of type multipart it might be
 *                represented as a list of body parts, and not as a string. In
 *                order to retrieve the body string length the body object must
 *                be encoded if it is presented as a list of body
 *                parts. This function will encode the message body on a
 *                temporary memory page when ever the massage body object
 *                contains a list of body parts (in order to calculate the
 *                message body string length). If this function did not
 *                succeed to encode the message body (Error occured while
 *                encoding), 0 is returned and an appropriate ERROR can be
 *                found in the log.
 * Return Value: The length.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *       hHeader    - Handle to the message object.
 *     stringName - Enumeration of the string name for which you require the length.
 ***************************************************************************/
RVAPI RvUint RVCALLCONV RvSipMsgGetStringLength(
                                      IN  RvSipMsgHandle            hSipMsg,
                                      IN  RvSipMessageStringName    stringName)
{
    RvInt32   string;
    MsgMessage* msg = (MsgMessage*)hSipMsg;

    if(hSipMsg == NULL)
        return 0;

    switch (stringName)
    {
        case RVSIP_MSG_REQUEST_METHOD:
        {
            string = SipMsgGetStrRequestMethod(hSipMsg);
            break;
        }
        case RVSIP_MSG_RESPONSE_PHRASE:
        {
            string = SipMsgGetReasonPhrase(hSipMsg);
            break;
        }
        case RVSIP_MSG_CALL_ID:
        {
            string = SipMsgGetCallIdHeader(hSipMsg);
            break;
        }
        case RVSIP_MSG_CONTENT_TYPE:
        {
#ifndef RV_SIP_PRIMITIVES
            return GetContentTypeStringLength(msg);
#else
            string = SipMsgGetContentTypeHeader(hSipMsg);
            break;
#endif /*#ifndef RV_SIP_PRIMITIVES*/
        }
        case RVSIP_MSG_BODY:
        {
#ifndef RV_SIP_PRIMITIVES
            return GetBodyStringLength(msg);
#else
            string = SipMsgGetBody(hSipMsg);
            break;
#endif /*#ifndef RV_SIP_PRIMITIVES*/
        }
        case RVSIP_MSG_BAD_SYNTAX_START_LINE:
        {
            string = SipMsgGetBadSyntaxStartLine(hSipMsg);
            break;
        }
        default:
        {
            RvLogExcep(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "RvSipMsgGetStringLength - unknown stringName %d", stringName));
            return 0;
        }
    }
    if(string > UNDEFINED)
        return (RPOOL_Strlen(msg->hPool, msg->hPage, string)+1);
    else
        return 0;
}


/****************************************************/
/*        MESSAGE TYPES FUNCTIONS                     */
/****************************************************/

/***************************************************************************
 * RvSipMsgGetMsgType
 * ------------------------------------------------------------------------
 * General: Gets the message type-request, response or undefined.
 * Return Value: Returns the enumeration of the message type.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:    hSipMsg - Handle to the message object.
 ***************************************************************************/
RVAPI RvSipMsgType RVCALLCONV RvSipMsgGetMsgType(IN const RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;

    if(hSipMsg == NULL)
        return RVSIP_MSG_UNDEFINED;

    msg = (MsgMessage*)hSipMsg;

    switch(msg->bIsRequest)
    {
        case RV_TRUE:
            return RVSIP_MSG_REQUEST;

        case RV_FALSE:
            return RVSIP_MSG_RESPONSE;

        default:
            return RVSIP_MSG_UNDEFINED;
    }
}


/****************************************************/
/*        START LINE FUNCTIONS                         */
/****************************************************/

/***************************************************************************
 * RvSipMsgGetRequestMethod
 * ------------------------------------------------------------------------
 * General: Gets the method type from the message object request line.
 *          The request method is supplied in the enum RvSipMethodType.
 *          If the function returns RVSIP_METHOD_OTHER you can use the
 *          RvSipMsgGetStrMethod() to get the method in a string format.
 * Return Value: The method type.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg    - Handle of a message object.
 ***************************************************************************/
RVAPI RvSipMethodType RVCALLCONV RvSipMsgGetRequestMethod(IN  RvSipMsgHandle   hSipMsg)
{
    MsgMessage* msg;

    if(hSipMsg == NULL)
        return RVSIP_METHOD_UNDEFINED;

    msg = (MsgMessage*)hSipMsg;

    if(msg->bIsRequest == RV_FALSE)
    {
        return RVSIP_METHOD_UNDEFINED;
    }
    else
    {
        return msg->startLine.requestLine.eMethod;
    }
}


/***************************************************************************
 * RvSipMsgGetStrRequestMethod
 * ------------------------------------------------------------------------
 * General: Copies the method type string from the message object request
 *          line into a given buffer. Use this function if RvSipMsgGetRequestMethod()
 *          returns RVSIP_METHOD_OTHER. If the bufferLen is adequate, the
 *          function copies the parameter into the strBuffer.
 *          Otherwise, the function returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen
 *          parameter contains the required buffer length.
 * Return Value: RV_OK or RV_ERROR_INSUFFICIENT_BUFFER.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg   - Handle to the message object.
 *        strBuffer - buffer to fill with the requested parameter.
 *        bufferLen - the length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to
 *                     include a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgGetStrRequestMethod(
                                               IN  RvSipMsgHandle   hSipMsg,
                                               IN  RvChar*         strBuffer,
                                               IN  RvUint          bufferLen,
                                               OUT RvUint*         actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgMessage*)hSipMsg)->hPool,
                                  ((MsgMessage*)hSipMsg)->hPage,
                                  SipMsgGetStrRequestMethod(hSipMsg),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}


/***************************************************************************
 * RvSipMsgSetMethodInRequestLine
 * ------------------------------------------------------------------------
 * General: Sets the method type in the request line of the message object.
 *          The function gets an enumeration and string as input. The string
 *          is set as method only if the enumeration is RVSIP_METHOD_OTHER.
 *          Otherwise, the string is ignored.
 * Return Value: RV_OK,
 *               RV_ERROR_UNKNOWN - If the startLine of the message is not requestLine.
 *               RV_ERROR_OUTOFRESOURCES - If didn't manage to allocate space for
 *                            setting strMethod.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSipMsg - Handle of a message object.
 *            eRequsetMethod -Request method type to set in the message.
 *           strMethod - A textual string which indicates the method type
 *                      when eRequsetMethod is RVSIP_METHOD_OTHER.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetMethodInRequestLine(
                                             IN    RvSipMsgHandle  hSipMsg,
                                             IN    RvSipMethodType eRequsetMethod,
                                             IN    RvChar*        strMethod)
{
    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipMsgSetMethodInRequestLine(hSipMsg, eRequsetMethod, strMethod,
                                        NULL, NULL_PAGE, UNDEFINED);
}


/***************************************************************************
 * RvSipMsgGetRequestUri
 * ------------------------------------------------------------------------
 * General: Gets the request URI address object handle.
 * Return Value: Returns a handle to the URI address object or NULL if no
 *               URI address exists.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipMsg - Handle to a message object.
 ***************************************************************************/
RVAPI RvSipAddressHandle RVCALLCONV RvSipMsgGetRequestUri(IN RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;

    if(hSipMsg == NULL)
        return NULL;

    msg = (MsgMessage*)hSipMsg;

    if(msg->bIsRequest == RV_TRUE)
        return msg->startLine.requestLine.hRequestUri;
    else
        return NULL;
}


/***************************************************************************
 * RvSipMsgSetRequestUri
 * ------------------------------------------------------------------------
 * General: Sets the request URI address int he message object request line.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle to a message object.
 *         hSipUrl - Handle to an address object with the request URI.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetRequestUri(IN    RvSipMsgHandle     hSipMsg,
                                                 IN    RvSipAddressHandle hSipUrl)
{
    MsgMessage*         msg;
    RvStatus            stat;
    RvSipAddressType    currentAddrType = RVSIP_ADDRTYPE_UNDEFINED;
    MsgAddress         *pAddr           = (MsgAddress*)hSipUrl;
#ifdef RV_SIP_JSR32_SUPPORT
	RvInt32             displayName;
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */


    
	if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    msg = (MsgMessage*)hSipMsg;

    if(msg->bIsRequest == UNDEFINED)
    {
        MsgConstructRequestLine(msg);
    }
    else if(msg->bIsRequest == RV_FALSE)
    {
        /* we need to turn the msg from response to request */
        MsgConstructStatusLine(msg);
        MsgConstructRequestLine(msg);
    }
    msg->bIsRequest = RV_TRUE;

    /* this is a requestLine */
    if(hSipUrl == NULL)
    {
        msg->startLine.requestLine.hRequestUri = NULL;
        return RV_OK;
    }
    else
    {
        if(pAddr->eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
        {
               return RV_ERROR_BADPARAM;
        }
        if (NULL != msg->startLine.requestLine.hRequestUri)
        {
            currentAddrType = RvSipAddrGetAddrType(msg->startLine.requestLine.hRequestUri);
        }

        /* if no address object was allocated, we will construct it */
        if((msg->startLine.requestLine.hRequestUri == NULL) ||
           (currentAddrType != pAddr->eAddrType))
        {
            RvSipAddressHandle hSipAddr;

            stat = RvSipAddrConstructInStartLine(hSipMsg,
                                                 pAddr->eAddrType,
                                                 &hSipAddr);
            if(stat != RV_OK)
                return stat;
        }
		
#ifdef RV_SIP_JSR32_SUPPORT
		/* Reset the diplay name, which is not allowed in Request-uri */
		displayName = pAddr->strDisplayName;
		pAddr->strDisplayName = UNDEFINED;
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

        stat = RvSipAddrCopy(msg->startLine.requestLine.hRequestUri,
                             hSipUrl);

#ifdef RV_SIP_JSR32_SUPPORT
		/* Restore the display name */
		pAddr->strDisplayName = displayName;
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

		return stat;
    }
}

/***************************************************************************
 * RvSipMsgGetStatusCode
 * ------------------------------------------------------------------------
 * General: Gets the response code from the message object start-line.
 * Return Value: Returns a response code or UNDEFINED if there is no response
 *               code or the message object represents a request message.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg - Handle of a message object.
 ***************************************************************************/
RVAPI RvInt16 RVCALLCONV RvSipMsgGetStatusCode(IN RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;

    if(hSipMsg == NULL)
        return UNDEFINED;

    msg = (MsgMessage*)hSipMsg;
    if(msg->bIsRequest)
        return UNDEFINED;

    return msg->startLine.statusLine.statusCode;
}


/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
RVAPI RvChar*  RVCALLCONV RvSipMsgGetSPIRENTBody(IN RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;
    if(hSipMsg == NULL)
        return NULL;
    msg = (MsgMessage*)hSipMsg;
    return msg->SPIRENT_msgBody;
}

RVAPI void  RVCALLCONV RvSipMsgReleaseSPIRENTBody(IN RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;
    if(hSipMsg)
    {
        msg = (MsgMessage*)hSipMsg;
        if ( msg->SPIRENT_msgBody )
        {
            HSTPOOL_Release ( msg->SPIRENT_msgBody );
            msg->SPIRENT_msgBody = NULL;
        }
    }
}

RVAPI RvStatus RVCALLCONV RvSipMsgSetSPIRENTBody(IN RvSipMsgHandle hSipMsg, RvChar* msg_ptr)
{
    MsgMessage* msg;
    if(hSipMsg == NULL)
        return RV_Failure;
    msg = (MsgMessage*)hSipMsg;
    msg->SPIRENT_msgBody = msg_ptr;
    return RV_OK;
}

RVAPI RvStatus RVCALLCONV RvSipMsgSetExternalEncoder (IN RvSipMsgHandle hSipMsg, IN SPIRENT_RvSipMsgEncode_Type f_Encoder, IN SPIRENT_Encoder_Cfg_Type* cfg )
{
    MsgMessage* msg;
    if(hSipMsg == NULL)
        return RV_Failure;
    msg = (MsgMessage*)hSipMsg;
    msg->f_Encoder = f_Encoder;
    msg->Encoder_cfg = *cfg;
    return RV_OK;

}
RVAPI SPIRENT_RvSipMsgEncode_Type RVCALLCONV RvSipMsgGetExternalEncoder (IN RvSipMsgHandle hSipMsg, OUT SPIRENT_Encoder_Cfg_Type* cfg )
{
    MsgMessage* msg;
    if(hSipMsg == NULL)
        return NULL;
    msg = (MsgMessage*)hSipMsg;
    *cfg = msg->Encoder_cfg;
    return msg->f_Encoder;
}

void RvSipMsgSetMsgType(RvSipMsgHandle hSipMsg, RvSipMsgType mtype)
{
  MsgMessage* msg;
  
  if(hSipMsg) {
    
    msg = (MsgMessage*)hSipMsg;
    
    if(mtype==RVSIP_MSG_REQUEST) msg->bIsRequest=1;
    else msg->bIsRequest=0;
  }
}


RVAPI int  RvSipMsgGetapp_param(IN RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;
    if(hSipMsg == NULL)
        return RV_Failure;
    msg = (MsgMessage*)hSipMsg;
    return msg->Encoder_cfg.app_param;	
}


RVAPI RvStatus  RvSipMsgSetapp_param(IN int cctUH, IN RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;
    if(hSipMsg == NULL)
        return RV_Failure;
    msg = (MsgMessage*)hSipMsg;
    msg->Encoder_cfg.app_param = cctUH;
	return RV_OK;
}

#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

/***************************************************************************
 * RvSipMsgSetStatusCode
 * ------------------------------------------------------------------------
 * General: Sets a status code in the message object status line.
 *          This will set the message object to represent a response message.
 * Return Value: RV_OK,
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle to a message object.
 *         code    - status code to be set in the message object.
 *         insertDefaultReasonPhrase - Determines whether or not to insert
 *                     a default reason phrase into the status line.
 *                     the reason phrase is set according to the code number.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetStatusCode(
                                         IN    RvSipMsgHandle hSipMsg,
                                         IN    RvInt16       code,
                                         IN    RvBool        insertDefaultReasonPhrase)
{
    MsgMessage*   msg;
    RvInt32     newStr;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(code < 100 || code >= 700)
    {
        return RV_ERROR_BADPARAM;
    }

    msg = (MsgMessage*)hSipMsg;

    if(msg->bIsRequest == UNDEFINED)
    {
        MsgConstructStatusLine(msg);
    }
    else if(msg->bIsRequest == RV_TRUE)
    {
        /* we need to turn from request to response */
        MsgConstructRequestLine(msg);
        MsgConstructStatusLine(msg);
    }
    msg->bIsRequest = RV_FALSE;

    msg->startLine.statusLine.statusCode = code;


    if(insertDefaultReasonPhrase == RV_TRUE)
    {
        newStr = MsgUtilsAllocSetString(msg->pMsgMgr,
                                        msg->hPool,
                                        msg->hPage,
                                        SipMsgStatusCodeToString(code));
        if(newStr == UNDEFINED)
            return RV_ERROR_OUTOFRESOURCES;
        else
        {
            msg->startLine.statusLine.strReasonePhrase = newStr;
#ifdef SIP_DEBUG
           msg->startLine.statusLine.pReasonePhrase =
                       (RvChar *)RPOOL_GetPtr(msg->hPool, msg->hPage,
                                    msg->startLine.statusLine.strReasonePhrase);
#endif
        }
    }

    return RV_OK;
}


/***************************************************************************
 * RvSipMsgGetReasonPhrase
 * ------------------------------------------------------------------------
 * General: Copies the reason phrase from the message object status line
 *          into a given buffer. If the bufferLen size adequate, the
 *          function copies the parameter into the strBuffer. Otherwise,
 *          it returns RV_ERROR_INSUFFICIENT_BUFFER and the actualLen param contains
 *          the required buffer length.
 * Return Value: RV_OK or RV_ERROR_INSUFFICIENT_BUFFER.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg   - Handle of the message object.
 *        strBuffer - buffer to fill with the requested parameter.
 *        bufferLen - the length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgGetReasonPhrase(IN RvSipMsgHandle hSipMsg,
                                               IN RvChar*           strBuffer,
                                               IN RvUint            bufferLen,
                                               OUT RvUint*          actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgMessage*)hSipMsg)->hPool,
                                  ((MsgMessage*)hSipMsg)->hPage,
                                  SipMsgGetReasonPhrase(hSipMsg),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * RvSipMsgSetReasonPhrase
 * ------------------------------------------------------------------------
 * General:      Sets the reason phrase in the message object status line.
 * Return Value:  RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle of a message object.
 *         strReasonPhrase - Reason phrase to be set in the message object.
 *                           If NULL is supplied, the existing
 *                           reason phrase of the message is removed
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetReasonPhrase(IN    RvSipMsgHandle hSipMsg,
                                                   IN    RvChar*       strReasonPhrase)
{
    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    return SipMsgSetReasonPhrase(hSipMsg, strReasonPhrase, NULL,
                                 NULL_PAGE, UNDEFINED);
}

/****************************************************/
/*        HEADERS FUNCTIONS                           */
/****************************************************/
/***************************************************************************
 * RvSipMsgGetHeader
 * ------------------------------------------------------------------------
 * General: Gets a header from the header list. The message object holds most
 *          headers in a sequential list except, To, From, CallId, Cseq,
 *          ContentLength and ContentType headers.
 *          The function returns the handle to the requested header as a void
 *          pointer. You should check the pHeaderType parameter and cast the
 *          return value to the appropriate header handle.
 *          Note - The RvSipMsgGetHeaderExt() function is an extension of this
 *                 function
 *
 * Return Value: Returns the header handle as void*, or NULL if there is no header
 *               to retrieve.
 * ------------------------------------------------------------------------
 * Arguments:
 *  input:
 *      hSipMsg   - Handle of the message object.
 *      location  - The location on list-next, previous, first or last.
 *      hListElem - Handle to the current position in the list. Supply this
 *                  value if you chose next or previous in the location parameter.
 *                  This is also an output parameter and will be set with a link
 *                  to requested header in the list.
 *  output:
 *      hListElem - Handle to the current position in the list. Supply this
 *                  value if you chose next or previous in the location parameter.
 *                  This is also an output parameter and will be set with a link
 *                  to requested header in the list.
 *      pHeaderType  - The type of the retrieved header.
 ***************************************************************************/
RVAPI void* RVCALLCONV RvSipMsgGetHeader(
                                     IN    RvSipMsgHandle                hSipMsg,
                                     IN    RvSipHeadersLocation          location,
                                     INOUT RvSipHeaderListElemHandle*    hListElem,
                                     OUT   RvSipHeaderType*              pHeaderType)
{
    return RvSipMsgGetHeaderExt(hSipMsg,
                                location,
                                RVSIP_MSG_HEADERS_OPTION_LEGAL_SYNTAX,
                                hListElem,
                                pHeaderType);
}


/***************************************************************************
 * RvSipMsgGetHeaderByType
 * ------------------------------------------------------------------------
 * General: Gets a header by type from the header list. The message object
 *          holds most headers in a sequential list except, To, From, CallId,
 *          Cseq, ContentLength and ContentType headers.
 *          The function returns the handle to the requested header as a void
 *          pointer. You should cast the return value to the header type you
 *          requested.
 *          Note - The RvSipMsgGetHeaderByTypeExt() function is an extension of this
 *                 function
 * Return Value:Returns the header handle as void*, or NULL if there is no
 *               header to retrieve.
 * ------------------------------------------------------------------------
 * Arguments:
 *  input:
 *      hSipMsg     - Handle of the message object.
 *      eHeaderType - The header type to be retrieved.
 *      location    - The location on list-next, previous, first or last.
 *      hListElem - Handle to the current position in the list. Supply this
 *                  value if you chose next or previous in the location parameter.
 *                  This is also an output parameter and will be set with a link
 *                  to requested header in the list.
 *  output:
 *      hListElem - Handle to the current position in the list. Supply this
 *                  value if you chose next or previous in the location parameter.
 *                  This is also an output parameter and will be set with a link
 *                  to requested header in the list.
 ***************************************************************************/
RVAPI void* RVCALLCONV RvSipMsgGetHeaderByType(
                                         IN    RvSipMsgHandle             hSipMsg,
                                         IN    RvSipHeaderType            eHeaderType,
                                         IN    RvSipHeadersLocation       location,
                                         INOUT RvSipHeaderListElemHandle* hListElem)
{
    return RvSipMsgGetHeaderByTypeExt(hSipMsg,
                                      eHeaderType,
                                      location,
                                      RVSIP_MSG_HEADERS_OPTION_LEGAL_SYNTAX,
                                      hListElem);
}
/***************************************************************************
 * RvSipMsgGetHeaderByName
 * ------------------------------------------------------------------------
 * General: Gets a header by name from the header list.
 *          The message object holds most headers in a sequential list except,
 *          To, From, CallId, Cseq, ContentLength and ContentType headers.
 *          This function should be used only for headers of type
 *          RVSIP_HEADERTYPE_OTHER.
 *          Note - The RvSipMsgGetHeaderByNameExt() function is an extension of this
 *                 function
 * Return Value: Returns the Other header handle, or NULL if no Other header
 *                handle with the same name exists.
 * ------------------------------------------------------------------------
 * Arguments:
 *  input:
 *      hSipMsg   - Handle of the message object.
 *      strName   - The header name to be retrieved.
 *      location    - The location on list-next, previous, first or last.
 *      hListElem - Handle to the current position in the list. Supply this
 *                  value if you chose next or previous in the location parameter.
 *                  This is also an output parameter and will be set with a link
 *                  to requested header in the list.
 *  output:
 *      hListElem - Handle to the current position in the list. Supply this
 *                  value if you chose next or previous in the location parameter.
 *                  This is also an output parameter and will be set with a link
 *                  to requested header in the list.
 ***************************************************************************/
RVAPI RvSipOtherHeaderHandle RVCALLCONV RvSipMsgGetHeaderByName(
                                             IN    RvSipMsgHandle             hSipMsg,
                                             IN    RvChar*                   strName,
                                             IN    RvSipHeadersLocation       location,
                                             INOUT RvSipHeaderListElemHandle* hListElem)
{
    return RvSipMsgGetHeaderByNameExt(hSipMsg,
                                    strName,
                                    location,
                                    RVSIP_MSG_HEADERS_OPTION_LEGAL_SYNTAX,
                                    hListElem);

}

/***************************************************************************
  * RvSipMsgGetHeaderExt
  * ------------------------------------------------------------------------
  * General: Gets a header from the header list. The message object holds most
  *          headers in a sequential list except, To, From, CallId, Cseq,
  *          ContentLength and ContentType headers.
  *          The function returns the handle to the requested header as a void
  *          pointer. You should check the pHeaderType parameter and cast the
  *          return value to the appropriate header handle.
  *          The header list can hold both valid syntax and bad syntax headers.
  *          Using the eOption parameter you should specify whether you wish to get
  *          any type of header or only headers with valid syntax.
  *          Note: this function extends the functionality of
  *                RvSipMsgGetHeader function.
  * Return Value: Returns the header handle as void*, or NULL if there is no header
  *               to retrieve.
  * ------------------------------------------------------------------------
  * Arguments:
  *  input:
  *        hSipMsg   - Handle of the message object.
  *      location  - The location on list: next, previous, first or last.
  *      eOption   - Specifies whether the application wish to get Only legal syntax headers,
  *                  or any header (legal and illegal headers)
  *      hListElem - Handle to the current position in the list. Supply this
  *                  value if you chose next or previous in the location parameter.
  *                  This is also an output parameter and will be set with a link
  *                  to requested header in the list.
  *  output:
  *      hListElem - Handle to the current position in the list. Supply this
  *                  value if you chose next or previous in the location parameter.
  *                  This is also an output parameter and will be set with a link
  *                  to requested header in the list.
  *        pHeaderType  - The type of the retrieved header.
  ***************************************************************************/
RVAPI void* RVCALLCONV RvSipMsgGetHeaderExt(
                                     IN    RvSipMsgHandle                hSipMsg,
                                     IN    RvSipHeadersLocation          location,
                                     IN    RvSipMsgHeadersOption         eOption,
                                     INOUT RvSipHeaderListElemHandle*    hListElem,
                                     OUT   RvSipHeaderType*              pHeaderType)
{
    void* pHeader = NULL;

    if(hSipMsg == NULL || hListElem == NULL || pHeaderType == NULL)
    {
        return NULL;
    }
    pHeader = MsgGetHeaderFromList(hSipMsg, location, hListElem, pHeaderType);

    switch(eOption)
    {
    case RVSIP_MSG_HEADERS_OPTION_LEGAL_SYNTAX:
        if(location == RVSIP_FIRST_HEADER || location == RVSIP_NEXT_HEADER)
        {
            /* search forward to find a legal header */
            while((RvSipHeaderIsBadSyntax(pHeader, *pHeaderType) == RV_TRUE) && (pHeader != NULL))
            {
                pHeader = MsgGetHeaderFromList(hSipMsg, RVSIP_NEXT_HEADER, hListElem, pHeaderType);
            }
        }
        else /*if(location == RVSIP_LAST_HEADER || location == RVSIP_PREV_HEADER)*/
        {
            /* search backward to find a legal header */
            while((RvSipHeaderIsBadSyntax(pHeader, *pHeaderType) == RV_TRUE) && (pHeader != NULL))
            {
                pHeader = MsgGetHeaderFromList(hSipMsg, RVSIP_PREV_HEADER, hListElem, pHeaderType);
            }
        }
        return pHeader;

    case RVSIP_MSG_HEADERS_OPTION_ALL:
        return pHeader;

    case RVSIP_MSG_HEADERS_OPTION_UNDEFINED:
    default:
        return NULL;
    }
}

/***************************************************************************
  * RvSipMsgGetHeaderByTypeExt
  * ------------------------------------------------------------------------
  * General: Gets a header by type from the header list. The message object
  *          holds most headers in a sequential list except, To, From, CallId,
  *          Cseq, ContentLength and ContentType headers.
  *          The function returns the handle to the requested header as a void
  *          pointer. You should cast the return value to the header type you
  *          requested.
  *          The header list can hold both valid syntax and bad syntax headers.
  *          Using the eOption parameter you should specify whether you wish to get
  *          any type of header or only headers with valid syntax.
  *          Note: this function extends the functionality of
  *                RvSipMsgGetHeaderByType function.
  * Return Value:Returns the header handle as void*, or NULL if there is no
  *               header to retrieve.
  * ------------------------------------------------------------------------
  * Arguments:
  *  input:
  *        hSipMsg   - Handle of the message object.
  *        eHeaderType - The header type to be retrieved.
  *      location  - The location on list-next, previous, first or last.
 *      eOption   - Specifies whether the application wish to get only legal syntax headers,
  *                  or any header (legal and illegal headers)
  *      hListElem - Handle to the current position in the list. Supply this
  *                  value if you chose next or previous in the location parameter.
  *                  This is also an output parameter and will be set with a link
  *                  to requested header in the list.
  *  output:
  *      hListElem - Handle to the current position in the list. Supply this
  *                  value if you chose next or previous in the location parameter.
  *                  This is also an output parameter and will be set with a link
  *                  to requested header in the list.
  ***************************************************************************/
RVAPI void* RVCALLCONV RvSipMsgGetHeaderByTypeExt(
                                         IN    RvSipMsgHandle             hSipMsg,
                                         IN    RvSipHeaderType            eHeaderType,
                                         IN    RvSipHeadersLocation       location,
                                         IN    RvSipMsgHeadersOption      eOption,
                                         INOUT RvSipHeaderListElemHandle* hListElem)
{
    void* pHeader = NULL;

    if(hSipMsg == NULL || hListElem == NULL )
    {
        return NULL;
    }

    pHeader = MsgGetHeaderByTypeFromList(hSipMsg, eHeaderType, location, hListElem);

    switch(eOption)
    {
    case RVSIP_MSG_HEADERS_OPTION_LEGAL_SYNTAX:
        while((RvSipHeaderIsBadSyntax(pHeader, eHeaderType) == RV_TRUE) && (pHeader != NULL))
        {
            pHeader = MsgGetHeaderByTypeFromList(hSipMsg, eHeaderType, RVSIP_NEXT_HEADER, hListElem);
        }
        return pHeader;

    case RVSIP_MSG_HEADERS_OPTION_ALL:
        return pHeader;

    case RVSIP_MSG_HEADERS_OPTION_UNDEFINED:
    default:
        return NULL;
    }
}
/***************************************************************************
  * RvSipMsgGetHeaderByNameExt
  * ------------------------------------------------------------------------
  * General:
  *          Gets a header by name from the header list.
  *          The message object holds most headers in a sequential list except,
  *          To, From, CallId, Cseq, ContentLength and ContentType headers.
  *          This function should be used only for headers of type
  *          RVSIP_HEADERTYPE_OTHER.
  *          The header list can hold both valid syntax and bad syntax headers.
  *          Using the eOption parameter you should specify whether you wish to get
  *          any type of header or only headers with valid syntax.
  *          Note: this function extends the functionality of
  *                RvSipMsgGetHeaderByName function.
  * Return Value: Returns the Other header handle, or NULL if no Other header
  *                handle with the same name exists.
  * ------------------------------------------------------------------------
  * Arguments:
  *  input:
  *        hSipMsg   - Handle of the message object.
  *        strName   - The header name to be retrieved.
  *      location  - The location on list-next, previous, first or last.
  *      eOption   - Specifies whether the application wish to get only legal syntax headers,
  *                  or any header (legal and illegal headers)
  *      hListElem - Handle to the current position in the list. Supply this
  *                  value if you chose next or previous in the location parameter.
  *                  This is also an output parameter and will be set with a link
  *                  to requested header in the list.
  *  output:
  *      hListElem - Handle to the current position in the list. Supply this
  *                  value if you chose next or previous in the location parameter.
  *                  This is also an output parameter and will be set with a link
  *                  to requested header in the list.
  ***************************************************************************/
RVAPI RvSipOtherHeaderHandle RVCALLCONV RvSipMsgGetHeaderByNameExt(
                                             IN    RvSipMsgHandle             hSipMsg,
                                             IN    RvChar*                   strName,
                                             IN    RvSipHeadersLocation       location,
                                             IN    RvSipMsgHeadersOption      eOption,
                                             INOUT RvSipHeaderListElemHandle* hListElem)
{
    RvSipOtherHeaderHandle pHeader = NULL;

    if(hSipMsg == NULL || hListElem == NULL)
    {
        return NULL;
    }

    pHeader = MsgGetHeaderByNameFromList(hSipMsg, strName, location, hListElem);

    switch(eOption)
    {
    case RVSIP_MSG_HEADERS_OPTION_LEGAL_SYNTAX:
        while((RvSipHeaderIsBadSyntax((void*)pHeader, RVSIP_HEADERTYPE_OTHER) == RV_TRUE) && (pHeader != NULL))
        {
            pHeader = MsgGetHeaderByNameFromList(hSipMsg, strName, RVSIP_NEXT_HEADER, hListElem);
        }
        return pHeader;

    case RVSIP_MSG_HEADERS_OPTION_ALL:
        return pHeader;

    case RVSIP_MSG_HEADERS_OPTION_UNDEFINED:
    default:
        return NULL;
    }

}

/***************************************************************************
* SipMsgIsBadSyntaxContentLength
* ------------------------------------------------------------------------
* General: Indicates if the message holds an invalid content-length header
*          value.
*
* Return Value: Returns the header handle as void*, or NULL if there is no header
*               to retrieve.
* ------------------------------------------------------------------------
* Arguments:
*  input:
*       hSipMsg   - Handle of the message object.
****************************************************************************/
RvBool RVCALLCONV SipMsgIsBadSyntaxContentLength(
                                     IN    RvSipMsgHandle                hSipMsg)
{
    MsgMessage*  pMsg = (MsgMessage*)hSipMsg;
    
    return pMsg->bBadSyntaxContentLength;
}
/***************************************************************************
* RvSipMsgGetBadSyntaxHeader
* ------------------------------------------------------------------------
* General: Gets a header with a syntax error from the message.
*          The message object holds most headers in a sequential list.
*          To, From, CallId, Cseq, and ContentType headers are held separately.
*          This function scans all headers in message object, (The ones that are in the header
*          list and the ones that are not) and retrieves only headers with syntax errors.
*          This function treats all headers as if they were located in one virtual list.
*          The virtual list includes the headers that are in the header list and the headers
*          that are not.
*          Note - this function does not check the ContentLength header value.
*          to check the validity of the content-length header, please check
*          its value, using RvSipMsgGetContentLengthHeader().
*          The function returns the handle to the requested header as a void
*          pointer. You should check the pHeaderType parameter and cast the
*          return value to the appropriate header handle.
*
* Return Value: Returns the header handle as void*, or NULL if there is no header
*               to retrieve.
* ------------------------------------------------------------------------
* Arguments:
*  input:
*       hSipMsg   - Handle of the message object.
*      location  - The location on the list: next, previous, first or last.
*      hListElem - Handle to the current position in the list. Supply this
*                  value if you chose next or previous in the location parameter.
*                  This is also an output parameter and will be set with a link
*                  to requested header in the list.
*  output:
*      hListElem - Handle to the current position in the list. Supply this
*                  value if you chose next or previous in the location parameter.
*                  This is also an output parameter and will be set with a link
*                  to requested header in the list.
*        pHeaderType  - The type of the retrieved header.
***************************************************************************/
RVAPI void* RVCALLCONV RvSipMsgGetBadSyntaxHeader(
                                     IN    RvSipMsgHandle                hSipMsg,
                                     IN    RvSipHeadersLocation          location,
                                     INOUT RvSipHeaderListElemHandle*    hListElem,
                                     OUT   RvSipHeaderType*              pHeaderType)
{
    void*        pHeader = NULL;
    MsgMessage*  pMsg = (MsgMessage*)hSipMsg;
    void*        eElem; /* casting between void* and enumeration are requeried by nucleus */

    RvInt32        safeCounter = 0;

    if(location == RVSIP_FIRST_HEADER)
    {
        *hListElem = NULL;
        eElem = (void*)BS_HEADER_FROM;
    }
    else
    {
        eElem = (void*)(*hListElem);
    }

    while (safeCounter < MAX_HEADERS_PER_MESSAGE)
    {
        safeCounter++;
        switch ((RvIntPtr)eElem)
        {
        case BS_HEADER_FROM:
            if(((MsgPartyHeader*)pMsg->hFromHeader) != NULL)
            {
                if(((MsgPartyHeader*)pMsg->hFromHeader)->strBadSyntax > UNDEFINED)
                {
                    /* from header is bad-syntax. return it, and set hListElem to be BS_HEADER_TO,
                    so in next enter to this function we will know to continue from To header checking */
                    *pHeaderType = RVSIP_HEADERTYPE_FROM;
                    *hListElem = (RvSipHeaderListElemHandle)BS_HEADER_TO;
                    return (void*)pMsg->hFromHeader;
                }
            }
            eElem = (void*)BS_HEADER_TO;
            break;

        case BS_HEADER_TO:
            if(((MsgPartyHeader*)pMsg->hToHeader) != NULL)
            {
                if(((MsgPartyHeader*)pMsg->hToHeader)->strBadSyntax > UNDEFINED)
                {
                    /* To header is bad-syntax. return it, and set hListElem to be BS_HEADER_CSeq,
                    so in next enter to this function we will know to continue from CSeq header checking */
                    *pHeaderType = RVSIP_HEADERTYPE_TO;
                    *hListElem = (RvSipHeaderListElemHandle)BS_HEADER_CSEQ;
                    return (void*)pMsg->hToHeader;
                }
            }
            eElem = (void*)BS_HEADER_CSEQ;
            break;

        case BS_HEADER_CSEQ:
            if(((MsgCSeqHeader*)pMsg->hCSeqHeader) != NULL)
            {
                if(((MsgCSeqHeader*)pMsg->hCSeqHeader)->strBadSyntax > UNDEFINED)
                {
                    /* CSeq header is bad-syntax. return it, and set hListElem to be BS_HEADER_CONTENT_TYPE,
                    so in next enter to this function we will know to continue from Content-type header checking */
                    *pHeaderType = RVSIP_HEADERTYPE_CSEQ;
                    *hListElem = (RvSipHeaderListElemHandle)BS_HEADER_CONTENT_TYPE;
                    return (void*)pMsg->hCSeqHeader;
                }
            }
            eElem = (void*)BS_HEADER_CONTENT_TYPE;
            break;

        case BS_HEADER_CONTENT_TYPE:
#ifndef RV_SIP_PRIMITIVES
            if(pMsg->pBodyObject != NULL)
            {
                if(pMsg->pBodyObject->pContentType != NULL)
                {
                    if(pMsg->pBodyObject->pContentType->strBadSyntax > UNDEFINED)
                    {
                        /* Content-type header is bad-syntax. return it, and set hListElem to be BS_HEADER_LIST,
                        so in next enter to this function we will know to continue from list headers checking */
                        *pHeaderType = RVSIP_HEADERTYPE_CONTENT_TYPE;
                        *hListElem = (RvSipHeaderListElemHandle)BS_HEADER_LIST;
                        return (void*)pMsg->pBodyObject->pContentType;
                    }
                }
            }
#endif /*#ifndef RV_SIP_PRIMITIVES*/
            eElem = (void*)BS_HEADER_LIST;
            break;

        case BS_HEADER_LIST:
            /* this case identify that this is the first time we access the headers list */
            location = RVSIP_FIRST_HEADER;
            /* no break - continue to next case */
        default:
            pHeader = MsgGetHeaderFromList(hSipMsg, location, hListElem, pHeaderType);
            while((pHeader != NULL))
            {
                if(RvSipHeaderIsBadSyntax(pHeader, *pHeaderType) == RV_TRUE)
                {
                    return pHeader;
                }
                else
                {
                    pHeader = MsgGetHeaderFromList(hSipMsg, RVSIP_NEXT_HEADER, hListElem, pHeaderType);
                }
            }
            return NULL;
        }
    }

    if(safeCounter >= MAX_HEADERS_PER_MESSAGE)
    {
        RvLogExcep(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
            "RvSipMsgGetBadSyntaxHeader: loop reached the limit. msg 0x%p", pMsg));
    }
    return NULL;
}

/***************************************************************************
 * RvSipMsgRemoveHeaderAt
 * ------------------------------------------------------------------------
 * General: Removes an header from the header list. The message object holds
 *          most headers in a sequential list except, To, From, CallId, Cseq,
 *          ContentLength and ContentType headers.
 *          You should supply this function with the list element of the
 *          header you wish to remove.
 * Return Value: RV_OK,RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle to the message object.
 *           hListElem - Handle to the list element to be removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgRemoveHeaderAt(
                                      IN    RvSipMsgHandle            hSipMsg,
                                      IN    RvSipHeaderListElemHandle hListElem)
{
    MsgMessage*     msg;

    if((hSipMsg == NULL) || (hListElem == NULL))
        return RV_ERROR_INVALID_HANDLE;

    msg = (MsgMessage*)hSipMsg;

    /* removing */
    RLIST_Remove (NULL , msg->headerList, (RLIST_ITEM_HANDLE)hListElem);

    return RV_OK;
}

/***************************************************************************
 * RvSipMsgPushHeader
 * ------------------------------------------------------------------------
 * General: Inserts a given header into the header list based on a given
 *          location. For example, first, last, before or after a given
 *          element. The message object holds most headers in a sequential
 *          list except, To, From, CallId, Cseq, ContentLength and
 *          ContentType headers. The header you supply is copied before it
 *          is inserted into the list. The pNewHeader output parameter
 *          contains the handle to the actual header pushed into the list.
 *          You should use this handle to refer to the header pushed into
 *          the list.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE, RV_ERROR_BADPARAM.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hSipMsg     - Handle of the message object.
 *             location    - The location on list-next, previous, first or last.
 *             pHeader     - Handle to the header pushed into the list.
 *             eHeaderType - Type of the header to be pushed.
 *      hListElem - Handle to the current position in the list. Supply this
 *                  value if you chose next or previous in the location parameter.
 *                  This is also an output parameter and will be set with a link
 *                  to requested header in the list.
 *  output:
 *      hListElem - Handle to the current position in the list. Supply this
 *                  value if you chose next or previous in the location parameter.
 *                  This is also an output parameter and will be set with a link
 *                  to requested header in the list.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgPushHeader(
                                    IN      RvSipMsgHandle                hSipMsg,
                                    IN      RvSipHeadersLocation          location,
                                    IN      void*                         pHeader,
                                    IN      RvSipHeaderType               eHeaderType,
                                    INOUT   RvSipHeaderListElemHandle*    hListElem,
                                    OUT     void**                        pNewHeader)
{
    RLIST_HANDLE       list;
    RLIST_ITEM_HANDLE  allocatedItem = NULL;
    RLIST_ITEM_HANDLE  insertLocation;
    MsgMessage*        msg;
    RvStatus          stat1=RV_OK, stat2=RV_OK;

    if ((hSipMsg == NULL) || (pHeader == NULL) || (hListElem == NULL))
        return RV_ERROR_INVALID_HANDLE;
    if(pNewHeader == NULL)
        return RV_ERROR_NULLPTR;

    msg = (MsgMessage*)hSipMsg;
    *pNewHeader = NULL;
    /* if HeaderList is NULL we will create a new list */
    if(msg->headerList == NULL)
    {
         list = RLIST_RPoolListConstruct(msg->hPool, msg->hPage,
                                         sizeof(MsgHeaderListElem), msg->pMsgMgr->pLogSrc);
         if(list == NULL)
            return RV_ERROR_OUTOFRESOURCES;
         else
            msg->headerList = list;
    }
    else
    {
        list = msg->headerList;
    }

    /* pushing */
    switch (location)
    {
        case RVSIP_FIRST_HEADER: /* push at the list beginning */
        {
            stat1 = RLIST_InsertHead (NULL, list, &allocatedItem);
            break;
        }
        case RVSIP_LAST_HEADER:
        {
            stat1 = RLIST_InsertTail (NULL, list, &allocatedItem);
            break;
        }
        case RVSIP_NEXT_HEADER: /* push at the list beginning */
        {
            insertLocation = (RLIST_ITEM_HANDLE)*hListElem;
            stat1           = RLIST_InsertAfter (NULL, list, insertLocation, &allocatedItem);
            break;
        }
        case RVSIP_PREV_HEADER:
        {
            insertLocation = (RLIST_ITEM_HANDLE)*hListElem;
            stat1           = RLIST_InsertBefore (NULL, list, insertLocation, &allocatedItem);
            break;
        }
        default:
            stat1 = RV_ERROR_BADPARAM;
            break;
    }
    if(stat1 != RV_OK)
        return stat1;

    /* set the correct information in the list element that was allocated. */
    ((MsgHeaderListElem*)allocatedItem)->headerType = eHeaderType;

    /* copy the given header to the list */

    switch (eHeaderType)
    {
#ifndef RV_SIP_LIGHT
        case RVSIP_HEADERTYPE_CONTACT:
        {
            RvSipContactHeaderHandle tempContact = (RvSipContactHeaderHandle)pHeader;

            if((((MsgContactHeader*)tempContact)->hPool == msg->hPool) &&
                (((MsgContactHeader*)tempContact)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipContactHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                    msg->hPool, msg->hPage, &tempContact);
                stat2 = RvSipContactHeaderCopy(tempContact, (RvSipContactHeaderHandle)pHeader);
                *pNewHeader = (void*)tempContact;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
       case RVSIP_HEADERTYPE_EXPIRES:
        {
            RvSipExpiresHeaderHandle tempExpires = (RvSipExpiresHeaderHandle)pHeader;

            if((((MsgExpiresHeader*)tempExpires)->hPool == msg->hPool) &&
                (((MsgExpiresHeader*)tempExpires)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipExpiresHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                   msg->hPool, msg->hPage, &tempExpires);
                stat2 = RvSipExpiresHeaderCopy(tempExpires, (RvSipExpiresHeaderHandle)pHeader);
                *pNewHeader = (void*)tempExpires;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
        case RVSIP_HEADERTYPE_DATE:
        {
            RvSipDateHeaderHandle tempExpires = (RvSipDateHeaderHandle)pHeader;

            if((((MsgDateHeader*)tempExpires)->hPool == msg->hPool) &&
                (((MsgDateHeader*)tempExpires)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipDateHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                 msg->hPool, msg->hPage, &tempExpires);
                stat2 = RvSipDateHeaderCopy(tempExpires, (RvSipDateHeaderHandle)pHeader);
                *pNewHeader = (void*)tempExpires;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
#endif /*#ifndef RV_SIP_LIGHT*/
        case RVSIP_HEADERTYPE_ROUTE_HOP:
        {
            RvSipRouteHopHeaderHandle tempRoute = (RvSipRouteHopHeaderHandle)pHeader;

            if((((MsgRouteHopHeader*)tempRoute)->hPool == msg->hPool) &&
                (((MsgRouteHopHeader*)tempRoute)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipRouteHopHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                   msg->hPool, msg->hPage, &tempRoute);
                stat2 = RvSipRouteHopHeaderCopy(tempRoute, (RvSipRouteHopHeaderHandle)pHeader);
                *pNewHeader = (void*)tempRoute;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }

#ifndef RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_EVENT:
        {
            RvSipEventHeaderHandle tempEvent = (RvSipEventHeaderHandle)pHeader;

            if((((MsgEventHeader*)tempEvent)->hPool == msg->hPool) &&
                (((MsgEventHeader*)tempEvent)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipEventHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                   msg->hPool, msg->hPage, &tempEvent);
                stat2 = RvSipEventHeaderCopy(tempEvent, (RvSipEventHeaderHandle)pHeader);
                *pNewHeader = (void*)tempEvent;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
        case RVSIP_HEADERTYPE_ALLOW:
        {
            RvSipAllowHeaderHandle tempAllow = (RvSipAllowHeaderHandle)pHeader;

            if((((MsgAllowHeader*)tempAllow)->hPool == msg->hPool) &&
                (((MsgAllowHeader*)tempAllow)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipAllowHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                   msg->hPool, msg->hPage, &tempAllow);
                stat2 = RvSipAllowHeaderCopy(tempAllow, (RvSipAllowHeaderHandle)pHeader);
                *pNewHeader = (void*)tempAllow;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
        case RVSIP_HEADERTYPE_RSEQ:
        {
            RvSipRSeqHeaderHandle tempRSeq = (RvSipRSeqHeaderHandle)pHeader;

            if((((MsgRSeqHeader*)tempRSeq)->hPool == msg->hPool) &&
                (((MsgRSeqHeader*)tempRSeq)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipRSeqHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, msg->hPage, &tempRSeq);
                stat2 = RvSipRSeqHeaderCopy(tempRSeq, (RvSipRSeqHeaderHandle)pHeader);
                *pNewHeader = (void*)tempRSeq;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
        case RVSIP_HEADERTYPE_RACK:
        {
            RvSipRAckHeaderHandle tempRAck = (RvSipRAckHeaderHandle)pHeader;

            if((((MsgRAckHeader*)tempRAck)->hPool == msg->hPool) &&
                (((MsgRAckHeader*)tempRAck)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipRAckHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                 msg->hPool, msg->hPage, &tempRAck);
                stat2 = RvSipRAckHeaderCopy(tempRAck, (RvSipRAckHeaderHandle)pHeader);
                *pNewHeader = (void*)tempRAck;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }

#ifdef RV_SIP_SUBS_ON
        /*--------------------------------------------------------------*/
        /*                    SUBS-REFER HEADERS                        */
        /*--------------------------------------------------------------*/
        case RVSIP_HEADERTYPE_REFER_TO:
        {
            RvSipReferToHeaderHandle tempReferTo = (RvSipReferToHeaderHandle)pHeader;

            if((((MsgReferToHeader*)tempReferTo)->hPool == msg->hPool) &&
                (((MsgReferToHeader*)tempReferTo)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipReferToHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                   msg->hPool, msg->hPage, &tempReferTo);
                stat2 = RvSipReferToHeaderCopy(tempReferTo, (RvSipReferToHeaderHandle)pHeader);
                *pNewHeader = (void*)tempReferTo;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
        case RVSIP_HEADERTYPE_REFERRED_BY:
        {
            RvSipReferredByHeaderHandle tempReferredBy = (RvSipReferredByHeaderHandle)pHeader;

            if((((MsgReferredByHeader*)tempReferredBy)->hPool == msg->hPool) &&
                (((MsgReferredByHeader*)tempReferredBy)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipReferredByHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                       msg->hPool, msg->hPage, &tempReferredBy);
                stat2 = RvSipReferredByHeaderCopy(tempReferredBy, (RvSipReferredByHeaderHandle)pHeader);
                *pNewHeader = (void*)tempReferredBy;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
        case RVSIP_HEADERTYPE_ALLOW_EVENTS:
        {
            RvSipAllowEventsHeaderHandle tempAllowEvents = (RvSipAllowEventsHeaderHandle)pHeader;

            if((((MsgAllowEventsHeader*)tempAllowEvents)->hPool == msg->hPool) &&
                (((MsgAllowEventsHeader*)tempAllowEvents)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipAllowEventsHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                   msg->hPool, msg->hPage, &tempAllowEvents);
                stat2 = RvSipAllowEventsHeaderCopy(tempAllowEvents, (RvSipAllowEventsHeaderHandle)pHeader);
                *pNewHeader = (void*)tempAllowEvents;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
        
        case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
        {
            RvSipSubscriptionStateHeaderHandle tempSubState = (RvSipSubscriptionStateHeaderHandle)pHeader;

            if((((MsgSubscriptionStateHeader*)tempSubState)->hPool == msg->hPool) &&
                (((MsgSubscriptionStateHeader*)tempSubState)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipSubscriptionStateHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                   msg->hPool, msg->hPage, &tempSubState);
                stat2 = RvSipSubscriptionStateHeaderCopy(tempSubState,
                                                    (RvSipSubscriptionStateHeaderHandle)pHeader);
                *pNewHeader = (void*)tempSubState;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
#endif /* #ifdef RV_SIP_SUBS_ON */
        case RVSIP_HEADERTYPE_REPLACES:
        {
            RvSipReplacesHeaderHandle tempReplaces = (RvSipReplacesHeaderHandle)pHeader;

            if((((MsgReplacesHeader*)tempReplaces)->hPool == msg->hPool) &&
                (((MsgReplacesHeader*)tempReplaces)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipReplacesHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                   msg->hPool, msg->hPage, &tempReplaces);
                stat2 = RvSipReplacesHeaderCopy(tempReplaces, (RvSipReplacesHeaderHandle)pHeader);
                *pNewHeader = (void*)tempReplaces;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
        case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
        {
            RvSipContentDispositionHeaderHandle tempContentDisp =
                                    (RvSipContentDispositionHeaderHandle)pHeader;

            if((((MsgContentDispositionHeader*)tempContentDisp)->hPool == msg->hPool) &&
                (((MsgContentDispositionHeader*)tempContentDisp)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipContentDispositionHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                         msg->hPool, msg->hPage, &tempContentDisp);
                stat2 = RvSipContentDispositionHeaderCopy(tempContentDisp, (RvSipContentDispositionHeaderHandle)pHeader);
                *pNewHeader = (void*)tempContentDisp;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
#endif /* #ifndef RV_SIP_PRIMITIVES*/

#ifdef RV_SIP_AUTH_ON
        case RVSIP_HEADERTYPE_AUTHENTICATION:
        {
            RvSipAuthenticationHeaderHandle tempAuthentication = (RvSipAuthenticationHeaderHandle)pHeader;

            if((((MsgAuthenticationHeader*)tempAuthentication)->hPool == msg->hPool) &&
                (((MsgAuthenticationHeader*)tempAuthentication)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipAuthenticationHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                       msg->hPool, msg->hPage, &tempAuthentication);
                stat2 = RvSipAuthenticationHeaderCopy(tempAuthentication, (RvSipAuthenticationHeaderHandle)pHeader);
                *pNewHeader = (void*)tempAuthentication;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
        case RVSIP_HEADERTYPE_AUTHORIZATION:
        {
            RvSipAuthorizationHeaderHandle tempAuthorization = (RvSipAuthorizationHeaderHandle)pHeader;

            if((((MsgAuthorizationHeader*)tempAuthorization)->hPool == msg->hPool) &&
                (((MsgAuthorizationHeader*)tempAuthorization)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipAuthorizationHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                         msg->hPool, msg->hPage, &tempAuthorization);
                stat2 = RvSipAuthorizationHeaderCopy(tempAuthorization, (RvSipAuthorizationHeaderHandle)pHeader);
                *pNewHeader = (void*)tempAuthorization;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifndef RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_RETRY_AFTER:
        {
            RvSipRetryAfterHeaderHandle tempRetryAfter = (RvSipRetryAfterHeaderHandle)pHeader;

            if((((MsgRetryAfterHeader*)tempRetryAfter)->hPool == msg->hPool) &&
                (((MsgRetryAfterHeader*)tempRetryAfter)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipRetryAfterHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                       msg->hPool, msg->hPage, &tempRetryAfter);
                stat2 = RvSipRetryAfterHeaderCopy(tempRetryAfter, (RvSipRetryAfterHeaderHandle)pHeader);
                *pNewHeader = (void*)tempRetryAfter;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }

        case RVSIP_HEADERTYPE_SESSION_EXPIRES:
        {
            RvSipSessionExpiresHeaderHandle tempSessionExpires = (RvSipSessionExpiresHeaderHandle)pHeader;

            if((((MsgSessionExpiresHeader*)tempSessionExpires)->hPool == msg->hPool) &&
                (((MsgSessionExpiresHeader*)tempSessionExpires)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipSessionExpiresHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                   msg->hPool, msg->hPage, &tempSessionExpires);
                stat2 = RvSipSessionExpiresHeaderCopy(tempSessionExpires, (RvSipSessionExpiresHeaderHandle)pHeader);
                *pNewHeader = (void*)tempSessionExpires;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
        case RVSIP_HEADERTYPE_MINSE:
        {
            RvSipMinSEHeaderHandle tempMinSE = (RvSipMinSEHeaderHandle)pHeader;

            if((((MsgMinSEHeader*)tempMinSE)->hPool == msg->hPool) &&
                (((MsgMinSEHeader*)tempMinSE)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipMinSEHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                   msg->hPool, msg->hPage, &tempMinSE);
                stat2 = RvSipMinSEHeaderCopy(tempMinSE, (RvSipMinSEHeaderHandle)pHeader);
                *pNewHeader = (void*)tempMinSE;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
#endif /* #ifndef RV_SIP_PRIMITIVES*/

        case RVSIP_HEADERTYPE_VIA:
        {
            RvSipViaHeaderHandle tempVia = (RvSipViaHeaderHandle)pHeader;

            if((((MsgViaHeader*)tempVia)->hPool == msg->hPool) &&
                (((MsgViaHeader*)tempVia)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipViaHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, msg->hPage, &tempVia);
                stat2 = RvSipViaHeaderCopy(tempVia, (RvSipViaHeaderHandle)pHeader);
                *pNewHeader = (void*)tempVia;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
        case RVSIP_HEADERTYPE_OTHER:
        {
            RvSipOtherHeaderHandle tempOther = (RvSipOtherHeaderHandle)pHeader;

            if((((MsgOtherHeader*)tempOther)->hPool == msg->hPool) &&
                (((MsgOtherHeader*)tempOther)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipOtherHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                 msg->hPool, msg->hPage, &tempOther);
                stat2 = RvSipOtherHeaderCopy(tempOther, (RvSipOtherHeaderHandle)pHeader);
                *pNewHeader = (void*)tempOther;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;

            break;
        }

#ifdef RV_SIP_AUTH_ON
		case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
        {
            RvSipAuthenticationInfoHeaderHandle tempAuthenticationInfo = (RvSipAuthenticationInfoHeaderHandle)pHeader;

            if((((MsgAuthenticationInfoHeader*)tempAuthenticationInfo)->hPool == msg->hPool) &&
                (((MsgAuthenticationInfoHeader*)tempAuthenticationInfo)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipAuthenticationInfoHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                         msg->hPool, msg->hPage, &tempAuthenticationInfo);
                stat2 = RvSipAuthenticationInfoHeaderCopy(tempAuthenticationInfo, (RvSipAuthenticationInfoHeaderHandle)pHeader);
                *pNewHeader = (void*)tempAuthenticationInfo;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
#endif /*#ifdef RV_SIP_AUTH_ON*/

#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
	case RVSIP_HEADERTYPE_REASON:
        {
            RvSipReasonHeaderHandle tempReason = (RvSipReasonHeaderHandle)pHeader;

            if((((MsgReasonHeader*)tempReason)->hPool == msg->hPool) &&
                (((MsgReasonHeader*)tempReason)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipReasonHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                   msg->hPool, msg->hPage, &tempReason);
                stat2 = RvSipReasonHeaderCopy(tempReason, (RvSipReasonHeaderHandle)pHeader);
                *pNewHeader = (void*)tempReason;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
		case RVSIP_HEADERTYPE_WARNING:
        {
            RvSipWarningHeaderHandle tempWarning = (RvSipWarningHeaderHandle)pHeader;

            if((((MsgWarningHeader*)tempWarning)->hPool == msg->hPool) &&
                (((MsgWarningHeader*)tempWarning)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
                stat1 = RvSipWarningHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                    msg->hPool, msg->hPage, &tempWarning);
                stat2 = RvSipWarningHeaderCopy(tempWarning, (RvSipWarningHeaderHandle)pHeader);
                *pNewHeader = (void*)tempWarning;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }

		case RVSIP_HEADERTYPE_TIMESTAMP:
        {
            RvSipTimestampHeaderHandle tempTimestamp = (RvSipTimestampHeaderHandle)pHeader;

            if((((MsgTimestampHeader*)tempTimestamp)->hPool == msg->hPool) &&
                (((MsgTimestampHeader*)tempTimestamp)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
					
                stat1 = RvSipTimestampHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                                         msg->hPool, 
																		 msg->hPage, 
																		 &tempTimestamp);
			
                stat2 = RvSipTimestampHeaderCopy(tempTimestamp,
					                             (RvSipTimestampHeaderHandle)pHeader);
                *pNewHeader = (void*)tempTimestamp;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }

		case RVSIP_HEADERTYPE_INFO:
        {
            RvSipInfoHeaderHandle tempInfo = (RvSipInfoHeaderHandle)pHeader;

            if((((MsgInfoHeader*)tempInfo)->hPool == msg->hPool) &&
                (((MsgInfoHeader*)tempInfo)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
					
                stat1 = RvSipInfoHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                                         msg->hPool, 
																		 msg->hPage, 
																		 &tempInfo);
			
                stat2 = RvSipInfoHeaderCopy(tempInfo,
					                             (RvSipInfoHeaderHandle)pHeader);
                *pNewHeader = (void*)tempInfo;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }

		case RVSIP_HEADERTYPE_ACCEPT:
        {
            RvSipAcceptHeaderHandle tempInfo = (RvSipAcceptHeaderHandle)pHeader;

            if((((MsgAcceptHeader*)tempInfo)->hPool == msg->hPool) &&
                (((MsgAcceptHeader*)tempInfo)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
					
                stat1 = RvSipAcceptHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                                         msg->hPool, 
																		 msg->hPage, 
																		 &tempInfo);
			
                stat2 = RvSipAcceptHeaderCopy(tempInfo,
					                             (RvSipAcceptHeaderHandle)pHeader);
                *pNewHeader = (void*)tempInfo;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }
		case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
        {
            RvSipAcceptEncodingHeaderHandle tempInfo = (RvSipAcceptEncodingHeaderHandle)pHeader;

            if((((MsgAcceptEncodingHeader*)tempInfo)->hPool == msg->hPool) &&
                (((MsgAcceptEncodingHeader*)tempInfo)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
					
                stat1 = RvSipAcceptEncodingHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                                         msg->hPool, 
																		 msg->hPage, 
																		 &tempInfo);
			
                stat2 = RvSipAcceptEncodingHeaderCopy(tempInfo,
					                             (RvSipAcceptEncodingHeaderHandle)pHeader);
                *pNewHeader = (void*)tempInfo;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }

		case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
        {
            RvSipAcceptLanguageHeaderHandle tempInfo = (RvSipAcceptLanguageHeaderHandle)pHeader;

            if((((MsgAcceptLanguageHeader*)tempInfo)->hPool == msg->hPool) &&
                (((MsgAcceptLanguageHeader*)tempInfo)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
					
                stat1 = RvSipAcceptLanguageHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                                         msg->hPool, 
																		 msg->hPage, 
																		 &tempInfo);
			
                stat2 = RvSipAcceptLanguageHeaderCopy(tempInfo,
					                             (RvSipAcceptLanguageHeaderHandle)pHeader);
                *pNewHeader = (void*)tempInfo;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }

		case RVSIP_HEADERTYPE_REPLY_TO:
        {
            RvSipReplyToHeaderHandle tempInfo = (RvSipReplyToHeaderHandle)pHeader;

            if((((MsgReplyToHeader*)tempInfo)->hPool == msg->hPool) &&
                (((MsgReplyToHeader*)tempInfo)->hPage == msg->hPage))
            {
                /* The header was already allocated on the correct page, (the msg's
                page, so we don't need to copy it. we can just attach it */
                *pNewHeader = pHeader;
            }
            else
            {
                /* we need to construct and copy the header, so it will be on the
                same page as the msg */
					
                stat1 = RvSipReplyToHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                    msg->hPool, 
													msg->hPage, 
													&tempInfo);
			
                stat2 = RvSipReplyToHeaderCopy(tempInfo,
					                             (RvSipReplyToHeaderHandle)pHeader);
                *pNewHeader = (void*)tempInfo;
            }

            ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
            break;
        }

		/* XXX */

#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */

#ifdef RV_SIP_IMS_HEADER_SUPPORT 
	case RVSIP_HEADERTYPE_P_URI:
    {
        RvSipPUriHeaderHandle tempInfo = (RvSipPUriHeaderHandle)pHeader;

        if((((MsgPUriHeader*)tempInfo)->hPool == msg->hPool) &&
            (((MsgPUriHeader*)tempInfo)->hPage == msg->hPage))
        {
            /* The header was already allocated on the correct page, (the msg's
			   page, so we don't need to copy it. we can just attach it			*/
            *pNewHeader = pHeader;
        }
        else
        {
            /* we need to construct and copy the header, so it will be on the
            same page as the msg */
            stat1 = RvSipPUriHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, 
												msg->hPage, 
												&tempInfo);
		
            stat2 = RvSipPUriHeaderCopy(tempInfo,
					                         (RvSipPUriHeaderHandle)pHeader);
            *pNewHeader = (void*)tempInfo;
        }

        ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
        break;
    }
	case RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID:
    {
        RvSipPVisitedNetworkIDHeaderHandle tempInfo = (RvSipPVisitedNetworkIDHeaderHandle)pHeader;

        if((((MsgPVisitedNetworkIDHeader*)tempInfo)->hPool == msg->hPool) &&
            (((MsgPVisitedNetworkIDHeader*)tempInfo)->hPage == msg->hPage))
        {
            /* The header was already allocated on the correct page, (the msg's
            page, so we don't need to copy it. we can just attach it */
            *pNewHeader = pHeader;
        }
        else
        {
            /* we need to construct and copy the header, so it will be on the
            same page as the msg */
				
            stat1 = RvSipPVisitedNetworkIDHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, 
												msg->hPage, 
												&tempInfo);
		
            stat2 = RvSipPVisitedNetworkIDHeaderCopy(tempInfo,
					                         (RvSipPVisitedNetworkIDHeaderHandle)pHeader);
            *pNewHeader = (void*)tempInfo;
        }

        ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
        break;
    }
	case RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO:
    {
        RvSipPAccessNetworkInfoHeaderHandle tempInfo = (RvSipPAccessNetworkInfoHeaderHandle)pHeader;

        if((((MsgPAccessNetworkInfoHeader*)tempInfo)->hPool == msg->hPool) &&
            (((MsgPAccessNetworkInfoHeader*)tempInfo)->hPage == msg->hPage))
        {
            /* The header was already allocated on the correct page, (the msg's
            page, so we don't need to copy it. we can just attach it */
            *pNewHeader = pHeader;
        }
        else
        {
            /* we need to construct and copy the header, so it will be on the
            same page as the msg */
				
            stat1 = RvSipPAccessNetworkInfoHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, 
												msg->hPage, 
												&tempInfo);
		
            stat2 = RvSipPAccessNetworkInfoHeaderCopy(tempInfo,
					                         (RvSipPAccessNetworkInfoHeaderHandle)pHeader);
            *pNewHeader = (void*)tempInfo;
        }

        ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
        break;
    }
	case RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES:
    {
        RvSipPChargingFunctionAddressesHeaderHandle tempInfo = (RvSipPChargingFunctionAddressesHeaderHandle)pHeader;

        if((((MsgPChargingFunctionAddressesHeader*)tempInfo)->hPool == msg->hPool) &&
            (((MsgPChargingFunctionAddressesHeader*)tempInfo)->hPage == msg->hPage))
        {
            /* The header was already allocated on the correct page, (the msg's
            page, so we don't need to copy it. we can just attach it */
            *pNewHeader = pHeader;
        }
        else
        {
            /* we need to construct and copy the header, so it will be on the
            same page as the msg */
				
            stat1 = RvSipPChargingFunctionAddressesHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, 
												msg->hPage, 
												&tempInfo);
		
            stat2 = RvSipPChargingFunctionAddressesHeaderCopy(tempInfo,
					                         (RvSipPChargingFunctionAddressesHeaderHandle)pHeader);
            *pNewHeader = (void*)tempInfo;
        }

        ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
        break;
    }
	case RVSIP_HEADERTYPE_P_CHARGING_VECTOR:
    {
        RvSipPChargingVectorHeaderHandle tempInfo = (RvSipPChargingVectorHeaderHandle)pHeader;

        if((((MsgPChargingVectorHeader*)tempInfo)->hPool == msg->hPool) &&
            (((MsgPChargingVectorHeader*)tempInfo)->hPage == msg->hPage))
        {
            /* The header was already allocated on the correct page, (the msg's
            page, so we don't need to copy it. we can just attach it */
            *pNewHeader = pHeader;
        }
        else
        {
            /* we need to construct and copy the header, so it will be on the
            same page as the msg */	
            stat1 = RvSipPChargingVectorHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, 
												msg->hPage, 
												&tempInfo);
		
            stat2 = RvSipPChargingVectorHeaderCopy(tempInfo,
					                         (RvSipPChargingVectorHeaderHandle)pHeader);
            *pNewHeader = (void*)tempInfo;
        }

        ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
        break;
    }
	case RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION:
    {
        RvSipPMediaAuthorizationHeaderHandle tempInfo = (RvSipPMediaAuthorizationHeaderHandle)pHeader;

        if((((MsgPMediaAuthorizationHeader*)tempInfo)->hPool == msg->hPool) &&
            (((MsgPMediaAuthorizationHeader*)tempInfo)->hPage == msg->hPage))
        {
            /* The header was already allocated on the correct page, (the msg's
            page, so we don't need to copy it. we can just attach it */
            *pNewHeader = pHeader;
        }
        else
        {
            /* we need to construct and copy the header, so it will be on the
            same page as the msg */				
            stat1 = RvSipPMediaAuthorizationHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, 
												msg->hPage, 
												&tempInfo);
		
            stat2 = RvSipPMediaAuthorizationHeaderCopy(tempInfo,
					                         (RvSipPMediaAuthorizationHeaderHandle)pHeader);
            *pNewHeader = (void*)tempInfo;
        }

        ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
        break;
    }
	case RVSIP_HEADERTYPE_SECURITY:
    {
        RvSipSecurityHeaderHandle tempInfo = (RvSipSecurityHeaderHandle)pHeader;

        if((((MsgSecurityHeader*)tempInfo)->hPool == msg->hPool) &&
            (((MsgSecurityHeader*)tempInfo)->hPage == msg->hPage))
        {
            /* The header was already allocated on the correct page, (the msg's
            page, so we don't need to copy it. we can just attach it */
            *pNewHeader = pHeader;
        }
        else
        {
            /* we need to construct and copy the header, so it will be on the
            same page as the msg */
				
            stat1 = RvSipSecurityHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, 
												msg->hPage, 
												&tempInfo);
		
            stat2 = RvSipSecurityHeaderCopy(tempInfo,
					                         (RvSipSecurityHeaderHandle)pHeader);
            *pNewHeader = (void*)tempInfo;
        }

        ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
        break;
    }
    case RVSIP_HEADERTYPE_P_PROFILE_KEY:
    {
        RvSipPProfileKeyHeaderHandle tempInfo = (RvSipPProfileKeyHeaderHandle)pHeader;

        if((((MsgPProfileKeyHeader*)tempInfo)->hPool == msg->hPool) &&
            (((MsgPProfileKeyHeader*)tempInfo)->hPage == msg->hPage))
        {
            /* The header was already allocated on the correct page, (the msg's
            page, so we don't need to copy it. we can just attach it */
            *pNewHeader = pHeader;
        }
        else
        {
            /* we need to construct and copy the header, so it will be on the
            same page as the msg */
				
            stat1 = RvSipPProfileKeyHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, 
												msg->hPage, 
												&tempInfo);
		
            stat2 = RvSipPProfileKeyHeaderCopy(tempInfo,
					                         (RvSipPProfileKeyHeaderHandle)pHeader);
            *pNewHeader = (void*)tempInfo;
        }

        ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
        break;
    }
    case RVSIP_HEADERTYPE_P_USER_DATABASE:
    {
        RvSipPUserDatabaseHeaderHandle tempInfo = (RvSipPUserDatabaseHeaderHandle)pHeader;

        if((((MsgPUserDatabaseHeader*)tempInfo)->hPool == msg->hPool) &&
            (((MsgPUserDatabaseHeader*)tempInfo)->hPage == msg->hPage))
        {
            /* The header was already allocated on the correct page, (the msg's
            page, so we don't need to copy it. we can just attach it */
            *pNewHeader = pHeader;
        }
        else
        {
            /* we need to construct and copy the header, so it will be on the
            same page as the msg */
				
            stat1 = RvSipPUserDatabaseHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, 
												msg->hPage, 
												&tempInfo);
		
            stat2 = RvSipPUserDatabaseHeaderCopy(tempInfo,
					                         (RvSipPUserDatabaseHeaderHandle)pHeader);
            *pNewHeader = (void*)tempInfo;
        }

        ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
        break;
    }
    case RVSIP_HEADERTYPE_P_ANSWER_STATE:
    {
        RvSipPAnswerStateHeaderHandle tempInfo = (RvSipPAnswerStateHeaderHandle)pHeader;

        if((((MsgPAnswerStateHeader*)tempInfo)->hPool == msg->hPool) &&
            (((MsgPAnswerStateHeader*)tempInfo)->hPage == msg->hPage))
        {
            /* The header was already allocated on the correct page, (the msg's
            page, so we don't need to copy it. we can just attach it */
            *pNewHeader = pHeader;
        }
        else
        {
            /* we need to construct and copy the header, so it will be on the
            same page as the msg */
				
            stat1 = RvSipPAnswerStateHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, 
												msg->hPage, 
												&tempInfo);
		
            stat2 = RvSipPAnswerStateHeaderCopy(tempInfo,
					                         (RvSipPAnswerStateHeaderHandle)pHeader);
            *pNewHeader = (void*)tempInfo;
        }

        ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
        break;
    }
    case RVSIP_HEADERTYPE_P_SERVED_USER:
    {
        RvSipPServedUserHeaderHandle tempInfo = (RvSipPServedUserHeaderHandle)pHeader;

        if((((MsgPServedUserHeader*)tempInfo)->hPool == msg->hPool) &&
            (((MsgPServedUserHeader*)tempInfo)->hPage == msg->hPage))
        {
            /* The header was already allocated on the correct page, (the msg's
            page, so we don't need to copy it. we can just attach it */
            *pNewHeader = pHeader;
        }
        else
        {
            /* we need to construct and copy the header, so it will be on the
            same page as the msg */
				
            stat1 = RvSipPServedUserHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, 
												msg->hPage, 
												&tempInfo);
		
            stat2 = RvSipPServedUserHeaderCopy(tempInfo,
					                         (RvSipPServedUserHeaderHandle)pHeader);
            *pNewHeader = (void*)tempInfo;
        }

        ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
        break;
    }
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
	case RVSIP_HEADERTYPE_P_DCS_TRACE_PARTY_ID:
    {
        RvSipPDCSTracePartyIDHeaderHandle tempInfo = (RvSipPDCSTracePartyIDHeaderHandle)pHeader;

        if((((MsgPDCSTracePartyIDHeader*)tempInfo)->hPool == msg->hPool) &&
            (((MsgPDCSTracePartyIDHeader*)tempInfo)->hPage == msg->hPage))
        {
            /* The header was already allocated on the correct page, (the msg's
            page, so we don't need to copy it. we can just attach it */
            *pNewHeader = pHeader;
        }
        else
        {
            /* we need to construct and copy the header, so it will be on the
            same page as the msg */
				
            stat1 = RvSipPDCSTracePartyIDHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, 
												msg->hPage, 
												&tempInfo);
		
            stat2 = RvSipPDCSTracePartyIDHeaderCopy(tempInfo,
					                         (RvSipPDCSTracePartyIDHeaderHandle)pHeader);
            *pNewHeader = (void*)tempInfo;
        }

        ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
        break;
    }
	case RVSIP_HEADERTYPE_P_DCS_OSPS:
    {
        RvSipPDCSOSPSHeaderHandle tempInfo = (RvSipPDCSOSPSHeaderHandle)pHeader;

        if((((MsgPDCSOSPSHeader*)tempInfo)->hPool == msg->hPool) &&
           (((MsgPDCSOSPSHeader*)tempInfo)->hPage == msg->hPage))
        {
            /* The header was already allocated on the correct page, (the msg's
            page, so we don't need to copy it. we can just attach it */
            *pNewHeader = pHeader;
        }
        else
        {
            /* we need to construct and copy the header, so it will be on the
            same page as the msg */
				
            stat1 = RvSipPDCSOSPSHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, 
												msg->hPage, 
												&tempInfo);
		
            stat2 = RvSipPDCSOSPSHeaderCopy(tempInfo,
					                         (RvSipPDCSOSPSHeaderHandle)pHeader);
            *pNewHeader = (void*)tempInfo;
        }

        ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
        break;
    }
	case RVSIP_HEADERTYPE_P_DCS_BILLING_INFO:
    {
        RvSipPDCSBillingInfoHeaderHandle tempInfo = (RvSipPDCSBillingInfoHeaderHandle)pHeader;

        if((((MsgPDCSBillingInfoHeader*)tempInfo)->hPool == msg->hPool) &&
           (((MsgPDCSBillingInfoHeader*)tempInfo)->hPage == msg->hPage))
        {
            /* The header was already allocated on the correct page, (the msg's
            page, so we don't need to copy it. we can just attach it */
            *pNewHeader = pHeader;
        }
        else
        {
            /* we need to construct and copy the header, so it will be on the
            same page as the msg */
				
            stat1 = RvSipPDCSBillingInfoHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, 
												msg->hPage, 
												&tempInfo);
		
            stat2 = RvSipPDCSBillingInfoHeaderCopy(tempInfo,
					                         (RvSipPDCSBillingInfoHeaderHandle)pHeader);
            *pNewHeader = (void*)tempInfo;
        }

        ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
        break;
    }
	case RVSIP_HEADERTYPE_P_DCS_LAES:
    {
        RvSipPDCSLAESHeaderHandle tempInfo = (RvSipPDCSLAESHeaderHandle)pHeader;

        if((((MsgPDCSLAESHeader*)tempInfo)->hPool == msg->hPool) &&
           (((MsgPDCSLAESHeader*)tempInfo)->hPage == msg->hPage))
        {
            /* The header was already allocated on the correct page, (the msg's
            page, so we don't need to copy it. we can just attach it */
            *pNewHeader = pHeader;
        }
        else
        {
            /* we need to construct and copy the header, so it will be on the
            same page as the msg */
				
            stat1 = RvSipPDCSLAESHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, 
												msg->hPage, 
												&tempInfo);
		
            stat2 = RvSipPDCSLAESHeaderCopy(tempInfo,
					                         (RvSipPDCSLAESHeaderHandle)pHeader);
            *pNewHeader = (void*)tempInfo;
        }

        ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
        break;
    }
	case RVSIP_HEADERTYPE_P_DCS_REDIRECT:
    {
        RvSipPDCSRedirectHeaderHandle tempInfo = (RvSipPDCSRedirectHeaderHandle)pHeader;

        if((((MsgPDCSRedirectHeader*)tempInfo)->hPool == msg->hPool) &&
           (((MsgPDCSRedirectHeader*)tempInfo)->hPage == msg->hPage))
        {
            /* The header was already allocated on the correct page, (the msg's
            page, so we don't need to copy it. we can just attach it */
            *pNewHeader = pHeader;
        }
        else
        {
            /* we need to construct and copy the header, so it will be on the
            same page as the msg */
				
            stat1 = RvSipPDCSRedirectHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                                msg->hPool, 
												msg->hPage, 
												&tempInfo);
		
            stat2 = RvSipPDCSRedirectHeaderCopy(tempInfo,
					                         (RvSipPDCSRedirectHeaderHandle)pHeader);
            *pNewHeader = (void*)tempInfo;
        }

        ((MsgHeaderListElem*)allocatedItem)->hHeader = *pNewHeader;
        break;
    }
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
        case RVSIP_HEADERTYPE_UNDEFINED:
        default:
        {
            RvLogExcep(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "RvSipMsgPushHeader - unknown headerType. cannot insert it to the list!"));
            *pNewHeader = NULL;
            return RV_ERROR_BADPARAM;
        }

    }

    if(stat2 != RV_OK || stat1 != RV_OK)
    {
        ((MsgHeaderListElem*)allocatedItem)->hHeader = NULL;
        RLIST_Remove(NULL, list, allocatedItem);
        *pNewHeader = NULL;
        return RV_ERROR_UNKNOWN;
    }
    *hListElem = (RvSipHeaderListElemHandle)allocatedItem;
    return RV_OK;
}

/***************************************************************************
 * RvSipMsgConstructHeaderInMsgByType
 * ------------------------------------------------------------------------
 * General: The function constructs a header in message, according to it's type.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMsg -    Handle to the message.
 *          eType -   Type of the header to construct.
 *          pushHeaderAtHead - Boolean value indicating whether the header should
 *                            be pushed to the head of the list (RV_TRUE),
 *                            or to the tail (RV_FALSE).
 * output:  phHeader   - Handle to the header structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgConstructHeaderInMsgByType(
                                          IN    RvSipMsgHandle  hMsg,
                                          IN    RvSipHeaderType eType,
                                          IN    RvBool         bPushHeaderAtHead,
                                          OUT   void**          phHeader)
{
    RvStatus stat;

    if((hMsg == NULL) || (phHeader == NULL) || (eType == RVSIP_HEADERTYPE_UNDEFINED))
        return RV_ERROR_BADPARAM;

    *phHeader = NULL;

    switch(eType)
    {
#ifndef RV_SIP_LIGHT
        case RVSIP_HEADERTYPE_CONTACT:
            stat = RvSipContactHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipContactHeaderHandle*)phHeader);
            break;
        case RVSIP_HEADERTYPE_EXPIRES:
            stat = RvSipExpiresHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipExpiresHeaderHandle*)phHeader);
            break;
            
        case RVSIP_HEADERTYPE_DATE:
            stat = RvSipDateHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                (RvSipDateHeaderHandle*)phHeader);
            break;
#endif /*#ifndef RV_SIP_LIGHT*/
        case RVSIP_HEADERTYPE_ROUTE_HOP:
            stat = RvSipRouteHopHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipRouteHopHeaderHandle*)phHeader);
            break;
#ifdef RV_SIP_AUTH_ON
        case RVSIP_HEADERTYPE_AUTHENTICATION:
            stat = RvSipAuthenticationHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                            (RvSipAuthenticationHeaderHandle*)phHeader);
            break;
        case RVSIP_HEADERTYPE_AUTHORIZATION:
            stat = RvSipAuthorizationHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                (RvSipAuthorizationHeaderHandle*)phHeader);;
            break;
#endif /* #ifdef RV_SIP_AUTH_ON */
#ifndef RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_EVENT:
            stat = RvSipEventHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipEventHeaderHandle*)phHeader);
            break;
        case RVSIP_HEADERTYPE_ALLOW:
            stat = RvSipAllowHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipAllowHeaderHandle*)phHeader);
            break;
        case RVSIP_HEADERTYPE_RSEQ:
            stat = RvSipRSeqHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipRSeqHeaderHandle*)phHeader);
            break;
        case RVSIP_HEADERTYPE_RACK:
            stat = RvSipRAckHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipRAckHeaderHandle*)phHeader);
            break;
#ifdef RV_SIP_SUBS_ON
        case RVSIP_HEADERTYPE_REFER_TO:
            stat = RvSipReferToHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipReferToHeaderHandle*)phHeader);
            break;
        case RVSIP_HEADERTYPE_REFERRED_BY:
            stat = RvSipReferredByHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipReferredByHeaderHandle*)phHeader);
            break;
        case RVSIP_HEADERTYPE_ALLOW_EVENTS:
            stat = RvSipAllowEventsHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipAllowEventsHeaderHandle*)phHeader);
            break;
        case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
            stat = RvSipSubscriptionStateHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipSubscriptionStateHeaderHandle*)phHeader);
            break;
#endif /* #ifdef RV_SIP_SUBS_ON */
        case RVSIP_HEADERTYPE_REPLACES:
            stat = RvSipReplacesHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipReplacesHeaderHandle*)phHeader);
            break;
        case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
            stat = RvSipContentDispositionHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipContentDispositionHeaderHandle*)phHeader);
            break;
        case RVSIP_HEADERTYPE_RETRY_AFTER:
            stat = RvSipRetryAfterHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipRetryAfterHeaderHandle*)phHeader);
            break;
        case RVSIP_HEADERTYPE_SESSION_EXPIRES:
            stat = RvSipSessionExpiresHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipSessionExpiresHeaderHandle*)phHeader);
            break;
        case RVSIP_HEADERTYPE_MINSE:
            stat = RvSipMinSEHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipMinSEHeaderHandle*)phHeader);
            break;
#endif /* RV_SIP_PRIMITIVES */
        case RVSIP_HEADERTYPE_VIA:
            stat = RvSipViaHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipViaHeaderHandle*)phHeader);
            break;
        case RVSIP_HEADERTYPE_OTHER:
            stat = RvSipOtherHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
                                                    (RvSipOtherHeaderHandle*)phHeader);
            break;
#ifndef RV_SIP_LIGHT
        case RVSIP_HEADERTYPE_CSEQ:
            stat = RvSipCSeqHeaderConstructInMsg(hMsg,
                                                (RvSipCSeqHeaderHandle*)phHeader);
            break;
        case RVSIP_HEADERTYPE_TO:
            stat = RvSipToHeaderConstructInMsg(hMsg, (RvSipPartyHeaderHandle*)phHeader);
            break;
        case RVSIP_HEADERTYPE_FROM:
            stat = RvSipFromHeaderConstructInMsg(hMsg, (RvSipPartyHeaderHandle*)phHeader);
            break;
        case RVSIP_HEADERTYPE_CONTENT_TYPE:
        {
#ifndef RV_SIP_PRIMITIVES
        RvSipBodyHandle hBodyHandle = RvSipMsgGetBodyObject(hMsg);
        if (NULL == hBodyHandle)
        {
            stat = RvSipBodyConstructInMsg(hMsg,&hBodyHandle);
            if (RV_OK != stat)
            {
                RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc ,(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc ,
                    "RvSipMsgConstructHeaderInMsgByType - error in RvSipBodyConstructInMsg (for constructing content-type header)"));
                return stat;
            }
        }

        stat = RvSipContentTypeHeaderConstructInBody(hBodyHandle,(RvSipContentTypeHeaderHandle*)phHeader);
        break;
#else
        RvLogExcep(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
                "RvSipMsgConstructHeaderInMsgByType - no content-type header in extra-lean!"));
        return RV_ERROR_BADPARAM;

#endif /*#ifndef RV_SIP_PRIMITIVES*/
        }

        case RVSIP_HEADERTYPE_CONTENT_ID:
        {
#ifndef RV_SIP_PRIMITIVES
        RvSipBodyHandle hBodyHandle = RvSipMsgGetBodyObject(hMsg);
        if (NULL == hBodyHandle)
        {
            stat = RvSipBodyConstructInMsg(hMsg,&hBodyHandle);
            if (RV_OK != stat)
            {
                RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc ,(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc ,
                    "RvSipMsgConstructHeaderInMsgByType - error in RvSipBodyConstructInMsg (for constructing content-ID header)"));
                return stat;
            }
        }

        stat = RvSipContentIDHeaderConstructInBody(hBodyHandle,(RvSipContentIDHeaderHandle*)phHeader);
        break;
#else
        RvLogExcep(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
                "RvSipMsgConstructHeaderInMsgByType - no content-ID header in extra-lean!"));
        return RV_ERROR_BADPARAM;

#endif /*#ifndef RV_SIP_PRIMITIVES*/
        }
#endif /*#ifndef RV_SIP_LIGHT*/

#ifdef RV_SIP_AUTH_ON
		case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
        {
            stat = RvSipAuthenticationInfoHeaderConstructInMsg(hMsg, bPushHeaderAtHead,
										(RvSipAuthenticationInfoHeaderHandle*)phHeader);;
            break;
        }
#endif /*#ifdef RV_SIP_AUTH_ON*/
#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_REASON:
		{
			stat = RvSipReasonHeaderConstructInMsg(hMsg, bPushHeaderAtHead, (RvSipReasonHeaderHandle*)phHeader);
			break;
		}
		case RVSIP_HEADERTYPE_WARNING:
		{
			stat = RvSipWarningHeaderConstructInMsg(hMsg, bPushHeaderAtHead, (RvSipWarningHeaderHandle*)phHeader);
			break;
		}
		case RVSIP_HEADERTYPE_TIMESTAMP:
			{
				stat = RvSipTimestampHeaderConstructInMsg(hMsg, bPushHeaderAtHead, (RvSipTimestampHeaderHandle*)phHeader);
				break;
			}
		case RVSIP_HEADERTYPE_INFO:
			{
				stat = RvSipInfoHeaderConstructInMsg(hMsg, bPushHeaderAtHead, (RvSipInfoHeaderHandle*)phHeader);
				break;
			}
		case RVSIP_HEADERTYPE_ACCEPT:
			{
				stat = RvSipAcceptHeaderConstructInMsg(hMsg, bPushHeaderAtHead, (RvSipAcceptHeaderHandle*)phHeader);
				break;
			}
		case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
			{
				stat = RvSipAcceptEncodingHeaderConstructInMsg(hMsg, bPushHeaderAtHead, (RvSipAcceptEncodingHeaderHandle*)phHeader);
				break;
			}
		case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
			{
				stat = RvSipAcceptLanguageHeaderConstructInMsg(hMsg, bPushHeaderAtHead, (RvSipAcceptLanguageHeaderHandle*)phHeader);
				break;
			}
		case RVSIP_HEADERTYPE_REPLY_TO:
			{
				stat = RvSipReplyToHeaderConstructInMsg(hMsg, bPushHeaderAtHead, (RvSipReplyToHeaderHandle*)phHeader);
				break;
			}
		/* XXX */

#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT 
		case RVSIP_HEADERTYPE_P_URI:
		{
			stat = RvSipPUriHeaderConstructInMsg(hMsg, bPushHeaderAtHead, 
															(RvSipPUriHeaderHandle*)phHeader);
			break;
		}
		case RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID:
		{
			stat = RvSipPVisitedNetworkIDHeaderConstructInMsg(hMsg, bPushHeaderAtHead, 
															(RvSipPVisitedNetworkIDHeaderHandle*)phHeader);
			break;
		}
		case RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO:
		{
			stat = RvSipPAccessNetworkInfoHeaderConstructInMsg(hMsg, bPushHeaderAtHead, 
															(RvSipPAccessNetworkInfoHeaderHandle*)phHeader);
			break;
		}
		case RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES:
		{
			stat = RvSipPChargingFunctionAddressesHeaderConstructInMsg(hMsg, bPushHeaderAtHead, 
															(RvSipPChargingFunctionAddressesHeaderHandle*)phHeader);
			break;
		}
		case RVSIP_HEADERTYPE_P_CHARGING_VECTOR:
		{
			stat = RvSipPChargingVectorHeaderConstructInMsg(hMsg, bPushHeaderAtHead, 
															(RvSipPChargingVectorHeaderHandle*)phHeader);
			break;
		}
		case RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION:
		{
			stat = RvSipPMediaAuthorizationHeaderConstructInMsg(hMsg, bPushHeaderAtHead, 
															(RvSipPMediaAuthorizationHeaderHandle*)phHeader);
			break;
		}
		case RVSIP_HEADERTYPE_SECURITY:
		{
			stat = RvSipSecurityHeaderConstructInMsg(hMsg, bPushHeaderAtHead, 
															(RvSipSecurityHeaderHandle*)phHeader);
			break;
		}
        case RVSIP_HEADERTYPE_P_PROFILE_KEY:
		{
			stat = RvSipPProfileKeyHeaderConstructInMsg(hMsg, bPushHeaderAtHead, 
															(RvSipPProfileKeyHeaderHandle*)phHeader);
			break;
		}
        case RVSIP_HEADERTYPE_P_USER_DATABASE:
		{
			stat = RvSipPUserDatabaseHeaderConstructInMsg(hMsg, bPushHeaderAtHead, 
															(RvSipPUserDatabaseHeaderHandle*)phHeader);
			break;
		}
        case RVSIP_HEADERTYPE_P_ANSWER_STATE:
		{
			stat = RvSipPAnswerStateHeaderConstructInMsg(hMsg, bPushHeaderAtHead, 
															(RvSipPAnswerStateHeaderHandle*)phHeader);
			break;
		}
        case RVSIP_HEADERTYPE_P_SERVED_USER:
		{
			stat = RvSipPServedUserHeaderConstructInMsg(hMsg, bPushHeaderAtHead, 
															(RvSipPServedUserHeaderHandle*)phHeader);
			break;
		}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_P_DCS_TRACE_PARTY_ID:
		{
			stat = RvSipPDCSTracePartyIDHeaderConstructInMsg(hMsg, bPushHeaderAtHead, 
															(RvSipPDCSTracePartyIDHeaderHandle*)phHeader);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_OSPS:
		{
			stat = RvSipPDCSOSPSHeaderConstructInMsg(hMsg, bPushHeaderAtHead, 
															(RvSipPDCSOSPSHeaderHandle*)phHeader);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_BILLING_INFO:
		{
			stat = RvSipPDCSBillingInfoHeaderConstructInMsg(hMsg, bPushHeaderAtHead, 
															(RvSipPDCSBillingInfoHeaderHandle*)phHeader);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_LAES:
		{
			stat = RvSipPDCSLAESHeaderConstructInMsg(hMsg, bPushHeaderAtHead, 
															(RvSipPDCSLAESHeaderHandle*)phHeader);
			break;
		}
		case RVSIP_HEADERTYPE_P_DCS_REDIRECT:
		{
			stat = RvSipPDCSRedirectHeaderConstructInMsg(hMsg, bPushHeaderAtHead, 
															(RvSipPDCSRedirectHeaderHandle*)phHeader);
			break;
		}
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 

        case RVSIP_HEADERTYPE_UNDEFINED:
        default:
        {
            RvLogExcep(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
                "RvSipMsgConstructHeaderInMsgByType - unknown headerType. cannot insert it to the list!"));
            return RV_ERROR_BADPARAM;
        }
    }
    return stat;
}

/***************************************************************************
 * RvSipMsgEncodeHeaderByType
 * ------------------------------------------------------------------------
 * General: Obsolete function - calls to RvSipHeaderEncode.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgEncodeHeaderByType(IN    RvSipHeaderType eType,
                                                    IN    void*          pHeader,
                                                    IN    RvSipMsgMgrHandle hMsgMgr,
                                                    IN    HRPOOL         hPool,
                                                    OUT   HPAGE*         hPage,
                                                    OUT   RvUint32*     length)
{
    RV_UNUSED_ARG(hMsgMgr);
    return RvSipHeaderEncode(pHeader, eType, hPool, hPage, length);
}

/***************************************************************************
 * RvSipMsgParseHeaderByType
 * ------------------------------------------------------------------------
 * General: Obsolete function - calls to RvSipHeaderParse.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgParseHeaderByType(
                                          IN    RvSipMsgMgrHandle hMsgMgr,
                                          IN    RvSipHeaderType eType,
                                          IN    void*           hHeader,
                                          IN    RvChar*        buffer)
{
    RV_UNUSED_ARG(hMsgMgr);
    return RvSipHeaderParse(hHeader, eType, buffer);
}
/****************************************************/
/*        "To" HEADERS FUNCTIONS                       */
/****************************************************/
#ifndef RV_SIP_LIGHT
/***************************************************************************
 * RvSipMsgGetToHeader
 * ------------------------------------------------------------------------
 * General: Gets the Handle to the To header object from the message object.
 * Return Value: Returns RvSipPartyHeaderHandle, or NULL if the message does
 *               not contain a To header.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg - Handle to a message object.
 ***************************************************************************/
RVAPI RvSipPartyHeaderHandle RVCALLCONV RvSipMsgGetToHeader(IN RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;

    if(hSipMsg == NULL)
        return NULL;

    msg = (MsgMessage*)hSipMsg;

    return msg->hToHeader;
}

/***************************************************************************
 * RvSipMsgSetToHeader
 * ------------------------------------------------------------------------
 * General: Sets a To header in the message object. If there is no To header
 *          in the message, the function constructs a To header.
 * Return Value: RV_OK,RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipMsg   - Handle to the message object.
 *    hToHeader - Handle to the To header to be set. If a NULL is supplied,
 *              the existing To header is removed from the message.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetToHeader(IN    RvSipMsgHandle         hSipMsg,
                                               IN    RvSipPartyHeaderHandle hToHeader)
{
    MsgMessage*   msg;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    msg = (MsgMessage*)hSipMsg;

    if(hToHeader == NULL)
    {
        msg->hToHeader = NULL;
        return RV_OK;
    }
    else
    {
        /* if there is no To header object in the msg, we will allocate one */
        if(msg->hToHeader == NULL)
        {
            RvSipPartyHeaderHandle hHeader;

            if(RvSipToHeaderConstructInMsg(hSipMsg, &hHeader) != RV_OK)
                return RV_ERROR_OUTOFRESOURCES;
        }
        return RvSipPartyHeaderCopy(msg->hToHeader, hToHeader);
    }
}
#endif /*#ifndef RV_SIP_LIGHT*/
/****************************************************/
/*        "FROM" HEADERS FUNCTIONS                  */
/****************************************************/
#ifndef RV_SIP_LIGHT
/***************************************************************************
 * RvSipMsgGetFromHeader
 * ------------------------------------------------------------------------
 * General: Gets a handle to the From header object from the message object.
 * Return Value: MsgPartyHeader handle, or NULL if the message does not
 *               contain a From header.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipMsg - Handle to a message object.
 ***************************************************************************/
RVAPI RvSipPartyHeaderHandle RVCALLCONV RvSipMsgGetFromHeader(IN RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;

    if(hSipMsg == NULL)
        return NULL;

    msg = (MsgMessage*)hSipMsg;

    return msg->hFromHeader;
}

/***************************************************************************
 * RvSipMsgSetFromHeader
 * ------------------------------------------------------------------------
 * General: Sets a From header in the message object. If no From header exists
 *          in the message, the function constructs a From header.
 * Return Value: RV_OK,RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipMsg     - Handle to a message object.
 *    hFromHeader -  Handle to the From header to be set. If a NULL value is
 *                 supplied, the existing From header is removed from the
 *                 message.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetFromHeader(IN    RvSipMsgHandle         hSipMsg,
                                                 IN    RvSipPartyHeaderHandle hFromHeader)
{
    MsgMessage*   msg;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    msg = (MsgMessage*)hSipMsg;

    if(hFromHeader == NULL)
    {
        msg->hFromHeader = NULL;
        return RV_OK;
    }
    else
    {
        /* if there is no From header object in the msg, we will allocate one */
        if(msg->hFromHeader == NULL)
        {
            RvSipPartyHeaderHandle hHeader;

            if(RvSipFromHeaderConstructInMsg(hSipMsg, &hHeader) != RV_OK)
                return RV_ERROR_OUTOFRESOURCES;
        }
        return RvSipPartyHeaderCopy(msg->hFromHeader, hFromHeader);
    }
}
#endif /*#ifndef RV_SIP_LIGHT*/
/****************************************************/
/*        CONTENT LENGTH HEADERS FUNCTIONS             */
/****************************************************/
/***************************************************************************
 * RvSipMsgGetContentLengthHeader
 * ------------------------------------------------------------------------
 * General:Gets the Content Length of the message object. The content
 *         length indicates the size of the body string.
 *         NOTE:
 *                It is relevant to request the Content-Length field
 *                ONLY when the body is of type OTHER than multipart.
 *                If you request the Content-Length for multipart body the
 *                function might return UNDEFINED even if the body string is
 *                not empty. UNDEFINED is returned when the body object
 *                contains a list of body parts. The list of body parts
 *                should be encoded in order to get the length of the body
 *                string. To receive the Content-Length of multipart body
 *                use RvSipMsgGetStringLength() with body enumeration.
 * Return Value: Returns content length or UNDEFINED if no body is set in
 *               the message object.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *        hSipMsg - Handle to the message object.
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipMsgGetContentLengthHeader(
                                               IN RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;

    if(hSipMsg == NULL)
        return UNDEFINED;

    msg = (MsgMessage*)hSipMsg;

    return msg->contentLength;

}

/***************************************************************************
 * RvSipMsgSetContentLengthHeader
 * ------------------------------------------------------------------------
 * General: Sets a Content Length value in the message object. The content
 *          length indicates the size of the message body.
 *          NOTE:
 *                 It is relevant to set the Content-Length field
 *                 ONLY when the body is of type OTHER than multipart.
 *                 Even if you set the Content-Length for multipart body the
 *                 Content-Length will be ran over when the message will be
 *                 encoded, according to the actual length of the message
 *                 body, when it is encoded. If you try to set the
 *                 Content-Length of a message in which the body has a list of
 *                 this function will return RV_ERROR_UNKNOWN.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *        hSipMsg - Handle to the message object.
 *        contentLen - The Content Length value to be set.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetContentLengthHeader(
                                              IN    RvSipMsgHandle  hSipMsg,
                                              IN    RvInt32        contentLen)
{
    MsgMessage* msg;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    msg = (MsgMessage*)hSipMsg;


#ifndef RV_SIP_PRIMITIVES
    if ((NULL != msg->pBodyObject) &&
        (NULL != msg->pBodyObject->hBodyPartsList))
    {
        return RV_ERROR_UNKNOWN;
    }
#endif /* #ifndef RV_SIP_PRIMITIVES*/

    msg->contentLength = contentLen;

    return RV_OK;
}

/****************************************************/
/*        "CSEQ" HEADERS FUNCTIONS                  */
/****************************************************/
#ifndef RV_SIP_LIGHT
/***************************************************************************
 * RvSipMsgGetCSeqHeader
 * ------------------------------------------------------------------------
 * General: Gets the Handle to the Cseq header object from the message object.
 * Return Value: Returns the CSeq header handle, or NULL if no CSeq Header
 *               handle exists.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipMsg - Handle to a message object.
 ***************************************************************************/
RVAPI RvSipCSeqHeaderHandle RVCALLCONV RvSipMsgGetCSeqHeader(
                                               IN RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;

    if(hSipMsg == NULL)
        return NULL;

    msg = (MsgMessage*)hSipMsg;

    return msg->hCSeqHeader;
}

/***************************************************************************
 * RvSipMsgSetCSeqHeader
 * ------------------------------------------------------------------------
 * General: Sets a CSeq header in the message object.
 *          If there is no CSeq header in the message, the function
 *          constructs one.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipMsg - Handle of a message object.
 *    hCSeqHeader - Handle of the CSEQ header to beset. if NULL value is
 *                supplied, the existing CSeq header will be removed from the
 *                message.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetCSeqHeader(
                                         IN    RvSipMsgHandle        hSipMsg,
                                         IN    RvSipCSeqHeaderHandle hCSeqHeader)
{
    MsgMessage*   msg;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    msg = (MsgMessage*)hSipMsg;

    if(hCSeqHeader == NULL)
    {
        msg->hCSeqHeader = NULL;
        return RV_OK;
    }
    else
    {
        if(msg->hCSeqHeader == NULL)
        {
            /* need to construct the cseq header */
            RvSipCSeqHeaderHandle hHeader;

            if(RvSipCSeqHeaderConstructInMsg(hSipMsg, &hHeader) != RV_OK)
                return RV_ERROR_OUTOFRESOURCES;
        }
        return RvSipCSeqHeaderCopy(msg->hCSeqHeader, hCSeqHeader);
    }
}
#endif /*#ifndef RV_SIP_LIGHT*/
/****************************************************/
/*        "CALL-ID" HEADERS FUNCTIONS                   */
/****************************************************/

/***************************************************************************
 * RvSipMsgGetCallIdHeader
 * ------------------------------------------------------------------------
 * General: Copies the message object Call-ID into a given buffer.
 *          If the bufferLen size is adequate, the function copies the Call-ID
 *          into the strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and the actualLen parameter contains the
 *          required buffer length.
 * Return Value: RV_OK or RV_ERROR_INSUFFICIENT_BUFFER.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg   - Handle to the message object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgGetCallIdHeader(
                                           IN RvSipMsgHandle     hSipMsg,
                                           IN RvChar*           strBuffer,
                                           IN RvUint            bufferLen,
                                           OUT RvUint*          actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return RvSipOtherHeaderGetValue(((MsgMessage*)hSipMsg)->hCallIdHeader,
                                                  strBuffer,
                                                  bufferLen,
                                                  actualLen);
}

/***************************************************************************
 * RvSipMsgSetCallIdHeader
 * ------------------------------------------------------------------------
 * General: Sets a Call-ID in the message object.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipMsg   - Handle to the message object.
 *    strCallId - The new Call-ID string to be set in the message object.
 *              If a NULL value is supplied, the existing Call-ID is removed
 *              from the message.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetCallIdHeader(IN    RvSipMsgHandle hSipMsg,
                                                   IN    RvChar*       strCallId)
{
    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strCallId == NULL)
    {
        MsgMessage* msg;

        msg = (MsgMessage*)hSipMsg;
        msg->hCallIdHeader = NULL;
        return RV_OK;
    }
    return SipMsgSetCallIdHeader(hSipMsg, strCallId, NULL,
                                 NULL_PAGE, UNDEFINED);
}


/****************************************************/
/*        "CONTENT-TYPE" HEADERS FUNCTIONS          */
/****************************************************/

/***************************************************************************
 * RvSipMsgGetContentTypeHeader
 * ------------------------------------------------------------------------
 * General:  Copies the Content-Type of the message object into a given buffer.
 *           If the bufferLen size is adequate, the function copies the
 *           Content-Type, into the strBuffer. Otherwise, the function returns
 *           RV_ERROR_INSUFFICIENT_BUFFER and the actualLen param contains the
 *           required buffer length.
 * Return Value: RV_OK or RV_ERROR_INSUFFICIENT_BUFFER.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg   - Handle to the message object.
 *        strBuffer - Buffer to fill with the requested parameter.
 *        bufferLen - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgGetContentTypeHeader(
                                                IN RvSipMsgHandle     hSipMsg,
                                                IN RvChar*           strBuffer,
                                                IN RvUint            bufferLen,
                                                OUT RvUint*          actualLen)
{
#ifndef RV_SIP_PRIMITIVES
    MsgMessage                   *pMsg;
    RvSipContentTypeHeaderHandle  hContentType;
    RvStatus                     stat;
#endif  /* #ifndef RV_SIP_PRIMITIVES */

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

#ifndef RV_SIP_PRIMITIVES

    pMsg = (MsgMessage *)hSipMsg;

    if (NULL == pMsg->pBodyObject)
    {
        return RV_ERROR_NOT_FOUND;
    }
    hContentType = (RvSipContentTypeHeaderHandle)(pMsg->pBodyObject->pContentType);
    if (NULL == hContentType)
    {
        return RV_ERROR_NOT_FOUND;
    }
    /* Encode the Content-Type header on a consecutive buffer */
    RvLogInfo(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
              "RvSipMsgGetContentTypeHeader - Encoding Content-Type header. hHeader 0x%p, buffer 0x%p",
              hContentType, strBuffer));
    stat = ContentTypeHeaderConsecutiveEncode(hContentType, strBuffer,
                                              bufferLen, actualLen);
    if (RV_OK != stat)
    {
        RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                  "RvSipMsgGetContentTypeHeader: Failed to encode Content-Type header. stat is %d",
                  stat));
        return stat;
    }
    /* Add '\0' ion the beginning of the Content-Type string */
    *actualLen += 1;
    if (bufferLen < *actualLen)
    {
        return RV_ERROR_INSUFFICIENT_BUFFER;
    }
    strBuffer[*actualLen-1] = '\0';
    return RV_OK;

#else /* #ifndef RV_SIP_PRIMITIVES */

    return MsgUtilsFillUserBuffer(((MsgMessage*)hSipMsg)->hPool,
                                  ((MsgMessage*)hSipMsg)->hPage,
                                  SipMsgGetContentTypeHeader(hSipMsg),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
#endif /* #ifndef RV_SIP_PRIMITIVES */
}

/***************************************************************************
 * RvSipMsgSetContentTypeHeader
 * ------------------------------------------------------------------------
 * General: Sets a Content-Type string in the message object.
 * Return Value: RV_OK,RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:RvStatus
 *    hSipMsg      - Handle to a message object.
 *    pContentType - The ContentType string to be set in the message object.
 *                 If a NULL value is supplied, the existing ContentType
 *                 header is removed from the message.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetContentTypeHeader(
                                                IN    RvSipMsgHandle hSipMsg,
                                                IN    RvChar*       pContentType)
{
    MsgMessage*   msg;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    msg = (MsgMessage*)hSipMsg;

#ifndef RV_SIP_PRIMITIVES
    if ((NULL == pContentType) &&
        (NULL != msg->pBodyObject))
    {
        msg->pBodyObject->pContentType = NULL;
    }
    else if (NULL == pContentType) /* NULL == msg->pBodyObject */
    {
        return RV_OK;
    }
    else
    {
        RvStatus          stat;
        RvSipBodyHandle    hTempBody;
        RvUint32          index = 0;

        if (NULL == msg->pBodyObject)
        {
            stat = RvSipBodyConstructInMsg(hSipMsg, &hTempBody);
            if (RV_OK != stat)
                return stat;
        }
        if (NULL == msg->pBodyObject->pContentType)
        {
            RvSipContentTypeHeaderHandle hTempContentType;

            stat = RvSipContentTypeHeaderConstructInBody(
                                            (RvSipBodyHandle)msg->pBodyObject,
                                            &hTempContentType);
            if (RV_OK != stat)
                return stat;
        }
        /* Skip white spaces and tabs */
        while ((MSG_SP == pContentType[index]) ||
               (MSG_HT == pContentType[index]))
        {
            index++;
        }

        stat = MsgParserParseStandAloneHeader(msg->pMsgMgr,
                                            SIP_PARSETYPE_CONTENT_TYPE,
                                            pContentType + index,
                                            RV_TRUE,
                                            (void*)msg->pBodyObject->pContentType);
        return stat;
    }
#else /* #ifndef RV_SIP_PRIMITIVES */
    if(pContentType == NULL)
    {
        msg->strContentType = UNDEFINED;
    }
    else
    {
        RvInt32      newStr;

        newStr = MsgUtilsAllocSetString(msg->pMsgMgr,
                                        msg->hPool,
                                        msg->hPage,
                                        pContentType);
        if(newStr == UNDEFINED)
            return RV_ERROR_OUTOFRESOURCES;

        else
        {
            msg->strContentType = newStr;
#ifdef SIP_DEBUG
            msg->pContentType = (RvChar *)RPOOL_GetPtr(msg->hPool, msg->hPage,
                                             msg->strContentType);
#endif /* SIP_DEBUG */
        }
    }
#endif /* #ifndef RV_SIP_PRIMITIVES */
    return RV_OK;
}

/***************************************************************************
 * RvSipMsgGetStrBadSyntaxStartLine
 * ------------------------------------------------------------------------
 * General: Copies the bad-syntax start line from the message object
 *          into a given buffer.
 *          When a message is received and the start line contains
 *          a syntax error, the start line is kept as a separate bad syntax
 *          string. This function retrieves this string.
 *          If the bufferLen size adequate, the function copies the parameter
 *          into the strBuffer. Otherwise, it returns RV_ERROR_INSUFFICIENT_BUFFER
 *          and the actualLen parameter contains the required buffer length.
 *          Use this function in RvSipTransportBadSyntaxStartLineMsgEv event
 *          implementation, in order to check and fix the defected start-line.
 *          Note: If the start line is valid the function will return RV_ERROR_NOT_FOUND
 *          and the actualLen parameter will be set to 0.
 * Return Value: RV_OK or RV_ERROR_INSUFFICIENT_BUFFER.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg   - Handle of the message object.
 *        strBuffer - buffer to fill with the bad syntax start line
 *        bufferLen - the length of the buffer.
 * output:actualLen - The length of the bad syntax start line + 1 to include
 *                    a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgGetStrBadSyntaxStartLine(
                                                   IN  RvSipMsgHandle hSipMsg,
                                                   IN  RvChar*       strBuffer,
                                                   IN  RvUint        bufferLen,
                                                   OUT RvUint*       actualLen)
{
    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;

    return MsgUtilsFillUserBuffer(((MsgMessage*)hSipMsg)->hPool,
                                  ((MsgMessage*)hSipMsg)->hPage,
                                  SipMsgGetBadSyntaxStartLine(hSipMsg),
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
}

/***************************************************************************
 * RvSipMsgStartLineFix
 * ------------------------------------------------------------------------
 * General: Fixes a start-line with bad-syntax information. When a message
 *          is received with a syntax error in the start line, the start line
 *          string is kept as a separate "bad-syntax" string.
 *          Use this function in order to fix the start line.
 *          The function parses a given correct start line string to the supplied message object.
 *          If parsing succeeds the function places all fields inside the message start
 *          line and removes the bad syntax string.
 *          If parsing fails, the bad-syntax string is remained untouched.
 *          Use this function in RvSipTransportBadSyntaxStartLineMsgEv event
 *          implementation, in order to fix a defected start-line.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSipMsg      - Handle to the message object.
 *        pFixedBuffer - Buffer containing a legal start-line value.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgStartLineFix(
                                     IN RvSipMsgHandle hSipMsg,
                                     IN RvChar*       pFixedBuffer)
{
    MsgMessage* pMsg = (MsgMessage*)hSipMsg;
    RvStatus   rv;

    if(hSipMsg == NULL || pFixedBuffer == NULL)
        return RV_ERROR_INVALID_HANDLE;

    RvLogDebug(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
            "RvSipMsgStartLineFix: start-line in msg 0x%p ", hSipMsg));
    /* this is a case of start-line with NULL instead of CRLF in the end -
       parse it as a stand-alone header */
    rv = MsgParserParseStandAloneHeader(pMsg->pMsgMgr, 
                                        SIP_PARSETYPE_START_LINE, 
                                        pFixedBuffer, 
                                        RV_TRUE, 
                                        (void*)hSipMsg);
    
    if(rv == RV_OK)
    {
        pMsg->strBadSyntaxStartLine = UNDEFINED;
#ifdef SIP_DEBUG
        pMsg->pBadSyntaxStartLine = NULL;
#endif
        RvLogDebug(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
            "RvSipMsgStartLineFix: start-line in msg 0x%p was fixed to %s", hSipMsg, pFixedBuffer));
    }
    else
    {
        RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
            "RvSipMsgStartLineFix: start-line in msg 0x%p - Failure in parsing start-line %s", hSipMsg, pFixedBuffer));
    }

    return rv;
}
/***************************************************************************
 * RvSipMsgSetBadSyntaxReasonPhrase
 * ------------------------------------------------------------------------
 * General: This function sets the bad-syntax reason phrase string in the message.
 *          For application that wishes to reject the bad-syntax request, with
 *          a different reason phrase.
 *          (can be used only in the transport bad-syntax callback).
 * Return Value:  RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsg     - Handle of a message object.
 *         pBSReasonPhrase - Pointer to the reason phrase string.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetBadSyntaxReasonPhrase(RvSipMsgHandle hMsg,
                                                           RvChar*       pBSReasonPhrase)
{
    MsgMessage* msg = (MsgMessage*)hMsg;
    if (hMsg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(pBSReasonPhrase == NULL)
    {
        msg->strBadSyntaxReasonPhrase = UNDEFINED;
#ifdef SIP_DEBUG
        msg->pBadSyntaxReasonPhrase = NULL;
#endif
        return RV_OK;
    }
    else
    {
        return SipMsgSetBadSyntaxReasonPhrase(hMsg, pBSReasonPhrase);
    }

    
}

/***************************************************************************
 * RvSipMsgParse
 * ------------------------------------------------------------------------
 * General: Parses a SIP textual message into a message object.
 *          All the textual fields are placed inside the object.
 *          Note that the given message buffer must be ended with two CRLF.
 *          one CRLF must separate between headers in the message.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hMsg      - A handle to the message object.
 *    buffer    - Buffer containing a textual msg ending with two CRLFs.
 *  bufferLen - Length of buffer.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgParse(IN RvSipMsgHandle hMsg,
                                         IN RvChar*       buffer,
                                         IN RvInt32       bufferLen)
{
    MsgMessage                 *pMsg = (MsgMessage*)hMsg;
    RvStatus                   rv;

    if(hMsg == NULL ||(buffer == NULL))
        return RV_ERROR_BADPARAM;

    RvLogDebug(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                  "RvSipMsgParse: msg 0x%p", hMsg));

    MsgClean(pMsg, RV_TRUE);

    rv = SipTransportMsgBuilderUdpMakeMsg(pMsg->pMsgMgr->hTransportMgr,
                                          buffer,
                                          bufferLen,
                                          RV_FALSE,
                                          hMsg,
 										  RVSIP_TRANSPORT_UNDEFINED,
                                          NULL,
                                          NULL,
                                          NULL);
    if(rv != RV_OK)
    {
        RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                  "RvSipMsgParse: SipTransportMsgBuilderUdpMakeMsg Failed (rv=%d). msg 0x%p",
                  rv, hMsg));
        return rv;
    }
    return RV_OK;
}


/****************************************************/
/*            "GENERAL HEADER" FUNCTIONS               */
/****************************************************/

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * RvSipMsgGetRpoolString
 * ------------------------------------------------------------------------
 * General: Copy a string parameter from the message into a given page
 *          from a specified pool. The copied string is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipMsg - Handle to the message object.
 *           eStringName   - The string the user wish to get. In this version RVSIP_MSG_CALL_ID
 *                         is the only string supported.
 *  Input/Output:
 *         pRpoolPtr     - pointer to a location inside an rpool. You need to
 *                         supply only the pool and page. The offset where the
 *                         returned string was located will be returned as an
 *                         output parameter.
 *                         If the function set the returned offset to
 *                         UNDEFINED is means that the parameter was not
 *                         set to the Other header object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgGetRpoolString(
                             IN    RvSipMsgHandle                 hSipMsg,
                             IN    RvSipMessageStringName         eStringName,
                             INOUT RPOOL_Ptr                     *pRpoolPtr)
{
    MsgMessage* pMsg = (MsgMessage*)hSipMsg;
    RvInt32    requestedParamOffset = UNDEFINED;

    if(hSipMsg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE)
    {
        RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                  "RvSipMsgGetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }


    switch(eStringName)
    {
    case RVSIP_MSG_CALL_ID:
        return RvSipOtherHeaderGetRpoolString(pMsg->hCallIdHeader,
                                       RVSIP_OTHER_HEADER_VALUE,
                                       pRpoolPtr);

    case RVSIP_MSG_BAD_SYNTAX_START_LINE:
        requestedParamOffset = pMsg->strBadSyntaxStartLine;
        break;
    case RVSIP_MSG_BODY:
#ifndef RV_SIP_PRIMITIVES
        if (pMsg->pBodyObject != NULL)
        {
            requestedParamOffset = pMsg->pBodyObject->strBody;
        }
#else
        requestedParamOffset = pMsg->strBody;
#endif /* RV_SIP_PRIMITIVES */
        break;
    default:
        RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                  "RvSipMsgGetRpoolString - Failed, the requested parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }

    /* the parameter does not exit in the header - return UNDEFINED*/
    if(requestedParamOffset == UNDEFINED)
    {
        pRpoolPtr->offset = UNDEFINED;
        return RV_OK;
    }

    pRpoolPtr->offset = MsgUtilsAllocCopyRpoolString(pMsg->pMsgMgr,
                                                     pRpoolPtr->hPool,
                                                     pRpoolPtr->hPage,
                                                     pMsg->hPool,
                                                     pMsg->hPage,
                                                     requestedParamOffset);

    if (pRpoolPtr->offset == UNDEFINED)
    {
        RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                  "RvSipMsgGetRpoolString - Failed to copy the string into the supplied page"));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;
}

/***************************************************************************
 * RvSipMsgSetRpoolString
 * ------------------------------------------------------------------------
 * General: Sets a string into a specified parameter in the message object.
 *          The given string is located on an RPOOL memory and is not consecutive.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hSipMsg       - Handle to the message object.
 *           eStringName   - The string the user wish to set. In this version
 *                         RVSIP_MSG_CALL_ID is the only string supported.
 *           pRpoolPtr     - pointer to a location inside an rpool where the
 *                         new string is located.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetRpoolString(
                             IN    RvSipMsgHandle          hSipMsg,
                             IN    RvSipMessageStringName  eStringName,
                             IN    RPOOL_Ptr               *pRpoolPtr)
{
    MsgMessage* pMsg = (MsgMessage*)hSipMsg;

    if(hSipMsg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pRpoolPtr == NULL ||
       pRpoolPtr->hPool == NULL ||
       pRpoolPtr->hPage == NULL_PAGE ||
       pRpoolPtr->offset == UNDEFINED   )
    {
        RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                 "RvSipMsgSetRpoolString - Failed, the supplied RPOOL pointer is invalid"));
        return RV_ERROR_BADPARAM;
    }

    switch(eStringName)
    {
    case RVSIP_MSG_CALL_ID:
        return RvSipOtherHeaderSetRpoolString(pMsg->hCallIdHeader,
                                       RVSIP_OTHER_HEADER_VALUE,
                                       pRpoolPtr);
    case RVSIP_MSG_BODY:
        return MsgSetRpoolBody(pMsg,pRpoolPtr);
    default:
        RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                "RvSipMsgSetRpoolString - Failed, the string parameter is not supported by this function"));
        return RV_ERROR_BADPARAM;
    }
}

#endif /* #ifndef RV_SIP_PRIMITIVES*/

/****************************************************/
/*        BODY FUNCTIONS                               */
/****************************************************/


/***************************************************************************
 * RvSipMsgGetBody
 * ------------------------------------------------------------------------
 * General: Copies the body from the message object into a given buffer.
 *          If the bufferLen is adequate, the function copies the message
 *          body into strBuffer. Otherwise, the function returns
 *          RV_ERROR_INSUFFICIENT_BUFFER and actualLen contains the required
 *          buffer length.
 *          NOTE:
 *                If the message body is of type multipart it might be
 *                represented as a list of body parts, and not as a string. If
 *                the body contains a list of body parts, in order
 *                to retrieve the body string, the body object must be encoded.
 *                This function will encode the message body on a
 *                temporary memory page when ever the body object contains a
 *                list of body parts. The encoded body string will be copied
 *                to the supplied buffer, and the temporary page will be
 *                freed.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg   - Handle to the message object.
 *        strBuffer - buffer to fill with the requested parameter.
 *        bufferLen - the length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgGetBody(IN RvSipMsgHandle     hSipMsg,
                                           IN RvChar*           strBuffer,
                                           IN RvUint            bufferLen,
                                           OUT RvUint*          actualLen)
{
    RvStatus   stat;
#ifndef RV_SIP_PRIMITIVES
    MsgMessage *msg;
    HPAGE       hTempPage;
    RvUint32   length;
#else
    RvInt32    strBody;
#endif /*#ifndef RV_SIP_PRIMITIVES*/

    if(actualLen == NULL)
        return RV_ERROR_NULLPTR;
    *actualLen = 0;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    if(strBuffer == NULL)
        return RV_ERROR_NULLPTR;


#ifndef RV_SIP_PRIMITIVES
    msg = (MsgMessage *)hSipMsg;

    if (NULL == msg->pBodyObject)
        return RV_ERROR_NOT_FOUND;

    if (NULL == msg->pBodyObject->hBodyPartsList)
    {
        /* The body is represented as a string */
        if ((UNDEFINED == msg->pBodyObject->strBody) ||
            (0 == msg->pBodyObject->length))
            return RV_ERROR_NOT_FOUND;

        /* Check the sufficiency of the buffer */
        *actualLen = msg->pBodyObject->length;
        if (*actualLen+1 > bufferLen)
            return RV_ERROR_INSUFFICIENT_BUFFER;

        /* Copy the body string to the buffer */
        stat = RPOOL_CopyToExternal(msg->pBodyObject->hPool,
                                    msg->pBodyObject->hPage,
                                    msg->pBodyObject->strBody,
                                    strBuffer,
                                    *actualLen);
        /* Add '\0' at the end of the buffer */
        strBuffer[*actualLen] = '\0';
        *actualLen += 1;
        return stat;
    }
    else
    {
        /* Create boundary if needed */
        stat = BodyUpdateContentType((RvSipBodyHandle)msg->pBodyObject);
        if (RV_OK != stat)
            return stat;

        /* The body is represented as list of body parts. Encode the body to
           a temporary page */
        stat = RvSipBodyEncode((RvSipBodyHandle)msg->pBodyObject,
                                  msg->pBodyObject->hPool, &hTempPage, &length);
        if (RV_OK != stat)
            return stat;

        /* Check the sufficiency of the buffer */
        *actualLen = length;
        if (*actualLen+1 > bufferLen)
        {
            RPOOL_FreePage(msg->pBodyObject->hPool, hTempPage);
            return RV_ERROR_INSUFFICIENT_BUFFER;
        }

         /* Copy the body string to the buffer */
        stat = RPOOL_CopyToExternal(msg->pBodyObject->hPool,
                                    hTempPage,
                                    0,
                                    strBuffer,
                                    length);
        RPOOL_FreePage(msg->pBodyObject->hPool, hTempPage);
        /* Add '\0' at the end of the buffer */
        strBuffer[*actualLen] = '\0';
        *actualLen += 1;
        return stat;
    }

#else

    /* Get the body string offset */
    strBody = SipMsgGetBody(hSipMsg);

    if (UNDEFINED == strBody)
        return RV_ERROR_NOT_FOUND;

    /* Copy the body string to the buffer */
    stat = MsgUtilsFillUserBuffer(((MsgMessage*)hSipMsg)->hPool,
                                  ((MsgMessage*)hSipMsg)->hPage,
                                  strBody,
                                  strBuffer,
                                  bufferLen,
                                  actualLen);
    return stat;
#endif /*#ifndef RV_SIP_PRIMITIVES*/

}

/***************************************************************************
 * RvSipMsgSetBody
 * ------------------------------------------------------------------------
 * General: Sets a body in the message object and updates the content-length
 *          field with the body length.
 *          Remember to update the Content-Type value according to the new
 *          body.
 *          NOTE:
 *                It is relevant to use RvSipMsgSetBody() ONLY when the body
 *                string does not contain NULL characters. To set body string
 *                that contains NULL characters use RvSipBodySetBodyStr()
 *                on the body object of the message (you can get this
 *                body object using RvSipMsgGetBodyObject(), or construct
 *                a new body object to the message using
 *                RvSipbodyConstructInMsg()).
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSipMsg - Handle to the message object.
 *            strBody - SIP message body to be set in the message object.
 *                    If NULL is supplied, the existing body is removed and
 *                    the message object contentLength is set to zero.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetBody(IN    RvSipMsgHandle hSipMsg,
                                           IN    RvChar*       strBody)
{
    MsgMessage*   msg;

    if (hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    msg = (MsgMessage*)hSipMsg;

#ifndef RV_SIP_PRIMITIVES
    if (NULL == strBody)
    {
        if (NULL != msg->pBodyObject)
        {
            msg->pBodyObject->strBody        = UNDEFINED;
            msg->pBodyObject->hBodyPartsList = NULL;
            msg->pBodyObject->length         = 0;
#ifdef SIP_DEBUG
            msg->pBodyObject->pBody          = NULL;
#endif /* SIP_DEBUG */
        }
        msg->contentLength                   = 0;
    }
    else
    {
        RvStatus          stat;
        RvSipBodyHandle   hTempBody = NULL;
        RvUint32          strLen;

        if (NULL == msg->pBodyObject)
        {
            stat = RvSipBodyConstructInMsg(hSipMsg, &hTempBody);
            if (RV_OK != stat)
                return stat;
        }
        strLen = (RvUint32)strlen(strBody);
        stat = RvSipBodySetBodyStr((RvSipBodyHandle)msg->pBodyObject,
                                      strBody, strLen);

        if(stat != RV_OK)
        {
           RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                     "RvSipMsgSetBody - RvSipBodySetBodyStr failed for msg 0x%p (rv=%d)",hSipMsg,stat));
           return stat;

        }
        msg->contentLength = strLen;
        return stat;
    }

#else /* #ifndef RV_SIP_PRIMITIVES */
    if (strBody == NULL)
    {
        msg->strBody = UNDEFINED;
        msg->contentLength = 0;
#ifdef SIP_DEBUG
        msg->pBody = NULL;
#endif /* SIP_DEBUG */
    }
    else
    {
        RvInt32      newStr;

        newStr = MsgUtilsAllocSetString(msg->pMsgMgr,
                                        msg->hPool,
                                        msg->hPage,
                                        strBody);
        if(newStr == UNDEFINED)
            return RV_ERROR_OUTOFRESOURCES;
        else
        {
            msg->contentLength = strlen(strBody);
            msg->strBody         = newStr;
#ifdef SIP_DEBUG
            msg->pBody = (RvChar *)RPOOL_GetPtr(msg->hPool, msg->hPage,
                                        msg->strBody);
#endif /* SIP_DEBUG */
        }
    }
#endif /* #ifndef RV_SIP_PRIMITIVES */

    return RV_OK;
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * RvSipMsgGetBodyObject
 * ------------------------------------------------------------------------
 * General: Gets the body object of the given message object. The
 *          function returns a handle to the actual body of the
 *          message. Any changes that will be made to this body object are
 *          reflected in the message.
 * Return Value: The handle to the body object.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *      hSipMsg   - Handle to the message object.
 ***************************************************************************/
RVAPI RvSipBodyHandle RVCALLCONV RvSipMsgGetBodyObject(
                                            IN RvSipMsgHandle       hSipMsg)
{
    MsgMessage* msg;

    if(hSipMsg == NULL)
        return NULL;

    msg = (MsgMessage*)hSipMsg;

    return (RvSipBodyHandle)msg->pBodyObject;
}

/***************************************************************************
 * RvSipMsgSetBodyObject
 * ------------------------------------------------------------------------
 * General: Sets a body to the message. The given body is copied to the
 *          message. If the body object contains a Content-Type header it
 *          will be copied to the message object as well.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSipMsg - Handle to the message object.
 *            hBody   - The body object to be set to the message object.
 *                    If NULL is supplied, the existing body is removed from
 *                    the message object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetBodyObject(
                                           IN   RvSipMsgHandle     hSipMsg,
                                           IN   RvSipBodyHandle hBody)
{
    MsgMessage*   msg;
    RvStatus     stat;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    msg = (MsgMessage*)hSipMsg;

    if(hBody == NULL)
    {
        if (NULL != msg->pBodyObject)
        {
            msg->pBodyObject->hBodyPartsList = NULL;
            msg->pBodyObject->length = 0;
            msg->pBodyObject->pContentType = NULL;
            msg->pBodyObject->pContentID = NULL;
            msg->pBodyObject->strBody = UNDEFINED;
#ifdef SIP_DEBUG
            msg->pBodyObject->pBody = NULL;
#endif
        }
        msg->contentLength = 0;
        return RV_OK;
    }
    if (NULL == msg->pBodyObject)
    {
        RvSipBodyHandle hTempBody;

        stat = RvSipBodyConstructInMsg(hSipMsg, &hTempBody);
        if (RV_OK != stat)
            return stat;
    }
    stat = RvSipBodyCopy((RvSipBodyHandle)msg->pBodyObject, hBody);
    if (RV_OK != stat)
        return stat;
    msg->contentLength = msg->pBodyObject->length;
    return RV_OK;
}
#endif /* #ifndef RV_SIP_PRIMITIVES */


/***************************************************************************
 * RvSipMsgForceCompactForm
 * ------------------------------------------------------------------------
 * General: Instruct a message object to force all its headers to use
 *          compact form. If compact form is forced and a message is
 *          encoded, all headers that can take compact form will be encoded 
 *          with compact form. Headers that are handled by the stack as Other 
 *          headers and are added to the message by the application will not 
 *          use compact form. It is the application's responsibility to set 
 *          such header names with compact form. Other headers that are added
 *          to the message by the stack such as Supported header will be encoded
 *          with compact form.
 *          Note: This flag effects only the encoding results of full messages.
 *          If you will retrieve a specific header from the message and encode
 *          it, it will not be encoded with compact form if this specific header
 *          is not marked with compact form.
 *          Note: Address header list and headers in a multipart mime bodies are
 *          not effected by this flag and will not be encoded with compact form.
 * Return Value: Returns RV_Status.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg    - Handle to the message object.
 *        bForceCompactForm - specify whether or not to force a compact form
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgForceCompactForm(
                                   IN RvSipMsgHandle    hSipMsg,
                                   IN RvBool            bForceCompactForm)
{
    RvStatus    rv;
    MsgMessage *pMsg  = (MsgMessage*)hSipMsg;

    if(hSipMsg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
             "RvSipMsgForceCompactForm - Forcing compact form in hMsg 0x%p.  bForceCompactForm=%d",
             hSipMsg, bForceCompactForm));
    pMsg->bForceCompactForm = bForceCompactForm;

    /* forcing compact form in call-id*/
    rv = RvSipMsgSetCallIdCompactForm(hSipMsg,bForceCompactForm);
    if(rv != RV_OK)
    {
        RvLogWarning(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                       "RvSipMsgForceCompactForm: Failed to set compact form in Call-ID header."));
    }

    rv = RvSipMsgSetContentLengthCompactForm(hSipMsg,bForceCompactForm);
    if(rv != RV_OK)
    {
        RvLogWarning(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                       "RvSipMsgForceCompactForm: Failed to set compact form in Content length header."));
    }
    return RV_OK;

}

/***************************************************************************
 * RvSipMsgGetCallIdCompactForm
 * ------------------------------------------------------------------------
 * General: Gets the compact form indication of the Call-ID header.
 *          RV_TRUE - the header uses compact form. RV_FALSE - the header does
 *          not use compact form.
 * Return Value: Returns RV_Status.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg    - Handle to the message object.
 *        bIsCompact - specify whether or not to compact form is used
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgGetCallIdCompactForm(
                                   IN RvSipMsgHandle    hSipMsg,
                                   IN RvBool            *pbIsCompact)
{
    MsgMessage *pMsg  = (MsgMessage*)hSipMsg;
	
    if(hSipMsg == NULL || pbIsCompact == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
	if(pMsg->hCallIdHeader == NULL)
	{
		*pbIsCompact = RV_FALSE;
	}
	else
	{
		if(RvSipOtherHeaderGetStringLength(pMsg->hCallIdHeader,RVSIP_OTHER_HEADER_NAME) > 2)
		{
			*pbIsCompact = RV_FALSE;
		}
		else
		{
			*pbIsCompact = RV_TRUE;
		}
	}
	return RV_OK;
	
}


/***************************************************************************
 * RvSipMsgSetCallIdCompactForm
 * ------------------------------------------------------------------------
 * General: Instructs the Call-ID header to use the compact header
 *          name when the it's encoded.
 * Return Value: Returns RV_Status.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg    - Handle to the message object.
 *        bIsCompact - specify whether or not to use a compact form
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetCallIdCompactForm(
                                   IN RvSipMsgHandle    hSipMsg,
                                   IN RvBool            bIsCompact)
{
    RvStatus    rv;
    RvChar     *strHeaderPrefix;
    MsgMessage *pMsg  = (MsgMessage*)hSipMsg;

    if(hSipMsg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
             "RvSipMsgSetCallIdCompactForm - Setting compact form of Call-ID for hMsg 0x%p to %d",
             hSipMsg, bIsCompact));

    /* Construct the Call-ID header if it doesn't exist */
    if(pMsg->hCallIdHeader == NULL)
    {
        rv = RvSipOtherHeaderConstruct((RvSipMsgMgrHandle)pMsg->pMsgMgr,
                                       pMsg->hPool, pMsg->hPage, &(pMsg->hCallIdHeader));
        if(rv != RV_OK)
        {
            RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                       "RvSipMsgSetCallIdCompactForm: Failed to construct Call-ID header."));
            return rv;
        }
    }

    strHeaderPrefix = (bIsCompact == RV_TRUE) ? "i" : "Call-ID";
    rv              = SipOtherHeaderSetName(
                         pMsg->hCallIdHeader, strHeaderPrefix, NULL, NULL, UNDEFINED);
    if(rv != RV_OK)
    {
        RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
            "RvSipMsgSetCallIdCompactForm: Failed to set Call-ID header name."));
        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * RvSipMsgSetContentLengthCompactForm
 * ------------------------------------------------------------------------
 * General: Instructs the Content-Length header to use the compact header
 *          name when the it's encoded.
 * Return Value: Returns RV_Status.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg    - Handle to the message object.
 *        bIsCompact - specify whether or not to use a compact form
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetContentLengthCompactForm(
                                   IN  RvSipMsgHandle  hSipMsg,
                                   IN  RvBool          bIsCompact)
{
    MsgMessage* pMsg = (MsgMessage*)hSipMsg;

    if(hSipMsg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
             "RvSipMsgSetContentLengthCompactForm - Setting compact form of Content-Length for hMsg 0x%x to %d",
             hSipMsg, bIsCompact));

    pMsg->bCompContentLen = bIsCompact;

    return RV_OK;
}

/***************************************************************************
 * RvSipMsgGetContentLengthCompactForm
 * ------------------------------------------------------------------------
 * General: Gets the compact form indication of the Content-length header.
 *          RV_TRUE - the header uses compact form. RV_FALSE - the header does
 *          not use compact form.
 * Return Value: Returns RV_Status.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg    - Handle to the message object.
 *        pbIsCompact - specify whether or not to compact form is used
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgGetContentLengthCompactForm(
                                   IN  RvSipMsgHandle  hSipMsg,
                                   IN  RvBool          *pbIsCompact)
{
    MsgMessage* pMsg = (MsgMessage*)hSipMsg;

    if(hSipMsg == NULL || pbIsCompact == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    *pbIsCompact =  pMsg->bCompContentLen;

    return RV_OK;
}


/***************************************************************************
 * SipMsgPushHeadersFromList
 * ------------------------------------------------------------------------
 * General: The function push headers from a list, to a message.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hMsg         - Handle to the message object.
 *           hHeadersList - The headers list.
 ***************************************************************************/
RvStatus RVCALLCONV SipMsgPushHeadersFromList(RvSipMsgHandle        hMsg,
                                               RvSipCommonListHandle hHeadersList)
{
    RvStatus rv;
    MsgMessage* pMsg;
    RvSipCommonListElemHandle hRelative = NULL;
    RvSipHeaderType eHeaderType;
    void*           pHeader;
    void*           pNewAloocatedHeader;
    RvSipHeadersLocation location = RVSIP_FIRST_HEADER;
    RvSipHeaderListElemHandle    hListElem = NULL;

    pMsg = (MsgMessage*)hMsg;

    rv = RvSipCommonListGetElem(hHeadersList, RVSIP_FIRST_ELEMENT, NULL,
                                (RvInt*)&eHeaderType, &pHeader, &hRelative);
    if(rv != RV_OK)
    {
        RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
            "SipMsgPushHeadersFromList: Failed to get first element from list 0x%p (rv=%d)",
            hHeadersList, rv));
        return rv;
    }
    while(pHeader != NULL)
    {
        /* push header to message */
        rv = RvSipMsgPushHeader(hMsg, location, pHeader, eHeaderType, &hListElem,
                                &pNewAloocatedHeader);
        if(rv != RV_OK)
        {
            RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
            "SipMsgPushHeadersFromList: Failed to push header to msg 0x%p (rv=%d)",
            hMsg, rv));
            return rv;
        }
        /* get next header from list */
        location = RVSIP_NEXT_HEADER;
        rv = RvSipCommonListGetElem(hHeadersList, RVSIP_NEXT_ELEMENT, hRelative,
                                (RvInt*)&eHeaderType, &pHeader, &hRelative);
        if(rv != RV_OK)
        {
            RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
            "SipMsgPushHeadersFromList: Failed to get next element from list 0x%p (rv=%d)",
            hHeadersList, rv));
            return rv;
        }
    }
    return RV_OK;
}

#ifdef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * RvSipMsgGetCallIDHeaderHandle
 * ------------------------------------------------------------------------
 * General: Gets the Handle to the Call-Id header object from the message object.
 *          Note that the Call-Id header is held by an RvSipOtherHeader object.
 * Return Value: Returns RvSipOtherHeaderHandle, or NULL if the message does
 *               not contain a Call-Id header.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg - Handle to a message object.
 ***************************************************************************/
RVAPI RvSipOtherHeaderHandle RVCALLCONV RvSipMsgGetCallIDHeaderHandle(
											IN    RvSipMsgHandle hSipMsg)
{
	if (hSipMsg == NULL)
	{
		return NULL;
	}

	return SipMsgGetCallIDHeaderHandle(hSipMsg);
}

/***************************************************************************
 * RvSipMsgSetCallIDHeaderHandle
 * ------------------------------------------------------------------------
 * General: Sets a Call-Id header in the message object. If there is no Call-Id
 *          header in the message, the function constructs a Call-Id header.
 *          Note that the Call-Id header is held by an RvSipOtherHeader object.
 * Return Value: RV_OK,RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipMsg       - Handle to the message object.
 *    hCallIdHeader - Handle to the Call-Id header to be set. If a NULL is supplied,
 *                    the existing Call-Id header is removed from the message.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetCallIDHeaderHandle(
									 IN  RvSipMsgHandle          hSipMsg,
									 IN  RvSipOtherHeaderHandle  hCallIdHeader)
{
	if (hSipMsg == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	return SipMsgSetCallIDHeaderHandle(hSipMsg, hCallIdHeader);
}

/***************************************************************************
 * RvSipMsgGetContentLengthHeaderHandle
 * ------------------------------------------------------------------------
 * General: Get the Content-Length header of this message.
 *          Notice: The Content-Length is handled by the SipMsg as an RvInt32.
 *          However, for JSR32 purposes we allow handling it as a header.
 * 
 * Return Value: A pointer to the Content-Length of the message
 * ------------------------------------------------------------------------
 * Input:   hSipMsg         - A handle to the message object.
 * Output:  phContentLength - A handle to the content-length header representation
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgGetContentLengthHeaderHandle(
										IN  RvSipMsgHandle                  hSipMsg,
										OUT RvSipContentLengthHeaderHandle* phContentLength)
{
	MsgMessage* pMsg;
	void**      pContentLengthStruct;
	
	if (hSipMsg == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (phContentLength == NULL)
	{
		return RV_ERROR_BADPARAM;
	}

	pMsg = (MsgMessage*)hSipMsg;
	
	/* The Content-Length header is in fact a structure of two pointers:
	1. A pointer to the RvInt32 contentLength field in the message
	2. A pointer to the RvBool bCompContentLen field in the message */

	if (pMsg->contentLength == UNDEFINED)
	{
		*phContentLength = NULL;
		return RV_OK;
	}

	pContentLengthStruct = (void**)SipMsgUtilsAllocAlign(pMsg->pMsgMgr, pMsg->hPool, pMsg->hPage, sizeof(void*)*2, RV_TRUE);
    if(pContentLengthStruct == NULL)
    {
        RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
					"RvSipMsgGetContentLengthHeaderHandle - Failed to allocate required memory. hPage 0x%p, hPool 0x%p.",
					pMsg->hPage, pMsg->hPool));
        return RV_ERROR_OUTOFRESOURCES;
    }

	pContentLengthStruct[0] = (void*)(&pMsg->contentLength);
	pContentLengthStruct[1] = (void*)(&pMsg->bCompContentLen);

	*phContentLength = (RvSipContentLengthHeaderHandle)pContentLengthStruct;
	return RV_OK;
}

/***************************************************************************
 * RvSipMsgSetContentLengthHeaderHandle
 * ------------------------------------------------------------------------
 * General: Set a Content-Length header to the message.
 *          Notice: The Content-Length is handled by the SipMsg as an RvInt32.
 *          However, for JSR32 purposes we allow handling it as a header.
 * 
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Input:   hSipMsg        - A handle to the message object.
 *          hContentLength - A handle to the Content-Length header to set
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetContentLengthHeaderHandle(
										IN  RvSipMsgHandle                 hSipMsg,
										IN  RvSipContentLengthHeaderHandle hContentLength)
{
	MsgMessage* pMsg;
	void**      pContentLength;
	RvInt32*    pContentLengthVal;
	RvBool*     pbCompContentLen;
	
	if (hSipMsg == NULL || hContentLength == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	pMsg = (MsgMessage*)hSipMsg;
	
	/* The Content-Length header is in fact a structure of two pointers:
	1. A pointer to the RvInt32 contentLength field in the message
	2. A pointer to the RvBool bCompContentLen field in the message */

	pContentLength    = (void**)hContentLength;
	pContentLengthVal = (RvInt32*)pContentLength[0];
	pbCompContentLen  = (RvBool*)pContentLength[1];
	
	pMsg->contentLength   = *pContentLengthVal;
	pMsg->bCompContentLen = *pbCompContentLen;

	return RV_OK;
}

/***************************************************************************
 * RvSipMsgGetNumOfHeaders
 * ------------------------------------------------------------------------
 * General: This function returns the total number of headers that are currently
 *          held by the given message. This includes specific headers such as
 *          To and CSeq headers, and general headers such as Other headers.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg          - A handle to the message object.
 * Output:  pNumOfHeaders - The number of the headers held by this message.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgGetNumOfHeaders(
											IN  RvSipMsgHandle     hMsg,
											OUT RvUint32*          pNumOfHeaders)
{
	RvStatus     rv;

	/* Good idea for optimizing the time - do not go over the list but rather have a counter
	of elements in rlist*/
	
    if (hMsg == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (pNumOfHeaders == NULL)
	{
		return RV_ERROR_BADPARAM;
	}
	
	rv = MsgGetNumOfHeaders(hMsg, pNumOfHeaders);
	if (RV_OK != rv)
	{
		RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
			"RvSipMsgGetNumOfHeaders - Failed to get the number of headers from message 0x%p",
			hMsg));		
	}
	
	return rv;
}

#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS                               */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RequestLineEncode
 * ------------------------------------------------------------------------
 * General: The function encode the requestLine on the given page.
 *          The encoding form for requestLine is:
 *          RequestLine = Method SP Request-URI SP SIP-Version CRLF
 *          note - the function does not allocate and does not free page!!!
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     reqLine - Pointer to the requestLine struct.
 *             hPage -  Handle of the page of the encoded buffer.
 * Output:      encoded - Pointer to the beginning of the encoded string on the page.
 *           length - The given length + The new encoded requestLine length.
 ***************************************************************************/
static RvStatus RequestLineEncode(IN    MsgMessage*     pMsg,
                                   IN    MsgRequestLine* reqLine,
                                   IN    HRPOOL          hPool,
                                   IN    HPAGE           hPage,
                                   INOUT RvUint32*      length)
{
    RvStatus   stat;

    /* bad - syntax */
    if(pMsg->strBadSyntaxStartLine > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pMsg->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pMsg->hPool,
                                    pMsg->hPage,
                                    pMsg->strBadSyntaxStartLine,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                "RequestLineEncode - Failed to encode bad-syntax string. RvStatus is %d, hPool 0x%p, hPage 0x%p",
                stat, hPool, hPage));
        }


    }
    else /* legal. not bad-syntax */
    {
        /* Method */
        stat = MsgUtilsEncodeMethodType(pMsg->pMsgMgr,
                                        hPool,
                                        hPage,
                                        reqLine->eMethod,
                                        pMsg->hPool,
                                        pMsg->hPage,
                                        reqLine->strMethod,
                                        length);


        /* this is a check that we should not omit, because it returns invalidParameter
        if eMethod is OTHER and strMethod is NULL*/
        if(stat!= RV_OK)
            return stat;

        /* SP */
        stat = MsgUtilsEncodeExternalString(pMsg->pMsgMgr, hPool, hPage, " ", length);

        /* RequestURI */
        if(reqLine->hRequestUri == NULL)
        {
            RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                "RequestLineEncode - Failed to encode request line. No URI for the request line. not an optional parameter"));
            return RV_ERROR_UNKNOWN;
        }

        stat = AddrEncode(reqLine->hRequestUri,
                           hPool,
                           hPage,
                           RV_FALSE,
						   RV_FALSE,
                           length);
        if (stat != RV_OK)
            return stat;

        /* SP */
        stat = MsgUtilsEncodeExternalString(pMsg->pMsgMgr, hPool, hPage, " ", length);

        /* SIP-Version */
        stat = MsgUtilsEncodeExternalString(pMsg->pMsgMgr, hPool, hPage, "SIP/2.0", length);
    }

    /* CRLF */
    stat = MsgUtilsEncodeCRLF(pMsg->pMsgMgr, hPool, hPage, length);

    return stat;
}

/***************************************************************************
 * StatusLineEncode
 * ------------------------------------------------------------------------
 * General: The function encode the statusLine on the given page.
 *          The encoding form for statusLine is:
 *          StatusLine = SIP-Version SP Status-Code SP Reason-Phrase CRLF
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     reqLine - Pointer to the requestLine struct.
 *             hPage -  Handle of the page of the encoded buffer.
 * Output:      encoded - Pointer to the beginning of the encoded string on the page.
 *           length - The given length + The new encoded requestLine length.
 ***************************************************************************/
static RvStatus StatusLineEncode(IN    MsgMessage*     pMsg,
                                   IN    MsgStatusLine*  statLine,
                                   IN    HRPOOL          hPool,
                                   IN    HPAGE           hPage,
                                   INOUT RvUint32*      length)
{
    RvStatus   stat;
    RvChar     code[11];

    /* SIP-Version */
    stat = MsgUtilsEncodeExternalString(pMsg->pMsgMgr, hPool, hPage, "SIP/2.0", length);
    if(stat != RV_OK)
    {
        return stat;
    }

    /* SP */
    stat = MsgUtilsEncodeExternalString(pMsg->pMsgMgr, hPool, hPage, " ", length);
    if(stat != RV_OK)
    {
        return stat;
    }

    /* Status-Code */
    /*itoa(statLine->statusCode, code, 10);*/
    MsgUtils_itoa(statLine->statusCode, code);
    stat = MsgUtilsEncodeExternalString(pMsg->pMsgMgr, hPool, hPage, code, length);
    if(stat != RV_OK)
    {
        return stat;
    }

    /* SP */
    stat = MsgUtilsEncodeExternalString(pMsg->pMsgMgr, hPool, hPage, " ", length);
    if(stat != RV_OK)
    {
        return stat;
    }

    /* Reason-Phrase */
    if(statLine->strReasonePhrase > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(pMsg->pMsgMgr,
                                    hPool,
                                    hPage,
                                    pMsg->hPool,
                                    pMsg->hPage,
                                    statLine->strReasonePhrase,
                                    length);
        if(stat != RV_OK)
        {
            return stat;
        }
    }

    /* CRLF */
    stat = MsgUtilsEncodeCRLF(pMsg->pMsgMgr, hPool, hPage, length);

    return stat;
}


/***************************************************************************
 * MsgEncodeHeaderList
 * ------------------------------------------------------------------------
 * General: Encodes the headers in the message headers list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     msg     - Pointer to the message holding the headers list.
 *          eOption - encode legal headers only, or all headers.
 *            hPool   - Pool of the page to be filled with encoded string.
 *            hPage   - Page to be filled with encoded string.
 * Output:     length  - Length of the encoded string.
 ***************************************************************************/
static RvStatus RVCALLCONV MsgEncodeHeaderList(IN   RvSipMsgHandle   hMsg,
                                               IN    RvSipMsgHeadersOption eOption,
                                               IN    HRPOOL          hPool,
                                               IN    HPAGE           hPage,
                                               INOUT RvUint32*      length)
{
    RvSipHeaderListElemHandle      elem;
    RvSipHeaderType                type, lastType;
    RvChar                         name[MAX_HEADER_NAME_LEN], lastName[MAX_HEADER_NAME_LEN];
    void*                          returnedHeader;
    RvStatus                       stat;
    MsgMessage*                    msg = (MsgMessage*)hMsg;
    RvInt                          safeCounter = 0;
    RvUint                         actLen;
    RvBool                         bFirstOtherWasAlreadyEncoded = RV_FALSE;

    lastType = RVSIP_HEADERTYPE_UNDEFINED;
    name[0]        = '\0';
    lastName[0] = '\0';

    if (msg->headerList == NULL)
    {
        return RV_OK;
    }

    stat = sortHeaderList(msg);
    if(stat != RV_OK)
    {
        return stat;
    }

    /* getting first header from source list -
    the function will return us only the headers we need - legal or all headers. */
    returnedHeader = RvSipMsgGetHeaderExt(hMsg,
                                       RVSIP_FIRST_HEADER,
                                       eOption,
                                       &elem,
                                       &type);

    while((returnedHeader != NULL) && (safeCounter < MAX_HEADERS_PER_MESSAGE))
    {
        ++safeCounter;
        if((lastType == RVSIP_HEADERTYPE_ALLOW && type != RVSIP_HEADERTYPE_ALLOW) ||
#ifndef RV_SIP_OTHER_HEADER_ENCODE_SEPARATELY
            (lastType == RVSIP_HEADERTYPE_OTHER && type != RVSIP_HEADERTYPE_OTHER)||
#endif /*RV_SIP_OTHER_HEADER_ENCODE_SEPARATELY*/
            (lastType == RVSIP_HEADERTYPE_ALLOW_EVENTS && type != RVSIP_HEADERTYPE_ALLOW_EVENTS))
        {
            /* CRLF */
            stat = MsgUtilsEncodeCRLF(msg->pMsgMgr, hPool, hPage, length);
        }
        
        /* If subsequence of OTHER headers was finished,
        reset flags before next such subsequence.
        These flags impact on how subsequnce of OTHER headers is encoded. */
        if (lastType == RVSIP_HEADERTYPE_OTHER && type != RVSIP_HEADERTYPE_OTHER)
        {
            bFirstOtherWasAlreadyEncoded = RV_FALSE;
            lastName[0] = '\0';
        }

        /* encoding the returnedHeader header */
        switch(type)
        {
#ifndef RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_ALLOW:
            if(lastType == type)
            {
                stat = AllowHeaderEncode((RvSipAllowHeaderHandle)returnedHeader,
                    hPool, hPage, RV_FALSE, RV_FALSE, length);
            }
            else
            {
                stat = AllowHeaderEncode((RvSipAllowHeaderHandle)returnedHeader,
                    hPool, hPage, RV_TRUE, RV_FALSE, length);
            }
            break;
#ifdef RV_SIP_SUBS_ON
        case RVSIP_HEADERTYPE_ALLOW_EVENTS:
            if(lastType == type)
            {
                stat = AllowEventsHeaderEncode((RvSipAllowEventsHeaderHandle)returnedHeader,
                    hPool, hPage, RV_FALSE, RV_FALSE, msg->bForceCompactForm,length);
            }
            else
            {
                stat = AllowEventsHeaderEncode((RvSipAllowEventsHeaderHandle)returnedHeader,
                    hPool, hPage, RV_TRUE, RV_FALSE, msg->bForceCompactForm, length);
            }
            break;
#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /* #ifndef RV_SIP_PRIMITIVES */
        case RVSIP_HEADERTYPE_OTHER:
/* encoding of other header is done without the CRLF at it end.
   for every other header encoding we must decide wether to encode a CRLF before it or not.
   if no other header was already encoded (bFirstOtherWasAlreadyEncoded == RV_FALSE) -
   then previous header had set the CRLF, and there is no need to set CRLF.
   if the previous header was an other header -
   if previous header name is different than current header name - set a CRLF before current header.
   if names are equal, then no CRLF, and headers will be concatenated.*/
#ifdef RV_SIP_OTHER_HEADER_ENCODE_SEPARATELY
            stat = OtherHeaderEncode((RvSipOtherHeaderHandle)returnedHeader,
                                      hPool, hPage, RV_TRUE, RV_FALSE, msg->bForceCompactForm,length);
            break;
#endif /*RV_SIP_OTHER_HEADER_ENCODE_SEPARATELY*/
            stat = RvSipOtherHeaderGetName((RvSipOtherHeaderHandle)returnedHeader,
                name, 50, &actLen);

            if(stat != RV_OK)
            {
                /* insert a CRLF only if this is not the first other header.
                   An other header with no name will always be encoded separately, therefore
                   it must have a CRLF before it.
                   if this is the first other header, then previous header set
                   a CRLF */
                if(bFirstOtherWasAlreadyEncoded == RV_TRUE)
                {
                    stat = MsgUtilsEncodeCRLF(msg->pMsgMgr, hPool, hPage, length);
                }
                stat = OtherHeaderEncode((RvSipOtherHeaderHandle)returnedHeader,
                                          hPool, hPage, RV_TRUE, RV_FALSE, msg->bForceCompactForm,length);
                bFirstOtherWasAlreadyEncoded = RV_TRUE;
            }
            else
            {
                if(SipCommonStricmp(name, lastName) == RV_TRUE)
                {
                    /* concatenate this header to previous header */
                    stat = OtherHeaderEncode((RvSipOtherHeaderHandle)returnedHeader,
                                              hPool, hPage, RV_FALSE, RV_FALSE, msg->bForceCompactForm,length);
                }
                else
                {
                    /* insert a CRLF only if this is not the first other header */
                    if(lastName[0] != '\0' || bFirstOtherWasAlreadyEncoded == RV_TRUE)
                    {
                        stat = MsgUtilsEncodeCRLF(msg->pMsgMgr, hPool, hPage, length);
                    }
                    if(stat == RV_OK)
                    {
                        stat = OtherHeaderEncode((RvSipOtherHeaderHandle)returnedHeader,
                                                hPool, hPage, RV_TRUE, RV_FALSE, msg->bForceCompactForm,length);
                        strcpy(lastName, name);
                        bFirstOtherWasAlreadyEncoded = RV_TRUE;
                    }
                }
            }
            break;

        default:
            stat = MsgHeaderEncode(returnedHeader, type, hPool, hPage, RV_FALSE, msg->bForceCompactForm,length);

        } /* end of switch */
        if(stat != RV_OK)
        {
            return stat;
        }

        if((type != RVSIP_HEADERTYPE_ALLOW) &&
#ifndef RV_SIP_OTHER_HEADER_ENCODE_SEPARATELY
           (type != RVSIP_HEADERTYPE_OTHER) &&
#endif /*RV_SIP_OTHER_HEADER_ENCODE_SEPARATELY*/
           (type != RVSIP_HEADERTYPE_ALLOW_EVENTS))
        {
            /* CRLF */
            stat = MsgUtilsEncodeCRLF(msg->pMsgMgr, hPool, hPage, length);
        }

        lastType = type;

        /* taking out next header form header list */
        returnedHeader = RvSipMsgGetHeaderExt(hMsg,
                                          RVSIP_NEXT_HEADER,
                                          eOption,
                                          &elem,
                                          &type);
    } /* end of while */
    if(safeCounter >= MAX_HEADERS_PER_MESSAGE)
    {
        RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
            "MsgEncodeHeaderList: msg 0x%p - too many headers in list (more than %d) - failed to encode all", 
            msg, MAX_HEADERS_PER_MESSAGE));
    }

    if((lastType == RVSIP_HEADERTYPE_ALLOW) ||
#ifndef RV_SIP_OTHER_HEADER_ENCODE_SEPARATELY
       (lastType == RVSIP_HEADERTYPE_OTHER) ||
#endif /*RV_SIP_OTHER_HEADER_ENCODE_SEPARATELY*/
       (lastType == RVSIP_HEADERTYPE_ALLOW_EVENTS))
    {
        /* CRLF */
        stat = MsgUtilsEncodeCRLF(msg->pMsgMgr, hPool, hPage, length);
    }

    return RV_OK;
}

/***************************************************************************
 * MsgEncode
 * ------------------------------------------------------------------------
 * General: Encodes the message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     msg     - Pointer to the message to encode
 *          eOption - encode legal headers only, or all headers.
 *            hPool   - Pool of the page to be filled with encoded string.
 *            hPage   - Page to be filled with encoded string.
 * Output:     length  - Length of the encoded string.
 ***************************************************************************/
static RvStatus RVCALLCONV MsgEncode(IN  RvSipMsgHandle         hSipMsg,
                                     IN  RvSipMsgHeadersOption   eOption,
                                     IN  HRPOOL                  hPool,
                                     IN  HPAGE                   hPage,
                                     OUT RvUint32*              length)
{
    RvStatus     stat;
    MsgMessage*   msg = (MsgMessage*)hSipMsg;

    /* start-line */
    if(msg->bIsRequest == RV_TRUE)
    {
        /* request */
        stat = RequestLineEncode(msg,
                                &(msg->startLine.requestLine),
                                hPool,
                                hPage,
                                length);
    }
    else if(msg->bIsRequest == RV_FALSE)
    {
        /* response */
        stat = StatusLineEncode(msg,
                               &(msg->startLine.statusLine),
                               hPool,
                               hPage,
                               length);
    }
    else if(eOption != RVSIP_MSG_HEADERS_OPTION_LEGAL_SYNTAX &&
            msg->strBadSyntaxStartLine > UNDEFINED)
    {
        stat = MsgUtilsEncodeString(msg->pMsgMgr,
                                    hPool,
                                    hPage,
                                    msg->hPool,
                                    msg->hPage,
                                    msg->strBadSyntaxStartLine,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                    "MsgEncode - hMsg 0x%p - strBadSyntaxStartLine Encode failed. ",
                    hSipMsg));
            return stat;
        }
        /* CRLF */
        stat = MsgUtilsEncodeCRLF(msg->pMsgMgr, hPool, hPage, length);
    }
    else
    {
        RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
            "MsgEncode - bIsRequest of message is undefined. hMsg is 0x%p",
            hSipMsg));
        return RV_ERROR_BADPARAM;
    }
    if(stat != RV_OK)
    {
        RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "MsgEncode - hMsg 0x%p - StatusLineEncode failed. ",
                hSipMsg));
        return stat;
    }
	/*if the application wishes to encode the header list before the
	  headers that are not in a list it should use the bellow flage*/
#ifdef RV_SIP_ENCODE_HEADER_LIST_FIRST    
	/* header list (Via, Allow, Contact, other) */
    stat = MsgEncodeHeaderList(hSipMsg, eOption, hPool, hPage, length);
    if(stat != RV_OK)
    {
        RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
			"MsgEncode - hMsg 0x%p. MsgEncodeHeaderList failed",
			hSipMsg));
        return stat;
    }
#endif
    /* message-headers*/
#ifndef RV_SIP_LIGHT
    /* From */
    if(msg->hFromHeader != NULL)
    {
        /* check bad-syntax */
        if(eOption == RVSIP_MSG_HEADERS_OPTION_LEGAL_SYNTAX)
        {
            if(((MsgPartyHeader*)msg->hFromHeader)->strBadSyntax > UNDEFINED)
            {
                RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                    "MsgEncode - hMsg 0x%p - From header has syntax error. eOption - LEGAL HEADERS.",
                    hSipMsg));
                return RV_ERROR_NOT_FOUND;
            }
        }

        stat = PartyHeaderEncode(msg->hFromHeader,
                                  hPool,
                                  hPage,
                                  RV_FALSE,
                                  RV_FALSE,
                                  msg->bForceCompactForm,
                                  length);
        if(stat != RV_OK)
        {
            RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "MsgEncode - hMsg 0x%p. From PartyHeaderEncode failed",
                hSipMsg));
            return stat;
        }

        /* CRLF */
        stat = MsgUtilsEncodeCRLF(msg->pMsgMgr, hPool, hPage, length);
    }

    /* To */
    if(msg->hToHeader != NULL)
    {
        /* check bad-syntax */
        if(eOption == RVSIP_MSG_HEADERS_OPTION_LEGAL_SYNTAX)
        {
            if(((MsgPartyHeader*)msg->hToHeader)->strBadSyntax > UNDEFINED)
            {
                RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                    "MsgEncode - hMsg 0x%p - To header has syntax error. eOption - LEGAL HEADERS. ",
                    hSipMsg));
                return RV_ERROR_NOT_FOUND;
            }
        }
        stat = PartyHeaderEncode(msg->hToHeader,
                                  hPool,
                                  hPage,
                                  RV_TRUE,
                                  RV_FALSE,
                                  msg->bForceCompactForm,
                                  length);
        if(stat != RV_OK)
        {
            RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "MsgEncode - hMsg 0x%p - To PartyHeaderEncode failed",
                hSipMsg));
            return stat;
        }

        /* CRLF */
        stat = MsgUtilsEncodeCRLF(msg->pMsgMgr, hPool, hPage, length);
    }

    /* Call-ID */
    if(msg->hCallIdHeader != NULL)
    {
        stat = OtherHeaderEncode(msg->hCallIdHeader, hPool, hPage, RV_TRUE, RV_FALSE, RV_FALSE,length);
        if(stat != RV_OK)
        {
            RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "MsgEncode - hMsg 0x%p - Call-ID OtherHeaderEncode failed",
                hSipMsg));
            return stat;
        }
        /* CRLF */
        stat = MsgUtilsEncodeCRLF(msg->pMsgMgr, hPool, hPage, length);
    }

    /* CSeq */
    if(msg->hCSeqHeader != NULL)
    {
        /* check bad-syntax */
        if(eOption == RVSIP_MSG_HEADERS_OPTION_LEGAL_SYNTAX)
        {
            if(((MsgCSeqHeader*)msg->hCSeqHeader)->strBadSyntax > UNDEFINED)
            {
                RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                    "MsgEncode - hMsg 0x%p - CSeq header has syntax error. eOption - LEGAL HEADERS. ",
                    hSipMsg));
                return RV_ERROR_NOT_FOUND;
            }
        }

        stat = CSeqHeaderEncode(msg->hCSeqHeader,
                                 hPool,
                                 hPage,
                                 RV_TRUE,
                                 RV_FALSE,
                                 length);
        if(stat != RV_OK)
        {
            RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "MsgEncode - hMsg 0x%p - CSeqHeaderEncode failed",
                hSipMsg));
            return stat;
        }

        /* CRLF */
        stat = MsgUtilsEncodeCRLF(msg->pMsgMgr, hPool, hPage, length);
    }
#endif /*#ifndef RV_SIP_LIGHT*/

#ifndef RV_SIP_ENCODE_HEADER_LIST_FIRST    
    /* header list (Via, Allow, Contact, other) */
    stat = MsgEncodeHeaderList(hSipMsg, eOption, hPool, hPage, length);
    if(stat != RV_OK)
    {
        RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "MsgEncode - hMsg 0x%p. MsgEncodeHeaderList failed",
                hSipMsg));
        return stat;
    }
#endif /*RV_SIP_ENCODE_HEADER_LIST_FIST*/
    /* body + content-type + content-length */
    stat = MsgEncodeBody(msg, eOption, hPool, hPage, length);
    if(stat != RV_OK)
    {
        RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "MsgEncode - hMsg 0x%p. MsgEncodeBody failed",
                hSipMsg));
        return stat;
    }
    return RV_OK;
}

/***************************************************************************
 * MsgEncodeBody
 * ------------------------------------------------------------------------
 * General: Encodes the message body.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     msg     - Pointer to the message to encode
 *          eOption - encode legal headers only, or all headers.
 *            hPool   - Pool of the page to be filled with encoded string.
 *            hPage   - Page to be filled with encoded string.
 * Output:     length  - Length of the encoded string.
 ***************************************************************************/
static RvStatus RVCALLCONV MsgEncodeBody(IN  MsgMessage*             msg,
                                          IN  RvSipMsgHeadersOption   eOption,
                                          IN  HRPOOL                  hPool,
                                          IN  HPAGE                   hPage,
                                          OUT RvUint32*              length)
{
    RvStatus  stat      = RV_OK;
    HPAGE     hTempPage = NULL_PAGE;
    RvInt32   contentLengthToEncode;
#ifndef RV_SIP_PRIMITIVES
    RvUint32  tempLength = 0;
#endif
   RvChar     tempString[16];


#ifndef RV_SIP_PRIMITIVES
    /* Update Content-Type header with media type and boundary if necessary */
    if (NULL != msg->pBodyObject)
    {
        stat = BodyUpdateContentType((RvSipBodyHandle)msg->pBodyObject);
        if (RV_OK != stat)
        {
            RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                      "MsgEncodeBody - msg 0x%p. BodyUpdateContentType failed. ",
                      msg));
            return stat;
        }
    }

    /* Encode Content-Type header */
    if ((NULL != msg->pBodyObject) &&
        (NULL != msg->pBodyObject->pContentType))
    {
        if(eOption == RVSIP_MSG_HEADERS_OPTION_LEGAL_SYNTAX)
        {
            if(((MsgContentTypeHeader*)msg->pBodyObject->pContentType)->strBadSyntax > UNDEFINED)
            {
                RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                    "MsgEncodeBody - msg 0x%p - Content-Type header has syntax error. eOption - LEGAL.",
                    msg));
                return stat;
            }
        }
        stat = ContentTypeHeaderEncode(
                            (RvSipContentTypeHeaderHandle)msg->pBodyObject->pContentType,
                            hPool, hPage, RV_FALSE, msg->bForceCompactForm, length);
        if(stat != RV_OK)
        {
            RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                     "MsgEncodeBody - msg 0x%p. ContentTypeHeaderEncode failed.",
                      msg));
            return stat;
        }
        /* CRLF */
        stat = MsgUtilsEncodeCRLF(msg->pMsgMgr, hPool, hPage, length);
    }
#else /* #ifndef RV_SIP_PRIMITIVES */
    /* content type */
    if(msg->strContentType > UNDEFINED)
    {
        /* Content-Type: */
        if(msg->bForceCompactForm == RV_TRUE)
        {
            stat = MsgUtilsEncodeExternalString(msg->pMsgMgr, hPool, hPage, " c:", length);
        }
        else
        {
            stat = MsgUtilsEncodeExternalString(msg->pMsgMgr, hPool, hPage, "Content-Type:", length);
        }

        stat = MsgUtilsEncodeString(msg->pMsgMgr,
                                    hPool,
                                    hPage,
                                    msg->hPool,
                                    msg->hPage,
                                    msg->strContentType,
                                    length);
        if(stat != RV_OK)
        {
            RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "MsgEncodeBody - msg 0x%p - Failed to encode content-type string. ",
                msg));
            return stat;
        }

        /* CRLF */
        stat = MsgUtilsEncodeCRLF(msg->pMsgMgr, hPool, hPage, length);
    }
    RV_UNUSED_ARG(eOption);
#endif /* #ifndef RV_SIP_PRIMITIVES */

#ifndef RV_SIP_PRIMITIVES
    /* Calculate updated Content-Length header */
    if ((NULL != msg->pBodyObject) &&
        (NULL != msg->pBodyObject->hBodyPartsList))
    {
        /* Encode message body on a temporary page to calculate the Content-Length */
        stat = RvSipBodyEncode((RvSipBodyHandle)msg->pBodyObject, hPool,
                                  &hTempPage, &tempLength);
        if(stat != RV_OK)
        {
            RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                      "MsgEncodeBody - msg 0x%p - RvSipBodyEncode failed.",
                      msg));
            return stat;
        }
        /* The length of the body is the length that is returned by
           RvSipBodyEncode() */
        contentLengthToEncode = tempLength;
    }
    else if ((NULL != msg->pBodyObject) &&
             (UNDEFINED != msg->pBodyObject->strBody))
    {
        contentLengthToEncode = msg->pBodyObject->length;
    }
    else
    {
        contentLengthToEncode = msg->contentLength;
    }
#else
    {
        contentLengthToEncode = msg->contentLength;
    }
#endif /* #ifndef RV_SIP_PRIMITIVES*/
    /* content length */
    if(contentLengthToEncode != UNDEFINED)
    {
        /* Content-Length: */
        RvChar *strHeaderPrefix;

        strHeaderPrefix = (msg->bCompContentLen == RV_TRUE ||
                           msg->bForceCompactForm == RV_TRUE) ? "l: " : "Content-Length: ";
        stat = MsgUtilsEncodeExternalString(msg->pMsgMgr, hPool, hPage, strHeaderPrefix, length);
        if(stat != RV_OK)
        {
            if (NULL_PAGE != hTempPage)
            {
                RPOOL_FreePage(hPool, hTempPage);
            }
            RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                      "MsgEncodeBody - msg 0x%p - encode of content length failed. Free temp page 0x%p on pool 0x%p. ",
                      msg, hTempPage, hPool));
            return stat;
        }
        MsgUtils_itoa(contentLengthToEncode, tempString);

        stat = MsgUtilsEncodeExternalString(msg->pMsgMgr, hPool, hPage, tempString, length);
        if(stat != RV_OK)
        {
            if (NULL_PAGE != hTempPage)
            {
                RPOOL_FreePage(hPool, hTempPage);
            }
            RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "MsgEncodeBody - msg 0x%p - encode of content length failed. Free temp page 0x%p on pool 0x%p.",
                msg, hTempPage, hPool));
            return stat;
        }

        /* CRLF */
        stat = MsgUtilsEncodeCRLF(msg->pMsgMgr, hPool, hPage, length);
    }

    /* CRLF */
    stat = MsgUtilsEncodeCRLF(msg->pMsgMgr, hPool, hPage, length);

#ifndef RV_SIP_PRIMITIVES
    /* Encode message-Body */
    if ((NULL != msg->pBodyObject) &&
        (msg->pBodyObject->strBody != UNDEFINED) &&
        (NULL == msg->pBodyObject->hBodyPartsList))
    {
        RvInt32 offset;

        /* There is a body object in the message. The list of body parts is empty.
           The body string is not empty. This means that the valid body is found
           completely in the string pointed by strBody and the length of the body
           is updated in the length parameter */
        /* Note: copying is done using specific length since the body may
           contain NULL characters */
        stat = RPOOL_Append(hPool, hPage,
                            msg->pBodyObject->length,
                            RV_FALSE,
                            &offset);
        if(stat != RV_OK)
        {
           if (NULL_PAGE != hTempPage)
           {
               RPOOL_FreePage(hPool, hTempPage);
           }
           RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                     "MsgEncodeBody - msg 0x%p - encode of body failed - Free  temp page 0x%p on pool 0x%p.",
                     msg, hTempPage, hPool));
           return stat;
        }
        stat = RPOOL_CopyFrom(hPool, hPage,
                              offset,
                              msg->hPool, msg->hPage,
                              msg->pBodyObject->strBody,
                              msg->pBodyObject->length);
        *length += msg->pBodyObject->length;
    }
    else if ((NULL != msg->pBodyObject) &&
             (NULL != msg->pBodyObject->hBodyPartsList))
    {
        RvInt32 offset;

        /* Copy the body string from the temporary page. The body was already
           encoded on the temporary page to calculate the Content-Length */
        /* Note: copying is done using specific length since the body may
           contain NULL characters */
        stat = RPOOL_Append(hPool,
                            hPage,
                            tempLength,
                            RV_FALSE,
                            &offset);
        if(stat != RV_OK)
        {
           if (NULL_PAGE != hTempPage)
           {
                RPOOL_FreePage(hPool, hTempPage);
           }
           RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                     "MsgEncodeBody - msg 0x%p - encode of body fail - Free temp page 0x%p on pool 0x%p. ",
                     msg, hTempPage, hPool));
           return stat;
        }
        stat = RPOOL_CopyFrom(hPool,
                              hPage,
                              offset,
                              hPool,
                              hTempPage,
                              0,
                              tempLength);
        if(stat != RV_OK)
        {
           if (NULL_PAGE != hTempPage)
           {
               RPOOL_FreePage(hPool, hTempPage);
           }
           RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                     "MsgEncodeBody - hMsg 0x%p - encode of body failed. Free temp page 0x%p on pool 0x%p. ",
                     msg, hTempPage, hPool));
           return stat;
        }
        *length += tempLength;

    }
#else /* #ifndef RV_SIP_PRIMITIVES */
    /* message-Body */
    if(msg->contentLength > 0)
    {
        if (msg->strBody > UNDEFINED)
        {
            stat = MsgUtilsEncodeString(msg->pMsgMgr,
                                        hPool,
                                        hPage,
                                        msg->hPool,
                                        msg->hPage,
                                        msg->strBody,
                                        length);
            if(stat != RV_OK)
            {
               RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                    "MsgEncodeBody - hMsg 0x%p. encode of body failed",
                    msg));
                return stat;
            }
        }
    }
#endif /* #ifndef RV_SIP_PRIMITIVES */

    if(stat != RV_OK)
    {
        if (NULL_PAGE != hTempPage)
        {
            RPOOL_FreePage(hPool, hTempPage);
        }
        RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "MsgEncodeBody - hMsg 0x%p. function failed to encode CRLF. Free temp page 0x%p on pool 0x%p.",
                msg, hTempPage, hPool));
    }
    else
    {
        if (NULL_PAGE != hTempPage)
        {
            RvLogInfo(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "MsgEncodeBody - Message 0x%p. tempPage 0x%p on pool 0x%p was free",
                msg, hTempPage, hPool));
            RPOOL_FreePage(hPool, hTempPage);
        }
        RvLogInfo(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "MsgEncodeBody - Message 0x%p. body was encoded successfully",
                msg));
    }
    return stat;
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * GetContentTypeStringLength
 * ------------------------------------------------------------------------
 * General: Gets the length of the Content-Type textual header  (when it is
 *          encoded).
 * Return Value:  The length of the Content-Type header.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  pSipMsg - A pointer to the message object.
 ***************************************************************************/
static RvUint GetContentTypeStringLength(IN MsgMessage *pSipMsg)
{
    RvSipContentTypeHeaderHandle hContentType;
    RvChar                      dummy;
    RvUint                      length;
    RvStatus                    stat;

    if (NULL == pSipMsg->pBodyObject)
    {
        return 0;
    }
    hContentType = (RvSipContentTypeHeaderHandle)pSipMsg->pBodyObject->pContentType;
    if (NULL == hContentType)
    {
        return 0;
    }
    /* Encode the Content-Type header object to a dummy buffer. The buffer length
       that is given to the function as a parameter is 0. This way, the encoding
       will fail (RV_ERROR_INSUFFICIENT_BUFFER) but the actual length will be returned
       in length */
    stat = ContentTypeHeaderConsecutiveEncode(hContentType, &dummy,
                                              0, &length);
    if ((stat != RV_OK) &&
        (stat != RV_ERROR_INSUFFICIENT_BUFFER))
    {
        /* The function failed for reason other than insufficient buffer */
        RvLogError(pSipMsg->pMsgMgr->pLogSrc,(pSipMsg->pMsgMgr->pLogSrc,
                  "GetContentTypeStringLength: Failed to encode Content-Type header. stat is %d",
                  stat));
        return 0;
    }

    return length+1;
}

/***************************************************************************
 * GetBodyStringLength
 * ------------------------------------------------------------------------
 * General: Gets the length of the textual message body  (when it is
 *          encoded).
 * Return Value:  The length of the message body object.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  pSipMsg - A pointer to the message object.
 ***************************************************************************/
static RvUint GetBodyStringLength(IN MsgMessage *pSipMsg)
{
    RvStatus                    stat;
    HPAGE                        hTempPage;
    RvUint32                    length;

    if (NULL == pSipMsg->pBodyObject)
        return 0;

    if (NULL == pSipMsg->pBodyObject->hBodyPartsList)
    {
        if (UNDEFINED == pSipMsg->pBodyObject->strBody)
        {
            return 0;
        }
        else
        {
            return pSipMsg->pBodyObject->length+1;
        }
    }

    /* Create boundary if needed */
    stat = BodyUpdateContentType((RvSipBodyHandle)pSipMsg->pBodyObject);
    if (RV_OK != stat)
        return 0;
    /* Encode the message body object on a temporary page to compute the length
       of the body object */
    stat = RvSipBodyEncode((RvSipBodyHandle)pSipMsg->pBodyObject,
                              pSipMsg->pBodyObject->hPool, &hTempPage, &length);
    if (RV_OK != stat)
        return 0;

    RPOOL_FreePage(pSipMsg->hPool, hTempPage);
    return length+1;
}

#endif /* #ifndef RV_SIP_PRIMITIVES */

/***************************************************************************
 * sortHeaderList
 * ------------------------------------------------------------------------
 * General: Sorts the header list. This function sorts the Allow headers together and the
 *            other headers according to each header name.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: msg       - Pointer to the message object.
 ***************************************************************************/
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
RVAPI  RV_Status sortHeaderList(IN MsgMessage* msg)
#else
static RvStatus sortHeaderList(IN MsgMessage* msg)
#endif
/* SPIRENT_END */
{
    RvSipHeaderListElemHandle hTmpElement;
    RvSipHeaderListElemHandle hPrevElement;
    RvSipHeaderListElemHandle hAllowLastElement;
    RvSipHeaderListElemHandle hAllowEventsLastElement;
    RvSipHeaderListElemHandle hOtherFirstElement, hOtherLastElement;
    RvSipHeaderType           type;
    RvStatus                  status;
    void                     *returnedHeader;
    RvInt32                  safeCounter = 0;

    hTmpElement                    = NULL;
    hPrevElement                = NULL;
    hAllowLastElement            = NULL;
    hAllowEventsLastElement     = NULL;
    hOtherFirstElement          = NULL;
    hOtherLastElement            = NULL;
    status                        = RV_OK;

    /* getting first header from source list */
    returnedHeader = MsgGetHeaderFromList((RvSipMsgHandle)msg,
                                       RVSIP_FIRST_HEADER,
                                       &hTmpElement,
                                       &type);

    while(returnedHeader != NULL && safeCounter < MAX_HEADERS_PER_MESSAGE)
    {
        safeCounter++;
        switch(type)
        {
#ifndef RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_ALLOW:
            if(hAllowLastElement != NULL) /* this is not the first allow header in the list */
            {
                if(hPrevElement == hAllowLastElement)
                {
                    hAllowLastElement = hTmpElement;
                }
                else
                {
                    status = RvSipMsgRemoveHeaderAt((RvSipMsgHandle)msg, hTmpElement);
                    if(status != RV_OK)
                    {
                        return status;
                    }
                    status = RvSipMsgPushHeader((RvSipMsgHandle)msg,
                                                RVSIP_NEXT_HEADER,
                                                returnedHeader,
                                                RVSIP_HEADERTYPE_ALLOW,
                                                &hAllowLastElement,
                                                &returnedHeader);
                    if(status != RV_OK)
                    {
                        return status;
                    }
                }
            }
            else
            {
                hAllowLastElement = hTmpElement;
            }
            break;
       case RVSIP_HEADERTYPE_ALLOW_EVENTS:
            if(hAllowEventsLastElement != NULL) /* this is not the first allow header in the list */
            {
                if(hPrevElement == hAllowEventsLastElement)
                {
                    hAllowEventsLastElement = hTmpElement;
                }
                else
                {
                    status = RvSipMsgRemoveHeaderAt((RvSipMsgHandle)msg, hTmpElement);
                    if(status != RV_OK)
                    {
                        return status;
                    }
                    status = RvSipMsgPushHeader((RvSipMsgHandle)msg,
                                                RVSIP_NEXT_HEADER,
                                                returnedHeader,
                                                RVSIP_HEADERTYPE_ALLOW_EVENTS,
                                                &hAllowEventsLastElement,
                                                &returnedHeader);
                    if(status != RV_OK)
                    {
                        return status;
                    }
                }
            }
            else
            {
                hAllowEventsLastElement = hTmpElement;
            }
            break;
#endif /* #ifndef RV_SIP_PRIMITIVES */
        case RVSIP_HEADERTYPE_OTHER:
            if(hOtherLastElement != NULL)
            {
                if(hPrevElement == hOtherLastElement)
                {
                    hOtherLastElement = hTmpElement;
                }
                else
                {
                    status = RvSipMsgRemoveHeaderAt((RvSipMsgHandle)msg, hTmpElement);
                    if(status != RV_OK)
                    {
                        return status;
                    }
                    status = RvSipMsgPushHeader((RvSipMsgHandle)msg,
                                                RVSIP_NEXT_HEADER,
                                                returnedHeader,
                                                RVSIP_HEADERTYPE_OTHER,
                                                &hOtherLastElement,
                                                &returnedHeader);
                    if(status != RV_OK)
                    {
                        return status;
                    }
                }
            }
            else
            {
                hOtherFirstElement = hTmpElement;
                hOtherLastElement  = hTmpElement;
            }
            break;
        default:
            break;
        }
        hPrevElement = hTmpElement;
        /* taking out next header form header list */
        returnedHeader = MsgGetHeaderFromList((RvSipMsgHandle)msg,
                                               RVSIP_NEXT_HEADER,
                                               &hTmpElement,
                                               &type);
    }
    if(safeCounter >= MAX_HEADERS_PER_MESSAGE)
    {
        RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
            "sortHeaderList: msg 0x%p - too many headers in list (more than %d) - failed to sort all", 
            msg, MAX_HEADERS_PER_MESSAGE));
    }

    if((hOtherFirstElement != NULL) || (hOtherFirstElement != hOtherLastElement))
    {
        /* there is more then one other header */
        status = sortOtherHeadersList((RvSipMsgHandle)msg, hOtherFirstElement, hOtherLastElement);
    }
    return status;
}

/***************************************************************************
 * sortOtherHeadersList
 * ------------------------------------------------------------------------
 * General: Sorts the other header list by headers name.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hMsg                 - Handle to the message object.
 *           hOtherFirstHeader - Handle to the first other header in the headers list.
 *           hOtherLastHeader  - Handle to the last other header in the headers list.
 ***************************************************************************/
static RvStatus sortOtherHeadersList(IN RvSipMsgHandle            hMsg,
                                     IN RvSipHeaderListElemHandle hOtherFirstHeader,
                                     IN RvSipHeaderListElemHandle hOtherLastHeader)
{
    listElem                  listArr[MAX_HEADERS_PER_MESSAGE];
    RvSipHeaderListElemHandle hTmpElement;
    void                     *returnedHeader, *pNewHeader;
    MsgMessage               *msg;
    RvInt32                   counter, i, safeCounter = 0;
    RvUint                    actLen;
    RvChar                    name[MAX_HEADER_NAME_LEN];
    RvStatus                  status;

    RV_UNUSED_ARG(hOtherLastHeader);
    status          = RV_OK;
    hTmpElement     = NULL;
    returnedHeader  = NULL;
    pNewHeader      = NULL;
    counter         = 0;

    memset((void*)&listArr, 0, sizeof(listArr));

    msg = (MsgMessage*)hMsg;

    returnedHeader = MsgGetHeaderByTypeFromList(hMsg,
                                                RVSIP_HEADERTYPE_OTHER,
                                                RVSIP_FIRST_HEADER,
                                                &hTmpElement);

    if(returnedHeader == NULL || hTmpElement != hOtherFirstHeader)
    {
        /* bug - this function is called only if there were at least 2 other headers*/
        RvLogExcep(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
            "sortOtherHeadersList: did not find first other header, or found header different than hOtherFirstHeader"));
    }

    while(returnedHeader != NULL && safeCounter < MAX_HEADERS_PER_MESSAGE)
    {
        safeCounter++;
        status = RvSipOtherHeaderGetName((RvSipOtherHeaderHandle)returnedHeader, name, 50, &actLen);
        if(status == RV_OK) /* other header has a name */
        {
            for(i = 0 ; i < counter ; i++)
            {
                /* search for same header name in list os other headers that were already checked */
                if(SipCommonStricmp(name, listArr[i].name) == RV_TRUE)
                {
                    status = RvSipMsgRemoveHeaderAt(hMsg, hTmpElement);
                    if(status != RV_OK)
                    {
                        return status;
                    }
                    status = RvSipMsgPushHeader(hMsg,
                                                RVSIP_NEXT_HEADER,
                                                returnedHeader,
                                                RVSIP_HEADERTYPE_OTHER,
                                                &listArr[i].hListElement,
                                                &pNewHeader);
                    if(status != RV_OK)
                    {
                        return status;
                    }
                    break;
                }
            }
            if(i == counter)
            {
                listArr[counter].hListElement = hTmpElement;
                strcpy(listArr[counter].name, name);
                counter++;
            }
        }
        /*else - header with no name - may stay at it's current location */
         returnedHeader = MsgGetHeaderByTypeFromList(hMsg,
                                                RVSIP_HEADERTYPE_OTHER,
                                                RVSIP_NEXT_HEADER,
                                                &hTmpElement);
    } /* while */

    if(safeCounter+1 >= MAX_HEADERS_PER_MESSAGE)
    {
        RvLogWarning(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
            "sortOtherHeadersList: hMsg=0x%p - Too many other headers. (more than %d). Failed to sort all",
            hMsg, MAX_HEADERS_PER_MESSAGE));
    }

    return RV_OK;
}


/***************************************************************************
 * MsgGetHeaderFromList
 * ------------------------------------------------------------------------
 * General: Gets next header from the header list.
 * Return Value: Returns the header handle as void*, or NULL if there is no header
 *               to retrieve.
 * ------------------------------------------------------------------------
 * Arguments:
 *  input:
 *        hSipMsg   - Handle of the message object.
 *      location  - The location on list-next, previous, first or last.
 *      hListElem - Handle to the current position in the list. Supply this
 *                  value if you chose next or previous in the location parameter.
 *                  This is also an output parameter and will be set with a link
 *                  to requested header in the list.
 *  output:
 *      hListElem - Handle to the current position in the list. Supply this
 *                  value if you chose next or previous in the location parameter.
 *                  This is also an output parameter and will be set with a link
 *                  to requested header in the list.
 *        pHeaderType  - The type of the retrieved header.
 ***************************************************************************/
static void* RVCALLCONV MsgGetHeaderFromList(
                                     IN    RvSipMsgHandle                hMsg,
                                     IN    RvSipHeadersLocation          location,
                                     INOUT RvSipHeaderListElemHandle*    hListElem,
                                     OUT   RvSipHeaderType*              pHeaderType)
{
    RLIST_ITEM_HANDLE   listElement;
    MsgMessage*         msg = (MsgMessage*)hMsg;

    if (msg->headerList == NULL)
    {
        *hListElem = NULL;
        return NULL;
    }
    switch(location)
    {
        case RVSIP_FIRST_HEADER:
        {
            RLIST_GetHead (NULL, msg->headerList, &listElement);
            break;
        }
        case RVSIP_LAST_HEADER:
        {
            RLIST_GetTail (NULL, msg->headerList, &listElement);
            break;
        }
        case RVSIP_NEXT_HEADER:
        {
            RLIST_GetNext (NULL,
                           msg->headerList,
                           (RLIST_ITEM_HANDLE)*hListElem,
                           &listElement);
            break;

        }
        case RVSIP_PREV_HEADER:
        {
            RLIST_GetPrev (NULL,
                           msg->headerList,
                           (RLIST_ITEM_HANDLE)*hListElem,
                           &listElement);
            break;
        }
        default:
            return NULL;
    }

    if(listElement == NULL)
        return NULL;
    else
    {
        *pHeaderType = ((MsgHeaderListElem*)listElement)->headerType;
        *hListElem = (RvSipHeaderListElemHandle)listElement;
        return ((MsgHeaderListElem*)listElement)->hHeader;
    }

}

/***************************************************************************
 * MsgGetHeaderByTypeFromList
 * ------------------------------------------------------------------------
 * General: Gets a header by type from the header list.
 * Return Value:Returns the header handle as void*, or NULL if there is no
 *               header to retrieve.
 * ------------------------------------------------------------------------
 * Arguments:
 *  input:
 *        hSipMsg     - Handle of the message object.
 *        eHeaderType - The header type to be retrieved.
 *      location    - The location on list-next, previous, first or last.
 *      hListElem - Handle to the current position in the list. Supply this
 *                  value if you chose next or previous in the location parameter.
 *                  This is also an output parameter and will be set with a link
 *                  to requested header in the list.
 *  output:
 *      hListElem - Handle to the current position in the list. Supply this
 *                  value if you chose next or previous in the location parameter.
 *                  This is also an output parameter and will be set with a link
 *                  to requested header in the list.
 ***************************************************************************/
static void* RVCALLCONV MsgGetHeaderByTypeFromList(
                                         IN    RvSipMsgHandle             hSipMsg,
                                         IN    RvSipHeaderType            eHeaderType,
                                         IN    RvSipHeadersLocation       location,
                                         INOUT RvSipHeaderListElemHandle* hListElem)
{
    void*               requestedHeader;
    RvSipHeaderType     type = RVSIP_HEADERTYPE_UNDEFINED;
    RvSipHeadersLocation   internalLocation;
    RvInt              safeCounter = 0;

    /* first we will get the current header (according to the location argument)
       if it's headerType is correct, then wev'e finished.
       else we will continue searching forward or backward on the list */

    requestedHeader = MsgGetHeaderFromList(hSipMsg, location, hListElem, &type);

    if(requestedHeader == NULL)
    {
        return NULL;
    }
    if(type == eHeaderType)
    {
        /* we found the requested header */
        return requestedHeader;
    }
    else
    {
        /* we will continue the searching from the retrieved hListElem */

        /* decision whether to search forward or backward */
        switch(location)
        {
            case RVSIP_FIRST_HEADER:
            case RVSIP_NEXT_HEADER:
                internalLocation = RVSIP_NEXT_HEADER;
                break;
            case RVSIP_PREV_HEADER:
            case RVSIP_LAST_HEADER:
                internalLocation = RVSIP_PREV_HEADER;
                break;
            default:
                return NULL;
        }

        while(requestedHeader != NULL && safeCounter < MAX_HEADERS_PER_MESSAGE)
        {
            requestedHeader = MsgGetHeaderFromList(hSipMsg, internalLocation, hListElem, &type);
            if(requestedHeader == NULL)
            {
                return NULL;
            }
            if(type == eHeaderType)
            {
                /* we found the requested header */
                return requestedHeader;
            }
            ++safeCounter;
        }
    }/* else - continue searching */

    return NULL;
}
/***************************************************************************
 * MsgGetHeaderByNameFromList
 * ------------------------------------------------------------------------
 * General: Gets a header by name from the header list.
 *          The message object holds most headers in a sequential list except,
 *          To, From, CallId, Cseq, ContentLength and ContentType headers.
 *          This function should be used only for headers of type
 *          RVSIP_HEADERTYPE_OTHER.
 * Return Value: Returns the Other header handle, or NULL if no Other header
 *                handle with the same name exists.
 * ------------------------------------------------------------------------
 * Arguments:
 *  input:
 *        hSipMsg   - Handle of the message object.
 *        strName   - The header name to be retrieved.
 *      location    - The location on list-next, previous, first or last.
 *      hListElem - Handle to the current position in the list. Supply this
 *                  value if you chose next or previous in the location parameter.
 *                  This is also an output parameter and will be set with a link
 *                  to requested header in the list.
 *  output:
 *      hListElem - Handle to the current position in the list. Supply this
 *                  value if you chose next or previous in the location parameter.
 *                  This is also an output parameter and will be set with a link
 *                  to requested header in the list.
 ***************************************************************************/
static RvSipOtherHeaderHandle RVCALLCONV MsgGetHeaderByNameFromList
                                            (IN    RvSipMsgHandle             hSipMsg,
                                             IN    RvChar*                   strName,
                                             IN    RvSipHeadersLocation       location,
                                             INOUT RvSipHeaderListElemHandle* hListElem)
{
    void*                  requestedHeader;
    RvSipHeadersLocation   internalLocation;
    RvBool                cont;
    RvInt                 safeCounter = 0;

    /* first we will get the current header (according to the location argument)
       if it's headerType is correct, then we have finished.
       else we will continue searching forward or backward on the list */

    requestedHeader = MsgGetHeaderByTypeFromList(hSipMsg,
                                              RVSIP_HEADERTYPE_OTHER,
                                              location,
                                              hListElem);

    if((requestedHeader != NULL))
    {
        if(RPOOL_CmpToExternal( (((MsgOtherHeader*)requestedHeader)->hPool),
                                (((MsgOtherHeader*)requestedHeader)->hPage),
                                (((MsgOtherHeader*)requestedHeader)->strHeaderName),
                                strName,
                                RV_TRUE) == RV_TRUE)
        {
            /* we found the requested header */
            return (RvSipOtherHeaderHandle)requestedHeader;
        }
        else
        {
            cont = RV_TRUE;

            /* decision whether to search forward or backward */
            switch(location)
            {
                case RVSIP_FIRST_HEADER:
                case RVSIP_NEXT_HEADER:
                    internalLocation = RVSIP_NEXT_HEADER;
                    break;
                case RVSIP_PREV_HEADER:
                case RVSIP_LAST_HEADER:
                    internalLocation = RVSIP_PREV_HEADER;
                    break;
                default:
                    return NULL;
            }

            while ((cont == RV_TRUE) && (safeCounter < MAX_HEADERS_PER_MESSAGE))
            {
                ++safeCounter;

                requestedHeader = MsgGetHeaderByTypeFromList(hSipMsg,
                                                          RVSIP_HEADERTYPE_OTHER,
                                                          internalLocation,
                                                          hListElem);
                if((requestedHeader != NULL))
                {
                    if(RPOOL_CmpToExternal((((MsgOtherHeader*)requestedHeader)->hPool),
                                           (((MsgOtherHeader*)requestedHeader)->hPage),
                                           (((MsgOtherHeader*)requestedHeader)->strHeaderName),
                                            strName,
                                            RV_TRUE) == RV_TRUE)
                    {
                        /* we found the requested header */
                        cont = RV_FALSE;
                    }
                }
                if (requestedHeader==NULL)
                    {
                        cont = RV_FALSE;
                    }
            }
        }
    }
    return (RvSipOtherHeaderHandle)requestedHeader;

}

/***************************************************************************
 * MsgClean
 * ------------------------------------------------------------------------
 * General: Clean the object structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *  pMsg        - Pointer to the message object to clean.
 *  bCleanBS    - Clean the bad-syntax parameters or not.
 ***************************************************************************/
static void MsgClean( IN MsgMessage* pMsg,
                      IN RvBool      bCleanBS)
{
    pMsg->hCSeqHeader       = NULL;
    pMsg->hFromHeader       = NULL;
    pMsg->hToHeader         = NULL;
    pMsg->hCallIdHeader     = NULL;

#ifndef RV_SIP_PRIMITIVES
    pMsg->pBodyObject       = NULL;
#else /* #ifndef RV_SIP_PRIMITIVES */
    pMsg->strBody           = UNDEFINED;
    pMsg->strContentType    = UNDEFINED;
#endif  /* #ifndef RV_SIP_PRIMITIVES */

    pMsg->headerList        = NULL;
    pMsg->contentLength     = 0;
    pMsg->bCompContentLen   = RV_FALSE;
    pMsg->bIsRequest        = UNDEFINED;
    pMsg->bForceCompactForm = RV_FALSE;
#ifdef SIP_DEBUG
    pMsg->pBody             = NULL;
    pMsg->pContentType      = NULL;
#endif /* SIP_DEBUG */

#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT
	pMsg->terminationCounter = 0;
#endif /* #ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */

    if(bCleanBS == RV_TRUE)
    {
        pMsg->strBadSyntaxStartLine = UNDEFINED;
        pMsg->strBadSyntaxReasonPhrase = UNDEFINED;
        pMsg->bBadSyntaxContentLength = RV_FALSE;
#ifdef SIP_DEBUG
        pMsg->pBadSyntaxStartLine = NULL;
        pMsg->pBadSyntaxReasonPhrase = NULL;
#endif
    }
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * MsgSetRpoolBody
 * ------------------------------------------------------------------------
 * General: Sets a rpool body into a message
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *  pMsg        - Pointer to the message object to clean.
 *  pRpoolPtr   - Rpool pointer to the set body.
 ***************************************************************************/
static RvStatus MsgSetRpoolBody(IN MsgMessage *pMsg,
                                IN RPOOL_Ptr  *pRpoolPtr)
{

    RvInt32  rpoolSize = RPOOL_GetSize(pRpoolPtr->hPool,pRpoolPtr->hPage);
    RvStatus rv        = RV_OK;

#ifndef RV_SIP_PRIMITIVES
    if (0 == rpoolSize && NULL != pMsg->pBodyObject)
    {
        pMsg->pBodyObject->strBody        = UNDEFINED;
        pMsg->pBodyObject->hBodyPartsList = NULL;
        pMsg->pBodyObject->length         = 0;
#ifdef SIP_DEBUG
        pMsg->pBodyObject->pBody          = NULL;
#endif /* SIP_DEBUG */
    }
    else if (0 < rpoolSize)
    {
        RvSipBodyHandle hTempBody = NULL;

        if (NULL == pMsg->pBodyObject)
        {
            rv = RvSipBodyConstructInMsg((RvSipMsgHandle)pMsg, &hTempBody);
            if (RV_OK != rv)
            {
                return rv;
            }
        }
        rv = RvSipBodySetRpoolString((RvSipBodyHandle)pMsg->pBodyObject,pRpoolPtr,rpoolSize);
        if(rv != RV_OK)
        {
            RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                "MsgSetRpoolBody - RvSipBodySetRpoolString failed for msg 0x%p (rv=%d)",pMsg,rv));
            return rv;

        }
    }
#else /* #ifndef RV_SIP_PRIMITIVES */
    if (rpoolSize == 0)
    {
        pMsg->strBody       = UNDEFINED;
#ifdef SIP_DEBUG
        pMsg->pBody         = NULL;
#endif /* SIP_DEBUG */
    }
    else
    {
        RvInt32 destOffset = 0;

        rv = RPOOL_Append(pMsg->hPool,pMsg->hPage,rpoolSize,RV_TRUE,&destOffset);
        if (RV_OK != rv)
        {
            RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                "MsgSetRpoolBody - RPOOL_Append failed for msg 0x%p (rv=%d)",pMsg,rv));
            return rv;
        }
        rv = RPOOL_CopyFrom(pMsg->hPool,
                            pMsg->hPage,
                            destOffset,
                            pRpoolPtr->hPool,
                            pRpoolPtr->hPage,
                            pRpoolPtr->offset,
                            rpoolSize);
        if(RV_OK != rv)
        {
            RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                "MsgSetRpoolBody - RPOOL_CopyFrom failed for msg 0x%p (rv=%d)",pMsg,rv));
            return rv;
        }
        else
        {
            pMsg->strBody = destOffset;
#ifdef SIP_DEBUG
            pMsg->pBody   = (RvChar *)RPOOL_GetPtr(pMsg->hPool, pMsg->hPage, pMsg->strBody);
#endif /* SIP_DEBUG */
        }
    }
#endif /* #ifndef RV_SIP_PRIMITIVES */

    pMsg->contentLength = rpoolSize;

    return RV_OK;
}

#endif /*#ifndef RV_SIP_PRIMITIVES*/


#ifdef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * MsgGetNumOfHeaders
 * ------------------------------------------------------------------------
 * General: This function returns the total number of headers that are currently
 *          held by the given message. This includes specific headers such as
 *          To and CSeq headers, and general headers such as Other headers.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg          - A handle to the message object.
 * Output:  pNumOfHeaders - The number of the headers held by this message.
 ***************************************************************************/
static RvStatus RVCALLCONV MsgGetNumOfHeaders(
									   IN  RvSipMsgHandle     hMsg,
									   OUT RvUint32*          pNumOfHeaders)
{
	MsgMessage* pMsg = (MsgMessage*)hMsg;
	RvUint32    numOfHeadersInList;
	RvStatus    rv;

	*pNumOfHeaders = 0; 

	if (pMsg->hCallIdHeader != NULL)
	{
		*pNumOfHeaders += 1;
	}
	if (pMsg->hCSeqHeader != NULL)
	{
		*pNumOfHeaders += 1;
	}
	if (pMsg->hFromHeader != NULL)
	{
		*pNumOfHeaders += 1;
	}
	if (pMsg->hToHeader != NULL)
	{
		*pNumOfHeaders += 1;
	}
	if (pMsg->contentLength != UNDEFINED)
	{
		*pNumOfHeaders += 1;
	}
	if (pMsg->pBodyObject != NULL)
	{
		RvSipContentTypeHeaderHandle hContentType;
		hContentType = RvSipBodyGetContentType((RvSipBodyHandle)pMsg->pBodyObject);
		if (hContentType != NULL)
		{
			*pNumOfHeaders += 1;
		}
	}

	rv = RLIST_GetNumOfElements(NULL, pMsg->headerList, &numOfHeadersInList);
	if (RV_OK != rv)
	{
		RvLogError(pMsg->pMsgMgr->pLogSrc, (pMsg->pMsgMgr->pLogSrc,
			"MsgGetNumOfHeaders - Failed to get num of headers in list from message 0x%p",
			hMsg));	
		return rv;
	}

	*pNumOfHeaders += numOfHeadersInList;

	return RV_OK;
}
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
RVAPI RvStatus RVCALLCONV RvSipMsgEncode(IN  RvSipMsgHandle hSipMsg,
                                          IN  HRPOOL         hPool,
                                          OUT HPAGE*         hPage,
                                          OUT RvUint32*     length)
{
    SPIRENT_RvSipMsgEncode_Type f_Encoder;
    SPIRENT_Encoder_Cfg_Type Enc_cfg;
    RvStatus retval;
    RvSipAddressHandle  hRequestUri = NULL;
    RvSipMsgType		eMsgType                = RVSIP_MSG_UNDEFINED;
    MsgMessage*         msg = (MsgMessage*)hSipMsg;
    
    /* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    unsigned int                 actual_len = 0;
#else
    int                 actual_len = 0;
#endif
    /* SPIRENT_END */

    char                buff[100]; // should be enough for any maddr
    // Store maddr-param from RequestUri
    eMsgType = RvSipMsgGetMsgType(hSipMsg);
    if (eMsgType == RVSIP_MSG_REQUEST)
    {
        hRequestUri = RvSipMsgGetRequestUri(hSipMsg);
        if (hRequestUri == NULL)
        {

			   RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
				   "RvSipMsgEncode - failed to get RequestUri"));
            return RV_ERROR_UNKNOWN;
        }
        retval = RvSipAddrUrlGetMaddrParam(hRequestUri,buff,sizeof(buff),(RvUint*)&actual_len);  // remember maddr param (if any)
        if ( retval == RV_OK )
        {
            RvSipAddrUrlSetMaddrParam(hRequestUri, NULL);  // remove maddr param (if any)
        }
        else
        {
            actual_len = 0;
        }
    }

    f_Encoder = RvSipMsgGetExternalEncoder ( hSipMsg, &Enc_cfg );
    if ( f_Encoder )
    {
       retval = f_Encoder (  hSipMsg,  hPool, hPage, length );
    }
    else
    {
       retval = RADVISION_RvSipMsgEncode ( hSipMsg,  hPool, hPage, length );       
    }

    if ( actual_len )
    {
        retval = RvSipAddrUrlSetMaddrParam(hRequestUri,buff);  // restore maddr param (if any)
    }

    return retval;
}
#endif
/* SPIRENT_END */

/* SPIRENT_BEGIN */
/*
* COMMENTS:
* Modified by Armyakov
* Counting raw messages 
*/
#if defined(UPDATED_BY_SPIRENT)

RVAPI void RVCALLCONV RvSipRawMessageCounterHandlersSet(
                                    IN RvSipRawMessageCounterHandlers* hCounterHandlers)
{
   if ( NULL == hCounterHandlers ) {
      SIP_SipRawMsgCntrHandlers.pfnIncoming = NULL;  
      SIP_SipRawMsgCntrHandlers.pfnOutgoing = NULL;
      SIP_SipRawMsgCntrHandlers.pfnPerChanIncoming = NULL;  
      SIP_SipRawMsgCntrHandlers.pfnPerChanOutgoing = NULL;
   } else {
      SIP_SipRawMsgCntrHandlers.pfnIncoming = hCounterHandlers->pfnIncoming;  
      SIP_SipRawMsgCntrHandlers.pfnOutgoing = hCounterHandlers->pfnOutgoing;
      SIP_SipRawMsgCntrHandlers.pfnPerChanIncoming = hCounterHandlers->pfnPerChanIncoming;  
      SIP_SipRawMsgCntrHandlers.pfnPerChanOutgoing = hCounterHandlers->pfnPerChanOutgoing;
   }
}

RVAPI void RVCALLCONV RvSipRawMessageCounterHandlersRun(
                                    IN RvSipRawMessageDirection eDirection)
{
    switch ( eDirection )
    {
        case SPIRENT_SIP_RAW_MESSAGE_INCOMING:
            if ( NULL != SIP_SipRawMsgCntrHandlers.pfnIncoming )
            {
                SIP_SipRawMsgCntrHandlers.pfnIncoming();
            }
            break;
        case SPIRENT_SIP_RAW_MESSAGE_OUTGOING:
            if ( NULL != SIP_SipRawMsgCntrHandlers.pfnOutgoing )
            {
                SIP_SipRawMsgCntrHandlers.pfnOutgoing();
            }
            break;
    }
}

RVAPI void RVCALLCONV RvSipRawMessageCounterPerChanHandlersRun(
                                    IN RvSipRawMessageDirection eDirection,
                                    IN int cctContext)
{
    switch ( eDirection )
    {
        case SPIRENT_SIP_RAW_MESSAGE_INCOMING:
            if ( NULL != SIP_SipRawMsgCntrHandlers.pfnPerChanIncoming )
            {
                SIP_SipRawMsgCntrHandlers.pfnPerChanIncoming(cctContext);
            }
            break;
        case SPIRENT_SIP_RAW_MESSAGE_OUTGOING:
            if ( NULL != SIP_SipRawMsgCntrHandlers.pfnPerChanOutgoing )
            {
                SIP_SipRawMsgCntrHandlers.pfnPerChanOutgoing(cctContext);
            }
            break;
	}
}
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */


#ifdef __cplusplus
}
#endif



