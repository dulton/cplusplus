
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

#ifndef SIP_COMPARTMENT_H
#define SIP_COMPARTMENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "AdsRlist.h"
#include "_SipCommonUtils.h"
#include "rvlog.h"
#include "RV_SIP_DEF.h"
#include "RvSipResourcesTypes.h"
#include "RvSipTransportTypes.h"
#include "RvSipCompartmentTypes.h"

#ifdef RV_SIGCOMP_ON
#include "RvSigComp.h"
#include "_SipTransportTypes.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
#define COMPARTMENT_DEFAULT_ALGORITHM NULL

typedef struct
{
    RvLogSource              *pLogSrc;
    RvLogMgr                 *pLogMgr;
    RvInt32                   maxNumOfCompartments;
    RvRandomGenerator        *pSeed;
} SipCompartmentMgrCfg;

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                     COMPARMENT MANAGER API                     */
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
                OUT RvSipCompartmentMgrHandle *phCompartmentMgr);

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
                  IN RvSipCompartmentMgrHandle hCompartmentMgr);

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
RvStatus RVCALLCONV SipCompartmentMgrGetResourcesStatus (
                                 IN  RvSipCompartmentMgrHandle  hMgr,
                                 OUT RvSipCompartmentResources *pResources);

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
                                 IN  RvSipCompartmentMgrHandle  hMgr);

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
                      IN   RvSigCompMgrHandle        hSigCompMgr);

/***************************************************************************
 * SipCompartmentMgrDetachFromSigCompModule
 * ------------------------------------------------------------------------
 * General: Release all the relations between SipStack compartments and 
 *          SigComp internal compartments.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hCompartmentMgr - Handle to the stack comp. manager.
 * Output:   -
 ***************************************************************************/
RvStatus RVCALLCONV SipCompartmentMgrDetachFromSigCompModule(
                      IN   RvSipCompartmentMgrHandle hCompartmentMgr);

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
                      IN   RvSipCompartmentHandle hCompartment);

/*-----------------------------------------------------------------------*/
/*                           COMPARMENT API                              */
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
				INOUT	RvSipCompartmentHandle		*phSigCompCompartment);

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
				INOUT	RvSipCompartmentHandle		*phSigCompCompartment);

/***************************************************************************
 * SipCompartmentAttach
 * ------------------------------------------------------------------------
 * General: Attaching to an existing compartment.
 * Return Value: The return value status.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: hCompartment - Handle to the compartment.
 *        hOwner       - Handle of the detached owner.
 ***************************************************************************/
RvStatus RVCALLCONV SipCompartmentAttach(
                        IN RvSipCompartmentHandle hCompartment,
                        IN void*                  hOwner);

/***************************************************************************
 * SipCompartmentDetach
 * ------------------------------------------------------------------------
 * General: Detaching from an existing compartment.
 * Return Value: The return value status.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input: pCompartment - Handle to the compartment.
 *        hOwner       - Handle of the detached owner.
 ***************************************************************************/
RvStatus RVCALLCONV SipCompartmentDetach(
					    IN RvSipCompartmentHandle hCompartment,
                        IN void*                  hOwner);

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
                   INOUT RvSigCompMessageInfo   *pMessageInfo);

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
										   IN RvSipTransportConnectionHandle hConn);

#ifdef PRINT_SIGCOMP_BYTE_STREAM
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
												 IN RV_BOOL	     bInOrOut);
#endif /* PRINT_SIGCOMP_BYTE_STREAM */

#endif /* RV_SIGCOMP_ON */

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef SIP_COMPARTMENT_H*/


