/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        _SipMsgUtils.h                                      *
 *                                                                            *
 * This file defines 'internal API' utility functions.                        *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey           Feb.2006                                             *
 ******************************************************************************/



#ifndef SIPMSGUTILS_H
#define SIPMSGUTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rpool_API.h"
#include "RvSipMsgTypes.h"

/***************************************************************************
 * SipMsgUtilsParseQValue
 * ------------------------------------------------------------------------
 * General: Separate QValue to integral part and Decimal Part.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * input:	hPool	  - The pool on which the string lays (if relevant).
 *			hPage	  - The page on which the string lays (if relevant).
 *			offset	  - Offset of the bad-syntax string (if relevant).
 *			strBuffer - Text string giving the bad-syntax to be set in the header. (Optional)
 * output:	intPart	  - The value of the integral part of the QValue. -1 if string is empty.
 *			decPart	  - The value of the decimal part of the QValue.
 ***************************************************************************/
RvStatus SipMsgUtilsParseQValue(IN  HRPOOL		hPool,
								IN  HPAGE		hPage,
								IN  RvInt32		offset,
								IN	RvChar*		strBuffer,
								OUT RvInt32*	intPart,
								OUT RvInt32*	decPart);


#ifdef __cplusplus
}
#endif


#endif /* SIPMSGUTILS_H */
