 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             AddrUrl.h                                      *
 *                                                                            *
 * The file defines internal functions of the url object.                     *
 * The methods are construct,destruct, copy an object, and methods for        *
 * providing access (get and set) to it's components. It also implement the   *
 * encode and parse methods.                                                  *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             Nov.2000                                             *
 ******************************************************************************/



#ifndef RVSIPURL_H
#define RVSIPURL_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "MsgTypes.h"
#include "RvSipCommonList.h"

/****************************************************/
/*        CONSTRUCTORS AND DESTRUCTORS                */
/****************************************************/


/***************************************************************************
 * AddrUrlConstruct
 * ------------------------------------------------------------------------
 * General: The function construct a url address object, which is 'stand alone'
 *          (means - relate to no message).
 *          The function 'allocate' the object (from the given page), initialized
 *          it's parameters, and return the handle of the object.
 *          Note - The function keeps the page in the url structure, therefore
 *          in every allocation that will come, it will be done from the same page..
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_OUTOFRESOURCES.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr - Handle to the message manager.
 *         hPool   - Handle to the memory pool.
 *         hPage   - Handle of the memory page that the url will be allocated from..
 * output: hSipUrl - Handle of the url address object..
 ***************************************************************************/
RvStatus AddrUrlConstruct(IN  RvSipMsgMgrHandle hMsgMgr,
                           IN  HRPOOL            hPool,
                           IN  HPAGE             hPage,
                           OUT MsgAddrUrlHandle* hSipUrl);

/***************************************************************************
 * AddrUrlCopy
 * ------------------------------------------------------------------------
 * General:Copy one url address object to another.
 * Return Value:RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle of the url address object.
 *    hSource      - Handle of the source object.
 ***************************************************************************/
RvStatus AddrUrlCopy(IN    RvSipAddressHandle hDestination,
                      IN    RvSipAddressHandle hSource);

/***************************************************************************
 * AddrUrlEncode
 * ------------------------------------------------------------------------
 * General: Accepts a pointer to an allocated memory and copies the value of
 *            Url as encoded buffer (string) onto it.
 *            The user should check the return code.  If the size of
 *            the buffer is not sufficient the method will return RV_ERROR_INSUFFICIENT_BUFFER
 *            and the user should send bigger buffer for the encoded message.
 * Return Value:RV_OK, RV_ERROR_INSUFFICIENT_BUFFER, RV_ERROR_UNKNOWN, RV_ERROR_BADPARAM.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hAddr    - Handle of the address object.
 *        hPool    - Handle of the pool of pages
 *        hPage    - Handle of the page that will contain the encoded message.
 *          bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded address length.
 ***************************************************************************/
RvStatus AddrUrlEncode(IN  RvSipAddressHandle hAddr,
                        IN  HRPOOL             hPool,
                        IN  HPAGE              hPage,
                        IN  RvBool            bInUrlHeaders,
                        OUT RvUint32*         length);

/***************************************************************************
 * AddrUrlParse
 * ------------------------------------------------------------------------
 * General:This function is used to parse a SIP url address text and construct
 *           a MsgAddrUrl object from it.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR,
 *                 RV_ERROR_ILLEGAL_SYNTAX, RV_ERROR_ILLEGAL_SYNTAX.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr  - A Handle of the url address object.
 *    buffer    - holds a pointer to the beginning of the null-terminated(!) SIP text header
 ***************************************************************************/
RvStatus AddrUrlParse( IN    RvSipAddressHandle hSipAddr,
                        IN    RvChar*           buffer);


/***************************************************************************
 * AddrUrlIsEqual
 * ------------------------------------------------------------------------
 * General:This function comapres between two url objects:
 *         SIP URL format is:
 *         sip:user:password@host:port;uri-parameters?headers.
 *           SIP URLs are compared for equality according to the following rules:
 *           1. Comparison of the user-info (user:password) is case-sensitive.
 *              compartion of all other components is case-insensitive.
 *           2. The ordering of parameters and headers is not significant.
 *         3. (not implemented) characters are equivalent to their encoding.
 *            (e.g. '%61lice' = 'alice')
 *         4. An IP address that is the result of a DNS lookup of a host name
 *            does not match that host name.
 *         5. The user, password, host and port components must match.
 *            if any of those parameters exists in one url, it must be present
 *            in the other url too.
 *         6. A Url omitting any component with a default value (port, transport,
 *            method, ttl, user) will not match a url explicitly containing
 *            that component with its default value.
 *         7. Any url-parameter appearing in both urls, must match.
 *         8. A url that includes a maddr parameter, will not match a url that
 *            includes no maddr parameter.
 *         9. Header components are never ignored. Any present header comonent
 *            MUST be present in both URIs, and match.
 *
 * Return Value: RV_TRUE if equal, else RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader      - Handle of the url object.
 *    pOtherHeader - Handle of the url object.
 ***************************************************************************/
RvBool AddrUrlIsEqual(IN RvSipAddressHandle pHeader,
                       IN RvSipAddressHandle pOtherHeader);

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
/***************************************************************************
 * AddrUrlCompare
 * ------------------------------------------------------------------------
 * General:This function comapres between two url objects:
 *		   SIP URLs are compared for equality according to the following rules:
 *		   1.Comparison of the scheme name ("sip"), domain names, parameter names
 *		   and header name are case-insensitive, all other comparisons are
 *		   case-sensitive.
 *		   2.The ordering of parameters and headers is not significant.
 * Return Value: Returns 0 if the URLs are equal. Otherwise, the function returns
 *               non-zero value: <0 if first URL is less, or >0 if first URL is more
 *               as the comparison result of a first non-equal URL string element.
 * ------------------------------------------------------------------------
 * Arguments:
 *	pHeader      - Handle of the url object.
 *	pOtherHeader - Handle of the url object.
 ***************************************************************************/
RvInt8 AddrUrlCompare(IN RvSipAddressHandle pHeader,
		               IN RvSipAddressHandle pOtherHeader);
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

/***************************************************************************
 * AddrUrl3261IsEqual
 * ------------------------------------------------------------------------
 * General:This function compares between two URL objects.
 * IMPORTANT: This function is equivalent to the previous function	
 *			  AddrUrlIsEqual(). Thus, if this function gets updated 
 *			  please update the compatible function.
 *         SIP URL format is:
 *         sip:user:password@host:port;uri-parameters?headers.
 *
 *           SIP URLs are compared for equality according to the following rules:
 *			1. A SIP and SIPS URI are never equivalent.
 *          2. Comparison of the user-info (user:password) is case-sensitive.
 *             compartion of all other components is case-insensitive.
 *          3. The ordering of parameters and headers is not significant.
 *			4. Characters are equivalent to their encoding - it's implemented 
 *			   (e.g. '%61lice' = 'alice')
 *			5. An IP address that is the result of a DNS lookup of a host name
 *             does not match that host name.
 *          6. The user, password, host and port components must match.
 *             if any of those parameters exists in one url, it must be present
 *             in the other url too.
 *          7. A Url omitting any component with a default value (port, transport,
 *             method, ttl, user) will not match a url explicitly containing
 *             that component with its default value.
 *          8. Any url-parameter appearing in both urls, must match.
 *          9. A url that includes a maddr parameter, will not match a url that
 *             includes no maddr parameter.
 *			10. Header components are never ignored. Any present header component
 *              MUST be present in both URIs, and match.
 *
 * Return Value: RV_TRUE if equal, else RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader         - Handle of the url object.
 *    pOtherHeader    - Handle of the url object.
 ***************************************************************************/
RvBool AddrUrl3261IsEqual(IN RvSipAddressHandle pHeader,
                          IN RvSipAddressHandle pOtherHeader); 

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
 * AddrUrlParseHeaders
 * ------------------------------------------------------------------------
 * General: The function takes the url headers parameter as string, and parse
 *          it into a list of headers.
 *          Application must supply a list structure, and a consecutive buffer
 *          with url headers parameter string.
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 *  hSipAddr        - Handle to the address object.
 *    hHeadersList    - A handle to the Address object.
 *    pHeadersBuffer  - Buffer containing a textual url headers parameter string.
 ***************************************************************************/
RvStatus RVCALLCONV AddrUrlParseHeaders(
                                     IN    MsgAddress*           pAddress,
                                     IN    RvSipCommonListHandle hHeadersList,
                                     IN    RvChar*              pHeadersBuffer);

/***************************************************************************
 * AddrUrlEncodeHeaders
 * ------------------------------------------------------------------------
 * General: This function is used to set the Headers param from headers list.
 *          The function encode all headers. during encoding each header
 *          coverts reserved characters to it's excaped form. each header
 *          also set '=' instead of ':' after header name.
 *          This function also sets '&' between headers.
 *          At last the function sets pAddress->strHeader to point to the new
 *          encoded string of headers.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 *    pAddress      - The url address object.
 *    hHeadersList  - The headers list to be set in the object.
 ***************************************************************************/
RvStatus AddrUrlEncodeHeadersList(IN    MsgAddress*           pAddress,
                                   IN    RvSipCommonListHandle hHeadersList);

#endif /*#ifndef RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif

#endif /* RVSIPURL_H */
