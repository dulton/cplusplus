/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        SipMessage.h                                        *
 *                                                                            *
 * This file defines the 'internal API' of the message object.                *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             Nov.2000                                             *
 ******************************************************************************/



#ifndef SIPMSG_H
#define SIPMSG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rpool_API.h"
#include "RvSipMsgTypes.h"
#include "RvSipCommonList.h"




/***************************************************************************************
    INTERNAL UTILS API FUNCTIONS
************************************************************************************/

/****************************************************/
/*        POOL-PAGE FUNCTIONS                           */
/****************************************************/

/***************************************************************************
 * SipMsgGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the message object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hMsg - The msg to take the page from.
 ***************************************************************************/
HRPOOL SipMsgGetPool(RvSipMsgHandle hMsg);

/***************************************************************************
 * SipMsgGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the message object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hMsg - The msg to take the page from.
 ***************************************************************************/
HPAGE SipMsgGetPage(RvSipMsgHandle hMsg);

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
                                 IN    RvUint32      bufferLen);

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
RvInt32 SipMsgGetStrRequestMethod(IN  RvSipMsgHandle   hSipMsg);

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
 *          methodOffset - the offset of the method string, in case of attach = RV_TRUE.
 *                   (if attach = RV_FALSE, it can be UNDEFINED).
 ***************************************************************************/
RvStatus SipMsgSetMethodInRequestLine(IN    RvSipMsgHandle  hSipMsg,
                                       IN    RvSipMethodType eRequsetMethod,
                                       IN    RvChar*        strMethod,
                                       IN    HRPOOL          hPool,
                                       IN    HPAGE           hPage,
                                       IN    RvInt32        methodOffset);

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
RvInt32 SipMsgGetReasonPhrase(IN RvSipMsgHandle hSipMsg);

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
 *         reasonOffset - The offset of the reason string, in case of attach = RV_TRUE.
 *                  (if attach = RV_FALSE, it can be UNDEFINED)
 *         hPool - The pool on which the string lays (if relevant)
 *         hPage - The page on which the string lays (if relevant)
 ***************************************************************************/
RvStatus SipMsgSetReasonPhrase(IN    RvSipMsgHandle hSipMsg,
                                IN    RvChar*       strReasonPhrase,
                                IN    HRPOOL         hPool,
                                IN    HPAGE          hPage,
                                IN    RvInt32       reasonOffset);

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
RvInt32 SipMsgGetBody(const RvSipMsgHandle hSipMsg);

/***************************************************************************
 * SipMsgGetCallIdHeader
 * ------------------------------------------------------------------------
 * General: Retrieves a pointer to the CALL-ID string from the message.
 * Return Value: Offset of the callId string, or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipMsg - Handle of a message object.
 ***************************************************************************/
RvInt32 SipMsgGetCallIdHeader(IN RvSipMsgHandle hSipMsg);

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
RvInt32 SipMsgGetCallIdHeaderName(IN RvSipMsgHandle hSipMsg);

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
 *         offset - The offset of the reason string, in case of attach = RV_TRUE.
 *                  (if attach = RV_FALSE, it can be UNDEFINED)
 *         hPool - The pool on which the string lays (if relevant)
 *         hPage - The page on which the string lays (if relevant)
 ***************************************************************************/
RvStatus SipMsgSetCallIdHeader(IN    RvSipMsgHandle hSipMsg,
                                IN    RvChar*       strCallId,
                                IN    HRPOOL         hPool,
                                IN    HPAGE          hPage,
                                IN    RvInt32       offset);

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
                                IN    RvInt32        offset);

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
RvInt32 SipMsgGetContentTypeHeader(IN RvSipMsgHandle hSipMsg);


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
                                     IN    RvInt32       offset);

/***************************************************************************
 * SipMsgGetContentIDHeader
 * ------------------------------------------------------------------------
 * General: Retrieves a pointer to the Content-ID string from the list.
 * Return Value: offset of the content type string, or UNDEFINED if not exist.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipMsg - Handle of a message object.
 ***************************************************************************/
RvInt32 SipMsgGetContentIDHeader(IN RvSipMsgHandle hSipMsg);


/***************************************************************************
 * SipMsgSetContentIDHeader
 * ------------------------------------------------------------------------
 * General: This method sets the Content-ID header in the message.
 *          If the string lays on a memory pool copying will be maid only if
 *          the page and pool are different than the object's page and pool.
 *          Otherwise attach will be maid.
 * Return Value:  RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hSipMsg - Handle of a message object.
 *         strID - Content-ID to be set in the message object.
 *                  If NULL, the exist Content-ID in the msg will be removed,
 *         offset - The offset of the reason string.
 *         hPool - The pool on which the string lays (if relevant)
 *         hPage - The page on which the string lays (if relevant)
 ***************************************************************************/
RvStatus SipMsgSetContentIDHeader(IN    RvSipMsgHandle hSipMsg,
                                  IN    RvChar*        strID,
                                  IN    HRPOOL         hPool,
                                  IN    HPAGE          hPage,
                                  IN    RvInt32        offset);

#endif /*#ifndef RV_SIP_PRIMITIVES */

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
RvChar* SipMsgStatusCodeToString(RvInt16 Code);

/*RvStatus RVCALLCONV SipHeaderValueParse(
                                         IN    SipParseType             parseType,
                                         IN    void*                    hHeader,
                                         IN    RvChar*                 buffer);
*/
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
                                    OUT RvSipHeaderType *type);

/***************************************************************************
 * RvSipMsgIsListHeader
 * ------------------------------------------------------------------------
 * General: Determines weather a header is in the list. The headers that are not
 *          in the list are: To, From, CSeq, ContentType, ContentLength and CallId.
 * Return Value: Returns RV_TRUE if the header is in the headers list, RV_FALSE
 *               otherwise.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: eHeaderType - type of the header.
 ***************************************************************************/
RvBool SipMsgIsListHeader(IN RvSipHeaderType eHeaderType);

/***************************************************************************
 * SipMsgGetCallIDHeaderHandle
 * ------------------------------------------------------------------------
 * General: Returns the handle of the Call ID in the message, that its handle is specified.
 * Return Value: RvSipOtherHeaderHandle - handle to the Call ID in the message
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipMsg - The handle to the message.
 ***************************************************************************/
RvSipOtherHeaderHandle SipMsgGetCallIDHeaderHandle(IN    RvSipMsgHandle hSipMsg);

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
									 IN  RvSipOtherHeaderHandle  hCallId);
#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

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
                                                      RvChar*        pStrStartLine);

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
RvInt32 RVCALLCONV SipMsgGetBadSyntaxStartLine(RvSipMsgHandle hMsg);

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
                                                    RvChar*       pBSReasonPhrase);

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
RvInt32 RVCALLCONV SipMsgGetBadSyntaxReasonPhrase(RvSipMsgHandle hMsg);

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
                                     IN    RvSipMsgHandle                hSipMsg);

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
                                               RvSipCommonListHandle hHeadersList);

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
RvChar *RVCALLCONV SipMsgGetCompTypeName(IN RvSipCompType eCompType);

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
RvSipCompType RVCALLCONV SipMsgGetCompTypeEnum(IN RvChar *strCompType);

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
                                OUT RvSipOtherHeaderHandle* phHeader);

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
                                OUT RvSipOtherHeaderHandle* phHeader);

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
void SipMsgIncCounter(IN  RvSipMsgHandle hSipMsg);
#endif /* #ifdef RV_SIP_ENHANCED_HIGH_AVAILABILITY_SUPPORT */


#ifdef __cplusplus
}
#endif


#endif /* SIPMSG_H */

