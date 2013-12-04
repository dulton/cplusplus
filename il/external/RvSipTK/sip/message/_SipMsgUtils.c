/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        _SipMsgUtils.c                                      *
 *                                                                            *
 * The file defines the internal methods of the Message object.               *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Mickey			 Feb. 2006                                            *
 ******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "rpool_API.h"
#include "_SipMsgUtils.h"
#include "AdsRpool.h"
#include "rvstdio.h"

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/


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
								OUT RvInt32*	decPart)
{
	RvChar	    strFromPool[6];
	RvChar*  	str;
	RvInt32		size;

	*decPart = 0;
	*intPart = -1;

	if(strBuffer != NULL)
	{
		str  = strBuffer;
		size = (RvInt32)strlen(str);
	}
	else
	{
		if(hPool == NULL || hPage == NULL)
		{
			return RV_ERROR_INVALID_HANDLE;
		}

		if(offset <= UNDEFINED)
		{
			return RV_ERROR_ILLEGAL_ACTION;
		}
		
		size = RPOOL_Strlen(hPool, hPage, offset);

		if(size > 5)
		{
			return RV_ERROR_ILLEGAL_ACTION;
		}

		RPOOL_CopyToExternal(hPool, hPage, offset, strFromPool, size+1);

		str = strFromPool;
	}
	
	/*
	 * Calculate the integral and decimal part.
	 */
	if(size > 5 || size < 1)
	{
		return RV_ERROR_ILLEGAL_ACTION;
	}

	if(isdigit((RvInt)str[0]))
	{
		*intPart = str[0] - '0';
		if(size > 2)
		{
			if(isdigit((RvInt)str[2]))
			{
				*decPart  = 100*(str[2]-'0');
			}
			else 
			{
				return RV_ERROR_ILLEGAL_ACTION;
			}
		
			if(size > 3)
			{
				if(isdigit((RvInt)str[3]))
				{
					*decPart += 10*(str[3]-'0');
				}
				else 
				{
					return RV_ERROR_ILLEGAL_ACTION;
				}

				if(size > 4)
				{
					if(isdigit((RvInt)str[4]))
					{
						*decPart += str[4]-'0';
					}
					else 
					{
						return RV_ERROR_ILLEGAL_ACTION;
					}
				}
			}
		}
	}
	else
	{
		return RV_ERROR_ILLEGAL_ACTION;
	}
	
	return RV_OK;
}

