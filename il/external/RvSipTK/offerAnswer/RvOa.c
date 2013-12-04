/*
*******************************************************************************
*                                                                             *
* NOTICE:                                                                     *
* This document contains information that is confidential and proprietary to  *
* RADVision LTD.. No part of this publication may be reproduced in any form   *
* whatsoever without written prior approval by RADVision LTD..                *
*                                                                             *
* RADVision LTD. reserves the right to revise this publication and make       *
* changes without obligation to notify any person of such revisions or changes*
*******************************************************************************
*/

/******************************************************************************
 *                              <RvOa.c>
 *
 *    Author                        Date
 *    ------                        --------
 *    Igor							Aug 2006
 *****************************************************************************/

/*@****************************************************************************
 * Module: Offer-Answer
 * ----------------------------------------------------------------------------
 * Title: Offer-Answer Functions
 *
 * The RvOa.h file defines API, provided by the Offer-Answer module.
 * The API functions can be divided in following groups:
 *  1. Offer-Answer Manager API
 *  2. Offer-Answer Session API
 *  3. Offer-Answer Stream API
 *
 * Each group contains functions for creation of correspondent object and
 * for inspection & modifying the object parameters.
 *
 * Offer-Answer (OA) module implements the Offer-Answer Model of negotiation
 * of media capabilities by SIP endpoints, described in RFC3264.
 * The Offer-Answer module implements the abstractions and their behavior, as
 * it is defined by RFC 3264.
 ***************************************************************************@*/

/*---------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                            */
/*---------------------------------------------------------------------------*/
#include "rvtypes.h"

#include "RvOa.h"
#include "RvOaVersion.h"
#include "OaMgrObject.h"
#include "OaSessionObject.h"
#include "OaStreamObject.h"
#include "OaSdpMsg.h"
#include "OaUtils.h"

/*---------------------------------------------------------------------------*/
/*                             TYPE DEFINITIONS                              */
/*---------------------------------------------------------------------------*/
#define DEFAULT_CFG_MAX_SESSIONS                 (10)
#define DEFAULT_CFG_MAX_STREAMS                  (20)
#define DEFAULT_CFG_MESSAGEPAGE_SIZE             (4096)
#define DEFAULT_CFG_MAX_SESSIONS_WITH_LOCAL_CAPS (0)
#define DEFAULT_CFG_AVG_FORMATS_PER_CAPS         (10)

/*---------------------------------------------------------------------------*/
/*                         STATIC FUNCTIONS DEFINITIONS                      */
/*---------------------------------------------------------------------------*/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static void RVCALLCONV LogMsg(
                            IN OaSession*   pOaSession,
                            IN RvChar*      strSdpMsg,
                            IN RvBool       bIncoming,
                            IN RvLogSource* pLogSource);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/*---------------------------------------------------------------------------*/
/*                      OFFER-ANSWER MANAGER API                             */
/*---------------------------------------------------------------------------*/

/*@****************************************************************************
 * RvOaInitCfg (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Initializes the RvOaCfg structure.
 *  The RvOaCfg structure should be provided by the Application to the module
 *  creating function in order to construct the OA Module.
 *  The module creating function for OA Module is RvOaConstruct.
 *  The RvOaInitCfg function fills the structure fields with the default values
 *
 * Arguments:
 * Input:  sizeOfCfg - size of the structure to be initialized in bytes.
 * Output: pOaCfg    - structure to be initialized.
 *
 * Return Value: none
 ***************************************************************************@*/
RVAPI void RVCALLCONV RvOaInitCfg(IN  RvUint32 sizeOfCfg,
                                  OUT RvOaCfg* pOaCfg)
{
    RvOaCfg  internalOaCfg;
    RvInt32  minCfgSize;

    /* For backward compatibility we initiate an internal structure and copy
       it to the caller structure */

    minCfgSize = RvMin(((RvUint)sizeOfCfg),sizeof(RvOaCfg));

    memset(pOaCfg,0,sizeOfCfg);
    memset(&internalOaCfg,0,sizeof(RvOaCfg));

    internalOaCfg.maxSessions = DEFAULT_CFG_MAX_SESSIONS;
    internalOaCfg.maxStreams  = DEFAULT_CFG_MAX_STREAMS;
    internalOaCfg.messagePoolPageSize  = DEFAULT_CFG_MESSAGEPAGE_SIZE;
    internalOaCfg.maxSessionsWithLocalCaps = DEFAULT_CFG_MAX_SESSIONS_WITH_LOCAL_CAPS;
    internalOaCfg.capPoolPageSize          = DEFAULT_CFG_MESSAGEPAGE_SIZE;
    internalOaCfg.avgNumOfMediaFormatsPerCaps = DEFAULT_CFG_AVG_FORMATS_PER_CAPS;
    internalOaCfg.bSetOneCodecPerMediaDescr = RV_FALSE;
    internalOaCfg.logHandle            = NULL;
    internalOaCfg.pfnLogEntryPrint     = NULL;
    internalOaCfg.logEntryPrintContext = NULL;
    internalOaCfg.logFilter =  RVOA_LOG_INFO_FILTER  |
                               RVOA_LOG_ERROR_FILTER |
                               RVOA_LOG_DEBUG_FILTER |
                               RVOA_LOG_EXCEP_FILTER |
                               RVOA_LOG_WARN_FILTER;
    internalOaCfg.logFilterCaps = internalOaCfg.logFilter;
    internalOaCfg.logFilterMsgs = internalOaCfg.logFilter;
    memcpy(pOaCfg, &internalOaCfg, minCfgSize);
}

/*@****************************************************************************
 * RvOaConstruct (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Constructs the OA Module using configuration, provided by the Application.
 *
 * Arguments:
 * Input:  pOaCfg    - OA Module configuration.
 *         sizeOfCfg - size of the configuration structure in bytes.
 * Output: phOaMgr   - handle of the OA Module's Manager object.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaConstruct(
                                IN  RvOaCfg*        pOaCfg,
                                IN  RvUint32        sizeOfCfg,
                                OUT RvOaMgrHandle*  phOaMgr)
{
    RvStatus  rv;
    RvOaCfg   internalOaCfg;
    RvInt32   minCfgSize;
    OaMgr*    pOaMgr;

    if (NULL == phOaMgr  ||  NULL == pOaCfg)
    {
        return RV_ERROR_BADPARAM;
    }

    /* For backward compatibility we use internal structure for configuration*/
    RvOaInitCfg(sizeof(RvOaCfg),&internalOaCfg);
    minCfgSize = RvMin(((RvUint)sizeOfCfg),sizeof(RvOaCfg));
    memcpy(&internalOaCfg, pOaCfg, minCfgSize);

    /* Construct the manager */
    rv = OaMgrConstruct(&internalOaCfg, &pOaMgr);

    if (RV_OK == rv)
    {
        OaPrintConfigParamsToLog(pOaMgr, &internalOaCfg);

        RvLogInfo(pOaMgr->pLogSource, (pOaMgr->pLogSource,
            "RvOaConstruct - Offer-Answer Manager was constructed successfully: hOaMgr=%p. Version %s",
            pOaMgr, RV_OA_MODULE_VERSION));
        *phOaMgr = (RvOaMgrHandle)pOaMgr;
    }

    return rv;
}

/*@****************************************************************************
 * RvOaDestruct (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Destructs the Manager object and free all resources, used by the OA Module.
 *
 * Arguments:
 * Input:  hOaMgr - handle to the Manager of the OA Module to be destructed.
 * Output: none.
 *
 * Return Value: none
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaDestruct(IN RvOaMgrHandle hOaMgr)
{
    RvStatus  rv;
    OaMgr*    pOaMgr = (OaMgr*)hOaMgr;

    if (NULL == pOaMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaMgrLock(pOaMgr);
    if (RV_OK != rv)
    {
        return rv;
    }

    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "RvOaDestruct(hOaMgr=%p) - going to destruct Offer-Answer Manager",
        pOaMgr));

    OaMgrDestruct(pOaMgr);

    /* Don't unlock Manager, because it's lock might been destructed */

    return RV_OK;
}

/*@****************************************************************************
 * RvOaGetResources (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets various statistics about consumption of resources by the OA Module.
 *
 * Arguments:
 * Input:  hOaMgr          - handle of the OA Module Manager.
 *         sizeOfResources - size of the resources structure in bytes.
 * Output: pOaResources    - resource consumption statistics.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaGetResources(
                                IN  RvOaMgrHandle   hOaMgr,
                                IN  RvUint32        sizeOfResources,
                                OUT RvOaResources*  pOaResources)
{
    RvStatus      rv;
    OaMgr*        pOaMgr = (OaMgr*)hOaMgr;
    RvOaResources oaResources;
    RvInt32       minResourcesSize;

    if (NULL == pOaMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaMgrLock(pOaMgr);
    if (RV_OK != rv)
    {
        return rv;
    }

    /* For backward compatibility we initiate an internal structure and copy
       it to the caller structure */
    minResourcesSize = RvMin(((RvUint)sizeOfResources),sizeof(RvOaResources));
    memset(&oaResources, 0, minResourcesSize);

    rv = OaGetResources(pOaMgr, &oaResources);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
            "RvOaGetResources(hOaMgr=%p) - failed to get resources (rv=%d:%s)",
            pOaMgr, rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaMgrUnlock(pOaMgr);
        return rv;
    }

    memcpy(pOaResources, &oaResources, minResourcesSize);

/*
    RvLogDebug(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "RvOaGetResources(hOaMgr=%p) - resources were got successfully",
        pOaMgr));
*/
    OaMgrUnlock(pOaMgr);

    return RV_OK;
}

/*@****************************************************************************
 * RvOaSetLogFilters (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Sets the Filters that filter information,printed by the OA Module into log.
 *  This function can be called anytime during module life cycle.
 *  The filters are represented by the Filter Mask. The Filter Mask combines
 *  different filters, using bit-wise OR operator.
 *  For example, it the Application wants to log only errors and exceptions,
 *  occur in OA Module during SDP Session processing, it should set the Filter
 *  Mask to (RVOA_LOG_ERROR_FILTER | RVOA_LOG_EXCEP_FILTER) value.
 *
 * Arguments:
 * Input:  hOaMgr     - handle of the OA Module Manager.
 *         eLogSource - source of data to be logged.
 *                      Example of log source can be a Capabilitites related
 *                      code, or Message handling functions etc.
 *         filters    - filter mask to be set.
 *
 * Output: none
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSetLogFilters(
                                IN RvOaMgrHandle  hOaMgr,
                                IN RvOaLogSource  eLogSource,
                                IN RvInt32        filters)
{
	
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    RvStatus  rv;
    OaMgr*    pOaMgr = (OaMgr*)hOaMgr;
    RvChar    strLogFilters[RVOA_LOGMASK_STRLEN];

    if (NULL == pOaMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaMgrLock(pOaMgr);
    if (RV_OK != rv)
    {
        return rv;
    }

    rv = OaSetLogFilters(pOaMgr, eLogSource, filters);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
            "RvOaSetLogFilters(hOaMgr=%p) - failed to set log filters for %s (rv=%d:%s)",
            pOaMgr, OaUtilsConvertEnum2StrLogSource(eLogSource),
            rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaMgrUnlock(pOaMgr);
        return rv;
    }

    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "RvOaSetLogFilters(hOaMgr=%p,log source=%s) - log filters (%s) were set successfully for source",
        pOaMgr, OaUtilsConvertEnum2StrLogSource(eLogSource),
        OaUtilsConvertEnum2StrLogMask(pOaMgr->logFilter, strLogFilters)));

    OaMgrUnlock(pOaMgr);

    return RV_OK;
#else
	RV_UNUSED_ARGS((filters,eLogSource,hOaMgr));
    return RV_OK;
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

}

/*@****************************************************************************
 * RvOaDoesLogFilterExist (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Check if the specific log filter is set into the OA Module.
 *
 * Arguments:
 * Input:  hOaMgr - handle of the OA Module Manager.
 *         filter - filter to be checked.
 * Output: none.
 *
 * Return Value: RV_TRUE, if the filter is set. RV_FALSE - otherwise.
 ***************************************************************************@*/
RVAPI RvBool RVCALLCONV RvOaDoesLogFilterExist(
                                IN RvOaMgrHandle   hOaMgr,
                                IN RvOaLogFilter   filter)
{
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    RvStatus rv;
    RvBool   bFilterExist = RV_FALSE;
    OaMgr*   pOaMgr = (OaMgr*)hOaMgr;
    RvChar   strLogFilters[RVOA_LOGMASK_STRLEN];

    if(pOaMgr == NULL)
    {
        return RV_FALSE;
    }

    rv = OaMgrLock(pOaMgr);
    if (RV_OK != rv)
    {
        return rv;
    }

    bFilterExist = RvLogIsSelected(pOaMgr->pLogSource,
                                   OaUtilsConvertOa2CcLogMask(filter));

    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "RvOaDoesLogFilterExist(hOaMgr=%p) - log filters(%s) %sexists",
        pOaMgr,
        OaUtilsConvertEnum2StrLogMask(pOaMgr->logFilter, strLogFilters),
        (RV_TRUE==bFilterExist) ? "" : "doesn't "));

    OaMgrUnlock(pOaMgr);

    return bFilterExist;
#else
	RV_UNUSED_ARGS((filter,hOaMgr));
    return RV_FALSE;
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
}

/*@****************************************************************************
 * RvOaSetCapabilities (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Loads default media capabilities into the OA Module.
 *  The capabilities should be provided in form of proper SDP message.
 *  Client Session (offerer) uses the default capabilities while generating
 *  OFFER message, if the local capabilities were not set into the Session.
 *  The local capabilities can be set using RvOaSessionSetCapabilities function.
 *  Server Session (answerer) uses the default capabilities while identifying
 *  set of codecs supported by both offerer and answerer, if the local
 *  capabilities were not set.
 *  Note the Client Session generates the OFFER by copy-construct of SDP
 *  message that contains the capabilities. That is why the capabilities should
 *  be represented as a proper SDP message.
 *
 * Arguments:
 * Input:  hOaMgr    - handle of the OA Module Manager.
 *         strSdpMsg - default capabilities as a NULL-terminated string.
 *                     Can be NULL.
 *         pSdpMsg   - default capabilities in form of RADVISION SDP Stack
 *                     message.
 *                     Can be NULL.
 *                     Has lower priority than strSdpMsg.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSetCapabilities(
                                IN  RvOaMgrHandle  hOaMgr,
                                IN  RvChar*        strSdpMsg,
                                IN  RvSdpMsg*      pSdpMsg)
{
    RvStatus rv;
    OaMgr*   pOaMgr = (OaMgr*)hOaMgr;

    if(NULL==pOaMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaMgrLock(pOaMgr);
    if (RV_OK != rv)
    {
        return rv;
    }

    if(NULL==strSdpMsg && NULL==pSdpMsg)
    {
        RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
            "RvOaSetCapabilities(hOaMgr=%p) - Capabilities were not provided: strSdpMsg=NULL, pSdpMsg=NULL",
            pOaMgr));
        OaMgrUnlock(pOaMgr);
        return RV_ERROR_BADPARAM;
    }

    rv = OaSetCapabilities(pOaMgr, strSdpMsg, pSdpMsg);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSourceCaps,(pOaMgr->pLogSourceCaps,
            "RvOaSetCapabilities(hOaMgr=%p) - failed to set capabilities (rv=%d:%s)",
            pOaMgr, rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaMgrUnlock(pOaMgr);
        return rv;
    }

    RvLogInfo(pOaMgr->pLogSourceCaps,(pOaMgr->pLogSourceCaps,
        "RvOaSetCapabilities(hOaMgr=%p) - capabilities were set", pOaMgr));

    OaMgrUnlock(pOaMgr);

    return RV_OK;
}

/*@****************************************************************************
 * RvOaGetCapabilities (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets the OA Module's default media capabilities.
 *  The capabilities can be set into module by the Application using
 *  the RvOaSetCapabilities function.
 *
 *  The default capabilities are hold in form of RADVISION SDP Stack Message
 *  object. This object uses RPOOL pages to hold it's data. It manages pages
 *  by means of the Allocator object. Therefore the Application has to provide
 *  the Allocator object to be used while building copy of default capabilities
 *  message.
 *  The Allocator object can be created using RADVISION SDP Stack API.
 *
 * Arguments:
 * Input:  hOaMgr         - handle of the OA Module Manager.
 *         lenOfStrSdpMsg - maximal length of the string returned by the OA
 *                          module using strSdpMsg parameter.
 *                          This parameter is meaningful only if strSdpMsg
 *                          parameter is not NULL.
 *         pSdpAllocator  - allocator object, to be used while building
 *                          the message that holds the default capabilities.
 *                          This parameter is meaningful only if pSdpMsg
 *                          parameter is not NULL.
 * Output: strSdpMsg      - default capabilities as a NULL-terminated string.
 *                          Can be NULL.
 *         pSdpMsg        - default capabilities in form of RADVISION SDP Stack
 *                          message.
 *                          Can be NULL.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaGetCapabilities(
                                IN  RvOaMgrHandle  hOaMgr,
                                IN  RvUint32       lenOfStrSdpMsg,
                                IN  RvAlloc*       pSdpAllocator,
                                OUT RvChar*        strSdpMsg,
                                OUT RvSdpMsg*      pSdpMsg)
{
    RvStatus rv;
    OaMgr*   pOaMgr = (OaMgr*)hOaMgr;

    if(NULL==pOaMgr)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaMgrLock(pOaMgr);
    if (RV_OK != rv)
    {
        return rv;
    }

    if (RV_FALSE == pOaMgr->defCapMsg.bSdpMsgConstructed)
    {
        RvLogDebug(pOaMgr->pLogSourceCaps,(pOaMgr->pLogSourceCaps,
            "RvOaGetCapabilities(hOaMgr=%p) - Capabilities were not set",pOaMgr));
        OaMgrUnlock(pOaMgr);
        return RV_ERROR_NOT_FOUND;
    }

    if(NULL==strSdpMsg && NULL==pSdpMsg)
    {
        RvLogError(pOaMgr->pLogSourceCaps,(pOaMgr->pLogSourceCaps,
            "RvOaGetCapabilities(hOaMgr=%p) - Capabilities can't be retrieved: strSdpMsg=NULL, pSdpMsg=NULL",
            pOaMgr));
        OaMgrUnlock(pOaMgr);
        return RV_ERROR_BADPARAM;
    }

    if (NULL!=pSdpMsg && NULL==pSdpAllocator)
    {
        RvLogError(pOaMgr->pLogSourceCaps,(pOaMgr->pLogSourceCaps,
            "RvOaGetCapabilities(hOaMgr=%p) - Capabilities can't be retrieved: pSdpAllocator=NULL",
            pOaMgr));
        OaMgrUnlock(pOaMgr);
        return RV_ERROR_BADPARAM;
    }

    rv = OaGetCapabilities(pOaMgr, lenOfStrSdpMsg, pSdpAllocator,
                          strSdpMsg, pSdpMsg);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSourceCaps,(pOaMgr->pLogSourceCaps,
            "RvOaGetCapabilities(hOaMgr=%p) - failed to get capabilities (rv=%d:%s)",
            pOaMgr, rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaMgrUnlock(pOaMgr);
        return rv;
    }

/*
    RvLogDebug(pOaMgr->pLogSourceCaps,(pOaMgr->pLogSourceCaps,
        "RvOaGetCapabilities(hOaMgr=%p) - capabilities were retrieved", pOaMgr));
*/

    OaMgrUnlock(pOaMgr);

    return RV_OK;
}

/*@****************************************************************************
 * RvOaCreateSession (Offer-Answer Manager API)
 * ----------------------------------------------------------------------------
 * General:
 *  Creates the Session object.
 *  Session object should be used by the Application for media capabilities
 *  negotiation. The Application should provided the OFFER and ANSWER messages,
 *  received as a bodies of SIP messages, to the Session object.
 *  As a result, the Session object will generate Stream objects for each
 *  negotiated media descriptions. Using this objects the Application can find
 *  various parameters of the Offer-Answer Stream to be transmitted, including remote
 *  address, media format to be used, direction of transmission and others.
 *  Session object can generate either OFFER to be sent to answerer, based on
 *  the local or default capabilities, or ANSWER to be sent to offerer, based
 *  on the received OFFER and local or default capabilities.
 *  The generated message can be get anytime during life cycle of the Session
 *  object, using RvOaSessionGetMsgToBeSent function.
 *
 *  The created Session object stays in the IDLE state until
 *  RvOaSessionGenerateOffer or RvOaSessionSetRcvdMsg functions is called.
 *
 * Arguments:
 * Input:  hOaMgr      - handle of the OA Module Manager.
 *         hAppSession - handle of the Application object, which is going
 *                       to be served by the Session.
 * Output: phSession   - handle to the created Session object.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaCreateSession(
                                IN  RvOaMgrHandle         hOaMgr,
                                IN  RvOaAppSessionHandle  hAppSession,
                                OUT RvOaSessionHandle*    phSession)
{
    RvStatus rv;
    OaMgr*   pOaMgr = (OaMgr*)hOaMgr;

    if(NULL==pOaMgr || NULL==phSession)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaMgrLock(pOaMgr);
    if (RV_OK != rv)
    {
        return rv;
    }

    /* Allocate new Session object */
    if (NULL == pOaMgr->hSessionList)
    {
        RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
            "RvOaCreateSession(hOaMgr=%p) - failed to allocate new Session",
            pOaMgr));
        OaMgrUnlock(pOaMgr);
        return RV_ERROR_OUTOFRESOURCES;
    }
    rv = RLIST_InsertTail(pOaMgr->hSessionListPool, pOaMgr->hSessionList,
                          (RLIST_ITEM_HANDLE*)phSession);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
            "RvOaCreateSession(hOaMgr=%p) - failed to insert add object to list of Sessions (rv=%d:%s)",
            pOaMgr, rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaMgrUnlock(pOaMgr);
        return rv;
    }

    /* Initialize the new Session */
    rv = OaSessionConstruct((OaSession*)(*phSession), hAppSession);
    if (RV_OK != rv)
    {
        RLIST_Remove(pOaMgr->hSessionListPool, pOaMgr->hSessionList,
                     (RLIST_ITEM_HANDLE)(*phSession));

        RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
            "RvOaCreateSession(hOaMgr=%p) - failed to initialize new Session (rv=%d:%s)",
            pOaMgr, rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaMgrUnlock(pOaMgr);
        return rv;
    }

    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "RvOaCreateSession(hOaMgr=%p) - Session %p was created successfully",
        pOaMgr, *phSession));

    OaMgrUnlock(pOaMgr);

    return RV_OK;
}

/*---------------------------------------------------------------------------*/
/*                                SESSION API                                */
/*---------------------------------------------------------------------------*/
/*@****************************************************************************
 * RvOaSessionGenerateOffer (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Causes Session object to built OFFER message.
 *  After call to this function the Application can get the OFFER in format of
 *  NULL-terminated string and send it with SIP message.
 *  This can be done using RvOaSessionGetMsgToBeSent() function anytime during
 *  Session life cycle.
 *  The RvOaSessionGenerateOffer function performs following actions:
 *  1. Builds the OFFER in RADVISION SDP Stack Message format. Local
 *     capabilities, if were set by the Application, are used as a source for
 *     copy-construct operation, as a result of which the OFFER is created.
 *     Local capabilities can be set into the Session by the Application using
 *     RvOaSessionSetCapabilities function. If local capabilities were not set,
 *     the default capabilities, set into the OA Module by the Application
 *     using RvOaSetCapabilities function.
 *  2. Adds missing mandatory lines to the built message,using default values.
 *  3. Generates Stream object for each media descriptor, found in the built
 *     message, and insert it into the list, managed by the Session object.
 *
 *  Anytime during Session life cycle the Application can iterate through list
 *  of Stream objects belonging to the Session object. For each Stream
 *  the Application can inspect and modify it's attributes and parameters.
 *  This can be done using the OA Module API for Stream object,
 *  or this can be done using the RADVISION SDP Stack API for Message object.
 *  Note the SDP Stack API requires handle to the Message structure that
 *  contains session description. This handle can be got from the Session
 *  object using RvOaSessionGetSdpMsg function.
 *
 *  RvOaSessionGenerateOffer moves the Session object from IDLE and from
 *  ANSWER_READY state into OFFER_READY state.
 *  The Session object stays in OFFER_READY state until RvOaSessionSetRcvdMsg
 *  function is called.
 *
 *  RvOaSessionGenerateOffer function can be called for sessions in IDLE,
 *  ANSWER_RCVD or ANSWER_READY states only.
 *
 * Arguments:
 * Input:  hSession - handle of the Session object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionGenerateOffer(
                                    IN  RvOaSessionHandle     hSession)
{
    RvStatus rv;
    OaSession* pOaSession = (OaSession*)hSession;

    if(NULL==pOaSession)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    /* OFFER can't be generated, if there is active OFFER-ANSWER transaction */
    if (RVOA_SESSION_STATE_IDLE         != pOaSession->eState &&
        RVOA_SESSION_STATE_ANSWER_RCVD  != pOaSession->eState &&
        RVOA_SESSION_STATE_ANSWER_READY != pOaSession->eState)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "RvOaSessionGenerateOffer(hSession=%p) - can't generate OFFER: ANSWER was not received/generated yet",
            pOaSession));
        OaSessionUnlock(pOaSession);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    rv = OaSessionGenerateOffer(pOaSession);
    if (RV_OK != rv)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "RvOaSessionGenerateOffer(hSession=%p) - failed to generate OFFER(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaSessionUnlock(pOaSession);
        return rv;
    }

    OaSessionUnlock(pOaSession);

    RvLogInfo(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaSessionGenerateOffer(hSession=%p) - succeeded",
        pOaSession));

    return RV_OK;
}

/*@****************************************************************************
 * RvOaSessionTerminate (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Terminates Session object.
 *  This function frees resources, consumed by the Session object.
 *  After call to this function the Session object can't be more referenced.
 *
 * Arguments:
 * Input:  hSession - handle of the Session object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *               The function should not fail normally. The failure should be
 *               treated as an exception.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionTerminate(
                                    IN  RvOaSessionHandle     hSession)
{
    RvStatus rv;
    OaSession* pOaSession = (OaSession*)hSession;
    OaMgr*     pOaMgr;

    if(NULL==pOaSession)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    pOaMgr = pOaSession->pMgr;

    rv = OaSessionTerminate(pOaSession);
    if (RV_OK != rv)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "RvOaSessionTerminate(hSession=%p) - failed to terminate the session(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaSessionUnlock(pOaSession);
        return rv;
    }

    OaSessionUnlock(pOaSession);

    /* Remove the Session from list of sessions */
    rv = OaMgrLock(pOaMgr);
    if (RV_OK != rv)
    {
        return rv;
    }
    RLIST_Remove(pOaMgr->hSessionListPool, pOaMgr->hSessionList,
                 (RLIST_ITEM_HANDLE)pOaSession);
    OaMgrUnlock(pOaMgr);

    RvLogInfo(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaSessionTerminate(hSession=%p) - session was terminated",
        pOaSession));

    return RV_OK;
}

/*@****************************************************************************
 * RvOaSessionSetAppHandle (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Sets the handle to the Application object, served by the Session object.
 *  Actually the Application handle is sizeof(RvOaAppSessionHandle) bytes of
 *  application data, stored in the Session object.
 *  This data can be retrieved using RvOaSessionGetAppHandle function.
 *
 * Arguments:
 * Input:  hSession    - handle of the Session object.
 *         hAppSession - handle of the Application object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionSetAppHandle(
                                    IN  RvOaSessionHandle     hSession,
                                    IN  RvOaAppSessionHandle  hAppSession)
{
    RvStatus rv;
    OaSession* pOaSession = (OaSession*)hSession;

    if(NULL==pOaSession)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    pOaSession->hAppHandle = hAppSession;

    OaSessionUnlock(pOaSession);

    RvLogInfo(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaSessionSetAppHandle(hSession=%p) - handle %p was set",
        pOaSession, hAppSession));

    return RV_OK;
}

/*@****************************************************************************
 * RvOaSessionGetAppHandle (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets the handle to the Application object.
 *  The application handle can be set into the Session object using
 *  RvOaSessionSetAppHandle or RvOaCreateSession functions.
 *
 * Arguments:
 * Input:  hSession    - handle of the Session object.
 * Output: hAppSession - handle of the Application object.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionGetAppHandle(
                                    IN  RvOaSessionHandle     hSession,
                                    OUT RvOaAppSessionHandle* phAppSession)
{
    RvStatus rv;
    OaSession* pOaSession = (OaSession*)hSession;

    if(NULL==pOaSession || NULL==phAppSession)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    *phAppSession = pOaSession->hAppHandle;

    OaSessionUnlock(pOaSession);

/*
    RvLogDebug(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaSessionGetAppHandle(hSession=%p) - handle %p was retrieved",
        pOaSession, *phAppSession));
*/
    return RV_OK;
}

/*@****************************************************************************
 * RvOaSessionSetCapabilities (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Loads local media capabilities into the Session object.
 *  The capabilities should be provided in form of proper SDP message.
 *  Client Session (offerer) uses the local capabilities in order to generate
 *  OFFER message.
 *  Server Session (answerer) uses the default capabilities in order to
 *  identify set of codecs supported by both offerer and answerer. This set
 *  will be reflected in the ANSWER message.
 *  Note the Client Session parses the local capabilities in the RADVISION SDP
 *  stack message, which should be sent as OFFER. That is why the capabilities
 *  should be represented as a proper SDP message.
 *
 *  The capabilities can be set into IDLE sessions only.
 *
 * Arguments:
 * Input:  hSession  - handle of the Session object.
 *         strSdpMsg - local capabilities as a NULL-terminated string.
 *                     Can be NULL.
 *         pSdpMsg   - local capabilities in form of RADVISION SDP Stack
 *                     message.
 *                     Can be NULL.
 *                     Has lower priority than strSdpMsg.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionSetCapabilities(
                                    IN  RvOaSessionHandle   hSession,
                                    IN  RvChar*             strSdpMsg,
                                    IN  RvSdpMsg*           pSdpMsg)
{
    RvStatus rv;
    OaSession* pOaSession = (OaSession*)hSession;

    if(NULL==pOaSession || (NULL==strSdpMsg && NULL==pSdpMsg))
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    /* Capabilities can be set into IDLE Session only */
    if (RVOA_SESSION_STATE_IDLE != pOaSession->eState)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "RvOaSessionSetCapabilities(hSession=%p) - can't set capabilities into not IDLE object",
            pOaSession));
    }

    rv = OaSessionSetCapabilities(pOaSession, strSdpMsg, pSdpMsg);
    if (RV_OK != rv)
    {
        RvLogError(pOaSession->pMgr->pLogSourceCaps,(pOaSession->pMgr->pLogSourceCaps,
            "RvOaSessionSetCapabilities(hSession=%p) - failed to set capabilities(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaSessionUnlock(pOaSession);
        return rv;
    }

    OaSessionUnlock(pOaSession);

    RvLogInfo(pOaSession->pMgr->pLogSourceCaps,(pOaSession->pMgr->pLogSourceCaps,
        "RvOaSessionSetCapabilities(hSession=%p) - capabilities were set",
        pOaSession));

    return RV_OK;
}

/*@****************************************************************************
 * RvOaSessionSetRcvdMsg (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Loads incoming SDP message into the Session object.
 *  The incoming SDP message is received with SIP message probably.
 *  The incoming SDP message can be OFFER or ANSWER. The Session determines
 *  which one of them using state. If the RvOaSessionGenerateOffer function
 *  was called for the Session before, the message is ANSWER.Otherwise - OFFER.
 *
 *  RvOaSessionSetRcvdMsg moves the Session object from OFFER_READY state to
 *  ANSWER_RCVD state on offerer, and from IDLE to ANSWER_READY on answerer
 *  side of the session.
 *  The Session object stays in ANSWER_RCVD or in ANSWER_READY states until
 *  RvOaSessionGenerateOffer or RvOaSessionSetRcvdMsg function is called.
 *  Call of these functions means the start of the new OFFER-ANSWER
 *  transaction, purpose of which is the modification of the session.
 *
 * Arguments:
 * Input:  hSession  - handle of the Session object.
 *         strSdpMsg - SDP message in form of NULL-terminated string.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionSetRcvdMsg(
                                    IN  RvOaSessionHandle   hSession,
                                    IN  RvChar*             strSdpMsg)
{
    RvStatus rv;
    OaSession* pOaSession = (OaSession*)hSession;

    if(NULL==pOaSession || NULL==strSdpMsg)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    /* Received message can be set into Session in state IDLE, ANSWER_RCVD,
       ANSWER_READY or OFFER_READY. If the Session is in OFFER_READY state,
       the received message is handled as incoming ANSWER. Otherwise - as
       incoming OFFER.
       Check state. */
    if (RVOA_SESSION_STATE_IDLE         != pOaSession->eState &&
        RVOA_SESSION_STATE_ANSWER_RCVD  != pOaSession->eState &&
        RVOA_SESSION_STATE_ANSWER_READY != pOaSession->eState &&
        RVOA_SESSION_STATE_OFFER_READY  != pOaSession->eState)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "RvOaSessionSetRcvdMsg(hSession=%p) - illegal Session state %s. Should be IDLE, ANSWER_RCVD, ANSWER_READy or OFFER_READY",
            pOaSession, OaUtilsConvertEnum2StrSessionState(pOaSession->eState)));
        OaSessionUnlock(pOaSession);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* Log the received SDP message */
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    if (pOaSession->pMgr->logFilterMsgs & RV_LOGLEVEL_INFO)
    {
        LogMsg(pOaSession, strSdpMsg, RV_TRUE /*bIncoming*/,
               pOaSession->pMgr->pLogSourceMsgs);
    }
#endif

    rv = OaSessionSetRcvdMsg(pOaSession, strSdpMsg);
    if (RV_OK != rv)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "RvOaSessionSetRcvdMsg(hSession=%p) - failed to set received message(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaSessionUnlock(pOaSession);
        return rv;
    }

    OaSessionUnlock(pOaSession);

    RvLogInfo(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaSessionSetRcvdMsg(hSession=%p) - received message was set",
        pOaSession));

    return RV_OK;
}

/*@****************************************************************************
 * RvOaSessionGetMsgToBeSent (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets outgoing SDP message from the Session object.
 *  The outgoing SDP message should be sent by the Application probably using
 *  SIP message.
 *  The outgoing SDP message can be OFFER or ANSWER.
 *  If it was generated as a result of call to RvOaSessionGenerateOffer
 *  function, it is OFFER. If it was generated as a result of call to
 *  RvOaSessionSetRcvdMsg function, it is ASNWER.
 *
 * Arguments:
 * Input:  hSession  - handle of the Session object.
 *         buffSize  - size in bytes of the memory buffer, pointed by strSdpMsg
 * Output: strSdpMsg - SDP message in form of NULL-terminated string.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionGetMsgToBeSent(
                                    IN  RvOaSessionHandle   hSession,
                                    IN  RvUint32            buffSize,
                                    OUT RvChar*             strSdpMsg)
{
    RvStatus    rv;
    RvSdpStatus srv;
    OaSession*  pOaSession = (OaSession*)hSession;
    char*       strEncodedBufEnd;

    if(NULL==pOaSession || 0==buffSize || NULL==strSdpMsg)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    if (NULL == pOaSession->msgLocal.pSdpMsg)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "RvOaSessionGetMsgToBeSent(hSession=%p) - there is no message to be sent. Session in %s state",
            pOaSession, OaUtilsConvertEnum2StrSessionState(pOaSession->eState)));
        OaSessionUnlock(pOaSession);
        return RV_ERROR_ILLEGAL_ACTION;
    }

    strEncodedBufEnd = rvSdpMsgEncodeToBuf(pOaSession->msgLocal.pSdpMsg,
                                           strSdpMsg, buffSize, &srv);
    if (NULL == strEncodedBufEnd)
    {
        RvLogError(pOaSession->pMgr->pLogSource, (pOaSession->pMgr->pLogSource,
            "RvOaSessionGetMsgToBeSent(hSession=%p) - failed to encode message(status=%d:%s)",
            pOaSession, srv, OaUtilsConvertEnum2StrSdpStatus(srv)));
        OaSessionUnlock(pOaSession);
        return OA_SRV2RV(srv);
    }

    OaSessionUnlock(pOaSession);

    RvLogInfo(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaSessionGetMsgToBeSent(hSession=%p) - message to be sent was got",
        pOaSession));

    /* Log the SDP message to be sent */
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    if (pOaSession->pMgr->logFilterMsgs & RV_LOGLEVEL_INFO)
    {
        LogMsg(pOaSession, strSdpMsg, RV_FALSE /*bIncoming*/,
               pOaSession->pMgr->pLogSourceMsgs);
    }
#endif

    return RV_OK;
}

/*@****************************************************************************
 * RvOaSessionGetStream (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets handle of the Stream objects that belong to the Session object.
 *  Using this handle and Stream API the Application can retrieve capabilities
 *  and addresses of the remote side or inspect and update local capabilities
 *  and local addresses.
 *  The remote capabilities and addresses are stored in the remote SDP message,
 *  pointer to which can be retrieved using RvOaSessionGetSdpMsg function
 *  The local capabilities and addresses are stored in the local SDP message,
 *  pointer to which can be retrieved using RvOaSessionGetSdpMsg function.
 *  The local and remote SDP messages can be referenced as OFFER or ANSWER.
 *  It depends on role of the Application - offerer or answerer.
 *  If the Application is offerer, the local message represents OFFER and
 *  remote message - ANSWER. If the Application is answerer - opposite.
 *
 *  The Application can retrieve and update session description also using
 *  the pointer to SDP message and RADVISION SDP Stack API functions.
 *
 * Arguments:
 * Input:  hSession    - handle of the Session object.
 *         hPrevStream - handle of the Stream object, next to which in list of
 *                       Sessions streams is requested by the function.
 *                       Can be NULL. In this case handle of the first Stream
 *                       will be supplied by the function.
 * Output: phStream    - handle of the requested Stream object.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvOaSessionGetStream(
                                    IN  RvOaSessionHandle   hSession,
                                    IN  RvOaStreamHandle    hPrevStream,
                                    OUT RvOaStreamHandle*   phStream)
{
    RvStatus    rv;
    OaSession*  pOaSession = (OaSession*)hSession;

    if(NULL==pOaSession || NULL==phStream)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    if (NULL == pOaSession->pMgr->hStreamListPool)
    {
        OaSessionUnlock(pOaSession);
        return RV_ERROR_DESTRUCTED;
    }

    if (NULL == pOaSession->hStreamList)
    {
        OaSessionUnlock(pOaSession);
        return RV_ERROR_UNINITIALIZED;
    }

    if (NULL == hPrevStream)
    {
        RLIST_GetHead(pOaSession->pMgr->hStreamListPool,
            pOaSession->hStreamList, (RLIST_ITEM_HANDLE*)phStream);
    }
    else
    {
        RLIST_GetNext(pOaSession->pMgr->hStreamListPool,
            pOaSession->hStreamList, (RLIST_ITEM_HANDLE)hPrevStream,
            (RLIST_ITEM_HANDLE*)phStream);
    }

    OaSessionUnlock(pOaSession);

/*
    RvLogDebug(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaSessionGetStream(hSession=%p,hPrevStream=%p) - stream %p was found",
        pOaSession, hPrevStream, *phStream));
*/
    return RV_OK;
}

/*@****************************************************************************
 * RvOaSessionAddStream (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Adds new Stream object to the Session object.
 *  As a result of this operation, the new Stream object is created, and local
 *  SDP message is extended with a new media descriptor, which describes
 *  the media to be transmitted over this stream.
 *  Handle of the new Stream can be used by the Application in order to update
 *  various parameters of the media using Stream API functions.
 *
 *  Configuration of the new stream, provided to this function as a parameter,
 *  supplies data required for building of the media descriptor.
 *  The Application can retrieve and update stream description using pointer
 *  to the media descriptor and RADVISION SDP Stack API functions.
 *  This pointer can be retrieved anytime using
 *  RvOaStreamGetMediaDescriptor function.
 *
 * Arguments:
 * Input:  hSession   - handle of the Session object.
 *         pStreamCfg - stream configuration - various parameters of media.
 *         sizeOfCfg  - size of configuration, provided by the Application.
 * Output: phStream   - handle of the new Stream object.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvOaSessionAddStream(
                                    IN  RvOaSessionHandle  hSession,
                                    IN  RvOaStreamCfg*     pStreamCfg,
                                    IN  RvUint32           sizeOfCfg,
                                    OUT RvOaStreamHandle*  phStream)
{
    RvStatus      rv;
    OaSession*    pOaSession = (OaSession*)hSession;
    RvOaStreamCfg internalStreamCfg;
    RvInt32       minCfgSize;

    if(NULL==pOaSession || NULL==pStreamCfg || NULL==phStream)
    {
        return RV_ERROR_BADPARAM;
    }

    /* For backward compatibility we copy the caller structure
       into internal structure. */
    minCfgSize = RvMin(((RvUint)sizeOfCfg),sizeof(RvOaStreamCfg));
    memset(&internalStreamCfg,0,sizeof(RvOaStreamCfg));
    memcpy(&internalStreamCfg, pStreamCfg, minCfgSize);

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    rv = OaSessionAddStream(pOaSession, &internalStreamCfg, phStream);
    if (RV_OK != rv)
    {
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "RvOaSessionAddStream(hSession=%p) - failed to add stream(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaSessionUnlock(pOaSession);
        return rv;
    }

    OaSessionUnlock(pOaSession);

    RvLogInfo(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaSessionAddStream(hSession=%p) - stream %p was added",
        pOaSession, *phStream));

    return RV_OK;
}

/*@****************************************************************************
 * RvOaSessionResetStream (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Makes the specified stream to be not usable for media delivery.
 *  To be compliant to the RFC 3264, the Stream object is not removed
 *  physically from the Session. Instead, it's port is set to zero and all
 *  media description parameters are removed.
 *
 * Arguments:
 * Input:  hSession - The handle to the session.
 *         hStream  - The handle to the stream.
 * Output: none.
 *
 * Return Values: Returns RvStatus.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvOaSessionResetStream(
                                    IN  RvOaSessionHandle  hSession,
                                    IN  RvOaStreamHandle   hStream)
{
    RvStatus   rv;
    OaSession* pOaSession = (OaSession*)hSession;
    OaStream*  pOaStream  = (OaStream*)hStream;

    if(NULL==pOaSession || NULL==pOaStream)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    if (RVOA_STREAM_STATE_REMOVED != pOaStream->eState)
    {
        OaStreamReset(pOaStream);
    }

    OaSessionUnlock(pOaSession);

    RvLogInfo(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaSessionResetStream(hSession=%p) - stream %p was removed",
        pOaSession, pOaStream));

    return RV_OK;
}

/*@****************************************************************************
 * RvOaSessionGetSdpMsg (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets the pointer to the Session local or remote message.
 *  Using this pointer the Application can inspect and modify the local message
 *  or inspect the remote message.
 *
 *  The local message can represent OFFER or ANSWER. It depends on role of
 *  the Application. The local message is OFFER, if the Application is offerer,
 *  or it is ANSWER, if the Application is answerer.
 *  Offerer builds the local message based on local or default capabilities.
 *  Answerer builds the local message based on local or default capabilities
 *  and received OFFER.
 *  Local capabilities can be set into the Session using
 *  RvOaSessionSetCapabilities function.
 *  Default capabilities can be set into the Session using RvOaSetCapabilities
 *  function.
 *
 *  The remote message can represent OFFER or ANSWER. It depends on role of
 *  the Application. The remote message is ANSWER, if the Application is
 *  offerer, or it is OFFER, if the Application is answerer.
 *  The remote message is built by the Session while parsing the string,
 *  provided to the Session by the Application using RvOaSessionSetRcvdMsg
 *  function.
 *
 * Arguments:
 * Input:  hSession       - The handle to the session.
 * Output: ppSdpMsgLocal  - The pointer to the RADVISION SDP Stack Message
 *                          object, used as a local message. Can be NULL.
 * Output: ppSdpMsgRemote - The pointer to the RADVISION SDP Stack Message
 *                          object, used as a local message. Can be NULL.
 *
 * Return Values: Returns RvStatus.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionGetSdpMsg(
                                    IN  RvOaSessionHandle   hSession,
                                    OUT RvSdpMsg**          ppSdpMsgLocal,
                                    OUT RvSdpMsg**          ppSdpMsgRemote)
{
    RvStatus rv;
    OaSession* pOaSession = (OaSession*)hSession;

    if(NULL==pOaSession)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    if (NULL != ppSdpMsgLocal)
    {
        *ppSdpMsgLocal = pOaSession->msgLocal.pSdpMsg;
    }
    if (NULL != ppSdpMsgRemote)
    {
        *ppSdpMsgRemote = pOaSession->msgRemote.pSdpMsg;
    }

    OaSessionUnlock(pOaSession);

/*
    RvLogDebug(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaSessionGetSdpMsg(hSession=%p) - local msg %p, remote msg %p were found",
        pOaSession, pOaSession->msgLocal.pSdpMsg, pOaSession->msgRemote.pSdpMsg));
*/
    return RV_OK;
}

/*@****************************************************************************
 * RvOaSessionGetState (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets current state of the Session object.
 *  State can be OFFER_READY, ANSWER_RCVD for offerer, ANSWER_READY for
 *  answerer and IDLE or TERMINATED for both sides.
 *
 * Arguments:
 * Input:  hSession - handle of the Session object.
 * Output: peState  - state.
 *
 * Return Value: RV_OK - if successful.
 *               RV_ERROR_NOTFOUND - if remote message doesn't include c-line.
 *               Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionGetState(
                                    IN  RvOaSessionHandle   hSession,
                                    OUT RvOaSessionState*   peState)
{
    RvStatus rv;
    OaSession* pOaSession = (OaSession*)hSession;

    if(NULL==pOaSession || NULL==peState)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    *peState = pOaSession->eState;

    OaSessionUnlock(pOaSession);

/*
    RvLogDebug(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaSessionGetState(hSession=%p) - state %s was got",
        pOaSession, OaUtilsConvertEnum2StrSessionState(*peState)));
*/
    return RV_OK;
}

/*@****************************************************************************
 * RvOaSessionHold (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 * For each stream in the session changes it parameters,
 * that put other side on hold.
 * For more details see put on hold procedure, described in RFC 3264.
 * Call to RvOaSessionGetMsgToBeSent() will bring SDP message,
 * where all media descriptions indicate put on hold.
 *
 * Arguments:
 * Input:  hStream  - handle of the Stream object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionHold(IN  RvOaSessionHandle   hSession)
{
    RvStatus    rv;
    OaSession*  pOaSession = (OaSession*)hSession;

    if(NULL==pOaSession)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    rv = OaSessionHold(pOaSession);
    if (RV_OK != rv)
    {
        OaSessionUnlock(pOaSession);
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "RvOaSessionHold(hSession=%p) - failed to hold(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    OaSessionUnlock(pOaSession);

    RvLogInfo(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaSessionHold(hSession=%p) - all streams were put on hold",
        pOaSession));

    return RV_OK;
}

/*@****************************************************************************
 * RvOaSessionResume (Offer-Answer Session API)
 * ----------------------------------------------------------------------------
 * General:
 * For each stream in the session changes it parameters,
 * that resume the other side from hold.
 * For details see put on hold procedure, described in RFC 3264.
 * Call to RvOaSessionGetMsgToBeSent() will bring SDP message,
 * where all media descriptions indicate resume.
 *
 * Arguments:
 * Input:  hStream  - handle of the Stream object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaSessionResume(IN  RvOaSessionHandle   hSession)
{
    RvStatus    rv;
    OaSession*  pOaSession = (OaSession*)hSession;

    if(NULL==pOaSession)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    rv = OaSessionResume(pOaSession);
    if (RV_OK != rv)
    {
        OaSessionUnlock(pOaSession);
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "RvOaSessionResume(hSession=%p) - failed to resume(rv=%d:%s)",
            pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    OaSessionUnlock(pOaSession);

    RvLogInfo(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaSessionResume(hSession=%p) - all streams were resumed",
        pOaSession));

    return RV_OK;
}

/*---------------------------------------------------------------------------*/
/*                                STREAM API                                 */
/*---------------------------------------------------------------------------*/

/*@****************************************************************************
 * RvOaStreamSetAppHandle (Offer-Answer Stream API)
 * ----------------------------------------------------------------------------
 * General:
 *  Sets the handle to the Application object, served by the Stream object.
 *  Actually the Application handle is sizeof(RvOaAppStreamHandle) bytes of
 *  application data, stored in the Stream object.
 *  This data can be retrieved using RvOaStreamGetAppHandle function.
 *
 * Arguments:
 * Input:  hStream     - handle of the Stream object.
 *         hSession    - handle of the Session object.
 *         hAppSession - handle of the Application object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaStreamSetAppHandle(
                                    IN  RvOaStreamHandle    hStream,
                                    IN  RvOaSessionHandle   hSession,
                                    IN  RvOaAppStreamHandle hAppStream)
{
    RvStatus rv;
    OaSession* pOaSession = (OaSession*)hSession;
    OaStream*  pOaStream = (OaStream*)hStream;

    if(NULL==pOaSession || NULL==pOaStream || NULL==hAppStream)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    pOaStream->hAppHandle = hAppStream;

    OaSessionUnlock(pOaSession);

    RvLogInfo(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaStreamSetAppHandle(hStream=%p) - application handle %p was set",
        pOaStream, hAppStream));

    return RV_OK;
}

/*@****************************************************************************
 * RvOaStreamGetAppHandle (Offer-Answer Stream API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets the handle to the Application object.
 *  The application handle can be set into the Stream object using
 *  RvOaStreamSetAppHandle function.
 *
 * Arguments:
 * Input:  hStream     - handle of the Stream object.
 *         hSession    - handle of the Session object.
 * Output: phAppStream - handle of the Application object.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaStreamGetAppHandle(
                                    IN  RvOaStreamHandle     hStream,
                                    IN  RvOaSessionHandle    hSession,
                                    OUT RvOaAppStreamHandle* phAppStream)
{
    RvStatus rv;
    OaSession* pOaSession = (OaSession*)hSession;
    OaStream*  pOaStream = (OaStream*)hStream;

    if(NULL==pOaSession || NULL==phAppStream)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    *phAppStream = pOaStream->hAppHandle;

    OaSessionUnlock(pOaSession);

/*
    RvLogDebug(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaStreamGetAppHandle(hStream=%p) - application handle %p was get",
        pOaStream, pOaStream->hAppHandle));
*/
    return RV_OK;
}

/*@****************************************************************************
 * RvOaStreamGetMediaDescriptor (Offer-Answer Stream API)
 * ----------------------------------------------------------------------------
 * General:
 *  Gets the pointer to the Media Descriptor subtree of the local message.
 *  Using this pointer the Application can inspect and modify the stream
 *  parameters, contained in the corresponded Media Descriptor subtree.
 *  For more details see documentation for RvOaSessionGetSdpMsg function.
 *
 * Arguments:
 * Input:  hStream      - handle of the Stream object.
 *         hSession     - handle of the Session object.
 * Output: ppMediaDescrLocal  - pointer to the Media Descriptor object from
 *                              locally generated SDP message.
 *                              Can be NULL.
 *         ppMediaDescrRemote - pointer to the Media Descriptor object from
 *                              SDP message received from remote side.
 *                              Can be NULL.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaStreamGetMediaDescriptor(
                                    IN  RvOaStreamHandle    hStream,
                                    IN  RvOaSessionHandle   hSession,
                                    OUT RvSdpMediaDescr**   ppMediaDescrLocal,
                                    OUT RvSdpMediaDescr**   ppMediaDescrRemote)
{
    RvStatus rv;
    OaSession* pOaSession = (OaSession*)hSession;
    OaStream*  pOaStream = (OaStream*)hStream;

    if(NULL==pOaSession || NULL==pOaStream)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    if(NULL!=ppMediaDescrLocal)
    {
        *ppMediaDescrLocal = pOaStream->pMediaDescrLocal;
    }
    if(NULL!=ppMediaDescrRemote)
    {
        *ppMediaDescrRemote = pOaStream->pMediaDescrRemote;
    }

    OaSessionUnlock(pOaSession);

/*
    RvLogDebug(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaStreamGetMediaDescriptor(hStream=%p) - media descriptor was get",
        pOaStream));
*/
    return RV_OK;
}

/*@****************************************************************************
 * RvOaStreamGetStatus (Offer-Answer Stream API)
 * ----------------------------------------------------------------------------
 * General:
 * Gets status of stream.
 * Status of stream can be affected by incoming OFFER.
 * As a result of incoming OFFER processing, the stream can be held, can be
 * resumed, can be modified, etc.
 *
 * Arguments:
 * Input:  hStream  - handle of the Stream object.
 *         hSession - handle of the Session object.
 * Output: pStatus  - status.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaStreamGetStatus(
                                    IN  RvOaStreamHandle    hStream,
                                    IN  RvOaSessionHandle   hSession,
                                    OUT RvOaStreamStatus*   pStatus)
{
    RvStatus rv;
    OaSession* pOaSession = (OaSession*)hSession;
    OaStream*  pOaStream = (OaStream*)hStream;

    if(NULL==pOaSession || NULL==pOaStream || NULL==pStatus)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    pStatus->eState               = pOaStream->eState;
    pStatus->bWasClosed           = pOaStream->bWasClosed;
    pStatus->bWasModified         = pOaStream->bWasModified;
    pStatus->bNewOffered          = pOaStream->bNewOffered;
    pStatus->bAddressWasModified  = pOaStream->bAddressWasModified;

    OaSessionUnlock(pOaSession);

/*
    RvLogDebug(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaStreamGetStatus(hStream=%p) - state=%s, bWasModified=%d, bWasClosed=%d, bNewOffered=%d, bAddrWasModified",
        pOaStream, OaUtilsConvertEnum2StrStreamState(pStatus->eState),
        pStatus->bWasModified, pStatus->bWasClosed, pStatus->bNewOffered));
*/
    return RV_OK;
}

/*@****************************************************************************
 * RvOaStreamHold (Offer-Answer Stream API)
 * ----------------------------------------------------------------------------
 * General:
 * Changes parameters of stream, that put other side on hold.
 * For more details see put on hold procedure, described in RFC 3264.
 * Call to RvOaSessionGetMsgToBeSent() will bring SDP message,
 * where the description of the stream media indicates put on hold.
 *
 * Arguments:
 * Input:  hSession - handle of the Session object.
 *         hStream  - handle of the Stream object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaStreamHold(
                                    IN  RvOaSessionHandle hSession,
                                    IN  RvOaStreamHandle  hStream)
{
    RvStatus rv;
    OaSession* pOaSession = (OaSession*)hSession;
    OaStream*  pOaStream = (OaStream*)hStream;

    if(NULL==pOaSession || NULL==pOaStream)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    rv = OaStreamHold(pOaStream);
    if (RV_OK != rv)
    {
        OaSessionUnlock(pOaSession);
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "RvOaStreamHold(hStream=%p) - failed to put stream on hold", pOaStream));
        return rv;
    }

    OaSessionUnlock(pOaSession);

    RvLogInfo(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaStreamHold(hStream=%p) - stream was put on hold", pOaStream));

    return RV_OK;
}

/*@****************************************************************************
 * RvOaStreamResume (Offer-Answer Stream API)
 * ----------------------------------------------------------------------------
 * General:
 * Changes parameters of stream, that resume the other side from hold.
 * For details see put on hold procedure, described in RFC 3264.
 * Call to RvOaSessionGetMsgToBeSent() will bring SDP message,
 * where the description of the stream media indicates put on hold.
 *
 * Arguments:
 * Input:  hSession - handle of the Session object.
 *         hStream  - handle of the Stream object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 ***************************************************************************@*/
RVAPI RvStatus RVCALLCONV RvOaStreamResume(
                                    IN  RvOaSessionHandle hSession,
                                    IN  RvOaStreamHandle  hStream)
{
    RvStatus rv;
    OaSession* pOaSession = (OaSession*)hSession;
    OaStream*  pOaStream = (OaStream*)hStream;

    if(NULL==pOaSession || NULL==pOaStream)
    {
        return RV_ERROR_BADPARAM;
    }

    rv = OaSessionLock(pOaSession);
    if (RV_OK != rv)
    {
        return rv;
    }

    rv = OaStreamResume(pOaStream);
    if (RV_OK != rv)
    {
        OaSessionUnlock(pOaSession);
        RvLogError(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
            "RvOaStreamResume(hStream=%p) - failed to resume stream", pOaStream));
        return rv;
    }

    OaSessionUnlock(pOaSession);

    RvLogInfo(pOaSession->pMgr->pLogSource,(pOaSession->pMgr->pLogSource,
        "RvOaStreamResume(hStream=%p) - stream was resumed", pOaStream));

    return RV_OK;
}

/*---------------------------------------------------------------------------*/
/*                              STATIC FUNCTIONS                             */
/*---------------------------------------------------------------------------*/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/******************************************************************************
 * LogMsg
 * ----------------------------------------------------------------------------
 * General:
 *  Prints the received message or message to be sent into the log.
 *  The message should represent the proper SDP OFFER or ANSWER.
 *
 * Arguments:
 * Input:  pOaSession - pointer to the Session object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static void RVCALLCONV LogMsg(
                            IN OaSession*   pOaSession,
                            IN RvChar*      strSdpMsg,
                            IN RvBool       bIncoming,
                            IN RvLogSource* pLogSource)
{
    RvChar*   strMessageType = (RVOA_SESSION_STATE_OFFER_READY==pOaSession->eState) ?
                               "ANSWER" : "OFFER";
    RvChar*   strDirection   = (RV_TRUE==bIncoming) ? "received" : "sent";
    RvChar*   strArrow       = (RV_TRUE==bIncoming) ? "<++" : "++>";
    RvChar*   strPadding     = "   ";
    RvChar    strLogOutput[RVOA_MAX_LOG_LINE];
    RvChar    *pLineStart, *pLineEnd;
    RvUint32  lineLen;
    RvChar*   pStrSdpMsgEnd;

    pStrSdpMsgEnd = strSdpMsg + strlen(strSdpMsg);

    switch (pOaSession->eState)
    {
        case RVOA_SESSION_STATE_IDLE:
            strMessageType = "OFFER";
            break;
        case RVOA_SESSION_STATE_OFFER_READY:
            if (RV_TRUE == bIncoming)
            {
                strMessageType = "ANSWER";
            }
            else
            {
                strMessageType = "OFFER";
            }
            break;
        case RVOA_SESSION_STATE_ANSWER_READY:
            if (RV_TRUE == bIncoming)
            {
                strMessageType = "OFFER";
            }
            else
            {
                strMessageType = "ANSWER";
            }
            break;
        case RVOA_SESSION_STATE_ANSWER_RCVD:
            if (RV_TRUE == bIncoming)
            {
                strMessageType = "OFFER";
            }
            else
            {
                strMessageType = "UNDEFINED";
            }
            break;
        default:
            strMessageType = "UNDEFINED";
    }

    RvLogInfo(pLogSource,(pLogSource, "hSession=%p, hAppSession=%p: %s was %s",
        pOaSession, pOaSession->hAppHandle, strMessageType, strDirection));

    /* Log the first line of message, using arrow */
    pLineStart = strSdpMsg;
    pLineEnd   = strchr(strSdpMsg, '\n');
    if (NULL == pLineEnd)
    {
        pLineEnd = pStrSdpMsgEnd;
    }
    lineLen = (RvUint32)(pLineEnd - pLineStart);
    if (0 == lineLen)
    {
        return;
    }
    if (RVOA_MAX_LOG_LINE < lineLen + 1) /* 1 - for '\0' */
    {
        lineLen = RVOA_MAX_LOG_LINE - 1;
    }
    memcpy(strLogOutput, pLineStart, lineLen);
    if (strLogOutput[lineLen-1]==0x0D  ||  strLogOutput[lineLen-1]==0x0A)
    {
        strLogOutput[lineLen-1] = '\0';
    }
    else
    {
        strLogOutput[lineLen] = '\0';
    }

    RvLogInfo(pLogSource,(pLogSource, "%s %s", strArrow, strLogOutput));

    /* Log the rest lines, using padding */
    while(pLineEnd != pStrSdpMsgEnd)
    {
        pLineStart = pLineEnd + 1;  /* Skip '\n' */
        pLineEnd   = strchr(pLineStart, '\n');
        if (NULL == pLineEnd)
        {
            pLineEnd = pStrSdpMsgEnd;
        }
        lineLen    = (RvUint32)(pLineEnd - pLineStart);
        if (0 == lineLen)
        {
            break;
        }
        if (RVOA_MAX_LOG_LINE < lineLen + 1) /* 1 - for '\0' */
        {
            lineLen = RVOA_MAX_LOG_LINE - 1;
        }
        memcpy(strLogOutput, pLineStart, lineLen);
        if (strLogOutput[lineLen-1]==0x0D  ||  strLogOutput[lineLen-1]==0x0A)
        {
            strLogOutput[lineLen-1] = '\0';
        }
        else
        {
            strLogOutput[lineLen] = '\0';
        }

        RvLogInfo(pLogSource,(pLogSource, "%s %s", strPadding, strLogOutput));
    }
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/*nl for linux */

