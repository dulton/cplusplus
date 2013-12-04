/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        RvSipAddressFieldDispatcher.c                       *
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
#include "RvSipAddressFieldDispatcher.h"
#include "AddressFieldDispatcher.h"
#include "MsgTypes.h"
#include "MsgUtils.h"


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
 * RvSipAddressDestruct
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
RVAPI void RVCALLCONV RvSipAddressDestruct(
                                          IN RvSipAddressHandle           hAddress)
{
	if (hAddress == NULL)
	{
		return;
	}
	
	RvLogInfo(((MsgAddress*)hAddress)->pMsgMgr->pLogSrc, (((MsgAddress*)hAddress)->pMsgMgr->pLogSrc,
		"RvSipAddressDestruct - Destructing address 0x%p",
		hAddress));	
	
	AddressDestruct(hAddress);
}

/***************************************************************************
 * RvSipAddrEncodeWithoutDisplayName
 * ------------------------------------------------------------------------
 * General: Encodes an Address object to a textual address object. The textual header is
 *          placed on a page taken from a specified pool. In order to copy the textual header
 *          from the page to a consecutive buffer, use RPOOL_CopyToExternal().
 *          Notice: This function does not encode the display name as part of
 *          the textual address. In order to include the display name, use 
 *          RvSipAddrEncode().
 * Return Value: Returns RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * input: hSipAddr - Handle to the Address object.
 *        hPool    - Handle to the specified memory pool.
 * output: hPage   - The memory page allocated to contain the encoded object.
 *         length  - The length of the encoded information.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipAddrEncodeWithoutDisplayName(
										   IN  RvSipAddressHandle hSipAddr,
                                           IN  HRPOOL             hPool,
                                           OUT HPAGE*             hPage,
                                           OUT RvUint32*          length)
{
	RvStatus stat;
    MsgAddress* pAddr ;

    pAddr = (MsgAddress*)hSipAddr;

    if(hPage == NULL || length == NULL)
        return RV_ERROR_NULLPTR;

    *hPage = NULL_PAGE;
    *length = 0;
    if ((hPool == NULL)||(hSipAddr == NULL))
        return RV_ERROR_INVALID_HANDLE;

    stat = RPOOL_GetPage(hPool, 0, hPage);
    if(stat != RV_OK)
    {
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "RvSipAddrEncodeWithoutDisplayName - Failed. RPOOL_GetPage failed, hPool is 0x%p, hSipAddr is 0x%p",
                hPool, hSipAddr));
        return stat;
    }
    else
    {
        RvLogDebug(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
                "RvSipAddrEncodeWithoutDisplayName - Got new page 0x%p on pool 0x%p. hSipAddr is 0x%p",
                *hPage, hPool, hSipAddr));
    }

    *length = 0; /* so length will give the real length, and will not just add the
                 new length to the old one */
	
    stat = EncodeAddrSpec(pAddr, hPool, *hPage, RV_FALSE, length);
	if(stat != RV_OK)
    {
        /* free the page */
        RPOOL_FreePage(hPool, *hPage);
        RvLogError(pAddr->pMsgMgr->pLogSrc,(pAddr->pMsgMgr->pLogSrc,
			"RvSipAddrEncodeWithoutDisplayName - Failed, free page 0x%p on pool 0x%p. AddrEncode fail",
			*hPage, hPool));
    }
    return stat;
}

/***************************************************************************
 * RvSipAddressSetStringField
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
RVAPI RvStatus RVCALLCONV RvSipAddressSetStringField(
                                          IN RvSipAddressHandle           hAddress,
										  IN RvSipAddressFieldName        eField,
                                          IN RvChar*                      buffer)
{
	RvStatus     rv;
	
    if (hAddress == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}

	rv = AddressSetStringField(hAddress, eField, buffer);
	if (RV_OK != rv)
	{
		RvLogError(((MsgAddress*)hAddress)->pMsgMgr->pLogSrc, (((MsgAddress*)hAddress)->pMsgMgr->pLogSrc,
				   "RvSipAddressSetStringField - Failed to set string field to address 0x%p",
				   hAddress));		
	}

	return rv;
}

/***************************************************************************
 * RvSipAddressGetStringFieldLength
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
RVAPI RvStatus RVCALLCONV RvSipAddressGetStringFieldLength(
                                          IN  RvSipAddressHandle           hAddress,
										  IN  RvSipAddressFieldName        eField,
                                          OUT RvInt32*                     pLength)
{
	RvStatus     rv;
	
    if (hAddress == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (pLength == NULL)
	{
		return RV_ERROR_BADPARAM;
	}
	
	rv = AddressGetStringFieldLength(hAddress, eField, pLength);
	if (RV_OK != rv)
	{
		RvLogError(((MsgAddress*)hAddress)->pMsgMgr->pLogSrc, (((MsgAddress*)hAddress)->pMsgMgr->pLogSrc,
			"RvSipAddressGetStringFieldLength - Failed to get string field length in address 0x%p",
			hAddress));		
	}
	
	return rv;
}

/***************************************************************************
 * RvSipAddressGetStringField
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
RVAPI RvStatus RVCALLCONV RvSipAddressGetStringField(
                                          IN  RvSipAddressHandle           hAddress,
										  IN  RvSipAddressFieldName        eField,
                                          IN  RvChar*                      buffer,
                                          IN  RvInt32                      bufferLen,
                                          OUT RvInt32*                     actualLen)
{
	RvStatus     rv;
	
	if (hAddress == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (buffer == NULL || actualLen == NULL)
	{
		return RV_ERROR_BADPARAM;
	}

	rv = AddressGetStringField(hAddress, eField, buffer, bufferLen, actualLen);
	if (RV_OK != rv)
	{
		RvLogError(((MsgAddress*)hAddress)->pMsgMgr->pLogSrc, (((MsgAddress*)hAddress)->pMsgMgr->pLogSrc,
					"RvSipAddressGetStringField - Failed to get string field of address 0x%p",
					hAddress));		
	}

	return rv;
}

/***************************************************************************
 * RvSipAddressSetIntField
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
RVAPI RvStatus RVCALLCONV RvSipAddressSetIntField(
                                          IN  RvSipAddressHandle           hAddress,
										  IN  RvSipAddressFieldName        eField,
                                          IN  RvInt32                      fieldValue)
{
	RvStatus     rv;
		
    if (hAddress == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
		
	rv = AddressSetIntField(hAddress, eField, fieldValue);
	if (RV_OK != rv)
	{
		RvLogError(((MsgAddress*)hAddress)->pMsgMgr->pLogSrc, (((MsgAddress*)hAddress)->pMsgMgr->pLogSrc,
			"RvSipAddressSetIntField - Failed to set integer field to address 0x%p",
			hAddress));		
	}
	
	return rv;
}

/***************************************************************************
 * RvSipAddressGetIntField
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
RVAPI RvStatus RVCALLCONV RvSipAddressGetIntField(
										IN  RvSipAddressHandle           hAddress,
										IN  RvSipAddressFieldName        eField,
										OUT RvInt32*                     pFieldValue)
{
	RvStatus     rv;
	
	if (hAddress == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	if (pFieldValue == NULL)
	{
		return RV_ERROR_BADPARAM;
	}
	
	rv = AddressGetIntField(hAddress, eField, pFieldValue);
	if (RV_OK != rv)
	{
		RvLogError(((MsgAddress*)hAddress)->pMsgMgr->pLogSrc, (((MsgAddress*)hAddress)->pMsgMgr->pLogSrc,
					"RvSipAddressGetIntField - Failed to get integer field of address 0x%p",
					hAddress));		
	}
	
	return rv;
}

/***************************************************************************
 * RvSipAddressSetBoolField
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
RVAPI RvStatus RVCALLCONV RvSipAddressSetBoolField(
									IN  RvSipAddressHandle           hAddress,
									IN  RvSipAddressFieldName        eField,
									IN  RvBool                       fieldValue)
{
	RvStatus     rv;
	
    if (hAddress == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	rv = AddressSetBoolField(hAddress, eField, fieldValue);
	if (RV_OK != rv)
	{
		RvLogError(((MsgAddress*)hAddress)->pMsgMgr->pLogSrc, (((MsgAddress*)hAddress)->pMsgMgr->pLogSrc,
					"RvSipAddressSetBoolField - Failed to set boolean field to address 0x%p",
					hAddress));		
	}
	
	return rv;
}

/***************************************************************************
 * RvSipAddressGetBoolField
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
RVAPI RvStatus RVCALLCONV RvSipAddressGetBoolField(
										IN  RvSipAddressHandle           hAddress,
										IN  RvSipAddressFieldName        eField,
										OUT RvBool*                      pFieldValue)
{
	RvStatus     rv;
	
    if (hAddress == NULL)
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	rv = AddressGetBoolField(hAddress, eField, pFieldValue);
	if (RV_OK != rv)
	{
		RvLogError(((MsgAddress*)hAddress)->pMsgMgr->pLogSrc, (((MsgAddress*)hAddress)->pMsgMgr->pLogSrc,
					"RvSipAddressGetBoolField - Failed to get boolean field of address 0x%p",
					hAddress));		
	}
	
	return rv;
}


#endif /* #ifdef RV_SIP_JSR32_SUPPORT */

#ifdef __cplusplus
}
#endif






















