/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             AddrAbsUri                                     *
 *                                                                            *
 * The file defines internal functions of the Abs-Uri object.                 *
 * The methods are construct,destruct and copy an object.It also implements   *
 * the encode and parse methods.                                              *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Oren Libis       Oct.2001                                             *
 ******************************************************************************/



#ifndef RVSIPABSURI_H
#define RVSIPABSURI_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "MsgTypes.h"

/****************************************************/
/*        CONSTRUCTORS AND DESTRUCTORS                */
/****************************************************/

/***************************************************************************
 * AddrAbsUriConstruct
 * ------------------------------------------------------------------------
 * General: The function construct an Abs-Uri address object, which is 'stand alone'
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
 * output: pAbsUri - Handle of the Abs-Uri address object..
 ***************************************************************************/
RvStatus AddrAbsUriConstruct(IN  RvSipMsgMgrHandle        hMsgMgr,
                              IN  HRPOOL          hPool,
                              IN  HPAGE           hPage,
                              OUT MsgAddrAbsUri**  pAbsUri);


/***************************************************************************
 * AddrAbsUriCopy
 * ------------------------------------------------------------------------
 * General:Copy one Abs-Uri address object to another.
 * Return Value:RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_NULLPTR.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hDestination - Handle of the Abs-Uri address object.
 *    hSource      - Handle of the source object.
 ***************************************************************************/
RvStatus AddrAbsUriCopy(IN RvSipAddressHandle hDestination,
                         IN RvSipAddressHandle hSource);


/***************************************************************************
 * AddrAbsUriEncode
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
RvStatus AddrAbsUriEncode(IN  RvSipAddressHandle hAddr,
                           IN  HRPOOL             hPool,
                           IN  HPAGE              hPage,
                           IN  RvBool            bInUrlHeaders,
                           OUT RvUint32*         length);

/***************************************************************************
 * AddrAbsUriParse
 * ------------------------------------------------------------------------
 * General:This function is used to parse an Abs-Uri address text and construct
 *           a MsgAddrAbsUri object from it.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES,RV_ERROR_NULLPTR,
 *                 RV_ERROR_ILLEGAL_SYNTAX,RV_ERROR_ILLEGAL_SYNTAX.
 * ------------------------------------------------------------------------
 * Arguments:
 *    hSipAddr  - A Handle of the Abs-Uri address object.
 *    buffer    - holds a pointer to the beginning of the null-terminated(!) SIP text header
 ***************************************************************************/
RvStatus AddrAbsUriParse( IN RvSipAddressHandle hSipAddr,
                           IN RvChar*           buffer);


/***************************************************************************
 * AddrAbsUriIsEqual
 * ------------------------------------------------------------------------
 * General:This function comapres between two url objects:
 *           SIP URLs are compared for equality according to the following rules:
 *           1.Comparison of the scheme name.
 *           2.Comparison of the identifier value.
 *         3.The ordering of parameters and headers is not significant.
 * Return Value: RV_TRUE if equal, else RV_FALSE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pHeader      - Handle of the url object.
 *    pOtherHeader - Handle of the url object.
 ***************************************************************************/
RvBool AddrAbsUriIsEqual(IN RvSipAddressHandle pHeader,
                          IN RvSipAddressHandle pOtherHeader);


/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
/***************************************************************************
 * AddrAbsUriCompare
 * ------------------------------------------------------------------------
 * General:This function comapres between two url objects:
 *		   SIP URLs are compared for equality according to the following rules:
 *		   1.Comparison of the scheme name. 
 *		   2.Comparison of the identifier value.
 *         3.The ordering of parameters and headers is not significant.
 * Return Value: Returns 0 if the URLs are equal. Otherwise, the function returns
 *               non-zero value: <0 if first URL is less, or >0 if first URL is more
 *               as the comparison result of a first non-equal URL string element.
 * ------------------------------------------------------------------------
 * Arguments:
 *	pHeader      - Handle of the url object.
 *	pOtherHeader - Handle of the url object.
 ***************************************************************************/
RvInt8 AddrAbsUriCompare(IN RvSipAddressHandle pHeader,
		                  IN RvSipAddressHandle pOtherHeader);

#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */


#ifdef __cplusplus
}
#endif

#endif /* RVSIPABSURI_H */

