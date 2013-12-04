/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                             SipDateHeader.h
 *
 * The file contains 'internal API' for date header.
 *
 *      Author           Date
 *     ------           ------------
 *     Tamar Barzuza    Jan 2001
 ******************************************************************************/
#ifndef SIPDATEHEADER_H
#define SIPDATEHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RV_SIP_DEF.h"
#ifndef RV_SIP_LIGHT

#include "RvSipDateHeader.h"
#include "MsgTypes.h"

/***************************************************************************
 * SipDateConstructInInterval
 * ------------------------------------------------------------------------
 * General: Constructs a Date header object in a given Interval object.
 *          The header handle is returned.
 * Return Value: RV_OK, RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_OUTOFRESOURCES
 * ------------------------------------------------------------------------
 * Arguments:
 * input:  pInterval - Pointer to the Interval object that relates to this date.
 * output: phDate    - Handle to the newly constructed date object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV SipDateConstructInInterval(
                                       IN  MsgInterval*           pInterval,
                                       OUT RvSipDateHeaderHandle *phDate);

/***************************************************************************
 * SipDateHeaderGetPool
 * ------------------------------------------------------------------------
 * General: The function returns the pool of the Date object.
 * Return Value: hPool
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the pool from.
 ***************************************************************************/
HRPOOL SipDateHeaderGetPool(RvSipDateHeaderHandle hHeader);


/***************************************************************************
 * SipDateHeaderGetPage
 * ------------------------------------------------------------------------
 * General: The function returns the page of the Date object.
 * Return Value: hPage
 * ------------------------------------------------------------------------
 * Arguments: hHeader - The header to take the page from.
 ***************************************************************************/
HPAGE SipDateHeaderGetPage(RvSipDateHeaderHandle hHeader);

/***************************************************************************
 * SipDateHeaderSetZone
 * ------------------------------------------------------------------------
 * General: Sets the zone parameter of the Date header.
 * Return Value: RV_OK - success.
 *               RV_ERROR_INVALID_HANDLE - The Date header handle is invalid.
 *               RV_Invalid parameter - The second integer is negative
 *                                      other than UNDEFINED.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeader - Handle to the Date header object.
 *         second - The second number.
 ***************************************************************************/
RvStatus RVCALLCONV SipDateHeaderSetZone(
                                       IN  RvSipDateHeaderHandle    hHeader,
                                       IN  MsgDateTimeZone          zone);

/***************************************************************************
 * SipContactHeaderGetStrBadSyntax
 * ------------------------------------------------------------------------
 * General: This method retrieves the bad-syntax string value from the
 *          header object.
 * Return Value: text string giving the method type
 * ------------------------------------------------------------------------
 * Arguments:
 *    hHeader - Handle of the Authorization header object.
 ***************************************************************************/
RvInt32 SipDateHeaderGetStrBadSyntax(
                                    IN RvSipDateHeaderHandle hHeader);

#endif /*#ifndef RV_SIP_LIGHT*/
#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef SIPDATEHEADER_H */
