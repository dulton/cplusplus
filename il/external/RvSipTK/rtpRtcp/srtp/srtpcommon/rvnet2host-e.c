/* rvnet2host.c - converst network/host organized header bytes */

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

#include "rvtypes.h"
#include "rvnet2host.h"
#include "rvsocket.h"

#if (RV_SOCKET_TYPE == RV_SOCKET_BSD)

#if (RV_OS_TYPE == RV_OS_TYPE_OSE)
#include <inet.h>

#elif (RV_OS_TYPE != RV_OS_TYPE_WINCE) && (RV_OS_TYPE != RV_OS_TYPE_MOPI)
#include <netinet/in.h>
#endif

#elif (RV_SOCKET_TYPE == RV_SOCKET_SYMBIAN)
#include <sys/types.h>
#include <netinet/in.h>

#endif

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
RvUint64 RvConvertHostToNetwork64(RvUint64 host) 
{
	RvUint64 result;
	RvUint8 *resultp;
	RvUint8 *hostp;

	resultp = (RvUint8 *)&result;
	hostp = (RvUint8 *)&host;

	resultp[0] = hostp[7];
	resultp[1] = hostp[6];
	resultp[2] = hostp[5];
	resultp[3] = hostp[4];
	resultp[4] = hostp[3];
	resultp[5] = hostp[2];
	resultp[6] = hostp[1];
	resultp[7] = hostp[0];

	return result;
}
*/
/* SPIRENT_END */
#endif	/* (RV_ARCH_ENDIAN == RV_ARCH_LITTLE_ENDIAN) */
