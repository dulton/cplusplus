/************************************************************************
 File Name	   : rvrtpaddresslist.h
 Description   :
*************************************************************************
 Copyright (c)	2001 , RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVision Inc. 
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVision Inc. 
 
 RADVision Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
*************************************************************************/
#if !defined(RVRTPADDRESSLIST_H)
#define RVRTPADDRESSLIST_H

#include "rvtypes.h"
#include "rvaddress.h"
#include "rvobjlist.h"
#include "rverror.h"

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************
*	RvRtpAddressList private module
******************************************************************/

typedef RvObjList RvRtpAddressList;

RvRtpAddressList *RvRtpAddressListConstruct(RvRtpAddressList *thisPtr);
void			  RvRtpAddressListDestruct(RvRtpAddressList *thisPtr);
RvBool	 RvRtpAddressListAddAddress(RvRtpAddressList *thisPtr, RvAddress *address);
RvBool	 RvRtpAddressListRemoveAddress(RvRtpAddressList *thisPtr, RvAddress *address);
RvBool	 RvRtpAddressListRemoveAllAddresses(RvRtpAddressList *thisPtr);

RVAPI
RvAddress*   RVCALLCONV  RvRtpAddressListGetNext(RvRtpAddressList *thisPtr, RvAddress *curitem);

#if defined(RV_RTP_DEST_POOL_OBJ)
RvStatus RvRtpDestinationAddressPoolConstruct(void);
RvStatus RvRtpDestinationAddressPoolDestruct(void);
#endif

#ifdef __cplusplus
}
#endif

#endif


