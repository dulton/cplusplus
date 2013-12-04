/*

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*/

/******************************************************************************
 *                        _SipHeader.h                                        *
 *                                                                            *
 * This file defines the 'internal API' of headers objects.                   *
 *                                                                            *
 *      Author           Date                                                 *
 *     ------           ------------                                          *
 *      Ofra             Nov.2000                                             *
 ******************************************************************************/



#ifndef SIP_HEADER_H
#define SIP_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RvSipMsgTypes.h"


/***************************************************************************
 * SipHeaderSetBadSyntaxStr
 * ------------------------------------------------------------------------
 * General: The function sets bad-syntax string in a header,
 *          according to it's type.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   eType -   Type of the header to fill with bad-syntax information.
 *          pHeader - A handle to the header object.
 *          pstrBadSyntax - pointer to the bad-syntax string.
 ***************************************************************************/
RvStatus RVCALLCONV SipHeaderSetBadSyntaxStr(
                                          IN    void*           pHeader,
                                          IN    RvSipHeaderType eType,
                                          IN    RvChar*         pstrBadSyntax);

/***************************************************************************
 * SipHeaderConstruct
 * ------------------------------------------------------------------------
 * General: Constructs a header of any type.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * input:  hMsgMgr  - Handle to the Message manager.
 *         eType    - Type of the header to construct.
 *         hPool    - Handle to the memory pool that the object will use.
 *         hPage    - Handle to the memory page that the object will use.
 * output: ppHeader - Pointer to the newly constructed header object.
 ***************************************************************************/
RvStatus RVCALLCONV SipHeaderConstruct(IN  RvSipMsgMgrHandle         hMsgMgr,
                                       IN  RvSipHeaderType           eType,
                                       IN  HRPOOL                    hPool,
                                       IN  HPAGE                     hPage,
                                       OUT void**                    ppHeader);

/***************************************************************************
 * SipHeaderConstructInMsg
 * ------------------------------------------------------------------------
 * General: ConstructInMsgs a header of any type.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * input:  hMsgMgr  - Handle to the Message manager.
 *         eType    - Type of the header to ConstructInMsg.
 *         hPool    - Handle to the memory pool that the object will use.
 *         hPage    - Handle to the memory page that the object will use.
 * output: ppHeader - Pointer to the newly ConstructInMsged header object.
 ***************************************************************************/
RvStatus RVCALLCONV SipHeaderConstructInMsg(
                                             IN  RvSipMsgHandle            hMsg,
                                             IN  RvSipHeaderType           eType,
                                             OUT void**                    ppHeader);

/* SPIRENT_BEGIN */
//#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
#if(RV_TRUE)   // make the following functions always available for usages 
/* SPIRENT_END */
/***************************************************************************
 * SipHeaderName2Str
 * ------------------------------------------------------------------------
 * General: The function converts header type to string.
 *          for printing to log usage.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMsg -    Handle to the message.
 *          eType -   Type of the header to fill with bad-syntax information.
 *          pHeader - A handle to the header object.
 *          badSyntaxOffset - offset to bad-syntax information.
 ***************************************************************************/
RvChar* RVCALLCONV SipHeaderName2Str(RvSipHeaderType eHeaderType);

/***************************************************************************
 * SipAddressType2Str
 * ------------------------------------------------------------------------
 * General: The function converts header type to string.
 *          for printing to log usage.
 *
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMsg -    Handle to the message.
 *          eType -   Type of the header to fill with bad-syntax information.
 *          pHeader - A handle to the header object.
 *          badSyntaxOffset - offset to bad-syntax information.
 ***************************************************************************/
RvChar* RVCALLCONV SipAddressType2Str(RvSipAddressType eAddrType);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

#ifdef __cplusplus
}
#endif


#endif /* SIP_HEADER_H */
