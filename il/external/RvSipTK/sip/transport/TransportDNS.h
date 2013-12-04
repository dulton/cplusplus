/************************************************************************************************************************
NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

************************************************************************************************************************/



/*********************************************************************************
 *                              TransportDNS.h
 *
 *  This file provides headers for the transport layer of the sip stack.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *
 *********************************************************************************/


#ifndef TRANSPORT_DNS_H
#define TRANSPORT_DNS_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "TransportMgrObject.h"
#include "RvSipTransportDNSTypes.h"
#include "rvares.h"
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/



#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT

/*
 *    SIP specific NAPTR record services
 */
#define SRV_DNS_NAPTR_TCP_STR  "SIP+D2T"
#define SRV_DNS_NAPTR_UDP_STR  "SIP+D2U"
#define SRV_DNS_NAPTR_TLS_STR  "SIPS+D2T"
#define SRV_DNS_NAPTR_SCTP_STR "SIP+D2S"


/* TransportDNSHostNameElement
 * --------------------------------------------------------------------------
 * TransportDNSHostNameElement defines the DNS host name list element, which keeps information
 * according to single host name that may be retrived by the SRV DNS query.
 *
 *    hostNameOffset -- offset of the host name for DNS 'A/AAAA' query on the memory page
 *    protocol       --  transport protocol, discovered by the SRV query, defined explicitly or undefined
 *    port           -- port , discovered by the SRV query, defined explicitly or undefined
 *    priority       -- priority , discovered by the SRV query or undefined
 *    weight         -- weight, discovered by the SRV query or undefined
 *    strHostName    -- String that continas the host name - for debug purposes.
 */

typedef struct
{
    RvInt32                 hostNameOffset;
    RvSipTransport          protocol;
    RvUint16                port;
    RvUint16                priority;
    RvUint16                weight;
#ifdef SIP_DEBUG
    RvChar*                 strHostName;
#endif
} TransportDNSHostNameElement;

/* TransportDNSSRVElement
 * --------------------------------------------------------------------------
 * TransportDNSSRVElement defines the DNS SRV list element, which keeps information
 * according to single SRV name that may be retrieved by the NAPTR DNS query.
 *
 *    nameOffset       - offset of the SRV name for DNS SRV query on the memory page
 *    protocol         - transport protocol, discovered by the NAPTR query or undefined
 *    order             - NAPTR order, discovered by the NAPTR query or undefined
 *    preference         - NAPTR preference, discovered by the NAPTR query or undefined
 *  strSrvName      - String that contains the SRV name - for debug purposes.
 */

typedef struct
{
    RvInt32                   nameOffset;
    RvSipTransport            protocol;
    RvUint16                  order;
    RvUint16                  preference;
#ifdef SIP_DEBUG
    RvChar*                   strSrvName;
#endif
} TransportDNSSRVElement;

#endif


/* TransportDNSList
 * --------------------------------------------------------------------------
 * TransportDNSList defines the DNS list object, which keeps information
 * according to name resolution procedure as defined in draft-sip-srv
 *
 * hMemPool - handler of memory pool where elements of lists will be allocated
 * hMemPage - handler of memory page where elements of lists will be allocated
 * pENUMResult - results from ENUM NAPTR query.
 * bAppList - Is this is a list that was constructed by application or not. 
 * hListPool - handler of list pool where lists will be allocated
 * hSrvNamesList - handler of SRV names list
 * hHostNamesList - handler of host names list
 * usedSRVElement - SRV elements used to produce the host name list
 * usedHostNameElement - Host Name element used to produce the IP address list
 * hIPAddrList - handler of IP addresses list
 */

typedef struct
{
    HRPOOL                              hMemPool;
    HPAGE                               hMemPage;
    RvChar*                             pENUMResult;
    RvBool                              bAppList;
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RLIST_HANDLE                        hSrvNamesList;
    RLIST_HANDLE                        hHostNamesList;
    RvSipTransportDNSSRVElement         usedSRVElement;
    RvSipTransportDNSHostNameElement    usedHostNameElement;
#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */
    RLIST_HANDLE                        hIPAddrList;
    RvUint32                            maxElementsInSingleDnsList;
} TransportDNSList;



/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/



/***************************************************************************
 * TransportDNSListConstruct
 * ------------------------------------------------------------------------
 * General: The SipTransportDNSListConstruct function allocates and fills
 *          the TransportDNSList structure and returns handler to the
 *            TransportDNSList and appropriate error code or RV_OK.
 *            Receives as input memory pool, page and list pool where lists
 *            element and the TransportDNSList object itself will be allocated.
 * Return Value: RV_ERROR_INVALID_HANDLE - One of the input handles is invalid
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMemPool    - Handle of the memory pool.
 *            hMemPage    - Handle of the memory page
 *            hListsPool    - Handle of the list pool
 *            maxElementsInSingleDnsList - maximum num. of elements
 *                        in single list
 *          bAppList    - Is this list constructed by application or by stack.
 * Output:     ppDnsList    - Pointer to a DNS list object handler.
 ***************************************************************************/
RvStatus RVCALLCONV TransportDNSListConstruct(IN  HRPOOL                       hMemPool,
                                              IN  HPAGE                        hMemPage,
                                              IN  RvUint32                    maxElementsInSingleDnsList,
                                              IN  RvBool                      bAppList,
                                              OUT TransportDNSList           **ppDnsList);

/***************************************************************************
 * TransportDNSListDestruct
 * ------------------------------------------------------------------------
 * General: The RvSipTransportDNSListDestruct function frees all memory allocated
 *          by the TransportDNSList object, including the TransportDNSList itself.
 * Return Value: RV_ERROR_INVALID_HANDLE - One of the input handles is invalid
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pDnsList    - Pointer to a DNS list object handler.
 * Output:     N/A
 ***************************************************************************/
RvStatus RVCALLCONV TransportDNSListDestruct(IN TransportDNSList            *pDnsList);


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
                                          OUT TransportDNSList*   pDnsListClone);


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
                                           IN     TransportDNSList                  *pDnsList,
                                           IN     RvSipListLocation                 location,
                                           INOUT  void                              **pRelative,
                                           OUT    RvSipTransportDNSSRVElement       *pSrvElement);
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/***************************************************************************
 * TransportDNSListGetHostElement
 * ------------------------------------------------------------------------
 * General: retrieves host element from the host elements list of the DNS
 * list object according to specified by input location.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - Handle to the transport manager
 *          pDnsList        - Handle of the DNS list object
 *          location        - starting element location
 *            pRelative        - relative host name element for 'next' or 'previous'
 *                            locations
 * Output:  pHostElement    - found element
 *            pRelative        - new relative host name element for consequent
 *            call to the RvSipTransportDNSListGetHostElement function.
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListGetHostElement (
                                           IN  TransportMgr*                     pTransportMgr,
                                           IN  TransportDNSList                 *pDnsList,
                                           IN  RvSipListLocation                location,
                                           INOUT  void                            **pRelative,
                                           OUT RvSipTransportDNSHostNameElement    *pHostElement);

/***************************************************************************
 * TransportDNSListRemoveTopmostSrvElement
 * ------------------------------------------------------------------------
 * General: removes topmost SRV element from the SRV elements list of the
 * DNS list object.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr    - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListRemoveTopmostSrvElement(
                                          IN TransportMgr*          pTransportMgr,
                                          IN TransportDNSList            *pDnsList);

/***************************************************************************
 * TransportDNSListRemoveTopmostHostElement
 * ------------------------------------------------------------------------
 * General: removes topmost host element from the head of the host elements
 * list of the DNS list object.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListRemoveTopmostHostElement(
                                        IN TransportMgr*          pTransportMgr,
                                        IN TransportDNSList            *pDnsList);


/***************************************************************************
 * TransportDNSListPopSrvElement
 * ------------------------------------------------------------------------
 * General: retrieves and removes topmost SRV name element from the SRV
 * elements list of the DNS list object.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr    - pointer to the transport manager
 *          hDnsList        - Handle of the DNS list object
 * Output:  pSrvElement     - retrieved element
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListPopSrvElement (
                                  IN  TransportMgr*                 pTransportMgr,
                                  IN  TransportDNSList              *pDnsList,
                                  OUT RvSipTransportDNSSRVElement    *pSrvElement);
/***************************************************************************
 * TransportDNSListPopHostElement
 * ------------------------------------------------------------------------
 * General: retrieves and removes topmost host element from the list of host
 * elements in the DNS list object.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr    - pointer to the transport manager
 *          hDnsList        - Handle of the DNS list object
 * Output:  pHostElement    - element
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListPopHostElement (
                                       IN  TransportMgr*                    pTransportMgr,
                                       IN  TransportDNSList                 *pDnsList,
                                       OUT RvSipTransportDNSHostNameElement    *pHostElement);
/***************************************************************************
 * TransportDNSListPushSrvElement
 * ------------------------------------------------------------------------
 * General: adds single SRV element to the head of the SRV names list of the
 * DNS list object.
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr    - pointer to the transport manager
 *          hDnsList        - Handle of the DNS list object
 *          pSrvElement        - SRV element structure to be added to the list
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListPushSrvElement(
                                    IN TransportMgr*                pTransportMgr,
                                    IN TransportDNSList            *pDnsList,
                                    IN RvSipTransportDNSSRVElement    *pSrvElement);

/***************************************************************************
 * TransportDNSListPushHostElement
 * ------------------------------------------------------------------------
 * General: adds host element to the head of the host elements list of the
 * DNS list object.
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr    - pointer to the transport manager
 *          hDnsList        - Handle of the DNS list object
 *          pHostElement    - host name element structure to be added to the list
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListPushHostElement(
                                              IN TransportMgr*                    pTransportMgr,
                                              IN TransportDNSList                 *pDnsList,
                                                IN RvSipTransportDNSHostNameElement *pHostElement);

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
 RvStatus RVCALLCONV TransportDNSListGetUsedSRVElement(
    IN TransportMgr*                pTransportMgr,
    IN TransportDNSList                *pDnsList,
    IN RvSipTransportDNSSRVElement    *pSRVElement);


/***************************************************************************
 * TransportDNSListGetUsedHostElement
 * ------------------------------------------------------------------------
 * General: retrieves host name element, used to produce the IP list
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 *          pHostElement    - host name element structure to be added to the list
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListGetUsedHostElement(
    IN TransportMgr*                    pTransportMgr,
    IN TransportDNSList                    *pDnsList,
    IN RvSipTransportDNSHostNameElement *pHostElement);

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
    IN RvSipTransportDNSSRVElement    *pSRVElement);


/***************************************************************************
 * TransportDNSListGetUsedHostElement
 * ------------------------------------------------------------------------
 * General: retrieves host name element, used to produce the IP list
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
    IN RvSipTransportDNSHostNameElement *pHostElement);

#endif/* RV_DNS_ENHANCED_FEATURES_SUPPORT*/

/***************************************************************************
 * TransportDNSListGetIPElement
 * ------------------------------------------------------------------------
 * General: retrieves IP address element from the DNS list objects IP
 * addresses list according to specified by input location.
 * Return Value: RV_OK, RV_ERROR_UNKNOWN or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr    - pointer to the transport manager
 *          hDnsList        - Handle of the DNS list object
 *          location        - starting element location
 *            pRelative        - relative IP element for get next/previous
 * Output:  pIPElement      - found element
 *            pRelative        - new relative IP element for consequent
 *            call to the RvSipTransportDNSListGetIPElement function.
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListGetIPElement (
                                           IN     TransportMgr*                 pTransportMgr,
                                           IN     TransportDNSList              *pDnsList,
                                           IN     RvSipListLocation                 location,
                                           INOUT  void                            **pRelative,
                                           OUT    RvSipTransportDNSIPElement    *pIPElement);
/***************************************************************************
 * TransportDNSListRemoveTopmostIPElement
 * ------------------------------------------------------------------------
 * General: removes topmost element from the head of the DNS list
 * object IP addresses list.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr    - pointer to the transport manager
 *          hDnsList        - Handle of the DNS list object
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListRemoveTopmostIPElement(
                                           IN TransportMgr*               pTransportMgr,
                                           IN TransportDNSList            *pDnsList);

/***************************************************************************
 * TransportDNSListPopIPElement
 * ------------------------------------------------------------------------
 * General: retrieves and removes from the IP addresses list of the DNS
 * list object the topmost IP address element.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr    - pointer to the transport manager
 *          hDnsList        - handle of the DNS list object
 * Output:  pIpElement      - element
 ***************************************************************************/
 RvStatus RVCALLCONV TransportDNSListPopIPElement(
                                        IN  TransportMgr*               pTransportMgr,
                                        IN  TransportDNSList            *pDnsList,
                                        OUT RvSipTransportDNSIPElement    *pIPElement);
/***************************************************************************
 * TransportDNSListPushIPElement
 * ------------------------------------------------------------------------
 * General: adds single IP address element to the head of the IP addresses list of the
 * DNS list object.
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr    - pointer to the transport manager
 *          pDnsList        - pointer to the DNS list object
 *          pIPElement        - IP address element structure to be added to the list
 ***************************************************************************/
RvStatus RVCALLCONV TransportDNSListPushIPElement(
                                           IN TransportMgr*                 pTransportMgr,
                                           IN TransportDNSList              *pDnsList,
                                           IN RvSipTransportDNSIPElement    *pIPElement);

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
    INOUT TransportDNSList*  pDnsList);

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
    OUT RvChar**         pEnumRes);

/***************************************************************************
 * TransportDNSListGetNumberOfEntries
 * ------------------------------------------------------------------------
 * General: Retrives number of elements in each of the DNS list object lists.
 *
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr    - pointer to the transport manager
 *          pDnsList        - pointer to of the DNS list object
 * Output:  pSrvElements        - number of SRV elements
 *            pHostNameElements    - number of host elements
 *            pIpAddrElements        - number of IP address elements
 ***************************************************************************/
RvStatus RVCALLCONV TransportDNSListGetNumberOfEntries(
                                      IN TransportMgr*                  pTransportMgr,
                                      IN TransportDNSList               *pDnsList,
                                      OUT RvUint32                        *pSrvElements,
                                      OUT RvUint32                        *pHostNameElements,
                                      OUT RvUint32                        *pIpAddrElements);

/***************************************************************************
 * TransportDNSGetMaxElements
 * ------------------------------------------------------------------------
 * General: returns maximum number of DNS list entries
 * Return Value: RvUint32 - maximum number of DNS list entries
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransport - handle to the transport
 *
 ***************************************************************************/
RvUint32 RVCALLCONV TransportDNSGetMaxElements(
                                    IN  TransportMgr        *pTransportMgr);

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
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
    INOUT TransportDNSList*  pDnsList);

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
    INOUT TransportDNSList*  pDnsList);

/*****************************************************************************
* TransportDnsListPrepare
* ---------------------------------------------------------------------------
* General: 1. Removes TLS if client does not support TLS
*          2. Removes non-TLS if transport supposed to be TSL (sips URI)
*          3. Does nothing if client supports TLS and uri was "sip"
*
* Return Value: RvStatus - RvStatus
*                           RV_ERROR_UNKNOWN
*                           RV_ERROR_BADPARAM
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pTransportMgr        - pointer to transport manager
*          strHostName          - the host name we handle
*          bIsSecure            - was the URI of the message secure (eg. sips)
*          hSrvNamesList        - the list that holds the srv records
*****************************************************************************/
void RVCALLCONV TransportDnsListPrepare(
      IN  TransportMgr*       pTransportMgr,
      IN  RvChar*             strHostName,
      IN  RLIST_HANDLE        hSrvNamesList);
#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/

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
    INOUT TransportDNSList*  pDnsList);

#ifdef __cplusplus
}
#endif

#endif /*TRANSPORT_DNS_H*/

