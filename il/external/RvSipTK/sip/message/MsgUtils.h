/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                           MsgUtils.h                                       *
 *                                                                            *
 * The file contains internal functions for encoding and allocating.          *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             Nov.2000                                             *
 ******************************************************************************/

#ifndef SIPINTERNALFUNCSS_H
#define SIPINTERNALFUNCSS_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipCommonTypes.h"
#include "RvSipMsgTypes.h"
#include "AdsRpool.h"
#include "MsgTypes.h"

#include "_SipParserManager.h"

/*-----------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                            */
/*-----------------------------------------------------------------------*/
#define MsgUtilsGetNameValueSeperatorChar(bInUrlHeaders)		((bInUrlHeaders == RV_TRUE)? "=" : ": ")
#define MsgUtilsGetNameValueSeperatorCharForAddr(bInUrlHeaders)	((bInUrlHeaders == RV_TRUE)? "=" : ":")
#define MsgUtilsGetColonChar(bInUrlHeaders)						((bInUrlHeaders == RV_TRUE)? "%3A" : ":")
#define MsgUtilsGetSemiColonChar(bInUrlHeaders)					((bInUrlHeaders == RV_TRUE)? "%3B" : ";")
#define MsgUtilsGetEqualChar(bInUrlHeaders)						((bInUrlHeaders == RV_TRUE)? "%3D" : "=")
#define MsgUtilsGetAtChar(bInUrlHeaders)						((bInUrlHeaders == RV_TRUE)? "%40" : "@")
#define MsgUtilsGetQuotationMarkChar(bInUrlHeaders)				((bInUrlHeaders == RV_TRUE)? "%22" : "\"")
#define MsgUtilsGetCommaChar(bInUrlHeaders)						((bInUrlHeaders == RV_TRUE)? "%2C" : ",")
#define MsgUtilsGetLeftAngleChar(bInUrlHeaders)					((bInUrlHeaders == RV_TRUE)? "%3C" : "<")
#define MsgUtilsGetRightAngleChar(bInUrlHeaders)				((bInUrlHeaders == RV_TRUE)? "%3E" : ">")
#define MsgUtilsGetSpaceChar(bInUrlHeaders)						((bInUrlHeaders == RV_TRUE)? "%20" : " ")
#define MsgUtilsGetPlusChar(bInUrlHeaders)						((bInUrlHeaders == RV_TRUE)? "%2B" : "+")


/***************************************************************************
 * MsgUtilsAllocCopyRpoolString
 * ------------------------------------------------------------------------
 * General: The function is for setting a given string (with the NULL in the end)
 *          on a non consecutive part of the page.
 *          The given string is on a RPOOL page, and is given as pool+page+offset.
 *          Note that string must be Null-terminated. and that the allocation is
 *          for strlen+1 because of that.
 * Return Value: offset of the new string on destPage, or UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pMsgMgr - Pointer to the message manager.
 *        destPool     - Handle of the destination rpool.
 *        destPage     - Handle of the destination page
 *        sourcePool   - Handle of the source rpool.
 *        destPage     - Handle of the source page
 *          stringOffset - The offset of the string to set.
 ***************************************************************************/
RvInt32 RVCALLCONV MsgUtilsAllocCopyRpoolString(
                                         IN MsgMgr*  pMsgMgr,
                                         IN HRPOOL   destPool,
                                         IN HPAGE    destPage,
                                         IN HRPOOL   sourcePool,
                                         IN HPAGE    sourcePage,
                                         IN RvInt32 stringOffset);

/***************************************************************************
 * MsgUtilsSetString
 * ------------------------------------------------------------------------
 * General: Uses to set a string to an object of the message family.
 *          hOutPool and hOutPage represents the pool and page of the object
 *          itself and hStringPool and hStringPage represents the pool and
 *          page on which the string lays. If the pool and page are equal no
 *          copying is done, and the out offset updated. else the string
 *          is copied to the object's pool. If the string pool and page are
 *          not specified this means that the string is given within the
 *          buffer and it is then copied to the object's pool as well.
 * Return Value: RV_ERROR_BADPARAM, RV_ERROR_OUTOFRESOURCES, RV_OK,
 *               RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hStringPool - The pool in which the string lays. (see explanations up)
 *        hStringPage - The pool on which the string lays.
 *        stringOffset - The offset of the string.
 *        buffer - A buffer containing the string. (see explanations up)
 *        hOutPool - The object's pool.
 *        hOutPage - The object's page.
 * Output: pOffset - The offset of the string within the object's pool and page
 ***************************************************************************/
RvStatus RVCALLCONV  MsgUtilsSetString(
                            IN    MsgMgr*     pMsgMgr,
                            IN    HRPOOL      hStringPool,
                            IN    HPAGE       hStringPage,
                            IN const RvInt32 stringOffset,
                            IN    RvChar*    buffer,
                            IN    HRPOOL      hOutPool,
                            IN    HPAGE       hOutPage,
                            OUT   RvInt32   *pOffset);

/***************************************************************************
 * MsgUtilsFillingUserBuffer
 * ------------------------------------------------------------------------
 * General: The function fill a given buffer with a string.
 * Return Value: RV_ERROR_INSUFFICIENT_BUFFER or RV_OK.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPool     - Handle of the pool with the string.
 *        hPage     - Handle of the page with the string.
 *        paramOffset - The offset of the string to be fill.
 *          buffer    - buffer to fill with the requested parameter.
 *        bufferLen - the length of the buffer.
 * output:actualLen - The length of the requested parameter + 1 for null in the end.
 ***************************************************************************/
RvStatus RVCALLCONV  MsgUtilsFillUserBuffer(
                                       HRPOOL   hPool,
                                       HPAGE    hPage,
                                 const RvInt32 paramOffset,
                                       RvChar* buffer,
                                       RvUint  bufferLen,
                                       RvUint* actualLen);

/***************************************************************************
 * SipMsgUtilsAllocAlign
 * ------------------------------------------------------------------------
 * General: The function allocate consecutive memory on the given page.
 *          This function is used when you allocate memory for a struct,
 *          The address of the struct need to be aligned.
 * Return Value: The pointer to the allocated buffer. NULL if didn't succeed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPage -  Handle of the page.
 *          length - length of buffer to be allocate.
 *        consecutiveMemory - is the memory for the string is consecutive or not.
 ***************************************************************************/
RvChar* RVCALLCONV  SipMsgUtilsAllocAlign(
                               IN MsgMgr*   pMsgMgr,
                               IN HRPOOL    hPool,
                               IN HPAGE     hPage,
                               IN RvUint32 length,
                               IN RvBool   consecutiveMemory);

/***************************************************************************
 * MsgUtilsAllocSetString
 * ------------------------------------------------------------------------
 * General: The function is for setting the given string (with the NULL in the end)
 *          on a consecutive part of the page.
 *          Note that string must be Null-terminated. and that the allocation is
 *          for strlen+1 because of that.
 * Return Value: NULL or pointer to the new allocated string.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPage  - Handle of the page.
 *          string - The string to set.
 ***************************************************************************/
RvInt32 RVCALLCONV  MsgUtilsAllocSetString(
                                IN MsgMgr*   pMsgMgr,
                                IN HRPOOL   hPool,
                                IN HPAGE    hPage,
                                IN RvChar* string);



/***************************************************************************
 * MsgUtils_itoa
 * ------------------------------------------------------------------------
 * General: The function fill the buffer with a string that is the number value.
 * Return Value: None.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: number - The integer value.
 *        buffer - The buffer that will contain the string of the integer.
 ***************************************************************************/
void RVCALLCONV  MsgUtils_itoa(RvInt32 number, RvChar* buffer);

/***************************************************************************
 * MsgUtils_uitoa
 * ------------------------------------------------------------------------
 * General: The function fill the buffer with a string that is the number value.
 *          The number here is an unsigned int.
 * Return Value: None.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: number - The integer value.
 *        buffer - The buffer that will contain the string of the integer.
 ***************************************************************************/
void RVCALLCONV MsgUtils_uitoa(RvUint32 number, RvChar* buffer);

/***************************************************************************
 * ContactHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Contact header (as string) on it.
 *          format: "Contact: "
 *                  ("*"|(1#((name-addr|addr-spec)
 *                    *(";"contact-params))))
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the contact header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  ContactHeaderEncode(
                                  IN    RvSipContactHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  IN    RvBool                   bForceCompactForm,
                                  INOUT RvUint32*                length);


#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * AllowHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Allow header (as string) on it.
 *          format: "Allow: " Method
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the allow header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *          bFirstAllowInMessage - RV_TRUE if this parameter is the first Allow header
 *                                 between the Allow headers in the headers list,
 *                                 RV_FALSE otherwise.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: encoded - Pointer to the beginning of the encoded string on the page.
 *         length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV  AllowHeaderEncode(
                              IN    RvSipAllowHeaderHandle hHeader,
                              IN    HRPOOL                 hPool,
                              IN    HPAGE                  hPage,
                              IN    RvBool                   bFirstAllowInMessage,
                              IN    RvBool                bInUrlHeaders,
                              INOUT RvUint32*             length);

/***************************************************************************
 * EventHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Event header (as string) on it.
 *          format: "Event /o :event-package.event-template;id=x;event-param"
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Event header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV EventHeaderEncode(
                            IN    RvSipEventHeaderHandle hHeader,
                            IN    HRPOOL                 hPool,
                            IN    HPAGE                  hPage,
                            IN    RvBool                 bInUrlHeaders,
                            IN    RvBool                 bForceCompactForm,
                            INOUT RvUint32*              length);

#endif /* #ifndef RV_SIP_PRIMITIVES*/

/***************************************************************************
 * CSeqHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            CSeq header (as string) on it.
 *          CSeq = "CSeq: " 1*DIGIT Method
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the CSeq header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        b_isCSeq - is the header a Cseq header or it is internal to another header
 *                   like RAck.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV CSeqHeaderEncode(
                              IN    RvSipCSeqHeaderHandle  hHeader,
                              IN    HRPOOL                 hPool,
                              IN    HPAGE                  hPage,
                              IN    RvBool                b_isCSeq,
                              IN    RvBool                bInUrlHeaders,
                              INOUT RvUint32*             length);

/***************************************************************************
 * OtherHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            other header (as string) on it.
 *          message-header = headerName ":" [header-value]
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no header were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the other header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *          bFirstInMessage - RV_TRUE if the header encoded is the first header encoded from
 *                            all the other headers with the same name.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - if RV_TRUE, will force compact form to Supported headers only.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV OtherHeaderEncode(
                              IN    RvSipOtherHeaderHandle hHeader,
                              IN    HRPOOL                 hPool,
                              IN    HPAGE                  hPage,
                              IN    RvBool                 bFirstInMessage,
                              IN    RvBool                 bInUrlHeaders,
                              IN    RvBool                 bForceCompactForm,
                              INOUT RvUint32*              length);

/***************************************************************************
 * PartyHeaderEncode
 * General: Accepts pool and page for allocating memory, and encode the
 *            party header (as string) on it.
 *          partyHeader = ("From: "|"To: ") (name-addr|addr-spec)
 *                                          *(";"party-param)
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the party header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV PartyHeaderEncode(
                            IN    RvSipPartyHeaderHandle hHeader,
                            IN    HRPOOL                 hPool,
                            IN    HPAGE                  hPage,
                            IN    RvBool                isTo,
                            IN    RvBool                bInUrlHeaders,
                            IN    RvBool                bForceCompactForm,
                            INOUT RvUint32*             length);

/***************************************************************************
 * ViaHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            via header (as string) on it.
 *          via-header = "Via: " 1#(sent-protocol sent-by *(";"via-params)
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the party header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV ViaHeaderEncode(
                          IN    RvSipViaHeaderHandle   hHeader,
                          IN    HRPOOL                 hPool,
                          IN    HPAGE                  hPage,
                          IN    RvBool                bInUrlHeaders,
                          IN    RvBool                bForceCompactForm,
                          INOUT RvUint32*             length);

/***************************************************************************
 * AddrEncode
 * ------------------------------------------------------------------------
 * General: Accepts a pointer to an allocated memory and copies the value of
 *            Url as encoded buffer (string) onto it.
 *            The user should check the return code.  If the size of
 *            the buffer is not sufficient the method will return RV_ERROR_INSUFFICIENT_BUFFER
 *            and the user should send bigger buffer for the encoded message.
 * Return Value: RV_OK, RV_ERROR_INSUFFICIENT_BUFFER, RV_ERROR_UNKNOWN, RV_ERROR_BADPARAM.
 *               RV_ERROR_INVALID_HANDLE
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr - Handle of the url address object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bAddBrackets - Indicates whether the address should be wrapped by
 *                       brackets (<>). If the address has a display name (jsr32)
 *                       then it will use brackets anyway.
 * output: length  - The given length + the encoded address length.
 ***************************************************************************/
RvStatus RVCALLCONV  AddrEncode(
                     IN  RvSipAddressHandle hSipAddr,
                     IN  HRPOOL             hPool,
                     IN  HPAGE              hPage,
                     IN  RvBool             bInUrlHeaders,
					 IN  RvBool             bAddBrackets,
                     OUT RvUint32*          length);

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * AddrEncodeWithReplaces
 * ------------------------------------------------------------------------
 * General: This function encodes an address object similarly to AddrEncode(),
 *          with the addition of a Replaces parameter that is received from
 *          the caller of this function and is encoded as part of the address
 *          object. Notice: the Replaces parameter is used for URL addresses.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr  - Handle of the url address object.
 *        hReplaces - Handle of the replaces parameter.
 *        hPool     - Handle of the pool of pages
 *        hPage     - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bAddBrackets - Indicates whether the address should be wrapped by
 *                       brackets (<>). If the address has a display name (jsr32)
 *                       then it will use brackets anyway.
 * output: length   - The given length + the encoded address length.
 ***************************************************************************/
RvStatus RVCALLCONV  AddrEncodeWithReplaces(
                     IN  RvSipAddressHandle        hSipAddr,
					 IN  RvSipReplacesHeaderHandle hReplaces,
                     IN  HRPOOL                    hPool,
                     IN  HPAGE                     hPage,
                     IN  RvBool                    bInUrlHeaders,
					 IN  RvBool                    bAddBrackets,
                     OUT RvUint32*                 length);
#endif /*#ifndef RV_SIP_PRIMITIVES*/
/***************************************************************************
 * EncodeAddrSpec
 * ------------------------------------------------------------------------
 * General: Encodes the addr-spec of an address object on a given pool and page
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * input: pAddr    - Pointer to the address object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * Inout: length  - The given length + the encoded address length.
 ***************************************************************************/
RvStatus RVCALLCONV  EncodeAddrSpec(
								IN    MsgAddress*        pAddr,
								IN    HRPOOL             hPool,
								IN    HPAGE              hPage,
								IN    RvBool             bInUrlHeaders,
								INOUT RvUint32*          length);

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * AuthenticationHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Authentication header (as string) on it.
 *          format: "WWW-Authenticate: " AuthScheme digest-challenge
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle of the allow header object.
 *        hPool             - Handle of the pool of pages
 *        hPage             - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output:length           - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV AuthenticationHeaderEncode(
                                       IN    RvSipAuthenticationHeaderHandle hHeader,
                                       IN    HRPOOL                          hPool,
                                       IN    HPAGE                           hPage,
                                       IN    RvBool                bInUrlHeaders,
                                       INOUT RvUint32*                      length);

/***************************************************************************
 * AuthorizationHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Authorization header (as string) on it.
 *          format: "WWW-Authenticate: " AuthScheme digest-challenge
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle of the allow header object.
 *        hPool             - Handle of the pool of pages
 *        hPage             - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length           - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV AuthorizationHeaderEncode(
                                      IN    RvSipAuthorizationHeaderHandle  hHeader,
                                      IN    HRPOOL                          hPool,
                                      IN    HPAGE                           hPage,
                                      IN    RvBool                bInUrlHeaders,
                                      INOUT RvUint32*                      length);
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * AuthenticationInfoHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          AuthenticationInfo header (as string) on it.
 *          Only the field indicated by eAuthInfoType is encoded
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle of the allow header object.
 *        hPool             - Handle of the pool of pages
 *        hPage             - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
  *                   if so, reserved characters should be encoded in escaped
  *                   form, and '=' should be set after header name instead of ':'.
  * output: length           - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV AuthenticationInfoHeaderEncode(
                                      IN    RvSipAuthenticationInfoHeaderHandle  hHeader,
                                      IN    HRPOOL                               hPool,
                                      IN    HPAGE                                hPage,
                                      IN    RvBool                               bInUrlHeaders,
                                      INOUT RvUint32*                            length);
#endif /*RV_SIP_AUTH_ON*/

#ifndef RV_SIP_PRIMITIVES
/*                          REFER ENCODING                            */
/*--------------------------------------------------------------------*/
#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * ReferToHeaderEncode
 * General: Accepts pool and page for allocating memory, and encode the
 *            Refer-To header (as string) on it.
 *          Refer-To Header = ("Refer-To" | "r") ":" URL
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Refer-To header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV ReferToHeaderEncode(
                            IN    RvSipReferToHeaderHandle hHeader,
                            IN    HRPOOL                   hPool,
                            IN    HPAGE                    hPage,
                            IN    RvBool                   bInUrlHeaders,
                            IN    RvBool                   bForceCompactForm,
                            INOUT RvUint32*                length);

/***************************************************************************
 * ReferredByHeaderEncode
 * General: Accepts pool and page for allocating memory, and encode the
 *            Referred-By header (as string) on it.
 *          referred-By Header = ("Referred-By" | "b") ":" (name-addr|addr-spec)
 *                                          (referenced-url |
 *                                           (referenced-url ";" ref-signature) |
 *                                           (ref-signature ";" referenced-url))
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Referred-By header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output:length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV ReferredByHeaderEncode(
                                 IN    RvSipReferredByHeaderHandle hHeader,
                                 IN    HRPOOL                      hPool,
                                 IN    HPAGE                       hPage,
                                 IN    RvBool                      bInUrlHeaders,
                                 IN    RvBool                      bForceCompactForm,
                                 INOUT RvUint32*                   length);
#endif /* #ifdef RV_SIP_SUBS_ON */

/***************************************************************************
 * RSeqHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            RSeq header (as string) on it.
 *          RSeq = "RSeq: " 1*DIGIT
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the RSeq header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
  *                   if so, reserved characters should be encoded in escaped
  *                   form, and '=' should be set after header name instead of ':'.
  * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV RSeqHeaderEncode(
                              IN    RvSipRSeqHeaderHandle  hHeader,
                              IN    HRPOOL                 hPool,
                              IN    HPAGE                  hPage,
                              IN    RvBool                bInUrlHeaders,
                              INOUT RvUint32*             length);

/***************************************************************************
 * RAckHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            RAck header (as string) on it.
 *          RAck = "RAck: " 1*DIGIT
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the RAck header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 * output: encoded - Pointer to the beginning of the encoded string on the page.
 *         length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV RAckHeaderEncode(
                              IN    RvSipRAckHeaderHandle  hHeader,
                              IN    HRPOOL                 hPool,
                              IN    HPAGE                  hPage,
                              IN    RvBool                bInUrlHeaders,
                              INOUT RvUint32*             length);

/***************************************************************************
 * RetryAfterHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Retry-After header (as string) on it.
 *          format: "Retry-After: "
 *                  (SIP-date|delta-seconds)[comment]*(;retry-param)
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPool are NULL or the
 *                                     header is not initialized.
 *               RV_ERROR_NULLPTR       - pLength is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the retry after header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 * output: pLength  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV RetryAfterHeaderEncode(
                                  IN    RvSipRetryAfterHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                bInUrlHeaders,
                                  INOUT RvUint32*               pLength);

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * AllowEventsHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Allow-events header (as string) on it.
 *          format: "Allow-Events: " event-package.event-template
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Allow-events header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *          bFirstInMessage - RV_TRUE if this parameter is the first Allow_Events header
 *                    between the Allow-Events headers in the headers list,
 *                    RV_FALSE otherwise.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV AllowEventsHeaderEncode(
                                  IN    RvSipAllowEventsHeaderHandle hHeader,
                                  IN    HRPOOL                       hPool,
                                  IN    HPAGE                        hPage,
                                  IN    RvBool                       bFirstInMessage,
                                  IN    RvBool                       bInUrlHeaders,
                                  IN    RvBool                       bForceCompactForm,
                                  INOUT RvUint32                    *length);

#endif /* #ifdef RV_SIP_SUBS_ON */

/***************************************************************************
 * SessionExpiresHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Session Expires header (as string) on it.
 *          format: "Session Expires: "
 *                  delta-seconds [refresher]
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPool are NULL or the
 *                                     header is not initialized.
 *               RV_ERROR_NULLPTR       - pLength is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Session Expires header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output: pLength  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV SessionExpiresHeaderEncode(
                        IN    RvSipSessionExpiresHeaderHandle hHeader,
                        IN    HRPOOL                          hPool,
                        IN    HPAGE                           hPage,
                        IN    RvBool                          bInUrlHeaders,
                        IN    RvBool                          bForceCompactForm,
                        INOUT RvUint32*                       pLength);

/***************************************************************************
 * MinSEHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Min SE header (as string) on it.
 *          format: "Min SE: "
 *                  delta-seconds
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPool are NULL or the
 *                                     header is not initialized.
 *               RV_ERROR_NULLPTR       - pLength is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Min SE header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 * output: pLength  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV MinSEHeaderEncode(
                        IN    RvSipMinSEHeaderHandle hHeader,
                        IN    HRPOOL                 hPool,
                        IN    HPAGE                  hPage,
                        IN    RvBool                bInUrlHeaders,
                        INOUT RvUint32*             pLength);

/***************************************************************************
 * ReplacesHeaderEncode
 * General: Accepts pool and page for allocating memory, and encode the
 *            Replaces header (as string) on it.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Replaces header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 * output: encoded - Pointer to the beginning of the encoded string on the page.
 *         length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV ReplacesHeaderEncode(
                            IN    RvSipReplacesHeaderHandle hHeader,
                            IN    HRPOOL                   hPool,
                            IN    HPAGE                    hPage,
                            IN    RvBool                  bInUrlHeaders,
                            INOUT RvUint32*               length);
#endif /* RV_SIP_PRIMITIVES */

/***************************************************************************
 * ExpiresHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Expires header (as string) on it.
 *          format: "Expires: "
 *                  (SIP-date|delta-seconds)
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPool are NULL or the
 *                                     header is not initialized.
 *               RV_ERROR_NULLPTR       - pLength is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the expires header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bIsParameter - True if the expires is a parameter of a Contact
 *                       header and False if this is an Expires header. The
 *                       encoding is done appropriately.
 * output: pLength  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  ExpiresHeaderEncode(
                                  IN    RvSipExpiresHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                  bIsParameter,
                                  IN    RvBool                bInUrlHeaders,
                                  INOUT RvUint32*               pLength);

/***************************************************************************
 * DateHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Date header (as string) on it.
 *          format: "Date: " SIP-date
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPool are NULL or the
 *                                     header is not initialized.
 *               RV_ERROR_NULLPTR       - pLength is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the date header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bIsExpires - True if the date is a part of and Expires header.
 *          bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: pLength  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV DateHeaderEncode (
                             IN    RvSipDateHeaderHandle    hHeader,
                             IN    HRPOOL                   hPool,
                             IN    HPAGE                    hPage,
                             IN    RvBool                  bIsExpires,
                             IN    RvBool                 bInUrlHeaders,
                             INOUT RvUint32*               pLength);

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
 * output: pLength   - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV IntervalEncode(
                          IN    MsgInterval* pInterval,
                          IN    HRPOOL      hPool,
                          IN    HPAGE       hPage,
                          IN    RvBool     bIsParameter,
                          IN    RvBool     bInUrlHeaders,
                          INOUT RvUint32*  pLength);

/***************************************************************************
 * RouteHopHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Route header (as string) on it.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPool are NULL or the
 *                                     header is not initialized.
 *               RV_ERROR_NULLPTR       - pLength is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Route header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 * output: pLength  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV RouteHopHeaderEncode(
                              IN    RvSipRouteHopHeaderHandle   hHeader,
                              IN    HRPOOL                      hPool,
                              IN    HPAGE                       hPage,
                              IN    RvBool                bInUrlHeaders,
                              INOUT RvUint32*                  pLength);

/***************************************************************************
 * MsgUtilsEncodeExternalString
 * ------------------------------------------------------------------------
 *  General: The function set a string in the rpool page, by asking for memory
 *          and setting the string (with RPOOL_AppendFromExternal).
 *          Note - The length parameter add the encoded string length, to the
 *          given length. therefore if the length at the beginning was not 0,
 *          this is not the real length.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPage - Handle of the rpool page.
 *          string - the string to set.
 ***************************************************************************/
#ifndef RV_USE_MACROS

RvStatus RVCALLCONV MsgUtilsEncodeExternalString(
                                       IN    MsgMgr*       pMsgMgr,
                                       IN    HRPOOL        hPool,
                                       IN    HPAGE         hPage,
                                       IN    RvChar*       string,
                                       INOUT RvUint32*     length);
#else

RVINLINE RvStatus RVCALLCONV MsgUtilsEncodeExternalString(
                                       IN    MsgMgr*         pMsgMgr,
                                       IN    HRPOOL         hPool,
                                       IN    HPAGE          hPage,
                                       IN    RvChar*       string,
                                       INOUT RvUint32*     length)
{
    RvInt32           offset;
    RvStatus          stat;
    RPOOL_ITEM_HANDLE encoded;

    *length += (RvUint32)strlen(string);

    stat = RPOOL_AppendFromExternal(hPool,
                                    hPage,
                                    (void*)string,
                                    (RvInt)strlen(string),
                                    RV_FALSE,
                                    &offset,
                                    &encoded);
    if(stat != RV_OK)
    {
        RvLogError(pMsgMgr->pLogSrc,(pMsgMgr->pLogSrc,
            "MsgUtilsEncodeString - Failed, AppendFromExternal failed. hPool 0x%p, hPage 0x%p, RvStatus: %d",
            hPool, hPage, stat));
    }
    return stat;
}

#endif /*#ifndef RV_USE_MACROS*/

/***************************************************************************
 * MsgUtilsEncodeString
 * ------------------------------------------------------------------------
 * General: The function set a string on the rpool page without NULL in the end of it.
 *          The given string is from another rpool page (pool+page+offset).
 *          The setting os done by asking for memory on the dest page (RPOOL_Append)
 *          and setting the string (with RPOOL_CopyFrom).
 *          Note - The length parameter add the encoded string length, to the
 *          given length. Therefore if the length at the beginning was not 0,
 *          this is not the real length.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hPool       - Handle of the pool of the given string.
 *         hPage       - Handle of the pool page of the given string.
 *           stringOffet - The offset of the string to encode.
 * Output: length      - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeString(
                               IN  MsgMgr*        pMsgMgr,
                               IN HRPOOL          destPool,
                               IN HPAGE           destPage,
                               IN  HRPOOL         hPool,
                               IN  HPAGE          hPage,
                               IN  RvInt32       stringOffset,
                               OUT RvUint32*     length);

/******************************************************************************
 * MsgUtilsEncodeStringExt
 * ------------------------------------------------------------------------
 * General: The function set a string on the rpool page without NULL in the end
 *          of it. The given string is from another rpool page
 *          (pool+page+offset).
 *          The memory for the string is allocated from the Destination Pool
 *          and is added to the Destination Page using RPOOL_Append().
 *          The string firstly, is extracted from the Source Page to the buffer
 *          and the copied into the allocated memory on the Destination Page.
 *          The function ensures, that the encoded string is / is not
 *          surrounded by the quotes,if it was required by the calling function
 *          Note - The length parameter add the encoded string length, to the
 *          given length. Therefore if the length at the beginning was not 0,
 *          this is not the real length.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES - If allocation failed (no resources)
 *               error code, defined in the RV_SIP_DEF.h, otherwise
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pMsgMgr       - Handle of the Message Manager.
 *         destPool      - Handle of the pool for the encoded string.
 *         destPage      - Handle of the page for the encoded string.
 *         hPool         - Handle of the pool of the given string.
 *         hPage         - Handle of the page of the given string.
 *         stringOffet   - The offset of the given string in the hPage page.
 *         bSetQuotes    - If RV_TRUE, the encoded string will be surrounded
 *                         by the quotes (only once).
 *                         If RV_FALSE, the encoded string will be not
 *                         surrounded by the quotes.
 *         bInUrlHeaders - If RV_TRUE, the '%22' symbol will be used for quotes
 *                         If RV_FALSE, the '"' symbol will be used for quotes
 * Output: length        - The given length + the encoded string length.
 *****************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeStringExt(
                                            IN  MsgMgr*     pMsgMgr,
                                            IN HRPOOL       destPool,
                                            IN HPAGE        destPage,
                                            IN  HRPOOL      hPool,
                                            IN  HPAGE       hPage,
                                            IN  RvInt32     stringOffset,
                                            IN  RvBool      bSetQuotes,
                                            IN  RvBool      bInUrlHeaders,
                                            OUT RvUint32*   length);

/***************************************************************************
 * MsgUtilsEncodeMethodType
 * ------------------------------------------------------------------------
 * General: The function add the string of the method to the page, without '\0'.
 *          Note - if the eMethod argument is RVSIP_METHOD_OTHER, the function
 *          will set the strMethod value. else, strMethod will be ignored.
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPool        - Handle of the memory pool for encoded buffer.
 *        hPage        - Handle of the memory page for encoded buffer.
 *          eMethod      - Value ot the method to encode.
 *        stringPool   - Handle of the memory Pool where the strMethod is.
 *        stringPage   - Handle of the memory Page where the strMethod is.
 *        strMethod    - In case that eMethod is RVSIP_METHOD_OTHER,
 *                       this string offset will contain the mehtod value. else,
 *                       it will be UNDEFINED.
 * Output:length       - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeMethodType(
                                   IN MsgMgr*         pMsgMgr,
                                   IN HRPOOL          hPool,
                                   IN HPAGE           hPage,
                                   IN RvSipMethodType eMethod,
                                   IN HRPOOL          stringPool,
                                   IN HPAGE           stringPage,
                                   IN RvInt32        strMethod,
                                   OUT RvUint32*     length);

/***************************************************************************
 * MsgUtilsEncodeTransportType
 * ------------------------------------------------------------------------
 * General: The function add the string of the transport to the page, without '\0'.
 *          Note - not good for RVSIP_TRANSPORT_OTHER
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPage        - Handle of the memory pool for encoded buffer.
 *        hPage        - Handle of the memory page for encoded buffer.
 *          eTransport   - Value ot the transport to encode.
 *        stringPool   - Handle of the memory Pool where the strTransport is.
 *        stringPage   - Handle of the memory Page where the strTransport is.
 *        strTransportOffset - In case that eTransport is RVSIP_TRANSPORT_OTHER,
 *                       this string offset will contain the transport value. else,
 *                       it will be UNDEFINED.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeTransportType(
                                      IN MsgMgr*        pMsgMgr,
                                      IN HRPOOL         hPool,
                                      IN HPAGE          hPage,
                                      IN RvSipTransport eTransport,
                                      IN HRPOOL         stringPool,
                                      IN HPAGE          stringPage,
                                      IN RvInt32       strTransportOffset,
                                      OUT RvUint32*    length);

/***************************************************************************
 * MsgUtilsEncodeCompType
 * ------------------------------------------------------------------------
 * General: The function add the string of the compression to the page, without '\0'.
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPool        - Handle of the memory pool for encoded buffer.
 *        hPage        - Handle of the memory page for encoded buffer.
 *          eComp        - Value ot the compression type to encode.
 *        stringPool   - Handle of the memory Pool where the strTransport is.
 *        stringPage   - Handle of the memory Page where the strTransport is.
 *        strCompOffset - In case that eComp is RVSIP_COMP_OTHER,
 *                       this string offset will contain the comp value. else,
 *                       it will be UNDEFINED.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeCompType(
                                 IN MsgMgr*        pMsgMgr,
                                 IN HRPOOL         hPool,
                                 IN HPAGE          hPage,
                                 IN RvSipCompType eComp,
                                 IN HRPOOL         stringPool,
                                 IN HPAGE          stringPage,
                                 IN RvInt32       strCompOffset,
                                 OUT RvUint32*    length);

/***************************************************************************
 * MsgUtilsEncodeCRLF
 * ------------------------------------------------------------------------
 * General: The function set CRLF on the given page.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPage - Handle of the page.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeCRLF(
                             IN    MsgMgr*    pMsgMgr,
                             IN    HRPOOL     hPool,
                             IN    HPAGE      hPage,
                             INOUT RvUint32* length);

#ifdef RV_SIP_AUTH_ON
/***************************************************************************
 * MsgUtilsEncodeAuthScheme
 * ------------------------------------------------------------------------
 * General: The function add the string of the authentication scheme to the page,
 *          without '\0'.
 *          Note - if the eAuthScheme argument is RVSIP_AUTHENTICATION_SCHEME_OTHER, the function
 *          will set the strAuthScheme value. else, strAuthScheme will be ignored.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPool            - Handle of the memory pool for encoded buffer.
 *          hPage            - Handle of the memory page for encoded buffer.
 *            eAuthAlgorithm   - Value of the authentication algorithm to encode.
 *          sourcePool       - Handle of memory pool of strAuthScheme.
 *          sourcePage       - Handle of memory page of strAuthScheme.
 *          strAuthScheme - String with authentication scheme in it - when the eAuthScheme is
 *                          RVSIP_AUTHENTICATION_SCHEME_OTHER
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeAuthScheme(
                                   IN MsgMgr*                   pMsgMgr,
                                   IN HRPOOL                    hPool,
                                   IN HPAGE                     hPage,
                                   IN RvSipAuthScheme           eAuthScheme,
                                   IN HRPOOL                    sourcePool,
                                   IN HPAGE                     sourcePage,
                                   IN RvInt32                  strAuthScheme,
                                   OUT RvUint32*               length);

/***************************************************************************
 * MsgUtilsEncodeAuthAlgorithm
 * ------------------------------------------------------------------------
 * General: The function add the string of the authentication algorithm to the page,
 *          without '\0'.
 *          Note - if the eAuthAlgorithm argument is RVSIP_AUTHENTICATION_ALGORITHM_OTHER, the function
 *          will set the strAuthAlgorithm value. else, strAuthalgorithm will be ignored.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPool            - Handle of the memory pool for encoded buffer.
 *          hPage            - Handle of the memory page for encoded buffer.
 *            eAuthAlgorithm   - Value of the authentication algorithm to encode.
 *          sourcePool       - Handle of memory pool of strAuthAlgorithm.
 *          sourcePage       - Handle of memory page of strAuthAlgorithm.
 *          strAuthAlgorithm - String with authentication algorithm in it - when the eAuthAlgorithm is
 *                             RVSIP_AUTHENTICATION_ALGORITHM_OTHER
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeAuthAlgorithm(
                                      IN MsgMgr*                   pMsgMgr,
                                      IN HRPOOL                    hPool,
                                      IN HPAGE                     hPage,
                                      IN RvSipAuthAlgorithm        eAuthAlgorithm,
                                      IN HRPOOL                    sourcePool,
                                      IN HPAGE                     sourcePage,
                                      IN RvInt32                  strAuthAlgorithm,
                                      OUT RvUint32*               length);

/***************************************************************************
 * MsgUtilsEncodeQopOptions
 * ------------------------------------------------------------------------
 * General: The function add the string of the qop options to the page,
 *          without '\0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPage  - Handle of the memory page for encoded buffer.
 *            eQop   - Value of the qop options to encode.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeQopOptions(
                                   IN MsgMgr*               pMsgMgr,
                                   IN HRPOOL                hPool,
                                   IN HPAGE                 hPage,
                                   IN RvSipAuthQopOption    eQop,
                                   OUT RvUint32*           length);
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * MsgUtilsEncodeMediaType
 * ------------------------------------------------------------------------
 * General: The function add the string of the media type to the page
 *          (without '\0').
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPage        - Handle of the memory pool for encoded buffer.
 *        hPage        - Handle of the memory page for encoded buffer.
 *          eMediaType   - Value ot the media type to encode.
 *        stringPool   - Handle of the memory Pool where the strMediaTypeOffset is.
 *        stringPage   - Handle of the memory Page where the strMediaTypeOffset is.
 *        strMediaTypeOffset - In case that eMediaType is RVSIP_MEDIATYPE_OTHER,
 *                       this string offset will contain the media-type value. else,
 *                       it will be UNDEFINED.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeMediaType(
                                  IN MsgMgr*        pMsgMgr,
                                  IN HRPOOL         hPool,
                                  IN HPAGE          hPage,
                                  IN RvSipMediaType eMediaType,
                                  IN HRPOOL         stringPool,
                                  IN HPAGE          stringPage,
                                  IN RvInt32       strMediaTypeOffset,
                                  OUT RvUint32*    length);

/***************************************************************************
 * MsgUtilsEncodeMediaSubType
 * ------------------------------------------------------------------------
 * General: The function add the string of the media sub type to the page
 *          (without '\0').
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPage        - Handle of the memory pool for encoded buffer.
 *        hPage        - Handle of the memory page for encoded buffer.
 *          eMediaSubType   - Value ot the media sub type to encode.
 *        stringPool   - Handle of the memory Pool where the strMediaSubTypeOffset is.
 *        stringPage   - Handle of the memory Page where the strMediaSubTypeOffset is.
 *        strMediaSubTypeOffset - In case that eMediaSubType is
 *                       RVSIP_MEDIASUBTYPE_OTHER, this string offset will
 *                       contain the media sub typetype value. else, it will
 *                       be UNDEFINED.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeMediaSubType(
                                     IN MsgMgr*         pMsgMgr,
                                     IN  HRPOOL             hPool,
                                     IN  HPAGE              hPage,
                                     IN  RvSipMediaSubType  eMediaSubType,
                                     IN  HRPOOL             stringPool,
                                     IN  HPAGE              stringPage,
                                     IN  RvInt32           strMediaSubTypeOffset,
                                     OUT RvUint32*         length);

/***************************************************************************
 * MsgUtilsEncodeConsecutiveMediaType
 * ------------------------------------------------------------------------
 * General: The function copies the string of the media type on the given
 *          buffer (without '\0').
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: strBuffer    - The buffer to copy the media type on.
 *          eMediaType   - Value of the media type to encode.
 *        stringPool   - Handle of the memory Pool where the strMediaTypeOffset is.
 *        stringPage   - Handle of the memory Page where the strMediaTypeOffset is.
 *        strMediaTypeOffset - In case that eMediaType is RVSIP_MEDIATYPE_OTHER,
 *                       this string offset will contain the media-type value. else,
 *                       it will be UNDEFINED.
 *        bufferLen    - The length of the given buffer.
 *        bIsSuffitient - Is the buffer suffitient.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeConsecutiveMediaType(
                                  IN     MsgMgr*          pMsgMgr,
                                  IN     RvSipMediaType   eMediaType,
                                  IN     HRPOOL           stringPool,
                                  IN     HPAGE            stringPage,
                                  IN     RvInt32         strMediaTypeOffset,
                                  IN     RvUint          bufferLen,
                                  INOUT  RvBool         *bIsSuffitient,
                                  INOUT  RvChar        **strBuffer,
                                  OUT    RvUint*         length);

/***************************************************************************
 * MsgUtilsEncodeConsecutiveMediaSubType
 * ------------------------------------------------------------------------
 * General: The function add the string of the media sub type to the given
 *          consecutive buffer (without '/0').
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: strBuffer    - The buffer to copy the media sub type on.
 *          eMediaSubType   - Value ot the media sub type to encode.
 *        stringPool   - Handle of the memory Pool where the strMediaSubTypeOffset is.
 *        stringPage   - Handle of the memory Page where the strMediaSubTypeOffset is.
 *        strMediaSubTypeOffset - In case that eMediaSubType is
 *                       RVSIP_MEDIASUBTYPE_OTHER, this string offset will
 *                       contain the media sub typetype value. else, it will
 *                       be UNDEFINED.
 *        bufferLen    - The maximum length of the buffer.
 *        bIsSuffitient - Is the buffer suffitient.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeConsecutiveMediaSubType(
                                     IN     MsgMgr*             pMsgMgr,
                                     IN     RvSipMediaSubType   eMediaSubType,
                                     IN     HRPOOL              stringPool,
                                     IN     HPAGE               stringPage,
                                     IN     RvInt32            strMediaSubTypeOffset,
                                     IN     RvUint             bufferLen,
                                     IN     RvBool            *bIsSuffitient,
                                     INOUT  RvChar           **strBuffer,
                                     OUT    RvUint*            length);

/***************************************************************************
 * MsgUtilsEncodeDispositionType
 * ------------------------------------------------------------------------
 * General: The function add the string of the disposition type to the page
 *          (without '\0').
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPage        - Handle of the memory pool for encoded buffer.
 *        hPage        - Handle of the memory page for encoded buffer.
 *          eDispType    - Value ot the disposition type to encode.
 *        stringPool   - Handle of the memory Pool where the strDispOffset is.
 *        stringPage   - Handle of the memory Page where the strDispOffset is.
 *        strDispOffset - In case that eDispType is RVSIP_DISPOSITIONTYPE_OTHER,
 *                       this string offset will contain the disposition type
 *                       value. else, it will be UNDEFINED.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeDispositionType(
                                        IN MsgMgr*              pMsgMgr,
                                        IN HRPOOL               hPool,
                                        IN HPAGE                hPage,
                                        IN RvSipDispositionType eDispType,
                                        IN HRPOOL               stringPool,
                                        IN HPAGE                stringPage,
                                        IN RvInt32             strDispOffset,
                                        OUT RvUint32*          length);

/***************************************************************************
 * MsgUtilsEncodeDispositionHandling
 * ------------------------------------------------------------------------
 * General: The function add the string of the disposition handling parameter
 *          to the page (without '\0').
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPage        - Handle of the memory pool for encoded buffer.
 *        hPage        - Handle of the memory page for encoded buffer.
 *          eDispHandling   - Value ot the disposition handling to encode.
 *        stringPool   - Handle of the memory Pool where the strDispHandlingOffset is.
 *        stringPage   - Handle of the memory Page where the strDispHandlingOffset is.
 *        strDispHandlingOffset - In case that eDispHandling is
 *                       RVSIP_DISPOSITIONHANDLING_OTHER, this string offset will
 *                       contain the transport value. else, it will be UNDEFINED.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeDispositionHandling(
                                    IN MsgMgr*                  pMsgMgr,
                                    IN HRPOOL                   hPool,
                                    IN HPAGE                    hPage,
                                    IN RvSipDispositionHandling eDispHandling,
                                    IN HRPOOL                   stringPool,
                                    IN HPAGE                    stringPage,
                                    IN RvInt32                 strDispHandlingOffset,
                                    OUT RvUint32*              length);

/***************************************************************************
 * ContentTypeHeaderEncode
 * General: Accepts pool and page for allocating memory, and encode the
 *            Content-Type header (as string) on it.
 *          Content-Type = "Content-Type:" type "/" subtype (;parameters)
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no
 *                                     method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Content-Type header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV ContentTypeHeaderEncode(
                                  IN    RvSipContentTypeHeaderHandle hHeader,
                                  IN    HRPOOL                       hPool,
                                  IN    HPAGE                        hPage,
                                  IN    RvBool                       bInUrlHeaders,
                                  IN    RvBool                       bForceCompactForm,
                                  INOUT RvUint32*                    length);

/***************************************************************************
 * ContentTypeHeaderConsecutiveEncode
 * General: Accepts a consecutive buffer and encodes the Content-Type header
 *          as string on the buffer.
 *          Content-Type = "Content-Type:" type "/" subtype (;parameters)
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no
 *                                     method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Content-Type header object.
 *        strBuffer - Buffer to fill with the encoded header.
 *        bufferLen - The length of the buffer.
 * output:actualLen - The length of the requested parameter, + 1 to include
 *                     a NULL value at the end of the parameter.
 ***************************************************************************/
RvStatus RVCALLCONV ContentTypeHeaderConsecutiveEncode(
                                     IN  RvSipContentTypeHeaderHandle hHeader,
                                     IN  RvChar*                     strBuffer,
                                     IN  RvUint                      bufferLen,
                                     OUT RvUint*                     actualLen);

/***************************************************************************
 * ContentIDHeaderEncode
 * General: Accepts pool and page for allocating memory, and encode the
 *            Content-ID header (as string) on it.
 *          Content-ID = "Content-ID:" <addr-spec>
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no
 *                                     method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Content-ID header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV ContentIDHeaderEncode(
                                  IN    RvSipContentIDHeaderHandle hHeader,
                                  IN    HRPOOL                       hPool,
                                  IN    HPAGE                        hPage,
                                  IN    RvBool                       bInUrlHeaders,
                                  INOUT RvUint32*                    length);

/***************************************************************************
 * ContentDispositionHeaderEncode
 * General: Accepts pool and page for allocating memory, and encode the
 *            Content-Disposition header (as string) on it.
 *          Content-Disposition =
 *          "Content-Disposition:" disposition-type *(hanling-parameter)
 *                                                  *(;parameters)
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no
 *                                     method was given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Content-Disposition header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV ContentDispositionHeaderEncode(
                             IN    RvSipContentDispositionHeaderHandle hHeader,
                             IN    HRPOOL                              hPool,
                             IN    HPAGE                               hPage,
                             IN    RvBool                bInUrlHeaders,
                             INOUT RvUint32*                          length);

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
                     INOUT RvUint32*                       length);

/***************************************************************************
 * BodyPartEncode
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
                         INOUT RvUint32*                   length);

/***************************************************************************
 * BodyPushBodyPart
 * ------------------------------------------------------------------------
 * General: Inserts a new body part into the body parts list based on a
 *          given location - first, last, before or after a given relative
 *          element. You don't supply a body part for this function.
 *          The function returnes the handle to the body part object that
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
                                IN   RvSipListLocation          location,
                                IN   RvSipBodyPartHandle        hRelative,
                                OUT  RvSipBodyPartHandle       *hNewBodyPart);

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
                              IN   RvSipBodyPartHandle          hBodyPart,
                              IN   RvSipHeadersLocation         location,
                              IN   RvSipOtherHeaderHandle       hRelative,
                              OUT  RvSipOtherHeaderHandle      *hNewHeader);

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
RvStatus RVCALLCONV BodyUpdateContentType(IN RvSipBodyHandle hBody);

#ifdef RV_SIP_SUBS_ON
/***************************************************************************
 * MsgUtilsEncodeSubstateVal
 * ------------------------------------------------------------------------
 * General: The function add the string of the substate-val to the Subscription-State
 *          encoding page,without '\0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPage  - Handle of the memory page for encoded buffer.
 *            eQop   - Value of the qop options to encode.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeSubstateVal(
                                   IN MsgMgr*                  pMsgMgr,
                                   IN HRPOOL                    hPool,
                                   IN HPAGE                     hPage,
                                   IN RvSipSubscriptionSubstate eSubstate,
                                   IN HRPOOL                    stringPool,
                                   IN HPAGE                     stringPage,
                                   IN RvInt32                  strSubstate,
                                   OUT RvUint32*               length);

/***************************************************************************
 * MsgUtilsEncodeSubsReason
 * ------------------------------------------------------------------------
 * General: The function add the string of the reason to the Subscription-State
 *          encoding page,without '\0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPage   - Handle of the memory page for encoded buffer.
 *            eReason - Value of the qop options to encode.
 *         length   - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeSubsReason(
                                   IN MsgMgr*                   pMsgMgr,
                                   IN HRPOOL                    hPool,
                                   IN HPAGE                     hPage,
                                   IN RvSipSubscriptionReason   eReason,
                                   IN HRPOOL                    stringPool,
                                   IN HPAGE                     stringPage,
                                   IN RvInt32                  strReason,
                                   OUT RvUint32*               length);

/***************************************************************************
 * SubscriptionStateHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Subscription-State header (as string) on it.
 *          format: "Subscription-State: "
 *                  substate-value *(";" subexp-params)
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle to the Subscription-State header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 * output: encoded - Pointer to the beginning of the encoded string on the page.
 *         length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV SubscriptionStateHeaderEncode(
                      IN    RvSipSubscriptionStateHeaderHandle  hHeader,
                      IN    HRPOOL                              hPool,
                      IN    HPAGE                               hPage,
                      IN    RvBool                bInUrlHeaders,
                      INOUT RvUint32*                          length);

 
#endif /* #ifdef RV_SIP_SUBS_ON */


#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
/***************************************************************************
 * ReasonHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          Reason header (as string) on it.
 * Return Value: RV_OK                     - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM         - If hHeader or hPage are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle of the allow header object.
 *        hPool             - Handle of the pool of pages
 *        hPage             - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders     - RV_TRUE if the header is in a url headers parameter.
 *                            if so, reserved characters should be encoded in escaped
 *                            form, and '=' should be set after header name instead of ':'.
 * output: length           - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV ReasonHeaderEncode(
                                      IN    RvSipReasonHeaderHandle     hHeader,
                                      IN    HRPOOL                      hPool,
                                      IN    HPAGE                       hPage,
                                      IN    RvBool                      bInUrlHeaders,
                                      INOUT RvUint32*                   length);

/***************************************************************************
 * WarningHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          Warning header (as string) on it.
 * Return Value: RV_OK                     - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM         - If hHeader or hPage are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle of the allow header object.
 *        hPool             - Handle of the pool of pages
 *        hPage             - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders     - RV_TRUE if the header is in a url headers parameter.
 *                            if so, reserved characters should be encoded in escaped
 *                            form, and '=' should be set after header name instead of ':'.
 * output: length           - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV WarningHeaderEncode(
                                      IN    RvSipWarningHeaderHandle    hHeader,
                                      IN    HRPOOL                      hPool,
                                      IN    HPAGE                       hPage,
                                      IN    RvBool                      bInUrlHeaders,
                                      INOUT RvUint32*                   length);


/***************************************************************************
 * TimestampHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          Timestamp header (as string) on it.
 * Return Value: RV_OK                     - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM         - If hHeader or hPage are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle of the Timestamp header object.
 *        hPool             - Handle of the pool of pages
 *        hPage             - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders     - RV_TRUE if the header is in a url headers parameter.
 *                            if so, reserved characters should be encoded in escaped
 *                            form, and '=' should be set after header name instead of ':'.
 * output: length           - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV TimestampHeaderEncode(
                                      IN    RvSipTimestampHeaderHandle  hHeader,
                                      IN    HRPOOL                      hPool,
                                      IN    HPAGE                       hPage,
                                      IN    RvBool                      bInUrlHeaders,
                                      INOUT RvUint32*                   length);

/***************************************************************************
 * InfoHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          Info header (as string) on it.
 * Return Value: RV_OK                     - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM         - If hHeader or hPage are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle of the allow header object.
 *        hPool             - Handle of the pool of pages
 *        hPage             - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders     - RV_TRUE if the header is in a url headers parameter.
 *                            if so, reserved characters should be encoded in escaped
 *                            form, and '=' should be set after header name instead of ':'.
 * output: length           - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV InfoHeaderEncode(
                                      IN    RvSipInfoHeaderHandle       hHeader,
                                      IN    HRPOOL                      hPool,
                                      IN    HPAGE                       hPage,
                                      IN    RvBool                      bInUrlHeaders,
                                      INOUT RvUint32*                   length);

/***************************************************************************
 * AcceptEncodingHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          Accept-Encoding header (as string) on it.
 * Return Value: RV_OK                     - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM         - If hHeader or hPage are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle of the allow header object.
 *        hPool             - Handle of the pool of pages
 *        hPage             - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders     - RV_TRUE if the header is in a url headers parameter.
 *                            if so, reserved characters should be encoded in escaped
 *                            form, and '=' should be set after header name instead of ':'.
 * output: length           - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV AcceptEncodingHeaderEncode(
                                      IN    RvSipAcceptEncodingHeaderHandle     hHeader,
                                      IN    HRPOOL							    hPool,
                                      IN    HPAGE								hPage,
                                      IN    RvBool								bInUrlHeaders,
                                      INOUT RvUint32*							length);

/***************************************************************************
 * AcceptHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          Accept header (as string) on it.
 * Return Value: RV_OK                     - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM         - If hHeader or hPage are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle of the allow header object.
 *        hPool             - Handle of the pool of pages
 *        hPage             - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders     - RV_TRUE if the header is in a url headers parameter.
 *                            if so, reserved characters should be encoded in escaped
 *                            form, and '=' should be set after header name instead of ':'.
 * output: length           - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV AcceptHeaderEncode(
                                      IN    RvSipAcceptHeaderHandle             hHeader,
                                      IN    HRPOOL							    hPool,
                                      IN    HPAGE								hPage,
                                      IN    RvBool								bInUrlHeaders,
                                      INOUT RvUint32*							length);

/***************************************************************************
 * AcceptLanguageHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          Accept-Language header (as string) on it.
 * Return Value: RV_OK                     - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM         - If hHeader or hPage are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle of the allow header object.
 *        hPool             - Handle of the pool of pages
 *        hPage             - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders     - RV_TRUE if the header is in a url headers parameter.
 *                            if so, reserved characters should be encoded in escaped
 *                            form, and '=' should be set after header name instead of ':'.
 * output: length           - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV AcceptLanguageHeaderEncode(
                                      IN    RvSipAcceptLanguageHeaderHandle     hHeader,
                                      IN    HRPOOL							    hPool,
                                      IN    HPAGE								hPage,
                                      IN    RvBool								bInUrlHeaders,
                                      INOUT RvUint32*							length);

/***************************************************************************
 * ReplyToHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *          ReplyTo header (as string) on it.
 * Return Value: RV_OK                     - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM         - If hHeader or hPage are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader           - Handle of the allow header object.
 *        hPool             - Handle of the pool of pages
 *        hPage             - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders     - RV_TRUE if the header is in a url headers parameter.
 *                            if so, reserved characters should be encoded in escaped
 *                            form, and '=' should be set after header name instead of ':'.
 * output: length           - The given length + the encoded header length.
 ***************************************************************************/
RvStatus RVCALLCONV ReplyToHeaderEncode(
                                      IN    RvSipReplyToHeaderHandle     hHeader,
                                      IN    HRPOOL                      hPool,
                                      IN    HPAGE                       hPage,
                                      IN    RvBool                      bInUrlHeaders,
                                      INOUT RvUint32*                   length);


/* XXX */
#endif /* #ifdef RV_SIP_EXTENDED_HEADER_SUPPORT */

#ifdef RV_SIP_IMS_HEADER_SUPPORT
/***************************************************************************
 * PUriHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            P-Uri header (as string) on it.
 *          format: "P-Uri: "
 *                  name-addr
 *                    *(";" general-params))))
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the P-Uri header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  PUriHeaderEncode(
                                  IN    RvSipPUriHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*                length);

/***************************************************************************
 * PVisitedNetworkIDHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            P-Visited-Network-ID header (as string) on it.
 *          format: "P-Visited-Network-ID: "
 *                  vnetworkspec
 *                    *(";" general-params))))
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the P-Visited-Network-ID header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  PVisitedNetworkIDHeaderEncode(
                                  IN    RvSipPVisitedNetworkIDHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*                length);

/***************************************************************************
 * PAccessNetworkInfoHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            P-Access-Network-Info header (as string) on it.
 *          format: "P-Access-Network-Info: "
 *                  
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the P-Access-Network-Info header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  PAccessNetworkInfoHeaderEncode(
                                  IN    RvSipPAccessNetworkInfoHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*                length);

/***************************************************************************
 * MsgUtilsEncodeAccessType
 * ------------------------------------------------------------------------
 * General: The function adds the string of the AccessType to the 
 *          encoding page,without '\0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hPage  - Handle of the memory page for encoded buffer.
 *			eAccessType - enumeration for the access type to encode.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeAccessType(
                                   IN MsgMgr*							pMsgMgr,
                                   IN HRPOOL							hPool,
                                   IN HPAGE								hPage,
                                   IN RvSipPAccessNetworkInfoAccessType eAccessType,
                                   IN HRPOOL							stringPool,
                                   IN HPAGE								stringPage,
                                   IN RvInt32							strAccessType,
                                   OUT RvUint32*						length);

/***************************************************************************
 * PChargingFunctionAddressesHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            P-Charging-Function-Addresses header (as string) on it.
 *          format: "P-Charging-Function-Addresses: charge-addr-params..."
 *                  
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the P-Charging-Function-Addresses header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  PChargingFunctionAddressesHeaderEncode(
                                  IN    RvSipPChargingFunctionAddressesHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*                length);

/***************************************************************************
 * MsgUtilsEncodePChargingFunctionAddressesList
 * ------------------------------------------------------------------------
 * General: The function adds the string of the list of ccf or ecf to the 
 *          encoding page,without '/0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hPage	- Handle of the memory page for encoded buffer.
 *			hList	- handle for the list.
 *			eListType - enumeration for the type of the list.
 *			hListPage - Handle of the memory page for the list.
 * Output: length	- The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodePChargingFunctionAddressesList(
                   IN MsgMgr*									pMsgMgr,
                   IN HRPOOL									hPool,
                   IN HPAGE										hPage,
                   IN RvSipCommonListHandle						hList,
				   IN HRPOOL									hListPool,
                   IN HPAGE										hListPage,
				   IN RvBool									bInUrlHeaders,
				   IN RvSipPChargingFunctionAddressesListType	eListType,
                   OUT RvUint32*								length);

/***************************************************************************
 * PChargingVectorHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            P-Charging-Vector header (as string) on it.
 *          format: "P-Charging-Vector: "
 *                  
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the P-Charging-Vector header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  PChargingVectorHeaderEncode(
                                  IN    RvSipPChargingVectorHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*                length);
 
/***************************************************************************
 * MsgUtilsEncodePChargingVectorInfoList
 * ------------------------------------------------------------------------
 * General: The function adds the string of the list of pdp-info or dsl-bearer-info to the 
 *          encoding page,without '/0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hPage	- Handle of the memory page for encoded buffer.
 *			hList	- Handle for the list.
 *			eListType - Enumeration for the type of the list.
 *			hListPage - Handle of the memory page for the list.
 * Output: length	- The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodePChargingVectorInfoList(
                                   IN MsgMgr*					pMsgMgr,
                                   IN HRPOOL                    hPool,
                                   IN HPAGE                     hPage,
                                   IN RvSipCommonListHandle		hList,
								   IN HRPOOL                    hListPool,
                                   IN HPAGE                     hListPage,
								   IN RvBool					bInUrlHeaders,
								   IN RvSipPChargingVectorInfoListType eListType,
                                   OUT RvUint32*				length);
 
/***************************************************************************
 * PMediaAuthorizationHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            P-Media-Authorization header (as string) on it.
 *          format: "P-Media-Authorization: P-Media-Authorization-Token *["," P-Media-Authorization-Token]"
 *                  
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the P-Media-Authorization header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  PMediaAuthorizationHeaderEncode(
                                  IN    RvSipPMediaAuthorizationHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*                length);

/***************************************************************************
 * SecurityHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            Security header (as string) on it.
 *          example format: "Security-Server: mechanism-name; mech-parameters, ..."
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the Security header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV SecurityHeaderEncode(
                                  IN    RvSipSecurityHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*               length);

/***************************************************************************
 * MsgUtilsEncodeSecurityMechanismType
 * ------------------------------------------------------------------------
 * General: The function adds the string of the Mechanism Type to the 
 *          encoding page,without '\0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hPage  - Handle of the memory page for encoded buffer.
 *			eMechanismType - Enumeration of the mechanism type.
 *			strMechanismType - String (on an RPool) for alternative mechanism type.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeSecurityMechanismType(
                                   IN MsgMgr*						pMsgMgr,
                                   IN HRPOOL						hPool,
                                   IN HPAGE							hPage,
                                   IN RvSipSecurityMechanismType	eMechanismType,
                                   IN HRPOOL						stringPool,
                                   IN HPAGE							stringPage,
                                   IN RvInt32						strMechanismType,
                                   OUT RvUint32*					length);

/***************************************************************************
 * MsgUtilsEncodeSecurityAlgorithmType
 * ------------------------------------------------------------------------
 * General: The function adds the string of the Algorithm Type to the 
 *          encoding page,without '\0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hPage  - Handle of the memory page for encoded buffer.
 *			eAlgorithmType - Enumeration of the algorithm type.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeSecurityAlgorithmType(
                                   IN MsgMgr*						pMsgMgr,
                                   IN HRPOOL						hPool,
                                   IN HPAGE							hPage,
                                   IN RvSipSecurityAlgorithmType	eAlgorithmType,
                                   OUT RvUint32*					length);

/***************************************************************************
 * MsgUtilsEncodeSecurityProtocolType
 * ------------------------------------------------------------------------
 * General: The function adds the string of the Protocol Type to the 
 *          encoding page,without '\0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hPage  - Handle of the memory page for encoded buffer.
 *			eProtocolType - Enumeration for the protocol type.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeSecurityProtocolType(
                                   IN MsgMgr*						pMsgMgr,
                                   IN HRPOOL						hPool,
                                   IN HPAGE							hPage,
                                   IN RvSipSecurityProtocolType		eProtocolType,
                                   OUT RvUint32*					length);

/***************************************************************************
 * MsgUtilsEncodeSecurityModeType
 * ------------------------------------------------------------------------
 * General: The function adds the string of the Mode Type to the 
 *          encoding page,without '\0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hPage  - Handle of the memory page for encoded buffer.
 *			eModeType - Enumeration for the mode type.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeSecurityModeType(
                                   IN MsgMgr*						pMsgMgr,
                                   IN HRPOOL						hPool,
                                   IN HPAGE							hPage,
                                   IN RvSipSecurityModeType			eModeType,
                                   OUT RvUint32*					length);

/***************************************************************************
 * MsgUtilsEncodeSecurityEncryptAlgorithmType
 * ------------------------------------------------------------------------
 * General: The function adds the string of the EncryptAlgorithm Type to the 
 *          encoding page,without '\0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	hPage  - Handle of the memory page for encoded buffer.
 *			eEncryptAlgorithmType - Enumeration for the Encrypt Algorithm type.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeSecurityEncryptAlgorithmType(
                                   IN MsgMgr*							pMsgMgr,
                                   IN HRPOOL							hPool,
                                   IN HPAGE								hPage,
                                   IN RvSipSecurityEncryptAlgorithmType	eEncryptAlgorithmType,
                                   OUT RvUint32*						length);

/***************************************************************************
 * PProfileKeyHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            P-Profile-Key header (as string) on it.
 *          format: "P-Profile-Key: "
 *                  name-addr / addr-spec
 *                    *(";" general-params))))
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the P-Profile-Key header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  PProfileKeyHeaderEncode(
                                  IN    RvSipPProfileKeyHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*                length);
 
 /***************************************************************************
 * PUserDatabaseHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            P-User-Database header (as string) on it.
 *          format: "P-User-Database: " <addr-spec>
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the P-User-Database header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  PUserDatabaseHeaderEncode(
                                  IN    RvSipPUserDatabaseHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*                length);
 
 /***************************************************************************
 * MsgUtilsEncodeDiameterProtocolType
 * ------------------------------------------------------------------------
 * General: The function adds the string of the Protocol to the page, without '/0'.
 * Return Value: RV_OK, RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hPool        - Handle of the memory pool for encoded buffer.
 *        hPage        - Handle of the memory page for encoded buffer.
 *        eProtocol    - Value of the Protocol to encode.
 * Output:length       - The length that was given + the new encoded string length.
 **************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeDiameterProtocolType(
                                      IN MsgMgr*                pMsgMgr,
                                      IN HRPOOL                 hPool,
                                      IN HPAGE                  hPage,
                                      IN RvSipDiameterProtocol  eProtocol,
                                     OUT RvUint32*              length);

/***************************************************************************
 * MsgUtilsEncodeUriCPCType
 * ------------------------------------------------------------------------
 * General: The function add the string of the CPC Type to the 
 *          encoding page,without '/0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPage  - Handle of the memory page for encoded buffer.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeUriCPCType(
                                   IN MsgMgr*				pMsgMgr,
                                   IN HRPOOL				hPool,
                                   IN HPAGE					hPage,
                                   IN RvSipUriCPCType	    eCPCType,
                                   IN HRPOOL				stringPool,
                                   IN HPAGE					stringPage,
                                   IN RvInt32				strCPCType,
                                   OUT RvUint32*			length);

/***************************************************************************
 * MsgUtilsEncodeAnswerType
 * ------------------------------------------------------------------------
 * General: The function adds the string of the AnswerType to the 
 *          encoding page,without '\0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hPage  - Handle of the memory page for encoded buffer.
 *			eAnswerType - enumeration for the answer type to encode.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeAnswerType(
                                   IN MsgMgr*							pMsgMgr,
                                   IN HRPOOL							hPool,
                                   IN HPAGE								hPage,
                                   IN RvSipPAnswerStateAnswerType       eAnswerType,
                                   IN HRPOOL							stringPool,
                                   IN HPAGE								stringPage,
                                   IN RvInt32							strAnswerType,
                                   OUT RvUint32*						length);

/***************************************************************************
 * PAnswerStateHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            P-Answer-State header (as string) on it.
 *          format: "P-Answer-State: " Answer-Type
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the P-Answer-State header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  PAnswerStateHeaderEncode(
                                  IN    RvSipPAnswerStateHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*                length);
 
 /***************************************************************************
 * MsgUtilsEncodeSessionCaseType
 * ------------------------------------------------------------------------
 * General: The function adds the string of the SessionCaseType to the 
 *          encoding page,without '\0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hPage  - Handle of the memory page for encoded buffer.
 *			eSessionCaseType - enumeration for the answer type to encode.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeSessionCaseType(
                                   IN MsgMgr*							pMsgMgr,
                                   IN HRPOOL							hPool,
                                   IN HPAGE								hPage,
                                   IN RvSipPServedUserSessionCaseType   eSessionCaseType,
                                   OUT RvUint32*						length);

/***************************************************************************
 * MsgUtilsEncodeRegistrationStateType
 * ------------------------------------------------------------------------
 * General: The function adds the string of the RegistrationStateType to the 
 *          encoding page,without '\0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hPage  - Handle of the memory page for encoded buffer.
 *			eRegistrationStateType - enumeration for the answer type to encode.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeRegistrationStateType(
                                   IN MsgMgr*							    pMsgMgr,
                                   IN HRPOOL							    hPool,
                                   IN HPAGE								    hPage,
                                   IN RvSipPServedUserRegistrationStateType eRegistrationStateType,
                                   OUT RvUint32*						    length);
                                                                            

 /***************************************************************************
 * PServedUserHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            P-Served-User header (as string) on it.
 *          format: "P-Served-User: User1 <user@host>" 
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the P-Served-User header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  PServedUserHeaderEncode(
                                  IN    RvSipPServedUserHeaderHandle    hHeader,
                                  IN    HRPOOL                          hPool,
                                  IN    HPAGE                           hPage,
                                  IN    RvBool                          bInUrlHeaders,
                                  INOUT RvUint32*                       length);
#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

#ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT
/***************************************************************************
 * PDCSTracePartyIDHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            P-DCS-Trace-Party-ID header (as string) on it.
 *          format: "P-DCS-Trace-Party-ID: name-addr"
 *                  
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the P-DCS-Trace-Party-ID header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  PDCSTracePartyIDHeaderEncode(
                                  IN    RvSipPDCSTracePartyIDHeaderHandle hHeader,
                                  IN    HRPOOL                   hPool,
                                  IN    HPAGE                    hPage,
                                  IN    RvBool                   bInUrlHeaders,
                                  INOUT RvUint32*                length);

/***************************************************************************
 * PDCSOSPSHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            P-DCS-OSPS header (as string) on it.
 *          format: "P-DCS-OSPS: OSPS-Tag"
 *                  
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the P-DCS-OSPS header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  PDCSOSPSHeaderEncode(
                                  IN    RvSipPDCSOSPSHeaderHandle hHeader,
                                  IN    HRPOOL					  hPool,
                                  IN    HPAGE					  hPage,
                                  IN    RvBool					  bInUrlHeaders,
                                  INOUT RvUint32*				  length);

/***************************************************************************
 * MsgUtilsEncodeOSPSTag
 * ------------------------------------------------------------------------
 * General: The function adds the string of the OSPSTag to the 
 *          encoding page,without '\0'.
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure
 *               RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hPage  - Handle of the memory page for encoded buffer.
 * Output: encoded - Pointer to the beginning of the encoded string in the page.
 *         length - The length that was given + the new encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgUtilsEncodeOSPSTag (
                                   IN MsgMgr*			pMsgMgr,
                                   IN HRPOOL			hPool,
                                   IN HPAGE				hPage,
                                   IN RvSipOSPSTagType	eOSPSTag,
								   IN HRPOOL            stringPool,
                                   IN HPAGE             stringPage,
                                   IN RvInt32           strOSPSTag,
                                  OUT RvUint32*			length);

/***************************************************************************
 * PDCSBillingInfoHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            P-DCS-Billing-Info header (as string) on it.
 *          format: "P-DCS-Billing-Info: Billing-Correlation-ID "/" FEID *(SEMI Billing-Info-param)"
 *                  
 * Return Value: RV_OK          - If succeeded.
 *               RV_ERROR_OUTOFRESOURCES   - If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN          - In case of a failure.
 *               RV_ERROR_BADPARAM - If hHeader or hPage are NULL or no method
 *                                     or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the P-DCS-Billing-Info header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  PDCSBillingInfoHeaderEncode(
                                  IN    RvSipPDCSBillingInfoHeaderHandle hHeader,
                                  IN    HRPOOL							 hPool,
                                  IN    HPAGE							 hPage,
                                  IN    RvBool							 bInUrlHeaders,
                                  INOUT RvUint32*						 length);

/***************************************************************************
 * PDCSLAESHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            P-DCS-LAES header (as string) on it.
 *          format: "P-DCS-LAES: Laes-sig *(SEMI Laes-param)"
 *                  
 * Return Value: RV_OK						- If succeeded.
 *               RV_ERROR_OUTOFRESOURCES	- If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN			- In case of a failure.
 *               RV_ERROR_BADPARAM			- If hHeader or hPage are NULL or no method
 *												or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the P-DCS-LAES header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  PDCSLAESHeaderEncode(
                                  IN    RvSipPDCSLAESHeaderHandle	hHeader,
                                  IN    HRPOOL						hPool,
                                  IN    HPAGE						hPage,
                                  IN    RvBool						bInUrlHeaders,
                                  INOUT RvUint32*					length);

/***************************************************************************
 * PDCSRedirectHeaderEncode
 * ------------------------------------------------------------------------
 * General: Accepts pool and page for allocating memory, and encode the
 *            P-DCS-Redirect header (as string) on it.
 *          format: "P-DCS-Redirect: Called-ID *(SEMI redir-params)"
 *                  
 * Return Value: RV_OK						- If succeeded.
 *               RV_ERROR_OUTOFRESOURCES	- If allocation failed (no resources)
 *               RV_ERROR_UNKNOWN			- In case of a failure.
 *               RV_ERROR_BADPARAM			- If hHeader or hPage are NULL or no method
 *												or no spec were given.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hHeader  - Handle of the P-DCS-Redirect header object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded header length.
 ***************************************************************************/
 RvStatus RVCALLCONV  PDCSRedirectHeaderEncode(
                                  IN    RvSipPDCSRedirectHeaderHandle	hHeader,
                                  IN    HRPOOL							hPool,
                                  IN    HPAGE							hPage,
                                  IN    RvBool							bInUrlHeaders,
                                  INOUT RvUint32*						length);
#endif /* #ifdef RV_SIP_IMS_DCS_HEADER_SUPPORT */  

/************************************************************************************
 * MsgUtilsFindCharInPage
 * ----------------------------------------------------------------------------------
 * General: try to finds a specific char in the header.
 * Return Value: RvBool - RV_TRUE if the char was found
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hPool   - handle to the pool to search the char
 *          hPage   - handle to the page to search the char
 *          offset    - offset in the page so we can get the pointer to start check.
 *          length  - the length of the buffer in the page to search in
 *          charToSearch - the char to find
 * output:    newOffset  - the pointer to the char after the "specific char"
 ***********************************************************************************/
RvBool RVCALLCONV MsgUtilsFindCharInPage(IN HRPOOL   hPool,
                                         IN HPAGE    hPage,
                                         IN RvInt32 offset,
                                         IN RvInt32 length,
                                         IN RvChar  charToSearch,
                                         OUT RvInt32 *newOffset);


#endif /* RV_SIP_PRIMITIVES */

/***************************************************************************
 * MsgHeaderEncode
 * ------------------------------------------------------------------------
 * General: The function gets a header and it's type and encode it.
 *
 * Return Value:
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pHeader -   Handle to the header structure.
 *            eType -     Type of the header to encode.
 *          hPool -     Pool of the given page.
 *            hPage -     A page to fill with encoded string.
 *          bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 *        bForceCompactForm - Encode with compact form even if the header is not
 *                            marked with compact form.
 * Output:     length -    previous length + the header encoded string length.
 ***************************************************************************/
RvStatus RVCALLCONV MsgHeaderEncode(
                                    IN    void*                   pHeader,
                                    IN    RvSipHeaderType         eType,
                                    IN    HRPOOL                  hPool,
                                    IN    HPAGE                   hPage,
                                    IN    RvBool                 bInUrlHeaders,
                                    IN    RvBool                 bForceCompactForm,
                                    INOUT RvUint32*              length);

/***************************************************************************
 * MsgConstructRequestLine
 * ------------------------------------------------------------------------
 * General: Constructs the request line of the message.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * input:   msg - Pointer to the message structure.
 ***************************************************************************/
void RVCALLCONV MsgConstructRequestLine(IN  MsgMessage*   msg);

/***************************************************************************
 * MsgConstructStatusLine
 * ------------------------------------------------------------------------
 * General: Constructs the status line of the message.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * input:   msg - Pointer to the message structure.
 ***************************************************************************/
void RVCALLCONV MsgConstructStatusLine(IN  MsgMessage*   msg);


/***************************************************************************
 * MsgHighAvalStore
 * ------------------------------------------------------------------------
 * General: Copies all message parameters from a given msg handle to a given buffer.
 *          This buffer should be supplied when restoring the message.
 * NOTE: The input memBuf should be aligned.
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the message is invalid.
 *              RV_ERROR_NULLPTR        - Bad pointer to the memPool or params structure.
 *              RV_ERROR_INSUFFICIENT_BUFFER - The buffer is too small.
 *              RV_ERROR_OUTOFRESOURCES - Resource problems.
 *              RV_OK                   - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg       - Handle to the stored message.
 *          hMsgPool   - Handle to the stored message pool.
 *          memBuff    - The buffer that will store the message parameters.
 *          maxBuffLen - The maximum length of the data to be stored.
 * Output:  pBuffLen   - The length of the stored message data.
 ***************************************************************************/
RvStatus RVCALLCONV MsgHighAvalStore(
                          IN    RvSipMsgHandle  hMsg,
                          IN    HRPOOL          hMsgPool,
                          IN    void           *memBuff,
                          IN    RvUint32        maxBuffLen,
                          OUT   RvUint32       *pBuffLen);
/***************************************************************************
 * MsgParserParse
 * ------------------------------------------------------------------------
 * General: Separate the message to headers (by the CRLF) and parse each one.
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
                                   IN RvInt32       bufferLen);

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
 *  eHeaderType   - The type of the header being parsed.
 *  pHeader       - Pointer to the header object.
 *  pStrHeader    - Null terminated string of the header.
 *  bOnlyValue    - true if the string does not contain the header name
 *                  false if contains the header name.
 ***************************************************************************/
RvStatus RVCALLCONV MsgParserParseStandAloneHeader(  IN MsgMgr*       pMsgMgr,
                                                     IN SipParseType  eHeaderType,
                                                     IN RvChar*       pStrHeader,
                                                     IN RvBool        bOnlyValue,
                                                     IN void*         pHeader);

/***************************************************************************
 * MsgParserParseStandAloneAddress
 * ------------------------------------------------------------------------
 * General: The function gets a null terminated address string and the type
 *          of the address.
 *          The function pass it to the parser.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  eHeaderType   - The type of the header being parsed.
 *  pHeader       - Pointer to the header object.
 *  pStrHeader    - Null terminated string of the header.
 *  bOnlyValue    - true if the string does not contain the header name
 *                  false if contains the header name.
 ***************************************************************************/
RvStatus RVCALLCONV MsgParserParseStandAloneAddress( IN MsgMgr*       pMsgMgr,
                                                     IN SipParseType  eAddrType,
                                                     IN RvChar*       pStrHeader,
                                                     IN void*         pAddress);

#ifdef __cplusplus
}
#endif

#endif /*SIPINTERNALFUNCSS_H*/

