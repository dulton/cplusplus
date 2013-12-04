/* rvnet2host.h - converst network/host organized header bytes */

/************************************************************************
        Copyright (c) 2005 RADVISION Inc. and RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Inc. and RADVISION Ltd.. No part of this document may be
reproduced in any form whatsoever without written prior approval by
RADVISION Inc. or RADVISION Ltd..

RADVISION Inc. and RADVISION Ltd. reserve the right to revise this
publication and make changes without obligation to notify any person of
such revisions or changes.
***********************************************************************/

/* NOTE: This module is an extention to rvnet2host and should be merged with it. */

#if !defined(RV_NET2HOST_E_H)
#define RV_NET2HOST_E_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rvtypes.h"


/* Macros for conversion of byte ordering */
#if (RV_ARCH_ENDIAN == RV_ARCH_LITTLE_ENDIAN)

/************************************************************************************************************************
 * RvConvertHostToNetwork64
 *
 * Converts a 64 bit integer in host order to a 64 bit integer in a network format.
 *
 * INPUT   :  host       : value to convert.
 * OUTPUT  :  None.
 * RETURN  :  The converted value.
 */
/* SPIRENT_BEGIN */
/* comment this version out and use the same on in sip/common/ccore
RvUint64 RvConvertHostToNetwork64(RvUint64 host);
*/
/* SPIRENT_END */
#define RvConvertNetworkToHost64(_network) RvConvertHostToNetwork64(_network)

#elif (RV_ARCH_ENDIAN == RV_ARCH_BIG_ENDIAN)

#define RvConvertHostToNetwork64(_host) (_host)
#define RvConvertNetworkToHost64(_network) (_network)

#endif

#ifdef __cplusplus
}
#endif

#endif
