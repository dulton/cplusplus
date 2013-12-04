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
* Copyright RADVision 1996.                                                     *
* Last Revision: Jan. 2000                                                      *
*********************************************************************************
*/


/*********************************************************************************
 *                              <RvSipCommonList.c>
 *
 * This file contain an API implementation of the Sip common list.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofra Wachsman                 Nov-2003
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILED                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipCommonList.h"
#include "rpool_API.h"
#include "RvSipStackTypes.h"
#include "AdsRlist.h"
#include "_SipCommonUtils.h"
#include "rvlog.h"

/*-----------------------------------------------------------------------*/
/*                            TYPE DEFINITIONS                           */
/*-----------------------------------------------------------------------*/
/*Sip Common List definition*/
typedef struct
{
    RLIST_HANDLE         hList;
    HRPOOL               hListPool;
    HPAGE                hListPage;
    RvLogSource          logSource;
    RvLogSource*         pLogSrc;
	RvBool				 bIsConstructedOnSeparatePage;
}CommonRvList;



/*Sip Common List element definition - header type */
typedef struct
{
     RvInt    eElemType;
     void*     hElemHandle;
}CommonRvListElem;

static RvStatus SipCommonListConstruct(
                IN  HRPOOL						hPool,
				IN  HPAGE						hPage,
				IN  RV_LOG_Handle				hLog,
                IN  RvLogSource                 logSource,
				IN	RvBool						bIsConstructedOnSeparatePage,
				OUT RvSipCommonListHandle*		phList);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipCommonListConstruct
 * ------------------------------------------------------------------------
 * General: Construct Sip Common list.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input :    hPool    - A handle to the List Pool.
 *            hLog     - A handle to the stack log module.
 * Output:    phList   - A handle to the created list.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCommonListConstruct(
                                    IN  HRPOOL                     hPool,
                                    IN  RV_LOG_Handle              hLog,
                                    OUT RvSipCommonListHandle*     phList)
{
    RvStatus     rv;
    HPAGE        hPage;
    RvLogSource  tempLogSource = NULL;
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    RvStatus     crv;
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

    if(NULL == hPool)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(NULL == phList)
    {
       return RV_ERROR_NULLPTR;
    }

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    crv = RvLogGetSourceByName((RvLogMgr*)hLog, "RLIST", &tempLogSource);
    if(RV_OK != crv || tempLogSource == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

    rv = RPOOL_GetPage(hPool, 0, &hPage);
    if(rv != RV_OK)
    {
        if (NULL != tempLogSource)
        {
            RvLogError(&tempLogSource,(&tempLogSource,
                "RvSipCommonListConstruct - Failed to allocate a new page on pool 0x%p(rv=%d:%s)",
                hPool, rv, SipCommonStatus2Str(rv)));
        }
        return rv;
    }

	return SipCommonListConstruct(hPool, hPage, hLog, tempLogSource, RV_TRUE, phList);
}

/***************************************************************************
 * RvSipCommonListConstructOnPage
 * ------------------------------------------------------------------------
 * General: Construct Sip Common list on a given Pool and Page.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input :    hPool    - A handle to the List Pool.
 *			  hPage	   - A handle to the List Page.	
 *            hLog     - A handle to the stack log module.
 * Output:    phList   - A handle to the created list.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCommonListConstructOnPage(
									IN  HRPOOL						hPool,
									IN  HPAGE						hPage,
									IN  RV_LOG_Handle				hLog,
									OUT RvSipCommonListHandle*		phList)
{
    if(NULL == hPool)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

	if(NULL == hPage)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    if(NULL == phList)
    {
       return RV_ERROR_NULLPTR;
    }

	return SipCommonListConstruct(hPool, hPage, hLog, NULL/*logSource*/, RV_FALSE, phList);
}

/***************************************************************************
 * RvSipCommonListDestruct
 * ------------------------------------------------------------------------
 * General: Destruct a Sip Common list. When the list is destructed its free
 *          all stored data (header and messages).
 * Return Value: RV_OK - on success.
 *               RV_ERROR_NULLPTR - hList is NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input: hHeaderList - handle to list to destruct.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCommonListDestruct(
                                       IN RvSipCommonListHandle hList)
{
    CommonRvList* pList = (CommonRvList*)hList;
	if(RV_TRUE == pList->bIsConstructedOnSeparatePage)
    {
		RPOOL_FreePage(pList->hListPool, pList->hListPage);
    }
    return RV_OK;
}

/***************************************************************************
 * RvSipCommonListPushElem
 * ------------------------------------------------------------------------
 * General: Add given element to the Sip Common list.
 *          Application must allocate the new element on the list page
 *          and then call this function to add the allocated element to the list.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input : hList       - handle to list.
 *         eElemType   - element type to be set in the list element.
 *                       (e.g RVSIP_HEADERTYPE_CONTACT when adding a contact
 *                       header to the list)
 *           pData       - pointer to allocated element data (e.g this is the contact
 *                       header handle that application got when calling
 *                       RvSipContactHeaderConstruct).
 *           eLocation   - inserted element location (first, last, etc).
 *         hPos        - current list position, relevant in case that
 *                       location is next or prev.
 * Output: pNewPos     - returned the location of the object that was pushed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCommonListPushElem(
                              IN     RvSipCommonListHandle       hList,
                              IN     RvInt                      eElemType,
                              IN     void*                       pData,
                              IN     RvSipListLocation           eLocation,
                              IN     RvSipCommonListElemHandle   hPos,
                              INOUT  RvSipCommonListElemHandle*  phNewPos)
{
    CommonRvList*        pList          = NULL;
    RvStatus             rv             = RV_OK;

    if(NULL == hList)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pList = (CommonRvList*)hList;

    if(NULL == pData || NULL == phNewPos)
    {
       return RV_ERROR_NULLPTR;
    }

    RvLogDebug(pList->pLogSrc,(pList->pLogSrc,
              "RvSipCommonListPushElem - CommonRvList=0x%p is about to push element=0x%p type=%d",
               pList, pData, eElemType));

    switch(eLocation)
    {
    case RVSIP_FIRST_ELEMENT:
        rv = RLIST_InsertHead(NULL, pList->hList, (RLIST_ITEM_HANDLE*)phNewPos);
        break;
    case RVSIP_LAST_ELEMENT:
        rv = RLIST_InsertTail(NULL, pList->hList, (RLIST_ITEM_HANDLE*)phNewPos);
        break;
    case RVSIP_NEXT_ELEMENT:
        rv = RLIST_InsertAfter(NULL, pList->hList, (RLIST_ITEM_HANDLE)hPos, (RLIST_ITEM_HANDLE*)phNewPos);
        break;
    case RVSIP_PREV_ELEMENT:
        rv = RLIST_InsertBefore(NULL, pList->hList, (RLIST_ITEM_HANDLE)hPos, (RLIST_ITEM_HANDLE*)phNewPos);
        break;
    default:
        RvLogError(pList->pLogSrc,(pList->pLogSrc,
              "RvSipCommonListPushElem - CommonRvList 0x%p - unknown location %d",
               pList, eLocation));
        return RV_ERROR_UNKNOWN;
    }
    if(RV_OK != rv)
    {
          RvLogError(pList->pLogSrc,(pList->pLogSrc,
              "RvSipCommonListPushElem - CommonRvList 0x%p - Failed to push element 0x%p (rv=%d)",
               pList, pData, rv));
        return rv;
    }

    ((CommonRvListElem*)*phNewPos)->eElemType    = eElemType;
    ((CommonRvListElem*)*phNewPos)->hElemHandle  = pData;


    RvLogDebug(pList->pLogSrc,(pList->pLogSrc,
        "RvSipCommonListPushElem - CommonRvList 0x%p - new element=0x%p was successfully pushed",
         pList, *phNewPos));

    return RV_OK;
}

/***************************************************************************
 * RvSipCommonListRemoveElem
 * ------------------------------------------------------------------------
 * General: Remove an element from the list.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input : hList       - List.
 *         hListElem   - handle to the list element to be removed.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCommonListRemoveElem(
                                    IN RvSipCommonListHandle     hList,
                                    IN RvSipCommonListElemHandle hListElem)
{
    CommonRvList*  pList  = NULL;

    if (NULL == hList)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pList = (CommonRvList*)hList;

    if(NULL == hListElem)
    {
       RvLogError(pList->pLogSrc,(pList->pLogSrc,
                 "RvSipCommonListRemoveElem - CommonRvList 0x%p - illegal action with hListElem=NULL",
                  pList));
        return RV_ERROR_ILLEGAL_ACTION;
    }

    RLIST_Remove(NULL,
                 pList->hList,
                 (RLIST_ITEM_HANDLE)hListElem);

    RvLogDebug(pList->pLogSrc,(pList->pLogSrc,
              "RvSipCommonListRemoveElem - CommonRvList 0x%p - element=0x%p was removed",
               pList, hListElem));

    return RV_OK;
}

/***************************************************************************
 * RvSipCommonListGetElem
 * ------------------------------------------------------------------------
 * General: Return element from the Sip Common list.
 *          The function returns the first/last elements, or the
 *          previous/following elements relatively to another element.
 * Return value: RV_ERROR_INVALID_HANDLE, RV_ERROR_NULLPTR, RV_ERROR_BADPARAM when
 *          having am error in the function.
 *          RV_OK - for all other cases. note that even if there are no
 *          more elements in the list, RV_OK is returned, and pElemData
 *          will contain NULL;
 * ------------------------------------------------------------------------
 * Arguments:
 * Input :    hList - List.
 *          eLocation  - Location of the wanted element in the list.
 *                       (first, last, next, prev).
 *          hPos       - current list position, relevant in case that
 *                       location is next or prev.
 * Output:    peElemType - the type of the list element that was got from list.
 *          pElemData  - the data of the list element that was got from list.
 *          phNewPos   - the location of the object that was got.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCommonListGetElem(
                               IN     RvSipCommonListHandle      hList,
                               IN     RvSipListLocation          eLocation,
                               IN     RvSipCommonListElemHandle  hPos,
                               OUT    RvInt                     *peElemType,
                               OUT    void*                      *pElemData,
                               OUT    RvSipCommonListElemHandle  *phNewPos)
{
    CommonRvList*    pList  = NULL;
    RLIST_ITEM_HANDLE hElem  = NULL;

    if (NULL == hList )
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pList = (CommonRvList*)hList;

    if(NULL == phNewPos || NULL == peElemType || NULL == pElemData)
    {
       RvLogError(pList->pLogSrc,(pList->pLogSrc,
             "RvSipCommonListGetElem - CommonRvList 0x%p - bad pointer: peElemType=0x%p pElemData=0x%p phListElem=0x%p",
              pList, peElemType, pElemData, phNewPos));
       return RV_ERROR_NULLPTR;
    }


    switch(eLocation)
    {
    case RVSIP_FIRST_ELEMENT:
        RLIST_GetHead(NULL, pList->hList, &hElem);
        break;
    case RVSIP_LAST_ELEMENT:
        RLIST_GetTail(NULL, pList->hList, &hElem);
        break;
    case RVSIP_NEXT_ELEMENT:
        RLIST_GetNext(NULL, pList->hList, (RLIST_ITEM_HANDLE)hPos, &hElem);
        break;
    case RVSIP_PREV_ELEMENT:
        RLIST_GetPrev(NULL, pList->hList, (RLIST_ITEM_HANDLE)hPos, &hElem);
        break;
    default:
        RvLogError(pList->pLogSrc,(pList->pLogSrc,
             "RvSipCommonListGetElem - CommonRvList 0x%p - unknown eLocation %d",
              pList, eLocation));
       return RV_ERROR_BADPARAM;
    }

    *phNewPos = (RvSipCommonListElemHandle)hElem;
    if(NULL == hElem)
    {
        *pElemData = NULL;
        *peElemType = UNDEFINED;
        RvLogDebug(pList->pLogSrc,(pList->pLogSrc,
                  "RvSipCommonListGetElem - CommonRvList=0x%p, no more elements",pList));
        return RV_OK;
    }

    *peElemType = ((CommonRvListElem*)hElem)->eElemType;
    *pElemData  = ((CommonRvListElem*)hElem)->hElemHandle;

    RvLogDebug(pList->pLogSrc,(pList->pLogSrc,
              "RvSipCommonListGetElem - CommonRvList 0x%p - returning element=0x%p, elemType=%d, elemData=0x%p",
               pList, hElem, *peElemType, *pElemData));

    return RV_OK;
}


/***************************************************************************
 * RvSipCommonListIsEmpty
 * ------------------------------------------------------------------------
 * General: Return indication whether the Sip Common list is empty.
 * Return Value: RV_OK    - on success.
 *                 RV_ERROR_NULLPTR - either hList or pbIsEmpty are NULL.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  hList - Handle to the list.
 *    Output: *pbIsEmpty  - RV_TRUE if Sip Common list is empty. otherwise RV_FALSE.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCommonListIsEmpty(
                                         IN  RvSipCommonListHandle hList,
                                         OUT RvBool*           pbIsEmpty)
{
    CommonRvList*     pList = NULL;
    RLIST_ITEM_HANDLE  hElem  = NULL;

    if(NULL == hList )
    {
        return RV_ERROR_NULLPTR;
    }
    pList = (CommonRvList*)hList;

    if(NULL == pbIsEmpty)
    {
       RvLogError(pList->pLogSrc,(pList->pLogSrc,
                 "RvSipCommonListIsEmpty - CommonRvList 0x%p - bad param pbIsEmpty=NULL", pList));
        return RV_ERROR_NULLPTR;
    }

    RLIST_GetHead(NULL, pList->hList, &hElem);
    if(NULL == hElem)
    {
        *pbIsEmpty = RV_TRUE;
    }
    else
    {
        *pbIsEmpty = RV_FALSE;
    }

    RvLogDebug(pList->pLogSrc,(pList->pLogSrc,
              "RvSipCommonListIsEmpty - CommonRvList=0x%p, IsEmpty=%d",
               pList, *pbIsEmpty));
    return(RV_OK);
}

/***************************************************************************
 * RvSipCommonListGetPool
 * ------------------------------------------------------------------------
 * General: Return the Sip Common list memory pool.
 * Return Value: the memory pool handle.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  hList - Handle to the list.
 ***************************************************************************/
RVAPI HRPOOL RVCALLCONV RvSipCommonListGetPool(
                                         IN  RvSipCommonListHandle hList)
{
    CommonRvList*     pList = NULL;

    if(NULL == hList )
    {
        return NULL;
    }
    pList = (CommonRvList*)hList;
    return pList->hListPool;
}

/***************************************************************************
 * RvSipCommonListGetPage
 * ------------------------------------------------------------------------
 * General: Return the Sip Common list memory page.
 * Return Value: the memory page handle.
 * ------------------------------------------------------------------------
 * Arguments:
 *    Input:  hList - Handle to the list.
 ***************************************************************************/
RVAPI HPAGE RVCALLCONV RvSipCommonListGetPage(
                                         IN  RvSipCommonListHandle hList)
{
    CommonRvList*     pList = NULL;

    if(NULL == hList )
    {
        return NULL;
    }
    pList = (CommonRvList*)hList;
    return pList->hListPage;
}

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS								 */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SipCommonListConstruct
 * ------------------------------------------------------------------------
 * General: Construct Sip Common list on a given Pool and Page.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input :    hPool     - A handle to the List Pool.
 *			  hPage	    - A handle to the List Page.	
 *            hLog      - A handle to the stack log module.
 *            logSource - log source. If NULL, RLIST source will be used.
 *            bIsConstructedOnSeparatePage	- A boolean stating whether to
 *											  free the page on List destruction.
 * Output:    phList    - A handle to the created list.
 ***************************************************************************/
static RvStatus SipCommonListConstruct(
                    IN  HRPOOL					hPool,
                    IN  HPAGE					hPage,
                    IN  RV_LOG_Handle			hLog,
                    IN  RvLogSource             logSource,
                    IN	RvBool					bIsConstructedOnSeparatePage,
				    OUT RvSipCommonListHandle*	phList)
{
    CommonRvList*	pNewSipList	= NULL;
    RvInt32			listOffset;

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    if (NULL == logSource)
    {
        RvStatus crv;
        crv = RvLogGetSourceByName((RvLogMgr*)hLog, "RLIST", &logSource);
        if(RV_OK != crv || logSource == NULL)
        {
            return RV_ERROR_UNKNOWN;
        }
    }
#else
	RV_UNUSED_ARG(hLog);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

    pNewSipList = (CommonRvList*)RPOOL_AlignAppend(hPool, hPage, sizeof(CommonRvList), &listOffset);
    if(NULL == pNewSipList)
    {
        RvLogError(&logSource,(&logSource,
              "SipCommonListConstruct - Failed to construct new CommonRvList object on pool 0x%p page 0x%p",
              hPool, hPage));
		if(RV_TRUE == bIsConstructedOnSeparatePage)
		{
			RPOOL_FreePage(hPool, hPage);
		}
        return RV_ERROR_UNKNOWN;
    }
	
    pNewSipList->hListPool = hPool;
    pNewSipList->hListPage = hPage;
    pNewSipList->logSource = logSource;
	pNewSipList->bIsConstructedOnSeparatePage = bIsConstructedOnSeparatePage;

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    pNewSipList->pLogSrc   = &(pNewSipList->logSource);
#else
    pNewSipList->pLogSrc   = NULL;
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
    
	pNewSipList->hList     = RLIST_RPoolListConstruct(hPool,hPage,sizeof(CommonRvListElem),pNewSipList->pLogSrc);
    
	if(NULL == pNewSipList->hList)
    {
        RvLogError(pNewSipList->pLogSrc,(pNewSipList->pLogSrc,
              "SipCommonListConstruct - Failed to construct new list on pool 0x%p page 0x%p",
              hPool, hPage));
        if(RV_TRUE == bIsConstructedOnSeparatePage)
		{
			RPOOL_FreePage(hPool, hPage);
		}
        return RV_ERROR_UNKNOWN;
    }

    *phList = (RvSipCommonListHandle)pNewSipList;

    RvLogDebug(pNewSipList->pLogSrc,(pNewSipList->pLogSrc,
              "SipCommonListConstruct - CommonRvList=0x%p was successfully constructed on pool 0x%p page 0x%p",
               pNewSipList, hPool, hPage));

    return RV_OK;
}

/* new line :)*/


