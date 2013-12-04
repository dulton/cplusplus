/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             AddrTelUri.h                                     *
 *                                                                            *
 * The file defines internal functions of the Tel-Uri object.                 *
 * The methods are construct,destruct and copy an object.It also implements   *
 * the encode and parse methods.                                              *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Tamar Barzuza     Feb.2005                                            *
 ******************************************************************************/



#ifndef RVSIPTELURI_H
#define RVSIPTELURI_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_TEL_URI_SUPPORT

#include "MsgTypes.h"

/****************************************************/
/*        CONSTRUCTORS AND DESTRUCTORS                */
/****************************************************/

/***************************************************************************
 * AddrTelUriConstruct
 * ------------------------------------------------------------------------
 * General: The function construct an Tel-Uri address object, which is 'stand alone'
 *          (means - relate to no message).
 *          The function 'allocate' the object (from the given page), initialized
 *          it's parameters, and return the handle of the object.
 *          Note - The function keeps the page in the MsgAddress structure, therefore
 *          in every allocation that will come, it will be done from the same page..
 * Return Value: RV_OK, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hMsgMgr   - Handle to the message manager.
 *         hPool   - Handle of the memory pool.
 *         hPage   - Handle of the memory page that the url will be allocated from..
 * output: pTelUri - Handle of the Tel-Uri address object..
 ***************************************************************************/
RvStatus AddrTelUriConstruct(IN  RvSipMsgMgrHandle        hMsgMgr,
                              IN  HRPOOL          hPool,
                              IN  HPAGE           hPage,
                              OUT MsgAddrTelUri**  pTelUri);


/***************************************************************************
 * AddrTelUriCopy
 * ------------------------------------------------------------------------
 * General:Copy one Tel-Uri address object to another.
 * Return Value:RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle of the Tel-Uri address object.
 *    hSource      - Handle of the source object.
 ***************************************************************************/
RvStatus AddrTelUriCopy(IN RvSipAddressHandle hDestination,
                         IN RvSipAddressHandle hSource);


/***************************************************************************
 * AddrTelUriEncode
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
 *        bInUrlHeaders - RV_TRUE if the header is in a url headers parameter.
 *                   if so, reserved characters should be encoded in escaped
 *                   form, and '=' should be set after header name instead of ':'.
 * output: length  - The given length + the encoded address length.
 ***************************************************************************/
RvStatus AddrTelUriEncode(IN  RvSipAddressHandle hAddr,
                          IN  HRPOOL             hPool,
                          IN  HPAGE              hPage,
                          IN  RvBool             bInUrlHeaders,
                          OUT RvUint32*          length);

/***************************************************************************
 * AddrTelUriParse
 * ------------------------------------------------------------------------
 * General:This function is used to parse an Tel-Uri address text and construct
 *           a MsgAddrTelUri object from it.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES,RV_ERROR_NULLPTR,
 *                 RV_ERROR_ILLEGAL_SYNTAX,RV_ERROR_ILLEGAL_SYNTAX.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr  - A Handle of the Tel-Uri address object.
 *    buffer    - holds a pointer to the beginning of the null-terminated(!) SIP text header
 ***************************************************************************/
RvStatus AddrTelUriParse( IN RvSipAddressHandle hSipAddr,
                           IN RvChar*           buffer);


/***************************************************************************
 * AddrTelUriIsEqual
 * ------------------------------------------------------------------------
 * General:This function comapres between two tel-uri objects:
 *           TEL URIs are compared according to the following rules:
 *           1. String-wise comparison of the parameters that the stack
 *              identifies and parses (phone number, Extension, isdn, post dial
 *              and context.
 *           2. No comparison of other parameters.
 * Return Value: RV_TRUE if equal, else RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader      - Handle of the url object.
 *    pOtherHeader - Handle of the url object.
 ***************************************************************************/
RvBool AddrTelUriIsEqual(IN RvSipAddressHandle pHeader,
                         IN RvSipAddressHandle pOtherHeader);


#endif /* RV_SIP_TEL_URI_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /* RVSIPTELURI_H */

