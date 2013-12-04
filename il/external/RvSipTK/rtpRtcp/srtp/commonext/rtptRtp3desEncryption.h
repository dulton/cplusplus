/************************************************************************
 File Name	   : rvrtp3desencryption.h
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

#if !defined(RVRTP3DESENCRYPTION_H)
#define RVRTP3DESENCRYPTION_H

#include "rvtypes.h"
#include "rtpt3desEncryption.h"
#include "rvrtpencryptionplugin.h"

#ifdef __cplusplus
extern "C" {
#endif


/*$
{type:
	{name: RvRtp3DesEncryption}
	{superpackage: RvRtp}
	{include: rvrtpdesencryption.h}
	{description:	
		{p: This class is an implementation of the RvRtpEncryptionPlugIn
			for the Triple DES algorithm. It supports one key per session.}
	}
	{attributes scope="private":
		{attribute: {t: Rv3DesEncryption } {n: encryptor } 
					{d: The encryptor used by the object.}}
		{attribute: {t: Rv3DesEncryption } {n: decryptor } 
					{d: The decryptor used by the object.}}
		{attribute: {t: RvRtpEncryptionPlugIn } {n: plugIn } 
					{d: The plug in interface used by the object.}}
	}
	{examples:
		{example:
			{description:
				{p: This is an example of how to construct, setup, and register
					the triple DES encryptor with a session.}
			}
			{code: 
RvRtp3DesEncryption encryptor;
RvUint8 key[21] = \{1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1\};

rvRtp3DesEncryptionConstruct(&encryptor);
rvRtp3DesEncryptionSetKey(&encryptor,key);
rvRtpSessionRegisterEncryptionPlugIn(&session,rvRtp3DesEncryptionGetPlugIn(&encryptor));
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
	Rv3DesEncryption	   encryptor;
	Rv3DesEncryption	   decryptor;
	RvRtpEncryptionPlugIn  plugIn;
} RvRtp3DesEncryption;

/* Construct/Destruct Methods */
RVAPI
RvRtp3DesEncryption* RVCALLCONV rvRtp3DesEncryptionConstruct(RvRtp3DesEncryption *thisPtr);
RVAPI
void	RVCALLCONV rvRtp3DesEncryptionDestruct(RvRtp3DesEncryption *thisPtr);

/* Accessors */
#define rvRtp3DesEncryptionGetPlugIn(t) &(t)->plugIn

#ifdef __cplusplus
}
#endif

#endif	/* Include guard */

