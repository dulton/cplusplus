/*
*********************************************************************************
*                                                                               *
* NOTICE:                                                                       *
* This document contains information that is confidential and proprietary to    *
* RADVision LTD.. No part of this publication may be reproduced in any form     *
* whatsoever without written prior approval by RADVision LTD..                  *
*                                                                               *
* RADVision LTD. reserves the right to revise this publication and make changes *
* without obligation to notify any person of such revisions or changes.         *
*********************************************************************************
*/


/*********************************************************************************
 *                              <_SipCommonUtils.c>
 *
 *  The file holds utils functions to be used in all stack layers.
 *    Author                         Date
 *    ------                        ------
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include <stdio.h>
#include "RV_SIP_DEF.h"
#include "rvstdio.h"
#include "_SipCommonUtils.h"
#include "_SipCommonCore.h"
#include "_SipCommonConversions.h"
#include "rvlog.h"
#include "rvstrutils.h"
#include "RvSipRetryAfterHeader.h"
#ifdef RV_SIGCOMP_ON
#include "RvSipAddress.h"
#include "RvSipViaHeader.h"
#include "_SipAddress.h"
#include "_SipViaHeader.h"
#endif 

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

#define SIP_COMMON_URN_PREFIX       ("urn:")
#define SIP_COMMON_URN_PREFIX_LEN   (strlen(SIP_COMMON_URN_PREFIX))

/***************************************************************************
 * SipCommonStrcmpUrn
 * ------------------------------------------------------------------------
 * General: The function does comparison between 2 sigcomp-id values.
 * Return Value: RV_TRUE - if equal, RV_FALSE - if not equal.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:	firstRpool - properties of the sigcomp-id first string inside a page
 *			firstLen  - the length of the first singcomp-id string
 *			bIsFirstNormalized - boolean indicating if the first sigcomp-id string
 *								is already case-normalized (to lowercase)
 *			bIsFirstConsec     - bollean indicating if the first sigcomp-id string
 *								is parsed over a single page or more
 *			secRpool  - properties of the sigcomp-id first string inside a page
 *			secLen    - the length of the second singcomp-id string
 *			bIsSecNormalized - boolean indicating if the second sigcomp-id string
 *								is already case-normalized (to lowercase)
 *			bIsSecConsec     - boolean indicating if the second sigcomp-id string
 *								is parsed over a single page or more
 ***************************************************************************/
RvBool RVCALLCONV SipCommonStrcmpUrn (
									  IN RPOOL_Ptr *firstRpool,
									  IN RvUint    firstLen,
									  IN RvBool    bIsFirstNormalized,
									  IN RvBool    bIsFirstConsec,
									  IN RPOOL_Ptr *secRpool,
									  IN RvUint    secLen,
									  IN RvBool    bIsSecNormalized,
									  IN RvBool    bIsSecConsec)
{
	HPAGE  firstDestPage = NULL;
	HPAGE  secDestPage = NULL;
	RvBool bIsFirstUrl;
	RvBool bShouldFirstNormalized = RV_FALSE;
	RvBool bShouldSecNormalized = RV_FALSE;
	RvBool bIsSecUrl;
	RvInt32 firstDestOffset;
	RvInt32 secDestOffset;
	RvChar *firstString;
	RvChar *secString;
	RvInt   res;
	HRPOOL  firstPool = firstRpool->hPool;
	HRPOOL  secPool   = secRpool->hPool;
	HPAGE   firstPage = firstRpool->hPage;
	HPAGE   secPage   = secRpool->hPage;
	RvInt32 firstOffset = firstRpool->offset;
	RvInt32 secOffset   = secRpool->offset;

	/* 
	URN comes in the following format: URN:<NID>:<NSS> where:
	URN is a mandatory prefix;
	NID is the namespace identifier;
	NSS is the namespace specific string.

	Comparison should be done, based on RFC 2141, according to the following guidelines:

	1. normalize the case of the leading "urn:" token
	2. normalize the case of the NID
	3. normalize the case of any %-escaping
	4. compare octet-by-octet
	*/

	if ((firstOffset == UNDEFINED) && (secOffset == UNDEFINED))
		return (RV_TRUE);
	else if ((firstOffset != UNDEFINED) && (secOffset == UNDEFINED))
		return (RV_FALSE);
	else if ((firstOffset == UNDEFINED) && (secOffset != UNDEFINED))
		return (RV_FALSE);

	/* ========= check if the first sigcomp-id value should be copied and normalized ============= */
	if (bIsFirstNormalized == RV_FALSE)
	{
		/* 
			either if the page is consecutive or not, we need to copy the URL to another page so we can
			normalize it.
		*/
		if (RPOOL_GetPage(firstPool, firstLen, &firstDestPage) != RV_OK)
		{
			return (RV_FALSE);
		}
		if (RPOOL_Append(firstPool, firstDestPage, firstLen, RV_TRUE, &firstDestOffset) != RV_OK)
		{
			RPOOL_FreePage(firstPool, firstDestPage);
			return (RV_FALSE);
		}
		
		if (RPOOL_CopyFrom(firstPool, firstDestPage, firstDestOffset, firstPool, firstPage,
			firstOffset, firstLen) != RV_OK)
		{
			RPOOL_FreePage(firstPool, firstDestPage);
			return (RV_FALSE);
		}
		firstString = RPOOL_GetPtr(firstPool, firstDestPage, firstDestOffset);
		if (NULL == firstString)
		{
			RPOOL_FreePage(firstPool, firstDestPage);
			return (RV_FALSE);
		}
		
		bShouldFirstNormalized = RV_TRUE;
	}
	else
	{
		if (bIsFirstConsec != RV_FALSE)
		{
			/* just take the pointer to the string */
			firstString = RPOOL_GetPtr (firstPool, firstPage, firstOffset);
			if (NULL == firstString)
			{
				return (RV_FALSE);
			}
		}
		else
		{
			/* copy, but don't mark it to be normalized */
			if (RPOOL_GetPage(firstPool, firstLen, &firstDestPage) != RV_OK)
			{
				return (RV_FALSE);
			}
			if (RPOOL_Append(firstPool, firstDestPage, firstLen, RV_TRUE, &firstDestOffset) != RV_OK)
			{
				RPOOL_FreePage(firstPool, firstDestPage);
				return (RV_FALSE);
			}
			
			if (RPOOL_CopyFrom(firstPool, firstDestPage, firstDestOffset, firstPool, firstPage,
				firstOffset, firstLen) != RV_OK)
			{
				RPOOL_FreePage(firstPool, firstDestPage);
				return (RV_FALSE);
			}
			firstString = RPOOL_GetPtr(firstPool, firstDestPage, firstDestOffset);
			if (NULL == firstString)
			{
				RPOOL_FreePage(firstPool, firstDestPage);
				return (RV_FALSE);
			}
		} /* if bIsFirstConsec */
	} /* if bIsFirstNormalized */


	/* ========= check if the sec sigcomp-id value should be copied and normalized ============= */
	if (bIsSecNormalized == RV_FALSE)
	{
		/* 
			either if the page is consecutive or not, we need to copy the URL to another page so we can
			normalize it.
		*/
		if (RPOOL_GetPage(secPool, secLen, &secDestPage) != RV_OK)
		{
			return (RV_FALSE);
		}
		if (RPOOL_Append(secPool, secDestPage, secLen, RV_TRUE, &secDestOffset) != RV_OK)
		{
			RPOOL_FreePage(secPool, secDestPage);
			return (RV_FALSE);
		}
		
		if (RPOOL_CopyFrom(secPool, secDestPage, secDestOffset, secPool, secPage,
			secOffset, secLen) != RV_OK)
		{
			RPOOL_FreePage(secPool, secDestPage);
			return (RV_FALSE);
		}

		secString = RPOOL_GetPtr(secPool, secDestPage, secDestOffset);
		if (NULL == secString)
		{
			RPOOL_FreePage(secPool, secDestPage);
			return (RV_FALSE);
		}
		
		bShouldSecNormalized = RV_TRUE;
	}
	else
	{
		if (bIsSecConsec != RV_FALSE)
		{
			/* just take the pointer to the string */
			secString = RPOOL_GetPtr (secPool, secPage, secOffset);
			if (NULL == secString)
			{
				return (RV_FALSE);
			}
		}
		else
		{
			/* copy, but don't mark it to be normalized */
			if (RPOOL_GetPage(secPool, secLen, &secDestPage) != RV_OK)
			{
				return (RV_FALSE);
			}
			if (RPOOL_Append(secPool, secDestPage, secLen, RV_TRUE, &secDestOffset) != RV_OK)
			{
				RPOOL_FreePage(secPool, secDestPage);
				return (RV_FALSE);
			}
			
			if (RPOOL_CopyFrom(secPool, secDestPage, secDestOffset, secPool, secPage,
				secOffset, secLen) != RV_OK)
			{
				RPOOL_FreePage(secPool, secDestPage);
				return (RV_FALSE);
			}
			secString = RPOOL_GetPtr(secPool, secDestPage, secDestOffset);
			if (NULL == secString)
			{
				RPOOL_FreePage(secPool, secDestPage);
				return (RV_FALSE);
			}
		} /* if bIsSecConsec */
	} /* if bIsSecNormalized */


	/* normalize "URN" section */
	if (bShouldFirstNormalized != RV_FALSE)
		SipCommonStrToLower (firstString, (RvInt32)SIP_COMMON_URN_PREFIX_LEN);
	if (bShouldSecNormalized != RV_FALSE)
		SipCommonStrToLower (secString, (RvInt32)SIP_COMMON_URN_PREFIX_LEN);
	
	bIsFirstUrl = (strstr(firstString, SIP_COMMON_URN_PREFIX) != NULL);
	bIsSecUrl   = (strstr(secString, SIP_COMMON_URN_PREFIX) != NULL);

	if ((bIsFirstUrl != RV_FALSE) && (bIsSecUrl != RV_FALSE))
	{
		RvChar *firstNss;
		RvChar *secNss;
		RvUint firstNidLen;
		RvUint secNidLen;
		RvUint firstNssLen;
		RvUint secNssLen;
		RvUint nssLenToLow;
		RvUint firstIndex;
		RvUint secIndex;

		firstNss = strchr (firstString+SIP_COMMON_URN_PREFIX_LEN, ':');
		if (firstNss == NULL)
			return (strcmp (firstString, secString));

		secNss = strchr (secString+SIP_COMMON_URN_PREFIX_LEN, ':');
		if (secNss == NULL)
			return (strcmp (firstString, secString));

		firstNidLen = (RvUint)(firstNss - (firstString + SIP_COMMON_URN_PREFIX_LEN));
		secNidLen   = (RvUint)(secNss - (secString + SIP_COMMON_URN_PREFIX_LEN));

		if (bShouldFirstNormalized != RV_FALSE)
		{
			/* normalize NID section */
			SipCommonStrToLower(firstString+SIP_COMMON_URN_PREFIX_LEN, firstNidLen);
	
			firstNssLen = (RvUint)(firstLen - SIP_COMMON_URN_PREFIX_LEN - (firstNidLen + 1 /* for the ':' */));

			/* normalize escaped sections in the first string */
			nssLenToLow = firstNssLen - 2;
			while (nssLenToLow--)
			{
				firstIndex = (RvUint)(SIP_COMMON_URN_PREFIX_LEN+firstNidLen+1+nssLenToLow);
				if (firstString[firstIndex] == '%')
				{
					SipCommonStrToLower(&firstString[firstIndex+1], 2);
				}
			}
		}

		if (bShouldSecNormalized != RV_FALSE)
		{
			/* normalize NID section */
			SipCommonStrToLower(secString+SIP_COMMON_URN_PREFIX_LEN, secNidLen);

			secNssLen   = (RvUint)(secLen - SIP_COMMON_URN_PREFIX_LEN - (secNidLen + 1 /* for the ':' */));

			/* normalize escaped sections in the second string */
			nssLenToLow = secNssLen - 2; /* start 2 places from the end, and go backwards */
			while (nssLenToLow--)
			{
				secIndex = (RvUint)(SIP_COMMON_URN_PREFIX_LEN+secNidLen+1+nssLenToLow);
				if (secString[secIndex] == '%')
				{
					SipCommonStrToLower(&secString[secIndex+1], 2);
				}
			}
		}

		/* compare both normalized strings */
		res = strcmp (firstString, secString);
		if (firstDestPage != NULL)
		{
			/* we allocated a new page. Time to free it */
			RPOOL_FreePage(firstPool, firstDestPage);
		}
		if (secDestPage != NULL)
		{
			RPOOL_FreePage(secPool, secDestPage);
		}
	}
	else
	{
		res = strcmp (firstString, secString);
	}
	
	if (res == 0)
		return (RV_TRUE);
	else
		return (RV_FALSE);
}

/***************************************************************************
 * SipCommonStricmp
 * ------------------------------------------------------------------------
 * General: The function does insensitive comparison between 2 strings..
 * Return Value: RV_TRUE - if equal, RV_FALSE - if not equal.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: firstStr - The first string.
 *        secStr   - The second string
 ***************************************************************************/
RvBool RVCALLCONV SipCommonStricmp(
                        IN RvChar* firstStr,
                        IN RvChar* secStr)
{
    if(firstStr == NULL)
    {
        return (secStr == NULL)? RV_TRUE: RV_FALSE;
    }

    if(RvStrcasecmp(firstStr, secStr)==0)
    {
        return RV_TRUE;
    }
    else
    {
        return RV_FALSE;
    }
}

/***************************************************************************
 * SipCommonStricmpByLen
 * ------------------------------------------------------------------------
 * General: The function does insensitive comparison between 2 strings which
 *          are not null terminated
 * Return Value: RV_TRUE - if equal, RV_FALSE - if not equal.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: firstStr - The first string.
 *        secStr   - The second string
 ***************************************************************************/
RvBool RVCALLCONV SipCommonStricmpByLen(
                        IN RvChar* firstStr,
                        IN RvChar* secStr,
                        IN RvInt32 len)
{
    

    if(firstStr == NULL)
    {
        return (secStr == NULL)? RV_TRUE: RV_FALSE;
    }

    if(RvStrncasecmp(firstStr, secStr, len)==0)
    {
        return RV_TRUE;
    }
    else
    {
        return RV_FALSE;
    }
}

/***************************************************************************
 * SipCommonStrToLower
 * ------------------------------------------------------------------------
 * General: The function changes all characters in a given string, to lower case
 * Return Value: RV_TRUE - if equal, RV_FALSE - if not equal.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: str - The given string.
 *        length  - The string length
 ***************************************************************************/
void RVCALLCONV SipCommonStrToLower(IN RvChar *str, IN RvInt32 length)
{
    RvInt32 i;

    for(i = 0 ; i < length; i++)
    {
        if (isupper((RvInt)str[i]))
        {
            str[i] = (RvChar )tolower((int)str[i]);
        }
    }
}


#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/********************************************************************************************
 * SipCommonStatus2Str
 * purpose : Convert a status into a string
 * input   : status             - Status
 * output  : none
 * return  : Status string
 ********************************************************************************************/
RvChar* RVCALLCONV SipCommonStatus2Str(IN RvStatus status)
{
    switch (status)
    {
        case RV_OK:                             return (char *)"OK";
        case RV_ERROR_UNKNOWN:                  return (char *)"UNKNOWN";
        case RV_ERROR_BADPARAM:                 return (char *)"BADPARAM";
        case RV_ERROR_NULLPTR:                  return (char *)"NULLPTR";
        case RV_ERROR_OUTOFRANGE:               return (char *)"OUTOFRANGE";
        case RV_ERROR_DESTRUCTED:               return (char *)"DESTRUCTED";
        case RV_ERROR_NOTSUPPORTED:             return (char *)"NOTSUPPORTED";
        case RV_ERROR_UNINITIALIZED:            return (char *)"UNINITIALIZED";
        case RV_ERROR_TRY_AGAIN:                return (char *)"TRY_AGAIN";

        case RV_ERROR_ILLEGAL_ACTION:           return (char *)"ILLEGAL_ACTION";
        case RV_ERROR_NETWORK_PROBLEM:          return (char *)"NETWORK_PROBLEM";
        case RV_ERROR_INVALID_HANDLE:           return (char *)"INVALID_HANDLE";
        case RV_ERROR_NOT_FOUND:                return (char *)"NOT_FOUND";
        case RV_ERROR_INSUFFICIENT_BUFFER:      return (char *)"INSUFFICIENT_BUFFER";
        case RV_ERROR_ILLEGAL_SYNTAX:           return (char *)"ILLEGAL_SYNTAX";
        case RV_ERROR_OBJECT_ALREADY_EXISTS:    return (char *)"OBJECT_ALREADY_EXISTS";
        case RV_ERROR_COMPRESSION_FAILURE:      return (char *)"COMPRESSION_FAILURE";
        case RV_ERROR_NUM_OF_THREADS_DECREASED: return (char *)"NUM_OF_THREADS_DECREASED";
        case RV_ERROR_OUTOFRESOURCES:           return (char *)"OUTOFRESOURCES";
        default:                                return (char *)"-Unknown status-";
    }
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/********************************************************************************************
 * SipCommonObjectType2Str
 * purpose : Convert an object type into a string
 * input   : status             - Status
 * output  : none
 * return  : Status string
 ********************************************************************************************/
RvChar* RVCALLCONV SipCommonObjectType2Str(IN RvStatus status)
{
    switch (status)
    {
        case RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_LEG:           return (char *)"Call-Leg";
        case RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSACTION:        return (char *)"Transc";
        case RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_TRANSACTION:   return (char *)"Call-Transc";
        case RVSIP_COMMON_STACK_OBJECT_TYPE_SUBSCRIPTION:       return (char *)"Subs";
        case RVSIP_COMMON_STACK_OBJECT_TYPE_NOTIFICATION:       return (char *)"Notify";
        case RVSIP_COMMON_STACK_OBJECT_TYPE_REG_CLIENT:         return (char *)"Reg-Client";
        case RVSIP_COMMON_STACK_OBJECT_TYPE_CONNECTION:         return (char *)"Conn";
        case RVSIP_COMMON_STACK_OBJECT_TYPE_APP_OBJECT:         return (char *)"App-Obj";
        case RVSIP_COMMON_STACK_OBJECT_TYPE_CALL_INVITE:        return (char *)"Call-Invite";
        case RVSIP_COMMON_STACK_OBJECT_TYPE_TRANSMITTER:        return (char *)"Trx";
        case RVSIP_COMMON_STACK_OBJECT_TYPE_SECURITY_AGREEMENT: return (char *)"Sec-Agree";
        default:												return (char *)"-Unknown object-";
    }
}

/***************************************************************************
 * SipCommonCopyStrFromPageToPage
 * ------------------------------------------------------------------------
 * General: Copy a string from a source rpool pointer to a destination.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:         pSrcPtr - The rpool pointer of the source string
 * Input Output:  pDestPtr- The rpool pointer of the destination string. The
 *                          string offset will be returned.
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonCopyStrFromPageToPage(
                                     IN    RPOOL_Ptr       *pSrcPtr,
                                     INOUT RPOOL_Ptr       *pDestPtr)
{
    RvUint32 length;
    RvStatus rv;

    if(pSrcPtr->hPool  == NULL       ||
       pSrcPtr->hPage  == NULL_PAGE  ||
       pSrcPtr->offset == UNDEFINED  ||
       pDestPtr->hPool == NULL       ||
       pDestPtr->hPage == NULL_PAGE)
    {
        return RV_ERROR_BADPARAM;
    }

    /* Append memory in the dest page */
    length = RPOOL_Strlen(pSrcPtr->hPool, pSrcPtr->hPage,
                          pSrcPtr->offset) + 1;
    rv  = RPOOL_Append(pDestPtr->hPool,pDestPtr->hPage,
                       length, RV_FALSE, &pDestPtr->offset);
    if (RV_OK != rv)
    {
        return rv;
    }
    /*Copy the string from src page to dest page*/
    rv = RPOOL_CopyFrom(pDestPtr->hPool,pDestPtr->hPage,pDestPtr->offset,
                        pSrcPtr->hPool, pSrcPtr->hPage,pSrcPtr->offset,length);
    if (RV_OK != rv)
    {
        return rv;
    }
    return RV_OK;
}

/******************************************************************************
 * SipCommonConstructTripleLock
 * ----------------------------------------------------------------------------
 * General: Constructs locks of the triple lock and initializes the lock
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTripleLock - Pointer to the Triple Lock
 *          pLogMgr     - Pointer to the Log Manager, to be used for logging
 *****************************************************************************/
RvStatus RVCALLCONV SipCommonConstructTripleLock(
                                    IN SipTripleLock* pTripleLock,
                                    IN RvLogMgr*      pLogMgr)
{
    RvStatus crv;
    pTripleLock->bIsInitialized = RV_FALSE;
    crv = RvMutexConstruct(pLogMgr, &pTripleLock->hLock);
    if(crv != RV_OK)
    {
        return CRV2RV(crv);
    }
    crv = RvMutexConstruct(pLogMgr, &pTripleLock->hProcessLock);
    if(crv != RV_OK)
    {
        RvMutexDestruct(&pTripleLock->hLock, pLogMgr);
        return CRV2RV(crv);
    }
    crv = RvSemaphoreConstruct(1, pLogMgr, &pTripleLock->hApiLock);
    if(crv != RV_OK)
    {
        RvMutexDestruct(&pTripleLock->hLock, pLogMgr);
        RvMutexDestruct(&pTripleLock->hProcessLock, pLogMgr);
        return CRV2RV(crv);
    }
    pTripleLock->apiCnt            = 0;
    pTripleLock->objLockThreadId   = 0;
    pTripleLock->bIsInitialized    = RV_TRUE;
    pTripleLock->threadIdLockCount = UNDEFINED;
    return RV_OK;
}

/******************************************************************************
 * SipCommonDestructTripleLock
 * ----------------------------------------------------------------------------
 * General: Destructs locks of the triple lock and resets it's fields
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTripleLock - Pointer to the Triple Lock
 *          pLogMgr     - Pointer to the Log Manager, to be used for logging
 *****************************************************************************/
void RVCALLCONV SipCommonDestructTripleLock(
                                    IN SipTripleLock* pTripleLock,
                                    IN RvLogMgr*      pLogMgr)
{
    if(pTripleLock->bIsInitialized == RV_TRUE)
    {
        RvMutexDestruct(&pTripleLock->hLock, pLogMgr);
        RvMutexDestruct(&pTripleLock->hProcessLock, pLogMgr);
        RvSemaphoreDestruct(&pTripleLock->hApiLock, pLogMgr);
        pTripleLock->bIsInitialized = RV_FALSE;
    }
}

/***************************************************************************
* SipCommonUnlockBeforeCallback
* ------------------------------------------------------------------------
* General:Prepare a handle for use in a callback to the app.
*         This function will make sure the handle is unlocked the necessary
*         number of times and initialize tripleLock->objLockThreadId to NULL
*         since we are currently unlocking the object lock.
*         SipCommonLockAfterCallback() should be called after the
*         callback.
* Return Value: void.
* ------------------------------------------------------------------------
* Arguments:
* input: pLogMgr       - log manager
*        pLogSrc       - log source
*        pTripleLock   - Pointer to the triplelock
* output:pNestedLock   - the number of locks need to be done after the cb.
*        pCurrThreadId - return thread id which will be restored after the cb
*        pThreadIdLockCount - the lock count of when the thread id was updated.
***************************************************************************/


void SipCommonUnlockBeforeCallback(IN    RvLogMgr      *pLogMgr,
                                   IN    RvLogSource   *pLogSrc,
                                   IN    SipTripleLock *pTripleLock,
                                   OUT   RvInt32       *pNestedLock,
                                   OUT   RvThreadId    *pCurrThreadId,
                                   OUT   RvInt32       *pThreadIdLockCount)
{
    RvThreadId currThreadId;
    
    /* Check, if we are going to free lock, locked by us */
    currThreadId = RvThreadCurrentId();
    if (pTripleLock->objLockThreadId != (RvThreadId)0 &&
        pTripleLock->objLockThreadId != currThreadId)
    {
        RvLogExcep(pLogSrc,(pLogSrc,
            "SipCommonUnlockBeforeCallback: thread %p: Lock %p (TripleLock=%p) is locked by other thread %p",
            currThreadId, &pTripleLock->hLock, pTripleLock, pTripleLock->objLockThreadId));
        return;
    }

    *pCurrThreadId               = pTripleLock->objLockThreadId;
    *pThreadIdLockCount          = pTripleLock->threadIdLockCount;
    pTripleLock->objLockThreadId = 0;
    pTripleLock->threadIdLockCount = UNDEFINED;
    RvMutexRelease(&(pTripleLock->hLock),pLogMgr,pNestedLock);
#if (RV_MUTEX_TYPE == RV_MUTEX_NONE)
    RV_UNUSED_ARG(pLogMgr);
    RV_UNUSED_ARG(pNestedLock);
#endif /*#if (RV_MUTEX_TYPE == RV_MUTEX_NONE)*/
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSrc);
#endif

}

/***************************************************************************
* SipCommonReturnFromCallback
* ------------------------------------------------------------------------
* General:- This function will ensure that the element is locked again
*           with the specified number of times when the server returns from
*           callback.
*           Since we are locking back the object lock, the function restore
*           the tripleLock->objLockThreadId.
* ------------------------------------------------------------------------
* Arguments:
* input: pLogMgr       - logMgr
*        pTripleLock   - Handle to the object that need to be lock after
*                        the cb.
*        nestedLock    - the number of locks need to be done after the cb.
*        currThreadId  - thread id to restore.
*        threadIdLockCount - lock count to restore.
***************************************************************************/
void SipCommonLockAfterCallback(IN RvLogMgr      *pLogMgr,
                                IN SipTripleLock *pTripleLock,
                                IN RvInt32        nestedLock,
                                IN RvThreadId     currThreadId,
                                IN RvInt32        threadIdLockCount)
{
    if(nestedLock <= 0)
        return;

    RvMutexRestore(&(pTripleLock->hLock),pLogMgr,nestedLock);
    pTripleLock->objLockThreadId   = currThreadId;
    pTripleLock->threadIdLockCount = threadIdLockCount;
#if (RV_MUTEX_TYPE == RV_MUTEX_NONE)
    RV_UNUSED_ARG(pLogMgr);
    RV_UNUSED_ARG(nestedLock);
#endif /*#if (RV_MUTEX_TYPE == RV_MUTEX_NONE)*/
}

/***************************************************************************
* SipCommonFindStrInMemBuff
* ------------------------------------------------------------------------
* General:  This function finds a string in a memory buffer that doesn't 
*           necessarily terminate with '\0'.
* 
* Returns: The position of the string in the memory buffer or NULL in case
*          it's not found. 
* ------------------------------------------------------------------------
* Arguments:
* input: memBuff     - Pointer to the memory buffer.
*        buffLen     - The memory buffer length.
*        strSearched - Pointer to the searched string.
***************************************************************************/
RvChar *RVCALLCONV SipCommonFindStrInMemBuff(
                                IN RvChar   *memBuff,
                                IN RvUint32  buffLen,
                                IN RvChar   *strSearched)
{
    RvChar   *maxCharPos  = memBuff + buffLen - strlen(strSearched);
    RvChar   *currPos     = memBuff;
    RvUint32  searchedLen = (RvUint32)strlen(strSearched);
    RvUint32  bytesLeft;
        
    while (currPos != NULL && currPos <= maxCharPos)
    {
        bytesLeft = (RvUint32)(buffLen - (currPos - memBuff));
        currPos = memchr(currPos,*strSearched,bytesLeft);
        if (currPos != NULL && memcmp(currPos,strSearched,searchedLen) == 0)
        {
            return currPos;
        }
		else if (currPos != NULL)
        {
			/* The first char of currPos and strSearched are equivalent but
			memcmp failed. Need to keep looking. */
			currPos++;
        }
		
    }

    return NULL;
}
               

/***************************************************************************
* SipCommonReverseFindStrInMemBuff
* ------------------------------------------------------------------------
* General:  This function finds reversely a string in a memory buffer
*            that doesn't necessarily terminate with '\0'.
* 
* Returns: The position of the string in the memory buffer or NULL in case
*          it's not found. 
* ------------------------------------------------------------------------
* Arguments:
* input: memBuff     - Pointer to the memory buffer.
*        buffLen     - The memory buffer length.
*        strSearched - Pointer to the searched string.
***************************************************************************/
RvChar *RVCALLCONV SipCommonReverseFindStrInMemBuff(
                                    IN RvChar   *memBuff,
                                    IN RvUint32  buffLen,
                                    IN RvChar   *strSearched)
{
    /* Starting from the end of the buffer */
    RvChar   *currPos     = memBuff + buffLen - strlen(strSearched);
    RvUint32  searchedLen = (RvUint32)strlen(strSearched);
            
    while (currPos >= memBuff)
    {
         if (memcmp(currPos,strSearched,searchedLen) == 0)
         {
             return currPos;
         }
         currPos -= 1;
    }

    return NULL;
}

/***************************************************************************
* SipCommonMemBuffExactlyMatchStr
* ------------------------------------------------------------------------
* General:  This function checks if the memory buffer exactly match the 
*           given string.
* 
* Returns: RvBool
* ------------------------------------------------------------------------
* Arguments:
* Input: memBuff     - Pointer to the memory buffer.
*        buffLen     - The memory buffer length.
*        strCompared - Pointer to the compared string.
***************************************************************************/
RvBool RVCALLCONV SipCommonMemBuffExactlyMatchStr(
                                  IN RvChar   *memBuff,
                                  IN RvUint32  buffLen,
                                  IN RvChar   *strCompared)
{
    RvUint32 comparedLen = (RvUint32)strlen(strCompared);

    /* Check the if lengths are equal first since the buffer might be */
    /* contained in the string or vice versa */
    if (comparedLen != buffLen)
    {
        return RV_FALSE;
    }

    if (memcmp(memBuff,strCompared,buffLen) != 0)
    {
        return RV_FALSE;
    }

    return RV_TRUE;
}

/***************************************************************************
 * SipCommonPower
 * ------------------------------------------------------------------------
 * General: Computes 2 to the power of the retransmissions count. This
 *          result will be used to determine the next timer set.
 * Return Value: 2 to the power of retransmissionCount.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: retransmissionCount - The number of retransmissions already sent.
 ***************************************************************************/
RvInt32 RVCALLCONV SipCommonPower(RvUint32 retransmissionCount)
{
    /*
    RvUint32 i;
    */
    RvUint32 result;

    result = 1;
    if(retransmissionCount > 15)
    {
        retransmissionCount = 15;
    }

    result <<= retransmissionCount;
    /*
    for (i=0; i<retransmissionCount; i++)
    {
        result = result*2;
    }
    */
    return result;
}

#ifndef RV_SIP_PRIMITIVES
/***************************************************************************
* SipCommonGetRetryAfterFromMsg
* ------------------------------------------------------------------------
* General: This function gets the retryAfter values from a given message.
* Return Value:
* ------------------------------------------------------------------------
* Arguments:
* Input:   hMsg         - Handle to the message.
* Output:  pRetryAfter  - the retry-after value.
* Return:  RvStatus
***************************************************************************/
RvStatus SipCommonGetRetryAfterFromMsg(IN  RvSipMsgHandle hMsg,
                                       OUT RvUint32 *pRetryAfter)
{
	RvSipHeaderListElemHandle hListElem;
	RvSipRetryAfterHeaderHandle hRetryAfter;
	RvStatus			rv;

	hRetryAfter = RvSipMsgGetHeaderByType(hMsg, RVSIP_HEADERTYPE_RETRY_AFTER, RVSIP_FIRST_HEADER, &hListElem);
	if (hRetryAfter == NULL)
	{
		*pRetryAfter = 0;
		return RV_ERROR_NOT_FOUND;
	}

	rv = RvSipRetryAfterHeaderGetDeltaSeconds(hRetryAfter, pRetryAfter);
	if (rv != RV_OK)
	{
		*pRetryAfter = 0;
		return rv;
	}
	
	return RV_OK;
}
#endif /*#ifndef RV_SIP_PRIMITIVES*/

#ifdef RV_SIGCOMP_ON
/***************************************************************************
* SipCommonLookForSigCompUrnInAddr
* ------------------------------------------------------------------------
* General: Searches for SigComp identifier URN (Uniform Resource Name) 
*          that uniquely identifies the application. SIP/SigComp identifiers 
*          are carried in the 'sigcomp-id' SIP URI (Uniform Resource 
*          Identifier) in outgoing requests and incoming responses.
*   
* Return Value: Returns RvStatus.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hAddr        - Handle to the address
*          pLogSrc      - Log source pointer for printing errors
* Output:  pbUrnFound   - Indication if the URN was found.
*          pUrnRpoolPtr - Pointer to the page, pool and offset which
*                         contains a copy of the 
*                         found URN  (might be NULL in case of missing URN).
***************************************************************************/
RvStatus SipCommonLookForSigCompUrnInAddr(IN   RvSipAddressHandle  hAddr,
                                          IN   RvLogSource        *pLogSrc,
                                          OUT  RvBool             *pbUrnFound,
                                          OUT  RPOOL_Ptr          *pUrnRpoolPtr)
{        
    RvStatus rv;
    RvUint   sigcompIdLen;

    /* Initialize output parameters */ 
    *pbUrnFound = RV_FALSE;
    
    /* Check that the given address is valid URI (and not absolute URI for example) */ 
    /* Moreover, check that there's an URN parameter within the given address.      */
    rv = RvSipAddrUrlGetSigCompIdParam(hAddr,NULL,0,&sigcompIdLen);
    if (rv != RV_ERROR_INSUFFICIENT_BUFFER) 
    {
        return RV_OK; /* No URN was found */ 
    }

    /* Retrieve the URN from the given address handle */ 
    pUrnRpoolPtr->offset = SipAddrUrlGetSigCompIdParam(hAddr);
    /* Retrieve the address page */ 
    pUrnRpoolPtr->hPage = SipAddrGetPage(hAddr);
    /* Retrieve the address pool */ 
    pUrnRpoolPtr->hPool = SipAddrGetPool(hAddr);

    /* Check the validity of these retrieved parameters */ 
    if (pUrnRpoolPtr->offset < 0 || pUrnRpoolPtr->hPage == NULL || pUrnRpoolPtr->hPool== NULL) 
    {
        RvLogError(pLogSrc ,(pLogSrc ,
            "SipCommonLookForSigCompUrnInAddr - Failed to retrieve from hAddr 0x%x, the URN offset(%d) /page(0x%x) /pool(0x%x) ", 
            hAddr,pUrnRpoolPtr->offset, pUrnRpoolPtr->hPage, pUrnRpoolPtr->hPool));
        
        return RV_ERROR_UNKNOWN; 
    }

    RvLogDebug(pLogSrc ,(pLogSrc ,
        "SipCommonLookForSigCompUrnInAddr - Retrieved successfully from hAddr 0x%x, the URN offset(%d) /page(0x%x) /pool(0x%x) ", 
        hAddr,pUrnRpoolPtr->offset, pUrnRpoolPtr->hPage, pUrnRpoolPtr->hPool));
    
    
    /* Set the output parameter in order to indicate that the URN was found successfully */ 
    *pbUrnFound = RV_TRUE;     
    
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSrc);
#endif

    return RV_OK;
}
#endif /* #ifdef RV_SIGCOMP_ON */ 

#ifdef RV_SIGCOMP_ON
/***************************************************************************
* SipCommonLookForSigCompUrnInTopViaHeader
* ------------------------------------------------------------------------
* General: Searches for SigComp identifier URN (Uniform Resource Name) 
*          that uniquely identifies the application. SIP/SigComp identifiers 
*          are carried in the 'sigcomp-id' SIP URI (Uniform Resource 
*          Identifier) in outgoing request messages (and incoming responses)
*          or within Via header field parameter within incoming request 
*          messages.
*   
* Return Value: Returns RvStatus.
* ------------------------------------------------------------------------
* Arguments:
* Input:   hMsg         - Handle to a message, which might include urn in 
*                         it's topmost Via header.
*          pLogSrc      - Log source pointer for printing errors
* Output:  pbUrnFound   - Indication if the URN was found.
*          pUrnRpoolPtr - Pointer to the page, pool and offset which
*                         contains a copy of the 
*                         found URN  (might be NULL in case of missing URN).
***************************************************************************/
RvStatus SipCommonLookForSigCompUrnInTopViaHeader(
                                     IN   RvSipMsgHandle         hMsg, 
                                     IN   RvLogSource           *pLogSrc, 
                                     OUT  RvBool                *pbUrnFound,
                                     OUT  RPOOL_Ptr             *pUrnRpoolPtr)
{
    RvStatus                  rv; 
    RvUint                    sigcompIdLen;
    RvSipHeaderListElemHandle hListElem;
    RvSipViaHeaderHandle      hTopVia;

    /* 1. Initialize output parameter */ 
    *pbUrnFound = RV_FALSE;
   
    /* 2. Retrieve the Via header */ 
    hTopVia = RvSipMsgGetHeaderByType(hMsg,RVSIP_HEADERTYPE_VIA,RVSIP_FIRST_HEADER,&hListElem);
    if (hTopVia == NULL) 
    {
        RvLogError(pLogSrc ,(pLogSrc ,
            "SipCommonLookForSigCompUrnInTopViaHeader - Failed to find topmost Via header in hMsg 0x%x", hMsg));
        return RV_ERROR_NOT_FOUND;
    }

    /* 3. Check if the given Via header includes a URN at all */ 
    rv = RvSipViaHeaderGetSigCompIdParam(hTopVia,NULL,0,&sigcompIdLen);
    if (rv != RV_ERROR_INSUFFICIENT_BUFFER) 
    {
        return RV_OK; /* No URN was found */ 
    }

    /* 4. Retrieve the 'sigcomp-id' parameter from the Via header */   
    pUrnRpoolPtr->offset = SipViaHeaderGetStrSigCompIdParam(hTopVia);
    /* 5. Retrieve the 'sigcomp-id' page */ 
    pUrnRpoolPtr->hPage = SipViaHeaderGetPage(hTopVia); 
    /* 6. Retrieve the 'sigcomp-id' pool */ 
    pUrnRpoolPtr->hPool = SipViaHeaderGetPool(hTopVia); 
    
    /* 7. Check the validity of these retrieved parameters  */ 
    if (pUrnRpoolPtr->offset < 0 || pUrnRpoolPtr->hPage == NULL || pUrnRpoolPtr->hPool== NULL) 
    {
        RvLogError(pLogSrc ,(pLogSrc ,
            "SipCommonLookForSigCompUrnInAddr - Failed to retrieve from hTopVia 0x%x, the URN offset(%d) /page(0x%x) /pool(0x%x) ", 
            hTopVia,pUrnRpoolPtr->offset, pUrnRpoolPtr->hPage, pUrnRpoolPtr->hPool));
        
        return RV_ERROR_UNKNOWN; 
    }

    RvLogDebug(pLogSrc ,(pLogSrc ,
        "SipCommonLookForSigCompUrnInAddr - Retrieved successfully from hTopVia 0x%x, the URN offset(%d) /page(0x%x) /pool(0x%x) ", 
        hTopVia,pUrnRpoolPtr->offset, pUrnRpoolPtr->hPage, pUrnRpoolPtr->hPool));


    /* 8.Set the output parameter in order to indicate that the URN was found successfully */ 
    *pbUrnFound = RV_TRUE; 

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pLogSrc);
#endif

    return RV_OK;
}
#endif /* #ifdef RV_SIGCOMP_ON */ 

/*-----------------------------------------------------------------------*/
/*                             STATIC FUNCTIONS                          */
/*-----------------------------------------------------------------------*/

