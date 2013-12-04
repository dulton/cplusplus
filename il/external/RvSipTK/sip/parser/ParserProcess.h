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
 *                              <ParserProcess.h>
 *
 * This file defines functions which are used from the parser to initialize sip
 * message or sip header.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *  Michal Mashiach                 DEC 2000
 *********************************************************************************/

#ifndef PROCESSPARSER_H
#define PROCESSPARSER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#include "ParserTypes.h"
#include "_SipParserManager.h"
#include "RvSipMsg.h"
#include "_SipMsg.h"


#if defined RV_SIP_LIGHT
#include "ParserEngineSipLight.h"
#elif defined RV_SIP_PRIMITIVES
#include "ParserEngineSipPrimitives.h"
#elif defined RV_SIP_JSR32_SUPPORT
#include "ParserEngineJSR32.h"
#elif defined RV_SIP_IMS_HEADER_SUPPORT
#include "ParserEngineIMS.h"
#elif defined RV_SIP_EXTENDED_HEADER_SUPPORT
#include "ParserEngineExtendedHeaders.h"
#elif defined RV_SIP_TEL_URI_SUPPORT
#include "ParserEngineTel.h"
#else
#include "ParserEngineClassic.h"
#endif

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
                                     INOUT ParserOtherHeader *pOther);

/***************************************************************************
 * ParserInitUrl
 * ------------------------------------------------------------------------
 * General: This function is used to init a sip url handle with the
 *          url values from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:      url  - The url parameters from the parser.
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
                          INOUT const void         *hUrl);

/***************************************************************************
 * ParserInitAbsUri
 * ------------------------------------------------------------------------
 * General: This function is used to init a abs url handle with the
 *          uri values from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:      uri  - The uri parameters from the parser.
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
                          INOUT const void         *hUri);

#ifdef RV_SIP_TEL_URI_SUPPORT
/***************************************************************************
 * ParserInitTelUri
 * ------------------------------------------------------------------------
 * General: This function is used to init a tel url handle with the
 *          uri values from the parser.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:      uri  - The uri parameters from the parser.
 *                eType- This type indicates that the parser is called to
 *                     parse stand alone tel-uri.
 *    In/OutPut:  hUri - Handle to the address structure which will be
 *                     set by the parser.
 ***************************************************************************/
RvStatus RVCALLCONV ParserInitTelUri(
                          IN    ParserMgr          *pParserMgr,
                          IN    SipParser_pcb_type *pcb,
                          IN    ParserTelUri       *uri,
                          IN    SipParseType       eType,
                          INOUT const void         *hUri);
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
                          INOUT const void          *hUri);
                          
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */
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
                            INOUT const void      *pSipObject);

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
                                    INOUT const void             *pSipMsg);

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
                             INOUT const void        *pSipMsg );

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
                                   INOUT const void      *pSipObject);
#else
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
                                  INOUT const void        *pSipMsg);

#endif /* RV_SIP_PRIMITIVES*/

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
                                   INOUT const void      *pSipObject);
#endif /* RV_SIP_PRIMITIVES */

/***************************************************************************
 * ParserInitStatusLine
 * ------------------------------------------------------------------------
 * General: This function is used from the parser to init the message with the
 *          status line header values.
 * Return Value:RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pStatusLine  - Status line structure holding the values
 *                            from the parser in a token form.
 *    In/Output: pSipMsg      - Pointer to the structure in which the values
 *                            from the parser will be set.
 ***************************************************************************/
RvStatus RVCALLCONV ParserInitStatusLine(
                                 IN    ParserMgr *pParserMgr,
                                 IN    SipParser_pcb_type* pcb,
                                 IN    RvUint8          *pcbPointer,
                                 IN    ParserStatusLine *pStatusLine,
                                 INOUT const void       *pSipMsg);
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
                                    INOUT const void        *pSipMsg);


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
                                   INOUT const void         *pSipObject);

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
                                     INOUT const void   *pSipObject);

#endif /* RV_SIP_PRIMITIVES*/

/***************************************************************************
* ParserInitParty
* ------------------------------------------------------------------------
* General: This function is used from the parser to init the message with the
*          To and From header values.
* Return Value: RV_OK        - on success
*               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
* ------------------------------------------------------------------------
* Arguments:
*    Input:   pParserMgr - Pointer to the parser manager.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             pPartyVal  - Party structure holding the values from the
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
                            INOUT const void         *pSipObject);


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
                                           INOUT const void      *pSipObject);

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
                          INOUT const void           *pSipObject);

/***************************************************************************
 * ParserInitExpiresHeaderInHeader
 * ------------------------------------------------------------------------
 * General: This function is used to init the expires handle with the
 *          expires param values.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pExpiresVal   - Expires structure holding the Expires values from the
 *                             parser in a token form.
 *  In/Output: hExpiresHeader- Pointer to the structure in which the values from
 *                             the parser will be set.
 ***************************************************************************/
/*RvStatus  RVCALLCONV ParserInitExpiresHeaderInHeader(
                                     IN    ParserMgr                *pParserMgr,
                                     IN    ParserExpiresHeader      *pExpiresVal,
                                     INOUT RvSipExpiresHeaderHandle  hExpiresHeader);
*/

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
                             INOUT const void                *pSipObject);

/***************************************************************************
 * ParserInitRoute
 * ------------------------------------------------------------------------
 * General: This function is used from the parser to init the message with the
 *          Route header values.
 * Return Value: RV_OK        - on success
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources).
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:     pRouteVal  - Route structure holding the Route values from the
 *                          parser in a token form.
 *             eType      - This type indicates whether the parser is called to
 *                          parse stand alone Route or sip message.
 *  In/Output: pSipObject - Pointer to the structure in which the values from
 *                          the parser will be set.
 *                          If eType == SIP_PARSETYPE_MSG it will be cast to
 *                          RvSipMsgHandle and if eType == SIP_PARSETYPE_ROUTE
 *                          it will be cast to RvSipContactHandle.
 ***************************************************************************/
RvStatus RVCALLCONV ParserInitRoute(   IN    ParserMgr          *pParserMgr,
                                       IN    SipParser_pcb_type *pcb,
                                       IN    RvUint8           *pcbPointer,
                                       IN    ParserRouteHeader  *pRouteVal,
                                       IN    SipParseType       eType,
                                       INOUT const void         *pSipObject);

#ifndef RV_SIP_PRIMITIVES /* defined by express users */

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
                             INOUT const void    *pSipObject);

/*-------------------------------------------------------------------*/
/*                          SUBS-REFER HEADERS                       */
/*-------------------------------------------------------------------*/
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
*    Input:    pParserMgr - Pointer to the parser manager.
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
                               INOUT const void          *pSipObject);

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
                               INOUT const void             *pSipObject);


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
                           INOUT const void        *pSipObject);

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
                             IN    SipParseType              eType,
                             INOUT const void                *pSipObject);


#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /* RV_SIP_PRIMITIVES */
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
                               INOUT const void          *pSipObject);

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
                              INOUT const void       *pSipObject);
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
                                  IN    SipParser_pcb_type *pcb,
                                  IN    RvUint8                   *pcbPointer,
                                  IN    ParserAuthenticationHeader *pAuthHeaderVal,
                                  IN    SipParseType               eType,
                                  IN    SipParseType              eWhichHeader,
                                  INOUT const void                 *pSipObject);

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
                                 IN    SipParser_pcb_type *pcb,
                                 IN    RvUint8                  *pcbPointer,
                                 IN    ParserAuthorizationHeader *pAuthHeaderVal,
                                 IN    SipParseType              eType,
                                 IN    SipParseType              eWhichHeader,
                                 INOUT const void                *pSipObject);
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
                                 INOUT const void                     *pSipObject);

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
                                INOUT const void      *pSipObject);


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
                            INOUT const void      *pSipObject);

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
                              INOUT const void                *pSipObject);

/***************************************************************************
 * ParserInitSessionExpiresHeader
 * ------------------------------------------------------------------------
 * General: This function is used to init a Session Expires header with the
 *          Session Sxpires values from the parser.
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
                        INOUT const void                  *pSipObject);

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
                           INOUT const void         *pSipObject);


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
                            INOUT const void        *pSipObject);

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
                        INOUT const void        *pSipObject);


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
*                             parse stand alone Reason.
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
                            INOUT const void         *pSipObject);

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
*                             RvSipMsgHandle and if eType == SIP_PARSETYPE_REASON
*                             it will be cast to RvSipWarningHeaderHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitWarningHeader(
                            IN    ParserMgr           *pParserMgr,
                            IN    RvUint8             *pcbPointer,
                            IN    ParserWarningHeader *pWarning,
                            IN    SipParseType         eType,
                            INOUT const void          *pSipObject);

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
                            INOUT const void            *pSipObject);

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
                            INOUT const void            *pSipObject);

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
                            INOUT const void          *pSipObject);

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
                            INOUT const void                  *pSipObject);

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
                            INOUT const void                  *pSipObject);

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
                             INOUT const void           *pSipObject);

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
                             INOUT const void                *pSipObject);

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
                             INOUT const void               *pSipObject);

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
                             INOUT const void               *pSipObject);

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
                             INOUT const void               *pSipObject);

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
                             INOUT const void               *pSipObject);

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
                             INOUT const void               *pSipObject);

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
*             eType        - This type indicates whether the parser is called to
*                            parse stand alone Security or sip message.
*			  eWhichHeader - This Type indicates which of the subtypes of Security
*						     Header the header is.
*  In/Output: pSipObject   - Pointer to the structure in which the values from
*                            the parser will be set.
*                            If eType == SIP_PARSETYPE_MSG it will be cast to
*                            RvSipMsgHandle and if eType is on of: SIP_PARSETYPE_SECURITY_CLIENT,
*							 SIP_PARSETYPE_SECURITY_SERVER, SIP_PARSETYPE_SECURITY_VERIFY, 
*                            it will be cast to RvSipSecurityHandle.
***************************************************************************/
RvStatus RVCALLCONV ParserInitSecurityHeader(
                             IN    ParserMgr			*pParserMgr,
                             IN    SipParser_pcb_type	*pcb,
                             IN    RvUint8				*pcbPointer,
                             IN    SipParseType			 eType,
							 IN	   SipParseType			 eWhichHeader,
                             INOUT const void	        *pSipObject);

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
                             INOUT const void                *pSipObject);

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
                             INOUT const void                *pSipObject);

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
                             INOUT const void               *pSipObject);

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
                             INOUT const void                *pSipObject);
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
*    Input:   pParserMgr - Pointer to the parser manager.
*             pcb		 - PDCSTracePartyID structure holding the  
*						   values from the parser in a token form.
*             pcbPointer - Pointer to the current location of parser pointer
*                          in msg buffer (the end of the given header).
*             eType      - This type indicates whether the parser is called to
                          parse stand alone PDCSTracePartyID or sip message.
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
                             INOUT const void               *pSipObject);

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
*                          parse stand alone PDCSOSPS or sip message.
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
                             INOUT const void               *pSipObject);

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
*             pcb		 - PDCSBillingInfo structure holding the  
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
                             INOUT const void               *pSipObject);

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
                             INOUT const void               *pSipObject);

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
*             pcb		 - PDCSRedirect structure holding the  
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
                             INOUT const void               *pSipObject);

#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */ 
#endif /* RV_SIP_PRIMITIVES */


#ifdef __cplusplus
}
#endif

#endif  /*END OF: define PROCESSPARSER_H*/
