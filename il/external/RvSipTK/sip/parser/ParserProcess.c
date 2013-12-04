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
* Copyright RADVision 1996.                                                     *
* Last Revision: Jan. 2000                                                      *
*********************************************************************************
*/


/*********************************************************************************
 *                              <ParserProcess.c>
 *
 * This file defines functions which are used from the parser to initialize sip
 * message or sip header.
 *
 *    Author                         Date
 *    ------                        ------
 * Michal Mashiach                   DEC 00
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"

#include "ParserUtils.h"
#include "ParserProcess.h"
#include "_SipParserManager.h"
#include "RvSipMsgTypes.h"
#include "ParserTypes.h"

#include "RvSipMsg.h"
#include "_SipHeader.h"
#include "RvSipHeader.h"
#include "RvSipAddress.h"
#include "RvSipCSeqHeader.h"
#include "RvSipPartyHeader.h"
#include "RvSipOtherHeader.h"
#include "RvSipViaHeader.h"
#include "RvSipExpiresHeader.h"
#include "RvSipDateHeader.h"
#include "_SipDateHeader.h"
#include "RvSipRouteHopHeader.h"
#include "RvSipReplacesHeader.h"
#include "_SipAddress.h"
#include "_SipCSeqHeader.h"
#include "_SipPartyHeader.h"
#include "_SipOtherHeader.h"
#include "_SipViaHeader.h"
#include "_SipExpiresHeader.h"
#include "_SipRouteHopHeader.h"

#ifndef RV_SIP_PRIMITIVES
#include "RvSipAllowHeader.h"
#include "_SipAllowHeader.h"
#include "_SipReferToHeader.h"
#include "_SipReferredByHeader.h"
#include "_SipContentTypeHeader.h"
#include "RvSipContentTypeHeader.h"
#include "_SipContentIDHeader.h"
#include "RvSipContentIDHeader.h"
#include "_SipContentDispositionHeader.h"
#include "RvSipContentDispositionHeader.h"
#include "RvSipReferToHeader.h"
#include "RvSipReferredByHeader.h"
#include "RvSipRAckHeader.h"
#include "_SipRAckHeader.h"
#include "RvSipRSeqHeader.h"
#include "RvSipBody.h"
#include "RvSipBodyPart.h"
#endif /* #ifndef RV_SIP_PRIMITIVES*/

#include "RvSipAuthenticationHeader.h"
#include "RvSipAuthorizationHeader.h"
#include "_SipAuthenticationHeader.h"
#include "_SipAuthorizationHeader.h"

#ifndef RV_SIP_PRIMITIVES
#include "RvSipRetryAfterHeader.h"
#include "_SipRetryAfterHeader.h"
#include "RvSipEventHeader.h"
#include "_SipEventHeader.h"
#include "RvSipAllowEventsHeader.h"
#include "_SipAllowEventsHeader.h"
#include "RvSipSubscriptionStateHeader.h"
#include "_SipSubscriptionStateHeader.h"
#include "RvSipSessionExpiresHeader.h"
#include "_SipSessionExpiresHeader.h"
#include "RvSipMinSEHeader.h"
#include "_SipMinSEHeader.h"
#include "_SipReplacesHeader.h"
#endif /* #ifndef RV_SIP_PRIMITIVES*/


#include "RvSipContactHeader.h"
#include "_SipContactHeader.h"

#ifndef RV_SIP_PRIMITIVES
#include "RvSipCommonList.h"
#endif /* RV_SIP_PRIMITIVES */
#ifdef RV_SIP_AUTH_ON
#include "RvSipAuthenticationInfoHeader.h"
#include "_SipAuthenticationInfoHeader.h"
#endif /*ifdef RV_SIP_AUTH_ON*/
#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
#include "RvSipReasonHeader.h"
#include "_SipReasonHeader.h"
#include "RvSipWarningHeader.h"
#include "_SipWarningHeader.h"
#include "RvSipTimestampHeader.h"
#include "_SipTimestampHeader.h"
#include "RvSipInfoHeader.h"
#include "_SipInfoHeader.h"
#include "RvSipAcceptHeader.h"
#include "_SipAcceptHeader.h"
#include "RvSipAcceptEncodingHeader.h"
#include "_SipAcceptEncodingHeader.h"
#include "RvSipAcceptLanguageHeader.h"
#include "_SipAcceptLanguageHeader.h"
#include "RvSipReplyToHeader.h"
#include "_SipReplyToHeader.h"
/* XXX */
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */

#ifdef RV_SIP_IMS_HEADER_SUPPORT
#include "RvSipPUriHeader.h"
#include "_SipPUriHeader.h"
#include "RvSipPVisitedNetworkIDHeader.h"
#include "_SipPVisitedNetworkIDHeader.h"
#include "RvSipPAccessNetworkInfoHeader.h"
#include "_SipPAccessNetworkInfoHeader.h"
#include "RvSipPChargingFunctionAddressesHeader.h"
#include "_SipPChargingFunctionAddressesHeader.h"
#include "RvSipPChargingVectorHeader.h"
#include "_SipPChargingVectorHeader.h"
#include "RvSipPMediaAuthorizationHeader.h"
#include "_SipPMediaAuthorizationHeader.h"
#include "RvSipSecurityHeader.h"
#include "_SipSecurityHeader.h"
#include "RvSipPProfileKeyHeader.h"
#include "_SipPProfileKeyHeader.h"
#include "RvSipPUserDatabaseHeader.h"
#include "_SipPUserDatabaseHeader.h"
#include "RvSipPAnswerStateHeader.h"
#include "_SipPAnswerStateHeader.h"
#include "RvSipPServedUserHeader.h"
#include "_SipPServedUserHeader.h"
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
#include "RvSipPDCSTracePartyIDHeader.h"
#include "_SipPDCSTracePartyIDHeader.h"
#include "RvSipPDCSOSPSHeader.h"
#include "_SipPDCSOSPSHeader.h"
#include "RvSipPDCSBillingInfoHeader.h"
#include "_SipPDCSBillingInfoHeader.h"
#include "RvSipPDCSLAESHeader.h"
#include "_SipPDCSLAESHeader.h"
#include "RvSipPDCSRedirectHeader.h"
#include "_SipPDCSRedirectHeader.h"
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

#define PROXY_REQUIRE_PREFIX "Proxy-Require"
#define REQUIRE_PREFIX "Require"
#define SUPPORTED_PREFIX "Supported"
#define SUPPORTED_COMPACT_PREFIX "k"
#define UNSUPPORTED_PREFIX "Unsupported"


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV ParserConstructHeader(IN ParserMgr       *pParserMgr,
                                                 IN SipParseType     eObjType,
                                                 IN const void*            pSipObject,
                                                 IN RvSipHeaderType  eHeaderType,
                                                 IN RvUint8         *pcbPointer,
                                                 OUT void**          ppHeader);

static void RVCALLCONV InitializePcbStructByHeaderType(
													   IN  SipParser_pcb_type* pcb,
													   IN  SipParseType        eType);

static void RVCALLCONV SetExternalHeaderPrefix(IN SipParser_pcb_type* pcb,
                                               IN SipParseType       eSpecificHeaderType);

static RvStatus RVCALLCONV ParserInitMethodType(
                                 IN  ParserMgr      *pParserMgr,
                                 IN  ParserMethod    method,
                                 IN  HRPOOL          hPool,
                                 IN  HPAGE           hPage,
                                 OUT RvSipMethodType *peMethodType,
                                 OUT RvInt32       *pAllocationOffset);

static RvStatus RVCALLCONV ParserInitUrlParams(
                                IN    ParserMgr             *pParserMgr,
                                IN    ParserUrlParameters   *urlParams,
                                IN    HRPOOL                hPool,
                                IN    HPAGE                 hPage,
                                INOUT RvSipAddressHandle    hUrl);

static RvStatus RVCALLCONV ParserInitHostPortInUrl(
                                IN    ParserMgr          *pParserMgr,
                                IN    ParserSipUrl       *url,
                                IN    HRPOOL             hPool,
                                IN    HPAGE              hPage,
                                INOUT RvSipAddressHandle hUrl);

#ifdef RV_SIP_JSR32_SUPPORT
static RvStatus RVCALLCONV ParserInitDisplayNameInAddr(
							IN    ParserMgr             *pParserMgr,
							IN    RvSipAddressHandle    hAddress,
							IN    SipParser_pcb_type    *pcb,
							IN    HRPOOL                hPool,
							IN    HPAGE                 hPage);
#endif

static RvStatus RVCALLCONV ParserInitAddressInHeader(IN ParserMgr         *pParserMgr,
                                                     IN ParserExUri       *exUri,
                                                     IN HRPOOL             hPool,
                                                     IN HPAGE              hPage,
                                                     IN RvSipAddressType     eAddrType,
                                                     IN RvSipAddressHandle hAddr);

static RvStatus RVCALLCONV ParserInitUrlInHeader(
                              IN    ParserMgr            *pParserMgr,
                              IN    SipParser_pcb_type   *pcb,
							  IN    ParserSipUrl         *url,
                              IN    HRPOOL                hPool,
                              IN    HPAGE                 hPage,
                              INOUT RvSipAddressHandle    hUrl);

static RvStatus RVCALLCONV ParserInitAbsUriInHeader(
                                 IN    ParserMgr                *pParserMgr,
                                 IN    SipParser_pcb_type       *pcb,
								 IN    ParserAbsoluteUri        *uri,
                                 IN    HRPOOL                   hPool,
                                 IN    HPAGE                    hPage,
                                 INOUT RvSipAddressHandle       hUri);

#ifdef RV_SIP_TEL_URI_SUPPORT
static RvStatus RVCALLCONV ParserInitTelUriInHeader(
								IN    ParserMgr                *pParserMgr,
								IN    SipParser_pcb_type       *pcb,
								IN    ParserTelUri             *uri,
								IN    HRPOOL                   hPool,
								IN    HPAGE                    hPage,
								INOUT RvSipAddressHandle       hUri);
#endif

static RvStatus ParserInitAddressInVia(IN    ParserMgr             *pParserMgr,
                                          IN    ParserSentByInVia     *pViaAddress,
                                          IN    HRPOOL                hPool,
                                          IN    HPAGE                 hPage,
                                          IN    RvSipViaHeaderHandle  hViaHeader);

static RvStatus RVCALLCONV ParserInitSentByInVia(
                              IN    ParserMgr             *pParserMgr,
                              IN    ParserSingleVia       *pVia,
                              IN    HRPOOL                hPool,
                              IN    HPAGE                 hPage,
                              IN    RvSipViaHeaderHandle  hViaHeader);

static RvStatus RVCALLCONV ParserInitParamsInVia(
                              IN    ParserMgr             *pParserMgr,
                              IN    ParserSingleVia       *pVia,
                              IN    HRPOOL                hPool,
                              IN    HPAGE                 hPage,
                              INOUT RvSipViaHeaderHandle  hViaHeader);

static RvStatus RVCALLCONV ParserInitPartyHeader(
                              IN    ParserMgr             *pParserMgr,
                              IN    ParserPartyHeader     *PartyVal,
                              IN    HRPOOL                hPool,
                              IN    HPAGE                 hPage,
                              INOUT RvSipPartyHeaderHandle *hPartyHeader);

static RvStatus RVCALLCONV ParserInitDateInHeader(
                               IN    ParserMgr              *pParserMgr,
                               IN    ParserSipDate          *date,
                               INOUT RvSipDateHeaderHandle  hDateHeader);


static RvStatus RVCALLCONV ParserInitExpiresHeaderInHeader(
                            IN    ParserMgr                *pParserMgr,
                            IN    ParserExpiresHeader      *pExpiresVal,
                            IN    HRPOOL                    hPool,
                            IN    HPAGE                     hPage,
                            INOUT RvSipExpiresHeaderHandle  hExpiresHeader);

static RvStatus RVCALLCONV ParserInitContactParams(
                            IN    ParserMgr                 *pParserMgr,
                            IN    ParserContactHeaderValues *pContactVal,
                            IN    HRPOOL                    hPool,
                            IN    HPAGE                     hPage,
                            INOUT RvSipContactHeaderHandle  hContactHeader);

#ifdef RV_SIP_AUTH_ON
static RvStatus RVCALLCONV ParserInitChallengeAuthentication(
                             IN    ParserMgr                        *pParserMgr,
                             IN    ParserAuthenticationHeader      *pAuthHeaderVal,
                             IN    HRPOOL                           hPool,
                             IN    HPAGE                            hPage,
                             INOUT RvSipAuthenticationHeaderHandle hAuthenHeader);

static RvStatus RVCALLCONV ParserInitChallengeAuthorization(
                            IN    ParserMgr                      *pParserMgr,
                            IN    ParserAuthorizationHeader      *pAuthHeaderVal,
                            IN    HRPOOL                         hPool,
                            IN    HPAGE                          hPage,
                            INOUT RvSipAuthorizationHeaderHandle hAuthorHeader);
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifndef RV_SIP_PRIMITIVES
static RvStatus  RVCALLCONV ParserInitSessionExpiresHeaderInHeader(
                        IN    ParserMgr                       *pParserMgr,
                        IN    ParserSessionExpiresHeader      *pSessionExpiresVal,
                        IN    HRPOOL                           hPool,
                        IN    HPAGE                            hPage,
                        INOUT RvSipSessionExpiresHeaderHandle  hSessionExpiresHeader);

static RvStatus  RVCALLCONV ParserInitMinSEHeaderInHeader(
                                   IN    ParserMgr              *pParserMgr,
                                   IN    ParserMinSEHeader      *pMinSEVal,
                                   IN    HRPOOL                  hPool,
                                   IN    HPAGE                   hPage,
                                   INOUT RvSipMinSEHeaderHandle  hMinSEHeader);
#endif /* #ifndef RV_SIP_PRIMITIVES*/

static void CleanHeaderBeforeComma(RvChar* pcbPointer, RvChar* pHeaderStart);

static RvStatus GetCSeqStepFromMsgBuff(IN  ParserMgr *pParserMgr,
									   IN  RvChar    *pBuffer,
									   IN  RvUint32   length,
									   OUT RvInt32   *pCSeqStep);

static RvSipAddressType ParserAddrType2MsgAddrType(ParserTypeAddr eParserType);

#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
static RvStatus RVCALLCONV ParserInitializeInfoFromPcb(
                                    IN    ParserMgr             *pParserMgr,
                                    IN    ParserInfoHeader      *pInfoVal,
                                    IN    HRPOOL                 hPool,
                                    IN    HPAGE                  hPage,
                                    INOUT RvSipInfoHeaderHandle *hInfoHeader);

static RvStatus RVCALLCONV ParserInitializeReplyToFromPcb(
												  IN    ParserMgr                *pParserMgr,
												  IN    ParserReplyToHeader      *ReplyToVal,
												  IN    HRPOOL                    hPool,
												  IN    HPAGE                     hPage,
												  INOUT RvSipReplyToHeaderHandle *hReplyToHeader);
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */

#ifdef RV_SIP_IMS_HEADER_SUPPORT
static RvStatus RVCALLCONV ParserInitDiameterUriInHeader(
                          IN    ParserMgr             *pParserMgr,
                          IN    SipParser_pcb_type    *pcb,
						  IN    ParserDiameterUri     *uri,
                          IN    HRPOOL                hPool,
                          IN    HPAGE                 hPage,
                          INOUT RvSipAddressHandle    hUri);
#endif


/*-----------------------------------------------------------------------*/
/*                         FUNCTIONS IMPLEMENTATION                      */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SipParserStartParsing
 * ------------------------------------------------------------------------
 * General:This function starts the parsing process.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources)
 *               RV_ERROR_NULLPTR     - When receiving NULL pointers.
 *               RV_ERROR_ILLEGAL_SYNTAX  - When there is syntax error from the parser.
 *               RV_ERROR_UNKNOWN        - When error occurred in reduction function.
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:     hParserMgr - Handle the the parser manager.
 *             eObjType   - The type of object that was allocated.
 *             pBuffer    - The buffer which will be parsed.
 *             lineNumber - The number of line in message being parsed - this parameter
 *                          is used for printing the correct location in case of error.
 *             eHeaderSpecificType  - The given header buffer specific type. 
 *                          (will be used in the reduction function, to set the
 *                          correct header type. e.g. Route/Record-Route)
 *             bCompactForm - Is the header being parsed is in a compact form or not.
 *                          (will be used in the reduction function)
 *             bSetHeaderPrefix - for a start line and for an other header that is not
 *                          option-tag, we do not want to set a prefix (we parse the header
 *                          from it's beginning).
 *                          For all other headers, we should set a prefix according
 *                          to the header type.
 *  In/Output: pSipObject - Pointer to the structure in which the values from
 *                          the parser will be set.
 ***************************************************************************/
RvStatus RVCALLCONV SipParserStartParsing(IN    SipParserMgrHandle hParserMgr,
                                         IN    SipParseType       eObjType,
                                         IN    RvChar            *pBuffer,
                                         IN    RvInt32            lineNumber,
                                         IN    SipParseType       eHeaderSpecificType,
                                         IN    RvBool             bCompactForm,
                                         IN    RvBool             bSetHeaderPrefix,
                                         INOUT void               *hSipObject)
{
    ParserMgr*          pParserMgr = (ParserMgr*)hParserMgr;
    RvStatus            rv = RV_OK;
    SipParser_pcb_type  pcb;
    ParserExtensionString extParams;
    ParserExtensionString extUrlParams;
    
    /* SPIRENT_BEGIN */
    /*
     * COMMENTS: prepare copy of string in buffer, cause SipParser can change string. If we would not get any memory ,we would work without a copy
     */
    #if defined(UPDATED_BY_SPIRENT)
    char *strToParser;
    #endif /* defined(UPDATED_BY_SPIRENT) */ 
    /* SPIRENT_END */

    /*Check the input parameters*/
    if( NULL == hSipObject || NULL == hParserMgr)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
            "SipParserStartParsing - Can not accept NULL handle : 0x%p",hSipObject));
        return RV_ERROR_NULLPTR;
    }
    if( NULL == pBuffer)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
            "SipParserStartParsing - Can not accept NULL buffer : 0x%p",pBuffer));
        return RV_ERROR_NULLPTR;
    }

    
    extParams.size  = 0;
    extParams.hPool = pParserMgr->pool;
    extParams.hPage = NULL_PAGE;
    
    extUrlParams.size  = 0;
    extUrlParams.hPool = pParserMgr->pool;
    extUrlParams.hPage = NULL_PAGE;

    /* Init parser parameters*/
    pcb.pParserMgr      = (void*)hParserMgr;
    pcb.eStat           = RV_OK;
    pcb.pSipObject      = hSipObject;
    
    /* SPIRENT_BEGIN */
    #if defined(UPDATED_BY_SPIRENT)
    strToParser= strdup(pBuffer);
    pcb.pointer         = (RvUint8 *)(strToParser ? strToParser : pBuffer );
    pcb.pBuffer         = (RvChar *)(strToParser ? strToParser : pBuffer);
    pcb.pCurToken       = strToParser ? strToParser : pBuffer;
    #endif /* defined(UPDATED_BY_SPIRENT) */ 
    /* SPIRENT_END */
    
    pcb.eWhichHeader    = eHeaderSpecificType;
    pcb.eHeaderType     = eObjType;
    pcb.isCompactForm   = bCompactForm;
    pcb.isNewChallenge  = RV_FALSE;
    pcb.isWithinAngleBrackets = RV_FALSE;
    pcb.parenCount      = 0;
    pcb.lineNumber      = lineNumber;
    
    pcb.pExtParams = (ParserExtensionString *)&extParams;
    pcb.pUrlExtParams = (ParserExtensionString *)&extUrlParams;
    
    InitializePcbStructByHeaderType(&pcb, eHeaderSpecificType);
    
    if(RV_TRUE == bSetHeaderPrefix)
    {
        SetExternalHeaderPrefix(&pcb,eHeaderSpecificType);
    }
    
    /* Call parser function */
    SipParser(&pcb);

    ParserCleanExtParams (pcb.pExtParams);
    ParserCleanExtParams (pcb.pUrlExtParams);
    
    if(pcb.exit_flag ==  AG_SYNTAX_ERROR_CODE ||
        pcb.exit_flag ==  AG_SEMANTIC_ERROR_CODE)
    {
        /* The parser failed due to illegal syntax */
        RvLogError( pParserMgr->pLogSrc ,( pParserMgr->pLogSrc ,
            "SipParserStartParsing - Syntax Error - The parser failed to parse line %d",lineNumber));
        rv = RV_ERROR_ILLEGAL_SYNTAX;
    }
    else if (pcb.exit_flag == AG_REDUCTION_ERROR_CODE)
    {
        /* Returning the status that was returned from the reduction functions */
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "SipParserStartParsing - line %d - error in reduction function; RvStatus:%d",lineNumber, (RvInt32)pcb.eStat));
        rv = (RvStatus)pcb.eStat;
    }
    else if (pcb.exit_flag == AG_STACK_ERROR_CODE)
    {
        /* stack error occured */
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "SipParserStartParsing - line %d - error in parser stack", lineNumber));
        rv = RV_ERROR_UNKNOWN;
    }
    
    ParserCleanExtParams(pcb.pExtParams);
    ParserCleanExtParams(pcb.pUrlExtParams);
      
    /* SPIRENT_BEGIN */
    /*
     * COMMENTS: free memory if we worked with copy
     */
    #if defined(UPDATED_BY_SPIRENT)
    if (strToParser) free(strToParser);
    #endif /* defined(UPDATED_BY_SPIRENT) */ 
    /* SPIRENT_END */
    
    
    return rv;
}

/* Parser Error Handlers */


/***************************************************************************
 * ParserMethodInOtherHeader
 * ------------------------------------------------------------------------
 * General: This function is used convert method type to string whenever
 *          there is method in the name of the other header.
 * Return Value: none
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pMethod - the method that was detected inside the header name.
 *  In/Output: pOther  - the other header structure that need to be filled.
 ***************************************************************************/
void RVCALLCONV ParserMethodInOtherHeader(
                                     IN    ParserMethod      *pMethod,
                                     INOUT ParserOtherHeader *pOther)
{
    switch (pMethod->type)
    {
        case PARSER_METHOD_TYPE_INVITE:
        {
            pOther->name.buf = "INVITE";
            pOther->name.len = 6;
            break;
        }
        case PARSER_METHOD_TYPE_ACK:
        {
            pOther->name.buf = "ACK";
            pOther->name.len = 3;
            break;
        }
        case PARSER_METHOD_TYPE_BYE:
        {
            pOther->name.buf = "BYE";
            pOther->name.len = 3;
            break;
        }
        case PARSER_METHOD_TYPE_REGISTER:
        {
            pOther->name.buf = "REGISTER";
            pOther->name.len = 8;
            break;
        }
        case PARSER_METHOD_TYPE_REFER:
        {
            pOther->name.buf = "REFER";
            pOther->name.len = 5;
            break;
        }
        case PARSER_METHOD_TYPE_NOTIFY:
        {
            pOther->name.buf = "NOTIFY";
            pOther->name.len = 6;
            break;
        }
        case PARSER_METHOD_TYPE_CANCEL:
        {
            pOther->name.buf = "CANCEL";
            pOther->name.len = 6;
            break;
        }
        case PARSER_METHOD_TYPE_PRACK:
        {
            pOther->name.buf = "PRACK";
            pOther->name.len = 5;
            break;
        }
        case PARSER_METHOD_TYPE_SUBSCRIBE:
        {
            pOther->name.buf = "SUBSCRIBE";
            pOther->name.len = 9;
            break;
        }
        default:
            break;
    }
}


/***************************************************************************
 * ParserInitUrl
 * ------------------------------------------------------------------------
 * General: This function is used to init a sip url handle with the
 *          url values from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:      pParserMgr  - Pointer to the parser manager.
 *              url  - The url parameters from the parser.
 *              eType- This type indicates that the parser is called to
 *                     parse stand alone Url.
 *    In/OutPut:  hUrl - Handle to the address structure which will be
 *                     set by the parser.
 ***************************************************************************/
RvStatus RVCALLCONV ParserInitUrl(
                          IN    ParserMgr         *pParserMgr,
                          IN    SipParser_pcb_type * pcb,
                          IN    ParserSipUrl       *url,
                          IN    SipParseType       eType,
                          INOUT const void         *hUrl)
{
    HRPOOL hPool;
    HPAGE  hPage;


#ifdef RV_SIP_JSR32_SUPPORT
	if (SIP_PARSETYPE_ANYADDRESS == eType)
	{
		/* This is a special case where the parse was done only to determine
		   the type of the address and not to initialize an address structure */
		RvSipAddressType* peAddrType = (RvSipAddressType*)hUrl;
		*peAddrType = ParserAddrType2MsgAddrType(pcb->exUri.uriType);
		return RV_OK;
	}
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

	/* Check that we get here only if we parse stand alone header */
    if (SIP_PARSETYPE_URLADDRESS == eType)
    {
        /*The parser is used to parse url stand alone header */
        hUrl=(RvSipAddressHandle)hUrl;
    }
    else
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                     "ParserInitUrl - Illegal buffer for this type of header: %d",eType));
        return RV_ERROR_UNKNOWN;
    }
    hPool = SipAddrGetPool(hUrl);
    hPage = SipAddrGetPage(hUrl);

    RV_UNUSED_ARG(pcb);
    return ParserInitUrlInHeader(pParserMgr, pcb, url, hPool, hPage, (RvSipAddressHandle)hUrl);
}

/***************************************************************************
 * ParserInitAbsUri
 * ------------------------------------------------------------------------
 * General: This function is used to init a abs url handle with the
 *          uri values from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:      pParserMgr  - Pointer to the parser manager.
 *              uri  - The uri parameters from the parser.
 *              eType- This type indicates that the parser is called to
 *                     parse stand alone abs-uri.
 *    In/OutPut:  hUrl - Handle to the address structure which will be
 *                     set by the parser.
 ***************************************************************************/
RvStatus RVCALLCONV ParserInitAbsUri(
                          IN    ParserMgr          *pParserMgr,
                          IN    SipParser_pcb_type *pcb,
                          IN    ParserAbsoluteUri  *uri,
                          IN    SipParseType       eType,
                          INOUT const void         *hUri)
{
    HRPOOL hPool;
    HPAGE  hPage;

#ifdef RV_SIP_JSR32_SUPPORT
	if (SIP_PARSETYPE_ANYADDRESS == eType)
	{
		/* This is a special case where the parse was done only to determine
		   the type of the address and not to initialize an address structure */
		RvSipAddressType* peAddrType = (RvSipAddressType*)hUri;
		*peAddrType = ParserAddrType2MsgAddrType(pcb->exUri.uriType);
		return RV_OK;
	}
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

    /* Check that we get here only if we parse stand alone header */
    if (SIP_PARSETYPE_URIADDRESS ==eType)
    {
        /*The parser is used to parse uri stand alone header */
        hUri=(RvSipAddressHandle)hUri;
    }
    else
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                     "ParserInitAbsUri - Illegal buffer for this type of header: %d",eType));
        return RV_ERROR_UNKNOWN;
    }

    hPool = SipAddrGetPool(hUri);
    hPage = SipAddrGetPage(hUri);

    return ParserInitAbsUriInHeader(pParserMgr, pcb, uri,hPool, hPage, (RvSipAddressHandle)hUri);
}

#ifdef RV_SIP_TEL_URI_SUPPORT
/***************************************************************************
 * ParserInitTelUri
 * ------------------------------------------------------------------------
 * General: This function is used to init a tel-uri handle with the
 *          uri values from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:    pParserMgr  - Pointer to the parser manager.
 *              uri  - The uri parameters from the parser.
 *              eType- This type indicates that the parser is called to
 *                     parse stand alone abs-uri.
 *    In/OutPut:hUri - Handle to the address structure which will be
 *                     set by the parser.
 ***************************************************************************/
RvStatus RVCALLCONV ParserInitTelUri(
                          IN    ParserMgr          *pParserMgr,
                          IN    SipParser_pcb_type *pcb,
                          IN    ParserTelUri       *uri,
                          IN    SipParseType       eType,
                          INOUT const void         *hUri)
{
    HRPOOL hPool;
    HPAGE  hPage;

#ifdef RV_SIP_JSR32_SUPPORT
	if (SIP_PARSETYPE_ANYADDRESS == eType)
	{
		/* This is a special case where the parse was done only to determine
		   the type of the address and not to initialize an address structure */
		RvSipAddressType* peAddrType = (RvSipAddressType*)hUri;
		*peAddrType = ParserAddrType2MsgAddrType(pcb->exUri.uriType);
		return RV_OK;
	}
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

    /* Check that we get here only if we parse stand alone header */
    if (SIP_PARSETYPE_TELURIADDRESS ==eType)
    {
        /*The parser is used to parse uri stand alone header */
        hUri=(RvSipAddressHandle)hUri;
    }
    else
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                     "ParserInitTelUri - Illegal buffer for this type of header: %d",eType));
        return RV_ERROR_UNKNOWN;
    }

    hPool = SipAddrGetPool(hUri);
    hPage = SipAddrGetPage(hUri);

    return ParserInitTelUriInHeader(pParserMgr, pcb, uri,hPool, hPage, (RvSipAddressHandle)hUri);
}
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */


#ifndef RV_SIP_LIGHT
/***************************************************************************
 * ParserInitCSeq
 * ------------------------------------------------------------------------
 * General:This function is used from the parser to init the message with the
 *         CSeq header values.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pParserMgr - Pointer to the parser manager.
 *             pcbPointer - Pointer to the current location of parser pointer
 *                          in msg buffer (the end of the given header).
 *             pCSeq      - CSeq  structure holding the CSeq values from the
 *                          parser in a token form.
 *             eType      - This type indicates whether the parser is called to
 *                          parse stand alone CSeq or sip message.
 *  In/Output: pSipObject - Pointer to the structure in which the values from
 *                          the parser will be set.
 *                          If eType == SIP_PARSETYPE_MSG it will be cast to
 *                          RvSipMsgHandle and if eType == SIP_PARSETYPE_CSEQ
 *                          it will be cast to RvSipCSeqHandle.
 ***************************************************************************/
RvStatus RVCALLCONV ParserInitCSeq(
                            IN    ParserMgr       *pParserMgr,
                            IN    RvUint8        *pcbPointer,
                            IN    ParserCSeq      *pCSeq,
                            IN    SipParseType    eType,
                            INOUT const void      *pSipObject)
{
    RvSipCSeqHeaderHandle hCSeqHeader;
    RvStatus			  rv;
    HRPOOL				  hPool;
    HPAGE				  hPage;
	RvInt32				  step	       = UNDEFINED;
    RvSipMethodType		  eMethod      = RVSIP_METHOD_UNDEFINED;
	RvInt32				  offsetMethod = UNDEFINED;

    if (SIP_PARSETYPE_CSEQ ==eType)
    {
        /*The parser is used to parse CSeq header */
        hCSeqHeader=(RvSipCSeqHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_CSEQ, 
                                   pcbPointer, (void**)&hCSeqHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitCSeq - Failed to construct header"));
            return rv;
        }
    }


    hPool = SipCSeqHeaderGetPool(hCSeqHeader);
    hPage = SipCSeqHeaderGetPage(hCSeqHeader);

    /* Change the token step into a number */
	rv = GetCSeqStepFromMsgBuff(
				pParserMgr,pCSeq->sequenceNumber.buf,pCSeq->sequenceNumber.len,&step);
	if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			"ParserInitCSeq - error in GetCSeqStepFromMsgBuff"));
        return rv;
    }

    /* Set the step value into CSeq header */
    rv = RvSipCSeqHeaderSetStep(hCSeqHeader,step);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitCSeq - error in RvSipCseqHeaderSetStep"));
        return rv;
    }

    /* Change the token method structure into the enumerate method type or/and
       a string method type*/
    rv = ParserInitMethodType(pParserMgr,
                                   pCSeq->method ,
                                   hPool,
                                   hPage,
                                   &eMethod,
                                   &offsetMethod);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitCSeq - error in ParserInitMethodType"));
        return rv;
    }

    /* Set the method type enumerate and string values into the CSeq header*/
    rv = SipCSeqHeaderSetMethodType(hCSeqHeader,
                                         eMethod,
                                         NULL,
                                         hPool,
                                         hPage,
                                         offsetMethod);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitCSeq - error in CseqHeaderSetMethodType"));
        return rv;
    }
    return  RV_OK;
}
#endif /*#ifndef RV_SIP_LIGHT*/

/***************************************************************************
 * ParserInitContentLength
 * ------------------------------------------------------------------------
 * General: This function is used from the parser to init the message with the
 *         content length header values.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pParserMgr  - Pointer to the parser manager.
 *            pcbPointer  - Pointer to the current location of parser pointer
 *                          in msg buffer (the end of the given header).
 *            pContentLength - Content length  structure holding the Content
 *                             length values from the parser in a token form.
 * In/Output: pSipMsg     - Pointer to the structure in which the values
 *                          from the parser will be set.
 ********************************************* ******************************/
RvStatus RVCALLCONV ParserInitContentLength(
                                    IN    ParserMgr              *pParserMgr,
                                    IN    RvUint8                *pcbPointer,
                                    IN    ParserContentLength    *pContentLength,
                                    INOUT const void             *pSipMsg)
{
    RvInt32    uNum = UNDEFINED;
    RvStatus   rv;

    /* check if parser read all header, until CRLF.
    (if not, a missing CRLF syntax error will appear afterwards)*/
    if((*(pcbPointer)!=NULL_CHAR))
    {
        return RV_OK;
    }

    /* Change the token content length into a number*/
    rv =  ParserGetINT32FromString(pParserMgr,
                                        pContentLength->contentLenVal.buf,
                                        pContentLength->contentLenVal.len,
                                        &uNum);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitContentLength - error in ParserGetINT32FromString"));
        return rv;

    }

    /* Set the content length number in the message */
    rv = RvSipMsgSetContentLengthHeader(pSipMsg, uNum);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitContentLength - error in setting the header value"));
        return rv;
    }

    /* set compact form */
    return RvSipMsgSetContentLengthCompactForm(pSipMsg, pContentLength->isCompact);
}
#ifndef RV_SIP_LIGHT
/***************************************************************************
 * ParserInitCallId
 * ------------------------------------------------------------------------
 * General: This function is used from the parser to init the message with the
 *          Call Id header values.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pParserMgr  - Pointer to the parser manager.
 *             pcbPointer  - Pointer to the current location of parser pointer
 *                           in msg buffer (the end of the given header).
 *             pCallIdVal  - Call Id structure holding the Call Id values
 *                           from the parser in a token form.
 *    In/Output: pSipMsg     - Pointer to the structure in which the values
 *                           from the parser will be set.
 ***************************************************************************/
RvStatus RVCALLCONV ParserInitCallId(
                             IN    ParserMgr         *pParserMgr,
                             IN    RvUint8          *pcbPointer,
                             IN    ParserCallId      *pCallIdVal,
                             INOUT const void        *pSipMsg )
{
    RvInt32    offsetCallId = UNDEFINED;
    RvStatus   rv;
    HRPOOL      hPool;
    HPAGE       hPage;

    /* check if parser read all header, until CRLF.
    (if not, a missing CRLF syntax error will appear afterwards)*/
    if((*(pcbPointer)!=NULL_CHAR))
    {
        return RV_OK;
    }


    hPool = SipMsgGetPool(pSipMsg);
    hPage = SipMsgGetPage(pSipMsg);

    /* change the Call Id in a token form to a string value */
    rv =  ParserAllocFromObjPage(pParserMgr,
                                      &offsetCallId,
                                      hPool,
                                      hPage,
                                      pCallIdVal->callIdVal.buf,
                                      pCallIdVal->callIdVal.len);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitCallId - error in ParserAllocFromObjPage"));
        return rv;
    }

    /* Set the string Call Id into the message */
    rv = SipMsgSetCallIdHeader(pSipMsg,NULL,hPool,hPage,offsetCallId);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitCallId - error in setting the header value"));
        return rv;
    }

    /* set compact form */
	if(pCallIdVal->isCompact == RV_TRUE)
	{
		rv = RvSipMsgSetCallIdCompactForm(pSipMsg, pCallIdVal->isCompact);
	}

	return rv;
}

#endif /*#ifndef RV_SIP_LIGHT*/
#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
* ParserInitContentType
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          ContentType header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr  - Pointer to the parser manager.
*             pcbPointer  - Pointer to the current location of parser pointer
*                           in msg buffer (the end of the given header).
*             pContentTypeVal - pContentTypeVal structure holding the ContentType
*                           values from the parser in a token form.
*             eType       - This type indicates whether the parser is called to
*                           parse stand alone ContentType or sip message.
*  In/Output: pSipObject  - Pointer to the structure in which the values from
*                           the parser will be set.
*                           If eType == SIP_PARSETYPE_MSG it will be cast to
*                           RvSipMsgHandle and if eType == SIP_PARSETYPE_CONTENT_TYPE
*                           it will be cast to RvSipContentTypeHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitContentType(
                                   IN    ParserMgr       *pParserMgr,
                                   IN    RvUint8        *pcbPointer,
                                   IN    ParserContentType *pContentTypeVal,
                                   IN    SipParseType    eType,
                                   INOUT const void      *pSipObject)
{
    RvSipContentTypeHeaderHandle hConTypeHeader;
    RvSipBodyHandle              hBodyHandle;
    RvInt32                      mediaTypeOffset    = UNDEFINED;
    RvInt32                      subMediaTypeOffset = UNDEFINED;
    RvInt32                      boundaryOffset     = UNDEFINED;
    RvInt32                      baseOffset			= UNDEFINED;
    RvInt32                      versionOffset		= UNDEFINED;
	RvSipAddressHandle			 hStartAddr;
    RvSipAddressType			 eStartAddrType;
    RvStatus                     rv;
    HRPOOL                       hPool;
    HPAGE                        hPage;
    ParserExtensionString        * pGenericParamList =
            (ParserExtensionString *)pContentTypeVal->genericParamList;

    if (SIP_PARSETYPE_CONTENT_TYPE == eType)
    {
        /*The parser is used to parse ContentType header */
        hConTypeHeader = (RvSipContentTypeHeaderHandle)pSipObject;
    }
    else if (SIP_PARSETYPE_MIME_BODY_PART == eType)
    {
        /*The parser is used to parse message*/
        pSipObject=(RvSipBodyPartHandle)pSipObject;

        /* Construct Body in the body part  */
        hBodyHandle = RvSipBodyPartGetBodyObject(pSipObject);
        if (NULL == hBodyHandle)
        {
            rv = RvSipBodyConstructInBodyPart(pSipObject,&hBodyHandle);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContentType - error in RvSipBodyConstructInBodyPart"));
                return rv;
            }
        }

        rv = RvSipContentTypeHeaderConstructInBody(hBodyHandle,&hConTypeHeader);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContentType - error in RvSipContentTypeHeaderConstructInBody"));
            return rv;
        }
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_CONTENT_TYPE, 
                                    pcbPointer, (void**)&hConTypeHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContentType - Failed to construct header"));
            return rv;
        }
    }
    
    hPool = SipContentTypeHeaderGetPool(hConTypeHeader);
    hPage = SipContentTypeHeaderGetPage(hConTypeHeader);

    if (pContentTypeVal->mediaType.type == RVSIP_MEDIATYPE_OTHER)
    {
        /* Allocate the media type token in a page */
        rv = ParserAllocFromObjPage(pParserMgr,
                                     &mediaTypeOffset,
                                     hPool,
                                     hPage,
                                     pContentTypeVal->mediaType.other.buf,
                                     pContentTypeVal->mediaType.other.len);
        if (RV_OK != rv)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitContentType - error in ParserAllocFromObjPage"));
             return rv;
         }

        /* Set the the media type into the header */
        rv =  SipContentTypeHeaderSetMediaType(
                                    hConTypeHeader,
                                    NULL,
                                    hPool,
                                    hPage,
                                    mediaTypeOffset);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitContentType - error in SipContentTypeHeaderSetMediaType"));
            return rv;
        }
    }
    else
    {
        rv = RvSipContentTypeHeaderSetMediaType(
                                    hConTypeHeader,
                                    pContentTypeVal->mediaType.type,
                                    NULL);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContentType - error in RvSipContentTypeHeaderSetMediaType"));
            return rv;
        }
    }
    if (pContentTypeVal->mediaSubType.type == RVSIP_MEDIASUBTYPE_OTHER)
    {    /* Allocate the sub media type token in a page */
        rv = ParserAllocFromObjPage(pParserMgr,
                                     &subMediaTypeOffset,
                                     hPool,
                                     hPage,
                                     pContentTypeVal->mediaSubType.other.buf,
                                     pContentTypeVal->mediaSubType.other.len);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitContentType - error in ParserAllocFromObjPage"));
            return rv;
        }

        /* Set the the sub media type into the header */
		rv =  SipContentTypeHeaderSetMediaSubType(
                                    hConTypeHeader,
                                    NULL,
                                    hPool,
                                    hPage,
                                    subMediaTypeOffset);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitContentType - error in SipContentTypeHeaderSetMediaSubType"));
            return rv;
        }
    }
    else
    {
        rv = RvSipContentTypeHeaderSetMediaSubType(
                                    hConTypeHeader,
                                    pContentTypeVal->mediaSubType.type,
                                    NULL);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContentType - error in RvSipContentTypeHeaderSetMediaSubType"));
            return rv;
        }
    }

    /* set boundary param */
    if (pContentTypeVal->params.isBoundary == RV_TRUE)
    {
        /* Allocate the boundary token in a page */
        rv = ParserAllocFromObjPage(pParserMgr,
                                         &boundaryOffset,
                                         hPool,
                                         hPage,
                                         pContentTypeVal->params.boundary.buf,
                                         pContentTypeVal->params.boundary.len);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                 "ParserInitContentType - error in ParserAllocFromObjPage"));
            return rv;
        }

        /* Set the the boundary param into the header */
        rv =  SipContentTypeHeaderSetBoundary(
                                        hConTypeHeader,
                                        NULL,
                                        hPool,
                                        hPage,
                                        boundaryOffset);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                 "ParserInitContentType - error in SipContentTypeHeaderSetBoundary"));
            return rv;
        }

    }

    /* set base param */
    if (pContentTypeVal->params.isBase == RV_TRUE)
    {
        /* Allocate the boundary token in a page */
        rv = ParserAllocFromObjPage(pParserMgr,
                                         &baseOffset,
                                         hPool,
                                         hPage,
                                         pContentTypeVal->params.base.buf,
                                         pContentTypeVal->params.base.len);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                 "ParserInitContentType - error in ParserAllocFromObjPage"));
            return rv;
        }

        /* Set the the boundary param into the header */
        rv =  SipContentTypeHeaderSetBase(
                                        hConTypeHeader,
                                        NULL,
                                        hPool,
                                        hPage,
                                        baseOffset);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                 "ParserInitContentType - error in SipContentTypeHeaderSetBase"));
            return rv;
        }

    }

    /* set version param */
    if (pContentTypeVal->params.isVersion == RV_TRUE)
    {
        /* Allocate the boundary token in a page */
        rv = ParserAllocFromObjPage(pParserMgr,
                                         &versionOffset,
                                         hPool,
                                         hPage,
                                         pContentTypeVal->params.version.buf,
                                         pContentTypeVal->params.version.len);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                 "ParserInitContentType - error in ParserAllocFromObjPage"));
            return rv;
        }

        /* Set the the boundary param into the header */
        rv =  SipContentTypeHeaderSetVersion(
                                        hConTypeHeader,
                                        NULL,
                                        hPool,
                                        hPage,
                                        versionOffset);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                 "ParserInitContentType - error in SipContentTypeHeaderSetVersion"));
            return rv;
        }

    }

	/*------------------------------*/
	/* multipart/related parameters */
	/*------------------------------*/
	if (pContentTypeVal->params.isType == RV_TRUE)
	{
		if (pContentTypeVal->params.type.mediaType.type == RVSIP_MEDIATYPE_OTHER)
		{
			/* Allocate the media type token in a page */
			rv = ParserAllocFromObjPage(pParserMgr,
				&mediaTypeOffset,
				hPool,
				hPage,
				pContentTypeVal->params.type.mediaType.other.buf,
				pContentTypeVal->params.type.mediaType.other.len);
			if (RV_OK != rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitContentType - error in ParserAllocFromObjPage"));
				return rv;
			}
			
			/* Set the the media type into the header */
			rv =  SipContentTypeHeaderTypeParamSetMediaType(
				hConTypeHeader,
				NULL,
				hPool,
				hPage,
				mediaTypeOffset);
			if (RV_OK != rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitContentType - error in SipContentTypeHeaderSetMediaType"));
				return rv;
			}
		}
		else
		{
			rv = RvSipContentTypeHeaderTypeParamSetMediaType(
				hConTypeHeader,
				pContentTypeVal->params.type.mediaType.type,
				NULL);
			if (RV_OK != rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitContentType - error in RvSipContentTypeHeaderSetMediaType"));
				return rv;
			}
		}
		if (pContentTypeVal->params.type.mediaSubType.type == RVSIP_MEDIASUBTYPE_OTHER)
		{    /* Allocate the sub media type token in a page */
			rv = ParserAllocFromObjPage(pParserMgr,
				&subMediaTypeOffset,
				hPool,
				hPage,
				pContentTypeVal->params.type.mediaSubType.other.buf,
				pContentTypeVal->params.type.mediaSubType.other.len);
			if (RV_OK != rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitContentType - error in ParserAllocFromObjPage"));
				return rv;
			}
			
			/* Set the the sub media type into the header */
			rv =  SipContentTypeHeaderTypeParamSetMediaSubType(
				hConTypeHeader,
				NULL,
				hPool,
				hPage,
				subMediaTypeOffset);
			if (RV_OK != rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitContentType - error in SipContentTypeHeaderSetMediaSubType"));
				return rv;
			}
		}
		else
		{
			rv = RvSipContentTypeHeaderTypeParamSetMediaSubType(
				hConTypeHeader,
				pContentTypeVal->params.type.mediaSubType.type,
				NULL);
			if (RV_OK != rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitContentType - error in RvSipContentTypeHeaderSetMediaSubType"));
				return rv;
			}
		}
		
	}
	/* set start param */
    if (pContentTypeVal->params.isStart == RV_TRUE)
    {
		eStartAddrType = ParserAddrType2MsgAddrType(pContentTypeVal->params.start.uriType);
		if(eStartAddrType == RVSIP_ADDRTYPE_UNDEFINED)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitContentType - unknown parserAddressType %d", 
				pContentTypeVal->params.start.uriType));
			return RV_ERROR_UNKNOWN;
		}

        /* Construct the sip-url\abs-uri in the Content-Type header */
		rv = RvSipAddrConstructInContentTypeHeader(hConTypeHeader,eStartAddrType,&hStartAddr);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitContentType - error in RvSipAddrConstructInContentTypeHeader"));
			return rv;
		}

        if(pContentTypeVal->params.isOldAddrSpec == RV_TRUE)
        {
            rv = ParserInitAddressInHeader(pParserMgr, &pContentTypeVal->params.start,
			hPool, hPage, eStartAddrType, hStartAddr);
            if (RV_OK != rv)
		    {
			    RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				    "ParserInitContentType - error in ParserInitAddressInHeader - isOldAddrSpec = TRUE"));
			    return rv;
		    }	
        }
        else
        {
            /* Initialize the address values from the parser */
		    rv = ParserInitAddressInHeader(pParserMgr, &pContentTypeVal->params.start,
			    hPool, hPage, eStartAddrType, hStartAddr);
		    if (RV_OK != rv)
		    {
			    RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				    "ParserInitContentType - error in ParserInitAddressInHeader"));
			    return rv;
		    }		
        }
    }

    /* set other params */
    if (pGenericParamList != NULL)
    {
        if (0 != pGenericParamList->size)
        {
            RvInt32 otherParamOffset = UNDEFINED;
            otherParamOffset = ParserAppendCopyRpoolString(
                                       pParserMgr,
                                       hPool,
                                       hPage,
                                       pGenericParamList->hPool,
                                       pGenericParamList->hPage,
                                       0,
                                       pGenericParamList->size);
            if (UNDEFINED == otherParamOffset)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContentType - error in ParserAppendCopyRpoolString"));
                return RV_ERROR_OUTOFRESOURCES;
            }

            /* set the other params */
            rv = SipContentTypeHeaderSetOtherParams(
                                    hConTypeHeader,
                                    NULL,
                                    hPool,
                                    hPage,
                                    otherParamOffset);

            if (RV_OK!= rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContentType - error in SipContentTypeHeaderSetOtherParams"));
                return rv;
            }
        }
    }

    /* set compact form */
    rv = RvSipContentTypeHeaderSetCompactForm(hConTypeHeader, pContentTypeVal->isCompactForm);
    if (rv != RV_OK)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitContentType - error in setting the compact form"));
        return rv;
    }

    return RV_OK;

}
#else /*RV_SIP_PRIMITIVES*/
#ifndef RV_SIP_LIGHT
/***************************************************************************
 * ParserInitContentType
 * ------------------------------------------------------------------------
 * General: This function is used from the parser to init the message with the
 *         Content Type header values.
 * Return Value:RV_OK        - on success
 *              RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pParserMgr  - Pointer to the parser manager.
 *             pcbPointer  - Pointer to the current location of parser pointer
 *                           in msg buffer (the end of the given header).
 *             pContentTypeVal - Content type structure holding the values
 *                           from the parser in a token form.
 *    In/Output: pSipMsg     - Pointer to the structure in which the values
 *                           from the parser will be set.
 ***************************************************************************/
RvStatus RVCALLCONV ParserInitContentType(
                                  IN    ParserMgr         *pParserMgr,
                                  IN    RvUint8          *pcbPointer,
                                  IN    ParserContentType *pContentTypeVal,
                                  INOUT const void        *pSipMsg)
{

    /* The offset on the page that the content type has started (given as parameter
       to set it in the message)*/
    RvInt32            mediaTypeOffset = UNDEFINED;
    RvInt32            offset= UNDEFINED;
    RvStatus           rv;
    HRPOOL              hPool;
    HPAGE               hPage;
    ParserExtensionString * pGenericParamList =
            (ParserExtensionString *)pContentTypeVal->genericParamList;

    /* check if parser read all header, until CRLF.
    (if not, a missing CRLF syntax error will appear afterwards)*/
    if((*(pcbPointer)!=NULL_CHAR))
    {
        return RV_OK;
    }


    hPool = SipMsgGetPool(pSipMsg);
    hPage = SipMsgGetPage(pSipMsg);

    /* append the media type into the message page */
    rv =  ParserAppendFromExternal( pParserMgr,
                                    &mediaTypeOffset,
                                    hPool, hPage,
                                    pContentTypeVal->mediaType.buf,
                                    pContentTypeVal->mediaType.len);
    if (RV_OK!= rv)
    {
         RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
         "ParserInitContentType - error in ParserAppendFromExternal"));
         return rv;
    }

    /* append the '/' between the media type and the media sub type*/
    rv =  ParserAppendFromExternal( pParserMgr, &offset, hPool, hPage, "/", 1);
    if (RV_OK!= rv)
    {
         RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
         "ParserInitContentType - error in ParserAppendFromExternal"));
         return rv;
    }

    /* append the media sub type into the message page with null terminator
       at the end */
    rv =  ParserAllocFromObjPage(pParserMgr, &offset, hPool, hPage,
                                pContentTypeVal->mediaSubType.buf,
                                pContentTypeVal->mediaSubType.len);

    if (RV_OK!= rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitContentType - error in ParserAllocFromObjPage"));
        return rv;
    }


    /* If the content type contains parameters concatenate them into the
       page message*/
    if (pGenericParamList!=NULL)
    {
        if (0!=pGenericParamList->size)
        {
            /* append the media sub type into the message page  */
            rv =  ParserAppendFromExternal(pParserMgr,
                                        &offset,
                                        hPool,
                                        hPage,
                                        pContentTypeVal->mediaSubType.buf,
                                        pContentTypeVal->mediaSubType.len);

            if (RV_OK!= rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContentType - error in ParserAppendFromExternal"));
                return rv;
            }
            rv =  ParserAppendFromExternal(pParserMgr,
                                        &offset,
                                        hPool,
                                        hPage,
                                        ";",
                                        1);
            if (RV_OK!= rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContentType - error in ParserAppendFromExternal"));
                return rv;
            }
            offset = ParserAppendCopyRpoolString(pParserMgr,
                                                   hPool,
                                                   hPage,
                                                   pGenericParamList->hPool,
                                                   pGenericParamList->hPage,
                                                   0,
                                                   pGenericParamList->size);
            if (offset == UNDEFINED)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContentType - error in ParserAppendCopyRpoolString"));
                return RV_ERROR_OUTOFRESOURCES;
            }
        }
    }

    /* Set the Content type string into the message */
    return SipMsgSetContentTypeHeader(
                                  pSipMsg,
                                  NULL,
                                  hPool,
                                  hPage,
                                  mediaTypeOffset);
}

#endif /*#ifndef RV_SIP_LIGHT*/
#endif /* RV_SIP_PRIMITIVES */

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
* ParserInitContentID
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          ContentID header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr  - Pointer to the parser manager.
*             pcbPointer  - Pointer to the current location of parser pointer
*                           in msg buffer (the end of the given header).
*             pContentIDVal - pContentIDVal structure holding the ContentID
*                           values from the parser in a token form.
*             eType       - This ID indicates whether the parser is called to
*                           parse stand alone ContentID or sip message.
*  In/Output: pSipObject  - Pointer to the structure in which the values from
*                           the parser will be set.
*                           If eType == SIP_PARSEID_MSG it will be cast to
*                           RvSipMsgHandle and if eType == SIP_PARSEID_CONTENT_ID
*                           it will be cast to RvSipContentIDHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitContentID(
                                   IN    ParserMgr       *pParserMgr,
                                   IN    RvUint8         *pcbPointer,
                                   IN    ParserContentID *pContentIDVal,
                                   IN    SipParseType     eType,
                                   INOUT const void      *pSipObject)
{
    RvSipContentIDHeaderHandle   hContentIDHeader;
    RvSipBodyHandle              hBodyHandle;
	RvSipAddressHandle			 hAddr;
    RvSipAddressType		     eAddrType;
    RvStatus                     rv;
    HRPOOL                       hPool;
    HPAGE                        hPage;

    if (SIP_PARSETYPE_CONTENT_ID == eType)
    {
        /* The parser is used to parse Content-ID header */
        hContentIDHeader = (RvSipContentIDHeaderHandle)pSipObject;
    }
    else if (SIP_PARSETYPE_MIME_BODY_PART == eType)
    {
        /* The parser is used to parse message */
        pSipObject=(RvSipBodyPartHandle)pSipObject;

        /* Construct Body in the body part  */
        hBodyHandle = RvSipBodyPartGetBodyObject(pSipObject);
        if (NULL == hBodyHandle)
        {
            rv = RvSipBodyConstructInBodyPart(pSipObject,&hBodyHandle);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContentID - error in RvSipBodyConstructInBodyPart"));
                return rv;
            }
        }

        rv = RvSipContentIDHeaderConstructInBody(hBodyHandle,&hContentIDHeader);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContentID - error in RvSipContentIDHeaderConstructInBody"));
            return rv;
        }
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_CONTENT_ID, 
                                    pcbPointer, (void**)&hContentIDHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContentID - Failed to construct header"));
            return rv;
        }
    }
    
    hPool = SipContentIDHeaderGetPool(hContentIDHeader);
    hPage = SipContentIDHeaderGetPage(hContentIDHeader);

	/* set Addr param */
	eAddrType = ParserAddrType2MsgAddrType(pContentIDVal->addrSpec.uriType);
	if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			"ParserInitContentID - unknown parserAddressID %d", 
			pContentIDVal->addrSpec.uriType));
		return RV_ERROR_UNKNOWN;
	}

    /* Construct the sip-url\abs-uri in the Content-ID header */
	rv = RvSipAddrConstructInContentIDHeader(hContentIDHeader,eAddrType,&hAddr);
	if (RV_OK!=rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			"ParserInitContentID - error in RvSipAddrConstructInContentIDHeader"));
		return rv;
	}
	
    if(pContentIDVal->isOldAddrSpec == RV_TRUE)
    {
        rv = ParserInitAddressInHeader(pParserMgr, &pContentIDVal->addrSpec,
		hPool, hPage, eAddrType, hAddr);
        if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitContentID - error in ParserInitAddressInHeader - isOldAddrSpec = TRUE"));
			return rv;
		}	
    }
    else
    {
	    /* Initialize the address values from the parser */
	    rv = ParserInitAddressInHeader(pParserMgr, &pContentIDVal->addrSpec,
		                               hPool, hPage, eAddrType, hAddr);
	    if (RV_OK != rv)
	    {
		    RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			    "ParserInitContentID - error in ParserInitAddressInHeader"));
		    return rv;
	    }
    }

    return RV_OK;
}
#endif /*RV_SIP_PRIMITIVES*/

/***************************************************************************
 * ParserInitStatusLine
 * ------------------------------------------------------------------------
 * General: This function is used from the parser to init the message with the
 *          status line header values.
 * Return Value:RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pParserMgr   - Pointer to the parser manager.
 *             pStatusLine  - Status line structure holding the values
 *                            from the parser in a token form.
 *    In/Output: pSipMsg      - Pointer to the structure in which the values
 *                            from the parser will be set.
 ***************************************************************************/
RvStatus RVCALLCONV ParserInitStatusLine(
                                 IN    ParserMgr        *pParserMgr,
                                 IN    SipParser_pcb_type* pcb,
                                 IN    RvUint8          *pcbPointer,
                                 IN    ParserStatusLine *pStatusLine,
                                 INOUT const void       *pSipMsg)
{
    RvInt32    offsetReasonPharse = UNDEFINED;
    RvStatus   rv;
    HRPOOL      hPool;
    HPAGE       hPage;

    /* check if parser read all header, until CRLF.
    (if not, a missing CRLF syntax error will appear afterwards)*/
    if(*(pcbPointer) != NULL_CHAR)
    {
        return RV_OK;
    }

    /* Set the Status Code number into the message*/
    rv=RvSipMsgSetStatusCode(pSipMsg,pStatusLine->statusCode,RV_FALSE);
    if (RV_OK!= rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitStatusLine - error in RvSipMsgSetStatusCode"));
        return rv;
    }

    hPool = SipMsgGetPool(pSipMsg);
    hPage = SipMsgGetPage(pSipMsg);

    /* Change the reason parse token into reason parse string*/
    rv = ParserAllocFromObjPage(pParserMgr,
                                     &offsetReasonPharse,
                                     hPool,
                                     hPage,
                                     pStatusLine->reasonPhrase.buf,
                                     pStatusLine->reasonPhrase.len);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitStatusLine - error in ParserAllocFromObjPage"));
        return rv;
    }

    RV_UNUSED_ARG(pcb)
    /* Set the reason parse string into the message*/
    return SipMsgSetReasonPhrase(pSipMsg,
                                 NULL,
                                 hPool,
                                 hPage,
                                 offsetReasonPharse);
}

/***************************************************************************
 * ParserInitRequsetLine
 * ------------------------------------------------------------------------
 * General:This function is used from the parser to init the message with the
 *         request line header values.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    pSipMsg - Pointer to the structure in which the values from the parser
 *            will be set.
 *    requestLine - Request Line  structure holding the Request Line values
 *                from the parser in a token form.
 ***************************************************************************/
RvStatus RVCALLCONV ParserInitRequsetLine(
                                    IN    ParserMgr         *pParserMgr,
                                    IN    SipParser_pcb_type* pcb,
                                    IN    RvUint8          *pcbPointer,
                                    IN    ParserRequestLine *pRequestLine,
                                    INOUT const void        *pSipMsg)
{
    RvSipMethodType      eMethod = RVSIP_METHOD_UNDEFINED;
    RvInt32             offsetMethod = UNDEFINED;
    RvSipAddressHandle   hRequestUri;
    RvStatus            rv;
    HRPOOL              hPool;
    HPAGE               hPage;
    RvSipAddressType    eAddrType;


    /* check if parser read all header, until CRLF.
    (if not, a missing CRLF syntax error will appear afterwards)*/
    if(*(pcbPointer) != NULL_CHAR)
    {
        return RV_OK;
    }

    hPool = SipMsgGetPool(pSipMsg);
    hPage = SipMsgGetPage(pSipMsg);

    /* Change the token method structure into the enumerate method type or/and
       a string method type*/
    rv = ParserInitMethodType(pParserMgr,
                                 pRequestLine->method,
                                 hPool,
                                 hPage,
                                 &eMethod,
                                 &offsetMethod);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitRequsetLine - error in ParserInitMethodType"));
        return rv;
    }

    /* Set the method type enumerate and string values into the message*/
    rv = SipMsgSetMethodInRequestLine(pSipMsg,
                                         eMethod,
                                         NULL,
                                         hPool,
                                         hPage,
                                         offsetMethod);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitRequsetLine - error in MsgSetMethodInRequestLine"));
        return rv;
    }

    eAddrType = ParserAddrType2MsgAddrType(pRequestLine->exUri.uriType);
    if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitRequsetLine - unknown parserAddressType %d", pRequestLine->exUri.uriType));
        return RV_ERROR_UNKNOWN;
    }

    rv = RvSipAddrConstructInStartLine(pSipMsg, eAddrType, &hRequestUri);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitRequsetLine - error in RvSipAddrConstructInStartLine"));
        return rv;
    }

    /* Initialize the address values from the parser, we sent the type
       SIP_PARSETYPE_MSG because we cannot parse stand alone Request Line*/
    rv = ParserInitAddressInHeader(pParserMgr, &pRequestLine->exUri,
                                     hPool, hPage, eAddrType, hRequestUri);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitRequsetLine - error in ParserInitAddressInHeader"));
        return rv;
    }
    RV_UNUSED_ARG(pcb)
    return RV_OK;
}


/***************************************************************************
* ParserInitVia
* ------------------------------------------------------------------------
* General:This function is used from the parser to init the message with the
*         Via header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr - Pointer to the parser manager.
*             pVia       - Via  structure holding the Via values from the
*                          parser in a token form.
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone Via or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_VIA
*                          it will be cast to RvSipViaHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitVia(
                                   IN    ParserMgr          *pParserMgr,                                   
                                   IN    SipParser_pcb_type *pcb,
                                   IN    RvUint8            *pcbPointer,
                                   IN    ParserSingleVia    *pVia,
                                   IN    SipParseType       eType,
                                   INOUT const void         *pSipObject)
{
    RvStatus            rv;
    RvSipViaHeaderHandle hViaHeader;
    HRPOOL               hPool;
    HPAGE                hPage;

    if (SIP_PARSETYPE_VIA ==eType)
    {
        /*The parser is used to parse Via header */
        hViaHeader=(RvSipViaHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_VIA, 
                                    pcbPointer, (void**)&hViaHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitVia - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipViaHeaderGetPool(hViaHeader);
    hPage = SipViaHeaderGetPage(hViaHeader);

    /* Init the "sent by" parameters from the parser*/
    rv= ParserInitSentByInVia(pParserMgr, pVia, hPool, hPage, hViaHeader);
    if (RV_OK !=rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                     "ParserInitVia - error in function ParserInitSentByInVia"));
        return rv;
    }

    /* Init the via Parameters from the parser */
    rv = ParserInitParamsInVia(pParserMgr, pVia, hPool, hPage, hViaHeader);
    if (RV_OK !=rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                     "ParserInitVia - error in function ParserInitParamsInVia"));
        return rv;
    }

    /* set compact form */
    rv = RvSipViaHeaderSetCompactForm(hViaHeader, pVia->isCompact);
    if (rv != RV_OK)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitVia - error in setting the compact form"));
        return rv;
    }
    if (SIP_PARSETYPE_MSG==eType)
    {
        CleanHeaderBeforeComma((RvChar*)pcbPointer, pcb->pBuffer);
    }
    return RV_OK;
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
* ParserInitAllow
* ------------------------------------------------------------------------
* General:This function is used from the parser to init the message with the
*         Allow header values.
* Return Value:RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr - Pointer to the parser manager.
*             pMethod    - Method structure holding the Method values from the
*                          parser in a token form.
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone Allow or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_ALLOW
*                          it will be cast to RvSipAllowHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitAllow(
                                     IN    ParserMgr    *pParserMgr,
                                     IN    SipParser_pcb_type *pcb,
                                     IN    RvUint8     *pcbPointer,
                                     IN    ParserMethod *pMethod,
                                     IN    SipParseType eType,
                                     INOUT const void   *pSipObject)
{
    RvStatus              rv;
    RvSipAllowHeaderHandle hAllowHeader;
    RvSipMethodType        eMethod;
    RvInt32               offsetMethod = UNDEFINED;
    HRPOOL                 hPool;
    HPAGE                  hPage;

    if (SIP_PARSETYPE_ALLOW ==eType)
    {
        /*The parser is used to parse Allow header */
        hAllowHeader=(RvSipAllowHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_ALLOW, 
                                    pcbPointer, (void**)&hAllowHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitAllow - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipAllowHeaderGetPool(hAllowHeader);
    hPage = SipAllowHeaderGetPage(hAllowHeader);

    /* Change the token method structure into the enumerate method type or/and
       a string method type*/
    rv = ParserInitMethodType(pParserMgr,
                                *pMethod,
                                hPool,
                                hPage,
                                &eMethod,
                                &offsetMethod);
    if (RV_OK != rv)
    {
       RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitAllow - error in ParserInitMethodType"));
        return rv;
    }

    /* Set the enumerate method type and the string in the Allow header*/
    rv = SipAllowHeaderSetMethodType(hAllowHeader,
                                        eMethod,
                                        NULL,
                                        hPool,
                                        hPage,
                                        offsetMethod);
    if (RV_OK != rv)
    {
       RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitAllow - error in AllowHeaderSetMethodType"));
        return rv;
    }
    if (SIP_PARSETYPE_MSG==eType)
    {
        CleanHeaderBeforeComma((RvChar*)pcbPointer, pcb->pBuffer);
    }
    return RV_OK;
}
#endif /* RV_SIP_PRIMITIVES */



#ifndef RV_SIP_LIGHT
/***************************************************************************
* ParserInitParty
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          To header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr - Pointer to the parser manager.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             pToVal     - To structure holding the To values from the
*                          parser in a token form.
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone To or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_TO
*                          it will be cast to RvSipToHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitParty(
                             IN    ParserMgr          *pParserMgr,
                             IN    RvUint8           *pcbPointer,
                             IN    ParserPartyHeader  *pPartyVal,
                             IN    SipParseType       eType,
                             IN    SipParseType       eSpecificHeaderType,
                             INOUT const void         *pSipObject)
{
    RvStatus              rv;
    RvSipPartyHeaderHandle hPartyHeader;
    HRPOOL hPool;
    HPAGE  hPage;
    
    if (SIP_PARSETYPE_TO ==eType || SIP_PARSETYPE_FROM == eType)
    {
        /*The parser is used to parse Party header */
        hPartyHeader=(RvSipPartyHeaderHandle)pSipObject;
    }
    else
    {
        RvSipHeaderType ePartyType;
        ePartyType = (eSpecificHeaderType == SIP_PARSETYPE_TO)?RVSIP_HEADERTYPE_TO:RVSIP_HEADERTYPE_FROM;

        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, ePartyType, 
                                    pcbPointer, (void**)&hPartyHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitParty - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipPartyHeaderGetPool(hPartyHeader);
    hPage = SipPartyHeaderGetPage(hPartyHeader);

    /* Init the party header in the message*/
    rv = ParserInitPartyHeader(pParserMgr,pPartyVal,hPool, hPage, &hPartyHeader);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
          "ParserInitParty - error in InitPartyHeaderByParser"));
        return rv;
    }

    return RV_OK;
}
#endif /*#ifndef RV_SIP_LIGHT*/

#ifndef RV_SIP_LIGHT
/***************************************************************************
* ParserInitDateHeader
* ------------------------------------------------------------------------
* General: This function is used to init the Date handle with the
*          date values from the parser.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr - Pointer to the parser manager.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             pDateVal   - Date structure holding the Date values from the
*                          parser in a token form.
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone Date or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_DATE
*                          it will be cast to RvSipDateHandle.
***************************************************************************/
RvStatus  RVCALLCONV ParserInitDateHeader(
                                           IN    ParserMgr       *pParserMgr,
                                           IN    RvUint8        *pcbPointer,
                                           IN    ParserSipDate   *pDateVal,
                                           IN    SipParseType    eType,
                                           INOUT const void      *pSipObject)
{
    RvStatus rv;
    RvSipDateHeaderHandle hDateHeader;

    if (SIP_PARSETYPE_DATE ==eType)
    {
        /*The parser is used to parse Party header */
        hDateHeader=(RvSipDateHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_DATE, 
                                    pcbPointer, (void**)&hDateHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitDateHeader - Failed to construct header"));
            return rv;
        }
    }

    return ParserInitDateInHeader(pParserMgr,pDateVal, (RvSipDateHeaderHandle)hDateHeader);

}
#endif /*#ifndef RV_SIP_LIGHT*/
#ifndef RV_SIP_LIGHT
/***************************************************************************
 * ParserInitExpiresHeader
 * ------------------------------------------------------------------------
 * General: This function is used to init a sip url handle with the
 *          url values from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pParserMgr  - Pointer to the parser manager.
 *             pcbPointer  - Pointer to the current location of parser pointer
 *                           in msg buffer (the end of the given header).
 *             pExpiresVal - the expires header value.
 ***************************************************************************/
RvStatus RVCALLCONV ParserInitExpiresHeader(
                          IN    ParserMgr            *pParserMgr,
                          IN    RvUint8             *pcbPointer,
                          IN    ParserExpiresHeader  *pExpiresVal,
                          IN    SipParseType         eType,
                          INOUT const void           *pSipObject)
{
    RvSipExpiresHeaderHandle hExpiresHeader;
    RvStatus               rv;
    HRPOOL                  hPool;
    HPAGE                   hPage;

    /* Check that we get here only if we parse stand alone header */
    if (SIP_PARSETYPE_EXPIRES ==eType)
    {
        /*The parser is used to parse expires stand alone header */
        hExpiresHeader=(RvSipExpiresHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_EXPIRES, 
                                    pcbPointer, (void**)&hExpiresHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitExpiresHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipExpiresHeaderGetPool(hExpiresHeader);
    hPage = SipExpiresHeaderGetPage(hExpiresHeader);

    return ParserInitExpiresHeaderInHeader(pParserMgr,
                                           pExpiresVal,
                                           hPool,
                                           hPage,
                                           (RvSipExpiresHeaderHandle)hExpiresHeader);
}
#endif /*#ifndef RV_SIP_LIGHT*/
#ifndef RV_SIP_LIGHT
/***************************************************************************
* ParserInitContactHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          Contact header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr - Pointer to the parser manager.
*             pContactVal- Contact structure holding the Contact values from the
*                          parser in a token form.
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone Contact or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_CONTACT
*                          it will be cast to RvSipContactHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitContactHeader(
                             IN    ParserMgr                 *pParserMgr,
                             IN    SipParser_pcb_type       *pcb,
                             IN    RvUint8                  *pcbPointer,
                             IN    ParserContactHeaderValues *pContactVal,
                             IN    SipParseType              eType,
                             INOUT const void                *pSipObject)
{
    RvStatus                rv;
    RvSipContactHeaderHandle hContactHeader;
    RvSipAddressHandle       hAddress;
    HRPOOL                   hPool;
    HPAGE                    hPage;
    RvSipAddressType         eAddrType;

    if (SIP_PARSETYPE_CONTACT ==eType)
    {
        /*The parser is used to parse Contact header */
        hContactHeader=(RvSipContactHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_CONTACT, 
                                   pcbPointer, (void**)&hContactHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipContactHeaderGetPool(hContactHeader);
    hPage = SipContactHeaderGetPage(hContactHeader);

    /* There is a possibility that the contact header will be the following :
       "Contact:*".
       In this cases we will create a new contact header with the flag. */
    if (RV_TRUE == pContactVal->isStar)
    {
        rv = RvSipContactHeaderSetStar(hContactHeader);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
            "ParserInitContactHeader - error in RvSipContactHeaderSetStar"));
        }
        return RV_OK;
    }

    eAddrType = ParserAddrType2MsgAddrType(pContactVal->header.nameAddr.exUri.uriType);
    if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitContactHeader - unknown parserAddressType %d", pContactVal->header.nameAddr.exUri.uriType));
        return RV_ERROR_UNKNOWN;
    }

    /* Construct the url in the Contact header */
    rv = RvSipAddrConstructInContactHeader(hContactHeader,
                                              eAddrType,
                                              &hAddress);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitContactHeader - error in RvSipAddrConstructInContactHeader"));
        return rv;
    }

    /* Initialize the sip-url\abs-uri values from the parser*/
    rv = ParserInitAddressInHeader(pParserMgr,
                                      &pContactVal->header.nameAddr.exUri,
                                      hPool, hPage, eAddrType, hAddress);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitContactHeader - error in ParserInitAddressInHeader"));
        return rv;
    }

	/* Check whether the display name has been set, if so attach
       it to the Contact header  */
    if (RV_TRUE == pContactVal->header.nameAddr.isDisplayName)
    {
        RvInt32 offsetName = UNDEFINED;
        rv= ParserAllocFromObjPage(pParserMgr,
                                     &offsetName,
                                     hPool,
                                     hPage,
                                     pContactVal->header.nameAddr.name.buf,
                                     pContactVal->header.nameAddr.name.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitContactHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

#ifndef RV_SIP_JSR32_SUPPORT
		/* This is the regular case where the display name is part of the contact header */
        /* Set the display name in the Contact header */
        rv = SipContactHeaderSetDisplayName(hContactHeader,
                                               NULL,
                                               hPool,
                                               hPage,
                                               offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitContactHeader - error in ContactHeaderSetDisplayName"));
            return rv;
        }
#else
		/* This is the jsr32 case where the display name is part of the address object */
		rv = SipAddrSetDisplayName(hAddress,
									  NULL,
									  hPool,
						  			  hPage,
									  offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
					   "ParserInitContactHeader - error in SipAddrSetDisplayName"));
            return rv;
        }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    }

    /* Check whether Contact parameters have been set*/
    if (RV_TRUE == pContactVal->header.isParams)
    {
        rv = ParserInitContactParams(pParserMgr,pContactVal,hPool, hPage, hContactHeader);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitContactHeader - error in InitContactParamsByParser"));
            return rv;
        }
    }


    /* set compact form */
    rv = RvSipContactHeaderSetCompactForm(hContactHeader, pContactVal->isCompact);
    if (rv != RV_OK)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitContactHeader - error in setting the compact form"));
        return rv;
    }

    if (SIP_PARSETYPE_MSG==eType)
    {
        CleanHeaderBeforeComma((RvChar*)pcbPointer, pcb->pBuffer);
    }
    return RV_OK;
}

#endif /*#ifndef RV_SIP_LIGHT*/
/****************  R O U T E  H E A D E R *******************/

/***************************************************************************
 * ParserInitRoute
 * ------------------------------------------------------------------------
 * General: This function is used from the parser to init the message with the
 *          Route header values.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pParserMgr - Pointer to the parser manager.
 *             pRouteVal  - Route structure holding the Route values from the
 *                          parser in a token form.
 *             eType      - This type indicates whether the parser is called to
 *                          parse stand alone Route or sip message.
 *  In/Output: pSipObject - Pointer to the structure in which the values from
 *                          the parser will be set.
 *                          If eType == SIP_PARSETYPE_MSG it will be cast to
 *                          RvSipMsgHandle and if eType == SIP_PARSETYPE_ROUTE
 *                          it will be cast to RvSipContactHandle.
 ***************************************************************************/
RvStatus RVCALLCONV ParserInitRoute(
                                       IN    ParserMgr          *pParserMgr,
                                       IN    SipParser_pcb_type *pcb,
                                       IN    RvUint8           *pcbPointer,
                                       IN    ParserRouteHeader  *pRouteVal,
                                       IN    SipParseType       eType,
                                       INOUT const void         *pSipObject)
{
    RvStatus                 rv;
    RvSipRouteHopHeaderHandle hRouteHeader;
    RvSipAddressHandle        hAddress;
    RvInt32                  paramsOffset;
    HRPOOL                    hPool;
    HPAGE                     hPage;
    RvSipAddressType eAddrType;


    if (SIP_PARSETYPE_ROUTE == eType ||
        SIP_PARSETYPE_RECORD_ROUTE == eType ||
        SIP_PARSETYPE_PATH == eType  ||
        SIP_PARSETYPE_SERVICE_ROUTE == eType)
    {
        /*The parser is used to parse Contact header */
        hRouteHeader=(RvSipRouteHopHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_ROUTE_HOP, 
                                    pcbPointer, (void**)&hRouteHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitRoute - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipRouteHopHeaderGetPool(hRouteHeader);
    hPage = SipRouteHopHeaderGetPage(hRouteHeader);

    /* set the route header to be route or record-route */
    if (SIP_PARSETYPE_RECORD_ROUTE == pRouteVal->eRouteType)
    {
        rv = RvSipRouteHopHeaderSetHeaderType(hRouteHeader, RVSIP_ROUTE_HOP_RECORD_ROUTE_HEADER);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
            "ParserInitRoute - error in RvSipRouteHopHeaderSetHeaderType"));
        }
    }
    else if (SIP_PARSETYPE_ROUTE == pRouteVal->eRouteType) 
    {
        rv = RvSipRouteHopHeaderSetHeaderType(hRouteHeader, RVSIP_ROUTE_HOP_ROUTE_HEADER);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
            "ParserInitRoute - error in RvSipRouteHopHeaderSetHeaderType"));
        }
    }
    else if (SIP_PARSETYPE_PATH == pRouteVal->eRouteType) 
    {
        rv = RvSipRouteHopHeaderSetHeaderType(hRouteHeader, RVSIP_ROUTE_HOP_PATH_HEADER);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                "ParserInitRoute - error in RvSipRouteHopHeaderSetHeaderType"));
        }
    }
    else if (SIP_PARSETYPE_SERVICE_ROUTE == pRouteVal->eRouteType) 
    {
        rv = RvSipRouteHopHeaderSetHeaderType(hRouteHeader, RVSIP_ROUTE_HOP_SERVICE_ROUTE_HEADER);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                "ParserInitRoute - error in RvSipRouteHopHeaderSetHeaderType"));
        }
    }

    /* set the name address */
    eAddrType = ParserAddrType2MsgAddrType(pRouteVal->nameAddr.exUri.uriType);
    if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitRoute - unknown parserAddressType %d", pRouteVal->nameAddr.exUri.uriType));
        return RV_ERROR_UNKNOWN;
    }

   /* Construct the url in the Route header */
    rv = RvSipAddrConstructInRouteHopHeader(hRouteHeader,
                                                   eAddrType,
                                                   &hAddress);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitRoute - error in RvSipAddrConstructInRouteHopHeader"));
        return rv;
    }

    /* Initialize the sip url values from the parser*/
    rv = ParserInitAddressInHeader(pParserMgr,
									  &pRouteVal->nameAddr.exUri,
                                      hPool,
                                      hPage,
                                      eAddrType,
                                      hAddress);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitRoute - error in ParserInitAddressInHeader"));
        return rv;
    }

	if (RV_TRUE == pRouteVal->nameAddr.isDisplayName)
    {
        RvInt32 offsetName = UNDEFINED;
        rv= ParserAllocFromObjPage(pParserMgr,
                                     &offsetName,
                                     hPool,
                                     hPage,
                                     pRouteVal->nameAddr.name.buf,
                                     pRouteVal->nameAddr.name.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitRoute - error in ParserAllocFromObjPage"));
            return rv;
        }

#ifndef RV_SIP_JSR32_SUPPORT
        /* Set the display name in the Route header */
		/* This is the regular case where the display name is part of the route header */
        rv = SipRouteHopHeaderSetDisplayName(hRouteHeader,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitRoute - error in SipRouteHeaderSetDisplayName"));
            return rv;
        }
#else
		/* This is the jsr32 case where the display name is part of the address object */
        rv = SipAddrSetDisplayName(hAddress,
									  NULL,
									  hPool,
						   			  hPage,
									  offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				       "ParserInitRoute - error in SipAddrSetDisplayName"));
            return rv;
        }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    }


    /* set the route params */
    if (RV_TRUE == pRouteVal->isParams &&
        NULL != pRouteVal->routeParams)
    {
		ParserExtensionString * pOtherParam = (ParserExtensionString*)pRouteVal->routeParams;
		
		paramsOffset = ParserAppendCopyRpoolString(pParserMgr,
													hPool,
													hPage,
													pOtherParam->hPool,
													pOtherParam->hPage,
													0,
													pOtherParam->size);
		if (UNDEFINED == paramsOffset)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				"ParserInitRoute - error in ParserAppendCopyRpoolString"));
			return RV_ERROR_OUTOFRESOURCES;
        }
		
        /* Set the other params in the Route header */
		rv = SipRouteHopHeaderSetOtherParams(hRouteHeader,
												NULL,
												hPool,
												hPage,
												paramsOffset);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				"ParserInitRoute - error in SipRouteHopHeaderSetOtherParams"));
            return rv;
        }
    }

    if (SIP_PARSETYPE_MSG==eType)
    {
        CleanHeaderBeforeComma((RvChar*)pcbPointer, pcb->pBuffer);
    }

    return RV_OK;
}

#ifndef RV_SIP_PRIMITIVES

/*------------------------------------------------------------------------*/
/*                          SUBS-REFER FUNCTIONS                          */
/*------------------------------------------------------------------------*/
#ifdef RV_SIP_SUBS_ON
/***************************************************************************
* ParserInitReferTo
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          Refer - To header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:   pParserMgr - Pointer to the parser manager.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             pReferToHeaderVal - ReferTo structure holding the refer-to values
*                          from the parser in a token form.
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone ReferTo or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_REFER_TO
*                          it will be cast to RvSipReferToHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitReferTo(
                                       IN    ParserMgr           *pParserMgr,
                                       IN    RvUint8            *pcbPointer,
                                       IN    ParserReferToHeader *pReferToHeaderVal,
                                       IN    SipParseType        eType,
                                       INOUT const void          *pSipObject)
{
    RvStatus               rv;
    RvSipReferToHeaderHandle hRefToHeader;
    RvSipAddressHandle      hRequestUri;
    HRPOOL                  hPool;
    HPAGE                   hPage;
    RvSipAddressType eAddrType;


    if (SIP_PARSETYPE_REFER_TO ==eType)
    {
        /*The parser is used to parse Other header */
        hRefToHeader=(RvSipReferToHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_REFER_TO, 
                                    pcbPointer, (void**)&hRefToHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitReferTo - Failed to construct header"));
            return rv;
        }
    }


    hPool = SipReferToHeaderGetPool(hRefToHeader);
    hPage = SipReferToHeaderGetPage(hRefToHeader);

    /* Construct a sip-url\abs-uri structure in the start line of the message*/
    eAddrType = ParserAddrType2MsgAddrType(pReferToHeaderVal->nameAddr.exUri.uriType);
    if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitReferTo - unknown parserAddressType %d", pReferToHeaderVal->nameAddr.exUri.uriType));
        return RV_ERROR_UNKNOWN;
    }

    rv = RvSipAddrConstructInReferToHeader(hRefToHeader,
                                              eAddrType,
                                              &hRequestUri);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitReferTo - error in RvSipAddrConstructInReferToHeader"));
        return rv;
    }

    /* Initialize the sip-url\abs-uri values from the parser, we sent the type
       SIP_PARSETYPE_MSG because we cannot parse stand alone Request Line*/
    rv = ParserInitAddressInHeader(pParserMgr,
									 &(pReferToHeaderVal->nameAddr.exUri),
                                     hPool,
                                     hPage,
                                     eAddrType,
                                     hRequestUri);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitReferTo - ParserInitAddressInHeader"));
        return rv;
    }

	 /* Check whether the display name has been set, if so attach
       it to the Contact header  */
    if (RV_TRUE == pReferToHeaderVal->nameAddr.isDisplayName)
    {
        RvInt32 offsetName = UNDEFINED;
        rv= ParserAllocFromObjPage(pParserMgr,
                                     &offsetName,
                                     hPool,
                                     hPage,
                                     pReferToHeaderVal->nameAddr.name.buf,
                                     pReferToHeaderVal->nameAddr.name.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitReferTo - error in ParserAllocFromObjPage"));
            return rv;
        }
#ifndef RV_SIP_JSR32_SUPPORT
        /* Set the display name in the Contact header */
        /* This is the regular case where the display name is part of the route header */
		rv = SipReferToHeaderSetDisplayName(hRefToHeader,
                                               NULL,
                                               hPool,
                                               hPage,
                                               offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
						"ParserInitReferTo - error in SipReferToHeaderSetDisplayName"));
            return rv;
        }
#else
		/* This is the jsr32 case where the display name is part of the address object */
		rv = SipAddrSetDisplayName(hRequestUri,
                                     NULL,
                                     hPool,
                                     hPage,
                                     offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
						"ParserInitReferTo - error in SipAddrSetDisplayName"));
            return rv;
        }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    }

    /* set compact form */
    rv = RvSipReferToHeaderSetCompactForm(hRefToHeader, pReferToHeaderVal->isCompact);
    if (rv != RV_OK)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitReferTo - error in setting the compact form"));
        return rv;
    }

    /* set other params */
    if(pReferToHeaderVal->isExtention == RV_TRUE)
    {
        RvInt32 offsetOtherParam = UNDEFINED;
        ParserExtensionString* pExtension = (ParserExtensionString *)pReferToHeaderVal->exten;

        offsetOtherParam = ParserAppendCopyRpoolString(
                                pParserMgr,
                                hPool,
                                hPage,
                                pExtension->hPool,
                                pExtension->hPage,
                                0,
                                pExtension->size);

        
        if (UNDEFINED == offsetOtherParam)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitReferTo - error in ParserAppendCopyRpoolString"));
            return RV_ERROR_OUTOFRESOURCES;
        }
        rv = SipReferToHeaderSetOtherParams(
                            hRefToHeader,
                            NULL,
                            hPool,
                            hPage,
                            offsetOtherParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitReferTo - error in SipReferToHeaderSetOtherParams"));
            return rv;
        }
    }

    return RV_OK;
}
#endif /* #ifdef RV_SIP_SUBS_ON - ReferTo Header */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
* ParserInitRefferredBy
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          Refer - By header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr - Pointer to the parser manager.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             pReferredByHeaderVal - Referred-By structure holding the referred-by
*                          values from the parser in a token form.
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone Referred-by or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_REFERRED_BY
*                          it will be cast to RvSipReferredByHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitRefferredBy(
                               IN    ParserMgr              *pParserMgr,
                               IN    RvUint8               *pcbPointer,
                               IN    ParserReferredByHeader *pReferredByHeaderVal,
                               IN    SipParseType           eType,
                               INOUT const void             *pSipObject)
{
    RvStatus   rv;
    RvSipAddressHandle          hReferrerAddress;
    RvSipReferredByHeaderHandle hRefByHeader;
    HRPOOL      hPool;
    HPAGE       hPage;
    RvSipAddressType eAddrType;


    if (SIP_PARSETYPE_REFERRED_BY ==eType)
    {
        /*The parser is used to parse Other header */
        hRefByHeader=(RvSipReferredByHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_REFERRED_BY, 
                                    pcbPointer, (void**)&hRefByHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitRefferredBy - Failed to construct header"));
            return rv;
        }
    }


    hPool = SipReferredByHeaderGetPool(hRefByHeader);
    hPage = SipReferredByHeaderGetPage(hRefByHeader);

    /* Construct the sip-url\abs-uri in the Referrer value */
    eAddrType = ParserAddrType2MsgAddrType(pReferredByHeaderVal->referrerAddrSpec.exUri.uriType);
    if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitRefferredBy - unknown parserAddressType %d", pReferredByHeaderVal->referrerAddrSpec.exUri.uriType));
        return RV_ERROR_UNKNOWN;
    }

    rv = RvSipAddrConstructInReferredByHeader(hRefByHeader,
                                                     eAddrType,
                                                     &hReferrerAddress);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitRefferredBy - error in RvSipAddrConstructInReferredByHeader"));
        return rv;
    }

    rv = ParserInitAddressInHeader(pParserMgr,
									  &pReferredByHeaderVal->referrerAddrSpec.exUri,
                                       hPool,
                                       hPage,
                                       eAddrType,
                                       hReferrerAddress);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitRefferredBy - error in ParserInitAddressInHeader"));
        return rv;
    }

	/* Check whether the display name has been set, if so attach
       it to the Referred-By header  */
    if (RV_TRUE == pReferredByHeaderVal->referrerAddrSpec.isDisplayName)
    {
        RvInt32 offsetName = UNDEFINED;
        rv= ParserAllocFromObjPage(pParserMgr,
                                      &offsetName,
                                      hPool,
                                      hPage,
                                      pReferredByHeaderVal->referrerAddrSpec.name.buf ,
                                      pReferredByHeaderVal->referrerAddrSpec.name.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitRefferredBy - error in ParserAllocFromObjPage"));
            return rv;
        }
#ifndef RV_SIP_JSR32_SUPPORT
        /* Set the display name in the Referred-By header */
        /* This is the regular case where the display name is part of the route header */
		rv = SipReferredByHeaderSetReferrerDispName(hRefByHeader,
                                             NULL,
                                             hPool,
                                             hPage,
                                             offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitRefferredBy - error in PartyHeaderSetDisplayName"));
            return rv;
        }
#else
		/* This is the jsr32 case where the display name is part of the address object */
		rv = SipAddrSetDisplayName(hReferrerAddress,
                                      NULL,
                                      hPool,
                                      hPage,
                                      offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitRefferredBy - error in SipAddrSetDisplayName"));
            return rv;
        }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    }

     /* Check whether the cid parameter has been set, if so attach it to the Referred-By header  */
    if (RV_TRUE == pReferredByHeaderVal->isCid)
    {
        RvInt32 offsetCid = UNDEFINED;
        rv= ParserAllocFromObjPage(pParserMgr,
                                      &offsetCid,
                                      hPool,
                                      hPage,
                                      pReferredByHeaderVal->cidParam.buf ,
                                      pReferredByHeaderVal->cidParam.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitRefferredBy - error in ParserAllocFromObjPage"));
            return rv;
        }

        /* Set the cid in the Referred-By header */
        rv = SipReferredByHeaderSetCidParam(hRefByHeader,
                                             NULL,
                                             hPool,
                                             hPage,
                                             offsetCid);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitRefferredBy - error in SipReferredByHeaderSetCidParam"));
            return rv;
        }
    }

    /* setting generic params */
    if (pReferredByHeaderVal->genericParams != NULL)
    {
        RvInt32 offsetParams = UNDEFINED;

        offsetParams = ParserAppendCopyRpoolString(
                    pParserMgr,
                    hPool,
                    hPage,
                    ((ParserExtensionString *)pReferredByHeaderVal->genericParams)->hPool,
                    ((ParserExtensionString *)pReferredByHeaderVal->genericParams)->hPage,
                    0,
                    ((ParserExtensionString *)pReferredByHeaderVal->genericParams)->size);

        if (UNDEFINED == offsetParams)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitRefferredBy - error in ParserAppendCopyRpoolString"));
            return RV_ERROR_OUTOFRESOURCES;
        }

        /* Set the generic params in the Referred header */
        rv = SipReferredByHeaderSetOtherParams(
                    hRefByHeader,
                    NULL,
                    hPool,
                    hPage,
                    offsetParams);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitRefferredBy - error in SipReferredByHeaderSetSignatureParams"));
            return rv;
        }
    }


    /* set compact form */
    rv = RvSipReferredByHeaderSetCompactForm(hRefByHeader, pReferredByHeaderVal->isCompact);
    if (rv != RV_OK)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitRefferredBy - error in setting the compact form"));
        return rv;
    }

    return RV_OK;

}
#endif /* #ifdef RV_SIP_SUBS_ON - ReferredBy Header */

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
* ParserInitEvent
* ------------------------------------------------------------------------
* General:This function is used from the parser to init the message with the
*         Event header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr  - Pointer to the parser manager.
*             pcbPointer  - Pointer to the current location of parser pointer
*                           in msg buffer (the end of the given header).
*             pEvent      - ParserEvent structure holding the event values from the
*                           parser in a token form.
*             eType       - This type indicates whether the parser is called to
*                           parse stand alone Event or sip message.
*  In/Output: pSipObject  - Pointer to the structure in which the values from
*                           the parser will be set.
*                           If eType == SIP_PARSETYPE_MSG it will be cast to
*                           RvSipMsgHandle and if eType == SIP_PARSETYPE_EVENT
*                           it will be cast to RvSipEventHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitEvent(
                             IN    ParserMgr     *pParserMgr,
                             IN    RvUint8      *pcbPointer,
                             IN    ParserEvent   *pEvent,
                             IN    SipParseType  eType,
                             INOUT const void    *pSipObject)
{
    RvSipEventHeaderHandle hEvent;
    RvStatus              rv = RV_OK;
    RvInt32               offsetPackage = UNDEFINED;
    HRPOOL                 hPool;
    HPAGE                  hPage;

    if (SIP_PARSETYPE_EVENT == eType)
    {
        /*The parser is used to parse Event header */
        hEvent=(RvSipEventHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_EVENT, 
                                    pcbPointer, (void**)&hEvent);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitEvent - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipEventHeaderGetPool(hEvent);
    hPage = SipEventHeaderGetPage(hEvent);

    /* set compact form */
    if(pEvent->isCompact == RV_TRUE)
    {
        rv = RvSipEventHeaderSetCompactForm(hEvent, RV_TRUE);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitEvent - error in RvSipEventHeaderSetCompactForm"));
            return rv;
        }
    }

    /* Set event-package */
    rv = ParserAllocFromObjPage(pParserMgr,
                                     &offsetPackage,
                                     hPool,
                                     hPage,
                                     pEvent->eventType.package.buf,
                                     pEvent->eventType.package.len);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitEvent - error in ParserAllocFromObjPage"));
        return rv;
    }

    rv = SipEventHeaderSetEventPackage(hEvent,
                                            NULL,
                                            hPool,
                                            hPage,
                                            offsetPackage);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitEvent - error in RvSipEventHeaderSetEventPackage"));
        return rv;
    }

    /* Set event-template */
    if(pEvent->eventType.isTemplate == RV_TRUE)
    {
        RvInt32 templateOffset;

         rv = ParserAllocFromObjPage(pParserMgr,
                                         &templateOffset,
                                         hPool,
                                         hPage,
                                         pEvent->eventType.templateVal.buf,
                                         pEvent->eventType.templateVal.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitEvent - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipEventHeaderSetEventTemplate(hEvent,
                                                NULL,
                                                hPool,
                                                hPage,
                                                templateOffset);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitEvent - error in RvSipEventHeaderSetEventTemplate"));
            return rv;
        }
    }

    /* Set event-params */
    if(pEvent->isEventParams == RV_TRUE)
    {
        /* Set event-id */
        if(pEvent->eventParams.isEventId == RV_TRUE)
        {
            RvInt32 idOffset;

            rv = ParserAllocFromObjPage(pParserMgr,
                                            &idOffset,
                                            hPool,
                                            hPage,
                                            pEvent->eventParams.eventId.buf,
                                            pEvent->eventParams.eventId.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitEvent - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipEventHeaderSetEventId(hEvent,
                                            NULL,
                                            hPool,
                                            hPage,
                                            idOffset);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitEvent - error in SipEventHeaderSetEventId"));
                return rv;
            }
        }

        if(pEvent->eventParams.isExtention == RV_TRUE)
        {
            RvInt32 offsetOtherParam = UNDEFINED;
            ParserExtensionString* pExtension = (ParserExtensionString *)pEvent->eventParams.exten;

            offsetOtherParam = ParserAppendCopyRpoolString(
                                        pParserMgr,
                                        hPool,
                                        hPage,
                                        pExtension->hPool,
                                        pExtension->hPage,
                                        0,
                                        pExtension->size);

            if (UNDEFINED == offsetOtherParam)
            {
                 RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                          "ParserInitEvent - error in ParserAppendCopyRpoolString"));
                 return RV_ERROR_OUTOFRESOURCES;
            }
            rv = SipEventHeaderSetEventParam(
                                       hEvent,
                                       NULL,
                                       hPool,
                                       hPage,
                                       offsetOtherParam);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitEvent - error in SipEventHeaderSetEventParam"));
                return rv;
            }
        }
    }
    return  RV_OK;
}
#endif /* #ifndef RV_SIP_PRIMITIVES - Event Header */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * ParserInitAllowEvents
 * ------------------------------------------------------------------------
 * General:This function is used from the parser to init the message with the
 *         Allow-Events header values.
 * Return Value:RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pParserMgr  - Pointer to the parser manager.
 *             pAllowEvents - Allow-events structure holding the values from the
 *                          parser.
 *             eType      - This type indicates whether the parser is called to
 *                          parse stand alone Allow or sip message.
 *  In/Output: pSipObject - Pointer to the structure in which the values from
 *                          the parser will be set.
 *                          If eType == SIP_PARSETYPE_MSG it will be cast to
 *                          RvSipMsgHandle and if eType == SIP_PARSETYPE_ALLOW_EVENTS
 *                          it will be cast to RvSipAllowEventsHandle.
 ***************************************************************************/
RvStatus RVCALLCONV ParserInitAllowEvents(
                           IN    ParserMgr         *pParserMgr,
                           IN    SipParser_pcb_type *pcb,
                           IN    RvUint8          *pcbPointer,
                           IN    ParserAllowEvents *pAllowEvents,
                           IN    SipParseType      eType,
                           INOUT const void        *pSipObject)
{
    RvStatus              rv;
    RvSipAllowEventsHeaderHandle hAllowEvents;
    RvInt32               offsetPackage = UNDEFINED;
    HRPOOL                 hPool;
    HPAGE                  hPage;

    if (SIP_PARSETYPE_ALLOW_EVENTS == eType)
    {
        /*The parser is used to parse Allow header */
        hAllowEvents=(RvSipAllowEventsHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_ALLOW_EVENTS, 
                                    pcbPointer, (void**)&hAllowEvents);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitAllowEvents - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipAllowEventsHeaderGetPool(hAllowEvents);
    hPage = SipAllowEventsHeaderGetPage(hAllowEvents);

    /* set compact form */
    if(pAllowEvents->isCompact == RV_TRUE)
    {
        rv = RvSipAllowEventsHeaderSetCompactForm(hAllowEvents, RV_TRUE);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitAllowEvents - error in RvSipAllowEventsHeaderSetCompactForm"));
            return rv;
        }
    }

       /* Set event-package */
    rv = ParserAllocFromObjPage(pParserMgr,
                                     &offsetPackage,
                                     hPool,
                                     hPage,
                                     pAllowEvents->eventType.package.buf,
                                     pAllowEvents->eventType.package.len);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitAllowEvents - error in ParserAllocFromObjPage"));
        return rv;
    }

    rv = SipAllowEventsHeaderSetEventPackage(hAllowEvents,
                                            NULL,
                                            hPool,
                                            hPage,
                                            offsetPackage);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitAllowEvents - error in RvSipEventHeaderSetEventPackage"));
        return rv;
    }

    /* Set event-template */
    if(pAllowEvents->eventType.isTemplate == RV_TRUE)
    {
        RvInt32 templateOffset;

        rv = ParserAllocFromObjPage(pParserMgr,
                                         &templateOffset,
                                         hPool,
                                         hPage,
                                         pAllowEvents->eventType.templateVal.buf,
                                         pAllowEvents->eventType.templateVal.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitAllowEvents - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipAllowEventsHeaderSetEventTemplate(hAllowEvents,
                                                NULL,
                                                hPool,
                                                hPage,
                                                templateOffset);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitAllowEvents - error in SipAllowEventsHeaderSetEventTemplate"));
            return rv;
        }
    }
    if (SIP_PARSETYPE_MSG==eType)
    {
        CleanHeaderBeforeComma((RvChar*)pcbPointer, pcb->pBuffer);
    }
    return RV_OK;
}
#endif /* #ifdef RV_SIP_SUBS_ON - AllowEvents Header */

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
* ParserInitSubsState
* ------------------------------------------------------------------------
* General:This function is used from the parser to init the message with the
*         Subscription-State header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr  - Pointer to the parser manager.
*             pcbPointer  - Pointer to the current location of parser pointer
*                           in msg buffer (the end of the given header).
*             pSubstate   - ParserEvent structure holding the event values from the
*                           parser in a token form.
*             eType       - This type indicates whether the parser is called to
*                           parse stand alone subscription-state or sip message.
*  In/Output: pSipObject  - Pointer to the structure in which the values from
*                           the parser will be set.
*                           If eType == SIP_PARSETYPE_MSG it will be cast to
*                           RvSipMsgHandle and if eType == SIP_PARSETYPE_SUBSCRIPTION_STATE
*                           it will be cast to RvSipSubscriptionStateHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitSubsState(
                             IN    ParserMgr                 *pParserMgr,
                             IN    RvUint8                  *pcbPointer,
                             IN    ParserSubscriptionState   *pSubstate,
                             IN    SipParseType               eType,
                             INOUT const void                *pSipObject)
{
    RvSipSubscriptionStateHeaderHandle hSubsState;
    RvStatus                 rv = RV_OK;
    RvSipSubscriptionSubstate eSubstate;
    RvInt32                  otherOffset = UNDEFINED;
    HRPOOL                    hPool;
    HPAGE                     hPage;

    if (SIP_PARSETYPE_SUBSCRIPTION_STATE == eType)
    {
        /*The parser is used to parse Event header */
        hSubsState=(RvSipSubscriptionStateHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_SUBSCRIPTION_STATE, 
                                    pcbPointer, (void**)&hSubsState);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitSubsState - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipSubscriptionStateHeaderGetPool(hSubsState);
    hPage = SipSubscriptionStateHeaderGetPage(hSubsState);

     /* Set substate-value */
    switch(pSubstate->substateValue.substateVal)
    {
        case PARSER_SUBS_STATE_VAL_ACTIVE:
            eSubstate = RVSIP_SUBSCRIPTION_SUBSTATE_ACTIVE;
            break;
        case PARSER_SUBS_STATE_VAL_PENDING:
            eSubstate = RVSIP_SUBSCRIPTION_SUBSTATE_PENDING;
            break;
        case PARSER_SUBS_STATE_VAL_TERMINATED:
            eSubstate = RVSIP_SUBSCRIPTION_SUBSTATE_TERMINATED;
            break;
        case PARSER_SUBS_STATE_VAL_OTHER:
            eSubstate = RVSIP_SUBSCRIPTION_SUBSTATE_OTHER;
            rv = ParserAllocFromObjPage(pParserMgr,
                                             &otherOffset,
                                             hPool,
                                             hPage,
                                             pSubstate->substateValue.otherSubstateVal.buf,
                                             pSubstate->substateValue.otherSubstateVal.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                        "ParserInitSubsState - error in ParserAllocFromObjPage"));
                return rv;
            }
            break;
        default:
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                        "ParserInitSubsState - unknown substate value"));
            return RV_ERROR_UNKNOWN;
    }
    rv = SipSubscriptionStateHeaderSetSubstate(hSubsState,
                                                 eSubstate,
                                                 NULL,
                                                 hPool,
                                                 hPage,
                                                 otherOffset);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitSubsState - error in SipSubscriptionStateHeaderSetSubstate"));
        return rv;
    }

    /* Set Subscription-State params */
    if(pSubstate->isParams == RV_TRUE)
    {
        /* reason */
        if(pSubstate->substateParams.isReason == RV_TRUE)
        {
            RvSipSubscriptionReason eReason;
            switch(pSubstate->substateParams.eReasonType)
            {
                case PARSER_SUBS_STATE_REASON_DEACTIVATED:
                    eReason = RVSIP_SUBSCRIPTION_REASON_DEACTIVATED;
                    break;
                case PARSER_SUBS_STATE_REASON_PROBATION:
                    eReason = RVSIP_SUBSCRIPTION_REASON_PROBATION;
                    break;
                case PARSER_SUBS_STATE_REASON_REJECTED:
                    eReason = RVSIP_SUBSCRIPTION_REASON_REJECTED;
                    break;
                case PARSER_SUBS_STATE_REASON_TIMEOUT:
                    eReason = RVSIP_SUBSCRIPTION_REASON_TIMEOUT;
                    break;
                case PARSER_SUBS_STATE_REASON_GIVEUP:
                    eReason = RVSIP_SUBSCRIPTION_REASON_GIVEUP;
                    break;
                case PARSER_SUBS_STATE_REASON_NORESOURCE:
                    eReason = RVSIP_SUBSCRIPTION_REASON_NORESOURCE;
                    break;
                case PARSER_SUBS_STATE_REASON_OTHER:
                    eReason = RVSIP_SUBSCRIPTION_REASON_OTHER;
                    rv = ParserAllocFromObjPage(pParserMgr,
                                             &otherOffset,
                                             hPool,
                                             hPage,
                                             pSubstate->substateParams.otherReason.buf,
                                             pSubstate->substateParams.otherReason.len);
                    if (RV_OK!=rv)
                    {
                        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                                "ParserInitSubsState - error in ParserAllocFromObjPage"));
                        return rv;
                    }
                    break;
                default:
                    RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                                "ParserInitSubsState - unknown reason value"));
                    return RV_ERROR_UNKNOWN;
            }
            rv = SipSubscriptionStateHeaderSetReason(hSubsState,
                                                         eReason,
                                                         NULL,
                                                         hPool,
                                                         hPage,
                                                         otherOffset);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                        "ParserInitSubsState - error in SipSubscriptionStateHeaderSetReason"));
                return rv;
            }
        }

        /* retry-after */
        if(pSubstate->substateParams.isRetryAfter == RV_TRUE)
        {
            RvInt32 retryAfter;

            /* Change the token into a number*/
            rv = ParserGetINT32FromString(pParserMgr,
                                             pSubstate->substateParams.retryAfter.buf ,
                                             pSubstate->substateParams.retryAfter.len,
                                             &retryAfter);
            if (RV_OK != rv)
            {
               RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                 "ParserInitSubsState - error in ParserGetINT32FromString"));
                return rv;
            }

            rv = RvSipSubscriptionStateHeaderSetRetryAfter(hSubsState, retryAfter);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                        "ParserInitSubsState - error in RvSipSubscriptionStateHeaderSetRetryAfter"));
                return rv;
            }
        }

        /* expires */
        if(pSubstate->substateParams.isExpires == RV_TRUE)
        {
            RvInt32 expires;

            /* Change the token into a number*/
            rv = ParserGetINT32FromString(pParserMgr,
                                             pSubstate->substateParams.expires.buf ,
                                             pSubstate->substateParams.expires.len,
                                             &expires);
            if (RV_OK != rv)
            {
               RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                 "ParserInitSubsState - error in ParserGetINT32FromString"));
                return rv;
            }

            rv = RvSipSubscriptionStateHeaderSetExpiresParam(hSubsState, expires);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                        "ParserInitSubsState - error in RvSipSubscriptionStateHeaderSetExpiresParam"));
                return rv;
            }
        }

        /* other params */
        if(pSubstate->substateParams.isExtention == RV_TRUE)
        {
            RvInt32 offsetOtherParam = UNDEFINED;
            ParserExtensionString* pExtension = (ParserExtensionString *)pSubstate->substateParams.exten;

            offsetOtherParam = ParserAppendCopyRpoolString(
                                        pParserMgr,
                                        hPool,
                                        hPage,
                                        pExtension->hPool,
                                        pExtension->hPage,
                                        0,
                                        pExtension->size);

            if (UNDEFINED == offsetOtherParam)
            {
                 RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                          "ParserInitSubsState - error in ParserAppendCopyRpoolString"));
                 return RV_ERROR_OUTOFRESOURCES;
            }
            rv = SipSubscriptionStateHeaderSetOtherParams(
                                       hSubsState,
                                       NULL,
                                       hPool,
                                       hPage,
                                       offsetOtherParam);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitSubsState - error in SipSubscriptionStateHeaderSetOtherParams"));
                return rv;
            }
        }
    }

    return  RV_OK;
}
#endif /* #ifdef RV_SIP_SUBS_ON */

#endif /*  RV_SIP_PRIMITIVES */
/***************************************************************************
* ParserInitOtherHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          From header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr - Pointer to the parser manager.
*             pOtherHeaderVal - Other structure holding the Other values from the
*                          parser in a token form.
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone Other or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_OTHER
*                          it will be cast to RvSipOtherHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitOtherHeader(
                               IN    ParserMgr           *pParserMgr,
                               IN    SipParser_pcb_type *pcb,
                               IN    RvUint8            *pcbPointer,
                               IN    ParserOtherHeader   *pOtherHeaderVal,
                               IN    SipParseType        eType,
                               INOUT const void          *pSipObject)
{
    RvStatus   rv;
    HRPOOL      hPool;
    HPAGE       hPage;
    RvSipOtherHeaderHandle hOtherHeader;
    RvInt32               offsetName = UNDEFINED;
    RvInt32               offsetValue = UNDEFINED;

    if (SIP_PARSETYPE_OTHER ==eType)
    {
        /*The parser is used to parse Other header */
        hOtherHeader=(RvSipOtherHeaderHandle)pSipObject;
    }
#ifndef RV_SIP_PRIMITIVES
    else if (SIP_PARSETYPE_MIME_BODY_PART==eType)
    {
        /*The parser is used to parse message*/
        pSipObject=(RvSipBodyHandle)pSipObject;

        /* Construct Other header in the message */
        rv = RvSipOtherHeaderConstructInBodyPart(pSipObject, RV_FALSE, &hOtherHeader);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                "ParserInitOtherHeader - error in RvSipOtherHeaderConstructInBodyPart"));
            return rv;
        }
    }
#endif /* RV_SIP_PRIMITIVES */
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_OTHER, 
                                   pcbPointer, (void**)&hOtherHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitOtherHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipOtherHeaderGetPool(hOtherHeader);
    hPage = SipOtherHeaderGetPage(hOtherHeader);

    /* Change the token name into a string and set it in the Other header*/
    rv= ParserAllocFromObjPage(pParserMgr,
                                  &offsetName,
                                  hPool,
                                  hPage,
                                  pOtherHeaderVal->name.buf,
                                  pOtherHeaderVal->name.len);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitOtherHeader - error in RvSipOtherHeaderConstructInMsg"));
        return rv;
    }
    rv = SipOtherHeaderSetName(hOtherHeader,
                                  NULL,
                                  hPool,
                                  hPage,
                                  offsetName);
    if (RV_OK != rv)
    {
        RvLogError(  pParserMgr->pLogSrc,(  pParserMgr->pLogSrc,
              "ParserInitOtherHeader - error in OtherHeaderSetName"));
        return rv;
    }
    if (pOtherHeaderVal->value.len > 0 )
    {
         /* Change the token value into a string and set it in the Other header*/
         rv= ParserAllocFromObjPage(pParserMgr,
                                       &offsetValue,
                                       hPool,
                                       hPage,
                                       pOtherHeaderVal->value.buf,
                                       pOtherHeaderVal->value.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitOtherHeader - error in ParserAllocFromObjPage"));
            return rv;
        }
        rv = SipOtherHeaderSetValue(hOtherHeader,
                                       NULL,
                                       hPool,
                                       hPage,
                                       offsetValue);
        if (RV_OK != rv)
        {
           RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitOtherHeader - error in OtherHeaderSetValue"));
            return rv;
        }
    }

    if (SIP_PARSETYPE_MSG==eType)
    {
        CleanHeaderBeforeComma((RvChar*)pcbPointer, pcb->pBuffer);
    }
    return RV_OK;
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
* ParserInitOptionTag
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          OptionTagBase header values = Supported, Unsupported, Require,
*          Proxy-Require headers.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr - Pointer to the parser manager.
*             pOpTag     - OptionTag structure holding the option tag values from the
*                          parser in a token form.
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone option tag or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_OTHER
*                          it will be cast to RvSipOtherHandle.
***************************************************************************/
RvStatus ParserInitOptionTag(
                              IN    ParserMgr        *pParserMgr,
                              IN    SipParser_pcb_type *pcb,
                              IN    RvUint8         *pcbPointer,
                              IN    ParserOptionTag  *pOpTag,
                              IN    SipParseType     eType,
                              IN    RvBool           bCompactForm,
                              INOUT const void       *pSipObject)
{
    RvStatus               rv;
    RvSipOtherHeaderHandle hOtherHeader;
    RvInt32                offsetValue = UNDEFINED;
    RvChar			      *pHeaderName = NULL;
    HRPOOL                 hPool;
    HPAGE                  hPage;

    if (eType == SIP_PARSETYPE_OTHER)
    {
        /*The parser is used to parse Other header */
        hOtherHeader=(RvSipOtherHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_OTHER, 
                                    pcbPointer, (void**)&hOtherHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitOptionTag - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipOtherHeaderGetPool(hOtherHeader);
    hPage = SipOtherHeaderGetPage(hOtherHeader);

    switch (pOpTag->headerType)
    {
        case SIP_PARSETYPE_PROXY_REQUIRE:
        {
            pHeaderName = PROXY_REQUIRE_PREFIX;
            break;
        }
        case SIP_PARSETYPE_REQUIRE:
        {
            pHeaderName = REQUIRE_PREFIX;
            break;
        }
        case SIP_PARSETYPE_SUPPORTED:
        {
            if(bCompactForm == RV_FALSE)
            {
                pHeaderName = SUPPORTED_PREFIX;
            }
            else
            {
                pHeaderName = SUPPORTED_COMPACT_PREFIX;
            }
            break;
        }
        case SIP_PARSETYPE_UNSUPPORTED:
        {
            pHeaderName = UNSUPPORTED_PREFIX;
            break;
        }
        default:
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
            "ParserInitOptionTag - header type is wrong"));
            return RV_ERROR_UNKNOWN;
        }
    }
    /* Change the token name into a string and set it in the Other header*/
    rv= RvSipOtherHeaderSetName(hOtherHeader, pHeaderName);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitOptionTag - error in RvSipOtherHeaderSetName"));
        return rv;
    }

    if (pOpTag->optionTag.len > 0 )
    {
         /* Change the token value into a string and set it in the Other header*/
         rv= ParserAllocFromObjPage(pParserMgr,
                                       &offsetValue,
                                       hPool,
                                       hPage,
                                       pOpTag->optionTag.buf,
                                       pOpTag->optionTag.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitOptionTag - error in ParserAllocFromObjPage"));
            return rv;
        }
        rv = SipOtherHeaderSetValue(hOtherHeader,
                                       NULL,
                                       hPool,
                                       hPage,
                                       offsetValue);
        if (RV_OK != rv)
        {
           RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitOptionTag - error in OtherHeaderSetValue"));
            return rv;
        }
    }
    if (SIP_PARSETYPE_MSG==eType)
    {
        CleanHeaderBeforeComma((RvChar*)pcbPointer, pcb->pBuffer);
    }
    return RV_OK;
}
#endif /* RV_SIP_PRIMITIVES */

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
* ParserInitAuthentication
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          Authentication header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr  - Pointer to the parser manager.
*             pcbPointer  - Pointer to the current location of parser pointer
*                           in msg buffer (the end of the given header).
*             pAuthHeaderVal - Authentication structure holding the Authentication
*                          values from the parser in a token form.
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone Authentication or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_AUTHENTICATION
*                          it will be cast to RvSipAuthenticationHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitAuthentication(
                                  IN    ParserMgr                  *pParserMgr,
                                  IN    SipParser_pcb_type        *pcb,
                                  IN    RvUint8                   *pcbPointer,
                                  IN    ParserAuthenticationHeader *pAuthHeaderVal,
                                  IN    SipParseType               eType,
                                  IN    SipParseType               eWhichHeader,
                                  INOUT const void                 *pSipObject)
{
    RvStatus						rv;
    RvSipAuthenticationHeaderHandle hAuthenHeader;
    RvInt32							offsetAuthScheme = UNDEFINED;
    HRPOOL                          hPool;
    HPAGE                           hPage;

    if (SIP_PARSETYPE_WWW_AUTHENTICATE   == eType ||
        SIP_PARSETYPE_PROXY_AUTHENTICATE == eType )
    {
        /* The parser is used to parse Other header */
        hAuthenHeader=(RvSipAuthenticationHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_AUTHENTICATION, 
                                    pcbPointer, (void**)&hAuthenHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitAuthentication - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipAuthenticationHeaderGetPool(hAuthenHeader);
    hPage = SipAuthenticationHeaderGetPage(hAuthenHeader);

    if (RVSIP_AUTH_SCHEME_OTHER ==pAuthHeaderVal->eAuthScheme)
    {

        /* Change the token value into a string and set it in the Other header*/
        rv= ParserAllocFromObjPage(pParserMgr,
                                      &offsetAuthScheme,
                                      hPool,
                                      hPage,
                                      pAuthHeaderVal->authScheme.buf,
                                      pAuthHeaderVal->authScheme.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitAuthentication - error in ParserAllocFromObjPage"));
            return rv;
        }
    }
    rv = SipAuthenticationHeaderSetAuthScheme(
                    hAuthenHeader,
                    pAuthHeaderVal->eAuthScheme,
                    NULL,
                    hPool,
                    hPage,
                    offsetAuthScheme);

    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitAuthentication - error in SipAuthenticationHeaderSetAuthScheme"));
        return rv;
    }

    if(eWhichHeader == SIP_PARSETYPE_WWW_AUTHENTICATE)
    {
        rv = RvSipAuthenticationHeaderSetHeaderType(hAuthenHeader,
                                                       RVSIP_AUTH_WWW_AUTHENTICATION_HEADER);
    }
    else
    {
        rv = RvSipAuthenticationHeaderSetHeaderType(hAuthenHeader,
                                                       RVSIP_AUTH_PROXY_AUTHENTICATION_HEADER);
    }
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitAuthentication - error in RvSipAuthenticationHeaderSetHeaderType"));
        return rv;
    }


    rv= ParserInitChallengeAuthentication(pParserMgr, pAuthHeaderVal, hPool, hPage, hAuthenHeader);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitAuthentication - error in ParserInitChallengeAuthentication"));
        return rv;
    }
    if (SIP_PARSETYPE_MSG==eType)
    {
        CleanHeaderBeforeComma((RvChar*)pcbPointer, pcb->pBuffer);
    }
    return RV_OK;
}

/***************************************************************************
* ParserInitAuthorization
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          Authorization header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr  - Pointer to the parser manager.
*             pcbPointer  - Pointer to the current location of parser pointer
*                           in msg buffer (the end of the given header).
*            pAuthHeaderVal - Authorization structure holding the Authorization
*                          values from the parser in a token form.
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone Authorization or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_AUTHORIZATION
*                          it will be cast to RvSipAuthorizationHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitAuthorization(
                                 IN    ParserMgr                 *pParserMgr,
                                 IN    SipParser_pcb_type       *pcb,
                                 IN    RvUint8                  *pcbPointer,
                                 IN    ParserAuthorizationHeader *pAuthHeaderVal,
                                 IN    SipParseType              eType,
                                 IN    SipParseType              eWhichHeader,
                                 INOUT const void                *pSipObject)
{
    RvStatus                      rv;
    RvSipAuthorizationHeaderHandle hAuthorHeader;
    RvInt32                       offsetAuthScheme = UNDEFINED;
    HRPOOL                         hPool;
    HPAGE                          hPage;

    if (SIP_PARSETYPE_AUTHORIZATION       == eType ||
             SIP_PARSETYPE_PROXY_AUTHORIZATION == eType)
    {
        /* The parser is used to parse Other header */
        hAuthorHeader=(RvSipAuthorizationHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_AUTHORIZATION, 
                                    pcbPointer, (void**)&hAuthorHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitAuthorization - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipAuthorizationHeaderGetPool(hAuthorHeader);
    hPage = SipAuthorizationHeaderGetPage(hAuthorHeader);

    if (RVSIP_AUTH_SCHEME_OTHER ==pAuthHeaderVal->eAuthScheme)
    {

        /* Change the token value into a string and set it in the Other header*/
        rv= ParserAllocFromObjPage(pParserMgr,
                                       &offsetAuthScheme,
                                       hPool,
                                       hPage,
                                      pAuthHeaderVal->authScheme.buf,
                                      pAuthHeaderVal->authScheme.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitAuthorization - error in ParserAllocFromObjPage"));
            return rv;
        }
    }
    rv = SipAuthorizationHeaderSetAuthScheme(
                    hAuthorHeader,
                    pAuthHeaderVal->eAuthScheme,
                    NULL,
                    hPool,
                    hPage,
                    offsetAuthScheme);

    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitAuthorization - error in SipAuthenticationHeaderSetAuthScheme"));
        return rv;
    }
    if(eWhichHeader == SIP_PARSETYPE_AUTHORIZATION)
    {
        rv = RvSipAuthorizationHeaderSetHeaderType(hAuthorHeader,
                                                  RVSIP_AUTH_AUTHORIZATION_HEADER);
    }
    else
    {
        rv = RvSipAuthorizationHeaderSetHeaderType(hAuthorHeader,
                                                  RVSIP_AUTH_PROXY_AUTHORIZATION_HEADER);
    }
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitAuthorization - error in RvSipAuthorizationHeaderSetHeaderType"));
        return rv;
    }
    rv= ParserInitChallengeAuthorization(pParserMgr,pAuthHeaderVal,hPool, hPage, hAuthorHeader);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitAuthorization - error in ParserInitChallengeAuthorization"));
        return rv;
    }

    if (SIP_PARSETYPE_MSG==eType)
    {
        CleanHeaderBeforeComma((RvChar*)pcbPointer, pcb->pBuffer);
    }
    return RV_OK;
}
#endif /* #ifdef RV_SIP_AUTH_ON */


#ifdef RV_SIP_AUTH_ON
/***************************************************************************
* ParserInitAuthenticationInfo
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          Authentication-Info header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:   pParserMgr  - Pointer to the parser manager.
*             pcbPointer  - Pointer to the current location of parser pointer
*                           in msg buffer (the end of the given header).
*             pAuthInfoHeaderVal - Authentication-Info structure holding the
*                          Authentication-Info values from the parser in a token form.
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone Authentication-Info or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_AUTHENTICATION_INFP
*                          it will be cast to RvSipAuthenticationInfoHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitAuthenticationInfo(
                                 IN    ParserMgr                      *pParserMgr,
                                 IN    RvUint8                        *pcbPointer,
                                 IN    ParserAuthenticationInfoHeader *pAuthInfoHeaderVal,
                                 IN    SipParseType                    eType,
                                 INOUT const void                     *pSipObject)
{
    RvStatus                            rv;
    RvSipAuthenticationInfoHeaderHandle hAuthInfoHeader;
    RvInt32                             offset = UNDEFINED;
    HRPOOL                              hPool;
    HPAGE                               hPage;

    if (SIP_PARSETYPE_AUTHENTICATION_INFO == eType)
    {
        /*The parser is used to parse Other header */
        hAuthInfoHeader=(RvSipAuthenticationInfoHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_AUTHENTICATION_INFO, 
                                    pcbPointer, (void**)&hAuthInfoHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitAuthenticationInfo - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipAuthenticationInfoHeaderGetPool(hAuthInfoHeader);
    hPage = SipAuthenticationInfoHeaderGetPage(hAuthInfoHeader);

	if (RV_TRUE == pAuthInfoHeaderVal->isMsgQop)
	{
		offset = UNDEFINED;
		/* Change the token value into a string*/
		if (pAuthInfoHeaderVal->eMsgQop == RVSIP_AUTH_QOP_OTHER)
		{
			rv= ParserAllocFromObjPage(pParserMgr,
                                       &offset,
                                       hPool,
                                       hPage,
									   pAuthInfoHeaderVal->strMsgQop.buf,
                                       pAuthInfoHeaderVal->strMsgQop.len);
			if (RV_OK!=rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					       "ParserInitAuthenticationInfo - error in ParserAllocFromObjPage"));
				return rv;
			}
		}
		rv = SipAuthenticationInfoHeaderSetStrQop(
												hAuthInfoHeader,
												pAuthInfoHeaderVal->eMsgQop,
												NULL,
												hPool,
												hPage,
												offset);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
					   "ParserInitAuthenticationInfo - error in SipAuthenticationInfoHeaderSetStrQop"));
			return rv;
		}
	}
	else
	{
		rv = SipAuthenticationInfoHeaderSetStrQop(
												hAuthInfoHeader,
												RVSIP_AUTH_QOP_UNDEFINED,
												NULL,
												hPool,
												hPage,
												offset);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
					   "ParserInitAuthenticationInfo - error in SipAuthenticationInfoHeaderSetStrQop"));
			return rv;
		}
	}
	
	if (RV_TRUE == pAuthInfoHeaderVal->isNonceCount)
	{
		RvInt32 nonceCount;
		/* Change the token nonce count into a 32 number in the Author header*/
		rv=ParserGetINT32FromStringHex(pParserMgr,
										   pAuthInfoHeaderVal->strNonceCount.buf ,
										   pAuthInfoHeaderVal->strNonceCount.len,
										   &nonceCount);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				       "ParserInitAuthenticationInfo - error in ParserAllocFromObjPage"));
			return rv;
		}
		rv = RvSipAuthenticationInfoHeaderSetNonceCount(
												hAuthInfoHeader,
												nonceCount);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
					   "ParserInitAuthenticationInfo - error in SipAuthenticationInfoHeaderSetNextNonce"));
			return rv;
		}
	}
	
	if (RV_TRUE == pAuthInfoHeaderVal->isNextNonce)
	{
		offset = UNDEFINED;
		/* Change the token value into a string*/
		rv= ParserAllocFromObjPage(pParserMgr,
                                       &offset,
                                       hPool,
                                       hPage,
									   pAuthInfoHeaderVal->strNextNonce.buf,
                                       pAuthInfoHeaderVal->strNextNonce.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				       "ParserInitAuthenticationInfo - error in ParserAllocFromObjPage"));
			return rv;
		}
		rv = SipAuthenticationInfoHeaderSetNextNonce(
												hAuthInfoHeader,
												NULL,
												hPool,
												hPage,
												offset);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
					   "ParserInitAuthenticationInfo - error in SipAuthenticationInfoHeaderSetNextNonce"));
			return rv;
		}
	}
	
	if (RV_TRUE == pAuthInfoHeaderVal->isCNonce)
	{
		offset = UNDEFINED;
		/* Change the token value into a string*/
		rv= ParserAllocFromObjPage(pParserMgr,
                                       &offset,
                                       hPool,
                                       hPage,
									   pAuthInfoHeaderVal->strCNonce.buf,
                                       pAuthInfoHeaderVal->strCNonce.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				       "ParserInitAuthenticationInfo - error in ParserAllocFromObjPage"));
			return rv;
		}
		rv = SipAuthenticationInfoHeaderSetCNonce(
												hAuthInfoHeader,
												NULL,
												hPool,
												hPage,
												offset);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
					   "ParserInitAuthenticationInfo - error in SipAuthenticationInfoHeaderSetCNonce"));
			return rv;
		}
	}
	
	if (RV_TRUE == pAuthInfoHeaderVal->isRspAuth)
	{
		offset = UNDEFINED;
		/* Change the token value into a string*/
		rv= ParserAllocFromObjPage(pParserMgr,
                                       &offset,
                                       hPool,
                                       hPage,
									   pAuthInfoHeaderVal->strRspAuth.buf,
                                       pAuthInfoHeaderVal->strRspAuth.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				       "ParserInitAuthenticationInfo - error in ParserAllocFromObjPage"));
			return rv;
		}
		rv = SipAuthenticationInfoHeaderSetResponseAuth(
												hAuthInfoHeader,
												NULL,
												hPool,
												hPage,
												offset);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
					   "ParserInitAuthenticationInfo - error in SipAuthenticationInfoHeaderSetResponseAuth"));
			return rv;
		}
	}

    return RV_OK;
}
#endif /*#ifdef RV_SIP_AUTH_ON*/


#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
* ParserInitRseq
* ------------------------------------------------------------------------
* General:This function is used from the parser to init the message with the
*         Rack header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr - Pointer to the parser manager.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             pRseq      - Rseq  structure holding the Rseq values from the
*                          parser in a token form.
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone Rseq or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_RSEQ
*                          it will be cast to RvSipRSeqHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitRSeq(
                        IN    ParserMgr       *pParserMgr,
                        IN    RvUint8        *pcbPointer,
                        IN    ParserRSeq      *pRSeq,
                        IN    SipParseType    eType,
                        INOUT const void      *pSipObject)
{
    RvUint32			  responseNum;
    RvSipRSeqHeaderHandle hRSeqHeader;
    RvStatus rv;

    if (SIP_PARSETYPE_RSEQ ==eType)
    {
        /*The parser is used to parse RSeq header */
        hRSeqHeader=(RvSipRSeqHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_RSEQ, 
                                    pcbPointer, (void**)&hRSeqHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitRSeq - Failed to construct header"));
            return rv;
        }
    }

    /* Change the token responseNum into a number */
    rv = ParserGetUINT32FromString(pParserMgr,
                                       pRSeq->buf,
                                       pRSeq->len,
                                       &responseNum);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitRSeq - error in ParserGetINT32FromString"));
        return rv;
    }

    /* Set the response number value into RSeq header */
    rv = RvSipRSeqHeaderSetResponseNum(hRSeqHeader,responseNum);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitRSeq - error in RvSipEseqHeaderSetResponseNum"));
        return rv;
    }

    return  RV_OK;
}

/***************************************************************************
* ParserInitRack
* ------------------------------------------------------------------------
* General:This function is used from the parser to init the message with the
*         CSeq header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr - Pointer to the parser manager.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             pRack      - Rack  structure holding the Rack values from the
*                          parser in a token form.
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone Rack or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_RACK
*                          it will be cast to RvSipRackHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitRack(
                            IN    ParserMgr       *pParserMgr,
                            IN    RvUint8        *pcbPointer,
                            IN    ParserRack      *pRack,
                            IN    SipParseType    eType,
                            INOUT const void      *pSipObject)
{
    RvUint32                responseNum;
    RvInt32                cseqNum     = UNDEFINED;
    RvSipMethodType         eMethod = RVSIP_METHOD_UNDEFINED;
    RvSipRAckHeaderHandle   hRackHeader;
    RvSipCSeqHeaderHandle   hCseqHeader;
    RvStatus               rv;
    RvInt32                offsetMethod = UNDEFINED;
    HRPOOL                  hPool;
    HPAGE                   hPage;

    if (SIP_PARSETYPE_RACK ==eType)
    {
        /*The parser is used to parse CSeq header */
        hRackHeader=(RvSipRAckHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_RACK, 
                                    pcbPointer, (void**)&hRackHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitRack - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipRAckHeaderGetPool(hRackHeader);
    hPage = SipRAckHeaderGetPage(hRackHeader);

    /* Change the token responseNum into a number */
    rv = ParserGetUINT32FromString(pParserMgr,
                                       pRack->responseNum.buf,
                                       pRack->responseNum.len,
                                       &responseNum);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitRack - error in ParserGetINT32FromString"));
        return rv;
    }

    /* Set the responseNum value into Rack header */
    rv = RvSipRAckHeaderSetResponseNum(hRackHeader,responseNum);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitRack - error in RvSipRAckHeaderSetResponseNum"));
        return rv;
    }


    /* Construct Cseq header in the message */
    rv = RvSipCSeqHeaderConstructInRAckHeader(hRackHeader, &hCseqHeader);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitRack - error in RvSipCSeqHeaderConstructInRAckHeader"));
       return rv;
    }

    /* Change the token cseqNum into a number */
	rv = GetCSeqStepFromMsgBuff(
				pParserMgr,pRack->sequenceNumber.buf,pRack->sequenceNumber.len,&cseqNum);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitRack - error in GetCSeqStepFromMsgBuff"));
        return rv;
    }

    /* Set the cseqNum value into cseq header */
    rv = RvSipCSeqHeaderSetStep(hCseqHeader, cseqNum);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitRack - error in RvSipRAckHeaderSetResponseNum"));
        return rv;
    }

    /* Change the token method structure into the enumerate method type or/and
       a string method type*/
    rv = ParserInitMethodType(pParserMgr,
                                    pRack->method,
                                    hPool,
                                    hPage,
                                   &eMethod,
                                   &offsetMethod);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitRack - error in ParserInitMethodType"));
        return rv;
    }

    /* Set the method type enumerate and string values into the Cseq handle */
    rv = SipCSeqHeaderSetMethodType(hCseqHeader,
                                         eMethod,
                                         NULL,
                                         hPool,
                                         hPage,
                                         offsetMethod);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitRack - error in CseqHeaderSetMethodType"));
        return rv;
    }
    return  RV_OK;
}


/***************************************************************************
* ParserInitContentDisposition
* ------------------------------------------------------------------------
* General:This function is used from the parser to init the message with the
*         ContentDisposition header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr  - Pointer to the parser manager.
*             pConDisposition - ConDisposition structure holding the ContentDisposition
*                           values from the parser in a token form.
*             eType       - This type indicates whether the parser is called to
*                           parse stand alone ContentDisposition or sip message.
*  In/Output: pSipObject  - Pointer to the structure in which the values from
*                           the parser will be set.
*                           If eType == SIP_PARSETYPE_MSG it will be cast to
*                           RvSipMsgHandle and if eType == SIP_PARSETYPE_CONTENT_DISPOSITION
*                           it will be cast to RvSipContentDispositionHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitContentDisposition(
                          IN    ParserMgr                 *pParserMgr,
                          IN    RvUint8                  *pcbPointer,
                          IN    ParserContentDisposition  *pConDisposition,
                          IN    SipParseType              eType,
                          INOUT const void                *pSipObject)
{
    RvSipContentDispositionHeaderHandle hConDispHeader;
    RvStatus   rv = RV_OK;
    HRPOOL      hPool;
    HPAGE       hPage;

    if (SIP_PARSETYPE_CONTENT_DISPOSITION == eType)
    {
        /*The parser is used to parse ContentDisposition header */
        hConDispHeader=(RvSipContentDispositionHeaderHandle)pSipObject;
    }
    else if (SIP_PARSETYPE_MIME_BODY_PART==eType)
    {
        /*The parser is used to parse message*/
        pSipObject=(RvSipBodyHandle)pSipObject;

        /* Construct Content Disposition header in the message */
        rv = RvSipContentDispositionHeaderConstructInBodyPart(pSipObject, &hConDispHeader);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContentDisposition - error in RvSipContentDispositionHeaderConstructInBodyPart"));
            return rv;
        }
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_CONTENT_DISPOSITION, 
                                    pcbPointer, (void**)&hConDispHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContentDisposition - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipContentDispositionHeaderGetPool(hConDispHeader);
    hPage = SipContentDispositionHeaderGetPage(hConDispHeader);

    /* Set disposition type */
    switch (pConDisposition->dispositionType.eDispositionType)
    {
        case PARSER_DISPOSITION_TYPE_RENDER:
        {
            rv = RvSipContentDispositionHeaderSetType(hConDispHeader,
                                                           RVSIP_DISPOSITIONTYPE_RENDER,
                                                           NULL);
            break;
        }
        case PARSER_DISPOSITION_TYPE_SESSION:
        {
            rv = RvSipContentDispositionHeaderSetType(hConDispHeader,
                                                           RVSIP_DISPOSITIONTYPE_SESSION,
                                                           NULL);

            break;
        }
        case PARSER_DISPOSITION_TYPE_ICON:
        {
            rv = RvSipContentDispositionHeaderSetType(hConDispHeader,
                                                           RVSIP_DISPOSITIONTYPE_ICON,
                                                           NULL);

            break;
        }
        case PARSER_DISPOSITION_TYPE_ALERT:
        {
            rv = RvSipContentDispositionHeaderSetType(hConDispHeader,
                                                           RVSIP_DISPOSITIONTYPE_ALERT,
                                                           NULL);

            break;
        }
        case PARSER_DISPOSITION_TYPE_SIGNAL:
        {
            rv = RvSipContentDispositionHeaderSetType(hConDispHeader,
                                                           RVSIP_DISPOSITIONTYPE_SIGNAL,
                                                           NULL);

            break;
        }
        case PARSER_DISPOSITION_TYPE_OTHER:
        {
            RvInt32 offsetOtherType = UNDEFINED;
            rv= ParserAllocFromObjPage(pParserMgr,
                                            &offsetOtherType,
                                            hPool,
                                            hPage,
                                            pConDisposition->dispositionType.otherDispositionType.buf,
                                            pConDisposition->dispositionType.otherDispositionType.len);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                 "ParserInitContentDisposition - error in ParserAllocFromObjPage"));
                return rv;
            }

            /* Set the multi address parameter in the Via header */
            rv = SipContentDispositionHeaderSetType(hConDispHeader,
                                                         NULL,
                                                         hPool,
                                                         hPage,
                                                         offsetOtherType);
            break;
        }
        default:
            break;
    }

    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
         "ParserInitContentDisposition - error in SipContentDispositionHeaderSetType"));
        return rv;
    }


    /* Set disposition handling param */
    switch (pConDisposition->dispositionParams.handlingParam.eParam)
    {
        case PARSER_DISPOSITION_PARAM_HANDLING_OPTIONAL:
        {
            rv = RvSipContentDispositionHeaderSetHandling(hConDispHeader,
                                                               RVSIP_DISPOSITIONHANDLING_OPTIONAL,
                                                               NULL);
            break;
        }
        case PARSER_DISPOSITION_PARAM_HANDLING_REQUIRED:
        {
             rv = RvSipContentDispositionHeaderSetHandling(hConDispHeader,
                                                                RVSIP_DISPOSITIONHANDLING_REQUIRED,
                                                                NULL);
             break;
        }
        case PARSER_DISPOSITION_PARAM_HANDLING_UNKNOWN:
        {
            rv = RvSipContentDispositionHeaderSetHandling(hConDispHeader,
                                                               RVSIP_DISPOSITIONHANDLING_UNDEFINED,
                                                               NULL);
            break;
        }
        case PARSER_DISPOSITION_PARAM_HANDLING_OTHER:
        {
            RvInt32 offsetOtherHandling = UNDEFINED;
            rv = ParserAllocFromObjPage(pParserMgr,
                                             &offsetOtherHandling,
                                             hPool,
                                             hPage,
                                             pConDisposition->dispositionParams.handlingParam.otherHandlingParam.buf,
                                             pConDisposition->dispositionParams.handlingParam.otherHandlingParam.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                 "ParserInitContentDisposition - error in ParserAllocFromObjPage"));
                return rv;
            }
            /* Set other handling param */
            rv = SipContentDispositionHeaderSetHandling(hConDispHeader,
                                                             NULL,
                                                             hPool,
                                                             hPage,
                                                             offsetOtherHandling);
            break;
        }
        default:
            break;
    }

    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
         "ParserInitContentDisposition - error in SipContentDispositionHeaderSetHandling"));
        return rv;
    }

    /*Set the generic params*/
    if (pConDisposition->dispositionParams.isGenericParam == RV_TRUE)
    {
        RvInt32 offsetOtherParam = UNDEFINED;
        offsetOtherParam = ParserAppendCopyRpoolString(
                    pParserMgr,
                    hPool,
                    hPage,
                    ((ParserExtensionString *)pConDisposition->dispositionParams.genericParamList)->hPool,
                    ((ParserExtensionString *)pConDisposition->dispositionParams.genericParamList)->hPage,
                    0,
                    ((ParserExtensionString *)pConDisposition->dispositionParams.genericParamList)->size);

        if (UNDEFINED == offsetOtherParam)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitContentDisposition - error in ParserAppendCopyRpoolString"));
             return RV_ERROR_OUTOFRESOURCES;
        }
        rv = SipContentDispositionHeaderSetOtherParams(
                                   hConDispHeader,
                                   NULL,
                                   hPool,
                                   hPage,
                                   offsetOtherParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitContentDisposition - error in SipContentDispositionHeaderSetOtherParams"));
            return rv;
        }
    }
    return  RV_OK;
}
#endif /* RV_SIP_PRIMITIVES */

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
* ParserInitSessionExpiresHeader
* ------------------------------------------------------------------------
* General: This function is used to init a Session Expires header with the
*          Session Expires values from the parser.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
* Input:  pParserMgr  - Pointer to the parser manager.
*         pcbPointer  - Pointer to the current location of parser pointer
*                       in msg buffer (the end of the given header).
*         pSessionExpiresVal - the Session expires parameters from the parser
*         eType       - This type indicates whether the parser is called to
*                       parse stand alone Session-Expires or sip message.
* Output: pSipObject  - Pointer to the structure in which the values from
*                       the parser will be set.
*                       If eType == SIP_PARSETYPE_MSG it will be cast to
*                       RvSipMsgHandle and if eType == SIP_PARSETYPE_SESSION_EXPIRES
*                       it will be cast to RvSipSessionExpiresHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitSessionExpiresHeader(
                                    IN    ParserMgr                   *pParserMgr,
                                    IN    RvUint8                    *pcbPointer,
                                    IN    ParserSessionExpiresHeader  *pSessionExpiresVal,
                                    IN    SipParseType                eType,
                                    INOUT const void                  *pSipObject)
{
    RvSipSessionExpiresHeaderHandle hSessionExpiresHeader;
    RvStatus   rv;
    HRPOOL      hPool;
    HPAGE       hPage;

    /* Check that we get here only if we parse stand alone header */
    if (SIP_PARSETYPE_SESSION_EXPIRES ==eType)
    {
        /*The parser is used to parse expires stand alone header */
        hSessionExpiresHeader=(RvSipSessionExpiresHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_SESSION_EXPIRES, 
                                    pcbPointer, (void**)&hSessionExpiresHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitSessionExpiresHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipSessionExpiresHeaderGetPool(hSessionExpiresHeader);
    hPage = SipSessionExpiresHeaderGetPage(hSessionExpiresHeader);

    return ParserInitSessionExpiresHeaderInHeader(
                            pParserMgr,
                            pSessionExpiresVal,
                            hPool,
                            hPage,
                            (RvSipSessionExpiresHeaderHandle)hSessionExpiresHeader);
}

#endif /* RV_SIP_PRIMITIVES*/

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * ParserInitMinSEHeader
 * ------------------------------------------------------------------------
 * General: This function is used to init a Min SE header with the
 *          Min SE values from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pParserMgr  - Pointer to the parser manager.
 *         pcbPointer  - Pointer to the current location of parser pointer
 *                       in msg buffer (the end of the given header).
 *         pMinSEVal  - the Session expires parameters from the parser
 *         eType      - This type indicates whether the parser is called to
 *                      parse stand alone Min-SE or sip message.
 * Output: pSipObject - Pointer to the structure in which the values from
 *                      the parser will be set.
 *                      If eType == SIP_PARSETYPE_MSG it will be cast to
 *                      RvSipMsgHandle and if eType == SIP_PARSETYPE_MINSE
 *                      it will be cast to RvSipMinSEHeaderHandle.
 ***************************************************************************/
RvStatus RVCALLCONV ParserInitMinSEHeader(
                           IN    ParserMgr          *pParserMgr,
                           IN    RvUint8           *pcbPointer,
                           IN    ParserMinSEHeader  *pMinSEVal,
                           IN    SipParseType       eType,
                           INOUT const void         *pSipObject)
{
    RvSipMinSEHeaderHandle hMinSEHeader;
    RvStatus               rv;
    HRPOOL                  hPool;
    HPAGE                   hPage;

    /* Check that we get here only if we parse stand alone header */
    if (SIP_PARSETYPE_MINSE ==eType)
    {
        /*The parser is used to parse expires stand alone header */
        hMinSEHeader=(RvSipMinSEHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_MINSE, 
                                    pcbPointer, (void**)&hMinSEHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitMinSEHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipMinSEHeaderGetPool(hMinSEHeader);
    hPage = SipMinSEHeaderGetPage(hMinSEHeader);

    return ParserInitMinSEHeaderInHeader(
                            pParserMgr,
                            pMinSEVal,
                            hPool,
                            hPage,
                            (RvSipMinSEHeaderHandle)hMinSEHeader);
}



#endif /* RV_SIP_PRIMITIVES */

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
* ParserInitRetryAfterHeader
* ------------------------------------------------------------------------
* General: This function is used to init a Retry-After header with the
*          values from the parser.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:     pParserMgr   - Pointer to the parser manager.
*              pcbPointer   - Pointer to the current location of parser pointer
*                             in msg buffer (the end of the given header).
*              pRetryAfter  - The Retry-After parameters from the parser.
*              eType        - This type indicates that the parser is called to
*                             parse stand alone Retry-After.
*    In/OutPut: pSipObject   - Pointer to the structure in which the values from
*                             the parser will be set.
*                             If eType == SIP_PARSETYPE_MSG it will be cast to
*                             RvSipMsgHandle and if eType == SIP_PARSETYPE_RETRY_AFTER
*                             it will be cast to RvSipRetryAfterHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitRetryAfterHeader(
                            IN    ParserMgr         *pParserMgr,
                            IN    RvUint8          *pcbPointer,
                            IN    ParserRetryAfter  *pRetryAfter,
                            IN    SipParseType      eType,
                            INOUT const void        *pSipObject)
{
    RvSipRetryAfterHeaderHandle hRetryHeader;
    RvStatus                   rv;
    HRPOOL                      hPool;
    HPAGE                       hPage;

    if (SIP_PARSETYPE_RETRY_AFTER ==eType)
    {
        /*The parser is used to parse retry-after stand alone header */
        hRetryHeader=(RvSipRetryAfterHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_RETRY_AFTER, 
                                    pcbPointer, (void**)&hRetryHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitRetryAfterHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipRetryAfterHeaderGetPool(hRetryHeader);
    hPage = SipRetryAfterHeaderGetPage(hRetryHeader);

    if (pRetryAfter->isSipDate == RV_FALSE)
    {
        RvUint32 deltaSeconds;
        /* Change the token deltaSeconds into a number */
        rv = ParserGetUINT32FromString(pParserMgr,
                                          pRetryAfter->deltaSeconds.buf,
                                          pRetryAfter->deltaSeconds.len,
                                           &deltaSeconds);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitRetryAfterHeader - error in ParserGetUINT32FromString"));
            return rv;
        }

        rv = RvSipRetryAfterHeaderSetDeltaSeconds(hRetryHeader, deltaSeconds);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitRetryAfterHeader - error in RvSipRetryAfterHeaderSetDeltaSeconds"));
            return rv;
        }
    }
    else
    {
        RvSipDateHeaderHandle hDateHeader;

        /* constructing date header in retryAfter and then set the date */
        rv = RvSipDateConstructInRetryAfterHeader(hRetryHeader, &hDateHeader);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitRetryAfterHeader - error in RvSipDateConstructInRetryAfterHeader"));
            return rv;
        }

        rv = ParserInitDateInHeader(pParserMgr,&(pRetryAfter->sipDate), hDateHeader);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitRetryAfterHeader - error in ParserInitDateInHeader"));
            return rv;
        }
    }
    /* set the comment if exists */
    if (pRetryAfter->isComment == RV_TRUE)
    {
        RvInt32 offsetComment = UNDEFINED;
        rv = ParserAllocFromObjPage(pParserMgr,
                                       &offsetComment,
                                       hPool,
                                       hPage,
                                       pRetryAfter->comment.buf,
                                       pRetryAfter->comment.len - 1);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitRetryAfterHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipRetryAfterHeaderSetStrComment(hRetryHeader,
                                                 NULL,
                                                 hPool,
                                                 hPage,
                                                 offsetComment);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitRetryAfterHeader - error in SipRetryAfterHeaderSetStrComment"));
            return rv;
        }

    }
    /* set generic params if exists */
    if (pRetryAfter->retryParam.genericParams != NULL)
    {
        RvInt32 offsetGenericParam = UNDEFINED;
       
        offsetGenericParam = ParserAppendCopyRpoolString(
                    pParserMgr,
                    hPool,
                    hPage,
                    ((ParserExtensionString *)pRetryAfter->retryParam.genericParams)->hPool,
                    ((ParserExtensionString *)pRetryAfter->retryParam.genericParams)->hPage,
                    0,
                    ((ParserExtensionString *)pRetryAfter->retryParam.genericParams)->size);

        if (UNDEFINED == offsetGenericParam)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitRetryAfterHeader - error in ParserAppendCopyRpoolString"));
             return RV_ERROR_OUTOFRESOURCES;
        }
        rv = SipRetryAfterHeaderSetRetryAfterParams(hRetryHeader,
                                                       NULL,
                                                       hPool,
                                                       hPage,
                                                       offsetGenericParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitRetryAfterHeader - error in SipRetryAfterHeaderSetRetryAfterParams"));
            return rv;
        }
    }
    /* set duration if exists */
    if (pRetryAfter->retryParam.isDeltaSeconds == RV_TRUE)
    {
        RvUint32 deltaSeconds;
        /* Change the token deltaSeconds into a number */
        rv = ParserGetUINT32FromString(pParserMgr,
                                          pRetryAfter->retryParam.deltaSeconds.buf,
                                          pRetryAfter->retryParam.deltaSeconds.len,
                                          &deltaSeconds);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitRetryAfterHeader - error in ParserGetUINT32FromString"));
            return rv;
        }
        rv = RvSipRetryAfterHeaderSetDuration(hRetryHeader, deltaSeconds);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitRetryAfterHeader - error in RvSipRetryAfterHeaderSetDuration"));
            return rv;
        }
    }
    return RV_OK;
}
#endif /* RV_SIP_PRIMITIVES*/


#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
* ParserInitReplaces
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          replaces header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr  - Pointer to the parser manager.
*             pcbPointer  - Pointer to the current location of parser pointer
*                           in msg buffer (the end of the given header).
*             pCallIdVal - Call Id structure holding the Call Id values
*                          from the parser in a token form.
*             pReplacesParams - replaces header's parameters values, in a
*                           ParserReplacesParams structure.
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone Replaces or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_REPLACES
*                          it will be cast to RvSipReplacesHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitReplaces(
                        IN    ParserMgr         *pParserMgr,
                        IN    RvUint8          *pcbPointer,
                        IN    ParserCallId      *pCallIdVal,
                        IN    ParserReplacesParams *pReplacesParams,
                        IN    SipParseType      eType,
                        INOUT const void        *pSipObject)
{
    RvStatus   rv;
    RvSipReplacesHeaderHandle hReplaces;
    RvInt32    offsetCallId;
    HRPOOL      hPool;
    HPAGE       hPage;

    if (SIP_PARSETYPE_REPLACES == eType)
    {
        /*The parser is used to parse replaces stand alone header */
        hReplaces=(RvSipReplacesHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_REPLACES, 
                                    pcbPointer, (void**)&hReplaces);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitReplaces - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipReplacesHeaderGetPool(hReplaces);
    hPage = SipReplacesHeaderGetPage(hReplaces);

    /* --------------------------
        Call-Id
       -------------------------*/

    rv = ParserAllocFromObjPage(pParserMgr,
                                    &offsetCallId,
                                    hPool,
                                    hPage,
                                    pCallIdVal->callIdVal.buf,
                                    pCallIdVal->callIdVal.len);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitReplaces - error in ParserAllocFromObjPage"));
        return rv;
    }

    rv = SipReplacesHeaderSetCallID(hReplaces,
                                        NULL,
                                        hPool,
                                        hPage,
                                        offsetCallId);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitReplaces - error in SipReplacesHeaderSetCallID"));
        return rv;
    }

    /* ---------------------------
        Replaces params
       --------------------------- */
    if(pReplacesParams == NULL)
    {
        return RV_OK;
    }

    /* Set to-tag  */
    if(pReplacesParams->isToTag == RV_TRUE)
    {
        RvInt32 toTagOffset;

        rv = ParserAllocFromObjPage(pParserMgr,
                                        &toTagOffset,
                                        hPool,
                                        hPage,
                                        pReplacesParams->toTag.buf,
                                        pReplacesParams->toTag.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitReplaces - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipReplacesHeaderSetToTag(hReplaces,
                                            NULL,
                                            hPool,
                                            hPage,
                                            toTagOffset);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitReplaces - error in SipReplacesHeaderSetToTag"));
            return rv;
        }
    }

    /* Set from-tag  */
    if(pReplacesParams->isFromTag == RV_TRUE)
    {
        RvInt32 fromTagOffset;

        rv = ParserAllocFromObjPage(pParserMgr,
                                        &fromTagOffset,
                                        hPool,
                                        hPage,
                                        pReplacesParams->fromTag.buf,
                                        pReplacesParams->fromTag.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitReplaces - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipReplacesHeaderSetFromTag(hReplaces,
                                            NULL,
                                            hPool,
                                            hPage,
                                            fromTagOffset);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitReplaces - error in SipReplacesHeaderSetToTag"));
            return rv;
        }
    }
    /* Set other params  */
    if(pReplacesParams->isOtherParams == RV_TRUE)
    {
        RvInt32 otherOffset;
        ParserExtensionString * pOtherParam  =
            (ParserExtensionString *)pReplacesParams->otherParam;

        otherOffset = ParserAppendCopyRpoolString(pParserMgr,
                                    hPool,
                                    hPage,
                                    pOtherParam->hPool,
                                    pOtherParam->hPage,
                                    0,
                                    pOtherParam->size);

        if (UNDEFINED == otherOffset)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitReplaces - error in ParserAppendCopyRpoolString"));
            return RV_ERROR_OUTOFRESOURCES;
        }

        rv = SipReplacesHeaderSetOtherParams(hReplaces,
                                                NULL,
                                                hPool,
                                                hPage,
                                                otherOffset);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitReplaces - error in SipReplacesHeaderSetOtherParams"));
            return rv;
        }
    }
    /* Set early-flag  */
    if(pReplacesParams->eEarlyFlagType != ParserReplacesEarlyFlagParamUndefined)
    {
        RvSipReplacesEarlyFlagType eEarlyFlag;
        switch(pReplacesParams->eEarlyFlagType)
        {
        case ParserReplacesEarlyFlagParam1:
            eEarlyFlag = RVSIP_REPLACES_EARLY_FLAG_TYPE_EARLY_ONLY_1;
            break;
        case ParserReplacesEarlyFlagParamEmpty:
            eEarlyFlag = RVSIP_REPLACES_EARLY_FLAG_TYPE_EARLY_ONLY_EMPTY;
            break;
        case ParserReplacesEarlyFlagParamTrue:
            eEarlyFlag = RVSIP_REPLACES_EARLY_FLAG_TYPE_EARLY_ONLY_TRUE;
            break;
        default:
            eEarlyFlag = RVSIP_REPLACES_EARLY_FLAG_TYPE_UNDEFINED;
        }
        rv = RvSipReplacesHeaderSetEarlyFlagParam(hReplaces,eEarlyFlag);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitReplaces - error in RvSipReplacesHeaderSetEarlyFlagParam"));
            return rv;
        }
    }
    return RV_OK;
}

#endif /* RV_SIP_PRIMITIVES */


#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT

/***************************************************************************
* ParserInitReasonHeader
* ------------------------------------------------------------------------
* General: This function is used to init a Reason header with the
*          values from the parser.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr   - Pointer to the parser manager.
*              pcbPointer   - Pointer to the current location of parser pointer
*                             in msg buffer (the end of the given header).
*              pReason      - The Reason parameters from the parser.
*              eType        - This type indicates that the parser is called to
*                             parse stand alone Retry-After.
*    In/OutPut: pSipObject   - Pointer to the structure in which the values from
*                             the parser will be set.
*                             If eType == SIP_PARSETYPE_MSG it will be cast to
*                             RvSipMsgHandle and if eType == SIP_PARSETYPE_REASON
*                             it will be cast to RvSipReasonHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitReasonHeader(
                            IN    ParserMgr          *pParserMgr,
                            IN    RvUint8            *pcbPointer,
                            IN    ParserReasonHeader *pReason,
                            IN    SipParseType        eType,
                            INOUT const void         *pSipObject)
{
    RvSipReasonHeaderHandle  hReasonHeader;
    RvStatus                 rv;
    HRPOOL                   hPool;
    HPAGE                    hPage;

    if (SIP_PARSETYPE_REASON ==eType)
    {
        /*The parser is used to parse retry-after stand alone header */
        hReasonHeader=(RvSipReasonHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_REASON, 
                                    pcbPointer, (void**)&hReasonHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitReasonHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipReasonHeaderGetPool(hReasonHeader);
    hPage = SipReasonHeaderGetPage(hReasonHeader);

	if (RV_TRUE == pReason->isProtocol)
	{
		RvInt32 offset = UNDEFINED;
		/* Change the token value into a string*/
		if (pReason->eProtocol == RVSIP_REASON_PROTOCOL_OTHER)
		{
			rv= ParserAllocFromObjPage(pParserMgr,
										  &offset,
										  hPool,
										  hPage,
				                          pReason->strProtocol.buf,
										  pReason->strProtocol.len);
			if (RV_OK!=rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitReasonHeader - error in ParserAllocFromObjPage"));
				return rv;
			}
		}
		rv = SipReasonHeaderSetProtocol(hReasonHeader,
										   pReason->eProtocol,
										   NULL,
										   hPool,
										   hPage,
										   offset);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				"ParserInitReasonHeader - error in SipReasonHeaderSetProtocol"));
			return rv;
		}
	}

	/* cause parameter */
    if (RV_TRUE == pReason->params.isCause)
    {
        RvInt32 cause;
        /* Change the token deltaSeconds into a number */
        rv = ParserGetINT32FromString(pParserMgr,
                                         pReason->params.strCause.buf,
                                         pReason->params.strCause.len,
                                         &cause);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					   "ParserInitReasonHeader - error in ParserGetINT32FromString"));
            return rv;
        }

        rv = RvSipReasonHeaderSetCause(hReasonHeader, cause);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					  "ParserInitReasonHeader - error in RvSipReasonSetCause"));
            return rv;
        }
    }

    /* text parameter */
    if (RV_TRUE == pReason->params.isText)
    {
        RvInt32 offset = UNDEFINED;
        /* Change the token value into a string*/
		rv = ParserAllocFromObjPage(pParserMgr,
                                       &offset,
                                       hPool,
                                       hPage,
                                       pReason->params.strText.buf,
                                       pReason->params.strText.len);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitReasonHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipReasonHeaderSetText(hReasonHeader,
                                       NULL,
                                       hPool,
                                       hPage,
                                       offset);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitReasonHeader - error in SipReasonHeaderSetText"));
            return rv;
        }

    }
    /* set other params if exists */
    if (NULL != pReason->params.strOtherParam)
    {
        RvInt32 offsetOtherParam = UNDEFINED;
        offsetOtherParam = ParserAppendCopyRpoolString(
					pParserMgr,
					hPool,
					hPage,
                    ((ParserExtensionString *)pReason->params.strOtherParam)->hPool,
                    ((ParserExtensionString *)pReason->params.strOtherParam)->hPage,
                    0,
                    ((ParserExtensionString *)pReason->params.strOtherParam)->size);

        if (UNDEFINED == offsetOtherParam)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitReasonHeader - error in ParserAppendCopyRpoolString"));
             return RV_ERROR_OUTOFRESOURCES;
        }
        rv = SipReasonHeaderSetOtherParams(hReasonHeader,
                                              NULL,
                                              hPool,
                                              hPage,
                                              offsetOtherParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitReasonHeader - error in SipReasonHeaderSetOtherParams"));
            return rv;
        }
    }

    return RV_OK;
}

/***************************************************************************
* ParserInitWarningHeader
* ------------------------------------------------------------------------
* General: This function is used to init a Warning header with the
*          values from the parser.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr   - Pointer to the parser manager.
*              pcbPointer   - Pointer to the current location of parser pointer
*                             in msg buffer (the end of the given header).
*              pWarning     - The Warning parameters from the parser.
*              eType        - This type indicates that the parser is called to
*                             parse stand alone Warning.
*    In/OutPut: pSipObject   - Pointer to the structure in which the values from
*                             the parser will be set.
*                             If eType == SIP_PARSETYPE_MSG it will be cast to
*                             RvSipMsgHandle and if eType == SIP_PARSETYPE_WARNING
*                             it will be cast to RvSipWarningHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitWarningHeader(
                            IN    ParserMgr           *pParserMgr,
                            IN    RvUint8             *pcbPointer,
                            IN    ParserWarningHeader *pWarning,
                            IN    SipParseType         eType,
                            INOUT const void          *pSipObject)
{
    RvSipWarningHeaderHandle  hWarningHeader;
    RvStatus                   rv;
    HRPOOL                     hPool;
    HPAGE                      hPage;
	RvInt32                    offset = UNDEFINED;
	
    if (SIP_PARSETYPE_WARNING ==eType)
    {
        /*The parser is used to parse retry-after stand alone header */
        hWarningHeader=(RvSipWarningHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_WARNING, 
                                    pcbPointer, (void**)&hWarningHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitWarningHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipWarningHeaderGetPool(hWarningHeader);
    hPage = SipWarningHeaderGetPage(hWarningHeader);

	/* warn-code */
    /* Change the token into a number */
    rv = RvSipWarningHeaderSetWarnCode(hWarningHeader, pWarning->warnCode);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitWarningHeader - error in RvSipWarningHeaderSetWarnCode"));
        return rv;
    }


	/* warn-agent */
	/* Change the token value into a string*/
	rv= ParserAllocFromObjPage(pParserMgr,
							&offset,
							hPool,
							hPage,
							pWarning->strWarnAgent.buf,
							pWarning->strWarnAgent.len);
	if (RV_OK!=rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitWarningHeader - error in ParserAllocFromObjPage"));
		return rv;
	}
	rv = SipWarningHeaderSetWarnAgent(hWarningHeader,
									NULL,
									hPool,
									hPage,
									offset);
	if (RV_OK != rv)
	{
		RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				"ParserInitWarningHeader - error in SipWarningHeaderSetWarnAgent"));
		return rv;
	}

	/* warn-text */
	/* Change the token value into a string*/
	rv= ParserAllocFromObjPage(pParserMgr,
									&offset,
									hPool,
									hPage,
									pWarning->strWarnText.buf,
									pWarning->strWarnText.len);
	if (RV_OK!=rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			"ParserInitWarningHeader - error in ParserAllocFromObjPage"));
		return rv;
	}
	rv = SipWarningHeaderSetWarnText(hWarningHeader,
										NULL,
										hPool,
										hPage,
										offset);
	if (RV_OK != rv)
	{
		RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				"ParserInitWarningHeader - error in SipWarningHeaderSetWarnText"));
		return rv;
	}

    return RV_OK;
}

/***************************************************************************
* ParserInitTimestampHeader
* ------------------------------------------------------------------------
* General: This function is used to init a Timestamp header with the
*          values from the parser.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr   - Pointer to the parser manager.
*              pcbPointer   - Pointer to the current location of parser pointer
*                             in msg buffer (the end of the given header).
*              pTimestamp   - The Timestamp parameters from the parser.
*              eType        - This type indicates that the parser is called to
*                             parse stand alone Timestamp.
*    In/OutPut: pSipObject   - Pointer to the structure in which the values from
*                             the parser will be set.
*                             If eType == SIP_PARSETYPE_MSG it will be cast to
*                             RvSipMsgHandle and if eType == SIP_PARSETYPE_TIMESTAMP
*                             it will be cast to RvSipTimestampHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitTimestampHeader(
                            IN    ParserMgr             *pParserMgr,
                            IN    RvUint8               *pcbPointer,
                            IN    ParserTimestampHeader *pTimestamp,
                            IN    SipParseType           eType,
                            INOUT const void            *pSipObject)
{
    RvSipTimestampHeaderHandle  hTimestampHeader;
    RvStatus                    rv;
    HRPOOL                      hPool;
    HPAGE                       hPage;
	RvInt32                     timestampInt = UNDEFINED;
	RvInt32                     timestampDec = UNDEFINED;
	RvInt32                     delayInt     = UNDEFINED;
	RvInt32                     delayDec     = UNDEFINED;
	
    if (SIP_PARSETYPE_TIMESTAMP ==eType)
    {
        /*The parser is used to parse retry-after stand alone header */
        hTimestampHeader=(RvSipTimestampHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_TIMESTAMP, 
                                    pcbPointer, (void**)&hTimestampHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitTimestampHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipTimestampHeaderGetPool(hTimestampHeader);
    hPage = SipTimestampHeaderGetPage(hTimestampHeader);

	/* timestamp val */
	if (pTimestamp->bIsTimestampInt == RV_TRUE)
	{
		/* get timestamp integer part */
		/* Change the token into a number */
		rv = ParserGetINT32FromString(pParserMgr,
										pTimestamp->strTimestampInt.buf,
										pTimestamp->strTimestampInt.len,
										&timestampInt);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitTimestampHeader - error in ParserGetINT32FromString"));
			return rv;
		}

		if (pTimestamp->bIsTimestampDec == RV_TRUE)
		{
			/* get timestamp decimal part */
			/* Change the token into a number */
			rv = ParserGetINT32FromString(pParserMgr,
											pTimestamp->strTimestampDec.buf,
											pTimestamp->strTimestampDec.len,
											&timestampDec);
			if (RV_OK!=rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
							"ParserInitTimestampHeader - error in ParserGetINT32FromString"));
				return rv;
			}
		}
		
		rv = RvSipTimestampHeaderSetTimestampVal(hTimestampHeader,
													timestampInt, 
													timestampDec);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
						"ParserInitTimestampHeader - error in RvSipTimestampHeaderSetTimestampVal"));
			return rv;
		}
	}
	else
	{
		RvLogExcep(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				   "ParserInitTimestampHeader - Missing mandatory timestamp integer value"));
		return RV_ERROR_ILLEGAL_SYNTAX;
	}

	/* delay val */
	if (pTimestamp->bIsDelayInt == RV_TRUE)
	{
		/* get delay integer part */
		/* Change the token into a number */
		rv = ParserGetINT32FromString(pParserMgr,
										pTimestamp->strDelayInt.buf,
										pTimestamp->strDelayInt.len,
										&delayInt);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitTimestampHeader - error in ParserGetINT32FromString"));
			return rv;
		}
		
		if (pTimestamp->bIsDelayDec == RV_TRUE)
		{
			/* get delay decimal part */
			/* Change the token into a number */
			rv = ParserGetINT32FromString(pParserMgr,
											pTimestamp->strDelayDec.buf,
											pTimestamp->strDelayDec.len,
											&delayDec);
			if (RV_OK!=rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitTimestampHeader - error in ParserGetINT32FromString"));
				return rv;
			}
		}
		
		rv = RvSipTimestampHeaderSetDelayVal(hTimestampHeader,
												delayInt, 
												delayDec);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
						"ParserInitTimestampHeader - error in RvSipTimestampHeaderSetDelayVal"));
			return rv;
		}
	}

    return RV_OK;
}

/***************************************************************************
* ParserInitInfoHeader
* ------------------------------------------------------------------------
* General: This function is used to init an Info header with the
*          values from the parser.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr   - Pointer to the parser manager.
*              pcbPointer   - Pointer to the current location of parser pointer
*                             in msg buffer (the end of the given header).
*              pInfo        - The Info parameters from the parser.
*              eType        - This type indicates that the parser is called to
*                             parse stand alone Info.
*              eSpecificHeaderType - The specific header type: alert-info, call-info
*                                     or error-info
*    In/OutPut: pSipObject   - Pointer to the structure in which the values from
*                             the parser will be set.
*                             If eType == SIP_PARSETYPE_MSG it will be cast to
*                             RvSipMsgHandle and if eType == SIP_PARSETYPE_TIMESTAMP
*                             it will be cast to RvSipInfoHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitInfoHeader(
                            IN    ParserMgr             *pParserMgr,
                            IN    RvUint8               *pcbPointer,
                            IN    ParserInfoHeader      *pInfo,
                            IN    SipParseType           eType,
							IN    SipParseType           eSpecificHeaderType,
                            INOUT const void            *pSipObject)
{
	RvStatus              rv;
    RvSipInfoHeaderHandle hInfoHeader;
	RvSipInfoHeaderType   eInfoType;
    HRPOOL hPool;
    HPAGE  hPage;
    
    if (SIP_PARSETYPE_ALERT_INFO ==eType || SIP_PARSETYPE_CALL_INFO == eType || SIP_PARSETYPE_ERROR_INFO == eType)
    {
        /*The parser is used to parse Info header */
        hInfoHeader=(RvSipInfoHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, 
									RVSIP_HEADERTYPE_INFO, 
									pcbPointer, (void**)&hInfoHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					  "ParserInitInfoHeader - Failed to construct header"));
            return rv;
        }
    }

	if (eSpecificHeaderType == SIP_PARSETYPE_ALERT_INFO)
	{
		eInfoType = RVSIP_INFO_ALERT_INFO_HEADER;
	}
	else if (eSpecificHeaderType == SIP_PARSETYPE_CALL_INFO)
	{
		eInfoType = RVSIP_INFO_CALL_INFO_HEADER;
	}
	else
	{
		eInfoType = RVSIP_INFO_ERROR_INFO_HEADER;
	}

	rv = RvSipInfoHeaderSetHeaderType(hInfoHeader, eInfoType);
	if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitInfoHeader - error in RvSipInfoHeaderSetHeaderType"));
        return rv;
    }

    hPool = SipInfoHeaderGetPool(hInfoHeader);
    hPage = SipInfoHeaderGetPage(hInfoHeader);
	
    /* Init the party header in the message*/
    rv = ParserInitializeInfoFromPcb(pParserMgr,pInfo,hPool, hPage, &hInfoHeader);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitInfoHeader - error in ParserInitializeInfoFromPcb"));
        return rv;
    }
	
    return RV_OK;
}

/***************************************************************************
* ParserInitAcceptHeader
* ------------------------------------------------------------------------
* General: This function is used to init a Accept header with the
*          values from the parser.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr   - Pointer to the parser manager.
*              pcbPointer   - Pointer to the current location of parser pointer
*                             in msg buffer (the end of the given header).
*              pAccept      - The Accept parameters from the parser.
*              eType        - This type indicates that the parser is called to
*                             parse stand alone Accept.
*    In/OutPut: pSipObject   - Pointer to the structure in which the values from
*                             the parser will be set.
*                             If eType == SIP_PARSETYPE_MSG it will be cast to
*                             RvSipMsgHandle and if eType == SIP_PARSETYPE_WARNING
*                             it will be cast to RvSipAcceptHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitAcceptHeader(
                            IN    ParserMgr           *pParserMgr,
                            IN    RvUint8             *pcbPointer,
                            IN    ParserAcceptHeader  *pAccept,
                            IN    SipParseType         eType,
                            INOUT const void          *pSipObject)
{
    RvSipAcceptHeaderHandle    hAcceptHeader;
    RvInt32                    mediaTypeOffset = UNDEFINED;
    RvInt32                    subMediaTypeOffset = UNDEFINED;
    RvStatus                   rv;
    HRPOOL                     hPool;
    HPAGE                      hPage;
	
    if (SIP_PARSETYPE_ACCEPT ==eType)
    {
        /*The parser is used to parse Accept stand alone header */
        hAcceptHeader=(RvSipAcceptHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_ACCEPT, 
                                    pcbPointer, (void**)&hAcceptHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitAcceptHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipAcceptHeaderGetPool(hAcceptHeader);
    hPage = SipAcceptHeaderGetPage(hAcceptHeader);

	if (pAccept->mediaType.type == RVSIP_MEDIATYPE_OTHER)
    {
        /* Allocate the media type token in a page */
        rv = ParserAllocFromObjPage(pParserMgr,
                                     &mediaTypeOffset,
                                     hPool,
                                     hPage,
                                     pAccept->mediaType.other.buf,
                                     pAccept->mediaType.other.len);
        if (RV_OK != rv)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitAcceptHeader - error in ParserAllocFromObjPage"));
             return rv;
         }

        /* Set the the media type into the header */
        rv =  SipAcceptHeaderSetMediaType(
                                    hAcceptHeader,
                                    NULL,
                                    hPool,
                                    hPage,
                                    mediaTypeOffset);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitAcceptHeader - error in SipAcceptHeaderSetMediaType"));
            return rv;
        }
    }
    else
    {
        rv = RvSipAcceptHeaderSetMediaType(
                                    hAcceptHeader,
                                    pAccept->mediaType.type,
                                    NULL);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitAcceptHeader - error in RvSipAcceptHeaderSetMediaType"));
            return rv;
        }
    }
    if (pAccept->mediaSubType.type == RVSIP_MEDIASUBTYPE_OTHER)
    {    /* Allocate the sub media type token in a page */
        rv = ParserAllocFromObjPage(pParserMgr,
                                     &subMediaTypeOffset,
                                     hPool,
                                     hPage,
                                     pAccept->mediaSubType.other.buf,
                                     pAccept->mediaSubType.other.len);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitAcceptHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        /* Set the the sub media type into the header */
         rv =  SipAcceptHeaderSetMediaSubType(
                                    hAcceptHeader,
                                    NULL,
                                    hPool,
                                    hPage,
                                    subMediaTypeOffset);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitAcceptHeader - error in SipAcceptHeaderSetMediaSubType"));
            return rv;
        }
    }
    else
    {
        rv = RvSipAcceptHeaderSetMediaSubType(
                                    hAcceptHeader,
                                    pAccept->mediaSubType.type,
                                    NULL);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitAcceptHeader - error in RvSipAcceptHeaderSetMediaSubType"));
            return rv;
        }
    }

	/* Check whether q parameter has been set, if so attach it to the header*/
	if (RV_TRUE == pAccept->isQVal)
    {
        RvInt32 offsetQval = UNDEFINED;
        rv = ParserAllocFromObjPage(pParserMgr,
                                       &offsetQval,
                                       hPool,
                                       hPage,
                                       pAccept->qVal.buf,
                                       pAccept->qVal.len);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitAcceptHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipAcceptHeaderSetQVal(hAcceptHeader,
                                        NULL,
                                        hPool,
                                        hPage,
                                        offsetQval);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitAcceptHeader - error in SipAcceptHeaderSetQVal"));
            return rv;
        }
    }

	/* Check whether there are other params, if so, concatenate it to the header*/
    if (RV_TRUE == pAccept->isOtherParams)
    {
        RvInt32 offsetOtherParam = UNDEFINED;
        offsetOtherParam = ParserAppendCopyRpoolString(
										pParserMgr,
										hPool,
										hPage,
										((ParserExtensionString *)pAccept->otherParams)->hPool,
										((ParserExtensionString *)pAccept->otherParams)->hPage,
										0,
										((ParserExtensionString *)pAccept->otherParams)->size);

        if (UNDEFINED == offsetOtherParam)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitAcceptHeader - error in ParserAppendCopyRpoolString"));
             return RV_ERROR_OUTOFRESOURCES;
        }
        rv = SipAcceptHeaderSetOtherParams(hAcceptHeader,
                                         NULL,
                                         hPool,
                                         hPage,
                                         offsetOtherParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitAcceptHeader - error in SipAcceptHeaderSetOtherParams"));
            return rv;
        }
    }


    return RV_OK;
}

/***************************************************************************
* ParserInitAcceptEncodingHeader
* ------------------------------------------------------------------------
* General: This function is used to init a Accept-Encoding header with the
*          values from the parser.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr   - Pointer to the parser manager.
*              pcbPointer   - Pointer to the current location of parser pointer
*                             in msg buffer (the end of the given header).
*              pAcceptEncoding - The Accept-Encoding parameters from the parser.
*              eType        - This type indicates that the parser is called to
*                             parse stand alone Accept-Encoding.
*    In/OutPut: pSipObject   - Pointer to the structure in which the values from
*                             the parser will be set.
*                             If eType == SIP_PARSETYPE_MSG it will be cast to
*                             RvSipMsgHandle and if eType == SIP_PARSETYPE_WARNING
*                             it will be cast to RvSipAcceptEncodingHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitAcceptEncodingHeader(
                            IN    ParserMgr                   *pParserMgr,
                            IN    RvUint8                     *pcbPointer,
                            IN    ParserAcceptEncodingHeader  *pAcceptEncoding,
                            IN    SipParseType                 eType,
                            INOUT const void                  *pSipObject)
{
    RvSipAcceptEncodingHeaderHandle    hAcceptEncodingHeader;
    RvInt32                            offsetCoding = UNDEFINED;
    RvStatus						   rv;
    HRPOOL							   hPool;
    HPAGE							   hPage;
	
    if (SIP_PARSETYPE_ACCEPT_ENCODING ==eType)
    {
        /*The parser is used to parse Accept stand alone header */
        hAcceptEncodingHeader=(RvSipAcceptEncodingHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_ACCEPT_ENCODING, 
                                    pcbPointer, (void**)&hAcceptEncodingHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitAcceptEncodingHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipAcceptEncodingHeaderGetPool(hAcceptEncodingHeader);
    hPage = SipAcceptEncodingHeaderGetPage(hAcceptEncodingHeader);

	/* coding */
	rv = ParserAllocFromObjPage(pParserMgr,
								&offsetCoding,
								hPool,
								hPage,
								pAcceptEncoding->coding.buf,
								pAcceptEncoding->coding.len);
	
	if (RV_OK != rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				  "ParserInitAcceptEncodingHeader - error in ParserAllocFromObjPage"));
		return rv;
	}
	
	rv = SipAcceptEncodingHeaderSetCoding(hAcceptEncodingHeader,
										  NULL,
										  hPool,
									      hPage,
										  offsetCoding);
	if (RV_OK != rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitAcceptEncodingHeader - error in SipAcceptEncodingHeaderSetCoding"));
		return rv;
	}

	/* Check whether q parameter has been set, if so attach it to the header*/
	if (RV_TRUE == pAcceptEncoding->isQVal)
    {
        RvInt32 offsetQval = UNDEFINED;
        rv = ParserAllocFromObjPage(pParserMgr,
                                       &offsetQval,
                                       hPool,
                                       hPage,
                                       pAcceptEncoding->qVal.buf,
                                       pAcceptEncoding->qVal.len);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitAcceptEncodingHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipAcceptEncodingHeaderSetQVal(hAcceptEncodingHeader,
                                        NULL,
                                        hPool,
                                        hPage,
                                        offsetQval);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitAcceptEncodingHeader - error in SipAcceptEncodingHeaderSetQVal"));
            return rv;
        }
    }

	/* Check whether there are other params, if so, concatenate it to the header*/
    if (RV_TRUE == pAcceptEncoding->isOtherParams)
    {
        RvInt32 offsetOtherParam = UNDEFINED;
        offsetOtherParam = ParserAppendCopyRpoolString(
										pParserMgr,
										hPool,
										hPage,
										((ParserExtensionString *)pAcceptEncoding->otherParams)->hPool,
										((ParserExtensionString *)pAcceptEncoding->otherParams)->hPage,
										0,
										((ParserExtensionString *)pAcceptEncoding->otherParams)->size);

        if (UNDEFINED == offsetOtherParam)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitAcceptEncodingHeader - error in ParserAppendCopyRpoolString"));
             return RV_ERROR_OUTOFRESOURCES;
        }
        rv = SipAcceptEncodingHeaderSetOtherParams(hAcceptEncodingHeader,
                                         NULL,
                                         hPool,
                                         hPage,
                                         offsetOtherParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitAcceptEncodingHeader - error in SipAcceptEncodingHeaderSetOtherParams"));
            return rv;
        }
    }


    return RV_OK;
}

/***************************************************************************
* ParserInitAcceptLanguageHeader
* ------------------------------------------------------------------------
* General: This function is used to init a Accept-Language header with the
*          values from the parser.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr   - Pointer to the parser manager.
*              pcbPointer   - Pointer to the current location of parser pointer
*                             in msg buffer (the end of the given header).
*              pAcceptLanguage - The Accept-Language parameters from the parser.
*              eType        - This type indicates that the parser is called to
*                             parse stand alone Accept-Language.
*    In/OutPut: pSipObject   - Pointer to the structure in which the values from
*                             the parser will be set.
*                             If eType == SIP_PARSETYPE_MSG it will be cast to
*                             RvSipMsgHandle and if eType == SIP_PARSETYPE_WARNING
*                             it will be cast to RvSipAcceptLanguageHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitAcceptLanguageHeader(
                            IN    ParserMgr                   *pParserMgr,
                            IN    RvUint8                     *pcbPointer,
                            IN    ParserAcceptLanguageHeader  *pAcceptLanguage,
                            IN    SipParseType                 eType,
                            INOUT const void                  *pSipObject)
{
    RvSipAcceptLanguageHeaderHandle    hAcceptLanguageHeader;
    RvInt32                            offsetLanguage = UNDEFINED;
    RvStatus						   rv;
    HRPOOL							   hPool;
    HPAGE							   hPage;
	
    if (SIP_PARSETYPE_ACCEPT_LANGUAGE ==eType)
    {
        /*The parser is used to parse Accept stand alone header */
        hAcceptLanguageHeader=(RvSipAcceptLanguageHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_ACCEPT_LANGUAGE, 
                                    pcbPointer, (void**)&hAcceptLanguageHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitAcceptLanguageHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipAcceptLanguageHeaderGetPool(hAcceptLanguageHeader);
    hPage = SipAcceptLanguageHeaderGetPage(hAcceptLanguageHeader);

	/* language */
	rv = ParserAllocFromObjPage(pParserMgr,
								&offsetLanguage,
								hPool,
								hPage,
								pAcceptLanguage->language.buf,
								pAcceptLanguage->language.len);
	
	if (RV_OK != rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				  "ParserInitAcceptLanguageHeader - error in ParserAllocFromObjPage"));
		return rv;
	}
	
	rv = SipAcceptLanguageHeaderSetLanguage(hAcceptLanguageHeader,
										  NULL,
										  hPool,
									      hPage,
										  offsetLanguage);
	if (RV_OK != rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitAcceptLanguageHeader - error in SipAcceptLanguageHeaderSetCoding"));
		return rv;
	}

	/* Check whether q parameter has been set, if so attach it to the header*/
	if (RV_TRUE == pAcceptLanguage->isQVal)
    {
        RvInt32 offsetQval = UNDEFINED;
        rv = ParserAllocFromObjPage(pParserMgr,
                                       &offsetQval,
                                       hPool,
                                       hPage,
                                       pAcceptLanguage->qVal.buf,
                                       pAcceptLanguage->qVal.len);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitAcceptLanguageHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipAcceptLanguageHeaderSetQVal(hAcceptLanguageHeader,
                                        NULL,
                                        hPool,
                                        hPage,
                                        offsetQval);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitAcceptLanguageHeader - error in SipAcceptLanguageHeaderSetQVal"));
            return rv;
        }
    }

	/* Check whether there are other params, if so, concatenate it to the header*/
    if (RV_TRUE == pAcceptLanguage->isOtherParams)
    {
        RvInt32 offsetOtherParam = UNDEFINED;
        offsetOtherParam = ParserAppendCopyRpoolString(
										pParserMgr,
										hPool,
										hPage,
										((ParserExtensionString *)pAcceptLanguage->otherParams)->hPool,
										((ParserExtensionString *)pAcceptLanguage->otherParams)->hPage,
										0,
										((ParserExtensionString *)pAcceptLanguage->otherParams)->size);

        if (UNDEFINED == offsetOtherParam)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitAcceptLanguageHeader - error in ParserAppendCopyRpoolString"));
             return RV_ERROR_OUTOFRESOURCES;
        }
        rv = SipAcceptLanguageHeaderSetOtherParams(hAcceptLanguageHeader,
                                         NULL,
                                         hPool,
                                         hPage,
                                         offsetOtherParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitAcceptLanguageHeader - error in SipAcceptLanguageHeaderSetOtherParams"));
            return rv;
        }
    }


    return RV_OK;
}

/***************************************************************************
* ParserInitReplyToHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          Reply-To header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:   pParserMgr - Pointer to the parser manager.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             pReplyToVal - Reply-To structure holding the Reply-To values from the
*                          parser in a token form.
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone Reply-To or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_REPLY_TO
*                          it will be cast to RvSipToHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitReplyToHeader(
                             IN    ParserMgr            *pParserMgr,
                             IN    RvUint8              *pcbPointer,
                             IN    ParserReplyToHeader  *pReplyToVal,
                             IN    SipParseType          eType,
                             INOUT const void           *pSipObject)
{
    RvStatus                 rv;
    RvSipReplyToHeaderHandle hReplyToHeader;
    HRPOOL                   hPool;
    HPAGE                    hPage;
    
    if (SIP_PARSETYPE_REPLY_TO ==eType)
    {
        /*The parser is used to parse Reply-To header */
        hReplyToHeader=(RvSipReplyToHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_REPLY_TO, 
                                    pcbPointer, (void**)&hReplyToHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitReplyToHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipReplyToHeaderGetPool(hReplyToHeader);
    hPage = SipReplyToHeaderGetPage(hReplyToHeader);

    /* Init the party header in the message*/
    rv = ParserInitializeReplyToFromPcb(pParserMgr,pReplyToVal,hPool, hPage, &hReplyToHeader);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitReplyToHeader - error in InitReplyToHeaderByParser"));
        return rv;
    }

    return RV_OK;
}

/* XXX */

#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
/***************************************************************************
* ParserInitPUriHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          PUri header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr - Pointer to the parser manager.
*             pPUriVal-	PUri structure holding the PUri values from the
*                          parser in a token form.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone PUri or sip message.
*			eWhichHeader - This Type indicates which of the subtypes of P-Uri
*						   Header the header is.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_P_URI
*                          it will be cast to RvSipPUriHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitPUriHeader(
                             IN    ParserMgr                 *pParserMgr,
                             IN    SipParser_pcb_type       *pcb,
                             IN    RvUint8                  *pcbPointer,
                             IN    SipParseType              eType,
							 IN	   SipParseType				 eWhichHeader,
                             INOUT const void                *pSipObject)
{
    RvStatus				rv;
    RvSipPUriHeaderHandle	hPUriHeader;
    RvSipAddressHandle      hAddress;
    HRPOOL                  hPool;
    HPAGE                   hPage;
    RvSipAddressType        eAddrType;
	ParserPUriHeader		*pPUriHeader;
	RvSipPUriHeaderType		ePHeaderType;

    if (SIP_PARSETYPE_P_ASSOCIATED_URI == eType || 
		SIP_PARSETYPE_P_CALLED_PARTY_ID == eType ||
		SIP_PARSETYPE_P_ASSERTED_IDENTITY == eType ||
		SIP_PARSETYPE_P_PREFERRED_IDENTITY == eType)
    {
        /*The parser is used to parse PUri header */
        hPUriHeader=(RvSipPUriHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_P_URI, 
                                   pcbPointer, (void**)&hPUriHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitPUriHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipPUriHeaderGetPool(hPUriHeader);
    hPage = SipPUriHeaderGetPage(hPUriHeader);

	if (SIP_PARSETYPE_P_ASSOCIATED_URI == eWhichHeader)
    {
		ePHeaderType = RVSIP_P_URI_ASSOCIATED_URI_HEADER;
	}
	else if (SIP_PARSETYPE_P_CALLED_PARTY_ID == eWhichHeader)
	{
		ePHeaderType = RVSIP_P_URI_CALLED_PARTY_ID_HEADER;
	}
	else if (SIP_PARSETYPE_P_ASSERTED_IDENTITY == eWhichHeader)
	{
		ePHeaderType = RVSIP_P_URI_ASSERTED_IDENTITY_HEADER;
	}
	else if (SIP_PARSETYPE_P_PREFERRED_IDENTITY == eWhichHeader)
	{
		ePHeaderType = RVSIP_P_URI_PREFERRED_IDENTITY_HEADER;
	}
	else
	{
		RvLogExcep(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
            "ParserInitPUriHeader - Unknown P-Header Type"));
		return RV_ERROR_UNKNOWN;
	}

    rv = RvSipPUriHeaderSetPHeaderType(hPUriHeader, ePHeaderType);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
        "ParserInitPUriHeader - error in RvSipPUriHeaderSetPHeaderType"));
		return rv;
    }
	
	pPUriHeader = &pcb->puriHeader;
    eAddrType = ParserAddrType2MsgAddrType(pPUriHeader->nameAddr.exUri.uriType);
    if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitPUriHeader - unknown parserAddressType %d", pPUriHeader->nameAddr.exUri.uriType));
        return RV_ERROR_UNKNOWN;
    }

    /* Construct the url in the PUri header */
    rv = RvSipAddrConstructInPUriHeader(hPUriHeader,
                                              eAddrType,
                                              &hAddress);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitPUriHeader - error in RvSipAddrConstructInPUriHeader"));
        return rv;
    }

    /* Initialize the sip-url\abs-uri values from the parser*/
    rv = ParserInitAddressInHeader(pParserMgr,
                                      &pPUriHeader->nameAddr.exUri,
                                      hPool, hPage, eAddrType, hAddress);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitPUriHeader - error in ParserInitAddressInHeader"));
        return rv;
    }

	/* Check whether the display name has been set, if so attach
       it to the PUri header  */
    if (RV_TRUE == pPUriHeader->nameAddr.isDisplayName)
    {
        RvInt32 offsetName = UNDEFINED;
        rv= ParserAllocFromObjPage(pParserMgr,
                                     &offsetName,
                                     hPool,
                                     hPage,
                                     pPUriHeader->nameAddr.name.buf,
                                     pPUriHeader->nameAddr.name.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitPUriHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        /* Set the display name in the PUri header */
        rv = SipPUriHeaderSetDisplayName(hPUriHeader,
                                               NULL,
                                               hPool,
                                               hPage,
                                               offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitPUriHeader - error in PUriHeaderSetDisplayName"));
            return rv;
        }
    }

	/* Check whether Other parameter has been set, if so attach it to
       strOtherParam in the PUri header*/
    if (RV_TRUE == pPUriHeader->isOtherParams)
    {
		RvInt32 offsetOtherParam = UNDEFINED;
		offsetOtherParam = ParserAppendCopyRpoolString(
														pParserMgr,
														hPool,
														hPage,
														((ParserExtensionString *)pPUriHeader->otherParams)->hPool,
														((ParserExtensionString *)pPUriHeader->otherParams)->hPage,
														0,
														((ParserExtensionString *)pPUriHeader->otherParams)->size);

		if (UNDEFINED == offsetOtherParam)
		{
			 RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPUriHeader - error in ParserAppendCopyRpoolString"));
			 return RV_ERROR_OUTOFRESOURCES;
		}
		rv = SipPUriHeaderSetOtherParams(hPUriHeader,
											 NULL,
											 hPool,
											 hPage,
											 offsetOtherParam);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPUriHeader - error in SipPUriHeaderSetOtherParams"));
			return rv;
		}
	}
    
    return RV_OK;
}

/***************************************************************************
* ParserInitPVisitedNetworkIDHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          PVisitedNetworkID header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:   pParserMgr - Pointer to the parser manager.
*             pcb		 - PVisitedNetworkID structure holding the PVisitedNetworkID 
*						   values from the parser in a token form.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone PVisitedNetworkID or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_P_VISITED_NETWORK_ID
*                          it will be cast to RvSipPVisitedNetworkIDHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitPVisitedNetworkIDHeader(
                             IN    ParserMgr                *pParserMgr,
                             IN    SipParser_pcb_type       *pcb,
                             IN    RvUint8                  *pcbPointer,
                             IN    SipParseType              eType,
                             INOUT const void               *pSipObject)
{
    RvStatus							rv;
	RvInt32								offsetName = UNDEFINED;
    RvSipPVisitedNetworkIDHeaderHandle	hHeader;
    HRPOOL								hPool;
    HPAGE								hPage;
	ParserPVisitedNetworkIDHeader		*pHeader;
	

    if (SIP_PARSETYPE_P_VISITED_NETWORK_ID == eType)
    {
        /*The parser is used to parse PVisitedNetworkID header */
        hHeader=(RvSipPVisitedNetworkIDHeaderHandle)pSipObject;
    }
    else
    {

        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_P_VISITED_NETWORK_ID, 
                                   pcbPointer, (void**)&hHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitPVisitedNetworkIDHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipPVisitedNetworkIDHeaderGetPool(hHeader);
    hPage = SipPVisitedNetworkIDHeaderGetPage(hHeader);

	pHeader = &pcb->pvisitedNetworkIDHeader;
    
    /* vnetwork-spec */
    
    rv= ParserAllocFromObjPage(pParserMgr,
                                 &offsetName,
                                 hPool,
                                 hPage,
                                 pHeader->vnetworkSpec.buf,
                                 pHeader->vnetworkSpec.len);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
          "ParserInitPVisitedNetworkIDHeader - error in ParserAllocFromObjPage"));
        return rv;
    }

    /* Set the vnetwork-spec in the PVisitedNetworkID header */
    rv = SipPVisitedNetworkIDHeaderSetVNetworkSpec(hHeader,
                                           NULL,
                                           hPool,
                                           hPage,
                                           offsetName);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
          "ParserInitPVisitedNetworkIDHeader - error in SipPVisitedNetworkIDHeaderSetVNetworkSpec"));
        return rv;
    }

	/* Check whether Other parameter has been set, if so attach it to
       strOtherParam in the PVisitedNetworkID header*/
    if (RV_TRUE == pHeader->isOtherParams)
    {
		RvInt32 offsetOtherParam = UNDEFINED;
		offsetOtherParam = ParserAppendCopyRpoolString(
														pParserMgr,
														hPool,
														hPage,
														((ParserExtensionString *)pHeader->otherParams)->hPool,
														((ParserExtensionString *)pHeader->otherParams)->hPage,
														0,
														((ParserExtensionString *)pHeader->otherParams)->size);

		if (UNDEFINED == offsetOtherParam)
		{
			 RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPVisitedNetworkIDHeader - error in ParserAppendCopyRpoolString"));
			 return RV_ERROR_OUTOFRESOURCES;
		}
		rv = SipPVisitedNetworkIDHeaderSetOtherParams(hHeader,
											 NULL,
											 hPool,
											 hPage,
											 offsetOtherParam);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPVisitedNetworkIDHeader - error in SipPVisitedNetworkIDHeaderSetOtherParams"));
			return rv;
		}
	}
    
    return RV_OK;
}

/***************************************************************************
* ParserInitPAccessNetworkInfoHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          PAccessNetworkInfo header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:   pParserMgr - Pointer to the parser manager.
*             pcb		 - PAccessNetworkInfo structure holding the PAccessNetworkInfo 
*						   values from the parser in a token form.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone PAccessNetworkInfo or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_P_ACCESS_NETWORK_INFO
*                          it will be cast to RvSipPAccessNetworkInfoHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitPAccessNetworkInfoHeader(
                             IN    ParserMgr                *pParserMgr,
                             IN    SipParser_pcb_type       *pcb,
                             IN    RvUint8                  *pcbPointer,
                             IN    SipParseType              eType,
                             INOUT const void               *pSipObject)
{
    RvStatus							rv;
	RvInt32								offsetName = UNDEFINED;
    RvSipPAccessNetworkInfoHeaderHandle	hHeader;
    HRPOOL								hPool;
    HPAGE								hPage;
	ParserPAccessNetworkInfoHeader		*pHeader;
	

    if (SIP_PARSETYPE_P_ACCESS_NETWORK_INFO == eType)
    {
        /* The parser is used to parse PAccessNetworkInfo header */
        hHeader=(RvSipPAccessNetworkInfoHeaderHandle)pSipObject;
    }
    else
    {

        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_P_ACCESS_NETWORK_INFO, 
                                   pcbPointer, (void**)&hHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitPAccessNetworkInfoHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipPAccessNetworkInfoHeaderGetPool(hHeader);
    hPage = SipPAccessNetworkInfoHeaderGetPage(hHeader);

	pHeader = &pcb->paccessNetworkInfoHeader;
    
    /* accessType */
	if(pHeader->accessType.type == PARSER_ACCESS_TYPE_OTHER)
	{
		rv= ParserAllocFromObjPage(pParserMgr,
									 &offsetName,
									 hPool,
									 hPage,
									 pHeader->accessType.other.buf,
									 pHeader->accessType.other.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPAccessNetworkInfoHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the access Type in the PAccessNetworkInfo header */
		rv = SipPAccessNetworkInfoHeaderSetAccessType(hHeader,
											   RVSIP_ACCESS_TYPE_OTHER, NULL,
											   hPool,
											   hPage,
											   offsetName);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPAccessNetworkInfoHeader - error in SipPAccessNetworkInfoHeaderSetAccessType"));
			return rv;
		}
	}
	else
	{
		RvSipPAccessNetworkInfoAccessType eAccessType = RVSIP_ACCESS_TYPE_UNDEFINED;
		switch(pHeader->accessType.type)
		{
			case(PARSER_ACCESS_TYPE_IEEE_802_11A):
				eAccessType = RVSIP_ACCESS_TYPE_IEEE_802_11A;
				break;
			case(PARSER_ACCESS_TYPE_IEEE_802_11B):
				eAccessType = RVSIP_ACCESS_TYPE_IEEE_802_11B;
				break;
			case(PARSER_ACCESS_TYPE_3GPP_GERAN):
				eAccessType = RVSIP_ACCESS_TYPE_3GPP_GERAN;
				break;
			case(PARSER_ACCESS_TYPE_3GPP_UTRAN_FDD):
				eAccessType = RVSIP_ACCESS_TYPE_3GPP_UTRAN_FDD;
				break;
			case(PARSER_ACCESS_TYPE_3GPP_UTRAN_TDD):
				eAccessType = RVSIP_ACCESS_TYPE_3GPP_UTRAN_TDD;
				break;
			case(PARSER_ACCESS_TYPE_3GPP_CDMA2000):
				eAccessType = RVSIP_ACCESS_TYPE_3GPP_CDMA2000;
				break;
			case(PARSER_ACCESS_TYPE_ADSL):
				eAccessType = RVSIP_ACCESS_TYPE_ADSL;
				break;
			case(PARSER_ACCESS_TYPE_ADSL2):
				eAccessType = RVSIP_ACCESS_TYPE_ADSL2;
				break;
			case(PARSER_ACCESS_TYPE_ADSL2_PLUS):
				eAccessType = RVSIP_ACCESS_TYPE_ADSL2_PLUS;
				break;
			case(PARSER_ACCESS_TYPE_RADSL):
				eAccessType = RVSIP_ACCESS_TYPE_RADSL;
				break;
			case(PARSER_ACCESS_TYPE_SDSL):
				eAccessType = RVSIP_ACCESS_TYPE_SDSL;
				break;
			case(PARSER_ACCESS_TYPE_HDSL):
				eAccessType = RVSIP_ACCESS_TYPE_HDSL;
				break;
			case(PARSER_ACCESS_TYPE_HDSL2):
				eAccessType = RVSIP_ACCESS_TYPE_HDSL2;
				break;
			case(PARSER_ACCESS_TYPE_G_SHDSL):
				eAccessType = RVSIP_ACCESS_TYPE_G_SHDSL;
				break;
			case(PARSER_ACCESS_TYPE_VDSL):
				eAccessType = RVSIP_ACCESS_TYPE_VDSL;
				break;
			case(PARSER_ACCESS_TYPE_IDSL):
				eAccessType = RVSIP_ACCESS_TYPE_IDSL;
				break;
			case(PARSER_ACCESS_TYPE_3GPP2_1X):
				eAccessType = RVSIP_ACCESS_TYPE_3GPP2_1X;
				break;
			case(PARSER_ACCESS_TYPE_3GPP2_1X_HRPD):
				eAccessType = RVSIP_ACCESS_TYPE_3GPP2_1X_HRPD;
				break;
            case(PARSER_ACCESS_TYPE_DOCSIS):
				eAccessType = RVSIP_ACCESS_TYPE_DOCSIS;
				break;
            case(PARSER_ACCESS_TYPE_IEEE_802_11):
				eAccessType = RVSIP_ACCESS_TYPE_IEEE_802_11;
				break;
			case(PARSER_ACCESS_TYPE_IEEE_802_11G):
				eAccessType = RVSIP_ACCESS_TYPE_IEEE_802_11G;
				break;
            default:
                eAccessType = RVSIP_ACCESS_TYPE_UNDEFINED;

		}

		/* Set the Access Type in the PAccessNetworkInfo header */
		rv = SipPAccessNetworkInfoHeaderSetAccessType(hHeader,
											   eAccessType, NULL,
											   NULL,
											   NULL,
											   UNDEFINED);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPAccessNetworkInfoHeader - error in SipPAccessNetworkInfoHeaderSetAccessType"));
			return rv;
		}
	}

    /* Set the network-provided parameter */
	RvSipPAccessNetworkInfoHeaderSetNetworkProvided(hHeader, pHeader->bNetworkProvided);

	/* Check whether cgi3gpp parameter has been set, if so attach it to
       Cgi3gpp in the PAccessNetworkInfo header */
    if (RV_TRUE == pHeader->isCgi3gpp)
    {
		RvInt32 offsetCgi3gpp = UNDEFINED;
		rv= ParserAllocFromObjPage(pParserMgr,
									 &offsetCgi3gpp,
									 hPool,
									 hPage,
									 pHeader->cgi3gpp.buf,
									 pHeader->cgi3gpp.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPAccessNetworkInfoHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the Cgi3gpp in the PAccessNetworkInfo header */
		rv = SipPAccessNetworkInfoHeaderSetStrCgi3gpp(hHeader,
														 NULL,
														 hPool,
														 hPage,
														 offsetCgi3gpp);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPAccessNetworkInfoHeader - error in SipPAccessNetworkInfoHeaderSetStrCgi3gpp"));
			return rv;
		}
	}

	/* Check whether Utran-Cell-id-3gpp parameter has been set, if so attach it to
       utranCellId3gpp in the PAccessNetworkInfo header */
    if (RV_TRUE == pHeader->isUtranCellId3gpp)
    {
		RvInt32 offsetUtranCellId3gpp = UNDEFINED;
		rv= ParserAllocFromObjPage(pParserMgr,
									 &offsetUtranCellId3gpp,
									 hPool,
									 hPage,
									 pHeader->utranCellId3gpp.buf,
									 pHeader->utranCellId3gpp.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPAccessNetworkInfoHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the UtranCellId3gpp in the PAccessNetworkInfo header */
		rv = SipPAccessNetworkInfoHeaderSetStrUtranCellId3gpp(hHeader,
														 NULL,
														 hPool,
														 hPage,
														 offsetUtranCellId3gpp);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPAccessNetworkInfoHeader - error in SipPAccessNetworkInfoHeaderSetStrUtranCellId3gpp"));
			return rv;
		}
	}

    /* Check whether i-wlan-node-id parameter has been set, if so attach it to
       IWlanNodeID in the PAccessNetworkInfo header */
    if (RV_TRUE == pHeader->isIWlanNodeID)
    {
		RvInt32 offsetIWlanNodeID = UNDEFINED;
		rv= ParserAllocFromObjPage(pParserMgr,
									 &offsetIWlanNodeID,
									 hPool,
									 hPage,
									 pHeader->iWlanNodeID.buf,
									 pHeader->iWlanNodeID.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPAccessNetworkInfoHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the IWlanNodeID in the PAccessNetworkInfo header */
		rv = SipPAccessNetworkInfoHeaderSetStrIWlanNodeID(hHeader,
														 NULL,
														 hPool,
														 hPage,
														 offsetIWlanNodeID);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPAccessNetworkInfoHeader - error in SipPAccessNetworkInfoHeaderSetStrIWlanNodeID"));
			return rv;
		}
	}

	/* Check whether dsl-location parameter has been set, if so attach it to
       DslLocation in the PAccessNetworkInfo header */
    if (RV_TRUE == pHeader->isDslLocation)
    {
		RvInt32 offsetDslLocation = UNDEFINED;
		rv= ParserAllocFromObjPage(pParserMgr,
									 &offsetDslLocation,
									 hPool,
									 hPage,
									 pHeader->dslLocation.buf,
									 pHeader->dslLocation.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPAccessNetworkInfoHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the DslLocation in the PAccessNetworkInfo header */
		rv = SipPAccessNetworkInfoHeaderSetStrDslLocation(hHeader,
														 NULL,
														 hPool,
														 hPage,
														 offsetDslLocation);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPAccessNetworkInfoHeader - error in SipPAccessNetworkInfoHeaderSetStrDslLocation"));
			return rv;
		}
	}

	/* Check whether ci-3gpp2 parameter has been set, if so attach it to
       ci3gpp2 in the PAccessNetworkInfo header */
    if (RV_TRUE == pHeader->isCi3gpp2)
    {
		RvInt32 offsetCi3gpp2 = UNDEFINED;
		rv= ParserAllocFromObjPage(pParserMgr,
									 &offsetCi3gpp2,
									 hPool,
									 hPage,
									 pHeader->ci3gpp2.buf,
									 pHeader->ci3gpp2.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPAccessNetworkInfoHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the Ci3gpp2 in the PAccessNetworkInfo header */
		rv = SipPAccessNetworkInfoHeaderSetStrCi3gpp2(hHeader,
														 NULL,
														 hPool,
														 hPage,
														 offsetCi3gpp2);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPAccessNetworkInfoHeader - error in SipPAccessNetworkInfoHeaderSetStrCi3gpp2"));
			return rv;
		}
	}
	
	/* Check whether Other parameter has been set, if so attach it to
       strOtherParam in the PAccessNetworkInfo header */
    if (RV_TRUE == pHeader->isOtherParams)
    {
		RvInt32 offsetOtherParam = UNDEFINED;
		offsetOtherParam = ParserAppendCopyRpoolString(
														pParserMgr,
														hPool,
														hPage,
														((ParserExtensionString *)pHeader->otherParams)->hPool,
														((ParserExtensionString *)pHeader->otherParams)->hPage,
														0,
														((ParserExtensionString *)pHeader->otherParams)->size);

		if (UNDEFINED == offsetOtherParam)
		{
			 RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPAccessNetworkInfoHeader - error in ParserAppendCopyRpoolString"));
			 return RV_ERROR_OUTOFRESOURCES;
		}
		rv = SipPAccessNetworkInfoHeaderSetOtherParams(hHeader,
											 NULL,
											 hPool,
											 hPage,
											 offsetOtherParam);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPAccessNetworkInfoHeader - error in SipPAccessNetworkInfoHeaderSetOtherParams"));
			return rv;
		}
	}
    
    return RV_OK;
}

/***************************************************************************
* ParserInitPChargingFunctionAddressesHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          PChargingFunctionAddresses header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:   pParserMgr - Pointer to the parser manager.
*             pcb		 - PChargingFunctionAddresses structure holding the  
*						   values from the parser in a token form.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone PChargingFunctionAddresses or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_P_CHARGING_FUNCTION_ADDRESSES
*                          it will be cast to RvSipPChargingFunctionAddressesHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitPChargingFunctionAddressesHeader(
                             IN    ParserMgr                *pParserMgr,
                             IN    SipParser_pcb_type       *pcb,
                             IN    RvUint8                  *pcbPointer,
                             IN    SipParseType              eType,
                             INOUT const void               *pSipObject)
{
    RvStatus									rv;
    RvSipPChargingFunctionAddressesHeaderHandle	hHeader;
    HRPOOL										hPool;
    HPAGE										hPage;
	ParserPChargingFunctionAddressesHeader		*pHeader;
	

    if (SIP_PARSETYPE_P_CHARGING_FUNCTION_ADDRESSES == eType)
    {
        /* The parser is used to parse PChargingFunctionAddresses header */
        hHeader=(RvSipPChargingFunctionAddressesHeaderHandle)pSipObject;
    }
    else
    {

        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, 
								   RVSIP_HEADERTYPE_P_CHARGING_FUNCTION_ADDRESSES, 
                                   pcbPointer, (void**)&hHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitPChargingFunctionAddressesHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipPChargingFunctionAddressesHeaderGetPool(hHeader);
    hPage = SipPChargingFunctionAddressesHeaderGetPage(hHeader);

	pHeader = &pcb->pchargingFunctionAddressesHeader;
    
	/* Place the ccfList in the header */
	if (RV_TRUE == pHeader->isCcfList)
	{
		MsgPChargingFunctionAddressesHeader* header = (MsgPChargingFunctionAddressesHeader*)hHeader;
		header->ccfList = pHeader->ccfList;
	}

	/* Place the ecfList in the header */
	if (RV_TRUE == pHeader->isEcfList)
	{
		MsgPChargingFunctionAddressesHeader* header = (MsgPChargingFunctionAddressesHeader*)hHeader;
		header->ecfList = pHeader->ecfList;
	}

	/* Check whether Other parameter has been set, if so attach it to
       strOtherParam in the PChargingFunctionAddresses header */
    if (RV_TRUE == pHeader->isOtherParams)
    {
		RvInt32 offsetOtherParam = UNDEFINED;
		offsetOtherParam = ParserAppendCopyRpoolString(
														pParserMgr,
														hPool,
														hPage,
														((ParserExtensionString *)pHeader->otherParams)->hPool,
														((ParserExtensionString *)pHeader->otherParams)->hPage,
														0,
														((ParserExtensionString *)pHeader->otherParams)->size);

		if (UNDEFINED == offsetOtherParam)
		{
			 RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPChargingFunctionAddressesHeader - error in ParserAppendCopyRpoolString"));
			 return RV_ERROR_OUTOFRESOURCES;
		}
		rv = SipPChargingFunctionAddressesHeaderSetOtherParams(hHeader,
																 NULL,
																 hPool,
																 hPage,
																 offsetOtherParam);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPChargingFunctionAddressesHeader - error in SipPChargingFunctionAddressesHeaderSetOtherParams"));
			return rv;
		}
	}
    
    return RV_OK;
}

/***************************************************************************
* ParserInitPChargingVectorHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          PChargingVector header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:   pParserMgr - Pointer to the parser manager.
*             pcb		 - PChargingVector structure holding the  
*						   values from the parser in a token form.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone PChargingVector or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_P_CHARGING_VECTOR
*                          it will be cast to RvSipPChargingVectorHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitPChargingVectorHeader(
                             IN    ParserMgr                *pParserMgr,
                             IN    SipParser_pcb_type       *pcb,
                             IN    RvUint8                  *pcbPointer,
                             IN    SipParseType              eType,
                             INOUT const void               *pSipObject)
{
    RvStatus							rv;
    RvSipPChargingVectorHeaderHandle	hHeader;
    HRPOOL								hPool;
    HPAGE								hPage;
	ParserPChargingVectorHeader			*pHeader;
	RvInt32								offsetIcidValue = UNDEFINED;

    if (SIP_PARSETYPE_P_CHARGING_VECTOR == eType)
    {
        /* The parser is used to parse PChargingVector header */
        hHeader=(RvSipPChargingVectorHeaderHandle)pSipObject;
    }
    else
    {

        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_P_CHARGING_VECTOR, 
                                   pcbPointer, (void**)&hHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitPChargingVectorHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipPChargingVectorHeaderGetPool(hHeader);
    hPage = SipPChargingVectorHeaderGetPage(hHeader);

	pHeader = &pcb->pchargingVectorHeader;
    
	/* icidValue */
	rv= ParserAllocFromObjPage(pParserMgr,
								 &offsetIcidValue,
								 hPool,
								 hPage,
								 pHeader->icidValue.buf,
								 pHeader->icidValue.len);
	if (RV_OK!=rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
		  "ParserInitPChargingVectorHeader - error in ParserAllocFromObjPage"));
		return rv;
	}

	/* Set the icidValue in the PChargingVector header */
	rv = SipPChargingVectorHeaderSetStrIcidValue(hHeader,
												   NULL,
												   hPool,
												   hPage,
												   offsetIcidValue);
	if (RV_OK != rv)
	{
		RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
		  "ParserInitPChargingVectorHeader - error in SipPChargingVectorHeaderSetStrIcidValue"));
		return rv;
	}


	/* Check whether icid-gen-addr parameter has been set, if so attach it to
       icid-gen-addr parameter in the PChargingVector header */
    if (RV_TRUE == pHeader->isIcidGenAddr)
    {
		RvInt32 offsetIcidGenAddr = UNDEFINED;
		rv= ParserAllocFromObjPage(pParserMgr,
									 &offsetIcidGenAddr,
									 hPool,
									 hPage,
									 pHeader->icidGenAddr.buf,
									 pHeader->icidGenAddr.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPChargingVectorHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the icid-gen-addr parameter in the PChargingVector header */
		rv = SipPChargingVectorHeaderSetStrIcidGenAddr(hHeader,
														 NULL,
														 hPool,
														 hPage,
														 offsetIcidGenAddr);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPChargingVectorHeader - error in SipPChargingVectorHeaderSetStrIcidGenAddr"));
			return rv;
		}
	}

	/* Check whether orig-ioi parameter has been set, if so attach it to
       orig-ioi parameter in the PChargingVector header */
    if (RV_TRUE == pHeader->isOrigIoi)
    {
		RvInt32 offsetOrigIoi = UNDEFINED;
		rv= ParserAllocFromObjPage(pParserMgr,
									 &offsetOrigIoi,
									 hPool,
									 hPage,
									 pHeader->origIoi.buf,
									 pHeader->origIoi.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPChargingVectorHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the orig-ioi parameter in the PChargingVector header */
		rv = SipPChargingVectorHeaderSetStrOrigIoi(hHeader,
													 NULL,
													 hPool,
													 hPage,
													 offsetOrigIoi);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPChargingVectorHeader - error in SipPChargingVectorHeaderSetStrOrigIoi"));
			return rv;
		}
	}

	/* Check whether term-ioi parameter has been set, if so attach it to
       term-ioi parameter in the PChargingVector header */
    if (RV_TRUE == pHeader->isTermIoi)
    {
		RvInt32 offsetTermIoi = UNDEFINED;
		rv= ParserAllocFromObjPage(pParserMgr,
									 &offsetTermIoi,
									 hPool,
									 hPage,
									 pHeader->termIoi.buf,
									 pHeader->termIoi.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPChargingVectorHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the term-ioi parameter in the PChargingVector header */
		rv = SipPChargingVectorHeaderSetStrTermIoi(hHeader,
													NULL,
													hPool,
													hPage,
													offsetTermIoi);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPChargingVectorHeader - error in SipPChargingVectorHeaderSetStrTermIoi"));
			return rv;
		}
	}

	/* Check whether Ggsn, auth-token and pdp-info-hierarchy parameters have been set, 
	   if so attach it to the PChargingVector header */
    if (RV_TRUE == pHeader->isGgsn)
    {
		RvInt32 offsetGgsn			= UNDEFINED;
		RvInt32 offsetGprsAuthToken = UNDEFINED;
		
		rv= ParserAllocFromObjPage( pParserMgr,
									&offsetGgsn,
									hPool,
									hPage,
									pHeader->ggsn.buf,
									pHeader->ggsn.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPChargingVectorHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the ggsn parameter in the PChargingVector header */
		rv = SipPChargingVectorHeaderSetStrGgsn(hHeader,
												NULL,
												hPool,
												hPage,
												offsetGgsn);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPChargingVectorHeader - error in SipPChargingVectorHeaderSetStrGgsn"));
			return rv;
		}

		/* Auth-token */
		rv= ParserAllocFromObjPage( pParserMgr,
									&offsetGprsAuthToken,
									hPool,
									hPage,
									pHeader->gprsAuthToken.buf,
									pHeader->gprsAuthToken.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPChargingVectorHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the gprsAuthToken parameter in the PChargingVector header */
		rv = SipPChargingVectorHeaderSetStrGprsAuthToken(hHeader,
														NULL,
														hPool,
														hPage,
														offsetGprsAuthToken);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPChargingVectorHeader - error in SipPChargingVectorHeaderSetStrGprsAuthToken"));
			return rv;
		}

		/* Place the pdpInfoList in the header */
		if (RV_TRUE == pHeader->isPdpInfoList)
		{
			MsgPChargingVectorHeader* header = (MsgPChargingVectorHeader*)hHeader;
			header->pdpInfoList = pHeader->pdpInfoList;
		}
	}
	
	/* Check whether Bras, auth-token and pdp-info-hierarchy parameters have been set, 
	   if so attach it to the PChargingVector header */
    if (RV_TRUE == pHeader->isBras)
    {
		RvInt32 offsetBras			= UNDEFINED;
		RvInt32 offsetXdslAuthToken = UNDEFINED;
		
		rv= ParserAllocFromObjPage( pParserMgr,
									&offsetBras,
									hPool,
									hPage,
									pHeader->bras.buf,
									pHeader->bras.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPChargingVectorHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the Bras parameter in the PChargingVector header */
		rv = SipPChargingVectorHeaderSetStrBras(hHeader,
												NULL,
												hPool,
												hPage,
												offsetBras);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPChargingVectorHeader - error in SipPChargingVectorHeaderSetStrBras"));
			return rv;
		}

		/* Auth-token */
		rv= ParserAllocFromObjPage( pParserMgr,
									&offsetXdslAuthToken,
									hPool,
									hPage,
									pHeader->xdslAuthToken.buf,
									pHeader->xdslAuthToken.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPChargingVectorHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the xdslAuthToken parameter in the PChargingVector header */
		rv = SipPChargingVectorHeaderSetStrXdslAuthToken(hHeader,
														NULL,
														hPool,
														hPage,
														offsetXdslAuthToken);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPChargingVectorHeader - error in SipPChargingVectorHeaderSetStrXdslAuthToken"));
			return rv;
		}

		/* Place the dslBearerInfoList in the header */
		if (RV_TRUE == pHeader->isDslBearerInfoList)
		{
			MsgPChargingVectorHeader* header = (MsgPChargingVectorHeader*)hHeader;
			header->dslBearerInfoList = pHeader->dslBearerInfoList;
		}
	}

	/* Set the IWlanChargingInfo parameter */
	RvSipPChargingVectorHeaderSetIWlanChargingInfo(hHeader, pHeader->bWLanChargingInfo);

    /* Set the PacketcableChargingInfo parameter */
	RvSipPChargingVectorHeaderSetPacketcableChargingInfo(hHeader, pHeader->bPacketcableChargingInfo);

    /* Check whether bcid parameter has been set, if so attach it to
       bcid parameter in the PChargingVector header */
    if (RV_TRUE == pHeader->isBCid)
    {
		RvInt32 offsetBCid = UNDEFINED;
		rv= ParserAllocFromObjPage(pParserMgr,
									 &offsetBCid,
									 hPool,
									 hPage,
									 pHeader->bcid.buf,
									 pHeader->bcid.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPChargingVectorHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the bcid parameter in the PChargingVector header */
		rv = SipPChargingVectorHeaderSetStrBCid(hHeader,
													 NULL,
													 hPool,
													 hPage,
													 offsetBCid);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPChargingVectorHeader - error in SipPChargingVectorHeaderSetStrBCid"));
			return rv;
		}
	}

	/* Check whether Other parameter has been set, if so attach it to
       strOtherParam in the PChargingVector header */
    if (RV_TRUE == pHeader->isOtherParams)
    {
		RvInt32 offsetOtherParam = UNDEFINED;
		offsetOtherParam = ParserAppendCopyRpoolString(
														pParserMgr,
														hPool,
														hPage,
														((ParserExtensionString *)pHeader->otherParams)->hPool,
														((ParserExtensionString *)pHeader->otherParams)->hPage,
														0,
														((ParserExtensionString *)pHeader->otherParams)->size);

		if (UNDEFINED == offsetOtherParam)
		{
			 RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPChargingVectorHeader - error in ParserAppendCopyRpoolString"));
			 return RV_ERROR_OUTOFRESOURCES;
		}
		rv = SipPChargingVectorHeaderSetOtherParams(hHeader,
													 NULL,
													 hPool,
													 hPage,
													 offsetOtherParam);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPChargingVectorHeader - error in SipPChargingVectorHeaderSetOtherParams"));
			return rv;
		}
	}
    
    return RV_OK;
}

/***************************************************************************
* ParserInitPMediaAuthorizationHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          PMediaAuthorization header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:   pParserMgr - Pointer to the parser manager.
*             pcb		 - PMediaAuthorization structure holding the  
*						   values from the parser in a token form.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone PMediaAuthorization or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_P_MEDIA_AUTHORIZATION
*                          it will be cast to RvSipPMediaAuthorizationHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitPMediaAuthorizationHeader(
                             IN    ParserMgr                *pParserMgr,
                             IN    SipParser_pcb_type       *pcb,
                             IN    RvUint8                  *pcbPointer,
                             IN    SipParseType              eType,
                             INOUT const void               *pSipObject)
{
    RvStatus								rv;
    RvSipPMediaAuthorizationHeaderHandle	hHeader;
    HRPOOL									hPool;
    HPAGE									hPage;
	ParserPMediaAuthorizationHeader		   *pHeader;
	RvInt32									offsetToken = UNDEFINED;

    if (SIP_PARSETYPE_P_MEDIA_AUTHORIZATION == eType)
    {
        /* The parser is used to parse PMediaAuthorization header */
        hHeader=(RvSipPMediaAuthorizationHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_P_MEDIA_AUTHORIZATION, 
                                   pcbPointer, (void**)&hHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitPMediaAuthorizationHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipPMediaAuthorizationHeaderGetPool(hHeader);
    hPage = SipPMediaAuthorizationHeaderGetPage(hHeader);

	pHeader = &pcb->pmediaAuthorizationHeader;
    
	/* token */
	rv= ParserAllocFromObjPage(pParserMgr,
								 &offsetToken,
								 hPool,
								 hPage,
								 pHeader->token.buf,
								 pHeader->token.len);
	if (RV_OK!=rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
		  "ParserInitPMediaAuthorizationHeader - error in ParserAllocFromObjPage"));
		return rv;
	}

	/* Set the token in the PMediaAuthorization header */
	rv = SipPMediaAuthorizationHeaderSetToken(hHeader,
												   NULL,
												   hPool,
												   hPage,
												   offsetToken);
	if (RV_OK != rv)
	{
		RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
		  "ParserInitPMediaAuthorizationHeader - error in SipPMediaAuthorizationHeaderSetToken"));
		return rv;
	}
    
    return RV_OK;
}

/***************************************************************************
* ParserInitSecurityHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          Security header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:   pParserMgr   - Pointer to the parser manager.
*             pSecurityVal - Security structure holding the Security values from the
*							 parser in a token form.
*             pcbPointer   - Pointer to the current location of parser pointer
*                            in msg buffer (the end of the given header).
*             eParseType   - This type indicates whether the parser is called to
*                            parse stand alone Security or sip message.
*			  eWhichHeader - This Type indicates which of the subtypes of Security
*						     Header the header is.
*  In/Output: pSipObject   - Pointer to the structure in which the values from
*                            the parser will be set.
*                            If eType == SIP_PARSETYPE_MSG it will be cast to
*                            RvSipMsgHandle and if eType == SIP_PARSETYPE_P_URI
*                            it will be cast to RvSipSecurityHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitSecurityHeader(
                             IN    ParserMgr			*pParserMgr,
                             IN    SipParser_pcb_type	*pcb,
                             IN    RvUint8				*pcbPointer,
                             IN    SipParseType			 eParseType,
							 IN	   SipParseType			 eWhichHeader,
                             INOUT const void	        *pSipObject)
{
    RvStatus					rv;
    RvSipSecurityHeaderHandle	hSecurityHeader;
    HRPOOL						hPool;
    HPAGE						hPage;
	ParserSecurityHeader	   *pSecurityHeader;
	RvSipSecurityHeaderType		eSecurityHeaderType;

    if (SIP_PARSETYPE_SECURITY_CLIENT == eParseType || 
		SIP_PARSETYPE_SECURITY_SERVER == eParseType ||
		SIP_PARSETYPE_SECURITY_VERIFY == eParseType)
    {
        /*The parser is used to parse Security header */
        hSecurityHeader=(RvSipSecurityHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eParseType, pSipObject,
                RVSIP_HEADERTYPE_SECURITY,pcbPointer,(void**)&hSecurityHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitSecurityHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipSecurityHeaderGetPool(hSecurityHeader);
    hPage = SipSecurityHeaderGetPage(hSecurityHeader);

	if (SIP_PARSETYPE_SECURITY_CLIENT == eWhichHeader)
    {
		eSecurityHeaderType = RVSIP_SECURITY_CLIENT_HEADER;
	}
	else if (SIP_PARSETYPE_SECURITY_SERVER == eWhichHeader)
	{
		eSecurityHeaderType = RVSIP_SECURITY_SERVER_HEADER;
	}
	else if (SIP_PARSETYPE_SECURITY_VERIFY == eWhichHeader)
	{
		eSecurityHeaderType = RVSIP_SECURITY_VERIFY_HEADER;
	}
	else
	{
		RvLogExcep(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
            "ParserInitSecurityHeader - Unknown Security Header Type"));
		return RV_ERROR_UNKNOWN;
	}

    rv = RvSipSecurityHeaderSetSecurityHeaderType(hSecurityHeader, eSecurityHeaderType);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
        "ParserInitSecurityHeader - error in RvSipSecurityHeaderSetSecurityHeaderType"));
		return rv;
    }
	
	pSecurityHeader = &pcb->securityHeader;

	/* MechanismType */
	if(pSecurityHeader->mechanism.type == PARSER_MECHANISM_TYPE_OTHER)
	{
		RvInt32 offsetName = UNDEFINED;
		
		rv= ParserAllocFromObjPage(pParserMgr,
									 &offsetName,
									 hPool,
									 hPage,
									 pSecurityHeader->mechanism.other.buf,
									 pSecurityHeader->mechanism.other.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitSecurityHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the Mechanism Type in the Security header */
		rv = SipSecurityHeaderSetMechanismType(hSecurityHeader,
											   RVSIP_SECURITY_MECHANISM_TYPE_OTHER, NULL,
											   hPool,
											   hPage,
											   offsetName);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitSecurityHeader - error in SipSecurityHeaderSetMechanismType"));
			return rv;
		}
	}
	else
	{
		RvSipSecurityMechanismType eMechanismType = RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED;
		switch(pSecurityHeader->mechanism.type)
		{
			case(PARSER_MECHANISM_TYPE_DIGEST):
				eMechanismType = RVSIP_SECURITY_MECHANISM_TYPE_DIGEST;
				break;
			case(PARSER_MECHANISM_TYPE_TLS):
				eMechanismType = RVSIP_SECURITY_MECHANISM_TYPE_TLS;
				break;
			case(PARSER_MECHANISM_TYPE_IPSEC_IKE):
				eMechanismType = RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_IKE;
				break;
			case(PARSER_MECHANISM_TYPE_IPSEC_MAN):
				eMechanismType = RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_MAN;
				break;
			case(PARSER_MECHANISM_TYPE_IPSEC_3GPP):
				eMechanismType = RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP;
				break;
            default:
                eMechanismType = RVSIP_SECURITY_MECHANISM_TYPE_UNDEFINED;
		}

		/* Set the mechanism type in the Security header */
		rv = SipSecurityHeaderSetMechanismType(hSecurityHeader,
											   eMechanismType, NULL,
											   NULL,
											   NULL,
											   UNDEFINED);

		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitSecurityHeader - error in SipSecurityHeaderSetMechanismType"));
			return rv;
		}
		
	}

	/* Check whether the Preference param has been set, if so attach
       it to the Security header  */
    if (RV_TRUE == pSecurityHeader->isPreference)
    {
        RvInt32 offsetName = UNDEFINED;
        rv= ParserAllocFromObjPage(pParserMgr,
                                     &offsetName,
                                     hPool,
                                     hPage,
                                     pSecurityHeader->preference.buf,
                                     pSecurityHeader->preference.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitSecurityHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        /* Set the preference in the Security header */
        rv = SipSecurityHeaderSetStrPreference(hSecurityHeader,
                                               NULL,
                                               hPool,
                                               hPage,
                                               offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitSecurityHeader - error in SipSecurityHeaderSetStrPreference"));
            return rv;
        }
    }

	/* Check whether the digest-algorithm param has been set, if so attach
       it to the Security header  */
    if (RV_TRUE == pSecurityHeader->isDigestAlgorithm)
    {
		RvInt32 offsetAlgorithm = UNDEFINED;

        /* Check whether the algorithm value is other than the enumerate values */
        if (RVSIP_AUTH_ALGORITHM_OTHER ==pSecurityHeader->digestAlgorithm.eAlgorithm)
        {
            /* Change the token value into a string and set it in the Security header*/
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetAlgorithm,
                                          hPool,
                                          hPage,
                                          pSecurityHeader->digestAlgorithm.algorithm.buf ,
                                          pSecurityHeader->digestAlgorithm.algorithm.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitSecurityHeader - error in ParserAllocFromObjPage"));
                return rv;
            }

        }

        rv = SipSecurityHeaderSetDigestAlgorithm(
                                                hSecurityHeader,
                                                pSecurityHeader->digestAlgorithm.eAlgorithm,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetAlgorithm);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitSecurityHeader - error in SipSecurityHeaderSetDigestAlgorithm"));
            return rv;
        }

		/* Check whether the AKAVersion param value has been set, if so attach
           it to the Security header  */
		if (RV_TRUE == pSecurityHeader->digestAlgorithm.isAKAVersion)
        {
            RvInt32	nAKAVersion;
			if(0 == pSecurityHeader->digestAlgorithm.nAKAVersion.len)
			{
				/* nAKAVersion is undefined */
				nAKAVersion = UNDEFINED;
			}
			else
			{
				rv = ParserGetINT32FromString(pParserMgr,
												pSecurityHeader->digestAlgorithm.nAKAVersion.buf ,
												pSecurityHeader->digestAlgorithm.nAKAVersion.len,
												&nAKAVersion);
				if (RV_OK != rv)
				{
					RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitSecurityHeader - error in ParserGetINT32FromString (nAKAVersion)"));
					return rv;
				}
			}

			rv = RvSipSecurityHeaderSetDigestAlgorithmAKAVersion(hSecurityHeader, nAKAVersion);
			if (RV_OK != rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitSecurityHeader - error in RvSipSecurityHeaderSetDigestAlgorithmAKAVersion"));
				return rv;
			}
        }
    }

	/* Check whether the digest-qop param has been set, if so attach
       it to the Security header  */
    if (RV_TRUE == pSecurityHeader->isDigestQop)
    {
		if(pSecurityHeader->eDigestQop == RVSIP_AUTH_QOP_OTHER)
		{
			RvInt32 offset = UNDEFINED;
			rv= ParserAllocFromObjPage(pParserMgr,
										 &offset,
										 hPool,
										 hPage,
										 pSecurityHeader->strDigestQop.buf,
										 pSecurityHeader->strDigestQop.len);
			if (RV_OK!=rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				  "ParserInitSecurityHeader - error in ParserAllocFromObjPage"));
				return rv;
			}

			/* Set the digestQop in the Security header */
			rv = SipSecurityHeaderSetStrDigestQop(hSecurityHeader,
												  pSecurityHeader->eDigestQop,
												  NULL,
												  hPool,
												  hPage,
												  offset);
			if (RV_OK != rv)
			{
				RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				  "ParserInitSecurityHeader - error in SipSecurityHeaderSetStrDigestQop"));
				return rv;
			}
		}
		else
		{
			/* Set the digestQop in the Security header */
			rv = SipSecurityHeaderSetStrDigestQop(hSecurityHeader,
												  pSecurityHeader->eDigestQop,
												  NULL,
												  NULL,
												  NULL,
												  UNDEFINED);
			if (RV_OK != rv)
			{
				RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				  "ParserInitSecurityHeader - error in SipSecurityHeaderSetStrDigestQop"));
				return rv;
			}
		}
    }

	/* Check whether the digest-verify param has been set, if so attach
       it to the Security header  */
    if (RV_TRUE == pSecurityHeader->isDigestVerify)
    {
        RvInt32 offsetName = UNDEFINED;
        rv= ParserAllocFromObjPage(pParserMgr,
                                     &offsetName,
                                     hPool,
                                     hPage,
                                     pSecurityHeader->digestVerify.buf,
                                     pSecurityHeader->digestVerify.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitSecurityHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        /* Set the digestVerify in the Security header */
        rv = SipSecurityHeaderSetStrDigestVerify(hSecurityHeader,
                                               NULL,
                                               hPool,
                                               hPage,
                                               offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitSecurityHeader - error in SipSecurityHeaderSetStrDigestVerify"));
            return rv;
        }
    }

	/* Check whether the algorithm param has been set, if so attach
       it to the Security header  */
    if (RV_TRUE == pSecurityHeader->isAlgorithm)
    {
		RvSipSecurityAlgorithmType eType;
		switch(pSecurityHeader->algorithm)
		{
			case(PARSER_SECURITY_ALGORITHM_TYPE_HMAC_MD5_96):
				eType = RVSIP_SECURITY_ALGORITHM_TYPE_HMAC_MD5_96;
				break;
			case(PARSER_SECURITY_ALGORITHM_TYPE_HMAC_SHA_1_96):
				eType = RVSIP_SECURITY_ALGORITHM_TYPE_HMAC_SHA_1_96;
				break;
            default:
                eType = RVSIP_SECURITY_ALGORITHM_TYPE_UNDEFINED;
		}
		
		/* Set the algorithm in the Security header */
        rv = RvSipSecurityHeaderSetAlgorithmType(hSecurityHeader, eType);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitSecurityHeader - error in RvSipSecurityHeaderSetAlgorithmType"));
            return rv;
        }
    }

	/* Check whether the protocol param has been set, if so attach
       it to the Security header  */
    if (RV_TRUE == pSecurityHeader->isProtocol)
    {
		RvSipSecurityProtocolType eType;
		switch(pSecurityHeader->protocol)
		{
			case(PARSER_SECURITY_PROTOCOL_TYPE_ESP):
				eType = RVSIP_SECURITY_PROTOCOL_TYPE_ESP;
				break;
			case(PARSER_SECURITY_PROTOCOL_TYPE_AH):
				eType = RVSIP_SECURITY_PROTOCOL_TYPE_AH;
				break;
            default:
                eType = RVSIP_SECURITY_PROTOCOL_TYPE_UNDEFINED;
		}
		
		/* Set the protocol in the Security header */
        rv = RvSipSecurityHeaderSetProtocolType(hSecurityHeader, eType);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitSecurityHeader - error in RvSipSecurityHeaderSetProtocolType"));
            return rv;
        }
    }

	/* Check whether the Mode param has been set, if so attach
       it to the Security header */
    if (RV_TRUE == pSecurityHeader->isMode)
    {
		RvSipSecurityModeType eType;
		switch(pSecurityHeader->mode)
		{
			case(PARSER_SECURITY_MODE_TYPE_TRANS):
				eType = RVSIP_SECURITY_MODE_TYPE_TRANS;
				break;
			case(PARSER_SECURITY_MODE_TYPE_TUN):
				eType = RVSIP_SECURITY_MODE_TYPE_TUN;
				break;
            case(PARSER_SECURITY_MODE_TYPE_UDP_ENC_TUN):
				eType = RVSIP_SECURITY_MODE_TYPE_UDP_ENC_TUN;
				break;
			default:
                eType = RVSIP_SECURITY_MODE_TYPE_UNDEFINED;
		}

        /* Set the Mode in the Security header */
        rv = RvSipSecurityHeaderSetModeType(hSecurityHeader, eType);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitSecurityHeader - error in RvSipSecurityHeaderSetModeType"));
            return rv;
        }
    }

	/* Check whether the EncryptAlgorithm param has been set, if so attach
       it to the Security header */
    if (RV_TRUE == pSecurityHeader->isEncryptAlgorithm)
    {
		RvSipSecurityEncryptAlgorithmType eType;
		switch(pSecurityHeader->encryptAlgorithm)
		{
			case(PARSER_SECURITY_ENCRYPT_ALGORITHM_TYPE_DES_EDE3_CBC):
				eType = RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_DES_EDE3_CBC;
				break;
			case(PARSER_SECURITY_ENCRYPT_ALGORITHM_TYPE_AES_CBC):
				eType = RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_AES_CBC;
				break;
			case(PARSER_SECURITY_ENCRYPT_ALGORITHM_TYPE_NULL):
				eType = RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_NULL;
				break;
            default:
                eType = RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_UNDEFINED;
		}

        /* Set the Mode in the Security header */
        rv = RvSipSecurityHeaderSetEncryptAlgorithmType(hSecurityHeader, eType);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitSecurityHeader - error in RvSipSecurityHeaderSetEncryptAlgorithmType"));
            return rv;
        }
    }

	/* Check whether spiC parameter has been set, if so attach it to
       the Security header */
	if (RV_TRUE == pSecurityHeader->isSpiC)
    {
		RvUint32 value;

		/* Change the token step into a number */
		rv = ParserGetUINT32FromString(pParserMgr, pSecurityHeader->spiC.buf,
									   pSecurityHeader->spiC.len, &value);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitSecurityHeader - error in ParserGetUINT32FromString"));
			return rv;
		}

		/* Set the spiC in the Security header */
        rv = RvSipSecurityHeaderSetSpiC(hSecurityHeader, RV_TRUE, value);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitSecurityHeader - error in RvSipSecurityHeaderSetSpiC"));
            return rv;
        }
	}
	else
	{
		/* Set the spiC in the Security header */
        rv = RvSipSecurityHeaderSetSpiC(hSecurityHeader, RV_FALSE, 0);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitSecurityHeader - error in RvSipSecurityHeaderSetSpiC"));
            return rv;
        }
	}

	/* Check whether spiS parameter has been set, if so attach it to
       the Security header */
	if (RV_TRUE == pSecurityHeader->isSpiS)
    {
		RvUint32 value;

		/* Change the token step into a number */
		rv = ParserGetUINT32FromString(pParserMgr, pSecurityHeader->spiS.buf,
									   pSecurityHeader->spiS.len, &value);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitSecurityHeader - error in ParserGetUINT32FromString"));
			return rv;
		}

		/* Set the spiS in the Security header */
        rv = RvSipSecurityHeaderSetSpiS(hSecurityHeader, RV_TRUE, value);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitSecurityHeader - error in RvSipSecurityHeaderSetSpiS"));
            return rv;
        }
	}
	else
	{
		/* Set the spiS in the Security header */
        rv = RvSipSecurityHeaderSetSpiS(hSecurityHeader, RV_FALSE, 0);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitSecurityHeader - error in RvSipSecurityHeaderSetSpiS"));
            return rv;
        }
	}

	/* Check whether there is a portC parameter, if so set it in the header */
    if (RV_TRUE == pSecurityHeader->isPortC)
    {
        RvInt32 portC;

        /* Change the token port into a number */
        rv = ParserGetINT32FromString(pParserMgr,
										 pSecurityHeader->portC.buf ,
										 pSecurityHeader->portC.len,
										 &portC);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitSecurityHeader - error in ParserGetINT32FromString"));
            return rv;
        }

        /* Set the port number into the header */
        rv = RvSipSecurityHeaderSetPortC(hSecurityHeader, portC);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitSecurityHeader - error in RvSipSecurityHeaderSetPortC"));
            return rv;
        }
	}

	/* Check whether there is a portS parameter, if so set it in the header */
    if (RV_TRUE == pSecurityHeader->isPortS)
    {
        RvInt32 portS;

        /* Change the token port into a number */
        rv = ParserGetINT32FromString(pParserMgr,
										 pSecurityHeader->portS.buf ,
										 pSecurityHeader->portS.len,
										 &portS);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitSecurityHeader - error in ParserGetINT32FromString"));
            return rv;
        }

        /* Set the port number into the header */
        rv = RvSipSecurityHeaderSetPortS(hSecurityHeader, portS);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitSecurityHeader - error in RvSipSecurityHeaderSetPortS"));
            return rv;
        }
	}

	/* Check whether Other parameter has been set, if so attach it to
       strOtherParam in the Security header*/
    if (RV_TRUE == pSecurityHeader->isOtherParams)
    {
		RvInt32 offsetOtherParam = UNDEFINED;
		offsetOtherParam = ParserAppendCopyRpoolString(
									pParserMgr,
									hPool,
									hPage,
									((ParserExtensionString *)pSecurityHeader->otherParams)->hPool,
									((ParserExtensionString *)pSecurityHeader->otherParams)->hPage,
									0,
									((ParserExtensionString *)pSecurityHeader->otherParams)->size);

		if (UNDEFINED == offsetOtherParam)
		{
			 RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitSecurityHeader - error in ParserAppendCopyRpoolString"));
			 return RV_ERROR_OUTOFRESOURCES;
		}
		rv = SipSecurityHeaderSetOtherParams(hSecurityHeader,
											 NULL,
											 hPool,
											 hPage,
											 offsetOtherParam);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitSecurityHeader - error in SipSecurityHeaderSetOtherParams"));
			return rv;
		}
	}
    
    return RV_OK;
}

/***************************************************************************
* ParserInitPProfileKeyHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          PProfileKey header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr - Pointer to the parser manager.
*             pPProfileKeyVal-	PProfileKey structure holding the PProfileKey values from the
*                          parser in a token form.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone PProfileKey or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_P_PROFILE_KEY
*                          it will be cast to RvSipPProfileKeyHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitPProfileKeyHeader(
                             IN    ParserMgr                 *pParserMgr,
                             IN    SipParser_pcb_type       *pcb,
                             IN    RvUint8                  *pcbPointer,
                             IN    SipParseType              eType,
                             INOUT const void                *pSipObject)
{
    RvStatus				rv;
    RvSipPProfileKeyHeaderHandle	hPProfileKeyHeader;
    RvSipAddressHandle      hAddress;
    HRPOOL                  hPool;
    HPAGE                   hPage;
    RvSipAddressType        eAddrType;
	ParserPProfileKeyHeader		*pPProfileKeyHeader;

    if (SIP_PARSETYPE_P_PROFILE_KEY == eType)
    {
        /* The parser is used to parse PProfileKey header */
        hPProfileKeyHeader=(RvSipPProfileKeyHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_P_PROFILE_KEY, 
                                   pcbPointer, (void**)&hPProfileKeyHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitPProfileKeyHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipPProfileKeyHeaderGetPool(hPProfileKeyHeader);
    hPage = SipPProfileKeyHeaderGetPage(hPProfileKeyHeader);

	pPProfileKeyHeader = &pcb->pprofileKeyHeader;
    eAddrType = ParserAddrType2MsgAddrType(pPProfileKeyHeader->nameAddr.exUri.uriType);
    if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitPProfileKeyHeader - unknown parserAddressType %d", pPProfileKeyHeader->nameAddr.exUri.uriType));
        return RV_ERROR_UNKNOWN;
    }

    /* Construct the url in the PProfileKey header */
    rv = RvSipAddrConstructInPProfileKeyHeader(hPProfileKeyHeader,
                                              eAddrType,
                                              &hAddress);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitPProfileKeyHeader - error in RvSipAddrConstructInPProfileKeyHeader"));
        return rv;
    }

    /* Initialize the sip-url\abs-uri values from the parser*/
    rv = ParserInitAddressInHeader(pParserMgr,
                                      &pPProfileKeyHeader->nameAddr.exUri,
                                      hPool, hPage, eAddrType, hAddress);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitPProfileKeyHeader - error in ParserInitAddressInHeader"));
        return rv;
    }

	/* Check whether the display name has been set, if so attach
       it to the PProfileKey header */
    if (RV_TRUE == pPProfileKeyHeader->nameAddr.isDisplayName)
    {
        RvInt32 offsetName = UNDEFINED;
        rv= ParserAllocFromObjPage(pParserMgr,
                                     &offsetName,
                                     hPool,
                                     hPage,
                                     pPProfileKeyHeader->nameAddr.name.buf,
                                     pPProfileKeyHeader->nameAddr.name.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitPProfileKeyHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        /* Set the display name in the PProfileKey header */
        rv = SipPProfileKeyHeaderSetDisplayName(hPProfileKeyHeader,
                                               NULL,
                                               hPool,
                                               hPage,
                                               offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitPProfileKeyHeader - error in PProfileKeyHeaderSetDisplayName"));
            return rv;
        }
    }

	/* Check whether Other parameter has been set, if so attach it to
       strOtherParam in the PProfileKey header */
    if (RV_TRUE == pPProfileKeyHeader->isOtherParams)
    {
		RvInt32 offsetOtherParam = UNDEFINED;
		offsetOtherParam = ParserAppendCopyRpoolString(
														pParserMgr,
														hPool,
														hPage,
														((ParserExtensionString *)pPProfileKeyHeader->otherParams)->hPool,
														((ParserExtensionString *)pPProfileKeyHeader->otherParams)->hPage,
														0,
														((ParserExtensionString *)pPProfileKeyHeader->otherParams)->size);

		if (UNDEFINED == offsetOtherParam)
		{
			 RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPProfileKeyHeader - error in ParserAppendCopyRpoolString"));
			 return RV_ERROR_OUTOFRESOURCES;
		}
		rv = SipPProfileKeyHeaderSetOtherParams(hPProfileKeyHeader,
											 NULL,
											 hPool,
											 hPage,
											 offsetOtherParam);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPProfileKeyHeader - error in SipPProfileKeyHeaderSetOtherParams"));
			return rv;
		}
	}
    
    return RV_OK;
}

/***************************************************************************
* ParserInitPUserDatabaseHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          PUserDatabase header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr - Pointer to the parser manager.
*             pPUserDatabaseVal-	PUserDatabase structure holding the PUserDatabase values from the
*                          parser in a token form.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone PUserDatabase or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_P_USER_DATABASE
*                          it will be cast to RvSipPUserDatabaseHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitPUserDatabaseHeader(
                             IN    ParserMgr                 *pParserMgr,
                             IN    SipParser_pcb_type       *pcb,
                             IN    RvUint8                  *pcbPointer,
                             IN    SipParseType              eType,
                             INOUT const void                *pSipObject)
{
    RvStatus				rv;
    RvSipPUserDatabaseHeaderHandle	hPUserDatabaseHeader;
    RvSipAddressHandle      hAddress;
    HRPOOL                  hPool;
    HPAGE                   hPage;
    RvSipAddressType        eAddrType;
	ParserPUserDatabaseHeader		*pPUserDatabaseHeader;

    if (SIP_PARSETYPE_P_USER_DATABASE == eType)
    {
        /* The parser is used to parse PUserDatabase header */
        hPUserDatabaseHeader=(RvSipPUserDatabaseHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_P_USER_DATABASE, 
                                   pcbPointer, (void**)&hPUserDatabaseHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitPUserDatabaseHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipPUserDatabaseHeaderGetPool(hPUserDatabaseHeader);
    hPage = SipPUserDatabaseHeaderGetPage(hPUserDatabaseHeader);

	pPUserDatabaseHeader = &pcb->puserDatabaseHeader;
    eAddrType = ParserAddrType2MsgAddrType(pPUserDatabaseHeader->exUri.uriType);
    if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitPUserDatabaseHeader - unknown parserAddressType %d", pPUserDatabaseHeader->exUri.uriType));
        return RV_ERROR_UNKNOWN;
    }

    /* Construct the url in the PUserDatabase header */
    rv = RvSipAddrConstructInPUserDatabaseHeader(hPUserDatabaseHeader,
                                              eAddrType,
                                              &hAddress);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitPUserDatabaseHeader - error in RvSipAddrConstructInPUserDatabaseHeader"));
        return rv;
    }

    /* Initialize the diameter-uri values from the parser */
    rv = ParserInitAddressInHeader(pParserMgr,
                                      &pPUserDatabaseHeader->exUri,
                                      hPool, hPage, eAddrType, hAddress);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitPUserDatabaseHeader - error in ParserInitAddressInHeader"));
        return rv;
    }
    
    return RV_OK;
}

/***************************************************************************
* ParserInitPAnswerStateHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          PAnswerState header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:   pParserMgr - Pointer to the parser manager.
*             pcb		 - PAnswerState structure holding the PAnswerState 
*						   values from the parser in a token form.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone PAnswerState or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_P_ANSWER_STATE
*                          it will be cast to RvSipPAnswerStateHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitPAnswerStateHeader(
                             IN    ParserMgr                *pParserMgr,
                             IN    SipParser_pcb_type       *pcb,
                             IN    RvUint8                  *pcbPointer,
                             IN    SipParseType              eType,
                             INOUT const void               *pSipObject)
{
    RvStatus			    		rv;
	RvInt32				    		offsetName = UNDEFINED;
    RvSipPAnswerStateHeaderHandle	hHeader;
    HRPOOL          				hPool;
    HPAGE	                        hPage;
	ParserPAnswerStateHeader       *pHeader;
	

    if (SIP_PARSETYPE_P_ANSWER_STATE == eType)
    {
        /* The parser is used to parse PAnswerState header */
        hHeader=(RvSipPAnswerStateHeaderHandle)pSipObject;
    }
    else
    {

        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_P_ANSWER_STATE, 
                                   pcbPointer, (void**)&hHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitPAnswerStateHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipPAnswerStateHeaderGetPool(hHeader);
    hPage = SipPAnswerStateHeaderGetPage(hHeader);

	pHeader = &pcb->panswerStateHeader;
    
    /* answerType */
	if(pHeader->answerType.type == PARSER_ANSWER_TYPE_OTHER)
	{
		rv= ParserAllocFromObjPage(pParserMgr,
									 &offsetName,
									 hPool,
									 hPage,
									 pHeader->answerType.other.buf,
									 pHeader->answerType.other.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPAnswerStateHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the answer Type in the PAnswerState header */
		rv = SipPAnswerStateHeaderSetAnswerType(hHeader,
											   RVSIP_ANSWER_TYPE_OTHER, NULL,
											   hPool,
											   hPage,
											   offsetName);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPAnswerStateHeader - error in SipPAnswerStateHeaderSetAnswerType"));
			return rv;
		}
	}
	else
	{
		RvSipPAnswerStateAnswerType eAnswerType = RVSIP_ANSWER_TYPE_UNDEFINED;
		switch(pHeader->answerType.type)
		{
			case(PARSER_ANSWER_TYPE_CONFIRMED):
				eAnswerType = RVSIP_ANSWER_TYPE_CONFIRMED;
				break;
			case(PARSER_ANSWER_TYPE_UNCONFIRMED):
				eAnswerType = RVSIP_ANSWER_TYPE_UNCONFIRMED;
				break;
            default:
                eAnswerType = RVSIP_ANSWER_TYPE_UNDEFINED;
		}

		/* Set the answer Type in the PAnswerState header */
		rv = SipPAnswerStateHeaderSetAnswerType(hHeader,
											   eAnswerType, NULL,
											   NULL,
											   NULL,
											   UNDEFINED);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPAnswerStateHeader - error in SipPAnswerStateHeaderSetAnswerType"));
			return rv;
		}
	}

	/* Check whether Other parameter has been set, if so attach it to
       strOtherParam in the PAnswerState header */
    if (RV_TRUE == pHeader->isOtherParams)
    {
		RvInt32 offsetOtherParam = UNDEFINED;
		offsetOtherParam = ParserAppendCopyRpoolString(
														pParserMgr,
														hPool,
														hPage,
														((ParserExtensionString *)pHeader->otherParams)->hPool,
														((ParserExtensionString *)pHeader->otherParams)->hPage,
														0,
														((ParserExtensionString *)pHeader->otherParams)->size);

		if (UNDEFINED == offsetOtherParam)
		{
			 RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPAnswerStateHeader - error in ParserAppendCopyRpoolString"));
			 return RV_ERROR_OUTOFRESOURCES;
		}
		rv = SipPAnswerStateHeaderSetOtherParams(hHeader,
											 NULL,
											 hPool,
											 hPage,
											 offsetOtherParam);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPAnswerStateHeader - error in SipPAnswerStateHeaderSetOtherParams"));
			return rv;
		}
	}
    
    return RV_OK;
}

/***************************************************************************
* ParserInitPServedUserHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          PServedUser header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr - Pointer to the parser manager.
*             pPServedUserVal-	PServedUser structure holding the PServedUser values from the
*                          parser in a token form.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone PServedUser or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_P_SERVED_USER
*                          it will be cast to RvSipPServedUserHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitPServedUserHeader(
                             IN    ParserMgr                 *pParserMgr,
                             IN    SipParser_pcb_type       *pcb,
                             IN    RvUint8                  *pcbPointer,
                             IN    SipParseType              eType,
                             INOUT const void                *pSipObject)
{
    RvStatus				        rv;
    RvSipPServedUserHeaderHandle	hPServedUserHeader;
    RvSipAddressHandle              hAddress;
    HRPOOL                          hPool;
    HPAGE                           hPage;
    RvSipAddressType                eAddrType;
	ParserPServedUserHeader		   *pPServedUserHeader;

    if (SIP_PARSETYPE_P_SERVED_USER == eType)
    {
        /* The parser is used to parse PServedUser header */
        hPServedUserHeader=(RvSipPServedUserHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_P_SERVED_USER, 
                                   pcbPointer, (void**)&hPServedUserHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitPServedUserHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipPServedUserHeaderGetPool(hPServedUserHeader);
    hPage = SipPServedUserHeaderGetPage(hPServedUserHeader);

	pPServedUserHeader = &pcb->pservedUserHeader;
    eAddrType = ParserAddrType2MsgAddrType(pPServedUserHeader->nameAddr.exUri.uriType);
    if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitPServedUserHeader - unknown parserAddressType %d", pPServedUserHeader->nameAddr.exUri.uriType));
        return RV_ERROR_UNKNOWN;
    }

    /* Construct the url in the PServedUser header */
    rv = RvSipAddrConstructInPServedUserHeader(hPServedUserHeader,
                                              eAddrType,
                                              &hAddress);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitPServedUserHeader - error in RvSipAddrConstructInPServedUserHeader"));
        return rv;
    }

    /* Initialize the sip-url\abs-uri values from the parser*/
    rv = ParserInitAddressInHeader(pParserMgr,
                                      &pPServedUserHeader->nameAddr.exUri,
                                      hPool, hPage, eAddrType, hAddress);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitPServedUserHeader - error in ParserInitAddressInHeader"));
        return rv;
    }

	/* Check whether the display name has been set, if so attach
       it to the PServedUser header */
    if (RV_TRUE == pPServedUserHeader->nameAddr.isDisplayName)
    {
        RvInt32 offsetName = UNDEFINED;
        rv= ParserAllocFromObjPage(pParserMgr,
                                     &offsetName,
                                     hPool,
                                     hPage,
                                     pPServedUserHeader->nameAddr.name.buf,
                                     pPServedUserHeader->nameAddr.name.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitPServedUserHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        /* Set the display name in the PServedUser header */
        rv = SipPServedUserHeaderSetDisplayName(hPServedUserHeader,
                                               NULL,
                                               hPool,
                                               hPage,
                                               offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitPServedUserHeader - error in PServedUserHeaderSetDisplayName"));
            return rv;
        }
    }

    /* Check whether the Session Case param has been set, if so attach
       it to the PServedUser header  */
    if (RV_TRUE == pPServedUserHeader->isSessionCase)
    {
		RvSipPServedUserSessionCaseType eType;
		switch(pPServedUserHeader->sessionCase)
		{
			case(PARSER_SESSION_CASE_TYPE_ORIG):
				eType = RVSIP_SESSION_CASE_TYPE_ORIG;
				break;
			case(PARSER_SESSION_CASE_TYPE_TERM):
				eType = RVSIP_SESSION_CASE_TYPE_TERM;
				break;
            default:
                eType = RVSIP_SESSION_CASE_TYPE_UNDEFINED;
		}
		
		/* Set the Session Case in the PServedUser header */
        rv = RvSipPServedUserHeaderSetSessionCaseType(hPServedUserHeader, eType);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitPServedUserHeader - error in RvSipPServedUserHeaderSetSessionCaseType"));
            return rv;
        }
    }    

    /* Check whether the RegistrationState param has been set, if so attach
       it to the PServedUser header  */
    if (RV_TRUE == pPServedUserHeader->isRegistrationState)
    {
		RvSipPServedUserRegistrationStateType eType;
		switch(pPServedUserHeader->registrationState)
		{
			case(PARSER_REGISTRATION_STATE_TYPE_UNREG):
				eType = RVSIP_REGISTRATION_STATE_TYPE_UNREG;
				break;
			case(PARSER_REGISTRATION_STATE_TYPE_REG):
				eType = RVSIP_REGISTRATION_STATE_TYPE_REG;
				break;
            default:
                eType = RVSIP_REGISTRATION_STATE_TYPE_UNDEFINED;
		}
		
		/* Set the RegistrationState in the PServedUser header */
        rv = RvSipPServedUserHeaderSetRegistrationStateType(hPServedUserHeader, eType);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitPServedUserHeader - error in RvSipPServedUserHeaderSetRegistrationStateType"));
            return rv;
        }
    }    

	/* Check whether Other parameter has been set, if so attach it to
       strOtherParam in the PServedUser header */
    if (RV_TRUE == pPServedUserHeader->isOtherParams)
    {
		RvInt32 offsetOtherParam = UNDEFINED;
		offsetOtherParam = ParserAppendCopyRpoolString(
														pParserMgr,
														hPool,
														hPage,
														((ParserExtensionString *)pPServedUserHeader->otherParams)->hPool,
														((ParserExtensionString *)pPServedUserHeader->otherParams)->hPage,
														0,
														((ParserExtensionString *)pPServedUserHeader->otherParams)->size);

		if (UNDEFINED == offsetOtherParam)
		{
			 RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPServedUserHeader - error in ParserAppendCopyRpoolString"));
			 return RV_ERROR_OUTOFRESOURCES;
		}
		rv = SipPServedUserHeaderSetOtherParams(hPServedUserHeader,
											 NULL,
											 hPool,
											 hPage,
											 offsetOtherParam);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPServedUserHeader - error in SipPServedUserHeaderSetOtherParams"));
			return rv;
		}
	}
    
    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
/***************************************************************************
* ParserInitPDCSTracePartyIDHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          PDCSTracePartyID header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:    pParserMgr - Pointer to the parser manager.
*             pPDCSTracePartyIDVal-	PDCSTracePartyID structure holding the 
*						   PDCSTracePartyID values from the parser in a token form.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone PDCSTracePartyID or sip message.
*			eWhichHeader - This Type indicates which of the subtypes of PDCSTracePartyID
*						   Header the header is.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_P_DCS_TRACE_PARTY_ID
*                          it will be cast to RvSipPDCSTracePartyIDHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitPDCSTracePartyIDHeader(
                             IN    ParserMgr                *pParserMgr,
                             IN    SipParser_pcb_type       *pcb,
                             IN    RvUint8                  *pcbPointer,
                             IN    SipParseType              eType,
                             INOUT const void               *pSipObject)
{
    RvStatus							rv;
    RvSipPDCSTracePartyIDHeaderHandle	hHeader;
    RvSipAddressHandle					hAddress;
    HRPOOL								hPool;
    HPAGE								hPage;
    RvSipAddressType					eAddrType;
	ParserPDCSTracePartyIDHeader		*pPDCSTracePartyIDHeader;

    if (SIP_PARSETYPE_P_DCS_TRACE_PARTY_ID == eType)
    {
        /*The parser is used to parse PDCSTracePartyID header */
        hHeader=(RvSipPDCSTracePartyIDHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_P_DCS_TRACE_PARTY_ID, 
                                   pcbPointer, (void**)&hHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitPDCSTracePartyIDHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipPDCSTracePartyIDHeaderGetPool(hHeader);
    hPage = SipPDCSTracePartyIDHeaderGetPage(hHeader);

	pPDCSTracePartyIDHeader = &pcb->pdcsTracePartyIDHeader;
    eAddrType = ParserAddrType2MsgAddrType(pPDCSTracePartyIDHeader->nameAddr.exUri.uriType);
    if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitPDCSTracePartyIDHeader - unknown parserAddressType %d", pPDCSTracePartyIDHeader->nameAddr.exUri.uriType));
        return RV_ERROR_UNKNOWN;
    }

    /* Construct the url in the PDCSTracePartyID header */
    rv = RvSipAddrConstructInPDCSTracePartyIDHeader(hHeader,
													  eAddrType,
													  &hAddress);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitPDCSTracePartyIDHeader - error in RvSipAddrConstructInPDCSTracePartyIDHeader"));
        return rv;
    }

    /* Initialize the sip-url\abs-uri values from the parser*/
    rv = ParserInitAddressInHeader(pParserMgr,
                                      &pPDCSTracePartyIDHeader->nameAddr.exUri,
                                      hPool, hPage, eAddrType, hAddress);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitPDCSTracePartyIDHeader - error in ParserInitAddressInHeader"));
        return rv;
    }

	/* Check whether the display name has been set, if so attach
       it to the PDCSTracePartyID header  */
    if (RV_TRUE == pPDCSTracePartyIDHeader->nameAddr.isDisplayName)
    {
        RvInt32 offsetName = UNDEFINED;
        rv= ParserAllocFromObjPage(pParserMgr,
                                     &offsetName,
                                     hPool,
                                     hPage,
                                     pPDCSTracePartyIDHeader->nameAddr.name.buf,
                                     pPDCSTracePartyIDHeader->nameAddr.name.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitPDCSTracePartyIDHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        /* Set the display name in the PDCSTracePartyID header */
        rv = SipPDCSTracePartyIDHeaderSetDisplayName(hHeader,
                                               NULL,
                                               hPool,
                                               hPage,
                                               offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitPDCSTracePartyIDHeader - error in PDCSTracePartyIDHeaderSetDisplayName"));
            return rv;
        }
    }

    return RV_OK;
}

/***************************************************************************
* ParserInitPDCSOSPSHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          PDCSOSPS header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:   pParserMgr - Pointer to the parser manager.
*             pcb		 - PDCSOSPS structure holding the  
*						   values from the parser in a token form.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone PMediaAuthorization or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_P_DCS_OSPS
*                          it will be cast to RvSipPDCSOSPSHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitPDCSOSPSHeader(
                             IN    ParserMgr                *pParserMgr,
                             IN    SipParser_pcb_type       *pcb,
                             IN    RvUint8                  *pcbPointer,
                             IN    SipParseType              eType,
                             INOUT const void               *pSipObject)
{
	RvStatus					rv;
	RvInt32						offsetName = UNDEFINED;
    RvSipPDCSOSPSHeaderHandle	hHeader;
    HRPOOL						hPool;
    HPAGE						hPage;
	ParserPDCSOSPSHeader		*pHeader;
	
	rv = RV_OK;

    if (SIP_PARSETYPE_P_DCS_OSPS == eType)
    {
        /*The parser is used to parse PDCSOSPS header */
        hHeader = (RvSipPDCSOSPSHeaderHandle)pSipObject;
    }
    else
    {
        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_P_DCS_OSPS, 
                                   pcbPointer, (void**)&hHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitPDCSOSPSHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipPDCSOSPSHeaderGetPool(hHeader);
    hPage = SipPDCSOSPSHeaderGetPage(hHeader);

	pHeader = &pcb->pdcsOSPSHeader;
    
    /* OSPS Tag */
	if(pHeader->tag.type == PARSER_OSPS_TAG_TYPE_OTHER)
	{
		rv= ParserAllocFromObjPage(pParserMgr,
									 &offsetName,
									 hPool,
									 hPage,
									 pHeader->tag.other.buf,
									 pHeader->tag.other.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPDCSOSPSHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the OSPS Tag in the PDCSOSPS header */
		rv = SipPDCSOSPSHeaderSetOSPSTag(hHeader,
											   RVSIP_P_DCS_OSPS_TAG_TYPE_OTHER, NULL,
											   hPool,
											   hPage,
											   offsetName);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPDCSOSPSHeader - error in SipPDCSOSPSHeaderSetOSPSTag"));
			return rv;
		}
	}
	else
	{
		RvSipOSPSTagType eOSPSTag = RVSIP_P_DCS_OSPS_TAG_TYPE_UNDEFINED;
		switch(pHeader->tag.type)
		{
			case(PARSER_OSPS_TAG_TYPE_BLV):
				eOSPSTag = RVSIP_P_DCS_OSPS_TAG_TYPE_BLV;
				break;
			case(PARSER_OSPS_TAG_TYPE_EI):
				eOSPSTag = RVSIP_P_DCS_OSPS_TAG_TYPE_EI;
				break;
			case(PARSER_OSPS_TAG_TYPE_RING):
				eOSPSTag = RVSIP_P_DCS_OSPS_TAG_TYPE_RING;
				break;
            default:
                eOSPSTag = RVSIP_P_DCS_OSPS_TAG_TYPE_UNDEFINED;
		}

		/* Set the OSPS Tag in the PDCSOSPS header */
		rv = SipPDCSOSPSHeaderSetOSPSTag(hHeader,
										   eOSPSTag, NULL,
										   NULL,
										   NULL,
										   UNDEFINED);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPDCSOSPSHeader - error in SipPDCSOSPSHeaderSetOSPSTag"));
			return rv;
		}
	}

    return RV_OK;
}

/***************************************************************************
* ParserInitPDCSBillingInfoHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          PDCSBillingInfo header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:   pParserMgr - Pointer to the parser manager.
*             pcb		 - PDCSBillingInfo structure holding the PDCSBillingInfo 
*						   values from the parser in a token form.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone PDCSBillingInfo or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_P_DCS_BILLING_INFO
*                          it will be cast to RvSipPDCSBillingInfoHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitPDCSBillingInfoHeader(
                             IN    ParserMgr                *pParserMgr,
                             IN    SipParser_pcb_type       *pcb,
                             IN    RvUint8                  *pcbPointer,
                             IN    SipParseType              eType,
                             INOUT const void               *pSipObject)
{
    RvStatus							rv;
    RvSipPDCSBillingInfoHeaderHandle	hHeader;
    HRPOOL								hPool;
    HPAGE								hPage;
	ParserPDCSBillingInfoHeader			*pHeader;
	RvSipAddressType					eAddrType;
	RvSipAddressHandle					hAddress;
	RvInt32								offsetBillingCorrelationID = UNDEFINED;
	RvInt32								offsetFEID		= UNDEFINED;
	RvInt32								offsetFEIDHost	= UNDEFINED;

    if (SIP_PARSETYPE_P_DCS_BILLING_INFO == eType)
    {
        /*The parser is used to parse PDCSBillingInfo header */
        hHeader=(RvSipPDCSBillingInfoHeaderHandle)pSipObject;
    }
    else
    {

        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_P_DCS_BILLING_INFO, 
                                   pcbPointer, (void**)&hHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitPDCSBillingInfoHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipPDCSBillingInfoHeaderGetPool(hHeader);
    hPage = SipPDCSBillingInfoHeaderGetPage(hHeader);

	pHeader = &pcb->pdcsBillingInfoHeader;

	/* BillingCorrelationID */
	rv= ParserAllocFromObjPage(pParserMgr,
								 &offsetBillingCorrelationID,
								 hPool,
								 hPage,
								 pHeader->billingCorrelationID.buf,
								 pHeader->billingCorrelationID.len);
	if (RV_OK!=rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
		  "ParserInitPDCSBillingInfoHeader - error in ParserAllocFromObjPage"));
		return rv;
	}

	/* Set the BillingCorrelationID in the PDCSBillingInfo header */
	rv = SipPDCSBillingInfoHeaderSetStrBillingCorrelationID(hHeader,
												   NULL,
												   hPool,
												   hPage,
												   offsetBillingCorrelationID);
	if (RV_OK != rv)
	{
		RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
		  "ParserInitPDCSBillingInfoHeader - error in SipPDCSBillingInfoHeaderSetStrBillingCorrelationID"));
		return rv;
	}


	/* FEID */
	rv= ParserAllocFromObjPage(pParserMgr,
								 &offsetFEID,
								 hPool,
								 hPage,
								 pHeader->feid.buf,
								 pHeader->feid.len);
	if (RV_OK!=rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
		  "ParserInitPDCSBillingInfoHeader - error in ParserAllocFromObjPage"));
		return rv;
	}

	/* Set the FEID parameter in the PDCSBillingInfo header */
	rv = SipPDCSBillingInfoHeaderSetStrFEID(hHeader,
													 NULL,
													 hPool,
													 hPage,
													 offsetFEID);
	if (RV_OK != rv)
	{
		RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
		  "ParserInitPDCSBillingInfoHeader - error in SipPDCSBillingInfoHeaderSetStrFEID"));
		return rv;
	}

	/* FEIDHost */
	rv= ParserAllocFromObjPage(pParserMgr,
								 &offsetFEIDHost,
								 hPool,
								 hPage,
								 pHeader->feidHost.buf,
								 pHeader->feidHost.len);
	if (RV_OK!=rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
		  "ParserInitPDCSBillingInfoHeader - error in ParserAllocFromObjPage"));
		return rv;
	}

	/* Set the FEIDHost parameter in the PDCSBillingInfo header */
	rv = SipPDCSBillingInfoHeaderSetStrFEIDHost(hHeader,
												 NULL,
												 hPool,
												 hPage,
												 offsetFEIDHost);
	if (RV_OK != rv)
	{
		RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
		  "ParserInitPDCSBillingInfoHeader - error in SipPDCSBillingInfoHeaderSetStrFEIDHost"));
		return rv;
	}

	/* Check whether RKSGroupID parameter has been set, if so attach it to
       RKSGroupID parameter in the PDCSBillingInfo header */
    if (RV_TRUE == pHeader->isRKSGroupID)
    {
		RvInt32 offsetRKSGroupID = UNDEFINED;
		rv= ParserAllocFromObjPage(pParserMgr,
									 &offsetRKSGroupID,
									 hPool,
									 hPage,
									 pHeader->rksGroupID.buf,
									 pHeader->rksGroupID.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPDCSBillingInfoHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the RKSGroupID parameter in the PDCSBillingInfo header */
		rv = SipPDCSBillingInfoHeaderSetStrRKSGroupID(hHeader,
													 NULL,
													 hPool,
													 hPage,
													 offsetRKSGroupID);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPDCSBillingInfoHeader - error in SipPDCSBillingInfoHeaderSetStrRKSGroupID"));
			return rv;
		}
	}

	/* ChargeAddr */
	if(RV_TRUE == pHeader->isChargeUri)
	{
		eAddrType = ParserAddrType2MsgAddrType(pHeader->chargeUri.uriType);
		if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitPDCSBillingInfoHeader - unknown parserAddressType %d", pHeader->chargeUri.uriType));
			return RV_ERROR_UNKNOWN;
		}

		/* Construct the url in the PDCSBillingInfo header */
		rv = RvSipAddrConstructInPDCSBillingInfoHeader(hHeader,
												  eAddrType, 
												  RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CHARGE,
												  &hAddress);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				  "ParserInitPDCSBillingInfoHeader - error in RvSipAddrConstructInPDCSBillingInfoHeader"));
			return rv;
		}

		/* Initialize the sip-url\abs-uri values from the parser*/
		rv = ParserInitAddressInHeader(pParserMgr,
										  &pHeader->chargeUri,
										  hPool, hPage, eAddrType, hAddress);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				  "ParserInitPDCSBillingInfoHeader - error in ParserInitAddressInHeader"));
			return rv;
		}
	}

	/* CallingAddr */
	if(RV_TRUE == pHeader->isCallingUri)
	{
		eAddrType = ParserAddrType2MsgAddrType(pHeader->callingUri.uriType);
		if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitPDCSBillingInfoHeader - unknown parserAddressType %d", pHeader->callingUri.uriType));
			return RV_ERROR_UNKNOWN;
		}

		/* Construct the url in the PDCSBillingInfo header */
		rv = RvSipAddrConstructInPDCSBillingInfoHeader(hHeader,
												  eAddrType, 
												  RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CALLING,
												  &hAddress);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				  "ParserInitPDCSBillingInfoHeader - error in RvSipAddrConstructInPDCSBillingInfoHeader"));
			return rv;
		}

		/* Initialize the sip-url\abs-uri values from the parser*/
		rv = ParserInitAddressInHeader(pParserMgr,
										  &pHeader->callingUri,
										  hPool, hPage, eAddrType, hAddress);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				  "ParserInitPDCSBillingInfoHeader - error in ParserInitAddressInHeader"));
			return rv;
		}
	}

	/* CalledAddr */
	if(RV_TRUE == pHeader->isCalledUri)
	{
		eAddrType = ParserAddrType2MsgAddrType(pHeader->calledUri.uriType);
		if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitPDCSBillingInfoHeader - unknown parserAddressType %d", pHeader->calledUri.uriType));
			return RV_ERROR_UNKNOWN;
		}

		/* Construct the url in the PDCSBillingInfo header */
		rv = RvSipAddrConstructInPDCSBillingInfoHeader(hHeader,
												  eAddrType, 
												  RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_CALLED,
												  &hAddress);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				  "ParserInitPDCSBillingInfoHeader - error in RvSipAddrConstructInPDCSBillingInfoHeader"));
			return rv;
		}

		/* Initialize the sip-url\abs-uri values from the parser*/
		rv = ParserInitAddressInHeader(pParserMgr,
										  &pHeader->calledUri,
										  hPool, hPage, eAddrType, hAddress);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				  "ParserInitPDCSBillingInfoHeader - error in ParserInitAddressInHeader"));
			return rv;
		}
	}

	/* RoutingAddr */
	if(RV_TRUE == pHeader->isRoutingUri)
	{	
		eAddrType = ParserAddrType2MsgAddrType(pHeader->routingUri.uriType);
		if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitPDCSBillingInfoHeader - unknown parserAddressType %d", pHeader->routingUri.uriType));
			return RV_ERROR_UNKNOWN;
		}

		/* Construct the url in the PDCSBillingInfo header */
		rv = RvSipAddrConstructInPDCSBillingInfoHeader(hHeader,
												  eAddrType, 
												  RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_ROUTING,
												  &hAddress);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				  "ParserInitPDCSBillingInfoHeader - error in RvSipAddrConstructInPDCSBillingInfoHeader"));
			return rv;
		}

		/* Initialize the sip-url\abs-uri values from the parser*/
		rv = ParserInitAddressInHeader(pParserMgr,
										  &pHeader->routingUri,
										  hPool, hPage, eAddrType, hAddress);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				  "ParserInitPDCSBillingInfoHeader - error in ParserInitAddressInHeader"));
			return rv;
		}
	}

	/* LocRoutingAddr */
	if(RV_TRUE == pHeader->isLocRoutingUri)
	{
		eAddrType = ParserAddrType2MsgAddrType(pHeader->locRoutingUri.uriType);
		if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitPDCSBillingInfoHeader - unknown parserAddressType %d", pHeader->locRoutingUri.uriType));
			return RV_ERROR_UNKNOWN;
		}

		/* Construct the url in the PDCSBillingInfo header */
		rv = RvSipAddrConstructInPDCSBillingInfoHeader(hHeader,
												  eAddrType, 
												  RVSIP_P_DCS_BILLING_INFO_ADDRESS_TYPE_LOC_ROUTE,
												  &hAddress);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				  "ParserInitPDCSBillingInfoHeader - error in RvSipAddrConstructInPDCSBillingInfoHeader"));
			return rv;
		}

		/* Initialize the sip-url\abs-uri values from the parser*/
		rv = ParserInitAddressInHeader(pParserMgr,
										  &pHeader->locRoutingUri,
										  hPool, hPage, eAddrType, hAddress);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				  "ParserInitPDCSBillingInfoHeader - error in ParserInitAddressInHeader"));
			return rv;
		}
	}
	
	/* Check whether Other parameter has been set, if so attach it to
       strOtherParam in the PChargingVector header */
    if (RV_TRUE == pHeader->isOtherParams)
    {
		RvInt32 offsetOtherParam = UNDEFINED;
		offsetOtherParam = ParserAppendCopyRpoolString(
														pParserMgr,
														hPool,
														hPage,
														((ParserExtensionString *)pHeader->otherParams)->hPool,
														((ParserExtensionString *)pHeader->otherParams)->hPage,
														0,
														((ParserExtensionString *)pHeader->otherParams)->size);

		if (UNDEFINED == offsetOtherParam)
		{
			 RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPDCSBillingInfoHeader - error in ParserAppendCopyRpoolString"));
			 return RV_ERROR_OUTOFRESOURCES;
		}
		rv = SipPDCSBillingInfoHeaderSetOtherParams(hHeader,
													 NULL,
													 hPool,
													 hPage,
													 offsetOtherParam);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPDCSBillingInfoHeader - error in SipPDCSBillingInfoHeaderSetOtherParams"));
			return rv;
		}
	}
    
    return RV_OK;
}    

/***************************************************************************
* ParserInitPDCSLAESHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          PDCSLAES header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:   pParserMgr - Pointer to the parser manager.
*             pcb		 - PDCSLAES structure holding the  
*						   values from the parser in a token form.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone PDCSLAES or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_P_DCS_LAES
*                          it will be cast to RvSipPDCSLAESHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitPDCSLAESHeader(
                             IN    ParserMgr                *pParserMgr,
                             IN    SipParser_pcb_type       *pcb,
                             IN    RvUint8                  *pcbPointer,
                             IN    SipParseType              eType,
                             INOUT const void               *pSipObject)
{
    RvStatus						rv;
    RvSipPDCSLAESHeaderHandle		hHeader;
    HRPOOL							hPool;
    HPAGE							hPage;
	ParserPDCSLAESHeader			*pHeader;
	RvInt32							offsetLaesSigHost = UNDEFINED;

    if (SIP_PARSETYPE_P_DCS_LAES == eType)
    {
        /* The parser is used to parse PDCSLAES header */
        hHeader=(RvSipPDCSLAESHeaderHandle)pSipObject;
    }
    else
    {

        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_P_DCS_LAES, 
                                   pcbPointer, (void**)&hHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitPDCSLAESHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipPDCSLAESHeaderGetPool(hHeader);
    hPage = SipPDCSLAESHeaderGetPage(hHeader);

	pHeader = &pcb->pdcsLAESHeader;
    
	/* laesSigHost */
	rv= ParserAllocFromObjPage(pParserMgr,
								 &offsetLaesSigHost,
								 hPool,
								 hPage,
								 pHeader->laesSigHost.buf,
								 pHeader->laesSigHost.len);
	if (RV_OK!=rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
		  "ParserInitPDCSLAESHeader - error in ParserAllocFromObjPage"));
		return rv;
	}

	/* Set the laesSigHost in the PDCSLAES header */
	rv = SipPDCSLAESHeaderSetStrLaesSigHost(hHeader,
												   NULL,
												   hPool,
												   hPage,
												   offsetLaesSigHost);
	if (RV_OK != rv)
	{
		RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
		  "ParserInitPDCSLAESHeader - error in SipPDCSLAESHeaderSetStrLaesSigHost"));
		return rv;
	}


	/* Check whether LaesSigPort parameter has been set, if so attach it to
       LaesSigPort parameter in the PDCSLAES header */
    if (RV_TRUE == pHeader->isLaesSigPort)
    {
		RvInt32 port;
		
        /* Change the token port into a number */
        rv = ParserGetINT32FromString(pParserMgr,
										 pHeader->laesSigPort.buf ,
										 pHeader->laesSigPort.len,
										 &port);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitPDCSLAESHeader - error in ParserGetINT32FromString"));
            return rv;
        }

        /* Set the port number into the header */
        rv = RvSipPDCSLAESHeaderSetLaesSigPort(hHeader, port);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitPDCSLAESHeader - error in RvSipPDCSLAESHeaderSetLaesSigPort"));
            return rv;
        }
	}

	/* Check whether content parameter has been set, if so attach it to
       content parameter in the PDCSLAES header */
    if (RV_TRUE == pHeader->isLaesContentHost)
    {
		RvInt32 offsetLaesContentHost = UNDEFINED;
		rv= ParserAllocFromObjPage(pParserMgr,
									 &offsetLaesContentHost,
									 hPool,
									 hPage,
									 pHeader->laesContentHost.buf,
									 pHeader->laesContentHost.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPDCSLAESHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the laes content host parameter in the PDCSLAES header */
		rv = SipPDCSLAESHeaderSetStrLaesContentHost(hHeader,
													 NULL,
													 hPool,
													 hPage,
													 offsetLaesContentHost);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPDCSLAESHeader - error in SipPDCSLAESHeaderSetStrLaesContentHost"));
			return rv;
		}

		/*  Check whether LaesContentPort parameter has been set, if so attach it to
			LaesContentPort parameter in the PDCSLAES header */
		if (RV_TRUE == pHeader->isLaesContentPort)
		{
			RvInt32 port;
			
			/* Change the token port into a number */
			rv = ParserGetINT32FromString(pParserMgr,
											 pHeader->laesContentPort.buf ,
											 pHeader->laesContentPort.len,
											 &port);
			if (RV_OK != rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				 "ParserInitPDCSLAESHeader - error in ParserGetINT32FromString"));
				return rv;
			}

			/* Set the port number into the header */
			rv = RvSipPDCSLAESHeaderSetLaesContentPort(hHeader, port);
			if (RV_OK != rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				 "ParserInitPDCSLAESHeader - error in RvSipPDCSLAESHeaderSetLaesContentPort"));
				return rv;
			}
		}
	}

	/* Check whether key parameter has been set, if so attach it to
       key parameter in the PDCSLAES header */
    if (RV_TRUE == pHeader->isLaesKey)
    {
		RvInt32 offsetLaesKey = UNDEFINED;
		rv= ParserAllocFromObjPage(pParserMgr,
									 &offsetLaesKey,
									 hPool,
									 hPage,
									 pHeader->laesKey.buf,
									 pHeader->laesKey.len);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			  "ParserInitPDCSLAESHeader - error in ParserAllocFromObjPage"));
			return rv;
		}

		/* Set the key parameter in the PDCSLAES header */
		rv = SipPDCSLAESHeaderSetStrLaesKey(hHeader,
													 NULL,
													 hPool,
													 hPage,
													 offsetLaesKey);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPDCSLAESHeader - error in SipPDCSLAESHeaderSetStrLaesKey"));
			return rv;
		}
	}

	/* Check whether Other parameter has been set, if so attach it to
       strOtherParam in the PDCSLAES header */
    if (RV_TRUE == pHeader->isOtherParams)
    {
		RvInt32 offsetOtherParam = UNDEFINED;
		offsetOtherParam = ParserAppendCopyRpoolString(
														pParserMgr,
														hPool,
														hPage,
														((ParserExtensionString *)pHeader->otherParams)->hPool,
														((ParserExtensionString *)pHeader->otherParams)->hPage,
														0,
														((ParserExtensionString *)pHeader->otherParams)->size);

		if (UNDEFINED == offsetOtherParam)
		{
			 RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPDCSLAESHeader - error in ParserAppendCopyRpoolString"));
			 return RV_ERROR_OUTOFRESOURCES;
		}
		rv = SipPDCSLAESHeaderSetOtherParams(hHeader,
												 NULL,
												 hPool,
												 hPage,
												 offsetOtherParam);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPDCSLAESHeader - error in SipPDCSLAESHeaderSetOtherParams"));
			return rv;
		}
	}
    
    return RV_OK;
}

/***************************************************************************
* ParserInitPDCSRedirectHeader
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          PDCSRedirect header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:   pParserMgr - Pointer to the parser manager.
*             pcb		 - PDCSRedirect structure holding the PDCSRedirect 
*						   values from the parser in a token form.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             eType      - This type indicates whether the parser is called to
*                          parse stand alone PDCSRedirect or sip message.
*  In/Output: pSipObject - Pointer to the structure in which the values from
*                          the parser will be set.
*                          If eType == SIP_PARSETYPE_MSG it will be cast to
*                          RvSipMsgHandle and if eType == SIP_PARSETYPE_P_DCS_REDIRECT
*                          it will be cast to RvSipPDCSRedirectHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitPDCSRedirectHeader(
                             IN    ParserMgr                *pParserMgr,
                             IN    SipParser_pcb_type       *pcb,
                             IN    RvUint8                  *pcbPointer,
                             IN    SipParseType              eType,
                             INOUT const void               *pSipObject)
{
    RvStatus						rv;
    RvSipPDCSRedirectHeaderHandle	hHeader;
    HRPOOL							hPool;
    HPAGE							hPage;
	ParserPDCSRedirectHeader		*pHeader;
	RvSipAddressType				eAddrType;
	RvSipAddressHandle				hAddress;

    if (SIP_PARSETYPE_P_DCS_REDIRECT == eType)
    {
        /*The parser is used to parse PDCSRedirect header */
        hHeader=(RvSipPDCSRedirectHeaderHandle)pSipObject;
    }
    else
    {

        rv = ParserConstructHeader(pParserMgr, eType, pSipObject, RVSIP_HEADERTYPE_P_DCS_REDIRECT, 
                                   pcbPointer, (void**)&hHeader);
        if(rv != RV_OK)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitPDCSRedirectHeader - Failed to construct header"));
            return rv;
        }
    }

    hPool = SipPDCSRedirectHeaderGetPool(hHeader);
    hPage = SipPDCSRedirectHeaderGetPage(hHeader);

	pHeader = &pcb->pdcsRedirectHeader;

	/* calledIDUri */
	eAddrType = ParserAddrType2MsgAddrType(pHeader->calledIDUri.uriType);
	if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			"ParserInitPDCSRedirectHeader - unknown parserAddressType %d", pHeader->calledIDUri.uriType));
		return RV_ERROR_UNKNOWN;
	}

	/* Construct the url in the PDCSRedirect header */
	rv = RvSipAddrConstructInPDCSRedirectHeader(hHeader,
											  eAddrType, 
											  RVSIP_P_DCS_REDIRECT_ADDRESS_TYPE_CALLED_ID,
											  &hAddress);
	if (RV_OK!=rv)
	{
		RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPDCSRedirectHeader - error in RvSipAddrConstructInPDCSRedirectHeader"));
		return rv;
	}

	/* Initialize the sip-url\abs-uri values from the parser*/
	rv = ParserInitAddressInHeader(pParserMgr,
									  &pHeader->calledIDUri,
									  hPool, hPage, eAddrType, hAddress);
	if (RV_OK != rv)
	{
		RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			  "ParserInitPDCSRedirectHeader - error in ParserInitAddressInHeader"));
		return rv;
	}

	/* redirectorUri */
	if(RV_TRUE == pHeader->isRedirectorUri)
	{
		eAddrType = ParserAddrType2MsgAddrType(pHeader->redirectorUri.uriType);
		if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitPDCSRedirectHeader - unknown parserAddressType %d", pHeader->redirectorUri.uriType));
			return RV_ERROR_UNKNOWN;
		}

		/* Construct the url in the PDCSRedirect header */
		rv = RvSipAddrConstructInPDCSRedirectHeader(hHeader,
												  eAddrType, 
												  RVSIP_P_DCS_REDIRECT_ADDRESS_TYPE_REDIRECTOR,
												  &hAddress);
		if (RV_OK!=rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				  "ParserInitPDCSRedirectHeader - error in RvSipAddrConstructInPDCSRedirectHeader"));
			return rv;
		}

		/* Initialize the sip-url\abs-uri values from the parser*/
		rv = ParserInitAddressInHeader(pParserMgr,
										  &pHeader->redirectorUri,
										  hPool, hPage, eAddrType, hAddress);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
				  "ParserInitPDCSRedirectHeader - error in ParserInitAddressInHeader"));
			return rv;
		}
	}
	
    /* Check whether there is a count parameter, if so set it in the header */
    if (RV_TRUE == pHeader->isCount)
    {
        RvInt32 count;

        /* Change the token count into a number */
        rv = ParserGetINT32FromString(pParserMgr,
										 pHeader->count.buf ,
										 pHeader->count.len,
										 &count);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitPDCSRedirectHeader - error in ParserGetINT32FromString"));
            return rv;
        }

        /* Set the count number into the header */
        rv = RvSipPDCSRedirectHeaderSetCount(hHeader, count);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitPDCSRedirectHeader - error in RvSipPDCSRedirectHeaderSetCount"));
            return rv;
        }
	}
	
	/* Check whether Other parameter has been set, if so attach it to
       strOtherParam in the PChargingVector header */
    if (RV_TRUE == pHeader->isOtherParams)
    {
		RvInt32 offsetOtherParam = UNDEFINED;
		offsetOtherParam  = ParserAppendCopyRpoolString(pParserMgr,
														hPool,
														hPage,
														((ParserExtensionString *)pHeader->otherParams)->hPool,
														((ParserExtensionString *)pHeader->otherParams)->hPage,
														0,
														((ParserExtensionString *)pHeader->otherParams)->size);

		if (UNDEFINED == offsetOtherParam)
		{
			 RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPDCSRedirectHeader - error in ParserAppendCopyRpoolString"));
			 return RV_ERROR_OUTOFRESOURCES;
		}
		rv = SipPDCSRedirectHeaderSetOtherParams(hHeader,
												 NULL,
												 hPool,
												 hPage,
												 offsetOtherParam);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPDCSRedirectHeader - error in SipPDCSRedirectHeaderSetOtherParams"));
			return rv;
		}
	}
    
    return RV_OK;
}

#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS IMPLEMENTATION                */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * ParserConstructHeader
 * ------------------------------------------------------------------------
 * General:This function constructs header in a message or in a headers list.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:   pParserMgr  - Pointer to the parser manager.
 *             eObjType    - The object type that was allocated and given to
 *                           the parser (MSG or HEADERS LIST)
 *             pSipObjec   - The object that was allocated and given to
 *                           the parser.
 *             eHeaderType - The type of header that should be constructed.
 *             pcbPointer  - Pointer to the current location of parser pointer
 *                           in msg buffer (the end of the given header).
 *    Output:  ppHeader    - Pointer to the new header handle.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserConstructHeader(IN ParserMgr       *pParserMgr,
                                               IN SipParseType     eObjType,
                                               IN const void*      pSipObject,
                                               IN RvSipHeaderType  eHeaderType,
                                               IN RvUint8         *pcbPointer,
                                               OUT void**          ppHeader)
{
    RvStatus rv = RV_OK;

    if (SIP_PARSETYPE_MSG == eObjType)
    {
        RvSipMsgHandle hMsg = (RvSipMsgHandle)pSipObject;

        /* check if parser read all header, until NULL (or COMMA).*/
        if((*(pcbPointer)!=NULL_CHAR) && (*(pcbPointer)!= ','))
        {
            return RV_ERROR_ILLEGAL_SYNTAX;
        }

        /* Construct header in the message */
        rv = SipHeaderConstructInMsg(hMsg, eHeaderType, ppHeader);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserConstructHeader - error in RvSipHeaderConstructInMsg (headerType = %s)",
                SipHeaderName2Str(eHeaderType)));
           return rv;
        }
    }
#ifndef RV_SIP_PRIMITIVES
    else if(SIP_PARSETYPE_URL_HEADERS_LIST == eObjType)
    {
        RvSipCommonListElemHandle hListElem;
        RvSipCommonListHandle hList =(RvSipCommonListHandle)pSipObject;
        HRPOOL hPool = RvSipCommonListGetPool(hList);
        HPAGE  hPage = RvSipCommonListGetPage(hList);

        rv = SipHeaderConstruct(pParserMgr->hMsgMgr, eHeaderType, hPool, hPage, ppHeader);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserConstructHeader - error in construct header in list"));
            return rv;
        }
        rv = RvSipCommonListPushElem(hList, eHeaderType,
                              *ppHeader, RVSIP_LAST_ELEMENT, NULL, &hListElem);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserConstructHeader - RvSipCommonListPushElem failed (rv =%d)", rv));
            return rv;
        }
    }
#endif /*#ifndef RV_SIP_PRIMITIVES*/
    else
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserConstructHeader - Illegal objType %d",eObjType));
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;
}

/***************************************************************************
 * initHeaderPrefix
 * ------------------------------------------------------------------------
 * General:This function converts SipParseType to RvChar*.
 * Return Value: RvChar*
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input : parseType     - The stand alone header type.
 *    Output: none
 ***************************************************************************/
static const RvChar* RVCALLCONV initHeaderPrefix(IN  SipParseType   parseType)
{
   switch (parseType)
    {
        case SIP_PARSETYPE_ALLOW:
        {
            static const RvChar allow[HEADER_PREFIX_LEN]      = {'A','W',':',' '};
            return allow;
        }
        case SIP_PARSETYPE_CONTACT:
        {
            static const RvChar contact[HEADER_PREFIX_LEN]    = {'C','O',':',' '};
            return contact;
        }
        case SIP_PARSETYPE_EXPIRES:
        {
            static const RvChar expires[HEADER_PREFIX_LEN]    = {'E','X',':',' '};
            return expires;
        }
        case SIP_PARSETYPE_DATE:
        {
            static const RvChar date[HEADER_PREFIX_LEN]       = {'D','T',':',' '};
            return date;
        }
        case SIP_PARSETYPE_VIA:
        {
            static const RvChar via[HEADER_PREFIX_LEN]        = {'V','I',':',' '};
            return via;
        }
        case SIP_PARSETYPE_CSEQ:
        {
            static const RvChar cseq[HEADER_PREFIX_LEN]       = {'C','S',':',' '};
            return cseq;

        }
        case SIP_PARSETYPE_TO:
        case SIP_PARSETYPE_FROM:
        {
            static const RvChar party[HEADER_PREFIX_LEN]      = {'P','R',':',' '};
            return party;
        }
        case SIP_PARSETYPE_RSEQ:
        {
            static const RvChar rseq[HEADER_PREFIX_LEN]       = {'R','Q',':',' '};
            return rseq;
        }
        case SIP_PARSETYPE_RACK:
        {
            static const RvChar rack[HEADER_PREFIX_LEN]       = {'R','K',':',' '};
            return rack;
        }
        case SIP_PARSETYPE_RETRY_AFTER:
        {
            static const RvChar retryAfter[HEADER_PREFIX_LEN] = {'R','A',':',' '};
            return retryAfter;
        }
        case SIP_PARSETYPE_REFER_TO:
        {
            static const RvChar referTo[HEADER_PREFIX_LEN]    = {'R','T',':',' '};
            return referTo;
        }
        case SIP_PARSETYPE_WWW_AUTHENTICATE:
        case SIP_PARSETYPE_PROXY_AUTHENTICATE:
        {
            static const RvChar a1[HEADER_PREFIX_LEN]         = {'A','1',':',' '};
            return a1;
        }
        case SIP_PARSETYPE_AUTHORIZATION:
        case SIP_PARSETYPE_PROXY_AUTHORIZATION:
        {
            static const RvChar a2[HEADER_PREFIX_LEN]         = {'A','2',':',' '};
            return a2;
        }
        case SIP_PARSETYPE_ROUTE:
        case SIP_PARSETYPE_RECORD_ROUTE:
#ifndef RV_SIP_JSR32_SUPPORT
        case SIP_PARSETYPE_PATH:
        case SIP_PARSETYPE_SERVICE_ROUTE:
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
        {
            static const RvChar route[HEADER_PREFIX_LEN]      = {'R','H',':',' '};
            return route;
        }
        case SIP_PARSETYPE_CALL_ID:
        {
            static const RvChar callId[HEADER_PREFIX_LEN]     = {'C','I',':',' '};
            return callId;
        }
        case SIP_PARSETYPE_CONTENT_TYPE:
        {
            static const RvChar conType[HEADER_PREFIX_LEN]    = {'C','T',':',' '};
            return conType;
        }
        case SIP_PARSETYPE_CONTENT_ID:
        {
            static const RvChar conType[HEADER_PREFIX_LEN]    = {'C','Y',':',' '};
            return conType;
        }
        case SIP_PARSETYPE_CONTENT_LENGTH:
        {
            static const RvChar conLength[HEADER_PREFIX_LEN]  = {'C','L',':',' '};
            return conLength;
        }
        case SIP_PARSETYPE_CONTENT_DISPOSITION:
        {
            static const RvChar conDisp[HEADER_PREFIX_LEN]    = {'C','D',':',' '};
            return conDisp;
        }
        case SIP_PARSETYPE_REQUIRE:
        case SIP_PARSETYPE_PROXY_REQUIRE:
        case SIP_PARSETYPE_UNSUPPORTED:
        {
            static const RvChar require[HEADER_PREFIX_LEN]    = {'O','T',':',' '};
            return require;
        }
        case SIP_PARSETYPE_SUPPORTED:
        {
            static const RvChar supported[HEADER_PREFIX_LEN]  = {'S','P',':',' '};
            return supported;
        }
        case SIP_PARSETYPE_EVENT:
        {
            static const RvChar event[HEADER_PREFIX_LEN]      = {'E','V',':',' '};
            return event;
        }
        case SIP_PARSETYPE_SUBSCRIPTION_STATE:
        {
            static const RvChar SubsState[HEADER_PREFIX_LEN]  = {'S','S',':',' '};
            return SubsState;
        }
        case SIP_PARSETYPE_ALLOW_EVENTS:
        {
            static const RvChar AllowEvent[HEADER_PREFIX_LEN] = {'A','E',':',' '};
            return AllowEvent;
        }
#ifdef RV_SIP_AUTH_ON
        case SIP_PARSETYPE_AUTHENTICATION_INFO:
		{
			static const RvChar auth_info[HEADER_PREFIX_LEN]  = {'A','I',':',' '};
			return auth_info;
		}
#endif /*#ifdef RV_SIP_AUTH_ON*/
#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
		
        case SIP_PARSETYPE_REASON:
        {
            static const RvChar Reason[HEADER_PREFIX_LEN]   = {'R','E',':',' '};
            return Reason;
        }
		case SIP_PARSETYPE_WARNING:
        {
            static const RvChar Warning[HEADER_PREFIX_LEN]   = {'W','A',':',' '};
            return Warning;
        }
		case SIP_PARSETYPE_TIMESTAMP:
        {
            static const RvChar Timestamp[HEADER_PREFIX_LEN]   = {'T','S',':',' '};
            return Timestamp;
        }
		case SIP_PARSETYPE_ALERT_INFO:
		case SIP_PARSETYPE_CALL_INFO:
		case SIP_PARSETYPE_ERROR_INFO:
        {
            static const RvChar Info[HEADER_PREFIX_LEN]   = {'I','N',':',' '};
            return Info;
        }
		case SIP_PARSETYPE_ACCEPT:
        {
            static const RvChar Accept[HEADER_PREFIX_LEN]   = {'A','C',':',' '};
            return Accept;
        }
		case SIP_PARSETYPE_ACCEPT_ENCODING:
        {
            static const RvChar AcceptEncoding[HEADER_PREFIX_LEN]   = {'A','G',':',' '};
            return AcceptEncoding;
        }
		case SIP_PARSETYPE_ACCEPT_LANGUAGE:
        {
            static const RvChar AcceptLanguage[HEADER_PREFIX_LEN]   = {'A','L',':',' '};
            return AcceptLanguage;
        }
		case SIP_PARSETYPE_REPLY_TO:
        {
            static const RvChar ReplyTo[HEADER_PREFIX_LEN]   = {'R','O',':',' '};
            return ReplyTo;
        }
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */
#ifndef RV_SIP_JSR32_SUPPORT
		/* The following are headers that are not supported by jsr32, therefore should appear
		as other-headers when compiled with jsr32 
		See also Route hop headers.*/
		case SIP_PARSETYPE_REFERRED_BY:
			{
				static const RvChar referredBy[HEADER_PREFIX_LEN] = {'R','B',':',' '};
				return referredBy;
			}
		case SIP_PARSETYPE_SESSION_EXPIRES:
			{
				static const RvChar SessExp[HEADER_PREFIX_LEN]    = {'S','E',':',' '};
				return SessExp;
			}
        case SIP_PARSETYPE_MINSE:
			{
				static const RvChar MinSE[HEADER_PREFIX_LEN]      = {'M','S',':',' '};
				return MinSE;
			}
        case SIP_PARSETYPE_REPLACES:
			{
				static const RvChar Replaces[HEADER_PREFIX_LEN]   = {'R','P',':',' '};
				return Replaces;
			}	
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
		case SIP_PARSETYPE_P_ASSOCIATED_URI:
		case SIP_PARSETYPE_P_CALLED_PARTY_ID:
		case SIP_PARSETYPE_P_ASSERTED_IDENTITY:
		case SIP_PARSETYPE_P_PREFERRED_IDENTITY:
			{
				static const RvChar PUri[HEADER_PREFIX_LEN]   = {'P','U',':',' '};
				return PUri;
			}
		case SIP_PARSETYPE_P_VISITED_NETWORK_ID:
			{
				static const RvChar PVisitedNetworkID[HEADER_PREFIX_LEN]   = {'P','V',':',' '};
				return PVisitedNetworkID;
			}
		case SIP_PARSETYPE_P_ACCESS_NETWORK_INFO:
			{
				static const RvChar PAccessNetworkInfo[HEADER_PREFIX_LEN]   = {'P','A',':',' '};
				return PAccessNetworkInfo;
			}
		case SIP_PARSETYPE_P_CHARGING_FUNCTION_ADDRESSES:
			{
				static const RvChar PChargingFunctionAddresses[HEADER_PREFIX_LEN]   = {'P','C',':',' '};
				return PChargingFunctionAddresses;
			}
		case SIP_PARSETYPE_P_CHARGING_VECTOR:
			{
				static const RvChar PChargingVector[HEADER_PREFIX_LEN]   = {'P','G',':',' '};
				return PChargingVector;
			}
		case SIP_PARSETYPE_P_MEDIA_AUTHORIZATION:
			{
				static const RvChar PMediaAuthorization[HEADER_PREFIX_LEN]   = {'P','M',':',' '};
				return PMediaAuthorization;
			}
		case SIP_PARSETYPE_SECURITY_CLIENT:
		case SIP_PARSETYPE_SECURITY_SERVER:
		case SIP_PARSETYPE_SECURITY_VERIFY:
			{
				static const RvChar Security[HEADER_PREFIX_LEN]   = {'S','C',':',' '};
				return Security;
			}
        case SIP_PARSETYPE_P_PROFILE_KEY:
			{
				static const RvChar PProfileKey[HEADER_PREFIX_LEN]   = {'P','P',':',' '};
				return PProfileKey;
			}
        case SIP_PARSETYPE_P_USER_DATABASE:
			{
				static const RvChar PUserDatabase[HEADER_PREFIX_LEN]   = {'P','S',':',' '};
				return PUserDatabase;
			}
        case SIP_PARSETYPE_P_ANSWER_STATE:
			{
				static const RvChar PAnswerState[HEADER_PREFIX_LEN]   = {'P','W',':',' '};
				return PAnswerState;
			}
        case SIP_PARSETYPE_P_SERVED_USER:
			{
				static const RvChar PServedUser[HEADER_PREFIX_LEN]   = {'P','E',':',' '};
				return PServedUser;
			}            
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
		case SIP_PARSETYPE_P_DCS_TRACE_PARTY_ID:
			{
				static const RvChar PDCSTracePartyID[HEADER_PREFIX_LEN]   = {'P','T',':',' '};
				return PDCSTracePartyID;
			}
		case SIP_PARSETYPE_P_DCS_OSPS:
			{
				static const RvChar PDCSOSPS[HEADER_PREFIX_LEN]   = {'P','O',':',' '};
				return PDCSOSPS;
			}
		case SIP_PARSETYPE_P_DCS_BILLING_INFO:
			{
				static const RvChar PDCSBillingInfo[HEADER_PREFIX_LEN]   = {'P','B',':',' '};
				return PDCSBillingInfo;
			}
		case SIP_PARSETYPE_P_DCS_LAES:
			{
				static const RvChar PDCSLAES[HEADER_PREFIX_LEN]   = {'P','L',':',' '};
				return PDCSLAES;
			}
		case SIP_PARSETYPE_P_DCS_REDIRECT:
			{
				static const RvChar PDCSRedirect[HEADER_PREFIX_LEN]   = {'P','D',':',' '};
				return PDCSRedirect;
			}
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
        default:
        {
            return NULL;
        }
    }

	/* XXX */
}


/***************************************************************************
 * InitializePcbStructByHeaderType
 * ------------------------------------------------------------------------
 * General: Initialize the fields of the PCB structure that are used while
 *          parsing the given header.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:     pcb        - pointer to the PCB structure.
 *             eType      - The type of the given header.
 ***************************************************************************/
static void RVCALLCONV InitializePcbStructByHeaderType(
											IN  SipParser_pcb_type* pcb,
											IN  SipParseType        eType)
{
    /*XXX*/
    
	switch(eType) {
	case SIP_PARSETYPE_UNDEFINED: /* start-line*/
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_NAME_ADDR,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_REQUEST_LINE,pcb);
		break;
	case SIP_PARSETYPE_CONTACT:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_CONTACT,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_NAME_ADDR,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXPIRES,pcb);
		break;
	case SIP_PARSETYPE_EXPIRES:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXPIRES,pcb);
		break;
	case SIP_PARSETYPE_VIA:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_VIA_SENT_BY, pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SINGLE_VIA,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SENT_PROTOCOL,pcb);	
		break;
	case SIP_PARSETYPE_TO:
	case SIP_PARSETYPE_FROM:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_PARTY,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_NAME_ADDR,pcb);
		break;
	case SIP_PARSETYPE_URLADDRESS:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_URL_PARAMETER,pcb);
		break;
	case SIP_PARSETYPE_TELURIADDRESS:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_URL_PARAMETER,pcb);
		break;
#ifdef RV_SIP_IMS_HEADER_SUPPORT
	case SIP_PARSETYPE_DIAMETER_URI_ADDRESS:
#if defined(UPDATED_BY_SPIRENT)        
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_TEL_URI,pcb);
#endif        
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL,pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_DIAMETER_URL,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_URL_PARAMETER,pcb);
		break;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
	case SIP_PARSETYPE_ROUTE:
	case SIP_PARSETYPE_RECORD_ROUTE:
	case SIP_PARSETYPE_PATH:
	case SIP_PARSETYPE_SERVICE_ROUTE:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_NAME_ADDR,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_ROUTE,pcb);
		break;
	case SIP_PARSETYPE_REPLACES:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_REPLACES_PARAMS,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_REPLACES_PARAM,pcb);
		break;
	case SIP_PARSETYPE_CONTENT_TYPE:
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_CONTENT_TYPE,pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL, pcb);
		break;
    case SIP_PARSETYPE_CONTENT_ID:
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_CONTENT_ID, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL, pcb);
        break;
	case SIP_PARSETYPE_CONTENT_DISPOSITION:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_CONTENT_DISPOSITION,pcb);
		break;
	case SIP_PARSETYPE_RACK:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_RACK,pcb);
		break;
	case SIP_PARSETYPE_PROXY_REQUIRE:
	case SIP_PARSETYPE_REQUIRE:
		break;
	case SIP_PARSETYPE_SUPPORTED:
	case SIP_PARSETYPE_UNSUPPORTED:
		break;
	case SIP_PARSETYPE_RETRY_AFTER:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_RETRY_AFTER,pcb);
		break;
    case SIP_PARSETYPE_EVENT:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EVENT_HEADER,pcb);
		break;
#ifdef RV_SIP_AUTH_ON
	case SIP_PARSETYPE_WWW_AUTHENTICATE:
	case SIP_PARSETYPE_PROXY_AUTHENTICATE:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_DIGEST_CHALLENGE,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_AUTHENTICATION,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI,pcb);
		break;
	case SIP_PARSETYPE_AUTHORIZATION:
	case SIP_PARSETYPE_PROXY_AUTHORIZATION:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_AUTHORIZATION,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_DIGEST_CHALLENGE,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI,pcb);
		break;
#endif /* #ifdef RV_SIP_AUTH_ON */
#ifdef RV_SIP_SUBS_ON
	case SIP_PARSETYPE_REFER_TO:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_NAME_ADDR,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_REFER_TO,pcb);
		break;
	case SIP_PARSETYPE_REFERRED_BY:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_NAME_ADDR,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_REFERRED_BY,pcb);
		break;
	case SIP_PARSETYPE_SUBSCRIPTION_STATE:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SUBS_STATE_HEADER,pcb);
		break;
#endif /* #ifdef RV_SIP_SUBS_ON */
	case SIP_PARSETYPE_START_LINE:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_REQUEST_LINE,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL,pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI,pcb);
		break;
#ifdef RV_SIP_AUTH_ON
	case SIP_PARSETYPE_AUTHENTICATION_INFO:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_AUTHENTICATION_INFO,pcb);
		break;
#endif /*#ifdef RV_SIP_AUTH_ON*/
#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
	case SIP_PARSETYPE_REASON:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_REASON,pcb);
		break;
	case SIP_PARSETYPE_WARNING:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_WARNING,pcb);
		break;
	case SIP_PARSETYPE_TIMESTAMP:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_TIMESTAMP,pcb);
		break;
	case SIP_PARSETYPE_ALERT_INFO:
	case SIP_PARSETYPE_CALL_INFO:
	case SIP_PARSETYPE_ERROR_INFO:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_INFO,pcb);
		break;
	case SIP_PARSETYPE_ACCEPT:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_ACCEPT,pcb);
		break;
	case SIP_PARSETYPE_ACCEPT_ENCODING:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_ACCEPT_ENCODING,pcb);
		break;
	case SIP_PARSETYPE_ACCEPT_LANGUAGE:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_ACCEPT_LANGUAGE,pcb);
		break;
	case SIP_PARSETYPE_REPLY_TO:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_NAME_ADDR, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI, pcb);
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_REPLY_TO,pcb);
		break;
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */	
#ifdef RV_SIP_IMS_HEADER_SUPPORT
	case SIP_PARSETYPE_P_ASSOCIATED_URI:
	case SIP_PARSETYPE_P_CALLED_PARTY_ID:
	case SIP_PARSETYPE_P_ASSERTED_IDENTITY:
	case SIP_PARSETYPE_P_PREFERRED_IDENTITY:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_P_URI,pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_NAME_ADDR, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL, pcb);
		break;
	case SIP_PARSETYPE_P_VISITED_NETWORK_ID:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_P_VISITED_NETWORK_ID,pcb);
		break;
	case SIP_PARSETYPE_P_ACCESS_NETWORK_INFO:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_P_ACCESS_NETWORK_INFO,pcb);
		break;
	case SIP_PARSETYPE_P_CHARGING_FUNCTION_ADDRESSES:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_P_CHARGING_FUNCTION_ADDRESSES,pcb);
		break;
	case SIP_PARSETYPE_P_CHARGING_VECTOR:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_P_CHARGING_VECTOR,pcb);
		break;
	case SIP_PARSETYPE_P_MEDIA_AUTHORIZATION:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_P_MEDIA_AUTHORIZATION,pcb);
		break;
	case SIP_PARSETYPE_SECURITY_CLIENT:
	case SIP_PARSETYPE_SECURITY_SERVER:
	case SIP_PARSETYPE_SECURITY_VERIFY:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SECURITY,pcb);
		break;
	case SIP_PARSETYPE_P_PROFILE_KEY:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_P_PROFILE_KEY,pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_NAME_ADDR, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL, pcb);
		break;
	case SIP_PARSETYPE_P_USER_DATABASE:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_P_USER_DATABASE,pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_NAME_ADDR, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL, pcb);
		break;
	case SIP_PARSETYPE_P_SERVED_USER:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_P_SERVED_USER,pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_NAME_ADDR, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL, pcb);
		break;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */ 
#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
	case SIP_PARSETYPE_P_DCS_TRACE_PARTY_ID:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_P_DCS_TRACE_PARTY_ID, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_NAME_ADDR, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL, pcb);
		break;
	case SIP_PARSETYPE_P_DCS_OSPS:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_P_DCS_OSPS, pcb);
		break;
	case SIP_PARSETYPE_P_DCS_BILLING_INFO:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_P_DCS_BILLING_INFO, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI, pcb);
		break;
	case SIP_PARSETYPE_P_DCS_LAES:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_P_DCS_LAES, pcb);
		break;
	case SIP_PARSETYPE_P_DCS_REDIRECT:
		ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_P_DCS_REDIRECT, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_SIP_URL, pcb);
        ParserInitializePCBStructs(SIP_PARSE_PCBSTRUCT_EXURI, pcb);
		break;
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
	
	default:
		break;
	}
}

/***************************************************************************
 * SetExternalAddressPrefix
 * ------------------------------------------------------------------------
 * General: Set the parser pointer to point to an external string, which 
 *          contains the header prefix 
 *          This function is called for every header that is not 'other header'
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:     pcb         - pointer to the buffer PCB structure.
 *             eHeaderType - The type of the header.
 ***************************************************************************/
static void RVCALLCONV SetExternalAddressPrefix(IN SipParser_pcb_type* pcb,
                                                IN SipParseType       eType)
{
    
#ifndef RV_SIP_PRIMITIVES
    static const RvChar strUri[HEADER_PREFIX_LEN] = {'U', 'R', ':', ' '};

    /* for abs-uri */
    if (eType == SIP_PARSETYPE_URIADDRESS)
    {
        pcb->pointer = (RvUint8 *)strUri;
    }
#ifdef RV_SIP_JSR32_SUPPORT
    /* for sip-url */
	if (eType == SIP_PARSETYPE_URLADDRESS ||
		eType == SIP_PARSETYPE_ANYADDRESS)
    {
        pcb->pointer = (RvUint8 *)strUri;
    }
#ifdef RV_SIP_TEL_URI_SUPPORT
	/* for tel-uri */
    if (eType == SIP_PARSETYPE_TELURIADDRESS)
    {
        pcb->pointer = (RvUint8 *)strUri;
    }
#endif /* RV_SIP_TEL_URI_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
	/* for diameter-uri */
    if (eType == SIP_PARSETYPE_DIAMETER_URI_ADDRESS)
    {
        pcb->pointer = (RvUint8 *)strUri;
    }
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */
#else
    RV_UNUSED_ARG(eType)
    RV_UNUSED_ARG(pcb)
#endif /* RV_SIP_PRIMITIVES*/
}

/***************************************************************************
 * SetExternalHeaderPrefix
 * ------------------------------------------------------------------------
 * General: Set the parser pointer to point to an external string, which 
 *          contains the header or address prefix.
 *          This function is called for every header that is not 'other header'
 *          Please note that an external prefix must be 2 characters long. 
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:     pcb         - pointer to the buffer PCB structure.
 *             eHeaderType - The type of the header.
 ***************************************************************************/
static void RVCALLCONV SetExternalHeaderPrefix(IN SipParser_pcb_type* pcb,
                                               IN SipParseType       eHeaderType)
{
    if (eHeaderType == SIP_PARSETYPE_URIADDRESS ||
        eHeaderType == SIP_PARSETYPE_URLADDRESS ||
		eHeaderType == SIP_PARSETYPE_ANYADDRESS ||
        eHeaderType == SIP_PARSETYPE_TELURIADDRESS || 
        eHeaderType == SIP_PARSETYPE_DIAMETER_URI_ADDRESS)
    {
        SetExternalAddressPrefix(pcb, eHeaderType);
    }
    else 
    {
        const RvChar *strHeaderName;
        
        /* 1. set the correct header prefix */
        strHeaderName = initHeaderPrefix(eHeaderType);

        /* 2. set the parser pointer to point to the prefix buffer */
        if(NULL != strHeaderName)
        {
            pcb->pointer = (RvUint8 *)strHeaderName;
        }

    }
    
}


/***************************************************************************
 * ParserInitMethod
 * ------------------------------------------------------------------------
 * General:This method Check the method type: if it is one of INVITE/ACK/BYE
 *         the enumerate method type will set accordingly, otherwise,
 *         the enumerate method type will be set to OTHER and the string method
 *         type will be set too.
 * Return Value: RV_OK        - on Success.
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 *               RV_ERROR_UNKNOWN        - Invalid enumerate method type.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input : method        - The method structure from the parser.
 *          hPool         - Handle to the pool to allocated from.
 *          hPage         - handle to the page to allocate from.
 *    Output: peMethodType   - The enumerate method type value.
 *            pAllocationOffset - Pointer to the offset on the page which the
 *                              string was allocated.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitMethodType(
                         IN  ParserMgr *pParserMgr,
                         IN  ParserMethod    method,
                         IN  HRPOOL          hPool,
                         IN  HPAGE           hPage,
                         OUT RvSipMethodType *peMethodType,
                         OUT RvInt32       *pAllocationOffset)
{
    RvStatus rv;
    switch (method.type)
    {
        case PARSER_METHOD_TYPE_INVITE:
        {
             *peMethodType = RVSIP_METHOD_INVITE;
             break;
        }
        case PARSER_METHOD_TYPE_ACK:
        {
             *peMethodType = RVSIP_METHOD_ACK;
             break;
        }
        case PARSER_METHOD_TYPE_BYE:
        {
             *peMethodType = RVSIP_METHOD_BYE;
             break;
        }
        case PARSER_METHOD_TYPE_REGISTER:
        {
             *peMethodType = RVSIP_METHOD_REGISTER;
             break;
        }
        case PARSER_METHOD_TYPE_REFER:
        {
            *peMethodType = RVSIP_METHOD_REFER;
            break;
        }
        case PARSER_METHOD_TYPE_SUBSCRIBE:
        {
            *peMethodType = RVSIP_METHOD_SUBSCRIBE;
            break;
        }
        case PARSER_METHOD_TYPE_NOTIFY:
        {
            *peMethodType = RVSIP_METHOD_NOTIFY;
            break;
        }
        case PARSER_METHOD_TYPE_CANCEL:
        {
            *peMethodType = RVSIP_METHOD_CANCEL;
            break;
        }
        case PARSER_METHOD_TYPE_PRACK:
        {
            *peMethodType = RVSIP_METHOD_PRACK;
            break;
        }
        case PARSER_METHOD_TYPE_OTHER:
        {
             *peMethodType = RVSIP_METHOD_OTHER;
             rv= ParserAllocFromObjPage(pParserMgr,
                                             pAllocationOffset,
                                             hPool,
                                             hPage,
                                             method.other.buf,
                                             method.other.len);
            if (RV_OK!= rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitMethodType - Unable to allocate string size %d",
                    method.other.len ));
                return rv;
            }
            break;
        }
        default:
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitMethodType - Invalid enumerate method type  %d",
            method.type));
            return RV_ERROR_UNKNOWN;
        }
    } /* switch (method.type)*/

    return RV_OK;
}


/***************************************************************************
 * ParserInitUrlParams
 * ------------------------------------------------------------------------
 * General:This method initialize sip url handle with url parameters
 *         from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:      pParserMgr - Pointer to the parser manager.
 *              urlParams  - The url parameters from the parser.
 *              hPool      - Handle to the message pool.
 *              hPage      - Handle to the message page.
 *    In/OutPut:  hUrl       - Handle to the address structure which will be
 *                           set by the parser.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitUrlParams(
                                IN    ParserMgr             *pParserMgr,
                                IN    ParserUrlParameters   *urlParams,
                                IN    HRPOOL                hPool,
                                IN    HPAGE                 hPage,
                                INOUT RvSipAddressHandle    hUrl)
{

    RvStatus rv;

    /* Check whether the there is transport parameter, if so,
       we will set the enumerate value or the string value */
    if (RV_TRUE == urlParams->isTransport )
    {
        RvInt32 offsetTransport = UNDEFINED;
        /* for all transport types we save also the transport string */
       rv = ParserAllocFromObjPage(pParserMgr,
                                       &offsetTransport,
                                       hPool,
                                       hPage,
                                       urlParams->transport.strTransport.buf,
                                       urlParams->transport.strTransport.len);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitUrlParams - error in ParserAllocFromObjPage"));
            return rv;
        }

        if (RVSIP_TRANSPORT_OTHER == urlParams->transport.transport)
        {
            rv = SipAddrUrlSetTransport(hUrl,
                                      urlParams->transport.transport,
                                      NULL,
                                      hPool,
                                      hPage,
                                      offsetTransport,
                                      UNDEFINED);
        }
        else
        {
            rv = SipAddrUrlSetTransport(hUrl,
                                      urlParams->transport.transport,
                                      NULL,
                                      hPool,
                                      hPage,
                                      UNDEFINED,
                                      offsetTransport);
        }
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitUrlParams - error in AddrUrlSetTransport"));
            return rv;
        }

    }/*if (RV_TRUE == urlParams->isTransport ) */

    /* Check whether the there is user parameter, if so,
       we will set the enumerate value or the string value */
    if (RV_TRUE == urlParams->isUserParam )
    {
        RvInt32 offsetUserParam = UNDEFINED;
        if (RVSIP_USERPARAM_OTHER == urlParams->userParam.paramType)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetUserParam,
                                           hPool,
                                           hPage,
                                           urlParams->userParam.otherUser.buf,
                                           urlParams->userParam.otherUser.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitUrlParams - error in ParserAllocFromObjPage"));
                return rv;
            }
        }
        rv = SipAddrUrlSetUserParam(hUrl,
                                       urlParams->userParam.paramType,
                                       NULL,
                                       hPool,
                                       hPage,
                                       offsetUserParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitUrlParams - error in AddrUrlSetUserParam"));
            return rv;
        }

    } /* if (RV_TRUE == urlParams->isUserParam )*/

    /* Check whether the there is multi address parameter, if so,
       we will set the string value */
    if (RV_TRUE == urlParams->isMaddrParam)
    {
        RvInt32 offsetMaddrParam = UNDEFINED;
        rv = ParserAllocFromObjPage(pParserMgr,
                                       &offsetMaddrParam,
                                       hPool,
                                       hPage,
                                       urlParams->maddrParam.buf,
                                       urlParams->maddrParam.len);
        if (RV_OK != rv)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitUrlParams - error in ParserAllocFromObjPage"));
             return rv;
        }
        rv = SipAddrUrlSetMaddrParam(hUrl,
                                        NULL,
                                        hPool,
                                        hPage,
                                        offsetMaddrParam);
        if (RV_OK != rv)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitUrlParams - error in AddrUrlSetMaddrParam"));
            return rv;
        }
    } /* (RV_TRUE == urlParams->isMaddrParam)*/

    /* Check whether there is ttl param , if so, we will set the value */
    if (RV_TRUE == urlParams->isTtlParam)
    {
        RvInt16 ttlNum;
        rv=ParserGetINT16FromString(pParserMgr,
                                       urlParams->ttlParam.buf ,
                                       urlParams->ttlParam.len,
                                       &ttlNum);
        if (RV_OK != rv)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitUrlParams - error in ParserGetINT16FromString"));
            return rv;
        }

        /* Set the ttl number from Via parameters in the Via header*/
        rv = RvSipAddrUrlSetTtlNum(hUrl,ttlNum);
        if (RV_OK != rv)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitUrlParams - error in RvSipAddrUrlSetTtlNum"));
            return rv;
        }
    }

    /* Check whether the there is method parameter, if so,
       we will set the enumerate value or the string value */
    if (RV_TRUE == urlParams->isMethodParam )
    {
        RvInt32        offsetMethodParam = UNDEFINED;
        RvSipMethodType eMethod;

        rv = ParserInitMethodType(pParserMgr,
                                     urlParams->methodParam,
                                     hPool,
                                     hPage,
                                     &eMethod,
                                     &offsetMethodParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitUrlParams - error in ParserInitMethodType"));
            return rv;
        }
        rv = SipAddrUrlSetMethod(hUrl,
                                    eMethod,
                                    NULL,
                                    hPool,
                                    hPage,
                                    offsetMethodParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitUrlParams - error in AddrUrlSetMethodType"));
            return rv;
        }

    } /* if (RV_TRUE == urlParams->isMethodParam )*/

    switch(urlParams->lrParamType)
    {
    case ParserLrParamEmpty:
        rv = RvSipAddrUrlSetLrParamType(hUrl, RVSIP_URL_LR_TYPE_EMPTY);
        break;
    case ParserLrParam1:
        rv = RvSipAddrUrlSetLrParamType(hUrl, RVSIP_URL_LR_TYPE_1);
        break;
    case ParserLrParamTrue:
        rv = RvSipAddrUrlSetLrParamType(hUrl, RVSIP_URL_LR_TYPE_TRUE);
        break;
	case ParserLrParamOn:
        rv = RvSipAddrUrlSetLrParamType(hUrl, RVSIP_URL_LR_TYPE_ON);
        break;
    default:
        rv = RV_OK;
        break;
    }
    if (RV_OK != rv)
    {
         RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitUrlParams - error in RvSipAddrUrlSetLrParamType"));
        return rv;
    }

	/* '#ifdef RV_SIGCOMP_ON' Shouldn't be here since the comp parameter should be referred even 
	    when SIGCOMP feature is turned off */ 
    /* Check whether the there is comp parameter, if so,
       we will set the enumerate value or the string value */
    if (RV_TRUE == urlParams->isCompParam)
    {
        RvInt32 offsetCompParam = UNDEFINED;
        if (RVSIP_COMP_OTHER == urlParams->compParam.compType)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetCompParam,
                                           hPool,
                                           hPage,
                                           urlParams->compParam.strCompParam.buf,
                                           urlParams->compParam.strCompParam.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitUrlParams - error in ParserAllocFromObjPage"));
                return rv;
            }
        }
        rv = SipAddrUrlSetCompParam(hUrl,
                                       urlParams->compParam.compType,
                                       NULL,
                                       hPool,
                                       hPage,
                                       offsetCompParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitUrlParams - error in SipAddrUrlSetCompParam"));
            return rv;
        }

    }/*if (RV_TRUE == urlParams->isCompParam ) */

    /* Check whether the there is sigcomp-id parameter, if so,
       we will set the enumerate value or the string value */
    if (RV_TRUE == urlParams->isSigCompIdParam)
    {
        RvInt32 offsetSigCompIdParam = UNDEFINED;
		rv = ParserAllocFromObjPage(pParserMgr,
			&offsetSigCompIdParam,
			hPool,
			hPage,
			urlParams->sigCompIdParam.buf,
			urlParams->sigCompIdParam.len);
		
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitUrlParams - error in ParserAllocFromObjPage"));
			return rv;
		}
        
        rv = SipAddrUrlSetSigCompIdParam(hUrl,
										NULL,
										hPool,
										hPage,
                                       offsetSigCompIdParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitUrlParams - error in SipAddrUrlSetSigCompIdParam"));
            return rv;
        }

    }/*if (RV_TRUE == urlParams->isSigCompIdParam) */
	
	/* Check whether the there is TokenizedBy parameter, if so,
       we will set the string value */
    if (RV_TRUE == urlParams->isTokenizedByParam)
    {
        RvInt32 offsetTokenizedByParam = UNDEFINED;
        rv = ParserAllocFromObjPage(pParserMgr,
                                       &offsetTokenizedByParam,
                                       hPool,
                                       hPage,
                                       urlParams->tokenizedByParam.buf,
                                       urlParams->tokenizedByParam.len);
        if (RV_OK != rv)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitUrlParams - error in ParserAllocFromObjPage"));
             return rv;
        }
        rv = SipAddrUrlSetTokenizedByParam(hUrl,
                                        NULL,
                                        hPool,
                                        hPage,
                                        offsetTokenizedByParam);
        if (RV_OK != rv)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitUrlParams - error in AddrUrlSetTokenizedByParam"));
            return rv;
        }
    }

	/* Check whether the there is Orig parameter, if so,
       we will set the value RV_TRUE */
	if (RV_TRUE == urlParams->bOrigParam)
    {
		rv = RvSipAddrUrlSetOrigParam(hUrl, RV_TRUE);
		if (RV_OK != rv)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitUrlParams - error in RvSipAddrUrlSetOrigParam"));
            return rv;
        }
	}

    /* If the url params contains other parameters then concatenate them into the
       page message*/
    if (RV_TRUE==urlParams->isOtherParams)
    {
        RvInt32 offsetOtherParam = UNDEFINED;
        offsetOtherParam = ParserAppendCopyRpoolString(
                    pParserMgr,
                    hPool,
                    hPage,
                    ((ParserExtensionString *)urlParams->genericParamList)->hPool,
                    ((ParserExtensionString *)urlParams->genericParamList)->hPage,
                    0,
                    ((ParserExtensionString *)urlParams->genericParamList)->size);

        if (UNDEFINED == offsetOtherParam)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitUrlParams - error in ParserAppendCopyRpoolString"));
             return RV_ERROR_OUTOFRESOURCES;
        }
        rv = SipAddrUrlSetUrlParams(hUrl,
                                       NULL,
                                       hPool,
                                       hPage,
                                       offsetOtherParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitUrlParams - error in PartyHeaderSetOtherParams"));
            return rv;
        }
    } /* if (RV_TRUE == urlParams->isOtherParams )*/

#ifdef RV_SIP_IMS_HEADER_SUPPORT
    /* Check whether the there is CPC parameter, if so,
       we will set the string value */
    if (RV_TRUE == urlParams->isCpcParam)
    {
        RvInt32 offsetCPCParam = UNDEFINED;

        if(urlParams->cpcParam.cpcType == RVSIP_CPC_TYPE_OTHER)
        {
        
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetCPCParam,
                                           hPool,
                                           hPage,
                                           urlParams->cpcParam.strCpcParam.buf,
                                           urlParams->cpcParam.strCpcParam.len);
            if (RV_OK != rv)
            {
                 RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                   "ParserInitUrlParams - error in ParserAllocFromObjPage"));
                 return rv;
            }
        }

        rv = SipAddrUrlSetCPC(  hUrl,
                                urlParams->cpcParam.cpcType,
                                NULL,
                                hPool,
                                hPage,
                                offsetCPCParam);
        if (RV_OK != rv)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitUrlParams - error in AddrUrlSetCPC"));
            return rv;
        }
    }

    /* Check whether the there is Gr parameter, if so,
       we will set the string and boolean value */
	if (RV_TRUE == urlParams->isGrParam)
    {
        RvInt32 offsetGrParam = UNDEFINED;
        if(urlParams->grParam.bGrParam)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                       &offsetGrParam,
                                        hPool,
                                        hPage,
                                        urlParams->grParam.strGrParam.buf,
                                        urlParams->grParam.strGrParam.len);
            if (RV_OK != rv)
            {
                 RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                   "ParserInitUrlParams - error in ParserAllocFromObjPage"));
                 return rv;
            }
        }

		rv = SipAddrUrlSetGr(hUrl, 
                             RV_TRUE, 
                             NULL,
                             hPool,
                             hPage,
                             offsetGrParam);
		if (RV_OK != rv)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitUrlParams - error in RvSipAddrUrlSetGrParam"));
            return rv;
        }
	}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    return RV_OK;
}

/***************************************************************************
 * ParserInitUserInfoInUrl
 * ------------------------------------------------------------------------
 * General: This function is used to init the user-info in Url with the
 *          url values from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:      pParserMgr - Pointer to the parser manager.
 *              url        - The url parameters from the parser.
 *              hPool      - Handle to the message pool.
 *              hPage      - Handle to the message page.
 *                hUrl - Handle to the address structure which will be
 *                     set by the parser.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitUserInfoInUrl(
                          IN    ParserMgr          *pParserMgr,
                          IN    ParserSipUrl       *url,
                          IN    HRPOOL             hPool,
                          IN    HPAGE              hPage,
                          IN    RvSipAddressHandle hUrl)
{
    RvStatus     rv;
    RvInt32      offsetUser = UNDEFINED;
    RvInt32      offset = UNDEFINED;

    /* Check whether there is user name in the url and set it */
    if (url->bIsUserInfo == RV_FALSE)
    {
        return RV_OK;
    }

    /* set the user info string on the ,message page */
    rv = ParserAppendFromExternal(pParserMgr,
                                   &offsetUser,
                                   hPool,
                                   hPage,
                                   url->urlUserInfo.userName.buf,
                                   url->urlUserInfo.userName.len);
    if (RV_OK!= rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitUserInfoInUrl - error in ParserAppendFromExternal"));
        return rv;
    }


    /* Check whether there is a password, if so concatenate it to the user
       with ':' between */
    if (RV_TRUE == url->urlUserInfo.isPassword)
    {
        /* append ":" between the user name and the password*/
        rv = ParserAppendFromExternal( pParserMgr,
                                       &offset,
                                       hPool,
                                       hPage,
                                       ":",
                                       1);
        if (RV_OK!= rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitUserInfoInUrl - error in ParserAppendFromExternal"));
            return rv;
        }
        /* append the password in the message page (with null terminator)*/
		rv = ParserAllocFromObjPage(pParserMgr,
                                       &offset,
                                       hPool,
                                       hPage,
                                       url->urlUserInfo.password.buf,
                                       url->urlUserInfo.password.len);
		
        if (RV_OK!= rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitUserInfoInUrl - error in ParserAllocFromObjPage"));
            return rv;
		}
    }
    else
    {
        rv = ParserAppendFromExternal( pParserMgr,
                                       &offset,
                                       hPool,
                                       hPage,
                                       "\0",
                                       1);
        if (RV_OK!= rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitUserInfoInUrl - error in ParserAppendFromExternal"));
            return rv;
        }
    }

    /* Set user name in the Url */
    rv = SipAddrUrlSetUser(hUrl,
                              NULL,
                              hPool,
                              hPage,
                              offsetUser);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
          "ParserInitUserInfoInUrl - error in AddrUrlSetUser"));
        return rv;
    }
    return RV_OK;
}
/***************************************************************************
 * ParserInitHostPortInUrl
 * ------------------------------------------------------------------------
 * General: This function is used to init host and port in Url with the
 *          url values from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:      pParserMgr  - Pointer to the parser manager.
 *              url  - The url parameters from the parser.
 *              hPool      - Handle to the message pool.
 *              hPage      - Handle to the message page.
 *    In/OutPut:  hUrl - Handle to the address structure which will be
 *                     set by the parser.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitHostPortInUrl(
                          IN    ParserMgr          *pParserMgr,
                          IN    ParserSipUrl       *url,
                          IN    HRPOOL             hPool,
                          IN    HPAGE              hPage,
                          INOUT RvSipAddressHandle hUrl)
{
    RvStatus             rv;
    RvInt32              offsetHost = UNDEFINED;

    /* Change the token host into a string host*/
    rv =  ParserAllocFromObjPage(pParserMgr,
                                    &offsetHost,
                                    hPool,
                                    hPage,
                                    url->urlHost.hostName.buf,
                                    url->urlHost.hostName.len);
    if (RV_OK != rv)
    {
         RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitHostPortInUrl - error in ParserAllocFromObjPage"));
         return rv;
    }

    /* Set host name string in the Url handle*/
    rv = SipAddrUrlSetHost(hUrl,
                              NULL,
                              hPool,
                              hPage,
                              offsetHost);
    if (RV_OK != rv)
    {
         RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitHostPortInUrl - error in AddrUrlSetHost"));
         return rv;
    }

    /* Check whether there is a port, if so set it in the Url */
    if (RV_TRUE == url->urlHost.isPort)
    {
        RvInt32 port;

        /* Change the token port into a number*/
        rv = ParserGetINT32FromString(pParserMgr,
                                         url->urlHost.port.buf ,
                                         url->urlHost.port.len,
                                         &port);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitHostPortInUrl - error in ParserGetINT32FromString"));
            return rv;
        }

        /* Set the port number into the url*/
        rv = RvSipAddrUrlSetPortNum(hUrl,port);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitHostPortInUrl - error in RvSipAddrUrlSetPortNum"));
            return rv;
        }
	}
	return RV_OK;
}

#ifdef RV_SIP_JSR32_SUPPORT
/***************************************************************************
 * ParserInitDisplayNameInAddr
 * ------------------------------------------------------------------------
 * General: This function is used to set a display name into an address object.
 * Return Value: RV_OK                   - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:    pParserMgr  - Pointer to the parser manager.
 *              hAddress    - Handle to the address object
 *              pcb         - The pcb pointer
 *              hPool       - Handle to the message pool.
 *              hPage       - Handle to the message page.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitDisplayNameInAddr(
						  IN    ParserMgr             *pParserMgr,
						  IN    RvSipAddressHandle    hAddress,
                          IN    SipParser_pcb_type    *pcb,
						  IN    HRPOOL                hPool,
                          IN    HPAGE                 hPage)
{
	RvStatus rv;
	RvInt32  offset = UNDEFINED;

	if (NULL == pcb)
	{
		/* There is no display name to set */
		return RV_OK;
	}

	if (RV_TRUE == pcb->nameAddr.isDisplayName)
    {
        rv =  ParserAllocFromObjPage(pParserMgr,
                                        &offset,
                                        hPool,
                                        hPage,
                                        pcb->nameAddr.name.buf,
                                        pcb->nameAddr.name.len);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitDisplayNameInAddr - error in ParserAllocFromObjPage"));
            return rv;
        }
        rv = SipAddrSetDisplayName(hAddress,
                                     NULL,
                                     hPool,
                                     hPage,
                                     offset);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitDisplayNameInAddr - error in SipAddrSetDisplayName"));
            return rv;
        }
    }

	return RV_OK;
}
#endif

/***************************************************************************
 * ParserAddrType2MsgAddrType
 * ------------------------------------------------------------------------
 * General: Converts parser address type to message address type.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 ***************************************************************************/
static RvSipAddressType ParserAddrType2MsgAddrType(ParserTypeAddr eParserType)
{
    switch(eParserType)
    {
    case PARSER_ADDR_PARAM_TYPE_SIP_URL:
        return RVSIP_ADDRTYPE_URL;
    case PARSER_ADDR_PARAM_TYPE_PRES_URI:
        return RVSIP_ADDRTYPE_PRES;
    case PARSER_ADDR_PARAM_TYPE_IM_URI:
        return RVSIP_ADDRTYPE_IM;
    case PARSER_ADDR_PARAM_TYPE_TEL_URI:
        return RVSIP_ADDRTYPE_TEL;
    case PARSER_ADDR_PARAM_TYPE_ABS_URI:
        return RVSIP_ADDRTYPE_ABS_URI;
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    case PARSER_ADDR_PARAM_TYPE_DIAMETER_URI:
        return RVSIP_ADDRTYPE_DIAMETER;
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    default:
        return RVSIP_ADDRTYPE_UNDEFINED;
    }
}

/***************************************************************************
 * ParserInitAddressInHeader
 * ------------------------------------------------------------------------
 * General: This function is used to init an address handle with the
 *          url values from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:    pParserMgr  - Pointer to the parser manager.
 *              pcb  - The pcb pointer
 *              url  - The url parameters from the parser.
 *              hPool      - Handle to the message pool.
 *              hPage      - Handle to the message page.
 *    In/OutPut:  hUrl - Handle to the address structure which will be
 *                  set by the parser.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitAddressInHeader(IN ParserMgr         *pParserMgr,
                                                     IN ParserExUri       *exUri,
                                                     IN HRPOOL             hPool,
                                                     IN HPAGE              hPage,
                                                     IN RvSipAddressType   eAddrType,
                                                     IN RvSipAddressHandle hAddr)
{
    switch(eAddrType)
    {
    case RVSIP_ADDRTYPE_URL:
#ifdef RV_SIP_OTHER_URI_SUPPORT
    case RVSIP_ADDRTYPE_PRES:
    case RVSIP_ADDRTYPE_IM:
#endif /*#ifdef RV_SIP_OTHER_URI_SUPPORT*/
        return ParserInitUrlInHeader(pParserMgr,
                                        NULL,
                                        &exUri->ExUriInfo.SipUrl,
                                        hPool,
                                        hPage,
                                        hAddr);
#ifdef RV_SIP_TEL_URI_SUPPORT
    case RVSIP_ADDRTYPE_TEL:
        return ParserInitTelUriInHeader(pParserMgr,
                                        NULL,
                                        &exUri->ExUriInfo.telUri,
                                        hPool,
                                        hPage,
                                        hAddr);
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */
    case RVSIP_ADDRTYPE_ABS_URI:
        return ParserInitAbsUriInHeader(pParserMgr,
                                        NULL,
                                        &exUri->ExUriInfo.absUri,
                                        hPool,
                                        hPage,
                                        hAddr);
#ifdef RV_SIP_IMS_HEADER_SUPPORT
    case RVSIP_ADDRTYPE_DIAMETER:
        return ParserInitDiameterUriInHeader(pParserMgr,
                                        NULL,
                                        &exUri->ExUriInfo.diameterUri,
                                        hPool,
                                        hPage,
                                        hAddr);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    default:
        return RV_ERROR_UNKNOWN;
    }
}

/***************************************************************************
 * ParserInitUrlInHeader
 * ------------------------------------------------------------------------
 * General: This function is used to init a sip url handle with the
 *          url values from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:    pParserMgr  - Pointer to the parser manager.
 *              pcb  - The pcb pointer
 *              url  - The url parameters from the parser.
 *              hPool      - Handle to the message pool.
 *              hPage      - Handle to the message page.
 *    In/OutPut:  hUrl - Handle to the address structure which will be
 *                     set by the parser.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitUrlInHeader(
                          IN    ParserMgr             *pParserMgr,
                          IN    SipParser_pcb_type    *pcb,
						  IN    ParserSipUrl          *url,
                          IN    HRPOOL                hPool,
                          IN    HPAGE                 hPage,
                          INOUT RvSipAddressHandle    hUrl)
{
    RvStatus          rv;

    RvSipAddrUrlSetIsSecure(hUrl, url->bIsSecure);
    
    RvSipAddrUrlSetOldAddrSpec(hUrl, url->bOldAddrSpec);

    /* Init the user-info in the Url handle */
    rv = ParserInitUserInfoInUrl(pParserMgr, url, hPool, hPage, hUrl);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitUrlInHeader - error in ParserInitUserInfoInUrl"));
        return rv;
    }

    /* Init the Host and Port in the Url handle */
    rv = ParserInitHostPortInUrl(pParserMgr, url, hPool, hPage, hUrl);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitUrlInHeader - error in ParserInitHostPortInUrl"));
        return rv;
    }

    /* Check whether this url is in old addr-spec format as defined in RFC 822,
       if so, only user and host should be initialized */
    if(url->bOldAddrSpec == RV_TRUE)
    {
        return RV_OK;
    }

    /* Check whether there is url parameters in the url from the parser, if so,
       init the url handle with this parameters */
    if (RV_TRUE == url->urlParameters.isValid)
    {
        rv = ParserInitUrlParams(pParserMgr, &url->urlParameters, hPool, hPage, hUrl);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitUrlInHeader - error in ParserInitUrlParams"));
            return rv;
        }
    }

    /* Check whether there is url headers in the url from the parser, if so,
       init the url handle with this headers */
    if (RV_TRUE == url->optionalHeaders.isSpecified)
    {
        RvInt32 offsetOptionalHeaders = UNDEFINED;
        rv =  ParserAllocFromObjPage(pParserMgr,
                                        &offsetOptionalHeaders,
                                        hPool,
                                        hPage,
                                        url->optionalHeaders.body.buf,
                                        url->optionalHeaders.body.len);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitUrlInHeader - error in ParserAllocFromObjPage"));
            return rv;
        }
        rv = SipAddrUrlSetHeaders(hUrl,
                                     NULL,
                                     hPool,
                                     hPage,
                                     offsetOptionalHeaders);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitUrlInHeader - error in AddrUrlSetHeaders"));
            return rv;
        }
    }

#ifdef RV_SIP_JSR32_SUPPORT
	rv = ParserInitDisplayNameInAddr(pParserMgr, hUrl, pcb, hPool, hPage);
	if (RV_OK != rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				   "ParserInitUrlInHeader - error in ParserInitDisplayNameInAddr"));
		return rv;
	}
#else
	RV_UNUSED_ARG(pcb);
#endif

    return RV_OK;
}

/***************************************************************************
 * ParserInitAbsUriInHeader
 * ------------------------------------------------------------------------
 * General: This function is used to init an address handle with the
 *          abs-uri values from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:      pParserMgr  - Pointer to the parser manager.
 *              uri  - The abs-uri parameters from the parser.
 *              hPool      - Handle to the message pool.
 *              hPage      - Handle to the message page.
 *    In/OutPut:  hUri - Handle to the address structure which will be
 *                     set by the parser.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitAbsUriInHeader(
                          IN    ParserMgr             *pParserMgr,
                          IN    SipParser_pcb_type    *pcb,
						  IN    ParserAbsoluteUri     *uri,
                          IN    HRPOOL                hPool,
                          IN    HPAGE                 hPage,
                          INOUT RvSipAddressHandle    hUri)
{
    RvStatus          rv;

    /* Check whether there is scheme in the abs-uri and set it */
    if (uri->scheme.len > 0)
    {
        RvInt32 offsetScheme = UNDEFINED;
        rv = ParserAllocFromObjPage(
                                   pParserMgr,
                                   &offsetScheme,
                                   hPool,
                                   hPage,
                                   uri->scheme.buf,
                                   uri->scheme.len);


        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitAbsUriInHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipAddrAbsUriSetScheme(hUri,
                                       NULL,
                                       hPool,
                                       hPage,
                                       offsetScheme);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitAbsUriInHeader - error in SipAddrAbsUriSetScheme"));
            return rv;
        }
    }

    /* Check whether there is Identifier in the abs-uri and set it */
    if (uri->idefntifier.len > 0)
    {
        RvInt32 offsetIdentifier = UNDEFINED;
        rv = ParserAllocFromObjPage(
                                   pParserMgr,
                                   &offsetIdentifier,
                                   hPool,
                                   hPage,
                                   uri->idefntifier.buf,
                                   uri->idefntifier.len);


        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitAbsUriInHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipAddrAbsUriSetIdentifier(hUri,
                                           NULL,
                                           hPool,
                                           hPage,
                                           offsetIdentifier);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitAbsUriInHeader - error in SipAddrAbsUriSetIdentifier"));
            return rv;
        }
    }

#ifdef RV_SIP_JSR32_SUPPORT
	rv = ParserInitDisplayNameInAddr(pParserMgr, hUri, pcb, hPool, hPage);
	if (RV_OK != rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			"ParserInitAbsUriInHeader - error in ParserInitDisplayNameInAddr"));
		return rv;
	}
#else
	RV_UNUSED_ARG(pcb);
#endif

    return RV_OK;
}

#ifdef RV_SIP_TEL_URI_SUPPORT
/***************************************************************************
 * ParserInitTelUriInHeader
 * ------------------------------------------------------------------------
 * General: This function is used to init an address handle with the
 *          tel-uri values from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:    pParserMgr  - Pointer to the parser manager.
 *              uri         - The tel-uri parameters from the parser.
 *              hPool      - Handle to the message pool.
 *              hPage      - Handle to the message page.
 *    In/OutPut:  hUri - Handle to the address structure which will be
 *                       set by the parser.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitTelUriInHeader(
                          IN    ParserMgr             *pParserMgr,
                          IN    SipParser_pcb_type    *pcb,
						  IN    ParserTelUri          *uri,
                          IN    HRPOOL                hPool,
                          IN    HPAGE                 hPage,
                          INOUT RvSipAddressHandle    hUri)
{
    RvStatus          rv;

	rv = RvSipAddrTelUriSetIsGlobalPhoneNumber(hUri, uri->bIsGlobalPhoneNumber);
	if (RV_OK != rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			"ParserInitTelUriInHeader - error in RvSipAddrTelUriSetIsGlobalPhoneNumber"));
		return rv;
	}

    /* set phone-number */
    if (uri->strPhoneNumber.bIsSpecified == RV_TRUE)
    {
        RvInt32 offsetPhoneNum = UNDEFINED;
        rv = ParserAllocFromObjPage(
                                   pParserMgr,
                                   &offsetPhoneNum,
                                   hPool,
                                   hPage,
                                   uri->strPhoneNumber.strToken.buf,
                                   uri->strPhoneNumber.strToken.len);


        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitTelUriInHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipAddrTelUriSetPhoneNumber(hUri,
                                            NULL,
                                            hPool,
                                            hPage,
                                            offsetPhoneNum);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                       "ParserInitTelUriInHeader - error in SipAddrTelUriSetPhoneNumber"));
            return rv;
        }
    }

	/* set context */
    if (uri->strContext.bIsSpecified == RV_TRUE)
    {
        RvInt32 offsetContext = UNDEFINED;
        rv = ParserAllocFromObjPage(
									pParserMgr,
									&offsetContext,
									hPool,
									hPage,
									uri->strContext.strToken.buf,
									uri->strContext.strToken.len);


        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitTelUriInHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipAddrTelUriSetContext(hUri,
										NULL,
										hPool,
										hPage,
										offsetContext);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitTelUriInHeader - error in SipAddrTelUriSetContext"));
            return rv;
        }
    }
	else if (uri->bIsGlobalPhoneNumber == RV_FALSE)
	{
		/* local phone number must contain context parameter */
		RvLogWarning(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			"ParserInitTelUriInHeader - missing context parameter which is mandatory for local numbers"));
	}

    /* set extension */
    if (uri->strExtension.bIsSpecified == RV_TRUE)
    {
        RvInt32 offsetExtension = UNDEFINED;
        rv = ParserAllocFromObjPage(
									pParserMgr,
									&offsetExtension,
									hPool,
									hPage,
									uri->strExtension.strToken.buf,
									uri->strExtension.strToken.len);


        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitTelUriInHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipAddrTelUriSetExtension(hUri,
										NULL,
										hPool,
										hPage,
										offsetExtension);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitTelUriInHeader - error in SipAddrTelUriSetExtension"));
            return rv;
        }
    }

	/* set post-dial */
    if (uri->strPostDial.bIsSpecified == RV_TRUE)
    {
        RvInt32 offsetPostDial = UNDEFINED;
        rv = ParserAllocFromObjPage(
									pParserMgr,
									&offsetPostDial,
									hPool,
									hPage,
									uri->strPostDial.strToken.buf,
									uri->strPostDial.strToken.len);


        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitTelUriInHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipAddrTelUriSetPostDial(hUri,
										NULL,
										hPool,
										hPage,
										offsetPostDial);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitTelUriInHeader - error in SipAddrTelUriSetPostDial"));
            return rv;
        }
    }

	/* set isdn-sub-addr */
    if (uri->strIsdnSubAddr.bIsSpecified == RV_TRUE)
    {
        RvInt32 offsetIsdnSubAddr = UNDEFINED;
        rv = ParserAllocFromObjPage(
									pParserMgr,
									&offsetIsdnSubAddr,
									hPool,
									hPage,
									uri->strIsdnSubAddr.strToken.buf,
									uri->strIsdnSubAddr.strToken.len);


        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitTelUriInHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipAddrTelUriSetIsdnSubAddr(hUri,
											NULL,
											hPool,
											hPage,
											offsetIsdnSubAddr);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitTelUriInHeader - error in SipAddrTelUriSetIsdnSubAddr"));
            return rv;
        }
    }

	/* set enumdi */
	switch(uri->eEnumdiType)
    {
    case ParserEnumdiParamEmpty:
        rv = RvSipAddrTelUriSetEnumdiParamType(hUri, RVSIP_ENUMDI_TYPE_EXISTS_EMPTY);
        break;
    default:
        rv = RV_OK;
        break;
    }
    if (RV_OK != rv)
    {
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				   "ParserInitTelUriInHeader - error in RvSipAddrTelUriSetEnumdiParamType"));
        return rv;
    }

	/* set other-params */
    if (uri->strOtherParams.bIsSpecified == RV_TRUE)
    {
	    RvInt32 offsetOtherParam = UNDEFINED;
        offsetOtherParam = ParserAppendCopyRpoolString(
									pParserMgr,
									hPool,
									hPage,
									((ParserExtensionString *)uri->strOtherParams.Params)->hPool,
									((ParserExtensionString *)uri->strOtherParams.Params)->hPage,
									0,
									((ParserExtensionString *)uri->strOtherParams.Params)->size);

       if (UNDEFINED == offsetOtherParam)
        {
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitTelUriInHeader - error in ParserAppendCopyRpoolString"));
			return RV_ERROR_OUTOFRESOURCES;
        }
        rv = SipAddrTelUriSetOtherParams(hUri,
											NULL,
											hPool,
											hPage,
											offsetOtherParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitTelUriInHeader - error in SipAddrTelUriSetOtherParams"));
            return rv;
        }
    }

#ifdef RV_SIP_JSR32_SUPPORT
	rv = ParserInitDisplayNameInAddr(pParserMgr, hUri, pcb, hPool, hPage);
	if (RV_OK != rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			"ParserInitTelUriInHeader - error in ParserInitDisplayNameInAddr"));
		return rv;
	}
#else
	RV_UNUSED_ARG(pcb);
#endif

#ifdef RV_SIP_IMS_HEADER_SUPPORT
    /* Check whether there is CPC parameter, if so,
       set the string value */
    if (RV_TRUE == uri->isCpcParam)
    {
        RvInt32 offsetCPCParam = UNDEFINED;

        if(uri->cpcParam.cpcType == RVSIP_CPC_TYPE_OTHER)
        {
        
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetCPCParam,
                                           hPool,
                                           hPage,
                                           uri->cpcParam.strCpcParam.buf,
                                           uri->cpcParam.strCpcParam.len);
            if (RV_OK != rv)
            {
                 RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                   "ParserInitTelUriInHeader - error in ParserAllocFromObjPage"));
                 return rv;
            }
        }

        rv = SipAddrTelUriSetCPC(hUri,
                                 uri->cpcParam.cpcType,
                                 NULL,
                                 hPool,
                                 hPage,
                                 offsetCPCParam);
        if (RV_OK != rv)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitTelUriInHeader - error in AddrUrlSetCPC"));
            return rv;
        }
    }

    /* set Rn */
    if (uri->strRn.bIsSpecified == RV_TRUE)
    {
        RvInt32 offsetRn = UNDEFINED;
        rv = ParserAllocFromObjPage(pParserMgr,
									&offsetRn,
									hPool,
									hPage,
									uri->strRn.strToken.buf,
									uri->strRn.strToken.len);


        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitTelUriInHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipAddrTelUriSetRn(hUri,
								NULL,
								hPool,
								hPage,
								offsetRn);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitTelUriInHeader - error in SipAddrTelUriSetRn"));
            return rv;
        }
    }

    /* set RnContext */
    if (uri->strRnContext.bIsSpecified == RV_TRUE)
    {
        RvInt32 offsetRnContext = UNDEFINED;
        rv = ParserAllocFromObjPage(pParserMgr,
									&offsetRnContext,
									hPool,
									hPage,
									uri->strRnContext.strToken.buf,
									uri->strRnContext.strToken.len);


        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitTelUriInHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipAddrTelUriSetRnContext( hUri,
								        NULL,
								        hPool,
								        hPage,
								        offsetRnContext);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitTelUriInHeader - error in SipAddrTelUriSetRnContext"));
            return rv;
        }
    }

    /* set Cic */
    if (uri->strCic.bIsSpecified == RV_TRUE)
    {
        RvInt32 offsetCic = UNDEFINED;
        rv = ParserAllocFromObjPage(pParserMgr,
									&offsetCic,
									hPool,
									hPage,
									uri->strCic.strToken.buf,
									uri->strCic.strToken.len);


        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitTelUriInHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipAddrTelUriSetCic(hUri,
								 NULL,
								 hPool,
								 hPage,
								 offsetCic);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitTelUriInHeader - error in SipAddrTelUriSetCic"));
            return rv;
        }
    }

    /* set CicContext */
    if (uri->strCicContext.bIsSpecified == RV_TRUE)
    {
        RvInt32 offsetCicContext = UNDEFINED;
        rv = ParserAllocFromObjPage(pParserMgr,
									&offsetCicContext,
									hPool,
									hPage,
									uri->strCicContext.strToken.buf,
									uri->strCicContext.strToken.len);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitTelUriInHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipAddrTelUriSetCicContext(hUri,
								        NULL,
								        hPool,
								        hPage,
								        offsetCicContext);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitTelUriInHeader - error in SipAddrTelUriSetCicContext"));
            return rv;
        }
    }

    /* set Npdi */
    if(uri->bNpdi == RV_TRUE)
    {
        rv = RvSipAddrTelUriSetNpdi(hUri, RV_TRUE);
	    if (RV_OK != rv)
	    {
		    RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			    "ParserInitTelUriInHeader - error in RvSipAddrTelUriSetNpdi"));
		    return rv;
	    }
    }
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

    return RV_OK;
}
#endif /* #ifdef RV_SIP_TEL_URI_SUPPORT */
#ifdef RV_SIP_IMS_HEADER_SUPPORT
/***************************************************************************
 * ParserInitDiameterUri
 * ------------------------------------------------------------------------
 * General: This function is used to init a diameter-uri handle with the
 *          uri values from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:    pParserMgr  - Pointer to the parser manager.
 *              uri     - The uri parameters from the parser.
 *              eType   - This type indicates that the parser is called to
 *                        parse stand alone abs-uri.
 *    In/OutPut:hUri - Handle to the address structure which will be
 *                     set by the parser.
 ***************************************************************************/
RvStatus RVCALLCONV ParserInitDiameterUri(
                          IN    ParserMgr           *pParserMgr,
                          IN    SipParser_pcb_type  *pcb,
                          IN    ParserDiameterUri   *uri,
                          IN    SipParseType        eType,
                          INOUT const void          *hUri)
{
    HRPOOL hPool;
    HPAGE  hPage;

#ifdef RV_SIP_JSR32_SUPPORT
	if (SIP_PARSETYPE_ANYADDRESS == eType)
	{
		/* This is a special case where the parse was done only to determine
		   the type of the address and not to initialize an address structure */
		RvSipAddressType* peAddrType = (RvSipAddressType*)hUri;
		*peAddrType = ParserAddrType2MsgAddrType(pcb->exUri.uriType);
		return RV_OK;
	}
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

    /* Check that we get here only if we parse stand alone header */
    if (SIP_PARSETYPE_DIAMETER_URI_ADDRESS == eType)
    {
        /* The parser is used to parse uri stand alone header */
        hUri=(RvSipAddressHandle)hUri;
    }
    else
    {
        RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                     "ParserInitDiameterUri - Illegal buffer for this type of header: %d",eType));
        return RV_ERROR_UNKNOWN;
    }

    hPool = SipAddrGetPool(hUri);
    hPage = SipAddrGetPage(hUri);

    return ParserInitDiameterUriInHeader(pParserMgr, pcb, uri,hPool, hPage, (RvSipAddressHandle)hUri);
}

/***************************************************************************
 * ParserInitDiameterUriInHeader
 * ------------------------------------------------------------------------
 * General: This function is used to init a diameter uri handle with the
 *          uri values from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:    pParserMgr  - Pointer to the parser manager.
 *              pcb  - The pcb pointer
 *              uri  - The uri parameters from the parser.
 *              hPool      - Handle to the message pool.
 *              hPage      - Handle to the message page.
 *    In/OutPut:  hUri - Handle to the address structure which will be
 *                     set by the parser.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitDiameterUriInHeader(
                          IN    ParserMgr             *pParserMgr,
                          IN    SipParser_pcb_type    *pcb,
						  IN    ParserDiameterUri     *uri,
                          IN    HRPOOL                hPool,
                          IN    HPAGE                 hPage,
                          INOUT RvSipAddressHandle    hUri)
{
    RvStatus    rv;
    RvInt32     offsetHost = UNDEFINED;
    
    RvSipAddrDiameterUriSetIsSecure(hUri, uri->bIsSecure);

    /* Init the Host and Port in the Uri handle */
    /* Change the token host into a string host */
    rv =  ParserAllocFromObjPage(pParserMgr,
                                    &offsetHost,
                                    hPool,
                                    hPage,
                                    uri->uriHostPort.hostName.buf,
                                    uri->uriHostPort.hostName.len);  
    if (RV_OK != rv)
    {
         RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitDiameterUriInHeader - error in ParserAllocFromObjPage"));
         return rv;
    }

    /* Set host name string in the Uri handle */
    rv = SipAddrDiameterUriSetHost( hUri,
                                    NULL,
                                    hPool,
                                    hPage,
                                    offsetHost);
    if (RV_OK != rv)
    {
         RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitDiameterUriInHeader - error in SipAddrDiameterUriSetHost"));
         return rv;
    }

    /* Check whether there is a port, if so set it in the Uri */
    if (RV_TRUE == uri->uriHostPort.isPort)
    {
        RvInt32 port;

        /* Change the token port into a number */
        rv = ParserGetINT32FromString(pParserMgr,
                                         uri->uriHostPort.port.buf ,
                                         uri->uriHostPort.port.len,
                                         &port);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitDiameterUriInHeader - error in ParserGetINT32FromString"));
            return rv;
        }
 
        /* Set the port number into the uri */
        rv = RvSipAddrDiameterUriSetPortNum(hUri, port);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitDiameterUriInHeader - error in RvSipAddrDiameterUriSetPortNum"));
            return rv;
        }
	}

    /* Check whether the there is transport parameter, if so,
       we will set the enumerate value or the string value */
    if (RV_TRUE == uri->isTransport)
    {
        RvInt32 offsetTransport = UNDEFINED;
        
        /* For all transport types we also save the transport string */
        rv = ParserAllocFromObjPage(pParserMgr,
                                       &offsetTransport,
                                       hPool,
                                       hPage,
                                       uri->transport.strTransport.buf,
                                       uri->transport.strTransport.len);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitDiameterUriInHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipAddrDiameterUriSetTransport(hUri,
                                              uri->transport.transport,
                                              NULL,
                                              hPool,
                                              hPage,
                                              offsetTransport);

        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitDiameterUriInHeader - error in SipAddrDiameterUriSetTransport"));
            return rv;
        }

    } /* if (RV_TRUE == uri->isTransport) */

    /* Set Protocol param */
    if (RV_TRUE == uri->isProtocol)
    {
        RvSipAddrDiameterUriSetProtocolType(hUri, uri->eProtocolType);
    }

    /* Set Other Params */
    if (uri->isOtherParams == RV_TRUE)
    {
	    RvInt32 offsetOtherParam = UNDEFINED;
        offsetOtherParam = ParserAppendCopyRpoolString(
									pParserMgr,
									hPool,
									hPage,
									((ParserExtensionString *)uri->otherParams)->hPool,
									((ParserExtensionString *)uri->otherParams)->hPage,
									0,
									((ParserExtensionString *)uri->otherParams)->size);

       if (UNDEFINED == offsetOtherParam)
        {
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitDiameterUriInHeader - error in ParserAppendCopyRpoolString"));
			return RV_ERROR_OUTOFRESOURCES;
        }
        rv = SipAddrDiameterUriSetOtherParams(hUri,
											NULL,
											hPool,
											hPage,
											offsetOtherParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitDiameterUriInHeader - error in SipAddrDiameterUriSetOtherParams"));
            return rv;
        }
    }
    
    RV_UNUSED_ARG(pcb);
    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

/***************************************************************************
 * ParserInitAddressInVia
 * ------------------------------------------------------------------------
 * General:This function is static, init the via header with the
 *         sent by values from the via header.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pParserMgr  - Pointer to the parser manager.
 *             pViaAddress - Via address information.
 *              hPool      - Handle to the message pool.
 *              hPage      - Handle to the message page.
 *             hViaHeader  - Handle to the via header.
 ***************************************************************************/
static RvStatus ParserInitAddressInVia(IN    ParserMgr             *pParserMgr,
                              IN    ParserSentByInVia     *pViaAddress,
                              IN    HRPOOL                hPool,
                              IN    HPAGE                 hPage,
                              IN    RvSipViaHeaderHandle  hViaHeader)
{
    RvStatus      rv;
    RvInt32       portNum         = UNDEFINED;
    RvInt32       offsetHost      = UNDEFINED;

    if (RV_TRUE == pViaAddress->isPort)
    {
        rv = ParserGetINT32FromString(pParserMgr,
                                        pViaAddress->port.buf ,
                                        pViaAddress->port.len,
                                        &portNum);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitAddressInVia - error in ParserGetINT32FromString"));
            return rv;
        }
    }

    /* Set The port number from the sent by in the Via header*/
    rv = RvSipViaHeaderSetPortNum(hViaHeader,portNum);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitAddressInVia - error in RvSipViaHeaderSetPortNum"));
        return rv;
    }

    rv = ParserAllocFromObjPage(pParserMgr,
                                &offsetHost,
                                hPool,
                                hPage,
                                pViaAddress->hostName.buf,
                                pViaAddress->hostName.len);
    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitAddressInVia - error in ParserAllocFromObjPage"));
        return rv;
    }

    /*Set the host string value from the sent by into the Via header*/
    rv = SipViaHeaderSetHost(hViaHeader,
                                NULL,
                                hPool,
                                hPage,
                                offsetHost);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitAddressInVia - error in ViaHeaderSetHost"));
        return rv;
    }
    return RV_OK;
}
/***************************************************************************
 * ParserInitSentByInVia
 * ------------------------------------------------------------------------
 * General:This function is used from the parser to init the message with the
 *         sent by values from the via header.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pParserMgr - Pointer to the parser manager.
 *             pVia       - Via  structure holding the Via values from the
 *                          parser in a token form.
 *             hPool      - Handle to the header pool.
 *             hPage      - Handle to the header page.
 *             hViaHeader - Handle to the via header.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitSentByInVia(
                            IN    ParserMgr             *pParserMgr,
                            IN    ParserSingleVia       *pVia,
                            IN    HRPOOL                hPool,
                            IN    HPAGE                 hPage,
                            IN    RvSipViaHeaderHandle  hViaHeader)
{
    RvStatus      rv;
    RvSipTransport eTransport      = RVSIP_TRANSPORT_UNDEFINED;
    RvInt32       offsetTransport = UNDEFINED;

    eTransport = pVia->sentProtocol.transport.transport;
    if (RVSIP_TRANSPORT_OTHER == pVia->sentProtocol.transport.transport)
    {
        rv= ParserAllocFromObjPage(pParserMgr,
                                      &offsetTransport,
                                      hPool,
                                      hPage,
                                      pVia->sentProtocol.transport.strTransport.buf,
                                      pVia->sentProtocol.transport.strTransport.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitSentByInVia - error in ParserAllocFromObjPage"));
            return rv;
        }
    }

    /*Set the transport enumerate and string value from the sent protocol*/
    rv = SipViaHeaderSetTransport(hViaHeader,
                                     eTransport,
                                     NULL,
                                     hPool,
                                     hPage,
                                     offsetTransport);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitSentByInVia - error in ViaHeaderSetTransport"));
        return rv;
    }

    rv = ParserInitAddressInVia(pParserMgr, &(pVia->sentBy), hPool, hPage, hViaHeader);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitSentByInVia - error in ParserInitAddressInVia"));
        return rv;
    }

    return RV_OK;
}


/***************************************************************************
 * ParserInitParamsInVia
 * ------------------------------------------------------------------------
 * General:This function is used from the parser to init the message with the
 *         params values of the via header.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pParserMgr - Pointer to the parser manager.
 *             pVia       - Via  structure holding the Via values from the
 *                          parser in a token form.
 *             hPool      - Handle to the header pool.
 *             hPage      - Handle to the header page.
 *             hViaHeader - Handle to the via header.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitParamsInVia(
                            IN    ParserMgr             *pParserMgr,
                            IN    ParserSingleVia       *pVia,
                            IN    HRPOOL                hPool,
                            IN    HPAGE                 hPage,
                            INOUT RvSipViaHeaderHandle  hViaHeader)
{
    RvStatus            rv;
    RvInt32             offsetViaParams = UNDEFINED;

    /* Check whether the multi address param was set*/
    if (RV_TRUE==pVia->params.isMaddr)
    {
        RvInt32 offsetMaddrParam = UNDEFINED;
        rv= ParserAllocFromObjPage(pParserMgr,
                                      &offsetMaddrParam,
                                      hPool,
                                      hPage,
                                      pVia->params.maddr.buf,
                                      pVia->params.maddr.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitParamsInVia - error in ParserAllocFromObjPage"));
            return rv;
        }
        /* Set the multi address parameter in the Via header */
        rv = SipViaHeaderSetMaddrParam(hViaHeader,
                                          NULL,
                                          hPool,
                                          hPage,
                                          offsetMaddrParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitParamsInVia - error in ViaHeaderSetMaddrParam"));
            return rv;
        }
    }

    /* Check whether the multi address param was set*/
    if (RV_TRUE==pVia->params.isReceived)
    {
        RvInt32 offsetReceivedParam = UNDEFINED;
        rv=ParserAllocFromObjPage(pParserMgr,
                                     &offsetReceivedParam,
                                     hPool,
                                     hPage,
                                     pVia->params.received.buf,
                                     pVia->params.received.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitParamsInVia - error in ParserAllocFromObjPage"));
            return rv;
        }

        /* Set the received parameter in the Via header */
        rv = SipViaHeaderSetReceivedParam(hViaHeader,
                                             NULL,
                                             hPool,
                                             hPage,
                                             offsetReceivedParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitParamsInVia - error in ViaHeaderSetReceivedParam"));
            return rv;
        }
    }

    /* Check whether the branch param was set*/
    if (RV_TRUE==pVia->params.isBranch)
    {
        RvInt32 offsetBranchParam = UNDEFINED;
        rv=ParserAllocFromObjPage(pParserMgr,
                                     &offsetBranchParam,
                                     hPool,
                                     hPage,
                                     pVia->params.branch.buf ,
                                     pVia->params.branch.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitParamsInVia - error in ParserAllocFromObjPage"));
            return rv;
        }
        /* Set the branch parameter in the Via header */
        rv = SipViaHeaderSetBranchParam(hViaHeader,
                                           NULL,
                                           hPool,
                                           hPage,
                                           offsetBranchParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitParamsInVia - error in ViaHeaderSetBranchParam"));
            return rv;
        }
    }

    /* Check whether the ttl param was set*/
    if (RV_TRUE==pVia->params.isTtl)
    {
        RvInt16 ttlNum;
        rv=ParserGetINT16FromString(pParserMgr,
                                       pVia->params.ttl.buf ,
                                       pVia->params.ttl.len,
                                       &ttlNum);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitParamsInVia - error in ParserGetINT16FromString"));
            return rv;
        }

        /* Set the ttl number from Via parameters in the Via header*/
        rv = RvSipViaHeaderSetTtlNum(hViaHeader,ttlNum);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitParamsInVia - error in RvSipViaHeaderSetTtlNum"));
            return rv;
        }
    }

    /* Check whether the hidden param was set*/
    if (RV_TRUE == pVia->params.hidden)
    {

        /* Set the hidden param from Via parameters in the Via header*/
        rv = RvSipViaHeaderSetHiddenParam(hViaHeader, RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitParamsInVia - error in RvSipViaHeaderSetHiddenParam"));
            return rv;
        }
    }

    /* Check whether the alias param was set*/
    if (RV_TRUE == pVia->params.alias)
    {

        /* Set the alias param from Via parameters in the Via header*/
        rv = RvSipViaHeaderSetAliasParam(hViaHeader, RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitParamsInVia - error in RvSipViaHeaderSetAliasParam"));
            return rv;
        }
    }

    if(RV_TRUE == pVia->params.isRport)
    {
        RvInt32    rport;
        if(0 == pVia->params.rPort.len)
        {
            /* rport is undefined */
            rport = UNDEFINED;
        }
        else
        {
            rv=ParserGetINT32FromString(pParserMgr,
                                            pVia->params.rPort.buf ,
                                            pVia->params.rPort.len,
                                            &rport);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitParamsInVia - error in ParserGetINT32FromString (rport)"));
                return rv;
            }
        }

        rv = RvSipViaHeaderSetRportParam(hViaHeader, rport, RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitParamsInVia - error in RvSipViaHeaderSetRportParam"));
            return rv;
        }
    }

/* '#ifdef RV_SIGCOMP_ON' Shouldn't be here since the comp parameter should be referred even 
	    when SIGCOMP feature is turned off */ 
    /* Check whether the comp param was set*/
    if (RV_TRUE==pVia->params.isComp)
    {
        RvInt32 offsetCompParam = UNDEFINED;
        if (RVSIP_COMP_OTHER == pVia->params.comp.compType)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetCompParam,
                                           hPool,
                                           hPage,
                                           pVia->params.comp.strCompParam.buf,
                                           pVia->params.comp.strCompParam.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitParamsInVia - error in ParserAllocFromObjPage"));
                return rv;
            }
        }
        rv = SipViaHeaderSetCompParam(hViaHeader,
                                         pVia->params.comp.compType,
                                         NULL,
                                         hPool,
                                         hPage,
                                         offsetCompParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitParamsInVia - error in SipViaHeaderSetCompParam"));
            return rv;
        }

    }

	/* check if the sigCompId was set */
	if (RV_TRUE == pVia->params.isSigCompId)
	{
		RvInt32 offsetSigCompIdParam = UNDEFINED;
		rv = ParserAllocFromObjPage(pParserMgr,
									&offsetSigCompIdParam,
									hPool,
									hPage,
									pVia->params.sigCompId.buf,
									pVia->params.sigCompId.len);
		if (RV_OK != rv)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitParamsInVia - error in ParserAllocFromObjPage"));
			return rv;
		}
		
		rv = SipViaHeaderSetSigCompIdParam(hViaHeader,
											NULL,
											hPool,
											hPage,
											offsetSigCompIdParam);
		if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitParamsInVia - error in SipViaHeaderSetSigCompIdParam"));
            return rv;
        }
	}

    /* If there are just other params, concatenated it to StrViaParams, which will
       be set in the header*/
    if (RV_TRUE==pVia->params.isOtherParams && NULL!= pVia->params.otherParam)
    {
        ParserExtensionString * pOtherParam  =
            (ParserExtensionString *)pVia->params.otherParam;
        /* There is no need to concatenate the hidden param*/
       offsetViaParams = ParserAppendCopyRpoolString(pParserMgr,
                                               hPool,
                                               hPage,
                                               pOtherParam->hPool,
                                               pOtherParam->hPage,
                                               0,
                                               pOtherParam->size);

        /* append the "NULL_CHAR" in the message page */
        if (UNDEFINED == offsetViaParams)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitParamsInVia - error in ParserAppendCopyRpoolString"));
            return RV_ERROR_OUTOFRESOURCES;
        }
    }


    if (UNDEFINED!=offsetViaParams)
    {
        /*Set the Via param in the Via header*/
        rv = SipViaHeaderSetViaParams(hViaHeader,
                                         NULL,
                                         hPool,
                                         hPage,
                                         offsetViaParams);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitParamsInVia - error in ViaHeaderSetViaParams"));
         return rv;
        }
    }
    return RV_OK;
}

#ifndef RV_SIP_LIGHT
/***************************************************************************
 * ParserInitPartyHeader
 * ------------------------------------------------------------------------
 * General: This function is used from the parser to init the message with the
 *         party (To/From) header values.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pParserMgr  - Pointer to the parser manager.
 *             PartyVal     - Party structure holding the Party values from the
 *                            parser in a token form.
 *             hPool      - Handle to the message pool.
 *             hPage      - Handle to the message page.
 *  In/Output: hPartyHeader - Handle to the Party header which will be filled.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitPartyHeader(
                                    IN    ParserMgr             *pParserMgr,
                                    IN    ParserPartyHeader     *PartyVal,
                                    IN    HRPOOL                hPool,
                                    IN    HPAGE                 hPage,
                                    INOUT RvSipPartyHeaderHandle *hPartyHeader)
{
    RvStatus          rv;
    RvSipAddressHandle hAddress;
    RvSipAddressType eAddrType;

    eAddrType = ParserAddrType2MsgAddrType(PartyVal->nameAddr.exUri.uriType);
    if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitPartyHeader - unknown parserAddressType %d", PartyVal->nameAddr.exUri.uriType));
        return RV_ERROR_UNKNOWN;
    }

    /* Construct the sip-url\abs-uri in the Party header */
    rv = RvSipAddrConstructInPartyHeader(*hPartyHeader,
                                            eAddrType,
                                            &hAddress);

    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitPartyHeader - error in RvSipAddrConstructInPartyHeader"));
        return rv;
    }

    /* Initialize the address values from the parser*/
    rv = ParserInitAddressInHeader(pParserMgr, &PartyVal->nameAddr.exUri,
                                        hPool, hPage, eAddrType, hAddress);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitPartyHeader - error in ParserInitAddressInHeader"));
        return rv;
    }

	/* Check whether the display name has been set, if so attach
       it to the Party header  */
    if (RV_TRUE == PartyVal->nameAddr.isDisplayName)
    {
        RvInt32 offsetName = UNDEFINED;
        rv= ParserAllocFromObjPage(pParserMgr,
                                      &offsetName,
                                      hPool,
                                      hPage,
                                      PartyVal->nameAddr.name.buf ,
                                      PartyVal->nameAddr.name.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitPartyHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

#ifndef RV_SIP_JSR32_SUPPORT
        /* Set the display name in the Party header */
        rv = SipPartyHeaderSetDisplayName(*hPartyHeader,
                                             NULL,
                                             hPool,
                                             hPage,
                                             offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitPartyHeader - error in PartyHeaderSetDisplayName"));
            return rv;
        }
#else
		/* This is the jsr32 case where the display name is part of the address object */
		rv = SipAddrSetDisplayName(hAddress,
                                      NULL,
                                      hPool,
                                      hPage,
                                      offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitPartyHeader - error in SipAddrSetDisplayName"));
            return rv;
        }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    }

    /* Check whether the tag parameter has been set, if so attach
       it to the Party header  */
    if (RV_TRUE == PartyVal->partyParams.isTag)
    {
        RvInt32 offsetTag = UNDEFINED;
        rv= ParserAllocFromObjPage(pParserMgr,
                                      &offsetTag,
                                      hPool,
                                      hPage,
                                      PartyVal->partyParams.tag.buf,
                                      PartyVal->partyParams.tag.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitPartyHeader - error in ParserAllocFromObjPage"));
            return rv;
        }

        /* Set the tag parameter in the Party header */
        rv = SipPartyHeaderSetTag(*hPartyHeader,
                                     NULL,
                                     hPool,
                                     hPage,
                                     offsetTag);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitPartyHeader - error in PartyHeaderSetTag"));
            return rv;
        }
    }

    /* Check whether there are other params, if so, concatenate it to the
       strOtherParams string in the party header*/
    if (RV_TRUE==PartyVal->partyParams.isAddrParams)
    {
        RvInt32 offsetOtherParam = UNDEFINED;
        offsetOtherParam = ParserAppendCopyRpoolString(
                    pParserMgr,
                    hPool,
                    hPage,
                    ((ParserExtensionString *)PartyVal->partyParams.addrParams)->hPool,
                    ((ParserExtensionString *)PartyVal->partyParams.addrParams)->hPage,
                    0,
                    ((ParserExtensionString *)PartyVal->partyParams.addrParams)->size);

        if (UNDEFINED == offsetOtherParam)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitPartyHeader - error in ParserAppendCopyRpoolString"));
             return RV_ERROR_OUTOFRESOURCES;
        }
        rv = SipPartyHeaderSetOtherParams(*hPartyHeader,
                                             NULL,
                                             hPool,
                                             hPage,
                                             offsetOtherParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitPartyHeader - error in PartyHeaderSetOtherParams"));
            return rv;
        }
    }

    /* set compact form */
    rv = RvSipPartyHeaderSetCompactForm(*hPartyHeader, PartyVal->isCompact);
    if (rv != RV_OK)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitPartyHeader - error in setting the compact form"));
        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * ParserInitDateInHeader
 * ------------------------------------------------------------------------
 * General: This function is used to init a sip date handle with the
 *          date values from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:      pParserMgr  - Pointer to the parser manager.
 *              date  - The date parameters from the parser.
 *    In/OutPut:  hDate - Handle to the date structure which will be
 *                      set by the parser.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitDateInHeader(
                          IN    ParserMgr             *pParserMgr,
                          IN    ParserSipDate         *date,
                          INOUT RvSipDateHeaderHandle hDateHeader)
{
    RvStatus rv;

    rv = RvSipDateHeaderSetWeekDay(hDateHeader,
                                      date->eWeekDay);

    if (RV_OK!=rv)
    {
       RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
         "ParserInitDateInHeader - error in RvSipDateHeaderSetWeekDay"));
       return rv;
    }
    /* Set the day from the date */
    rv = RvSipDateHeaderSetDay(hDateHeader,
                                  (RvInt8)date->ddmmyy.day);

    if (RV_OK!=rv)
    {
       RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
         "ParserInitDateInHeader - error in RvSipDateHeaderSetDay"));
       return rv;
    }
    /* Set the month from the date */
    rv = RvSipDateHeaderSetMonth(hDateHeader,
                                    date->ddmmyy.eMonth);

    if (RV_OK!=rv)
    {
       RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
         "ParserInitDateInHeader - error in RvSipDateHeaderSetMonth"));
       return rv;
    }
    /* Set the year from the date*/
    rv = RvSipDateHeaderSetYear(hDateHeader,
                                   (RvInt16)date->ddmmyy.year);

    if (RV_OK!=rv)
    {
       RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
         "ParserInitDateInHeader - error in RvSipDateHeaderSetYear"));
       return rv;
    }
    /* Set the hour from the time*/
    rv = RvSipDateHeaderSetHour(hDateHeader,
                                   (RvInt8)date->time.hour);

    if (RV_OK!=rv)
    {
       RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
         "ParserInitDateInHeader - error in RvSipDateHeaderSetHour"));
       return rv;
    }
    /* Set the minute from the time */
    rv = RvSipDateHeaderSetMinute(hDateHeader,
                                     (RvInt8)date->time.minute);

    if (RV_OK!=rv)
    {
       RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
         "ParserInitDateInHeader - error in RvSipDateHeaderSetMinute"));
       return rv;
    }
    /* Set the second from the time*/
    rv = RvSipDateHeaderSetSecond(hDateHeader,
                                    (RvInt8)date->time.second);

    if (RV_OK!=rv)
    {
       RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
         "ParserInitDateInHeader - error in RvSipDateHeaderSetSecond"));
       return rv;
    }

    /* Set the zone from the time */
    switch(date->time.zone)
    {
    case SIPPARSER_TIME_ZONE_UTC:
        rv = SipDateHeaderSetZone(hDateHeader, MSG_DATE_TIME_ZONE_UTC);
        break;
    case SIPPARSER_TIME_ZONE_GMST:
        rv = SipDateHeaderSetZone(hDateHeader, MSG_DATE_TIME_ZONE_GMST);
        break;
    case SIPPARSER_TIME_ZONE_GMT:
        rv = SipDateHeaderSetZone(hDateHeader, MSG_DATE_TIME_ZONE_GMT);
        break;
    }
    if (RV_OK!=rv)
    {
       RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
         "ParserInitDateInHeader - error in SipDateHeaderSetZone"));
       return rv;
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pParserMgr);
#endif

	return RV_OK;
}
/***************************************************************************
 * ParserInitExpiresHeaderInHeader
 * ------------------------------------------------------------------------
 * General: This function is used to init the expires handle with the
 *          expires param values.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pParserMgr    - Pointer to the parser manager.
 *             pExpiresVal   - Expires structure holding the Expires values from the
 *                             parser in a token form.
 *              hPool      - Handle to the message pool.
 *              hPage      - Handle to the message page.
 *  In/Output: hExpiresHeader- Pointer to the structure in which the values from
 *                             the parser will be set.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitExpiresHeaderInHeader(
                                     IN    ParserMgr                *pParserMgr,
                                     IN    ParserExpiresHeader      *pExpiresVal,
                                     IN    HRPOOL                    hPool,
                                     IN    HPAGE                     hPage,
                                     INOUT RvSipExpiresHeaderHandle  hExpiresHeader)
{
    RvStatus rv;

    RV_UNUSED_ARG(hPool);
    RV_UNUSED_ARG(hPage);
    /* Check whether the expires is sip date, if so, set the values
       in sip date of the expires:
       <Thu, 01 Dec 1994 16:00:00 GMT> */
    if (RVSIP_EXPIRES_FORMAT_DATE == pExpiresVal->eFormat)
    {
        /* Set week day*/
        RvSipDateHeaderHandle hDateHeader;
        rv = RvSipDateConstructInExpiresHeader(hExpiresHeader,
                                                  &hDateHeader);
        if (RV_OK!=rv)
        {
           RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitExpiresHeaderInHeader - error in RvSipDateConstructInExpiresHeader"));
           return rv;
        }
        rv = ParserInitDateInHeader(pParserMgr, &pExpiresVal->sipDate ,(RvSipDateHeaderHandle)hDateHeader);

        if (RV_OK!=rv)
        {
           RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitExpiresHeaderInHeader - error in ParserInitDateInHeader"));
           return rv;
        }
    }
    /* Check whether the expires parameter is delta seconds, if so,
       set the delta second parameter */
    if (RVSIP_EXPIRES_FORMAT_DELTA_SECONDS == pExpiresVal->eFormat)
    {
         RvInt32 deltaSeconds;

#ifdef RV_SIP_UINT_DELTASECONDS
        /* Change the token port into an unsigned number*/
        rv = ParserGetUINT32FromString(pParserMgr,
                                       pExpiresVal->deltaSeconds.buf ,
                                       pExpiresVal->deltaSeconds.len,
                                       (RvUint32*)&deltaSeconds);
#else /* #ifndef RV_SIP_UINT_DELTASECONDS */
		/* Change the token port into a signed number*/
        rv = ParserGetINT32FromString(pParserMgr,
                                      pExpiresVal->deltaSeconds.buf ,
                                      pExpiresVal->deltaSeconds.len,
                                      &deltaSeconds);
#endif /* #ifndef RV_SIP_UINT_DELTASECONDS */

        if (RV_OK!=rv)
        {
           RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitExpiresHeaderInHeader - error in ParserGetINT32FromString"));
            return rv;
        }
        rv = RvSipExpiresHeaderSetDeltaSeconds(hExpiresHeader,deltaSeconds);
        if (RV_OK!=rv)
        {
           RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitExpiresHeaderInHeader - error in RvSipExpiresHeaderSetDeltaSeconds"));
            return rv;
        }

    }
    return RV_OK;

}

/***************************************************************************
 * ParserInitContactParams
 * ------------------------------------------------------------------------
 * General: This function is used from the parser to init the message with the
 *          Contact header values.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    pSipMsg - Pointer to the structure in which the values from the parser
 *            will be set.
 *    pContactVal - Contact header structure holding the Contact header values
 *                from the parser in a token form .
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitContactParams(
                                       IN    ParserMgr                 *pParserMgr,
                                       IN    ParserContactHeaderValues *pContactVal,
                                       IN    HRPOOL                    hPool,
                                       IN    HPAGE                     hPage,
                                       INOUT RvSipContactHeaderHandle  hContactHeader)
{
    RvInt32                 offsetContactParams = UNDEFINED;
    RvSipExpiresHeaderHandle hExpires;
    RvStatus                rv;


    /* Check whether expires parameter has been set, if so attach it to
       Expires header in the Contact header*/
    if (RV_TRUE == pContactVal->header.params.isExpires)
    {
        rv = RvSipExpiresConstructInContactHeader(hContactHeader,&hExpires);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in RvSipExpiresConstructInContactHeader"));
            return rv;
        }
        rv =  ParserInitExpiresHeaderInHeader(pParserMgr,
                                                 &pContactVal->header.params.expires,
                                                 hPool,
                                                 hPage,
                                                 hExpires);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserInitExpiresHeaderInHeader"));
            return rv;
        }
    }/* if (RV_TRUE == pContactVal->header.params.isExpires)*/


     /* Check whether q parameter has been set, if so attach it to
       strContactParam in the Contact header*/

    if (RV_TRUE == pContactVal->header.params.isQ)
    {
        RvInt32 offsetQval = UNDEFINED;
        rv = ParserAllocFromObjPage(pParserMgr,
                                       &offsetQval,
                                       hPool,
                                       hPage,
                                       pContactVal->header.params.q.buf,
                                       pContactVal->header.params.q.len);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipContactHeaderSetQVal(hContactHeader,
                                        NULL,
                                        hPool,
                                        hPage,
                                        offsetQval);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitContactParams - error in SipContactHeaderSetQVal"));
            return rv;
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isQ) */

    /* Check whether action parameter has been set, if so attach it to
       strContactParam in the Contact header*/
    if (RV_TRUE == pContactVal->header.params.isAction)
    {
        if (pContactVal->header.params.action == PARSER_CONTACT_ACTION_REDIRECT)
        {
            rv = RvSipContactHeaderSetAction(hContactHeader,
                                                RVSIP_CONTACT_ACTION_REDIRECT);
        }
        else if (pContactVal->header.params.action == PARSER_CONTACT_ACTION_PROXY)
        {
            rv = RvSipContactHeaderSetAction(hContactHeader,
                                                RVSIP_CONTACT_ACTION_PROXY);
        }
        else
        {
            rv = RV_ERROR_UNKNOWN;
        }

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitContactParams - error in RvSipContactHeaderSetAction"));
            return rv;
        }
    }/* if (RV_TRUE == pContactVal->header.params.isAction)*/

#ifdef RV_SIP_IMS_HEADER_SUPPORT
    /* Check whether temp-gruu parameter has been set, if so attach it to
       strContactParam in the Contact header*/
    if (RV_TRUE == pContactVal->header.params.isTempGruu)
    {
        RvInt32 offsetTempGruu = UNDEFINED;
        rv = ParserAllocFromObjPage(pParserMgr,
                                       &offsetTempGruu,
                                       hPool,
                                       hPage,
                                       pContactVal->header.params.tempGruu.buf,
                                       pContactVal->header.params.tempGruu.len);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipContactHeaderSetTempGruu(hContactHeader,
                                        NULL,
                                        hPool,
                                        hPage,
                                        offsetTempGruu);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitContactParams - error in SipContactHeaderSetTempGruu"));
            return rv;
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isTempGruu) */

    /* Check whether pub-gruu parameter has been set, if so attach it to
       strContactParam in the Contact header*/
    if (RV_TRUE == pContactVal->header.params.isPubGruu)
    {
        RvInt32 offsetPubGruu = UNDEFINED;
        rv = ParserAllocFromObjPage(pParserMgr,
                                       &offsetPubGruu,
                                       hPool,
                                       hPage,
                                       pContactVal->header.params.pubGruu.buf,
                                       pContactVal->header.params.pubGruu.len);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }

        rv = SipContactHeaderSetPubGruu(hContactHeader,
                                        NULL,
                                        hPool,
                                        hPage,
                                        offsetPubGruu);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
              "ParserInitContactParams - error in SipContactHeaderSetPubGruu"));
            return rv;
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isPubGruu) */
    
    /* Check whether reg-id parameter has been set, if so attach it to
       the Contact header */
    if (RV_TRUE == pContactVal->header.params.isRegIDNum)
    {
        RvInt32 regIDNum;
        rv=ParserGetINT32FromString(pParserMgr,
                                       pContactVal->header.params.regIDNum.buf,
                                       pContactVal->header.params.regIDNum.len,
                                       &regIDNum);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitContactParams - error in ParserGetINT32FromString"));
            return rv;
        }

        /* Set the regIDNum number from Contact parameters in the Contact header */
        rv = RvSipContactHeaderSetRegIDNum(hContactHeader, regIDNum);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitContactParams - error in RvSipContactHeaderSetRegIDNum"));
            return rv;
        }
    }

    /* Check whether Actor parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureActor)
    {
        RvInt32 offsetActor = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_ACTOR,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureActor.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetActor,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureActor.value.buf,
                                           pContactVal->header.params.featureActor.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_ACTOR,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetActor);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetActor"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureActor) */

    /* Check whether Application parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureApplication)
    {
        RvInt32 offsetApplication = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_APPLICATION,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureApplication.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetApplication,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureApplication.value.buf,
                                           pContactVal->header.params.featureApplication.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_APPLICATION,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetApplication);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetApplication"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureApplication) */

    /* Check whether Audio parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureAudio)
    {
        RvInt32 offsetAudio = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_AUDIO,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureAudio.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetAudio,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureAudio.value.buf,
                                           pContactVal->header.params.featureAudio.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_AUDIO,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetAudio);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetAudio"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureAudio) */

    /* Check whether Automata parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureAutomata)
    {
        RvInt32 offsetAutomata = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_AUTOMATA,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureAutomata.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetAutomata,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureAutomata.value.buf,
                                           pContactVal->header.params.featureAutomata.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_AUTOMATA,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetAutomata);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetAutomata"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureAutomata) */

    /* Check whether Class parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureClass)
    {
        RvInt32 offsetClass = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_CLASS,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureClass.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetClass,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureClass.value.buf,
                                           pContactVal->header.params.featureClass.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_CLASS,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetClass);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetClass"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureClass) */

    /* Check whether Control parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureControl)
    {
        RvInt32 offsetControl = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_CONTROL,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureControl.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetControl,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureControl.value.buf,
                                           pContactVal->header.params.featureControl.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_CONTROL,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetControl);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetControl"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureControl) */

    /* Check whether Data parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureData)
    {
        RvInt32 offsetData = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_DATA,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureData.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetData,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureData.value.buf,
                                           pContactVal->header.params.featureData.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_DATA,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetData);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetData"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureData) */

    /* Check whether Description parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureDescription)
    {
        RvInt32 offsetDescription = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_DESCRIPTION,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureDescription.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetDescription,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureDescription.value.buf,
                                           pContactVal->header.params.featureDescription.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_DESCRIPTION,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetDescription);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetDescription"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureDescription) */

    /* Check whether Duplex parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureDuplex)
    {
        RvInt32 offsetDuplex = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_DUPLEX,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureDuplex.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetDuplex,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureDuplex.value.buf,
                                           pContactVal->header.params.featureDuplex.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_DUPLEX,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetDuplex);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetDuplex"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureDuplex) */

    /* Check whether Events parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureEvents)
    {
        RvInt32 offsetEvents = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_EVENTS,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureEvents.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetEvents,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureEvents.value.buf,
                                           pContactVal->header.params.featureEvents.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_EVENTS,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetEvents);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetEvents"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureEvents) */

    /* Check whether Extensions parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureExtensions)
    {
        RvInt32 offsetExtensions = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_EXTENSIONS,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureExtensions.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetExtensions,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureExtensions.value.buf,
                                           pContactVal->header.params.featureExtensions.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_EXTENSIONS,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetExtensions);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetExtensions"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureExtensions) */

    /* Check whether IsFocus parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureIsFocus)
    {
        RvInt32 offsetIsFocus = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_ISFOCUS,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureIsFocus.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetIsFocus,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureIsFocus.value.buf,
                                           pContactVal->header.params.featureIsFocus.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_ISFOCUS,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetIsFocus);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetIsFocus"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureIsFocus) */

    /* Check whether Language parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureLanguage)
    {
        RvInt32 offsetLanguage = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_LANGUAGE,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureLanguage.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetLanguage,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureLanguage.value.buf,
                                           pContactVal->header.params.featureLanguage.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_LANGUAGE,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetLanguage);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetLanguage"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureLanguage) */

    /* Check whether Methods parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureMethods)
    {
        RvInt32 offsetMethods = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_METHODS,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureMethods.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetMethods,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureMethods.value.buf,
                                           pContactVal->header.params.featureMethods.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_METHODS,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetMethods);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetMethods"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureMethods) */

    /* Check whether Mobility parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureMobility)
    {
        RvInt32 offsetMobility = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_MOBILITY,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureMobility.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetMobility,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureMobility.value.buf,
                                           pContactVal->header.params.featureMobility.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_MOBILITY,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetMobility);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetMobility"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureMobility) */

    /* Check whether Priority parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeaturePriority)
    {
        RvInt32 offsetPriority = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_PRIORITY,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featurePriority.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetPriority,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featurePriority.value.buf,
                                           pContactVal->header.params.featurePriority.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_PRIORITY,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetPriority);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetPriority"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeaturePriority) */

    /* Check whether Schemes parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureSchemes)
    {
        RvInt32 offsetSchemes = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_SCHEMES,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureSchemes.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetSchemes,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureSchemes.value.buf,
                                           pContactVal->header.params.featureSchemes.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_SCHEMES,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetSchemes);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetSchemes"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureSchemes) */

    /* Check whether Text parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureText)
    {
        RvInt32 offsetText = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_TEXT,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureText.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetText,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureText.value.buf,
                                           pContactVal->header.params.featureText.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_TEXT,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetText);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetText"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureText) */

    /* Check whether Type parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureType)
    {
        RvInt32 offsetType = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_TYPE,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureType.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetType,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureType.value.buf,
                                           pContactVal->header.params.featureType.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_TYPE,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetType);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetType"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureType) */

    /* Check whether Video parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureVideo)
    {
        RvInt32 offsetVideo = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_VIDEO,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureVideo.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetVideo,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureVideo.value.buf,
                                           pContactVal->header.params.featureVideo.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_VIDEO,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetVideo);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetVideo"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureVideo) */

    /* Check whether SipInstance parameter has been set, if so attach it to the Contact header */
    if (RV_TRUE == pContactVal->header.params.isFeatureSipInstance)
    {
        RvInt32 offsetSipInstance = UNDEFINED;
        rv = RvSipContactHeaderSetFeatureTagActivation(hContactHeader,
                                                       RVSIP_CONTACT_FEATURE_TAG_TYPE_SIP_INSTANCE,
                                                       RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                "ParserInitContactParams - error in ParserAllocFromObjPage"));
            return rv;
        }
        
        if(pContactVal->header.params.featureSipInstance.isValue == RV_TRUE)
        {
            rv = ParserAllocFromObjPage(pParserMgr,
                                           &offsetSipInstance,
                                           hPool,
                                           hPage,
                                           pContactVal->header.params.featureSipInstance.value.buf,
                                           pContactVal->header.params.featureSipInstance.value.len);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitContactParams - error in ParserAllocFromObjPage"));
                return rv;
            }

            rv = SipContactHeaderSetStrFeatureTag(hContactHeader,
                                                RVSIP_CONTACT_FEATURE_TAG_TYPE_SIP_INSTANCE,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetSipInstance);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                  "ParserInitContactParams - error in SipContactHeaderSetSipInstance"));
                return rv;
            }
        }
    }/*  if (RV_TRUE == pContactVal->header.params.isFeatureSipInstance) */
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
    
    /* Check whether extension parameter has been set, if so attach it to
       strContactParam in the Contact header*/
    if (RV_TRUE == pContactVal->header.params.isExtention)
    {
       /* append the extension param in the message page */
        offsetContactParams = ParserAppendCopyRpoolString(pParserMgr,
            hPool,
            hPage,
            ((ParserExtensionString *)pContactVal->header.params.exten)->hPool,
            ((ParserExtensionString *)pContactVal->header.params.exten)->hPage,
            0,
            ((ParserExtensionString *)pContactVal->header.params.exten)->size);
       
       if (UNDEFINED == offsetContactParams)
       {
           RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
               "ParserInitContactParams - error in ParserAppendCopyRpoolString"));
           return RV_ERROR_OUTOFRESOURCES;
       }
	   /* Set The contact param in the header */
	   return SipContactHeaderSetContactParam(
                                        hContactHeader,
                                        NULL,
                                        hPool,
                                        hPage,
                                        offsetContactParams);
	}

	return RV_OK;
}
#endif /*#ifndef RV_SIP_LIGHT*/
#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * ParserInitChallengeAuthentication
 * ------------------------------------------------------------------------
 * General: This function is used from the parser to init the message with the
 *          Authentication header values.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pParserMgr  - Pointer to the parser manager.
 *             pAuthHeaderVal - Authentication structure holding the Authentication
 *                          values from the parser in a token form.
 *             hPool      - Handle to the message pool.
 *             hPage      - Handle to the message page.
 *  In/Output: pSipObject - Pointer to the structure in which the values from
 *                          the parser will be set.
 *                          If eType == SIP_PARSETYPE_MSG it will be cast to
 *                          RvSipMsgHandle and if eType == SIP_PARSETYPE_AUTHENTICATION
 *                          it will be cast to RvSipAuthenticationHeaderHandle.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitChallengeAuthentication(
                           IN    ParserMgr                  *pParserMgr,
                           IN    ParserAuthenticationHeader *pAuthHeaderVal,
                           IN    HRPOOL                     hPool,
                           IN    HPAGE                      hPage,
                           INOUT RvSipAuthenticationHeaderHandle hAuthenHeader)

{
    RvStatus            rv;
    RvInt32             offsetAlgorithm = UNDEFINED;


    if (pAuthHeaderVal->eAuthScheme == RVSIP_AUTH_SCHEME_DIGEST)
    {
        /* Check whether the realm value has been set, if so attach
       it to the Authentication header  */
        if (RV_TRUE == pAuthHeaderVal->isRealm)
        {
            RvInt32              offsetRealm = UNDEFINED;
            /* Change the token realm into a string and set it in the Authentication header*/
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetRealm,
                                          hPool,
                                          hPage,
                                          pAuthHeaderVal->realm.buf,
                                          pAuthHeaderVal->realm.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthentication - error in ParserAllocFromObjPage"));
                return rv;
            }
            /* Set the realm string value in the Authentication header */
            rv = SipAuthenticationHeaderSetRealm(
                                    hAuthenHeader,
                                    NULL,
                                    hPool,
                                    hPage,
                                    offsetRealm);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthentication - error in SipAuthenticationHeaderSetRealm"));
                return rv;
            }
        }

        /* Check whether the domain value has been set, if so attach
           it to the Authen header  */
        if (RV_TRUE == pAuthHeaderVal->isDomain)
        {
            RvInt32                 offsetDomain = UNDEFINED;
            /* Change the token realm into a string and set it in the Authentication header*/
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetDomain,
                                          hPool,
                                          hPage,
                                          pAuthHeaderVal->domain.buf,
                                          pAuthHeaderVal->domain.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthentication - error in ParserAllocFromObjPage"));
                return rv;
            }
            /* Set the realm string value in the Authentication header */
            rv = SipAuthenticationHeaderSetDomain(
                                                    hAuthenHeader,
                                                    NULL,
                                                    hPool,
                                                    hPage,
                                                    offsetDomain);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthentication - error in SipAuthenticationHeaderSetDomain"));
                return rv;
            }
        }


        /* Check whether the nonce value has been set, if so attach
           it to the Authentication header  */
        if (RV_TRUE == pAuthHeaderVal->isNonce)
        {
            RvInt32                 offsetNonce = UNDEFINED;
            /* Change the token nonce to a string and set it in the Authen header*/
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetNonce,
                                          hPool,
                                          hPage,
                                          pAuthHeaderVal->nonce.buf,
                                          pAuthHeaderVal->nonce.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthentication - error in ParserAllocFromObjPage"));
                return rv;
            }
            /* Set the nonce string value in the Authen header */
            rv = SipAuthenticationHeaderSetNonce(
                                hAuthenHeader,
                                NULL,
                                hPool,
                                hPage,
                                offsetNonce);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthentication - error in SipAuthenticationHeaderSetNonce"));
                return rv;
            }
        }

        /* Check whether the opaque value has been set, if so attach
           it to the Authen header  */
        if (RV_TRUE == pAuthHeaderVal->isOpaque)
        {
            RvInt32                 offsetOpaque = UNDEFINED;
            /* Change the token opaque into a string and set it in the Authen header*/
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetOpaque,
                                          hPool,
                                          hPage,
                                          pAuthHeaderVal->opaque.buf,
                                          pAuthHeaderVal->opaque.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthentication - error in ParserAllocFromObjPage"));
                return rv;
            }
            /* Set the opaque string value in the Authen header */
            rv = SipAuthenticationHeaderSetOpaque(
                            hAuthenHeader,
                            NULL,
                            hPool,
                            hPage,
                            offsetOpaque);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthentication - error in SipAuthenticationHeaderSetOpaque"));
                return rv;
            }
        }

        /* Set the stale variable in the Authen header */
        rv = RvSipAuthenticationHeaderSetStale(hAuthenHeader,
                                                  pAuthHeaderVal->eStale);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                "ParserInitChallengeAuthentication - error in RvSipAuthenticationHeaderSetStale"));
            return rv;
        }

        /* Check whether the algorithm value is other than the enumerate values */
        if (RVSIP_AUTH_ALGORITHM_OTHER ==pAuthHeaderVal->algorithm.eAlgorithm)
        {
            /* Change the token value into a string and set it in the Authen header*/
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetAlgorithm,
                                          hPool,
                                          hPage,
                                          pAuthHeaderVal->algorithm.algorithm.buf ,
                                          pAuthHeaderVal->algorithm.algorithm.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthentication - error in ParserAllocFromObjPage"));
                return rv;
            }

        }
        else if (RVSIP_AUTH_ALGORITHM_UNDEFINED == pAuthHeaderVal->algorithm.eAlgorithm)
        {
            /* if no algorithm param was set in Authentication header - MD5 is the default */
            pAuthHeaderVal->algorithm.eAlgorithm = RVSIP_AUTH_ALGORITHM_MD5;
        }

        rv = SipAuthenticationHeaderSetAuthAlgorithm(
                                                    hAuthenHeader,
                                                    pAuthHeaderVal->algorithm.eAlgorithm,
                                                    NULL,
                                                    hPool,
                                                    hPage,
                                                    offsetAlgorithm);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitChallengeAuthentication - error in SipAuthenticationHeaderSetAuthAlgorithm"));
            return rv;
        }

		/* Check whether the AKAVersion param value has been set, if so attach
           it to the Authen header  */
		if (RV_TRUE == pAuthHeaderVal->algorithm.isAKAVersion)
        {
            RvInt32	nAKAVersion;
			if(0 == pAuthHeaderVal->algorithm.nAKAVersion.len)
			{
				/* nAKAVersion is undefined */
				nAKAVersion = UNDEFINED;
			}
			else
			{
				rv = ParserGetINT32FromString(pParserMgr,
												pAuthHeaderVal->algorithm.nAKAVersion.buf ,
												pAuthHeaderVal->algorithm.nAKAVersion.len,
												&nAKAVersion);
				if (RV_OK != rv)
				{
					RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitChallengeAuthentication - error in ParserGetINT32FromString (nAKAVersion)"));
					return rv;
				}
			}

			rv = RvSipAuthenticationHeaderSetAKAVersion(hAuthenHeader, nAKAVersion);
			if (RV_OK != rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitChallengeAuthentication - error in RvSipAuthenticationHeaderSetAKAVersion"));
				return rv;
			}
        }

		/* Check whether the auth param value has been set, if so attach
		   it to the Authen header  */
        if (RV_TRUE == pAuthHeaderVal->isAuthParam)
        {
            RvInt32              offsetAuthParam = UNDEFINED;
            /* Change the token auth param into a string and set it in the Authen header*/
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetAuthParam,
                                          hPool,
                                          hPage,
                                          pAuthHeaderVal->authParam.buf,
                                          pAuthHeaderVal->authParam.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthentication - error in ParserAllocFromObjPage"));
                return rv;
            }
            /* Set the opaque string value in the Authen header */
            rv = SipAuthenticationHeaderSetOtherParams(
                                                        hAuthenHeader,
                                                        NULL,
                                                        hPool,
                                                        hPage,
                                                        offsetAuthParam);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthentication - error in SipAuthenticationHeaderSetOtherParams"));
                return rv;
            }
        }

        rv = RvSipAuthenticationHeaderSetQopOption(hAuthenHeader,
                                                      pAuthHeaderVal->qopValue.eAuthQop);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                "ParserInitChallengeAuthentication - error in RvSipAuthenticationHeaderQopType"));
            return rv;
        }

		/* Check whether the integrity-key value has been set, if so attach
           it to the Authen header  */
        if (RV_TRUE == pAuthHeaderVal->isIntegrityKey)
        {
            RvInt32                 offsetIntegrityKey = UNDEFINED;
            /* Change the token IntegrityKey into a string and set it in the Authen header*/
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetIntegrityKey,
                                          hPool,
                                          hPage,
                                          pAuthHeaderVal->integrityKey.buf,
                                          pAuthHeaderVal->integrityKey.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthentication - error in ParserAllocFromObjPage"));
                return rv;
            }
            /* Set the IntegrityKey string value in the Authen header */
            rv = SipAuthenticationHeaderSetStrIntegrityKey(
                            hAuthenHeader,
                            NULL,
                            hPool,
                            hPage,
                            offsetIntegrityKey);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthentication - error in SipAuthenticationHeaderSetStrIntegrityKey"));
                return rv;
            }
        }
		
		/* Check whether the cipher-key value has been set, if so attach
           it to the Authen header  */
        if (RV_TRUE == pAuthHeaderVal->isCipherKey)
        {
            RvInt32                 offsetCipherKey = UNDEFINED;
            /* Change the token IntegrityKey into a string and set it in the Authen header*/
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetCipherKey,
                                          hPool,
                                          hPage,
                                          pAuthHeaderVal->cipherKey.buf,
                                          pAuthHeaderVal->cipherKey.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthentication - error in ParserAllocFromObjPage"));
                return rv;
            }
            /* Set the CipherKey string value in the Authen header */
            rv = SipAuthenticationHeaderSetStrCipherKey(
                            hAuthenHeader,
                            NULL,
                            hPool,
                            hPage,
                            offsetCipherKey);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthentication - error in SipAuthenticationHeaderSetStrCipherKey"));
                return rv;
            }
        }

        /* Check whether the QOP value is more than the enumerate values */
        if (RV_TRUE ==pAuthHeaderVal->qopValue.isOther)
        {

            RvInt32              offsetQop = UNDEFINED;

            /* Change the token value into a string and set it in the Other header*/
            offsetQop = ParserAppendCopyRpoolString(
                                    pParserMgr,
                                    hPool,
                                    hPage,
                                    ((ParserExtensionString *)pAuthHeaderVal->qopValue.qop)->hPool,
                                    ((ParserExtensionString *)pAuthHeaderVal->qopValue.qop)->hPage,
                                    0,
                                    ((ParserExtensionString *)pAuthHeaderVal->qopValue.qop)->size);

            if (UNDEFINED == offsetQop)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthentication - error in ParserAppendCopyRpoolString"));
                return RV_ERROR_OUTOFRESOURCES;
            }
            rv = SipAuthenticationHeaderSetStrQop(
                                                    hAuthenHeader,
                                                    NULL,
                                                    hPool,
                                                    hPage,
                                                    offsetQop);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthentication - error in SipAuthenticationHeaderSetStrQop"));
                return rv;
            }
        }
    }
    else
    {
        /* the scheme is not digest so set only the auth param list */
        if (RV_TRUE ==pAuthHeaderVal->isAuthParamList)
        {

            RvInt32              offsetAuthParam = UNDEFINED;
      
            /* Change the token value into a string and set it in the Other header*/
            offsetAuthParam = ParserAppendCopyRpoolString(
                        pParserMgr,
                        hPool,
                        hPage,
                        ((ParserExtensionString *)pAuthHeaderVal->authParamList)->hPool,
                        ((ParserExtensionString *)pAuthHeaderVal->authParamList)->hPage,
                        0,
                        ((ParserExtensionString *)pAuthHeaderVal->authParamList)->size);

            if (UNDEFINED == offsetAuthParam)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthentication - error in ParserAppendCopyRpoolString"));
                return RV_ERROR_OUTOFRESOURCES;
            }
            rv = SipAuthenticationHeaderSetOtherParams(
                            hAuthenHeader,
                            NULL,
                            hPool,
                            hPage,
                            offsetAuthParam);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthentication - error in SipAuthenticationHeaderSetOtherParams"));
                return rv;
            }
        }
    }


    return RV_OK;
}

/***************************************************************************
 * ParserInitChallengeAuthorization
 * ------------------------------------------------------------------------
 * General: This function is used from the parser to init the message with the
 *          Authentication header values.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pParserMgr  - Pointer to the parser manager.
 *             pAuthHeaderVal - Authentication structure holding the Authentication
 *                              values from the parser in a token form.
 *              hPool      - Handle to the message pool.
 *              hPage      - Handle to the message page.
 *  In/Output: hAuthorHeader -  Pointer to the structure in which the values from
 *                              the parser will be set.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitChallengeAuthorization(
                           IN    ParserMgr                      *pParserMgr,
                           IN    ParserAuthorizationHeader      *pAuthHeaderVal,
                           IN    HRPOOL                         hPool,
                           IN    HPAGE                          hPage,
                           INOUT RvSipAuthorizationHeaderHandle hAuthorHeader)

{
    RvStatus            rv;
    RvInt32             offsetAlgorithm = UNDEFINED;
    RvSipAddressHandle   hAddress;
    RvSipAddressType eAddrType;

   if (pAuthHeaderVal->eAuthScheme == RVSIP_AUTH_SCHEME_DIGEST)
    {
        /* Check whether the username value has been set, if so attach
       it to the Author header  */
        if (RV_TRUE == pAuthHeaderVal->isUsername)
        {
            RvInt32              offsetUsername = UNDEFINED;
            /* Change the token username into a string and set it in the Author header*/
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetUsername,
                                          hPool,
                                          hPage,
                                          pAuthHeaderVal->userName.buf ,
                                          pAuthHeaderVal->userName.len );
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthorization - error in ParserAllocFromObjPage"));
                return rv;
            }
            /* Set the username string value in the Author header */
            rv = SipAuthorizationHeaderSetUserName(
                                                    hAuthorHeader,
                                                    NULL,
                                                    hPool,
                                                    hPage,
                                                    offsetUsername);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthorization - error in SipAuthorizationHeaderSetUserName"));
                return rv;
            }
        }

        /* Check whether the realm value has been set, if so attach
           it to the Author header  */
        if (RV_TRUE == pAuthHeaderVal->isRealm)
        {
            RvInt32              offsetRealm = UNDEFINED;
            /* Change the token realm into a string and set it in the Authen header*/
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetRealm,
                                          hPool,
                                          hPage,
                                          pAuthHeaderVal->realm.buf,
                                          pAuthHeaderVal->realm.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthorization - error in ParserAllocFromObjPage"));
                return rv;
            }
            /* Set the realm string value in the Author header */
            rv = SipAuthorizationHeaderSetRealm(
                                                    hAuthorHeader,
                                                    NULL,
                                                    hPool,
                                                    hPage,
                                                    offsetRealm);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthorization - error in SipAuthorizationHeaderSetRealm"));
                return rv;
            }
        }

        /* Check whether the nonce value has been set, if so attach
           it to the Author header  */
        if (RV_TRUE == pAuthHeaderVal->isNonce)
        {
            RvInt32                 offsetNonce = UNDEFINED;
            /* Change the token nonce into a string and set it in the Author header*/
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetNonce,
                                          hPool,
                                          hPage,
                                          pAuthHeaderVal->nonce.buf,
                                          pAuthHeaderVal->nonce.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthorization - error in ParserAllocFromObjPage"));
                return rv;
            }
            /* Set the nonce string value in the Author header */
            rv = SipAuthorizationHeaderSetNonce(
                                                hAuthorHeader,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetNonce);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthorization - error in SipAuthorizationHeaderSetNonce"));
                return rv;
            }
        }

        /* Construct the sip-url\abs-uri in the authorization header */
        if(pAuthHeaderVal->isUri == RV_TRUE)
        {
			eAddrType = ParserAddrType2MsgAddrType(pAuthHeaderVal->hDigestUri.uriType);
			if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitChallengeAuthorization - unknown parserAddressType %d", pAuthHeaderVal->hDigestUri.uriType));
				return RV_ERROR_UNKNOWN;
			}


			rv = RvSipAddrConstructInAuthorizationHeader(hAuthorHeader,
																eAddrType,
																&hAddress);
			if (RV_OK!=rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				   "ParserInitChallengeAuthorization - error in RvSipAddrConstructInAuthorizationHeader"));
				return rv;
			}

			/* Initialize the address values from the parser*/
			rv = ParserInitAddressInHeader(pParserMgr,
											  &pAuthHeaderVal->hDigestUri,
											  hPool,
											  hPage,
											  eAddrType,
											  hAddress);
			if (RV_OK != rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				   "ParserInitChallengeAuthorization - error in ParserInitAddressInHeader"));
				return rv;
			}
/*#ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/
			/*patch - save the uri string as is, in order to use it in the MD5 calculation.
			(come to solve the problem that we received 
			"uri="sip:3002@10.17.129.50:5060;user=phone;transport=tcp", and during MD5
			because we use address encode we use 
			"sip:3002@10.17.129.50:5060;transport=tcp;user=phone" */
			if (NULL != pAuthHeaderVal->uriBegin)
			{
				RvInt32     offsetOrigUri = UNDEFINED;
				/* Change the token nonce into a string and set it in the Author header*/
				rv= ParserAllocFromObjPage(pParserMgr,
											  &offsetOrigUri,
											  hPool,
											  hPage,
											  pAuthHeaderVal->uriBegin,
											  (RvUint32)(pAuthHeaderVal->uriEnd - pAuthHeaderVal->uriBegin - 1));
				if (RV_OK!=rv)
				{
					RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitChallengeAuthorization - error in ParserAllocFromObjPage"));
					return rv;
				}
				/* Set the nonce string value in the Author header */
				rv = SipAuthorizationHeaderSetOrigUri(
													hAuthorHeader,
													NULL,
													hPool,
													hPage,
													offsetOrigUri);

				if (RV_OK != rv)
				{
					RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
						"ParserInitChallengeAuthorization - error in SipAuthorizationHeaderSetOrigUri"));
					return rv;
				}
			}
/*#endif #ifdef RV_SIP_SAVE_AUTH_URI_PARAMS_ORDER*/
        }

        /* Check whether the response value has been set, if so attach
           it to the Author header  */
        if (RV_TRUE == pAuthHeaderVal->isResponse)
        {
            RvInt32             offsetResponse = UNDEFINED;
            /* Change the token response into a string and set it in the Author header*/
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetResponse,
                                          hPool,
                                          hPage,
                                          pAuthHeaderVal->response.buf,
                                          pAuthHeaderVal->response.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthorization - error in ParserAllocFromObjPage"));
                return rv;
            }
            /* Set the realm string value in the Author header */
            rv = SipAuthorizationHeaderSetResponse(
                            hAuthorHeader,
                            NULL,
                            hPool,
                            hPage,
                            offsetResponse);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthorization - error in SipAuthorizationHeaderSetResponse"));
                return rv;
            }
        }

        /* Check whether the algorithm value is other than the enumerate values */
        if (RVSIP_AUTH_ALGORITHM_OTHER ==pAuthHeaderVal->algorithm.eAlgorithm)
        {
            /* Change the token value into a string and set it in the Other header*/
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetAlgorithm,
                                          hPool,
                                          hPage,
                                          pAuthHeaderVal->algorithm.algorithm.buf ,
                                          pAuthHeaderVal->algorithm.algorithm.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthorization - error in ParserAllocFromObjPage"));
                return rv;
            }
        }
     /* bug fix. if the Authorization header did not contain an algorithm param,
        leave it as is. */
      /* else if(RVSIP_AUTH_ALGORITHM_UNDEFINED == pAuthHeaderVal->algorithm.eAlgorithm)
        {
            / * if there was no algorithm in the Authorization header, the default
            value is MD5 * /
            pAuthHeaderVal->algorithm.eAlgorithm = RVSIP_AUTH_ALGORITHM_MD5;
        }*/

        rv = SipAuthorizationHeaderSetAuthAlgorithm(
                            hAuthorHeader,
                            pAuthHeaderVal->algorithm.eAlgorithm,
                            NULL,
                            hPool,
                            hPage,
                            offsetAlgorithm);

        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
              "ParserInitChallengeAuthorization - error in SipAuthorizationHeaderSetAuthAlgorithm"));
            return rv;
        }

		/* Check whether the AKAVersion param value has been set, if so attach
           it to the Authorization header  */
		if (RV_TRUE == pAuthHeaderVal->algorithm.isAKAVersion)
        {
            RvInt32	nAKAVersion;
			if(0 == pAuthHeaderVal->algorithm.nAKAVersion.len)
			{
				/* nAKAVersion is undefined */
				nAKAVersion = UNDEFINED;
			}
			else
			{
				rv = ParserGetINT32FromString(pParserMgr,
												pAuthHeaderVal->algorithm.nAKAVersion.buf ,
												pAuthHeaderVal->algorithm.nAKAVersion.len,
												&nAKAVersion);
				if (RV_OK != rv)
				{
					RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitChallengeAuthorization - error in ParserGetINT32FromString (nAKAVersion)"));
					return rv;
				}
			}

			rv = RvSipAuthorizationHeaderSetAKAVersion(hAuthorHeader, nAKAVersion);
			if (RV_OK != rv)
			{
				RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitChallengeAuthorization - error in RvSipAuthorizationHeaderSetAKAVersion"));
				return rv;
			}
        }

        /* Check whether the cnonce value has been set, if so attach
           it to the Author header  */
        if (RV_TRUE == pAuthHeaderVal->isCnonce)
        {
            RvInt32                 offsetCnonce = UNDEFINED;
            /* Change the token nonce into a string and set it in the Author header*/
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetCnonce,
                                          hPool,
                                          hPage,
                                          pAuthHeaderVal->cnonce.buf,
                                          pAuthHeaderVal->cnonce.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthorization - error in ParserAllocFromObjPage"));
                return rv;
            }
            /* Set the nonce string value in the Author header */
            rv = SipAuthorizationHeaderSetCNonce(
                            hAuthorHeader,
                            NULL,
                            hPool,
                            hPage,
                            offsetCnonce);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthorization - error in SipAuthorizationHeaderSetCNonce"));
                return rv;
            }
        }

        /* Check whether the opaque value has been set, if so attach
           it to the Author header  */
        if (RV_TRUE == pAuthHeaderVal->isOpaque)
        {
            RvInt32                 offsetOpaque = UNDEFINED;
            /* Change the token opaque into a string and set it in the Author header*/
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetOpaque,
                                          hPool,
                                          hPage,
                                          pAuthHeaderVal->opaque.buf,
                                          pAuthHeaderVal->opaque.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthorization - error in ParserAllocFromObjPage"));
                return rv;
            }
            /* Set the opaque string value in the Author header */
            rv = SipAuthorizationHeaderSetOpaque(
                                                hAuthorHeader,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetOpaque);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthorization - error in SipAuthorizationHeaderSetOpaque"));
                return rv;
            }
        }
		
        /* Check whether the nonce count value has been set, if so attach
           it to the Author header  */
        if (RV_TRUE == pAuthHeaderVal->isNonceCount)
        {
            RvUint32 nonceCount;
             /* Change the token nonce count into a 32 number in the Author header*/
            rv = ParserGetUINT32FromStringHex(pParserMgr,
                                               pAuthHeaderVal->nonceCount.buf ,
                                               pAuthHeaderVal->nonceCount.len,
                                               &nonceCount);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthorization - error in ParserAllocFromObjPage"));
                return rv;
            }
            /* Set the nonce string value in the Author header */
            rv = RvSipAuthorizationHeaderSetNonceCount(hAuthorHeader,(RvInt32)nonceCount);
            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthorization - error in RvSipAuthorizationHeaderSetNonceCount"));
                return rv;
            }
        }

        rv = RvSipAuthorizationHeaderSetQopOption(hAuthorHeader,
                                                     pAuthHeaderVal->eAuthQop);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                "ParserInitChallengeAuthorization - error in RvSipAuthorizationHeaderSetQopType"));
            return rv;
        }

		/* Check whether the Auts count value has been set, if so attach
           it to the Author header  */
        if (RV_TRUE == pAuthHeaderVal->isAuts)
        {
			RvInt32	offsetAuts = UNDEFINED;
            /* Change the token auts into a string and set it in the Author header */
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetAuts,
                                          hPool,
                                          hPage,
                                          pAuthHeaderVal->auts.buf,
                                          pAuthHeaderVal->auts.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthorization - error in ParserAllocFromObjPage"));
                return rv;
            }
            /* Set the auts string value in the Author header */
            rv = SipAuthorizationHeaderSetStrAuts(
                                                hAuthorHeader,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetAuts);
			if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthorization - error in SipAuthorizationHeaderSetStrAuts"));
                return rv;
            }
        }

		rv = RvSipAuthorizationHeaderSetIntegrityProtected(hAuthorHeader,
                                                     pAuthHeaderVal->eProtected);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                "ParserInitChallengeAuthorization - error in RvSipAuthorizationHeaderSetIntegrityProtected"));
            return rv;
        }

		/* Check whether the auth param value has been set, if so attach
           it to the Author header  */
        if (RV_TRUE == pAuthHeaderVal->isAuthParam)
        {
            RvInt32              offsetAuthParam = UNDEFINED;
            /* Change the token auth param into a string and set it in the Author header*/
            rv= ParserAllocFromObjPage(pParserMgr,
                                          &offsetAuthParam,
                                          hPool,
                                          hPage,
                                          pAuthHeaderVal->authParam.buf,
                                          pAuthHeaderVal->authParam.len);
            if (RV_OK!=rv)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthorization - error in ParserAllocFromObjPage"));
                return rv;
            }
            /* Set the opaque string value in the Author header */
            rv = SipAuthorizationHeaderSetOtherParams(
                                                hAuthorHeader,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetAuthParam);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthorization - error in SipAuthorizationHeaderSetOtherParams"));
                return rv;
            }
        }
        
    }
    else /* not DIGEST */
    {
        /* set only the auth params if the scheme is digest */
        if (RV_TRUE ==pAuthHeaderVal->isAuthParamList)
        {

            RvInt32              offsetAuthParam = UNDEFINED;

            /* Change the token value into a string and set it in the Other header*/
            offsetAuthParam = ParserAppendCopyRpoolString(
                        pParserMgr,
                        hPool,
                        hPage,
                        ((ParserExtensionString *)pAuthHeaderVal->authParamList)->hPool,
                        ((ParserExtensionString *)pAuthHeaderVal->authParamList)->hPage,
                        0,
                        ((ParserExtensionString *)pAuthHeaderVal->authParamList)->size);

            if (UNDEFINED == offsetAuthParam)
            {
                RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                    "ParserInitChallengeAuthorization - error in ParserAppendCopyRpoolString"));
                return RV_ERROR_OUTOFRESOURCES;
            }
            rv = SipAuthorizationHeaderSetOtherParams(
                                                hAuthorHeader,
                                                NULL,
                                                hPool,
                                                hPage,
                                                offsetAuthParam);

            if (RV_OK != rv)
            {
                RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                    "ParserInitChallengeAuthorization - error in SipAuthorizationHeaderSetOtherParams"));
                return rv;
            }
        }
    }

    return RV_OK;
}
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * ParserInitSessionExpiresHeaderInHeader
 * ------------------------------------------------------------------------
 * General: This function is used to init the Session Expires handle with the
 *          Session Expires param values.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pParserMgr           - Pointer to the Parser manager object.
 *             pSessionExpiresVal   - Session Expires structure holding the Session
 *                                    Expires values from the parser in a token form.
 *              hPool      - Handle to the message pool.
 *              hPage      - Handle to the message page.
 *  In/Output: hSessionExpiresHeader- Pointer to the structure in which the values from
 *                                    the parser will be set.
 ***************************************************************************/
static RvStatus  RVCALLCONV ParserInitSessionExpiresHeaderInHeader(
                                     IN    ParserMgr                       *pParserMgr,
                                     IN    ParserSessionExpiresHeader      *pSessionExpiresVal,
                                     IN    HRPOOL                           hPool,
                                     IN    HPAGE                            hPage,
                                     INOUT RvSipSessionExpiresHeaderHandle  hSessionExpiresHeader)
{
    RvStatus rv = RV_OK;
    RvUint32 deltaSeconds = 0;
	
	/* Change the token delta second into a number*/
	rv = ParserGetUINT32FromString(pParserMgr,
                                       pSessionExpiresVal->deltaSeconds.buf ,
                                       pSessionExpiresVal->deltaSeconds.len,
                                       &deltaSeconds);
	if (RV_OK!=rv)
	{
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			"ParserInitSessionExpiresHeaderInHeader - error in ParserGetINT32FromString"));
        return rv;
	}
	rv = RvSipSessionExpiresHeaderSetDeltaSeconds(hSessionExpiresHeader,deltaSeconds);
	if (RV_OK!=rv)
	{
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitSessionExpiresHeaderInHeader - error in RvSipExpiresHeaderSetDeltaSeconds"));
        return rv;
	}
	rv = RvSipSessionExpiresHeaderSetRefresherType(hSessionExpiresHeader,
		pSessionExpiresVal->refresherType);
	if (RV_OK!=rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			"ParserInitSessionExpiresHeaderInHeader - error in RvSipSessionExpiresHeaderSetRefresherType"));
		return rv;
	}
	/* set the route params */
	if (RV_TRUE == pSessionExpiresVal->isExtention &&
		NULL != pSessionExpiresVal->exten)
	{
		RvInt32                paramsOffset = 0;
		ParserExtensionString * pOtherParam  =
			(ParserExtensionString *)pSessionExpiresVal->exten;
		
		paramsOffset = ParserAppendCopyRpoolString(pParserMgr,
                                           hPool,
                                           hPage,
                                           pOtherParam->hPool,
                                            pOtherParam->hPage,
                                            0,
                                            pOtherParam->size);

		if (UNDEFINED == paramsOffset)
		{
			RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				"ParserInitSessionExpiresHeaderInHeader - error in ParserAppendCopyRpoolString"));
			return RV_ERROR_OUTOFRESOURCES;
		}
		
		/* Set the other params in the SessionExpires header */
		rv = SipSessionExpiresHeaderSetOtherParams(	hSessionExpiresHeader,
													NULL,
													hPool,
													hPage,
													paramsOffset);
		if (RV_OK != rv)
		{
		   RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
			   "ParserInitSessionExpiresHeaderInHeader - error in SipSessionExpiresHeaderSetOtherParams"));
		   return rv;
		}
	}

	rv = RvSipSessionExpiresHeaderSetCompactForm(hSessionExpiresHeader,
		pSessionExpiresVal->isCompact);
	if (RV_OK!=rv)
	{
		RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			"ParserInitSessionExpiresHeaderInHeader - error in RvSipSessionExpiresHeaderSetCompactForm"));
		return rv;
	}

    return rv;
}
#endif /* RV_SIP_PRIMITIVES */

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * ParserInitMinSEHeaderInHeader
 * ------------------------------------------------------------------------
 * General: This function is used to init the Min SE handle with the
 *          Min SE param values.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pParserMgr  - Pointer to the Parser manager object.
 *             pMinSEVal   - Min SE structure holding the Min-SE
 *                           values from the parser in a token form.
 *              hPool      - Handle to the message pool.
 *              hPage      - Handle to the message page.
 *  In/Output: hMinSEHeader- Pointer to the structure in which the values from
 *                           the parser will be set.
 ***************************************************************************/
static RvStatus  RVCALLCONV ParserInitMinSEHeaderInHeader(
                                     IN    ParserMgr              *pParserMgr,
                                     IN    ParserMinSEHeader      *pMinSEVal,
                                     IN    HRPOOL                  hPool,
                                     IN    HPAGE                   hPage,
                                     INOUT RvSipMinSEHeaderHandle  hMinSEHeader)
{
    RvStatus rv = RV_OK;
    RvUint32 deltaSeconds = 0;

     /* Change the token delta second into a number*/
     rv = ParserGetUINT32FromString(pParserMgr,
                                       pMinSEVal->deltaSeconds.buf ,
                                       pMinSEVal->deltaSeconds.len,
                                       &deltaSeconds);
     if (RV_OK!=rv)
     {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
             "ParserInitMinSEHeaderInHeader - error in ParserGetINT32FromString"));
        return rv;
     }
     rv = RvSipMinSEHeaderSetDeltaSeconds(hMinSEHeader,deltaSeconds);
     if (RV_OK!=rv)
     {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
            "ParserInitMinSEHeaderInHeader - error in RvSipMinSEHeaderSetDeltaSeconds"));
        return rv;
     }
     /* set the route params */
     if (RV_TRUE == pMinSEVal->isExtention &&
         NULL != pMinSEVal->exten)
     {
         RvInt32                paramsOffset = 0;
         ParserExtensionString * pOtherParam  =
             (ParserExtensionString *)pMinSEVal->exten;

         paramsOffset = ParserAppendCopyRpoolString(pParserMgr,
                                             hPool,
                                             hPage,
                                             pOtherParam->hPool,
                                              pOtherParam->hPage,
                                              0,
                                              pOtherParam->size);

         if (UNDEFINED == paramsOffset)
         {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
                 "ParserInitMinSEHeaderInHeader - error in ParserAppendCopyRpoolString"));
             return RV_ERROR_OUTOFRESOURCES;
         }

         /* Set the other params in the MinSE header */
         rv = SipMinSEHeaderSetOtherParams(hMinSEHeader,
                                              NULL,
                                              hPool,
                                              hPage,
                                              paramsOffset);
         if (RV_OK != rv)
         {
             RvLogError(pParserMgr->pLogSrc,(pParserMgr->pLogSrc,
                 "ParserInitMinSEHeaderInHeader - error in SipMinSEHeaderSetOtherParams"));
             return rv;
         }
     }
    return rv;

}
#endif /* RV_SIP_PRIMITIVES */

/* ------------------------------------------------------------------------
    B A D _ S Y N T A X   F U N C T I O N S
   ------------------------------------------------------------------------*/



/***************************************************************************
 * CleanHeaderBeforeComma
 * ------------------------------------------------------------------------
 * General: this function handles this particular case:
 *          a list of headers, with comma between them. one of the headers
 *          (not the first) has a syntax error. without this function, the
 *          first header, which is legal, will be set to the message twice:
 *          once from the legal header that the parser created when parsing it,
 *          and once in the bad-syntax header string, that takes all the string
 *          from header name to the end.
 *          solution:
 *          Whenever we parse a legal header, that ends with comma, we go
 *          backwards, and erase this header, by setting spaces instead.
 *          now, if the next header after the comma has a syntax error, it won't
 *          take the already parsed header.
 *          note that the comma itself will be removed in the parser rule
 *          COMMA BETWEEN HEADERS.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:     pcb        - Pointer to the parser PCB structure.
 ***************************************************************************/
static void CleanHeaderBeforeComma(RvChar* pcbPointer,
                                   RvChar* pHeaderStart)
{
    RvChar *tempPtr = NULL;

    if((*(pcbPointer) != ','))
    {
        return;
    }

    tempPtr = pcbPointer;

    if(*(tempPtr) == NULL_CHAR)
    {
        --tempPtr;
    }

    /* 1. set tempPtr to point to the beginning of the header value buffer */
    tempPtr = pHeaderStart;
    
    /* 2. set spaces until first comma */
    while((*tempPtr) != ',')
    {
        *tempPtr = ' ';
        ++tempPtr;
    }
}

#ifndef RV_SIP_LIGHT
/***************************************************************************
 * GetCSeqStepFromMsgBuff
 * ------------------------------------------------------------------------
 * General: Retrieve CSeq step value from the parsed message buffer
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 *  Input:  pBuffer - The string buffer.
 *          length  - The length of the string number in the buffer.
 *  Output: pNum    - pointer to the output numeric number.
 ***************************************************************************/
static RvStatus GetCSeqStepFromMsgBuff(IN  ParserMgr *pParserMgr,
								  	   IN  RvChar    *pBuffer,
									   IN  RvUint32   length,
									   OUT RvInt32   *pCSeqStep)
{
	RvStatus rv;

#ifdef RV_SIP_UNSIGNED_CSEQ
	rv = ParserGetUINT32FromString(pParserMgr,pBuffer,length,(RvUint32 *)pCSeqStep);
	if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			"GetCSeqStepFromMsgBuff - error in ParserGetUINT32FromString"));
        return rv;
    }
#else /* #ifdef RV_SIP_UNSIGNED_CSEQ */
    rv = ParserGetINT32FromString(pParserMgr,pBuffer,length,pCSeqStep);
	if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			"GetCSeqStepFromMsgBuff - error in ParserGetINT32FromString"));
        return rv;
    }
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */

	return RV_OK;
}

#endif /*#ifndef RV_SIP_LIGHT*/

#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
/***************************************************************************
 * ParserInitializeInfoFromPcb
 * ------------------------------------------------------------------------
 * General: This function is used from the parser to initialize an Info header.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:   pParserMgr  - Pointer to the parser manager.
 *             pInfoVal    - Info structure holding the Info values from the
 *                            parser in a token form.
 *             hPool       - Handle to the header pool.
 *             hPage       - Handle to the header page.
 *  In/Output: hInfoHeader - Handle to the Info header which will be filled.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitializeInfoFromPcb(
                                    IN    ParserMgr             *pParserMgr,
                                    IN    ParserInfoHeader      *pInfoVal,
                                    IN    HRPOOL                 hPool,
                                    IN    HPAGE                  hPage,
                                    INOUT RvSipInfoHeaderHandle *hInfoHeader)
{
    RvStatus           rv;
    RvSipAddressHandle hAddress;
    RvSipAddressType   eAddrType;

    eAddrType = ParserAddrType2MsgAddrType(pInfoVal->exUri.uriType);
    if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitializeInfoFromPcb - unknown parserAddressType %d", pInfoVal->exUri.uriType));
        return RV_ERROR_UNKNOWN;
    }

    /* Construct the address in the Info header */
    rv = RvSipAddrConstructInInfoHeader(*hInfoHeader,
                                        eAddrType,
                                        &hAddress);

    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				   "ParserInitializeInfoFromPcb - error in RvSipAddrConstructInInfoHeader"));
        return rv;
    }

    /* Initialize the address values from the parser*/
    rv = ParserInitAddressInHeader(pParserMgr, &pInfoVal->exUri,
                                   hPool, hPage, eAddrType, hAddress);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
				  "ParserInitializeInfoFromPcb - error in ParserInitAddressInHeader"));
        return rv;
    }

	/* Check whether there are other params, if so, concatenate it to the
       strOtherParams string in the Info header*/
    if (RV_TRUE==pInfoVal->isOtherParams)
    {
        RvInt32 offsetOtherParam = UNDEFINED;
        offsetOtherParam = ParserAppendCopyRpoolString(
                    pParserMgr,
                    hPool,
                    hPage,
                    ((ParserExtensionString *)pInfoVal->otherParams)->hPool,
                    ((ParserExtensionString *)pInfoVal->otherParams)->hPage,
                    0,
                    ((ParserExtensionString *)pInfoVal->otherParams)->size);

        if (UNDEFINED == offsetOtherParam)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitializeInfoFromPcb - error in ParserAppendCopyRpoolString"));
             return RV_ERROR_OUTOFRESOURCES;
        }
        rv = SipInfoHeaderSetOtherParams(*hInfoHeader,
                                         NULL,
                                         hPool,
                                         hPage,
                                         offsetOtherParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitializeInfoFromPcb - error in SipInfoHeaderSetOtherParams"));
            return rv;
        }
    }

    return RV_OK;
}

/***************************************************************************
 * ParserInitializeReplyToFromPcb
 * ------------------------------------------------------------------------
 * General: This function is used from the parser to init the message with the
 *         Reply-To header values.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:   pParserMgr  - Pointer to the parser manager.
 *             ReplyToVal  - ReplyTo structure holding the Reply-To values from the
 *                           parser in a token form.
 *             hPool       - Handle to the message pool.
 *             hPage       - Handle to the message page.
 *  In/Output: hReplyToHeader - Handle to the Reply-To header which will be filled.
 ***************************************************************************/
static RvStatus RVCALLCONV ParserInitializeReplyToFromPcb(
                                    IN    ParserMgr                *pParserMgr,
                                    IN    ParserReplyToHeader      *ReplyToVal,
                                    IN    HRPOOL                    hPool,
                                    IN    HPAGE                     hPage,
                                    INOUT RvSipReplyToHeaderHandle *hReplyToHeader)
{
    RvStatus          rv;
    RvSipAddressHandle hAddress;
    RvSipAddressType eAddrType;

    eAddrType = ParserAddrType2MsgAddrType(ReplyToVal->nameAddr.exUri.uriType);
    if(eAddrType == RVSIP_ADDRTYPE_UNDEFINED)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
           "ParserInitializeReplyToFromPcb - unknown parserAddressType %d", ReplyToVal->nameAddr.exUri.uriType));
        return RV_ERROR_UNKNOWN;
    }

    /* Construct the address in the Reply-To header */
    rv = RvSipAddrConstructInReplyToHeader(*hReplyToHeader,
                                            eAddrType,
                                            &hAddress);

    if (RV_OK!=rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
			"ParserInitializeReplyToFromPcb - error in RvSipAddrConstructInReplyToHeader"));
        return rv;
    }

    /* Initialize the address values from the parser*/
    rv = ParserInitAddressInHeader(pParserMgr, &ReplyToVal->nameAddr.exUri,
                                   hPool, hPage, eAddrType, hAddress);
    if (RV_OK != rv)
    {
        RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
					"ParserInitializeReplyToFromPcb - error in ParserInitAddressInHeader"));
        return rv;
    }

	/* Check whether the display name has been set, if so attach
       it to the Reply-To header  */
    if (RV_TRUE == ReplyToVal->nameAddr.isDisplayName)
    {
        RvInt32 offsetName = UNDEFINED;
        rv= ParserAllocFromObjPage(pParserMgr,
                                      &offsetName,
                                      hPool,
                                      hPage,
                                      ReplyToVal->nameAddr.name.buf ,
                                      ReplyToVal->nameAddr.name.len);
        if (RV_OK!=rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitializeReplyToFromPcb - error in ParserAllocFromObjPage"));
            return rv;
        }

#ifndef RV_SIP_JSR32_SUPPORT
        /* Set the display name in the Reply-To header */
        rv = SipReplyToHeaderSetDisplayName(*hReplyToHeader,
                                             NULL,
                                             hPool,
                                             hPage,
                                             offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitializeReplyToFromPcb - error in ReplyToHeaderSetDisplayName"));
            return rv;
        }
#else
		/* This is the jsr32 case where the display name is part of the address object */
		rv = SipAddrSetDisplayName(hAddress,
                                      NULL,
                                      hPool,
                                      hPage,
                                      offsetName);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitializeReplyToFromPcb - error in SipAddrSetDisplayName"));
            return rv;
        }
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
    }

    /* Check whether there are other params, if so, concatenate it to the
       strOtherParams string in the Reply-To header*/
    if (RV_TRUE==ReplyToVal->isOtherParams)
    {
        RvInt32 offsetOtherParam = UNDEFINED;
        offsetOtherParam = ParserAppendCopyRpoolString(
                    pParserMgr,
                    hPool,
                    hPage,
                    ((ParserExtensionString *)ReplyToVal->otherParams)->hPool,
                    ((ParserExtensionString *)ReplyToVal->otherParams)->hPage,
                    0,
                    ((ParserExtensionString *)ReplyToVal->otherParams)->size);

        if (UNDEFINED == offsetOtherParam)
        {
             RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitializeReplyToFromPcb - error in ParserAppendCopyRpoolString"));
             return RV_ERROR_OUTOFRESOURCES;
        }
        rv = SipReplyToHeaderSetOtherParams(*hReplyToHeader,
                                             NULL,
                                             hPool,
                                             hPage,
                                             offsetOtherParam);
        if (RV_OK != rv)
        {
            RvLogError(pParserMgr->pLogSrc ,(pParserMgr->pLogSrc ,
						"ParserInitializeReplyToFromPcb - error in ReplyToHeaderSetOtherParams"));
            return rv;
        }
    }

    return RV_OK;
}
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */


