/************************************************************************
 File Name	   : rvsrtpkey.h
 Description   :
*************************************************************************
 Copyright (c)	2000-2005, RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVision Inc. 
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVision Inc. 
 
 RADVision Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
*************************************************************************/
#if !defined(RV_SRTP_KEY_H)
#define RV_SRTP_KEY_H

#include "rvtypes.h"
#include "rvobjpool.h"
#include "rvobjhash.h"
#include "rvobjlist.h"
#include "rvmemory.h"

#if defined(__cplusplus)
extern "C" {
#endif 

/*$
{type scope="private":
	{name: RvSrtpKey}
	{superpackage: RvSrtpEncryptionPlugin}
	{include: rvsrtpkey.h}
	{description:
		{p: This type holds a master key and any information related to it.}
	}
}
$*/
typedef struct {
	RvObjHashElement hashElem;   /* master key MKI lookup */
	RvObjPoolElement poolElem;   /* master key allocation pool */
	RvObjList contextList;       /* list of context entries using this key */
	RvBool    maxLimit;          /* RV_TRUE if a stream has hit the packet count max, stopping all streams using this key */
	RvUint8  *masterKey;         /* Master Salt */
	RvUint8  *masterSalt;        /* Master Key */
	RvUint8  *mki;               /* MKI */
#ifdef UPDATED_BY_SPIRENT
	RvUint8   direction;         /* RV_SRTP_KEY_LOCAL or RV_SRTP_KEY_REMOTE should be used (defined in the rv_srtp.h) */
	RvBool    use_lifetime;      /* RV_TRUE when life time is used*/
	RvUint64  rtp_lifetime;      /* key's lifetime for RTP in packets*/
    RvUint64  rtcp_lifetime;     /* key's lifetime for RTCP in packets*/
	RvUint64  threshold;         /* threshold for re-invite signal in packets */
#endif // UPDATED_BY_SPIRENT
	RvSize_t  keyDerivationRate; /* As per RFC, 0 = do not change session keys automatically */
	RvSize_t  keyBufSize;        /* Number of bytes required to store all the keys */
    RvUint8   keys[1];           /* variable length structure for storing all keys */
} RvSrtpKey;

/*$
{type scope="private":
	{name: RvSrtpKeyPool}
	{superpackage: RvSrtpEncryptionPlugin}
	{include: rvsrtpkey.h}
	{description:
		{p: This is a pool of key objects. Since key objects are
			variable size, the actual object size will depend on the
			sizes of the keys.}
	}
}
$*/
typedef struct {
	RvObjPool pool;
	RvMemory *region;
	RvSize_t masterKeySize;
	RvSize_t masterSaltSize;
	RvSize_t mkiSize;
} RvSrtpKeyPool;

/* Use only for stand-alone key objects (not pooled) */
RvSrtpKey *rvSrtpKeyConstruct(RvSrtpKey *thisPtr, RvSize_t masterKeySize, RvSize_t masterSaltSize, RvSize_t mkiSize);
void rvSrtpKeyDestruct(RvSrtpKey *thisPtr);
RvSize_t rvSrtpKeyCalcSize(RvSize_t masterKeySize, RvSize_t masterSaltSize, RvSize_t mkiSize);

/* Standard key functions */
RvSize_t rvSrtpKeyGetSize(RvSrtpKey *thisPtr);
#define rvSrtpKeyGetHashElem(thisPtr)             (&((thisPtr)->hashElem))
#define rvSrtpKeyGetMaxLimit(thisPtr)             ((thisPtr)->maxLimit)
#define rvSrtpKeySetMaxLimit(thisPtr, v)          ((thisPtr)->maxLimit = (v))
#define rvSrtpKeyGetMasterKey(thisPtr)            ((thisPtr)->masterKey)
#define rvSrtpKeyGetMasterSalt(thisPtr)           ((thisPtr)->masterSalt)
#define rvSrtpKeyGetMki(thisPtr)                  ((thisPtr)->mki)
#define rvSrtpKeyGetKeyDerivationRate(thisPtr)    ((thisPtr)->keyDerivationRate)
#define rvSrtpKeySetKeyDerivationRate(thisPtr, r) ((thisPtr)->keyDerivationRate = (r))
#define rvSrtpKeyGetContextList(thisPtr)          (&((thisPtr)->contextList))
#define rvSrtpKeyGetContextListSize(thisPtr)      (RvObjListSize(&((thisPtr)->contextList)))

/* Pool functions */
RvSrtpKeyPool *rvSrtpKeyPoolConstruct(RvSrtpKeyPool *pool, RvSize_t masterKeySize, RvSize_t masterSaltSize, RvSize_t mkiSize, RvMemory *region, RvSize_t pageitems, RvSize_t pagesize, RvInt pooltype, RvSize_t maxitems, RvSize_t minitems, RvSize_t freelevel);
RvBool rvSrtpKeyPoolDestruct(RvSrtpKeyPool *pool);
RvSrtpKey *rvSrtpKeyPoolNew(RvSrtpKeyPool *pool);
void rvSrtpKeyPoolRelease(RvSrtpKeyPool *pool, RvSrtpKey *key);

#if defined(RV_TEST_CODE)
#include "rvrtpstatus.h"
RvRtpStatus RvSrtpKeyTest();
#endif /* RV_TEST_CODE */

/* Function Docs */
/*$
{function scope="private":
    {name:    rvSrtpKeyConstruct}
    {class:   RvSrtpKey}
    {include: rvsrtpkey.h}
    {description:
        {p: This function constructs a master key object with the given
			key sizes. It assumes there is enough memory at the end of the
			object to store all of the keys.}
    }
    {proto: RvSrtpkey *rvSrtpKeyConstruct(RvSrtpKey *thisPtr, RvSize_t masterKeySize, RvSize_t masterSaltSize, RvSize_t mkiSize, RvSize_t keyDerivationRate);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpKey object to construct.}}
        {param: {n:masterKeySize} {d:The size of the master key.}}
        {param: {n:masterSaltSize} {d:The size of the master salt.}}
        {param: {n:mkiSize} {d:The size of the MKI.}}
    }
    {returns: The master key constructed or NULL if there was an error. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpKeyDestruct}
    {class:   RvSrtpKey}
    {include: rvsrtpkey.h}
    {description:
        {p: This function destructs a master key object.}
    }
    {proto: void rvSrtpKeyDestruct(RvSrtpKey *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpKey object to destruct.}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpKeyCalcSize}
    {class:   RvSrtpKey}
    {include: rvsrtpkey.h}
    {description:
        {p: This function calculates the memory required to construct a
			key object with the key sizes specified.}
    }
    {proto: RvSize_t rvSrtpKeyCalcSize(RvSize_t masterKeySize, RvSize_t masterSaltSize, RvSize_t mkiSize);}
    {params:
        {param: {n:masterKeySize} {d:The size of the master key.}}
        {param: {n:masterSaltSize} {d:The size of the master salt.}}
        {param: {n:mkiSize} {d:The size of the MKI.}}
    }
    {returns: The memory size required for the object (in bytes).}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpKeyGetSize}
    {class:   RvSrtpKey}
    {include: rvsrtpkey.h}
    {description:
        {p: This function gets the size of the object.}
    }
    {proto: RvSize_t rvSrtpKeyGetSize(RvSrtpKey *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpKey object.}}
    }
    {returns: The size of the object (in bytes).}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpKeyGetHashElem}
    {class:   RvSrtpKey}
    {include: rvsrtpkey.h}
    {description:
        {p: This function gets the hash element pointer of the object.}
    }
    {proto: RvObjHashElement *rvSrtpKeyGetHashElem(RvSrtpKey *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpKey object.}}
    }
    {returns: The hash element pointer.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpKeyGetMaxLimit}
    {class:   RvSrtpKey}
    {include: rvsrtpkey.h}
    {description:
        {p: This function gets the max limit flag of the object. If any
			stream has encoded or decoded the maximum number of packets
			using this key then this will be set to RV_TRUE.}
    }
    {proto: RvBool rvSrtpKeyGetMaxLimit(RvSrtpKey *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpKey object.}}
    }
    {returns: RV_TRUE if no more packets may be encrypted or decrypted with this key, RV_FALSE otherwise.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpKeySetMaxLimit}
    {class:   RvSrtpKey}
    {include: rvsrtpkey.h}
    {description:
        {p: This function sets max limit flag of the object. If any
			stream has encoded or decoded the maximum number of packets
			using this key then this will be set to RV_TRUE.}
    }
    {proto: void rvSrtpKeySetMaxLimit(RvSrtpKey *thisPtr, RvBool maxLimit);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpKey object.}}
        {param: {n:thisPtr} {d:RV_TRUE for max limit reached, RV_FALSE for key still usable.}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpKeyGetMasterKey}
    {class:   RvSrtpKey}
    {include: rvsrtpkey.h}
    {description:
        {p: This function gets the master key.}
    }
    {proto: RvUint8 *rvSrtpKeyGetMasterKey(RvSrtpKey *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpKey object.}}
    }
    {returns: The master key.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpKeyGetMasterSalt}
    {class:   RvSrtpKey}
    {include: rvsrtpkey.h}
    {description:
        {p: This function gets the master salt.}
    }
    {proto: RvUint8 *rvSrtpKeyGetMasterSalt(RvSrtpKey *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpKey object.}}
    }
    {returns: The master salt.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpKeyGetMki}
    {class:   RvSrtpKey}
    {include: rvsrtpkey.h}
    {description:
        {p: This function gets the MKI.}
    }
    {proto: RvUint8 *rvSrtpKeyGetMki(RvSrtpKey *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpKey object.}}
    }
    {returns: The MKI.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpKeyGetKeyDerivationRate}
    {class:   RvSrtpKey}
    {include: rvsrtpkey.h}
    {description:
        {p: This function gets the key derivation rate of the master
			key. A value of 0 indicates that session keys should only
			be derived once.}
    }
    {proto: RvSize_t rvSrtpKeyGetKeyDerivationRate(RvSrtpKey *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpKey object.}}
    }
    {returns: The key derivation rate.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpKeySetKeyDerivationRate}
    {class:   RvSrtpKey}
    {include: rvsrtpkey.h}
    {description:
        {p: This function sets the key derivation rate of the master
			key. This is how often the session keys will be regenerated
			from the master key (based on index % rate). A value of 0
			indicates that session keys should only be derived once.}
    }
    {proto: void rvSrtpKeySetKeyDerivationRate(RvSrtpKey *thisPtr, RvSize_t keyDerivationRate);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpKey object.}}
        {param: {n:keyDerivationRate} {d:The key derivation rate for this key (0 = none).}}
    }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpKeyGetContextList}
    {class:   RvSrtpKey}
    {include: rvsrtpkey.h}
    {description:
        {p: This function gets the list of contexts derived from this master key object.}
    }
    {proto: RvObjList *rvSrtpKeyGetContextList(RvSrtpKey *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpKey object.}}
    }
    {returns: The context list.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpKeyGetContextListSize}
    {class:   RvSrtpKey}
    {include: rvsrtpkey.h}
    {description:
        {p: This function gets the number of current contexts that are derived from this master key object.}
    }
    {proto: RvSize_t rvSrtpKeyGetContextListSize(RvSrtpKey *thisPtr);}
    {params:
        {param: {n:thisPtr} {d:The RvSrtpKey object.}}
    }
    {returns: The context list size.}
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpKeyPoolConstruct}
    {class:   RvSrtpKeyPool}
    {include: rvsrtpkey.h}
    {description:
        {p: This function constructs a pool of master key objects with the given
			key sizes and pool parameters.}
		{p: Refer to the RvObjPool documentation for more details on
			the pool parameters.}
    }
    {proto: RvSrtpKeyPool *rvSrtpKeyPoolConstruct(RvSrtpKeyPool *pool, RvSize_t masterKeySize, RvSize_t masterSaltSize, RvSize_t mkiSize, RvSize_t pageitems, RvSize_t pagesize, RvInt pooltype, RvSize_t maxitems, RvSize_t minitems, RvSize_t freelevel);}
    {params:
        {param: {n:pool} {d:The RvSrtpKeyPool object to construct.}}
        {param: {n:masterKeySize} {d:The size of the master key.}}
        {param: {n:masterSaltSize} {d:The size of the master salt.}}
        {param: {n:mkiSize} {d:The size of the MKI.}}
        {param: {n:region} {d:The memory region to allocate the pool from (NULL = default region).}}
        {param: {n:pageitems} {d:Number of items per page (0 = calculate from pagesize).}}
        {param: {n:pagesize} {d:Size of each page (0 = calculate from pageitems).}}
        {param: {n:pooltype} {d:The type of pool (FIXED, EXPANDING, DYNAMIC).}}
        {param: {n:maxitems} {d: Number of items never to exceed this value (0 = no max).}}
        {param: {n:minitems} {d:Number of items never to go below this value.}}
        {param: {n:freelevel} {d:Minimum number of items free per 100 (0 to 100). Used for DYNAMIC pools only.}}
    }
    {returns: The master key pool constructed or NULL if there was an error. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpKeyPoolDestruct}
    {class:   RvSrtpKeyPool}
    {include: rvsrtpkey.h}
    {description:
        {p: This function destructs a pool of master key objects. All
			keys must have been released back to the pool in order for
			the destuct to be successful.}
    }
    {proto: RvBool rvSrtpKeyPoolDestruct(RvSrtpKeyPool *pool);}
    {params:
        {param: {n:pool} {d:The RvSrtpKeyPool object to destruct.}}
    }
    {returns: RV_TRUE if the pool was destructed, otherwise RV_FALSE. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpKeyPoolNew}
    {class:   RvSrtpKeyPool}
    {include: rvsrtpkey.h}
    {description:
        {p: This function allocates and constructs a master key object from
			the pool.}
    }
    {proto: RvSrtpKey *rvSrtpKeyPoolNew(RvSrtpKeyPool *pool);}
    {params:
        {param: {n:pool} {d:The RvSrtpKeyPool object to allocate from.}}
    }
    {returns: The constructed master key object. }
}
$*/
/*$
{function scope="private":
    {name:    rvSrtpKeyPoolRelease}
    {class:   RvSrtpKeyPool}
    {include: rvsrtpkey.h}
    {description:
        {p: This function destructs and releases a master key object to
			the pool.}
    }
    {proto: void rvSrtpKeyPoolRelease(RvSrtpKeyPool *pool, RvSrtpKey *key);}
    {params:
        {param: {n:pool} {d:The RvSrtpKeyPool object to allocate from.}}
        {param: {n:key} {d:The RvSrtpKey object to release.}}
    }
}
$*/

#if defined(__cplusplus)
}
#endif

#endif
