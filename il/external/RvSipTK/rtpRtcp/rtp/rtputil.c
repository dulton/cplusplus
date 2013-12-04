/***********************************************************************
        Copyright (c) 2002 RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Ltd.. No part of this document may be reproduced in any
form whatsoever without written prior approval by RADVISION Ltd..

RADVISION Ltd. reserve the right to revise this publication and make
changes without obligation to notify any person of such revisions or
changes.
***********************************************************************/

#include "rvtypes.h"
#include "rvlock.h"
#include "rvmemory.h"
#include "rtputil.h"



RVAPI
void   RVCALLCONV ConvertToNetwork(void *data, RvInt32 pos, RvInt32 n)
{
    RvInt32 i;
    for (i = pos; i < pos + n; ++i)
      ((RvUint32*)data)[i] = RvConvertHostToNetwork32(((RvUint32*)data)[i]);
}

RVAPI
void   RVCALLCONV ConvertFromNetwork(void *data, RvInt32 pos, RvInt32 n)
{
    RvInt32 i;
    for (i = pos; i < pos + n; ++i)
      ((RvUint32*)data)[i] = RvConvertNetworkToHost32(((RvUint32*)data)[i]);
}

RVAPI
void   RVCALLCONV ConvertToNetwork2(void *data, RvInt32 pos, RvInt32 n)
{
    RvInt32 i;
    for (i = pos; i < pos + n; ++i)
        ((RvUint16*)data)[i] = RvConvertHostToNetwork16(((RvUint16*)data)[i]);
}

RVAPI
void   RVCALLCONV ConvertFromNetwork2(void *data, RvInt32 pos, RvInt32 n)
{
    RvInt32 i;
    for (i = pos; i < pos + n; ++i)
        ((RvUint16*)data)[i] = RvConvertNetworkToHost16(((RvUint16*)data)[i]);
}

