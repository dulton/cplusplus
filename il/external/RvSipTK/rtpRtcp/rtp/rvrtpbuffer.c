/*

NOTICE:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

#include "rvstdio.h"
#include "rvrtpbuffer.h"

#ifdef __cplusplus
extern "C" {
#endif


RvBool buffAddToBuffer(RvRtpBuffer* to, RvRtpBuffer* from, RvUint32 offset)
{
    if (from->length + offset <= to->length)
    {
        memcpy((RvUint8*)to->buffer + offset, from->buffer, from->length);

        return RV_TRUE;
    }

    return RV_FALSE;
}

RvBool buffValid(RvRtpBuffer* buff, RvUint32 size)
{
    return (size <= buff->length  &&  buff->buffer);
}

RvRtpBuffer buffCreate(void* data,RvInt32 size)
{
    RvRtpBuffer buff;

    buff.buffer = (unsigned char*)data;
    buff.length = size;
    return buff;
}


#ifdef __cplusplus
}
#endif



