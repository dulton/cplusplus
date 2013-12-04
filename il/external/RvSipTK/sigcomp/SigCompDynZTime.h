#ifndef RV_SIGCOMP_DYN_Z_TIME_H
#define RV_SIGCOMP_DYN_Z_TIME_H
/*
*********************************************************************************
*                                                                               *
* NOTICE:                                                                       *
* This document contains information that is confidential and proprietary to    *
* RADVision LTD.. No part of this publication may be reproduced in any form     *
* whatsoever without written prior approval by RADVision LTD..                  *
*                                                                               *
* RADVision LTD. reserves the right to revise this publication and make changes *
* without obligation to notify any person of such revisions or changes.         *
*********************************************************************************
*/

/*********************************************************************************
 *                              <SigCompDynZTime.h>
 *
 * This file defines dynamic compression utility functions
 *
 *    Author                         Date
 *    ------                        ------
 *    Sasha G                      July 2006
 *********************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
	
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RvSigCompDyn.h"

void SigCompDynZTimeSecSet(RvSigCompZTimeSec time); 

RvSigCompZTimeSec SigCompDynZTimeSecGet(); 

#ifdef __cplusplus
}
#endif

#endif /* RV_SIGCOMP_DYN_Z_TIME_H */ 
