/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        RvSipMsgFieldDispatcher.c                           *
 *                                                                            *
 * This file defines a dispatcher for all set and get actions.                *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza     Mar 2005                                             *
 ******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipMsgFieldDispatcher.h"
#include "MsgFieldDispatcher.h"
#include "MsgTypes.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
#ifdef RV_SIP_JSR32_SUPPORT

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------*/
/*                         FUNCTIONS IMPLEMENTATION                      */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipMsgSetHeaderField
 * ------------------------------------------------------------------------
 * General: This function is used to set any header to a message object.
 *          For example: to set the To header of a message, call this function with
 *          eField RVSIP_HEADERTYPE_TO and a pointer to the To value to set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
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
RVAPI RvStatus RVCALLCONV RvSipMsgSetHeaderField(IN    RvSipMsgHandle              hMsg,
												 IN    void*                       pHeader,
												 IN    RvSipHeaderType             eHeaderType,
												 IN    RvSipHeadersLocation        eLocation,
												 INOUT RvSipHeaderListElemHandle*  phListElem,
												 OUT   void**                      pNewHeader)
{
	RvStatus     rv;
	
    if (hMsg == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
		
	rv = MsgSetHeaderField(hMsg, pHeader, eHeaderType, eLocation, phListElem, pNewHeader);
	if (RV_OK != rv)
	{
		RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
			"RvSipMsgSetHeaderField - Failed to set header 0x%p to message 0x%p",
			pHeader, hMsg));		
	}
	
	return rv;
}

/***************************************************************************
 * RvSipMsgGetHeaderField
 * ------------------------------------------------------------------------
 * General: This function is used to get any header from a message object
 *          For example: to get the To header of a message, call this function 
 *          with eField RVSIP_HEADERTYPE_TO.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
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
RVAPI RvStatus RVCALLCONV RvSipMsgGetHeaderField(IN    RvSipMsgHandle               hMsg,
												 IN    RvSipHeaderType              eHeaderType,
												 IN    RvSipHeadersLocation         eLocation,
												 INOUT RvSipHeaderListElemHandle*   phListElem,
												 OUT   void**                       ppHeader)
{
	RvStatus     rv;
	
	if (hMsg == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (ppHeader == NULL)
	{
		return RV_ERROR_BADPARAM;
	}
		
	rv = MsgGetHeaderField(hMsg, eHeaderType, eLocation, phListElem, ppHeader);
	if (RV_OK != rv)
	{
		RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
			"RvSipMsgGetHeaderField - Failed to get header %d from message 0x%p",
			eHeaderType, hMsg));		
	}
	
	return rv;
}

/***************************************************************************
 * RvSipMsgRemoveHeaderField
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
RVAPI RvStatus RVCALLCONV RvSipMsgRemoveHeaderField(
												 IN    RvSipMsgHandle               hMsg,
												 IN    RvSipHeaderType              eHeaderType,
												 IN    RvSipHeaderListElemHandle    hListElem)
{
	RvStatus     rv;
	
	if (hMsg == NULL) 
	{
		return RV_ERROR_INVALID_HANDLE;
	}
		
	rv = MsgRemoveHeaderField(hMsg, eHeaderType, hListElem);
	if (RV_OK != rv)
	{
		RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
			"RvSipMsgRemoveHeaderField - Failed to remove header %d from message 0x%p",
			eHeaderType, hMsg));		
	}
	
	return rv;
}

/***************************************************************************
 * RvSipMsgSetStringField
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
RVAPI RvStatus RVCALLCONV RvSipMsgSetStringField(
										  IN RvSipMsgHandle               hMsg,
										  IN RvSipMsgFieldName            eField,
                                          IN RvChar*                      buffer)
{
	RvStatus     rv;
	
    if (hMsg == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	rv = MsgSetStringField(hMsg, eField, buffer);
	if (RV_OK != rv)
	{
		RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
				   "RvSipMsgSetStringField - Failed to set string field to message 0x%p",
				   hMsg));		
	}

	return rv;
}

/***************************************************************************
 * RvSipMsgGetStringFieldLength
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
RVAPI RvStatus RVCALLCONV RvSipMsgGetStringFieldLength(
                                          IN  RvSipMsgHandle               hMsg,
										  IN  RvSipMsgFieldName            eField,
                                          OUT RvInt32*                     pLength)
{
	RvStatus     rv;
	
    if (hMsg == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (pLength == NULL)
	{
		return RV_ERROR_BADPARAM;
	}
	
	rv = MsgGetStringFieldLength(hMsg, eField, pLength);
	if (RV_OK != rv)
	{
		RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
			"RvSipMsgGetStringFieldLength - Failed to evaluate string field length for message 0x%p",
			hMsg));		
	}
	
	return rv;
}

/***************************************************************************
 * RvSipMsgGetStringField
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
RVAPI RvStatus RVCALLCONV RvSipMsgGetStringField(
										  IN  RvSipMsgHandle               hMsg,
										  IN  RvSipMsgFieldName            eField,
										  IN  RvChar*                      buffer,
                                          IN  RvInt32                      bufferLen,
                                          OUT RvInt32*                     actualLen)
{
	RvStatus     rv;
	
	if (hMsg == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (buffer == NULL || actualLen == NULL)
	{
		return RV_ERROR_BADPARAM;
	}

	rv = MsgGetStringField(hMsg, eField, buffer, bufferLen, actualLen);
	if (RV_OK != rv)
	{
		RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
			"RvSipMsgGetStringField - Failed to get string field of message 0x%p",
			hMsg));		
	}

	return rv;
}

/***************************************************************************
 * RvSipMsgSetIntField
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
RVAPI RvStatus RVCALLCONV RvSipMsgSetIntField(
                                          IN  RvSipMsgHandle               hMsg,
										  IN  RvSipMsgFieldName            eField,
										  IN  RvInt32                      fieldValue)
{
	RvStatus     rv;
	
    if (hMsg == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
		
	rv = MsgSetIntField(hMsg, eField, fieldValue);
	if (RV_OK != rv)
	{
		RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
			"RvSipMsgSetIntField - Failed to set integer field to message 0x%p",
			hMsg));		
	}
	
	return rv;
}

/***************************************************************************
 * RvSipMsgGetIntField
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
RVAPI RvStatus RVCALLCONV RvSipMsgGetIntField(
										IN  RvSipMsgHandle               hMsg,
										IN  RvSipMsgFieldName            eField,
										OUT RvInt32*                     pFieldValue)
{
	RvStatus     rv;
	
	if (hMsg == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (pFieldValue == NULL)
	{
		return RV_ERROR_BADPARAM;
	}
	
	rv = MsgGetIntField(hMsg, eField, pFieldValue);
	if (RV_OK != rv)
	{
		RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
					"RvSipMsgGetIntField - Failed to get integer field of message 0x%p",
					hMsg));		
	}
	
	return rv;
}

/***************************************************************************
 * RvSipMsgSetAddressField
 * ------------------------------------------------------------------------
 * General: This function is used to set an address field to a messages object.
 *          For example: to set the Request-Uri of a request message, call
 *          this function with eType RVSIP_MSG_FIELD_REQUEST_URI and
 *          the address object to set.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg      - A handle to the message object.
 *          eField    - The enumeration value that indicates the field being set.          
 * Inout:   phAddress - Handle to the address to set. Notice that the header
 *                      actually saves a copy of this address. Then the handle
 *                      is changed to point to the new address object.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMsgSetAddressField(
											IN    RvSipMsgHandle      hMsg,
											IN    RvSipMsgFieldName   eField,
											INOUT RvSipAddressHandle* phAddress)
{
	RvStatus     rv;
	
    if (hMsg == NULL || phAddress == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	rv = MsgSetAddressField(hMsg, eField, phAddress);
	if (RV_OK != rv)
	{
		RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
					"RvSipMsgSetAddressField - Failed to set address field to message 0x%p",
					hMsg));		
	}
	
	return rv;
}

/***************************************************************************
 * RvSipMsgGetAddressField
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
RVAPI RvStatus RVCALLCONV RvSipMsgGetAddressField(
											IN  RvSipMsgHandle               hMsg,
											IN  RvSipMsgFieldName            eField,
											OUT RvSipAddressHandle          *phAddress)
{
	RvStatus     rv;
	
    if (hMsg == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (phAddress == NULL)
	{
		return RV_ERROR_BADPARAM;
	}
	
	rv = MsgGetAddressField(hMsg, eField, phAddress);
	if (RV_OK != rv)
	{
		RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
					"RvSipMsgGetAddressField - Failed to get address field of message 0x%p",
					hMsg));		
	}
	
	return rv;
}

#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

#ifdef __cplusplus
}
#endif

