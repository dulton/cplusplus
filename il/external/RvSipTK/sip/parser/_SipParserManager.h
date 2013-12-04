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
 *                              <SipParserManager.h>
 *    This file contains the initialization of the sip parser moudle.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *  Michal Mashiach                Dec 2000
 *********************************************************************************/


#ifndef SIPPARSERMANAGER_H
#define SIPPARSERMANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "AdsRpool.h"
#include "rvlog.h"
#include "RvSipMsgTypes.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/* declaring the handle of the DNS list object */
RV_DECLARE_HANDLE(SipParserMgrHandle);

#define CR        0x0D
#define LF        0x0A
#define NULL_CHAR 0x00

typedef enum
{

    SIP_PARSETYPE_UNDEFINED = -1,
    SIP_PARSETYPE_ALLOW,                /*0*/
    SIP_PARSETYPE_CONTACT,
    SIP_PARSETYPE_EXPIRES,
    SIP_PARSETYPE_DATE,
    SIP_PARSETYPE_VIA,
    SIP_PARSETYPE_CALL_ID,
    SIP_PARSETYPE_CSEQ,
    SIP_PARSETYPE_TO,
    SIP_PARSETYPE_FROM,
    SIP_PARSETYPE_OTHER,
    SIP_PARSETYPE_URLADDRESS,           /*10*/
    SIP_PARSETYPE_URIADDRESS,
    SIP_PARSETYPE_TELURIADDRESS,
    SIP_PARSETYPE_ANYADDRESS,
	SIP_PARSETYPE_WWW_AUTHENTICATE,
    SIP_PARSETYPE_PROXY_AUTHENTICATE,
    SIP_PARSETYPE_AUTHORIZATION,
    SIP_PARSETYPE_PROXY_AUTHORIZATION,
    SIP_PARSETYPE_AUTHENTICATION_INFO,
    SIP_PARSETYPE_ROUTE,
    SIP_PARSETYPE_RECORD_ROUTE,         /*20*/
    SIP_PARSETYPE_REFER_TO,				
    SIP_PARSETYPE_REFERRED_BY,			
    SIP_PARSETYPE_REPLACES,             
    SIP_PARSETYPE_CONTENT_TYPE,
    SIP_PARSETYPE_CONTENT_DISPOSITION,
    SIP_PARSETYPE_RSEQ,
    SIP_PARSETYPE_RACK,
    SIP_PARSETYPE_PROXY_REQUIRE,
    SIP_PARSETYPE_SUPPORTED,
    SIP_PARSETYPE_UNSUPPORTED,          /*30*/
    SIP_PARSETYPE_REQUIRE,				
    SIP_PARSETYPE_MIME_BODY_PART,		
    SIP_PARSETYPE_RETRY_AFTER,          
    SIP_PARSETYPE_ALLOW_EVENTS,
    SIP_PARSETYPE_EVENT,
    SIP_PARSETYPE_SUBSCRIPTION_STATE,
    SIP_PARSETYPE_SESSION_EXPIRES,
    SIP_PARSETYPE_MINSE,
	SIP_PARSETYPE_REASON,
	SIP_PARSETYPE_WARNING,              /*40*/
    SIP_PARSETYPE_MSG,                  
    SIP_PARSETYPE_START_LINE,           
    SIP_PARSETYPE_CONTENT_LENGTH,
    SIP_PARSETYPE_URL_HEADERS_LIST,
	SIP_PARSETYPE_TIMESTAMP,
	SIP_PARSETYPE_ALERT_INFO,
	SIP_PARSETYPE_CALL_INFO,
	SIP_PARSETYPE_ERROR_INFO,
	SIP_PARSETYPE_ACCEPT,
	SIP_PARSETYPE_ACCEPT_ENCODING,      /*50*/
	SIP_PARSETYPE_ACCEPT_LANGUAGE,      
	SIP_PARSETYPE_REPLY_TO,
    SIP_PARSETYPE_PATH,
    SIP_PARSETYPE_SERVICE_ROUTE,
	SIP_PARSETYPE_P_ASSOCIATED_URI,
	SIP_PARSETYPE_P_CALLED_PARTY_ID,
	SIP_PARSETYPE_P_ASSERTED_IDENTITY,
	SIP_PARSETYPE_P_PREFERRED_IDENTITY,
	SIP_PARSETYPE_P_VISITED_NETWORK_ID,
	SIP_PARSETYPE_P_ACCESS_NETWORK_INFO,/*60*/
	SIP_PARSETYPE_P_CHARGING_FUNCTION_ADDRESSES,
	SIP_PARSETYPE_P_CHARGING_VECTOR,
	SIP_PARSETYPE_P_MEDIA_AUTHORIZATION,
    SIP_PARSETYPE_SECURITY_CLIENT,
	SIP_PARSETYPE_SECURITY_SERVER,
	SIP_PARSETYPE_SECURITY_VERIFY,
	SIP_PARSETYPE_P_PROFILE_KEY,
	SIP_PARSETYPE_P_DCS_TRACE_PARTY_ID,
	SIP_PARSETYPE_P_DCS_OSPS,
	SIP_PARSETYPE_P_DCS_BILLING_INFO,
	SIP_PARSETYPE_P_DCS_LAES,           /*70*/
	SIP_PARSETYPE_P_DCS_REDIRECT,
    SIP_PARSETYPE_CONTENT_ID,
    SIP_PARSETYPE_DIAMETER_URI_ADDRESS,
    SIP_PARSETYPE_P_USER_DATABASE,
    SIP_PARSETYPE_P_ANSWER_STATE,
    SIP_PARSETYPE_P_SERVED_USER
}SipParseType;

/* XXX */

/*
 * ParserMgr is used to save the global variables of the parser module.
 */
typedef struct
{
    RvLogMgr*              pLogMgr;
    RvLogSource*           pLogSrc;     /* register number from the log manager*/
    HRPOOL                 pool;      /* Handle to the pool */
    RvSipMsgMgrHandle      hMsgMgr;

}ParserMgr;

typedef struct
{
    RvLogMgr*            pLogMgr;
    RvLogSource*        moduleLogId;
    HRPOOL                         hGeneralPool;
} SipParserMgrCfg;

/***************************************************************************
 * SipParserMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct the parser module.
 * Return Value: RV_OK
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCfg  - Structure with relevant configuration parameters.
 * Output:  phParserMgr - Handle to the parser mgr.
 ***************************************************************************/
RvStatus RVCALLCONV SipParserMgrConstruct(
                           IN SipParserMgrCfg*     pCfg,
                           OUT SipParserMgrHandle* phParserMgr);

/***************************************************************************
 * SipParserMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destruct the parser module.
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     phParserMgr - Handle to the parser mgr.
 ***************************************************************************/
void RVCALLCONV SipParserMgrDestruct(SipParserMgrHandle hParserMgr);

/***************************************************************************
 * SipParserMgrSetMsgMgrHandle
 * ------------------------------------------------------------------------
 * General: Set msgMgrHandle in parser manager.
 * Return Value: RV_OK
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 ***************************************************************************/
RvStatus RVCALLCONV SipParserMgrSetMsgMgrHandle(
                           IN SipParserMgrHandle hParserMgr,
                           IN RvSipMsgMgrHandle  hMsgMgr);
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
                                         INOUT void               *hSipObject);

#ifdef __cplusplus
}
#endif

#endif  /*END OF: define SIPPARSERMANAGER_H*/
