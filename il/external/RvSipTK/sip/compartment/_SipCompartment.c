
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
 *                      <_SipCompartment.h>
 *
 * This file defines the  compartment manager object,that holds all the
 *  Compartments resources including memory pool, pools of lists, locks and
 * more. Its main functionality is to manage the compartments list, to create new
 *  compartments and to associate stack objects to a specific compartment.
 * Moreover,the file contains Internal API functions of the  Compartment layer.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Dikla Dror                  Nov 2003
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#include "AdsRlist.h"
#include "_SipCommonUtils.h"
#include "rvlog.h"
#include "RvSipTransportTypes.h"

#ifdef RV_SIGCOMP_ON
#include "RvSipCompartmentTypes.h"
#include "CompartmentObject.h"
#include "CompartmentMgrObject.h"
#include "_SipCompartment.h"
#include "RvSigComp.h"

#ifdef PRINT_SIGCOMP_BYTE_STREAM
#include <math.h>
#include "rvansi.h"
#endif /* PRINT_SIGCOMP_BYTE_STREAM */
/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                COMPARMENT MANAGER INTERNAL API                        */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SipCompartmentMgrConstruct
 * ------------------------------------------------------------------------
 * General: Construct a new  Compartment manager. The compartment
 *          manager is in charge of all the  compartment. The manager
 *          holds a list of compartments and is used to create new
 *          compartments.
 * Return Value: RV_ERROR_NULLPTR     - The pointer to the compartment mgr is
 *                                   invalid.
 *               RV_ERROR_OUTOFRESOURCES - Not enough resources to create the
 *                                    Compartment manager.
 *               RV_OK        -  Compartment manager was
 *                                   created Successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     cfg           - A pointer to the configuration structure,
 *                          containing all the required init. information.
 *          hStack        - A handle to the stack instance.
 * Output: *phCompartmentMgr - Handle to the newly created
 *                                    compartment manager.
 ***************************************************************************/
RvStatus RVCALLCONV SipCompartmentMgrConstruct(
                IN  SipCompartmentMgrCfg      *cfg,
                IN  void                      *hStack,
                OUT RvSipCompartmentMgrHandle *phCompartmentMgr)
{
    RvStatus               rv;

    if (NULL == cfg)
    {
        return RV_ERROR_BADPARAM;
    }
    if (NULL == hStack || NULL == phCompartmentMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    *phCompartmentMgr = NULL;

    rv = CompartmentMgrConstruct(cfg,hStack,phCompartmentMgr);

    return rv;
}

/***************************************************************************
 * SipCompartmentMgrDestruct
 * ------------------------------------------------------------------------
 * General: Destructs the  Compartment manager freeing all resources.
 * Return Value:  RV_OK -  Compartment manager was destructed.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hCompartmentMgr - Handle to the manager to destruct.
 ***************************************************************************/

RvStatus RVCALLCONV SipCompartmentMgrDestruct(
                  IN RvSipCompartmentMgrHandle hCompartmentMgr)
{
    RvStatus              rv;

    if (NULL == hCompartmentMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    /* Destructing the  Compartment Manager. The lock is  */
    /* internal since the manager mutex has to be destructed too */
    rv = CompartmentMgrDestruct(hCompartmentMgr);

    return rv;
}

/***************************************************************************
 * SipCompartmentMgrGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: Returns a structure with the status of all resources used by
 *          the  Compartment module. It includes the Compartments
 *          list.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *               RV_ERROR_NULLPTR    - The pointer to the resource structure is
 *                                  NULL.
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr       - Handle to the  Compartment manager.
 * Output:     pResources - Pointer to a  Compartment resource structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipCompartmentMgrGetResourcesStatus(
                                 IN  RvSipCompartmentMgrHandle   hMgr,
                                 OUT RvSipCompartmentResources  *pResources)
{
    RvUint32     allocNumOfLists;
    RvUint32     allocMaxNumOfUserElement;
    RvUint32     currNumOfUsedLists;
    RvUint32     currNumOfUsedUserElement;
    RvUint32     maxUsageInLists;
    RvUint32     maxUsageInUserElement;
	RvUint32     allocNumOfPoolElement;
	RvUint32     currNumOfAllocPoolElement;
	RvUint32     maxUsagePoolElement;

    if(hMgr == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if(pResources == NULL)
    {
        return RV_ERROR_NULLPTR;
    }
    /* register-clients list */
    if (NULL == ((CompartmentMgr *)hMgr)->hCompartmentListPool)
    {
        pResources->compartments.currNumOfUsedElements  = 0;
        pResources->compartments.maxUsageOfElements     = 0;
        pResources->compartments.numOfAllocatedElements = 0;
    }
    else
    {
        RLIST_GetResourcesStatus(((CompartmentMgr *)hMgr)->hCompartmentListPool,
                             &allocNumOfLists, /*always 1*/
                             &allocMaxNumOfUserElement,
                             &currNumOfUsedLists,/*always 1*/
                             &currNumOfUsedUserElement,
                             &maxUsageInLists,
                             &maxUsageInUserElement);
        pResources->compartments.currNumOfUsedElements  = currNumOfUsedUserElement;
        pResources->compartments.maxUsageOfElements     = maxUsageInUserElement;
        pResources->compartments.numOfAllocatedElements = allocMaxNumOfUserElement;
    }

	if (NULL == ((CompartmentMgr *)hMgr)->hCompartmentPool)
	{
		pResources->sigCompIdElem.currNumOfUsedElements = 0;
        pResources->sigCompIdElem.maxUsageOfElements     = 0;
        pResources->sigCompIdElem.numOfAllocatedElements = 0;
	}
	else
	{
		RPOOL_GetResources(((CompartmentMgr *)hMgr)->hCompartmentPool,
			&allocNumOfPoolElement, &currNumOfAllocPoolElement, &maxUsagePoolElement);
		pResources->sigCompIdElem.numOfAllocatedElements = allocNumOfPoolElement;
		pResources->sigCompIdElem.maxUsageOfElements = maxUsagePoolElement;
		pResources->sigCompIdElem.currNumOfUsedElements = currNumOfAllocPoolElement;
	}

    return RV_OK;
}

/***************************************************************************
 * SipCompartmentMgrResetMaxUsageResourcesStatus
 * ------------------------------------------------------------------------
 * General: Reset the counter that counts the max number of  compartments
 *          that were used ( at one time ) until the call to this routine.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hMgr - The  Compartment manager.
 ***************************************************************************/
void RVCALLCONV SipCompartmentMgrResetMaxUsageResourcesStatus (
                                 IN  RvSipCompartmentMgrHandle  hMgr)
{
    CompartmentMgr *pMgr = (CompartmentMgr*)hMgr;

    RLIST_ResetMaxUsageResourcesStatus(pMgr->hCompartmentListPool);
}

/***************************************************************************
 * SipCompartmentMgrSetSigCompMgrHandle
 * ------------------------------------------------------------------------
 * General: Stores the SigComp manager handle in the compartment manager
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hCompartmentMgr - Handle to the stack  comp. manager.
 *           hSigCompMgr     - Handle of the SigComp manager handle to be
 *                             stored.
 * Output:   -
 ***************************************************************************/
RvStatus RVCALLCONV SipCompartmentMgrSetSigCompMgrHandle(
                      IN   RvSipCompartmentMgrHandle hCompartmentMgr,
                      IN   RvSigCompMgrHandle        hSigCompMgr)
{
    CompartmentMgr *pMgr;
	RvUint32 numOfElem;

    if(NULL == hCompartmentMgr || NULL == hSigCompMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pMgr = (CompartmentMgr *)hCompartmentMgr;

    LOCK_MGR(pMgr); /*lock the manager*/

	/* we do not want to allow attaching new internal manager if the current sipSigComp manager */
	/* has allocated resources. */
	RLIST_GetNumOfElements (pMgr->hCompartmentListPool, pMgr->hCompartmentList, &numOfElem);
	if (numOfElem == 0)
	{
		pMgr->hSigCompMgr = hSigCompMgr;
	}
    UNLOCK_MGR(pMgr); /*unlock the manager*/

    return RV_OK;
}

/***************************************************************************
 * SipCompartmentMgrDetachFromSigCompModule
 * ------------------------------------------------------------------------
 * General: Release all the relations between SipStack compartments and
 *          SigComp internal compartments.
 *
 * Note:    This function is called upon stack destruction. No lock is being done.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hCompartmentMgr - Handle to the stack comp. manager.
 * Output:   -
 ***************************************************************************/
RvStatus RVCALLCONV SipCompartmentMgrDetachFromSigCompModule(
                      IN   RvSipCompartmentMgrHandle hCompartmentMgr)
{
    CompartmentMgr *pMgr;
    RvStatus        rv;

    if(NULL == hCompartmentMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pMgr = (CompartmentMgr *)hCompartmentMgr;

    rv = CompartmentMgrDetachFromSigCompModule(pMgr);

    return rv;
}

/***************************************************************************
 * SipCompartmentMgrDetachFromSigCompCompartment
 * ------------------------------------------------------------------------
 * General: Close sigcomp compartment, and deatch from it
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hCompartment - Handle to the stack compartment.
 * Output:   -
 ***************************************************************************/
RvStatus RVCALLCONV SipCompartmentMgrDetachFromSigCompCompartment(
                      IN   RvSipCompartmentHandle    hCompartment)
{
    Compartment    *pComp;
	CompartmentMgr *pMgr;
    RvStatus        rv;

    if(NULL == hCompartment)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pComp = (Compartment *)hCompartment;
	pMgr  = pComp->pMgr;

    rv = CompartmentMgrDetachFromSigCompCompartment(pMgr,hCompartment);

    return rv;
}

/*-----------------------------------------------------------------------*/
/*                     COMPARMENT INTERNAL API                    */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SipCompartmentRelateCompHandleToInMsg
 * ------------------------------------------------------------------------
 * General: returns the associated compartment to the incoming message. If the compartment
 *          is not found, the function creates it based on message details.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hCompartmentMgr - the compartment manager
 *			 pUrnPoolPtr     - pointer to the RPOOL containing the sigcomp-id
 *			 pRecvFrom       - address and port of the message source
 *           pSendTo         - address to which the message was sent
 *           hMsg            - the message created the compartment
 *			 hConn           - the connection handle, in case of TCP
 *			 uniqueID        - the internal compartment identifier
 * Output:   phSigCompCompartment - pointer to the found/created compartment handle
 ***************************************************************************/
RvStatus RVCALLCONV SipCompartmentRelateCompHandleToInMsg (
				IN      RvSipCompartmentMgrHandle	hCompartmentMgr,
				IN      RPOOL_Ptr					*pUrnPoolPtr,
				IN	    SipTransportAddress			*pRecvFrom,
				IN      SipTransportAddress			*pSendTo,
				IN      RvSipMsgHandle                 hMsg, 
				IN      RvSipTransportConnectionHandle hConn,
				IN	    RvUint32					 uniqueID,
				INOUT	RvSipCompartmentHandle		*phSigCompCompartment)
{
	Compartment    *pCompartment;
	CompartmentMgr *pMgr;
	RvChar		   *strAlgoName;		
	RvStatus rv = RV_OK;
 
	if ((phSigCompCompartment == NULL) || (hCompartmentMgr == NULL))
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	pMgr         = (CompartmentMgr *)hCompartmentMgr;
	pCompartment = (Compartment *)*phSigCompCompartment;
	
	/* check if compartment is unauthorized. If so, release the internal compartment's resources */
	if ((RvUintPtr)pCompartment == RVSIGCOMP_UNAUTHORIZED_COMPARTMENT_ID)
	{
        RvSigCompCompartmentHandle hInternalComp = 
                (RvSigCompCompartmentHandle)RVSIGCOMP_UNAUTHORIZED_COMPARTMENT_ID;
		rv = RvSigCompDeclareCompartmentEx(pMgr->hSigCompMgr, uniqueID, NULL, &hInternalComp);
		if (rv != RV_OK)
		{
			RvLogError(pMgr->pLogSrc ,(pMgr->pLogSrc ,
				"SipCompartmentRelateCompHandleToInMsg - pMgr 0x%p failed to free SigComp unique ID %d",
				pMgr,uniqueID));
		}
		else
		{
			RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
				"SipCompartmentInternalCompHandleToInMsg - pMgr 0x%p free SigComp unique ID %d",
				pMgr,uniqueID));
		}
		return rv;
	}
	
	/* check if compartment is NULL */
	if ((Compartment *)*phSigCompCompartment == NULL)
	{
		/* look for the compartment. If not exist, create it */
		rv = CompartmentMgrFindCompartment(pMgr, pUrnPoolPtr, pRecvFrom, hConn, phSigCompCompartment);

		if (RV_ERROR_NOT_FOUND == rv)
		{
			RvLogInfo(pMgr->pLogSrc ,(pMgr->pLogSrc ,
				"SipCompartmentRelateCompHandleToInMsg - pMgr 0x%p failed to find compartment, creating new (pUrnPoolPtr 0x%p, pRecvFrom 0x%p, hConn 0x%p)",
				pMgr, pUrnPoolPtr, pRecvFrom, hConn));

			/* compartment not found, create new one */
			rv = CompartmentMgrCreateCompartment(pMgr, pUrnPoolPtr, pRecvFrom, pSendTo, RV_TRUE, hMsg,hConn, phSigCompCompartment);
			if (rv != RV_OK)
			{
				RvLogError(pMgr->pLogSrc ,(pMgr->pLogSrc ,
					"SipCompartmentRelateCompHandleToInMsg - pMgr 0x%p failed to create new compartment (pUrnPoolPtr 0x%p, pRecvFrom 0x%p, hConn 0x%p)",
					pMgr, pUrnPoolPtr, pRecvFrom, hConn));
				*phSigCompCompartment = NULL;
				return rv;
			}

			/* add it to the hash */
			rv = CompartmentMgrAddCompartment(pMgr,*phSigCompCompartment,hConn);
			if (rv != RV_OK)
			{
				RvLogError(pMgr->pLogSrc ,(pMgr->pLogSrc ,
					"SipCompartmentRelateCompHandleToInMsg - pMgr 0x%p failed to add new compartment (pUrnPoolPtr 0x%p, pRecvFrom 0x%p, hConn 0x%p)",
					pMgr, pUrnPoolPtr, pRecvFrom, hConn));
				*phSigCompCompartment = NULL;
				return rv;
			}
		}
	}

	pCompartment = (Compartment *)(*phSigCompCompartment);

	rv = CompartmentLockMsg(pCompartment);
	if (rv != RV_OK)
	{
		RvLogError(pMgr->pLogSrc ,(pMgr->pLogSrc ,
			"SipCompartmentRelateCompHandleToInMsg - pMgr 0x%p failed lock compartment 0x%p",
			pMgr, pCompartment));
		*phSigCompCompartment = NULL;
		return rv;
	}
	strAlgoName = (pCompartment->strAlgoName[0] == '\0' ? COMPARTMENT_DEFAULT_ALGORITHM : 
														  pCompartment->strAlgoName);

	/* relate the compartment with the sigcomp internal compartment */
	rv = RvSigCompDeclareCompartmentEx(pMgr->hSigCompMgr, uniqueID, strAlgoName, &(pCompartment->hInternalComp));
	if (rv != RV_OK)
	{
        RvStatus                   inRv;
        RvSigCompCompartmentHandle hUnAuthInternalComp =
                    (RvSigCompCompartmentHandle)RVSIGCOMP_UNAUTHORIZED_COMPARTMENT_ID;

        RvLogError(pCompartment->pMgr->pLogSrc ,(pCompartment->pMgr->pLogSrc ,
            "SipCompartmentRelateCompHandleToInMsg - Compartment 0x%p, Failed to relate to unique ID %d - free udvm",
            pCompartment,uniqueID));

        /* Free the UDVM resources, which were used for decompression */
        /* WITHOUT changing the current compartment internal data     */
        inRv = RvSigCompDeclareCompartmentEx(
                                pCompartment->pMgr->hSigCompMgr,
                                uniqueID,
								strAlgoName,
                                &hUnAuthInternalComp); 
        if (inRv != RV_OK)
        {
            RvLogError(pCompartment->pMgr->pLogSrc ,(pCompartment->pMgr->pLogSrc ,
            "SipCompartmentRelateCompHandleToInMsg - Compartment 0x%p, Failed to free unique ID %d",
            pCompartment,uniqueID));
        }
		*phSigCompCompartment = NULL;
	}
	else
	{
		RvLogDebug(pMgr->pLogSrc ,(pMgr->pLogSrc ,
			"SipCompartmentRelateCompHandleToInMsg - pMgr 0x%p relate SigComp unique ID %d with compartment 0x%p",
			pMgr,uniqueID, pCompartment));
	}
	CompartmentUnLockMsg(pCompartment);
	return rv;
}

/***************************************************************************
 * SipCompartmentRelateCompHandleToOutMsg
 * ------------------------------------------------------------------------
 * General: returns the associated compartment to the outgoing message. If the compartment
 *          is not found, the function creates it based on message details.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hCompartmentMgr - the compartment manager
 *			 pUrnPoolPtr     - pointer to the RPOOL containing the sigcomp-id
 *           pSrc            - the source address of the message
 *			 pSendTo         - address and port of the message destination
 *           hMsg            - handle to the message associated with the compartment
 *			 hConn           - the connection handle, in case of TCP
 * Output:   phSigCompCompartment - pointer to the found/created compartment handle
 ***************************************************************************/
RvStatus RVCALLCONV SipCompartmentRelateCompHandleToOutMsg (
				IN		RvSipCompartmentMgrHandle	hCompartmentMgr,
				IN		RPOOL_Ptr					*pUrnPoolPtr,
				IN		SipTransportAddress			*pSrc,
				IN		SipTransportAddress			*pSendTo,
				IN      RvSipMsgHandle                 hMsg, 
				IN		RvSipTransportConnectionHandle hConn,
				INOUT	RvSipCompartmentHandle		*phSigCompCompartment)
{
	RvStatus rv = RV_OK;
	CompartmentMgr *pMgr;

	if ((phSigCompCompartment == NULL) || (hCompartmentMgr == NULL))
	{
		return RV_ERROR_INVALID_HANDLE;
	}
	
	pMgr = (CompartmentMgr *)hCompartmentMgr;

	/* check if compartment is NULL */
	if (*phSigCompCompartment == NULL)
	{
		/* look for the compartment. If not exist, create it */
		rv = CompartmentMgrFindCompartment(pMgr, pUrnPoolPtr, pSendTo, hConn, phSigCompCompartment);
		if (RV_ERROR_NOT_FOUND == rv)
		{
			RvLogInfo(pMgr->pLogSrc ,(pMgr->pLogSrc ,
				"SipCompartmentRelateCompHandleToOutMsg - pMgr 0x%p failed to find compartment, creating new (pUrnPoolPtr 0x%p, pSendTo 0x%p, hConn 0x%p)",
				pMgr, pUrnPoolPtr, pSendTo, hConn));

			/* compartment not found, create new one */
			rv = CompartmentMgrCreateCompartment(pMgr, pUrnPoolPtr, pSrc, pSendTo, RV_FALSE, hMsg, hConn, phSigCompCompartment);
			if (rv != RV_OK)
			{
				RvLogError(pMgr->pLogSrc ,(pMgr->pLogSrc ,
					"SipCompartmentRelateCompHandleToOutMsg - pMgr 0x%p failed to create new compartment (pUrnPoolPtr 0x%p, pSendTo 0x%p, hConn 0x%p)",
					pMgr, pUrnPoolPtr, pSendTo, hConn));
				*phSigCompCompartment = NULL;
				return rv;
			}
		}
	}

	return rv;
}

/***************************************************************************
 * SipCompartmentAttach
 * ------------------------------------------------------------------------
 * General: Attaching to an existing compartment.
 * Return Value: The return value status.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hCompartment - Handle to the compartment.
 ***************************************************************************/
RvStatus RVCALLCONV SipCompartmentAttach(
                        IN RvSipCompartmentHandle hCompartment,
                        IN void*                  hOwner)
{
    RvStatus     rv;
    Compartment *pCompartment = (Compartment *)hCompartment;

	rv = CompartmentLockMsg(pCompartment);
	if (rv != RV_OK)
	{
		RvLogError(pCompartment->pMgr->pLogSrc ,(pCompartment->pMgr->pLogSrc ,
			"SipCompartmentAttach - failed lock compartment 0x%p",
			pCompartment));
		return rv;
	}
    rv = CompartmentAttach(pCompartment, hOwner);
    CompartmentUnLockMsg(pCompartment);

    return rv;
}

/***************************************************************************
 * SipCompartmentDetach
 * ------------------------------------------------------------------------
 * General: Detaching from an existing compartment.
 * Return Value: The return value status.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hCompartment - Handle to the compartment.
 ***************************************************************************/
RvStatus RVCALLCONV SipCompartmentDetach(
					    IN RvSipCompartmentHandle hCompartment,
                        IN void*                  hOwner)
{
	RvStatus	rv;
	Compartment *pCompartment = (Compartment *)hCompartment;

	rv = CompartmentLockMsg(pCompartment);
	if (rv != RV_OK)
	{
		RvLogError(pCompartment->pMgr->pLogSrc ,(pCompartment->pMgr->pLogSrc ,
			"SipCompartmentDetach - failed lock compartment 0x%p", pCompartment));
		return rv;
	}
    rv = CompartmentDetach(pCompartment, hOwner);
    CompartmentUnLockMsg(pCompartment);

	return (rv);
}

/***************************************************************************
 * SipCompartmentCompressMessage
 * ------------------------------------------------------------------------
 * General: Compress a message. To be called before sending.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hCompartment   - Handle to the Compartment. In case that the it's
 *                          value is NULL the given unique ID is release.
 * Input/Output:
 *         pMessageInfo   - a pointer to 'SipSigCompMessageInfo' structure,
 *                          holds the compressed message + its size
 ***************************************************************************/
RvStatus RVCALLCONV SipCompartmentCompressMessage(
                   IN    RvSipCompartmentHandle  hCompartment,
                   INOUT RvSigCompMessageInfo   *pMessageInfo)
{
    Compartment  *pCompartment     = (Compartment *)hCompartment;
    RvStatus      rv               = RV_OK;

    rv = CompartmentLockMsg(pCompartment);
	if (rv != RV_OK)
	{
		RvLogError(pCompartment->pMgr->pLogSrc ,(pCompartment->pMgr->pLogSrc ,
			"SipCompartmentCompressMessage - failed lock compartment 0x%p",
			pCompartment));
		return rv;
	}

    rv = RvSigCompCompressMessageEx(
		pCompartment->pMgr->hSigCompMgr,
		(pCompartment->strAlgoName[0] == '\0' ? NULL : pCompartment->strAlgoName),
		&(pCompartment->hInternalComp),
		pMessageInfo);
	
    if (rv != RV_OK)
    {
        RvLogError(pCompartment->pMgr->pLogSrc ,(pCompartment->pMgr->pLogSrc ,
            "SipCompartmentCompressMessage - Compartment 0x%p, Failed to compress messege",
            pCompartment));
    }
#ifdef PRINT_SIGCOMP_BYTE_STREAM
	SipCompartmentPrintMsgByteStream(pMessageInfo->pCompressedMessageBuffer,
									 pMessageInfo->compressedMessageBuffSize,
									 pCompartment->pMgr->pLogSrc,
									 RV_FALSE);
#endif /* PRINT_SIGCOMP_BYTE_STREAM */
	
    CompartmentUnLockMsg(pCompartment);

    return rv;
}

/***************************************************************************
 * SipCompartmentAddToMgr
 * ------------------------------------------------------------------------
 * General: Adds a compartment to be managed by the compartment manager
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hCompartment   - Handle to the Compartment.
 *         hConn          - Handle to the connection-oriented object
 ***************************************************************************/
RvStatus RVCALLCONV SipCompartmentAddToMgr(IN RvSipCompartmentHandle  hCompartment,
										   IN RvSipTransportConnectionHandle hConn)
{
	Compartment    *pCompartment = (Compartment *)hCompartment;
	CompartmentMgr *pMgr;
	RvStatus        rv;

	if (NULL == hCompartment)
	{
		return (RV_OK);
	}

	pMgr = pCompartment->pMgr;
	rv = CompartmentMgrAddCompartment(pMgr, hCompartment, hConn);

	if (rv != RV_OK)
	{
        RvLogError(pCompartment->pMgr->pLogSrc ,(pCompartment->pMgr->pLogSrc ,
            "SipCompartmentAddToMgr - Compartment 0x%p, Failed to assigned with mgr",
            pCompartment));
	}
	return (rv);
}

#ifdef PRINT_SIGCOMP_BYTE_STREAM
#define BYTES_PER_LINE 10
/***************************************************************************
 * SipCompartmentPrintMsgByteStream
 * ------------------------------------------------------------------------
 * General: Prints to the log a SigComp message byte stream. 
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pStreamBuf  - The printed stream message. 
 *		   streamSize  - The printed stream message size. 
 *		   pLogSrc     - The log source (used for the printing). 
 *		   bInOrOut    - Indication if the stream is sent (was compressed) 
 *						 or received (before decompression). 
 ***************************************************************************/
void RVCALLCONV SipCompartmentPrintMsgByteStream(IN RvUint8     *pStreamBuf,
												 IN RvUint32     streamSize,
												 IN RvLogSource *pLogSrc,
												 IN RV_BOOL	     bInOrOut)
{
	RvUint32 currByte      = 0;
	RvChar   data[BYTES_PER_LINE*10+100]; /* 10 chars for each byte + spaces and more */
	RvUint8	*pSrcPos       = pStreamBuf;
	RvChar	*pDstPos       = data;
	RvChar	*strInOrOut    = (bInOrOut == RV_TRUE)? "before decompression" : "after compression";
	
	RvLogDebug(pLogSrc ,(pLogSrc ,
		"SipCompartmentPrintMsgByteStream - the stream message(%d) %s is.....",
		streamSize,strInOrOut));
	
	memset(data,0,sizeof(data));
	while (currByte < streamSize)
	{
		pDstPos  += RvSprintf(pDstPos,"0x%p ",*pSrcPos); 
		pSrcPos  += 1;
		currByte += 1;
		
		/* Print a complete line */
		if (fmod(currByte,BYTES_PER_LINE) == 0)
		{
			RvLogDebug(pLogSrc ,(pLogSrc ,data));
			pDstPos = data;
			memset(data,0,sizeof(data));
		}
	}
	/* Print the left bytes */
	if (pDstPos > data)
	{
		RvLogDebug(pLogSrc ,(pLogSrc ,data));
	}
}
#endif /* PRINT_SIGCOMP_BYTE_STREAM */

#endif /* RV_SIGCOMP_ON */



