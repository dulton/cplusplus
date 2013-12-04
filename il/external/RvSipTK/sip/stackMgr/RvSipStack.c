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
* Last Revision: Jan. 2000                                                      *
*********************************************************************************
*/


/*********************************************************************************
 *                              RvSipStack.c
 *
 * This c file contains implementations for the stack module.
 * it contains functions for initializing, constructing and destructing the entire stack.
 * The file contains 10 api functions (Construct, Destruct, getLocalIp, setNewLogFilters,
 * getFilter, getResources, getVersion, proccessEvents, getCallLegMgrHandle and
 * getLogHandle)
 * and all other functions are internal.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *  Oren Libis                    20 Nov 2000
 *********************************************************************************/



/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#include "_SipCommonConversions.h"

#include "rvhost.h"
#include "rvtimestamp.h"
#include "rvstdio.h"
#include "rvclock.h"

#include "AdsRpool.h"

#include "RvSipStack.h"

#ifndef RV_SIP_PRIMITIVES
#include "RvSipCallLeg.h"
#include "RvSipContactHeader.h"
#include "_SipCallLegMgr.h"
#include "_SipRegClientMgr.h"
#include "_SipPubClientMgr.h"
#include "_SipTranscClientMgr.h"
#endif /* RV_SIP_PRIMITIVES */

#include "RvSipMsgTypes.h"
#include "RvSipOtherHeader.h"
#include "RvSipCSeqHeader.h"
#include "RvSipAllowHeader.h"
#include "RvSipViaHeader.h"
#include "RvSipResourcesTypes.h"
#include "RvSipVersion.h"
#include "RvSipTransaction.h"
#include "_SipCommonUtils.h"
#include "_SipMsgMgr.h"
#include "_SipParserManager.h"
#include "_SipTransactionMgr.h"
#include "_SipTransactionTypes.h"
#include "_SipTransport.h"
#include "_SipAuthenticator.h"
#include "_SipCommonCore.h"
#include "_SipCompartment.h"
#include "_SipTransmitterMgr.h"
#include "_SipCallLeg.h"
#include "StackInternal.h"
#include "_SipResolverMgr.h"

#ifdef RV_SIP_IMS_ON
#include "_SipSecAgreeMgr.h"
#include "_SipSecurityMgr.h"
#endif /* RV_SIP_IMS_ON */


#ifndef RV_SIP_PRIMITIVES
#include "_SipSubsMgr.h"
#endif

#if defined(UPDATED_BY_SPIRENT)
#include "rvexternal.h"
#endif

#if defined(UPDATED_BY_SPIRENT_ABACUS)
#define printf_meminfo(A) 
#endif

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/
#if defined(RV_DEPRECATED_CORE)
#define DEPRECATED_MAX_TIMERS  100
#endif /*RV_DEPRECATED_CORE*/

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static RvStatus RVCALLCONV StackConstructResolverModule(INOUT RvSipStackCfg*   pStackCfg,
                                                        INOUT StackMgr*        pStackMgr);
#if !defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_JSR32_SUPPORT)
#ifdef RV_SIP_SUBS_ON
static RvStatus RVCALLCONV StackConstructSubsModule(INOUT RvSipStackCfg*   pStackCfg,
                                                    INOUT StackMgr*        pStackMgr);
#endif /*#ifdef RV_SIP_SUBS_ON*/
static RvStatus RVCALLCONV StackConstructTranscClientModule(INOUT RvSipStackCfg*   pStackCfg,
														    INOUT StackMgr*        pStackMgr);

static RvStatus RVCALLCONV StackConstructRegClientModule(INOUT RvSipStackCfg*   pStackCfg,
                                                    INOUT StackMgr*        pStackMgr);

static RvStatus RVCALLCONV StackConstructPubClientModule(INOUT RvSipStackCfg*   pStackCfg,
														 INOUT StackMgr*        pStackMgr);

static RvStatus RVCALLCONV StackConstructCallLegModule(INOUT RvSipStackCfg*   pStackCfg,
                                                       INOUT StackMgr*        pStackMgr,
                                                       IN RvBool            b100rel);

static RvStatus RVCALLCONV StackConstructSupportedList(
                                      IN    RvChar        *supportedList,
                                      OUT   StackMgr      *pStackMgr);

#endif /*!defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_JSR32_SUPPORT)*/
#if !defined(RV_SIP_LIGHT) && !defined(RV_SIP_JSR32_SUPPORT)
static RvStatus RVCALLCONV StackConstructTranscModule(INOUT RvSipStackCfg*   pStackCfg,
                                                       INOUT StackMgr*        pStackMgr,
                                                       IN RvBool            b100rel);

#endif /*!defined(RV_SIP_LIGHT) && !defined(RV_SIP_JSR32_SUPPORT)*/

static RvStatus RVCALLCONV StackConstructTrxModule(INOUT RvSipStackCfg*   pStackCfg,
                                                   INOUT StackMgr*        pStackMgr);

static RvStatus RVCALLCONV StackConstructTransportModule(INOUT RvSipStackCfg*   pStackCfg,
                                                         INOUT StackMgr*        pStackMgr);

static RvStatus RVCALLCONV StackConstructMessageModule(INOUT StackMgr*        pStackMgr);

static RvStatus RVCALLCONV StackConstructParserModule(INOUT StackMgr*        pStackMgr);

#ifdef RV_SIGCOMP_ON
static RvStatus RVCALLCONV StackConstructCompartmentModule(INOUT RvSipStackCfg*   pStackCfg,
                                                           INOUT StackMgr*        pStackMgr);
#endif /*#ifdef RV_SIGCOMP_ON*/

#ifdef RV_SIP_IMS_ON
static RvStatus RVCALLCONV StackConstructSecAgreeModule(INOUT RvSipStackCfg*   pStackCfg,
														INOUT StackMgr*        pStackMgr);
static RvStatus RVCALLCONV StackConstructSecurityModule(INOUT RvSipStackCfg*   pStackCfg,
                                                        INOUT StackMgr*        pStackMgr);
#endif /* #ifdef RV_SIP_IMS_ON */

static RvStatus RVCALLCONV ConstructStackLog(
                                    IN  StackMgr          *pStackMgr,
                                    IN  RvSipStackCfg     *pStackCfg);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static void RVCALLCONV SetTemporaryCoreLogMasks(IN    StackMgr      *pStackMgr,
                                                IN    RvSipStackCfg *pStackCfg);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

static RvStatus RVCALLCONV CalculateConfigParams(IN StackMgr*      pStackMgr,
                                                 IN RvSipStackCfg *pStackCfg);


static RvStatus RVCALLCONV StackAllocateMemoryPools(
                                    IN    RvSipStackCfg   *pStackCfg,
                                    INOUT StackMgr        *pStackMgr);


static RvStatus RVCALLCONV StackConstructModules(
                                    IN    RvSipStackCfg   *pStackCfg,
                                    INOUT StackMgr        *pStackMgr);


static RvStatus RVCALLCONV ConvertZeroAddrToRealAddr(
                                    INOUT RvSipStackCfg   *pStackCfg,
                                    INOUT StackMgr        *pStackMgr);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static RvStatus RVCALLCONV ConstructLogSource(
                                    IN  StackMgr          *pStackMgr,
                                    IN  RvSipStackModule   eLogSource);

static void RVCALLCONV DestructLogSource(
                                    IN  StackMgr          *pStackMgr,
                                    IN  RvSipStackModule   eLogSource);
static void RVCALLCONV DestructLogListeners(
                                    IN  StackMgr          *pStackMgr);


static RvInt32 RVCALLCONV GetModuleLogMaskFromCfg(
                                    IN  RvSipStackCfg     *pStackCfg,
                                    IN  RvSipStackModule   eLogModule);

static RvChar* RVCALLCONV GetModuleName(
                                    IN   RvSipStackModule  eModule);


static void RVCALLCONV PrintConfigParamsToLog(
                                     IN StackMgr          *pStackMgr,
                                     IN RvSipStackCfg     *pStackCfg);

static void RVCALLCONV PrintLogFiltersForAllModules(
                                     IN  StackMgr         *pStackMgr,
                                     IN  RvSipStackCfg    *pStackCfg);

static void  RVCALLCONV  GetLogMaskAsString(
                                     IN  RvLogMessageType  coreLogMask,
                                     OUT RvChar           *strFilters);


static void RVCALLCONV PrintNumOfErrorExcepAndWarn(
                                     IN   StackMgr        *pStackMgr);



static void RVCALLCONV LogPrintCallbackFunction(
                                      IN RvLogRecord      *pLogRecord,
                                      IN void             *context);

#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/



static RvInt32 RVCALLCONV CalculateMaxTimers(
                                             IN RvSipStackCfg* pStackCfg,
                                             IN StackMgr*      pStackMgr);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)

#include "CallLegMgrObject.h"
int RvShow ( IN RvSipStackHandle stack_handle )
{
   // I don't know why these following unsigned number with -1s. I might cause a side affect
   RV_UINT32          AllocNumOfLists           = -1;
   RV_UINT32          AllocMaxNumOfUserElement  = -1;
   RV_UINT32          CurrNumOfUsedLists        = -1;
   RV_UINT32          CurrNumOfUsedUsreElement  = -1;
   RV_UINT32          MaxUsageInLists           = -1; 
   RV_UINT32          MaxUsageInUserElement     = -1;
   RV_UINT32          NumOfAlloc                = -1;
   RV_UINT32          CurrNumOfUsed             = -1;
   RV_UINT32          MaxUsage                  = -1;


   StackMgr *stInfo     = (StackMgr*)(stack_handle);
   TransactionMgr *trMgr = (TransactionMgr*)(stInfo->hTranscMgr);
   CallLegMgr  *calMgr     = (CallLegMgr*)(stInfo->hCallLegMgr);

   dprintf ( "\n            ------- nLists ------  ----- nElements ------\n");
   dprintf ( "            Start     Cur     Max   Start     Cur     Max\n");

   RLIST_GetResourcesStatus ( calMgr->hCallLegPool,
                                     &AllocNumOfLists,
                                     &AllocMaxNumOfUserElement,
                                     &CurrNumOfUsedLists,
                                     &CurrNumOfUsedUsreElement,
                                     &MaxUsageInLists,
                                     &MaxUsageInUserElement );
   dprintf ( "Call-Leg  %7d %7d %7d %7d %7d %7d\n", 
                        AllocNumOfLists,
                        CurrNumOfUsedLists,
                        MaxUsageInLists,
                        AllocMaxNumOfUserElement,
                        CurrNumOfUsedUsreElement,
                        MaxUsageInUserElement );

   RLIST_GetResourcesStatus ( trMgr->hTranasactionsListPool,
                                     &AllocNumOfLists,
                                     &AllocMaxNumOfUserElement,
                                     &CurrNumOfUsedLists,
                                     &CurrNumOfUsedUsreElement,
                                     &MaxUsageInLists,
                                     &MaxUsageInUserElement );
   dprintf ( "Trans     %7d %7d %7d %7d %7d %7d\n", 
                        AllocNumOfLists,
                        CurrNumOfUsedLists,
                        MaxUsageInLists,
                        AllocMaxNumOfUserElement,
                        CurrNumOfUsedUsreElement,
                        MaxUsageInUserElement );

   RPOOL_GetResources ( stInfo->hMessagePool,
                                 &NumOfAlloc,   
                                 &CurrNumOfUsed,
                                 &MaxUsage );
   dprintf ( "MsgPool                           %7d %7d %7d\n", 
                                 NumOfAlloc,   
                                 CurrNumOfUsed,
                                 MaxUsage );

   RPOOL_GetResources ( stInfo->hGeneralPool,
                                 &NumOfAlloc,   
                                 &CurrNumOfUsed,
                                 &MaxUsage );
   dprintf ( "GnrlPool                          %7d %7d %7d\n",
                                 NumOfAlloc,   
                                 CurrNumOfUsed,
                                 MaxUsage );
   
   RPOOL_GetResources ( stInfo->hElementPool,
                                 &NumOfAlloc,   
                                 &CurrNumOfUsed,
                                 &MaxUsage );
   dprintf ( "ElemPool                          %7d %7d %7d\n",
                                 NumOfAlloc,   
                                 CurrNumOfUsed,
                                 MaxUsage );

   return 0;
}

#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

/************************************************************************************
 * RvSipStackConstruct
 * ----------------------------------------------------------------------------------
 * General: Constructs and initializes the RADVISION SIP stack. This function allocates the
 *          required memory and constructs RADVISION SIP stack objects according to
 *          configuration parameters specified in the RvSipStackCfg structure. The function
 *          returns a handle to the Stack Manager. You need this handle in order to use the
 *          Stack Manager API functions.
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg     - Structure containing RADVISION SIP stack configuration parameters.
 *                          This parameter is also an output parameter. Fields that contain zero or invalid
 *                          values are given default values by the RADVISION SIP stack.
 *          sizeOfCfg - The size of the configuration structure.
 * Output:  hStack        - Handle to the Stack Manager.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackConstruct(
                                         IN    RvInt32                 sizeOfCfg,
                                         INOUT RvSipStackCfg          *pStackCfg,
                                         OUT   RvSipStackHandle       *hStack)
{
    StackMgr*      pStackMgr;
    RvStatus       status;
    RvSipStackCfg  internalCfgStruct;
    RvInt          minCfgSize = 0;
    RvStatus       crv        = RV_OK;

    *hStack = NULL;
    if (pStackCfg == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    printf_meminfo(__FUNCTION__);
#endif
/* SPIRENT_END */

    /*initialize the common core modules*/
    crv = RvCBaseInit();
    if (crv != RV_OK)
    {
        return RV_ERROR_UNKNOWN;
    }

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    printf_meminfo(__FUNCTION__);
#endif
/* SPIRENT_END */


    /* allocating the struct that holds the stack information */
    if (RV_OK != RvMemoryAlloc(NULL,sizeof(StackMgr),NULL,(void**)&pStackMgr))
    {
        RvCBaseEnd();
        return RV_ERROR_UNKNOWN;
    }
    memset(pStackMgr, 0, sizeof(StackMgr));

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    pStackMgr->seed.state = rand();
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    printf_meminfo(__FUNCTION__);
#endif
#endif
/* SPIRENT_END */

    /*for backward compatibility we copy the configuration to internal struct*/
    RvSipStackInitCfg(sizeof(internalCfgStruct),&internalCfgStruct);
    minCfgSize = RvMin(((RvInt)sizeOfCfg),(RvInt)sizeof(internalCfgStruct));

    memcpy(&internalCfgStruct, pStackCfg, minCfgSize);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    printf_meminfo(__FUNCTION__);
#endif
/* SPIRENT_END */


    /* initiating log */
    status = ConstructStackLog(pStackMgr,&internalCfgStruct);
    if (status != RV_OK)
    {
        RvMemoryFree(pStackMgr,NULL);
        RvCBaseEnd();
        return RV_ERROR_UNKNOWN;
    }

    /* Set stack, memory pool and list elements allocations if the application set them to -1 */
    status = CalculateConfigParams(pStackMgr, &internalCfgStruct);
    if (status != RV_OK)
    {
        RvSipStackDestruct((RvSipStackHandle)pStackMgr);
        return status;
    }

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
              "RvSipStackConstruct - RADVISION SIP stack - version %s - Constructing...",SIP_STACK_VERSION));


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    printf_meminfo(__FUNCTION__);
#endif
/* SPIRENT_END */


    /* allocating memory pools */
    status = StackAllocateMemoryPools(&internalCfgStruct, pStackMgr);
    if (status != RV_OK)
    {
        RvSipStackDestruct((RvSipStackHandle)pStackMgr);
        return status;
    }

#if !defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_JSR32_SUPPORT)
    status = StackConstructSupportedList(internalCfgStruct.supportedExtensionList,pStackMgr);
    if(status != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "RvSipStackConstruct - Failed to construct supported list"));
        RvSipStackDestruct((RvSipStackHandle)pStackMgr);
        return status;
    }


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    printf_meminfo(__FUNCTION__);
#endif
/* SPIRENT_END */

#endif  /* ifndef RV_SIP_PRIMITIVES */

    /* initialization for using random numbers */
    RvRandomGeneratorConstruct(&pStackMgr->seed,(RvUint32)RvClockGet(NULL,NULL));


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    printf_meminfo(__FUNCTION__);
#endif
/* SPIRENT_END */

    /* construct the stack modules */
    status = StackConstructModules(&internalCfgStruct, pStackMgr);
    if (status != RV_OK)
    {
        /*print the configuration to the log on failure*/
        memcpy(pStackCfg,&internalCfgStruct,minCfgSize);
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
           "RvSipStackConstruct - Failed to construct stack modules. The supplied configuration was:"));
        PrintConfigParamsToLog(pStackMgr,pStackCfg);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
        RvSipStackDestruct((RvSipStackHandle)pStackMgr);
        return status;
    }


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    printf_meminfo(__FUNCTION__);
#endif
/* SPIRENT_END */

    crv = RvSelectGetThreadEngine(pStackMgr->pLogMgr,&pStackMgr->pSelect);
    if (RV_OK != crv)
    {
        RvSipStackDestruct((RvSipStackHandle)pStackMgr);
        return CRV2RV(crv);
    }

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    printf_meminfo(__FUNCTION__);
#endif
/* SPIRENT_END */


    *hStack = (RvSipStackHandle)pStackMgr;

    memcpy(pStackCfg,&internalCfgStruct,minCfgSize);
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    PrintConfigParamsToLog(pStackMgr,pStackCfg);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "RvSipStackConstruct - RADVISION SIP stack - Version %s - was constructed successfully",SIP_STACK_VERSION));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "RvSipStackConstruct - stack handle: 0x%p",pStackMgr));

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "***********************************************************************************"));


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    printf_meminfo(__FUNCTION__);
#endif
/* SPIRENT_END */

    return RV_OK;

}


/************************************************************************************
 * RvSipStackDestruct
 * ----------------------------------------------------------------------------------
 * General: destructs and terminates the stack. the function destroys all stack
 *          modules and free all the memory that was allocated.
 * Return Value:
 *          RV_OK     - the stack was destroyed successfully
 *          RV_ERROR_UNKNOWN     - the handle to the stack was NULL
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack         - Handle to stack instance
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackDestruct(IN RvSipStackHandle hStack)
{
    StackMgr*    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
              "RvSipStackDestruct - Destructing the Sip Stack..."));

    /* First we should terminate processing threads (if any) */
    if (NULL != pStackMgr->hTransport)
    {
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                  "RvSipStackDestruct - Stopping processing threads..."));

        SipTransportStopProcessingThreads(pStackMgr->hTransport);
    }

    /* now we kill ARES so we wont get any DNS answers */
    if(NULL != pStackMgr->hTrxMgr)
    {
        SipResolverMgrStopAres(pStackMgr->hRslvMgr);
    }

#ifdef RV_SIGCOMP_ON
    if(NULL != pStackMgr->hCompartmentMgr)
    {
        SipCompartmentMgrDetachFromSigCompModule(pStackMgr->hCompartmentMgr);
    }
#endif

    
    /* ATTENTION: destruct Transaction Module before Call-Leg / Subscription,
                  since Security Transactions can access Call-Leg's triple lock
                  on destruction */
#ifndef RV_SIP_LIGHT
    if (NULL != pStackMgr->hTranscMgr)
    {
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "RvSipStackDestruct - Destructing the Transaction manager..."));
        SipTransactionMgrDestruct(pStackMgr->hTranscMgr);
        pStackMgr->hTranscMgr = NULL;
    }
#endif /*#ifndef RV_SIP_LIGHT*/
    

#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON
    if (NULL != pStackMgr->hSubsMgr)
    {
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "RvSipStackDestruct - Destructing the Subscription manager..."));
        SipSubsMgrDestruct(pStackMgr->hSubsMgr);
        pStackMgr->hSubsMgr = NULL;
    }
#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /* #ifndef RV_SIP_PRIMITIVES*/

    /* destruct stack modules */
#ifndef RV_SIP_PRIMITIVES
    if (NULL != pStackMgr->hCallLegMgr)
    {
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                  "RvSipStackDestruct - Destructing the Call-Leg manager..."));
        SipCallLegMgrDestruct(pStackMgr->hCallLegMgr); /*must be destructed first*/
        pStackMgr->hCallLegMgr = NULL;
    }
#endif /* RV_SIP_PRIMITIVES */

    /* ATTENTION: The Trx module should be destructed only after the call-leg module, 
                  since the Invite object tries to access it's Trx through it's 
                  destruction */
    if (NULL != pStackMgr->hTrxMgr)
    {
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "RvSipStackDestruct - Destructing the Transmitter manager..."));
        SipTransmitterMgrDestruct(pStackMgr->hTrxMgr);
        pStackMgr->hTrxMgr = NULL;
    }

#ifndef RV_SIP_PRIMITIVES
    if (NULL != pStackMgr->hRegClientMgr)
    {
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                 "RvSipStackDestruct - Destructing the Reg-Client manager..."));
        SipRegClientMgrDestruct(pStackMgr->hRegClientMgr);
        pStackMgr->hRegClientMgr = NULL;
    }

	if (NULL != pStackMgr->hPubClientMgr)
    {
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
			"RvSipStackDestruct - Destructing the Pub-Client manager..."));
        SipPubClientMgrDestruct(pStackMgr->hPubClientMgr);
        pStackMgr->hPubClientMgr = NULL;
    }

	if (NULL != pStackMgr->hTranscClientMgr)
    {
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
			"RvSipStackDestruct - Destructing the Transc-Client manager..."));
        SipTranscClientMgrDestruct(pStackMgr->hTranscClientMgr);
        pStackMgr->hTranscClientMgr = NULL;
    }
#endif /* #ifndef RV_SIP_PRIMITIVES */

    /* ATTENTION: destruct Security Module before Transport, since Security
                  Objects close local addresses on destruction */
#ifdef RV_SIP_IMS_ON
    if (NULL != pStackMgr->hSecMgr)
    {
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "RvSipStackDestruct - Destructing the Security manager..."));
        SipSecurityMgrDestruct(pStackMgr->hSecMgr);
        pStackMgr->hSecMgr = NULL;
    }
#endif /*#ifdef RV_SIP_IMS_ON*/
    
    if (NULL != pStackMgr->hTransport)
    {
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                  "RvSipStackDestruct - Destructing the Transport manager..."));
        SipTransportDestruct(pStackMgr->hTransport);
        pStackMgr->hTransport = NULL;
    }

    if (NULL != pStackMgr->hMsgMgr)
    {
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "RvSipStackDestruct - Destructing the Message manager..."));
        SipMessageMgrDestruct(pStackMgr->hMsgMgr);
        pStackMgr->hMsgMgr = NULL;
    }
    if (NULL != pStackMgr->hParserMgr)
    {
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "RvSipStackDestruct - Destructing the Parser manager..."));
        SipParserMgrDestruct(pStackMgr->hParserMgr);
        pStackMgr->hParserMgr = NULL;
    }

	if (NULL != pStackMgr->hRslvMgr) 
	{
		RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "RvSipStackDestruct - Destructing the Resolver manager..."));
		SipResolverMgrDestruct(pStackMgr->hRslvMgr);
        pStackMgr->hRslvMgr = NULL;
	}
	
#ifdef RV_SIP_AUTH_ON
    if (NULL != pStackMgr->hAuthentication)
    {
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "RvSipStackDestruct - Destructing the Authenticator..."));
        SipAuthenticatorDestruct(pStackMgr->hAuthentication);
        pStackMgr->hAuthentication = NULL;
    }
#endif /* #ifdef RV_SIP_AUTH_ON */

#ifdef RV_SIGCOMP_ON
    if (NULL != pStackMgr->hCompartmentMgr)
    {
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "RvSipStackDestruct - Destructing the Compartment manager..."));
        SipCompartmentMgrDestruct(pStackMgr->hCompartmentMgr);
        pStackMgr->hCompartmentMgr = NULL;
    }
#endif

#ifdef RV_SIP_IMS_ON
    if (NULL != pStackMgr->hSecAgreeMgr)
    {
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "RvSipStackDestruct - Destructing the Security-Agreement manager..."));
        SipSecAgreeMgrDestruct(pStackMgr->hSecAgreeMgr);
        pStackMgr->hSecAgreeMgr = NULL;
    }
#endif /*#ifdef RV_SIP_IMS_ON*/

    RvRandomGeneratorDestruct(&pStackMgr->seed);

#ifndef RV_SIP_PRIMITIVES
    /* Destruct the supported list */
    if(pStackMgr->supportedExtensionList != NULL)
    {
        RvInt32 i;
        for(i = 0 ; i < pStackMgr->extensionListSize ; i++)
        {
            if(pStackMgr->supportedExtensionList[i] != NULL)
            {
                RvMemoryFree((void *)pStackMgr->supportedExtensionList[i],pStackMgr->pLogMgr);
            }
        }
        RvMemoryFree((void *)pStackMgr->supportedExtensionList,pStackMgr->pLogMgr);
    }
#endif /*#ifndef RV_SIP_PRIMITIVES */

    /* destructing the pools handles */
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "RvSipStackDestruct - Destructing memory pools..."));

    if (NULL != pStackMgr->hMessagePool)
    {
        RPOOL_Destruct(pStackMgr->hMessagePool);
    }
    if (NULL != pStackMgr->hGeneralPool)
    {
        RPOOL_Destruct(pStackMgr->hGeneralPool);
    }
    if (NULL != pStackMgr->hElementPool)
    {
        RPOOL_Destruct(pStackMgr->hElementPool);
    }


#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    PrintNumOfErrorExcepAndWarn(pStackMgr);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
             "RvSipStackDestruct - Sip Stack version %s was destructed successfully",
              SIP_STACK_VERSION));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "***********************************************************************************"));

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    /*destruction of common core cbase*/
    {
        RvInt32 i;
        for(i=0; i<STACK_NUM_OF_LOG_SOURCE_MODULES; i++)
        {
            DestructLogSource(pStackMgr,(RvSipStackModule)i);
        }
    }
    DestructLogListeners(pStackMgr);
    RvLogDestruct(pStackMgr->pLogMgr);

#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

    /*destruction of 'thread' structure, used by Common Core (e.g. for select engine)*/
/*    RvThreadDestruct(&pStackMgr->thread);*/
    /* free the stack info memory */
    RvMemoryFree(pStackMgr,NULL);

    /*destruction of common core*/
    RvCBaseEnd();

    return RV_OK;

}

/***************************************************************************
 * RvSipStackSetAppHandle
 * ------------------------------------------------------------------------
 * General: Set an application handle to the entire sip stack.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hStack    - Handle to the stack.
 *            hAppStack - A new application handle to the stack
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackSetAppHandle (
                                      IN  RvSipStackHandle       hStack,
                                      IN  RvSipAppStackHandle    hAppStack)
{
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "RvSipStackSetAppHandle - Setting stack app handle 0x%p",hAppStack));

    pStackMgr->hAppStack = hAppStack;

    return RV_OK;

}

/***************************************************************************
 * RvSipStackGetAppHandle
 * ------------------------------------------------------------------------
 * General: Returns the application handle of this stack instance
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hStack - Handle to the stack.
 * Output:     phAppStack     - The application handle of the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetAppHandle (
                                      IN    RvSipStackHandle        hStack,
                                      OUT   RvSipAppStackHandle     *phAppStack)
{
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }
    *phAppStack = pStackMgr->hAppStack;

    return RV_OK;

}

/************************************************************************************
 * RvSipStackGetResolverMgrHandle
 * ----------------------------------------------------------------------------------
 * General: get the Resolver manager handle
 * Return Value:
 *          RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack          - handle to the SIP Stack.
 * Output:  phResolverMgr   - The handle to the Resolver manager object.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetResolverMgrHandle(
                                         IN    RvSipStackHandle       hStack,
                                         OUT   RvSipResolverMgrHandle  *phResolverMgr)
{
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    *phResolverMgr = pStackMgr->hRslvMgr;
    return RV_OK;

}

#ifndef RV_SIP_PRIMITIVES
/************************************************************************************
 * RvSipStackGetCallLegMgrHandle
 * ----------------------------------------------------------------------------------
 * General: Gets the Call-Leg Manager handle. You need this handle in order to use the
 *          Call-leg API functions.
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack          - Handle to the RADVISION SIP stack.
 * Output:  phCallLegMgr     - Handle to the Call-Leg Manager.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetCallLegMgrHandle(
                                         IN    RvSipStackHandle       hStack,
                                         OUT   RvSipCallLegMgrHandle  *phCallLegMgr)
{
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    *phCallLegMgr = pStackMgr->hCallLegMgr;
    return RV_OK;

}

#endif /* #ifndef RV_SIP_PRIMITIVES */
/************************************************************************************
 * RvSipStackGetTransportMgrHandle
 * ----------------------------------------------------------------------------------
 * General: Gets the Transport Manager handle. You need this handle in order to use the
 *          Transport API functions and callbacks.
 * Return Value:RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack          - handle to the SIP Stack.
 * Output:  phTransportMgr   - handle to the Transport Manager.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetTransportMgrHandle(
                                       IN    RvSipStackHandle          hStack,
                                       OUT   RvSipTransportMgrHandle  *phTransportMgr)
{
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    *phTransportMgr = pStackMgr->hTransport;
    return RV_OK;

}

/************************************************************************************
 * RvSipStackGetTransmitterMgrHandle
 * ----------------------------------------------------------------------------------
 * General: get the transmitter manager handle.You need this handle in order to use the
 *          Transmitter API functions.
 * Return Value:RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack          - handle to the sip stack.
 * Output:  phTrxMgr        - handle to the Transmitter Manager.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetTransmitterMgrHandle(
                                         IN    RvSipStackHandle       hStack,
                                         OUT   RvSipTransmitterMgrHandle* phTrxMgr)
{
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    *phTrxMgr = pStackMgr->hTrxMgr;
    return RV_OK;

}
#ifndef RV_SIP_LIGHT
/************************************************************************************
 * RvSipStackGetTransactionMgrHandle
 * ----------------------------------------------------------------------------------
 * General: get the transaction manager handle
 * Return Value:
 *          RV_OK        - the transaction manager handle was got successfully
 *          RV_ERROR_UNKNOWN        - the handle to the stack was NULL.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack          - handle to the entire stack.
 * Output:  phTranscMgr      - handle to the transaction manager.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetTransactionMgrHandle(
                                         IN    RvSipStackHandle       hStack,
                                         OUT   RvSipTranscMgrHandle  *phTranscMgr)
{
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    *phTranscMgr = pStackMgr->hTranscMgr;
    return RV_OK;

}
#endif /*#ifndef RV_SIP_LIGHT*/

/************************************************************************************
 * RvSipStackGetMsgMgrHandle
 * ----------------------------------------------------------------------------------
 * General: Gets the Message Manager handle. You need this handle in order to use
 *          the Message API functions.
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack         - Handle to the SIP stack.
 * Output:  phMsgMgr        - Handle to the Message Manager.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetMsgMgrHandle(
                                  IN    RvSipStackHandle  hStack,
                                  OUT   RvSipMsgMgrHandle *phMsgMgr)
{
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    *phMsgMgr = pStackMgr->hMsgMgr ;
    return RV_OK;

}

#ifndef RV_SIP_PRIMITIVES
/************************************************************************************
 * RvSipStackGetRegClientMgrHandle
 * ----------------------------------------------------------------------------------
 * General: get the register-clients manager handle
 * Return Value:
 *          RV_OK        - the register-clients manager was returned successfully
 *          RV_ERROR_UNKNOWN        - the handle to the stack was NULL.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack          - handle to the entire stack.
 * Output:  phRegClientMgr   - handle to the register-client manager.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetRegClientMgrHandle(
                                  IN    RvSipStackHandle         hStack,
                                  OUT   RvSipRegClientMgrHandle *phRegClientMgr)
{
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    *phRegClientMgr = pStackMgr->hRegClientMgr ;
    return RV_OK;

}

/************************************************************************************
 * RvSipStackGetPubClientMgrHandle
 * ----------------------------------------------------------------------------------
 * General: get the publish-clients manager handle
 * Return Value:
 *          RV_OK        - the publish-clients manager was returned successfully
 *          RV_ERROR_UNKNOWN        - the handle to the stack was NULL.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack          - handle to the entire stack.
 * Output:  phPubClientMgr   - handle to the publish-client manager.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetPubClientMgrHandle(
                                  IN    RvSipStackHandle         hStack,
                                  OUT   RvSipPubClientMgrHandle *phPubClientMgr)
{
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    *phPubClientMgr = pStackMgr->hPubClientMgr ;
    return RV_OK;

}
#endif /* #ifndef RV_SIP_PRIMITIVES */

#ifdef RV_SIP_AUTH_ON
/************************************************************************************
 * RvSipStackGetAuthenticatorHandle
 * ----------------------------------------------------------------------------------
 * General: get the Authentication layer handle
 * Return Value:
 *          RV_OK        - the Authentication handle was got successfully
 *          RV_ERROR_UNKNOWN        - the handle to the stack was NULL.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack          - handle to the entire stack.
 * Output:  phAuth           - handle to the Authentication layer.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetAuthenticatorHandle(
                                         IN    RvSipStackHandle           hStack,
                                         OUT   RvSipAuthenticatorHandle  *phAuth)
{
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    *phAuth = pStackMgr->hAuthentication;
    return RV_OK;

}
#endif /* #ifdef RV_SIP_AUTH_ON */

#if !defined(RV_SIP_PRIMITIVES) && defined(RV_SIP_SUBS_ON)
/************************************************************************************
 * RvSipStackGetSubsMgrHandle
 * ----------------------------------------------------------------------------------
 * General: Gets the Subscription manager object handle. You need this handle in
 *          order to use the Subscription API.
 * Return Value:RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack          - Handle to the SIP stack.
 * Output:  phSubsMgr        - Handle to the Subscription manager object.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetSubsMgrHandle(
                                  IN    RvSipStackHandle    hStack,
                                  OUT   RvSipSubsMgrHandle *phSubsMgr)
{
#ifdef RV_SIP_SUBS_ON
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    *phSubsMgr = pStackMgr->hSubsMgr;
    return RV_OK;
#else /* RV_SIP_SUBS_ON */
	RV_UNUSED_ARG(hStack);
	*phSubsMgr = NULL;

	return RV_ERROR_ILLEGAL_ACTION;
#endif /* RV_SIP_SUBS_ON */ 
}
#endif /* #if !defined(RV_SIP_PRIMITIVES) && defined(RV_SIP_SUBS_ON) */

#ifdef RV_SIGCOMP_ON
/************************************************************************************
 * RvSipStackGetSigCompCompartmentMgrHandle
 * ----------------------------------------------------------------------------------
 * General: Gets the SigComp Compartment manager object handle. You need this handle
 *          in order to use the SigComp Compartment API.
 * Return Value:RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack          - Handle to the SIP stack.
 * Output:  phCompartmentMgr - Handle to the Compartment manager object.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetCompartmentMgrHandle(
                   IN    RvSipStackHandle           hStack,
                   OUT   RvSipCompartmentMgrHandle *phCompartmentMgr)
{
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    if (NULL != phCompartmentMgr)
    {
        *phCompartmentMgr = pStackMgr->hCompartmentMgr;
    }
    else
    {
        return RV_ERROR_NULLPTR;
    }
    return RV_OK;
}
#endif /* RV_SIGCOMP_ON */
/************************************************************************************
 * RvSipStackGetLogHandle
 * ----------------------------------------------------------------------------------
 * General: get the Log handle of the stack.
 * Return Value:
 *          RV_OK.
 *          RV_ERROR_UNKNOWN        - the handle to the stack was NULL.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack          - handle to the stack manager.
 * Output:  phLog            - handle to the Log.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetLogHandle(
                                         IN    RvSipStackHandle       hStack,
                                         OUT   RV_LOG_Handle          *phLog)
{
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    *phLog = (RV_LOG_Handle)pStackMgr->pLogMgr;
    return RV_OK;

}

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * RvSipStackGetSecMgrHandle
 * ----------------------------------------------------------------------------
 * General: gets the Security Manager
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack   - Handle to the Stack Manager.
 * Output:  phSecMgr - handle to the Security Manager.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetSecMgrHandle(
                                         IN  RvSipStackHandle   hStack,
                                         OUT RvSipSecMgrHandle* phSecMgr)
{
    StackMgr *pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_BADPARAM;
    }
    *phSecMgr = pStackMgr->hSecMgr;
    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIGCOMP_ON

/************************************************************************************
 * RvSipStackGetSigCompMgrHandle
 * ----------------------------------------------------------------------------------
 * General: Gets the SigComp  manager object handle. You need this handle
 *          in order to use the SigComp API.
 * Return Value:RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack       - Handle to the SIP stack.
 * Output:  phCompMgr    - Handle to the Compartment manager object.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetSigCompMgrHandle(
                   IN    RvSipStackHandle    hStack,
                   OUT   RvSigCompMgrHandle *phCompMgr)
{
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    if (NULL != phCompMgr)
    {
        *phCompMgr = pStackMgr->hSigCompMgr;
    }
    else
    {
        return RV_ERROR_NULLPTR;
    }
    return RV_OK;
}
#endif /* RV_SIGCOMP_ON */

#ifdef RV_SIP_IMS_ON
/************************************************************************************
 * RvSipStackGetSecAgreeMgrHandle
 * ----------------------------------------------------------------------------------
 * General: get the Security-Agreement layer handle
 * Return Value:
 *          RV_OK              - the Security handle was got successfully
 *          RV_ERROR_UNKNOWN   - the handle to the stack was NULL.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack          - handle to the SIP stack.
 * Output:  phSecAgree      - handle to the Security-Agreement layer.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetSecAgreeMgrHandle(
                                         IN    RvSipStackHandle           hStack,
                                         OUT   RvSipSecAgreeMgrHandle    *phSecAgree)
{
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    *phSecAgree = pStackMgr->hSecAgreeMgr;
    return RV_OK;
}
#endif /* #ifdef RV_SIP_IMS_ON */

#if defined(RV_DEPRECATED_CORE)
/************************************************************************************
 * RvSipStackGetSeliHandle
 * ----------------------------------------------------------------------------------
 * General: This function can be used only if the stack is compiled with
 *          RV_DEPRECATED_CORE compilation flag which means that the application
 *          is using the deprecated core implementation.
 *          This function returns the seli handle. The seli handle is used by
 *          some of the deprecated core functions, specifically select related functions.
 *          Using the deprecated core is not recommended.
 *          For low level functionality please use the mid layer API.
 *
 * Return Value:RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack          - handle to the stack manager.
 * Output:  phSeli          - handle to the seli.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetSeliHandle(
                                         IN    RvSipStackHandle           hStack,
                                         OUT   RV_SELI_Handle             *phSeli)
{
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;

    if (NULL == phSeli)
    {
        return RV_ERROR_NULLPTR;
    }
    if (NULL == hStack)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phSeli = (RV_SELI_Handle)pStackMgr->pSelect;
    return RV_OK;

}

/************************************************************************************
 * RvSipStackGetTimerPoolHandle
 * ----------------------------------------------------------------------------------
 * General: This function can be used only if the sip stack is compiled with
 *          RV_DEPRECATED_CORE compilation flag which means that the application
 *          is using the deprecated core implementation.
 *          However, this function is deprecated and will always return NULL.
 *          If you are using the deprecated core to set timers you must first
 *          call the TIMER pool initialization functions to allocate the timer pool.
 *          only then you will be able to set timers.
 *          Using the deprecated core is not recommended.
 *          For low level functionality please use the mid layer API.
 * Return Value:
 *          RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack          - Handle to the stack manager.
 * Output:  phTimerPool      - Will always return NULL.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetTimerPoolHandle(
                                         IN    RvSipStackHandle           hStack,
                                         OUT   RV_TIMER_PoolHandle        *phTimerPool)
{
    StackMgr *pStackMgr;

    pStackMgr = (StackMgr*)hStack;
    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }
    *phTimerPool = NULL;
    return RV_OK;

}
#endif /*defined(RV_DEPRECATED_CORE)*/

/************************************************************************************
 * RvSipStackSetNewLogFilters
 * ----------------------------------------------------------------------------------
 * General: set the new log filters for the specified module in run time.
 *          note that the new filters are not only the ones that we want to change,
 *          but they are a new set of filters that the module is going to use.
 * Return Value:
 *          RV_OK          - log filters were set successfully
 *          RV_ERROR_INVALID_HANDLE    - the handle to the stack was NULL
 *          RV_ERROR_BADPARAM - the register id of this module is not used
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack         - Handle to stack instance
 *          eModule        - the module whose filters are going to be changed
 *                           If the module name is RVSIP_CORE the function will change
 *                           the logging level for all the core modules.
 *          filters        - the new set of filters
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackSetNewLogFilters(
                                                IN RvSipStackHandle  hStack,
                                                IN RvSipStackModule  eModule,
                                                IN RvInt32          filters)
{

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    StackMgr  *pStackMgr;
    RvInt32   moduleNumber;
    RvInt32   firstModule = 0;
    RvInt32   lastModule  = 0;
    RvInt32   i           = 0;

    moduleNumber = (RvInt32)eModule;

    if(hStack == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pStackMgr = (StackMgr*)hStack;

    if ((moduleNumber >= STACK_NUM_OF_LOG_SOURCE_MODULES &&
         moduleNumber != RVSIP_CORE && moduleNumber != RVSIP_ADS)
        || moduleNumber < 0)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                     "RvSipStackSetNewLogFilters - Failed, Invalid eModule %d",
                  moduleNumber));
        return RV_ERROR_BADPARAM;
    }


    /*change login for all core modules*/
    if(eModule == RVSIP_CORE)
    {
        firstModule = (RvInt32)RVSIP_CORE_SEMAPHORE;
        lastModule  = (RvInt32)RVSIP_CORE_IMSIPSEC;
        for(i = firstModule; i<= lastModule; i++)
        {
            /*no need for return value - the function can't fail*/
            RvLogSourceSetMask(pStackMgr->pLogSourceArrey[i],
                               SipCommonConvertSipToCoreLogMask(filters));
        }
    }
    else if(eModule == RVSIP_ADS)
    {
        firstModule = (RvInt32)RVSIP_ADS_RLIST;
        lastModule  = (RvInt32)RVSIP_ADS_PQUEUE;
        for(i = firstModule; i<= lastModule; i++)
        {
            /*no need for return value - the function can't fail*/
            RvLogSourceSetMask(pStackMgr->pLogSourceArrey[i],
                               SipCommonConvertSipToCoreLogMask(filters));
        }
    }
    else
    {
        /*change logging for a specific module*/
        RvLogSourceSetMask(pStackMgr->pLogSourceArrey[eModule],
                           SipCommonConvertSipToCoreLogMask(filters));
    }
    return RV_OK;
#else
	RV_UNUSED_ARGS((filters,eModule,hStack));
    return RV_OK;
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
}



/************************************************************************************
 * RvSipStackIsLogFilterExist
 * ----------------------------------------------------------------------------------
 * General: check existence of a filter for a given module
 * Return Value:
 *          RV_TRUE        - the filter exists for the given module
 *          RV_False       - the filter does not exist for the given module
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack         - Handle to stack instance
 *          eModule         - the module whose filter is being checked
 *          filter         - the filter that we want to check existence for
 * Output:  none
 ***********************************************************************************/
RVAPI RvBool RVCALLCONV RvSipStackIsLogFilterExist(
                                              IN RvSipStackHandle  hStack,
                                              IN RvSipStackModule  eModule,
                                              IN RvSipLogFilters   filter)
{
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    RvBool isFilterExist = RV_FALSE;
    StackMgr  *pStackMgr;
    RvInt32   moduleNumber;

    moduleNumber = (RvInt32)eModule;

    if(hStack == NULL)
    {
        return RV_FALSE;
    }

    pStackMgr = (StackMgr*)hStack;

    if (moduleNumber >= STACK_NUM_OF_LOG_SOURCE_MODULES || moduleNumber < 0)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                     "RvSipStackIsLogFilterExist - Failed, Invalid eModule %d",
                  moduleNumber));
        return RV_FALSE;
    }


    isFilterExist = RvLogIsSelected(pStackMgr->pLogSourceArrey[eModule],
                                    SipCommonConvertSipToCoreLogFilter(filter));


    return isFilterExist;
#else
	RV_UNUSED_ARGS((filter,eModule,hStack));
    return RV_FALSE;
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
}

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
/************************************************************************************
 * RvSipStackGetLogFilters
 * ----------------------------------------------------------------------------------
 * General: returns a filter for a given module
 * Return Value:
 *          filter        - the filter for the given module
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack         - Handle to stack instance
 *          eModule         - the module whose filter is being checked
 * Output:  none
 ***********************************************************************************/
RVAPI RvSipLogFilters RVCALLCONV RvSipStackGetLogFilters(
                                              IN RvSipStackHandle  hStack,
                                              IN RvSipStackModule  eModule)
{
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    StackMgr  *pStackMgr;
    RvInt32   moduleNumber;

    moduleNumber = (RvInt32)eModule;

    if(hStack == NULL)
    {
        return RV_FALSE;
    }

    pStackMgr = (StackMgr*)hStack;

    if (moduleNumber >= STACK_NUM_OF_LOG_SOURCE_MODULES || moduleNumber < 0)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                     "RvSipStackGetLogFilters - Failed, Invalid eModule %d",
                  moduleNumber));
        return 0;
    }

    return  SipCommonCoreConvertCoreToSipLogMask(RvLogSourceGetMask(pStackMgr->pLogSourceArrey[eModule]));
#else
    return 0;
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
}
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

/************************************************************************************
 * RvSipStackGetResources
 * ----------------------------------------------------------------------------------
 * General: get the resources status used by the stack. it is done only for the
 *          Call Leg, Message, Transaction, RegClient and Stack modules. the user should declare
 *          the right resources structure according to the given module in the second
 *          arguement. this structure will be filled and will be the output of the function.
 * Return Value:
 *          RV_OK     - the resources' status was got successfully
 *          RV_ERROR_UNKNOWN     - failed in getting the resources' status
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack         - Handle to stack instance
 *          module         - the module whose resources status is being checked
 *          pResources     - the resources in use for the given module
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetResources(
                                              IN RvSipStackHandle      hStack,
                                              IN RvSipStackModule      module,
                                              OUT void                 *pResources)
{
    StackMgr*                  pStackMgr = (StackMgr*)hStack;
    RvStatus                     status;
    RvSipStackResources*       stackResources;

    if (pStackMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }


    switch (module)
    {
#ifndef RV_SIP_PRIMITIVES
        case RVSIP_CALL:
            {
                status = SipCallLegMgrGetResourcesStatus(pStackMgr->hCallLegMgr,
                                                           (RvSipCallLegResources*)pResources);
                return status;
            }
#endif /* #ifndef RV_SIP_PRIMITIVES*/
#ifndef RV_SIP_LIGHT
        case RVSIP_TRANSACTION:
            {
                status = SipTransactionMgrGetResourcesStatus(pStackMgr->hTranscMgr,
                                                               (RvSipTranscResources*)pResources);
                return status;
            }
#endif /*#ifndef RV_SIP_LIGHT*/
        case RVSIP_STACK:
            {
                stackResources = (RvSipStackResources*)pResources;
                RPOOL_GetResources(pStackMgr->hMessagePool,
                                   &(stackResources->msgPoolElements.numOfAllocatedElements),
                                   &(stackResources->msgPoolElements.currNumOfUsedElements),
                                   &(stackResources->msgPoolElements.maxUsageOfElements));
                RPOOL_GetResources(pStackMgr->hGeneralPool,
                                   &(stackResources->generalPoolElements.numOfAllocatedElements),
                                   &(stackResources->generalPoolElements.currNumOfUsedElements),
                                   &(stackResources->generalPoolElements.maxUsageOfElements));
                if(pStackMgr->hElementPool != NULL)
                {

                    RPOOL_GetResources(pStackMgr->hElementPool,
                                       &(stackResources->headerPoolElements.numOfAllocatedElements),
                                       &(stackResources->headerPoolElements.currNumOfUsedElements),
                                       &(stackResources->headerPoolElements.maxUsageOfElements));
                }
                else
                {
                    memset(&stackResources->headerPoolElements,0,sizeof(stackResources->headerPoolElements));
                }
                stackResources->timerPool.maxUsageOfElements     = (RvUint32)RvTimerQueueMaxConcurrentEvents(&pStackMgr->pSelect->tqueue);
                stackResources->timerPool.currNumOfUsedElements  = (RvUint32)RvTimerQueueNumEvents(&pStackMgr->pSelect->tqueue);
                stackResources->timerPool.numOfAllocatedElements = (RvUint32)RvTimerQueueGetSize(&pStackMgr->pSelect->tqueue);
                return RV_OK;
            }
#ifndef RV_SIP_PRIMITIVES
        case RVSIP_REGCLIENT:
            {
                status = SipRegClientMgrGetResourcesStatus(pStackMgr->hRegClientMgr,
                                                           (RvSipRegClientResources*)pResources);
                return status;
            }
		case RVSIP_PUBCLIENT:
            {
                status = SipPubClientMgrGetResourcesStatus(pStackMgr->hPubClientMgr,
					(RvSipPubClientResources*)pResources);
                return status;
            }
#endif /* #ifndef RV_SIP_PRIMITIVES */

        case RVSIP_TRANSPORT:
            {
                status = SipTransportGetResourcesStatus(pStackMgr->hTransport,
                                                        (RvSipTransportResources*)pResources);
                return status;
            }
#if !defined(RV_SIP_PRIMITIVES) && defined(RV_SIP_SUBS_ON)
        case RVSIP_SUBSCRIPTION:
            {
                status = SipSubsMgrGetResourcesStatus(pStackMgr->hSubsMgr,
                                                        (RvSipSubsResources*)pResources);
                return status;
            }
#endif /* #ifndef !defined(RV_SIP_PRIMITIVES) && defined(RV_SIP_SUBS_ON) */
#ifdef RV_SIGCOMP_ON
        case RVSIP_COMPARTMENT:
            {
                status = SipCompartmentMgrGetResourcesStatus(pStackMgr->hCompartmentMgr,
                                                             (RvSipCompartmentResources*)pResources);

                return status;
            }
#endif /* RV_SIGCOMP_ON */
        case RVSIP_TRANSMITTER:
            {
                status = SipTransmitterMgrGetResourcesStatus(pStackMgr->hTrxMgr,
                                                             (RvSipTransmitterResources*)pResources);
                return status;
            }
        case RVSIP_RESOLVER:
            {
                status = SipResolverMgrGetResourcesStatus(pStackMgr->hRslvMgr,
                                                             (RvSipResolverResources*)pResources);
                return status;
            }
#ifdef RV_SIP_IMS_ON
		case RVSIP_SEC_AGREE:
		    {
		        status = SipSecAgreeMgrGetResourcesStatus(pStackMgr->hSecAgreeMgr,
		                                                  (RvSipSecAgreeResources*)pResources);
		        return status;
		    }
        case RVSIP_SECURITY:
            {
                status = RV_OK;
                if (pStackMgr->hSecMgr != NULL)
                {
                    status = SipSecurityMgrGetResourcesStatus(pStackMgr->hSecMgr,
                                            (RvSipSecurityResources*)pResources);
                }
                return status;
            }
#endif /*#ifdef RV_SIP_IMS_ON*/
        default:
            {
                RvLogExcep(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                           "RvSipStackGetResources - Unknown type of module"));
                return RV_ERROR_UNKNOWN;
            }
    }
}

#ifndef RV_SIP_LIGHT
#ifdef SIP_DEBUG
/************************************************************************************
 * RvSipStackGetStatistics
 * ----------------------------------------------------------------------------------
 * General: Gets the statistics about number of messages sent and received by the stack.
 *          You must supply here a pointer to a valid RvSipStackStatistics structure.
 *          The stack will fill this structure with the current statistics.
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack         - Handle to the RADVISION SIP stack object.
 * Output:  pStatistics    - The structure to be filled with the current statistics.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackGetStatistics(
                                           IN    RvSipStackHandle       hStack,
                                           OUT   RvSipStackStatistics  *pStatistics)
{
    StackMgr                    *pStackMgr;
    RvStatus                    status;
    SipTransactionMgrStatistics  msgStatistics;

    pStackMgr = (StackMgr*)hStack;
    if (NULL == hStack || NULL == pStatistics)
    {
        return RV_ERROR_NULLPTR;
    }

    memset(pStatistics, 0, sizeof(RvSipStackStatistics));

    status = SipTransactionMgrGetStatistics(pStackMgr->hTranscMgr, &msgStatistics);
    if (RV_OK != status)
    {
        return status;
    }

    pStatistics->rcvdINVITE = msgStatistics.rcvdINVITE;
    pStatistics->rcvdINVITERetrans = msgStatistics.rcvdINVITERetrans;
    pStatistics->rcvdNonInviteReq = msgStatistics.rcvdNonInviteReq;
    pStatistics->rcvdNonInviteReqRetrans = msgStatistics.rcvdNonInviteReqRetrans;
    pStatistics->rcvdResponse = msgStatistics.rcvdResponse;
    pStatistics->rcvdResponseRetrans = msgStatistics.rcvdResponseRetrans;
    pStatistics->sentINVITE = msgStatistics.sentINVITE;
    pStatistics->sentINVITERetrans = msgStatistics.sentINVITERetrans;
    pStatistics->sentNonInviteReq = msgStatistics.sentNonInviteReq;
    pStatistics->sentNonInviteReqRetrans = msgStatistics.sentNonInviteReqRetrans;
    pStatistics->sentResponse = msgStatistics.sentResponse;
    pStatistics->sentResponseRetrans = msgStatistics.sentResponseRetrans;


    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
              "Stack statistics: ******************************"));
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "rcvd INVITE: %u", pStatistics->rcvdINVITE));
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "rcvd INVITE Retrans: %u", pStatistics->rcvdINVITERetrans));
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "rcvd Non Invite Req: %u", pStatistics->rcvdNonInviteReq));
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "rcvd Non Invite Req Retrans: %u", pStatistics->rcvdNonInviteReqRetrans));
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "rcvd Response: %u", pStatistics->rcvdResponse));
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "rcvd Response Retrans: %u", pStatistics->rcvdResponseRetrans));
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "sent INVITE: %u", pStatistics->sentINVITE));
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "sent INVITE Retrans: %u", pStatistics->sentINVITERetrans));
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "sent Non Invite Req: %u", pStatistics->sentNonInviteReq));
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "sent Non Invite Req Retrans: %u", pStatistics->sentNonInviteReqRetrans));
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "sent Response: %u", pStatistics->sentResponse));
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "sent Response Retrans: %u", pStatistics->sentResponseRetrans));
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "*********************************"));

    return RV_OK;

}
#endif
#endif /*#ifndef RV_SIP_LIGHT*/


/************************************************************************************
 * RvSipStackGetVersion
 * ----------------------------------------------------------------------------------
 * General: get the current version of the RADVISION SIP stack.
 * Return Value: RvChar   - the version
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   none
 * Output:  none
 ***********************************************************************************/
RVAPI RvChar * RVCALLCONV RvSipStackGetVersion(void)
{
    return SIP_STACK_VERSION;
}

/************************************************************************************
 * RvSipStackProcessEvents
 * ----------------------------------------------------------------------------------
 * General:  Checks for events and process them as they come.
 *           This function is usually used for console applications.
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   (-)
 * Output:  (-)
 ***********************************************************************************/
RVAPI void RVCALLCONV RvSipStackProcessEvents(void)
{
#define RV_SELECT_TIMEOUT RvUint64Const(0xFFFFFF,0xFFFFFFFF)
	RvStatus		crv;	
    RvSelectEngine *pEngine = NULL;

    crv = RvSelectGetThreadEngine(NULL,&pEngine);
	if (crv != RV_OK || pEngine == NULL)
    {
        return;
    }
	    
    for (;;)
    {
        RvSelectWaitAndBlock(pEngine,RV_SELECT_TIMEOUT);
    }
}

/************************************************************************************
 * RvSipStackStopEventProcessing
 * ----------------------------------------------------------------------------------
 * General: Causes RvSipStackProcessEvents API function to return.
 *          In order to shutdown event processing thread gracefully,
 *          the application has to stop infinite loop, implemented in
 *          RvSipStackProcessEvents API function. This can be done by call
 *          to RvSipStackStopEventProcessing API function.
 * Return Value: RV_OK on success, error otherwise.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   (-)
 * Output:  (-)
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackStopEventProcessing(void)
{
	RvStatus		crv;	
    RvSelectEngine *pEngine = NULL;

    crv = RvSelectGetThreadEngine(NULL,&pEngine);
	if (crv != RV_OK || pEngine == NULL)
    {
        return crv;
    }

    crv = RvSelectStopWaiting(pEngine,RvSelectEmptyPreemptMsg,NULL/*logMgr*/);
	if (crv != RV_OK || pEngine == NULL)
    {
        return crv;
    }
    return RV_OK;
}


/************************************************************************
 * RvSipStackSelect
 * General: On operating systems that support the select interface,
 *          this function will preform one call to the select() function.
 *          An application should write a "while (1) RvSipStackSelect();"
 *          as its main loop.
 * input  : none
 * output : none
 * return : RV_OK on success, negative value on failure
 ************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackSelect(void)
{
    RvStatus crv;
    crv = RvSipStackSelectUntil(RV_UINT32_MAX);
    return CRV2RV(crv);
}

/******************************************************************************
 * RvSipStackSelectUntil
 * ----------------------------------------------------------------------------
 * General: On operating systems that support the select interface,
 *          this function will preform one call to the select() function.
 *          This function also gives the application the ability to give a
 *          maximum blocking delay.
 *
 * Return Value: RV_OK on success, negative value on failure.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  delay    - Maximum time to block in milliseconds.
 * Output: None.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackSelectUntil(IN RvUint32 delay)
{
    RvSelectEngine *selectEngine = NULL;
    RvStatus		crv;

    crv = RvSelectGetThreadEngine(NULL, &selectEngine);
    if (crv != RV_OK)
    {
        return CRV2RV(crv);
    }
    if (selectEngine == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    crv = RvSelectWaitAndBlock(selectEngine, RvInt64Mul(RV_TIME64_NSECPERMSEC,RvInt64FromRvUint32(delay)));

    return CRV2RV(crv);
}

/************************************************************************************
 * RvSipStackInitCfg
 * ----------------------------------------------------------------------------------
 * General: Initializes the RvSipStackCfg  structure. This structure is given to the
 *          RvSipStackConstruct function and it is used to initialize the stack.
 *          The RvSipStackInitCfg function relates to two types of parameters found in
 *          the RvSipStackCfg structure:
 *          (A) Parameters that influence the value of other parameters
 *          (B) Parameters that are influenced by the value of other parameters.
 *          The RvSipStackInitCfg will set all parameters of type A to default values
 *          and parameters of type B to -1.
 *          For example the maxCallLegs is of type A and will be set to 10.
 *          The maxTransaction is of type B and is set to -1.
 *          When calling the RvSipStackConstruct function all the B type parameters
 *          will be calculated using the values found in the A type parameters.
 *          if you change the A type values the B type values will be changed
 *          accordingly.
 *
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:  sizeOfCfg - The size of the configuration structure.
 * output: pStackCfg - The configuration structure containing the RADVISION SIP stack default values.
 *                     values.
 ***********************************************************************************/
RVAPI void RvSipStackInitCfg(IN  RvInt32        sizeOfCfg,
                             OUT RvSipStackCfg *pStackCfg)
{
     RvSipStackCfg internalStackCfg;
     RvInt32 minCfgSize;

     /*for backwords compatablity we initiate and internal struc and copy
       it to the given struct*/
     minCfgSize = RvMin(((RvUint)sizeOfCfg),sizeof(internalStackCfg));

     memset(pStackCfg,0,sizeOfCfg);
     memset(&internalStackCfg,0,sizeof(internalStackCfg));

     /*Stack objects allocation*/
#ifndef RV_SIP_PRIMITIVES
     internalStackCfg.maxCallLegs           = DEFAULT_MAX_CALL_LEGS;
     internalStackCfg.maxRegClients         = DEFAULT_MAX_REG_CLIENTS;
	 internalStackCfg.maxPubClients         = DEFAULT_MAX_PUB_CLIENTS;
	 internalStackCfg.regAlertTimeout		= DEFAULT_CLIENT_ALERT_TIMER;
	 internalStackCfg.pubAlertTimeout		= DEFAULT_CLIENT_ALERT_TIMER;
#endif /* #ifndef RV_SIP_PRIMITIVES*/
     internalStackCfg.maxTransactions       = UNDEFINED;
     internalStackCfg.maxTransmitters       = UNDEFINED;
     /*Memory pools allocation*/
     internalStackCfg.messagePoolNumofPages = UNDEFINED;
     internalStackCfg.messagePoolPageSize   = DEFAULT_MSG_POOL_PAGE_SIZE;
     internalStackCfg.generalPoolNumofPages = UNDEFINED;
     internalStackCfg.generalPoolPageSize   = DEFAULT_GENERAL_POOL_PAGE_SIZE;
     internalStackCfg.elementPoolNumofPages  = UNDEFINED;
     internalStackCfg.elementPoolPageSize    = DEFAULT_ELEMENT_POOL_PAGE_SIZE;

     /*log print function*/
     internalStackCfg.pfnPrintLogEntryEvHandler = NULL;
     internalStackCfg.logContext = NULL;
     /*Network parameters*/
     internalStackCfg.sendReceiveBufferSize  = DEFAULT_MAX_BUFFER_SIZE;
     strcpy(internalStackCfg.localUdpAddress,IPV4_LOCAL_ADDRESS);
     internalStackCfg.localUdpPort            = RVSIP_TRANSPORT_DEFAULT_PORT;
     internalStackCfg.outboundProxyPort       = RVSIP_TRANSPORT_DEFAULT_PORT;
     internalStackCfg.eOutboundProxyTransport = RVSIP_TRANSPORT_UNDEFINED;
     internalStackCfg.outboundProxyHostName   = NULL;
     internalStackCfg.maxConnections          = UNDEFINED;
     internalStackCfg.ePersistencyLevel       = RVSIP_TRANSPORT_PERSISTENCY_LEVEL_UNDEFINED;
     internalStackCfg.serverConnectionTimeout = UNDEFINED;
     internalStackCfg.connectionCapacityPercent = 0;
     strcpy(internalStackCfg.localTcpAddress,IPV4_LOCAL_ADDRESS);
     internalStackCfg.localTcpPort            = RVSIP_TRANSPORT_DEFAULT_PORT;
     internalStackCfg.localUdpAddresses       = NULL;
     internalStackCfg.localTcpAddresses       = NULL;
     internalStackCfg.localUdpPorts           = NULL;
     internalStackCfg.localTcpPorts           = NULL;
     internalStackCfg.tcpEnabled              = RV_FALSE;
     internalStackCfg.bDLAEnabled             = RV_FALSE;
#ifdef RV_SIP_AUTH_ON
     internalStackCfg.enableServerAuth        = RV_FALSE;
#endif /* #ifdef RV_SIP_AUTH_ON */
     internalStackCfg.numOfExtraTcpAddresses  = 0;
     internalStackCfg.numOfExtraUdpAddresses  = 0;
     internalStackCfg.maxNumOfLocalAddresses  = UNDEFINED; /* One UDP and one TCP addr */
     /*Timers*/
     internalStackCfg.retransmissionT1        = DEFAULT_T1;
     internalStackCfg.retransmissionT2        = DEFAULT_T2;
     internalStackCfg.retransmissionT4        = DEFAULT_T4;
     internalStackCfg.generalLingerTimer           = UNDEFINED;
     internalStackCfg.inviteLingerTimer            = DEFAULT_INVITE_LINGER_TIMER;
     internalStackCfg.cancelGeneralNoResponseTimer = UNDEFINED;
     internalStackCfg.cancelInviteNoResponseTimer  = UNDEFINED;
     internalStackCfg.generalRequestTimeoutTimer   = UNDEFINED;
     internalStackCfg.provisionalTimer             = DEFAULT_PROVISIONAL_TIMER;

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT)
    internalStackCfg.getAlternativeSecObjFunc = NULL;
    internalStackCfg.if_name[0] = 0;
    internalStackCfg.vdevblock = 0;
    internalStackCfg.localVDevBlocks = NULL;
    internalStackCfg.localIfNames = NULL;
#endif
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    /* default the maximum retransmit INVITE message to 6 */
    internalStackCfg.maxRetransmitINVITENumber = 6;
    /* default the general request timeout per message to DEFAULT_T1=500ms */
    internalStackCfg.initialGeneralRequestTimeout = DEFAULT_T1;
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */


     /*log*/
     internalStackCfg.defaultLogFilters =  RVSIP_LOG_INFO_FILTER  |
                                           RVSIP_LOG_ERROR_FILTER |
                                           RVSIP_LOG_DEBUG_FILTER |
                                           RVSIP_LOG_EXCEP_FILTER |
                                           RVSIP_LOG_WARN_FILTER;

     internalStackCfg.enableInviteProceedingTimeoutState = RV_FALSE;
     internalStackCfg.bEnableForking                = RV_FALSE;

#ifndef RV_SIP_PRIMITIVES
     internalStackCfg.rejectUnsupportedExtensions   = RV_FALSE;
     internalStackCfg.addSupportedListToMsg         = RV_TRUE;
     internalStackCfg.supportedExtensionList        = NULL;
     internalStackCfg.manualPrack                   = RV_FALSE;
     internalStackCfg.forkedAckTrxTimeout           = DEFAULT_FORKED_CALL_ACK_TIMER;
     internalStackCfg.forked1xxTimerTimeout         = UNDEFINED;
#endif /* RV_SIP_PRIMITIVES */
     internalStackCfg.manualAckOn2xx    = RV_FALSE;
     internalStackCfg.isProxy           = RV_FALSE;
     internalStackCfg.proxy2xxRcvdTimer = UNDEFINED;
     internalStackCfg.proxy2xxSentTimer = DEFAULT_PROXY_2XX_SEND_TIMER;
     internalStackCfg.bDisableMerging   = RV_FALSE;
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON
     /* subscription */
     internalStackCfg.bEnableSubsForking        = RV_FALSE;
     internalStackCfg.maxSubscriptions          = DEFAULT_MAX_SUBSCRIPTIONS;
     internalStackCfg.subsAlertTimer            = DEFAULT_SUBS_ALERT_TIMER;
     internalStackCfg.subsAutoRefresh           = RV_FALSE;
     internalStackCfg.subsNoNotifyTimer         = DEFAULT_SUBS_NO_NOTIFY_TIMER;
     internalStackCfg.bDisableRefer3515Behavior = RV_FALSE;
#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /* RV_SIP_PRIMITIVES */
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
     internalStackCfg.maxElementsInSingleDnsList = DEFAULT_MAX_NUM_OF_ELEMENTS_IN_DNS_LIST;
#endif
     /* Multi-threaded parameters */
     internalStackCfg.numberOfProcessingThreads = DEFAULT_NUM_OF_PROCESSING_THREADS;
     internalStackCfg.processingTaskPriority    = DEFAULT_PROCESSING_THREAD_PRIORITY;
     internalStackCfg.processingTaskStackSize   = DEFAULT_PROCESSING_THREAD_STACK_SIZE;
     internalStackCfg.processingQueueSize       = UNDEFINED;
     internalStackCfg.numOfReadBuffers          = UNDEFINED;
#ifndef RV_SIP_PRIMITIVES
     /*Session timer parameters */
     internalStackCfg.sessionExpires         = 1800;
     internalStackCfg.minSE                  = UNDEFINED;
     internalStackCfg.manualSessionTimer     = RV_FALSE;
     internalStackCfg.manualBehavior         = RV_FALSE;
     internalStackCfg.bDynamicInviteHandling = RV_FALSE;
#else
     internalStackCfg.bDynamicInviteHandling = RV_TRUE;
#endif /* RV_SIP_PRIMITIVES */
     internalStackCfg.bUseRportParamInVia    = RV_FALSE;

     /* tls */
     internalStackCfg.localTlsAddresses = NULL;
     internalStackCfg.localTlsPorts     = NULL;
     internalStackCfg.numOfTlsAddresses = 0;
     internalStackCfg.numOfTlsEngines   = 0;
     internalStackCfg.maxTlsSessions    = UNDEFINED;

#ifdef RV_SIGCOMP_ON
     internalStackCfg.eOutboundProxyCompression = RVSIP_COMP_UNDEFINED;
     internalStackCfg.maxCompartments           = UNDEFINED;
     internalStackCfg.sigCompTcpTimer           = UNDEFINED;
#endif /* RV_SIGCOMP_ON */
     internalStackCfg.pDnsServers               = NULL;
     internalStackCfg.numOfDnsServers           = UNDEFINED;
     internalStackCfg.numOfDnsDomains           = UNDEFINED;
     internalStackCfg.pDnsDomains               = NULL;
     internalStackCfg.maxDnsDomains             = UNDEFINED;
     internalStackCfg.maxDnsServers             = UNDEFINED;
     internalStackCfg.maxDnsBuffLen             = DEFAULT_ARES_BUFFER_SIZE;

     /*version 4.0 parameters*/
     internalStackCfg.bOldInviteHandling        = RV_FALSE;
     internalStackCfg.bResolveTelUrls           = RV_FALSE;
     internalStackCfg.strDialPlanSuffix          = NULL;
     /* SCTP */
#if (RV_NET_TYPE & RV_NET_SCTP)
     internalStackCfg.numOfSctpAddresses			 = 0;
     internalStackCfg.localSctpAddresses			 = NULL;
     internalStackCfg.localSctpPorts				 = NULL;
     internalStackCfg.numOfOutStreams				 = DEFAULT_SCTP_OUT_STREAMS;
     internalStackCfg.numOfInStreams				 = DEFAULT_SCTP_IN_STREAMS;
     internalStackCfg.avgNumOfMultihomingAddresses   = RVSIP_TRANSPORT_SCTP_MULTIHOMED_ADDR_NUM_AVG_PER_CONN;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_IMS_ON
     internalStackCfg.maxSecurityObjects = UNDEFINED;
     internalStackCfg.maxIpsecSessions   = UNDEFINED;
	 internalStackCfg.maxSecAgrees       = UNDEFINED;
	 internalStackCfg.maxOwnersPerSecAgree	= UNDEFINED;
#endif /*#ifdef RV_SIP_IMS_ON*/

	 /*ignore local addresses*/
	 internalStackCfg.bIgnoreLocalAddresses     = RV_FALSE;

     /* copying the configuration struct to the internal struct */
     memcpy(pStackCfg,&internalStackCfg,minCfgSize);
}

#ifndef RV_SIP_PRIMITIVES
/************************************************************************************
 * RvSipStackIsSessionTimerSupported
 * ----------------------------------------------------------------------------------
 * General: Returns weather the stack supports the session timer feature.
 *          bIsSupported output parameter will be set to RV_TRUE if the
 *          stack supported list includs the "timer" option tag. Otherwise it will be
 *          set to RV_FALSE.
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:    hStack         - Handle to the RADVISION SIP stack.
 * Output:   bIsSupported   - RV_TRUE if session timer is supported, RV_FALSE otherwise.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackIsSessionTimerSupported(
                                           IN    RvSipStackHandle      hStack,
                                           OUT   RvBool               *pbIsSupported)
{
    RvInt32    i =0;
    StackMgr *pStackMgr = (StackMgr *)hStack;
    if (NULL == hStack )
    {
        return RV_ERROR_NULLPTR;
    }
    *pbIsSupported = RV_FALSE;
    for(i=0;i < pStackMgr->extensionListSize;i++)
    {
        if(strcmp(pStackMgr->supportedExtensionList[i],"timer") == 0)
        {
            *pbIsSupported = RV_TRUE;
            break;
        }
    }
    return RV_OK;

}

/***************************************************************************
 * RvSipStackIsReplacesSupported
 * ------------------------------------------------------------------------
 * General: Returns weather the stack supports the replaces feature.
 *          bIsSupported output parameter will be set to RV_TRUE if the
 *          stack supported list includes the "replaces" option tag. Otherwise it will be
 *          set to RV_FALSE.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCallLegMgr    - The call-leg manager handle.
 * Output:  pbIsSupported  - RV_TRUE if replaces is supported, RV_FALSE otherwise.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackIsReplacesSupported(
                                    IN  RvSipStackHandle            hStack,
                                    OUT RvBool                    *pbIsSupported)
{
    RvInt32    i =0;
    StackMgr *pStackMgr = (StackMgr *)hStack;
    if (NULL == hStack)
    {
        return RV_ERROR_NULLPTR;
    }
    for(i=0;i < pStackMgr->extensionListSize;i++)
    {
        if(strcmp(pStackMgr->supportedExtensionList[i],"replaces") == 0)
        {
            *pbIsSupported = RV_TRUE;
            break;
        }
    }
    return RV_OK;
}

#endif /*#ifndef RV_SIP_PRIMITIVES */


/************************************************************************************
 * RvSipStackMgrIsEnhancedDnsFeatureEnabled
 * ----------------------------------------------------------------------------------
 * General: Returns RV_TRUE if SIP stack was compiled with RV_DNS_ENHANCED_FEATURES_SUPPORT
 * flag.
 * Return Value: returns RV_TRUE if SIP stack was compiled with RV_DNS_ENHANCED_FEATURES_SUPPORT
 * flag and RV_FALSE otherwise.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   none
 * Output:  none
 ***********************************************************************************/
RVAPI RvBool RvSipStackMgrIsEnhancedDnsFeatureEnabled(void)
{
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    return RV_TRUE;
#else
    return RV_FALSE;
#endif
}

/************************************************************************************
 * RvSipStackMgrIsTlsFeatureEnabled
 * ----------------------------------------------------------------------------------
 * General: Returns RV_TRUE if SIP stack was compiled with RV_TLS_TYPE==RV_TLS_OPENSSL
 * flag.
 * Return Value: returns RV_TRUE if SIP stack was compiled with RV_TLS_TYPE==RV_TLS_OPENSSL)
 * flag and RV_FALSE otherwise.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   none
 * Output:  none
 ***********************************************************************************/
RVAPI RvBool RvSipStackMgrIsTlsFeatureEnabled(void)
{
#if (RV_TLS_TYPE != RV_TLS_NONE)
    return RV_TRUE;
#else
    return RV_FALSE;
#endif
}

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * RvSipStackSetSigCompMgrHandle
 * ------------------------------------------------------------------------
 * General: Stores the SigComp manager handle in the stack manager.
 *          This function must be called only after the stack was constructed
 *          successfully. You get the SigComp manager when the SigComp module
 *          is constructed. The sip stack will use the SigComp manager to compress
 *          and decompress SIP messages.
 *          This function also set the SigComp manager in the compartment manager,
 *          and in the transport manager.
 * NOTE:    This function MUST be called when no activity (either SigComp or
 *          non-SigComp) takes place within the SIP Stack 
 *          (otherwise you might encounter an error). 
 *          For example, this function might be called right after the SIP 
 *          Stack construction before any SIP Stack objects are activated 
 *          (due to incoming or outgoing messages). 
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hStack      - The stack handle.
 *            hSigCompMgr - Handle of the SigComp manager.
 * Output:    -
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipStackSetSigCompMgrHandle(
                      IN  RvSipStackHandle       hStack,
                      IN  RvSigCompMgrHandle     hSigCompMgr)
{
    RvStatus  rv; 
    StackMgr *pStackMgr = (StackMgr *)hStack;

    if (NULL == hStack)
    {
        return RV_ERROR_NULLPTR;
    }

    if(NULL == hSigCompMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }


    if (pStackMgr->hCompartmentMgr == NULL ||
        pStackMgr->hTransport      == NULL ||
        pStackMgr->hTranscMgr      == NULL)
    {
        return RV_ERROR_ILLEGAL_ACTION;
    }

    /* Update the constructed SigComp manager handle in the different stack managers */
    rv = SipCompartmentMgrSetSigCompMgrHandle(pStackMgr->hCompartmentMgr,hSigCompMgr);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "RvSipStackSetSigCompMgrHandle - Stack 0x%p failed to set the SigComp manager handle 0x%p in Compartment layer (rv=%d)",
            pStackMgr, hSigCompMgr, rv));
        return rv; 
    }
    rv = SipTransportMgrSetSigCompMgrHandle(pStackMgr->hTransport,hSigCompMgr);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "RvSipStackSetSigCompMgrHandle - Stack 0x%p failed to set the SigComp manager handle 0x%p in Transport layer (rv=%d)",
            pStackMgr, hSigCompMgr, rv));
        /* Reset the SigComp manager handle within the Compartment layer */ 
        SipCompartmentMgrSetSigCompMgrHandle(pStackMgr->hCompartmentMgr,pStackMgr->hSigCompMgr); 

        return rv; 
    }

    /* Store the SigComp manager handle locally */ 
    pStackMgr->hSigCompMgr = hSigCompMgr;

    return RV_OK;
}
#endif /* RV_SIGCOMP_ON */

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/


/************************************************************************************
 * ConstructStackLog
 * ----------------------------------------------------------------------------------
 * General: Initialize the log manager, required log listener and log sources
 *          Note: if the function fails it cleans all allocated resources
 *                (destructs log, listeners and sources).
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackMgr      -  pointer to the stack manager
 *          pStackCfg      -  the stack configuration structure
 ***********************************************************************************/
static RvStatus RVCALLCONV ConstructStackLog(IN    StackMgr      *pStackMgr,
                                        IN    RvSipStackCfg *pStackCfg)

{
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    int           i,j;
    RvStatus     status = RV_OK;
    RvInt32       logMask;
    RvStatus      crv    = RV_OK;

    pStackMgr->pLogMgr = NULL;

    /*construct the log manager*/
    crv = RvLogConstruct(&pStackMgr->logMgr);
    if(crv != RV_OK)
    {
        return CRV2RV(crv);
    }
    pStackMgr->pLogMgr = &pStackMgr->logMgr;
    RvLogPrintThreadId(pStackMgr->pLogMgr);
    /*initializing the default log incase the application did not register
    a callback function*/
    if(pStackCfg->pfnPrintLogEntryEvHandler == NULL)
    {
#if (RV_LOGLISTENER_TYPE == RV_LOGLISTENER_TERMINAL)
        crv = RvLogListenerConstructTerminal(&pStackMgr->logListener, pStackMgr->pLogMgr, RV_TRUE);
        if(crv == RV_OK)
        {
            pStackMgr->bTerminalListenerEnabled = RV_TRUE;
        }
#elif (RV_LOGLISTENER_TYPE != RV_LOGLISTENER_NONE)

#if (RV_OS_TYPE == RV_OS_TYPE_SYMBIAN)
#define LOG_FILE_NAME 	"C:/DATA/SipLog.txt"
#else
#define LOG_FILE_NAME 	"SipLog.txt"
#endif

        crv = RvLogListenerConstructLogfile(&pStackMgr->logListener,
                                            pStackMgr->pLogMgr,
											LOG_FILE_NAME,1,0,RV_TRUE);
        if(crv == RV_OK)
        {
            pStackMgr->bLogFileListenerEnabled = RV_TRUE;
        }
#endif
    }
    else
    {
        /*register the application callback to the common core*/
        pStackMgr->pfnPrintLogEntryEvHandler = pStackCfg->pfnPrintLogEntryEvHandler;
        pStackMgr->logContext = pStackCfg->logContext;
        crv = RvLogRegisterListener(pStackMgr->pLogMgr,
                                    LogPrintCallbackFunction,
                                    (void*)pStackMgr);
        pStackMgr->bLogFileListenerEnabled = RV_FALSE;
    }

    if(crv != RV_OK)
    {
        RvLogDestruct(pStackMgr->pLogMgr);
        pStackMgr->pLogMgr = NULL;
        return CRV2RV(crv);
    }

    /* set the core log masks*/
    SetTemporaryCoreLogMasks(pStackMgr,pStackCfg);


    /*allocate and construct all log sources*/
    for(i=0; i<STACK_NUM_OF_LOG_SOURCE_MODULES; i++)
    {
        status = ConstructLogSource(pStackMgr,(RvSipStackModule)i);
        if(status != RV_OK)
        {
            for(j=0; j<i; j++)
            {
                DestructLogSource(pStackMgr,(RvSipStackModule)j);
            }
            DestructLogListeners(pStackMgr);
            RvLogDestruct(pStackMgr->pLogMgr);
            pStackMgr->pLogMgr = NULL;
            return status;
        }
    }
    pStackMgr->pLogSrc = pStackMgr->pLogSourceArrey[RVSIP_STACK];

    /*set the global mask to all log source according to the defaultLogFilter*/
    RvLogSetGlobalMask(pStackMgr->pLogMgr,
                       SipCommonConvertSipToCoreLogMask(pStackCfg->defaultLogFilters));

    /*set global mask to all core modules only if the coreFilter is bigger then 0*/
    if(pStackCfg->coreLogFilters > 0)
    {
        status = RvSipStackSetNewLogFilters((RvSipStackHandle)pStackMgr,RVSIP_CORE,pStackCfg->coreLogFilters);
        if (status != RV_OK)
        {
            for(i=0; i<STACK_NUM_OF_LOG_SOURCE_MODULES; i++)
            {
                DestructLogSource(pStackMgr,(RvSipStackModule)i);
            }
            DestructLogListeners(pStackMgr);
            RvLogDestruct(pStackMgr->pLogMgr);
            pStackMgr->pLogMgr = NULL;
            return status;
        }
    }

    if(pStackCfg->adsLogFilters > 0)
    {
        status = RvSipStackSetNewLogFilters((RvSipStackHandle)pStackMgr,RVSIP_ADS,pStackCfg->adsLogFilters);
        if (status != RV_OK)
        {
            for(i=0; i<STACK_NUM_OF_LOG_SOURCE_MODULES; i++)
            {
                DestructLogSource(pStackMgr,(RvSipStackModule)i);
            }
            DestructLogListeners(pStackMgr);
            RvLogDestruct(pStackMgr->pLogMgr);
            pStackMgr->pLogMgr = NULL;
            return status;
        }

    }

    for(i=0; i<STACK_NUM_OF_LOG_SOURCE_MODULES; i++)
    {
        logMask = GetModuleLogMaskFromCfg(pStackCfg,(RvSipStackModule)i);
        if(logMask > 0)
        {
            RvSipStackSetNewLogFilters((RvSipStackHandle)pStackMgr,(RvSipStackModule)i,logMask);
        }
    }

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "ConstructStackLog - Log was initialized successfully"));
    RvLogSourcePrintInterfacesData(pStackMgr->pLogSrc, RvCCoreInterfaces());

    return RV_OK;

#else
    pStackMgr->pLogMgr = NULL;
    memset(pStackMgr->pLogSourceArrey, 0,  sizeof(pStackMgr->pLogSourceArrey));
	RV_UNUSED_ARG(pStackCfg);
    return RV_OK;
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/************************************************************************************
 * SetTemporaryCoreLogMasks
 * ----------------------------------------------------------------------------------
 * General: set masks to the core modules before starting using core functions.
 * Return Value: the log mask
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackMgr     - The stack manager
 *          pStackCfg     - The configuration structure
 * Output:  none
 ***********************************************************************************/
static void RVCALLCONV SetTemporaryCoreLogMasks(IN    StackMgr      *pStackMgr,
                                                IN    RvSipStackCfg *pStackCfg)
{
    RvInt32       firstModule   = 0;
    RvInt32       lastModule    = 0;
    RvInt32       i             = 0;
    RvLogSource   tempLogSource = NULL;

    /*set default mask*/
    RvLogSetGlobalMask(pStackMgr->pLogMgr,
                       SipCommonConvertSipToCoreLogMask(pStackCfg->defaultLogFilters));

    /*override with core default mask*/
    if(pStackCfg->coreLogFilters > 0)
    {
        RvLogSetGlobalMask(pStackMgr->pLogMgr,
                       SipCommonConvertSipToCoreLogMask(pStackCfg->defaultLogFilters));
    }


    firstModule = (RvInt32)RVSIP_CORE_SEMAPHORE;
    lastModule  = (RvInt32)RVSIP_CORE_IMSIPSEC;
    for(i = firstModule; i<= lastModule; i++)
    {
        RvInt32 moduleMask = GetModuleLogMaskFromCfg(pStackCfg,(RvSipStackModule)i);
        if(moduleMask > 0)
        {
            tempLogSource = NULL;
            RvLogGetSourceByName(pStackMgr->pLogMgr,GetModuleName((RvSipStackModule)i),&tempLogSource);
            if(tempLogSource != NULL)
            {
               RvLogSourceSetMask(
                            &tempLogSource,
                            SipCommonConvertSipToCoreLogMask(moduleMask));
            }
        }
    }
}



/************************************************************************************
 * GetModuleLogMaskFromCfg
 * ----------------------------------------------------------------------------------
 * General: get the log mask of a specific module from the configuration structure
 * Return Value: the log mask
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg     - The configuration structure
 *          eLogModule    - The module we want to get its filters.
 * Output:  none
 ***********************************************************************************/
static RvInt32 RVCALLCONV GetModuleLogMaskFromCfg(IN  RvSipStackCfg    *pStackCfg,
                                              IN  RvSipStackModule  eLogModule)

{
    switch(eLogModule)
    {
    case RVSIP_CALL:
        return pStackCfg->callLogFilters;
    case RVSIP_TRANSACTION:
        return pStackCfg->transactionLogFilters;
    case RVSIP_MESSAGE:
        return pStackCfg->msgLogFilters;
    case RVSIP_TRANSPORT:
        return pStackCfg->transportLogFilters;
    case RVSIP_PARSER:
        return pStackCfg->parserLogFilters;
    case RVSIP_STACK:
        return pStackCfg->stackLogFilters;
    case RVSIP_MSGBUILDER:
        return pStackCfg->msgBuilderLogFilters;
#ifdef RV_SIP_AUTH_ON
    case RVSIP_AUTHENTICATOR:
        return pStackCfg->authenticatorLogFilters;
#endif /* #ifdef RV_SIP_AUTH_ON */
    case RVSIP_TRANSMITTER:
        return pStackCfg->transmitterLogFilters;
    case RVSIP_RESOLVER:
        return pStackCfg->resolverLogFilters;
	case RVSIP_TRANSCCLIENT:
		return pStackCfg->transcClientLogFilters;
    case RVSIP_REGCLIENT:
        return pStackCfg->regClientLogFilters;
	case RVSIP_PUBCLIENT:
        return pStackCfg->pubClientLogFilters;
#ifdef RV_SIP_SUBS_ON
    case RVSIP_SUBSCRIPTION:
        return pStackCfg->subscriptionLogFilters;
#endif /* #ifdef RV_SIP_SUBS_ON */
    case RVSIP_COMPARTMENT:
        return pStackCfg->compartmentLogFilters;
#ifdef RV_SIP_IMS_ON
    case RVSIP_SEC_AGREE:
        return pStackCfg->secAgreeLogFilters;
    case RVSIP_SECURITY:
        return pStackCfg->securityLogFilters;
#endif /* #ifdef RV_SIP_IMS_ON */
    case RVSIP_ADS_RLIST:
        return pStackCfg->adsFiltersCfg.adsRListLogFilters;
    case RVSIP_ADS_RA:
        return pStackCfg->adsFiltersCfg.adsRaLogFilters;
    case RVSIP_ADS_RPOOL:
        return pStackCfg->adsFiltersCfg.adsRPoolLogFilters;
    case RVSIP_ADS_HASH:
        return pStackCfg->adsFiltersCfg.adsHashLogFilters;
    case RVSIP_ADS_PQUEUE:
        return pStackCfg->adsFiltersCfg.adsPQueueLogFilters;
    case RVSIP_CORE_SEMAPHORE:
        return pStackCfg->coreFiltersCfg.coreSemaphoreLogFilters;
    case RVSIP_CORE_MUTEX:
        return pStackCfg->coreFiltersCfg.coreMutexLogFilters;
    case RVSIP_CORE_LOCK:
        return pStackCfg->coreFiltersCfg.coreLockLogFilters;
    case RVSIP_CORE_MEMORY:
        return pStackCfg->coreFiltersCfg.coreMemoryLogFilters;
    case RVSIP_CORE_THREAD:
        return pStackCfg->coreFiltersCfg.coreThreadLogFilters;
    case RVSIP_CORE_QUEUE:
        return pStackCfg->coreFiltersCfg.coreQueueLogFilters;
    case RVSIP_CORE_TIMER:
        return pStackCfg->coreFiltersCfg.coreTimerLogFilters;
    case RVSIP_CORE_TIMESTAMP:
        return pStackCfg->coreFiltersCfg.coreTimestampLogFilters;
    case RVSIP_CORE_CLOCK:
        return pStackCfg->coreFiltersCfg.coreClockLogFilters;
    case RVSIP_CORE_TM:
        return pStackCfg->coreFiltersCfg.coreTmLogFilters;
    case RVSIP_CORE_SOCKET:
        return pStackCfg->coreFiltersCfg.coreSocketLogFilters;
    case RVSIP_CORE_PORTRANGE:
        return pStackCfg->coreFiltersCfg.corePortRangeLogFilters;
    case RVSIP_CORE_SELECT:
        return pStackCfg->coreFiltersCfg.coreSelectLogFilters;
    case RVSIP_CORE_HOST:
        return pStackCfg->coreFiltersCfg.coreHostLogFilters;
    case RVSIP_CORE_TLS:
        return pStackCfg->coreFiltersCfg.coreTlsLogFilters;
    case RVSIP_CORE_ARES:
        return pStackCfg->coreFiltersCfg.coreAresLogFilters;
    case RVSIP_CORE_RCACHE:
        return pStackCfg->coreFiltersCfg.coreRcacheLogFilters;
    case RVSIP_CORE_EHD:
        return pStackCfg->coreFiltersCfg.coreEhdLogFilters;
    case RVSIP_CORE_IMSIPSEC:
        return pStackCfg->coreFiltersCfg.coreImsipsecLogFilters;
    default:
        return 0;
    }

}


/************************************************************************************
 * ConstructLogSource
 * ----------------------------------------------------------------------------------
 * General: allocate and construct a log source in a specific index in the log source
 *          array. The index is represented by the log source enumeration.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackMgr     - pointer to the struct that holds the stack information
 * Output:  none
 ***********************************************************************************/
static RvStatus RVCALLCONV ConstructLogSource(IN  StackMgr         *pStackMgr,
                                              IN  RvSipStackModule  eLogModule)
{
    RvStatus crv = RV_OK;

    if(eLogModule >= STACK_NUM_OF_LOG_SOURCE_MODULES)
    {
        return RV_ERROR_BADPARAM;
    }

    crv = RvMemoryAlloc(NULL,sizeof(RvLogSource),NULL,
                       (void*)&(pStackMgr->pLogSourceArrey[eLogModule]));
    if(crv != RV_OK)
    {
        pStackMgr->pLogSourceArrey[eLogModule] = NULL;
        return CRV2RV(crv);
    }
    crv = RvLogSourceConstruct(pStackMgr->pLogMgr,pStackMgr->pLogSourceArrey[eLogModule],
          GetModuleName(eLogModule), GetModuleName(eLogModule));
    if(crv != RV_OK)
    {
        RvMemoryFree(pStackMgr->pLogSourceArrey[eLogModule],pStackMgr->pLogMgr);
        return CRV2RV(crv);
    }
    return RV_OK;
}

/************************************************************************************
 * DestructLogSource
 * ----------------------------------------------------------------------------------
 * General: free and destruct a log source in a specific index in the log source
 *          array. The index is represented by the log source enumeration.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackMgr     - pointer to the struct that holds the stack information
 * Output:  none
 ***********************************************************************************/
static void RVCALLCONV DestructLogSource(IN  StackMgr        *pStackMgr,
                                         IN  RvSipStackModule  eLogModule)
{
    if(eLogModule >= STACK_NUM_OF_LOG_SOURCE_MODULES)
    {
        return;
    }
    if(pStackMgr->pLogSourceArrey[eLogModule] == NULL)
    {
        return;
    }
    RvLogSourceDestruct(pStackMgr->pLogSourceArrey[eLogModule]);
    RvMemoryFree(pStackMgr->pLogSourceArrey[eLogModule],pStackMgr->pLogMgr);
    pStackMgr->pLogSourceArrey[eLogModule] = NULL;

}



/************************************************************************************
 * DestructLogListeners
 * ----------------------------------------------------------------------------------
 * General: Destructs the default listener or unregister the stack listener.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackMgr     - pointer to the stack mgr
 * Output:  none
 ***********************************************************************************/
static void RVCALLCONV DestructLogListeners(IN  StackMgr        *pStackMgr)
{
    if(pStackMgr->bLogFileListenerEnabled == RV_TRUE)
    {
        RvLogListenerDestructLogfile(&pStackMgr->logListener);
    }
    else if(pStackMgr->bTerminalListenerEnabled == RV_TRUE)
    {
        RvLogListenerDestructTerminal(&pStackMgr->logListener);
    }
    else
    {
        RvLogUnregisterListener(pStackMgr->pLogMgr,
                                LogPrintCallbackFunction);
    }
}

#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/************************************************************************************
 * StackAllocateMemoryPools
 * ----------------------------------------------------------------------------------
 * General: constructs the rpools for the stack (message pool, header pool,authentication
 *          pool and timer pool).
 * Return Value: RvStatus - RV_OK, RV_ERROR_OUTOFRESOURCES
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg      - the structure that holds the configuration parameters.
 *          pStackMgr     - pointer to the struct that holds the stack information.
 * Output:  none
 ***********************************************************************************/
static RvStatus RVCALLCONV StackAllocateMemoryPools(
                                          IN    RvSipStackCfg        *pStackCfg,
                                          INOUT StackMgr         *pStackMgr)


{

    pStackMgr->hMessagePool = RPOOL_CoreConstruct(pStackCfg->messagePoolPageSize,
                                               pStackCfg->messagePoolNumofPages,
                                               pStackMgr->pLogMgr,
                                               RV_TRUE,
                                               "MessagePool");
    if (pStackMgr->hMessagePool == NULL)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                  "StackAllocateMemoryPools - The message pool couldn't be constructed"));
        return RV_ERROR_OUTOFRESOURCES;
    }

    pStackMgr->hGeneralPool  = RPOOL_CoreConstruct(pStackCfg->generalPoolPageSize,
                                                pStackCfg->generalPoolNumofPages,
                                                pStackMgr->pLogMgr,
                                                RV_TRUE,
                                                "GeneralPool");
    if (pStackMgr->hGeneralPool == NULL)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                   "StackAllocateMemoryPools - The general pool couldn't be constructed"));
        return RV_ERROR_OUTOFRESOURCES;
    }

    if(pStackCfg->elementPoolNumofPages > 0)
    {
        pStackMgr->hElementPool  = RPOOL_CoreConstruct(pStackCfg->elementPoolPageSize,
                                                   pStackCfg->elementPoolNumofPages,
                                                   pStackMgr->pLogMgr,
                                                   RV_TRUE,
                                                   "ElementPool");

        if (pStackMgr->hElementPool == NULL)
        {
            RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                      "StackAllocateMemoryPools - The element pool couldn't be constructed"));
            return RV_ERROR_OUTOFRESOURCES;
        }

    }

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
              "StackAllocateMemoryPools - Memory pools were allocated successfully"));


    return RV_OK;
}


/************************************************************************************
 * StackConstructModules
 * ----------------------------------------------------------------------------------
 * General: construct the stack modules by invoking the construct api of each module
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg      - the structure that holds the configuration parameters.
 *                           if the ipAddress field of the structure is "0.0.0.0"
 *                           then we use transport layer api to get the local address.
 *          pStackMgr     - pointer to the struct that holds the stack information
 * Output:  none
 ***********************************************************************************/
static RvStatus RVCALLCONV StackConstructModules(IN    RvSipStackCfg   *pStackCfg,
                                                 INOUT StackMgr        *pStackMgr)
{
    RvStatus rv;
#ifndef RV_SIP_LIGHT
	RvBool               b100RelSupported   = RV_FALSE;
#endif

#ifndef RV_SIP_PRIMITIVES
    RvInt32              i       = 0;
    RvBool               b100relInString    = RV_FALSE;
#ifdef RV_SIP_IMS_ON
	RvBool               bSecAgreeSupported = RV_FALSE;
#endif /* RV_SIP_IMS_ON */
    
    for(;i<pStackMgr->extensionListSize;i++)
    {
        if (strcmp(pStackMgr->supportedExtensionList[i],"100rel")==0)
        {
            b100relInString = RV_TRUE;
        }
#ifdef RV_SIP_IMS_ON
		if (strcmp(pStackMgr->supportedExtensionList[i],"sec-agree")==0) 
		{
			bSecAgreeSupported = RV_TRUE;
		}
 #endif /* RV_SIP_IMS_ON */
   }
    if(b100relInString == RV_TRUE)
    {
        b100RelSupported = pStackCfg->manualPrack;
    }
    else
    {
        b100RelSupported = RV_FALSE;
    }
#endif /* RV_SIP_PRIMITIVES */
    
    /*This parameter is passed to all layer*/
#ifdef RV_SIGCOMP_ON

    /*---------------------------------
     constructing the SigComp module
    -----------------------------------*/
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    printf_meminfo("StackConstructModules, SIGCOMP_COMPARTMENT_MODULE START");
#endif
/* SPIRENT_END */	
    rv = StackConstructCompartmentModule(pStackCfg, pStackMgr);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructModules - The compartment module couldn't be constructed, rv=%d",rv));
        pStackMgr->hCompartmentMgr   = NULL;
        return rv;
    }
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
	printf_meminfo("StackConstructModules, SIGCOMP_COMPARTMENT_MODULE END");
#endif
/* SPIRENT_END */
#endif /* RV_SIGCOMP_ON */

    /*---------------------------------
     constructing the parser module
    -----------------------------------*/
    rv = StackConstructParserModule(pStackMgr);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructModules - The parser module couldn't be constructed, rv=%d",rv));
        pStackMgr->hParserMgr = NULL;
        return rv;
    }

    /*---------------------------------
     constructing the message module
    -----------------------------------*/
    rv = StackConstructMessageModule(pStackMgr);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructModules - The Transport module couldn't be constructed, rv=%d",rv));
        pStackMgr->hMsgMgr = NULL;
        return rv;
    }

    /*-----------------------------------
     constructing the transport module
    --------------------------------------*/
    rv = StackConstructTransportModule(pStackCfg,pStackMgr);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructModules - The Transport module couldn't be constructed, rv=%d",rv));
        pStackMgr->hTransport = NULL;
        return rv;
    }

    /* set the local ip address */
    rv = ConvertZeroAddrToRealAddr(pStackCfg, pStackMgr);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructModules - Failed to convert addresses rv=%d",rv));
        return rv;
    }
    
    rv = StackConstructResolverModule(pStackCfg,pStackMgr);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructModules - The resolver module couldn't be constructed, rv=%d",rv));
        pStackMgr->hRslvMgr = NULL;
        return rv;
    }

#ifdef RV_SIP_IMS_ON
    /*-----------------------------------
     constructing the Security module
     !!! Should be done before CallLeg, RegClient, Transaction and Transmitter
     modules, but after Transport and Message modules !!!
    --------------------------------------*/
    if(pStackCfg->maxSecurityObjects > 0)
    {
        rv = StackConstructSecurityModule(pStackCfg, pStackMgr);
        if (rv != RV_OK)
        {
            RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                "StackConstructModules(pStackMgr=%p) - Failed to construct Security Module",
                pStackMgr));
            pStackMgr->hSecMgr = NULL;
            return rv;
        }
    }
    else
    {
        RvLogWarning(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructModules(pStackMgr=%p) - Security Module is not constructed: maxSecurityObjects <= 0",
            pStackMgr));
    }
#endif /* #ifdef RV_SIP_IMS_ON */

#if defined(RV_SIP_AUTH_ON) && !defined(RV_SIP_JSR32_SUPPORT)
    /*-----------------------------------
     constructing the authenticator module
    --------------------------------------*/
    /* constructing the authentication module */
    rv = SipAuthenticatorConstruct(pStackMgr->hMsgMgr,
                                       pStackMgr->pLogMgr,
                                       pStackMgr->pLogSourceArrey[RVSIP_AUTHENTICATOR],
                                       pStackMgr->hGeneralPool,
                                       pStackMgr->hElementPool,
                                       pStackMgr,
                                       &(pStackMgr->hAuthentication));
    if (rv != RV_OK)
    {
         RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                   "StackConstructModules - The authentication module couldn't be constructed"));
         pStackMgr->hAuthentication = NULL;
        return rv;
    }
#endif /*defined(RV_SIP_AUTH_ON) && !defined(RV_SIP_JSR32_SUPPORT)*/

    /*-----------------------------------
     constructing the transmitter module
    --------------------------------------*/
    rv = StackConstructTrxModule(pStackCfg,pStackMgr);
    if(rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructModules - The Transmitter module couldn't be constructed"));
        pStackMgr->hTrxMgr = NULL;
        return rv;
    }

    /*-----------------------------------
     constructing the transaction module
    --------------------------------------*/
#if !defined(RV_SIP_LIGHT) && !defined(RV_SIP_JSR32_SUPPORT)
    rv = StackConstructTranscModule(pStackCfg,pStackMgr,b100RelSupported);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructModules - The transaction module couldn't be constructed"));
        pStackMgr->hTranscMgr = NULL;
        return rv;
    }
#endif /* !defined(RV_SIP_LIGHT) && !defined(RV_SIP_JSR32_SUPPORT) */
    
#if !defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_JSR32_SUPPORT)
    /*----------------------------
     constructing the call module
    ------------------------------*/
    rv = StackConstructCallLegModule(pStackCfg,pStackMgr,b100RelSupported);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructModules - The call-leg module couldn't be constructed"));
        pStackMgr->hCallLegMgr = NULL;
        return rv;
    }
    

	/*---------------------------------
     constructing the transcClient module
    -----------------------------------*/
    rv = StackConstructTranscClientModule(pStackCfg,pStackMgr);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructModules - The transc-client module couldn't be constructed"));
        pStackMgr->hTranscClientMgr = NULL;
        return rv;
    }

    /*---------------------------------
     constructing the regClient module
    -----------------------------------*/
    rv = StackConstructRegClientModule(pStackCfg,pStackMgr);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructModules - The reg client module couldn't be constructed"));
        pStackMgr->hRegClientMgr = NULL;
        return rv;
    }

	/*---------------------------------
     constructing the pubClient module
    -----------------------------------*/
    rv = StackConstructPubClientModule(pStackCfg,pStackMgr);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructModules - The pub client module couldn't be constructed"));
        pStackMgr->hPubClientMgr = NULL;
        return rv;
    }




#ifdef RV_SIP_SUBS_ON
    /*---------------------------------
     constructing the subscription module
    -----------------------------------*/
    if(pStackCfg->maxSubscriptions > 0)
    {
        rv = StackConstructSubsModule(pStackCfg,pStackMgr);
        if (rv != RV_OK)
        {
            RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                "StackConstructModules - The subscription module couldn't be constructed"));
            pStackMgr->hSubsMgr = NULL;
            return rv;
        }
    }
#endif /* #ifdef RV_SIP_SUBS_ON */

#ifdef RV_SIP_IMS_ON
    /*-----------------------------------------
     constructing the security-agreement module
    -------------------------------------------*/
	if(pStackCfg->maxSecAgrees > 0 && bSecAgreeSupported == RV_TRUE)
	{
		rv = StackConstructSecAgreeModule(pStackCfg,pStackMgr);
		if (rv != RV_OK)
		{
			RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
					  "StackConstructModules - The security-agreement module couldn't be constructed"));
			pStackMgr->hSecAgreeMgr = NULL;
			return rv;
		}
	}
#endif /* #ifdef RV_SIP_IMS_ON */

#endif /* !defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_JSR32_SUPPORT) */

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
              "StackConstructModules - All modules were constructed successfully"));

    return RV_OK;
}

#ifdef RV_SIGCOMP_ON
/************************************************************************************
 * StackConstructCompartmentModule
 * ----------------------------------------------------------------------------------
 * General: construct the stack compartment module
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg      - the structure that holds the configuration parameters.
 *                           if the ipAddress field of the structure is "0.0.0.0"
 *                           then we use transport layer api to get the local address.
 *          pStackMgr     - pointer to the struct that holds the stack information
 * Output: none
 ***********************************************************************************/
static RvStatus RVCALLCONV StackConstructCompartmentModule(INOUT RvSipStackCfg*   pStackCfg,
                                                           INOUT StackMgr*        pStackMgr)
{
    SipCompartmentMgrCfg compartmentMgrCfg;
    RvStatus             rv     = RV_OK;
    memset(&compartmentMgrCfg,0,sizeof(SipCompartmentMgrCfg));
    compartmentMgrCfg.pLogSrc              = pStackMgr->pLogSourceArrey[RVSIP_COMPARTMENT];
    compartmentMgrCfg.maxNumOfCompartments = pStackCfg->maxCompartments;
    compartmentMgrCfg.pLogMgr              = pStackMgr->pLogMgr;
	compartmentMgrCfg.pSeed                = &(pStackMgr->seed);

    rv = SipCompartmentMgrConstruct(&compartmentMgrCfg,(void*)pStackMgr,
                                    &pStackMgr->hCompartmentMgr);

    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructCompartmentModule - The Compartment module couldn't be constructed"));
        return rv;
    }
    
    SipCompartmentMgrResetMaxUsageResourcesStatus(pStackMgr->hCompartmentMgr);
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "StackConstructCompartmentModule - compartment module constructed"));
    return rv;
}
#endif /*#ifdef RV_SIGCOMP_ON*/

/************************************************************************************
 * StackConstructParserModule
 * ----------------------------------------------------------------------------------
 * General: construct the stack parser module
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg      - the structure that holds the configuration parameters.
 *                           if the ipAddress field of the structure is "0.0.0.0"
 *                           then we use transport layer api to get the local address.
 *          pStackMgr     - pointer to the struct that holds the stack information
 * Output: none
 ***********************************************************************************/
static RvStatus RVCALLCONV StackConstructParserModule(INOUT StackMgr*        pStackMgr)
{
    SipParserMgrCfg parserMgrCfg;
    RvStatus        rv  = RV_OK;
    memset(&parserMgrCfg,0,sizeof(parserMgrCfg));

    parserMgrCfg.hGeneralPool = pStackMgr->hGeneralPool;
    parserMgrCfg.moduleLogId  = pStackMgr->pLogSourceArrey[RVSIP_PARSER];
    parserMgrCfg.pLogMgr      = pStackMgr->pLogMgr;
    rv = SipParserMgrConstruct(&parserMgrCfg,&(pStackMgr->hParserMgr));

    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructParserModule - The parser module couldn't be constructed"));
        return rv;
    }
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "StackConstructParserModule - parser module constructed"));
    return rv;
}

/************************************************************************************
 * StackConstructMessageModule
 * ----------------------------------------------------------------------------------
 * General: construct the stack massage module
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg      - the structure that holds the configuration parameters.
 *                           if the ipAddress field of the structure is "0.0.0.0"
 *                           then we use transport layer api to get the local address.
 *          pStackMgr     - pointer to the struct that holds the stack information
 * Output: none
 ***********************************************************************************/
static RvStatus RVCALLCONV StackConstructMessageModule(INOUT StackMgr*        pStackMgr)
{
    SipMessageMgrCfg    msgMgrCfg;
    RvStatus            rv  = RV_OK;
    memset(&msgMgrCfg,0,sizeof(msgMgrCfg));

    msgMgrCfg.pLogMgr    = pStackMgr->pLogMgr;
    msgMgrCfg.logId      = pStackMgr->pLogSourceArrey[RVSIP_MESSAGE];;
    msgMgrCfg.seed       = &(pStackMgr->seed);
    msgMgrCfg.hParserMgr = pStackMgr->hParserMgr;
    rv = SipMessageMgrConstruct(&msgMgrCfg,&(pStackMgr->hMsgMgr));
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructMessageModule - The message module couldn't be constructed"));
        return rv;
    }

    SipParserMgrSetMsgMgrHandle(pStackMgr->hParserMgr, pStackMgr->hMsgMgr);
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "StackConstructMessageModule - message module constructed"));
    return rv;
}

/************************************************************************************
 * StackConstructTrxModule
 * ----------------------------------------------------------------------------------
 * General: construct the stack transaction module
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg      - the structure that holds the configuration parameters.
 *                           if the ipAddress field of the structure is "0.0.0.0"
 *                           then we use transport layer api to get the local address.
 *          pStackMgr     - pointer to the struct that holds the stack information
 * Output: none
 ***********************************************************************************/
static RvStatus RVCALLCONV StackConstructTransportModule(INOUT RvSipStackCfg*   pStackCfg,
                                                         INOUT StackMgr*        pStackMgr)
{
    SipTransportCfg transportCfg;
    RvStatus        rv      = RV_OK;

    memset(&transportCfg,0,sizeof(transportCfg));

    transportCfg.pLogMgr         = pStackMgr->pLogMgr;
    transportCfg.regId              = pStackMgr->pLogSourceArrey[RVSIP_TRANSPORT];
    transportCfg.pMBLogSrc          = pStackMgr->pLogSourceArrey[RVSIP_MSGBUILDER];
    transportCfg.hMsgMgr            = pStackMgr->hMsgMgr;
    transportCfg.hMsgPool           = pStackMgr->hMessagePool;
    transportCfg.hGeneralPool       = pStackMgr->hGeneralPool;
    transportCfg.hElementPool        = pStackMgr->hElementPool;
    transportCfg.maxBufSize         = pStackCfg->sendReceiveBufferSize;
    strcpy(transportCfg.udpAddress, pStackCfg->localUdpAddress);
    transportCfg.udpPort            = pStackCfg->localUdpPort;
    transportCfg.seed               = &(pStackMgr->seed);
    transportCfg.hStack             = (void*)pStackMgr;
    transportCfg.maxConnOwners      = pStackCfg->maxTransactions+
                                      +pStackCfg->maxTransmitters +
#ifndef RV_SIP_PRIMITIVES
                                 pStackCfg->maxCallLegs +
                                 pStackCfg->maxPubClients + pStackCfg->maxRegClients +
#endif /* RV_SIP_PRIMITIVES */
                                 pStackCfg->maxConnections;
    transportCfg.maxConnections             = pStackCfg->maxConnections;
    transportCfg.ePersistencyLevel          = pStackCfg->ePersistencyLevel;
    transportCfg.serverConnectionTimeout    = pStackCfg->serverConnectionTimeout;
    transportCfg.maxNumOfLocalAddresses     = pStackCfg->maxNumOfLocalAddresses;
    transportCfg.numOfExtraTcpAddresses     = pStackCfg->numOfExtraTcpAddresses;
    transportCfg.numOfExtraUdpAddresses     = pStackCfg->numOfExtraUdpAddresses;
    transportCfg.localTcpAddresses          = pStackCfg->localTcpAddresses;
    transportCfg.localTcpPorts              = pStackCfg->localTcpPorts;
    transportCfg.localUdpAddresses          = pStackCfg->localUdpAddresses;
    transportCfg.localUdpPorts              = pStackCfg->localUdpPorts;
    strcpy(transportCfg.tcpAddress, pStackCfg->localTcpAddress);
    transportCfg.tcpPort                    = pStackCfg->localTcpPort;
    transportCfg.tcpEnabled                 = pStackCfg->tcpEnabled;
    transportCfg.connectionCapacityPercent  = pStackCfg->connectionCapacityPercent;
    transportCfg.bDLAEnabled                = pStackCfg->bDLAEnabled;
	transportCfg.bIgnoreLocalAddresses      = pStackCfg->bIgnoreLocalAddresses;
    transportCfg.maxPageNuber               = pStackCfg->messagePoolNumofPages;

    transportCfg.outboundProxyIp            = pStackCfg->outboundProxyIpAddress;
    if (pStackCfg->outboundProxyIpAddress[0] == '\0')
    {
        transportCfg.outboundProxyHostName  = pStackCfg->outboundProxyHostName;
    }
    transportCfg.outboundProxyPort          = pStackCfg->outboundProxyPort;
    transportCfg.eOutboundProxyTransport    = pStackCfg->eOutboundProxyTransport;
#ifdef RV_SIGCOMP_ON
    transportCfg.eOutboundProxyCompression  = pStackCfg->eOutboundProxyCompression;
#endif /* RV_SIGCOMP_ON */

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    transportCfg.maxElementsInSingleDnsList = pStackCfg->maxElementsInSingleDnsList;
#endif /* #ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT */

    transportCfg.processingTaskPriority     = pStackCfg->processingTaskPriority;
    transportCfg.processingTaskStackSize    = pStackCfg->processingTaskStackSize;

    transportCfg.numberOfProcessingThreads  = pStackCfg->numberOfProcessingThreads;
    transportCfg.processingQueueSize        = pStackCfg->processingQueueSize;
    transportCfg.numOfReadBuffers           = pStackCfg->numOfReadBuffers;

/* SPIRENT_BEGIN */
/*
 * COMMENTS: Assign the Signaling priorities to the SipTransportCfg object
*/
#if defined(UPDATED_BY_SPIRENT)
	transportCfg.localPriority             = pStackCfg->localPriority;
	transportCfg.localPriorities           = pStackCfg->localPriorities;
	transportCfg.isImsClient               = pStackCfg->isImsClient;
	strcpy(transportCfg.if_name, pStackCfg->if_name);
	transportCfg.vdevblock = pStackCfg->vdevblock;
	transportCfg.localVDevBlocks          = pStackCfg->localVDevBlocks;
	transportCfg.localIfNames          = pStackCfg->localIfNames;
#endif /* defined(UPDATED_BY_SPIRENT) */ 
    /* SPIRENT_END */

#if (RV_TLS_TYPE != RV_TLS_NONE)
    /* TLS */
    transportCfg.numOfTlsAddresses          = pStackCfg->numOfTlsAddresses;
    transportCfg.localTlsAddresses          = pStackCfg->localTlsAddresses;
    transportCfg.localTlsPorts              = pStackCfg->localTlsPorts;
    transportCfg.numOfTlsEngines            = pStackCfg->numOfTlsEngines;
    transportCfg.maxTlsSessions             = pStackCfg->maxTlsSessions;
#define TLS_HS_SAFTY_TIMER 300000 /* 5 minutes */
    transportCfg.TlsHandshakeTimeout        = TLS_HS_SAFTY_TIMER;
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

    transportCfg.maxTimers                 = CalculateMaxTimers(pStackCfg,pStackMgr);

#if (RV_NET_TYPE & RV_NET_SCTP)
    /* SCTP */
    transportCfg.sctpCfg.localSctpAddresses         = pStackCfg->localSctpAddresses;
    transportCfg.sctpCfg.localSctpPorts             = pStackCfg->localSctpPorts;
    transportCfg.sctpCfg.numOfSctpAddresses         = pStackCfg->numOfSctpAddresses;
    transportCfg.sctpCfg.numOfOutStreams            = pStackCfg->numOfOutStreams;
    transportCfg.sctpCfg.numOfInStreams             = pStackCfg->numOfInStreams;
    transportCfg.sctpCfg.avgNumOfMultihomingAddresses    = pStackCfg->avgNumOfMultihomingAddresses;
#endif /* (RV_NET_TYPE & RV_NET_SCTP)*/
    
    rv = SipTransportConstruct(&transportCfg, &pStackMgr->hTransport);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructTransportModule - The transport module couldn't be constructed"));
        return rv;
    }


    /* -------------------------------------------------
       set transport handle in message manager
       ------------------------------------------------- */
    SipMsgMgrSetTransportHandle(pStackMgr->hMsgMgr, pStackMgr->hTransport);

    pStackCfg->sendReceiveBufferSize    =transportCfg.maxBufSize;

    pStackCfg->maxConnections           = transportCfg.maxConnections;
    strcpy(pStackCfg->localTcpAddress, transportCfg.tcpAddress);
    pStackCfg->localTcpPort             = transportCfg.tcpPort;
    pStackCfg->localUdpPort             = transportCfg.udpPort;
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "StackConstructTransportModule - transport module constructed"));
    
    return rv;
}

/************************************************************************************
 * StackConstructTrxModule
 * ----------------------------------------------------------------------------------
 * General: construct the stack transaction module
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg      - the structure that holds the configuration parameters.
 *                           if the ipAddress field of the structure is "0.0.0.0"
 *                           then we use transport layer api to get the local address.
 *          pStackMgr     - pointer to the struct that holds the stack information
 * Output: none
 ***********************************************************************************/
static RvStatus RVCALLCONV StackConstructTrxModule(INOUT RvSipStackCfg*   pStackCfg,
                                                   INOUT StackMgr*        pStackMgr)
{
    SipTransmitterMgrCfg    trxMgrCfg;
    RvStatus                rv          = RV_OK;

    memset(&trxMgrCfg,0,sizeof(SipTransmitterMgrCfg));
    trxMgrCfg.bIsPersistent       = (pStackCfg->ePersistencyLevel !=
                                     RVSIP_TRANSPORT_PERSISTENCY_LEVEL_UNDEFINED)?
                                     RV_TRUE:RV_FALSE;
    trxMgrCfg.bUseRportParamInVia = pStackCfg->bUseRportParamInVia;
#ifdef RV_SIGCOMP_ON
    trxMgrCfg.hCompartmentMgr = pStackMgr->hCompartmentMgr;
#endif /* RV_SIGCOMP_ON */
    trxMgrCfg.hMsgMgr         = pStackMgr->hMsgMgr;
    trxMgrCfg.hTransportMgr   = pStackMgr->hTransport;
#ifdef RV_SIP_IMS_ON
    trxMgrCfg.hSecMgr         = pStackMgr->hSecMgr;
#endif /* RV_SIP_IMS_ON */
    trxMgrCfg.maxNumOfTrx     = pStackCfg->maxTransmitters;
    trxMgrCfg.pLogMgr         = pStackMgr->pLogMgr;
    trxMgrCfg.pLogSrc         = pStackMgr->pLogSourceArrey[RVSIP_TRANSMITTER];
    trxMgrCfg.seed            = &pStackMgr->seed;
    trxMgrCfg.hGeneralPool    = pStackMgr->hGeneralPool;
    trxMgrCfg.hMessagePool    = pStackMgr->hMessagePool;
    trxMgrCfg.hElementPool    = pStackMgr->hElementPool;
    trxMgrCfg.maxMessages     = pStackCfg->messagePoolNumofPages;
    trxMgrCfg.hRslvMgr        = pStackMgr->hRslvMgr;
    trxMgrCfg.bResolveTelUrls = pStackCfg->bResolveTelUrls;
    
    rv = SipTransmitterMgrConstruct(&trxMgrCfg,(void*)pStackMgr,&pStackMgr->hTrxMgr);
    if(rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                   "StackConstructTrxModule - The Transmitter module couldn't be constructed"));
        return rv;
    }
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "StackConstructTrxModule - resolver module constructed"));
    return rv;
}

#if !defined(RV_SIP_LIGHT) && !defined(RV_SIP_JSR32_SUPPORT)
/************************************************************************************
 * StackConstructTranscModule
 * ----------------------------------------------------------------------------------
 * General: construct the stack transaction module
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg      - the structure that holds the configuration parameters.
 *                           if the ipAddress field of the structure is "0.0.0.0"
 *                           then we use transport layer api to get the local address.
 *          pStackMgr     - pointer to the struct that holds the stack information
 * Output: none
 ***********************************************************************************/
static RvStatus RVCALLCONV StackConstructTranscModule(INOUT RvSipStackCfg*   pStackCfg,
                                                      INOUT StackMgr*        pStackMgr,
                                                      IN    RvBool           b100rel)
{
    SipTranscMgrConfigParams    transcConfig;
    RvStatus                    rv              = RV_OK;

    memset(&transcConfig,0, sizeof(SipTranscMgrConfigParams));
    /*use name only if IP was not supplied*/
    transcConfig.pLogMgr                               = pStackMgr->pLogMgr;
    transcConfig.hStack                                = (void*)pStackMgr;
#ifdef RV_SIP_IMS_ON
    transcConfig.hSecMgr                               = pStackMgr->hSecMgr;
#endif /* RV_SIP_IMS_ON */
    transcConfig.maxTransactionNumber                  = pStackCfg->maxTransactions;
    transcConfig.T1                                    = pStackCfg->retransmissionT1;
    transcConfig.T2                                    = pStackCfg->retransmissionT2;
    transcConfig.T4                                    = pStackCfg->retransmissionT4;
    transcConfig.genLinger                             = pStackCfg->generalLingerTimer;
    transcConfig.inviteLinger                          = pStackCfg->inviteLingerTimer;
    transcConfig.cancelGeneralNoResponseTimer          = pStackCfg->cancelGeneralNoResponseTimer;
    transcConfig.cancelInviteNoResponseTimer           = pStackCfg->cancelInviteNoResponseTimer;
    transcConfig.generalRequestTimeoutTimer            = pStackCfg->generalRequestTimeoutTimer;
    transcConfig.seed                                  = &(pStackMgr->seed);
    transcConfig.enableInviteProceedingTimeoutState    = pStackCfg->enableInviteProceedingTimeoutState;

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    /* copy the configured retransmit "INVITE" messages */
    transcConfig.maxRetransmitINVITENumber             = pStackCfg->maxRetransmitINVITENumber;
    transcConfig.initialGeneralRequestTimeout          = pStackCfg->initialGeneralRequestTimeout;
#endif
#if defined(UPDATED_BY_SPIRENT)
#if defined(UPDATED_BY_SPIRENT_ABACUS)    
    transcConfig.pfnRetransmitEvHandler                = pStackCfg->pfnRetransmitEvHandler;
#else
    transcConfig.pfnRetransmitEvHandler                = pStackCfg->pfnRetransmitEvHandler;
    transcConfig.pfnRetransmitRcvdEvHandler            = pStackCfg->pfnRetransmitRcvdEvHandler;
#endif    
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

#ifndef RV_SIP_PRIMITIVES
    transcConfig.rejectUnsupportedExtensions = pStackCfg->rejectUnsupportedExtensions;
    transcConfig.addSupportedListToMsg       = pStackCfg->addSupportedListToMsg;
#ifdef RV_SIP_SUBS_ON
    transcConfig.maxSubscriptions = pStackCfg->maxSubscriptions;
#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /* RV_SIP_PRIMITIVES */
#ifdef RV_SIP_AUTH_ON
    transcConfig.hAuthMgr           = pStackMgr->hAuthentication;
    transcConfig.enableServerAuth   = pStackCfg->enableServerAuth;
#endif /* #ifdef RV_SIP_AUTH_ON */
    transcConfig.provisional       = pStackCfg->provisionalTimer;
    transcConfig.hMessagePool      = pStackMgr->hMessagePool;
    transcConfig.hGeneralPool      = pStackMgr->hGeneralPool;
    transcConfig.pLogSrc           = pStackMgr->pLogSourceArrey[RVSIP_TRANSACTION];
    transcConfig.hTransportHandle  = pStackMgr->hTransport;
    transcConfig.hMsgMgr           = pStackMgr->hMsgMgr;
    transcConfig.maxPageNumber     = pStackCfg->messagePoolNumofPages;
    transcConfig.isProxy           = pStackCfg->isProxy;
    transcConfig.proxy2xxSentTimer = pStackCfg->proxy2xxSentTimer;
    transcConfig.proxy2xxRcvdTimer = pStackCfg->proxy2xxRcvdTimer;
    transcConfig.hTrxMgr           = pStackMgr->hTrxMgr;
    transcConfig.bDisableMerging   = pStackCfg->bDisableMerging;
    transcConfig.bEnableForking    = pStackCfg->bEnableForking;
#ifndef RV_SIP_PRIMITIVES
    transcConfig.supportedExtensionList = pStackMgr->supportedExtensionList;
    transcConfig.extensionListSize      = pStackMgr->extensionListSize;

    transcConfig.manualPrack = b100rel;
#endif /*#ifndef RV_SIP_PRIMITIVES*/
    transcConfig.manualBehavior = pStackCfg->manualBehavior;
    transcConfig.bIsPersistent = (pStackCfg->ePersistencyLevel ==
                                  RVSIP_TRANSPORT_PERSISTENCY_LEVEL_UNDEFINED)?RV_FALSE:RV_TRUE;
    transcConfig.bDynamicInviteHandling = pStackCfg->bDynamicInviteHandling;
    transcConfig.bOldInviteHandling = pStackCfg->bOldInviteHandling;
#ifdef RV_SIGCOMP_ON
    transcConfig.sigCompTcpTimer = pStackCfg->sigCompTcpTimer;
    transcConfig.hCompartmentMgr = pStackMgr->hCompartmentMgr;
#endif /* RV_SIGCOMP_ON */
    rv = SipTransactionMgrConstruct(&transcConfig,&(pStackMgr->hTranscMgr));
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructTranscModule - The transaction module couldn't be constructed"));
        return rv;
    }

    pStackCfg->retransmissionT1             = transcConfig.T1;
    pStackCfg->retransmissionT2             = transcConfig.T2;
    pStackCfg->retransmissionT4             = transcConfig.T4;
    pStackCfg->generalLingerTimer           = transcConfig.genLinger;
    pStackCfg->inviteLingerTimer            = transcConfig.inviteLinger;
    pStackCfg->provisionalTimer             = transcConfig.provisional;
    pStackCfg->generalRequestTimeoutTimer   = transcConfig.generalRequestTimeoutTimer;
    pStackCfg->cancelGeneralNoResponseTimer = transcConfig.cancelGeneralNoResponseTimer;
    pStackCfg->cancelInviteNoResponseTimer  = transcConfig.cancelInviteNoResponseTimer;
    transcConfig.proxy2xxSentTimer          = pStackCfg->proxy2xxSentTimer;
    transcConfig.proxy2xxRcvdTimer          = pStackCfg->proxy2xxRcvdTimer;

    SipTransactionMgrResetMaxUsageResourcesStatus(pStackMgr->hTranscMgr);
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "StackConstructTranscModule - transaction module constructed"));

#ifdef RV_SIP_PRIMITIVES
    RV_UNUSED_ARG(b100rel)
#endif
        
    return rv;
}
#endif /* #if !defined(RV_SIP_LIGHT) && !defined(RV_SIP_JSR32_SUPPORT) */

#if !defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_JSR32_SUPPORT)
/************************************************************************************
 * StackConstructCallLegModule
 * ----------------------------------------------------------------------------------
 * General: construct the stack call leg module
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg      - the structure that holds the configuration parameters.
 *                           if the ipAddress field of the structure is "0.0.0.0"
 *                           then we use transport layer api to get the local address.
 *          pStackMgr     - pointer to the struct that holds the stack information
 * Output: none
 ***********************************************************************************/
static RvStatus RVCALLCONV StackConstructCallLegModule(INOUT RvSipStackCfg*   pStackCfg,
                                                       INOUT StackMgr*        pStackMgr,
                                                       IN RvBool            b100rel)
{
    RvStatus            rv = RV_OK;
    SipCallLegMgrCfg    callLegMgrCfg;

    memset(&callLegMgrCfg,0,sizeof(SipCallLegMgrCfg));
    callLegMgrCfg.pLogMgr                      = pStackMgr->pLogMgr;
    callLegMgrCfg.moduleLogId                  = pStackMgr->pLogSourceArrey[RVSIP_CALL];
    callLegMgrCfg.hTransport                   = pStackMgr->hTransport;
    callLegMgrCfg.hTranscMgr                   = pStackMgr->hTranscMgr;
    callLegMgrCfg.hMsgMgr                      = pStackMgr->hMsgMgr;
    callLegMgrCfg.hTrxMgr                      = pStackMgr->hTrxMgr;
#ifdef RV_SIP_IMS_ON
    callLegMgrCfg.hSecMgr                      = pStackMgr->hSecMgr;
#endif /* RV_SIP_IMS_ON */

	callLegMgrCfg.hGeneralPool                 = pStackMgr->hGeneralPool;
    callLegMgrCfg.hMessagePool                 = pStackMgr->hMessagePool;
    callLegMgrCfg.hElementPool                 = pStackMgr->hElementPool;
    callLegMgrCfg.maxNumOfCalls                = pStackCfg->maxCallLegs;
    callLegMgrCfg.maxNumOfTransc               = pStackCfg->maxTransactions;
    callLegMgrCfg.seed                         = &(pStackMgr->seed);
    callLegMgrCfg.enableInviteProceedingTimeoutState = pStackCfg->enableInviteProceedingTimeoutState;
    callLegMgrCfg.manualAckOn2xx               = pStackCfg->manualAckOn2xx;
#ifdef RV_SIP_AUTH_ON
    callLegMgrCfg.hAuthMgr                     = pStackMgr->hAuthentication;
#endif /* #ifdef RV_SIP_AUTH_ON */
    callLegMgrCfg.manualPrack                  = b100rel;
#ifdef RV_SIP_SUBS_ON
    callLegMgrCfg.maxSubscriptions             = pStackCfg->maxSubscriptions;
    callLegMgrCfg.bDisableRefer3515Behavior    = pStackCfg->bDisableRefer3515Behavior;
    callLegMgrCfg.bEnableSubsForking           = pStackCfg->bEnableSubsForking;
#endif /* #ifdef RV_SIP_SUBS_ON */
    /* Session Timer parameters */
    callLegMgrCfg.supportedExtensionList       = pStackMgr->supportedExtensionList;
    callLegMgrCfg.extensionListSize            = pStackMgr->extensionListSize;
    callLegMgrCfg.addSupportedListToMsg        = pStackCfg->addSupportedListToMsg;
    callLegMgrCfg.manualSessionTimer           = pStackCfg->manualSessionTimer;
    callLegMgrCfg.MinSE                        = pStackCfg->minSE;
    callLegMgrCfg.sessionExpires               = pStackCfg->sessionExpires;
    callLegMgrCfg.manualBehavior               = pStackCfg->manualBehavior;
    callLegMgrCfg.bIsPersistent = (pStackCfg->ePersistencyLevel ==
                                  RVSIP_TRANSPORT_PERSISTENCY_LEVEL_TRANSC_USER)?RV_TRUE:RV_FALSE;
    callLegMgrCfg.bEnableForking               = pStackCfg->bEnableForking;
    callLegMgrCfg.forkedAckTrxTimeout          = pStackCfg->forkedAckTrxTimeout;

    if(pStackCfg->forked1xxTimerTimeout >= 0)
    {
        callLegMgrCfg.forked1xxTimerTimeout = pStackCfg->forked1xxTimerTimeout;
    }
    else
    {
        if(pStackCfg->provisionalTimer <= 0)
        {
            callLegMgrCfg.forked1xxTimerTimeout = DEFAULT_PROVISIONAL_TIMER;
        }
        else
        {
            callLegMgrCfg.forked1xxTimerTimeout = pStackCfg->provisionalTimer;
        }
    }
    
    callLegMgrCfg.inviteLingerTimeout = pStackCfg->inviteLingerTimer;
    callLegMgrCfg.bOldInviteHandling = pStackCfg->bOldInviteHandling;

    rv = SipCallLegMgrConstruct(&callLegMgrCfg, (void*)pStackMgr,&(pStackMgr->hCallLegMgr));
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                  "StackConstructCallLegModule - The call module couldn't be constructed"));
        return rv;
    }

    SipCallLegMgrResetMaxUsageResourcesStatus(pStackMgr->hCallLegMgr);
    
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "StackConstructCallLegModule - call leg module constructed"));

    return rv;
}

/************************************************************************************
 * StackConstructTarnscClientModule
 * ----------------------------------------------------------------------------------
 * General: construct the stack transc-client module, which will be the platform for all
 *			transaction-clients such regClient and pubClient.
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg      - the structure that holds the configuration parameters.
 *                           if the ipAddress field of the structure is "0.0.0.0"
 *                           then we use transport layer api to get the local address.
 *          pStackMgr     - pointer to the struct that holds the stack information
 * Output: none
 ***********************************************************************************/
static RvStatus RVCALLCONV StackConstructTranscClientModule(INOUT RvSipStackCfg*   pStackCfg,
                                                         INOUT StackMgr*        pStackMgr)
{
    SipTranscClientMgrCfg  transcClientMgrCfg;
    RvStatus            rv = RV_OK;

    memset(&transcClientMgrCfg,0,sizeof(SipTranscClientMgrCfg));
    transcClientMgrCfg.pLogMgr     = pStackMgr->pLogMgr;
    transcClientMgrCfg.moduleLogId = pStackMgr->pLogSourceArrey[RVSIP_TRANSCCLIENT];
    transcClientMgrCfg.hTranscMgr  = pStackMgr->hTranscMgr;
    transcClientMgrCfg.hTransport  = pStackMgr->hTransport;
    transcClientMgrCfg.hMsgMgr     = pStackMgr->hMsgMgr;
#ifdef RV_SIP_IMS_ON
    transcClientMgrCfg.hSecMgr     = pStackMgr->hSecMgr;
	transcClientMgrCfg.hSecAgreeMgr= pStackMgr->hSecAgreeMgr;
#endif /* RV_SIP_IMS_ON */
    transcClientMgrCfg.hGeneralPool =  pStackMgr->hGeneralPool;
    transcClientMgrCfg.seed                    = &(pStackMgr->seed);
    transcClientMgrCfg.hMessagePool = pStackMgr->hMessagePool;
#ifdef RV_SIP_AUTH_ON
    transcClientMgrCfg.hAuthModule     = pStackMgr->hAuthentication;
#endif /* #ifdef RV_SIP_AUTH_ON */
    transcClientMgrCfg.bIsPersistent   = (pStackCfg->ePersistencyLevel ==
                                  RVSIP_TRANSPORT_PERSISTENCY_LEVEL_TRANSC_USER)?RV_TRUE:RV_FALSE;
	
    rv = SipTranscClientMgrConstruct(&transcClientMgrCfg, (void*)pStackMgr,
                                      &(pStackMgr->hTranscClientMgr));

    if (rv != RV_OK)
    {
         RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
             "StackConstructTranscClientModule - The transc-client module couldn't be constructed"));
        return rv;
    }
   
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "StackConstructTranscClientModule - transc-client module constructed"));
    return rv;
}


/************************************************************************************
 * StackConstructRegClientModule
 * ----------------------------------------------------------------------------------
 * General: construct the stack reg client module
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg      - the structure that holds the configuration parameters.
 *                           if the ipAddress field of the structure is "0.0.0.0"
 *                           then we use transport layer api to get the local address.
 *          pStackMgr     - pointer to the struct that holds the stack information
 * Output: none
 ***********************************************************************************/
static RvStatus RVCALLCONV StackConstructRegClientModule(INOUT RvSipStackCfg*   pStackCfg,
                                                         INOUT StackMgr*        pStackMgr)
{
    SipRegClientMgrCfg  regClientMgrCfg;
    RvStatus            rv = RV_OK;

    memset(&regClientMgrCfg,0,sizeof(SipRegClientMgrCfg));
    regClientMgrCfg.pLogMgr     = pStackMgr->pLogMgr;
    regClientMgrCfg.moduleLogId = pStackMgr->pLogSourceArrey[RVSIP_REGCLIENT];
    regClientMgrCfg.hGeneralPool =  pStackMgr->hGeneralPool;
    regClientMgrCfg.maxNumOfRegClients = pStackCfg->maxRegClients;
	regClientMgrCfg.alertTimeout     = pStackCfg->regAlertTimeout;
    regClientMgrCfg.seed                    = &(pStackMgr->seed);
    regClientMgrCfg.hMessagePool = pStackMgr->hMessagePool;
	regClientMgrCfg.hTranscMgr	 = pStackMgr->hTranscMgr;
	regClientMgrCfg.hTransport	 = pStackMgr->hTransport;
	regClientMgrCfg.hTranscClientMgr = pStackMgr->hTranscClientMgr;
	regClientMgrCfg.hMsgMgr			 = pStackMgr->hMsgMgr;

	rv = SipRegClientMgrConstruct(&regClientMgrCfg, (void*)pStackMgr, &(pStackMgr->hRegClientMgr));

    if (rv != RV_OK)
    {
         RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
             "StackConstructRegClientModule - The register-client module couldn't be constructed"));
        return rv;
    }
    pStackCfg->maxRegClients = regClientMgrCfg.maxNumOfRegClients;
    SipRegClientMgrResetMaxUsageResourcesStatus(pStackMgr->hRegClientMgr);
    
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "StackConstructRegClientModule - reg client module constructed"));
    return rv;
}
/************************************************************************************
 * StackConstructPubClientModule
 * ----------------------------------------------------------------------------------
 * General: construct the stack reg client module
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg      - the structure that holds the configuration parameters.
 *                           if the ipAddress field of the structure is "0.0.0.0"
 *                           then we use transport layer api to get the local address.
 *          pStackMgr     - pointer to the struct that holds the stack information
 * Output: none
 ***********************************************************************************/
static RvStatus RVCALLCONV StackConstructPubClientModule(INOUT RvSipStackCfg*   pStackCfg,
                                                         INOUT StackMgr*        pStackMgr)
{
    SipPubClientMgrCfg  pubClientMgrCfg;
    RvStatus            rv = RV_OK;

    memset(&pubClientMgrCfg,0,sizeof(SipPubClientMgrCfg));
    pubClientMgrCfg.pLogMgr     = pStackMgr->pLogMgr;
    pubClientMgrCfg.moduleLogId = pStackMgr->pLogSourceArrey[RVSIP_PUBCLIENT];
    pubClientMgrCfg.hGeneralPool =  pStackMgr->hGeneralPool;
    pubClientMgrCfg.maxNumOfPubClients = pStackCfg->maxPubClients;
	pubClientMgrCfg.alertTimeout     = pStackCfg->pubAlertTimeout;
    pubClientMgrCfg.seed                    = &(pStackMgr->seed);
    pubClientMgrCfg.hMessagePool = pStackMgr->hMessagePool;
	pubClientMgrCfg.hTranscMgr	 = pStackMgr->hTranscMgr;
	pubClientMgrCfg.hTransport	 = pStackMgr->hTransport;
	pubClientMgrCfg.hTranscClientMgr = pStackMgr->hTranscClientMgr;
	pubClientMgrCfg.hMsgMgr			 = pStackMgr->hMsgMgr;
	

    rv = SipPubClientMgrConstruct(&pubClientMgrCfg, (void*)pStackMgr, &(pStackMgr->hPubClientMgr));

    if (rv != RV_OK)
    {
         RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
             "StackConstructPubClientModule - The publish-client module couldn't be constructed"));
        return rv;
    }
    pStackCfg->maxPubClients = pubClientMgrCfg.maxNumOfPubClients;
    SipPubClientMgrResetMaxUsageResourcesStatus(pStackMgr->hPubClientMgr);
    
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "StackConstructPubClientModule - pub client module constructed"));
    return rv;
}

#ifdef RV_SIP_SUBS_ON
/************************************************************************************
 * StackConstructSubsModule
 * ----------------------------------------------------------------------------------
 * General: construct the stack subscription module
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg      - the structure that holds the configuration parameters.
 *                           if the ipAddress field of the structure is "0.0.0.0"
 *                           then we use transport layer api to get the local address.
 *          pStackMgr     - pointer to the struct that holds the stack information
 * Output: none
 ***********************************************************************************/
static RvStatus RVCALLCONV StackConstructSubsModule(INOUT RvSipStackCfg*   pStackCfg,
                                                    INOUT StackMgr*        pStackMgr)
{
    SipSubsMgrCfg   subsMgrCfg;
    RvStatus        rv = RV_OK;

    memset(&subsMgrCfg,0,sizeof(SipSubsMgrCfg));
    subsMgrCfg.pLogMgr          = pStackMgr->pLogMgr;
    subsMgrCfg.alertTimeout     = pStackCfg->subsAlertTimer;
    subsMgrCfg.autoRefresh      = pStackCfg->subsAutoRefresh;
    subsMgrCfg.hCallLegMgr      = pStackMgr->hCallLegMgr;
    subsMgrCfg.hTransportMgr    = pStackMgr->hTransport;
    subsMgrCfg.hTransactionMgr  = pStackMgr->hTranscMgr;
    subsMgrCfg.hMsgMgr          = pStackMgr->hMsgMgr;
    subsMgrCfg.hGeneralPool     = pStackMgr->hGeneralPool;
    subsMgrCfg.hElementPool      = pStackMgr->hElementPool;
    subsMgrCfg.maxNumOfSubscriptions = pStackCfg->maxSubscriptions;
    subsMgrCfg.pLogSrc      = pStackMgr->pLogSourceArrey[RVSIP_SUBSCRIPTION];
    subsMgrCfg.noNotifyTimeout  = pStackCfg->subsNoNotifyTimer;
    subsMgrCfg.seed  = &(pStackMgr->seed);
    subsMgrCfg.hStack           = (void*)pStackMgr;
    subsMgrCfg.bDisableRefer3515Behavior = pStackCfg->bDisableRefer3515Behavior;
    rv = SipSubsMgrConstruct(&subsMgrCfg, &(pStackMgr->hSubsMgr));
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructSubsModule - The subscription module couldn't be constructed"));
        pStackMgr->hSubsMgr = NULL;
        return rv;
    }
    
    rv = SipCallLegMgrSetSubsMgrHandle(pStackMgr->hCallLegMgr, pStackMgr->hSubsMgr);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructSubsModule - Couldn't set a reference to SubsMgr in CallLegMgr"));
        return rv;
    }
    
    rv = SipSubsMgrResetMaxUsageResourcesStatus(pStackMgr->hSubsMgr);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructSubsModule - Failure in SipSubsMgrResetMaxUsageResourcesStatus - may cause inconsistency"));
    }

    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "StackConstructSubsModule - subscription module constructed"));
    return rv;
}
#endif /*#ifdef RV_SIP_SUBS_ON*/
#endif /*#if !defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_JSR32_SUPPORT)*/

/************************************************************************************
 * StackConstructResolverModule
 * ----------------------------------------------------------------------------------
 * General: construct the stack resolver module
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg      - the structure that holds the configuration parameters.
 *                           if the ipAddress field of the structure is "0.0.0.0"
 *                           then we use transport layer api to get the local address.
 *          pStackMgr     - pointer to the struct that holds the stack information
 * Output: none
 ***********************************************************************************/
static RvStatus RVCALLCONV StackConstructResolverModule(INOUT RvSipStackCfg*   pStackCfg,
                                                        INOUT StackMgr*        pStackMgr)
{
    RvStatus rv = RV_OK;
    SipResolverMgrCfg cfg;

    memset(&cfg,0,sizeof(cfg));
    cfg.hElementPool    = pStackMgr->hElementPool;
    cfg.hGeneralPool    = pStackMgr->hGeneralPool;
    cfg.hMessagePool    = pStackMgr->hMessagePool;
    cfg.hTransportMgr   = pStackMgr->hTransport;
    cfg.maxDnsBuffLen   = pStackCfg->maxDnsBuffLen;
    cfg.maxDnsDomains   = pStackCfg->maxDnsDomains;
    cfg.maxDnsServers   = pStackCfg->maxDnsServers;
    cfg.maxNumOfRslv    = pStackCfg->maxTransmitters;
    cfg.pLogMgr         = pStackMgr->pLogMgr;
    cfg.pLogSrc         = pStackMgr->pLogSourceArrey[RVSIP_RESOLVER];
    cfg.seed            = &(pStackMgr->seed);
    cfg.strDialPlanSuffix= pStackCfg->strDialPlanSuffix;


    rv = SipResolverMgrConstruct(&cfg,
                                (void*)pStackMgr,
                                (0 >= pStackCfg->numOfDnsServers) ? RV_TRUE : RV_FALSE,
                                (0 >= pStackCfg->numOfDnsDomains) ? RV_TRUE : RV_FALSE,
                                &pStackMgr->hRslvMgr);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructResolverModule - The resolver module couldn't be constructed, rv=%d",rv));
        return rv;
    }
    if (0 < pStackCfg->numOfDnsServers && NULL != pStackCfg->pDnsServers)
    {
#if defined(UPDATED_BY_SPIRENT)
      {
	RvUint16 vdevblock = 0;
	for(vdevblock=0;vdevblock<RVSIP_MAX_NUM_VDEVBLOCK;vdevblock++) {
	  rv |= SipResolverMgrSetDnsServers(pStackMgr->hRslvMgr,pStackCfg->pDnsServers,pStackCfg->numOfDnsServers,vdevblock);
	}
      }
#else
        rv = SipResolverMgrSetDnsServers(pStackMgr->hRslvMgr,pStackCfg->pDnsServers,pStackCfg->numOfDnsServers);
#endif
        if (rv != RV_OK)
        {
            RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                "StackConstructResolverModule - failed to set DNS servers"));
            return rv;
        }
    }

    if (0 < pStackCfg->numOfDnsDomains && NULL != pStackCfg->pDnsDomains)
    {
#if defined(UPDATED_BY_SPIRENT)
      {
	RvUint16 vdevblock = 0;
	for(vdevblock=0;vdevblock<RVSIP_MAX_NUM_VDEVBLOCK;vdevblock++) {
	  rv |= SipResolverMgrSetDnsDomains(pStackMgr->hRslvMgr,pStackCfg->pDnsDomains,pStackCfg->numOfDnsDomains,vdevblock);
	}
      }
#else
        rv = SipResolverMgrSetDnsDomains(pStackMgr->hRslvMgr,pStackCfg->pDnsDomains,pStackCfg->numOfDnsDomains);
#endif
        if (rv != RV_OK)
        {
            RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                "StackConstructResolverModule - failed to set DNS domains"));
            return rv;
        }
    }
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "StackConstructResolverModule - resolver module constructed"));
    
    return rv;
}

#ifdef RV_SIP_IMS_ON
/************************************************************************************
 * StackConstructSecAgreeModule
 * ----------------------------------------------------------------------------------
 * General: construct the stack security-agreement module
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg     - the structure that holds the configuration parameters.
 *          pStackMgr     - pointer to the struct that holds the stack information
 * Output: none
 ***********************************************************************************/
static RvStatus RVCALLCONV StackConstructSecAgreeModule(INOUT RvSipStackCfg*   pStackCfg,
														INOUT StackMgr*        pStackMgr)
{
    SipSecAgreeMgrCfg   secAgreeMgrCfg;
    RvStatus			rv = RV_OK;

    memset(&secAgreeMgrCfg,0,sizeof(SipSecAgreeMgrCfg));
	secAgreeMgrCfg.hSipStack               = (void*)pStackMgr;
	secAgreeMgrCfg.hAuthenticator          = pStackMgr->hAuthentication;
	secAgreeMgrCfg.hMemoryPool             = pStackMgr->hGeneralPool;
	secAgreeMgrCfg.hMsgMgr                 = pStackMgr->hMsgMgr;
	secAgreeMgrCfg.hTransport              = pStackMgr->hTransport;
	secAgreeMgrCfg.hSecMgr                 = pStackMgr->hSecMgr;
	secAgreeMgrCfg.pLogMgr                 = pStackMgr->pLogMgr;
	secAgreeMgrCfg.pLogSrc                 = pStackMgr->pLogSourceArrey[RVSIP_SEC_AGREE];
	secAgreeMgrCfg.seed                    = &pStackMgr->seed;
	secAgreeMgrCfg.maxSecAgrees            = pStackCfg->maxSecAgrees;
	secAgreeMgrCfg.maxSecAgreeOwners       = pStackCfg->maxTransactions + pStackCfg->maxCallLegs + pStackCfg->maxPubClients + pStackCfg->maxRegClients;
	secAgreeMgrCfg.maxOwnersPerSecAgree    = pStackCfg->maxOwnersPerSecAgree;
	
    rv = SipSecAgreeMgrConstruct(&secAgreeMgrCfg, &(pStackMgr->hSecAgreeMgr));
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
					"StackConstructSecAgreeModule - The security-agreement module couldn't be constructed"));
        pStackMgr->hSubsMgr = NULL;
        return rv;
    }

	SipSecAgreeMgrResetMaxUsageResourcesStatus(pStackMgr->hSecAgreeMgr);

	rv = SipCallLegMgrSetSecAgreeMgrHandle(pStackMgr->hCallLegMgr, pStackMgr->hSecAgreeMgr);
	if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
					"StackConstructSecAgreeModule - Couldn't set a reference to Security-Agreement Mgr in CallLegMgr"));
        return rv;
    }

	rv = SipRegClientMgrSetSecAgreeMgrHandle(pStackMgr->hRegClientMgr, pStackMgr->hSecAgreeMgr);
	if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
					"StackConstructSecAgreeModule - Couldn't set a reference to Security-Agreement Mgr in RegClientMgr"));
        return rv;
    }

	rv = SipPubClientMgrSetSecAgreeMgrHandle(pStackMgr->hPubClientMgr, pStackMgr->hSecAgreeMgr);
	if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
			"StackConstructSecAgreeModule - Couldn't set a reference to Security-Agreement Mgr in PubClientMgr"));
        return rv;
    }

	rv = SipTransactionMgrSetSecAgreeMgrHandle(pStackMgr->hTranscMgr, pStackMgr->hSecAgreeMgr);
	if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
					"StackConstructSecAgreeModule - Couldn't set a reference to Security-Agreement Mgr in TranscMgr"));
        return rv;
    }

    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
				"StackConstructSecAgreeModule - security-agreement module constructed"));

    return rv;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

#ifdef RV_SIP_IMS_ON
/************************************************************************************
 * StackConstructSecurityModule
 * ----------------------------------------------------------------------------------
 * General: construct the stack IMS security module
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg - the structure that holds the configuration parameters.
 *          pStackMgr - pointer to the struct that holds the stack information
 * Output: none
 ***********************************************************************************/
static RvStatus RVCALLCONV StackConstructSecurityModule(
                                            INOUT RvSipStackCfg* pStackCfg,
                                            INOUT StackMgr*      pStackMgr)
{
    RvStatus rv;
    SipSecurityMgrCfg cfg;
    
    memset(&cfg,0,sizeof(cfg));
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
    cfg.getAlternativeSecObjFunc = pStackCfg->getAlternativeSecObjFunc;
#endif
//SPIRENT_END
    cfg.maxNumOfSecObj        = pStackCfg->maxSecurityObjects;
    cfg.maxNumOfIpsecSessions = pStackCfg->maxIpsecSessions;
    cfg.spiRangeStart         = pStackCfg->spiRangeStart;
    cfg.spiRangeEnd           = pStackCfg->spiRangeEnd;
    cfg.pLogMgr       = pStackMgr->pLogMgr;
    cfg.pLogSrc       = pStackMgr->pLogSourceArrey[RVSIP_SECURITY];
    cfg.hMsgMgr       = pStackMgr->hMsgMgr;
    cfg.hTransportMgr = pStackMgr->hTransport;
    cfg.hStack        = (void*)pStackMgr;
    cfg.bTcpEnabled   = pStackCfg->tcpEnabled;
    cfg.pSeed         = &(pStackMgr->seed);
    
    rv = SipSecurityMgrConstruct(&cfg, &pStackMgr->hSecMgr);
    if (rv != RV_OK)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "StackConstructSecurityModule(pStackMgr=%p) - Failed to construct Security Manager (rv=%d:%s)",
            pStackMgr, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "StackConstructSecurityModule(pStackMgr=%p) - Security Module (hSecMgr=%p) was constructed",
        pStackMgr, pStackMgr->hSecMgr));
    
    return RV_OK;
}
#endif /*#ifdef RV_SIP_IMS_ON*/

/************************************************************************************
 * CalculateConfigParams
 * ----------------------------------------------------------------------------------
 * General: Calculate the valued of the configuration parameters that the user set to
 *          -1. Parameters that cannot have the zero value will also be calculated.
 *          The calculations are based on the number of call-legs transactions and
 *          register clients.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg   - the structure that holds the configuration parameters.
 * Output:  none
 ***********************************************************************************/
static RvStatus RVCALLCONV CalculateConfigParams(IN StackMgr*      pStackMgr,
                                                 IN RvSipStackCfg *pStackCfg)
{
    RvInt32 calculatedNumOfLocalAddresses;

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pStackMgr);
#endif

#if (RV_THREADNESS_TYPE == RV_THREADNESS_SINGLE)
    if (pStackCfg->numberOfProcessingThreads > 0)
    {
        RvLogError(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
              "CalculateConfigParams - The non-MT Stack doesn't support %d processing threads",
               pStackCfg->numberOfProcessingThreads));

        return RV_ERROR_NOTSUPPORTED;
    }
#endif /* #if (RV_THREAD_TYPE != RV_THREADNESS_MULTI) */

#ifndef RV_SIP_PRIMITIVES
    if(pStackCfg->maxCallLegs < 0)
    {
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "CalculateConfigParams - maxCallLegs - default value %d was taken instead of given value %d",
               DEFAULT_MAX_CALL_LEGS, pStackCfg->maxCallLegs));
        pStackCfg->maxCallLegs = DEFAULT_MAX_CALL_LEGS;
    }

    if(pStackCfg->maxRegClients < 0)
    {
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "CalculateConfigParams - maxRegClients - default value %d was taken instead of given value %d",
               DEFAULT_MAX_REG_CLIENTS, pStackCfg->maxRegClients));
        pStackCfg->maxRegClients = DEFAULT_MAX_REG_CLIENTS;
    }

	if(pStackCfg->maxPubClients < 0)
    {
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
			"CalculateConfigParams - maxPubClients - default value %d was taken instead of given value %d",
			DEFAULT_MAX_PUB_CLIENTS, pStackCfg->maxPubClients));
        pStackCfg->maxPubClients = DEFAULT_MAX_PUB_CLIENTS;
    }

#ifdef RV_SIP_SUBS_ON
    if(pStackCfg->maxSubscriptions <= 0)
    {
        pStackCfg->maxSubscriptions = 0;
    }
#endif /* #ifdef RV_SIP_SUBS_ON */

#endif /* #ifndef RV_SIP_PRIMITIVES */

#ifndef RV_SIP_PRIMITIVES
    /*it is not allowed to use 0 transaction*/
    if (pStackCfg->maxTransactions <= 0)
    {
        if(pStackCfg->maxCallLegs == 0)
        {
            pStackCfg->maxTransactions = DEFAULT_MAX_CALL_LEGS;
        }
        else
        {
            pStackCfg->maxTransactions = pStackCfg->maxCallLegs;

        }
#ifdef RV_SIP_SUBS_ON
        pStackCfg->maxTransactions += pStackCfg->maxSubscriptions;
#endif /* #ifdef RV_SIP_SUBS_ON */

        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
           "CalculateConfigParams - maxTransactions - default value %d was taken.",
           pStackCfg->maxTransactions));

    }
#else
    pStackCfg->maxTransactions = DEFAULT_MAX_TRANSACTIONS;
#endif /* RV_SIP_PRIMITIVES */

    if (pStackCfg->maxTransmitters <= 0)
    {
#ifndef RV_SIP_PRIMITIVES
        pStackCfg->maxTransmitters = pStackCfg->maxTransactions + pStackCfg->maxCallLegs + 10;
#else
        pStackCfg->maxTransmitters = pStackCfg->maxTransactions + 10;
#endif /* RV_SIP_PRIMITIVES */

#ifndef RV_SIP_PRIMITIVES
        if(pStackCfg->bEnableForking == RV_TRUE &&
            pStackCfg->maxCallLegs > 0)
        {
            pStackCfg->maxTransmitters = pStackCfg->maxTransmitters +
                                         pStackCfg->maxCallLegs/2;

        }
#endif /*#ifndef RV_SIP_PRIMITIVES*/
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                  "CalculateConfigParams - maxTransmitters - default value %d was taken instead of -1",
                   pStackCfg->maxTransmitters));
    }
    else
    {
        pStackCfg->maxTransmitters += pStackCfg->maxTransactions;
    }

    if(pStackCfg->tcpEnabled == RV_TRUE && pStackCfg->maxConnections <= 0)
    {
        pStackCfg->maxConnections = (pStackCfg->maxTransactions/2)+1;/*T/2*/
    }

#if (RV_TLS_TYPE != RV_TLS_NONE)
    if(pStackCfg->maxTlsSessions < 0    &&
       pStackCfg->numOfTlsAddresses > 0 &&
       pStackCfg->localTlsAddresses != NULL)
    {
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "CalculateConfigParams - maxTlsSessions - default value %d was taken instead of given value %d",
               pStackCfg->maxConnections, pStackCfg->maxTlsSessions));
        pStackCfg->maxTlsSessions = pStackCfg->maxConnections;
    }

#endif /*  (RV_TLS_TYPE != RV_TLS_NONE) */


    if (pStackCfg->messagePoolNumofPages <= 0)
    {
        pStackCfg->messagePoolNumofPages = 2*(pStackCfg->maxTransactions) +
                                            (pStackCfg->maxTransactions)/2 ;/*2.5T*/

        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "CalculateConfigParams - messagePoolNumofPages - default value %d was taken",
               pStackCfg->messagePoolNumofPages));

    }
    /*do not allow less then 4 pages needed to complete one client invite transaction*/
    if(pStackCfg->messagePoolNumofPages < 4)
    {
        pStackCfg->messagePoolNumofPages = 4;
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
           "CalculateConfigParams - messagePoolNumofPages - 4 pages were configured. minimum value!"));
    }

    if (pStackCfg->messagePoolPageSize <= 0)
    {
        pStackCfg->messagePoolPageSize = DEFAULT_MSG_POOL_PAGE_SIZE;
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "CalculateConfigParams - messagePoolPageSize - default value %d was taken",
               pStackCfg->messagePoolPageSize));
    }

    if (pStackCfg->generalPoolNumofPages <= 0)
    {
        pStackCfg->generalPoolNumofPages =
#ifndef RV_SIP_PRIMITIVES
                        (pStackCfg->maxCallLegs) + (pStackCfg->maxCallLegs) /2 +
                       2*pStackCfg->maxPubClients + 2*pStackCfg->maxRegClients + 5 /*1.5N + 1.5T+ 1R + 1P +5*/+
#ifdef RV_SIP_SUBS_ON
						pStackCfg->maxSubscriptions /*each subscription holds its own page*/+ 
				
#endif /* #ifdef RV_SIP_SUBS_ON */

#endif /* RV_SIP_PRIMITIVES */
                        (pStackCfg->maxTransactions) +(pStackCfg->maxTransactions)/2 ;

        /*add one page for each stand alone transmitter*/
        pStackCfg->generalPoolNumofPages += (pStackCfg->maxTransmitters-
                                             pStackCfg->maxTransactions);
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "CalculateConfigParams - generalPoolNumofPages - default value %d was taken",
               pStackCfg->generalPoolNumofPages));

    }
    if(pStackCfg->generalPoolNumofPages < 3)
    {
        pStackCfg->generalPoolNumofPages = 3;
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
           "CalculateConfigParams - messagePoolNumofPages - 4 pages were configured. minimum value!"));
    }

    if (pStackCfg->generalPoolPageSize <= 0)
    {
        pStackCfg->messagePoolPageSize = DEFAULT_GENERAL_POOL_PAGE_SIZE;
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
           "CalculateConfigParams - messagePoolPageSize - default value %d was taken",
           pStackCfg->messagePoolPageSize));
    }
	if(pStackCfg->generalPoolPageSize < 512)
	{
		pStackCfg->generalPoolPageSize = DEFAULT_GENERAL_POOL_PAGE_SIZE;
	}


    /*transport*/
    if (pStackCfg->sendReceiveBufferSize <=0)
    {
        pStackCfg->sendReceiveBufferSize = DEFAULT_MAX_BUFFER_SIZE;
    }

    if(pStackCfg->localUdpAddress[0] == '\0')
    {
       strcpy(pStackCfg->localUdpAddress,IPV4_LOCAL_ADDRESS);
    }

    if(pStackCfg->localTcpAddress[0] == '\0')
    {
       strcpy(pStackCfg->localTcpAddress,IPV4_LOCAL_ADDRESS);
    }
    if(pStackCfg->numOfExtraTcpAddresses < 0)
    {
       pStackCfg->numOfExtraTcpAddresses = 0;
    }
    if(pStackCfg->numOfExtraUdpAddresses < 0)
    {
       pStackCfg->numOfExtraUdpAddresses = 0;
    }
    if(pStackCfg->numOfTlsAddresses < 0)
    {
        pStackCfg->numOfTlsAddresses = 0;
    }
    if(pStackCfg->bIgnoreLocalAddresses == RV_FALSE)
	{
		calculatedNumOfLocalAddresses =
			2 +     /* 2 stands for the first UDP and the first TCP addresses */
			pStackCfg->numOfSctpAddresses     +
			pStackCfg->numOfTlsAddresses      +
			pStackCfg->numOfExtraTcpAddresses +
			pStackCfg->numOfExtraUdpAddresses;
		if(pStackCfg->maxNumOfLocalAddresses <= 0)
		{
			pStackCfg->maxNumOfLocalAddresses = calculatedNumOfLocalAddresses;
			RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
				"CalculateConfigParams - maxNumOfLocalAddresses - default value %d was taken",
				pStackCfg->maxNumOfLocalAddresses));
		}
		else
		{
			pStackCfg->maxNumOfLocalAddresses =
				RvMax(calculatedNumOfLocalAddresses,pStackCfg->maxNumOfLocalAddresses);
		}
	}
	else
	{
		pStackCfg->maxNumOfLocalAddresses =
			RvMax(DEFAULT_MAX_LOCAL_ADDRESSES,pStackCfg->maxNumOfLocalAddresses);
	}

    if (pStackCfg->proxy2xxRcvdTimer < 0)
    {
        pStackCfg->proxy2xxRcvdTimer = 64*pStackCfg->retransmissionT1;
    }
    if (pStackCfg->proxy2xxSentTimer < 0)
    {
        pStackCfg->proxy2xxSentTimer = DEFAULT_PROXY_2XX_SEND_TIMER;
    }
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON
    if(pStackCfg->subsAlertTimer < 0 && pStackCfg->maxSubscriptions > 0)
    {
        pStackCfg->subsAlertTimer = DEFAULT_SUBS_ALERT_TIMER;
    }
    if(pStackCfg->subsNoNotifyTimer < 0 && pStackCfg->maxSubscriptions > 0)
    {
        pStackCfg->subsNoNotifyTimer = DEFAULT_SUBS_NO_NOTIFY_TIMER;
    }

    if(pStackCfg->subsAlertTimer == 0 && pStackCfg->maxSubscriptions > 0)
    {
        pStackCfg->subsAutoRefresh = RV_FALSE;
    }
#endif /* #ifdef RV_SIP_SUBS_ON */
    if (pStackCfg->sessionExpires < 0)
    {
        pStackCfg->sessionExpires = DEFAULT_SESSION_EXPIRES_TIME;
    }
    if (pStackCfg->minSE < 0 && pStackCfg->minSE != UNDEFINED)
    {
        pStackCfg->minSE = RVSIP_CALL_LEG_MIN_MINSE;
    }
    if(pStackCfg->minSE > pStackCfg->sessionExpires && pStackCfg->sessionExpires > 0)
    {
        RvLogWarning(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "CalculateConfigParams - minSE is bigger than the SessionExpires value (%d). SessionExpires changed to %d",
            pStackCfg->sessionExpires, pStackCfg->minSE));
        pStackCfg->sessionExpires = pStackCfg->minSE;

    }
#endif /* RV_SIP_PRIMITIVES */
    if(pStackCfg->processingQueueSize <= 0)
    {
        pStackCfg->processingQueueSize = pStackCfg->maxTransactions  +
#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON
                                         pStackCfg->maxSubscriptions +
#endif /* #ifdef RV_SIP_SUBS_ON */
                                         pStackCfg->maxCallLegs +
                                         pStackCfg->maxPubClients + pStackCfg->maxRegClients +
#endif /* RV_SIP_PRIMITIVES */
                                         10;

        if(pStackCfg->maxConnections > 0)
        {
            pStackCfg->processingQueueSize += pStackCfg->maxConnections;
        }
    }

    if(pStackCfg->numOfReadBuffers < 0)
    {
        pStackCfg->numOfReadBuffers = (pStackCfg->processingQueueSize * 1/2);
                                      
#ifdef RV_SIGCOMP_ON
        pStackCfg->numOfReadBuffers += pStackCfg->maxTransactions/2;
#endif
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
           "CalculateConfigParams - numOfReadBuffers - default value %d was taken",
           pStackCfg->numOfReadBuffers));
    }



#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    if(pStackCfg->maxElementsInSingleDnsList == 0)
    {
        pStackCfg->maxElementsInSingleDnsList = DEFAULT_MAX_NUM_OF_ELEMENTS_IN_DNS_LIST;
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
           "CalculateConfigParams - maxElementsInSingleDnsList - default value %d was taken",
           pStackCfg->maxElementsInSingleDnsList));
    }
#endif
    if (strcmp(pStackCfg->outboundProxyIpAddress, "0.0.0.0") == 0 ||
        strcmp(pStackCfg->outboundProxyIpAddress, "0") == 0)
    {
        pStackCfg->outboundProxyIpAddress[0] = '\0';
    }

#if(RV_NET_TYPE & RV_NET_IPV6)
    if (0 == memcmp(pStackCfg->outboundProxyIpAddress, IPV6_LOCAL_ADDRESS, IPV6_LOCAL_ADDRESS_SZ))
    {
        pStackCfg->outboundProxyIpAddress[0] = '\0';
    }
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/

    /* a default port is given only for ip outbound address*/
    if(pStackCfg->outboundProxyIpAddress[0] != '\0')
    {
        if (pStackCfg->outboundProxyPort == 0)
        {
            pStackCfg->outboundProxyPort = UNDEFINED;
        }
    }

    if(pStackCfg->eOutboundProxyTransport != RVSIP_TRANSPORT_UNDEFINED &&
       pStackCfg->eOutboundProxyTransport != RVSIP_TRANSPORT_TCP &&
       pStackCfg->eOutboundProxyTransport != RVSIP_TRANSPORT_TLS &&
#if (RV_NET_TYPE & RV_NET_SCTP)
       pStackCfg->eOutboundProxyTransport != RVSIP_TRANSPORT_SCTP &&
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
       pStackCfg->eOutboundProxyTransport != RVSIP_TRANSPORT_UDP)
    {
        pStackCfg->eOutboundProxyTransport = RVSIP_TRANSPORT_UNDEFINED;
    }
    if(pStackCfg->ePersistencyLevel != RVSIP_TRANSPORT_PERSISTENCY_LEVEL_TRANSC_USER &&
       pStackCfg->ePersistencyLevel != RVSIP_TRANSPORT_PERSISTENCY_LEVEL_TRANSC &&
       pStackCfg->ePersistencyLevel != RVSIP_TRANSPORT_PERSISTENCY_LEVEL_UNDEFINED)
    {
        pStackCfg->ePersistencyLevel = RVSIP_TRANSPORT_PERSISTENCY_LEVEL_UNDEFINED;
    }

    if (pStackCfg->elementPoolPageSize <= 0)
    {
        pStackCfg->elementPoolPageSize = DEFAULT_ELEMENT_POOL_PAGE_SIZE;
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
           "CalculateConfigParams - elementPoolPageSize - default value %d was taken",
           pStackCfg->elementPoolPageSize));
    }

    /*calculating the elementPoolNumofPages*/
    if(pStackCfg->elementPoolNumofPages <=0)
    {
        pStackCfg->elementPoolNumofPages = 0;
#ifndef RV_SIP_PRIMITIVES
        /*for holding the remote contact in a call-leg and the original remote contact*/
        if(pStackCfg->maxCallLegs > 0)
        {
            pStackCfg->elementPoolNumofPages += (pStackCfg->maxCallLegs*2);
        }
#ifdef RV_SIP_SUBS_ON
        if(pStackCfg->maxSubscriptions > 0)
        {
            pStackCfg->elementPoolNumofPages += pStackCfg->maxSubscriptions/2;
        }
#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /* #ifndef RV_SIP_PRIMITIVES  */
        if(pStackCfg->maxConnections > 0)
        {
            pStackCfg->elementPoolNumofPages += pStackCfg->maxConnections;
        }

        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "CalculateConfigParams - elementPoolNumofPages - default value %d was taken",
               pStackCfg->elementPoolNumofPages));
    }
#ifdef RV_SIGCOMP_ON
    if (pStackCfg->maxCompartments <= 0)
    {
#ifndef RV_SIP_PRIMITIVES
        pStackCfg->maxCompartments = pStackCfg->maxCallLegs + pStackCfg->maxPubClients + pStackCfg->maxRegClients;
#else
        pStackCfg->maxCompartments = pStackCfg->maxTransactions;
#endif /*#ifndef RV_SIP_PRIMITIVES*/

        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
           "CalculateConfigParams - maxCompartments - default value %d was taken",
           pStackCfg->maxCompartments));
    }
    if (pStackCfg->sigCompTcpTimer < 0)
    {
        pStackCfg->sigCompTcpTimer = 0;
    }

#endif
    if (0 == pStackCfg->outboundProxyPort)
    {
        pStackCfg->outboundProxyPort = (RvInt16)UNDEFINED;
    }
#ifdef RV_SIP_PRIMITIVES
    pStackCfg->bDynamicInviteHandling = RV_TRUE;
#endif /*#ifdef RV_SIP_PRIMITIVES*/
    if (UNDEFINED == pStackCfg->maxDnsDomains)
    {
        pStackCfg->maxDnsDomains = (UNDEFINED == pStackCfg->numOfDnsDomains) ? DEFAULT_MAX_DNS_DOMAINS : pStackCfg->numOfDnsDomains;
    }
    if (UNDEFINED == pStackCfg->maxDnsServers)
    {
        pStackCfg->maxDnsServers = (UNDEFINED == pStackCfg->numOfDnsServers) ? DEFAULT_MAX_DNS_SERVERS : pStackCfg->numOfDnsServers;
    }
#if (RV_NET_TYPE & RV_NET_SCTP)
    if (UNDEFINED == pStackCfg->numOfOutStreams)
    {
        pStackCfg->numOfOutStreams = DEFAULT_SCTP_OUT_STREAMS;
    }
    if (UNDEFINED == pStackCfg->numOfInStreams)
    {
        pStackCfg->numOfInStreams = DEFAULT_SCTP_IN_STREAMS;
    }
    if (UNDEFINED == pStackCfg->avgNumOfMultihomingAddresses)
    {
        pStackCfg->avgNumOfMultihomingAddresses   = RVSIP_TRANSPORT_SCTP_MULTIHOMED_ADDR_NUM_AVG_PER_CONN;
    }
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

    /* Security */
#ifdef RV_SIP_IMS_ON
    if (UNDEFINED == pStackCfg->maxSecurityObjects)
    {
        pStackCfg->maxSecurityObjects = DEFAULT_MAX_SECURITY_OBJECTS;
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "CalculateConfigParams - maxSecurityObjects was set to default %d",
            DEFAULT_MAX_SECURITY_OBJECTS));
    }
	if (UNDEFINED == pStackCfg->maxSecAgrees)
    {
        pStackCfg->maxSecAgrees = DEFAULT_MAX_SEC_AGREE_OBJECTS;
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
				  "CalculateConfigParams - maxSecAgrees was set to default %d",
				  pStackCfg->maxSecAgrees));
    }
	if (UNDEFINED == pStackCfg->maxOwnersPerSecAgree)
    {
        pStackCfg->maxOwnersPerSecAgree = DEFAULT_MAX_OWNERS_OF_SEC_AGREE;
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
				  "CalculateConfigParams - maxOwnersPerSecAgree was set to default %d",
				  pStackCfg->maxOwnersPerSecAgree));
    }
#if (RV_IMS_IPSEC_ENABLED==RV_YES)
    if (UNDEFINED == pStackCfg->maxIpsecSessions)
    {
        pStackCfg->maxIpsecSessions = pStackCfg->maxSecurityObjects;
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "CalculateConfigParams - maxIpsecSessions was set to default %d - maxSecurityObjects",
            pStackCfg->maxSecurityObjects));
    }
    if (0 == pStackCfg->spiRangeStart || 0 == pStackCfg->spiRangeEnd)
    {
        pStackCfg->spiRangeStart  = DEFAULT_SPI_RANGE_START;
        pStackCfg->spiRangeEnd    = DEFAULT_SPI_RANGE_END;
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "CalculateConfigParams - SPI range was set to default [%u-%u]",
            pStackCfg->spiRangeStart, pStackCfg->spiRangeEnd));
    }
    if (DEFAULT_SPI_RANGE_START > pStackCfg->spiRangeStart)
    {
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "CalculateConfigParams - SPI range start (%u) should be greater than %u. Set it to %u",
            pStackCfg->spiRangeStart, (DEFAULT_SPI_RANGE_START-1), DEFAULT_SPI_RANGE_START));
        pStackCfg->spiRangeStart = DEFAULT_SPI_RANGE_START;
    }
    if (pStackCfg->spiRangeStart >= pStackCfg->spiRangeEnd)
    {
        RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "CalculateConfigParams - SPI range end (%u) should be greater than range start. Set it to %u",
            pStackCfg->spiRangeEnd, DEFAULT_SPI_RANGE_END));
        pStackCfg->spiRangeEnd = DEFAULT_SPI_RANGE_END;
    }
    /* Force DLA */
    pStackCfg->bDLAEnabled = RV_TRUE;
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "CalculateConfigParams - bDLAEnabled was set to TRUE, since it is required by IMS Security"));
    /* Update the maxNumOfLocalAddresses.
       Each Security Oject may consume 5 Local Addresses:
    for TLS protection - 1 address; for IPsec protection - 2 UDP and 2 TCP */
    pStackCfg->maxNumOfLocalAddresses += pStackCfg->maxSecurityObjects + 
                                         4 * pStackCfg->maxIpsecSessions;
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "CalculateConfigParams - maxNumOfLocalAddresses was increased by %d, as required by IMS Security",
        pStackCfg->maxSecurityObjects + 4 * pStackCfg->maxIpsecSessions));
    /* Update the maxConnections.
       Each Security Oject may consume 4 TCP connections for IPsec:
    1 client, 1 server and 2 multi-server, used by 2 TCP Local Addresses */
    pStackCfg->maxConnections += 4 * pStackCfg->maxIpsecSessions;
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "CalculateConfigParams - maxConnections was increased by %d, as required by IMS Security for IPsec",
        4 * pStackCfg->maxIpsecSessions));
#endif /*#if (RV_IMS_IPSEC_ENABLED==RV_YES)*/
#if (RV_TLS_TYPE != RV_TLS_NONE)
    /* Update the maxConnections.
    Each Security Oject on UE side may consume 1 client connection for TLS.
    On P-CSCF SecAgree object sets the established connection into the Security
    Objects, so no Connection is constructed by the Security Object. */
    pStackCfg->maxConnections += pStackCfg->maxSecurityObjects;
    RvLogDebug(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "CalculateConfigParams - maxConnections was increased by %d, as required by IMS Security for TLS",
        pStackCfg->maxSecurityObjects));
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/
#endif /*#ifdef RV_SIP_IMS_ON*/
    return RV_OK;
}

/************************************************************************************
 * ConvertZeroAddrToRealAddr
 * ----------------------------------------------------------------------------------
 * General: convert zero addresses found in the configuration to actual addresses
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackCfg      - the structure that holds the configuration parameters.
 *                           if the ipAddress field of the structure is "0.0.0.0"
 *                           then we use transport layer api to get the local address.
 *          pStackMgr     - pointer to the struct that holds the stack information
 * Output:  none
 ***********************************************************************************/
static RvStatus RVCALLCONV ConvertZeroAddrToRealAddr(INOUT RvSipStackCfg    *pStackCfg,
                                                      INOUT StackMgr        *pStackMgr)
{

    RvAddress              localAddrs[2];
    RvUint                 numLocalAddresses   = 2;
    RvStatus               rv                  = RV_OK;
    RvAddress*             pDefIpv4            = NULL;
    RvBool                 bCanConvertZero4    = RV_FALSE;
    RvUint32               i;

#if(RV_NET_TYPE & RV_NET_IPV6)
    RvBool                 bCanConvertZero6 = RV_FALSE;
    RvAddress*             pDefIpv6         = NULL;
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/

    /* getting the default address */
    rv = TransportMgrGetDefaultHost((TransportMgr*)pStackMgr->hTransport,RV_ADDRESS_TYPE_IPV4,&localAddrs[0]);
    if (RV_OK == rv)
    {
        bCanConvertZero4 = RV_TRUE;
        pDefIpv4 = &localAddrs[0];
    }
#if(RV_NET_TYPE & RV_NET_IPV6)
    rv = TransportMgrGetDefaultHost((TransportMgr*)pStackMgr->hTransport,RV_ADDRESS_TYPE_IPV6,&localAddrs[1]);
    if (RV_OK == rv)
    {
        bCanConvertZero6 = RV_TRUE;
        pDefIpv6 = &localAddrs[1];
    }
#endif /*(RV_NET_TYPE & RV_NET_IPV6)*/

    /* UDP addresses */
    /* Update configuration structure with local host name */
    if ((0 == strcmp(pStackCfg->localUdpAddress, "0")) ||
        (0 == strcmp(pStackCfg->localUdpAddress, IPV4_LOCAL_ADDRESS)) ||
        (0 == strcmp(pStackCfg->localUdpAddress, IPV4_LOCAL_LOOP)))
    {
        if (RV_FALSE == bCanConvertZero4)
        {
            return RV_ERROR_UNKNOWN;
        }

        RvAddressGetString(pDefIpv4,48,pStackCfg->localUdpAddress);
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                  "ConvertZeroAddrToRealAddr - local UDP address set as %s",pStackCfg->localUdpAddress));

    }
#if(RV_NET_TYPE & RV_NET_IPV6)
    /* Update configuration structure with local host name */
    if  (0 == memcmp(pStackCfg->localUdpAddress, IPV6_LOCAL_ADDRESS, IPV6_LOCAL_ADDRESS_SZ))
    {
        RvChar    strIp6AddressHelper[48];
        RvChar*   sidLocation      = NULL;
        RvInt32   sid              = 0;

        if (RV_FALSE == bCanConvertZero6)
        {
            return RV_ERROR_UNKNOWN;
        }

        sidLocation = strchr(pStackCfg->localUdpAddress,'%');
        if (NULL != sidLocation)
        {
            sidLocation++;
            sscanf(sidLocation,"%d",&sid);
        }

        RvSprintf(pStackCfg->localUdpAddress, "[%s]%c%d",
            RvAddressGetString(pDefIpv6,48,strIp6AddressHelper),
            '%',sid);

        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                  "ConvertZeroAddrToRealAddr - local UDP address set as %s",pStackCfg->localUdpAddress));
    }
#endif /* (RV_NET_TYPE & RV_NET_IPV6) */
    /* TCP addresses */
    if(pStackCfg->tcpEnabled == RV_FALSE)
    {
        return RV_OK;
    }

    if ((0 == strcmp(pStackCfg->localTcpAddress, "0")) ||
        (0 == strcmp(pStackCfg->localTcpAddress, IPV4_LOCAL_ADDRESS)) ||
        (0 == strcmp(pStackCfg->localTcpAddress, IPV4_LOCAL_LOOP)))
    {
        if (RV_FALSE == bCanConvertZero4)
        {
            return RV_ERROR_UNKNOWN;
        }

        RvAddressGetString(pDefIpv4,48,pStackCfg->localTcpAddress);
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                  "ConvertZeroAddrToRealAddr - local TCP address set as %s",pStackCfg->localTcpAddress));

    }
#if(RV_NET_TYPE & RV_NET_IPV6)
    if (0 == memcmp(pStackCfg->localTcpAddress,IPV6_LOCAL_ADDRESS, IPV6_LOCAL_ADDRESS_SZ))
    {
        RvChar    strIp6AddressHelper[48];
        RvChar*   sidLocation      = NULL;
        RvInt32   sid              = 0;

        if (RV_FALSE == bCanConvertZero6)
        {
            return RV_ERROR_UNKNOWN;
        }

        sidLocation = strchr(pStackCfg->localTcpAddress,'%');
        if (NULL != sidLocation)
        {
            sidLocation++;
            sscanf(sidLocation,"%d",&sid);
        }

        RvSprintf(pStackCfg->localTcpAddress, "[%s]%c%d",
            RvAddressGetString(pDefIpv6,48,strIp6AddressHelper),
            '%',sid);

        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                  "ConvertZeroAddrToRealAddr - local TCP address set as %s",strIp6AddressHelper));

    }
#endif /* (RV_NET_TYPE & RV_NET_IPV6) */
    for (i=0 ;i< numLocalAddresses; i++)
        RvAddressDestruct(&localAddrs[i]);

    return RV_OK;
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/************************************************************************************
 * LogPrintCallbackFunction
 * ----------------------------------------------------------------------------------
 * General: This function will be registered to the log module as the
 *          function that prints to the log. It will call the application
 *          print function.
 * Return Value: void
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   logRecord - log record received from the common core.
 *          context   - log context.
 ***********************************************************************************/
static void RVCALLCONV LogPrintCallbackFunction(
                               IN RvLogRecord* pLogRecord,
                               IN void*        context)
{

    StackMgr        *pStackMgr     = (StackMgr*)context;
    RvChar          *strFinalText;

    SipCommonCoreFormatLogMessage(pLogRecord,
                                  &strFinalText);

    /*call the application print function*/
    if(NULL != pStackMgr->pfnPrintLogEntryEvHandler)
    {
        pStackMgr->pfnPrintLogEntryEvHandler(pStackMgr->logContext,
                                             SipCommonConvertCoreToSipLogFilter(pLogRecord->messageType),
                                             strFinalText);
    }
}

/************************************************************************************
 * PrintConfigParamsToLog
 * ----------------------------------------------------------------------------------
 * General: Prints all the configuration parameters to the log.
 * Return Value: void
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pStackMgr - the stack manager
 *          pStackCfg  - the configuration structure
 ***********************************************************************************/
static void RVCALLCONV PrintConfigParamsToLog(
                                    IN StackMgr*     pStackMgr,
                                    IN RvSipStackCfg* pStackCfg)
{
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "[======Compilation Parameters=========]"));

#ifndef SIP_DEBUG
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - RELEASE = on"));
#else
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - SIP_DEBUG"));
#endif
#ifdef RV_SIP_PRIMITIVES
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - RV_SIP_PRIMITIVES = on"));
#endif
#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - RV_DNS_ENHANCED_FEATURES_SUPPORT = on"));
#endif
#ifdef RV_SIP_IMS_ON
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - RV_SIP_IMS_ON = on"));
#endif

#ifdef RV_SIP_IPSEC_NEG_ONLY
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - RV_SIP_IPSEC_NEG_ONLY = on"));
#endif
	
#ifdef RV_DEPRECATED_CORE
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - RV_DEPRECATED_CORE = on"));
#endif
	
#ifdef RV_SIP_OTHER_HEADER_ENCODE_SEPARATELY
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - RV_SIP_OTHER_HEADER_ENCODE_SEPARATELY = on"));
#endif

#ifdef RV_SIP_HIGH_AVAL_3_0
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - RV_SIP_HIGH_AVAL_3_0 = on"));
#endif

#ifdef RV_SIP_SUBS_ON
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - RV_SIP_SUBS_ON = on"));
#endif

#ifdef RV_SIP_AUTH_ON
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - RV_SIP_AUTH_ON = on"));
#endif

#ifdef RV_SIP_HIGHAVAL_ON
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - RV_SIP_HIGHAVAL_ON = on"));
#endif

#ifdef RV_SIP_TEL_URI_SUPPORT
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - RV_SIP_TEL_URI_SUPPORT = on"));
#endif

#ifdef RV_SIP_OTHER_URI_SUPPORT
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - RV_SIP_OTHER_URI_SUPPORT = on"));
#endif

#ifdef RV_SIP_JSR32_SUPPORT
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - RV_SIP_JSR32_SUPPORT = on"));
#endif

#ifdef RV_SIP_EXTENDED_HEADER_SUPPORT
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - RV_SIP_EXTENDED_HEADER_SUPPORT = on"));
#endif

#ifdef RV_SIP_UNSIGNED_CSEQ
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - RV_SIP_UNSIGNED_CSEQ = on"));
#endif

#ifdef RV_SIP_DISABLE_LISTENING_SOCKETS
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - RV_SIP_DISABLE_LISTENING_SOCKETS = on"));
#endif

#ifdef RV_SIP_LIGHT
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - RV_SIP_LIGHT = on"));
#endif


#if(RV_NET_TYPE & RV_NET_IPV6)
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - IPV6 = on"));
#endif

#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - TLS = on"));
#endif
#if(RV_NET_TYPE & RV_NET_SCTP)
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - SCTP = on"));
#endif
#if (RV_THREADNESS_TYPE == RV_THREADNESS_SINGLE)
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - RV_THREADNESS_TYPE = RV_THREADNESS_SINGLE"));
#else
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - RV_THREADNESS_TYPE = RV_THREADNESS_MULTI"));

#endif /*#if (RV_THREADNESS_TYPE == RV_THREADNESS_SINGLE)*/
#ifdef RV_SIGCOMP_ON
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - RV_SIGCOMP_ON = on"));
#endif /*RV_SIGCOMP_ON*/
#if RV_TLS_ENABLE_RENEGOTIATION
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - RV_TLS_ENABLE_RENEGOTIATION = on"));
#endif /*RV_TLS_ENABLE_RENEGOTIATION*/
#ifdef RV_SSL_SESSION_STATUS
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - RV_SSL_SESSION_STATUS = on"));
#endif /*RV_SSL_SESSION_STATUS*/

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "[======Stack objects allocation=========]"));
#ifndef RV_SIP_JSR32_SUPPORT
#ifndef RV_SIP_PRIMITIVES
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - maxCallLegs = %d",pStackCfg->maxCallLegs));
#endif /* #ifndef RV_SIP_PRIMITIVES */

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - maxTransactions = %d",pStackCfg->maxTransactions));
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */ 
	
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - maxTransmitters = %d",pStackCfg->maxTransmitters));

#ifndef RV_SIP_JSR32_SUPPORT
#ifndef RV_SIP_PRIMITIVES
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - maxRegClients = %d",pStackCfg->maxRegClients));
	RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - maxPubClients = %d",pStackCfg->maxPubClients));
#endif /* #ifndef RV_SIP_PRIMITIVES */
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
	
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "[=======Memory pools allocation=========]"));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - messagePoolNumofPages = %d",pStackCfg->messagePoolNumofPages));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - messagePoolPageSize = %d",pStackCfg->messagePoolPageSize));

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - generalPoolNumofPages = %d",pStackCfg->generalPoolNumofPages));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - generalPoolPageSize = %d",pStackCfg->generalPoolPageSize));

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - elementPoolNumofPages = %d",pStackCfg->elementPoolNumofPages));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - elementPoolPageSize = %d",pStackCfg->elementPoolPageSize));

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "[=======Network parameters==============]"));

	RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
		"PrintConfigParamsToLog - bIgnoreLocalAddresses = %d",pStackCfg->bIgnoreLocalAddresses));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "PrintConfigParamsToLog - maxNumOfLocalAddresses = %d",pStackCfg->maxNumOfLocalAddresses));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - localUdpAddress = %s",pStackCfg->localUdpAddress));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "PrintConfigParamsToLog - localUdpPort = %d",pStackCfg->localUdpPort));

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
              "PrintConfigParamsToLog - numOfExtraUdpAddresses = %d",pStackCfg->numOfExtraUdpAddresses));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - tcpEnabled = %d",pStackCfg->tcpEnabled));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - localTcpAddress = %s",pStackCfg->localTcpAddress));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - localTcpPort = %d",pStackCfg->localTcpPort));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - numOfExtraTcpAddresses = %d",pStackCfg->numOfExtraTcpAddresses));

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - maxConnections = %d",pStackCfg->maxConnections));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - ePersistencyLevel = %d",pStackCfg->ePersistencyLevel));
    
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "PrintConfigParamsToLog - connectionCapacityPercent = %d",pStackCfg->connectionCapacityPercent));

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - serverConnectionTimeout = %d",pStackCfg->serverConnectionTimeout));

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - DLAEnabled = %d",pStackCfg->bDLAEnabled));

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - outboundProxyIpAddress = %s",pStackCfg->outboundProxyIpAddress));

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - outboundProxyHostName = %s",
               (pStackCfg->outboundProxyHostName!= NULL)?pStackCfg->outboundProxyHostName:"None"));

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - outboundProxyPort = %d",pStackCfg->outboundProxyPort));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - eOutboundProxyTransport = %d",pStackCfg->eOutboundProxyTransport));

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - sendReceiveBufferSize = %d",pStackCfg->sendReceiveBufferSize));
#if (RV_TLS_TYPE != RV_TLS_NONE)
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "[=======TLS parameters==============]"));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - numOfTlsAddresses = %d",pStackCfg->numOfTlsAddresses));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - numOfTlsEngines = %d",pStackCfg->numOfTlsEngines));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - maxTlsSessions = %d",pStackCfg->maxTlsSessions));
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/
#if (RV_NET_TYPE & RV_NET_SCTP)
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "[=======SCTP parameters==============]"));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "PrintConfigParamsToLog - numOfSctpAddresses = %d",pStackCfg->numOfSctpAddresses));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "PrintConfigParamsToLog - sctpAvgNumOfMultihomingAddr = %d",pStackCfg->avgNumOfMultihomingAddresses));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "PrintConfigParamsToLog - numOfSctpOutStreams = %d",pStackCfg->numOfOutStreams));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "PrintConfigParamsToLog - numOfSctpInStreams = %d",pStackCfg->numOfInStreams));
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

#ifndef RV_SIP_JSR32_SUPPORT
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
              "[======Transaction behavior=============]"));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - bUseRportParamInVia = %d",pStackCfg->bUseRportParamInVia));

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - bDynamicInviteHandling = %d",pStackCfg->bDynamicInviteHandling));
#ifdef RV_SIP_AUTH_ON
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
              "PrintConfigParamsToLog - enableServerAuth = %d",pStackCfg->enableServerAuth));
#endif /* #ifdef RV_SIP_AUTH_ON */
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
              "PrintConfigParamsToLog - bDisableMerging = %d",pStackCfg->bDisableMerging));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
              "PrintConfigParamsToLog - bOldInviteHandling = %d",pStackCfg->bOldInviteHandling));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
              "PrintConfigParamsToLog - bResolveTelUrls = %d",pStackCfg->bResolveTelUrls));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
              "PrintConfigParamsToLog - strDialPlanSuffix = %s",
              (NULL == pStackCfg->strDialPlanSuffix) ? "NULL" : pStackCfg->strDialPlanSuffix));
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */ 
	
#ifndef RV_SIP_JSR32_SUPPORT
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
              "[======Call-Leg behavior================]"));
#ifndef RV_SIP_PRIMITIVES
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
             "PrintConfigParamsToLog - manualAckOn2xx = %d",pStackCfg->manualAckOn2xx));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
             "PrintConfigParamsToLog - manualPrack = %d",pStackCfg->manualPrack));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
              "PrintConfigParamsToLog - bEnableForking = %d",pStackCfg->bEnableForking));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
              "PrintConfigParamsToLog - forkedAckTrxTimeout = %d",pStackCfg->forkedAckTrxTimeout));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
              "PrintConfigParamsToLog - forked1xxTimerTimeout = %d",pStackCfg->forked1xxTimerTimeout));
#endif /*RV_SIP_PRIMITIVES */
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
             "PrintConfigParamsToLog - manualBehavior = %d",pStackCfg->manualBehavior));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - enableInviteProceedingTimeoutState = %d",pStackCfg->enableInviteProceedingTimeoutState));
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */
	
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "[======Multi-Threaded parameters========]"));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - numberOfProcessingThreads = %d",pStackCfg->numberOfProcessingThreads));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - processingQueueSize = %d",pStackCfg->processingQueueSize));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - numOfReadBuffers = %d",pStackCfg->numOfReadBuffers));

#ifndef RV_SIP_JSR32_SUPPORT
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "[======Timers configuration=============]"));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - retransmissionT1 = %d",pStackCfg->retransmissionT1));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - retransmissionT2 = %d",pStackCfg->retransmissionT2));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - generalLingerTimer = %d",pStackCfg->generalLingerTimer));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - inviteLingerTimer = %d",pStackCfg->inviteLingerTimer));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - provisionalTimer = %d",pStackCfg->provisionalTimer));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - retransmissionT4 = %d",pStackCfg->retransmissionT4));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - cancelGeneralNoResponseTimer = %d",pStackCfg->cancelGeneralNoResponseTimer));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - cancelInviteNoResponseTimer = %d",pStackCfg->cancelInviteNoResponseTimer));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - generalRequestTimeoutTimer = %d",pStackCfg->generalRequestTimeoutTimer));
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */

#ifndef RV_SIP_JSR32_SUPPORT
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                "[======Proxy Timers configuration========]"));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                "PrintConfigParamsToLog - isProxy = %d",pStackCfg->isProxy));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                "PrintConfigParamsToLog - proxy2xxRcvdTimer = %d",pStackCfg->proxy2xxRcvdTimer));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                "PrintConfigParamsToLog - proxy2xxSentTimer = %d",pStackCfg->proxy2xxSentTimer));
#endif /* #ifndef RV_SIP_JSR32_SUPPORT */ 
	
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "[======DNS parameters====================]"));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - maxElementsInSingleDnsList = %d",pStackCfg->maxElementsInSingleDnsList));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - maxDnsDomains = %d",pStackCfg->maxDnsDomains));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - maxDnsServers = %d",pStackCfg->maxDnsServers));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - maxDnsBuffLen = %d",pStackCfg->maxDnsBuffLen));

    if(pStackMgr->hRslvMgr != NULL)
    {
        SipResolverMgrPrintDnsParams(pStackMgr->hRslvMgr);
    }

#if !defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_JSR32_SUPPORT)
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
        "[======Extension support=================]"));
    if(pStackCfg->supportedExtensionList != NULL)
    {
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
            "PrintConfigParamsToLog - supportedExtensionList = %s",pStackCfg->supportedExtensionList));
    }
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - rejectUnsupportedExtensions = %d",pStackCfg->rejectUnsupportedExtensions));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - addSupportedListToMsg = %d",pStackCfg->addSupportedListToMsg));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "[======Session Timer behavior============]"));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - sessionExpires = %d",pStackCfg->sessionExpires));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - minSE = %d",pStackCfg->minSE));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintConfigParamsToLog - manualSessionTimer = %d",pStackCfg->manualSessionTimer));

#ifdef RV_SIP_SUBS_ON
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "[======Subscription behavior=============]"));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                "PrintConfigParamsToLog - maxSubscriptions = %d",pStackCfg->maxSubscriptions));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                "PrintConfigParamsToLog - subsAlertTimer = %d",pStackCfg->subsAlertTimer));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                "PrintConfigParamsToLog - subsNoNotifyTimer = %d",pStackCfg->subsNoNotifyTimer));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                "PrintConfigParamsToLog - subsAutoRefresh = %d",pStackCfg->subsAutoRefresh));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                "PrintConfigParamsToLog - bDisableRefer3515Behavior = %d",pStackCfg->bDisableRefer3515Behavior));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                "PrintConfigParamsToLog - enableSubsForking = %d",pStackCfg->bEnableSubsForking));
#endif /* #ifdef RV_SIP_SUBS_ON */
#endif /* #if !defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_JSR32_SUPPORT) */
#ifdef RV_SIGCOMP_ON
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "[======SigComp parameters================]"));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                "PrintConfigParamsToLog - eOutboundProxyCompression = %d",pStackCfg->eOutboundProxyCompression));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                "PrintConfigParamsToLog - maxCompartments = %d",pStackCfg->maxCompartments));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                "PrintConfigParamsToLog - sigCompTcpTimer = %d",pStackCfg->sigCompTcpTimer));
#endif

    PrintLogFiltersForAllModules(pStackMgr, pStackCfg);
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/


#if !defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_JSR32_SUPPORT)
/***************************************************************************
* StackConstructSupportedList
* ------------------------------------------------------------------------
* General: construct the supported list given by the configuration.
* ------------------------------------------------------------------------
* Arguments:
* Input: supportedList - string with the supported options.
* Output:pStackMgr     - pointer to the struct that holds the stack information
***************************************************************************/
static RvStatus RVCALLCONV StackConstructSupportedList(
                              IN    RvChar                *supportedList,
                              OUT   StackMgr              *pStackMgr)
{
    int size;
    int i;
    int numOfTokens = 1;
    int tokenSize = 0;
    int j;
    int tokenStart;



    if(supportedList == NULL)
    {
        return RV_OK;
    }
    size = (int)strlen(supportedList);

    /*count the "," inside the list*/
    for(i=0; i<size ; i++)
    {
        if(supportedList[i] == ',')
        {
            numOfTokens++;
        }
    }

    /*create an array of pointers to strings*/
    if (RV_OK != RvMemoryAlloc(NULL,numOfTokens * sizeof(RvChar*),pStackMgr->pLogMgr,(void**)&pStackMgr->supportedExtensionList))
    {
        return RV_ERROR_OUTOFRESOURCES;
    }
    memset(pStackMgr->supportedExtensionList, 0, numOfTokens*sizeof(RvChar*));

    j=0;
    tokenStart = 0;
    for(i=0; i<= size; i++)
    {
       /*search for "," or for the end of the string*/
       if(supportedList[i] == ',' || i == size)
       {
            tokenSize = i - tokenStart;
            if (RV_OK != RvMemoryAlloc(NULL,(tokenSize+1)*sizeof(RvChar),pStackMgr->pLogMgr,(void**)&pStackMgr->supportedExtensionList[j]))
            {
                return RV_ERROR_OUTOFRESOURCES;
            }
            memset(pStackMgr->supportedExtensionList[j],0,tokenSize+1);
            memcpy(pStackMgr->supportedExtensionList[j],supportedList+tokenStart,tokenSize);
            j++;
            tokenStart = i+1;
       }

    }
    pStackMgr->extensionListSize = numOfTokens;
    return RV_OK;
}

#endif /* #if !defined(RV_SIP_PRIMITIVES) && !defined(RV_SIP_JSR32_SUPPORT)  */

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
* GetModuleName
* ------------------------------------------------------------------------
* General: Get the string that describes the module's name
* ------------------------------------------------------------------------
* Arguments:
* Input: eModule - The module by enumeration
***************************************************************************/
static RvChar*  RVCALLCONV GetModuleName(IN   RvSipStackModule  eModule)
{
    /* The name should contain no more than 11 characters.
       Otherwise it will be truncated by Common Core Log code. */
    switch (eModule)
    {
    case RVSIP_CALL:
        return "CALL";
    case RVSIP_MESSAGE:
        return "MESSAGE";
    case RVSIP_TRANSACTION:
        return "TRANSACTION";
    case RVSIP_TRANSPORT:
        return "TRANSPORT";
    case RVSIP_PARSER:
        return "PARSER";
    case RVSIP_STACK:
        return "STACK";
    case RVSIP_MSGBUILDER:
        return "MSGBUILDER";
    case RVSIP_AUTHENTICATOR:
        return "AUTH";
    case RVSIP_SUBSCRIPTION:
        return "SUBS";
    case RVSIP_COMPARTMENT:
        return "COMPARTMENT";
    case RVSIP_TRANSMITTER:
        return "TRANSMITTER";
	case RVSIP_TRANSCCLIENT:
        return "TRANSC_CLIENT";
    case RVSIP_REGCLIENT:
        return "REG_CLIENT";
	case RVSIP_PUBCLIENT:
        return "PUB_CLIENT";
    case RVSIP_RESOLVER:
        return "RESOLVER";
	case RVSIP_SEC_AGREE:
        return "SEC AGREE";
    case RVSIP_SECURITY:
        return "SECURITY";
    case RVSIP_ADS_RLIST:
        return "RLIST";
    case RVSIP_ADS_RA:
        return "RA";
    case RVSIP_ADS_RPOOL:
        return "RPOOL";
    case RVSIP_ADS_HASH:
        return "HASH";
    case RVSIP_ADS_PQUEUE:
        return "PQUEUE";

    case RVSIP_CORE_SEMAPHORE:
        return "SEMA4";
    case RVSIP_CORE_MUTEX:
        return "MUTEX";
    case RVSIP_CORE_LOCK:
        return "LOCK";
    case RVSIP_CORE_MEMORY:
        return "MEMORY";
    case RVSIP_CORE_THREAD:
        return "THREAD";
    case RVSIP_CORE_QUEUE:
        return "QUEUE";
    case RVSIP_CORE_TIMER:
        return "TIMER";
    case RVSIP_CORE_TIMESTAMP:
        return "TIMESTAMP";
    case RVSIP_CORE_CLOCK:
        return "CLOCK";
    case RVSIP_CORE_TM:
        return "TM";
    case RVSIP_CORE_SOCKET:
        return "SOCKET";
    case RVSIP_CORE_PORTRANGE:
        return "PORT";
    case RVSIP_CORE_SELECT:
        return "SELECT";
    case RVSIP_CORE_HOST:
        return "HOST";
    case RVSIP_CORE_TLS:
        return "TLS";
    case RVSIP_CORE_ARES:
        return "ARES";
    case RVSIP_CORE_RCACHE:
        return "RCACHE";
    case RVSIP_CORE_EHD:
        return "EHD";
    case RVSIP_CORE_IMSIPSEC:
        return "IMSIPSEC";
    default:
        return "Undefined";
    }
}

/***************************************************************************
* PrintLogFiltersForAllModules
* ------------------------------------------------------------------------
* General: Print to the log the log-filters that are defined for every module
* ------------------------------------------------------------------------
* Arguments:
* Input: pStackMgr - Pointer to the stack instance
*        pStackCfg - The configuration structure
***************************************************************************/
static void RVCALLCONV PrintLogFiltersForAllModules(
                                        IN  StackMgr          *pStackMgr,
                                        IN  RvSipStackCfg      *pStackCfg)
{

    RvInt32 i;
    RvChar   strFilters[128];
    RvLogMessageType coreLogMask = RV_LOGLEVEL_NONE;
    RvLogMessageType adsLogMask = RV_LOGLEVEL_NONE;


    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "[================General Log Filters===================]"));

    /*print the default filter supplied by the application*/
    coreLogMask = SipCommonConvertSipToCoreLogMask(pStackCfg->defaultLogFilters);
    GetLogMaskAsString(coreLogMask,strFilters);
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                   "General Filters - default Filter = %s", strFilters));

    /*print the core filter supplied by the application*/
    coreLogMask = SipCommonConvertSipToCoreLogMask(pStackCfg->coreLogFilters);
    GetLogMaskAsString(coreLogMask,strFilters);
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                    "General Filters - core Filter    = %s", strFilters));

    /*print the core filter supplied by the application*/
    adsLogMask = SipCommonConvertSipToCoreLogMask(pStackCfg->adsLogFilters);
    GetLogMaskAsString(adsLogMask,strFilters);
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                    "General Filters - ads Filter     = %s", strFilters));


    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "[================Modules Log Filters===================]"));

    for(i = 0; i< STACK_NUM_OF_LOG_SOURCE_MODULES; i++)
    {
        coreLogMask = RvLogSourceGetMask(pStackMgr->pLogSourceArrey[i]);
        GetLogMaskAsString(coreLogMask,strFilters);
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                   "Module Filters - %-12s = %s", GetModuleName((RvSipStackModule)i), strFilters));

    }
}



/***************************************************************************
 * GetLogMaskAsString
 * ------------------------------------------------------------------------
 * General: Get the list of filters for this module
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: coreLogMask - core log mask
 * Output  strFilters - the log mask as a string.
 ***************************************************************************/
static void  RVCALLCONV  GetLogMaskAsString(
                                IN  RvLogMessageType coreLogMask,
                                OUT RvChar*     strFilters)
{
    RvInt32 stringLen;

    strcpy(strFilters,"");

    if(coreLogMask & RV_LOGLEVEL_INFO)
    {
        RvSprintf(strFilters,"%s%s", strFilters,"INFO,");
    }
    if(coreLogMask & RV_LOGLEVEL_DEBUG)
    {
        RvSprintf(strFilters,"%s%s", strFilters,"DEBUG,");
    }
    if(coreLogMask & RV_LOGLEVEL_WARNING)
    {
        RvSprintf(strFilters,"%s%s", strFilters, "WARN,");
    }
    if(coreLogMask & RV_LOGLEVEL_ERROR)
    {
        RvSprintf(strFilters,"%s%s", strFilters, "ERROR,");
    }
    if(coreLogMask & RV_LOGLEVEL_EXCEP)
    {
        RvSprintf(strFilters,"%s%s", strFilters, "EXCEP,");
    }
    if(coreLogMask & RV_LOGLEVEL_SYNC)
    {
        RvSprintf(strFilters,"%s%s", strFilters, "SYNC,");
    }
    if(coreLogMask & RV_LOGLEVEL_ENTER)
    {
        RvSprintf(strFilters,"%s%s", strFilters, "ENTER,");
    }
    if(coreLogMask & RV_LOGLEVEL_LEAVE)
    {
        RvSprintf(strFilters,"%s%s", strFilters, "LEAVE");
    }
    stringLen = (RvInt32)strlen(strFilters);
    if(0 != stringLen && strFilters[stringLen-1] == ',')
    {
        strFilters[stringLen-1] = '\0';
    }
}


/***************************************************************************
* PrintNumOfErrorExcepAndWarn
* ------------------------------------------------------------------------
* General: Print the summary of ERRORs, EXCEPs and WARNs during the run of
*          the stack
* ------------------------------------------------------------------------
* Arguments:
* Input: pStackMgr - Handle to the stack instance
***************************************************************************/
static void   RVCALLCONV PrintNumOfErrorExcepAndWarn(
                                            IN StackMgr    *pStackMgr)
{
    RvInt32 numOfError, numOfExcep, numOfWarn;
    numOfError = 0;
    numOfExcep = 0;
    numOfWarn  = 0;

    numOfError = RvLogGetMessageCount(pStackMgr->pLogMgr, RV_LOGID_ERROR);
    numOfExcep = RvLogGetMessageCount(pStackMgr->pLogMgr, RV_LOGID_EXCEP);
    numOfWarn  = RvLogGetMessageCount(pStackMgr->pLogMgr, RV_LOGID_WARNING);

    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintNumOfErrorExcepAndWarn - Total number of ERRORs printed to the log = %d", numOfError));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintNumOfErrorExcepAndWarn - Total number of EXCEPs printed to the log = %d", numOfExcep));
    RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
               "PrintNumOfErrorExcepAndWarn - Total number of WARNs printed to the log = %d", numOfWarn));

    if (0 == numOfError && 0 == numOfExcep && 0 == numOfWarn)
    {
        RvLogInfo(pStackMgr->pLogSrc,(pStackMgr->pLogSrc,
                  "PrintNumOfErrorExcepAndWarn - No ERROR/EXCEP/WARN were printed to the log"));
    }
}


#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE) */


/***************************************************************************
* CalculateMaxTimers
* ------------------------------------------------------------------------
* General: calculates what is the maximum number of timers needed for
*          stack operation.
*          Each transaction needs 2 timers:                  | X = TRANSACTIONS*2
*          Each subscription needs 2 timers                  | +   SUBSCRIPTON
*          Session timer require timer per callleg           |(?+) CALL-LEGS
*          Call-leg forking support require timer per callLeg|(?+) CALL-LEGS
*          Each connection has a safety timer                | +   CONNECTIONS
*          Each Tls session needs 1 safety timer             | +   TLS-SESSIONs
*          If we use deprecated core warppers, add timers    |(?+) DEPRECATED_MAX_TIMERS
*                                                              ----------------
*                                                                  return val
* ------------------------------------------------------------------------
* Arguments:
* Input: pStackCfg - pointer to stack config
*        pStackMgr - Handle to the stack instance
***************************************************************************/
static RvInt32 RVCALLCONV CalculateMaxTimers(
                                             IN RvSipStackCfg* pStackCfg,
                                             IN StackMgr*      pStackMgr)
{
    RvInt32 maxtimers = 0;
#ifndef RV_SIP_PRIMITIVES
    RvInt32 i=0;
#endif /* #ifndef RV_SIP_PRIMITIVES */
    maxtimers = pStackCfg->maxTransactions*2 + pStackCfg->maxConnections;

#ifndef RV_SIP_PRIMITIVES
#ifdef RV_SIP_SUBS_ON
    maxtimers += pStackCfg->maxSubscriptions*2; /* *2 for noNoitfy timer? */
#endif /* #ifdef RV_SIP_SUBS_ON */
    for(;i<pStackMgr->extensionListSize;i++)
    {
        if (strcmp(pStackMgr->supportedExtensionList[i],"timer")==0)
        {
            maxtimers += pStackCfg->maxCallLegs;
            break;
        }
    }
    if(pStackCfg->bEnableForking == RV_TRUE)
    {
        maxtimers += pStackCfg->maxCallLegs;
    }
#else
    RV_UNUSED_ARG(pStackMgr);
#endif /*RV_SIP_PRIMITIVES*/
#if (RV_TLS_TYPE==RV_TLS_OPENSSL)
    maxtimers += pStackCfg->maxTlsSessions;
#endif /*RV_SIP_PRIMITIVES*/

#if defined(RV_DEPRECATED_CORE)
    maxtimers += DEPRECATED_MAX_TIMERS;
#endif /*RV_DEPRECATED_CORE*/

    return maxtimers;
}

#ifdef __cplusplus
}
#endif

