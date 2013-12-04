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
 *                              <SigCompCompressorDispatcherObject.c>
 *
 * Description :
 * this file hold the implementation of the compressor dispatcher manager
 *    Author                         Date
 *    ------                        ------
 *    ellyAmitai                    20031214
 *    Gil Keini                     20040111
 *********************************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILED                           */
/*-----------------------------------------------------------------------*/
#include "SigCompCompressorDispatcherObject.h"
#include "SigCompMgrObject.h"
#include "SigCompCommon.h"
#include "SigCompCallbacks.h"
#include <string.h> /* for memset & memcpy */

/*-----------------------------------------------------------------------*/
/*                          DEFINES                                      */
/*-----------------------------------------------------------------------*/
#define SIGCOMP_PREFIX             (0xF8)

#define SIGCOMP_CODE_LEN_FIRST_BYTE(len)          ((len) >> 4)

#define SIGCOMP_DEST_CALC(dest)                   (((dest) >> 6) - 1)
#define SIGCOMP_CODE_LEN_SECOND_BYTE(len, dest)   \
                                      ((((len) & 0xF) << 4) | (SIGCOMP_DEST_CALC(dest)))

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

#if 0 /* Currently there is no support for changing the algorithm during run-time */ 
static RvBool SigCompCompareAlgorithmCapabilities(IN RvSigCompAlgorithm *pAlgorithm,
                                                  IN SigCompCompartment *pCompartment);
                                                  
static RvStatus SigCompChooseAlgorithm(IN  SigCompCompressionDispatcherMgr *pCDMgr,
                                       IN  RLIST_POOL_HANDLE               hAlgorithmListPool,
                                       IN  RLIST_HANDLE                    hAlgorithmList,
                                       IN  SigCompCompartment              *pCompartment,
                                       OUT RvSigCompAlgorithm              **ppAlgorithm);
#endif /* 0 */ 
                                       


/*-----------------------------------------------------------------------*/
/*                           MODULE  FUNCTIONS                           */
/*-----------------------------------------------------------------------*/
/*************************************************************************
* SigCompCompressorDispatcherConstruct
* ------------------------------------------------------------------------
* General: constructs and initializes the compressor dispatcher ,there
*          is one compressor dispatcher per entity and therefore it is
*          called upon initialization of the sigComp module
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:     hSigCompMgr - A handle to the sigComp manager
*************************************************************************/
RvStatus SigCompCompressorDispatcherConstruct(IN RvSigCompMgrHandle hSigCompMgr)

{
    SigCompMgr  *pSigCompMgr = (SigCompMgr *)hSigCompMgr;
  
    RvLogEnter(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
		"SigCompCompressorDispatcherConstruct(mgr=0x%x)",hSigCompMgr));
    if (NULL == hSigCompMgr)
    {
        return RV_ERROR_NULLPTR;
    }
    
    pSigCompMgr->compressorDispMgr.pSCMgr = pSigCompMgr;
    
    RvLogInfo(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource, 
        "SigCompCompressorDispatcherConstruct - object constructed successfully (pCDMgr=0x%X)",
        &pSigCompMgr->compressorDispMgr));
    RvLogLeave(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource, 
		"SigCompCompressorDispatcherConstruct(mgr=0x%x)",hSigCompMgr));
    return RV_OK;
} /* end of SigCompCompressorDispatcherConstruct */

/*************************************************************************
* SigCompCompressorDispatcherDestruct
* ------------------------------------------------------------------------
* General: destructor of the compressor dispatcher, will be called
*          when destructing the sigComp module
* Return Value: void
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCDMgr - a handle to the compressor dispatcher
*************************************************************************/
void SigCompCompressorDispatcherDestruct(IN SigCompCompressionDispatcherMgr* pCDMgr)
{
    
    if (NULL == pCDMgr)
    {
        return;
    }
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    if(!pCDMgr->pSCMgr || !pCDMgr->pSCMgr->pLogSource )
	   return;
#endif
/* SPIRENT_END */
    RvLogEnter(pCDMgr->pSCMgr->pLogSource,(pCDMgr->pSCMgr->pLogSource, 
		"SigCompCompressorDispatcherDestruct(mgr=0x%x)",pCDMgr));
    
    /* Nothing to do (yet) */
    
    RvLogInfo(pCDMgr->pSCMgr->pLogSource,(pCDMgr->pSCMgr->pLogSource, 
        "SigCompCompressorDispatcherDestruct - object destructed (pCDMgr=0x%X)",pCDMgr));
    RvLogLeave(pCDMgr->pSCMgr->pLogSource,(pCDMgr->pSCMgr->pLogSource, 
		"SigCompCompressorDispatcherDestruct(mgr=0x%x)",pCDMgr));
    return;
} /* end of SigCompCompressorDispatcherDestruct */

/*************************************************************************
* SigCompCompressorDispatcherCompress
* ------------------------------------------------------------------------
* General: this function constructs the sigComp message,
*          SigComp message should constructed from a header, a compressed
*          message and some optional additional information
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pCDMgr - a handle to the compressor dispatcher
*           *pCompartment - A pointer to the compartment
*           *pMsgInfo - a pointer to 'RvSigCompMessageInfo' structure,
*                       holds the plain message + its size
* Output:   *pMsgInfo - a pointer to 'RvSigCompMessageInfo' structure,
*                       holds the compressed message + its size
*************************************************************************/
RvStatus SigCompCompressorDispatcherCompress(
                          IN    SigCompCompressionDispatcherMgr *pCDMgr,
                          IN    SigCompCompartment              *pCompartment,
                          INOUT RvSigCompMessageInfo            *pMsgInfo)
{ 
    RvStatus                 rv;
    SigCompMgr               *pSigCompMgr;
    RvSigCompAlgorithm       *pAlgorithm;
    RvSigCompCompressionInfo compressionInfo;
	pSigCompMgr = pCDMgr->pSCMgr;

    RvLogEnter(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
		"SigCompCompressorDispatcherCompress(mgr=0x%x,comp=0x%x)",pCDMgr,pCompartment));

    /* check to see if remote capabilities have changed */
#if 0 /* Currently, no support for changing algorithm at run time */
    if (pCompartment->bRemoteCapabilitiesHaveChanged == RV_TRUE) 
    {
        /* choose compression algorithm according to new information */
        rv = SigCompChooseAlgorithm(pCDMgr,
                                    pSigCompMgr->hAlgorithmListPool,
                                    pSigCompMgr->hAlgorithmList,
                                    pCompartment,
                                    &pAlgorithm);
        if (rv != RV_OK) 
        {
            /* failed to choose algorithm */
            RvLogWarning(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
                "SigCompCompressorDispatcherCompress - failed to choose algorithm (rv=%d)",
                rv));
            /* return to default algorithm (first on the list) */
            RLIST_GetHead(pSigCompMgr->hAlgorithmListPool,
                          pSigCompMgr->hAlgorithmList,
                          (RLIST_ITEM_HANDLE *)&pAlgorithm);
            
            if (pAlgorithm == NULL) 
            {
                /* empty list - user did not map/add at least one compression algorithm */
                RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
                    "SigCompCompressorDispatcherCompress - empty algorithms list"));
                return(RV_ERROR_UNKNOWN);
            }
            
        }
        if (pAlgorithm != pCompartment->pCompressionAlg) 
        {
            /* a different algorithm was recommended then the current one */
            /* thus: */

            /* delete old context (if exists) and create a new one */
            if (pCompartment->algContext.pData != NULL) 
            {
                rv = SigCompStateHandlerAlgContextDataDealloc(&pSigCompMgr->stateHandlerMgr,
                                                              pCompartment->algContext.pData,
                                                              pCompartment->algContext.curRA);
                if (rv != RV_OK) 
                {
                    /* failed to dealloc old algorithm context */
                    RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
                        "SigCompCompressorDispatcherCompress - failed to dealloc algorithm context (rv=%d)",
                        rv));
                    return(rv);
                }
                pCompartment->algContext.pData = NULL;
                pCompartment->pCompressionAlg = pAlgorithm;
            } /* end of deleteing context of old algorithm */
            if (pCompartment->pCompressionAlg->contextSize > 0) 
            {
                rv = SigCompStateHandlerAlgContextDataAlloc(&pSigCompMgr->stateHandlerMgr,
                                                            pCompartment->pCompressionAlg->contextSize,
                                                            &pCompartment->algContext.pData,
                                                            &pCompartment->algContext.curRA);
                if (rv != RV_OK) 
                {
                    /* failed to realloc old algorithm context */
                    RvLogError(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
                        "SigCompCompressorDispatcherCompress - failed to realloc algorithm context (rv=%d)",
                        rv));
                    return(rv);
                }
            } /* end of allocating context for the new algorithm */
        } /* end of handling change in algorithm */

        /* reset flag */
        pCompartment->bRemoteCapabilitiesHaveChanged = RV_FALSE;
    } 
#endif /* 0 - Disable the algorithm update during run-time */ 
    
	pAlgorithm = pCompartment->pCompressionAlg;
	/* end of "choosing/updating algorithm" section */
    
    /* compress and add data-payload directly to the output buffer */
    compressionInfo.pReturnedFeedbackItem    = pCompartment->returnedFeedbackItem;
    compressionInfo.returnedFeedbackItemSize = pCompartment->rtFeedbackItemLength;
    compressionInfo.bResetContext            = pCompartment->bSendBytecodeInNextMessage;
    
    compressionInfo.bStaticDic3485Mandatory  = pCompartment->pCHMgr->pSCMgr->bStaticDic3485Mandatory;

	compressionInfo.remoteSMS = pCompartment->remoteSMS;
    compressionInfo.remoteDMS = pCompartment->remoteDMS;
    compressionInfo.remoteCPB = pCompartment->remoteCPB;
	compressionInfo.pRemoteStateIds = pCompartment->remoteIdList;

	compressionInfo.localCPB = pCompartment->localCPB;
	compressionInfo.localDMS = pCompartment->localDMS;
	compressionInfo.localSMS = pCompartment->localSMS;

	compressionInfo.localStatesSize = pSigCompMgr->localIdListUsedLength;
	compressionInfo.pLocalStateIds  = pSigCompMgr->localIdList;

    compressionInfo.bSendLocalCapabilities = pCompartment->bSendLocalCapabilities;
    compressionInfo.bSendLocalStatesIdList = pCompartment->bSendLocalStatesIdList;
    
    compressionInfo.nIncomingMessages = pCompartment->nIncomingMessages;
    compressionInfo.lastIncomingMessageTime = pCompartment->lastIncomingMessageTime;

    RvLogInfo(pSigCompMgr->pLogSource ,(pSigCompMgr->pLogSource ,
        "SigCompCompressorDispatcherCompress - using algorithm %s (sendByteCode=%d)",
        pAlgorithm->algorithmName,pCompartment->bSendBytecodeInNextMessage));

    /* Calling CB compression function */
    rv = SigCompCallbackCompressMsgEv(pCompartment->pCHMgr->pSCMgr,
									  pCompartment,
									  &compressionInfo,
									  pMsgInfo);
    /* Return from callback */
    
    if (RV_OK != rv) 
    {
        RvLogError(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "SigCompCompressorDispatcherCompress - compression error (rv=%d)",
            rv));
       
        return(rv);        
    }

	/* In case that a SigComp user is asking to compress a message with no bytecode */
	/* it is being checked here if the compressed message (to be sent) actually     */
	/* includes the bytecode by checking partial state id len - if it's value is	*/
	/* zero it means that the compressor included a bytecode in the message         */
	/* according to RFC3320 (see Figure 3: Format of a SigComp message) */
	if (pCompartment->bSendBytecodeInNextMessage == RV_FALSE &&
		(pMsgInfo->pCompressedMessageBuffer[0] & 0x03) == 0)
	{
		RvLogInfo(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
            "SigCompCompressorDispatcherCompress - the compressed message includes bytecode although sendByteCode=RV_FALSE"));
	}

    pCompartment->rtFeedbackItemLength       = 0;
    pCompartment->bSendBytecodeInNextMessage = RV_FALSE;
    RvLogLeave(pSigCompMgr->pLogSource,(pSigCompMgr->pLogSource,
		"SigCompCompressorDispatcherCompress(mgr=0x%x,comp=0x%x)",pCDMgr,pCompartment));

    return(RV_OK);
} /* end of SigCompCompressorDispatcherCompress */



#if 0 /* Currently there's no support of changing the algorithm during run-time */
/*************************************************************************
* SigCompChooseAlgorithm
* ------------------------------------------------------------------------
* General: Call this function when the remote EP capabilities have changed
*           This function will choose an algorithm from the list that 
*           has the best compression ratio (the list must be built that way 
*           by the application), and that fits the limitations of the remote EP.
*
* NOTE: The first algorithm in the list is the default algorithm, and must assume
*       a stateless EP with minimal capabilities.
*       The list must be built in an increasing order of needed capabilities. 
*       This means that the last algorithm to be added to the list, will have
*       the best compression ratio and the highest demands from the decompression (remote EP).
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input     *pCDMgr - a handle to the compressor dispatcher
*           hAlgorithmListPool - handle to the pool of algorithms list
*           hAlgorithmList - handle to the algorithms list
*           *pCompartment - pointer to compartment
*
* Output    **ppAlgorithm - return the pointer to the chosen algorithm
*************************************************************************/
static RvStatus SigCompChooseAlgorithm(
                          IN  SigCompCompressionDispatcherMgr *pCDMgr,
                          IN  RLIST_POOL_HANDLE               hAlgorithmListPool,
                          IN  RLIST_HANDLE                    hAlgorithmList,
                          IN  SigCompCompartment              *pCompartment,
                          OUT RvSigCompAlgorithm              **ppAlgorithm)
{
    RLIST_ITEM_HANDLE  hElement;
    RLIST_ITEM_HANDLE  hPrevElement;
    RvBool             bFoundFit;
    RvSigCompAlgorithm *pAlgorithm;
    
    if ((NULL == pCDMgr) ||
        (NULL == hAlgorithmListPool)||
        (NULL == hAlgorithmList)||
        (NULL == pCompartment))
    {
        return(RV_ERROR_NULLPTR);
    }
    RvLogEnter(pCDMgr->pSCMgr->pLogSource,(pCDMgr->pSCMgr->pLogSource,
		"SigCompChooseAlgorithm(mgr=0x%x,comp=0x%x)",pCDMgr,pCompartment));
    
    /* get list tail/end */
    RLIST_GetTail(hAlgorithmListPool, hAlgorithmList, &hElement);
    if (NULL == hElement) 
    {
        /* failed to get list's tail */
        RvLogWarning(pCDMgr->pSCMgr->pLogSource,(pCDMgr->pSCMgr->pLogSource,
            "SigCompChooseAlgorithm - failed to get algorithm-list's tail (list is empty)"));
        return(RV_ERROR_BADPARAM);
    }
    pAlgorithm = (RvSigCompAlgorithm *)hElement;
    hPrevElement = hElement;

    bFoundFit = SigCompCompareAlgorithmCapabilities(pAlgorithm, pCompartment);

    while (!bFoundFit) 
    {
        RLIST_GetPrev(hAlgorithmListPool, hAlgorithmList, hPrevElement, &hElement);
        if (NULL == hElement) 
        {
            /* failed to get next element in list */
            RvLogWarning(pCDMgr->pSCMgr->pLogSource,(pCDMgr->pSCMgr->pLogSource,
                "SigCompChooseAlgorithm - failed to get next element in algorithm-list (reached end of list)"));
            return(RV_ERROR_BADPARAM);
        }
        pAlgorithm = (RvSigCompAlgorithm *)hElement;
        hPrevElement = hElement;
        
        bFoundFit = SigCompCompareAlgorithmCapabilities(pAlgorithm, pCompartment);
    }
    /* at least the first mapped algorithm (list-head) must fit the minimal requirements */

    *ppAlgorithm = pAlgorithm;

    RvLogLeave(pCDMgr->pSCMgr->pLogSource,(pCDMgr->pSCMgr->pLogSource,
		"SigCompChooseAlgorithm(mgr=0x%x,comp=0x%x)",pCDMgr,pCompartment));
    return(RV_OK);
} /* end of SigCompChooseAlgorithm */

/*************************************************************************
* SigCompCompareAlgorithmCapabilities
* ------------------------------------------------------------------------
* General: compare the decompression requirements to the remote end-point's
*           capabilities.
*
* Return Value: RV_TRUE     if the remote EP can handle the decompression algorithm
*               RV_FALSE    if the remote EP can't handle the decomp. algorithm
* ------------------------------------------------------------------------
* Arguments:
* Input:    *pAlgorithm - pointer to the algorithm
*           *pCompartment - pointer to the compartment
*************************************************************************/
static RvBool SigCompCompareAlgorithmCapabilities(IN RvSigCompAlgorithm *pAlgorithm,
                                                  IN SigCompCompartment *pCompartment)
{
    if ((pAlgorithm->decompCPB > pCompartment->remoteCPB) ||
        (pAlgorithm->decompDMS > pCompartment->remoteDMS) ||
        (pAlgorithm->decompSMS > pCompartment->remoteSMS) ||
        (pAlgorithm->decompVer > pCompartment->remoteSigCompVersion))
    {
        return(RV_FALSE);
    }

    return(RV_TRUE);
} /* end of SigCompCompareAlgorithmCapabilities */
#endif /* 0 - Currently there's no support of changing the algorithm during run-time */
