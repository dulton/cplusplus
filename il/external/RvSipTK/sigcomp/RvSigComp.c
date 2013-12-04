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
 *                              <RvSigComp.c>
 *
 * The SigComp functions of the RADVISION SIP stack enable you to
 * create and manage SigComp Compartment objects, attach/detach to/from
 * compartments and control the compartment parameters.
 *
 *
 * SigComp Compartment API functions are grouped as follows:
 * The SigComp Compartment Manager API
 * ------------------------------------
 * The SigComp Compartment manager is in charge of all the compartments. It is used
 * to create new compartments.
 *
 * The SigComp Compartment API
 * ----------------------------
 * A compartment represents a SIP SigComp Compartment as defined in RFC3320.
 * This compartment unifies a group of SIP Stack objects such as CallLegs or
 * Transactions that is identify by a compartment ID when sending request
 * through a compressor entity. Using the API, the user can initiate compartments,
 * or destruct it. Functions to set and access the compartment fields are also
 * available in the API.
 *
 *    Author                         Date
 *    ------                        ------
 *    Ofer Goren                  20031100
 *    Gil Keini                   20040114
*********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIGCOMP_DEF.h"
#include "RvSigCompTypes.h"
#include "RvSigCompDyn.h"
#include "SigCompDynState.h"
#include "SigCompDynZTime.h"

#include "rvcbase.h"
#include "rvmemory.h"
#include "rvlog.h"
#include <string.h> /* for memset & memcpy */

#include "RvSigComp.h"
#include "RvSigCompVersion.h"
#include "SigCompCommon.h"
#include "SigCompMgrObject.h"
#include "SigCompCallbacks.h"


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

#include "rvexternal.h"

#define  DBCOUNT 1000
int      ratiodb[DBCOUNT];
int      totalcnt = 0;
#endif
/*SPIRENT_END*/
/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static RvStatus RVCALLCONV SigCompLogConstruct (IN RvSigCompCfg *pSigCompCfg,
                                                IN SigCompMgr   *pSigCompMgr);

static RvStatus RVCALLCONV SigCompLogDestruct (IN SigCompMgr *pSigCompMgr);

static RvLogMessageType RVCALLCONV SigCompLogMaskConvertToCommonCore (
                                                     IN RvInt32 sigCompFilter);

static void RVCALLCONV SigCompLogListenerDestruct (IN SigCompMgr *pSigCompMgr);

static RvStatus RVCALLCONV SigCompLogSourceConstruct (IN SigCompMgr *pSigCompMgr);

static void RVCALLCONV SigCompLogSourceDestruct (IN SigCompMgr *pSigCompMgr);

static void RVCALLCONV SigCompLogCB (IN RvLogRecord* pLogRecord,
                                     IN void*        context);

static RvSigCompLogFilters RVCALLCONV SigCompLogMaskConvertCommonToSigComp (
                                               IN RvLogMessageType coreFilter);

static void RVCALLCONV SigCompLogMsgFormat(IN  RvLogRecord  *logRecord,
                                           OUT RvChar       **strFinalText);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

static RvStatus SigCompSigCompMgrGetResources(IN  SigCompMgr            *pSCMgr,
                                              OUT RvSigCompMgrResources *pResources);


static void RVCALLCONV SigCompPrintConfigParamsToLog(IN SigCompMgr *pSCMgr);

static void SigCompAlgorithmListDestruct(IN SigCompMgr *pSigCompMgr);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
                                              

/*************************************************************************
* RvSigCompSHA1
* ------------------------------------------------------------------------
* General: Calculates the hash of the provided data, using SHA1.
*           This code implements the Secure Hashing Algorithm 1 as defined
*           in FIPS PUB 180-1 published April 17, 1995.
* Caveats:
*           SHA-1 is designed to work with messages less than 2^64 bits
*           long. Although SHA-1 allows a message digest to be generated
*           for messages of any number of bits less than 2^64, this
*           implementation only works with messages with a length that is
*           a multiple of the size of an 8-bit character.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pData - pointer the data buffer
*            dataSize - the size of the data buffer, in bytes
* output:   hash - an array, into which the 20 byte result will be inserted
*************************************************************************/

RVAPI RvStatus RVCALLCONV RvSigCompSHA1 (IN  RvUint8  *pData,
                                         IN  RvUint32 dataSize,
                                         OUT RvUint8  hash[20])
{
    RvStatus rv;

    if ((NULL == pData) || (0 == dataSize) || (NULL == hash))
    {
        /* illegal input */
        return(RV_ERROR_NULLPTR);
    }
    rv = SigCompSHA1Calc(pData, dataSize, hash);

    return(rv);
}/* end of RvSigCompSHA1 */


/*************************************************************************
* RvSigCompStreamToMessage
* ------------------------------------------------------------------------
* General: convert stream-based data into message-based format
*           Handles the 0xFF occurrence as per RFC-3320 section 4.2.2.
*           Usually, the end result (message data)  will be 2 to a few bytes smaller
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    streamDataSize - the size in bytes of the data in the buffer below
*           *pStreamData - a pointer to a buffer containing the stream data
*           *pMessageDataSize - the size in bytes of the buffer below
*
* Output:   *pMessageDataSize - the size in bytes of the data (used) in the buffer below
*           *pMessageData - a pointer to a buffer containing the message data
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompStreamToMessage(
                                   IN    RvUint32 streamDataSize,
                                   IN    RvUint8  *pStreamData,
                                   INOUT RvUint32 *pMessageDataSize,
                                   OUT   RvUint8  *pMessageData)
{
    RvUint8  *pPrevFfPosition    = pStreamData;
    RvUint8  *pCurrFfPosition    = pStreamData;
    RvUint8  *pCurrMssgPosition  = pMessageData;
    RvUint8  *pEndOfStreamBuffer = pStreamData + streamDataSize - 1;
    RvUint32 bytesLeftInDstBuffer;
    RvUint32 bytesLeftInSrcBuffer= streamDataSize;
    RvUint32 sizeOfDataToCopy    = 0;
    
    if ((NULL == pStreamData)|| 
        (streamDataSize == 0)||
        (NULL == pMessageData)||
        (NULL == pMessageDataSize))
    {
        return(RV_ERROR_NULLPTR);
    }
    bytesLeftInDstBuffer= *pMessageDataSize;
    
    while (pCurrFfPosition < pEndOfStreamBuffer) 
    {
        /* find the first occurrence of 0xFF in the stream */
        pCurrFfPosition = (RvUint8 *) memchr(pPrevFfPosition,0xFF,bytesLeftInSrcBuffer);
        if (pCurrFfPosition == NULL) 
        {
            /* Error - no 0xFF has been found, not a legal SigComp stream */
            return(RV_ERROR_BADPARAM);
        }
        
        sizeOfDataToCopy = (RvUint32)(pCurrFfPosition - pPrevFfPosition);
        
        if (bytesLeftInDstBuffer < sizeOfDataToCopy) 
        {
            /* not enough memory to complete the task */
            return(RV_ERROR_BADPARAM);
        }
        bytesLeftInSrcBuffer -= sizeOfDataToCopy;

        /* copy the data from stream to message up to 0xFF */
        memcpy(pCurrMssgPosition, pPrevFfPosition, sizeOfDataToCopy);
        pCurrMssgPosition += sizeOfDataToCopy;
        bytesLeftInDstBuffer -= sizeOfDataToCopy;
        
        if (pCurrFfPosition[1] == 0xFF)
        {
            /* reached end of SigComp message */
            break;
        }
        else if(pCurrFfPosition[1] > 0x7F)
        {
            /* illegal SigComp stream, discard data buffer */
            return(RV_ERROR_BADPARAM);
        }
        else
        {
            register RvUint8 size = pCurrFfPosition[1];
            *pCurrMssgPosition = 0xFF; pCurrMssgPosition++;
            /* copy xx bytes literally and move position of pointers */
            memcpy(pCurrMssgPosition, &pCurrFfPosition[2], size);
            pCurrMssgPosition += size;
            pPrevFfPosition = pCurrFfPosition + size + 2;
            bytesLeftInDstBuffer -= (size + 1);
            bytesLeftInSrcBuffer -= (size + 2);
        }
    } /* end of LOOP */
    
    *pMessageDataSize = (RvUint32)(pCurrMssgPosition - pMessageData);
    
    return(RV_OK);
} /* end of RvSigCompStreamToMessage */
  
/*************************************************************************
* RvSigCompMessageToStream
* ------------------------------------------------------------------------
* General: convert message-based data into stream-based format
*           Handles the 0xFF occurrence as per RFC-3320 section 4.2.2.
*           Usually, the end result (stream data)  will be 2 to a few bytes bigger
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    messageDataSize - the size in bytes of the data in the buffer above
*           *pMessageData - a pointer to a buffer containing the message data
*           *pStreamDataSize - the size in bytes of the buffer below
*
* Output:   *pStreamDataSize - the size in bytes of the data (used) in the buffer below
*           *pStreamData - a pointer to a buffer containing the stream data
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompMessageToStream(
                                   IN    RvUint32 messageDataSize,
                                   IN    RvUint8  *pMessageData,
                                   INOUT RvUint32 *pStreamDataSize,
                                   OUT   RvUint8  *pStreamData)
{
    RvUint8  *pPrevFfPosition    = pMessageData;
    RvUint8  *pCurrFfPosition    = pMessageData;
    RvUint8  *pCurrStrmPosition  = pStreamData;
    RvUint8  *pEndOfMessageBuffer= pMessageData + messageDataSize - 1;
    RvUint32 bytesLeftInDstBuffer;
    RvUint32 bytesLeftInSrcBuffer= messageDataSize;
    RvUint32 sizeOfDataToCopy    = 0;
    
    if ((NULL == pMessageData)|| 
        (messageDataSize == 0)||
        (NULL == pStreamData)||
        (NULL == pStreamDataSize))
    {
        return(RV_ERROR_NULLPTR);
    }

    if (pMessageData == pStreamData) 
    {
        /* inplace processing not allowed */
        return(RV_ERROR_BADPARAM);
    }
    
    bytesLeftInDstBuffer= *pStreamDataSize;

    while (pCurrFfPosition <= pEndOfMessageBuffer) 
    {
        /* find the first occurrence of 0xFF in the stream */
        pCurrFfPosition = (RvUint8 *)memchr(pPrevFfPosition,0xFF,bytesLeftInSrcBuffer);
        
        if (pCurrFfPosition == NULL)
        {
            /* reached end of SigComp message */
            sizeOfDataToCopy = bytesLeftInSrcBuffer;
            bytesLeftInSrcBuffer = 0;
        }
        else 
        {
            sizeOfDataToCopy = (RvUint32)(pCurrFfPosition - pPrevFfPosition);
            bytesLeftInSrcBuffer -= sizeOfDataToCopy;
        }
        
        if (bytesLeftInDstBuffer < sizeOfDataToCopy) 
        {
            /* not enough memory to complete the task */
            return(RV_ERROR_BADPARAM);
        }
        /* copy the data from message to stream */
        memcpy(pCurrStrmPosition, pPrevFfPosition, sizeOfDataToCopy);
        pCurrStrmPosition += sizeOfDataToCopy;
        
        if (pCurrFfPosition == NULL)
        {
            /* reached end of SigComp message */
            break;
        }
        else
        {
            register RvUint8 sz = 127;
            if (bytesLeftInSrcBuffer < 127) 
            {
                sz = (RvUint8)bytesLeftInSrcBuffer;
            }
            if (bytesLeftInDstBuffer < (2 + (RvUint32)sz)) 
            {
                /* not enough memory to complete the task */
                return(RV_ERROR_BADPARAM);
            }
            
            *pCurrStrmPosition = 0xFF; pCurrStrmPosition++;
            *pCurrStrmPosition = (RvUint8) (sz - 1); pCurrStrmPosition++;
            
            /* copy xx bytes literally and move position of pointers */
            memcpy(pCurrStrmPosition, &pCurrFfPosition[1], sz-1);
            pCurrStrmPosition += (sz - 1);
            pCurrFfPosition += sz;
            pPrevFfPosition = pCurrFfPosition;
            bytesLeftInDstBuffer -= (sz + 2);
            bytesLeftInSrcBuffer -= sz;
        }
    } /* end of LOOP */
    
    if (bytesLeftInDstBuffer < 2) 
    {
        /* not enough memory to complete the task */
        return(RV_ERROR_BADPARAM);
    }
    
    *pCurrStrmPosition = 0xFF; pCurrStrmPosition++;
    *pCurrStrmPosition = 0xFF; pCurrStrmPosition++;
    
    *pStreamDataSize = (RvUint32)(pCurrStrmPosition - pStreamData);
    
    return(RV_OK);
} /* end of RvSigCompMessageToStream */
 
/************************************************************************************
* RvSigCompInitCfg
* ----------------------------------------------------------------------------------
* General: Initializes the RvSigCompCfg structure. This structure is given to the
*          RvSigCompConstruct function and it is used to initialize the SigComp module.
*          The RvSigCompInitCfg function relates to two types of parameters found in
*          the RvSigCompCfg structure:
*          (A) Parameters that influence the value of other parameters
*          (B) Parameters that are influenced by the value of other parameters.
*          The RvSigCompInitCfg will set all parameters of type A to default values
*          and parameters of type B to -1.
*
*          When calling the RvSigCompConstruct function all the B type parameters
*          will be calculated using the values found in the A type parameters.
*          if you change the A type values the B type values will be changed
*          accordingly.
*
* Return Value: void
* ----------------------------------------------------------------------------------
* Arguments:
* Input:  sizeOfCfg - The size of the configuration structure.
*
* output: pSigCompCfg - The configuration structure containing the RADVISION SigComp default values.
***********************************************************************************/
RVAPI void RvSigCompInitCfg(IN  RvUint32     sizeOfCfg,
                            OUT RvSigCompCfg *pSigCompCfg)
{
    RvSigCompCfg internalCfg;

    if ((NULL == pSigCompCfg)||
        (0 == sizeOfCfg) ||
        (sizeOfCfg > sizeof(RvSigCompCfg)))
    {
        return;
    }
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    memset(ratiodb,0,sizeof(ratiodb));
	totalcnt = 0;
#endif
/* SPIRENT_END */

    memset (pSigCompCfg, 0, sizeOfCfg);
    memset (&internalCfg, 0, sizeof(internalCfg));

    internalCfg.maxOpenCompartments   = DEFAULT_SC_OPEN_COMPARTMENTS;
    internalCfg.maxTempCompartments   = DEFAULT_SC_TEMP_COMPARTMENTS;
    internalCfg.maxStatesPerCompartment    = DEFAULT_SC_MAX_STATES_PER_COMP;
    internalCfg.timeoutForTempCompartments = DEFAULT_SC_TIMEOUT;
    internalCfg.bStaticDic3485Mandatory    = RV_TRUE;
    internalCfg.decompMemSize         = DEFAULT_SC_DECMOP_MEM_SIZE;
    internalCfg.stateMemSize          = DEFAULT_SC_STATE_MEM_SIZE;
    internalCfg.cyclePerBit           = DEFAULT_SC_CYCLES;
    internalCfg.smallBufPoolSize      = DEFAULT_SC_SMALL_BUF_SIZE;
    internalCfg.smallPoolAmount       = DEFAULT_SC_SMALL_BUF_AMOUNT;
    internalCfg.mediumBufPoolSize     = DEFAULT_SC_MED_BUF_SIZE;
    internalCfg.mediumPoolAmount      = DEFAULT_SC_MED_BUF_AMOUNT;
    internalCfg.largeBufPoolSize      = DEFAULT_SC_LARGE_BUF_SIZE;
    internalCfg.largePoolAmount       = DEFAULT_SC_LARGE_BUF_AMOUNT;
    internalCfg.logHandle             = NULL;
    internalCfg.pfnLogEntryPrint      = NULL;
    internalCfg.logEntryPrintContext  = NULL;
    internalCfg.logFilter             = 
        RVSIGCOMP_LOG_INFO_FILTER  |
        RVSIGCOMP_LOG_ERROR_FILTER |
        RVSIGCOMP_LOG_DEBUG_FILTER |
        RVSIGCOMP_LOG_EXCEP_FILTER |
        RVSIGCOMP_LOG_WARN_FILTER  |
        RVSIGCOMP_LOG_LOCKDBG_FILTER;

	/* New version parameters */ 
	internalCfg.dynMaxTripTime		  = DEFAULT_SC_DYN_MAX_TRIP_TIME;
	internalCfg.dynMaxStatesNo		  = DEFAULT_SC_DYN_MAX_STATES_NO; 
	internalCfg.dynMaxTotalStatesSize = DEFAULT_SC_DYN_MAX_TOTAL_STATES_SIZE;
	
    memcpy (pSigCompCfg, &internalCfg, sizeOfCfg);
} /* end of RvSigCompInitCfg */

/*************************************************************************
* SigCompConstruct
* ------------------------------------------------------------------------
* General: SigComp module constructor, called on initialization of the module
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    sizeOfCfg   - The size of the configuration structure in bytes
*           pSigCompCfg - A pointer to a 'SipSigCompCfg' configuration structure
*
* Output    hSigCompMgr - A handle to the sigComp manager
*************************************************************************/


RVAPI RvStatus RVCALLCONV RvSigCompConstruct(IN  RvUint32            sizeOfCfg,
                                             IN  RvSigCompCfg       *pSigCompCfg,
                                             OUT RvSigCompMgrHandle *hSigCompMgr)
                                             
{
    SigCompMgr        *pSigCompMgr;
    RLIST_POOL_HANDLE algoListPoolHndl;
    RLIST_POOL_HANDLE dictListPoolHndl;
    RLIST_HANDLE      algoListHndl;
    RLIST_HANDLE      dictListHndl;
    RvStatus          rv;
    RvStatus          crv; /* will be used for common-core RvStatus values */
    RvSigCompCfg      internalCfgStruct;
    RvUint32          minCfgSize = 0;

    if ((NULL == pSigCompCfg) || (NULL == hSigCompMgr))
    {
        return (RV_ERROR_NULLPTR);
    }
    if ((pSigCompCfg->smallBufPoolSize > pSigCompCfg->mediumBufPoolSize) ||
        (pSigCompCfg->mediumBufPoolSize > pSigCompCfg->largeBufPoolSize) ||
        (pSigCompCfg->smallBufPoolSize > pSigCompCfg->largeBufPoolSize))
    {
        return(RV_ERROR_BADPARAM);
    }

    crv = RvCBaseInit ();
    if (RV_OK != crv)
    {
        return (CCS2SCS(crv));
    }

    /* for backward compatability we copy the configuration to an internal struct */
    RvSigCompInitCfg(sizeof(internalCfgStruct),&internalCfgStruct);
    minCfgSize = RvMin(((RvUint32)sizeOfCfg),(RvUint32)sizeof(internalCfgStruct));
    
    memcpy(&internalCfgStruct, pSigCompCfg, minCfgSize);
    

    
    /* allocate the SigCompMgr */
    crv = RvMemoryAlloc(NULL, /* NULL = default region */
                        sizeof(SigCompMgr),
                        (RvLogMgr *)pSigCompCfg->logHandle,
                        (void**)&pSigCompMgr);
    if (RV_OK != crv)
    {
        /* failed to allocate SigCompMgr */
        return (CCS2SCS(crv));
    }
    memset (pSigCompMgr, 0, sizeof(SigCompMgr));
    
	crv = RvMutexConstruct(pSigCompMgr->pLogMgr, &pSigCompMgr->lock);
    if (crv != RV_OK)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompConstruct - failed to construct SigCompMgr->lock mutex (crv=%d)",
            crv));
        RvSigCompDestruct((RvSigCompMgrHandle)pSigCompMgr);
        return (CCS2SCS(crv));
    }
    pSigCompMgr->pLock = &pSigCompMgr->lock;

	
    if (NULL != pSigCompCfg->logHandle)
    {
        /* logMgr provided by the application */
        pSigCompMgr->bLogMgrProvidedByAppl = RV_TRUE;
        pSigCompMgr->pLogMgr = (RvLogMgr*)pSigCompCfg->logHandle;
    }
    else
    {
        pSigCompMgr->bLogMgrProvidedByAppl = RV_FALSE;
    }

    /*lint -e{506} */
    pSigCompMgr->sigCompVersion             = RvMax(SIGCOMP_VERSION,1);
    pSigCompMgr->maxOpenCompartments        = RvMax(pSigCompCfg->maxOpenCompartments,1);
    pSigCompMgr->maxTempCompartments        = RvMax(pSigCompCfg->maxTempCompartments,1);
    pSigCompMgr->maxStatesPerCompartment    = RvMin(RvMax(pSigCompCfg->maxStatesPerCompartment,1),512);
    pSigCompMgr->timeoutForTempCompartments = RvMax(pSigCompCfg->timeoutForTempCompartments,10);
    pSigCompMgr->bStaticDic3485Mandatory    = pSigCompCfg->bStaticDic3485Mandatory;
    pSigCompMgr->decompMemSize              = RvMax(pSigCompCfg->decompMemSize,MIN_DECOMPRESSION_MEMORY_SIZE);
    pSigCompMgr->stateMemSize               = RvMax(pSigCompCfg->stateMemSize,MIN_STATE_MEMORY_SIZE);
    pSigCompMgr->cyclePerBit                = RvMax(pSigCompCfg->cyclePerBit,MIN_CYCLES_PER_BIT);
    pSigCompMgr->smallBufPoolSize           = RvMax(pSigCompCfg->smallBufPoolSize,8); /* to guard against RAI_ElementMinSize in RA_Construct (AdsRa.c) */
    pSigCompMgr->smallPoolAmount            = RvMax(pSigCompCfg->smallPoolAmount,1);
    pSigCompMgr->mediumBufPoolSize          = RvMax(pSigCompCfg->mediumBufPoolSize,8); /* to guard against RAI_ElementMinSize in RA_Construct (AdsRa.c) */
    pSigCompMgr->mediumPoolAmount           = RvMax(pSigCompCfg->mediumPoolAmount,1);
    pSigCompMgr->largeBufPoolSize           = RvMax(pSigCompCfg->largeBufPoolSize,8); /* to guard against RAI_ElementMinSize in RA_Construct (AdsRa.c) */
    pSigCompMgr->largePoolAmount            = RvMax(pSigCompCfg->largePoolAmount,1);
    pSigCompMgr->logEntryPrintContext       = pSigCompCfg->logEntryPrintContext;
    pSigCompMgr->logFilter                  = pSigCompCfg->logFilter;
	/* New version parameters */ 
	pSigCompMgr->dynMaxTripTime				= RvMax(pSigCompCfg->dynMaxTripTime,0);
	pSigCompMgr->dynMaxStatesNo				= RvMax(pSigCompCfg->dynMaxStatesNo,0);
	pSigCompMgr->dynMaxTotalStatesSize	    = RvMax(pSigCompCfg->dynMaxTotalStatesSize,0);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    rv = SigCompLogConstruct (pSigCompCfg, pSigCompMgr);
    if (RV_OK != rv)
    {
        RvSigCompDestruct((RvSigCompMgrHandle)pSigCompMgr);
        return (RV_ERROR_UNKNOWN);
    }
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */

    algoListPoolHndl = RLIST_PoolListConstruct (RVSIGCOMP_MAX_NUM_ALGORITHMS,
                                                1, /* one list */
                                                sizeof(RvSigCompAlgorithm),
                                                pSigCompMgr->pLogMgr,
                                                "SigCompAlgoListPool");
    if (NULL == algoListPoolHndl)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompConstruct - failed to construct algorithm list pool"));
        RvSigCompDestruct((RvSigCompMgrHandle)pSigCompMgr);
        return (RV_ERROR_NULLPTR);
    }

    dictListPoolHndl = RLIST_PoolListConstruct (RVSIGCOMP_MAX_NUM_DICTIONARIES,
                                                1, /* just one list */
                                                sizeof(RvSigCompDictionary),
                                                pSigCompMgr->pLogMgr,
                                                "SigCompDictListPool");

    if (NULL == dictListPoolHndl)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompConstruct - failed to construct dictionary list pool"));
        RvSigCompDestruct((RvSigCompMgrHandle)pSigCompMgr);
        return (RV_ERROR_NULLPTR);
    }

    algoListHndl = RLIST_ListConstruct (algoListPoolHndl);
    if (NULL == algoListHndl)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompConstruct - failed to construct algorithm list"));
        RvSigCompDestruct((RvSigCompMgrHandle)pSigCompMgr);
        return (RV_ERROR_NULLPTR);
    }

    dictListHndl = RLIST_ListConstruct (dictListPoolHndl);
    if (NULL == dictListHndl)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompConstruct - failed to construct dictionary list"));
        RvSigCompDestruct((RvSigCompMgrHandle)pSigCompMgr);
        return (RV_ERROR_NULLPTR);
    }

    pSigCompMgr->hDictionaryList     = dictListHndl;
    pSigCompMgr->hAlgorithmList      = algoListHndl;
    pSigCompMgr->hDictionaryListPool = dictListPoolHndl;
    pSigCompMgr->hAlgorithmListPool  = algoListPoolHndl;

    rv = SigCompCompressorDispatcherConstruct ((RvSigCompMgrHandle)pSigCompMgr);
    if (rv != RV_OK)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompConstruct - failed to construct CompressorDispatcher (rv=%d)",
            rv));
        RvSigCompDestruct((RvSigCompMgrHandle)pSigCompMgr);
        return (rv);
    }

    rv = SigCompDecompressorDispatcherConstruct ((RvSigCompMgrHandle)pSigCompMgr);
    if (rv != RV_OK)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompConstruct - failed to construct DecompressorDispatcher (rv=%d)",
            rv));
        RvSigCompDestruct((RvSigCompMgrHandle)pSigCompMgr);
        return (rv);
    }

    rv = SigCompCompartmentHandlerConstruct((RvSigCompMgrHandle)pSigCompMgr);
    if (rv != RV_OK)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompConstruct - failed to construct CompartmentHandler (rv=%d)",
            rv));
        RvSigCompDestruct((RvSigCompMgrHandle)pSigCompMgr);
        return (rv);
    }

    rv = SigCompStateHandlerConstruct ((RvSigCompMgrHandle)pSigCompMgr);
    if (rv != RV_OK)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompConstruct - failed to construct StateHandler (rv=%d)",
            rv));
        RvSigCompDestruct((RvSigCompMgrHandle)pSigCompMgr);
        return (rv);
    }

    *hSigCompMgr = (RvSigCompMgrHandle)pSigCompMgr;

    SigCompPrintConfigParamsToLog(pSigCompMgr);

    RvLogInfo(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
        "RvSigCompConstruct - sigcomp module version %s was constructed successfully",
        SIGCOMP_MODULE_VERSION));

    return (RV_OK);
} /* end of SipSigCompConstruct */

/*************************************************************************
* RvSigCompDestruct
* ------------------------------------------------------------------------
* General: SigComp module destructor, called on terminating of the module
*
* Return Value: void
* ------------------------------------------------------------------------
* Arguments:
* Input:    hSigCompMgr - A handle to the sigComp manager
*************************************************************************/
RVAPI void RVCALLCONV RvSigCompDestruct(IN RvSigCompMgrHandle hSigCompMgr)
{
#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
    RvInt32     dummyCounter = 0;
#endif /*#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)*/
    SigCompMgr  *pSigCompMgr = (SigCompMgr *)hSigCompMgr;
    
    if (NULL == pSigCompMgr)
    {
        return;
    }

    if (pSigCompMgr->pLogSource != NULL) 
    {
        RvLogEnter(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"RvSigCompDestruct(mgr=0x%x)",hSigCompMgr));
    }

    if (pSigCompMgr->pLock != NULL) 
    {
        RvMutexLock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    }
    
    /* destruct entities */
    SigCompStateHandlerDestruct (&pSigCompMgr->stateHandlerMgr);
    SigCompCompartmentHandlerDestruct (&pSigCompMgr->compartmentHandlerMgr);
    SigCompDecompressorDispatcherDestruct (&pSigCompMgr->decompressorDispMgr);
    SigCompCompressorDispatcherDestruct (&pSigCompMgr->compressorDispMgr);

    /* destruct Manager allocations */
    if (NULL != pSigCompMgr->hAlgorithmList)
    {
		SigCompAlgorithmListDestruct(pSigCompMgr); 
    }
    if (NULL != pSigCompMgr->hDictionaryList)
    {
        RLIST_ListDestruct (pSigCompMgr->hDictionaryListPool, pSigCompMgr->hDictionaryList);
    }
    if (NULL != pSigCompMgr->hAlgorithmListPool)
    {
        RLIST_PoolListDestruct (pSigCompMgr->hAlgorithmListPool);
    }
    if (NULL != pSigCompMgr->hDictionaryListPool)
    {
        RLIST_PoolListDestruct (pSigCompMgr->hDictionaryListPool);
    }

    /* destruct lock */
    if (pSigCompMgr->pLock != NULL) 
    {
        RvMutexRelease(&pSigCompMgr->lock, pSigCompMgr->pLogMgr, &dummyCounter);
        RvMutexDestruct(&pSigCompMgr->lock, pSigCompMgr->pLogMgr);
    }
    
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    /* ignore irrelevant (in this context) return value */
    /*lint -e{534}*/
    SigCompLogDestruct (pSigCompMgr);
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */

    /* ignoring return values since there is nothing to do with it in the destructor */
    RvMemoryFree(pSigCompMgr,NULL); /*lint !e534*/
    RvCBaseEnd (); /*lint !e534*/

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
	{
		int i,count = 0;
		int averatio = 0;
		for(i = 0;i< DBCOUNT;i++)
		{	
			if(ratiodb[i] > 0)
			{
				averatio += ratiodb[i];
				count++;
			}			
		}
		if(count)
			averatio /= count;
		dprintf("averatio = %d count %d\n",averatio,count);
	}
#endif
/* SPIRENT_END */
    return;
} /* end of SigCompDestruct */

/************************************************************************************
* RvSigCompGetResources
* ----------------------------------------------------------------------------------
* General: Gets the status of resources used by the RADVISION SigComp module.
*
* Return Value: RvStatus
* ----------------------------------------------------------------------------------
* Arguments:
* Input:   hSigCompMgr       - Handle to the RADVISION SigComp module object.
*
* Output:  pSCMResources     - The resources in use by the whole SigComp module
***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompGetResources(
                                   IN  RvSigCompMgrHandle       hSigCompMgr,
                                   OUT RvSigCompModuleResources *pSCMResources)
{
    SigCompMgr  *pSigCompMgr = (SigCompMgr *)hSigCompMgr;
    RvStatus    rv;
    
    if ((NULL == hSigCompMgr) ||
        (NULL == pSCMResources))
    {
        return (RV_ERROR_NULLPTR);
    }
    RvMutexLock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    RvLogEnter(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
		"RvSigCompGetResources(mgr=0x%x)",hSigCompMgr));
    
    /* get the SigCompMgr resources report */
    rv = SigCompSigCompMgrGetResources(pSigCompMgr, &pSCMResources->sigCompMgrResources);
    if (RV_OK != rv) 
    {
        RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompGetResources - failed to get resource report from SigCompMgr (rv=%d)",
            rv));
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return(rv);
    }
    /* get the StateHandler resources report */
    rv = SigCompStateHandlerGetResources(&pSigCompMgr->stateHandlerMgr,
                                         &pSCMResources->stateHandlerResources);
    if (RV_OK != rv) 
    {
        RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompGetResources - failed to get resource report from StateHandler (rv=%d)",
            rv));
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return(rv);
    }
    /* get the DecompressorDispatcher resources report */
    rv = SigCompDecompressorDispatcherGetResources(&pSigCompMgr->decompressorDispMgr,
                                                   &pSCMResources->decompressorDispatcherResources);
    if (RV_OK != rv) 
    {
        RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompGetResources - failed to get resource report from DecompressorDispatcher (rv=%d)",
            rv));
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return(rv);
    }
    /* get the CompartmentHandler resources report */
    rv = SigCompCompartmentHandlerGetResources(&pSigCompMgr->compartmentHandlerMgr,
                                               &pSCMResources->compartmentHandlerResources);
    if (RV_OK != rv) 
    {
        RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompGetResources - failed to get resource report from CompartmentHandler (rv=%d)",
            rv));
    }
    else 
    {
        RvLogLeave(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"RvSigCompGetResources(mgr=0x%x)",hSigCompMgr));
    }
    
    RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    return (rv);
} /* end of RvSigCompGetResources */


/*************************************************************************
* RvSigCompCloseCompartment
* ------------------------------------------------------------------------
* General: Close session and delete it's memory compartment
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    hSigCompMgr - A handle to the sigComp manager
*           hCompartment - The handle of the session's compartment
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompCloseCompartment(
                                   IN RvSigCompMgrHandle         hSigCompMgr,
                                   IN RvSigCompCompartmentHandle hCompartment)
{
    SigCompMgr  *pSigCompMgr = (SigCompMgr *)hSigCompMgr;
    RvStatus    rv;
    
    if (NULL == hSigCompMgr)
    {
        return (RV_ERROR_NULLPTR);
    }
    RvMutexLock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    RvLogEnter(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
		"RvSigCompCloseCompartment(mgr=0x%x,comp=0x%x)",hSigCompMgr,hCompartment));

    if (NULL == hCompartment)
    {
        RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompCloseCompartment - hCompartment == NULL"));
        RvLogLeave(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,"RvSigCompCloseCompartment"));
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return (RV_OK);
    }
    
    RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
        "RvSigCompCloseCompartment - deleting hCompartment =0x%x",
        hCompartment));

    rv = SigCompCompartmentHandlerDeleteComprtment (&pSigCompMgr->compartmentHandlerMgr, hCompartment);

    if (RV_OK != rv) 
    {
        RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompCloseCompartment - failed to delete compartment (rv=%d)",
            rv));
    }
    else 
    {
        RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompCloseCompartment - compartment 0x%x deleted",hCompartment));
        RvLogLeave(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"RvSigCompCloseCompartment(mgr=0x%x,comp=0x%x)",hSigCompMgr,hCompartment));
    }
    
    RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    return (rv);
} /* RvSigCompCloseCompartment */

/*************************************************************************
* RvSigCompDeclareCompartment
* ------------------------------------------------------------------------
* General: The SIP stack declares to which memory compartment ("location")
*          the decompressed message (state variables) should be correlated.
*          The message is identified by its unique ID (number).
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    hSigCompMgr - A handle to the sigComp manager
*           unqID		   - a unique ID (number) of the message processed 
*						     by the stack
*			algoName       - The algorithm name which will affect the 
*						     compartment compression manner. The algorithm
*						     name MUST be equal to the set algorithm name 
*						     during algorithm initialization. If this
*						     parameter value is NULL the default algo is used
*           *phCompartment - pointer to a handle of the memory compartment
*                               Pass zero (0) value for new compartment
*                               Pass one (1) value if message is not "legal"
*
* Output:   *phCompartment - pointer to a handle of the memory compartment
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompDeclareCompartmentEx(
                                    IN RvSigCompMgrHandle            hSigCompMgr,
                                    IN RvInt32                       unqID,
                                    IN RvChar                        *algoName,
                                    INOUT RvSigCompCompartmentHandle *phCompartment)
{
    SigCompMgr                  *pSigCompMgr  = (SigCompMgr *)hSigCompMgr;
    SigCompCompartment          *pCompartment = NULL;
    RvSigCompCompartmentHandle  hCompartment  = (RvSigCompCompartmentHandle) *phCompartment;
    RvStatus                    rv;
    

    if (NULL == hSigCompMgr)
    {
        return (RV_ERROR_NULLPTR);
    }
    RvLogEnter(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
		"RvSigCompDeclareCompartmentEx(mgr=0x%x,id=0x%x,comp=0x%p)",hSigCompMgr,unqID,hCompartment));

    if (0 == unqID) 
    {
        return(RV_OK);
    }

    RvMutexLock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    if (hCompartment == NULL)
    {
        /* must create a new compartment */
        rv = SigCompCompartmentHandlerCreateComprtmentEx(&pSigCompMgr->compartmentHandlerMgr, algoName,
                                                       &pCompartment);
        if (rv != RV_OK)
        {
            /* failed to create compartment */
            RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
                "RvSigCompDeclareCompartmentEx - failed to create compartment by unqID %d (rv=%d)",
                unqID,rv));
            RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
            return(rv);
        }
        /* compartment created and locked */
        RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompDeclareCompartmentEx - created a new compartment 0x%p by unqID %d",
            pCompartment,unqID));
    }
    else if ((RvUintPtr)hCompartment != RVSIGCOMP_UNAUTHORIZED_COMPARTMENT_ID)
    {
        /* use the supplied handle as a pointer to the compartment */
        rv = SigCompCompartmentHandlerGetComprtment(&pSigCompMgr->compartmentHandlerMgr,
                                                    hCompartment,
                                                    &pCompartment);
        if (RV_OK != rv)
        {
            RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
                "RvSigCompDeclareCompartmentEx - failed to get compartment 0x%p by unqID %d (rv=%d)",
                hCompartment,unqID,rv));
            RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
            return (rv);
        }
        /* compartment fetched and locked */
        RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompDeclareCompartmentEx - fetched compartment 0x%p by unqID %d (hCompartment=0x%p)",
            pCompartment,unqID,hCompartment));
    }
    else if ((RvUintPtr)hCompartment == RVSIGCOMP_UNAUTHORIZED_COMPARTMENT_ID)
    {
        /* message was is not authorized and all the related info must be discarded */
        pCompartment = 0;
        RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompDeclareCompartmentEx - unauthorized compartment (unqID=%d)",unqID));
    }

    RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
        "RvSigCompDeclareCompartmentEx - hCompartment=0x%p (pCompartment=0x%p) uniqueID=%d",
        hCompartment, pCompartment, unqID));
    rv = SigCompDecompressorDispatcherDeclareCompartment(&pSigCompMgr->decompressorDispMgr, 
                                                         pCompartment,
                                                         unqID);
    if (RV_OK != rv) 
    {
        RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompDeclareCompartmentEx - failed to declare hCompartment 0x%p (pComp=0x%p) (rv=%d)",
            hCompartment,pCompartment,rv));
		if (hCompartment == NULL)
		{
			/* If a new compartment was created then it should be deleted */ 
			SigCompCompartmentHandlerDeleteComprtment(
				&pSigCompMgr->compartmentHandlerMgr,(RvSigCompCompartmentHandle)pCompartment);
            /* Unlock the compartment the new compartment.
               It was locked by SigCompCompartmentHandlerCreateComprtmentEx */
            SigCompCompartmentHandlerSaveComprtment(
                &pSigCompMgr->compartmentHandlerMgr,pCompartment);
		}
		else if ((RvUintPtr)hCompartment != RVSIGCOMP_UNAUTHORIZED_COMPARTMENT_ID)
		{
			/* Just unlock the compartment, the return value of this operation not relevant */
			/*lint -e{534}*/
			SigCompCompartmentHandlerSaveComprtment(
				&pSigCompMgr->compartmentHandlerMgr,pCompartment);
		}
        
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return (rv);
    }

    if (pCompartment != 0) 
    {
        /* save compartment only if used */
        rv = SigCompCompartmentHandlerSaveComprtment(&pSigCompMgr->compartmentHandlerMgr,
            pCompartment);
        if (RV_OK != rv)
        {
            RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
                "RvSigCompDeclareCompartmentEx - failed to save compartment 0x%p (pComp=0x%p) (rv=%d)",
                hCompartment,pCompartment,rv));
            RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
            return (rv);
        }
        RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompDeclareCompartmentEx - temporary compartment saved (hCompartment=0x%x,pComp=0x%p)",
            hCompartment,pCompartment));
    }
    
    *phCompartment = (RvSigCompCompartmentHandle)pCompartment;

    RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    
    RvLogLeave(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
		"RvSigCompDeclareCompartmentEx(mgr=0x%x,id=0x%x,comp=0x%p,0x%p)",hSigCompMgr,unqID,hCompartment,*phCompartment));
    return (RV_OK);
} /* RvSipSigCompDeclareCompartment */


/*************************************************************************
* RvSigCompCompressMessage
* ------------------------------------------------------------------------
* General: Compress a message. To be called before sending.
*           If the compartment handle is NULL, this means a new compartment should
*           be created, otherwise, use the handle to access the compartment.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    hSigCompMgr - A handle to the sigComp manager
*			algoName    - The algorithm name which will affect the 
*						  compression manner. This parameter is influencing 
*						  only when *phCompartment equals NULL. The algorithm
*						  name MUST be equal to the set algorithm name  
*						  during algorithm initialization. If this
*						  parameter value is NULL the default algo is used
*           *phCompartment - a pointer to a handle to the memory compartment
*           *pMessageInfo - a pointer to 'SipSigCompMessageInfo' structure,
*                           holds the plain message + its size
* Output:   *phCompartment - return a handle to the compartment used
*           *pMessageInfo - a pointer to 'SipSigCompMessageInfo' structure,
*                           holds the compressed message + its size
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompCompressMessageEx(
                               IN    RvSigCompMgrHandle         hSigCompMgr,
                               IN    RvChar                     *algoName,
                               INOUT RvSigCompCompartmentHandle *phCompartment,
                               INOUT RvSigCompMessageInfo       *pMessageInfo)
{
    SigCompMgr                  *pSigCompMgr  = (SigCompMgr *)hSigCompMgr;
    SigCompCompartment          *pCompartment = NULL;
    RvSigCompCompartmentHandle  hCompartment  = (RvSigCompCompartmentHandle) *phCompartment;
    RvStatus                    rv;

    if ((NULL == hSigCompMgr) ||
        (NULL == pMessageInfo))
    {
        return (RV_ERROR_NULLPTR);
    }
	RvMutexLock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    RvLogEnter(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
		"RvSigCompCompressMessageEx(mgr=0x%x,comp=0x%x)",hSigCompMgr,*phCompartment));
    
    if (NULL == hCompartment)
    {
        /* must create a new compartment */
        rv = SigCompCompartmentHandlerCreateComprtmentEx(&pSigCompMgr->compartmentHandlerMgr, 
                                                          algoName, 
                                                         &pCompartment);
        if (rv != RV_OK)
        {
            /* failed to create compartment */
            RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
                "RvSigCompCompressMessageEx - failed to create compartment (rv=%d)",
                rv));
			RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
            return(rv);
        }
        RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompCompressMessageEx - created new hCompartment =0x%x",
            pCompartment));
    }
    else
    {
        /* use the supplied handle as a pointer to the compartment */
        rv = SigCompCompartmentHandlerGetComprtment(&pSigCompMgr->compartmentHandlerMgr,
                                                    hCompartment,
                                                    &pCompartment);
        if (RV_OK != rv)
        {
            RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
                "RvSigCompCompressMessageEx - failed to get compartment (rv=%d)",
                rv));
			RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
            return (rv);
        }
        RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompCompressMessageEx - using fetched hCompartment =0x%x",
            pCompartment));
    }


    rv = SigCompCompressorDispatcherCompress (&pSigCompMgr->compressorDispMgr,
                                              pCompartment,
                                              pMessageInfo);
    if (RV_OK != rv) 
    {
        RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompCompressMessageEx - failed to compress (rv=%d)",
            rv));
		RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
		return (rv);
    }

    rv = SigCompCompartmentHandlerSaveComprtment(&pSigCompMgr->compartmentHandlerMgr,
                                                 pCompartment);
    if (RV_OK != rv)
    {
        RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompCompressMessageEx - failed to save compartment (rv=%d)",
            rv));
		RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return (rv);
    }
    
    *phCompartment = (RvSigCompCompartmentHandle)pCompartment;
    
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
	{
		RvUint32 ratio = 0;
		if(pMessageInfo->plainMessageBuffSize)
			ratio = pMessageInfo->compressedMessageBuffSize*100/pMessageInfo->plainMessageBuffSize;
		if(totalcnt >= DBCOUNT)
			totalcnt = 0; //wrap back		    		
        ratiodb[totalcnt++] = ratio;
     }
#endif
/* SPIRENT_END */
    RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
        "RvSigCompCompressMessageEx - message compressed (plainMessageSize=%d compressedMessageSize=%d)",
        pMessageInfo->plainMessageBuffSize,
        pMessageInfo->compressedMessageBuffSize));
    RvLogDebug(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
        "RvSigCompCompressMessageEx - compressed message assumes remote state (partialStateIdLengthCode=%d)",
        (pMessageInfo->pCompressedMessageBuffer[0] & 0x03)));
    RvLogLeave(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
		"RvSigCompCompressMessageEx(mgr=0x%x,comp=0x%x)",hSigCompMgr,*phCompartment));
	RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);

    return (rv);
} /* end of RvSigCompCompressMessage */

/*************************************************************************
* RvSigCompDecompressMessage
* ------------------------------------------------------------------------
* General: Decompress a message. To be called after receiving a compressed message.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    hSigCompMgr - A handle to the sigComp manager
*           *pMessageInfo - a pointer to 'SipSigCompMessageInfo' structure,
*                           holds the plain message + its size
*
* Output:   *pMessageInfo - a pointer to 'SipSigCompMessageInfo' structure,
*                           holds the compressed message + its size
*           *pUnqId - a unique identifier (number) for use in the
*                        SigCompDeclareCompartment() function which must follow
*                        from the stack to the SigComp
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompDecompressMessage(
                               IN    RvSigCompMgrHandle   hSigCompMgr,
                               INOUT RvSigCompMessageInfo *pMessageInfo,
                               OUT   RvUint32             *pUnqId)
{
    SigCompMgr  *pSigCompMgr = (SigCompMgr *)hSigCompMgr;
    RvBool      outputFlag;
    RvStatus    rv;

    if ((NULL == hSigCompMgr) ||
        (NULL == pMessageInfo) ||
        (NULL == pUnqId))
    {
        return (RV_ERROR_NULLPTR);
    }
    RvMutexLock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    RvLogEnter(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
		"RvSigCompDecompressMessage(mgr=0x%x)",hSigCompMgr));
    
    rv = SigCompDecompressorDispatcherDecompressMessage (&pSigCompMgr->decompressorDispMgr,
                                                         pMessageInfo, 
                                                         &outputFlag, 
                                                         pUnqId);

    if (RV_OK != rv) 
    {
        RvLogWarning(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompDecompressMessage - failed to decompress (rv=%d)",
            rv));
    }
    else 
    {
        RvLogLeave(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"RvSigCompDecompressMessage(mgr=0x%x,id=0x%x)",hSigCompMgr,*pUnqId));
    }
    RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    return (rv);
} /* end of RvSigCompDecompressMessage */

/***************************************************************************
 * RvSigCompDictionaryAdd
 * ------------------------------------------------------------------------
 * General: adds an a dictionary to the list of dictionaries
 *
 * Return Value: RV_OK -        Success.
 *               RV_ERROR_UNKNOWN -        Failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSigCompMgr - a handle to the sigComp manager
 *        pDictionary - the dictionary to add
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompDictionaryAdd(IN RvSigCompMgrHandle  hSigCompMgr,
                                                 IN RvSigCompDictionary *pDictionary)
{
    SigCompMgr        *pSigCompMgr = (SigCompMgr *)hSigCompMgr;
    RLIST_ITEM_HANDLE hElement;
    SigCompState      *pState;
    RvStatus          rv;

    if ((NULL == hSigCompMgr) ||
        (NULL == pDictionary))
    {
        return (RV_ERROR_NULLPTR);
    }
    RvMutexLock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    RvLogEnter(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
		"RvSigCompDictionaryAdd(mgr=0x%x)",hSigCompMgr));
    
    /* create a state especially for the dictionary (w/o allocating data buffer) */
    rv = SigCompStateHandlerCreateStateForDictionary(&(pSigCompMgr->stateHandlerMgr),
                                                     pDictionary->dictionarySize,
                                                     &pState);
    if (RV_OK != rv)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompDictionaryAdd - failed to create state for dictionary (rv=%d)",
            rv));
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return (rv);
    }

    /* set the state data pointer to the "external" data pointer (the application handles this memory) */
    pState->pData = pDictionary->dictionary;

    rv = SigCompStateHandlerStateSetPriority(&(pSigCompMgr->stateHandlerMgr),
                                             (RV_UINT16_MAX), 
                                             pState);

    if (RV_OK != rv)
    {
        /* ignore irrelevant (in this context) return value */
        /*lint -e{534}*/
        SigCompStateHandlerDeleteState (&pSigCompMgr->stateHandlerMgr, pState, RV_FALSE);
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompDictionaryAdd - failed to set state priority (rv=%d)",
            rv));
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return (rv);
    }

    rv = SigCompStateHandlerStateSetID(&pSigCompMgr->stateHandlerMgr, pState);
    if (RV_OK != rv)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompDictionaryAdd - failed to set state ID (rv=%d)",
            rv));
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return (rv);
    }

    rv = SigCompStateHandlerSaveState(&pSigCompMgr->stateHandlerMgr, NULL, NULL, &pState);
    if (RV_OK != rv)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompDictionaryAdd - failed to save state (rv=%d)",
            rv));
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return (rv);
    }

    rv = RLIST_InsertTail (pSigCompMgr->hDictionaryListPool,
                           pSigCompMgr->hDictionaryList,
                           &hElement);
    if (RV_OK == rv)
    {
        memcpy ((RvUint8 *)hElement, (RvUint8 *)pDictionary, sizeof (RvSigCompDictionary));
    }
    else
    {
        /* ignore irrelevant (in this context) return value */
        /*lint -e{534}*/
        SigCompStateHandlerDeleteState (&pSigCompMgr->stateHandlerMgr, pState, RV_TRUE);
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompDictionaryAdd - failed to insert state into state-list (rv=%d)",
            rv));
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return (rv);
    }

    /* guard the minimalAccessLength parameter */
    if (pDictionary->minimalAccessLength < 6) 
    {
        pDictionary->minimalAccessLength = 6;
    }
    else if (pDictionary->minimalAccessLength > 20) 
    {
        pDictionary->minimalAccessLength = 20;
    }
    /* update list of locally available states */
    pSigCompMgr->localIdList[pSigCompMgr->localIdListUsedLength] = (RvUint8)pDictionary->minimalAccessLength;
    pSigCompMgr->localIdListUsedLength++;
    memcpy((RvUint8 *)&(pSigCompMgr->localIdList[pSigCompMgr->localIdListUsedLength]), 
           (RvUint8 *)(pState->stateID),
           pDictionary->minimalAccessLength);
    pSigCompMgr->localIdListUsedLength += pDictionary->minimalAccessLength;

    RvLogLeave(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
		"RvSigCompDictionaryAdd(mgr=0x%x)",hSigCompMgr));
    RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    return (rv);
} /* end of RvSigCompDictionaryAdd */



/***************************************************************************
 * RvSigCompAlgorithmAdd
 * ------------------------------------------------------------------------
 * General: Adds an algorithm to the list of algorithms.
 *          The number of algorithms is limited to RvSIGCOMP_MAX_NUM_ALGORITHMS
 *          which is defined in the RvSigCompTypes header-file. 
 * NOTE: More than a single Algorithm might be added to the SigComp module.
 *       In this case the preceding algorithms are preferable when the 
 *       SigComp module determines, which algorithm will be used.
 *
 * Return Value: RV_OK            - Success.
 *               RV_ERROR_UNKNOWN - Failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSigCompMgr - a handle to the sigComp manager
 *        pAlgorithm  - the algorithm to add
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompAlgorithmAdd(IN RvSigCompMgrHandle hSigCompMgr,
                                                IN RvSigCompAlgorithm *pAlgorithm)
{
    SigCompMgr        *pSigCompMgr = (SigCompMgr *)hSigCompMgr;
    RLIST_ITEM_HANDLE hElement;
    SigCompState      *pState;
    RvStatus          rv;
	RvSigCompAlgorithm *phAlgorithm;

    if ((NULL == pAlgorithm) || 
        (NULL == hSigCompMgr))
    {
        return (RV_ERROR_NULLPTR);
    }
    
    RvMutexLock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    RvLogEnter(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
		"RvSigCompAlgorithmAdd(mgr=0x%x,algo=0x%x)",hSigCompMgr,pAlgorithm));

	/* first check that the algorithm wasn't added already */
	rv = SigCompAlgorithmFind (pSigCompMgr, pAlgorithm->algorithmName, &phAlgorithm);
	if (RV_OK != rv)
	{
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompAlgorithmAdd - failed to look for existing algorithm 0x%x (rv=%d)",
            pAlgorithm, rv));
	}

	if (phAlgorithm	!= NULL)
	{
		/* algorithm found */
		RvLogDebug(pSigCompMgr->pLogSource, (pSigCompMgr->pLogSource,
			"RvSigCompAlgorithmAdd - Algorithm 0x%x (%s) already in algorithm list",
			pAlgorithm, pAlgorithm->algorithmName));
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
		return (RV_OK);
	}

    /* create a state */
    rv = SigCompStateHandlerCreateState (&pSigCompMgr->stateHandlerMgr,
                                         pAlgorithm->decompBytecodeSize,
                                         &pState);
    if (RV_OK != rv)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompAlgorithmAdd - failed to create state (rv=%d)",
            rv));
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return (RV_ERROR_UNKNOWN);
    }

    rv = SigCompStateHandlerStateSetPriority (&(pSigCompMgr->stateHandlerMgr),
                                              (RV_UINT16_MAX), 
                                              pState);

    if (RV_OK != rv)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompAlgorithmAdd - failed to set state priority (rv=%d)",
            rv));
        /* return value is not relevant, so ignore */
        /*lint -e{534}*/
        SigCompStateHandlerDeleteState (&pSigCompMgr->stateHandlerMgr, pState, RV_FALSE);
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return (RV_ERROR_UNKNOWN);
    }

    /* copy the algorithm to the state */
    rv = SigCompStateHandlerStateWriteData (&pSigCompMgr->stateHandlerMgr, 
                                            pState,
                                            0, 
                                            pAlgorithm->decompBytecodeSize,
                                            pAlgorithm->pDecompBytecode);

    if (RV_OK != rv)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompAlgorithmAdd - failed to write data to state (rv=%d)",
            rv));
        /* ignore irrelevant (in this context) return value */
        /*lint -e{534}*/
        SigCompStateHandlerDeleteState (&pSigCompMgr->stateHandlerMgr, pState, RV_FALSE);
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return (RV_ERROR_UNKNOWN);
    }

    rv = SigCompStateHandlerStateSetID(&pSigCompMgr->stateHandlerMgr, pState);
    if (RV_OK != rv)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompAlgorithmAdd - failed to set stateID (rv=%d)",
            rv));
        /* return value not relevant */
        /*lint -e{534}*/
        SigCompStateHandlerDeleteState (&pSigCompMgr->stateHandlerMgr, pState, RV_FALSE);
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return (RV_ERROR_UNKNOWN);
    }

    rv = SigCompStateHandlerSaveState(&pSigCompMgr->stateHandlerMgr, NULL, NULL, &pState);
    if (RV_OK != rv)
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompAlgorithmAdd - failed to save state (rv=%d)",
            rv));
        /* return value not relevant */
        /*lint -e{534}*/
        SigCompStateHandlerDeleteState (&pSigCompMgr->stateHandlerMgr, pState, RV_FALSE);
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return (rv);
    }

    rv = RLIST_InsertTail(pSigCompMgr->hAlgorithmListPool,
                          pSigCompMgr->hAlgorithmList,
                          &hElement);

    if (RV_OK == rv)
    {
        memcpy ((void *)hElement, (void *)pAlgorithm, sizeof (RvSigCompAlgorithm));
    }
    else
    {
        RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
            "RvSigCompAlgorithmAdd - failed to insert state into the compartment's list (rv=%d)",
            rv));
        rv = SigCompStateHandlerDeleteState (&pSigCompMgr->stateHandlerMgr, pState, RV_TRUE);
        if (RV_OK != rv)
        {
            RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
                "RvSigCompAlgorithmAdd - failed to delete state (rv=%d)",
                rv));
            RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
            return (rv);
        }
    }

	/* Notify that an algorithm was initialized successfully */ 
	SigCompCallbackAlgorithmInitializedEv(pSigCompMgr,pAlgorithm); 

    RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    RvLogLeave(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
		"RvSigCompAlgorithmAdd(mgr=0x%x,algo=0x%x)",hSigCompMgr,pAlgorithm));
    return (rv);
} /* end of RvSigCompAlgorithmAdd */




/***************************************************************************
 * RvSigCompGetState
 * ------------------------------------------------------------------------
 * General: retrieve a state (such as dictionary) given the partial ID
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSigCompMgr - a handle to the sigComp manager
 *         partialIdLength - length of partialID in bytes (6..20)
 *         *pPartialID  - pointer to the partial ID (derived from SHA1 of the state)
 *         **ppStateData  - to be filled with the pointer to the state's data
 *
 * Output: **ppStateData - filled with the pointer to the state's data
 *         *pStateAddress  - part of the state information
 *         *pStateInstruction  - part of the state information
 *         *pMinimumAccessLength  - part of the state information
 *         *pStateDataSize  - part of the state information
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompGetState(IN    RvSigCompMgrHandle  hSigCompMgr,
                                            IN    RvUint8             partialIdLength,
                                            IN    RvUint8             *pPartialID,
                                            INOUT RvUint8             **ppStateData,
                                            OUT   RvUint16            *pStateAddress,
                                            OUT   RvUint16            *pStateInstruction,
                                            OUT   RvUint16            *pMinimumAccessLength,
                                            OUT   RvUint16            *pStateDataSize)
{
    SigCompMgr *pSigCompMgr = (SigCompMgr *)hSigCompMgr;
    RvStatus rv;
    SigCompState *pState;
    
    if ((NULL == hSigCompMgr) ||
        (NULL == pPartialID) ||
        (NULL == ppStateData))
    {
        return(RV_ERROR_NULLPTR);
    }
    if ((partialIdLength < 6) ||
        (partialIdLength > 20))
    {
        return(RV_ERROR_BADPARAM);
    }

    RvMutexLock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    rv = SigCompStateHandlerGetState((SigCompStateHandlerMgr *)&pSigCompMgr->stateHandlerMgr,
                                     pPartialID,
                                     partialIdLength,
                                     RV_FALSE,
                                     &pState);
    if (rv != RV_OK) 
    {
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        /* error retrieving state */
        return(rv);
    }

    *ppStateData = pState->pData;
    *pStateAddress = pState->stateAddress;
    *pStateInstruction = pState->stateInstruction;
    *pMinimumAccessLength = pState->minimumAccessLength;
    *pStateDataSize = pState->stateDataSize;
    RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);

    return(RV_OK);
} /* end of RvSigCompGetState */

/***************************************************************************
 * RvSigCompGetLocalStatesIdList
 * ------------------------------------------------------------------------
 * General: The compressor may call this function to get a list of the available
 *          states IDs, in order to inform the remote side of these states (dictionaries)
 *
 *          Structure of the local states list is:
 *          +---+---+---+---+---+---+---+---+
 *          | length_of_partial_state_ID_1  |
 *          +---+---+---+---+---+---+---+---+
 *          |                               |
 *          :  partial_state_identifier_1   :
 *          |                               |
 *          +---+---+---+---+---+---+---+---+
 *          :                               :
 *          +---+---+---+---+---+---+---+---+
 *          | length_of_partial_state_ID_n  |
 *          +---+---+---+---+---+---+---+---+
 *          |                               |
 *          :  partial_state_identifier_n   :
 *          |                               |
 *          +---+---+---+---+---+---+---+---+
 *          | 0   0   0   0   0   0   0   0 |
 *          +---+---+---+---+---+---+---+---+
 *
 *
 * Return Value: RV_OK            - On success.
 *               RV_ERROR_UNKNOWN - On failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hSigCompMgr - a handle to the sigComp manager
 *         *pList - pointer to a list to be filled with data
 *
 * Output: *pList - pointer to a list to filled with data
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompGetLocalStatesIdList(
                                                 IN    RvSigCompMgrHandle  hSigCompMgr,
                                                 INOUT RvSigCompLocalStatesList *pList)
{
    SigCompMgr *pSigCompMgr = (SigCompMgr *)hSigCompMgr;

    if ((NULL == hSigCompMgr) ||
        (NULL == pList))
    {
        return(RV_ERROR_NULLPTR);
    }

	RvMutexLock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    pList->pLocalStates     = pSigCompMgr->localIdList;
    pList->nLocalStatesSize = pSigCompMgr->localIdListUsedLength;
	RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);

    return(RV_OK);
} /* end of RvSigCompGetLocalStatesIdList */


/***************************************************************************
 * RvSigCompSetNewLogFilters
 * ------------------------------------------------------------------------
 * General: sets new filters to log. Note that the new
 * filters are not only the ones to be changed, but are a new set of 
 * filters that the module is going to use.
 *
 * Return Value: RV_OK - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSigCompMgr - a handle to the sigComp manager
 *        filters     - the new filters that should be used
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompSetNewLogFilters(IN RvSigCompMgrHandle hSigCompMgr,
                                                    IN RvInt32 filters)
{
    RvStatus   rv = RV_OK;
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    SigCompMgr *pSigCompMgr = (SigCompMgr *)hSigCompMgr;


    if (NULL == pSigCompMgr)
    {
        return (RV_ERROR_NULLPTR);
    }

	RvMutexLock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    rv = RvLogSourceSetMask (pSigCompMgr->pLogSource,
                             SigCompLogMaskConvertToCommonCore(filters));

    if (RV_OK != rv)
    {
        RvLogError (pSigCompMgr->pLogSource, (pSigCompMgr->pLogSource,
                                              "RvSigCompSetNewLogFilters - failed to set new filters (rv=%d)",
                                              rv));
        RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return (rv);
    }

    pSigCompMgr->logFilter = filters;

    RvLogLeave(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
               "RvSigCompSetNewLogFilters (rv=%d)",
               rv));
	RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
	
#else
	RV_UNUSED_ARGS((filters,hSigCompMgr));
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
    return (rv);
} /* end of RvSigCompSetNewLogFilters */

/************************************************************************************
 * RvSigCompIsLogFilterExist
 * ----------------------------------------------------------------------------------
 * General: check existence of a filter in the sigcomp module
 * Return Value:
 *          RV_TRUE        - the filter exists
 *          RV_False       - the filter does not exist
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hStack         - Handle to sigcomp manager
 *          filter         - the filter that we want to check existence for
 * Output:  none
 ***********************************************************************************/
RVAPI RvBool RVCALLCONV RvSigCompIsLogFilterExist(
                                         IN RvSigCompMgrHandle   hSigCompMgr,
                                         IN RvSigCompLogFilters  filter)
{
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    RvBool isFilterExist = RV_FALSE;
    SigCompMgr*  pSigCompMgr;

    if(hSigCompMgr == NULL)
    {
        return RV_FALSE;
    }
    pSigCompMgr = (SigCompMgr*)hSigCompMgr;

	RvMutexLock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    isFilterExist = RvLogIsSelected(pSigCompMgr->pLogSource,
                                    SigCompLogMaskConvertToCommonCore(filter));

	RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    return isFilterExist;
#else
	RV_UNUSED_ARGS((filter,hSigCompMgr));
    return RV_FALSE;
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
}


/***************************************************************************
 * RvSigCompCompartmentClearBytecodeFlag
 * ------------------------------------------------------------------------
 * General: This function resets the specified compartment compression bytecode flag
 *          It may be used for error-recovery purposes (to force resending bytecode)
 *
 * Return Value: RV_OK            - On success.
 *               RV_ERROR_UNKNOWN - On failure.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hSigCompMgr - a handle to the SigComp manager
 *        hCompartment  - a handle to the sigComp compartment
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompCompartmentClearBytecodeFlag(
                            IN RvSigCompMgrHandle         hSigCompMgr,
                            IN RvSigCompCompartmentHandle hCompartment)
{
    RvStatus    rv;
    SigCompMgr  *pSigCompMgr = (SigCompMgr *)hSigCompMgr;
    
    if (NULL == hSigCompMgr)
    {
        return (RV_ERROR_NULLPTR);
    }
    RvMutexLock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    RvLogEnter(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
		"RvSigCompCompartmentClearBytecodeFlag(mgr=0x%x,comp=0x%x)",hSigCompMgr,hCompartment));
    
    rv = SigCompCompartmentHandlerClearBytecodeFlag(&pSigCompMgr->compartmentHandlerMgr,
                                                    hCompartment);

    if (RV_OK != rv) 
    {
        RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "RvSigCompCompartmentClearBytecodeFlag - failed to clear flag (rv=%d)",
            rv));
    }
    else 
    {
        RvLogLeave(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
			"RvSigCompCompartmentClearBytecodeFlag(mgr=0x%x,comp=0x%x)",hSigCompMgr,hCompartment));
    }
    RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    return (rv);
} /* end of RvSigCompCompartmentClearBytecodeFlag */



/*************************************************************************
* RvSigCompGenerateHeader
* ------------------------------------------------------------------------
* General: builds the sigComp header as described in RFC-3320 section 7
*           and figure 3 (p.24)
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    hCompartment  - A handle to the compartment
*           *pBytecode    - a pointer to a buffer containing the bytecode
*           bytecodeSize  - size of bytecode in bytes
*           bytecodeStart - zero (0) indicates stateID of the bytecode is sent
*                           non-zero indicates starting address for bytecode
*           *pBuffer      - a pointer to the output buffer to contain the header
*           *pBufferSize  - on input, will contain the allocated size of the buffer
*
* Output:   *pBuffer      - a pointer to the output buffer to contain the header
*           *pBufferSize  - on output, will contain the number of used bytes
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompGenerateHeader(
                                    IN    RvSigCompCompartmentHandle hCompartment,
                                    IN    RvUint8                    *pBytecode,
                                    IN    RvUint32                   bytecodeSize,
                                    IN    RvUint32                   bytecodeStart,
                                    INOUT RvUint8                    *pBuffer,
                                    INOUT RvUint32                   *pBufferSize)
{
    SigCompCompartment  *pCompartment = (SigCompCompartment *)hCompartment;
	SigCompMgr			*pSigCompMgr;
    RvUint8             *pCur         = pBuffer;
    RvUint8             *pFirst       = pBuffer;
    RvUint8             *pLast;
    RvUint8             *pRfi         = 0;
    RvUint32            rfiSize       = 0; /* size of the Returned Feedback Item */
    RvUint32            bufSize;
    RvUint32            destination;
    RvLogSource         *pLogSource;
    
    if(pCompartment == 0 || pBytecode == 0 || pBuffer == 0 || pBufferSize == 0)
    {
        return RV_ERROR_NULLPTR;
    }

    pSigCompMgr  = pCompartment->pCHMgr->pSCMgr;
    pLogSource   = pCompartment->pCHMgr->pSCMgr->pLogSource;

	RvMutexLock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
    RvLogEnter(pLogSource,(pLogSource,"RvSigCompGenerateHeader(comp=0x%x)",hCompartment));
    
/* Figures of SigComp header types and structure taken from RFC-3320
+---+---+---+---+---+---+---+---+   +---+---+---+---+---+---+---+---+
| 1   1   1   1   1 | T |  len  |   | 1   1   1   1   1 | T |   0   |
+---+---+---+---+---+---+---+---+   +---+---+---+---+---+---+---+---+
|                               |   |                               |
:    returned feedback item     :   :    returned feedback item     :
|                               |   |                               |
+---+---+---+---+---+---+---+---+   +---+---+---+---+---+---+---+---+
|                               |   |           code_len            |
:   partial state identifier    :   +---+---+---+---+---+---+---+---+
|                               |   |   code_len    |  destination  |
+---+---+---+---+---+---+---+---+   +---+---+---+---+---+---+---+---+
|                               |   |                               |
:   remaining SigComp message   :   :    uploaded UDVM bytecode     :
|                               |   |                               |
+---+---+---+---+---+---+---+---+   +---+---+---+---+---+---+---+---+
|                               |
:   remaining SigComp message   :
|                               |
+---+---+---+---+---+---+---+---+
*/

    bufSize = *pBufferSize;
    pLast = pBuffer + bufSize; /* pointer to the end of the buffer */
    
    if(pCur >= pLast)
    {
        RvLogError(pLogSource,(pLogSource,
            "RvSigCompGenerateHeader - buffer pointers mismatch 1 (startAddress = 0x%X   endAddres = 0x%X)",
            pCur, pLast));
        RvLogLeave(pLogSource,(pLogSource,"RvSigCompGenerateHeader"));
		RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
        return(RV_ERROR_UNKNOWN);
    }

    *pCur++ = 0xF8;

    rfiSize = pCompartment->rqFeedbackItemLength;
    if(rfiSize != 0) 
    {
        pRfi = pCompartment->requestedFeedbackItem;
        *pFirst |= 0x4;   /* set T bit - returned feedback item present */
        if(rfiSize == 1 && *pRfi < 128) 
        {
            if(pCur >= pLast)
            {
                RvLogError(pLogSource,(pLogSource,
                    "RvSigCompGenerateHeader - buffer pointers mismatch 2 (startAddress = 0x%X   endAddres = 0x%X)",
                    pCur, pLast));
                RvLogLeave(pLogSource,(pLogSource,"RvSigCompGenerateHeader"));
				RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
                return(RV_ERROR_UNKNOWN);
            }

            *pCur++ = *pRfi;
        } 
        else
        {
            if(pCur + rfiSize >= pLast) 
            {
                RvLogError(pLogSource,(pLogSource,
                    "RvSigCompGenerateHeader - not enough mem in buffer for RFI (startAddress = 0x%X   endAddres = 0x%X   rfiSize = 0x%X)",
                    pCur, pLast, rfiSize));
                RvLogLeave(pLogSource,(pLogSource,"RvSigCompGenerateHeader"));
				RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
                return(RV_ERROR_UNKNOWN);
            }

            /* copy the RFI and advance the writing-pointer */
            *pCur++ = (RvUint8) ((RvUint8)rfiSize | 0x80); /* Mark the first bit */
            memcpy(pCur, pRfi, rfiSize);
            pCur += rfiSize;
        }
    } 

    if(bytecodeStart == 0)
    {
        /* bytecodeStart == 0 indicates that we got stateID and not a bytecode */
        RvUint32 len = (bytecodeSize / 3) - 1;

        if(bytecodeSize != 6 && bytecodeSize != 9 && bytecodeSize != 12)
        {
            /* Illegal state id size: should be 6, 9 or 12 */
            RvLogError(pLogSource,(pLogSource,
                "RvSigCompGenerateHeader - illegal stateID size (stateID size = %d)",
                bytecodeSize));
            RvLogLeave(pLogSource,(pLogSource,"RvSigCompGenerateHeader"));
			RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
            return(RV_ERROR_BADPARAM);
        }

        *pFirst |= len;
        if(pCur + bytecodeSize >= pLast)
        {
            RvLogError(pLogSource,(pLogSource,
                "RvSigCompGenerateHeader - not enough mem in buffer for stateID (startAddress = 0x%X   endAddres = 0x%X   ID size = 0x%X)",
                pCur, pLast, bytecodeSize));
            RvLogLeave(pLogSource,(pLogSource,"RvSigCompGenerateHeader"));
			RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
            return(RV_ERROR_UNKNOWN);
        }
    } 
    else
    {
        if((pCur + 2 + bytecodeSize >= pLast) || bytecodeSize > 4095)
        {
            RvLogError(pLogSource,(pLogSource,
                "RvSigCompGenerateHeader - not enough mem in buffer for bytecode (startAddress = 0x%X   endAddres = 0x%X   bytecodeSize = 0x%X)",
                pCur, pLast, bytecodeSize));
            RvLogLeave(pLogSource,(pLogSource,"RvSigCompGenerateHeader"));
			RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
            return(RV_ERROR_UNKNOWN);
        }

        *pCur++ = (RvUint8)(bytecodeSize >> 4);

        destination = bytecodeStart >> 6;

        /* destination has to be 64 * i, where 2 <= i <= 16 */
        if((destination << 6) != bytecodeStart || destination < 2 || destination > 16) 
        {
            RvLogError(pLogSource,(pLogSource,
                "RvSigCompGenerateHeader - destination parameter mismatch (destination = %d)",
                destination));
            RvLogLeave(pLogSource,(pLogSource,"RvSigCompGenerateHeader"));
			RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);
            return(RV_ERROR_BADPARAM);
        }

        destination--;
        *pCur++ = (RvUint8)(((bytecodeSize & 0xf) << 4) | destination);
    }

    memcpy(pCur, pBytecode, bytecodeSize);
    *pBufferSize = (RvUint32)(pCur + bytecodeSize - pFirst);

    RvLogLeave(pLogSource,(pLogSource,"RvSigCompGenerateHeader(comp=0x%x)",hCompartment));
	RvMutexUnlock(pSigCompMgr->pLock, pSigCompMgr->pLogMgr);

    return(RV_OK);
} /* end of RvSigCompGenerateHeader */


/*************************************************************************
* RvSigCompSHA1Reset
* ------------------------------------------------------------------------
* General: Call this function if a piece-wise calculation is needed.
*           Sequence of calls: SigCompSHA1Reset -> SigCompSHA1Input -> SigCompSHA1Result
*           This function will initialize the SHA1Context in preparation
*           for computing a new SHA1 message digest.
*
* Return Value: ShaStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pContext - The context to reset/initialize
*
*************************************************************************/
RVAPI RvSigCompShaStatus RVCALLCONV RvSigCompSHA1Reset(
						IN RvSigCompSHA1Context *pContext)
{
	if (!pContext)
    {
        return RVSIGCOMP_SHA_NULL;
    }
    pContext->lengthLow = 0;
    pContext->lengthHigh = 0;
    pContext->messageBlockIndex = 0;
    pContext->intermediateHash[0] = 0x67452301;
    pContext->intermediateHash[1] = 0xEFCDAB89;
    pContext->intermediateHash[2] = 0x98BADCFE;
    pContext->intermediateHash[3] = 0x10325476;
    pContext->intermediateHash[4] = 0xC3D2E1F0;
    pContext->isComputed = 0;
    pContext->isCorrupted = RVSIGCOMP_SHA_SUCCESS;
	
    return RVSIGCOMP_SHA_SUCCESS;
}

/*************************************************************************
* RvSigCompSHA1Input
* ------------------------------------------------------------------------
* General: Call this function if a piece-wise calculation is needed.
*           Sequence of calls: SigCompSHA1Reset -> SigCompSHA1Input -> SigCompSHA1Result
*           This function accepts an array of octets as the next portion
*           of the message.
*
* Return Value: ShaStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pContext - The SHA context to update
*           *message_array - An array of characters representing the next portion of the message.
*           length -  The length of the message in message_array
*
*************************************************************************/

RVAPI RvSigCompShaStatus RVCALLCONV RvSigCompSHA1Input(
						   IN RvSigCompSHA1Context *pContext,
                           IN const RvUint8		   *message_array,
                           IN RvUint32			    length)
{
	if (!length)
    {
        return RVSIGCOMP_SHA_SUCCESS;
    }
    if (!pContext || !message_array)
    {
        return RVSIGCOMP_SHA_NULL;
    }
    if (pContext->isComputed)
    {
        pContext->isCorrupted = RVSIGCOMP_SHA_STATE_ERROR;
        return RVSIGCOMP_SHA_STATE_ERROR;
    }
    if (pContext->isCorrupted)
    {
        return(pContext->isCorrupted);
    }
    while(length && !pContext->isCorrupted)
    {
        pContext->messageBlock[pContext->messageBlockIndex++] = (RvUint8) (*message_array & 0xFF);
        pContext->lengthLow += 8;
        if (0 == pContext->lengthLow)
        {
            pContext->lengthHigh += 1;
            if (0 == pContext->lengthHigh)
            {
                /* Message is too long */
                pContext->isCorrupted = RVSIGCOMP_SHA_INPUT_TOO_LONG;
            }
        }
        if (64 == pContext->messageBlockIndex)
        {
            SigCompCommonSHA1ProcessMessageBlock(pContext);
        }
        message_array += 1;
        length -= 1;
    }
    return RVSIGCOMP_SHA_SUCCESS;
}

/*************************************************************************
* RvSigCompSHA1Result
* ------------------------------------------------------------------------
* General: Call this function if a piece-wise calculation is needed.
*           Sequence of calls: SigCompSHA1Reset -> SigCompSHA1Input -> SigCompSHA1Result
*           This function will return the 160-bit message digest into the
*           messageDigest[] array provided by the caller.
*           NOTE: The first octet of hash is stored in the 0th element,
*           the last octet of hash in the 19th element.
*
* Return Value: ShaStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pContext - The context to use to calculate the SHA-1 hash
*           *messageDigest[SHA1HASHSIZE] - Where the digest is returned
*
*************************************************************************/
RVAPI RvSigCompShaStatus RVCALLCONV RvSigCompSHA1Result(
						IN  RvSigCompSHA1Context *pContext,
                        OUT RvUint8				  messageDigest[RVSIGCOMP_SHA1_HASH_SIZE])
{
	RvInt32 i;
	
    if (!pContext || !messageDigest)
    {
        return RVSIGCOMP_SHA_NULL;
    }
    if (pContext->isCorrupted)
    {
        return(pContext->isCorrupted);
    }
    if (!pContext->isComputed)
    {
        SigCompCommonSHA1PadMessage(pContext);
        for(i=0; i<64; i++)
        {
            /* message may be sensitive, clear it out */
            pContext->messageBlock[i] = 0;
        }
        pContext->lengthLow  = 0; /* and clear length */
        pContext->lengthHigh = 0;
        pContext->isComputed = 1;
    }
    for(i = 0; i < RVSIGCOMP_SHA1_HASH_SIZE; i++)
    {
        messageDigest[i] = (RvUint8)( pContext->intermediateHash[i>>2] >> (8  * ( 3 - ( i & 0x03 ) )) );
    }
    return RVSIGCOMP_SHA_SUCCESS;
}

/*************************************************************************
 *				Dynamic Compression Utility Functions					 *
 *************************************************************************/ 

/*************************************************************************
* RvSigCompDynCompress
* ------------------------------------------------------------------------
* General: This function might be called by a SigComp Compression Algorithm.
*		   This function performs a dynamic compression using given 
*		   algorithm callbacks (which resides within the application context) 
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr      - SigComp manager handle 
*		 hCompartment     - SigComp compartment handle
*		 pCompressionInfo - The compression info structure which 
*							contains essentials details for compression
*		 pMsgInfo         - Details of the message to be compressed
*		 pAlgStruct       - Details of the dynamic compression algorithm 
*		 pInstanceContext - The compressor (compartment) context which  
*							is crucial for dynamic compression 
*		 pAppContext      - General application context (currently it's 
*							useless - always NULL)
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompDynCompress(  
                        IN    RvSigCompMgrHandle         hSigCompMgr,
                        IN    RvSigCompCompartmentHandle hCompartment,
                        IN    RvSigCompCompressionInfo   *pCompressionInfo,
                        IN    RvSigCompMessageInfo       *pMsgInfo,
                        IN    RvSigCompAlgorithm         *pAlgStruct,
                        INOUT RvUint8                    *pInstanceContext,
                        INOUT RvUint8                    *pAppContext) 
{
    RvStatus		   s;
    RvSigCompDynState *self		   = (RvSigCompDynState *)pInstanceContext;
	
    (void)pAppContext;
    (void)pAlgStruct;
    (void)hSigCompMgr;
	
    s = SigCompDynStateCompress(self, hCompartment, pCompressionInfo, pMsgInfo);
	
    return s;
}

/*************************************************************************
* RvSigCompDynStateConstruct
* ------------------------------------------------------------------------
* General: This function might be called by a SigComp Compression Algorithm
*		   This function performs a compartment context construction using  
*		   given algorithm callbacks (which resides within the application  
*		   context)
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr      - SigComp manager handle 
*		 hCompartment     - SigComp compartment handle
*		 pAlgStruct       - Details of the dynamic compression algorithm 
*		 instance		  - The compressor (compartment) context which  
*							is crucial for dynamic compression 
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompDynStateConstruct(
							   IN    RvSigCompMgrHandle    hSigCompMgr,
							   IN    RvSigCompDynZStream  *initStream,
							   INOUT RvSigCompDynState    *instance)
{	
	return SigCompDynStateConstruct(instance,initStream,hSigCompMgr);
}

/*************************************************************************
* RvSigCompDynStateDestruct
* ------------------------------------------------------------------------
* General: This function might be called by a SigComp Compression Algorithm
*		   This function performs a compartment context destruction using  
*		   given algorithm callbacks (which resides within the application  
*		   context)
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr      - SigComp manager handle 
*		 hCompartment     - SigComp compartment handle
*		 pAlgStruct       - Details of the dynamic compression algorithm 
*		 instance		  - The compressor (compartment) context which  
*							is crucial for dynamic compression 
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompDynStateDestruct(
							   IN    RvSigCompMgrHandle          hSigCompMgr,
							   IN    RvSigCompCompartmentHandle  hCompartment,
							   IN    RvSigCompAlgorithm         *pAlgStruct,
							   INOUT void						*instance)
{
    (void)hSigCompMgr,(void)hCompartment,(void)pAlgStruct;
		
	return SigCompDynStateDestruct((RvSigCompDynState*)instance);
}

/*************************************************************************
* RvSigCompDynOnPeerMsg
* ------------------------------------------------------------------------
* General: This function might be called by a SigComp Compression Algorithm.
*		   This function performs an update of a compartment context (which
*		   is used in dynamic compression) regarding an incoming message.
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr      - SigComp manager handle 
*		 hCompartment     - SigComp compartment handle
*		 pInfo			  - The compression info structure which 
*		 pAlgStruct       - Details of the dynamic compression algorithm 
*		 pInstanceContext - The compressor (compartment) context which  
*							is crucial for dynamic compression 
*************************************************************************/
RVAPI RvStatus RVCALLCONV RvSigCompDynOnPeerMsg(
							IN    RvSigCompMgrHandle          hSigCompMgr,
							IN    RvSigCompCompartmentHandle  hCompartment,
							IN    RvSigCompAlgorithm         *pAlgStruct,
							IN    RvSigCompCompressionInfo   *pInfo,
							INOUT void			             *instance)
{
    RvSigCompDynState *self = (RvSigCompDynState *)instance;
    
    self->curTime++;
    self->curTimeSec = SigCompDynZTimeSecGet();
	
	(void)hSigCompMgr, (void)hCompartment, (void)pAlgStruct;
		
	return SigCompDynStateAnalyzeFeedback(self, pInfo);
}

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/***************************************************************************
 * SigCompLogMaskConvertCommonToSigComp
 * ------------------------------------------------------------------------
 * General: converts common core's filters to SIGCOMP filters
 *
 * Return Value: RvSigCompLogFilters - SigComp filters
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: coreFilters - the common core's filter value
 ***************************************************************************/
static RvSigCompLogFilters
    RVCALLCONV SigCompLogMaskConvertCommonToSigComp (IN RvLogMessageType coreFilter)
{
    switch (coreFilter)
    {
        case RV_LOGLEVEL_DEBUG:
            return RVSIGCOMP_LOG_DEBUG_FILTER;
        case  RV_LOGLEVEL_INFO:
            return RVSIGCOMP_LOG_INFO_FILTER;
        case RV_LOGLEVEL_WARNING:
            return RVSIGCOMP_LOG_WARN_FILTER;
        case RV_LOGLEVEL_ERROR:
            return RVSIGCOMP_LOG_ERROR_FILTER;
        case RV_LOGLEVEL_EXCEP:
            return RVSIGCOMP_LOG_EXCEP_FILTER;
        case RV_LOGLEVEL_SYNC:
            return RVSIGCOMP_LOG_LOCKDBG_FILTER;
        case RV_LOGLEVEL_ENTER:
              return RVSIGCOMP_LOG_ENTER_FILTER;
        case RV_LOGLEVEL_LEAVE:
              return RVSIGCOMP_LOG_LEAVE_FILTER;
        default:
            /* RV_LOGLEVEL_NONE */
            return RVSIGCOMP_LOG_NONE_FILTER;

    } /* end of switch block */
    
} /* end of sigCompLogMaskConvertCommonToSigComp */

/***************************************************************************
* SigCompLogMsgFormat
* ------------------------------------------------------------------------
* General: Format the core log message to look like a sigcomp log message
* Return Value:
* ------------------------------------------------------------------------
* Arguments:
* Input:    logRecord - Core log record
*           strFinalText - The formated text
***************************************************************************/
static void RVCALLCONV SigCompLogMsgFormat(IN  RvLogRecord *logRecord,
                                           OUT RvChar      **strFinalText)
{
    RvChar         *ptr;
    const RvChar   *strSourceName =  RvLogSourceGetName(logRecord->source);
    const RvChar   *mtypeStr;

    /* Find the message type string */
    switch (RvLogRecordGetMessageType(logRecord))
    {
        case RV_LOGLEVEL_EXCEP:   mtypeStr = "EXCEP  - "; break;
        case RV_LOGLEVEL_ERROR:   mtypeStr = "ERROR  - "; break;
        case RV_LOGLEVEL_WARNING: mtypeStr = "WARN   - "; break;
        case RV_LOGLEVEL_INFO :   mtypeStr = "INFO   - "; break;
        case RV_LOGLEVEL_DEBUG:   mtypeStr = "DEBUG  - "; break;
            /*case RV_LOGLEVEL_ENTER:   mtypeStr = "ENTER  - "; break;
              case RV_LOGLEVEL_LEAVE:   mtypeStr = "LEAVE  - "; break;*/
        case RV_LOGLEVEL_SYNC:    mtypeStr = "SYNC   - "; break;
        default:
           mtypeStr = "NOT_VALID"; break;
    }

    /* Format the message type */
    ptr = (char *)logRecord->text - LOG_MSG_PREFIX;
    *strFinalText = ptr;

    /* Format the message type */
    /*ptr = (char *)strFinalText;*/
    strcpy(ptr, mtypeStr);
    ptr += strlen(ptr);

    /* Pad the module name */
    memset(ptr, ' ', LOG_SOURCE_NAME_LEN+2);

    memcpy(ptr, strSourceName, strlen(strSourceName));
    ptr += LOG_SOURCE_NAME_LEN;
    *ptr = '-'; ptr++;
    *ptr = ' '; ptr++;
} /* end of SigCompLogMsgFormat */

/***************************************************************************
* SigCompLogConstruct
* ------------------------------------------------------------------------
* General: construct log for sigComp
* Return Value: RV_OK - success
*               other - failure
* ------------------------------------------------------------------------
* Arguments:
* Input:    pSigCompCfg - sigComp configuration structure
*           pSigCompMgr - sigComp manager structure
***************************************************************************/
static RvStatus RVCALLCONV SigCompLogConstruct (IN RvSigCompCfg *pSigCompCfg,
                                                IN SigCompMgr   *pSigCompMgr)
{
    RvStatus rv;

    if (RV_FALSE == pSigCompMgr->bLogMgrProvidedByAppl)
    {
        rv = RvLogConstruct (&pSigCompMgr->logMgr);
        if (RV_OK != rv)
        {
            return (rv);
        }
        pSigCompMgr->pLogMgr = &pSigCompMgr->logMgr;

        if (NULL == pSigCompCfg->pfnLogEntryPrint)
        {
            rv = RvLogListenerConstructLogfile(&pSigCompMgr->listener,
                                               pSigCompMgr->pLogMgr,
                                               RV_SIGCOMP_LOGFILE_NAME,
                                               1,
                                               0,
                                               RV_TRUE);
            pSigCompMgr->bLogFuncProvidedByAppl = RV_FALSE;
        }
        else
        {
            pSigCompMgr->pfnLogEntryPrint = pSigCompCfg->pfnLogEntryPrint;
            rv = RvLogRegisterListener ((RvLogMgr *)pSigCompCfg->logHandle,
                                        SigCompLogCB,
                                        (void *)pSigCompMgr);
            pSigCompMgr->bLogFuncProvidedByAppl = RV_TRUE;
        }

        if (RV_OK != rv)
        {
            RvLogDestruct (&pSigCompMgr->logMgr);
            return (rv);
        }
        pSigCompMgr->pLogMgr = &pSigCompMgr->logMgr;
    }
    else
    {
        pSigCompMgr->pLogMgr = (RvLogMgr*)pSigCompCfg->logHandle;
    }
    rv = SigCompLogSourceConstruct (pSigCompMgr);

    if (RV_OK != rv)
    {
        SigCompLogListenerDestruct (pSigCompMgr);
        if (RV_FALSE == pSigCompMgr->bLogFuncProvidedByAppl)
        {
            RvLogDestruct (&pSigCompMgr->logMgr);
        }
        return (rv);
    }

    /* set filters for log */
    rv = RvSigCompSetNewLogFilters ((RvSigCompMgrHandle)pSigCompMgr, pSigCompCfg->logFilter);
    if (RV_OK != rv)
    {
        SigCompLogSourceDestruct (pSigCompMgr);
        SigCompLogListenerDestruct (pSigCompMgr);
        if (RV_FALSE == pSigCompMgr->bLogFuncProvidedByAppl)
        {
            RvLogDestruct (pSigCompMgr->pLogMgr);
        }
        return (rv);
    }

    return (rv);
} /* sigCompLogConstruct */

/***************************************************************************
* SigCompLogDestruct
* ------------------------------------------------------------------------
* General: destruct log for SigComp
* Return Value: RV_OK - success
* ------------------------------------------------------------------------
* Arguments:
* Input:     pSigCompMgr - sigComp manager structure
***************************************************************************/
static RvStatus RVCALLCONV SigCompLogDestruct (IN SigCompMgr *pSigCompMgr)
{
    if (NULL == pSigCompMgr)
    {
        return (RV_ERROR_NULLPTR);
    }

    SigCompLogListenerDestruct (pSigCompMgr);
    SigCompLogSourceDestruct (pSigCompMgr);
    if ((pSigCompMgr->bLogMgrProvidedByAppl == RV_FALSE) &&
        (pSigCompMgr->pLogMgr != NULL))
    {
        RvLogDestruct (pSigCompMgr->pLogMgr);
    }

    return (RV_OK);
} /* SigCompLogDestruct */


/***************************************************************************
* SigCompLogMaskConvertToCommonCore
* ------------------------------------------------------------------------
* General: converts sigComp log filters to common core's
* Return Value: common core's filters
* ------------------------------------------------------------------------
* Arguments:
* Input:    sigCompFilter - sigComp filters value
***************************************************************************/
static RvLogMessageType RVCALLCONV SigCompLogMaskConvertToCommonCore (IN RvInt32 sigCompFilter)
{
    RvLogMessageType coreFilters = RV_LOGLEVEL_NONE;

    if(sigCompFilter & RVSIGCOMP_LOG_DEBUG_FILTER)
    {
        coreFilters |= RV_LOGLEVEL_DEBUG;
    }
    if(sigCompFilter & RVSIGCOMP_LOG_INFO_FILTER)
    {
        coreFilters |= RV_LOGLEVEL_INFO;
    }
    if(sigCompFilter & RVSIGCOMP_LOG_WARN_FILTER)
    {
        coreFilters |= RV_LOGLEVEL_WARNING;
    }
    if(sigCompFilter & RVSIGCOMP_LOG_ERROR_FILTER)
    {
        coreFilters |= RV_LOGLEVEL_ERROR;
    }
    if(sigCompFilter & RVSIGCOMP_LOG_EXCEP_FILTER)
    {
        coreFilters |= RV_LOGLEVEL_EXCEP;
    }
    if(sigCompFilter & RVSIGCOMP_LOG_LOCKDBG_FILTER)
    {
        coreFilters |= RV_LOGLEVEL_SYNC;
    }
	if(sigCompFilter & RVSIGCOMP_LOG_ENTER_FILTER)
	{
	    coreFilters |= RV_LOGLEVEL_ENTER;
	}
	if(sigCompFilter & RVSIGCOMP_LOG_LEAVE_FILTER)
	{
		coreFilters |= RV_LOGLEVEL_LEAVE;
	}

    return coreFilters;
} /* sigCompLogMaskConvertToCommonCore */

/***************************************************************************
* SigCompLogListenerDestruct
* ------------------------------------------------------------------------
* General: destruct sigComp log listeners
*
* Return Value:
* ------------------------------------------------------------------------
* Arguments:
* Input:    pSigCompMgr - sigComp manager structure
***************************************************************************/
static void RVCALLCONV SigCompLogListenerDestruct (IN SigCompMgr *pSigCompMgr)
{
    if((pSigCompMgr->bLogFuncProvidedByAppl == RV_TRUE) &&
        (pSigCompMgr->pLogMgr != NULL))
    {
        /* logMgr was created by the SigComp module */
        RvLogUnregisterListener(pSigCompMgr->pLogMgr, SigCompLogCB);
    }
    else
    {
        /* logMgr was provided by the application */
        RvLogListenerDestructLogfile(&pSigCompMgr->listener);
    }
} /* end of sigCompLogListenerDestruct */

/***************************************************************************
* SigCompLogSourceConstruct
* ------------------------------------------------------------------------
* General: construct sigComp log source
*
* Return Value: RV_OK - Success
*               other - Failure
* ------------------------------------------------------------------------
* Arguments:
* Input:    pSigCompMgr - sigComp manager structure
***************************************************************************/
static RvStatus RVCALLCONV SigCompLogSourceConstruct (IN SigCompMgr *pSigCompMgr)
{
    RvStatus rv = RV_OK;

    if (NULL == pSigCompMgr)
    {
        return (RV_ERROR_NULLPTR);
    }

    rv  = RvMemoryAlloc(NULL,
                        sizeof(RvLogSource),
                        pSigCompMgr->pLogMgr,
                        (void*)&(pSigCompMgr->pLogSource));
    if(RV_OK != rv)
    {
        pSigCompMgr->pLogSource = NULL;
        return (rv);
    }
    rv = RvLogSourceConstruct(pSigCompMgr->pLogMgr,
                              pSigCompMgr->pLogSource,
                              "SIGCOMP",
                              "Signaling Compression");
    if(RV_OK != rv)
    {
        RvMemoryFree(pSigCompMgr->pLogSource,pSigCompMgr->pLogMgr);
        return (rv);
    }
    return RV_OK;

} /* end of sigCompLogSourceConstruct */

/***************************************************************************
* SigCompLogSourceDestruct
* ------------------------------------------------------------------------
* General: destruct sigComp log source
* Return Value:
* ------------------------------------------------------------------------
* Arguments:
* Input:    pSigCompMgr - sigComp manager structure
***************************************************************************/
static void RVCALLCONV SigCompLogSourceDestruct (IN SigCompMgr *pSigCompMgr)
{
    if ((NULL == pSigCompMgr) ||
        (NULL == pSigCompMgr->pLogSource) ||
        (NULL == pSigCompMgr->pLogMgr))
    {
        return;
    }

    RvLogSourceDestruct (pSigCompMgr->pLogSource);
    RvMemoryFree(pSigCompMgr->pLogSource,pSigCompMgr->pLogMgr);
    return;
} /* end of sigCompLogSourceDestruct */

/***************************************************************************
* SigCompLogCB
* ------------------------------------------------------------------------
* General: sigComp's log callback function
* Return Value:
* ------------------------------------------------------------------------
* Arguments:
* Input:    pLogRecord - log record to log
*           context    - context to be used by called function
***************************************************************************/
static void RVCALLCONV SigCompLogCB (IN RvLogRecord *pLogRecord,
                                     IN void        *context)
{
    SigCompMgr *pSigCompMgr = (SigCompMgr *)context;
    RvChar          *strFinalText;

    SigCompLogMsgFormat (pLogRecord, &strFinalText);

    if (NULL != pSigCompMgr->pfnLogEntryPrint)
    {
        pSigCompMgr->pfnLogEntryPrint (pSigCompMgr->logEntryPrintContext,
                                       SigCompLogMaskConvertCommonToSigComp ((RvLogMessageType)pLogRecord->messageType),
                                       strFinalText);
    }
} /* sigCompLogCB */

#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/*************************************************************************
* SigCompSigCompMgrGetResources
* ------------------------------------------------------------------------
* General: retrieves the resource usage of the SigCompMgr
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     *pSCMgr - a pointer to the state handler
*
* Output:    *pResources - a pointer a resource structure
*************************************************************************/
static RvStatus SigCompSigCompMgrGetResources(IN  SigCompMgr            *pSCMgr,
                                              OUT RvSigCompMgrResources *pResources)
{
    RvUint32 allocNumOfLists;
    RvUint32 allocMaxNumOfUserElement;
    RvUint32 currNumOfUsedLists;
    RvUint32 currNumOfUsedUserElement;
    RvUint32 maxUsageInLists;
    RvUint32 maxUsageInUserElement;

    if ((NULL == pSCMgr) || (NULL == pResources))
    {
        return(RV_ERROR_NULLPTR);
    }

    RvLogEnter(pSCMgr->pLogSource ,(pSCMgr->pLogSource,
        "SigCompSigCompMgrGetResources(mgr=0x%x)",pSCMgr));

    /* clear the resource report */
    memset(pResources,0,sizeof(RvSigCompMgrResources));
    
    /* get the algorithm's list resource status */
    if (NULL != pSCMgr->hAlgorithmListPool)
    {
        RLIST_GetResourcesStatus(pSCMgr->hAlgorithmListPool,
                                 &allocNumOfLists,   /*always 1*/
                                 &allocMaxNumOfUserElement,
                                 &currNumOfUsedLists,/*always 1*/
                                 &currNumOfUsedUserElement,
                                 &maxUsageInLists,
                                 &maxUsageInUserElement);
        pResources->algorithms.currNumOfUsedElements  = currNumOfUsedUserElement;
        pResources->algorithms.numOfAllocatedElements = allocMaxNumOfUserElement;
        pResources->algorithms.maxUsageOfElements     = maxUsageInUserElement;
    }
    
    /* get the dictionary's list resource status */
    if (NULL != pSCMgr->hDictionaryListPool)
    {
        RLIST_GetResourcesStatus(pSCMgr->hDictionaryListPool,
                                 &allocNumOfLists,   /*always 1*/
                                 &allocMaxNumOfUserElement,
                                 &currNumOfUsedLists,/*always 1*/
                                 &currNumOfUsedUserElement,
                                 &maxUsageInLists,
                                 &maxUsageInUserElement);
        pResources->dictionaries.currNumOfUsedElements  = currNumOfUsedUserElement;
        pResources->dictionaries.numOfAllocatedElements = allocMaxNumOfUserElement;
        pResources->dictionaries.maxUsageOfElements     = maxUsageInUserElement;
    }
    
    RvLogLeave(pSCMgr->pLogSource,(pSCMgr->pLogSource,
		"SigCompSigCompMgrGetResources(mgr=0x%x)",pSCMgr));
    return(RV_OK);
} /* end of SigCompSigCompMgrGetResources */




/************************************************************************************
 * SigCompPrintConfigParamsToLog
 * ----------------------------------------------------------------------------------
 * General: Prints all the configuration parameters to the log.
 * Return Value: void
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pSCMgr - the SigComp manager
 ***********************************************************************************/
static void RVCALLCONV SigCompPrintConfigParamsToLog(IN SigCompMgr *pSCMgr)
{
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "[======Version=========]"));
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - SigComp Version = %s",SIGCOMP_MODULE_VERSION));

    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
            "[======Compilation Parameters=========]"));
#ifndef RV_DEBUG
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
               "SigCompPrintConfigParamsToLog - RELEASE = on"));
#else
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
               "SigCompPrintConfigParamsToLog - RV_DEBUG"));
#endif

    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
            "[======SigComp Configuration=========]"));
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - maxOpenCompartments = %u",pSCMgr->maxOpenCompartments));
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - maxTempCompartments = %u",pSCMgr->maxTempCompartments));
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - maxStatesPerCompartment = %u",pSCMgr->maxStatesPerCompartment));
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - timeoutForTempCompartments = %u",pSCMgr->timeoutForTempCompartments));
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - bStaticDic3485Mandatory = %d",pSCMgr->bStaticDic3485Mandatory));
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - decompMemSize = %u",pSCMgr->decompMemSize));
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - stateMemSize = %u",pSCMgr->stateMemSize));
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - cyclePerBit = %u",pSCMgr->cyclePerBit));
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - smallBufPoolSize = %u",pSCMgr->smallBufPoolSize));
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - smallPoolAmount = %u",pSCMgr->smallPoolAmount));
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - mediumBufPoolSize = %u",pSCMgr->mediumBufPoolSize));
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - mediumPoolAmount = %u",pSCMgr->mediumPoolAmount));
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - largeBufPoolSize = %u",pSCMgr->largeBufPoolSize));
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - largePoolAmount = %d",pSCMgr->largePoolAmount));
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - bLogFuncProvidedByAppl = %d",pSCMgr->bLogFuncProvidedByAppl));
    RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - bLogMgrProvidedByAppl = %d",pSCMgr->bLogMgrProvidedByAppl));
	RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - dynMaxTripTime = %u",pSCMgr->dynMaxTripTime));
	RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - dynMaxStatesNo = %u",pSCMgr->dynMaxStatesNo));
	RvLogInfo(pSCMgr->pLogSource,(pSCMgr->pLogSource,
        "SigCompPrintConfigParamsToLog - dynMaxTotalStatesSize = %u",pSCMgr->dynMaxTotalStatesSize));
#else
	RV_UNUSED_ARG(pSCMgr);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/
} /* end of SigCompPrintConfigParamsToLog */

/*************************************************************************
* SigCompAlgorithmListDestruct
* ------------------------------------------------------------------------
* General: Destruct the SigComp module algorithm list 
*
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input:     pSigCompMgr - Pointer to the SigComp manager 
*************************************************************************/
static void SigCompAlgorithmListDestruct(IN SigCompMgr *pSigCompMgr)
{
	RvSigCompAlgorithm *pAlgorithm = NULL;
	RvSigCompAlgorithm *pNextAlgo  = NULL;

	RLIST_GetHead(pSigCompMgr->hAlgorithmListPool,
				  pSigCompMgr->hAlgorithmList,
				  (RLIST_ITEM_HANDLE *)&pAlgorithm);

	while (pAlgorithm != NULL) 
	{
		/* Notify that the algorithm is being destructed */ 
		SigCompCallbackAlgorithmEndedEv(pSigCompMgr,pAlgorithm); 

		RLIST_GetNext(pSigCompMgr->hAlgorithmListPool,
					  pSigCompMgr->hAlgorithmList,
					  (RLIST_ITEM_HANDLE)pAlgorithm,
					  (RLIST_ITEM_HANDLE *)&pNextAlgo);

		pAlgorithm = pNextAlgo;
	}

	RLIST_ListDestruct(pSigCompMgr->hAlgorithmListPool, pSigCompMgr->hAlgorithmList);
}

/*************************************************************************
* RvSigCompDynZTimeSecSet
* ------------------------------------------------------------------------
* General: Set the time in seconds
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input: time - The time to be set    
*************************************************************************/
RVAPI void RvSigCompDynZTimeSecSet(RvSigCompZTimeSec time) {
    SigCompDynZTimeSecSet(time);
}

/*************************************************************************
* RvSigCompDynZTimeSecGet
* ------------------------------------------------------------------------
* General: Retrieve the time in seconds
* Return Value: -
* ------------------------------------------------------------------------
* Arguments:
* Input: time - The time to be set    
*************************************************************************/
RVAPI RvSigCompZTimeSec RvSigCompDynZTimeSecGet() {
    return SigCompDynZTimeSecGet();
}

