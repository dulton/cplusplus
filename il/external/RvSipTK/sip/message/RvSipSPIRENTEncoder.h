#ifndef SPIRENTSIPENCODER_H
#define SPIRENTSIPENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipMsgTypes.h"
#include "rpool_API.h"

#if defined(UPDATED_BY_SPIRENT_ABACUS)

void SPIRENT_SIPConfigurable_Init ( void );
RVAPI RV_Status RVCALLCONV SPIRENT_RvSipMsgEncode(IN  RvSipMsgHandle hSipMsg,
                                          IN  HRPOOL         hPool,
										            OUT HPAGE*         hPage,
                                          OUT RV_UINT32*     length);
#endif

#ifdef __cplusplus
}
#endif

#endif
