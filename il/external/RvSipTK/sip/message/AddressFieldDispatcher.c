/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        HeaderFieldDispatcher.c                             *
 *                                                                            *
 * This file contains set and get functions for each of the headers supported *
 * by the stack. These functions receive either RvInt32 or string parameters, *
 * and convert them according to the actual field expected by the header.     *
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
#include "RvSipAddress.h"
#include "_SipAddress.h"
#include "HeaderFieldDispatcher.h"
#include "_SipCommonConversions.h"
#include "MsgTypes.h"


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
									IN  RvSipAddressFieldName        eFieldType,
									OUT RvSipAddressStringName      *peStringName);

/*-----------------------------------------------------------------------*/
/*                         FUNCTIONS IMPLEMENTATION                      */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * AddressDestruct
 * ------------------------------------------------------------------------
 * General: Destruct an address object and the memory it lies on.
 *          Notice: This function will free the page that the header lies on.
 *          Therefore it can only be used when it is the only information on 
 *          this page, i.e. if it is a stand-alone header with no page partners.          
 *
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAddress  - A handle to the address object to destruct.
 ***************************************************************************/
void RVCALLCONV AddressDestruct(IN RvSipAddressHandle           hAddress)
{
	HRPOOL hPool;
	HPAGE  hPage;

	hPool = SipAddrGetPool(hAddress);
	hPage = SipAddrGetPage(hAddress);

	RPOOL_FreePage(hPool, hPage);
}

/***************************************************************************
 * AddressSetStringField
 * ------------------------------------------------------------------------
 * General: This function is used to set any string field of an address object.
 *          For example: to set the host of a SIP URL, call this function with
 *          eField RVSIP_ADDRESS_SIP_URL_FIELD_HOST and buffer containing the 
 *          host value.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAddress - A handle to the address object.
 *          eField   - The enumeration value that indicates the field being set.
 *          buffer   - Buffer containing the textual field value (null terminated)
 ***************************************************************************/
RvStatus RVCALLCONV AddressSetStringField(
                                          IN RvSipAddressHandle           hAddress,
										  IN RvSipAddressFieldName        eField,
                                          IN RvChar*                      buffer)
{

	switch (eField) 
	{
	case RVSIP_ADDRESS_FIELD_DISPLAY_NAME:
		{
			return RvSipAddrSetDisplayName(hAddress, buffer);
		}
	/*Sip URL related fields */
	case RVSIP_ADDRESS_SIP_URL_FIELD_USER:
		{
			return RvSipAddrUrlSetUser(hAddress, buffer);
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_HOST:
		{
			return RvSipAddrUrlSetHost(hAddress, buffer);
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_METHOD:
		{
			/* Before setting, convert string to enumeration */
			RvSipMethodType eMethod = SipCommonConvertStrToEnumMethodType(buffer);
			return RvSipAddrUrlSetMethod(hAddress, eMethod, buffer);
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_TRANSPORT:
		{
			/* Before setting, convert string to enumeration */
			RvSipTransport eTransport = SipCommonConvertStrToEnumTransportType(buffer);
			return RvSipAddrUrlSetTransport(hAddress, eTransport, buffer);
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_MADDR:
		{
			return RvSipAddrUrlSetMaddrParam(hAddress, buffer);
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_USER_PARAM:
		{
			/* Before setting, convert string to enumeration */
			RvSipUserParam eUserParam = SipCommonConvertStrToEnumUserParam(buffer);
			return RvSipAddrUrlSetUserParam(hAddress, eUserParam, buffer);
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_COMP:
		{
			/* Before setting, convert string to enumeration */
			RvSipCompType eCompType = SipCommonConvertStrToEnumCompType(buffer);
			return RvSipAddrUrlSetCompParam(hAddress, eCompType, buffer);
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_OTHER_PARAMS:
		{
			return RvSipAddrUrlSetOtherParams(hAddress, buffer);
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_HEADERS:
		{
			return RvSipAddrUrlSetHeaders(hAddress, buffer);
		}
	/*Tel URI related fields */
	case RVSIP_ADDRESS_TEL_URI_FIELD_PHONE_NUM:
		{
			return RvSipAddrTelUriSetPhoneNumber(hAddress, buffer);
		}
	case RVSIP_ADDRESS_TEL_URI_FIELD_ISDN_SUB_ADDR:
		{
			return RvSipAddrTelUriSetIsdnSubAddr(hAddress, buffer);
		}
	case RVSIP_ADDRESS_TEL_URI_FIELD_POST_DIAL:
		{
			return RvSipAddrTelUriSetPostDial(hAddress, buffer);
		}
	case RVSIP_ADDRESS_TEL_URI_FIELD_CONTEXT:
		{
			return RvSipAddrTelUriSetContext(hAddress, buffer);
		}
	case RVSIP_ADDRESS_TEL_URI_FIELD_EXTENSION:
		{
			return RvSipAddrTelUriSetExtension(hAddress, buffer);
		}
	case RVSIP_ADDRESS_TEL_URI_FIELD_OTHER_PARAMS:
		{
			return RvSipAddrTelUriSetOtherParams(hAddress, buffer);
		}
	/*Abs URI related fields */
	case RVSIP_ADDRESS_ABS_URI_FIELD_SCHEME:
		{
			return RvSipAddrAbsUriSetScheme(hAddress, buffer);
		}
	case RVSIP_ADDRESS_ABS_URI_FIELD_IDENTIFIER:
		{
			return RvSipAddrAbsUriSetIdentifier(hAddress, buffer);
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_PORT:
	case RVSIP_ADDRESS_SIP_URL_FIELD_TTL:
	case RVSIP_ADDRESS_SIP_URL_FIELD_LR:
	case RVSIP_ADDRESS_SIP_URL_FIELD_IS_SECURE:
    case RVSIP_ADDRESS_TEL_URI_FIELD_IS_GLOBAL_PHONE_NUM:
	case RVSIP_ADDRESS_TEL_URI_FIELD_ENUMDI:
	default:
		{
			RvLogError(((MsgAddress*)hAddress)->pMsgMgr->pLogSrc, (((MsgAddress*)hAddress)->pMsgMgr->pLogSrc,
				"AddressSetStringField - field %d does not indicate a string field of the address object",
				eField));	
			return RV_ERROR_BADPARAM;
		}
	}
}

/***************************************************************************
 * AddressGetStringFieldLength
 * ------------------------------------------------------------------------
 * General: This function is used to get the length of any string field of 
 *          an address object. Use this function to evaluate the appropriate 
 *          buffer size to allocate before calling the GetStringField function.
 *          For example: to get the length of the host field of a SIP URL, 
 *          call this function with eField RVSIP_ADDRESS_SIP_URL_FIELD_HOST. 
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAddress  - A handle to the address object.
 *          eField    - The enumeration value that indicates the field being get.
 * Output:  pLength   - The length of the requested string including the null
 *                      terminator character.
 ***************************************************************************/
RvStatus RVCALLCONV AddressGetStringFieldLength(
                                          IN  RvSipAddressHandle           hAddress,
										  IN  RvSipAddressFieldName        eField,
                                          OUT RvInt32*                     pLength)
{
	RvSipAddressStringName eStringName;
	RvStatus               rv;

	rv = ConvertFieldTypeToStringType(eField, &eStringName);
	if (RV_OK != rv)
	{
		return rv;
	}

	switch (eField) 
	{
	/*handle Sip URL related fields */
	case RVSIP_ADDRESS_FIELD_DISPLAY_NAME:
	case RVSIP_ADDRESS_SIP_URL_FIELD_USER:
	case RVSIP_ADDRESS_SIP_URL_FIELD_HOST:
	case RVSIP_ADDRESS_SIP_URL_FIELD_MADDR:
	case RVSIP_ADDRESS_SIP_URL_FIELD_OTHER_PARAMS:
	case RVSIP_ADDRESS_SIP_URL_FIELD_HEADERS:
		{
			/* these are the string fields */
			*pLength = (RvInt32)RvSipAddrGetStringLength(hAddress, eStringName);
			break;
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_METHOD:
		{
			/* enumeration field */
			RvSipMethodType eMethodType = RvSipAddrUrlGetMethod(hAddress);
			if (eMethodType != RVSIP_METHOD_OTHER &&
				eMethodType != RVSIP_METHOD_UNDEFINED)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = (RvInt32)RvSipAddrGetStringLength(hAddress, eStringName);
			}
			break;
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_TRANSPORT:
		{
			/* enumeration field */
			RvSipTransport eTransport = RvSipAddrUrlGetTransport(hAddress);
			if (eTransport != RVSIP_TRANSPORT_OTHER &&
				eTransport != RVSIP_TRANSPORT_UNDEFINED)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = (RvInt32)RvSipAddrGetStringLength(hAddress, eStringName);
			}
			break;
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_USER_PARAM:
		{
			/* enumeration field */
			RvSipUserParam eUserParam = RvSipAddrUrlGetUserParam(hAddress);
			if (eUserParam != RVSIP_USERPARAM_OTHER &&
				eUserParam != RVSIP_USERPARAM_UNDEFINED)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = (RvInt32)RvSipAddrGetStringLength(hAddress, eStringName);
			}
			break;
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_COMP:
		{
			/* enumeration field */
			RvSipCompType eCompType = RvSipAddrUrlGetCompParam(hAddress);
			if (eCompType != RVSIP_COMP_OTHER &&
				eCompType != RVSIP_COMP_UNDEFINED)
			{
				/* We use a default length for enumeration values */
				*pLength = MAX_LENGTH_OF_ENUMERATION_STRING;
			}
			else
			{
				*pLength = (RvInt32)RvSipAddrGetStringLength(hAddress, eStringName);
			}
			break;
		}
	/*handle tel URI related fields */
	case RVSIP_ADDRESS_TEL_URI_FIELD_PHONE_NUM:
	case RVSIP_ADDRESS_TEL_URI_FIELD_ISDN_SUB_ADDR:
	case RVSIP_ADDRESS_TEL_URI_FIELD_POST_DIAL:
	case RVSIP_ADDRESS_TEL_URI_FIELD_EXTENSION:
	case RVSIP_ADDRESS_TEL_URI_FIELD_CONTEXT:
	case RVSIP_ADDRESS_TEL_URI_FIELD_OTHER_PARAMS:
		{
			/* these are the string fields */
			*pLength = (RvInt32)RvSipAddrGetStringLength(hAddress, eStringName);
			break;
		}
	/*handle abs URI related fields */
	case RVSIP_ADDRESS_ABS_URI_FIELD_SCHEME:
	case RVSIP_ADDRESS_ABS_URI_FIELD_IDENTIFIER:
		{
			/* these are the string fields */
			*pLength = (RvInt32)RvSipAddrGetStringLength(hAddress, eStringName);
			break;
		}
		
	case RVSIP_ADDRESS_SIP_URL_FIELD_PORT:
	case RVSIP_ADDRESS_SIP_URL_FIELD_TTL:
	case RVSIP_ADDRESS_SIP_URL_FIELD_LR:
	case RVSIP_ADDRESS_SIP_URL_FIELD_IS_SECURE:
	case RVSIP_ADDRESS_TEL_URI_FIELD_IS_GLOBAL_PHONE_NUM:
	case RVSIP_ADDRESS_TEL_URI_FIELD_ENUMDI:  		
	default:
		{
			RvLogError(((MsgAddress*)hAddress)->pMsgMgr->pLogSrc, (((MsgAddress*)hAddress)->pMsgMgr->pLogSrc,
				"AddressGetStringFieldLength - field %d does not indicate a string field of the address object",
				eField));	
			return RV_ERROR_BADPARAM;
		}
	}

	return RV_OK;
}

/***************************************************************************
 * AddressGetStringField
 * ------------------------------------------------------------------------
 * General: This function is used to get any string field of an address object.
 *          For example: to get the host of a SIP URL, call this function with
 *          eField RVSIP_ADDRESS_SIP_URL_FIELD_HOST and an adequate buffer.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAddress  - A handle to the address object.
 *          eField    - The enumeration value that indicates the field being get.
 *          buffer    - Buffer to fill with the requested parameter.
 *          bufferLen - The length of the buffer.
 * Output:  actualLen - The length of the requested parameter, + 1 to include a
 *                      NULL value at the end.
 ***************************************************************************/
RvStatus RVCALLCONV AddressGetStringField(
                                          IN  RvSipAddressHandle           hAddress,
										  IN  RvSipAddressFieldName        eField,
                                          IN  RvChar*                      buffer,
                                          IN  RvInt32                      bufferLen,
                                          OUT RvInt32*                     actualLen)
{
	
	switch (eField) 
	{
	case RVSIP_ADDRESS_FIELD_DISPLAY_NAME:
		{
			return RvSipAddrGetDisplayName(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_USER:
		{
			return RvSipAddrUrlGetUser(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_HOST:
		{
			return RvSipAddrUrlGetHost(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_METHOD:
		{
			RvSipMethodType eMethodType = RvSipAddrUrlGetMethod(hAddress);
			if (eMethodType == RVSIP_METHOD_OTHER)
			{
				/* If the enumeraions is OTHER - there is a string value for this field */
				return RvSipAddrUrlGetStrMethod(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (eMethodType != RVSIP_METHOD_UNDEFINED)
			{
				/* Convert the enumeration into string */
				RvChar *strNethodType = SipCommonConvertEnumToStrMethodType(eMethodType);
				return StringFieldCopy(strNethodType, buffer, bufferLen, actualLen);
			}
			else
			{
				/* This field was not set */
				*actualLen = 0;
				return RV_OK;
			}
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_TRANSPORT:
		{
			RvSipTransport eTransport = RvSipAddrUrlGetTransport(hAddress);
			if (eTransport == RVSIP_TRANSPORT_OTHER)
			{
				/* If the enumeraions is OTHER - there is a string value for this field */
				return RvSipAddrUrlGetStrTransport(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (eTransport != RVSIP_TRANSPORT_UNDEFINED)
			{
				/* Convert the enumeration into string */
				RvChar *strTransport = SipCommonConvertEnumToStrTransportType(eTransport);
				return StringFieldCopy(strTransport, buffer, bufferLen, actualLen);
			}
			else
			{
				/* This field was not set */
				*actualLen = 0;
				return RV_OK;
			}
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_MADDR:
		{
			return RvSipAddrUrlGetMaddrParam(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_USER_PARAM:
		{
			RvSipUserParam eUserParam = RvSipAddrUrlGetUserParam(hAddress);
			if (eUserParam == RVSIP_USERPARAM_OTHER)
			{
				/* If the enumeraions is OTHER - there is a string value for this field */
				return RvSipAddrUrlGetStrUserParam(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (eUserParam != RVSIP_USERPARAM_UNDEFINED)
			{
				/* Convert the enumeration into string */
				RvChar *strUserParam = SipCommonConvertEnumToStrUserParam(eUserParam);
				return StringFieldCopy(strUserParam, buffer, bufferLen, actualLen);
			}
			else
			{
				/* This field was not set */
				*actualLen = 0;
				return RV_OK;
			}
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_COMP:
		{
			RvSipCompType eCompType = RvSipAddrUrlGetCompParam(hAddress);
			if (eCompType == RVSIP_COMP_OTHER)
			{
				/* If the enumeraions is OTHER - there is a string value for this field */
				return RvSipAddrUrlGetStrCompParam(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
			}
			else if (eCompType != RVSIP_COMP_UNDEFINED)
			{
				/* Convert the enumeration into string */
				RvChar *strCompType = SipCommonConvertEnumToStrCompType(eCompType);
				return StringFieldCopy(strCompType, buffer, bufferLen, actualLen);
			}
			else
			{
				/* This field was not set */
				*actualLen = 0;
				return RV_OK;
			}
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_OTHER_PARAMS:
		{
			return RvSipAddrUrlGetOtherParams(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_HEADERS:
		{
			return RvSipAddrUrlGetHeaders(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ADDRESS_TEL_URI_FIELD_PHONE_NUM:
		{
			return RvSipAddrTelUriGetPhoneNumber(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ADDRESS_TEL_URI_FIELD_ISDN_SUB_ADDR:
		{
			return RvSipAddrTelUriGetIsdnSubAddr(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ADDRESS_TEL_URI_FIELD_POST_DIAL:
		{
			return RvSipAddrTelUriGetPostDial(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}		
	case RVSIP_ADDRESS_TEL_URI_FIELD_CONTEXT:
		{
			return RvSipAddrTelUriGetContext(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}		
	case RVSIP_ADDRESS_TEL_URI_FIELD_EXTENSION:
		{
			return RvSipAddrTelUriGetExtension(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}		
	case RVSIP_ADDRESS_TEL_URI_FIELD_OTHER_PARAMS:
		{
			return RvSipAddrTelUriGetOtherParams(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}		
	case RVSIP_ADDRESS_ABS_URI_FIELD_SCHEME:
		{
			return RvSipAddrAbsUriGetScheme(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}
	case RVSIP_ADDRESS_ABS_URI_FIELD_IDENTIFIER:
		{
			return RvSipAddrAbsUriGetIdentifier(hAddress, buffer, (RvUint)bufferLen, (RvUint*)actualLen);
		}

	case RVSIP_ADDRESS_SIP_URL_FIELD_PORT:
	case RVSIP_ADDRESS_SIP_URL_FIELD_TTL:
	case RVSIP_ADDRESS_SIP_URL_FIELD_LR:
	case RVSIP_ADDRESS_SIP_URL_FIELD_IS_SECURE:
	case RVSIP_ADDRESS_TEL_URI_FIELD_IS_GLOBAL_PHONE_NUM:
	case RVSIP_ADDRESS_TEL_URI_FIELD_ENUMDI:
	default:
		{
			RvLogError(((MsgAddress*)hAddress)->pMsgMgr->pLogSrc, (((MsgAddress*)hAddress)->pMsgMgr->pLogSrc,
				"AddressGetStringField - field %d does not indicate a string field of the address object",
				eField));	
			return RV_ERROR_BADPARAM;
		}
	}
}

/***************************************************************************
 * AddressSetIntField
 * ------------------------------------------------------------------------
 * General: This function is used to set an integer field of an address object.
 *          For example: to set the port of a SIP URL, call this function with
 *          eField RVSIP_ADDRESS_SIP_URL_FIELD_PORT and the port value
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAddress   - A handle to the address object.
 *          eField     - The enumeration value that indicates the field being set.
 *          fieldValue - The Int value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV AddressSetIntField(IN  RvSipAddressHandle           hAddress,
									   IN  RvSipAddressFieldName        eField,
                                       IN  RvInt32                      fieldValue)
{
	
	switch (eField) 
	{
	case RVSIP_ADDRESS_SIP_URL_FIELD_PORT:
		{
			return RvSipAddrUrlSetPortNum(hAddress, fieldValue);
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_TTL:
		{
			return RvSipAddrUrlSetTtlNum(hAddress, (RvInt16)fieldValue);
		}
	case RVSIP_ADDRESS_FIELD_DISPLAY_NAME:
	case RVSIP_ADDRESS_SIP_URL_FIELD_USER:
	case RVSIP_ADDRESS_SIP_URL_FIELD_HOST:
	case RVSIP_ADDRESS_SIP_URL_FIELD_METHOD:
	case RVSIP_ADDRESS_SIP_URL_FIELD_TRANSPORT:
	case RVSIP_ADDRESS_SIP_URL_FIELD_MADDR:
	case RVSIP_ADDRESS_SIP_URL_FIELD_USER_PARAM:
	case RVSIP_ADDRESS_SIP_URL_FIELD_COMP:
	case RVSIP_ADDRESS_SIP_URL_FIELD_OTHER_PARAMS:
	case RVSIP_ADDRESS_SIP_URL_FIELD_HEADERS:
	case RVSIP_ADDRESS_SIP_URL_FIELD_LR:
	case RVSIP_ADDRESS_SIP_URL_FIELD_IS_SECURE:
    case RVSIP_ADDRESS_ABS_URI_FIELD_SCHEME:
	case RVSIP_ADDRESS_ABS_URI_FIELD_IDENTIFIER:
	case RVSIP_ADDRESS_TEL_URI_FIELD_IS_GLOBAL_PHONE_NUM:
	case RVSIP_ADDRESS_TEL_URI_FIELD_PHONE_NUM:
	case RVSIP_ADDRESS_TEL_URI_FIELD_EXTENSION:
	case RVSIP_ADDRESS_TEL_URI_FIELD_ISDN_SUB_ADDR:
	case RVSIP_ADDRESS_TEL_URI_FIELD_POST_DIAL:
	case RVSIP_ADDRESS_TEL_URI_FIELD_CONTEXT:
	case RVSIP_ADDRESS_TEL_URI_FIELD_ENUMDI:
	case RVSIP_ADDRESS_TEL_URI_FIELD_OTHER_PARAMS:
	default:
		{
			RvLogError(((MsgAddress*)hAddress)->pMsgMgr->pLogSrc, (((MsgAddress*)hAddress)->pMsgMgr->pLogSrc,
				"AddressSetIntField - field %d does not indicate an integer field of the address object",
				eField));	
			return RV_ERROR_BADPARAM;
		}
	}
}

/***************************************************************************
 * AddressGetIntField
 * ------------------------------------------------------------------------
 * General: This function is used to get an integer field of an address object.
 *          For example: to get the port of a SIP URL, call this function with
 *          eField RVSIP_ADDRESS_SIP_URL_FIELD_PORT.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAddress    - A handle to the address object.
 *          eField      - The enumeration value that indicates the field being get.          
 * Output:  pFieldValue - The Int value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV AddressGetIntField(
										IN  RvSipAddressHandle           hAddress,
										IN  RvSipAddressFieldName        eField,
										OUT RvInt32*                     pFieldValue)
{
	
	switch (eField) 
	{
	case RVSIP_ADDRESS_SIP_URL_FIELD_PORT:
		{
			*pFieldValue = RvSipAddrUrlGetPortNum(hAddress);
			break;
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_TTL:
		{
			*pFieldValue = (RvInt16)RvSipAddrUrlGetTtlNum(hAddress);
			break;
		}
	case RVSIP_ADDRESS_FIELD_DISPLAY_NAME:
	case RVSIP_ADDRESS_SIP_URL_FIELD_USER:
	case RVSIP_ADDRESS_SIP_URL_FIELD_HOST:
	case RVSIP_ADDRESS_SIP_URL_FIELD_METHOD:
	case RVSIP_ADDRESS_SIP_URL_FIELD_TRANSPORT:
	case RVSIP_ADDRESS_SIP_URL_FIELD_MADDR:
	case RVSIP_ADDRESS_SIP_URL_FIELD_USER_PARAM:
	case RVSIP_ADDRESS_SIP_URL_FIELD_COMP:
	case RVSIP_ADDRESS_SIP_URL_FIELD_OTHER_PARAMS:
	case RVSIP_ADDRESS_SIP_URL_FIELD_HEADERS:
	case RVSIP_ADDRESS_SIP_URL_FIELD_LR:
	case RVSIP_ADDRESS_SIP_URL_FIELD_IS_SECURE:
    case RVSIP_ADDRESS_ABS_URI_FIELD_SCHEME:
	case RVSIP_ADDRESS_ABS_URI_FIELD_IDENTIFIER:
	case RVSIP_ADDRESS_TEL_URI_FIELD_IS_GLOBAL_PHONE_NUM:
	case RVSIP_ADDRESS_TEL_URI_FIELD_PHONE_NUM:
	case RVSIP_ADDRESS_TEL_URI_FIELD_EXTENSION:
	case RVSIP_ADDRESS_TEL_URI_FIELD_ISDN_SUB_ADDR:
	case RVSIP_ADDRESS_TEL_URI_FIELD_POST_DIAL:
	case RVSIP_ADDRESS_TEL_URI_FIELD_CONTEXT:
	case RVSIP_ADDRESS_TEL_URI_FIELD_ENUMDI:
	case RVSIP_ADDRESS_TEL_URI_FIELD_OTHER_PARAMS:
	default:
		{
			RvLogError(((MsgAddress*)hAddress)->pMsgMgr->pLogSrc, (((MsgAddress*)hAddress)->pMsgMgr->pLogSrc,
				"AddressGetIntField - field %d does not indicate an integer field of the address object",
				eField));	
			return RV_ERROR_BADPARAM;
		}
	}

	return RV_OK;
}

/***************************************************************************
 * AddressSetBoolField
 * ------------------------------------------------------------------------
 * General: This function is used to set a boolean field of an address object.
 *          For example: to set the Is-Secure indication of a SIP URL, call 
 *          this function with eField RVSIP_ADDRESS_SIP_URL_FIELD_IS_SECURE.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAddress    - A handle to the address object.
 *          eField      - The enumeration value that indicates the field being set.          
 *          fieldValue  - The Bool value to be set.
 ***************************************************************************/
RvStatus RVCALLCONV AddressSetBoolField(
									IN  RvSipAddressHandle           hAddress,
									IN  RvSipAddressFieldName        eField,
									IN  RvBool                       fieldValue)
{
	
	switch (eField) 
	{
	case RVSIP_ADDRESS_SIP_URL_FIELD_IS_SECURE:
		{
			return RvSipAddrUrlSetIsSecure(hAddress, fieldValue);
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_LR:
		{
			return RvSipAddrUrlSetLrParam(hAddress, fieldValue);
		}
	case RVSIP_ADDRESS_TEL_URI_FIELD_ENUMDI:
		{	
			/* First convert boolean into enumeration and then set */
			RvSipTelUriEnumdiType eEnumdiType = SipCommonConvertBoolToEnumEnumdi(fieldValue);
			return RvSipAddrTelUriSetEnumdiParamType(hAddress, eEnumdiType);
		}
	case RVSIP_ADDRESS_TEL_URI_FIELD_IS_GLOBAL_PHONE_NUM:
		{		   	
			return RvSipAddrTelUriSetIsGlobalPhoneNumber(hAddress, fieldValue);
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_PORT:
	case RVSIP_ADDRESS_SIP_URL_FIELD_TTL:
	case RVSIP_ADDRESS_FIELD_DISPLAY_NAME:
	case RVSIP_ADDRESS_SIP_URL_FIELD_USER:
	case RVSIP_ADDRESS_SIP_URL_FIELD_HOST:
	case RVSIP_ADDRESS_SIP_URL_FIELD_METHOD:
	case RVSIP_ADDRESS_SIP_URL_FIELD_TRANSPORT:
	case RVSIP_ADDRESS_SIP_URL_FIELD_MADDR:
	case RVSIP_ADDRESS_SIP_URL_FIELD_USER_PARAM:
	case RVSIP_ADDRESS_SIP_URL_FIELD_COMP:
	case RVSIP_ADDRESS_SIP_URL_FIELD_OTHER_PARAMS:
	case RVSIP_ADDRESS_SIP_URL_FIELD_HEADERS:
    case RVSIP_ADDRESS_ABS_URI_FIELD_SCHEME:
	case RVSIP_ADDRESS_ABS_URI_FIELD_IDENTIFIER:
	case RVSIP_ADDRESS_TEL_URI_FIELD_PHONE_NUM:
	case RVSIP_ADDRESS_TEL_URI_FIELD_EXTENSION:
	case RVSIP_ADDRESS_TEL_URI_FIELD_ISDN_SUB_ADDR:
	case RVSIP_ADDRESS_TEL_URI_FIELD_POST_DIAL:
	case RVSIP_ADDRESS_TEL_URI_FIELD_CONTEXT:
	case RVSIP_ADDRESS_TEL_URI_FIELD_OTHER_PARAMS:
	default:
		{
			RvLogError(((MsgAddress*)hAddress)->pMsgMgr->pLogSrc, (((MsgAddress*)hAddress)->pMsgMgr->pLogSrc,
				"AddressSetBoolField - field %d does not indicate a boolean field of the address object",
				eField));	
			return RV_ERROR_BADPARAM;
		}
	}
}

/***************************************************************************
 * AddressGetBoolField
 * ------------------------------------------------------------------------
 * General: This function is used to get a boolean field of an address object.
 *          For example: to get the Is-Secure indication of a SIP URL, call 
 *          this function with eField RVSIP_ADDRESS_SIP_URL_FIELD_IS_SECURE.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hAddress    - A handle to the address object.
 *          eField      - The enumeration value that indicates the field being set.          
 * Output:  pFieldValue - The Bool value retrieved.
 ***************************************************************************/
RvStatus RVCALLCONV AddressGetBoolField(
										IN  RvSipAddressHandle           hAddress,
										IN  RvSipAddressFieldName        eField,
										OUT RvBool*                      pFieldValue)
{
	
	switch (eField) 
	{
	case RVSIP_ADDRESS_SIP_URL_FIELD_IS_SECURE:
		{
			*pFieldValue = RvSipAddrUrlIsSecure(hAddress);
			break;
		}
	case RVSIP_ADDRESS_SIP_URL_FIELD_LR:
		{
			*pFieldValue = RvSipAddrUrlGetLrParam(hAddress);
			break;
		}
	case RVSIP_ADDRESS_TEL_URI_FIELD_IS_GLOBAL_PHONE_NUM:
		{
			*pFieldValue = RvSipAddrTelUriGetIsGlobalPhoneNumber(hAddress);
			break;
		}
	case RVSIP_ADDRESS_TEL_URI_FIELD_ENUMDI:
		{
			RvSipTelUriEnumdiType eEnumdiType = RvSipAddrTelUriGetEnumdiParamType(hAddress);
			/* Convert the returned enumeration into boolean */
			*pFieldValue = SipCommonConvertEnumToBoolEnumdi(eEnumdiType);
			break;
		}
		
	case RVSIP_ADDRESS_SIP_URL_FIELD_PORT:
	case RVSIP_ADDRESS_SIP_URL_FIELD_TTL:
	case RVSIP_ADDRESS_FIELD_DISPLAY_NAME:
	case RVSIP_ADDRESS_SIP_URL_FIELD_USER:
	case RVSIP_ADDRESS_SIP_URL_FIELD_HOST:
	case RVSIP_ADDRESS_SIP_URL_FIELD_METHOD:
	case RVSIP_ADDRESS_SIP_URL_FIELD_TRANSPORT:
	case RVSIP_ADDRESS_SIP_URL_FIELD_MADDR:
	case RVSIP_ADDRESS_SIP_URL_FIELD_USER_PARAM:
	case RVSIP_ADDRESS_SIP_URL_FIELD_COMP:
	case RVSIP_ADDRESS_SIP_URL_FIELD_OTHER_PARAMS:
	case RVSIP_ADDRESS_SIP_URL_FIELD_HEADERS:
    case RVSIP_ADDRESS_ABS_URI_FIELD_SCHEME:
	case RVSIP_ADDRESS_ABS_URI_FIELD_IDENTIFIER:
	case RVSIP_ADDRESS_TEL_URI_FIELD_PHONE_NUM:
	case RVSIP_ADDRESS_TEL_URI_FIELD_EXTENSION:
	case RVSIP_ADDRESS_TEL_URI_FIELD_ISDN_SUB_ADDR:
	case RVSIP_ADDRESS_TEL_URI_FIELD_POST_DIAL:
	case RVSIP_ADDRESS_TEL_URI_FIELD_CONTEXT:
	case RVSIP_ADDRESS_TEL_URI_FIELD_OTHER_PARAMS:
	default:
		{
			RvLogError(((MsgAddress*)hAddress)->pMsgMgr->pLogSrc, (((MsgAddress*)hAddress)->pMsgMgr->pLogSrc,
				"AddressGetBoolField - field %d does not indicate a boolean field of the address object",
				eField));	
			return RV_ERROR_BADPARAM;
		}
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
                                    IN  RvSipAddressFieldName        eFieldType,
									OUT RvSipAddressStringName      *peStringName)
{			
	switch(eFieldType) {
		case RVSIP_ADDRESS_FIELD_DISPLAY_NAME:
		{
			*peStringName = RVSIP_ADDRESS_DISPLAY_NAME;
			break;
		}
		/*handle Sip URI related fields*/
		case RVSIP_ADDRESS_SIP_URL_FIELD_USER:
		{		
			*peStringName = RVSIP_ADDRESS_USER;
			break;
		}
		case RVSIP_ADDRESS_SIP_URL_FIELD_HOST:
		{			
			*peStringName = RVSIP_ADDRESS_HOST;
			break;
		}
		case RVSIP_ADDRESS_SIP_URL_FIELD_TRANSPORT:
		{		
			*peStringName = RVSIP_ADDRESS_TRANSPORT;
			break;
		}
		case RVSIP_ADDRESS_SIP_URL_FIELD_MADDR:
		{		
			*peStringName = RVSIP_ADDRESS_MADDR_PARAM;
			break;
		}
		case RVSIP_ADDRESS_SIP_URL_FIELD_USER_PARAM:
		{		
			*peStringName = RVSIP_ADDRESS_USER_PARAM;
			break;
		}
		case RVSIP_ADDRESS_SIP_URL_FIELD_COMP:
		{		
			*peStringName = RVSIP_ADDRESS_COMP_PARAM;
			break;
		}
		case RVSIP_ADDRESS_SIP_URL_FIELD_OTHER_PARAMS:
		{		
			*peStringName = RVSIP_ADDRESS_URL_OTHER_PARAMS;
			break;
		}
		case RVSIP_ADDRESS_SIP_URL_FIELD_HEADERS:
		{		
			*peStringName = RVSIP_ADDRESS_HEADERS;
			break;
		}
		case RVSIP_ADDRESS_SIP_URL_FIELD_METHOD:
		{		
			*peStringName = RVSIP_ADDRESS_METHOD;
			break;
		}
		/*handle tel URI related fields*/
		case RVSIP_ADDRESS_TEL_URI_FIELD_PHONE_NUM:
		{		
			*peStringName = RVSIP_ADDRESS_TEL_URI_NUMBER;
			break;
		}
		case RVSIP_ADDRESS_TEL_URI_FIELD_ISDN_SUB_ADDR:
		{
			*peStringName = RVSIP_ADDRESS_TEL_URI_ISDN_SUBS;
			break;
		}
		case RVSIP_ADDRESS_TEL_URI_FIELD_POST_DIAL:
		{
			*peStringName = RVSIP_ADDRESS_TEL_URI_POST_DIAL;
			break;
		}
		case RVSIP_ADDRESS_TEL_URI_FIELD_OTHER_PARAMS:
		{
			*peStringName = RVSIP_ADDRESS_TEL_URI_OTHER_PARAMS;
			break;
		}
		case RVSIP_ADDRESS_TEL_URI_FIELD_EXTENSION:
		{
			*peStringName = RVSIP_ADDRESS_TEL_URI_EXTENSION;
			break;
		}
		case RVSIP_ADDRESS_TEL_URI_FIELD_CONTEXT:	
		{
			*peStringName = RVSIP_ADDRESS_TEL_URI_CONTEXT;
			break;
		}
		/*handle abs URI related fields*/
		case RVSIP_ADDRESS_ABS_URI_FIELD_SCHEME:
		{
			*peStringName = RVSIP_ADDRESS_ABS_URI_SCHEME;
			break;
		}
		case RVSIP_ADDRESS_ABS_URI_FIELD_IDENTIFIER:
		{
			*peStringName = RVSIP_ADDRESS_ABS_URI_IDENTIFIER;
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






















