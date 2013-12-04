 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             AddrDiameterUri.h                              *
 *                                                                            *
 * The file defines internal functions of the url object.                     *
 * The methods are construct,destruct, copy an object, and methods for        *
 * providing access (get and set) to it's components. It also implement the   *
 * encode and parse methods.                                                  *
 *                                                                            *
 *      Author           Date                                                 *
 *     --------         ------------                                          *
 *      Mickey           Mar.2007                                             *
 ******************************************************************************/



#ifndef RVSIPDIAMETERURI_H
#define RVSIPDIAMETERURI_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "MsgTypes.h"

#ifdef RV_SIP_IMS_HEADER_SUPPORT

/****************************************************/
/*        CONSTRUCTORS AND DESTRUCTORS              */
/****************************************************/


/***************************************************************************
 * AddrDiameterUriConstruct
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
RvStatus AddrDiameterUriConstruct(IN  RvSipMsgMgrHandle     hMsgMgr,
                                  IN  HRPOOL                hPool,
                                  IN  HPAGE                 hPage,
                                  OUT MsgAddrDiameterUri**  hDiameterUri);

/***************************************************************************
 * AddrDiameterUriCopy
 * ------------------------------------------------------------------------
 * General:Copy one url address object to another.
 * Return Value:RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle of the url address object.
 *    hSource      - Handle of the source object.
 ***************************************************************************/
RvStatus AddrDiameterUriCopy(IN    RvSipAddressHandle hDestination,
                             IN    RvSipAddressHandle hSource);

/***************************************************************************
 * AddrDiameterUriEncode
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
RvStatus AddrDiameterUriEncode(IN  RvSipAddressHandle hAddr,
                        IN  HRPOOL             hPool,
                        IN  HPAGE              hPage,
                        IN  RvBool            bInUrlHeaders,
                        OUT RvUint32*         length);

/***************************************************************************
 * AddrDiameterUriParse
 * ------------------------------------------------------------------------
 * General:This function is used to parse a SIP url address text and construct
 *           a MsgAddrDiameterUri object from it.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR,
 *                 RV_ERROR_ILLEGAL_SYNTAX, RV_ERROR_ILLEGAL_SYNTAX.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr  - A Handle of the url address object.
 *    buffer    - holds a pointer to the beginning of the null-terminated(!) SIP text header
 ***************************************************************************/
RvStatus AddrDiameterUriParse( IN    RvSipAddressHandle hSipAddr,
                        IN    RvChar*           buffer);


/***************************************************************************
 * AddrDiameterUriIsEqual
 * ------------------------------------------------------------------------
 * General:This function compares between two URL objects.
 *
 *         DIAMETER URL format is:
 *         aaa://host:port.
 *
 *           DIAMETER URLs are compared for equality according to the following rules:
 *			1. A AAA and AAAS URI are never equivalent.
 *          2. Comparison of all components is case-insensitive.
 *          3. The ordering of parameters is not significant.
 *			4. (not implemented) characters are equivalent to their encoding.
 *			   (e.g. '%61lice' = 'alice')
 *			5. An IP address that is the result of a DNS lookup of a host name
 *             does not match that host name.
 *          6. The host and port components must match.
 *             if any of those parameters exists in one url, it must be present
 *             in the other url too.
 *          7. A Url omitting any component with a default value (port, transport,
 *             protocol) will not match a url explicitly containing
 *             that component with its default value.
 *          8. Any other-parameter appearing in both urls, must match.
 *
 * Return Value: RV_TRUE if equal, else RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader      - Handle of the url object.
 *    pOtherHeader - Handle of the url object.
 ***************************************************************************/
RvBool AddrDiameterUriIsEqual(IN RvSipAddressHandle pHeader,
                              IN RvSipAddressHandle pOtherHeader);


#endif /* #ifdef RV_SIP_IMS_HEADER_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /* RVSIPDIAMETERURI_H */
