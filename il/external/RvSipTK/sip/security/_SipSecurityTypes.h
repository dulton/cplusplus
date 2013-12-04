/*
*******************************************************************************
*                                                                             *
* NOTICE:                                                                     *
* This document contains information that is confidential and proprietary to  *
* RADVision LTD.. No part of this publication may be reproduced in any form   *
* whatsoever without written prior approval by RADVision LTD..                *
*                                                                             *
* RADVision LTD. reserves the right to revise this publication and make       *
* changes without obligation to notify any person of such revisions or changes*
*******************************************************************************
*/

/******************************************************************************
 *                              <_SipSecurityTypes.h>
 *
 * The _SipSecurityTypes.h file contains all type and callback function
 * definitions used by Security module that are available for internal Toolkit
 * modules.
 *
 *    Author                        Date
 *    -----------                   ------------
 *    Igor                          Februar 2006
 *****************************************************************************/


#ifndef SIP_SECURITY_TYPES_H
#define SIP_SECURITY_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                          INCLUDE HEADER FILES                             */
/*---------------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#ifdef RV_SIGCOMP_ON
#include "RvSipTransmitterTypes.h"
#endif /* #ifdef RV_SIGCOMP_ON */

#ifdef RV_SIP_IMS_ON

/*---------------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                               */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*                           HANDLE TYPE DEFINITIONS                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                     SECURITY MANAGER TYPE DEFINITIONS                     */
/*---------------------------------------------------------------------------*/

/* SipSecTlsParams
 * ----------------------------------------------------------------------------
 * SipSecTlsParams struct represents collection of parameters,
 * required for security by TLS.
 *
 * hLocalAddr   - local address to be used for client connection 
 * ccRemoteAddr - Common Core address structure, containing IP and port
 *                of the remote side, that should b eused for client connection
 * eMsgCompType - Compression type of the TLS connection 
 */
typedef struct
{
    RvSipTransportLocalAddrHandle hLocalAddr;
    RvAddress*                    pRemoteAddr;
#ifdef RV_SIGCOMP_ON
	RvSipTransmitterMsgCompType   eMsgCompType;
	RPOOL_Ptr					  strSigcompId;
#endif /* RV_SIGCOMP_ON */
} SipSecTlsParams;

/*---------------------------------------------------------------------------*/
/*                      SECURITY OBJECT TYPE DEFINITIONS                     */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*               SECURITY MANAGER CALLBACK FUNCTION DEFINITIONS              */
/*---------------------------------------------------------------------------*/

#endif /* END OF: #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_SECURITY_TYPES_H */

