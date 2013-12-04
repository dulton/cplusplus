/************************************************************************
 File Name     : rvencryption.h
 Description   :
*************************************************************************
 Copyright (c)  2002 , RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVISION Inc. 
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVISION Inc. 
 
 RADVISION Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
************************************************************************/

#if !defined(RVENCRYPTION_H)
#define RVENCRYPTION_H

#include "rvtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Encryption direction */
typedef enum RvEncryptionDirection_
{
    RV_ENCRYPTIONDIRECTION_ENCRYPT, 
    RV_ENCRYPTIONDIRECTION_DECRYPT
} RvEncryptionDirection; 

/* Encryption mode */
typedef enum RvEncryptionMode_
{
    RV_ENCRYPTIONMODE_ECB, 
    RV_ENCRYPTIONMODE_CBC
} RvEncryptionMode; 

#ifdef __cplusplus
}
#endif

#endif  /* Include guard */

