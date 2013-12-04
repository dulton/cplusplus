/************************************************************************
 File Name	   : rvrtpdesencryption.h
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
*************************************************************************
 $Revision: #1 $
 $Date: 2011/08/05 $
 $Author: songkamongkol $
************************************************************************/

#if !defined(RVRTPDESENCRYPTION_H)
#define RVRTPDESENCRYPTION_H

#include "rvtypes.h"
#include "rtptDesEncryption.h"
#include "rvrtpencryptionplugin.h"
#include "rvmutex.h"

#ifdef __cplusplus
extern "C" {
#endif


/*$
{type:
	{name: RvRtpDesEncryption}
	{superpackage: RvRtp}
	{include: rvrtpdesencryption.h}
	{description:	
		{p: This class is an implementation of the RvRtpEncryptionPlugIn
			for the DES algorithm. It supports one key per session, and  
			runs CBC mode.}
	}
	{attributes scope="private":
		{attribute: {t: RvDesEncryption } {n: encryptor } 
					{d: The encryptor used by the object.}}
		{attribute: {t: RvDesEncryption } {n: decryptor } 
					{d: The decryptor used by the object.}}
		{attribute: {t: RvRtpEncryptionPlugIn } {n: plugIn } 
					{d: The plug in interface used by the object.}}
	}
	{examples:
		{example:
			{description:
				{p: This is an example of how to construct, setup, and register
					the DES encryptor with a session.}
			}
			{code: 
RvRtpDesEncryption encryptor;
RvUint8 key[7] = \{1,2,3,4,5,6,7\};

rvRtpDesEncryptionConstruct(&encryptor);
rvRtpDesEncryptionSetKey(&encryptor,key);
rvRtpSessionRegisterEncryptionPlugIn(&session,rvRtpDesEncryptionGetPlugIn(&encryptor));
			}
		}
	}
	{see_also:
		{n:RvRtpEncryptionPlugIn}
	}
}
$*/
typedef struct
{
	RvDesEncryption 	  encryptor;
	RvDesEncryption 	  decryptor;
	RvRtpEncryptionPlugIn plugIn;

} RvRtpDesEncryption;

/* Construct/Destruct Methods */
RVAPI
RvRtpDesEncryption* RVCALLCONV rvRtpDesEncryptionConstruct(RvRtpDesEncryption *thisPtr);

RVAPI
void RVCALLCONV	rvRtpDesEncryptionDestruct(RvRtpDesEncryption *thisPtr);

void rvRtpDesEncryptionSetKey(RvRtpDesEncryption *thisPtr, const RvKey* keyPtr);

/* Accessors */
#define rvRtpDesEncryptionGetPlugIn(t) &(t)->plugIn


#ifdef __cplusplus
}
#endif

#endif	/* Include guard */

