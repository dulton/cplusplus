/************************************************************************
 File Name	   : RtpProfileRfc3711.h
 Description   : scope: Private
     declaration of inplementation of RtpProfilePlugin for SRTP (RFC 3711)
*************************************************************************
************************************************************************
        Copyright (c) 2005 RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Ltd.. No part of this document may be reproduced in any
form whatsoever without written prior approval by RADVISION Ltd..

RADVISION Ltd. reserve the right to revise this publication and make
changes without obligation to notify any person of such revisions or
changes.
***********************************************************************/

#ifndef __RTP_PROFILE_RFC_3711_H__
#define __RTP_PROFILE_RFC_3711_H__

#include "rvsrtp.h"
#include "rtpProfilePlugin.h"

#ifdef __cplusplus
extern "C" {
#endif

struct __RtpProfileRfc3711;

/***************************************************************************************
 * RtpProfileRfc3711 :
 * description:
 *   implementation of RtpProfilePlugin for SRTP (RFC 3711)
 ****************************************************************************************/
typedef struct __RtpProfileRfc3711
{
    RvSrtp             srtp;                          /* SRTP related parameters */  
    RtpProfilePlugin   plugin;                        /* pointer to the basic plugin class */

} RtpProfileRfc3711;

/***************************************************************************************
 * RtpProfileRfc3711Construct
 * description:  This method constructs a RtpProfilePlugin. All of
 *               the callbacks must be suppled for this plugin to work.
 * parameters:
 *    plugin - the RtpProfileRfc3711 object.
 *   userData - the user data associated with the object.
 * returns: A pointer to the object, if successful. NULL, otherwise.
 ***************************************************************************************/
RtpProfileRfc3711* RtpProfileRfc3711Construct(
     RtpProfileRfc3711*                          plugin
#ifdef UPDATED_BY_SPIRENT
    ,RvSrtpMasterKeyEventCB                      masterKeyEventCB
#endif // UPDATED_BY_SPIRENT
);

/***************************************************************************************
 * RtpProfilePluginDestruct
 * description:  This method destructs a RtpProfilePlugin. 
 * parameters:
 *    plugin - the RtpProfileRfc3711 object.
 * returns: none
 ***************************************************************************************/
void RtpProfileRfc3711Destruct(
     IN RtpProfileRfc3711*  plugin);

/* Accessors */
#define RtpProfileRfc3711GetPlugIn(t) &(t)->plugin

#ifdef __cplusplus
}
#endif
    
#endif /* __RTP_PROFILE_RFC_3711_H__ */
