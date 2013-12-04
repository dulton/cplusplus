/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             MsgParser.c                                    
 *                                                                            
 * This file implements the message parsing process.                          
 * The parsing process is done by both the message layer and the parser layer.
 *
 * The message layer is responsible for:
 * 1. Seperate a given message to parts. (a part is a header or a start-line.)
 *    The message layer set a NULL character in the end of the line (instead 
 *    of CR LF), before passing it to the parser.
 * 2. Check the header prefix, and decide the following:
 *    A) The header type (which object should be allocated for it, and which 
 *    header rule should the parser perform in paring this header)
 *    B) The header specific type (for objects that may hold several header
 *    types. (e.g. TO/FROM for party header, ROUTE/RECORD-ROUTE for route-hop
 *    header.)
 *    C) Compact form - is the header value is on compact form or not.
 * 3. In case of parser failure, the message layer saves the header as a bad-
 *    synax header.
 *
 * In the end the message layer passes all the information to the parser layer.
 * The parser should use this information to parse the header with the correct 
 * parsing rule, allocate the correct header object, and fill it with the 
 * information.
 *
 *                                                                            
 *      Author           Date                                                 
 *     ------           ------------                                          
 *      Ofra             May 2005                                             
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/
    
#define MSG_NULL_CHAR '\0'
#define TAB_CHAR 0x09
#define SPACE_CHAR ' '
#define HEADER_SAFE_COUNTER_LIMIT 100000
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
    
#include "MsgTypes.h"
#include "MsgUtils.h"
 
#include "_SipMsg.h"
#include "_SipHeader.h"
#include "RvSipOtherHeader.h"
#include "RvSipPartyHeader.h"
#include "RvSipContentTypeHeader.h"
#include "RvSipAllowEventsHeader.h"
#include "RvSipEventHeader.h"
#include "RvSipReferToHeader.h"
#include "RvSipReferredByHeader.h"
#include "RvSipViaHeader.h"
#include "RvSipContactHeader.h"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvSipHeaderType RVCALLCONV GetHeaderType(IN RvChar* pHeaderStart, 
                                                IN RvChar* pHeaderNameEnd,
                                                IN RvInt32 headerNameLen,
                                                OUT RvBool* pbCompactForm,
                                                OUT SipParseType *peSpecificHeaderType);
static RvStatus RVCALLCONV ParseHeader(IN MsgMgr*   pMsgMgr,
                                       IN void*      pObj,
                                       IN SipParseType eObjType,
                                       IN RvBool     bStartLine,
                                       IN RvChar* pHeaderStart,
                                       IN RvChar* pHeaderNameEnd,  
                                       IN RvInt32 headerNameLen,
                                       IN RvChar* pHeaderVal,
                                        IN RvInt32 lineNumber);
static RvBool IsOptionTagHeader(SipParseType eSpecificHeaderType);
static RvStatus RVCALLCONV SetCompactFormInBSHeader(void*           pHeader,
                                                  RvSipHeaderType eHeaderType);

static void RVCALLCONV SetBsHeaderSpecificType( IN  void*          pHeader,
                                              IN  SipParseType   eSpecificHeaderType);

static RvStatus RVCALLCONV SetBsViaTransport(IN void*   pHeader, IN RvChar* pHeaderVal);

static RvStatus RVCALLCONV ParseBadSyntaxHeader(IN MsgMessage*     pMsg,
                                                IN RvSipHeaderType eHeaderType,
                                                IN RvBool          bCompactForm,
                                                IN SipParseType    eSpecificHeaderType,
                                                IN RvChar*         pHeaderStart,
                                                IN RvChar*         pHeaderNameEnd,  
                                                IN RvChar*         pHeaderVal);

static RvStatus RVCALLCONV PrepareAndParseMessageHeader(IN MsgMgr*        pMsgMgr,
                                                        IN void*          pObj,
                                                        IN SipParseType   eObjType,
                                                        IN RvInt32        lineNumber,
                                                        INOUT RvChar**    pBuffer,
                                                        INOUT RvInt32*    pIndex);
/*-------------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                              */
/*-------------------------------------------------------------------------*/
/***************************************************************************
 * MsgParserParse
 * ------------------------------------------------------------------------
 * General: The function gets several headers, separated by CRLF.
 *          It separates the message headers and pass each one to the parser.
 *          The function may get a whole message or list of body-parts headers.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hMsg      - A handle to the message object.
 *  eObjType  - SIP_PARSETYPE_MSG or SIP_PARSETYPE_MIME_BODY_PART
 *  buffer    - Buffer containing a textual msg ending with two CRLFs.
 *  bufferLen - Length of buffer.
 ***************************************************************************/
RvStatus RVCALLCONV MsgParserParse(IN MsgMgr*       pMsgMgr,
                                   IN void*          pObj,
                                   IN SipParseType   eObjType,
                                   IN RvChar*       buffer,
                                   IN RvInt32       bufferLen)
{
    RvStatus   rv = RV_OK;
    RvChar*    pBuf = buffer;
    RvInt32    numOfHeadersSafeCounter = 1;
    RvInt      index = 0;

#define MSG_MAX_NUM_OF_HEADERS 100

    if(pMsgMgr == NULL || pObj == NULL ||(buffer == NULL))
        return RV_ERROR_BADPARAM;
    
    RvLogInfo(pMsgMgr->pLogSrc, (pMsgMgr->pLogSrc,
                "MsgParserParse - Start to parse message 0x%p", pObj));
    
    while (index < bufferLen && numOfHeadersSafeCounter < MSG_MAX_NUM_OF_HEADERS &&
           *pBuf != MSG_NULL_CHAR) /* go over all message */
    {
        rv = PrepareAndParseMessageHeader(pMsgMgr, pObj, eObjType, numOfHeadersSafeCounter, &pBuf, &index);
        if(rv != RV_OK)
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgParserParse - message 0x%p. Failed to parse header %d. (rv=%d)",
                 pObj, numOfHeadersSafeCounter, rv));
            return rv;

        }
        ++numOfHeadersSafeCounter;

        /* skip the reset of CRLF in the end of the header */
        while (*pBuf == CR || *pBuf == LF)
        {
            ++pBuf;
            ++index;
        }
    }    
    RvLogInfo(pMsgMgr->pLogSrc, (pMsgMgr->pLogSrc,
                "MsgParserParse - Finish to parse message 0x%p successfuly", pObj));
    return RV_OK;
}

/***************************************************************************
 * MsgParserParseStandAloneHeader
 * ------------------------------------------------------------------------
 * General: The function gets a null terminated header string (with or without
 *          the header name), and the type of the header.
 *          The function finds the beginning of the header value, and pass
 *          it to the parser.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  pMsgMgr       - Pointer to the parser manager.
 *  eHeaderType   - The type of the header being parsed.
 *  pStrHeader    - Null terminated string of the header.
 *  bOnlyValue    - true if the string does not contain the header name
 *                  false if contains the header name.
 *  pHeader       - Pointer to the header object.
 ***************************************************************************/
RvStatus RVCALLCONV MsgParserParseStandAloneHeader(  IN MsgMgr*       pMsgMgr,
                                                     IN SipParseType  eHeaderType,
                                                     IN RvChar*       pStrHeader,
                                                     IN RvBool        bOnlyValue,
                                                     IN void*         pHeader)
{
    RvStatus   rv = RV_OK;
   
    RvLogInfo(pMsgMgr->pLogSrc, (pMsgMgr->pLogSrc,
                "MsgParserParseStandAloneHeader - Start to parse header 0x%p", pHeader));

    if(RV_TRUE == bOnlyValue)
    {
        /* we don't have to investigate the header name. call directly to the parser */
        /* skip spaces before the header value: */
        while(*pStrHeader == SPACE_CHAR || *pStrHeader == TAB_CHAR)
        {
            ++pStrHeader;
        }

        rv = SipParserStartParsing(pMsgMgr->hParserMgr, 
                                    eHeaderType, 
                                    pStrHeader, 
                                    1,           /* lineNumber*/
                                    eHeaderType, /* eSpecificHeaderType*/
                                    RV_FALSE,    /* bCompactForm*/ 
                                    RV_TRUE,     /* bSetHeaderPrefix */
                                    pHeader) ;

    }
    else
    {
        RvChar*    pHeaderVal = NULL, *pHeaderNameEnd = NULL;
        RvInt32    safeCounter = 0, headerNameLen = 0;
        RvChar*    pBuf = pStrHeader;
        /* skip spaces after header name and before colon */        
        while(*pBuf != ':' && *pBuf != MSG_NULL_CHAR && safeCounter < HEADER_SAFE_COUNTER_LIMIT)
        {
            ++safeCounter;
            ++pBuf;
            ++ headerNameLen;
            
        }
        if(HEADER_SAFE_COUNTER_LIMIT == safeCounter)
        {
            RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgParserParseStandAloneHeader - header 0x%p. safe counter reached max value %d.",
                 pHeader, safeCounter));
            return RV_ERROR_UNKNOWN;            
        }
        
        if(*pBuf == ':')
        {
            pHeaderNameEnd = pBuf-1;
            pHeaderVal = ++pBuf;
        }
        else
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "MsgParserParseStandAloneHeader - header 0x%p. no colon was found in the given string (%s).",
                pHeader, pStrHeader));
            return RV_ERROR_ILLEGAL_SYNTAX; 
        }
            
        /* need to check the header name before calling the parser 
           (check compact form, and specific header type )*/
        rv = ParseHeader(pMsgMgr, 
                        pHeader, 
                        eHeaderType, 
                        RV_FALSE, /* bStartLine */
                        pStrHeader, 
                        pHeaderNameEnd, 
                        headerNameLen, 
                        pHeaderVal, 
                        1 /* lineNumber*/);
        if(rv != RV_OK)
        {
            RvLogError(pMsgMgr->pLogSrc, (pMsgMgr->pLogSrc,
                "MsgParserParseStandAloneHeader - Failed to parse header 0x%p", pHeader));
        }
        else
        {
            RvLogInfo(pMsgMgr->pLogSrc, (pMsgMgr->pLogSrc,
                "MsgParserParseStandAloneHeader - Finished to parse header 0x%p successfuly", pHeader));
        }
    }
    
    return rv;
}

/***************************************************************************
 * MsgParserParseStandAloneAddress
 * ------------------------------------------------------------------------
 * General: The function gets a null terminated address string pass it to
 *          the parser for parsing.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  pMsgMgr     - Pointer to the message manager.
 *  eAddrType   - Type of the given address.
 *  pStrAddr    - Null terminated string of the address.
 *  pAddress    - Pointer to the address object that should be filled with 
 *                address information.
 ***************************************************************************/
RvStatus RVCALLCONV MsgParserParseStandAloneAddress( IN MsgMgr*       pMsgMgr,
                                                     IN SipParseType  eAddrType,
                                                     IN RvChar*       pStrAddr,
                                                     IN void*         pAddress)
{
    RvStatus   rv = RV_OK;
   
    rv = SipParserStartParsing( pMsgMgr->hParserMgr, 
                                eAddrType, 
                                pStrAddr, 
                                1,          /* lineNumber*/
                                eAddrType,  /* eSpecificHeaderType*/
                                RV_FALSE,   /* bCompactForm*/ 
                                RV_TRUE,    /* bSetHeaderPrefix */
                                pAddress) ;

    if(rv != RV_OK)
    {
        RvLogError(pMsgMgr->pLogSrc, (pMsgMgr->pLogSrc,
            "MsgParserParseStandAloneAddress - Failed to parse address 0x%p", pAddress));
    }
    else
    {
        RvLogInfo(pMsgMgr->pLogSrc, (pMsgMgr->pLogSrc,
            "MsgParserParseStandAloneAddress - Finished to parse address 0x%p successfuly", pAddress));
    }

    return rv;
}

/*-------------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                              */
/*-------------------------------------------------------------------------*/

/*1: compact form */
static RvSipHeaderType RVCALLCONV GetHeaderType1char(IN RvChar* pHeaderStart,
                                                     OUT SipParseType *peSpecificHeaderType)
{
    switch(*pHeaderStart)
    {
    case 'B':case 'b':
        {
            *peSpecificHeaderType = SIP_PARSETYPE_REFERRED_BY;
            return RVSIP_HEADERTYPE_REFERRED_BY;
        }
    case 'C':case 'c':
        {
            *peSpecificHeaderType = SIP_PARSETYPE_CONTENT_TYPE;
            return RVSIP_HEADERTYPE_CONTENT_TYPE;
        }
    case 'F':case 'f':
        {
            *peSpecificHeaderType = SIP_PARSETYPE_FROM;
            return RVSIP_HEADERTYPE_FROM;
        }
    case 'I':case 'i':
        {
            *peSpecificHeaderType = SIP_PARSETYPE_CALL_ID;
            return RVSIP_HEADERTYPE_UNDEFINED;
        }
    case 'K':case 'k':
        {
            *peSpecificHeaderType = SIP_PARSETYPE_SUPPORTED;
            return RVSIP_HEADERTYPE_OTHER;
        }
    case 'M':case 'm':
        {
            *peSpecificHeaderType = SIP_PARSETYPE_CONTACT;
            return RVSIP_HEADERTYPE_CONTACT;
        }
    case 'R':case 'r':
        {
            *peSpecificHeaderType = SIP_PARSETYPE_REFER_TO;
            return RVSIP_HEADERTYPE_REFER_TO;
        }
    case 'U':case 'u':
        {
            *peSpecificHeaderType = SIP_PARSETYPE_ALLOW_EVENTS;
            return RVSIP_HEADERTYPE_ALLOW_EVENTS;
        }
    case 'X':case 'x':
        {
            *peSpecificHeaderType = SIP_PARSETYPE_SESSION_EXPIRES;
            return RVSIP_HEADERTYPE_SESSION_EXPIRES;
        }
    case 'O':case 'o':
        {
            *peSpecificHeaderType = SIP_PARSETYPE_EVENT;
            return RVSIP_HEADERTYPE_EVENT;
        }
    case 'T':case 't':
        {
            *peSpecificHeaderType = SIP_PARSETYPE_TO;
            return RVSIP_HEADERTYPE_TO;
        }
    case 'V':case 'v':
        {
            *peSpecificHeaderType = SIP_PARSETYPE_VIA;
            return RVSIP_HEADERTYPE_VIA;
        }
    case 'L':case 'l':
        {
            *peSpecificHeaderType = SIP_PARSETYPE_CONTENT_LENGTH;
            return RVSIP_HEADERTYPE_UNDEFINED;
        }
    default:
        break;
    }

    return RVSIP_HEADERTYPE_OTHER;
}
/* 2 chars : TO*/
static RvSipHeaderType RVCALLCONV GetHeaderType2chars(IN RvChar* pHeaderStart,
                                                      OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "To", 2))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_TO;
        return RVSIP_HEADERTYPE_TO;
    }
    return RVSIP_HEADERTYPE_OTHER;
}

/* 3 chars : VIA*/
static RvSipHeaderType RVCALLCONV GetHeaderType3chars(IN RvChar* pHeaderStart,
                                                      OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Via", 3))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_VIA;
         return RVSIP_HEADERTYPE_VIA;
    }
    return RVSIP_HEADERTYPE_OTHER;
}

/* 4: DATE,RSEQ,RACK,FROM,CSEQ,PATH*/
static RvSipHeaderType RVCALLCONV GetHeaderType4chars(IN RvChar* pHeaderStart,
                                                     OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Date", 4))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_DATE;
        return RVSIP_HEADERTYPE_DATE;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "From", 4))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_FROM;
        return RVSIP_HEADERTYPE_FROM;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "CSeq", 4))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_CSEQ;
        return RVSIP_HEADERTYPE_CSEQ;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "RAck", 4))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_RACK;
        return RVSIP_HEADERTYPE_RACK;
    }
    if (RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "RSeq", 4))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_RSEQ;
        return RVSIP_HEADERTYPE_RSEQ;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Path", 4))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_PATH;
        return RVSIP_HEADERTYPE_ROUTE_HOP;
    }

    return RVSIP_HEADERTYPE_OTHER;
}

/*5: ALLOW,ROUTE,EVENT,*/
static RvSipHeaderType RVCALLCONV GetHeaderType5chars(IN RvChar* pHeaderStart,
                                                      OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Allow", 5))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_ALLOW;
        return RVSIP_HEADERTYPE_ALLOW;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Route", 5))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_ROUTE;
        return RVSIP_HEADERTYPE_ROUTE_HOP;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Event", 5))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_EVENT;
        return RVSIP_HEADERTYPE_EVENT;
    }
    return RVSIP_HEADERTYPE_OTHER;
}

/* 6: REASON,MINSE,CALL-ID*/
static RvSipHeaderType RVCALLCONV GetHeaderType6chars(IN RvChar* pHeaderStart,
                                                      OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Reason", 6))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_REASON;
        return RVSIP_HEADERTYPE_REASON;
    }
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Accept", 6))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_ACCEPT;
        return RVSIP_HEADERTYPE_ACCEPT;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Min-SE", 6))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_MINSE;
        return RVSIP_HEADERTYPE_MINSE;
    }
    return RVSIP_HEADERTYPE_OTHER;
}

/*7: CONTACT,EXPIRES,REQUIRE*/
static RvSipHeaderType RVCALLCONV GetHeaderType7chars(IN RvChar* pHeaderStart,
                                                      OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Contact", 7))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_CONTACT;
        return RVSIP_HEADERTYPE_CONTACT;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Expires", 7))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_EXPIRES;
        return RVSIP_HEADERTYPE_EXPIRES;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Call-ID", 7))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_CALL_ID;
        return RVSIP_HEADERTYPE_UNDEFINED;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Require", 7))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_REQUIRE;
        return RVSIP_HEADERTYPE_OTHER;
    }
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Warning", 7))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_WARNING;
        return RVSIP_HEADERTYPE_WARNING;
    }
    return RVSIP_HEADERTYPE_OTHER;
}

/*8: REFER_TO,REPLACES,WARNING */
static RvSipHeaderType RVCALLCONV GetHeaderType8chars(IN RvChar* pHeaderStart,
                                                      OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Refer-To", 8))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_REFER_TO;
        return RVSIP_HEADERTYPE_REFER_TO;
    }
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Reply-To", 8))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_REPLY_TO;
        return RVSIP_HEADERTYPE_REPLY_TO;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Replaces", 8))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_REPLACES;
        return RVSIP_HEADERTYPE_REPLACES;
    }
    
    return RVSIP_HEADERTYPE_OTHER;
}

/*9: Supported */
static RvSipHeaderType RVCALLCONV GetHeaderType9chars(IN RvChar* pHeaderStart,
                                                       OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Supported", 9))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_SUPPORTED;
        return RVSIP_HEADERTYPE_OTHER;
    }
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Call-Info", 9))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_CALL_INFO;
        return RVSIP_HEADERTYPE_INFO;
    }
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Timestamp", 9))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_TIMESTAMP;
        return RVSIP_HEADERTYPE_TIMESTAMP;
    }
    
    return RVSIP_HEADERTYPE_OTHER;
}

/* 10: Alert-Info, Error-Info, P-DCS-OSPS, P-DCS-LAES */
static RvSipHeaderType RVCALLCONV GetHeaderType10chars(IN RvChar* pHeaderStart,
                                                       OUT SipParseType *peSpecificHeaderType)
{
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Alert-Info", 10))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_ALERT_INFO;
        return RVSIP_HEADERTYPE_INFO;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Error-Info", 10))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_ERROR_INFO;
        return RVSIP_HEADERTYPE_INFO;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Content-ID", 10))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_CONTENT_ID;
        return RVSIP_HEADERTYPE_CONTENT_ID;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-DCS-OSPS", 10))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_DCS_OSPS;
        return RVSIP_HEADERTYPE_P_DCS_OSPS;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-DCS-LAES", 10))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_DCS_LAES;
        return RVSIP_HEADERTYPE_P_DCS_LAES;
    }
    
    return RVSIP_HEADERTYPE_OTHER;
}

 /* 11: REFERRED_BY,RETRY_AFTER,Unsupported*/
static RvSipHeaderType RVCALLCONV GetHeaderType11chars(IN RvChar* pHeaderStart,
                                                       OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Referred-By", 11))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_REFERRED_BY;
        return RVSIP_HEADERTYPE_REFERRED_BY;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Retry-After", 11))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_RETRY_AFTER;
        return RVSIP_HEADERTYPE_RETRY_AFTER;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Unsupported", 11))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_UNSUPPORTED;
        return RVSIP_HEADERTYPE_OTHER;
    }
    
    return RVSIP_HEADERTYPE_OTHER;
}

/*  12: RECORD-ROUTE,ALLOW_EVENTS, CONTENT_TYPE, */
static RvSipHeaderType RVCALLCONV GetHeaderType12chars(IN RvChar* pHeaderStart,
                                                       OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Record-Route", 12))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_RECORD_ROUTE;
        return RVSIP_HEADERTYPE_ROUTE_HOP;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Allow-Events", 12))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_ALLOW_EVENTS;
        return RVSIP_HEADERTYPE_ALLOW_EVENTS;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Content-Type", 12))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_CONTENT_TYPE;
        return RVSIP_HEADERTYPE_CONTENT_TYPE;
    }
    return RVSIP_HEADERTYPE_OTHER;
}

/* 13: AUTHORIZATION,PROXY-REQUIRE,SERVICE_ROUTE*/
static RvSipHeaderType RVCALLCONV GetHeaderType13chars(IN RvChar* pHeaderStart,
                                                       OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Authorization", 13))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_AUTHORIZATION;
        return RVSIP_HEADERTYPE_AUTHORIZATION;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Proxy-Require", 13))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_PROXY_REQUIRE;
        return RVSIP_HEADERTYPE_OTHER;
    }
    
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Service-Route", 13))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_SERVICE_ROUTE;
        return RVSIP_HEADERTYPE_ROUTE_HOP;
    }
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-Profile-Key", 13))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_PROFILE_KEY;
        return RVSIP_HEADERTYPE_P_PROFILE_KEY;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-Served-User", 13))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_SERVED_USER;
        return RVSIP_HEADERTYPE_P_SERVED_USER;
    }
    return RVSIP_HEADERTYPE_OTHER;
}

/*  14: CONTENT-LENGTH, P-DCS-REDIRECT */
static RvSipHeaderType RVCALLCONV GetHeaderType14chars(IN RvChar* pHeaderStart,
                                                       OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Content-Length", 14))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_CONTENT_LENGTH;
        return RVSIP_HEADERTYPE_UNDEFINED;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-Answer-State", 14))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_ANSWER_STATE;
        return RVSIP_HEADERTYPE_P_ANSWER_STATE;
    }
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-DCS-Redirect", 14))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_DCS_REDIRECT;
        return RVSIP_HEADERTYPE_P_DCS_REDIRECT;
    }
    return RVSIP_HEADERTYPE_OTHER;
}
/*	15: SESSION_EXPIRES,ACCEPT-ENCODING,ACCEPT-LANGUAGE,
		SECURITY-CLIENT,SECURITY-SERVER,SECURITY-VERIFY	*/
static RvSipHeaderType RVCALLCONV GetHeaderType15chars(IN RvChar* pHeaderStart,
                                                       OUT SipParseType *peSpecificHeaderType)
{
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Accept-Encoding", 15))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_ACCEPT_ENCODING;
        return RVSIP_HEADERTYPE_ACCEPT_ENCODING;
    }
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Accept-Language", 15))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_ACCEPT_LANGUAGE;
        return RVSIP_HEADERTYPE_ACCEPT_LANGUAGE;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Session-Expires", 15))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_SESSION_EXPIRES;
        return RVSIP_HEADERTYPE_SESSION_EXPIRES;
    }
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Security-Client", 15))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_SECURITY_CLIENT;
        return RVSIP_HEADERTYPE_SECURITY;
	}
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Security-Server", 15))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_SECURITY_SERVER;
        return RVSIP_HEADERTYPE_SECURITY;
	}
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Security-Verify", 15))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_SECURITY_VERIFY;
        return RVSIP_HEADERTYPE_SECURITY;
	}
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-User-Database", 15))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_USER_DATABASE;
        return RVSIP_HEADERTYPE_P_USER_DATABASE;
	}
    return RVSIP_HEADERTYPE_OTHER;
}
  /*16: WWW-AUTHENTICATE, P-ASSOCIATED-URI*/
static RvSipHeaderType RVCALLCONV GetHeaderType16chars(IN RvChar* pHeaderStart,
                                                       OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "WWW-Authenticate", 16))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_WWW_AUTHENTICATE;
        return RVSIP_HEADERTYPE_AUTHENTICATION;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-Associated-URI", 16))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_ASSOCIATED_URI;
        return RVSIP_HEADERTYPE_P_URI;
    }
    return RVSIP_HEADERTYPE_OTHER;
}
/*17: P_CALLED_PARTY_ID, P-CHARGING-VECTOR */
static RvSipHeaderType RVCALLCONV GetHeaderType17chars(IN RvChar* pHeaderStart,
                                                       OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-Called-Party-ID", 17))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_CALLED_PARTY_ID;
        return RVSIP_HEADERTYPE_P_URI;
    }
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-Charging-Vector", 17))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_CHARGING_VECTOR;
        return RVSIP_HEADERTYPE_P_CHARGING_VECTOR;
    }
    return RVSIP_HEADERTYPE_OTHER;
}
/*  18: SUBSCRIPTION_STATE, PROXY-AUTHENTICATE, P-DCS-BILLING-INFO*/
static RvSipHeaderType RVCALLCONV GetHeaderType18chars(IN RvChar* pHeaderStart,
                                                       OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Subscription-State", 18))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_SUBSCRIPTION_STATE;
        return RVSIP_HEADERTYPE_SUBSCRIPTION_STATE;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Proxy-Authenticate", 18))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_PROXY_AUTHENTICATE;
        return RVSIP_HEADERTYPE_AUTHENTICATION;
    }
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-DCS-Billing-Info", 18))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_DCS_BILLING_INFO;
        return RVSIP_HEADERTYPE_P_DCS_BILLING_INFO;
    }
    return RVSIP_HEADERTYPE_OTHER;
}

/*  19: CONTENT_DISPOSITION,AUTHENTICATION_INFO,PROXY-AUTHORIZATION,P_ASSERTED_IDENTITY */
static RvSipHeaderType RVCALLCONV GetHeaderType19chars(IN RvChar* pHeaderStart,
                                                       OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Content-Disposition", 19))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_CONTENT_DISPOSITION;
         return RVSIP_HEADERTYPE_CONTENT_DISPOSITION;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Authentication-Info", 19))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_AUTHENTICATION_INFO;
        return RVSIP_HEADERTYPE_AUTHENTICATION_INFO;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "Proxy-Authorization", 19))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_PROXY_AUTHORIZATION;
        return RVSIP_HEADERTYPE_AUTHORIZATION;
    }    
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-Asserted-Identity", 19))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_ASSERTED_IDENTITY;
        return RVSIP_HEADERTYPE_P_URI;
    }
    return RVSIP_HEADERTYPE_OTHER;
}
/*  20: P_PREFERRED_IDENTITY, P_VISITED_NETWORK_ID */
static RvSipHeaderType RVCALLCONV GetHeaderType20chars(IN RvChar* pHeaderStart,
                                                       OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-Preferred-Identity", 20))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_PREFERRED_IDENTITY;
         return RVSIP_HEADERTYPE_P_URI;
    }
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-Visited-Network-ID", 20))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_VISITED_NETWORK_ID;
         return RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID;
    }
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-DCS-Trace-Party-ID", 20))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_DCS_TRACE_PARTY_ID;
         return RVSIP_HEADERTYPE_P_DCS_TRACE_PARTY_ID;
    }
    return RVSIP_HEADERTYPE_OTHER;
}
/*  21: P-ACCESS-NETWORK-INFO, P-MEDIA-AUTHORIZATION */
static RvSipHeaderType RVCALLCONV GetHeaderType21chars(IN RvChar* pHeaderStart,
                                                       OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-Access-Network-Info", 21))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_ACCESS_NETWORK_INFO;
         return RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO;
    }
	if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-Media-Authorization", 21))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_MEDIA_AUTHORIZATION;
         return RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION;
    }
	    
    return RVSIP_HEADERTYPE_OTHER;
}
/*  29: P-CHARGING-FUNCTION-ADDRESSES */
static RvSipHeaderType RVCALLCONV GetHeaderType29chars(IN RvChar* pHeaderStart,
                                                       OUT SipParseType *peSpecificHeaderType)
{
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderStart, "P-Charging-Function-Addresses", 29))
    {
        *peSpecificHeaderType = SIP_PARSETYPE_P_CHARGING_FUNCTION_ADDRESSES;
         return RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES;
    }
	
    return RVSIP_HEADERTYPE_OTHER;
} 

/***************************************************************************
 * ConvertHeaderSpecificTypeByCompilation
 * ------------------------------------------------------------------------
 * General: The function converts header specific types according to the compilation
 *          flags. for example:
 *          if the stack was compiled with RV_SIP_PRIMITIVES then option-tag
 *          headers must be parsed as a regular other header.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *    eHeaderType     - The header type we found in the given buffer.
 ***************************************************************************/
static SipParseType RVCALLCONV ConvertHeaderSpecificTypeByCompilation(IN SipParseType eType)
{
#ifdef RV_SIP_PRIMITIVES
    /* in RV_SIP_PRIMITIVES compilation, only the route, and via headers have specific implementation */
    if(IsOptionTagHeader(eType) == RV_TRUE)
    {
        return SIP_PARSETYPE_OTHER;
    }
#endif /*#ifdef RV_SIP_PRIMITIVES*/
    return eType;
}
/***************************************************************************
 * ConvertHeaderTypeByCompilation
 * ------------------------------------------------------------------------
 * General: The function converts header types according to the compilation
 *          flags. for example:
 *          if we found that the given header is 'WWW-Authenticate', but the 
 *          stack was not compiled with RV_SIP_AUTH_ON, then we must treat
 *          it as Other header, and not as an authentication header...
 * Return Value: header-type according to the compilation flags.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *    eHeaderType     - The header type we found in the given buffer.
 *    eSpecificHeaderType - The specific header type, e.g. the type of Route-Hop,
 *                          the type of Party header, etc.
 ***************************************************************************/
static RvSipHeaderType RVCALLCONV ConvertHeaderTypeByCompilation(
													IN RvSipHeaderType eHeaderType,
													IN SipParseType    eSpecificHeaderType)
{
#ifndef RV_SIP_JSR32_SUPPORT
	RV_UNUSED_ARG(eSpecificHeaderType)
#endif /*RV_SIP_JSR32_SUPPORT*/
#ifdef RV_SIP_LIGHT
    /* in SIP_LIGHT compilation, only the route, and via headers have specific implementation */
    if(eHeaderType != RVSIP_HEADERTYPE_ROUTE_HOP &&
       eHeaderType != RVSIP_HEADERTYPE_VIA)   
    {
        return RVSIP_HEADERTYPE_OTHER;
    }
#endif /*#ifdef RV_SIP_LIGHT*/
 
    switch (eHeaderType)
    {
#ifndef RV_SIP_AUTH_ON
        case RVSIP_HEADERTYPE_AUTHENTICATION:
        case RVSIP_HEADERTYPE_AUTHORIZATION:
		case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
            return RVSIP_HEADERTYPE_OTHER;
#endif /* #ifdef RV_SIP_AUTH_ON */
#ifndef RV_SIP_SUBS_ON
        case RVSIP_HEADERTYPE_REFER_TO:
        case RVSIP_HEADERTYPE_ALLOW_EVENTS:
        case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
        	return RVSIP_HEADERTYPE_OTHER;
#endif /* #ifdef RV_SIP_SUBS_ON */
#ifdef RV_SIP_PRIMITIVES
        case RVSIP_HEADERTYPE_EVENT:
        case RVSIP_HEADERTYPE_ALLOW:
        case RVSIP_HEADERTYPE_RSEQ:
        case RVSIP_HEADERTYPE_RACK:
        case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
        case RVSIP_HEADERTYPE_RETRY_AFTER:
        case RVSIP_HEADERTYPE_CONTENT_TYPE:
        case RVSIP_HEADERTYPE_CONTENT_ID:
		    return RVSIP_HEADERTYPE_OTHER;
#endif /* RV_SIP_PRIMITIVES */
#if (defined RV_SIP_JSR32_SUPPORT || defined RV_SIP_PRIMITIVES)
		case RVSIP_HEADERTYPE_SESSION_EXPIRES:
        case RVSIP_HEADERTYPE_REPLACES:
        case RVSIP_HEADERTYPE_MINSE:
			return RVSIP_HEADERTYPE_OTHER;
#endif /* #if (defined RV_SIP_JSR32_SUPPORT || defined RV_SIP_PRIMITIVES) */
#if (defined RV_SIP_JSR32_SUPPORT || !defined RV_SIP_SUBS_ON)
		case RVSIP_HEADERTYPE_REFERRED_BY:
			return RVSIP_HEADERTYPE_OTHER;
#endif /* #if (defined RV_SIP_JSR32_SUPPORT || !defined RV_SIP_SUBS_ON) */
#ifdef RV_SIP_JSR32_SUPPORT
		case RVSIP_HEADERTYPE_ROUTE_HOP:
		{
			if (eSpecificHeaderType == SIP_PARSETYPE_PATH || eSpecificHeaderType == SIP_PARSETYPE_SERVICE_ROUTE)
			{
				return RVSIP_HEADERTYPE_OTHER;
			}
			return eHeaderType;
		}
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */
#ifndef RV_SIP_EXTENDED_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_ACCEPT:
		case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
		case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
		case RVSIP_HEADERTYPE_INFO:
        case RVSIP_HEADERTYPE_REASON:
		case RVSIP_HEADERTYPE_REPLY_TO:
		case RVSIP_HEADERTYPE_WARNING:
		case RVSIP_HEADERTYPE_TIMESTAMP:
			return RVSIP_HEADERTYPE_OTHER;
		/* XXX */
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */
#ifndef RV_SIP_IMS_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_P_URI:
		case RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID:
		case RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO:
		case RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES:
		case RVSIP_HEADERTYPE_P_CHARGING_VECTOR:
		case RVSIP_HEADERTYPE_SECURITY:
		case RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION:
        case RVSIP_HEADERTYPE_P_PROFILE_KEY:
        case RVSIP_HEADERTYPE_P_USER_DATABASE:
        case RVSIP_HEADERTYPE_P_ANSWER_STATE:
        case RVSIP_HEADERTYPE_P_SERVED_USER:
			return RVSIP_HEADERTYPE_OTHER;
#endif /* #ifndef RV_SIP_IMS_HEADER_SUPPORT */
#ifndef RV_SIP_IMS_DCS_HEADER_SUPPORT
		case RVSIP_HEADERTYPE_P_DCS_TRACE_PARTY_ID:
		case RVSIP_HEADERTYPE_P_DCS_OSPS:
		case RVSIP_HEADERTYPE_P_DCS_BILLING_INFO:
		case RVSIP_HEADERTYPE_P_DCS_LAES:
		case RVSIP_HEADERTYPE_P_DCS_REDIRECT:
			return RVSIP_HEADERTYPE_OTHER;
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
       case RVSIP_HEADERTYPE_UNDEFINED:
        default:
        {
            return eHeaderType;
        }
    }
}


/***************************************************************************
 * GetHeaderType
 * ------------------------------------------------------------------------
 * General: The function retrieves the type of a given header string.
 * Return Value: header-type.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:
 *    pHeaderStart     - Pointer to the start of the header string.
 *    pHeaderNameEnd   - Pointer to the end of the header name (before the colon).
 *    headerNameLen    - Length of the header name.
 *  Output:
 *    pbCompactForm    - Is the header type is in it's compact form or not.
 *    peSpecificHeaderType - specific type of header. This argument is relevant 
 *                      for headers that holds several types in a single object.
 *                      (e.g. party, route-hop, authentication, authorization)
 ***************************************************************************/
static RvSipHeaderType RVCALLCONV GetHeaderType( IN RvChar* pHeaderStart, 
                                                 IN RvChar* pHeaderNameEnd,
                                                 IN RvInt32 headerNameLen,
                                                 OUT RvBool* pbCompactForm,
                                                 OUT SipParseType *peSpecificHeaderType)
{
    RvInt32 len = headerNameLen;
    RvSipHeaderType eHeaderType = RVSIP_HEADERTYPE_OTHER;

    *pbCompactForm = RV_FALSE;
    *peSpecificHeaderType = SIP_PARSETYPE_UNDEFINED;
   
    /* remove spaces and tabs at the end of the header */
    if(NULL == pHeaderStart)
    {
        /* exception */
        return RVSIP_HEADERTYPE_UNDEFINED;
    }

    if(NULL == pHeaderNameEnd)
    {
        /*header that do not have a colon is a bad-syntax other header */
        return RVSIP_HEADERTYPE_OTHER;
    }
    
    /* go back over spaces or tabs that are between the header name and the colon */
    while((*pHeaderNameEnd == SPACE_CHAR || *pHeaderNameEnd == TAB_CHAR) && 
           (pHeaderNameEnd != pHeaderStart))
    {
        --len;
        --pHeaderNameEnd;
    }   

    /* find the header type according to the header name length */
    switch(len)
    {
    case 1:
        *pbCompactForm = RV_TRUE;
        eHeaderType = GetHeaderType1char(pHeaderStart, peSpecificHeaderType);
        break;
    case 2:
        eHeaderType = GetHeaderType2chars(pHeaderStart,peSpecificHeaderType);
        break;
    case 3:
        eHeaderType = GetHeaderType3chars(pHeaderStart, peSpecificHeaderType);
        break;
    case 4:
        eHeaderType = GetHeaderType4chars(pHeaderStart,peSpecificHeaderType);
        break;
    case 5:
        eHeaderType = GetHeaderType5chars(pHeaderStart,peSpecificHeaderType);
        break;
    case 6:
        eHeaderType = GetHeaderType6chars(pHeaderStart, peSpecificHeaderType);
        break;
    case 7:
        eHeaderType = GetHeaderType7chars(pHeaderStart, peSpecificHeaderType);
        break;
    case 8:
        eHeaderType = GetHeaderType8chars(pHeaderStart, peSpecificHeaderType);
        break;
    case 9:
        eHeaderType = GetHeaderType9chars(pHeaderStart, peSpecificHeaderType);
        break;
	case 10:
		eHeaderType =  GetHeaderType10chars(pHeaderStart, peSpecificHeaderType);
        break;
    case 11:
        eHeaderType = GetHeaderType11chars(pHeaderStart, peSpecificHeaderType);
        break;
    case 12:
        eHeaderType = GetHeaderType12chars(pHeaderStart,peSpecificHeaderType);
        break;
    case 13:
        eHeaderType = GetHeaderType13chars(pHeaderStart,peSpecificHeaderType);
        break;
    case 14:
        eHeaderType = GetHeaderType14chars(pHeaderStart, peSpecificHeaderType);
        break;
    case 15:
        eHeaderType = GetHeaderType15chars(pHeaderStart, peSpecificHeaderType);
        break;
    case 16:
        eHeaderType = GetHeaderType16chars(pHeaderStart, peSpecificHeaderType);
        break;
	case 17:
        eHeaderType = GetHeaderType17chars(pHeaderStart, peSpecificHeaderType);
        break;
    case 18:
        eHeaderType = GetHeaderType18chars(pHeaderStart, peSpecificHeaderType);
        break;
    case 19:
        eHeaderType = GetHeaderType19chars(pHeaderStart, peSpecificHeaderType);
        break;
	case 20:
        eHeaderType = GetHeaderType20chars(pHeaderStart, peSpecificHeaderType);
        break;
	case 21:
        eHeaderType = GetHeaderType21chars(pHeaderStart, peSpecificHeaderType);
        break;
	case 29:
        eHeaderType = GetHeaderType29chars(pHeaderStart, peSpecificHeaderType);
        break;
    default:
        eHeaderType = RVSIP_HEADERTYPE_OTHER;
    }
    eHeaderType = ConvertHeaderTypeByCompilation(eHeaderType, *peSpecificHeaderType);
    *peSpecificHeaderType = ConvertHeaderSpecificTypeByCompilation(*peSpecificHeaderType);
    return eHeaderType;
}

/***************************************************************************
 * SetCompactFormInBSHeader
 * ------------------------------------------------------------------------
 * General: The function sets the compact-form boolean in a bad-syntax header.
 *
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:     eHeaderType - type of header
 *             pHeader     - pointer to the header.
 ***************************************************************************/
static RvStatus RVCALLCONV SetCompactFormInBSHeader(void*           pHeader,
                                                  RvSipHeaderType eHeaderType)
{
    switch(eHeaderType)
    {
     case RVSIP_HEADERTYPE_VIA:
        return RvSipViaHeaderSetCompactForm((RvSipViaHeaderHandle)pHeader, RV_TRUE);
#ifndef RV_SIP_LIGHT
    case RVSIP_HEADERTYPE_CONTACT:
        return RvSipContactHeaderSetCompactForm((RvSipContactHeaderHandle)pHeader, RV_TRUE);
    case RVSIP_HEADERTYPE_TO:
    case RVSIP_HEADERTYPE_FROM:
        return RvSipPartyHeaderSetCompactForm((RvSipPartyHeaderHandle)pHeader, RV_TRUE);
#endif /*#ifndef RV_SIP_LIGHT*/
#ifndef RV_SIP_PRIMITIVES
    case RVSIP_HEADERTYPE_SESSION_EXPIRES:
        return RvSipSessionExpiresHeaderSetCompactForm((RvSipSessionExpiresHeaderHandle)pHeader, RV_TRUE);
    case RVSIP_HEADERTYPE_CONTENT_TYPE:
        return RvSipContentTypeHeaderSetCompactForm((RvSipContentTypeHeaderHandle)pHeader, RV_TRUE);
    case RVSIP_HEADERTYPE_EVENT:
            return RvSipEventHeaderSetCompactForm((RvSipEventHeaderHandle)pHeader, RV_TRUE);
#ifdef RV_SIP_SUBS_ON
    case RVSIP_HEADERTYPE_REFER_TO:
        return RvSipReferToHeaderSetCompactForm((RvSipReferToHeaderHandle)pHeader, RV_TRUE);
    case RVSIP_HEADERTYPE_REFERRED_BY:
        return RvSipReferredByHeaderSetCompactForm((RvSipReferredByHeaderHandle)pHeader, RV_TRUE);
    case RVSIP_HEADERTYPE_ALLOW_EVENTS:
        return RvSipAllowEventsHeaderSetCompactForm((RvSipAllowEventsHeaderHandle)pHeader, RV_TRUE);
#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /* RV_SIP_PRIMITIVES */
    default:
        break;
    }
    return RV_OK;

}

/***************************************************************************
 * SetBsHeaderSpecificType
 * ------------------------------------------------------------------------
 * General: This function sets the header-type for headers structures that
 *          are used for more than one header type (route/record-route,
 *          authorization/proxy-authorization, www-authenticate/proxy-authenticate).
 *          This function is used only for bad-syntax headers.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: pHeader      - Pointer to the already constructed header.
 *           eSpecificHeaderType - The header specific type.
 ***************************************************************************/
static void RVCALLCONV SetBsHeaderSpecificType(IN void*          pHeader,
                                               IN SipParseType    eSpecificHeaderType)
{
    switch(eSpecificHeaderType)
    {
    case SIP_PARSETYPE_PATH:
        RvSipRouteHopHeaderSetHeaderType((RvSipRouteHopHeaderHandle)pHeader, RVSIP_ROUTE_HOP_PATH_HEADER);
        break;
    case SIP_PARSETYPE_SERVICE_ROUTE:
        RvSipRouteHopHeaderSetHeaderType((RvSipRouteHopHeaderHandle)pHeader, RVSIP_ROUTE_HOP_SERVICE_ROUTE_HEADER);
        break;

    case SIP_PARSETYPE_RECORD_ROUTE:
        RvSipRouteHopHeaderSetHeaderType((RvSipRouteHopHeaderHandle)pHeader, RVSIP_ROUTE_HOP_RECORD_ROUTE_HEADER);
        break;
    case SIP_PARSETYPE_ROUTE:
        RvSipRouteHopHeaderSetHeaderType((RvSipRouteHopHeaderHandle)pHeader, RVSIP_ROUTE_HOP_ROUTE_HEADER);  
        break;
#ifndef RV_SIP_LIGHT
    case SIP_PARSETYPE_FROM:
        RvSipPartyHeaderSetType((RvSipPartyHeaderHandle)pHeader, RVSIP_HEADERTYPE_FROM);
        break;
    case SIP_PARSETYPE_TO:
        RvSipPartyHeaderSetType((RvSipPartyHeaderHandle)pHeader, RVSIP_HEADERTYPE_TO);
        break;
#endif /*#ifndef RV_SIP_LIGHT*/
#ifdef RV_SIP_AUTH_ON
    case SIP_PARSETYPE_AUTHORIZATION:
        RvSipAuthorizationHeaderSetHeaderType((RvSipAuthorizationHeaderHandle)pHeader,
                                                    RVSIP_AUTH_AUTHORIZATION_HEADER);
        break;
    case SIP_PARSETYPE_PROXY_AUTHORIZATION:
        RvSipAuthorizationHeaderSetHeaderType((RvSipAuthorizationHeaderHandle)pHeader,
                                                 RVSIP_AUTH_PROXY_AUTHORIZATION_HEADER);
        break;
    case SIP_PARSETYPE_WWW_AUTHENTICATE:
        RvSipAuthenticationHeaderSetHeaderType((RvSipAuthenticationHeaderHandle)pHeader,
                                               RVSIP_AUTH_WWW_AUTHENTICATION_HEADER);
        break;
    case SIP_PARSETYPE_PROXY_AUTHENTICATE:
        RvSipAuthenticationHeaderSetHeaderType((RvSipAuthenticationHeaderHandle)pHeader,
                                                RVSIP_AUTH_PROXY_AUTHENTICATION_HEADER);
        break;
#endif /* #ifdef RV_SIP_AUTH_ON */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
	case SIP_PARSETYPE_P_ASSOCIATED_URI:
        RvSipPUriHeaderSetPHeaderType((RvSipPUriHeaderHandle)pHeader,
                                       RVSIP_P_URI_ASSOCIATED_URI_HEADER);
        break;
	case SIP_PARSETYPE_P_CALLED_PARTY_ID:
		RvSipPUriHeaderSetPHeaderType((RvSipPUriHeaderHandle)pHeader,
                                       RVSIP_P_URI_CALLED_PARTY_ID_HEADER);
        break;
	case SIP_PARSETYPE_P_ASSERTED_IDENTITY:
		RvSipPUriHeaderSetPHeaderType((RvSipPUriHeaderHandle)pHeader,
                                       RVSIP_P_URI_ASSERTED_IDENTITY_HEADER);
        break;
	case SIP_PARSETYPE_P_PREFERRED_IDENTITY:
		RvSipPUriHeaderSetPHeaderType((RvSipPUriHeaderHandle)pHeader,
                                       RVSIP_P_URI_PREFERRED_IDENTITY_HEADER);
        break;
	case SIP_PARSETYPE_SECURITY_CLIENT:
		RvSipSecurityHeaderSetSecurityHeaderType((RvSipSecurityHeaderHandle)pHeader,
                                       RVSIP_SECURITY_CLIENT_HEADER);
        break;
	case SIP_PARSETYPE_SECURITY_SERVER:
		RvSipSecurityHeaderSetSecurityHeaderType((RvSipSecurityHeaderHandle)pHeader,
                                       RVSIP_SECURITY_SERVER_HEADER);
        break;
	case SIP_PARSETYPE_SECURITY_VERIFY:
		RvSipSecurityHeaderSetSecurityHeaderType((RvSipSecurityHeaderHandle)pHeader,
                                       RVSIP_SECURITY_VERIFY_HEADER);
        break;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 
    default:
            break;
    }
}

/***************************************************************************
 * UpdateStandAloneHeaderSpecificType
 * ------------------------------------------------------------------------
 * General: This function updates the specific-type of a stand alone header 
 *          according to the given string to parse.
 *          Example: 
 *          application created a route-hop header by calling to
 *          RvSipRouteHopHeaderConstructInMsg().
 *          now application parses a header string to it:
 *          RvSipRouteHopHeaderParse(h, "Record-Route:<sip:127.0.0.1:5060;lr>").
 *          We need to update the header-type to RVSIP_ROUTE_HOP_RECORD_ROUTE_HEADER
 *          here, and not to force application to use RvSipRouteHopHeaderSetHeaderType() 
 *          
 * Return Value: RV_OK        - on success
 *               RV_ERROR_UNKNOWN - If the header specific type is not allowed for this
 *                              kind of header (parse call-id into route-hop header...)
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: pHeader      - Pointer to the already constructed header.
 *           eSpecificHeaderType - The header specific type.
 ***************************************************************************/
static RvStatus RVCALLCONV UpdateStandAloneHeaderSpecificType(IN void*          pHeader,
                                                          IN RvSipHeaderType eType,
                                                          IN SipParseType    eSpecificHeaderType)
{
    switch(eType)
    {
    case RVSIP_HEADERTYPE_ROUTE_HOP:
        switch(eSpecificHeaderType)
        {
        case SIP_PARSETYPE_PATH:
            RvSipRouteHopHeaderSetHeaderType((RvSipRouteHopHeaderHandle)pHeader, RVSIP_ROUTE_HOP_PATH_HEADER);
            break;
        case SIP_PARSETYPE_SERVICE_ROUTE:
            RvSipRouteHopHeaderSetHeaderType((RvSipRouteHopHeaderHandle)pHeader, RVSIP_ROUTE_HOP_SERVICE_ROUTE_HEADER);
            break;
        case SIP_PARSETYPE_RECORD_ROUTE:
            RvSipRouteHopHeaderSetHeaderType((RvSipRouteHopHeaderHandle)pHeader, RVSIP_ROUTE_HOP_RECORD_ROUTE_HEADER);
            break;
        case SIP_PARSETYPE_ROUTE:
            RvSipRouteHopHeaderSetHeaderType((RvSipRouteHopHeaderHandle)pHeader, RVSIP_ROUTE_HOP_ROUTE_HEADER);  
            break;
        default:
            return RV_ERROR_UNKNOWN;
        }
        return RV_OK;
#ifndef RV_SIP_LIGHT
    case RVSIP_HEADERTYPE_TO:
    case RVSIP_HEADERTYPE_FROM:
        switch(eSpecificHeaderType)
        {
        case SIP_PARSETYPE_FROM:
            RvSipPartyHeaderSetType((RvSipPartyHeaderHandle)pHeader, RVSIP_HEADERTYPE_FROM);
            break;
        case SIP_PARSETYPE_TO:
            RvSipPartyHeaderSetType((RvSipPartyHeaderHandle)pHeader, RVSIP_HEADERTYPE_TO);
            break;
        default:
            return RV_ERROR_UNKNOWN;
        }
        return RV_OK;
#endif /*#ifndef RV_SIP_LIGHT*/
#ifdef RV_SIP_AUTH_ON
    case RVSIP_HEADERTYPE_AUTHENTICATION:
        switch(eSpecificHeaderType)
        {
        case SIP_PARSETYPE_WWW_AUTHENTICATE:
            RvSipAuthenticationHeaderSetHeaderType((RvSipAuthenticationHeaderHandle)pHeader,
                                                    RVSIP_AUTH_WWW_AUTHENTICATION_HEADER);
        break;
        case SIP_PARSETYPE_PROXY_AUTHENTICATE:
            RvSipAuthenticationHeaderSetHeaderType((RvSipAuthenticationHeaderHandle)pHeader,
                                                RVSIP_AUTH_PROXY_AUTHENTICATION_HEADER);
        break;
        default:
            return RV_ERROR_UNKNOWN;
        }
        return RV_OK;
    case RVSIP_HEADERTYPE_AUTHORIZATION:
        switch(eSpecificHeaderType)
        {
        case SIP_PARSETYPE_AUTHORIZATION:
            RvSipAuthorizationHeaderSetHeaderType((RvSipAuthorizationHeaderHandle)pHeader,
                                                   RVSIP_AUTH_AUTHORIZATION_HEADER);
        break;
        case SIP_PARSETYPE_PROXY_AUTHORIZATION:
            RvSipAuthorizationHeaderSetHeaderType((RvSipAuthorizationHeaderHandle)pHeader,
                                                   RVSIP_AUTH_PROXY_AUTHORIZATION_HEADER);
        break;
        default:
            return RV_ERROR_UNKNOWN;
		
        }
        return RV_OK;
#ifdef RV_SIP_IMS_HEADER_SUPPORT
	case RVSIP_HEADERTYPE_P_URI:
        switch(eSpecificHeaderType)
        {
        case SIP_PARSETYPE_P_ASSOCIATED_URI:
            RvSipPUriHeaderSetPHeaderType((RvSipPUriHeaderHandle)pHeader,
                                                   RVSIP_P_URI_ASSOCIATED_URI_HEADER);
        break;
        case SIP_PARSETYPE_P_CALLED_PARTY_ID:
            RvSipPUriHeaderSetPHeaderType((RvSipPUriHeaderHandle)pHeader,
                                                   RVSIP_P_URI_CALLED_PARTY_ID_HEADER);
        break;
		case SIP_PARSETYPE_P_ASSERTED_IDENTITY:
            RvSipPUriHeaderSetPHeaderType((RvSipPUriHeaderHandle)pHeader,
                                                   RVSIP_P_URI_ASSERTED_IDENTITY_HEADER);
        break;
		case SIP_PARSETYPE_P_PREFERRED_IDENTITY:
            RvSipPUriHeaderSetPHeaderType((RvSipPUriHeaderHandle)pHeader,
                                                   RVSIP_P_URI_PREFERRED_IDENTITY_HEADER);
        break;
        default:
            return RV_ERROR_UNKNOWN;
        }
        return RV_OK;
	case RVSIP_HEADERTYPE_SECURITY:
        switch(eSpecificHeaderType)
        {
        case SIP_PARSETYPE_SECURITY_CLIENT:
            RvSipSecurityHeaderSetSecurityHeaderType((RvSipSecurityHeaderHandle)pHeader,
                                                   RVSIP_SECURITY_CLIENT_HEADER);
        break;
        case SIP_PARSETYPE_SECURITY_SERVER:
            RvSipSecurityHeaderSetSecurityHeaderType((RvSipSecurityHeaderHandle)pHeader,
                                                   RVSIP_SECURITY_SERVER_HEADER);
        break;
		case SIP_PARSETYPE_SECURITY_VERIFY:
            RvSipSecurityHeaderSetSecurityHeaderType((RvSipSecurityHeaderHandle)pHeader,
                                                   RVSIP_SECURITY_VERIFY_HEADER);
        break;
        default:
            return RV_ERROR_UNKNOWN;
		
        }
        return RV_OK;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 
#endif /* #ifdef RV_SIP_AUTH_ON */
    default:
        return RV_OK;
    }
}
/***************************************************************************
 * SetBsViaTransport
 * ------------------------------------------------------------------------
 * General: The function tries to rescue the transport parameter from a 
 *          bad-syntax via header
 *          sent-protocol = protocol-name "/" protocol-version "/" transport 
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:   pHeader      - Pointer to the via header.
 *             pHeaderVal   - Pointer to the header value string.
 ***************************************************************************/
static RvStatus RVCALLCONV SetBsViaTransport(IN void*   pHeader, IN RvChar* pHeaderVal)
{
    RvSipViaHeaderHandle hVia = (RvSipViaHeaderHandle)pHeader;
    RvInt                numOfSlashes=0;
    RvSipTransport       eTransport = RVSIP_TRANSPORT_UNDEFINED;

    /* pass the beginning of the header - until finding the second slash */
    while (*pHeaderVal != MSG_NULL_CHAR && numOfSlashes<2)
    {
        if(*pHeaderVal == '/')
        {
            ++numOfSlashes;
        }
        ++pHeaderVal;
    }
    if(*pHeaderVal == MSG_NULL_CHAR)
    {
        /* the end of the header - defected header. nothing to to */
        return RV_ERROR_NOT_FOUND;
    }

    /* skip any spaces */
    while (*pHeaderVal != MSG_NULL_CHAR &&
          (*pHeaderVal == SPACE_CHAR || *pHeaderVal == TAB_CHAR))
    {
        ++pHeaderVal;
    }

    if(*pHeaderVal == MSG_NULL_CHAR)
    {
        /* the end of the header - defected header. nothing to to */
        return RV_ERROR_NOT_FOUND;
    }
    
    /* set the transport */
    if(RV_TRUE == SipCommonStricmpByLen(pHeaderVal, "UDP", 3))
    {
        eTransport = RVSIP_TRANSPORT_UDP;
    }
    else if(RV_TRUE == SipCommonStricmpByLen(pHeaderVal, "TCP", 3))
    {
        eTransport = RVSIP_TRANSPORT_TCP;
    }
    else if(RV_TRUE == SipCommonStricmpByLen(pHeaderVal, "SCTP", 4))
    {
        eTransport = RVSIP_TRANSPORT_SCTP;
    }
    else if(RV_TRUE == SipCommonStricmpByLen(pHeaderVal, "TLS", 3))
    {
        eTransport = RVSIP_TRANSPORT_TLS;
    }

    if(RVSIP_TRANSPORT_UNDEFINED != eTransport)
    {
        RvSipViaHeaderSetTransport(hVia, eTransport, NULL);
        return RV_OK;
    }
    else
    {
        return RV_ERROR_NOT_FOUND;
    }
}



/***************************************************************************
* IsOptionTagHeader
* ------------------------------------------------------------------------
* General: Defines if the given header type is one of the option-tag headers
*          or not.
* Return Value: RV_TRUE - option tag header.
*               RV_FALSE - not option tag header
* ------------------------------------------------------------------------
* Arguments:
*    Input:   
***************************************************************************/
static RvBool IsOptionTagHeader(SipParseType eSpecificHeaderType)
{
    if(eSpecificHeaderType == SIP_PARSETYPE_SUPPORTED ||
        eSpecificHeaderType == SIP_PARSETYPE_UNSUPPORTED ||
        eSpecificHeaderType == SIP_PARSETYPE_REQUIRE ||
        eSpecificHeaderType == SIP_PARSETYPE_PROXY_REQUIRE)
    {
        return RV_TRUE;
    }
    else
    {
        return RV_FALSE;
    }
}

/***************************************************************************
* ParseBadSyntaxHeader
* ------------------------------------------------------------------------
* General: The function handles the bad-syntax case.
*          if the parser returned error when trying to parse a message line,
*          this function is called. The function constructs the header 
*          object (according to the header type), and saves the whole
*          header value (containing the syntax errors) in it.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input: pMsg           - Pointer to the message object.
*           eHeaderType    - The type of the bad-syntax header.
*           bCompactForm   - Does the name of the bad-syntax header is in compact form
*           eSpecificHeaderType - The specific type of the bad-syntax header.
*           pHeaderStart   - Pointer to the beginning of the header string.
*           pHeaderNameEnd - Pointer to the end of the header name (if NULL -
*                            it means that we did not find a colon character).
*           pHeaderVal     - Pointer to the beginning of the header value.
***************************************************************************/
static RvStatus RVCALLCONV ParseBadSyntaxHeader( IN MsgMessage*     pMsg,
                                                 IN RvSipHeaderType eHeaderType,
                                                 IN RvBool          bCompactForm,
                                                 IN SipParseType    eSpecificHeaderType,
                                                 IN RvChar*         pHeaderStart,
                                                 IN RvChar*         pHeaderNameEnd,  
                                                 IN RvChar*         pHeaderVal)
{
    RvStatus        rv;
    void*           pHeader = NULL;
                                                 
    
    /* ==========================================
        Handle 2 special cases 
       ==========================================*/
    /* Special case (1) - no colon in header.
          in this case we insert the whole header as a header name of other header */
    if(pHeaderNameEnd == NULL)
    {
        /* we did not find the colon */
        rv = RvSipOtherHeaderConstructInMsg((RvSipMsgHandle)pMsg, RV_FALSE, (RvSipOtherHeaderHandle*)&pHeader);
        if(rv != RV_OK)
        {
            return rv;
        }
        rv = RvSipOtherHeaderSetName((RvSipOtherHeaderHandle)pHeader, pHeaderStart);
        if(rv != RV_OK)
        {
            return rv;
        }
        return RV_OK;
    }

    /* Special case (2) - Call-Id.
          other header, which is set directly to the message, and is not set to the message headers list */
    if(SIP_PARSETYPE_CALL_ID == eSpecificHeaderType)
    {
        /* change the Call Id in a token form to a string value */
        if (bCompactForm == RV_TRUE)
        {
            RvSipMsgSetCallIdCompactForm((RvSipMsgHandle)pMsg,RV_TRUE);
        }
        /* Set the string Call Id into the message */
        return RvSipMsgSetCallIdHeader((RvSipMsgHandle)pMsg, pHeaderVal);
    }

    /* Special case (3) - Content-Length.
       numeric value, which is set directly to the message, and is not set to the message headers list */
    if(SIP_PARSETYPE_CONTENT_LENGTH == eSpecificHeaderType)
    {
        RvInt32 ContentLength = UNDEFINED;
        
        ContentLength = atoi(pHeaderVal);

        pMsg->bBadSyntaxContentLength = RV_TRUE;
        
        /* Set the string Call Id into the message */
        return RvSipMsgSetContentLengthHeader((RvSipMsgHandle)pMsg, ContentLength);
    }

    /* ============================================
        Construct the header object
       ============================================ */
    rv = RvSipMsgConstructHeaderInMsgByType((RvSipMsgHandle)pMsg, eHeaderType, RV_FALSE, &pHeader);
    if(rv != RV_OK)
    {
        RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                "ParseBadSyntaxHeader - message 0x%p. Failed to construct header. (rv=%d)",
                 pMsg, rv));
        return rv;
    }

    /* ===========================================
        Set header name information  
       ========================================== */
    if(RV_TRUE == bCompactForm) 
    {
        SetCompactFormInBSHeader(pHeader, eHeaderType);
    }

    SetBsHeaderSpecificType(pHeader, eSpecificHeaderType);

    /* =============================================
        Set the bad-syntax string in the header 
       =============================================*/
    /* skip spaces before the header value: */
    while(*pHeaderVal == SPACE_CHAR || *pHeaderVal == TAB_CHAR)
    {
        ++pHeaderVal;
    }
    
    rv = SipHeaderSetBadSyntaxStr(pHeader, eHeaderType, pHeaderVal);
    if(rv != RV_OK)
    {
        RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
            "ParseBadSyntaxHeader - message 0x%p. Failed to set bad-syntax string in header. (rv=%d)",
            pMsg, rv));
        return rv;
    }

    
    /* =================================================
       For other header - set the header name string
       ================================================= */
    if(eHeaderType == RVSIP_HEADERTYPE_OTHER)
    {
        /* set NULL instead of ':' */
        *(pHeaderNameEnd+1) = MSG_NULL_CHAR;
        
        /* set the header name */
        rv = RvSipOtherHeaderSetName((RvSipOtherHeaderHandle)pHeader,
                                     pHeaderStart);
        if (RV_OK != rv)
        {
            RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                "ParseBadSyntaxHeader - message 0x%p. Failed to set name in bad-syntax other header. (rv=%d:%s)",
                pMsg, rv, SipCommonStatus2Str(rv)));
            return rv;
        }
    }
    /* =======================================================
       Special case (3) - Via header.
       Get the transport parameter from the defected via header, 
       in order to send later the 400 response 
       ======================================================= */
    if(eHeaderType == RVSIP_HEADERTYPE_VIA)
    {
        rv = SetBsViaTransport(pHeader,pHeaderVal);
        if(rv != RV_OK)
        {
            RvLogError(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                "ParseBadSyntaxHeader - message 0x%p. Failed to rescue the transport from bad-syntax via header. (rv=%d)",
                pMsg, rv));
        }
        else
        {
            RvLogDebug(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
                "ParseBadSyntaxHeader - message 0x%p. managed to rescue the transport from bad-syntax via header(rv=%d:%s)",
                pMsg, rv, SipCommonStatus2Str(rv)));
        }
    }

    RvLogDebug(pMsg->pMsgMgr->pLogSrc,(pMsg->pMsgMgr->pLogSrc,
              "ParseBadSyntaxHeader - message 0x%p. set bad-syntax %s header.",
               pMsg, SipHeaderName2Str(eHeaderType)));

    return RV_OK;
}

/***************************************************************************
* ParseLegalHeader
* ------------------------------------------------------------------------
* General:  The function passes a message line (header or start-line)
*           to the parser.
*           This function may be called to parse lines in a message, or to
*           parse lines in a body part.
* Return Value: RvStatus.
* ------------------------------------------------------------------------
* Arguments:
*    Input: pMsgMgr        - Pointer to the message manager object.
*           pObj           - Pointer to the object that holds the headers. 
*           eObjType       - The type of pObj (message or body-part).
*           eHeaderType    - The type of the header that should be parsed.
*           bCompactForm   - Does the name of the header is in compact form.
*           bStartLine     - Is the given line is a start-line or not.
*           eSpecificHeaderType - The specific type of the header.
*           lineNumber     - The number of given line in the message.
*           pStringForParsing - Pointer to the line string.
***************************************************************************/
static RvStatus RVCALLCONV ParseLegalHeader(IN MsgMgr*          pMsgMgr,
                                            IN void*            pObj,
                                            IN SipParseType     eObjType,
                                            IN RvSipHeaderType  eHeaderType,
                                            IN RvBool           bCompactForm,
                                            IN RvBool           bStartLine,
                                            IN SipParseType     eSpecificHeaderType,
                                            IN RvInt32          lineNumber,
                                            IN RvChar*          pStringForParsing)
{
    RvStatus        rv;
    RvBool    bSetHeaderPrefix;
     
    if(RV_TRUE == bStartLine || 
      (eHeaderType == RVSIP_HEADERTYPE_OTHER && 
       RV_FALSE == IsOptionTagHeader(eSpecificHeaderType))) /*start-line*/
    {
        /* start-line, and other-headers which are not option-tag header, are being
           parsed from the beginning of the line */
       bSetHeaderPrefix = RV_FALSE;
    }
    else
    {
        /* for all the rest - only the header value is given to the parser. 
           the parser will set an external unique prefix according to the header 
           type, in order to perform the correct parsing rule */
        bSetHeaderPrefix = RV_TRUE;
    }

    rv = SipParserStartParsing(pMsgMgr->hParserMgr, 
                               eObjType, 
                               pStringForParsing, 
                               lineNumber, 
                               eSpecificHeaderType, 
                               bCompactForm, 
                               bSetHeaderPrefix, 
                               pObj) ;
    
    return rv;
}


/***************************************************************************
* ParseHeader
* ------------------------------------------------------------------------
* General: This function gets a null terminated header string.
*          It finds the correct type of the header, and tries to parse it.
*          In case of a parsing error - if this is a header in a message -
*          save the header as a bad-syntax header.
* Return Value: RV_OK - on success
*               else  - on failure
* ------------------------------------------------------------------------
* Arguments:
*    Input: pMsgMgr        - Pointer to the message manager object.
*           pObj           - Pointer to the object that holds the headers. 
*           eObjType       - The type of pObj (message or body-part).
*           bStartLine     - Is the given line is a start-line or not.
*           pHeaderStart   - Pointer to the beginning of the header string.
*           pHeaderNameEnd - Pointer to the end of the header name (if NULL -
*                            it means that we did not find a colon character).
*           headerNameLen  - The length of the header name.
*           pHeaderVal     - Pointer to the beginning of the header value.
*           lineNumber     - The number of given line in the message.
***************************************************************************/
static RvStatus RVCALLCONV ParseHeader(IN MsgMgr* pMsgMgr,
                                       IN void*   pObj,
                                       IN SipParseType eObjType,
                                       IN RvBool  bStartLine,
                                       IN RvChar* pHeaderStart,
                                       IN RvChar* pHeaderNameEnd,  
                                       IN RvInt32 headerNameLen,
                                       IN RvChar* pHeaderVal,
                                       IN RvInt32 lineNumber)
{
    RvStatus        rv = RV_OK;
    RvChar*         pStringForParsing; 
    RvSipHeaderType eHeaderType = RVSIP_HEADERTYPE_UNDEFINED;
    SipParseType    eSpecificHeaderType = SIP_PARSETYPE_UNDEFINED;
    RvBool          bCompactForm = RV_FALSE;
    
    if(pHeaderNameEnd != NULL && bStartLine == RV_FALSE)
    {
        /* ============================
            Decides of the header type 
           ============================ */
        eHeaderType = GetHeaderType(pHeaderStart, pHeaderNameEnd, headerNameLen, 
                                    &bCompactForm, &eSpecificHeaderType);
        if((eObjType != SIP_PARSETYPE_MSG && eObjType != SIP_PARSETYPE_MIME_BODY_PART && eObjType != SIP_PARSETYPE_URL_HEADERS_LIST) && /* --> stand alone header */
            eObjType != eSpecificHeaderType)
        {
            rv = UpdateStandAloneHeaderSpecificType(pObj, eHeaderType, eSpecificHeaderType);
            if(rv != RV_OK)
            {
                /* user gave us a header prefix that does not fit the constructed header type */
                RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                    "ParseHeader - MISMATCH: Given header is of type %d, header prefix is of type %d. can't parse!",
                    eObjType, eSpecificHeaderType));
                return RV_ERROR_UNKNOWN;
            }
        }

        if(eObjType == SIP_PARSETYPE_MIME_BODY_PART)
        {
            /* Body part objects saves all headers as other header, 
               except content-type and content disposition */
            if(eSpecificHeaderType != SIP_PARSETYPE_CONTENT_DISPOSITION &&
               eSpecificHeaderType != SIP_PARSETYPE_CONTENT_TYPE &&
			   eSpecificHeaderType != SIP_PARSETYPE_CONTENT_ID)
               {
                    eHeaderType = RVSIP_HEADERTYPE_OTHER;
               }
        }

        /* 2. skip spaces before header val */
        while((*pHeaderVal == ' ' ||  *pHeaderVal == 0x09 /*tab */)
              && *pHeaderVal != MSG_NULL_CHAR)
        {
            ++pHeaderVal;
        }

        if(eHeaderType == RVSIP_HEADERTYPE_OTHER 
#ifndef RV_SIP_PRIMITIVES /* in primitives the option-tag is like every other other header */
            && RV_FALSE == IsOptionTagHeader(eSpecificHeaderType)
#endif /*RV_SIP_PRIMITIVES*/
            )
        {
            /* other-header which is not option-tag header, is being
               parsed from the beginning of the line */
            pStringForParsing = pHeaderStart;
        }
        else
        {
            /* for all the rest - only the header value is given to the parser. 
               the parser will set an external unique prefix according to the header 
               type, in order to perform the correct parsing rule */
            pStringForParsing = pHeaderVal;
        }
    }
    else /* pHeaderNameEnd == NULL or bStartLine == RV_TRUE */
    {
        /* start line and a header which has no colon in it 
           are being parsed from the start of the header line. */
        pStringForParsing = pHeaderStart;
    }  

    /* =================================================
        Pass the message line to the parser for parsing
       =================================================*/
    rv = ParseLegalHeader(pMsgMgr, pObj, eObjType, eHeaderType, bCompactForm, bStartLine, 
                          eSpecificHeaderType, lineNumber, pStringForParsing);
    if (RV_ERROR_ILLEGAL_SYNTAX == rv)
    {
        if(eObjType == SIP_PARSETYPE_MSG)
        {
            /* ==============================
                handle the bad-syntax case 
               ============================== */
            if(SipMsgGetBadSyntaxReasonPhrase((RvSipMsgHandle)pObj) == UNDEFINED)
            {
                RvChar tempErrStr[36];
                /* save the bad syntax information (if wasn't already set) as a reason phrase.
                   this string can be used later in 400 response to indicate the reason for
                   the rejection of the request. */
                sprintf((RvChar*)tempErrStr, "syntax problem in line %d", lineNumber);
                SipMsgSetBadSyntaxReasonPhrase((RvSipMsgHandle)pObj, (RvChar*)tempErrStr);
            }
            if(RV_TRUE == bStartLine) 
            {
                /* Save bad-syntax start line information in the message object */
                rv = SipMsgSetBadSyntaxInfoInStartLine((RvSipMsgHandle)pObj, pHeaderStart);
                if(rv != RV_OK)
                {
                    RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                        "ParseHeader - message 0x%p. Failed to set bad-syntax start-line. (rv=%d)",
                        pObj, rv));
                    return rv;
                }
                RvLogDebug(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                      "ParseHeader - message 0x%p. set bad-syntax start-line.", pObj));
            }
            else
            {
                rv = ParseBadSyntaxHeader(pObj, eHeaderType, bCompactForm, eSpecificHeaderType, 
                                          pHeaderStart, pHeaderNameEnd, pHeaderVal);
                if(rv != RV_OK)
                {
                    RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                        "ParseHeader - message 0x%p. Failed to parse bad-syntax header. (rv=%d)",
                        pObj, rv));
                    return rv;
                }
            }
        }
        else
        {
            RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
                "ParseHeader - header 0x%p. Failed to parse stand-alone header. (rv=%d)",
                pObj, rv));
            return rv;
        }
    }
	else if (RV_OK != rv)
	{
		RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
			"ParseHeader - header 0x%p. Failed within reduction function. Abort processing. (rv=%d)",
			pObj, rv));
	}

    return rv;
}
/***************************************************************************
 * PrepareAndParseMessageHeader
 * ------------------------------------------------------------------------
 * General: The function analyzes a new line in a message or body-part:
 *          1. Search for the end of the header (CR or LF).
 *          2. During the search - save pointers to the header value, and to 
 *             the end of the header name.
 *          3. Parse the header.
 *          4. The function retrieves updated pointer and index.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *      pMsgMgr  - Pointer to the message manager object.
 *      pObj     - Pointer to the message or body part object.
 *      eObjType - Type of pObj.
 * Output:
 *      pBuffer  - Pointer to the end of the header line (after first CR or LF).
 *      pIndex   - The last index of the message buffer, that was treated.
 ***************************************************************************/
static RvStatus RVCALLCONV PrepareAndParseMessageHeader(IN MsgMgr*        pMsgMgr,
                                                        IN void*          pObj,
                                                        IN SipParseType   eObjType,
                                                        IN RvInt32        lineNumber,
                                                        INOUT RvChar**    pBuffer,
                                                        INOUT RvInt32*    pIndex)
{
    RvStatus   rv = RV_OK;
    RvChar*    pBuf = *pBuffer; 
    RvChar    *pHeaderVal = NULL;      /* pointer to the beginning of the header value */
    RvChar    *pHeaderStart = *pBuffer;/* pointer to the start of the header */
    RvChar    *pHeaderNameEnd = NULL;  /* pointer to the end of the header name */
    RvInt32    safeCounter = 0, headerNameLen = 0;
    RvBool     bInHeaderVal = RV_FALSE; /* already found the first colon or not */
    RvChar     tmpChar=0;
    RvBool     bStartLine;

    bStartLine = (eObjType == SIP_PARSETYPE_MSG && 1==lineNumber)?RV_TRUE:RV_FALSE;
    
    /* =========================
       1. Search for a CR or LF 
       ========================= */
    while(*pBuf != CR && *pBuf != LF && safeCounter < HEADER_SAFE_COUNTER_LIMIT)
    {
        if(*pBuf == ':' && bInHeaderVal == RV_FALSE)
        {
            /* found the first colon (after the header name) */
            bInHeaderVal = RV_TRUE;
            pHeaderVal = pBuf+1;
            pHeaderNameEnd = pBuf - 1;
        }
        ++safeCounter;
        ++pBuf;
        ++(*pIndex);
        if(bInHeaderVal == RV_FALSE)
        {
            ++ headerNameLen;
        }
    }
    if(safeCounter == HEADER_SAFE_COUNTER_LIMIT)
    {
        /* exception */
        RvLogExcep(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "PrepareAndParseMessageHeader - 0x%p. safe counter reached max value %d.",
             pObj, safeCounter));
        return RV_ERROR_UNKNOWN;  
    }
    else
    {
        RvChar* pEndOfHeader = pBuf;

        while(*(pBuf-1) == TAB_CHAR || *(pBuf-1) == SPACE_CHAR)
        {
            /* go back over the spaces in the end of hte header */
            --pBuf;
        }
        
        /* remember the CR or LF character */
        tmpChar = *pBuf;
        
        /* set a NULL at the end of the header */
        *pBuf = MSG_NULL_CHAR;

        /* =========================
           2. Parse the header.
           ========================= */
        rv = ParseHeader(pMsgMgr, pObj, eObjType, bStartLine, pHeaderStart, 
                         pHeaderNameEnd, headerNameLen, pHeaderVal, lineNumber);

        /* returns the char we changed earlier */
        *pBuf=tmpChar;

        pBuf = pEndOfHeader;
    }

    *pBuffer = ++pBuf; /* increase the buffer by one, to skip the CR/LF */
    ++(*pIndex);
    return rv;
}



#ifdef __cplusplus
}
#endif
