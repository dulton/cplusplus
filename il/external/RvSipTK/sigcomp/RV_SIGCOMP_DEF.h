#ifndef _RV_SIGCOMP_DEF
#define _RV_SIGCOMP_DEF

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */


#include "rvtypes.h"
#include "rverror.h"



/*****************************************************************************
 *  RvStatus -- status codes returned from API functions.
 *  Note: only general status codes are defined here,
 *****************************************************************************/
#define RV_ERROR_COMPRESSION_FAILED         -4000  /* Compression process failure    */
#define RV_ERROR_DECOMPRESSION_FAILED       -4001  /* Decompression process failure  */
#define RV_ERROR_DUPLICATESTATE             -4002  /* Used within the SIgComp module */



#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _RV_SIGCOMP_DEF */


