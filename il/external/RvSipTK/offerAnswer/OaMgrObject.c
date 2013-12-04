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
 *                              <OaMgrObject.c>
 *
 * The OaMgrObject.c file defines Offer-Answer module Manager object.
 * The Manager object serves 2 goals:
 *      1. Provides the Offer-Answer module with resources like memory or locks
 *      2. Stores global data, required by the other module objects.
 *
 * The Manager holds pools of Session and Stream objects, pools of RADVISION
 * SDP Stack Message and Allocator objects and pool of pages, to be used by
 * RADVISION SDP Stack Message objects to store their string data.
 * The Manager holds pool of RADVISION SDP Stack Message objects that are
 * used to store per session capabilities, and hash, where the codecs from
 * these capabilities are kept.
 *
 * The Manager stores global capabilities that are used by the Session objects
 * while constructing OFFER and ANSWER messages.
 *
 *    Author                        Date
 *    ------                        ----
 *    Igor                          Aug 2006
 *****************************************************************************/

/*---------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                            */
/*---------------------------------------------------------------------------*/
#include "rvcbase.h"
#include "rvmutex.h"
#include "rvmemory.h"
#include "rvlog.h"

#include "RvOaTypes.h"
#include "OaMgrObject.h"
#include "OaSessionObject.h"
#include "OaStreamObject.h"
#include "OaSdpMsg.h"
#include "OaUtils.h"

/*---------------------------------------------------------------------------*/
/*                              TYPE DEFINITIONS                             */
/*---------------------------------------------------------------------------*/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)

#define OA_LOG_FILE_NAME        "oa.log"
#define OA_LOG_SOURCE_NAME_LEN  (13)
#define OA_LOG_MSG_PREFIX_LEN   (24)

#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/*---------------------------------------------------------------------------*/
/*                        STATIC FUNCTION DEFENITIONS                        */
/*---------------------------------------------------------------------------*/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static RvStatus RVCALLCONV ConstructLog(IN OaMgr* pOaMgr, IN RvOaCfg* pOaCfg);

static RvStatus RVCALLCONV ConstructLogSource(
                                IN   OaMgr*        pOaMgr,
                                IN   RvOaLogSource eLogSource,
                                IN   RvUint32      logFilter,
                                OUT  RvLogSource** ppLogSource);

static void RVCALLCONV DestructLog(IN OaMgr* pOaMgr);

static void RVCALLCONV OaLogCB(IN RvLogRecord* pLogRecord, IN void* context);

static void RVCALLCONV OaLogMsgFormat(IN RvLogRecord* logRecord,
                                      OUT RvChar** strFinalText);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/


static RvStatus RVCALLCONV InitializeSessionPoolElements(IN OaMgr* pOaMgr);

static void RVCALLCONV DestructSessions(IN OaMgr*  pOaMgr);

static void RVCALLCONV CheckConfiguration(
                            IN  OaMgr*   pOaMgr,
                            IN  RvOaCfg* pOaCfg,
                            OUT RvBool*  pbValidCfg);

/*---------------------------------------------------------------------------*/
/*                            FUNCTION IMPLEMENTATIONS                       */
/*---------------------------------------------------------------------------*/
/******************************************************************************
 * OaMgrConstruct
 * ----------------------------------------------------------------------------
 * General:
 *  Constructs the manager of OA Module using module configuration.
 *  Allocates pools of Session and Stream objects, pool of pages to be used
 *  for SDP messages, etc.
 *
 * Arguments:
 * Input:  pOaCfg    - OA Module configuration.
 * Output: ppOaMgr   - pointer to the OA Module's Manager object.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaMgrConstruct(
                                    IN  RvOaCfg*  pOaCfg,
                                    OUT OaMgr**   ppOaMgr)
{
    RvStatus  rv;
    RvStatus  crv; /* Common Core RvStatus values */
    OaMgr*    pOaMgr;
    RvUint32  maxNumOfMessages;
    RvBool    bValidCfg;

    crv = RvCBaseInit();
    if (RV_OK != crv)
    {
        return RV_ERROR_UNKNOWN;
    }

    /* Allocate the OaMgr object */
    crv = RvMemoryAlloc(NULL /*default region*/, sizeof(OaMgr),
                        (RvLogMgr*)pOaCfg->logHandle, (void**)&pOaMgr);
    if (RV_OK != crv)
    {
        RvCBaseEnd();
        return RV_ERROR_OUTOFRESOURCES;
    }
    memset(pOaMgr, 0, sizeof(OaMgr));

    /* Construct Log Manager and Log Source */
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    rv = ConstructLog(pOaMgr, pOaCfg);
    if (RV_OK != rv)
    {
        RvMemoryFree((void*)pOaMgr,NULL);
        RvCBaseEnd();
        return RV_ERROR_UNKNOWN;
    }
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */

#if !defined(RV_SDP_ADS_IS_USED)
    RvLogError(pOaMgr->pLogSource ,(pOaMgr->pLogSource,
        "OaMgrConstruct - module should be compiled with flag RV_SDP_ADS_IS_USED"));
    OaMgrDestruct(pOaMgr);
    return RV_ERROR_UNKNOWN;
#else

    bValidCfg = RV_TRUE;
    CheckConfiguration(pOaMgr, pOaCfg, &bValidCfg);
    if (bValidCfg == RV_FALSE)
    {
        RvLogError(pOaMgr->pLogSource ,(pOaMgr->pLogSource,
            "OaMgrConstruct - invalid configuration"));
        OaMgrDestruct(pOaMgr);
        return RV_ERROR_UNKNOWN;
    }

    /* Construct Lock */
    crv = RvMutexConstruct(pOaMgr->pLogMgr, &pOaMgr->lock);
    if (crv != RV_OK)
    {
        RvLogError(pOaMgr->pLogSource ,(pOaMgr->pLogSource,
            "OaMgrConstruct - failed to construct Manager mutex (crv=%d:%s)",
            OA_CRV2RV(crv), OaUtilsConvertEnum2StrStatus(crv)));
        OaMgrDestruct(pOaMgr);
        return OA_CRV2RV(crv);
    }
    pOaMgr->pLock = &pOaMgr->lock;

    /* Construct Pool of pages to be used by SDP messages.
       Each Session contains at least 2 SDP messages - Local and Remote.
       The one more message is consumed temporary while handling modifying
       OFFER (see HandleModifyingOffer() function). Let's assume 10% of
       sessions can be in process of the modifying OFFER handling
       simultaneously. */
    maxNumOfMessages = pOaCfg->maxSessions * 2 +
                       pOaCfg->maxSessions * 10/100;
    pOaMgr->hMessagePool = RPOOL_Construct(pOaCfg->messagePoolPageSize,
                                maxNumOfMessages,(RV_LOG_Handle)pOaMgr->pLogMgr,
                                RV_TRUE /*Initialize new page with zeroes*/,
                                "OA_MessagePagePool");
    if (NULL == pOaMgr->hMessagePool)
    {
        RvLogError(pOaMgr->pLogSource ,(pOaMgr->pLogSource,
            "OaMgrConstruct - failed to construct pool of Messages"));
        OaMgrDestruct(pOaMgr);
        return RV_ERROR_UNKNOWN;
    }

    /* Construct Pool and List of Sessions */
    pOaMgr->maxNumOfSessions = pOaCfg->maxSessions;
    pOaMgr->hSessionListPool = RLIST_PoolListConstruct(
        pOaMgr->maxNumOfSessions, 1 /*Total Number of lists*/,
        sizeof(OaSession), pOaMgr->pLogMgr, "OA_SessionPool");
    if (NULL == pOaMgr->hSessionListPool)
    {
        RvLogError(pOaMgr->pLogSource ,(pOaMgr->pLogSource,
        "OaMgrConstruct - failed to construct pool of Sessions"));
        OaMgrDestruct(pOaMgr);
        return RV_ERROR_UNKNOWN;
    }
    pOaMgr->hSessionList = RLIST_ListConstruct(pOaMgr->hSessionListPool);
    if(NULL == pOaMgr->hSessionList)
    {
        RvLogError(pOaMgr->pLogSource ,(pOaMgr->pLogSource,
        "OaMgrConstruct - failed to allocate list of Sessions"));
        OaMgrDestruct(pOaMgr);
        return RV_ERROR_UNKNOWN;
    }

    /* Construct Pool of Streams. Lists of Streams are managed by Session */
    pOaMgr->maxNumOfStreams = pOaCfg->maxStreams;
    pOaMgr->hStreamListPool = RLIST_PoolListConstruct(
                        pOaMgr->maxNumOfStreams,
                        pOaMgr->maxNumOfSessions/*Total Number of lists*/,
                        sizeof(OaStream), pOaMgr->pLogMgr, "OA_StreamPool");
    if (NULL == pOaMgr->hStreamListPool)
    {
        RvLogError(pOaMgr->pLogSource ,(pOaMgr->pLogSource,
        "OaMgrConstruct - failed to construct pool of Streams"));
        OaMgrDestruct(pOaMgr);
        return RV_ERROR_UNKNOWN;
    }

    /* Initialize elements of Session pool.
       IMPORTANT: should be called after Pool of Streams is constrcuted */
    rv = InitializeSessionPoolElements(pOaMgr);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
            "OaMgrConstruct - failed to initialize Session Pool Elements (rv=%d:%s)",
            rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaMgrDestruct(pOaMgr);
        return rv;
    }

    /* Construct Pool of pages to be used by capabilities.
       Each Session can contain no more than 1 SDP message, holding
       capabilities. One more message can be consumed by Default
       Capabilities, set per module. */
    pOaMgr->maxNumOfCaps = pOaCfg->maxSessionsWithLocalCaps + 1;
    pOaMgr->hCapPool = RPOOL_Construct(pOaCfg->capPoolPageSize,
                           pOaMgr->maxNumOfCaps,(RV_LOG_Handle)pOaMgr->pLogMgr,
                           RV_TRUE /*Initialize new page with zeroes*/,
                           "OA_CapabilityPagePool");
    if (NULL == pOaMgr->hCapPool)
    {
        RvLogError(pOaMgr->pLogSource ,(pOaMgr->pLogSource,
            "OaMgrConstruct - failed to construct pool of Capabilities"));
        OaMgrDestruct(pOaMgr);
        return RV_ERROR_UNKNOWN;
    }
    /* Construct Pool of Allocators to be used by capabilities.
       Capabilities are kept in SDP messages. Each SDP message requires a one
       Allocator object. */
    pOaMgr->hRaCapAllocs = RA_Construct(sizeof(RvAlloc), pOaMgr->maxNumOfCaps,
                                        NULL /*print func*/, pOaMgr->pLogMgr,
                                        "OA_AllocatorsPool");
    if (NULL == pOaMgr->hRaCapAllocs)
    {
        RvLogError(pOaMgr->pLogSource ,(pOaMgr->pLogSource,
            "OaMgrConstruct - failed to construct pool of Allocators"));
        OaMgrDestruct(pOaMgr);
        return RV_ERROR_UNKNOWN;
    }
    /* Construct Pool of SDP Message objects to be used by capabilities.
       Capabilities are kept in SDP messages. Each SDP message requires a one
       SDP message object. */
    pOaMgr->hRaCapMsgs = RA_Construct(sizeof(RvSdpMsg), pOaMgr->maxNumOfCaps,
                                       NULL /*print func*/, pOaMgr->pLogMgr,
                                       "OA_MsgsPool");
    if (NULL == pOaMgr->hRaCapMsgs)
    {
        RvLogError(pOaMgr->pLogSource ,(pOaMgr->pLogSource,
            "OaMgrConstruct - failed to construct pool of SDP Messages"));
        OaMgrDestruct(pOaMgr);
        return RV_ERROR_UNKNOWN;
    }

    /* Construct Hash for codecs */
    rv = OaCodecHashConstruct(
            (pOaMgr->maxNumOfCaps * pOaCfg->avgNumOfMediaFormatsPerCaps),
            pOaMgr->maxNumOfCaps, pOaMgr->pLogMgr, pOaMgr->pLogSourceCaps,
            pOaMgr->pLock, &pOaMgr->hashCodecs);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
            "OaMgrConstruct - failed to construct hash for codecs(rv=%d:%s)",
            rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaMgrDestruct(pOaMgr);
        return rv;
    }

    /* Allocate memory for temporary buffer that hold  capabilities string */
    crv = RvMemoryAlloc(NULL /*default region*/, OA_CAPS_STRING_LEN,
                (RvLogMgr*)pOaCfg->logHandle, (void**)&pOaMgr->pCapBuff);
    if (RV_OK != crv)
    {
        rv = OA_CRV2RV(rv);
        RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
            "OaMgrConstruct - failed to construct hash for codecs(rv=%d:%s)",
            rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaMgrDestruct(pOaMgr);
        return rv;
    }

    /* Set format parameters derivation callback */
    pOaMgr->pfnDeriveFormatParams = pOaCfg->pfnDeriveFormatParams;

    /* Set bSetOneCodecPerMediaDescr flag */
    pOaMgr->bChooseOneFormatOnly = pOaCfg->bSetOneCodecPerMediaDescr;

    *ppOaMgr = pOaMgr;

    RvLogInfo(pOaMgr->pLogSource ,(pOaMgr->pLogSource,
        "OaMgrConstruct(pOaMgr=%p) - Offer-Answer Manager was constructed",pOaMgr));
    RvLogInfo(pOaMgr->pLogSource ,(pOaMgr->pLogSource,
        "***************************************************"));

    return RV_OK;
#endif /* ENDOF: #if !defined(RV_SDP_ADS_IS_USED) else */
}

/******************************************************************************
 * OaMgrDestruct
 * ----------------------------------------------------------------------------
 * General:
 *  Destructs the Manager object and free all resources, used by the OA Module.
 *
 * Arguments:
 * Input:  pOaMgr - pointer to the OA Module's Manager object to be destructed.
 * Output: none.
 *
 * Return Value: none
 *****************************************************************************/
void RVCALLCONV OaMgrDestruct(IN OaMgr* pOaMgr)
{
    RvLogMgr* pLogMgr;

    /* If LOg Manager was provided by the application, don't use at this stage
       in order to redice chance to use Log Manager that was already destroyed
       by the application by mistake. */
    pLogMgr = (pOaMgr->bLogMgrProvidedByAppl==RV_TRUE) ? NULL:pOaMgr->pLogMgr;

    /* Destruct Sessions */
    if (NULL != pOaMgr->hSessionList)
    {
        DestructSessions(pOaMgr);
    }

    /* Destruct Pool of Streams. Do this after the Session were destructed. */
    if (NULL != pOaMgr->hStreamListPool)
    {
        RLIST_PoolListDestruct(pOaMgr->hStreamListPool);
        pOaMgr->hStreamListPool = NULL;
    }

    /* Destruct Default Capabilities message if it was constructed */
    if (NULL != pOaMgr->hCodecHashElements)
    {
        RLIST_ListDestruct(pOaMgr->hashCodecs.hHashElemPool,
                           pOaMgr->hCodecHashElements);
        pOaMgr->hCodecHashElements = NULL;
    }
    if (NULL != pOaMgr->defCapMsg.pSdpMsg)
    {
        OaUtilsOaPSdpMsgDestruct(&pOaMgr->defCapMsg);
    }

    /* Free memory of temporary buffer */
    if (NULL != pOaMgr->pCapBuff)
    {
        RvMemoryFree(pOaMgr->pCapBuff, pLogMgr);
        pOaMgr->pCapBuff = NULL;
    }

    /* Destruct Pool of Capability Allocators */
    if (NULL != pOaMgr->hRaCapAllocs)
    {
        RA_Destruct(pOaMgr->hRaCapAllocs);
        pOaMgr->hRaCapAllocs = NULL;
    }
    /* Destruct Pool of Capability Messages */
    if (NULL != pOaMgr->hRaCapMsgs)
    {
        RA_Destruct(pOaMgr->hRaCapMsgs);
        pOaMgr->hRaCapMsgs = NULL;
    }
    /* Destruct Pool of Capabilities Pages */
    if (NULL != pOaMgr->hCapPool)
    {
        RPOOL_Destruct(pOaMgr->hCapPool);
        pOaMgr->hCapPool = NULL;
    }
    /* Destruct hash of codecs */
    OaCodecHashDestruct(&pOaMgr->hashCodecs);

    /* Destruct Pool of Message Pages */
    if (NULL != pOaMgr->hMessagePool)
    {
        RPOOL_Destruct(pOaMgr->hMessagePool);
        pOaMgr->hMessagePool = NULL;
    }

    if (NULL != pOaMgr->pLock)
    {
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
        RvInt32 dummyCounter;
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/
        RvMutexRelease(&pOaMgr->lock, pLogMgr, &dummyCounter);
        RvMutexDestruct(&pOaMgr->lock, pLogMgr);
        pOaMgr->pLock = NULL;
    }

    RvLogInfo(pOaMgr->pLogSource ,(pOaMgr->pLogSource,
        "OaMgrDestruct(pOaMgr=%p) - Offer-Answer Manager was destructed",pOaMgr));
    RvLogInfo(pOaMgr->pLogSource ,(pOaMgr->pLogSource,
        "***************************************************"));

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    DestructLog(pOaMgr);
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */

    RvMemoryFree(pOaMgr,NULL);
    RvCBaseEnd();
}

/******************************************************************************
 * OaGetResources
 * ----------------------------------------------------------------------------
 * General:
 *  Gets various statistics about consumption of resources by the OA Module.
 *
 * Arguments:
 * Input:  pOaMgr          - pointer to the OA Module Manager.
 * Output: pOaResources    - resource consumption statistics.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaGetResources(
                                   IN OaMgr* pOaMgr,
                                   OUT RvOaResources* pResources)
{
    RvUint32 dummyNumOfListsAllocated;
    RvUint32 dummyNumOfListsCurr;
    RvUint32 dummyNumOfListsMaxUsage;

    /* Get the Session resource status */
    if (NULL != pOaMgr->hSessionListPool)
    {
        RLIST_GetResourcesStatus(pOaMgr->hSessionListPool,
            &dummyNumOfListsAllocated,
            &pResources->sessions.numOfAllocatedElements,
            &dummyNumOfListsCurr,
            &pResources->sessions.currNumOfUsedElements,
            &dummyNumOfListsMaxUsage,
            &pResources->sessions.maxUsageOfElements);
    }

    /* Get the Stream resource status */
    if (NULL != pOaMgr->hStreamListPool)
    {
        RLIST_GetResourcesStatus(pOaMgr->hStreamListPool,
            &dummyNumOfListsAllocated,
            &pResources->streams.numOfAllocatedElements,
            &dummyNumOfListsCurr,
            &pResources->streams.currNumOfUsedElements,
            &dummyNumOfListsMaxUsage,
            &pResources->streams.maxUsageOfElements);
    }

    /* Get the Message Pool resource status */
    if (NULL != pOaMgr->hMessagePool)
    {
        RPOOL_GetResources(pOaMgr->hMessagePool,
            &pResources->messagePoolPages.numOfAllocatedElements,
            &pResources->messagePoolPages.currNumOfUsedElements,
            &pResources->messagePoolPages.maxUsageOfElements);
    }

    /* Get the Capability Pool resource status */
    if (NULL != pOaMgr->hCapPool)
    {
        RPOOL_GetResources(pOaMgr->hCapPool,
            &pResources->capPoolPages.numOfAllocatedElements,
            &pResources->capPoolPages.currNumOfUsedElements,
            &pResources->capPoolPages.maxUsageOfElements);
    }

    return RV_OK;
}

/******************************************************************************
 * OaPrintConfigParamsToLog
 * ----------------------------------------------------------------------------
 * General:
 *  Prints configuration parameters, provided by the Application while
 *  constructing module, into the log.
 *
 * Arguments:
 * Input:  pOaMgr - pointer to the OA Module Manager.
 *         pOaCfg - configuration, used for construction of OA Module Manager.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
void RVCALLCONV OaPrintConfigParamsToLog(IN OaMgr*   pOaMgr,
                                         IN RvOaCfg* pOaCfg)
{
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    RvChar strLogFilters[RVOA_LOGMASK_STRLEN];

    /* Compilation configuration */
    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "[======Offer-Answer Compilation Parameters=========]"));
#ifndef RV_DEBUG
    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "pOaMgr=%p: RV_DEBAUG is off", pOaMgr));
#else
    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "pOaMgr=%p: RV_DEBUG is on", pOaMgr));
#endif

    /* Add-on configuration */
    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "[===========Offer-Answer Configuration=============]"));
    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "pOaMgr=%p: maxNumOfSessions = %d",pOaMgr,pOaMgr->maxNumOfSessions));
    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "pOaMgr=%p: maxNumOfStreams = %d",pOaMgr,pOaMgr->maxNumOfStreams));
    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "pOaMgr=%p: messagePoolPageSize = %d",pOaMgr,pOaCfg->messagePoolPageSize));
    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "pOaMgr=%p: maxSessionsWithLocalCaps = %d",pOaMgr,pOaCfg->maxSessionsWithLocalCaps));
    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "pOaMgr=%p: capPoolPageSize = %d",pOaMgr,pOaCfg->capPoolPageSize));
    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "pOaMgr=%p: avgNumOfMediaFormatsPerCaps = %d",pOaMgr,pOaCfg->avgNumOfMediaFormatsPerCaps));
    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "pOaMgr=%p: bSetOneCodecPerMediaDescr = %d",pOaMgr,pOaMgr->bChooseOneFormatOnly));
    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "pOaMgr=%p: bLogFuncProvidedByAppl = %d",pOaMgr,pOaMgr->bLogFuncProvidedByAppl));
    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "pOaMgr=%p: logFilters = %s",pOaMgr,
        OaUtilsConvertEnum2StrLogMask(pOaMgr->logFilter, strLogFilters)));
    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "pOaMgr=%p: MsglogFilters = %s",pOaMgr,
        OaUtilsConvertEnum2StrLogMask(pOaMgr->logFilterMsgs, strLogFilters)));
    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "pOaMgr=%p: CapsLogFilters = %s",pOaMgr,
        OaUtilsConvertEnum2StrLogMask(pOaMgr->logFilterCaps, strLogFilters)));
    RvLogInfo(pOaMgr->pLogSource,(pOaMgr->pLogSource,
        "[==================================================]"));
#else
	RV_UNUSED_ARGS((pOaCfg,pOaMgr));
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/******************************************************************************
 * OaSetLogFilters
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
 * Input:  pOaMgr     - pointer to the OA Module Manager.
 *         eLogSource - source of data to be logged.
 *                      Example of log source can be a Capabilitites related
 *                      code, or Message handling functions etc.
 *         filters    - filter mask to be set.
 * Output: none
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaSetLogFilters(
                                IN OaMgr*         pOaMgr,
                                IN RvOaLogSource  eLogSource,
                                IN RvInt32        filters)
{
    RvStatus crv;
    RvLogSource* pLogSource;
    RvChar strLogFilters[RVOA_LOGMASK_STRLEN];

    switch (eLogSource)
    {
        case RVOA_LOG_SRC_GENERAL:
            pLogSource = pOaMgr->pLogSource; break;
        case RVOA_LOG_SRC_CAPS:
            pLogSource = pOaMgr->pLogSourceCaps; break;
        case RVOA_LOG_SRC_MSGS:
            pLogSource = pOaMgr->pLogSourceMsgs; break;
        default:
            RvLogError (pOaMgr->pLogSource, (pOaMgr->pLogSource,
                "OaSetLogFilters(pOaMgr=%p) - enexpected type of source (%d)",
                pOaMgr, eLogSource));
            return RV_ERROR_BADPARAM;
    }

    crv = RvLogSourceSetMask(pLogSource, OaUtilsConvertOa2CcLogMask(filters));
    if (RV_OK != crv)
    {
        RvLogError (pOaMgr->pLogSource, (pOaMgr->pLogSource,
            "OaSetLogFilters(pOaMgr=%p) - failed to set new filters for log source %s (crv=%d:%s)",
            pOaMgr, OaUtilsConvertEnum2StrLogSource(eLogSource),
            OA_CRV2RV(crv), OaUtilsConvertEnum2StrStatus(crv)));
        return OA_CRV2RV(crv);
    }

    switch (eLogSource)
    {
        case RVOA_LOG_SRC_GENERAL:
            pOaMgr->logFilter = filters; break;
        case RVOA_LOG_SRC_CAPS:
            pOaMgr->logFilterCaps = filters; break;
        case RVOA_LOG_SRC_MSGS:
            pOaMgr->logFilterMsgs = filters; break;
        default:
            break;
    }

    RvLogInfo(pOaMgr->pLogSource, (pOaMgr->pLogSource,
        "OaSetLogFilters(pOaMgr=%p) - new filters were set for log source %s: %s",
        pOaMgr, OaUtilsConvertEnum2StrLogSource(eLogSource),
        OaUtilsConvertEnum2StrLogMask(filters, strLogFilters)));

    return RV_OK;
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/******************************************************************************
 * OaSetCapabilities
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
 * Input:  pOaMgr    - pointer to the OA Module Manager.
 *         strSdpMsg - default capabilities as a NULL-terminated string.
 *                     Can be NULL.
 *         pSdpMsg   - default capabilities in form of RADVISION SDP Stack
 *                     message.
 *                     Can be NULL.
 *                     Has lower priority than strSdpMsg.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaSetCapabilities(
                                    IN OaMgr*    pOaMgr,
                                    IN RvChar*   strSdpMsg,
                                    IN RvSdpMsg* pSdpMsg)
{
    RvStatus rv;
    OaCodecHash* pHashCodecs = &pOaMgr->hashCodecs;

    if (NULL != strSdpMsg  &&
        strlen(strSdpMsg) >= OA_CAPS_STRING_LEN)
    {
        RvLogError(pOaMgr->pLogSourceCaps, (pOaMgr->pLogSourceCaps,
            "OaSetCapabilities(pOaMgr=%p) - SDP message string is too long(%d>=%d)",
            pOaMgr, strlen(strSdpMsg), OA_CAPS_STRING_LEN));
        return RV_ERROR_UNKNOWN;
    }

    /* If the capabilities were set already, remove their codecs from hash
       before they will be overwritten. */
    if (NULL != pOaMgr->hCodecHashElements)
    {
        OaCodecHashRemoveElements(pHashCodecs, pOaMgr->hCodecHashElements);
    }

    /* If this is the first time when the capabilities are set, create list
       of references to hash elements */
    if (NULL == pOaMgr->hCodecHashElements)
    {
        RvMutexLock(pHashCodecs->pLock, pHashCodecs->pLogMgr);
        pOaMgr->hCodecHashElements = RLIST_ListConstruct(pHashCodecs->hHashElemPool);
        RvMutexUnlock(pHashCodecs->pLock, pHashCodecs->pLogMgr);
        if (NULL == pOaMgr->hCodecHashElements)
        {
            RvLogError(pOaMgr->pLogSourceCaps, (pOaMgr->pLogSourceCaps,
                "OaSetCapabilities(pOaMgr=%p) - failed to create list for hash elements",
                pOaMgr));
            return RV_ERROR_UNKNOWN;
        }
    }

    /* Create new Message from string or from the application Message */
    if (NULL != strSdpMsg)
    {
        /* Copy the provided string into the temporary buffer.
           SDP Stack requires the string to be allocated on heap. Application
           may provide string from static memory (eg. defined with define).
           To ensure the dynamic memory for SDP Stack input, use temporary
           buffer. */
        strcpy(pOaMgr->pCapBuff, strSdpMsg);

        rv = OaUtilsOaPSdpMsgConstructParse(
                pOaMgr->hRaCapAllocs, pOaMgr->hRaCapMsgs, pOaMgr->hCapPool,
                &pOaMgr->defCapMsg, pOaMgr->pCapBuff, pOaMgr->pLogSource,
                (void*)pOaMgr);
        if (RV_OK != rv)
        {
            RvLogError(pOaMgr->pLogSourceCaps, (pOaMgr->pLogSourceCaps,
                "OaSetCapabilities(pOaMgr=%p) - failed to parse SDP message string(rv=%d:%s)",
                pOaMgr, rv, OaUtilsConvertEnum2StrStatus(rv)));
            return RV_ERROR_UNKNOWN;
        }
    }
    else
    {
        rv = OaUtilsOaPSdpMsgConstructCopy(
                pOaMgr->hRaCapAllocs, pOaMgr->hRaCapMsgs, pOaMgr->hCapPool,
                &pOaMgr->defCapMsg, pSdpMsg, pOaMgr->pLogSource,(void*)pOaMgr);
        if (RV_OK != rv)
        {
            RvLogError(pOaMgr->pLogSourceCaps, (pOaMgr->pLogSourceCaps,
                "OaSetCapabilities(pOaMgr=%p) - failed to copy SDP message(rv=%d:%s)",
                pOaMgr, rv, OaUtilsConvertEnum2StrStatus(rv)));
            return RV_ERROR_UNKNOWN;
        }
    }

    /* Initialize the message: set default mandatory data */
    rv = OaSdpMsgSetDefaultData(pOaMgr->defCapMsg.pSdpMsg, pOaMgr->pLogSource);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSourceCaps,(pOaMgr->pLogSourceCaps,
            "OaSetCapabilities(pOaMgr=%p) - failed to set default data into message(rv=%d:%s)",
            pOaMgr, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

    /* Insert Capabilities codecs into hash */
    rv = OaCodecHashInsertSdpMsgCodecs(pHashCodecs, pOaMgr->defCapMsg.pSdpMsg,
                                    (void*)pOaMgr, pOaMgr->hCodecHashElements);
    if (RV_OK != rv)
    {
        RvLogError(pOaMgr->pLogSourceCaps,(pOaMgr->pLogSourceCaps,
            "OaSetCapabilities(pOaMgr=%p) - failed to insert capabilities codecs into hash(rv=%d:%s)",
            pOaMgr, rv, OaUtilsConvertEnum2StrStatus(rv)));
        return rv;
    }

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    if (pOaMgr->logFilterCaps & RV_LOGLEVEL_DEBUG)
    {
        RvLogDebug(pOaMgr->pLogSourceCaps,(pOaMgr->pLogSourceCaps,
            "OaSetCapabilities(pOaMgr=%p) - following default capabilities were set, rv=%d (%s)",
            pOaMgr, rv, OaUtilsConvertEnum2StrStatus(rv)));
        OaSdpMsgLogCapabilities(pOaMgr->defCapMsg.pSdpMsg, pOaMgr->pLogSource);
    }
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

    return RV_OK;
}

/******************************************************************************
 * OaGetCapabilities
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
 * Input:  pOaMgr         - pointer to the OA Module Manager.
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
 *****************************************************************************/
RvStatus RVCALLCONV OaGetCapabilities(
                                    IN  OaMgr*    pOaMgr,
                                    IN  RvUint32  lenOfStrSdpMsg,
                                    IN  RvAlloc*  pSdpAllocator,
                                    OUT RvChar*   strSdpMsg,
                                    OUT RvSdpMsg* pSdpMsg)
{
    /* Get message in string format */
    if (NULL != strSdpMsg)
    {
        RvChar*     strRv;
        RvSdpStatus srv;

        strRv = rvSdpMsgEncodeToBuf(pOaMgr->defCapMsg.pSdpMsg, strSdpMsg,
                                    lenOfStrSdpMsg, &srv);
        if (NULL == strRv)
        {
            RvLogError(pOaMgr->pLogSourceCaps, (pOaMgr->pLogSourceCaps,
                "OaGetCapabilities(pOaMgr=%p) - failed to parse SDP message string(status=%d:%s)",
                pOaMgr, srv, OaUtilsConvertEnum2StrSdpStatus(srv)));
            return OA_SRV2RV(srv);
        }
    }

    /* Get message in SDP Message format */
    if (NULL != pSdpMsg)
    {
        RvSdpMsg* sdpRv;
        sdpRv = rvSdpMsgConstructCopyA(pSdpMsg, pOaMgr->defCapMsg.pSdpMsg,
                                       pSdpAllocator);
        if (NULL == sdpRv)
        {
            RvLogError(pOaMgr->pLogSourceCaps, (pOaMgr->pLogSourceCaps,
                "OaGetCapabilities(pOaMgr=%p) - failed to Copy-Construct SDP message",
                pOaMgr));
            return RV_ERROR_UNKNOWN;
        }
    }

    return RV_OK;
}

/******************************************************************************
 * OaMgrLock
 * ----------------------------------------------------------------------------
 * General:
 *  Locks the Manager object.
 *
 * Arguments:
 * Input:  pOaMgr - pointer to the OA Module Manager.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaMgrLock(IN OaMgr* pOaMgr)
{
    RvStatus crv;

    crv = RvMutexLock(pOaMgr->pLock, pOaMgr->pLogMgr);
    if (RV_OK != crv)
    {
        if (NULL != pOaMgr->pLogSource)
        {
            RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                "OaMgrLock(pOaMgr=%p) - failed to lock Manager lock %p (crv=%d:%s)",
                pOaMgr, pOaMgr->pLock, OA_CRV2RV(crv),
                OaUtilsConvertEnum2StrStatus(crv)));
        }
        return OA_CRV2RV(crv);
    }
    return RV_OK;
}

/******************************************************************************
 * OaMgrUnlock
 * ----------------------------------------------------------------------------
 * General:
 *  Unlocks the Manager object.
 *
 * Arguments:
 * Input:  pOaMgr - pointer to the OA Module Manager.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
void RVCALLCONV OaMgrUnlock(IN OaMgr* pOaMgr)
{
    RvStatus crv;

    crv = RvMutexUnlock(pOaMgr->pLock, pOaMgr->pLogMgr);
    if (RV_OK != crv)
    {
        if (NULL != pOaMgr->pLogSource)
        {
            RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                "OaMgrUnlock(pOaMgr=%p) - failed to unlock Manager lock %p (crv=%d:%s)",
                pOaMgr, pOaMgr->pLock, OA_CRV2RV(crv),
                OaUtilsConvertEnum2StrStatus(crv)));
        }
    }
}

/*---------------------------------------------------------------------------*/
/*                        STATIC FUNCTION IMPLEMENTATIONS                    */
/*---------------------------------------------------------------------------*/
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/******************************************************************************
 * ConstructLog
 * ----------------------------------------------------------------------------
 * General:
 *  Constructs and initializes structures, used for logging.
 *  Creates Log Manager object, if it was not provided by the Application,
 *  creates module's log source, opens file for logging, if the Application
 *  didn't provide the print callback, etc.
 *
 * Arguments:
 * Input:  pOaMgr - pointer to the OA Module Manager.
 *         pOaCfg - module configuration, provided by the Application.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
static RvStatus RVCALLCONV ConstructLog(IN OaMgr* pOaMgr, IN RvOaCfg* pOaCfg)
{
    RvStatus crv, rv;

    /* Construct Log Manager, if it was not supplied by the Application */
    if (NULL != pOaCfg->logHandle)
    {
        pOaMgr->pLogMgr = (RvLogMgr*)pOaCfg->logHandle;
        pOaMgr->bLogMgrProvidedByAppl = RV_TRUE;
    }
    else
    {
        crv = RvLogConstruct(&pOaMgr->logMgr);
        if (RV_OK != crv)
        {
            return OA_CRV2RV(crv);
        }
        pOaMgr->pLogMgr = &pOaMgr->logMgr;

        if (NULL == pOaCfg->pfnLogEntryPrint)
        {
            crv = RvLogListenerConstructLogfile(&pOaMgr->listener,
                    pOaMgr->pLogMgr, OA_LOG_FILE_NAME, 1 /*numFiles*/,
                    0 /*fileSize*/, RV_TRUE /*flushEachMessage*/);
            pOaMgr->bLogFuncProvidedByAppl = RV_FALSE;
        }
        else
        {
            pOaMgr->pfnLogEntryPrint = pOaCfg->pfnLogEntryPrint;
            crv = RvLogRegisterListener(pOaMgr->pLogMgr,OaLogCB,(void*)pOaMgr);
            pOaMgr->bLogFuncProvidedByAppl = RV_TRUE;
        }
        if (RV_OK != crv)
        {
            DestructLog(pOaMgr);
            return OA_CRV2RV(crv);
        }
    } /* ENDOF: if (NULL != pOaCfg->logHandle) else */

    /* Construct Offer-Answer Log Sources */
    rv = ConstructLogSource(pOaMgr, RVOA_LOG_SRC_GENERAL, pOaCfg->logFilter,
                            &pOaMgr->pLogSource);
    if(RV_OK != rv)
    {
        DestructLog(pOaMgr);
        return rv;
    }
    rv = ConstructLogSource(pOaMgr, RVOA_LOG_SRC_CAPS, pOaCfg->logFilterCaps,
                            &pOaMgr->pLogSourceCaps);
    if(RV_OK != rv)
    {
        DestructLog(pOaMgr);
        return rv;
    }
    rv = ConstructLogSource(pOaMgr, RVOA_LOG_SRC_MSGS, pOaCfg->logFilterMsgs,
                            &pOaMgr->pLogSourceMsgs);
    if(RV_OK != rv)
    {
        DestructLog(pOaMgr);
        return rv;
    }

    return RV_OK;
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/******************************************************************************
 * ConstructLogSource
 * ----------------------------------------------------------------------------
 * General:
 *  Construct source of logs and initializes it with log filters.
 *
 * Arguments:
 * Input:  pOaMgr      - pointer to the OA Module Manager.
 *         eLogSource  - type of log source to be created.
 *         logFilter   - filter to be set into the log source.
 * Output: ppLogSource - the constructed log source.
 *
 * Return Value: none.
 *****************************************************************************/
static RvStatus RVCALLCONV ConstructLogSource(
                                IN   OaMgr*        pOaMgr,
                                IN   RvOaLogSource eLogSource,
                                IN   RvUint32      logFilter,
                                OUT  RvLogSource** ppLogSource)
{
    RvStatus crv;
    /* Construct log source */
    crv = RvMemoryAlloc(NULL /*default region*/, sizeof(RvLogSource),
                        pOaMgr->pLogMgr, (void*)ppLogSource);
    if(RV_OK != crv)
    {
        return OA_CRV2RV(crv);
    }
    crv = RvLogSourceConstruct(pOaMgr->pLogMgr, *ppLogSource, 
            OaUtilsConvertEnum2StrLogSource(eLogSource), "Offer-Answer");
    if(RV_OK != crv)
    {
        return OA_CRV2RV(crv);
    }
    /* Set filters for log source */
    crv = OaSetLogFilters(pOaMgr, eLogSource, logFilter);
    if (RV_OK != crv)
    {
        return OA_CRV2RV(crv);
    }
    return RV_OK;
}
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/******************************************************************************
 * DestructLog
 * ----------------------------------------------------------------------------
 * General:
 *  Destructs structures, used for logging, and frees their memory.
 *  Destructs Log Manager object, if it was not provided by the Application,
 *  destructs module's log source, closes file for logging if it was opened,etc.
 *
 * Arguments:
 * Input:  pOaMgr - pointer to the OA Module Manager.
 * Output: none.
 *
 * Return Value: none.
 *****************************************************************************/
static void RVCALLCONV DestructLog(IN OaMgr* pOaMgr)
{
    RvStatus crv;

    /* Destruct Log Source and free it's memory */
    if (NULL != pOaMgr->pLogSourceCaps)
    {
        crv = RvLogSourceDestruct(pOaMgr->pLogSourceCaps);
        if (RV_OK != crv)
        {
            RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                "DestructLog(pOaMgr=%p) - failed to destruct Log Source %p for Capabailities (crv=%d:%s)",
                pOaMgr, pOaMgr->pLogSource, OA_CRV2RV(crv),
                OaUtilsConvertEnum2StrStatus(crv)));
            return;
        }
        RvMemoryFree(pOaMgr->pLogSourceCaps, pOaMgr->pLogMgr);
        pOaMgr->pLogSourceCaps = NULL;
    }
    if (NULL != pOaMgr->pLogSourceMsgs)
    {
        crv = RvLogSourceDestruct(pOaMgr->pLogSourceMsgs);
        if (RV_OK != crv)
        {
            RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                "DestructLog(pOaMgr=%p) - failed to destruct Log Source %p for Messages (crv=%d:%s)",
                pOaMgr, pOaMgr->pLogSource, OA_CRV2RV(crv),
                OaUtilsConvertEnum2StrStatus(crv)));
            return;
        }
        RvMemoryFree(pOaMgr->pLogSourceMsgs, pOaMgr->pLogMgr);
        pOaMgr->pLogSourceMsgs = NULL;
    }
    if (NULL != pOaMgr->pLogSource)
    {
        crv = RvLogSourceDestruct(pOaMgr->pLogSource);
        if (RV_OK != crv)
        {
            RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                "DestructLog(pOaMgr=%p) - failed to destruct Log Source %p (crv=%d:%s)",
                pOaMgr, pOaMgr->pLogSource, OA_CRV2RV(crv),
                OaUtilsConvertEnum2StrStatus(crv)));
            return;
        }
        RvMemoryFree(pOaMgr->pLogSource, pOaMgr->pLogMgr);
        pOaMgr->pLogSource = NULL;
    }

    /* Destruct Listener and Log Manager and free their memory */
    if (NULL != pOaMgr->pLogMgr)
    {
        if (RV_FALSE == pOaMgr->bLogMgrProvidedByAppl)
        {
            if (RV_TRUE == pOaMgr->bLogFuncProvidedByAppl)
            {
                crv = RvLogUnregisterListener(pOaMgr->pLogMgr, OaLogCB);
                pOaMgr->pfnLogEntryPrint = NULL;
            }
            else
            {
                RvLogListenerDestructLogfile(&pOaMgr->listener);
            }
            RvLogDestruct(pOaMgr->pLogMgr);
            RvMemoryFree(pOaMgr->pLogMgr, NULL/*pOaMgr->pLogMgr*/);
        }
        pOaMgr->pLogMgr = NULL;
    }
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/******************************************************************************
 * OaLogCB
 * ----------------------------------------------------------------------------
 * General:
 *  Calls the Application print callback, while formatting the log string to be
 *  passed to the Application.
 *
 * Arguments:
 * Input:  pLogRecord - log string.
 *         context    - pointer to the OA Module Manager.
 * Output: none.
 *
 * Return Value: none.
 *****************************************************************************/
static void RVCALLCONV OaLogCB(IN RvLogRecord* pLogRecord, IN void* context)
{
    OaMgr*  pOaMgr = (OaMgr*)context;
    RvChar* strFinalText;

    OaLogMsgFormat(pLogRecord, &strFinalText);

    if (NULL != pOaMgr->pfnLogEntryPrint)
    {
        pOaMgr->pfnLogEntryPrint(pOaMgr->logEntryPrintContext,
            OaUtilsConvertCc2OaLogFilter(pLogRecord->messageType),
            strFinalText);
    }
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/******************************************************************************
 * OaLogMsgFormat
 * ----------------------------------------------------------------------------
 * General:
 *  Formats string to be printed into log:
 *  adds source name and used filter before the string.
 *
 * Arguments:
 * Input:  pLogRecord   - log string.
 * Output: strFinalText - formatted string.
 *
 * Return Value: none.
 *****************************************************************************/
static void RVCALLCONV OaLogMsgFormat(IN RvLogRecord* logRecord,
                                      OUT RvChar**    strFinalText)
{
    RvChar*       ptr;
    const RvChar* strSourceName = RvLogSourceGetName(logRecord->source);
    const RvChar* mtypeStr;

    /* Find the message type string */
    switch (RvLogRecordGetMessageType(logRecord))
    {
        case RV_LOGID_EXCEP:   mtypeStr = "EXCEP  - "; break;
        case RV_LOGID_ERROR:   mtypeStr = "ERROR  - "; break;
        case RV_LOGID_WARNING: mtypeStr = "WARN   - "; break;
        case RV_LOGID_INFO:    mtypeStr = "INFO   - "; break;
        case RV_LOGID_DEBUG:   mtypeStr = "DEBUG  - "; break;
        case RV_LOGID_ENTER:   mtypeStr = "ENTER  - "; break;
        case RV_LOGID_LEAVE:   mtypeStr = "LEAVE  - "; break;
        case RV_LOGID_SYNC:    mtypeStr = "SYNC   - "; break;
        default:
           mtypeStr = "NOT_VALID"; break;
    }

    /* Format the message type */
    ptr = (char *)logRecord->text - OA_LOG_MSG_PREFIX_LEN;
    *strFinalText = ptr;

    /* Format the message type */
    /*ptr = (char *)strFinalText;*/
    strcpy(ptr, mtypeStr);
    ptr += strlen(ptr);

    /* Pad the module name */
    memset(ptr, ' ', OA_LOG_SOURCE_NAME_LEN+2);

    memcpy(ptr, strSourceName, strlen(strSourceName));
    ptr += OA_LOG_SOURCE_NAME_LEN;
    *ptr = '-'; ptr++;
    *ptr = ' '; ptr++;
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/******************************************************************************
 * InitializeSessionPoolElements
 * ----------------------------------------------------------------------------
 * General:
 *  Initialize elements of the Session pool:
 *  constructs their locks, stream lists and set pointer to the module Manager.
 *  These structures are constructed once on Manager construction and are valid
 *  till the Manager is destructed.
 *
 * Arguments:
 * Input:  pOaMgr - pointer to the OA Module Manager.
 * Output: none.
 *
 * Return Value: none.
 *****************************************************************************/
static RvStatus RVCALLCONV InitializeSessionPoolElements(IN OaMgr* pOaMgr)
{
    RvStatus          crv;
    RLIST_ITEM_HANDLE hPoolElem;
    OaSession*        pOaSession;

    /* Reset the "Initialized" flag for each element in the list */
    hPoolElem = NULL;
    RLIST_PoolGetFirstElem(pOaMgr->hSessionListPool, &hPoolElem);
    while (NULL != hPoolElem)
    {
        ((OaSession*)hPoolElem)->bInitialized = RV_FALSE;
        RLIST_PoolGetNextElem(pOaMgr->hSessionListPool, hPoolElem, &hPoolElem);
    }

    /* Initialize all elements */
    hPoolElem = NULL;
    RLIST_PoolGetFirstElem(pOaMgr->hSessionListPool, &hPoolElem);
    while (NULL != hPoolElem)
    {
        pOaSession = (OaSession*)hPoolElem;

        /* Set the Manager */
        pOaSession->pMgr = pOaMgr;

        /* Construct the Session lock */
        crv = RvMutexConstruct(pOaMgr->pLogMgr, &pOaSession->lock);
        if (RV_OK != crv)
        {
            RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                "InitializeSessionPoolElements(pOaMgr=%p) - failed to construct lock(crv=%d:%s)",
                pOaMgr, crv, OaUtilsConvertEnum2StrStatus(crv)));
            return crv;
        }

        /* Construct the List of Streams*/
        pOaSession->hStreamList = RLIST_ListConstruct(pOaMgr->hStreamListPool);
        if (NULL == pOaSession->hStreamList)
        {
            RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                "InitializeSessionPoolElements(pOaMgr=%p) - failed to construct list of Streams",
                pOaMgr));
            RvMutexDestruct(&((OaSession*)hPoolElem)->lock, pOaMgr->pLogMgr);
            return RV_ERROR_UNKNOWN;
        }

        pOaSession->bInitialized = RV_TRUE;

        RLIST_PoolGetNextElem(pOaMgr->hSessionListPool, hPoolElem, &hPoolElem);
    }

    return RV_OK;
}

/******************************************************************************
 * DestructSessions
 * ----------------------------------------------------------------------------
 * General:
 *  Destructs all Session objects, contained in session pool.
 *
 * Arguments:
 * Input:  pOaMgr - pointer to the OA Module Manager.
 * Output: none.
 *
 * Return Value: none.
 *****************************************************************************/
static void RVCALLCONV DestructSessions(IN OaMgr*  pOaMgr)
{
    RvStatus          rv;
    RLIST_ITEM_HANDLE hPoolElem;
    OaSession*        pOaSession;

    /* Destruct Session objects, stored in the list of Sessions */
    hPoolElem = NULL;
    RLIST_GetHead(pOaMgr->hSessionListPool, pOaMgr->hSessionList,
                  (RLIST_ITEM_HANDLE*)&pOaSession);
    while (NULL != pOaSession)
    {
        rv = OaSessionDestruct(pOaSession);
        if (RV_OK != rv)
        {
            RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
                "DestructSessions(pOaMgr=%p) - failed to destruct Session %p (rv=%d:%s). Continue.",
                pOaMgr, pOaSession, rv, OaUtilsConvertEnum2StrStatus(rv)));
        }

        RLIST_GetNext(pOaMgr->hSessionListPool, pOaMgr->hSessionList,
            (RLIST_ITEM_HANDLE)pOaSession, (RLIST_ITEM_HANDLE*)&pOaSession);
    }

    /* Destruct the Session list itself */
    RLIST_ListDestruct(pOaMgr->hSessionListPool, pOaMgr->hSessionList);
    pOaMgr->hSessionList = NULL;

    /* Destruct lock and Stream list for each element in Session Pool,
    that was initialized on Manager construction */
    hPoolElem = NULL;
    RLIST_PoolGetFirstElem(pOaMgr->hSessionListPool, &hPoolElem);
    while (NULL != hPoolElem)
    {
        pOaSession = (OaSession*)hPoolElem;

        if (RV_TRUE == pOaSession->bInitialized)
        {
            RLIST_ListDestruct(pOaMgr->hStreamListPool, pOaSession->hStreamList);
            RvMutexDestruct(&pOaSession->lock, pOaMgr->pLogMgr);
            pOaSession->bInitialized = RV_FALSE;
            pOaSession->pMgr = NULL;
        }
        RLIST_PoolGetNextElem(pOaMgr->hSessionListPool, hPoolElem, &hPoolElem);
    }

    /* Destruct Session Pool itself */
    RLIST_PoolListDestruct(pOaMgr->hSessionListPool);
    pOaMgr->hSessionListPool = NULL;
}

/******************************************************************************
 * CheckConfiguration
 * ----------------------------------------------------------------------------
 * General:
 *  Ensures valid values of configuration parameters.
 *
 * Arguments:
 * Input:  pOaMgr - pointer to the OA Module Manager.
 *         pOaCfg - configuration to be checked.
 * Output: pbValidCfg - RV_TRUE, if configuration is valid.
 *                      RV_FALSE - otherwise.
 *
 * Return Value: none.
 *****************************************************************************/
static void RVCALLCONV CheckConfiguration(
                            IN  OaMgr*   pOaMgr,
                            IN  RvOaCfg* pOaCfg,
                            OUT RvBool*  pbValidCfg)
{
    *pbValidCfg = RV_TRUE;
    if (pOaCfg->maxSessions == 0)
    {
        *pbValidCfg = RV_FALSE;
        RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
            "CheckConfiguration(pOaMgr=%p) - maxSessions (%d) should be positive integer",
            pOaMgr, pOaCfg->maxSessions));
    }
    if (pOaCfg->maxStreams == 0)
    {
        *pbValidCfg = RV_FALSE;
        RvLogError(pOaMgr->pLogSource,(pOaMgr->pLogSource,
            "CheckConfiguration(pOaMgr=%p) - maxStreams (%d) should be positive integer",
            pOaMgr, pOaCfg->maxStreams));
    }
#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pOaMgr);
#endif
}

/*nl for linux */

