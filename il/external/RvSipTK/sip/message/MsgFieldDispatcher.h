/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        MsgFieldDispatcher.h                                *
 *                                                                            *
 * This file defines a dispatcher for all set and get actions.                *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza     Apr 2005                                             *
 ******************************************************************************/

#ifndef MSGFIELDDISPATCHER_H
#define MSGFIELDDISPATCHER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipMsgTypes.h"
#include "RvSipMsg.h"
#include "rpool_API.h"

/*-----------------------------------------------------------------------*/
/*                   DECLERATIONS                                        */
/*-----------------------------------------------------------------------*/

#ifdef RV_SIP_JSR32_SUPPORT

/***************************************************************************
 * MsgSetHeaderField
 * ------------------------------------------------------------------------
 * General: This function is used to set any header to a message object.
 *          For example: to set the To header of a message, call this function with
 *          eField RVSIP_HEADERTYPE_TO and a pointer to the To value to set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg        - A handle to the message object.
 *          pHeader     - A pointer to the header value to be set. Notice that the 
 *                        message actually saves a copy of this header.
 *          eHeaderType - The enumeration that indicates the header to set.
 *          eLocation   - The location in the list of headers: next, previous, first or last.
 *                        This will be relevant only to the headers that are part of
 *                        the list (for example Allow, Via, Contact and not To, From etc.)
 * Inout:   phListElem  - Handle to the current position in the list. Supply this
 *                        value if you chose next or previous in the location parameter.
 *                        This is also an output parameter and will be set with a link
 *                        to requested header in the list. Notice that for headers that 
 *                        are not part of the list such as To and From it will be NULL.
 * Output:  pNewHeader  - An inside pointer to the message pointing at its actual
 *                        new header (after copy).
 ***************************************************************************/
RvStatus RVCALLCONV MsgSetHeaderField(
								 IN    RvSipMsgHandle              hMsg,
								 IN    void*                       pHeader,
								 IN    RvSipHeaderType             eHeaderType,
                                 IN    RvSipHeadersLocation        eLocation,
								 INOUT RvSipHeaderListElemHandle*  phListElem,
								 OUT   void**                      pNewHeader);

/***************************************************************************
 * MsgGetHeaderField
 * ------------------------------------------------------------------------
 * General: This function is used to get any header from a message object
 *          For example: to get the To header of a message, call this function 
 *          with eField RVSIP_HEADERTYPE_TO.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg      - A handle to the message object.
 *          eField    - The enumeration that indicates the header to set.
 *          eLocation - The location in the list of headers: next, previous, first or last.
 *                      This will be relevant only to the headers that are part of
 *                      the list (for example Allow, Via, Contact and not To, From etc.)
 * Inout:   hListElem - Handle to the current position in the list. Supply this
 *                      value if you chose next or previous in the location parameter.
 *                      This is also an output parameter and will be set with a link
 *                      to requested header in the list. Notice that for headers that 
 *                      are not part of the list such as To and From it will be NULL.
 * Output:  ppHeader - A pointer to the requetsed header.
 ***************************************************************************/
RvStatus RVCALLCONV MsgGetHeaderField(
							     IN    RvSipMsgHandle               hMsg,
								 IN    RvSipHeaderType              eHeaderType,
								 IN    RvSipHeadersLocation         eLocation,
                                 INOUT RvSipHeaderListElemHandle*   phListElem,
                                 OUT   void**                       ppHeader);

/***************************************************************************
 * MsgRemoveHeaderField
 * ------------------------------------------------------------------------
 * General: This function is used to remove any header from a message object
 *          For example: to remove the To header of a message, call this function 
 *          with eField RVSIP_HEADERTYPE_TO.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Input:   hMsg      - A handle to the message object.
 *          eField    - The enumeration that indicates the header to set.
 *          hListElem - Handle to the headers position in the list. Notice that for headers that 
 *                      are not part of the list such as To and From it will be NULL.
 ***************************************************************************/
RvStatus RVCALLCONV MsgRemoveHeaderField(IN    RvSipMsgHandle               hMsg,
										 IN    RvSipHeaderType              eHeaderType,
										 IN    RvSipHeaderListElemHandle    hListElem);

/***************************************************************************
 * MsgSetStringField
 * ------------------------------------------------------------------------
 * General: This function is used to set any string field of a message object.
 *          For example: to set the Method of a request message, call this function 
 *          with eField RVSIP_MSG_FIELD_METHOD and buffer containing the 
 *          method value.
 *
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg     - A handle to the message object.
 *          eField   - The enumeration value that indicates the field being set.
 *          buffer   - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV MsgSetStringField(IN RvSipMsgHandle               hMsg,
									  IN RvSipMsgFieldName            eField,
                                      IN RvChar*                      buffer);

/***************************************************************************
 * MsgGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: This function is used to get the length of any string field of 
 *          a message object. Use this function to evaluate the appropriate 
 *          buffer size to allocate before calling the GetStringField function.
 *          For example: to get the length of the body, 
 *          call this function with eField RVSIP_MSG_FIELD_BODY. 
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg    - A handle to the message object.
 *          eField  - The enumeration value that indicates the field being evaluated.
 *          pLength - The length of the requested string including the null
 *                    terminator character.
 ***************************************************************************/
RvStatus RVCALLCONV MsgGetStringFieldLength(
                                          IN  RvSipMsgHandle               hMsg,
										  IN  RvSipMsgFieldName            eField,
                                          OUT RvInt32*                     pLength);

/***************************************************************************
 * MsgGetStringField
 * ------------------------------------------------------------------------
 * General: This function is used to get any string field of a message object.
 *          For example: to get the Method of a request message, call this function with
 *          eField RVSIP_MSG_FIELD_METHOD and an adequate buffer.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg      - A handle to the message object.
 *          eField    - The enumeration value that indicates the field being get.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV MsgGetStringField(IN  RvSipMsgHandle               hMsg,
									  IN  RvSipMsgFieldName            eField,
									  IN  RvChar*                      buffer,
                                      IN  RvInt32                      bufferLen,
                                      OUT RvInt32*                     actualLen);

/***************************************************************************
 * MsgSetIntField
 * ------------------------------------------------------------------------
 * General: This function is used to set an integer field to a messages object.
 *          For example: to set the status code of a response message, call 
 *          this function with eField RVSIP_MSG_FIELD_STATUS_CODE and the 
 *          status code value
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg       - A handle to the message object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Int value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV MsgSetIntField(IN  RvSipMsgHandle               hMsg,
								   IN  RvSipMsgFieldName            eField,
								   IN  RvInt32                      fieldValue);

/***************************************************************************
 * MsgGetIntField
 * ------------------------------------------------------------------------
 * General: This function is used to get an integer field of a messages object.
 *          For example: to get the status code of a response message, call this 
 *          function with eField RVSIP_MSG_FIELD_STATUS_CODE.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg        - A handle to the message object.
 *          eField      - The enumeration value that indicates the field being get.          
 * Output:  pFieldValue - The Int value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV MsgGetIntField(IN  RvSipMsgHandle               hMsg,
								   IN  RvSipMsgFieldName            eField,
								   OUT RvInt32*                     pFieldValue);

/***************************************************************************
 * MsgSetAddressField
 * ------------------------------------------------------------------------
 * General: This function is used to set an address field to a messages object.
 *          For example: to set the Request-Uri of a request message, call
 *          this function with eType RVSIP_MSG_FIELD_REQUEST_URI and
 *          the address object to set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg     - A handle to the message object.
 *          eField   - The enumeration value that indicates the field being set.          
 * Inout:   phAddress - Handle to the address to set. Notice that the header
 *                      actually saves a copy of this address. Then the handle
 *                      is changed to point to the new address object.
 ***************************************************************************/
RvStatus RVCALLCONV MsgSetAddressField(IN    RvSipMsgHandle       hMsg,
									   IN    RvSipMsgFieldName    eField,
									   INOUT RvSipAddressHandle*  phAddress);

/***************************************************************************
 * MsgGetAddressField
 * ------------------------------------------------------------------------
 * General: This function is used to get an address field from a messages object.
 *          For example: to get the Request-Uri of a request message, call
 *          this function with eType RVSIP_MSG_FIELD_REQUEST_URI.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg      - A handle to the message object.
 *          eField    - The enumeration value that indicates the field being get.          
 * Output:  phAddress - A handle to the address object that is held by this header.
 ***************************************************************************/
RvStatus RVCALLCONV MsgGetAddressField(IN  RvSipMsgHandle               hMsg,
									   IN  RvSipMsgFieldName            eField,
									   OUT RvSipAddressHandle          *phAddress);

#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

#ifdef __cplusplus
}
#endif


#endif /* MSGFIELDDISPATCHER_H */

