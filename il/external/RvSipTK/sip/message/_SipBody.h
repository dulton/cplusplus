/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        SipBody.h                                        *
 *                                                                            *
 * This file defines the 'internal API' of the message body object.           *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza    Sep 2001                                              *
 ******************************************************************************/



#ifndef SIPBODY_H
#define SIPBODY_H

#ifdef __cplusplus
extern "C" {
#endif
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_PRIMITIVES

#include "rpool_API.h"
#include "RvSipMsgTypes.h"

/****************************************************/
/*        POOL-PAGE FUNCTIONS                           */
/****************************************************/

/***************************************************************************
 * SipBodyGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the message body object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hBody - The body to take the page from.
 ***************************************************************************/
HRPOOL SipBodyGetPool(RvSipBodyHandle hBody);


/***************************************************************************
 * SipBodyGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the message body object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hBody - The body to take the page from.
 ***************************************************************************/
HPAGE SipBodyGetPage(IN RvSipBodyHandle hBody);

/***************************************************************************
 * SipBodySetRawBuffer
 * ------------------------------------------------------------------------
 * General: This method sets the raw buffer of the message body.
 *          The raw buffer string is given on a memory page, indicated
 *          by it's offset.
 * Return Value:  RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  hBody - Handle of a message body object.
 *         bufferOffset - The offset of the raw buffer string.
 *         hPool - The pool on which the string lays (if relevant)
 *         hPage - The page on which the string lays (if relevant)
 *         length - The length of the raw buffer string.
 ***************************************************************************/
RvStatus SipBodySetRawBuffer(IN    RvSipBodyHandle     hBody,
                              IN    HRPOOL              hPool,
                              IN    HPAGE               hPage,
                              IN    RvInt32            bufferOffset,
                              IN    RvUint32           length);


#endif /* #ifndef RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif


#endif /* SIPBODY_H */

