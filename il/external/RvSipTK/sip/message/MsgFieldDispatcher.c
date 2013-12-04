/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        MsgFieldDispatcher.c                                *
 *                                                                            *
 * This file defines a dispatcher for all set and get actions.                *
 *                                                                            *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *     Tamar Barzuza     Apr 2005                                             *
 ******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "MsgFieldDispatcher.h"
#include "_SipCommonConversions.h"
#include "MsgTypes.h"
#include "RvSipBody.h"


/*#include <memory.h>*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
#ifdef RV_SIP_JSR32_SUPPORT

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV StringFieldCopy(
								IN  RvChar*                     strToCopy,
								IN  RvChar*                     buffer,
								IN  RvInt32                     bufferLen,
								OUT RvInt32*                    actualLen);

static RvStatus RVCALLCONV ConvertFieldTypeToStringType(
									IN  RvSipMsgFieldName        eFieldType,
									OUT RvSipMessageStringName  *peStringName);
														

/*-----------------------------------------------------------------------*/
/*                         FUNCTIONS IMPLEMENTATION                      */
/*-----------------------------------------------------------------------*/


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
									  OUT   void**                      pNewHeader)
{
	RvStatus                   rv;

	switch (eHeaderType) 
	{
	case RVSIP_HEADERTYPE_ALLOW:
	case RVSIP_HEADERTYPE_CONTACT:
	case RVSIP_HEADERTYPE_VIA:
	case RVSIP_HEADERTYPE_EXPIRES:
	case RVSIP_HEADERTYPE_DATE:
	case RVSIP_HEADERTYPE_ROUTE_HOP:
	case RVSIP_HEADERTYPE_AUTHENTICATION:
	case RVSIP_HEADERTYPE_AUTHORIZATION:
	case RVSIP_HEADERTYPE_REFER_TO:
	case RVSIP_HEADERTYPE_REFERRED_BY:
	case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
	case RVSIP_HEADERTYPE_RETRY_AFTER:
	case RVSIP_HEADERTYPE_OTHER:
	case RVSIP_HEADERTYPE_RSEQ:
	case RVSIP_HEADERTYPE_RACK:
	case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
	case RVSIP_HEADERTYPE_EVENT:
	case RVSIP_HEADERTYPE_ALLOW_EVENTS:
	case RVSIP_HEADERTYPE_SESSION_EXPIRES:
	case RVSIP_HEADERTYPE_MINSE:
	case RVSIP_HEADERTYPE_REPLACES:
	case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
	case RVSIP_HEADERTYPE_REASON:
	case RVSIP_HEADERTYPE_WARNING:
	case RVSIP_HEADERTYPE_TIMESTAMP:
	case RVSIP_HEADERTYPE_INFO:
	case RVSIP_HEADERTYPE_ACCEPT:
	case RVSIP_HEADERTYPE_ACCEPT_ENCODING:
	case RVSIP_HEADERTYPE_ACCEPT_LANGUAGE:
	case RVSIP_HEADERTYPE_REPLY_TO:
		{
			/* These headers are held within a list in the message object */
			return RvSipMsgPushHeader(hMsg, eLocation, 
									  pHeader, eHeaderType, 
									  phListElem, pNewHeader);
		}
	case RVSIP_HEADERTYPE_TO:
		{
			*phListElem = NULL;/*NANA*/
			rv = RvSipMsgSetToHeader(hMsg, (RvSipPartyHeaderHandle)pHeader);
			if (RV_OK == rv)
			{
				*pNewHeader = (void*)RvSipMsgGetToHeader(hMsg);
			}
			break;
		}
	case RVSIP_HEADERTYPE_FROM:
		{
			*phListElem = NULL;/*NANA*/
			rv = RvSipMsgSetFromHeader(hMsg, (RvSipPartyHeaderHandle)pHeader);
			if (RV_OK == rv)
			{
				*pNewHeader = (void*)RvSipMsgGetFromHeader(hMsg);
			}
			break;
		}
	case RVSIP_HEADERTYPE_CSEQ:
		{
			*phListElem = NULL;/*NANA*/
			rv = RvSipMsgSetCSeqHeader(hMsg, (RvSipCSeqHeaderHandle)pHeader);
			if (RV_OK == rv)
			{
				*pNewHeader = (void*)RvSipMsgGetCSeqHeader(hMsg);
			}
			break;
		}
	case RVSIP_HEADERTYPE_CONTENT_TYPE:
		{	
			/* The Content-Type header is held within the Body of the message object */
			RvSipBodyHandle hMsgBody = RvSipMsgGetBodyObject(hMsg);
			if (hMsgBody == NULL)
			{
				rv = RvSipBodyConstructInMsg(hMsg, &hMsgBody);
				if (RV_OK != rv)
				{
					RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
							"MsgSetHeaderField - failed to construct body header in order to set Content-Type in message 0x%p",
							hMsg));	
				}
			}
			rv = RvSipBodySetContentType(hMsgBody, (RvSipContentTypeHeaderHandle)pHeader);
			if (RV_OK == rv)
			{
				*pNewHeader = (void*)RvSipBodyGetContentType(hMsgBody);
			}
			break;
		}
	default:
		{
			RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
				"MsgSetHeaderField - field %d does not indicate a header field of the message object",
				eHeaderType));	
			return RV_ERROR_BADPARAM;
		}
	}

	return rv;
}

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
 * Output:  ppHeader - A pointer to the requested header.
 ***************************************************************************/
RvStatus RVCALLCONV MsgGetHeaderField(IN    RvSipMsgHandle               hMsg,
									  IN    RvSipHeaderType              eHeaderType,
									  IN    RvSipHeadersLocation         eLocation,
									  INOUT RvSipHeaderListElemHandle*   phListElem,
									  OUT   void**                       ppHeader)
{
	
	switch (eHeaderType) 
	{
	case RVSIP_HEADERTYPE_ALLOW:
	case RVSIP_HEADERTYPE_CONTACT:
	case RVSIP_HEADERTYPE_VIA:
	case RVSIP_HEADERTYPE_EXPIRES:
	case RVSIP_HEADERTYPE_DATE:
	case RVSIP_HEADERTYPE_ROUTE_HOP:
	case RVSIP_HEADERTYPE_AUTHENTICATION:
	case RVSIP_HEADERTYPE_AUTHORIZATION:
	case RVSIP_HEADERTYPE_REFER_TO:
	case RVSIP_HEADERTYPE_REFERRED_BY:
	case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
	case RVSIP_HEADERTYPE_RETRY_AFTER:
	case RVSIP_HEADERTYPE_OTHER:
	case RVSIP_HEADERTYPE_RSEQ:
	case RVSIP_HEADERTYPE_RACK:
	case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
	case RVSIP_HEADERTYPE_EVENT:
	case RVSIP_HEADERTYPE_ALLOW_EVENTS:
	case RVSIP_HEADERTYPE_SESSION_EXPIRES:
	case RVSIP_HEADERTYPE_MINSE:
	case RVSIP_HEADERTYPE_REPLACES:
	case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
	case RVSIP_HEADERTYPE_REASON:
	case RVSIP_HEADERTYPE_WARNING:
		{
			/* These headers are held within a list in the message object */
			*ppHeader = RvSipMsgGetHeaderByType(hMsg, 
												eHeaderType, 
												eLocation, 
												phListElem);
			break;
		}
	case RVSIP_HEADERTYPE_TO:
		{
			*ppHeader = (void*)RvSipMsgGetToHeader(hMsg);
			break;
		}
	case RVSIP_HEADERTYPE_FROM:
		{
			*ppHeader = (void*)RvSipMsgGetFromHeader(hMsg);
			break;
		}
	case RVSIP_HEADERTYPE_CSEQ:
		{
			*ppHeader = (void*)RvSipMsgGetCSeqHeader(hMsg);
			break;
		}
	case RVSIP_HEADERTYPE_CONTENT_TYPE:
		{
			/* The Content-Type header is held within the Body of the message object */
			RvSipBodyHandle hMsgBody = RvSipMsgGetBodyObject(hMsg);
			if (hMsgBody == NULL)
			{
				*ppHeader = NULL;
			}
			else
			{
				*ppHeader = (void*)RvSipBodyGetContentType(hMsgBody);
			}
			break;
		}
	default:
		{
			RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
				"MsgGetHeaderField - field %d does not indicate a header field of the message object",
				eHeaderType));	
			return RV_ERROR_BADPARAM;
		}
	}

	if (*ppHeader == NULL)
	{
		return RV_ERROR_NOT_FOUND;
	}

	return RV_OK;
}

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
										 IN    RvSipHeaderListElemHandle    hListElem)
{
	
	switch (eHeaderType) 
	{
	case RVSIP_HEADERTYPE_ALLOW:
	case RVSIP_HEADERTYPE_CONTACT:
	case RVSIP_HEADERTYPE_VIA:
	case RVSIP_HEADERTYPE_EXPIRES:
	case RVSIP_HEADERTYPE_DATE:
	case RVSIP_HEADERTYPE_ROUTE_HOP:
	case RVSIP_HEADERTYPE_AUTHENTICATION:
	case RVSIP_HEADERTYPE_AUTHORIZATION:
	case RVSIP_HEADERTYPE_REFER_TO:
	case RVSIP_HEADERTYPE_REFERRED_BY:
	case RVSIP_HEADERTYPE_CONTENT_DISPOSITION:
	case RVSIP_HEADERTYPE_RETRY_AFTER:
	case RVSIP_HEADERTYPE_OTHER:
	case RVSIP_HEADERTYPE_RSEQ:
	case RVSIP_HEADERTYPE_RACK:
	case RVSIP_HEADERTYPE_SUBSCRIPTION_STATE:
	case RVSIP_HEADERTYPE_EVENT:
	case RVSIP_HEADERTYPE_ALLOW_EVENTS:
	case RVSIP_HEADERTYPE_SESSION_EXPIRES:
	case RVSIP_HEADERTYPE_MINSE:
	case RVSIP_HEADERTYPE_REPLACES:
	case RVSIP_HEADERTYPE_AUTHENTICATION_INFO:
	case RVSIP_HEADERTYPE_REASON:
	case RVSIP_HEADERTYPE_WARNING:
	case RVSIP_HEADERTYPE_INFO:
		{
			/* These headers are held within a list in the message object */
			return RvSipMsgRemoveHeaderAt(hMsg, hListElem);
		}
	case RVSIP_HEADERTYPE_TO:
		{
			return RvSipMsgSetToHeader(hMsg, NULL);
		}
	case RVSIP_HEADERTYPE_FROM:
		{
			return RvSipMsgSetFromHeader(hMsg, NULL);
		}
	case RVSIP_HEADERTYPE_CSEQ:
		{
			return RvSipMsgSetCSeqHeader(hMsg, NULL);
		}
	case RVSIP_HEADERTYPE_CONTENT_TYPE:
		{
			/* The Content-Type header is held within the Body of the message object */
			RvSipBodyHandle hMsgBody = RvSipMsgGetBodyObject(hMsg);
			if (hMsgBody == NULL)
			{
				return RV_OK;
			}
			else
			{
				return RvSipBodySetContentType(hMsgBody, NULL);
			}
		}
	default:
		{
			RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
				"MsgRemoveHeaderField - field %d does not indicate a header field of the message object",
				eHeaderType));	
			return RV_ERROR_BADPARAM;
		}
	}
	
	return RV_OK;
}

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
                                      IN RvChar*                      buffer)
{

	switch (eField) 
	{
	case RVSIP_MSG_FIELD_METHOD:
		{
			/* First convert string into enumeration and then set */
			RvSipMethodType eMethod = SipCommonConvertStrToEnumMethodType(buffer);
			return RvSipMsgSetMethodInRequestLine(hMsg, eMethod, buffer);
		}
	case RVSIP_MSG_FIELD_REASON_PHRASE:
		{
			return RvSipMsgSetReasonPhrase(hMsg, buffer);
		}
	case RVSIP_MSG_FIELD_BODY:
		{
			return RvSipMsgSetBody(hMsg, buffer);
		}
	case RVSIP_MSG_FIELD_REQUEST_URI:
	case RVSIP_MSG_FIELD_STATUS_CODE:
	default:
		{
			RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
				"MsgSetStringField - field %d does not indicate a string field of the message object",
				eField));	
			return RV_ERROR_BADPARAM;
		}
	}
}

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
                                      OUT RvInt32*                     actualLen)
{
	
	switch (eField) 
	{
	case RVSIP_MSG_FIELD_METHOD:
		{
			RvSipMethodType eMethodType = RvSipMsgGetRequestMethod(hMsg);
			if (eMethodType == RVSIP_METHOD_OTHER)
			{
			/* The OTHER enumeration indicates that there is a string 
				method set to this allowed header */
				return RvSipMsgGetStrRequestMethod(hMsg, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else
			{
				/* Convert the retrieved enumeration into string */
				RvChar *strMethodType = SipCommonConvertEnumToStrMethodType(eMethodType);
				return StringFieldCopy(strMethodType, buffer, bufferLen, actualLen);
			}
		}
	case RVSIP_MSG_FIELD_REASON_PHRASE:
		{
			return RvSipMsgGetReasonPhrase(hMsg, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_MSG_FIELD_BODY:
		{
			return RvSipMsgGetBody(hMsg, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_MSG_FIELD_REQUEST_URI:
	case RVSIP_MSG_FIELD_STATUS_CODE:
	default:
		{
			RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
				"MsgGetStringField - field %d does not indicate a string field of the message object",
				eField));	
			return RV_ERROR_BADPARAM;
		}
	}
}

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
                                          OUT RvInt32*                     pLength)
{
	RvSipMessageStringName eStringName;
	RvStatus               rv;

	/* Convert between field-type enumerations and string-type enumerations */
	rv = ConvertFieldTypeToStringType(eField, &eStringName);
	if (RV_OK != rv)
	{
		return rv;
	}

	switch (eField) 
	{
	case RVSIP_MSG_FIELD_REASON_PHRASE:
	case RVSIP_MSG_FIELD_BODY:
		{
			*pLength = (RvInt32)RvSipMsgGetStringLength(hMsg, eStringName);
			break;
		}
	case RVSIP_MSG_FIELD_METHOD:
		{
			RvSipMethodType eMethodType = RvSipMsgGetRequestMethod(hMsg);
			
			if (eMethodType != RVSIP_METHOD_OTHER &&
				eMethodType != RVSIP_METHOD_UNDEFINED)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = (RvInt32)RvSipMsgGetStringLength(hMsg, eStringName);
			}
			break;
		}
	case RVSIP_MSG_FIELD_REQUEST_URI:
	case RVSIP_MSG_FIELD_STATUS_CODE:
	default:
		{
			RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
				"MsgGetStringFieldLength - field %d does not indicate a string field of the message object",
				eField));	
			return RV_ERROR_BADPARAM;
		}
	}

	return RV_OK;
}

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
								   IN  RvInt32                      fieldValue)
{
	
	switch (eField) 
	{
	case RVSIP_MSG_FIELD_STATUS_CODE:
		{
			return RvSipMsgSetStatusCode(hMsg, (RvInt16)fieldValue, RV_TRUE);
		}
	case RVSIP_MSG_FIELD_METHOD:
	case RVSIP_MSG_FIELD_REQUEST_URI:
	case RVSIP_MSG_FIELD_REASON_PHRASE:
	case RVSIP_MSG_FIELD_BODY:
	default:
		{
			RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
				      "MsgSetIntField - field %d does not indicate an integer field of the message object",
				      eField));	
			return RV_ERROR_BADPARAM;
		}
	}
}

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
								   OUT RvInt32*                     pFieldValue)
{

	switch (eField) 
	{
	case RVSIP_MSG_FIELD_STATUS_CODE:
		{
			*pFieldValue = (RvInt16)RvSipMsgGetStatusCode(hMsg);
			break;
		}
	case RVSIP_MSG_FIELD_METHOD:
	case RVSIP_MSG_FIELD_REQUEST_URI:
	case RVSIP_MSG_FIELD_REASON_PHRASE:
	case RVSIP_MSG_FIELD_BODY:
	default:
		{
			RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
				"MsgGetIntField - field %d does not indicate an integer field of the message object",
				eField));	
			return RV_ERROR_BADPARAM;
		}
	}

	return RV_OK;
}

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
									   INOUT RvSipAddressHandle*  phAddress)
{
	RvStatus rv;

	switch (eField) 
	{
	case RVSIP_MSG_FIELD_REQUEST_URI:
		{
			rv = RvSipMsgSetRequestUri(hMsg, *phAddress);
			if (RV_OK != rv)
			{
				return rv;
			}
			*phAddress = RvSipMsgGetRequestUri(hMsg);
			break;
		}
	case RVSIP_MSG_FIELD_STATUS_CODE:
	case RVSIP_MSG_FIELD_METHOD:
	case RVSIP_MSG_FIELD_REASON_PHRASE:
	case RVSIP_MSG_FIELD_BODY:
	default:
		{
			RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
				      "MsgSetAddressField - field %d does not indicate an address field of the message object",
				      eField));	
			return RV_ERROR_BADPARAM;
		}
	}

	return RV_OK;
}

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
									   OUT RvSipAddressHandle          *phAddress)
{
	
	switch (eField) 
	{
	case RVSIP_MSG_FIELD_REQUEST_URI:
		{
			*phAddress = RvSipMsgGetRequestUri(hMsg);
			break;
		}
	case RVSIP_MSG_FIELD_STATUS_CODE:
	case RVSIP_MSG_FIELD_METHOD:
	case RVSIP_MSG_FIELD_REASON_PHRASE:
	case RVSIP_MSG_FIELD_BODY:
	default:
		{
			RvLogError(((MsgMessage*)hMsg)->pMsgMgr->pLogSrc, (((MsgMessage*)hMsg)->pMsgMgr->pLogSrc,
				"MsgGetAddressField - field %d does not indicate an address field of the message object",
				eField));	
			return RV_ERROR_BADPARAM;
		}
	}

	if (*phAddress == NULL)
	{
		return RV_ERROR_NOT_FOUND;
	}

	return RV_OK;
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * StringFieldCopy
 * ------------------------------------------------------------------------
 * General: This function copies a static string onto a given buffer.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   strToCopy - The string to copy
 *          buffer    - The buffer to fill
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the copied string
 ***************************************************************************/
static RvStatus RVCALLCONV StringFieldCopy(
                                    IN  RvChar*                     strToCopy,
                                    IN  RvChar*                     buffer,
                                    IN  RvInt32                     bufferLen,
                                    OUT RvInt32*                    actualLen)
{
	RvInt32     strLen;

	strLen  = strlen(strToCopy);

	if (strLen > bufferLen - 1)
	{
		return RV_ERROR_INSUFFICIENT_BUFFER;
	}

	strcpy(buffer, strToCopy);
	*actualLen = strLen + 1;

	return RV_OK;
}

/***************************************************************************
 * ConvertFieldTypeToStringType
 * ------------------------------------------------------------------------
 * General: This function converts a field type enumeration into a string 
 *          name enumeration.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   eFieldType   - The field type enumeration
 * Output:  peStringName - The returned string name enumeration
 ***************************************************************************/
static RvStatus RVCALLCONV ConvertFieldTypeToStringType(
                                    IN  RvSipMsgFieldName        eFieldType,
									OUT RvSipMessageStringName  *peStringName)
{			
	switch(eFieldType) {
		case RVSIP_MSG_FIELD_METHOD:
			{
				*peStringName = RVSIP_MSG_REQUEST_METHOD;
				break;
			}
		case RVSIP_MSG_FIELD_REASON_PHRASE:
		{		
			*peStringName = RVSIP_MSG_RESPONSE_PHRASE;
			break;
		}
		case RVSIP_MSG_FIELD_BODY:
		{			
			*peStringName = RVSIP_MSG_BODY;
			break;
		}
		default:
		{
			return RV_ERROR_BADPARAM;
		}
	}

	return RV_OK;
}

#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

#ifdef __cplusplus
}
#endif

