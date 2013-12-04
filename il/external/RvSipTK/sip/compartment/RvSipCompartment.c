
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
 *                              <RvSipCompartment.c>
 *
 * The  functions of the RADVISION SIP stack enable you to
 * create and manage Compartment objects, attach/detach to/from
 * compartments and control the compartment parameters.
 *
 *
 * Compartment API functions are grouped as follows:
 * The  Compartment Manager API
 * ------------------------------------
 * The Compartment manager is in charge of all the compartments. It is used
 * to create new compartments.
 *
 * The  Compartment API
 * ----------------------------
 * A compartment represents a SIP  Compartment as defined in RFC3320.
 * This compartment unifies a group of SIP Stack objects such as CallLegs or
 * Transacions that is identify by a compartment ID when sending request
 * through a compressor entity. Using the API, the user can initiate compartments,
 * or destruct it. Functions to set and access the compartment fields are also
 * available in the API.
 *
 *    Author                         Date
 *    ------                        ------
 *    Dikla Dror                  Nov 2003
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonCore.h"
#include "CompartmentObject.h"
#include "CompartmentMgrObject.h"
#include "RvSipCompartmentTypes.h"
#include "RvSipCompartment.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                      COMPARTMENT MANAGER  API                         */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipCompartmentMgrCreateCompartmentEx
 * ------------------------------------------------------------------------
 * General: Creates a new Compartment, related to a specific algorithm.
 *			The creation triggers an automatic attachment to the newly 
 *			created compartment. Each SigComp message, sent by the
 *			created compartment, will be compressed using the given 
 *			algorithm.
 * NOTE: The given algorithm must be configured firstly, using the SigComp 
 *		 module API function RvSigCompAlgorithmAdd(), i.e the algorithm name
 *		 MUST match the configured RvSigCompAlgorithm.algorithmName parameter
 *
 *		 
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the manager is invalid.
 *               RV_ERROR_NULLPTR -     The pointer to the compartment handle
 *                                   is invalid.
 *               RV_ERROR_OUTOFRESOURCES - Compartment list is full,compartment
 *                                   was not created.
 *               RV_OK -        Success.
 *
 * NOTE: Function is deprecated. 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hCompartmentMgr - Handle to the  compartment manager
 *         hAppCompartment - Application handle to the compartment.
 *		   strAlgoName     - The algorithm name which will affect the 
 *		 					 compartment compression manner. The algorithm
 *							 name MUST be equal to the set algorithm name 
 *						     during algorithm initialization. If this
 *						     parameter value is NULL the default algo is used
 * Output: phCompartment   - RADVISION SIP stack handle to the compartment.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCompartmentMgrCreateCompartmentEx(
                IN  RvSipCompartmentMgrHandle hCompartmentMgr,
                IN  RvSipAppCompartmentHandle hAppCompartment,
				IN  RvChar					 *strAlgoName,
                OUT RvSipCompartmentHandle   *phCompartment)
{

    RV_UNUSED_ARG(hCompartmentMgr);
    RV_UNUSED_ARG(hAppCompartment || strAlgoName);

    if (NULL != phCompartment)
    {
        *phCompartment = NULL;
    }
    return RV_ERROR_ILLEGAL_ACTION;
}

/***************************************************************************
 * RvSipCompartmentMgrCreateCompartment
 * ------------------------------------------------------------------------
 * General: Creates a new Compartment, related to the SigComp module default 
 *			compression algorithm. The creation triggers an automatic 
 *			attachment to the newly created compartment.
 *
 * Return Value: RV_ERROR_INVALID_HANDLE -  The handle to the manager is invalid.
 *               RV_ERROR_NULLPTR -     The pointer to the compartment handle
 *                                   is invalid.
 *               RV_ERROR_OUTOFRESOURCES - Compartment list is full,compartment
 *                                   was not created.
 *               RV_OK -        Success.
 *
 * Note: Function is deprecated.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hCompartmentMgr - Handle to the  compartment manager
 *         hAppCompartment - Application handle to the compartment.
 * Output: phCompartment   - RADVISION SIP stack handle to the compartment.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCompartmentMgrCreateCompartment(
				   IN  RvSipCompartmentMgrHandle hCompartmentMgr,
				   IN  RvSipAppCompartmentHandle hAppCompartment,
				   OUT RvSipCompartmentHandle   *phCompartment)
{
    RV_UNUSED_ARG(hCompartmentMgr || hAppCompartment)
    *phCompartment = NULL;
    return RV_ERROR_ILLEGAL_ACTION;
}

/***************************************************************************
 * RvSipCompartmentMgrGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this
 *          compartment manager belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCompartmentMgr - Handle to the stack compartment manager.
 * Output:    phStackInstance - A valid pointer which will be updated with a
 *                              handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCompartmentMgrGetStackInstance(
                      IN   RvSipCompartmentMgrHandle hCompartmentMgr,
                      OUT  void                    **phStackInstance)
{
#ifdef RV_SIGCOMP_ON
    CompartmentMgr *pMgr;

    if (NULL == phStackInstance)
    {
        return RV_ERROR_NULLPTR;
    }

    *phStackInstance = NULL;

    if(NULL == hCompartmentMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    pMgr = (CompartmentMgr *)hCompartmentMgr;

    LOCK_MGR(pMgr); /*lock the manager*/
    *phStackInstance = pMgr->hStack;
    UNLOCK_MGR(pMgr); /*unlock the manager*/

    return RV_OK;
#else /* RV_SIGCOMP_ON */
    RV_UNUSED_ARG(hCompartmentMgr);
    if (NULL != phStackInstance)
    {
        *phStackInstance = NULL;
    }

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* RV_SIGCOMP_ON */
}

/*-----------------------------------------------------------------------*/
/*                        COMPARTMENT API                                */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * RvSipCompartmentDetach
 * ------------------------------------------------------------------------
 * General: Detaches from a  Compartment. If the compartment is not
 *          used by any of the stack objects (Call-Leg/Transaction etc.)
 *          or even by the application it's physically deleted. Otherwise 
 *          the termination will be only logically.
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCompartment - Handle to the  compartment the application
 *                          wishes to detach from.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCompartmentDetach(
                        IN RvSipCompartmentHandle hCompartment)
{
#ifdef RV_SIGCOMP_ON
    RvStatus    rv;
    Compartment *pCompartment;

    if (hCompartment == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    pCompartment = (Compartment*)hCompartment;

    RvLogInfo(pCompartment->pMgr->pLogSrc,(pCompartment->pMgr->pLogSrc,
              "RvSipCompartmentDetach - Detaching from Compartment... 0x%p",pCompartment));

    rv = CompartmentLockAPI(pCompartment);
    if(rv != RV_OK)
    {
        RvLogError(pCompartment->pMgr->pLogSrc,(pCompartment->pMgr->pLogSrc,
            "RvSipCompartmentDetach - Compartment 0x%p - failed to lock the compartment",
            hCompartment));
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = CompartmentDetach(pCompartment, NULL/*hOwner*/);
    CompartmentUnLockAPI(pCompartment);

    return rv;
#else /* RV_SIGCOMP_ON */
    RV_UNUSED_ARG(hCompartment);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* RV_SIGCOMP_ON */
}

/***************************************************************************
 * RvSipCompartmentAttach
 * ------------------------------------------------------------------------
 * General: Attaches to a  Compartment. This function enables the
 *          application to attach a compartment. This attachment keeps the 
 *          compartment "alive" until the compartment is detached by all objects
 *          attached to it and by the application.
 *
 * Note:    This function can be called only within the CompartmentCreatedEv
 *          Callback. Any attempt to call it outside of that scope will result
 *          RV_ERROR_ILLEGAL_ACTION.
 *
 * Return Value:RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCompartment - Handle to the  compartment the application
 *                          wishes to attach to.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCompartmentAttach(
                        IN RvSipCompartmentHandle hCompartment)
{
#ifdef RV_SIGCOMP_ON
    RvStatus           rv;
    Compartment *pCompartment;

    if (hCompartment == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    pCompartment = (Compartment*)hCompartment;

    RvLogInfo(pCompartment->pMgr->pLogSrc,(pCompartment->pMgr->pLogSrc,
              "RvSipCompartmentAttach - Attaching to Compartment 0x%p",pCompartment));

    rv = CompartmentLockAPI(pCompartment);
    if(rv != RV_OK)
    {
        RvLogError(pCompartment->pMgr->pLogSrc,(pCompartment->pMgr->pLogSrc,
            "RvSipCompartmentAttach - Compartment 0x%p - failed to lock the compartment",
            hCompartment));
        return RV_ERROR_INVALID_HANDLE;
    }


	if (pCompartment->bIsInCreatedEv == RV_TRUE)
	{
	    rv = CompartmentAttach(pCompartment,NULL/*hOwner*/);
	}
	else
	{
	    RvLogError(pCompartment->pMgr->pLogSrc,(pCompartment->pMgr->pLogSrc,
		          "RvSipCompartmentAttach - Cannot call from outside the CompartmentCreatedEv Callback, Compartment 0x%p",pCompartment));
		rv = RV_ERROR_ILLEGAL_ACTION;
	}

    CompartmentUnLockAPI(pCompartment);

    return rv;
#else /* RV_SIGCOMP_ON */
    RV_UNUSED_ARG(hCompartment);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* RV_SIGCOMP_ON */
}

/***************************************************************************
 * RvSipCompartmentSetAppHandle
 * ------------------------------------------------------------------------
 * General: Sets the  Compartment application handle. Usually the
 *          application replaces handles with the stack in the
 *          RvSipCompartmentMgrCreateCompartment() API function.
 *          This function is used if the application wishes to set a new
 *          application handle.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCompartment    - Handle to the Compartment.
 *            hAppCompartment - A new application handle to the Compartment.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCompartmentSetAppHandle(
                   IN  RvSipCompartmentHandle     hCompartment,
                   IN  RvSipAppCompartmentHandle  hAppCompartment)
{
#ifdef RV_SIGCOMP_ON
    Compartment *pCompartment= (Compartment*)hCompartment;
	RvStatus     rv;

    if(pCompartment== NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /* Try to lock the Compartment */
    rv = CompartmentLockAPI(pCompartment);
    if(rv != RV_OK)
    {
        RvLogError(pCompartment->pMgr->pLogSrc,(pCompartment->pMgr->pLogSrc,
            "RvSipCompartmentSetAppHandle - Compartment 0x%p - failed to lock the compartment",
            hCompartment));
        return RV_ERROR_INVALID_HANDLE;
    }


    RvLogInfo(pCompartment->pMgr->pLogSrc,(pCompartment->pMgr->pLogSrc,
              "RvSipCompartmentSetAppHandle - Setting a Compartment 0x%p Application handle 0x%p",
              pCompartment,hAppCompartment));

    pCompartment->hAppCompartment = hAppCompartment;

    CompartmentUnLockAPI(pCompartment); /* unlock the compartment */

    return RV_OK;
#else /* RV_SIGCOMP_ON */
    RV_UNUSED_ARG(hCompartment);
    RV_UNUSED_ARG(hAppCompartment);

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* RV_SIGCOMP_ON */
}

/***************************************************************************
 * RvSipCompartmentGetAppHandle
 * ------------------------------------------------------------------------
 * General: Returns the application handle of this  Compartment.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCompartment     - Handle to the Compartment.
 * Output:     phAppCompartment - A pointer to application handle of the Compartment
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCompartmentGetAppHandle(
                   IN  RvSipCompartmentHandle     hCompartment,
                   OUT RvSipAppCompartmentHandle *phAppCompartment)
{
#ifdef RV_SIGCOMP_ON
    Compartment *pCompartment = (Compartment*)hCompartment;
	RvStatus     rv;

    if(pCompartment == NULL || phAppCompartment == NULL)
    {
        return RV_ERROR_NULLPTR;
    }

    /* Try to lock the Compartment*/
    rv = CompartmentLockAPI(pCompartment);
    if(rv != RV_OK)
    {
        RvLogError(pCompartment->pMgr->pLogSrc,(pCompartment->pMgr->pLogSrc,
            "RvSipCompartmentGetAppHandle - Compartment 0x%p - failed to lock the compartment",
            hCompartment));
        return RV_ERROR_INVALID_HANDLE;
    }


    *phAppCompartment = pCompartment->hAppCompartment;

    CompartmentUnLockAPI(pCompartment); /*unlock the call leg*/

    return RV_OK;
#else /* RV_SIGCOMP_ON */
    RV_UNUSED_ARG(hCompartment);
    if (NULL != phAppCompartment)
    {
        *phAppCompartment = NULL;
    }

    return RV_ERROR_ILLEGAL_ACTION;
#endif /* RV_SIGCOMP_ON */
}


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

RVAPI RvStatus RVCALLCONV RvSipCompartmentGetUrn(
                   IN  RvSipCompartmentHandle     hCompartment,
                   OUT RvChar *   purn)
{
   Compartment* pComp =   (Compartment*)hCompartment; 
   if(pComp && pComp->pMgr &&   pComp->pMgr->hCompartmentPool && pComp->hPageForUrn && pComp->offsetInPage)
       purn = (RvChar *)RPOOL_GetPtr(pComp->pMgr->hCompartmentPool, pComp->hPageForUrn,pComp->offsetInPage);
   return RV_OK;
}

#endif /* defined(UPDATED_BY_SPIRENT) */
/* SPIRENT_END */

/***************************************************************************
 * RvSipCompartmentMgrSetEvHandlers
 * ------------------------------------------------------------------------
 * General: Sets event handlers for all compartment events.
 * Return Value:RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *              RV_ERROR_NULLPTR    - Bad pointer to the event handler structure.
 *              RV_OK       - Success.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr - Handle to the compartment manager.
 *            pEvHandlers - Pointer to the application event handler structure
 *            structSize - The size of the event handler structure.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCompartmentMgrSetEvHandlers(
                                   IN  RvSipCompartmentMgrHandle   hMgr,
                                   IN  RvSipCompartmentEvHandlers *pEvHandlers,
                                   IN  RvInt32                     structSize)
{
#ifdef RV_SIGCOMP_ON
    CompartmentMgr       *pMgr;

    if (NULL == hMgr)
    {
        return RV_ERROR_INVALID_HANDLE;
    }
    if (NULL == pEvHandlers)
    {
        return RV_ERROR_NULLPTR;
    }
    if (0 > structSize)
    {
        return RV_ERROR_BADPARAM;
    }
    pMgr = (CompartmentMgr *)hMgr;

    LOCK_MGR(pMgr);
    /* Copy the event handlers struct according to the given size */
    memset(&(pMgr->compartmentEvHandlers), 0, sizeof(RvSipCompartmentEvHandlers));
    memcpy(&(pMgr->compartmentEvHandlers), pEvHandlers, structSize);
    UNLOCK_MGR(pMgr);

    return RV_OK;
#else
	RV_UNUSED_ARG(hMgr);
	RV_UNUSED_ARG(pEvHandlers);
	RV_UNUSED_ARG(structSize);
	return RV_ERROR_ILLEGAL_ACTION;
#endif /* RV_SIGCOMP_ON */
}

/***************************************************************************
 * RvSipCompartmentGetStackInstance
 * ------------------------------------------------------------------------
 * General: Returns the handle to the stack instance to which this compartment
 *          belongs to.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hCompartment    - Handle to the compartment object.
 * Output:     phStackInstance - A valid pointer which will be updated with a
 *                               handle to the stack instance.
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCompartmentGetStackInstance(
                                   IN   RvSipCompartmentHandle   hCompartment,
                                   OUT  void                   **phStackInstance)
{
#ifdef RV_SIGCOMP_ON
    Compartment          *pCompartment;
    RvStatus              rv;

    if (NULL == phStackInstance)
    {
        return RV_ERROR_NULLPTR;
    }
    *phStackInstance = NULL;

    pCompartment = (Compartment *)hCompartment;
    if(pCompartment == NULL)
    {
        return RV_ERROR_INVALID_HANDLE;
    }

    rv = CompartmentLockAPI(pCompartment);
    if(rv != RV_OK)
    {
        RvLogError(pCompartment->pMgr->pLogSrc,(pCompartment->pMgr->pLogSrc,
            "RvSipCompartmentGetStackInstance - Compartment 0x%p - failed to lock the compartment",
            hCompartment));
        return RV_ERROR_INVALID_HANDLE;
    }

    *phStackInstance = pCompartment->pMgr->hStack;

    CompartmentUnLockAPI(pCompartment);
    return RV_OK;
#else
    RV_UNUSED_ARG(hCompartment);
    if (NULL != phStackInstance)
    {
        *phStackInstance = NULL;
    }

    return RV_ERROR_ILLEGAL_ACTION;
#endif
}


