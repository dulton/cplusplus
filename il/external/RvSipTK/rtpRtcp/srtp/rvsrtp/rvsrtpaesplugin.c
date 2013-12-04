/************************************************************************
 File Name     : rvsrtpaesplugin.c
 Description   :
*************************************************************************
 Copyright (c)  2005 , RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVision Inc.
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVision Inc.

 RADVision Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
*************************************************************************/

#include "rvsrtpaesplugin.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************
 * RvSrtpAesPlugInConstruct
 * description:  This method constructs a RvRtpEncryptionPlugIn. All of
 *               the callbacks must be suppled for this plugin to work.
 * parameters:
 *    plugin - the RvRtpEncryptionPlugIn object.
 *   userData - the user data associated with the object.
 *    encrypt - the RvRtpEncryptionPlugInEncryptCB callback to use with the object.
 *    decrypt - the RvRtpEncryptionPlugInGetBlockSizeCB callback to use with the object.
 * getBlockSize - the RvRtpEncryptionPlugInGetBlockSizeCB callback to use with the object.
 * getHeaderSize - the RvRtpEncryptionPlugInGetHeaderSizeCB callback to use with the object.
 * returns: A pointer to the object, if successful. NULL, otherwise.
 ***************************************************************************************/
RVAPI
RvSrtpAesPlugIn* RVCALLCONV RvSrtpAesPlugInConstruct(
     IN RvSrtpAesPlugIn*                         plugin,
     IN void*								     userData,
     IN RvSrtpAesPlugInEncryptDataConstructCB    construct,
     IN RvSrtpAesPlugInEncryptDataDestructCB     destruct,
     IN RvSrtpAesPlugInInitializeECBModeCB       initECBMode,
     IN RvSrtpAesPlugInInitializeCBCModeCB       initCBCMode,
     IN RvSrtpAesPlugInEncryptCB		         encrypt,
     IN RvSrtpAesPlugInDecryptCB		         decrypt)
{
    plugin->construct    = construct;
    plugin->destruct     = destruct;
    plugin->initECBMode  = initECBMode;
    plugin->initCBCMode  = initCBCMode;
    plugin->encrypt      = encrypt;
    plugin->decrypt      = decrypt;
    plugin->userData     = userData;
    return plugin;
}


/***************************************************************************************
 * RvSrtpAesPlugInDestruct
 * description:  This method destructs a RvSrtpAesPlugIn.
 * parameters:
 *    plugin - the RvSrtpAesPlugIn object.
 * returns: A pointer to the object, if successful. NULL, otherwise.
 ***************************************************************************************/
RVAPI
RvSrtpAesPlugIn* RVCALLCONV RvSrtpAesPlugInDestruct(
    RvSrtpAesPlugIn*                         plugin)
{
    plugin->construct    = NULL;
    plugin->destruct     = NULL;
    plugin->initECBMode  = NULL;
    plugin->initCBCMode  = NULL;
    plugin->encrypt      = NULL;
    plugin->decrypt      = NULL;
    plugin->userData     = NULL;
    return plugin;
}

#ifdef __cplusplus
}
#endif

