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
 *                              <RvSipCompartment.h>
 *
 * The  Compartment functions of the RADVISION SIP stack enable you to
 * create and manage  Compartment objects, attach/detach to/from
 * compartments and control the compartment parameters.
 *
 *
 *  Compartment API functions are grouped as follows:
 * The  Compartment Manager API
 * ------------------------------------
 * The  Compartment manager is in charge of all the compartments. It is used
 * to create new compartments.
 *
 * The  Compartment API
 * ----------------------------
 * A compartment represents a SIP  Compartment as defined in RFC3320.
 * This compartment unifies a group of SIP Stack objects such as CallLegs or
 * Transactions that is identify by a compartment ID when sending request
 * through a compressor entity. Using the API, the user can initiate compartments,
 * or destruct it. Functions to set and access the compartment fields are also
 * available in the API.
 *
 *    Author                         Date
 *    ------                        ------
 *    Dikla Dror                  Nov 2003
 *********************************************************************************/

#ifndef RV_SIP_COMPARTMENT_H
#define RV_SIP_COMPARTMENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "rpool_API.h"
#include "RvSipCompartmentTypes.h"
    
/*-----------------------------------------------------------------------*/
/*                      COMPARTMENT MANAGER  API                         */
/*-----------------------------------------------------------------------*/



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
                      OUT  void                    **phStackInstance);

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
				   OUT RvSipCompartmentHandle   *phCompartment);

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
                OUT RvSipCompartmentHandle   *phCompartment);

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
                                   IN  RvInt32                     structSize);

/*-----------------------------------------------------------------------*/
/*                             COMPARTMENT API                           */
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
                        IN RvSipCompartmentHandle hCompartment);

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
                        IN RvSipCompartmentHandle hCompartment);

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
                   IN  RvSipAppCompartmentHandle  hAppCompartment);

/***************************************************************************
 * RvSipCompartmentGetAppHandle
 * ------------------------------------------------------------------------
 * General: Returns the application handle of this Compartment.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hCompartment     - Handle to the Compartment.
 * Output:     phAppCompartment - A pointer to application handle of the Compartment
 ***************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipCompartmentGetAppHandle(
                   IN  RvSipCompartmentHandle     hCompartment,
                   OUT RvSipAppCompartmentHandle *phAppCompartment);


/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)

RVAPI RvStatus RVCALLCONV RvSipCompartmentGetUrn(
                   IN  RvSipCompartmentHandle     hCompartment,
                   OUT RvChar *   purn);

#endif /* defined(UPDATED_BY_SPIRENT) */
/* SPIRENT_END */

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
                                   OUT  void                   **phStackInstance);


#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef RV_SIP_COMPARTMENT_H*/


