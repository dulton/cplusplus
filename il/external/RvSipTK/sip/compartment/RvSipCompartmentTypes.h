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
 *                              <RvSipCompartmentTypes.h>
 *
 * The RvSipCompartmentTypes.h file contains all type definitions for the  module.
 *
 * includes:
 * 1.Handle Type definitions
 * 2.Compartment Type definitions
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Dikla Dror                  Nov 2003
 *********************************************************************************/

#ifndef RV_SIP_COMPARTMENT_TYPES_H
#define RV_SIP_COMPARTMENT_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "rpool_API.h"
#include "RvSipTransportTypes.h"

/*-----------------------------------------------------------------------*/
/*                     COMMON TYPE DEFINITIONS                           */
/*-----------------------------------------------------------------------*/

/*@**************************************************************************
 *  RvSipCompartmentType
 *  --------------------
 *  Represents the key type a compartment was created based on.
 *
 *  RVSIP_COMPARTMENT_TYPE_UNDEFINED : The type is not defined
 *  RVSIP_COMPARTMENT_TYPE_URN:        
 *                    The compartment was created based on a sigcomp-id key
 *  RVSIP_COMPARTMENT_TYPE_ADDRESS:
 *                    The compartment was created based on an IP:PORT key, meaning
 *                    connectionless transport.
 *  RVSIP_COMPARTMENT_TYPE_CONNECTION:
 *                    The compartment was created based on a connection-oriented
 *                    transport key.
 ****************************************************************************@*/
typedef enum
{
	RVSIP_COMPARTMENT_TYPE_UNDEFINED = -1,
	RVSIP_COMPARTMENT_TYPE_URN,
	RVSIP_COMPARTMENT_TYPE_ADDRESS,
	RVSIP_COMPARTMENT_TYPE_CONNECTION
} RvSipCompartmentType;
	
 
/* SIP/SigComp parameters */ 
#define RVSIP_COMPARTMENT_MAX_URN_LENGTH        128

/*@***********************************************************************
 * RvSipCompatmentCbInfo - compartment information structure.
 *                         This structure holds several parameters helping the
 *                         calling layer understand the nature and type of the 
 *                         compartment.
 *
 ************************************************************************@*/                         
typedef struct  
{
	RvSipCompartmentType			compType;
					/* compartment type, based on the key used to create it */
	HPAGE                           hSigCompIdPage;
	                /* The page holding the sigcomp-id, if applicable */
	RvInt32                         sigCompIdOffset;
	                /* The offset in hSigcompIdPage */
	RvSipTransportAddr             *pSrcAddr;
					/* The source address of the message created the compartment */
	RvSipTransportAddr             *pDstAddr;
					/* The destination address of the message created the compartment */
	RvBool                          bIsIncoming;
					/* indicating the direction of the message, to understand
					   which of the two addresses were used to create the compartment,
					   if IP:PORT was the key */
	RvSipTransportConnectionHandle  hConn;
					/* The connection used as a key to create the compartment, if applicable.  */
					/* Note: for outgoing initial requests, this handle will be NULL at        */
					/* that point, as the connection is initialized later, just before sending */
					/* the message.                                                            */
	RvSipMsgHandle                  hMsg;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT) 
	char                            urn[128];//save the urn to pass to App
	                /* The message triggered the compartment creation */
#endif
/*SPIRENT_END*/

} RvSipCompartmentCbInfo;

/*
 * RvSipCompartmentMgrHandle
 * ---------------------------------------------------------------------------
 * Declaration of handle to a  compartment Manager instance. The manager
 * object manages all the module compartments and its handle is needed in all
 * manager related API such as creation of new  compartments.
 */
RV_DECLARE_HANDLE(RvSipCompartmentMgrHandle);


/*
 * RvSipCompartmentHandle
 * ---------------------------------------------------------------------------
 * Declaration of a  Compartment handle.  Compartment is needed
 * in all  compartments API function and is used to reference the
 * correct compartment object.
 */
RV_DECLARE_HANDLE(RvSipCompartmentHandle);

/*
 * RvSipAppCompartmentHandle
 * ---------------------------------------------------------------------------
 * Declaration  of application handle to a  Compartment. This handle
 * is used by the application in order to associate sip stack
 * Compartment objects with application  Compartment object. The
 * application gives the application handle when a new  Compartment is
 * created.
 */

RV_DECLARE_HANDLE(RvSipAppCompartmentHandle);

/*-----------------------------------------------------------------------
       C O M P R T M E N T :  C A L L - B A C K  F U N C T I O N S
 ------------------------------------------------------------------------*/

/*@**************************************************************************
 * RvSipCompartmentCreatedEv
 * ------------------------------------------------------------------------
 * General:  Notifies the application that a new Compartment was created. The application can examine
 *           the parameters used to create this compartment (sigcomp-id, IP and Port). It can also
 *           change the default compression algorithm used. Once everything is checked, the application
 *           can approve or disapprove the compartment creation by setting the pbIsApproved parameter.
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:		hCompartment    - handle to the newly created compartment
 *              pCompInfo       - structure holding information regarding the compartment
 * InOut        strAlgoName     - the compression algorithm configured into the compartment
 * Output:      phAppl          - the application handle to be associated with the compartment
 *              pbIsApproved    - boolean indicating if the compartment creation is approved by
 *                                application.
 **************************************************************************@*/
typedef void
        (RVCALLCONV * RvSipCompartmentCreatedEv)(
                                   IN    RvSipCompartmentHandle          hCompartment,
								   IN    RvSipCompartmentCbInfo          *pCompInfo,
								   INOUT RvChar                          *strAlogName, 
								   OUT   RvSipAppCompartmentHandle       *phAppl,
								   OUT   RvBool                          *pbIsApproved);


/*@**************************************************************************
 * RvSipCompartmentEvHandlers
 * ----------------------------------------------------------------------------
 * A structure with function pointers to the callback of the module.
 * This structure is used to set the application callbacks in the
 * RvSipCompartmentMgrSetEvHandlers() function.
 *
 * pfnCompartmentCreatedEvHandler    - Notifies the application of a new compartment.
 *****************************************************************************@*/

typedef struct 
{
    RvSipCompartmentCreatedEv   pfnCompartmentCreatedEvHandler;
} RvSipCompartmentEvHandlers;

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef RV_SIP_COMPARTMENT_TYPES_H*/


