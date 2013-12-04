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
 *                              <OaMgrObject.h>
 * The OaMgrObject.h file defines interface for Manager of Offer-Answer module.
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

#ifndef OA_MGR_OBJECT_H
#define OA_MGR_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                            */
/*---------------------------------------------------------------------------*/
#include "rvmutex.h"
#include "rvloglistener.h"

#include "RvOaTypes.h"
#include "AdsRlist.h"

#include "OaUtils.h"
#include "OaCodecHash.h"

/*---------------------------------------------------------------------------*/
/*                             TYPE DEFINITIONS                              */
/*---------------------------------------------------------------------------*/
#define OA_CAPS_STRING_LEN  (2048)

/******************************************************************************
 * OaMgr
 * ----------------------------------------------------------------------------
 * OaMgr structure represents Manager object, which manages data structures,
 * used by the Offer-Answer module.
 *****************************************************************************/
typedef struct
{
    /* --------- Pools ---------- */
    RLIST_POOL_HANDLE   hSessionListPool; /* Pool of Session objects */
    RLIST_HANDLE        hSessionList;     /* List of allocated Session object*/
    RLIST_POOL_HANDLE   hStreamListPool;  /* Pool of Stream objects */
    HRPOOL              hMessagePool;     /* Pool of pages used by SDP Messages */
    /* Capabilities are stored in form of RADVISION SDP Stack messages. */
    HRPOOL              hCapPool;         /* Pool of pages used by capabilities */
    HRA                 hRaCapAllocs;     /* Pool of SDP Allocators for caps.*/
    HRA                 hRaCapMsgs;       /* Pool of SDP Message objects for caps. */

    /* Configuration */
    RvUint32            maxNumOfSessions; /* Number of Session pool elements */
    RvUint32            maxNumOfStreams;  /* Number of Stream pool elements */
    RvUint32            maxNumOfCaps;     /* Number of capabilities elements */

    /* --------- Default Capabilities --------- */
    OaPSdpMsg  defCapMsg;
        /* Capabilities set by the Application using RvOaSetCapabilities API.
           These capabilities are used by Session objects, when they build
           OFFER and ANSWER messages. */

    RvChar* pCapBuff;
        /* The SDP Message parser modifies (temporary) input strings.
           Therefore it can't work with statically defined string.
           To ensure the dynamic allocation of string, the capabilities,
           provided by the application as a string, are copied to this buffer.
           */

    RvBool  bChooseOneFormatOnly;
        /* This flag is set by the Application, when it constructs the Manager.
           If it is RV_TRUE, each Media Description in the ANSWER, built by
           Session object, will contain only one media format / codec. */

    /* Hash of supported codecs */
    OaCodecHash   hashCodecs;
        /* Hash that contains codecs from default capabilities, set by
           the Application using RvOaSetCapabilities API, and per session
           capabilities, set by the Application using RvOaSessionSetCapabilities
           API.
           The hash is used for efficient derivation of subset of codecs,
           supported by the Application from set of codecs, received from
           remote side with OFFER. */

    RLIST_HANDLE  hCodecHashElements;
        /* List of hash elements, that reference to codec from Default
           Capabilities. This list is used for efficient removal of codecs
           from the hash, if the Default Capabilities are destroyed. */

    RvOaDeriveFormatParams pfnDeriveFormatParams;
        /*  The application implementation of parameter derivation algorithm,
            used in order to get codec parameters, acceptable by both sides of
            the Offer-Answer session, while building ANSWER.
            For mor details see documentation for RvOaDeriveFormatParams type.
            Default value: NULL */

    /* --------- Logging --------- */
    RvOaLogEntryPrint  pfnLogEntryPrint;
        /* Callback, using which the Offer-Answer module provides
           the Application with log string.
           This callback can be set by the Application on module construction,
           using configuration structure. */

    void*  logEntryPrintContext;
        /* Context that is provided by module to the Application, while calling
           pfnLogEntryPrint callback.
           This context can be set by the Application on module construction,
           using configuration structure. */

    RvBool  bLogFuncProvidedByAppl;
        /* If RV_TRUE, pfnLogEntryPrint callback was provided by
        the Application on module construction, using configuration structure*/

    RvLogMgr       logMgr;   /* Log Manager to be used while printing to log.*/
    RvLogMgr*      pLogMgr;  /* If NULL, the logMgr manager can't be used. */
    RvBool         bLogMgrProvidedByAppl;
    RvLogListener  listener;   /*Listener, to which the log data is sent. */
    RvLogSource*   pLogSource; /*Log Source, that serves Offer-Answer module.*/
    RvUint32       logFilter;  /*Mask of filters of data to be printed in log*/
    RvLogSource*   pLogSourceCaps; /*Log Source for capabilities, that serves Offer-Answer module.*/
    RvUint32       logFilterCaps;  /*Mask of filters of capabilities related data to be printed in log*/
    RvLogSource*   pLogSourceMsgs; /*Log Source for messages, that serves Offer-Answer module.*/
    RvUint32       logFilterMsgs;  /*Mask of filters of SDP message data to be printed in log*/

    /* --------- Lock --------- */
    RvMutex* pLock;     /* If NULL, the lock can't be used. */
    RvMutex  lock;

} OaMgr;

/*---------------------------------------------------------------------------*/
/*                            FUNCTION DEFINITIONS                           */
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
                                    OUT OaMgr**   ppOaMgr);

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
void RVCALLCONV OaMgrDestruct(IN OaMgr* pOaMgr);

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
                                    OUT RvOaResources* pResources);

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
                                         IN RvOaCfg* pOaCfg);

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
                                IN RvInt32        filters);
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
                                    IN RvSdpMsg* pSdpMsg);

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
                                    OUT RvSdpMsg* pSdpMsg);

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
RvStatus RVCALLCONV OaMgrLock(IN OaMgr* pOaMgr);

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
void RVCALLCONV OaMgrUnlock(IN OaMgr* pOaMgr);

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef OA_MGR_OBJECT_H */

