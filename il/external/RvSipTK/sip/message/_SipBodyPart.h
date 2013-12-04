/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        SipBodyPart.h                                    *
 *                                                                            *
 * This file defines the 'internal API' of the message body-part object.      *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza    Sep 2001                                              *
 ******************************************************************************/



#ifndef SIPBODYPART_H
#define SIPBODYPART_H

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
 * SipBodyPartGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the message body-part object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hBodyPart - The body-part to take the page from.
 ***************************************************************************/
HRPOOL SipBodyPartGetPool(RvSipBodyPartHandle hBodyPart);


/***************************************************************************
 * SipBodyPartGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the message body-part object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hBodyPart - The body-part to take the page from.
 ***************************************************************************/
HPAGE SipBodyPartGetPage(IN RvSipBodyPartHandle hBodyPart);



#endif /* #ifndef RV_SIP_PRIMITIVES*/

#ifdef __cplusplus
}
#endif


#endif /* SIPBODYPART_H */

