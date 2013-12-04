/************************************************************************
 File Name     : g711.h
 Description   :
*************************************************************************
 Copyright (c)  2001 , RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVision Inc. 
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVision Inc. 
 
 RADVision Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
*************************************************************************
 $Revision: #1 $
 $Date: 2011/08/05 $
 $Author: songkamongkol $
************************************************************************/

#if !defined(RVG711_H)
#define RVG711_H

#include "rvtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void rvG711PCMToULaw(const void *src, void *dst, RvInt length);
void rvG711ULawToPCM(const void *src, void *dst, RvInt byteLength);

#ifdef __cplusplus
}
#endif

#endif  /* Include guard */

