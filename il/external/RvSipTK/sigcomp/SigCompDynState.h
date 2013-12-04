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
 *                              <SigCompDynState.h>
 *
 * This file defines dynamic compression utility functions
 *
 *    Author                         Date
 *    ------                        ------
 *    Sasha G                      July 2006
 *********************************************************************************/

#ifndef SIGCOMP_DYN_STATE_H
#define SIGCOMP_DYN_STATE_H

#if defined(__cplusplus)
extern "C" {
#endif
	
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIGCOMP_DEF.h"
#include "RvSigCompTypes.h"

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/*************************************************************************
* SigCompDynStateCompress
* ------------------------------------------------------------------------
* General: This function performs a dynamic compression using given 
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
RvStatus RVCALLCONV SigCompDynStateCompress(
						IN  RvSigCompDynState		  *self,
						IN  RvSigCompCompartmentHandle hCompartment,
						IN  RvSigCompCompressionInfo  *pCompressionInfo,
						OUT RvSigCompMessageInfo      *pMsgInfo);

/*************************************************************************
* SigCompDynStateConstruct
* ------------------------------------------------------------------------
* General: This function construct a dynamic compression state
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: hSigCompMgr - SigComp manager handle 
*		 pSelf       - Pointer to the dynamic state
*        initStream  - Pointer to the stream to be related to the dynamic
*					   state.
*************************************************************************/
RvStatus RVCALLCONV SigCompDynStateConstruct(
										IN RvSigCompDynState   *self, 
										IN RvSigCompDynZStream *initStream,
										IN RvSigCompMgrHandle   hSigCompMgr); 

/*************************************************************************
* SigCompDynStateDestruct
* ------------------------------------------------------------------------
* General: This function destruct a dynamic compression state
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: pSelf       - Pointer to the destructed dynamic state
*************************************************************************/
RvStatus SigCompDynStateDestruct(IN RvSigCompDynState *self);


/*************************************************************************
* SigCompDynStateAnalyzeFeedback
* ------------------------------------------------------------------------
* General: Strategy before we get first acknowledge (e.g. before we know 
*		   how much space is available for saving states at peer: each 
*		   message is compressed relative to sip dictionary and sent with 
*		   priority 0 and sequential zid. 
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments:
* Input: pSelf       - Pointer to the dynamic state
*************************************************************************/
RvStatus RVCALLCONV SigCompDynStateAnalyzeFeedback(
						IN RvSigCompDynState        *self, 
						IN RvSigCompCompressionInfo *info); 


#if defined(__cplusplus)
}
#endif

#endif /* endof SIGCOMP_DYN_STATE_H */ 

