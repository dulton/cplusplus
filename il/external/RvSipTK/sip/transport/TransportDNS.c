#ifdef __cplusplus
extern "C" {
#endif

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
* Last Revision:                                                       *
*********************************************************************************
*/


/*********************************************************************************
 *                              TransportDNS.c
 *
 * This c file contains implementations for dns transport functions:
 * 1. The transport decision algorithm
 * 2. The port decision algorithm
 * 3. Functions needed to manipulate the dns list
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Galanos                 08/12/02
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#include "TransportDNS.h"
#include "_SipTransport.h"
#include "_SipCommonUtils.h"
#include "RvSipAddress.h"
#include "_SipCommonConversions.h"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV DnsCloneIPAddressList(IN  TransportDNSList    *dnsOrigList,
                                                 OUT TransportDNSList    *dnsCloneList);

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
static RvStatus RVCALLCONV DnsCloneSrvList(IN  TransportDNSList    *dnsOrigList,
                                          OUT TransportDNSList    *dnsCloneList);

static RvStatus RVCALLCONV DnsCloneHostNameList(IN  TransportDNSList            *dnsOrigList,
                                               OUT TransportDNSList            *dnsCloneList);
static RvStatus DnsNaptrFindHigherPriorityRecord(
    IN  RvUint16            order,
    IN  RvUint16            preference,
    IN  RLIST_HANDLE        hList,
    OUT RLIST_ITEM_HANDLE    *pSrvElement);

static RvStatus DnsSrvFindHigherPriorityRecord(
    IN  TransportMgr*       pMgr,
    IN  RvUint16            priority,
    IN  RvUint32            weight,
    IN  RLIST_HANDLE        hList,
    OUT RLIST_ITEM_HANDLE    *pElement,
    OUT RvBool              *pbInsertBefore);
#endif /* RV_DNS_ENHANCED_FEATURES_SUPPORT */

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/***************************************************************************
 * TransportDNSListConstruct
 * ------------------------------------------------------------------------
 * General: The SipTransportDNSListConstruct function allocates and fills
 *          the TransportDNSList structure and returns handler to the
 *          TransportDNSList and appropriate error code or RV_OK.
 *          Receives as input memory pool, page and list pool where lists
 *          element and the TransportDNSList object itself will be allocated.
 * Return Value: RV_ERROR_INVALID_HANDLE - One of the input handles is invalid
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMemPool    - Handle of the memory pool.
 *          hMemPage    - Handle of the memory page
 *          hListsPool  - Handle of the list pool
 *          maxElementsInSingleDnsList - maximum num. of elements
 *                        in single list
 *          bAppList    - Is this list constructed by application or by stack.
 * Output:  ppDnsList   - Pointer to a DNS list object handle.
 ***************************************************************************/
RvStatus RVCALLCONV TransportDNSListConstruct(IN  HRPOOL                      hMemPool,
                                              IN  HPAGE                       hMemPage,
                                              IN  RvUint32                    maxElementsInSingleDnsList,
                                              IN  RvBool                      bAppList,
                                              OUT TransportDNSList            **ppDnsList)
{
    TransportDNSList dnsList;
    RvInt32         offset      = 0;
    RvStatus        rv     = RV_OK;
    
    memset((void*)&dnsList,0,sizeof(dnsList));
    /* Initiate DNS list structure */
    dnsList.hMemPage        = hMemPage;
    dnsList.hMemPool        = hMemPool;
    dnsList.bAppList        = bAppList;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    dnsList.hSrvNamesList   = NULL;
    dnsList.hHostNamesList  = NULL;
    memset(&dnsList.usedHostNameElement,0,sizeof(dnsList.usedHostNameElement));
    memset(&dnsList.usedSRVElement,0,sizeof(dnsList.usedSRVElement));
#endif
    dnsList.hIPAddrList     = NULL;
    dnsList.pENUMResult     = NULL;
    dnsList.maxElementsInSingleDnsList = maxElementsInSingleDnsList;
    rv = RPOOL_Align(hMemPool, hMemPage);
    if(rv != RV_OK)
    {
        return rv;
    }

    /* Save the initiated structure on the input memory page */
    rv = RPOOL_AppendFromExternal(hMemPool,hMemPage,&dnsList,sizeof(TransportDNSList),
                                       RV_TRUE,&offset,(RPOOL_ITEM_HANDLE *)ppDnsList);
    if(rv != RV_OK)
    {
        return rv;
    }

    return RV_OK;
}

/***************************************************************************
 * TransportDNSListDestruct
 * ------------------------------------------------------------------------
 * General: The RvSipTransportDNSListDestruct function frees all memory allocated
 *          by the TransportDNSList object, including the TransportDNSList itself.
 * Return Value: RV_ERROR_INVALID_HANDLE - One of the input handles is invalid
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   phDnsList   - Pointer to a DNS list object handler.
 * Output:  N/A
 ***************************************************************************/
RvStatus RVCALLCONV TransportDNSListDestruct(IN TransportDNSList* pDnsList)
{
    /* Check input and initiate internal parameters */
    if (pDnsList == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* Clear DNS list object */
    pDnsList->hMemPage       = NULL;
    pDnsList->hMemPool       = NULL;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    pDnsList->hSrvNamesList  = NULL;
    pDnsList->hHostNamesList = NULL;
#endif
    pDnsList->hIPAddrList    = NULL;

    /* Note that there is no possibility to free memory, used by the
    DNS list itself rather than freeing entire memory page. The memory
    page should be free by other function. */
    return RV_OK;
}


/***************************************************************************
 * TransportDNSListClone
 * ------------------------------------------------------------------------
 * General: The SipTransportDNSListClone function copies entire original
 *          TransportDNSList object to it's clone object.
 *          Note that clone object should be constructed before by call to the
 *          SipTransportDNSListConstruct function.
 * Return Value: RV_ERROR_INVALID_HANDLE - One of the input handles is invalid
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr                - transport Mgr
 *          pDnsListOriginal    - Original DNS list object handler.
 * Output:  pDnsListClone       - Clone DNS list object handler
 ***************************************************************************/
RvStatus RVCALLCONV TransportDNSListClone(IN TransportMgr* pMgr,
                                          IN  TransportDNSList*  pDnsListOriginal,
                                          OUT TransportDNSList*   pDnsListClone)
{
    RvStatus        rv;

    /* Check input */
    if ((pDnsListClone == NULL) || (pDnsListOriginal == NULL))
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = DnsCloneIPAddressList(pDnsListOriginal,pDnsListClone);
    if (rv != RV_OK)
    {
        RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "TransportDNSListClone - (org=0x%p,dest=0x%p) failed to clone IPs list (rv=%d)", 
            pDnsListOriginal,pDnsListClone,rv));
        return rv;
    }
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    rv = DnsCloneHostNameList(pDnsListOriginal,pDnsListClone);
    if (rv != RV_OK)
    {
        RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "TransportDNSListClone - (org=0x%p,dest=0x%p) failed to clone hosts list (rv=%d)", 
            pDnsListOriginal,pDnsListClone,rv));
        return rv;
    }

    rv = DnsCloneSrvList(pDnsListOriginal,pDnsListClone);
    if (rv != RV_OK)
    {
        RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "TransportDNSListClone - (org=0x%p,dest=0x%p) failed to copy Srv list (rv=%d)", 
            pDnsListOriginal,pDnsListClone,rv));
        return rv;
    }
#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT */

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pMgr);
#endif

    return RV_OK;
}

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/***************************************************************************
 * TransportDNSListGetSrvElement
 * ------------------------------------------------------------------------
 * General: retrieves SRV element from the SRV list of the DNS list object
 * according to specified by input location.
 * Return Value: RV_OK, RV_ERROR_UNKNOWN, RV_ERROR_INVALID_HANDLE or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 *          location        - starting element location
 *          pRelative       - relative SRV element. Used when location
 *                            is 'next' or 'previous'
 * Output:  pSrvElement     - found element
 *          pRelative       - new relative SRV element for get consequent
 *                            RvSipTransportDNSListGetSrvElement function
 *                            calls.
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListGetSrvElement (
                                           IN     TransportMgr*                     pTransportMgr,
                                           IN     TransportDNSList*                 pDnsList,
                                           IN     RvSipListLocation                 location,
                                           INOUT  void                              **pRelative,
                                           OUT    RvSipTransportDNSSRVElement       *pSrvElement)
{
    TransportDNSSRVElement**  srvElement = NULL;
    RvStatus                  retCode    = RV_OK;
    RvInt32                   srvNameLen; /* Could be negative and compared with -1 further */

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListGetSrvElement - getting element from list 0x%p", pDnsList));

    if (NULL == pDnsList->hSrvNamesList)
    {
        return RV_ERROR_NOT_FOUND;
    }
    switch(location)
    {
    case RVSIP_NEXT_ELEMENT:
        if (*pRelative != NULL)
        {
            RLIST_GetNext(NULL,pDnsList->hSrvNamesList,
                (RLIST_ITEM_HANDLE)*pRelative,(RLIST_ITEM_HANDLE *)&srvElement);
            break;
        }
        /* In case no relative where specified, retrieve head of the list */
    case RVSIP_FIRST_ELEMENT:
        RLIST_GetHead (NULL,pDnsList->hSrvNamesList,
            (RLIST_ITEM_HANDLE *)&srvElement);
        break;
    case RVSIP_PREV_ELEMENT:
        if (*pRelative != NULL)
        {
            RLIST_GetPrev(NULL,pDnsList->hSrvNamesList,
                (RLIST_ITEM_HANDLE)*pRelative,(RLIST_ITEM_HANDLE *)&srvElement);
            break;
        }
        /* In case no relative where specified, retrive tail of the list */
    case RVSIP_LAST_ELEMENT:
        RLIST_GetTail (NULL,pDnsList->hSrvNamesList,
            (RLIST_ITEM_HANDLE *)&srvElement);
        break;
    default:
        return RV_ERROR_UNKNOWN;
    }

    if (srvElement == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }
    *pRelative = (void *)srvElement;
    pSrvElement->order = srvElement[0]->order;
    pSrvElement->preference = srvElement[0]->preference;
    pSrvElement->protocol = srvElement[0]->protocol;

    srvNameLen = RPOOL_Strlen(pDnsList->hMemPool,
                              pDnsList->hMemPage,
                              srvElement[0]->nameOffset);
    if (srvNameLen == -1)
    {
        return RV_ERROR_UNKNOWN;
    }

    retCode = RPOOL_CopyToExternal(pDnsList->hMemPool,pDnsList->hMemPage,
        srvElement[0]->nameOffset,pSrvElement->srvName,srvNameLen+1);

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListGetSrvElement - got element from list %s (ret code = %d)", pSrvElement->srvName, retCode));

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransportMgr);
#endif

    return retCode;
}

/***************************************************************************
 * TransportDNSListGetHostElement
 * ------------------------------------------------------------------------
 * General: retrieves host element from the host elements list of the DNS
 * list object according to specified by input location.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport manager
 *          pDnsList        - pointer of the DNS list object
 *          location        - starting element location
 *          pRelative       - relative host name element for 'next' or 'previous'
 *                            locations
 * Output:  pHostElement    - found element
 *          pRelative       - new relative host name element for consequent
 *          call to the RvSipTransportDNSListGetHostElement function.
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListGetHostElement (
                                           IN  TransportMgr*                     pTransportMgr,
                                           IN  TransportDNSList                 *pDnsList,
                                           IN  RvSipListLocation                location,
                                           INOUT  void                          **pRelative,
                                           OUT RvSipTransportDNSHostNameElement *pHostElement)
{
    TransportDNSHostNameElement **hostNameElement = NULL;
    RvInt32                       srvNameLen; /* Can be negative and compared to -1 later */
    RvUint32                      retCode;

    if ((pDnsList == NULL) || (pDnsList->hHostNamesList == NULL) ||
        (pHostElement == NULL) || (pRelative == NULL))
    {
        return RV_ERROR_BADPARAM;
    }
    if (NULL == pTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListGetHostElement - getting element from list 0x%p", pDnsList));

    switch(location)
    {
    case RVSIP_NEXT_ELEMENT:
        if (*pRelative != NULL)
        {
            RLIST_GetNext(NULL,pDnsList->hHostNamesList,
                (RLIST_ITEM_HANDLE)*pRelative,(RLIST_ITEM_HANDLE *)&hostNameElement);
            break;
        }
        /* In case pRelative is NULL, first list element should be retrived */
    case RVSIP_FIRST_ELEMENT:
        RLIST_GetHead (NULL,pDnsList->hHostNamesList,
            (RLIST_ITEM_HANDLE *)&hostNameElement);
        break;
    case RVSIP_PREV_ELEMENT:
        if (*pRelative != NULL)
        {
            RLIST_GetPrev(NULL,pDnsList->hHostNamesList,
                (RLIST_ITEM_HANDLE)*pRelative,(RLIST_ITEM_HANDLE *)&hostNameElement);
            break;
        }
        /* In case pRelative is NULL, last list element should be retrived */
    case RVSIP_LAST_ELEMENT:
        RLIST_GetTail (NULL,pDnsList->hHostNamesList,
            (RLIST_ITEM_HANDLE *)&hostNameElement);
        break;
    default:
        return RV_ERROR_UNKNOWN;
    }

    if (hostNameElement == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    *pRelative = (void *)hostNameElement;
    pHostElement->port = hostNameElement[0]->port;
    pHostElement->priority = hostNameElement[0]->priority;
    pHostElement->weight = hostNameElement[0]->weight;
    pHostElement->protocol = hostNameElement[0]->protocol;

    srvNameLen = RPOOL_Strlen(pDnsList->hMemPool,
                              pDnsList->hMemPage,
                              hostNameElement[0]->hostNameOffset);
    if (srvNameLen == -1)
    {
        return RV_ERROR_UNKNOWN;
    }

    retCode = RPOOL_CopyToExternal(pDnsList->hMemPool,pDnsList->hMemPage,
        hostNameElement[0]->hostNameOffset,pHostElement->hostName,srvNameLen+1);

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListGetHostElement - got element from list %s (ret code = %d)", pHostElement->hostName, retCode));

    return retCode;
}

/***************************************************************************
 * TransportDNSListRemoveTopmostSrvElement
 * ------------------------------------------------------------------------
 * General: removes topmost SRV element from the SRV elements list of the
 * DNS list object.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListRemoveTopmostSrvElement(
                                          IN TransportMgr*          pTransportMgr,
                                          IN TransportDNSList            *pDnsList)
{
    RLIST_ITEM_HANDLE           element = NULL;

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListRemoveTopmostSrvElement - removing element from list 0x%p", pDnsList));

    RLIST_GetHead (NULL,pDnsList->hSrvNamesList,&element);
    if (element == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    RLIST_Remove (NULL,pDnsList->hSrvNamesList,element);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransportMgr);
#endif

	return RV_OK;
}

/***************************************************************************
 * TransportDNSListRemoveTopmostHostElement
 * ------------------------------------------------------------------------
 * General: removes topmost host element from the head of the host elements
 * list of the DNS list object.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListRemoveTopmostHostElement(
                                        IN TransportMgr*          pTransportMgr,
                                        IN TransportDNSList            *pDnsList)
{
    RLIST_ITEM_HANDLE           element  = NULL;

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListRemoveTopmostHostElement - removing element from list 0x%p", pDnsList));

    RLIST_GetHead (NULL,pDnsList->hHostNamesList,&element);
    if (element == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    RLIST_Remove (NULL,pDnsList->hHostNamesList,element);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransportMgr);
#endif

	return RV_OK;
}

/***************************************************************************
 * TransportDNSListPopSrvElement
 * ------------------------------------------------------------------------
 * General: retrieves and removes topmost SRV name element from the SRV
 * elements list of the DNS list object.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr    - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 * Output:  pSrvElement     - retrieved element
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListPopSrvElement (
                                  IN  TransportMgr*                 pTransportMgr,
                                  IN  TransportDNSList            *pDnsList,
                                  OUT RvSipTransportDNSSRVElement   *pSrvElement)
{
    RvStatus retCode;
    void      *pRelative;

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListPopSrvElement - popping element from list 0x%p", pDnsList));

    retCode = TransportDNSListGetSrvElement (pTransportMgr,
                                             pDnsList,
                                             RVSIP_FIRST_ELEMENT,
                                             &pRelative,
                                             pSrvElement);
    if (retCode != RV_OK)
    {
        return retCode;
    }
    return TransportDNSListRemoveTopmostSrvElement(pTransportMgr, pDnsList);
}

/***************************************************************************
 * TransportDNSListPopHostElement
 * ------------------------------------------------------------------------
 * General: retrieves and removes topmost host element from the list of host
 * elements in the DNS list object.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 * Output:  pHostElement    - element
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListPopHostElement (
                                       IN  TransportMgr*                    pTransportMgr,
                                       IN  TransportDNSList            *pDnsList,
                                       OUT RvSipTransportDNSHostNameElement *pHostElement)
{
    RvStatus retCode;
    void      *pRelative;

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListPopHostElement - popping element from list 0x%p", pDnsList));

    retCode = TransportDNSListGetHostElement (pTransportMgr,
                                              pDnsList,
                                              RVSIP_FIRST_ELEMENT,
                                              &pRelative,
                                              pHostElement);
    if (retCode != RV_OK)
    {
        return retCode;
    }

    return TransportDNSListRemoveTopmostHostElement(pTransportMgr, pDnsList);
}

/***************************************************************************
 * TransportDNSListPushSrvElement
 * ------------------------------------------------------------------------
 * General: adds single SRV element to the head of the SRV names list of the
 * DNS list object.
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 *          pSrvElement     - SRV element structure to be added to the list
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListPushSrvElement(
                                    IN TransportMgr*                pTransportMgr,
                                    IN TransportDNSList            *pDnsList,
                                    IN RvSipTransportDNSSRVElement  *pSrvElement)
{
    TransportDNSSRVElement      srvElement;
    RvUint32                   retCode;
    RPOOL_ITEM_HANDLE           itemPtr;
    RvInt32                    addrOffset;
    RLIST_ITEM_HANDLE           listElem;
    RvUint32                   numOfElements;

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListPushSrvElement - pushing element to list 0x%p", pDnsList));

    pDnsList->hSrvNamesList = (pDnsList->hSrvNamesList == NULL)?
        RLIST_RPoolListConstruct(pDnsList->hMemPool,pDnsList->hMemPage,
        sizeof(void*), NULL):pDnsList->hSrvNamesList;

    if (pDnsList->hSrvNamesList == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    retCode = RLIST_GetNumOfElements(NULL,pDnsList->hSrvNamesList,
        &numOfElements);
    if ((pDnsList->maxElementsInSingleDnsList > 0) &&
        (pDnsList->maxElementsInSingleDnsList <= numOfElements))
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "TransportDNSListPushSrvElement - maximum number of srv elements exceeded (list=0x%p, max allowed=%d rv=%d)",
                  pDnsList,
                  pDnsList->maxElementsInSingleDnsList,
                  RV_ERROR_OUTOFRESOURCES));
        return RV_ERROR_OUTOFRESOURCES;
    }

    srvElement.order = pSrvElement->order;
    srvElement.preference = pSrvElement->preference;
    srvElement.protocol = pSrvElement->protocol;

    retCode = RPOOL_AppendFromExternal(pDnsList->hMemPool,
        pDnsList->hMemPage,pSrvElement->srvName ,
        (RvInt)strlen(pSrvElement->srvName)+1,
        RV_FALSE,&srvElement.nameOffset,&itemPtr);
#ifdef SIP_DEBUG
    srvElement.strSrvName = (RvChar *)itemPtr;
#endif

    if(retCode != RV_OK)
    {
        return RV_ERROR_UNKNOWN;
    }

    /* Insert SRV element into DNS list */
    retCode = RPOOL_Align(pDnsList->hMemPool,pDnsList->hMemPage);
    if(retCode != RV_OK)
    {
      return RV_ERROR_UNKNOWN;
    }

    retCode = RPOOL_AppendFromExternal(pDnsList->hMemPool,
        pDnsList->hMemPage,&srvElement,sizeof(TransportDNSSRVElement),
        RV_TRUE,&addrOffset,&itemPtr);
    if(retCode != RV_OK)
    {
        return RV_ERROR_UNKNOWN;
    }

    retCode = RLIST_InsertHead(NULL,
        pDnsList->hSrvNamesList,&listElem);
    if (retCode != RV_OK)
    {
        return RV_ERROR_UNKNOWN;
    }
    *(void **)listElem = (void *)itemPtr;

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListPushSrvElement - element successfully pushed to list 0x%p", pDnsList));

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransportMgr);
#endif

    return RV_OK;
}

/***************************************************************************
 * TransportDNSListPushHostElement
 * ------------------------------------------------------------------------
 * General: adds host element to the head of the host elements list of the
 * DNS list object.
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 *          pHostElement    - host name element structure to be added to the list
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListPushHostElement(
                                              IN TransportMgr*                    pTransportMgr,
                                              IN TransportDNSList            *pDnsList,
                                              IN RvSipTransportDNSHostNameElement *pHostElement)
{
    TransportDNSHostNameElement hostNameElement;
    RvUint32                   retCode;
    RPOOL_ITEM_HANDLE           itemPtr;
    RvInt32                    addrOffset;
    RLIST_ITEM_HANDLE           listElem;
    RvUint32                   numOfElements;

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListPushHostElement - pushing element to list 0x%p", pDnsList));


    pDnsList->hHostNamesList = (pDnsList->hHostNamesList == NULL)?
        RLIST_RPoolListConstruct(pDnsList->hMemPool,pDnsList->hMemPage,
        sizeof(void*), NULL):pDnsList->hHostNamesList;

    if (pDnsList->hHostNamesList == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    retCode = RLIST_GetNumOfElements(NULL,pDnsList->hHostNamesList,
        &numOfElements);
    if ((pDnsList->maxElementsInSingleDnsList > 0) &&
        (pDnsList->maxElementsInSingleDnsList <= numOfElements))
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "TransportDNSListPushHostElement - maximum number of host elements exceeded (list=0x%p, max allowed=%d rv=%d)",
                  pDnsList,
                  pDnsList->maxElementsInSingleDnsList,
                  RV_ERROR_OUTOFRESOURCES));
        return RV_ERROR_OUTOFRESOURCES;
    }

    hostNameElement.port        = pHostElement->port;
    hostNameElement.priority    = pHostElement->priority;
    hostNameElement.weight      = pHostElement->weight;
    hostNameElement.protocol    = pHostElement->protocol;



    retCode = RPOOL_AppendFromExternal(pDnsList->hMemPool,
                                       pDnsList->hMemPage,
                                       pHostElement->hostName ,
                                       (RvInt)strlen(pHostElement->hostName)+1,
                                       RV_FALSE,
                                       &hostNameElement.hostNameOffset,
                                       &itemPtr);
#ifdef SIP_DEBUG
    hostNameElement.strHostName = (RvChar *)itemPtr;
#endif

    if(retCode != RV_OK)
    {
        return RV_ERROR_UNKNOWN;
    }

    /* Insert element into DNS list */
    retCode = RPOOL_Align(pDnsList->hMemPool,pDnsList->hMemPage);
    if(retCode != RV_OK)
    {
      return RV_ERROR_UNKNOWN;
    }

    retCode = RPOOL_AppendFromExternal(pDnsList->hMemPool,
                                      pDnsList->hMemPage,
                                      &hostNameElement,
                                      sizeof(TransportDNSHostNameElement),
                                      RV_TRUE,
                                      &addrOffset,
                                      &itemPtr);
    if(retCode != RV_OK)
    {
        return RV_ERROR_UNKNOWN;
    }

    retCode = RLIST_InsertHead(NULL, pDnsList->hHostNamesList,&listElem);
    if (retCode != RV_OK)
    {
        return RV_ERROR_UNKNOWN;
    }
    *(void **)listElem = (void *)itemPtr;

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListPushHostElement - element successfully pushed to list 0x%p", pDnsList));

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransportMgr);
#endif

    return RV_OK;
}


/***************************************************************************
 * TransportDNSListGetUsedSRVElement
 * ------------------------------------------------------------------------
 * General: retrieves SRV element, used to produce the IP list
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 * Output:  pHostElement    - host name element structure to be added to the list
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListGetUsedSRVElement(
    IN TransportMgr*                pTransportMgr,
    IN TransportDNSList                *pDnsList,
    IN RvSipTransportDNSSRVElement    *pSRVElement)
{
     if ((pTransportMgr == NULL) || (pDnsList == NULL) || (pSRVElement == NULL))
     {
         return RV_ERROR_BADPARAM;
     }

     if ('\0' == pDnsList->usedSRVElement.srvName[0])
     {
         return RV_ERROR_NOT_FOUND;
     }

     memcpy(pSRVElement,&pDnsList->usedSRVElement,
         sizeof(RvSipTransportDNSSRVElement));

     return RV_OK;
}

/***************************************************************************
 * TransportDNSListGetUsedHostElement
 * ------------------------------------------------------------------------
 * General: retrieves host name element, used to produce the IP list
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 * Output:  pHostElement    - host name element structure to be added to the list
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListGetUsedHostElement(
    IN  TransportMgr*                        pTransportMgr,
    IN  TransportDNSList                    *pDnsList,
    OUT RvSipTransportDNSHostNameElement    *pHostElement)
{
     if ((pTransportMgr == NULL) || (pDnsList == NULL) || (pHostElement == NULL))
     {
         return RV_ERROR_BADPARAM;
     }

     if ('\0' == pDnsList->usedHostNameElement.hostName[0])
     {
         return RV_ERROR_NOT_FOUND;
     }

     memcpy(pHostElement,&pDnsList->usedHostNameElement,
         sizeof(RvSipTransportDNSHostNameElement));

     return RV_OK;
}

/***************************************************************************
 * TransportDNSListGetUsedSRVElement
 * ------------------------------------------------------------------------
 * General: retrieves SRV element, used to produce the IP list
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 *          pHostElement    - host name element structure to be added to the list
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListSetUsedSRVElement(
    IN TransportMgr*                pTransportMgr,
    IN TransportDNSList                *pDnsList,
    IN RvSipTransportDNSSRVElement    *pSRVElement)
{
     if ((pTransportMgr == NULL) || (pDnsList == NULL) || (pSRVElement == NULL))
     {
         return RV_ERROR_BADPARAM;
     }

     memcpy(&pDnsList->usedSRVElement,pSRVElement,
         sizeof(RvSipTransportDNSSRVElement));

     return RV_OK;
}


/***************************************************************************
 * TransportDNSListSetUsedHostElement
 * ------------------------------------------------------------------------
 * General: sets host name element, used to produce the IP list
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 *          pHostElement    - host name element structure to be added to the list
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListSetUsedHostElement(
    IN TransportMgr*                    pTransportMgr,
    IN TransportDNSList                    *pDnsList,
    IN RvSipTransportDNSHostNameElement *pHostElement)
{
     if ((pTransportMgr == NULL) || (pDnsList == NULL) || (pHostElement == NULL))
     {
         return RV_ERROR_BADPARAM;
     }

     memcpy(&pDnsList->usedHostNameElement,pHostElement,
         sizeof(RvSipTransportDNSHostNameElement));

     return RV_OK;
}

#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/

/***************************************************************************
 * TransportDNSListGetIPElement
 * ------------------------------------------------------------------------
 * General: retrieves IP address element from the DNS list objects IP
 * addresses list according to specified by input location.
 * Return Value: RV_OK, RV_ERROR_UNKNOWN or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 *          location        - starting element location
 *          pRelative       - relative IP element for get next/previous
 * Output:  pIPElement      - found element
 *          pRelative       - new relative IP element for consequent
 *          call to the RvSipTransportDNSListGetIPElement function.
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListGetIPElement (
                                           IN     TransportMgr*          pTransportMgr,
                                           IN     TransportDNSList            *pDnsList,
                                           IN     RvSipListLocation             location,
                                           INOUT  void                          **pRelative,
                                           OUT    RvSipTransportDNSIPElement    *pIPElement)
{
    SipTransportAddress**         ipElement = NULL;

    if ((pDnsList == NULL) || (pDnsList->hIPAddrList == NULL) ||
        (pIPElement == NULL) || (pRelative == NULL))
    {
        return RV_ERROR_BADPARAM;
    }
    if (NULL == pTransportMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListGetIPElement - getting element from list 0x%p", pDnsList));

    switch(location)
    {
    case RVSIP_NEXT_ELEMENT:
        if (*pRelative != NULL)
        {
            RLIST_GetNext(NULL,pDnsList->hIPAddrList,
                (RLIST_ITEM_HANDLE)*pRelative,(RLIST_ITEM_HANDLE *)&ipElement);
            break;
        }
        /* In case pRelative is NULL, first element should be returned */
    case RVSIP_FIRST_ELEMENT:
        RLIST_GetHead (NULL,pDnsList->hIPAddrList,
            (RLIST_ITEM_HANDLE *)&ipElement);
        break;
    case RVSIP_PREV_ELEMENT:
        if (*pRelative != NULL)
        {
            RLIST_GetPrev(NULL,pDnsList->hIPAddrList,
                (RLIST_ITEM_HANDLE)*pRelative,(RLIST_ITEM_HANDLE *)&ipElement);
            break;
        }
        /* In case pRelative is NULL, last element should be returned */
    case RVSIP_LAST_ELEMENT:
        RLIST_GetTail (NULL,pDnsList->hIPAddrList, (RLIST_ITEM_HANDLE *)&ipElement);
        break;
    default:
        return RV_ERROR_UNKNOWN;
    }

    if (ipElement == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    *pRelative = (void *)ipElement;
    pIPElement->protocol = (*ipElement)->eTransport;


    switch ((*ipElement)->addr.addrtype)
    {
#if(RV_NET_TYPE & RV_NET_IPV6)
    case RV_ADDRESS_TYPE_IPV6:
        {
            pIPElement->port = (*ipElement)->addr.data.ipv6.port;
            pIPElement->bIsIpV6 = RV_TRUE;
            memcpy(pIPElement->ip,&((*ipElement)->addr.data.ipv6.ip),RVSIP_TRANSPORT_LEN_BYTES_IPV6);
        }
        break;
#endif /*(RV_NET_TYPE & RV_NET_IPV6) */
    case RV_ADDRESS_TYPE_IPV4:
        {
            pIPElement->port = (*ipElement)->addr.data.ipv4.port;
            pIPElement->bIsIpV6 = RV_FALSE;
            memcpy(pIPElement->ip,&((*ipElement)->addr.data.ipv4.ip),4);
        }
        break;
    default:
        return RV_ERROR_UNKNOWN;
    } /* switch (ipElement->type) */
    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListGetIPElement - got element successfully from list 0x%p", pDnsList));

    return RV_OK;
}

/***************************************************************************
 * TransportDNSListRemoveTopmostIPElement
 * ------------------------------------------------------------------------
 * General: removes topmost element from the head of the DNS list
 * object IP addresses list.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListRemoveTopmostIPElement(
                                           IN TransportMgr*          pTransportMgr,
                                           IN TransportDNSList            *pDnsList)
{
    RLIST_ITEM_HANDLE       element = NULL;

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListRemoveTopmostIPElement - removing element from list 0x%p", pDnsList));

    RLIST_GetHead (NULL,pDnsList->hIPAddrList,&element);
    if (element == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    RLIST_Remove (NULL,pDnsList->hIPAddrList,element);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransportMgr);
#endif

    return RV_OK;
}


/***************************************************************************
 * TransportDNSListPopIPElement
 * ------------------------------------------------------------------------
 * General: retrieves and removes from the IP addresses list of the DNS
 * list object the topmost IP address element.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport manager
 *          pDnsList        - handle of the DNS list object
 * Output:  pIpElement      - element
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListPopIPElement(
                                        IN  TransportMgr*          pTransportMgr,
                                        IN  TransportDNSList            *pDnsList,
                                        OUT RvSipTransportDNSIPElement  *pIPElement)
{
    RvStatus retCode;
    void      *pRelative;
    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListPopIPElement - popping element from list 0x%p", pDnsList));

    retCode = TransportDNSListGetIPElement (pTransportMgr,
                                            pDnsList,
                                            RVSIP_FIRST_ELEMENT,
                                            &pRelative,
                                            pIPElement);
    if (retCode != RV_OK)
    {
        return retCode;
    }
    return TransportDNSListRemoveTopmostIPElement(pTransportMgr, pDnsList);
}

/***************************************************************************
 * TransportDNSListPushIPElement
 * ------------------------------------------------------------------------
 * General: adds single IP address element to the head of the IP addresses list of the
 * DNS list object.
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport manager
 *          pDnsList        - pointer of the DNS list object
 *          pIPElement      - IP address element structure to be added to the list
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListPushIPElement(
                                           IN TransportMgr*          pTransportMgr,
                                           IN TransportDNSList            *pDnsList,
                                           IN RvSipTransportDNSIPElement    *pIPElement)

{
    SipTransportAddress         addrElement;
    RvUint32                   retCode;
    RPOOL_ITEM_HANDLE           itemPtr;
    RvInt32                    addrOffset;
    RLIST_ITEM_HANDLE           listElem;
    RvUint32                   numOfElements;

    pDnsList->hIPAddrList = (pDnsList->hIPAddrList == NULL)?
        RLIST_RPoolListConstruct(pDnsList->hMemPool,pDnsList->hMemPage,sizeof(void*), NULL):
        pDnsList->hIPAddrList;

    if (pDnsList->hIPAddrList == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    retCode = RLIST_GetNumOfElements(NULL,pDnsList->hIPAddrList, &numOfElements);

    if ((pDnsList->maxElementsInSingleDnsList > 0) &&
        (pDnsList->maxElementsInSingleDnsList <= numOfElements))
    {
        RvLogError(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
                  "TransportDNSListPushIPElement - maximum number of ip elements exceeded (list=0x%p, max allowed=%d rv=%d)",
                  pDnsList, pDnsList->maxElementsInSingleDnsList, RV_ERROR_OUTOFRESOURCES));

        return RV_ERROR_OUTOFRESOURCES;
    }

    addrElement.eTransport = pIPElement->protocol;
    switch (pIPElement->bIsIpV6)
    {
        case RV_FALSE:
            {
                addrElement.addr.data.ipv4.port        = pIPElement->port;
                addrElement.addr.addrtype = RV_ADDRESS_TYPE_IPV4;
                memcpy(&addrElement.addr.data.ipv4.ip,pIPElement->ip,4);

            }
            break;
#if(RV_NET_TYPE & RV_NET_IPV6)
        case RV_TRUE:
            {
                addrElement.addr.data.ipv6.port        = pIPElement->port;
                addrElement.addr.addrtype = RV_ADDRESS_TYPE_IPV6;
                memcpy(&addrElement.addr.data.ipv6.ip,pIPElement->ip,RVSIP_TRANSPORT_LEN_BYTES_IPV6);
            }
            break;
#endif /* (RV_NET_TYPE & RV_NET_IPV6)*/
        default:
            break;

    }/* switch (pIPElement->bIsIpV6) */

    /* Insert clone address element into DNS list */
    retCode = RPOOL_Align(pDnsList->hMemPool,pDnsList->hMemPage);
    if(retCode != RV_OK)
    {
      return RV_ERROR_UNKNOWN;
    }
    retCode = RPOOL_AppendFromExternal(pDnsList->hMemPool,
        pDnsList->hMemPage,&addrElement,sizeof(SipTransportAddress),
        RV_TRUE,&addrOffset,&itemPtr);
    if(retCode != RV_OK)
    {
        return RV_ERROR_UNKNOWN;
    }

    retCode = RLIST_InsertHead(NULL,pDnsList->hIPAddrList,&listElem);
    if (retCode != RV_OK)
    {
        return RV_ERROR_UNKNOWN;
    }
    *(void **)listElem = (void *)itemPtr;
    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListPushIPElement - element successfully pushed to list 0x%p", pDnsList));

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransportMgr);
#endif

    return RV_OK;
}

 /***********************************************************************************************************************
 * TransportDnsGetEnumResult
 * purpose : get the result on an ENUM NAPTR query
 * input   : hTransportMgr  - transport mgr
 *           hDnsList       - DNS list to set record to
 * output  : pEnumRes       - pointer to ENUM string
 * return  : RvStatus       - RV_OK or error
 *************************************************************************************************************************/
RvStatus RVCALLCONV TransportDnsGetEnumResult(
    IN TransportMgr*     pMgr,
    IN TransportDNSList* pDnsList,
    OUT RvChar**         pEnumRes)
{
    if (NULL == pDnsList->pENUMResult)
    {
        return RV_ERROR_NOT_FOUND;
    }
    *pEnumRes = pDnsList->pENUMResult;
    RV_UNUSED_ARG(pMgr);
    return RV_OK;
}

 /***********************************************************************************************************************
 * TransportDnsSetEnumResult
 * purpose : sets the result on an ENUM NAPTR query
 * input   : pMgr       - transport mgr
 *           pDnsData   - single record from the DNS query result
 * output  : pDnsList   - dns list to add record to
 * return  : RvStatus        - RV_OK or error
 *************************************************************************************************************************/
RvStatus RVCALLCONV TransportDnsSetEnumResult(
    IN TransportMgr*         pMgr,
    IN RvDnsData*            pDnsData,
    INOUT TransportDNSList*  pDnsList)
{
    RvInt32     addrOffset  = 0;
    RvChar*     strInPool   = NULL;
    
    /* Check input parameters */
    if ((pMgr == NULL) || (pDnsData == NULL) || (pDnsList == NULL))
    {
        return RV_ERROR_NULLPTR;
    }
    strInPool = RPOOL_AlignAppend(pDnsList->hMemPool,pDnsList->hMemPage,(RvInt32)strlen(pDnsData->data.dnsNaptrData.regexp)+1,&addrOffset);
    if (NULL == strInPool)
    {
        RvLogError(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "TransportDnsSetEnumResult - list 0x%p: failed to allocate space in pool",
            pDnsList));
        return RV_ERROR_UNKNOWN;        
    }
    RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
        "TransportDnsSetEnumResult - list 0x%p: setting enum result to %s",
        pDnsList,pDnsData->data.dnsNaptrData.regexp));
    pDnsList->pENUMResult = strInPool;

    strcpy(pDnsList->pENUMResult,pDnsData->data.dnsNaptrData.regexp);
    return RV_OK;
}
 
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/***************************************************************************
 * TransportDNSListGetNumberOfEntries
 * ------------------------------------------------------------------------
 * General: Retrives number of elements in each of the DNS list object lists.
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport manager
 *          pDnsList            - Handle of the DNS list object
 * Output:  pSrvElements        - number of SRV elements
 *          pHostNameElements   - number of host elements
 *          pIpAddrElements     - number of IP address elements
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListGetNumberOfEntries(
                                      IN TransportMgr*                  pTransportMgr,
                                      IN TransportDNSList               *pDnsList,
                                      OUT RvUint32                     *pSrvElements,
                                      OUT RvUint32                     *pHostNameElements,
                                      OUT RvUint32                     *pIpAddrElements)
{
    RvStatus           retCode     = RV_OK;

    RvLogDebug(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
              "TransportDNSListGetNumberOfEntries - getting element from list 0x%p", pDnsList));

    *pSrvElements = *pHostNameElements = *pIpAddrElements = 0;

    retCode = RLIST_GetNumOfElements(NULL,pDnsList->hSrvNamesList,  pSrvElements);

    retCode = (retCode == RV_OK)?RLIST_GetNumOfElements(NULL,
        pDnsList->hHostNamesList,pHostNameElements):retCode;

    retCode = (retCode == RV_OK)?RLIST_GetNumOfElements(NULL,
        pDnsList->hIPAddrList,pIpAddrElements):retCode;

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransportMgr);
#endif

    return retCode;
}

 /***********************************************************************************************************************
 * TransportDnsAddNAPTRRecord
 * purpose : Adds NAPTR record into the DNS list. Note that the new element is added
 *           in correct sorting order.
 * input   : pMgr       - transport mgr
 *           pDnsData   - single record from the DNS query result
 *           bIsSecure      - Allow ONLY Secure resilts (i.e. TLS)
 *
 * output  : pDnsList   - dns list to add record to
 * return  : RvStatus        - RV_OK or error
 *************************************************************************************************************************/
RvStatus TransportDnsAddNAPTRRecord(
    IN TransportMgr*         pMgr,
    IN RvDnsData*            pDnsData,
    IN RvBool                bIsSecure,
    INOUT TransportDNSList*  pDnsList)
{
    RPOOL_ITEM_HANDLE        itemPtr;
    RLIST_ITEM_HANDLE        listElem;
    RvUint32                 retCode;
    RvInt32                  addrOffset;
    TransportDNSSRVElement   srvElement;
    RLIST_ITEM_HANDLE        upstreamNeighbor;
    RvChar*                  strDnsName;
    RvUint32                 Count4, Count6;


    /* Check input parameters */
    if ((pMgr == NULL) || (pDnsData == NULL) || (pDnsList == NULL))
    {
        return RV_ERROR_NULLPTR;
    }

    /* Convert DNS record to the SIP SRV list element */
    srvElement.order = pDnsData->data.dnsNaptrData.order;
    srvElement.preference = pDnsData->data.dnsNaptrData.preference;

    if (SipCommonStricmp(SRV_DNS_NAPTR_TCP_STR,
        pDnsData->data.dnsNaptrData.service) == RV_TRUE)
    {
        srvElement.protocol = RVSIP_TRANSPORT_TCP;
    }
    else if (SipCommonStricmp(SRV_DNS_NAPTR_UDP_STR,
        pDnsData->data.dnsNaptrData.service) == RV_TRUE)
    {
        srvElement.protocol = RVSIP_TRANSPORT_UDP;
    }
    else if (SipCommonStricmp(SRV_DNS_NAPTR_TLS_STR,
        pDnsData->data.dnsNaptrData.service) == RV_TRUE)
    {
        srvElement.protocol = RVSIP_TRANSPORT_TLS;
    }
#if (RV_NET_TYPE & RV_NET_SCTP)
    else if (SipCommonStricmp(SRV_DNS_NAPTR_SCTP_STR,
        pDnsData->data.dnsNaptrData.service) == RV_TRUE)
    {
        srvElement.protocol = RVSIP_TRANSPORT_SCTP;
    }
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    else
    {
        return RV_ERROR_BADPARAM;
    }


    RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
        "TransportDnsAddNAPTRRecord - list 0x%p: adding NAPTR %s,%s, proto=%s, ord=%u, pref=%u",
        pDnsList,pDnsData->data.dnsNaptrData.service,pDnsData->data.dnsNaptrData.replacement,
        SipCommonConvertEnumToStrTransportType(srvElement.protocol),
        srvElement.order,srvElement.preference));

    TransportMgrCounterLocalAddrsGet(pMgr,srvElement.protocol,&Count4,&Count6);
    if ((Count6+Count4) == 0)
    {
        RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "TransportDnsAddNAPTRRecord - list 0x%p: Transport %s is not supported. ignoring",
            pDnsList,SipCommonConvertEnumToStrTransportType(srvElement.protocol)));
        return RV_OK;
    }
    if (bIsSecure == RV_TRUE && RVSIP_TRANSPORT_TLS != srvElement.protocol)
    {
        RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
            "TransportDnsAddNAPTRRecord - list 0x%p: only TLS recordes are allowed. ignoring",
            pDnsList));
        return RV_OK;
    }

    retCode = RPOOL_AppendFromExternal(pDnsList->hMemPool,
        pDnsList->hMemPage,pDnsData->data.dnsNaptrData.replacement,
        (RvInt)strlen(pDnsData->data.dnsNaptrData.replacement)+1,RV_FALSE,
        &srvElement.nameOffset,
        (RPOOL_ITEM_HANDLE *)&strDnsName);
#ifdef SIP_DEBUG
    srvElement.strSrvName = strDnsName;
#endif /* #ifdef SIP_DEBUG*/

    if(retCode != RV_OK)
    {
        return retCode;
    }


    /* Copy entire SRV element to the memory page */
    retCode = RPOOL_Align(pDnsList->hMemPool,pDnsList->hMemPage);
    if(retCode != RV_OK)
    {
        return RV_ERROR_UNKNOWN;
    }
    retCode = RPOOL_AppendFromExternal(pDnsList->hMemPool,
        pDnsList->hMemPage,&srvElement,
        sizeof(TransportDNSSRVElement),RV_TRUE,&addrOffset,&itemPtr);

    if (retCode != RV_OK)
    {
        return RV_ERROR_UNKNOWN;
    }

    pDnsList->hSrvNamesList = (pDnsList->hSrvNamesList == NULL)?
        RLIST_RPoolListConstruct(pDnsList->hMemPool,pDnsList->hMemPage,
        sizeof(void*), NULL):pDnsList->hSrvNamesList;

    if (NULL == pDnsList->hSrvNamesList)
    {
        return RV_ERROR_UNKNOWN;
    }

    /* Find if there are records with higher priority */
    retCode = DnsNaptrFindHigherPriorityRecord(srvElement.order,
        srvElement.preference,pDnsList->hSrvNamesList,&upstreamNeighbor);
    if (retCode != RV_OK)
    {
        return retCode;
    }

    if (upstreamNeighbor != NULL)
    {
        /* Insert new item after more important one */
        retCode = RLIST_InsertAfter (NULL,pDnsList->hSrvNamesList,upstreamNeighbor,&listElem);
    }
    else
    {
        /* Create new entry at the tail of SRV list */
        retCode = RLIST_InsertHead(NULL,pDnsList->hSrvNamesList,&listElem);
    }

    if (retCode != RV_OK)
    {
        return retCode;
    }


    /*    Insert pointer to memory buffer where query structure was
    saved, into the list of next hop structures */
    *(void **)listElem = (void *)itemPtr;

    return RV_OK;
}

/***********************************************************************************************************************
 * TransportDnsAddSRVRecord
 * purpose : Adds SRV record into the DNS list. Note that the new element is added
 *           in correct sorting order.
 * input   : pMgr           - transport mgr
 *           pDnsData       - single record from the DNS query result
 *           eTransport     - transport of the added record
 * output  : pDnsList       - dns list to add record to
 * return  : RvStatus        - RV_OK or error
 *************************************************************************************************************************/
RvStatus TransportDnsAddSRVRecord(
    IN TransportMgr*         pMgr,
    IN RvDnsData*            pDnsData,
    IN RvSipTransport        eTransport,
    INOUT TransportDNSList*  pDnsList)
{
    RvStatus                        rv        = RV_ERROR_UNKNOWN;
    RvInt32                         addrOffset;
    RPOOL_ITEM_HANDLE               itemPtr;
    RLIST_ITEM_HANDLE               listElem;
    TransportDNSHostNameElement     hostNameElement;
    RvChar*                         strHostName;
    RLIST_ITEM_HANDLE               upstreamNeighbor;
    RvBool                          bInsertBefore = RV_FALSE;

    /* Check input parameters */
    if ((pMgr == NULL) || (pDnsData == NULL) || (pDnsList == NULL))
    {
        return RV_ERROR_BADPARAM;
    }

    /* Convert DNS record to the SIP host name list element */
    hostNameElement.protocol    = eTransport;
    hostNameElement.port        = pDnsData->data.dnsSrvData.port;
    hostNameElement.priority    = pDnsData->data.dnsSrvData.priority;
    hostNameElement.weight      = pDnsData->data.dnsSrvData.weight;

    RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
        "TransportDnsAddSRVRecord - list 0x%p: adding SRV %s, port=%u, prio=%u, weight=%u",
        pDnsList,pDnsData->data.dnsSrvData.targetName,hostNameElement.port,
        hostNameElement.priority,hostNameElement.weight));

    rv = RPOOL_AppendFromExternal(pDnsList->hMemPool,
        pDnsList->hMemPage,pDnsData->data.dnsSrvData.targetName,
        (RvInt)strlen(pDnsData->data.dnsSrvData.targetName)+1,RV_FALSE,
        &hostNameElement.hostNameOffset,
        (RPOOL_ITEM_HANDLE *)&strHostName);
    if(rv != RV_OK)
    {
        return rv;
    }
#ifdef SIP_DEBUG
    hostNameElement.strHostName = strHostName;
#endif

    rv = RPOOL_Align(pDnsList->hMemPool,pDnsList->hMemPage);
    if(rv != RV_OK)
    {
        return RV_ERROR_UNKNOWN;
    }

    rv = RPOOL_AppendFromExternal(pDnsList->hMemPool,
        pDnsList->hMemPage,&hostNameElement,
        sizeof(TransportDNSHostNameElement),
        RV_TRUE,&addrOffset,&itemPtr);
    if(rv != RV_OK)
    {
        return RV_ERROR_UNKNOWN;
    }

    pDnsList->hHostNamesList = (pDnsList->hHostNamesList == NULL)?
        RLIST_RPoolListConstruct(pDnsList->hMemPool,pDnsList->hMemPage,
        sizeof(void*), NULL):pDnsList->hHostNamesList;

    rv = DnsSrvFindHigherPriorityRecord(pMgr,hostNameElement.priority,
        hostNameElement.weight,pDnsList->hHostNamesList,
        &upstreamNeighbor,&bInsertBefore);
    if (rv !=RV_OK)
    {
        return rv;
    }

    if (NULL == upstreamNeighbor)
    {
        /* Create new entry at the tail of the list */
        rv = RLIST_InsertHead(NULL,
        pDnsList->hHostNamesList,
        &listElem);
    }
    else if (RV_TRUE == bInsertBefore)
    {
        /* Insert new item before less important one */
        rv = RLIST_InsertBefore(NULL,
        pDnsList->hHostNamesList,
        upstreamNeighbor,&listElem);
    }
    else
    {
        rv = RLIST_InsertAfter(NULL,
        pDnsList->hHostNamesList,
        upstreamNeighbor,&listElem);
    }

    if (rv != RV_OK)
    {
        return rv;
    }

    *(void **)listElem = (void *)itemPtr;

    return RV_OK;
}

#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */


/***************************************************************************
 * TransportDNSGetMaxElements
 * ------------------------------------------------------------------------
 * General: returns maximum number of DNS list entries
 * Return Value: RvUint32 - maximum number of DNS list entries
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransport - handle to the transport
 *
 ***************************************************************************/
RvUint32 RVCALLCONV TransportDNSGetMaxElements(
                                    IN  TransportMgr        *pTransportMgr)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    return pTransportMgr->maxElementsInSingleDnsList;
#else
    RV_UNUSED_ARG(pTransportMgr);

    return 1;
#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/
}

/***********************************************************************************************************************
 * TransportDnsAddIPRecord
 * purpose : Adds IP record into the DNS list. Note that the new element is added
 *           in correct sorting order.
 * input   : pMgr       - transport mgr
 *           pDnsData   - single record from the DNS query result
 *           eTransport - transport of record
 *           port       - port of record
 *
 * output  : pDnsList   - dns list to add record to
 * return  : RvStatus        - RV_OK or error
 *************************************************************************************************************************/
RvStatus TransportDnsAddIPRecord(
    IN TransportMgr*         pMgr,
    IN RvDnsData*            pDnsData,
    IN RvSipTransport        eTransport,
    IN RvUint16              port,
    INOUT TransportDNSList*  pDnsList)
{
    RvStatus            retCode;
    SipTransportAddress addr;
    RvInt32             addrOffset  = 0;
    RPOOL_ITEM_HANDLE   itemPtr     = NULL;
    RLIST_ITEM_HANDLE   listElem    = NULL;
#if (RV_LOGMASK & RV_LOGLEVEL_DEBUG)
    RvChar              logip[RVSIP_TRANSPORT_LEN_STRING_IP];
#endif

    if ((pMgr == NULL) || (pDnsList == NULL) || (pDnsData == NULL))
    {
        return RV_ERROR_UNKNOWN;
    }


    /* Allocate memory on the page from input and copy there
    the IP address structure */
    retCode = RPOOL_Align(pDnsList->hMemPool,pDnsList->hMemPage);
    if(retCode != RV_OK)
    {
        return retCode;
    }

    memcpy(&addr.addr,&pDnsData->data.hostAddress,sizeof(RvAddress));
    addr.eTransport = eTransport;

    switch (pDnsData->dataType)
    {
    case RV_DNS_HOST_IPV4_TYPE:
        addr.addr.addrtype = RV_ADDRESS_TYPE_IPV4;
        addr.addr.data.ipv4.port = port;
        break;
#if(RV_NET_TYPE & RV_NET_IPV6)
    case RV_DNS_HOST_IPV6_TYPE:
        addr.addr.addrtype = RV_ADDRESS_TYPE_IPV6;
        addr.addr.data.ipv6.port = port;
        break;
#endif
    default:
        break;
    }

    RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
        "TransportDnsAddIPRecord - list 0x%p: adding IP %s:%u, proto=%s",
        pDnsList,RvAddressGetString(&addr.addr,RVSIP_TRANSPORT_LEN_STRING_IP,(RvChar*)logip),RvAddressGetIpPort(&addr.addr),
        SipCommonConvertEnumToStrTransportType(addr.eTransport)));

    retCode = RPOOL_AppendFromExternal(pDnsList->hMemPool,
        pDnsList->hMemPage,&addr,sizeof(SipTransportAddress),
        RV_TRUE,&addrOffset,&itemPtr);
    if(retCode != RV_OK)
    {
        return retCode;
    }

    /* Allocate IP addresses DNS list */
    pDnsList->hIPAddrList = (pDnsList->hIPAddrList == NULL)?
        RLIST_RPoolListConstruct(pDnsList->hMemPool,pDnsList->hMemPage, sizeof(void*), NULL) : pDnsList->hIPAddrList;
    if (pDnsList->hIPAddrList == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }


    /* Add new IP address to the end of the address list. */
    retCode = RLIST_InsertTail(NULL,pDnsList->hIPAddrList,&listElem);
    if (retCode != RV_OK)
    {
        return retCode;
    }
    *(void **)listElem = (void *)itemPtr;

    return RV_OK;
}

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/*****************************************************************************
* TransportDnsListPrepare
* ---------------------------------------------------------------------------
* General: 1. Removes TLS if client does not support TLS
*          2. Removes non-TLS if transport supposed to be TSL (sips URI)
*          3. Does nothing if client supports TLS and uri was "sip"
*		   4. Remove an addtional SRV record with different Order value 
*			  according RFC2915: "Low (Order) numbers are processed before 
*			  high numbers, and once a NAPTR is found whose rule "matches" 
*			  the target, the client MUST NOT consider any NAPTRs with a  
*			  higher value for order."
*
* Return Value: RvStatus - RvStatus
*                           RV_ERROR_UNKNOWN
*                           RV_ERROR_BADPARAM
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pTransportMgr        - pointer to transport manager
*          strHostName          - the host name we handle
*          hSrvNamesList        - the list that holds the srv records
*****************************************************************************/
void RVCALLCONV TransportDnsListPrepare(
      IN  TransportMgr*       pTransportMgr,
      IN  RvChar*             strHostName,
      IN  RLIST_HANDLE        hSrvNamesList)
{
    TransportDNSSRVElement  **pCurElement      = NULL;
    RvInt32                order               = -1;
    RvInt32                goodRecords         = 0;

    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
        "TransportDnsListPrepare - Removing irrelevant records from Srv List 0x%x for host %s",
            hSrvNamesList,strHostName));


    RLIST_GetHead(NULL,hSrvNamesList, (RLIST_ITEM_HANDLE *)&pCurElement);

    while (NULL != pCurElement)
    {
        TransportDNSSRVElement** pTmpElem;
        RvBool                   bRemove = RV_FALSE;
        pTmpElem = pCurElement;

        RLIST_GetNext(NULL,hSrvNamesList, (RLIST_ITEM_HANDLE)pCurElement,(RLIST_ITEM_HANDLE*)&pCurElement);

        /* if we have enough records we accept no more records */
        if (goodRecords >= (RvInt)pTransportMgr->maxElementsInSingleDnsList)
        {
            bRemove = RV_TRUE;
        }

        if (-1 != order && pTmpElem[0]->order != order)
        {
			RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
				"TransportDnsListPrepare - Found a SRV record (0x%x) with higher order(%d) than the first order (%d)",
				pTmpElem,order,pTmpElem[0]->order));
            bRemove = RV_TRUE;
        }

        /* if the order is not of the first matched order */
        if (bRemove == RV_FALSE)
        {
            goodRecords ++;
            if (-1 == order)
            {
                order = pTmpElem[0]->order;
            }
        }
        else
        {
			RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
				"TransportDnsListPrepare - Removing DNS element 0x%x from SRV DNS list 0x%x",
				pTmpElem,hSrvNamesList));
            RLIST_Remove(NULL,hSrvNamesList,(RLIST_ITEM_HANDLE)pTmpElem);
        }
    }

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(strHostName);
#endif

    return;
}
#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/


/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
* DnsCloneIPAddressList
* ------------------------------------------------------------------------
* General: Copy entire IP list between two DNS list objects
* ------------------------------------------------------------------------
* Arguments:
* Input:    hDnsListOriginal - Original DNS list object
* Output:   hDnsListClone    - Destination DNS list object
***************************************************************************/
static RvStatus RVCALLCONV DnsCloneIPAddressList(IN  TransportDNSList    *dnsOrigList,
                                                 OUT TransportDNSList    *dnsCloneList)
{
    RvStatus           rv        = RV_OK;
    RvInt32            addrOffset;
    SipTransportAddress **addr = NULL;
    RPOOL_ITEM_HANDLE   itemPtr;
    RLIST_ITEM_HANDLE   listElem;

    /* Check Input and initialization */
    if (dnsCloneList == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    if (dnsOrigList == NULL)
    {
        dnsCloneList->hIPAddrList = NULL;
        return RV_OK;
    }

    /* In case no IP list exists in original DNS list,
    the clone DNS list IP list should be NULL */
    if (dnsOrigList->hIPAddrList == NULL)
    {
        dnsCloneList->hIPAddrList = NULL;
        return RV_OK;
    }
    /* In case there is IP list in original DNS list object
    we should first create the IP list in the destination object */
    dnsCloneList->hIPAddrList = RLIST_RPoolListConstruct(dnsCloneList->hMemPool,
        dnsCloneList->hMemPage, sizeof(void*), NULL);

    if (dnsCloneList->hIPAddrList == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    /* Loop for each element in the original IP list */
    RLIST_GetHead(NULL,dnsOrigList->hIPAddrList,
        (RLIST_ITEM_HANDLE *)&addr);

    while (addr != NULL)
    {
        /* Allocate memory on the clone object memory page */
        rv = RPOOL_Align(dnsCloneList->hMemPool,
                      dnsCloneList->hMemPage);
        if (rv != RV_OK)
        {
            return rv;
        }
        rv = RPOOL_AppendFromExternal(dnsCloneList->hMemPool,
            dnsCloneList->hMemPage,*addr,sizeof(SipTransportAddress),
            RV_TRUE,&addrOffset,&itemPtr);
        if(rv != RV_OK)
        {
            return RV_ERROR_UNKNOWN;
        }
        /*  Insert IP address to the end of address list. */
        rv = RLIST_InsertTail(NULL,dnsCloneList->hIPAddrList,&listElem);
        if (rv != RV_OK)
        {
            return RV_ERROR_UNKNOWN;
        }
        *(void **)listElem = (void *)itemPtr;
        /* Lookup for next element in the original IP list */
        RLIST_GetNext(NULL,dnsOrigList->hIPAddrList,
            (RLIST_ITEM_HANDLE)addr,(RLIST_ITEM_HANDLE *)&addr);
    }

    return rv;
}


#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/***************************************************************************
* DnsCloneHostNameList
* ------------------------------------------------------------------------
* General: Copy entire Host Name list between two DNS list objects
* ------------------------------------------------------------------------
* Arguments:
* Input:    hDnsListOriginal - Original DNS list object
* Output:   hDnsListClone    - Destination DNS list object
***************************************************************************/
static RvStatus RVCALLCONV DnsCloneHostNameList(IN  TransportDNSList            *dnsOrigList,
                                               OUT TransportDNSList            *dnsCloneList)
{
    RvStatus                      retCode       = RV_OK;
    RvInt32                       addrOffset;
    TransportDNSHostNameElement **nextHopOrig = NULL;
    TransportDNSHostNameElement   nextHopClone;
    RPOOL_ITEM_HANDLE             itemPtr;
    RLIST_ITEM_HANDLE             listElem;
    RvInt32                       srvNameLen; /* Can receive negative value */

    /* Check Input and initialization */
    if (dnsCloneList == NULL)
        return RV_ERROR_UNKNOWN;

    if (dnsOrigList == NULL)
    {
        dnsCloneList->hHostNamesList = NULL;
        return RV_OK;
    }

    if (dnsOrigList->hHostNamesList == NULL)
    {
        dnsCloneList->hHostNamesList = NULL;
        return RV_OK;
    }
    /* Construct clone host name list when it is clear that original list
    exists */
    dnsCloneList->hHostNamesList = RLIST_RPoolListConstruct(dnsCloneList->hMemPool,
        dnsCloneList->hMemPage, sizeof(void*), NULL);
    if (dnsCloneList->hHostNamesList == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    /* Loop till end of the original host name list */
    RLIST_GetHead(NULL,dnsOrigList->hHostNamesList,
        (RLIST_ITEM_HANDLE *)&nextHopOrig);

    while (nextHopOrig != NULL)
    {
        /* Initiate most values of the clone host name list element*/
        memcpy(&nextHopClone,*nextHopOrig,sizeof(TransportDNSHostNameElement));
        /* Copy host name */
        srvNameLen = RPOOL_Strlen(dnsOrigList->hMemPool,dnsOrigList->hMemPage,
            nextHopOrig[0]->hostNameOffset);
        if (srvNameLen == -1 || retCode != RV_OK)
        {
            break;
        }

        retCode = RPOOL_Append(dnsCloneList->hMemPool,
            dnsCloneList->hMemPage, srvNameLen+1, RV_FALSE,
            &nextHopClone.hostNameOffset);
        if(retCode != RV_OK)
        {
            break;
        }

        retCode = RPOOL_CopyFrom(dnsCloneList->hMemPool,
                                 dnsCloneList->hMemPage,
                                 nextHopClone.hostNameOffset,
                                 dnsOrigList->hMemPool,
                                 dnsOrigList->hMemPage,
                                 nextHopOrig[0]->hostNameOffset,
                                 srvNameLen+1);
        if(retCode != RV_OK)
        {
            break;
        }
        /* Add clone element to memory page of clone DNS list
        and insert it into the list */
        retCode = RPOOL_Align(dnsCloneList->hMemPool,
                      dnsCloneList->hMemPage);
        if (retCode != RV_OK)
        {
            return retCode;
        }
        retCode = RPOOL_AppendFromExternal(dnsCloneList->hMemPool,
                                           dnsCloneList->hMemPage,
                                           &nextHopClone,
                                           sizeof(TransportDNSHostNameElement),
                                           RV_TRUE,
                                           &addrOffset,
                                           &itemPtr);
        if(retCode != RV_OK)
        {
            break;
        }
        /*  Insert host elements to the end of the list. */
        retCode = RLIST_InsertTail(NULL,
            dnsCloneList->hHostNamesList,&listElem);
        if (retCode != RV_OK)
        {
            break;
        }
        *(void **)listElem = (void *)itemPtr;
        /* Retrive next original element */
        RLIST_GetNext(NULL,dnsOrigList->hHostNamesList,
            (RLIST_ITEM_HANDLE)nextHopOrig,(RLIST_ITEM_HANDLE *)&nextHopOrig);
    }

    return retCode;
}


/***************************************************************************
* DnsCloneSrvList
* ------------------------------------------------------------------------
* General: Copy entire SRV names list between two DNS list objects
* ------------------------------------------------------------------------
* Arguments:
* Input:    hDnsListOriginal - Original DNS list object
* Output:   hDnsListClone    - Destination DNS list object
***************************************************************************/
static RvStatus RVCALLCONV DnsCloneSrvList(IN  TransportDNSList    *dnsOrigList,
                                          OUT TransportDNSList    *dnsCloneList)
{
    RvStatus           retCode        = RV_OK;
    RvInt32            addrOffset;
    TransportDNSSRVElement **srvOrig = NULL;
    TransportDNSSRVElement srvClone;
    RPOOL_ITEM_HANDLE   itemPtr;
    RLIST_ITEM_HANDLE   listElem;
    RvInt32             srvNameLen; /* Can receive negative value */

    /* Check Input and initialization */
    if (dnsCloneList == NULL)
        return RV_ERROR_UNKNOWN;

    if (dnsOrigList == NULL)
    {
        dnsCloneList->hSrvNamesList = NULL;
        return RV_OK;
    }

    if (dnsOrigList->hSrvNamesList == NULL)
    {
        dnsCloneList->hSrvNamesList = NULL;
        return RV_OK;
    }

    /* Construct clone SRV list in case original SRV list exists */
    dnsCloneList->hSrvNamesList = RLIST_RPoolListConstruct(dnsCloneList->hMemPool,
        dnsCloneList->hMemPage, sizeof(void*), NULL);
    if (dnsCloneList->hSrvNamesList == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    /* Loop for original SRV elements */
    RLIST_GetHead(NULL,dnsOrigList->hSrvNamesList,
        (RLIST_ITEM_HANDLE *)&srvOrig);

    while (srvOrig != NULL)
    {
        /* Initiate most parameters of the clone SRV element */
        memcpy(&srvClone,*srvOrig,sizeof(TransportDNSSRVElement));
        /* Clone SRV name */
        srvNameLen = RPOOL_Strlen(dnsOrigList->hMemPool,dnsOrigList->hMemPage,
            srvOrig[0]->nameOffset);
        if (srvNameLen == -1 || retCode != RV_OK)
        {
            break;
        }

        retCode = RPOOL_Append(dnsCloneList->hMemPool,
            dnsCloneList->hMemPage, srvNameLen+1, RV_FALSE,
            &srvClone.nameOffset);
        if(retCode != RV_OK)
        {
            break;
        }

        retCode = RPOOL_CopyFrom(dnsCloneList->hMemPool,
            dnsCloneList->hMemPage ,srvClone.nameOffset,
            dnsOrigList->hMemPool,dnsOrigList->hMemPage,
            srvOrig[0]->nameOffset,srvNameLen+1);
        if(retCode != RV_OK)
        {
            break;
        }
        /* Insert clone SRV element into clone DNS list */
        retCode = RPOOL_Align(dnsCloneList->hMemPool,dnsCloneList->hMemPage);
        if (retCode != RV_OK)
        {
            return retCode;
        }
        retCode = RPOOL_AppendFromExternal(dnsCloneList->hMemPool,
            dnsCloneList->hMemPage,&srvClone,sizeof(TransportDNSSRVElement),
            RV_TRUE,&addrOffset,&itemPtr);
        if(retCode != RV_OK)
        {
            break;
        }
        /*  Insert host elements to the end of the list. */
        retCode = RLIST_InsertTail(NULL,
            dnsCloneList->hSrvNamesList,&listElem);
        if (retCode != RV_OK)
        {
            break;
        }
        *(void **)listElem = (void *)itemPtr;
        /* Retrive next original SRV element */
        RLIST_GetNext(NULL,dnsOrigList->hSrvNamesList,
            (RLIST_ITEM_HANDLE)srvOrig,(RLIST_ITEM_HANDLE *)&srvOrig);
    }

    return retCode;
}

/***********************************************************************************************************************
 * DnsSrvFindHigherPriorityRecord
 * purpose : This function retrieves SRV DNS Record with priority and weight,
 *             higher than specified by the input.
 * input   : priority       - priority of the new element
 *           weight         - weight of the new element
 *           hList            - SRV elements list
 * output  : pElement        - single srv element structure, filled by DNS result
 *           pbInsertBefore  - RV_TRUE - insert before pElement
 *                             RV_FALSE - insert after pElement
 * return  : RvStatus        - RV_OK, RV_ERROR_BADPARAM
 *************************************************************************************************************************/
static RvStatus DnsSrvFindHigherPriorityRecord(
                                        IN  TransportMgr*       pMgr,
                                        IN  RvUint16            priority,
                                        IN  RvUint32            weight,
                                        IN  RLIST_HANDLE        hList,
                                        OUT RLIST_ITEM_HANDLE    *pElement,
                                        OUT RvBool              *pbInsertBefore)
{
    TransportDNSHostNameElement** inlistHost        = NULL;
    TransportDNSHostNameElement** nextHost          = NULL;
    TransportDNSHostNameElement** pFirstInMyGroup      = NULL;
    RvRandom                      position          = 0;
    TransportDNSHostNameElement** pFirstBiggerPrio  = NULL;

    if ((hList == NULL) ||(pElement == NULL))
    {
        return RV_ERROR_BADPARAM;
    }

    *pElement = NULL;


    RLIST_GetHead(NULL,hList,(RLIST_ITEM_HANDLE *)&nextHost);

    /* the list is empty, return */
    if (NULL == nextHost)
    {
        return RV_OK;
    }

    /* the first group has bigger priority than specified element, insert at head */
    if (nextHost[0]->priority > priority)
    {
        return RV_OK;
    }

    /* first sum up the weights of the relevent priority and save some info*/
    while (NULL != nextHost)
    {
        inlistHost = nextHost;

        /* if we are in the right priority add to sum */
        if (inlistHost[0]->priority == priority)
        {
            /* save the head of the priority group */
            pFirstInMyGroup = (NULL == pFirstInMyGroup) ? inlistHost : pFirstInMyGroup;
        }
        /* if we are "over" the priority, stop */
        else if (inlistHost[0]->priority > priority)
        {
            pFirstBiggerPrio = inlistHost;
            break;
        }

        RLIST_GetNext(NULL,hList,(RLIST_ITEM_HANDLE)inlistHost,
            (RLIST_ITEM_HANDLE *)&nextHost);
    }


    /* it is the first of a priority group,
       insert it before the first element of the next prio group */
    if (NULL == pFirstInMyGroup)
    {
        /* there is a group with a bigger priority, insert before it */
        if (pFirstBiggerPrio != NULL)
        {
            *pElement = (RLIST_ITEM_HANDLE)pFirstBiggerPrio;
            *pbInsertBefore = RV_TRUE;
        }
        /* this is the highest priority member in the list,
           insert it after the last member of the list */
        else
        {
            *pElement = (RLIST_ITEM_HANDLE)inlistHost;
            *pbInsertBefore = RV_FALSE;
        }
        return RV_OK;
    }


    nextHost = pFirstInMyGroup;

    while(nextHost != NULL)
    {
        inlistHost = nextHost;

        if (inlistHost[0]->priority > priority)
        {
            *pElement = (RLIST_ITEM_HANDLE)nextHost;
            *pbInsertBefore = RV_TRUE;
            return RV_OK;
        }

        /* we have acummelated some weight, lets position the element in the right
           place in the priority group */
        RvRandomGeneratorGetInRange(pMgr->seed,(inlistHost[0]->weight+weight),&position);

        if (position < weight)
        {
            *pElement = (RLIST_ITEM_HANDLE)nextHost;
            *pbInsertBefore = RV_TRUE;
            return RV_OK;
        }


        RLIST_GetNext(NULL,hList,(RLIST_ITEM_HANDLE)inlistHost,
            (RLIST_ITEM_HANDLE *)&nextHost);
    }

    *pElement = (RLIST_ITEM_HANDLE)inlistHost;
    *pbInsertBefore = RV_FALSE;
    return RV_OK;
}

/***********************************************************************************************************************
 * DnsNaptrFindHigherPriorityRecord
 * purpose : This function finds NAPTR record with higher than specified by the input priority.
 * input   : order            - order of the new element
 *           preference        - preference of the new element
 *           hList            - SRV elements list
 * output  : pSrvElement    - single SRV element structure, filled by DNS result
 * return  : RvStatus        - RV_OK or error
 *************************************************************************************************************************/
static RvStatus DnsNaptrFindHigherPriorityRecord(
    IN  RvUint16            order,
    IN  RvUint16            preference,
    IN  RLIST_HANDLE        hList,
    OUT RLIST_ITEM_HANDLE    *pSrvElement)
{
    TransportDNSSRVElement  **srvElement = NULL;

    if ((hList == NULL) ||(pSrvElement == NULL))
        return RV_ERROR_BADPARAM;

    RLIST_GetTail(NULL,hList,(RLIST_ITEM_HANDLE *)&srvElement);

    while(srvElement != NULL)
    {
        if ((srvElement[0]->order < order) ||
            ((srvElement[0]->order == order) &&
            (srvElement[0]->preference <= preference)))
        {
            *pSrvElement = (RLIST_ITEM_HANDLE)srvElement;
            return RV_OK;
        }
        RLIST_GetPrev(NULL,hList,(RLIST_ITEM_HANDLE)srvElement,
            (RLIST_ITEM_HANDLE *)&srvElement);
    }

    *pSrvElement = NULL;
    return RV_OK;
}
#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/

#ifdef __cplusplus
}
#endif

