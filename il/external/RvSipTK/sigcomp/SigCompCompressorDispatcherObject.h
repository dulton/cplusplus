/***********************************************************************
Filename   : SigCompCompressorDispatcherObject.h
Description: SigComp main internal interface header file
************************************************************************
      Copyright (c) 2001,2002 RADVISION Inc. and RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Inc. and RADVISION Ltd.. No part of this document may be
reproduced in any form whatsoever without written prior approval by
RADVISION Inc. or RADVISION Ltd..

RADVISION Inc. and RADVISION Ltd. reserve the right to revise this
publication and make changes without obligation to notify any person of
such revisions or changes.
***********************************************************************/


#ifndef SIGCOMP_COMPRESSOR_DISPATCHER_OBJECT_H
#define SIGCOMP_COMPRESSOR_DISPATCHER_OBJECT_H

#if defined(__cplusplus)
extern "C" {
#endif 

/*-----------------------------------------------------------------------*/
/*                   Additional header-files                             */
/*-----------------------------------------------------------------------*/
#include "RvSigCompTypes.h"
#include "SigCompStateHandlerObject.h"
#include "SigCompCompartmentHandlerObject.h"

    
/*-----------------------------------------------------------------------*/
/*                   Definitions & Constants                             */
/*-----------------------------------------------------------------------*/
typedef struct {
    struct _SigCompMgr *pSCMgr;
} SigCompCompressionDispatcherMgr;
    
/*-----------------------------------------------------------------------*/
/*                    Whole Module Functions                             */
/*-----------------------------------------------------------------------*/

/*************************************************************************
* SigCompCompressorDispatcherConstruct
* ------------------------------------------------------------------------
* General: constructs and initializes the compressor dispatcher, there 
*          is one compressor dispatcher per entity and therefore it is 
*          called upon initialization of the SigComp module
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:     hSigCompMgr - A handle to the SigComp manager
*************************************************************************/
RvStatus SigCompCompressorDispatcherConstruct(IN RvSigCompMgrHandle hSigCompMgr);

/*************************************************************************
* SigCompCompressorDispatcherDestruct
* ------------------------------------------------------------------------
* General: destructor of the compressor dispatcher, will be called 
*          when destructing the SigComp module
*
* Return Value: void
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pCDMgr - a handle to the compressor dispatcher Mgr struct
*************************************************************************/
void SigCompCompressorDispatcherDestruct(IN SigCompCompressionDispatcherMgr *pCDMgr);

/*************************************************************************
* SigCompCompressorDispatcherCompress
* ------------------------------------------------------------------------
* General: this function constructs the SigComp message, 
*          SigComp message should constructed from a header, a compressed 
*          message and some optional additional information
*
* Return Value: RvStatus
* ------------------------------------------------------------------------
* Arguments: 
* Input:    *pCDMgr - a handle to the compressor dispatcher Mgr
*           *pCompartment - A pointer to the compartment
*           *pMsgInfo - a pointer to 'SipSigCompMessageInfo' structure, 
*                       holds the plain message + its size
*
* Output:   *pMsgInfo - a pointer to 'RvSigCompMessageInfo' structure, 
*                       holds the compressed message + its size
*************************************************************************/
RvStatus SigCompCompressorDispatcherCompress(IN    SigCompCompressionDispatcherMgr *pCDMgr,
                                             IN    SigCompCompartment              *pCompartment,
                                             INOUT RvSigCompMessageInfo            *pMsgInfo);


#if defined(__cplusplus)
}
#endif

#endif /* SIGCOMP_COMPRESSOR_DISPATCHER_OBJECT_H */























