 /*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                         MsgInterval.h                                      *
 *                                                                            *
 * The file defines internal functions of the interval object.                *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             Dec.2001                                             *
 ******************************************************************************/



#ifndef MSG_INTERVAL_H
#define MSG_INTERVAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "MsgTypes.h"

/****************************************************/
/*        CONSTRUCTORS AND DESTRUCTORS                */
/****************************************************/

/***************************************************************************
 * IntervalConstruct
 * ------------------------------------------------------------------------
 * General: Constructs and initializes a stand-alone MsgInterval object.
 *          The objectr is constructed on a given page taken from a specified
 *          pool. The pointer to the new header object is returned.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES.
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  pMsgMgr - Pointer to the message manager.
 *         hPool -   Handle to the memory pool that the object will use.
 *         hPage   - Handle to the memory page that the object will use.
 * output: pInterval - Pointer to the newly constructed interval object.
 ***************************************************************************/
RvStatus RVCALLCONV IntervalConstruct( IN  MsgMgr*       pMsgMgr,
                                        IN  HRPOOL       hPool,
                                        IN  HPAGE        hPage,
                                        OUT MsgInterval** pInterval);

/***************************************************************************
 * IntervalCopy
 * ------------------------------------------------------------------------
 * General:Copies all fields from a source Interval object to a
 *         destination Interval object.
 *         You must construct the destination object before using this function.
 * Return Value: RV_OK, RV_ERROR_OUTOFRESOURCES, RV_ERROR_INVALID_HANDLE.
 * ------------------------------------------------------------------------
 * Arguments:
 *    pDestination - Pointer to the destination Interval object.
 *    pSource      - Pointer to the source Interval object.
 ***************************************************************************/
RvStatus RVCALLCONV IntervalCopy(INOUT MsgInterval* pDestination,
                                  IN    MsgInterval* pSource);

/***************************************************************************
 * IntervalSetDate
 * ------------------------------------------------------------------------
 * General: Sets a new Date header in the Interval object and changes
 *          the Interval format to date. (The function allocates a date header,
 *          and copy the given hDate object to it).
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Retry-After header handle is invalid.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Retry-After header object.
 *         hDate - The date handle to be set to the Retry-After header.
 *                 If the date handle is NULL, the existing date header
 *                 is removed from the expires header.
 ***************************************************************************/
RvStatus RVCALLCONV IntervalSetDate(IN  MsgInterval*             pInterval,
                                     IN  RvSipDateHeaderHandle    hDate);

#endif /*#ifndef RV_SIP_LIGHT*/
#ifdef __cplusplus
}
#endif

#endif /* MSG_INTERVAL_H */
