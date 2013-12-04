/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        _SipMsg.c                                           *
 *                                                                            *
 * The file defines the internal methods of the Message object.               *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra                                                                  *
 ******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#include "_SipMsg.h"
#include "_SipMsgMgr.h"
#include "_SipOtherHeader.h"
#include "RvSipMsgTypes.h"
#include "MsgTypes.h"
#include "MsgUtils.h"

#include "RvSipOtherHeader.h"
#include "rvmemory.h"

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/
#define SIGCOMP_STR "sigcomp"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

    /****************************************************/
    /*        CONSTRUCTORS AND DESTRUCTORS                */
    /****************************************************/

/***************************************************************************
 * SipMessageMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct a new MsgMgr.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: msgMgrCfg - The message manager configuration parameters.
 * Output: hMsgMgr - Handle of the constructed msg manager.
 ***************************************************************************/
RvStatus SipMessageMgrConstruct(IN  SipMessageMgrCfg *msgMgrCfg,
                                 OUT RvSipMsgMgrHandle *phMsgMgr)
{
    MsgMgr            *pMsgMgr;
    if (NULL == phMsgMgr)
    {
        return RV_ERROR_NULLPTR;
    }
    /* Allocate a new message manager object */
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(MsgMgr), msgMgrCfg->pLogMgr, (void*)&pMsgMgr))
    {
        RvLogError(msgMgrCfg->logId,(msgMgrCfg->logId,
                  "SipMessageMgrConstruct - Error trying to construct a new Message manager"));
        return RV_ERROR_OUTOFRESOURCES;
    }
    pMsgMgr->pLogMgr = msgMgrCfg->pLogMgr;
    pMsgMgr->pLogSrc = msgMgrCfg->logId;
    pMsgMgr->seed     = msgMgrCfg->seed;
    pMsgMgr->hParserMgr = msgMgrCfg->hParserMgr;

    RvLogInfo(msgMgrCfg->logId,(msgMgrCfg->logId,
              "SipMessageMgrConstruct - Message Manager was constructed successfully."));


    *phMsgMgr = (RvSipMsgMgrHandle)pMsgMgr;
    return RV_OK;
}

/***************************************************************************
 * SipMsgMgrSetTransportHandle
 * ------------------------------------------------------------------------
 * General: Sets the transport manager in message manager.
 * Return Value: RV_OK
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hMsgMgr       - The message manager.
 *        hTransportMgr - The transport manager handle.
 ***************************************************************************/
RvStatus SipMsgMgrSetTransportHandle(IN RvSipMsgMgrHandle       hMsgMgr,
                                            IN RvSipTransportMgrHandle hTransportMgr)
{
    MsgMgr *pMsgMgr = (MsgMgr*)hMsgMgr;
    if (NULL == hMsgMgr)
    {
        return RV_ERROR_NULLPTR;
    }

    pMsgMgr->hTransportMgr = hTransportMgr;
    return RV_OK;
}

/***************************************************************************
 * SipMessageMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destruct all paramaters of message module.
 *          (hHeadersPool
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsgMgr    - Handle of the message manager.
 ***************************************************************************/
void SipMessageMgrDestruct (IN RvSipMsgMgrHandle hMsgMgr)
{
    MsgMgr* pMsgMgr = (MsgMgr*)hMsgMgr;

    if(pMsgMgr == NULL)
    {
        return;
    }
    RvLogInfo(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "SipMessageMgrDestruct - Message Manager was destructed."));
    RvMemoryFree(pMsgMgr, pMsgMgr->pLogMgr);

}

/***************************************************************************
 * SipMsgParse
 * ------------------------------------------------------------------------
 * General:This function is used to parse a SIP text message into a message
 *         object. You must construct a message object before using this
 *         function.
 * Return Value:RV_OK, RV_ERROR_OUTOFRESOURCES,RV_ERROR_NULLPTR,
 *                RV_ERROR_ILLEGAL_SYNTAX,RV_ERROR_ILLEGAL_SYNTAX.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipMsg   - Handle to the message object.
 *  buffer    - A buffer with a SIP text message.
 *  bufferLen - The length of the buffer including the CRLF at the end.
 ***************************************************************************/
RvStatus RVCALLCONV SipMsgParse(IN    RvSipMsgHandle hSipMsg,
                                 IN    RvChar*       buffer,
                                 IN    RvUint32      bufferLen)
{
    MsgMessage* pMsg = (MsgMessage*)hSipMsg;
    if(hSipMsg == NULL || (buffer == NULL))
        return RV_ERROR_BADPARAM;
   
#define SIP_MSG_BUILDER_SIGCOMP_PREFIX_MASK 0xF8
    
    /* if this is a sigComp message, we will not parse it -
    may happen when a stack which is not compiled with SigComp receives a sigComp message... */
    
    if ((*buffer & SIP_MSG_BUILDER_SIGCOMP_PREFIX_MASK) ==
        SIP_MSG_BUILDER_SIGCOMP_PREFIX_MASK)
    {
		/* We might reach here upon : */ 
		/* 1. Receiving a valid SigComp message but the RV_SIGCOMP_ON is turned off      */ 
		/* 2. Receiving an invalid msg that includes SIP_MSG_BUILDER_SIGCOMP_PREFIX_MASK */
		/*    (for example, a buffer with SIP_MSG_BUILDER_SIGCOMP_PREFIX_MASK, preceded  */
		/*    by CRLFs). */ 
        RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
            "SipMsgParse - Can not parse a message that starts with sigcomp prefix (0xF8). Please verify: 1. that RV_SIGCOMP_ON is defined. 2. the incoming buffer validity"));
        return RV_ERROR_NOTSUPPORTED;
    }

    return MsgParserParse(pMsg->pMsgMgr, (void*)hSipMsg, SIP_PARSETYPE_MSG, buffer, bufferLen);
}

/***************************************************************************
 * SipMsgGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the message object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hMsg - The msg to take the page from.
 ***************************************************************************/
HRPOOL SipMsgGetPool(RvSipMsgHandle hMsg)
{
    return ((MsgMessage*)hMsg)->hPool;
}

/***************************************************************************
 * SipMsgGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the message object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hMsg - The msg to take the page from.
 ***************************************************************************/
HPAGE SipMsgGetPage(RvSipMsgHandle hMsg)
{
    return ((MsgMessage*)hMsg)->hPage;
}


/****************************************************/
/*        BODY FUNCTIONS                               */
/****************************************************/

/***************************************************************************
 * SipMsgGetBody
 * ------------------------------------------------------------------------
 * General: This method gets the string body of the message. You can use
 *          this function only when the mesaage body is a string and not
 *          a list of body parts.
 * Return Value: offset of sip message body string(MIME/SDP) or UNDEFINED
 *               if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:   hSipMsg - Handle of the message totake the body from.
 ***************************************************************************/
RvInt32 SipMsgGetBody(const RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;

    msg = (MsgMessage*)hSipMsg;

#ifndef RV_SIP_PRIMITIVES

    if ((NULL == msg->pBodyObject) ||
        (NULL != msg->pBodyObject->hBodyPartsList))
    {
        return UNDEFINED;
    }
    else
    {
        return msg->pBodyObject->strBody;
    }
#else
    return msg->strBody;
#endif /* #ifndef RV_SIP_PRIMITIVES */
}

/****************************************************/
/*        START LINE FUNCTIONS                         */
/****************************************************/

/***************************************************************************
 * MsgConstructRequestLine
 * ------------------------------------------------------------------------
 * General: Constructs the request line of the message.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * input:   msg - Pointer to the message structure.
 ***************************************************************************/
void RVCALLCONV MsgConstructRequestLine(IN  MsgMessage*   msg)
{
    msg->startLine.requestLine.strMethod   = UNDEFINED;
    msg->startLine.requestLine.eMethod     = RVSIP_METHOD_UNDEFINED;
    msg->startLine.requestLine.hRequestUri = NULL;
}

/***************************************************************************
 * MsgConstructStatusLine
 * ------------------------------------------------------------------------
 * General: Constructs the status line of the message.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * input:   msg - Pointer to the message structure.
 ***************************************************************************/
void RVCALLCONV MsgConstructStatusLine(IN  MsgMessage*   msg)
{
    msg->startLine.statusLine.strReasonePhrase = UNDEFINED;
    msg->startLine.statusLine.statusCode     = UNDEFINED;
#ifdef SIP_DEBUG
    msg->startLine.statusLine.pReasonePhrase = NULL;
#endif
}

/***************************************************************************
 * SipMsgGetStrRequestMethod
 * ------------------------------------------------------------------------
 * General: This method retrieves the method type in the request line.
 *          It gives the enum value and the str value. if the enum value
 *          is OTHER, then the str will contain the method, else the str
 *          value should be ignored.
 * Return Value: Offset of the method string on the message page, or
 *               UNDEFINED If the startLine is not requestLine but statusCode.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg    - Handle of a message object.
 * output: eMethod   - The enum method type value
 *         strMethod - The str method value (when the enum value is OTHE)
 ***************************************************************************/
RvInt32 SipMsgGetStrRequestMethod(IN  RvSipMsgHandle   hSipMsg)
{
    MsgMessage* msg;

    msg = (MsgMessage*)hSipMsg;

    if(msg->bIsRequest == RV_FALSE)
    {
        return UNDEFINED;
    }
    else
    {
        return msg->startLine.requestLine.strMethod;
    }
}

/***************************************************************************
 * SipMsgSetMethodInRequestLine
 * ------------------------------------------------------------------------
 * * General: This method sets the method type in the request line of the msg.
 *          it gets enum argument, and string argument. Only if eRequsetMethod
 *          is RVSIP_METHOD_OTHER, the strMethod will be set, else it will be
 *          ignored.
 *          If the string lays on a memory pool copying will be maid only if
 *          the page and pool are different than the object's page and pool.
 *          Otherwise attach will be maid.
 * Return Value: RV_OK,
 *               RV_ERROR_OUTOFRESOURCES - If didn't manage to allocate space for
 *                            setting strMethod.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hSipMsg - Handle of a message object.
 *            eRequsetMethod - requset method type to set in the message.
 *           strMethod - A textual string indicates the method type, for the case
 *                      that eRequsetMethod is RVSIP_METHOD_OTHER.
 *          hPool - The pool on which the string lays (if relevant).
 *          hPage - The page on which the string lays (if relevant).
 *          methodOffset - the offset of the method string.
 ***************************************************************************/
RvStatus SipMsgSetMethodInRequestLine(IN    RvSipMsgHandle  hSipMsg,
                                       IN    RvSipMethodType eRequsetMethod,
                                       IN    RvChar*        strMethod,
                                       IN    HRPOOL          hPool,
                                       IN    HPAGE           hPage,
                                       IN    RvInt32        methodOffset)
{
    MsgMessage*   msg;
    RvInt32      newStr;
    RvStatus     retStatus;

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

    msg->startLine.requestLine.eMethod = eRequsetMethod;

    if(eRequsetMethod == RVSIP_METHOD_OTHER)
    {
        retStatus = MsgUtilsSetString(msg->pMsgMgr, hPool, hPage, methodOffset, strMethod,
                                      msg->hPool, msg->hPage, &newStr);
        if (RV_OK != retStatus)
        {
            return retStatus;
        }
        msg->startLine.requestLine.strMethod = newStr;
#ifdef SIP_DEBUG
        msg->startLine.requestLine.pMethod =
                          (RvChar *)RPOOL_GetPtr(msg->hPool, msg->hPage,
                                       msg->startLine.requestLine.strMethod );
#endif
    }
    else
    {
        msg->startLine.requestLine.strMethod = UNDEFINED;
#ifdef SIP_DEBUG
        msg->startLine.requestLine.pMethod = NULL;
#endif
    }

    return RV_OK;
}

/***************************************************************************
 * SipMsgStatusCodeToString
 * ------------------------------------------------------------------------
 * General: This is an internal function, which returns the suitable reason pharse
 *          to a given status code number.
 * Return Value: string with the reason pharse.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: Code - Status code number.
 ***************************************************************************/
RvChar* SipMsgStatusCodeToString(RvInt16 Code)
{
    RvChar *strReasonPhrase = NULL;
    RvBool checkDefaults = RV_FALSE;

    switch( Code )
    {
        case 100: /*TRYING:*/
            strReasonPhrase = "Trying";
            break;
        case 180: /*RINGING:*/
            strReasonPhrase = "Ringing";
            break;
        case 181:/*CALL_IS_BEING_FORWARDED:*/
            strReasonPhrase = "Call Is Being Forwarded";
            break;
        case 182:/*QUEUED:*/
            strReasonPhrase = "Queued";
            break;
        case 183:/*SESSION_PROGRESS:*/
            strReasonPhrase = "Session Progress";
            break;
        case 200:/*OK:*/
            strReasonPhrase = "OK";
            break;
        case 202: /*ACCEPTED*/
            strReasonPhrase = "Accepted";
            break;
        case 300:/*MULTIPLE_CHOICES:*/
            strReasonPhrase = "Multiple Choices";
            break;
        case 301:/*MOVED_PERMANENTLY:*/
            strReasonPhrase = "Moved Permanently";
            break;
        case 302:/*MOVED_TEMPORARILY:*/
            strReasonPhrase = "Moved Temporarily";
            break;
        case 305:/*USE_PROXY:*/
            strReasonPhrase = "Use Proxy";
            break;
        case 380:/*ALTERNATIVE_SERVICE:*/
            strReasonPhrase = "Alternative Service";
            break;
        case 400:/*BAD_REQUEST*/
            strReasonPhrase = "Bad Request";
            break;
        case 401:/*UNAUTHORIZED:*/
            strReasonPhrase = "Unauthorized";
            break;
        case 402:/*PAYMENT_REQUIRED:*/
            strReasonPhrase = "Payment Required";
            break;
        case 403:/*FORBIDDEN:*/
            strReasonPhrase = "Forbidden";
            break;
        case 404:/*REQUEST_NOT_FOUND:*/
            strReasonPhrase = "Not Found";
            break;
        case 405:/*METHOD_NOT_ALLOWED:*/
            strReasonPhrase = "Method Not Allowed";
            break;
        case 406:/*NOT_ACCEPTABLE:*/
            strReasonPhrase = "Not Acceptable";
            break;
        case 407:/*PROXY_AUTHENTICATION_REQUIRED:*/
            strReasonPhrase = "Proxy Authentication Required";
            break;
        case 408:/*REQUEST_TIMEOUT:*/
            strReasonPhrase = "Request Timeout";
            break;
        case 409:/*CONFLICT:*/
            strReasonPhrase = "Conflict";
            break;
        case 410:/*GONE:*/
            strReasonPhrase = "Gone";
            break;
        case 412:
            strReasonPhrase = "Conditional Request Failed";
            break;
         case 413:/*REQUEST_ENTITY_TOO_LARGE:*/
            strReasonPhrase = "Request Entity Too Large";
            break;
        case 414:/*REQUEST_URI_TOO_LONG:*/
            strReasonPhrase = "Request Uri Too Long";
            break;
        case 415:/*UNSUPPORTED_MEDIA_TYPE:*/
            strReasonPhrase = "Unsupported Media Type";
            break;
        case 420:/*BAD_EXTENTION:*/
            strReasonPhrase = "Bad Extension";
            break;
		case 421:/*EXTENSION_REQUIRED:*/
            strReasonPhrase = "Extension Required";
            break;
        case 422:/*SESSION INTERVAL TOO SMALL:*/
            strReasonPhrase = "Session Interval Too Small";
            break;
        case 423:/*INTERVAL TOO BRIEF:*/
            strReasonPhrase = "Interval Too Brief";
            break;
        case 429: /*PROVIDE REFERRER IDENTIFY*/
            strReasonPhrase = "Provide Referrer Identify";
            break;
        case 480:/*TEMPORARILY_UNAVAILABLE:*/
            strReasonPhrase = "Temporarily Unavailable";
            break;
        case 481:/*CAL_LEG_DOES_NOT_EXIST:*/
            strReasonPhrase = "Call Leg/Transaction Does Not Exist";
            break;
        case 482:/*LOOP_DETECTED:*/
            strReasonPhrase = "Loop Detected";
            break;
        case 483:/*TOO_MANY_HOPS:*/
            strReasonPhrase = "Too Many Hops";
            break;
        case 484:/*ADDRESS_INCOMPLETE:*/
            strReasonPhrase = "Address Incomplete";
            break;
        case 485:/*AMBIGUOUS:*/
            strReasonPhrase = "Ambiguous";
            break;
        case 486:/*BUSY_HERE:*/
            strReasonPhrase = "Busy Here";
            break;
        case 487:/*REQUEST_TERMINATED:*/
            strReasonPhrase = "Request Terminated";
            break;
        case 488:/*NOT_ACCEPTABLE_HERE*/
            strReasonPhrase = "Not Acceptable Here";
            break;
        case 489:
            strReasonPhrase = "Bad Event";
            break;
        case 491:/*REQUEST_PENDING*/
            strReasonPhrase = "Request Pending";
            break;
        case 493:/*UNDECIPHERABLE*/
            strReasonPhrase = "Undecipherable";
            break;
		case 494:/*SECURITY_AGREEMENT_REQUIRED*/
            strReasonPhrase = "Security Agreement Required";
            break;
        case 500:/*SERVER_INTERNAL_ERROR:*/
            strReasonPhrase = "Server Internal Error";
            break;
        case 501:/*NOT_IMPLEMENTED:*/
            strReasonPhrase = "Not Implemented";
            break;
        case 502:/*BAD_GATEWAY:*/
            strReasonPhrase = "Bad Gateway";
            break;
        case 503:/*SERVICE_UNAVAILABLE:*/
            strReasonPhrase = "Service Unavailable";
            break;
        case 504:/*Server_TIME_OUT:*/
            strReasonPhrase = "Server Time-out";
            break;
        case 505:/*VERSION_NOT_SUPPORTED:*/
            strReasonPhrase = "Version Not Supported";
            break;
        case 600:/*BUSY_EVERYWHERE:*/
            strReasonPhrase = "Busy Everywhere";
            break;
        case 603:/*DECLINE:*/
            strReasonPhrase = "Decline";
            break;
        case 604:/*DOES_NOT_EXIST_ANYWHERE:*/
            strReasonPhrase = "Does Not Exist Anywhere";
            break;
        case 606:/*GLOBAL_NOT_ACCEPTABLE:*/
            strReasonPhrase = "Not Acceptable";
            break;
        default:
            checkDefaults = RV_TRUE;
        }

    /* if we haven't set the reasonPharse yet, we need to give the default value
    according to the range of the Code number */
    if(checkDefaults == RV_TRUE)
    {
        if((Code >= 100) && (Code < 200))
            strReasonPhrase = "Trying";
        else if ((Code >= 200) && (Code < 300))
            strReasonPhrase = "OK";
        else if ((Code >= 300) && (Code < 400))
            strReasonPhrase = "Multiple Choices";
        else if ((Code >= 400) && (Code < 500))
            strReasonPhrase = "Bad Request";
        else if ((Code >= 500) && (Code < 600))
            strReasonPhrase = "Server Internal Error";
        else if ((Code >= 600) && (Code < 700))
            strReasonPhrase = "Busy Everywhere";
        else
            strReasonPhrase = NULL;
    }
    return(strReasonPhrase);
}

/***************************************************************************
 * SipMsgGetReasonPhrase
 * ------------------------------------------------------------------------
 * General: This method returns the reason phrase, from a status line,
 *          in string format.
 * Return Value: Offset of the textual description of status code or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg - Handle of a message object.
 ***************************************************************************/
RvInt32 SipMsgGetReasonPhrase(IN RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;

    msg = (MsgMessage*)hSipMsg;

    return msg->startLine.statusLine.strReasonePhrase;
}

/***************************************************************************
 * SipMsgSetReasonPhrase
 * ------------------------------------------------------------------------
 * General: This method sets the reason phrase in the statusLine.
 *          If the string lays on a memory pool copying will be maid only if
 *          the page and pool are different than the object's page and pool.
 *          Otherwise attach will be maid.
 * Return Value:  RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle of a message object.
 *         strReasonPhrase - reason phrase to be set in the message object.
 *                  If NULL, the exist reason phrase in the msg will be removed,
 *         reasonOffset - The offset of the reason string.
 *         hPool - The pool on which the string lays (if relevant)
 *         hPage - The page on which the string lays (if relevant)
 ***************************************************************************/
RvStatus SipMsgSetReasonPhrase(IN    RvSipMsgHandle hSipMsg,
                                IN    RvChar*       strReasonPhrase,
                                IN    HRPOOL         hPool,
                                IN    HPAGE          hPage,
                                IN    RvInt32       reasonOffset)
{
    RvInt32      newStr;
    MsgMessage*   msg;
    RvStatus     retStatus;

    msg = (MsgMessage*)hSipMsg;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

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


    retStatus = MsgUtilsSetString(msg->pMsgMgr, hPool, hPage, reasonOffset, strReasonPhrase,
                                  msg->hPool, msg->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    msg->startLine.statusLine.strReasonePhrase = newStr;
#ifdef SIP_DEBUG
    msg->startLine.statusLine.pReasonePhrase =
                       (RvChar *)RPOOL_GetPtr(msg->hPool, msg->hPage,
                           msg->startLine.statusLine.strReasonePhrase);
#endif

    return RV_OK;
}

/***************************************************************************
 * SipMsgGetCallIdHeader
 * ------------------------------------------------------------------------
 * General: Retrieves a pointer to the CALL-ID string from the message.
 * Return Value: Offset of the callId string, or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipMsg - Handle of a message object.
 ***************************************************************************/
RvInt32 SipMsgGetCallIdHeader(IN RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;

    msg = (MsgMessage*)hSipMsg;

    if(msg->hCallIdHeader == NULL)
    {
        return UNDEFINED;
    }
    return SipOtherHeaderGetValue(msg->hCallIdHeader);
}


/***************************************************************************
 * SipMsgGetCallIdHeaderName
 * ------------------------------------------------------------------------
 * General: Retrieves a pointer to the CALL-ID header name string from 
 *          the message.
 * Return Value: Offset of the callId header name string, or UNDEFINED 
 *        if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *	hSipMsg - Handle of a message object.
 ***************************************************************************/
RvInt32 SipMsgGetCallIdHeaderName(IN RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;

    msg = (MsgMessage*)hSipMsg;

    if(msg->hCallIdHeader == NULL)
    {
        return UNDEFINED;
    }
    return SipOtherHeaderGetName(msg->hCallIdHeader);
}


/***************************************************************************
 * SipMsgSetCallIdHeader
 * ------------------------------------------------------------------------
 * General: This method sets the Call-Id header in the message.
 *          If the string lays on a memory pool copying will be maid only if
 *          the page and pool are different than the object's page and pool.
 *          Otherwise attach will be maid.
 * Return Value:  RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle of a message object.
 *         strCallId - Call-Id to be set in the message object.
 *                  If NULL, the exist Call-Id in the msg will be removed,
 *         offset - The offset of the reason string.
 *         hPool - The pool on which the string lays (if relevant)
 *         hPage - The page on which the string lays (if relevant)
 ***************************************************************************/
RvStatus SipMsgSetCallIdHeader(IN    RvSipMsgHandle hSipMsg,
                               IN    RvChar*       strCallId,
                               IN    HRPOOL         hPool,
                               IN    HPAGE          hPage,
                               IN    RvInt32       offset)
{
    MsgMessage*   msg;
    RvStatus     retStatus;

    msg = (MsgMessage*)hSipMsg;

    if(hSipMsg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
        "SipMsgSetCallIdHeader: setting Call-Id in msg 0x%p.", msg));

    if(msg->hCallIdHeader == NULL)
    {
        retStatus = RvSipOtherHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                              msg->hPool, msg->hPage, &(msg->hCallIdHeader));
        if(retStatus != RV_OK)
        {
            RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                  "SipMsgSetCallIdHeader: Failed to construct other header."));
            return retStatus;
        }
        retStatus = SipOtherHeaderSetName(msg->hCallIdHeader, SIP_MSG_CALL_ID_HEADER_NAME, NULL, NULL, UNDEFINED);
        if(retStatus != RV_OK)
        {
            RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                  "SipMsgSetCallIdHeader: Failed to set other header name."));
            return retStatus;
        }
    }
    retStatus = SipOtherHeaderSetValue(msg->hCallIdHeader, strCallId, hPool, hPage, offset);
    if(retStatus != RV_OK)
    {
        RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
              "SipMsgSetCallIdHeader: Failed to set other header value."));
        return retStatus;
    }

    return RV_OK;
}

/***************************************************************************
 * SipMsgSetCallIdHeaderName
 * ------------------------------------------------------------------------
 * General: This method sets the Call-Id header name (compact or not)
 *          in the message.
 *          If the name string lays on a memory pool copying will be maid only if
 *          the page and pool are different than the object's page and pool.
 *          Otherwise attach will be made.
 * Return Value:  RV_Success, RV_OutOfResources, RV_InvalidHandle.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle of a message object.
 *         strCallIdName - 
 *                   Call-Id header name (might be "Call-ID" or "i"),
 *                   to be set in the message object.
 *         offset  - The offset of the reason string.
 *         hPool   - The pool on which the string lays (if relevant)
 *         hPage   - The page on which the string lays (if relevant)
 ***************************************************************************/
RvStatus SipMsgSetCallIdHeaderName(
                                IN    RvSipMsgHandle hSipMsg,
		                        IN    RvChar        *strCallIdName,
                                IN    HRPOOL         hPool,
                                IN    HPAGE          hPage,
                                IN    RvInt32        offset)
{
    MsgMessage  *msg;
    RvStatus     retStatus;

    msg = (MsgMessage*)hSipMsg;

    if(hSipMsg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
        "SipMsgSetCallIdHeaderName: setting Call-Id name in msg 0x%p.", msg));
        
    /* First construct it, if there's no Call-ID header */
    if(msg->hCallIdHeader == NULL)
    {
        retStatus = RvSipOtherHeaderConstruct((RvSipMsgMgrHandle)msg->pMsgMgr,
                                              msg->hPool, msg->hPage, &(msg->hCallIdHeader));
        if(retStatus != RV_OK)
        {
            RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
                "SipMsgSetCallIdHeaderName: Failed to construct other header in msg 0x%p.",msg));
            return retStatus;
        }
        
    }
   
    retStatus = SipOtherHeaderSetName(msg->hCallIdHeader, strCallIdName, hPool, hPage, offset);
    if(retStatus != RV_OK)
    {
        RvLogError(msg->pMsgMgr->pLogSrc,(msg->pMsgMgr->pLogSrc,
            "SipMsgSetCallIdHeaderName: Failed to set other header name in msg 0x%p.",msg));
        return retStatus;
    }

    return RV_OK;
}

/***************************************************************************
 * SipMsgGetCallIDHeaderHandle
 * ------------------------------------------------------------------------
 * General: Returns the handle of the Call ID in the message, that its handle is specified.
 * Return Value: RvSipOtherHeaderHandle - handle to the Call ID in the message
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg - The handle to the message.
 ***************************************************************************/
RvSipOtherHeaderHandle SipMsgGetCallIDHeaderHandle(IN    RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;

    if(hSipMsg == NULL)
        return NULL;

    msg = (MsgMessage*)hSipMsg;
    return msg->hCallIdHeader;
}

#ifdef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * SipMsgSetCallIDHeaderHandle
 * ------------------------------------------------------------------------
 * General: Sets the given Call ID to the message. The Call-Id is held in
 *          an RvSipOtherHeader.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsg  - The message to set the call-id to.
 *         hCallId - The handle to the other header that contains the Call-Id
 ***************************************************************************/
RvStatus SipMsgSetCallIDHeaderHandle(IN  RvSipMsgHandle          hMsg,
									 IN  RvSipOtherHeaderHandle  hCallId)
{
	MsgMessage*   pMsg;
	RvStatus      stat;
	
    pMsg = (MsgMessage*)hMsg;
	
    if(hCallId == NULL)
    {
        pMsg->hCallIdHeader = NULL;
        return RV_OK;
    }
    
	/* if there is no Call-Id header object in the msg, we will allocate one */
    if(pMsg->hCallIdHeader == NULL)
    {
        stat = RvSipOtherHeaderConstruct((RvSipMsgMgrHandle)pMsg->pMsgMgr, 
										 pMsg->hPool, pMsg->hPage, 
										 &(pMsg->hCallIdHeader));
		if(stat != RV_OK)
		{
			RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                       "SipMsgSetCallIDHeaderHandle: Failed to construct other header in msg 0x%p.",pMsg));
			return stat;
		}
    }

    return RvSipOtherHeaderCopy(pMsg->hCallIdHeader, hCallId);
}
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

#ifndef RV_SIP_PRIMITIVES
#else
/***************************************************************************
 * SipMsgGetContentTypeHeader
 * ------------------------------------------------------------------------
 * General: Retrieves a pointer to the CONTENT-TYPE string from the list.
 * Return Value: offset of the content type string, or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipMsg - Handle of a message object.
 ***************************************************************************/
RvInt32 SipMsgGetContentTypeHeader(IN RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;

    msg = (MsgMessage*)hSipMsg;

    return msg->strContentType;
}
/***************************************************************************
 * SipMsgGetContentIDHeader
 * ------------------------------------------------------------------------
 * General: Retrieves a pointer to the CONTENT-TYPE string from the list.
 * Return Value: offset of the content type string, or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipMsg - Handle of a message object.
 ***************************************************************************/
RvInt32 SipMsgGetContentIDHeader(IN RvSipMsgHandle hSipMsg)
{
    MsgMessage* msg;

    msg = (MsgMessage*)hSipMsg;

    return msg->strContentID;
}
#endif /* #ifndef RV_SIP_PRIMITIVES */

#ifndef RV_SIP_PRIMITIVES
#else /* RV_SIP_PRIMITIVES */
/***************************************************************************
 * SipMsgSetContentTypeHeader
 * ------------------------------------------------------------------------
 * General: This method sets the Call-Id header in the message.
 *          If the string lays on a memory pool copying will be maid only if
 *          the page and pool are different than the object's page and pool.
 *          Otherwise attach will be maid.
 * Return Value:  RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle of a message object.
 *         strType - Content-type to be set in the message object.
 *                  If NULL, the exist Call-Id in the msg will be removed,
 *         offset - The offset of the reason string.
 *         hPool - The pool on which the string lays (if relevant)
 *         hPage - The page on which the string lays (if relevant)
 ***************************************************************************/
RvStatus SipMsgSetContentTypeHeader(IN    RvSipMsgHandle hSipMsg,
                                     IN    RvChar*       strType,
                                     IN    HRPOOL         hPool,
                                     IN    HPAGE          hPage,
                                     IN    RvInt32       offset)
{
    RvInt32      newStr;
    MsgMessage*   msg;
    RvStatus     retStatus;

    msg = (MsgMessage*)hSipMsg;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    retStatus = MsgUtilsSetString(msg->pMsgMgr, hPool, hPage, offset, strType,
                                  msg->hPool, msg->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    msg->strContentType = newStr;
#ifdef SIP_DEBUG
    msg->pContentType = (RvChar *)RPOOL_GetPtr(msg->hPool, msg->hPage,
                                     msg->strContentType);
#endif /* SIP_DEBUG */

    return RV_OK;
}

/***************************************************************************
 * SipMsgSetContentIDHeader
 * ------------------------------------------------------------------------
 * General: This method sets the Content-Id header in the message.
 *          If the string lays on a memory pool copying will be maid only if
 *          the page and pool are different than the object's page and pool.
 *          Otherwise attach will be maid.
 * Return Value:  RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle of a message object.
 *         strID - Content-ID to be set in the message object.
 *                  If NULL, the exist Content-Id in the msg will be removed,
 *         offset - The offset of the reason string.
 *         hPool - The pool on which the string lays (if relevant)
 *         hPage - The page on which the string lays (if relevant)
 ***************************************************************************/
RvStatus SipMsgSetContentIDHeader(IN    RvSipMsgHandle hSipMsg,
                                     IN    RvChar*       strID,
                                     IN    HRPOOL         hPool,
                                     IN    HPAGE          hPage,
                                     IN    RvInt32       offset)
{
    RvInt32      newStr;
    MsgMessage*   msg;
    RvStatus     retStatus;

    msg = (MsgMessage*)hSipMsg;

    if(hSipMsg == NULL)
        return RV_ERROR_INVALID_HANDLE;

    retStatus = MsgUtilsSetString(msg->pMsgMgr, hPool, hPage, offset, strID,
                                  msg->hPool, msg->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }
    msg->strContentID = newStr;
#ifdef SIP_DEBUG
    msg->pContentID = (RvChar *)RPOOL_GetPtr(msg->hPool, msg->hPage,
                                     msg->strContentID);
#endif /* SIP_DEBUG */

    return RV_OK;
}
#endif /* RV_SIP_PRIMITIVES*/

/***************************************************************************
 * SipMsgSetBadSyntaxInfoInStartLine
 * ------------------------------------------------------------------------
 * General: This function sets the bad-syntax string in the message.
 *          The function is for parser usage.
 * Return Value:  RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsg     - Handle of a message object.
 *         BSoffset - Offset of the bad-syntax string on message page.
 ***************************************************************************/
RvStatus RVCALLCONV SipMsgSetBadSyntaxInfoInStartLine(RvSipMsgHandle hMsg,
                                                      RvChar*        pStrStartLine)
{
    RvInt32      newStr;
    MsgMessage*  msg;
    RvStatus     retStatus;
    
    msg = (MsgMessage*)hMsg;
    if (hMsg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    retStatus = MsgUtilsSetString(msg->pMsgMgr, NULL, NULL, UNDEFINED, pStrStartLine,
                                  msg->hPool, msg->hPage, &newStr);
    if (RV_OK != retStatus)
    {
        return retStatus;
    }

    msg->strBadSyntaxStartLine = newStr;
#ifdef SIP_DEBUG
    msg->pBadSyntaxStartLine = (RvChar *)RPOOL_GetPtr(msg->hPool, msg->hPage, newStr);
#endif /* SIP_DEBUG */


    return RV_OK;
}

/***************************************************************************
 * SipMsgGetBadSyntaxStartLine
 * ------------------------------------------------------------------------
 * General: This function gets the bad-syntax start-line offset.
 *          The function is for parser usage.
 * Return Value:  offset
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsg     - Handle of a message object.
 ***************************************************************************/
RvInt32 RVCALLCONV SipMsgGetBadSyntaxStartLine(RvSipMsgHandle hMsg)
{
    return ((MsgMessage*)hMsg)->strBadSyntaxStartLine;
}

/***************************************************************************
 * SipMsgSetBadSyntaxReasonPhrase
 * ------------------------------------------------------------------------
 * General: This function sets the bad-syntax reason phrase string in the message.
 *          The function is for parser usage.
 * Return Value:  RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsg     - Handle of a message object.
 *         pBSReasonPhrase - Pointer to the reason phrase string.
 ***************************************************************************/
RvStatus RVCALLCONV SipMsgSetBadSyntaxReasonPhrase(RvSipMsgHandle hMsg,
                                                    RvChar*       pBSReasonPhrase)
{
    MsgMessage* msg = (MsgMessage*)hMsg;
    if (hMsg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    MsgUtilsSetString(msg->pMsgMgr, NULL, NULL, UNDEFINED,
                      pBSReasonPhrase, msg->hPool, msg->hPage,
                      &(msg->strBadSyntaxReasonPhrase));

#ifdef SIP_DEBUG
    msg->pBadSyntaxReasonPhrase = (RvChar *)RPOOL_GetPtr(msg->hPool, msg->hPage,
                              msg->strBadSyntaxReasonPhrase);
#endif /* SIP_DEBUG */

    return RV_OK;
}

/***************************************************************************
 * SipMsgGetBadSyntaxReasonPhrase
 * ------------------------------------------------------------------------
 * General: This function gets the bad-syntax reason phrase string from the message.
 *          The function is for parser usage.
 * Return Value:  RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsg     - Handle of a message object.
 ***************************************************************************/
RvInt32 RVCALLCONV SipMsgGetBadSyntaxReasonPhrase(RvSipMsgHandle hMsg)
{
    MsgMessage* msg = (MsgMessage*)hMsg;
    if (hMsg == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    return msg->strBadSyntaxReasonPhrase;

}

/***************************************************************************
 * SipMsgHeaderGetType
 * ------------------------------------------------------------------------
 * General: Gets the type of the header hHeader
 * Return Value:  (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * input:     hHeader - a pointer to a header of any kind.
 * Output:    type    - the type of the header.
 ***************************************************************************/
void RVCALLCONV SipMsgHeaderGetType(IN  void            *hHeader,
                                    OUT RvSipHeaderType *type)
{
    *type = *((RvSipHeaderType *)hHeader);
}


/***************************************************************************
 * SipMsgGetCompTypeName
 * ------------------------------------------------------------------------
 * General: This is an internal function, which returns the compression type
 *          string.
 * Return Value: string of the compression type.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: eCompType - The compression type.
 ***************************************************************************/
RvChar *RVCALLCONV SipMsgGetCompTypeName(IN RvSipCompType eCompType)
{
    switch (eCompType)
    {
    case RVSIP_COMP_SIGCOMP:
        return SIGCOMP_STR;
    default:
        return "Undefined";
    }
}

/***************************************************************************
 * SipMsgGetCompTypeEnum
 * ------------------------------------------------------------------------
 * General: This is an internal function, which returns the compression type
 *          enum the suits the compression type string.
 * Return Value: string of the compression type.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: eCompType - The compression type.
 ***************************************************************************/
RvSipCompType RVCALLCONV SipMsgGetCompTypeEnum(IN RvChar *strCompType)
{
    if (SipCommonStricmp(strCompType,SIGCOMP_STR) == RV_TRUE)
    {
        return RVSIP_COMP_SIGCOMP;
    }

    return RVSIP_COMP_UNDEFINED;
}


/***************************************************************************
 * SipMsgDoesOtherHeaderExist
 * ------------------------------------------------------------------------
 * General: Checks if a specific other header name with a specific value
 *          exists in a given message.
 * Return Value: boolean value that indicates if the headers exists.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hMsg - The msg handle to look in
 *        strHeaderName - The Other header name
 *        strHeaderValue - The Other header value we look for.
 *        phHeader - The header that was found. If you are not interested
 *                   in the header itself you can supply null in this parameter.
 ***************************************************************************/
RvBool RVCALLCONV SipMsgDoesOtherHeaderExist(
                                IN RvSipMsgHandle           hMsg,
                                IN RvChar*                  strHeaderName,
                                IN RvChar*                  strHeaderValue,
                                OUT RvSipOtherHeaderHandle* phHeader)
{
    RvSipOtherHeaderHandle    hHeader    = NULL;
    RvSipHeaderListElemHandle hElement   = NULL;
    RvInt32                   nameOffset = UNDEFINED;
    MsgMessage*               pMsg       = NULL;

    pMsg = (MsgMessage*)hMsg;


     
    if(pMsg == NULL)
    {
        return RV_FALSE;
    }

    if(phHeader != NULL)
    {
        *phHeader = NULL;
    }
   
    if(strHeaderName == NULL)
    {
         RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                    "SipMsgDoesOtherHeaderExist: Msg 0x%p, Failed, header name is NULL",hMsg));
         return RV_FALSE;
    }

    if(strHeaderValue == NULL)
    {
         RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                    "SipMsgDoesOtherHeaderExist: Msg 0x%p, Failed, header value is NULL",hMsg));
         return RV_FALSE;
    }


    RvLogDebug(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
            "SipMsgDoesOtherHeaderExist: Msg 0x%p, looking for %s %s",hMsg,strHeaderName, strHeaderValue));

    
    hHeader =  RvSipMsgGetHeaderByName(hMsg,strHeaderName,
                                       RVSIP_FIRST_HEADER,&hElement);

    while(hHeader != NULL)
    {
        nameOffset = SipOtherHeaderGetValue(hHeader);
        if(nameOffset != UNDEFINED)
        {
            if(RPOOL_CmpToExternal(SipOtherHeaderGetPool(hHeader),
                SipOtherHeaderGetPage(hHeader),
                nameOffset,
                strHeaderValue,
                RV_FALSE) == RV_TRUE)
            {
                if(phHeader != NULL)
                {
                    *phHeader = hHeader;
                }
                RvLogDebug(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                    "SipMsgDoesOtherHeaderExist: Msg 0x%p, %s %s was found. header=0x%p",hMsg,strHeaderName, strHeaderValue,hHeader));

                return RV_TRUE;
            }
        }
        hHeader =  RvSipMsgGetHeaderByName(hMsg,strHeaderName,
                                       RVSIP_NEXT_HEADER,&hElement);
    }
    RvLogDebug(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
               "SipMsgDoesOtherHeaderExist: Msg 0x%p, %s %s was not found",hMsg,strHeaderName, strHeaderValue));

    return RV_FALSE;
}

/***************************************************************************
 * SipMsgAddNewOtherHeaderToMsg
 * ------------------------------------------------------------------------
 * General: Add an other header with a specific name and value to the message.
 *          the new header handle is returned.
 *          the header is added to the end of the headers list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hMsg - The msg handle.
 *        strHeaderName - The Other header name
 *        strHeaderValue - The Other header value.
 *        phHeader - The new header that was created. If you are not interested
 *                   in the header itself you can supply null in this parameter.
 ***************************************************************************/
RvStatus RVCALLCONV SipMsgAddNewOtherHeaderToMsg(
                                IN RvSipMsgHandle           hMsg,
                                IN RvChar*                  strHeaderName,
                                IN RvChar*                  strHeaderValue,
                                OUT RvSipOtherHeaderHandle* phHeader)
{
    RvStatus                  rv     = RV_OK;
    MsgMessage*               pMsg   = NULL;
    RvSipOtherHeaderHandle    hOther = NULL;

    pMsg = (MsgMessage*)hMsg;
    
    if(pMsg == NULL)
    {
        return RV_FALSE;
    }

    if(phHeader != NULL)
    {
        *phHeader = NULL;
    }
   
    if(strHeaderName == NULL)
    {
         RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                    "SipMsgAddNewOtherHeaderToMsg: Msg 0x%p, Failed, header name is NULL",hMsg));
         return RV_ERROR_BADPARAM;
    }

    if(strHeaderValue == NULL)
    {
         RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                    "SipMsgAddNewOtherHeaderToMsg: Msg 0x%p, Failed, header value is NULL",hMsg));
         return RV_ERROR_BADPARAM;
    }

    RvLogDebug(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
            "SipMsgAddNewOtherHeaderToMsg: Msg 0x%p, adding %s: %s to the message",hMsg,strHeaderName, strHeaderValue));

    /*constructing header*/
    rv = RvSipOtherHeaderConstructInMsg(hMsg,RV_FALSE,&hOther);
    if(rv != RV_OK)
    {
         RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
            "SipMsgAddNewOtherHeaderToMsg - Msg 0x%p,Failed to construct new other header in msg, rv=%d",
            hMsg,rv));
        return rv;
    }
    /*set name*/
    rv = RvSipOtherHeaderSetName(hOther,strHeaderName);
    if (RV_OK != rv)
    {
         RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
            "SipMsgAddNewOtherHeaderToMsg: Msg 0x%p - failed to set header name. rv=%d",
            hMsg, rv));
        return rv;
    }
    /*set value*/
    rv = RvSipOtherHeaderSetValue(hOther,strHeaderValue);
    if (RV_OK != rv)
    {
         RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
            "SipMsgAddNewOtherHeaderToMsg: Msg 0x%p - failed to set header value. rv=%d",
            hMsg, rv));
        return rv;
    }
    
    RvLogDebug(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
            "SipMsgAddNewOtherHeaderToMsg: Msg 0x%p, header 0x%p was added succussfuly",hMsg,hOther));

    if(phHeader != NULL)
    {
        *phHeader = hOther;
    }
    return RV_OK;

}

#ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT
/***************************************************************************
 * SipMsgIncCounter
 * ------------------------------------------------------------------------
 * General: Increments the counter field of the message. The message will not 
 *          be deallocated as long as counter > 0, i.e., RvSipMsgDistruct() will not
 *			be effective if counter > 0. The counter will be decremented with 
 *          any call to RvSipMsgDistruct().
 *          If an object wished to call RvSipMsgDistruct() twice for the same message
 *          it can increment the counter once. This was the message will be distructed
 *          only in the second call to RvSipMsgDistruct().
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * input:   hSipMsg - Handle of the message to increment its counter.
 ***************************************************************************/
void SipMsgIncCounter(IN  RvSipMsgHandle hSipMsg)
{
	MsgMessage  *pMsg = (MsgMessage*)hSipMsg;

	if (NULL == hSipMsg)
		return;

	pMsg->terminationCounter++;
}
#endif /* #ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */


#ifdef __cplusplus
}
#endif
