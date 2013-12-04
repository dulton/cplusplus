
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
 *                              <CompartmentObject.h>
 *
 * This file defines the  compartment object, attributes and interface
 * functions. The compartment represents a  Compartment as defined in RFC3320.
 * This compartment unifies a group of SIP Stack objects such as CallLegs or
 * Transacions that is identify by a compartment ID when sending request
 * through a compressor entity.
 *
 *    Author                         Date
 *    ------                        ------
 *    Dikla Dror                  Nov 2003
 *********************************************************************************/

#ifndef COMPARTMENT_OBJECT_H
#define COMPARTMENT_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "rvmutex.h"
#include "RV_SIP_DEF.h"
#include "CompartmentMgrObject.h"
#include "RvSipCompartmentTypes.h"

#ifdef RV_SIGCOMP_ON
#include "RvSigComp.h"
#include "_SipTransportTypes.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

#define LOCK_MGR(pMgr)    RvMutexLock  (&pMgr->lock,pMgr->pLogMgr)
#define UNLOCK_MGR(pMgr)  RvMutexUnlock(&pMgr->lock,pMgr->pLogMgr)

/* Compartment
 * --------------------------------------------------------------------------
 * The Compartment defines an  Compartment as defined in RFC3320.
 * This compartment unifies a group of SIP Stack objects such as CallLegs or
 * Transactions that is identify by a compartment ID when sending request
 * through a compressor entity.
 *
 * compTripleLock  -- TripleLock object for protecting the compartment object in
 *                    multi threaded app.
 * pTripleLock     -- pointer to the tripleLock object, for lock sharing
 * referenceCount  -- A counter that contains the number of Stack object
 *                   that refer to the compartment object.
 * pMgr            -- Pointer to the compartment manager object.
 * hAppCompartment -- Application handle of the compartment.
 * hInternalComp   -- Handle to the internal SigComp DLL compartment
 * hashElemPtr     -- element object for hash management
 * hPageForUrn     -- page to store the sigcomp ID associated with this compartment
 * offsetInPage    -- offset of the sigcompID string inside the page (hPageForUrn)
 * srcAddr         -- the transport source address of the message created the compartment
 * dstAddr         -- the transport destination address of the message created the compartment
 * hConnection     -- in case of TCP, the connection handle associated with this compartment
 * compartmentUniqueIdentifier -- A unique identifier for this Compartment.
 *                    It is generated randomly on Compartment creation.
 *					  If it is 0, the Compartment object is vacant.
 * bIsApproved     -- flag the compartment as approved by application
 * bIsHashed       -- flag indicating if the object is in the hash
 * bIsInCreatedEv  -- flag indicating if the compartment is accessed from createdEv.
 * bIsSrc          -- Flag indicating the address associated with this compartment.
 * compType        -- The compartment type
 * hMsg            -- Handle to the message resulted the creation of the compartment.
 * algoName        -- The algorithm name which will affect the 
 *					  compartment compression manner. The algorithm
 *					  name MUST be equal to the set algorithm name 
 *					  during algorithm initialization. If this
 *					  parameter value is NULL the default also is used
 */
typedef struct
{
    SipTripleLock					compTripleLock;
	SipTripleLock					*pTripleLock;
    RvInt32							referenceCount;
    CompartmentMgr					*pMgr;
    RvSipAppCompartmentHandle		hAppCompartment;
    RvSigCompCompartmentHandle		hInternalComp;
	void							*hashElemPtr;
	HPAGE                           hPageForUrn;
	RvInt32                         offsetInPage;
	SipTransportAddress             srcAddr;
	SipTransportAddress				dstAddr;
	RvSipTransportConnectionHandle  hConnection;
	RvInt32                         compartmentUniqueIdentifier;
	RvBool                          bIsApproved;
	RvBool                          bIsHashed;
	RvBool                          bIsInCreatedEv;
	RvBool                          bIsSrc;
	RvSipCompartmentType            compType;
	RvSipMsgHandle                  hMsg; 
	RvChar				            strAlgoName[RVSIGCOMP_ALGO_NAME];
}  Compartment;



typedef struct
{
	CompartmentMgr		      *pCompartmentMgr;
	RvSipCompartmentType      keyType;
	/* for URN-based key */
	RPOOL_Ptr            urn;
	/* for ip:port key, UDP/SCTP transport */
	SipTransportAddress  *pAddress;
	/* for connection-based key, for non UDP transport */
	RvSipTransportConnectionHandle  hConnection;
}  CompartmentHashKeyElement;


/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * CompartmentInitialize
 * ------------------------------------------------------------------------
 * General: Initialize a new compartment object. Sets all values to their
 *          initial values.
 * Return Value: RV_ERROR_UNKNOWN - Failed to initialize the compartment.
 *               RV_OK - Call-Id was generated successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pCompartment - pointer to the new Compartment.
 *            pMgr         - pointer to the manager of this call.
 ***************************************************************************/
RvStatus RVCALLCONV CompartmentInitialize(
                            IN  Compartment     *pCompartment,
                            IN  CompartmentMgr  *pMgr);

/***************************************************************************
 * CompartmentAttach
 * ------------------------------------------------------------------------
 * General: Attaching to an existing compartment.
 *
 * Return Value: The return value status.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCompartment - pointer to the  compartment.
 *        hOwner       - Handle of the detached owner.
 ***************************************************************************/
RvStatus RVCALLCONV CompartmentAttach(IN Compartment *pCompartment,
                                      IN void*        hOwner);

/***************************************************************************
 * CompartmentDetach
 * ------------------------------------------------------------------------
 * General: Detaching from an existing compartment. In case that reference
 *          count becomes zero the compartment is terminated automatically,
 *          since the comparment is not used by any of the stack objects or
 *          by the application above.
 *
 * Return Value: The return value status.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCompartment - pointer to the  compartment.
 *        hOwner       - Handle of the detached owner.
 ***************************************************************************/
RvStatus RVCALLCONV CompartmentDetach(IN Compartment *pCompartment,
                                      IN void*        hOwner);

/***************************************************************************
 * CompartmentTerminate
 * ------------------------------------------------------------------------
 * General: Destructs a Compartment object, Free the resources taken
 *          by it and remove it from the manager compartments list.
 * Return Value: RV_OK - If the compartment is destructed successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCompartment - pointer to the terminated  Compartment
 ***************************************************************************/
RvStatus RVCALLCONV CompartmentTerminate(
                           IN Compartment *pCompartment);

#if (RV_THREADNESS_TYPE != RV_THREADNESS_SINGLE)
/************************************************************************************
 * CompartmentLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks compartment according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCompartment - pointer to the compartment.
***********************************************************************************/
RvStatus RVCALLCONV CompartmentLockAPI(IN  Compartment*   pCompartment);

/************************************************************************************
 * CompartmentUnLockAPI
 * ----------------------------------------------------------------------------------
 * General: Locks Compartment according to API schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCompartment - pointer to the compartment.
***********************************************************************************/
void RVCALLCONV CompartmentUnLockAPI(IN  Compartment*   pCompartment);

/************************************************************************************
 * CompartmentLockMsg
 * ----------------------------------------------------------------------------------
 * General: Locks compartment according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCompartment - pointer to the compartment
***********************************************************************************/
RvStatus RVCALLCONV CompartmentLockMsg(IN  Compartment*   pCompartment);

/************************************************************************************
 * CompartmentUnLockMsg
 * ----------------------------------------------------------------------------------
 * General: UnLocks compartment according to MSG processing schema
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pCompartment - pointer to the compartment.
***********************************************************************************/
void RVCALLCONV CompartmentUnLockMsg(IN  Compartment*   pCompartment);

#else
#define CompartmentLockAPI(a) (RV_OK)
#define CompartmentUnLockAPI(a)
#define CompartmentLockMsg(a) (RV_OK)
#define CompartmentUnLockMsg(a)

#endif /* RV_THREADNESS_TYPE */

#endif /* RV_SIGCOMP_ON */

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef COMPARTMENT_OBJECT_H*/


