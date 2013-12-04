/*
*******************************************************************************
*                                                                             *
* NOTICE:                                                                     *
* This document contains information that is confidential and proprietary to  *
* RADVision LTD.. No part of this publication may be reproduced in any form   *
* whatsoever without written prior approval by RADVision LTD..                *
*                                                                             *
* RADVision LTD. reserves the right to revise this publication and make       *
* changes without obligation to notify any person of such revisions or changes*
*******************************************************************************
*/


/******************************************************************************
 *                              <OaUtils.h>
 *
 * The OaUtils.h file defines various functions that are used by different
 * components of the Offer-Answer module.
 * The utility functions can be divided in following groups:
 *  1. Offer-Answer SDP Message.
 *  2. Conversions.
 *
 * Offer-Answer SDP Message represents the wrapper for the Message object,
 * defined in RADVISION SDP Stack. The Offer-Answer SDP Message provides
 * memory to hold SDP Stack Message itself and keeps the Allocator to be used
 * with the SDP Stack Message. Also the Offer-Answer SDP Message holds
 * reference to pool that feeds the Allocator object.
 *
 * Conversion functions convert mainly different enumeration values to string
 * and via versa, or SDP Stack enumeration values to Offer-Answer module
 * enumerations and via versa. The first type of function is used for logging
 * the second is used for wrappers of SDp Stack functions, provided
 * by the Offer-Answer module to the Application.
 *
 *    Author                        Date
 *    ------                        ----
 *    Igor                          Aug 2006
 *****************************************************************************/

#ifndef _OA_UTILS_H
#define _OA_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*                           INCLUDE HEADER FILES                            */
/*---------------------------------------------------------------------------*/
#include "AdsRa.h"
#include "RvOaTypes.h"

/*---------------------------------------------------------------------------*/
/*                              TYPE DEFINITIONS                             */
/*---------------------------------------------------------------------------*/
#define OA_CRV2RV(crv) RvErrorGetCode(crv)
#define OA_SRV2RV(srv) OaUtilsConvertSdp2OaStatus(srv)

/* length of "DEBUG,INFO,WARN,ERROR,EXCEP,LOCKDBG" string */
#define RVOA_LOGMASK_STRLEN (36)
#define RVOA_MAX_LOG_LINE   (80)
#define RVOA_UNDEFINED      -1


/******************************************************************************
 * OaSdpMsg
 * ----------------------------------------------------------------------------
 * OaSdpMsg represents the object that provides memory for RADVISION SDP Stack
 * Message object and holds auxiliary data, required by the SDP Stack in order
 * to manipulate this message.
 * The memory for the message is provided as a part of this structure.
 * See field sdpMsg.
 *****************************************************************************/
typedef struct
{
    HRPOOL  hPool;
        /* Handle to the pool, which feeds the Allocator object with pages. */

    RvSdpMsg  sdpMsg;
        /* Memory for RADVISION Stack SDP Message object. */

    RvAlloc  sdpAllocator;
        /* RADVISION Stack SDP Allocator object.
           The Allocator brings pieces of memory in form of RPOOL pages,
           required by the SDP Message to hold its string data. */

    RvSdpMsg*  pSdpMsg;
        /* Pointer to RADVISION SDP Stack Message object, contained in sdpMsg
           field. If it is NULL, the field contains invalid data and can't be
           used as a SDP Message. */

    RvAlloc*  pSdpAllocator;
        /* Pointer to RADVISION SDP Stack Allocator object, contained in
           sdpAllocator field. If it is NULL, the field contains invalid data
           and can't be used as an Allocator. The message can't be used also.*/

} OaSdpMsg;

/******************************************************************************
 * OaPSdpMsg
 * ----------------------------------------------------------------------------
 * OaPSdpMsg represents the object that provides memory for RADVISION SDP Stack
 * Message object and holds auxiliary data, required by the SDP Stack in order
 * to manipulate this message.
 * The memory for the message is provided from the RA pool. See hRaMsgs field.
 *****************************************************************************/
typedef struct
{
    HRA  hRaMsgs;
        /* Handle to the pool of RADVISION SDP Stack Message objects.
           Elements of this pool are used as messages.
           Each element has size of sizeof(RvSdpMsg) bytes. */

    HRA  hRaAllocators;
        /* Handle to the pool of RADVISION SDP Stack Allocator objects.
           Elements of this pool are used as allocators.
           Each element has size of sizeof(RvAlloc) bytes. */

    RvSdpMsg*  pSdpMsg;
        /* Pointer to the RADVISION SDP Stack Message object.
           Actually points to the element of hRaMsgs pool.
           If NULL, the message is invalid and can't be used. */

    RvAlloc*  pSdpAllocator;
        /* Pointer to the RADVISION SDP Stack Allocator object.
           Actually points to the element of hRaAllocators pool.
           If NULL, the message is invalid and can't be used. */

    RvBool  bSdpMsgConstructed;
        /* RV_TRUE, if the pSdpMsg points to the valid SDP message.
           RV_FALSE, if pSdpMsg NULL or points to the page, that can't be used
           as a valid SDP message. */

} OaPSdpMsg;

/*---------------------------------------------------------------------------*/
/*                            FUNCTION DEFINITIONS                           */
/*---------------------------------------------------------------------------*/
/******************************************************************************
 * OaUtilsConvertEnum2StrStatus
 * ----------------------------------------------------------------------------
 * General:
 *  Converts RvStatus enumeration value into string.
 *
 * Arguments:
 * Input:  crv - value of RvStatus enumeration.
 * Output: none.
 *
 * Return Value: string.
 *****************************************************************************/
const RvChar* RVCALLCONV OaUtilsConvertEnum2StrStatus(RvStatus crv);

/******************************************************************************
 * OaUtilsConvertEnum2StrLogMask
 * ----------------------------------------------------------------------------
 * General:
 *  Converts mask of log filters into string.
 *  String representation of filters set in the mask, is concatenated into
 *  the returned string.
 *
 * Arguments:
 * Input:  logMask - RvStatus enumeration value.
 * Output: strMask - concatenation of log mask filters in string form.
 *                   This parameter should point to the buffer large enough
 *                   to contain all possible filters.
 *                   Use buffers of at least RVOA_LOGMASK_STRLEN size.
 *
 * Return Value: strMask parameter.
 *****************************************************************************/
RvChar* RVCALLCONV OaUtilsConvertEnum2StrLogMask(
                                            IN  RvInt32 logMask,
                                            OUT RvChar* strMask);

/******************************************************************************
 * OaUtilsConvertOa2CcLogMask
 * ----------------------------------------------------------------------------
 * General:
 *  Converts log mask in Offer-Answer format into log mask in Common Core
 *  format.
 *
 * Arguments:
 * Input:  oaLogMask - log mask in Offer-Answer format.
 * Output: none.
 *
 * Return Value: log mask in Common Core format.
 *****************************************************************************/
RvLogMessageType RVCALLCONV OaUtilsConvertOa2CcLogMask(IN RvInt32 oaLogMask);

/******************************************************************************
 * OaUtilsConvertCc2OaLogFilter
 * ----------------------------------------------------------------------------
 * General:
 *  Converts log filter in Common Core format into log filter in Offer-Answer
 *  format.
 *
 * Arguments:
 * Input:  ccLogFilter - log filter in Offer-Answer format.
 * Output: none.
 *
 * Return Value: log filter in Common Core format.
 *****************************************************************************/
RvOaLogFilter RVCALLCONV OaUtilsConvertCc2OaLogFilter(IN RvInt ccLogFilter);

/******************************************************************************
 * OaUtilsConvertEnum2StrSdpParseStatus
 * ----------------------------------------------------------------------------
 * General:
 *  Converts value of SDP Stack parse status enumeration into string.
 *
 * Arguments:
 * Input:  eSdpParseStatus - RvSdpParseStatus enumeration value.
 * Output: none.
 *
 * Return Value: string.
 *****************************************************************************/
const RvChar* RVCALLCONV OaUtilsConvertEnum2StrSdpParseStatus(
                                        IN RvSdpParseStatus eSdpParseStatus);

/******************************************************************************
 * OaUtilsConvertSdp2OaStatus
 * ----------------------------------------------------------------------------
 * General:
 *  Converts status, returned by SDP Stack API into status, returned by
 *  Offer-Answer API.
 *
 * Arguments:
 * Input:  eSdpStatus - SDP Stack value.
 * Output: none.
 *
 * Return Value: Offer-Answer value.
 *****************************************************************************/
RvStatus RVCALLCONV OaUtilsConvertSdp2OaStatus(IN RvSdpStatus sdpStatus);

/******************************************************************************
 * OaUtilsConvertEnum2StrSdpStatus
 * ----------------------------------------------------------------------------
 * General:
 *  Converts value of RvSdpStatus enumeration into string.
 *
 * Arguments:
 * Input:  eSdpStatus - RvSdpStatus enumeration value.
 * Output: none.
 *
 * Return Value: string.
 *****************************************************************************/
const RvChar* RVCALLCONV OaUtilsConvertEnum2StrSdpStatus(
                                        IN RvSdpStatus eSdpStatus);

/******************************************************************************
 * OaUtilsConvertEnum2StrSessionState
 * ----------------------------------------------------------------------------
 * General:
 *  Converts value of RvOaSessionState enumeration into string.
 *
 * Arguments:
 * Input:  eState - RvOaSessionState enumeration value.
 * Output: none.
 *
 * Return Value: string.
 *****************************************************************************/
const RvChar* RVCALLCONV OaUtilsConvertEnum2StrSessionState(
                                        IN RvOaSessionState  eState);

/******************************************************************************
 * OaUtilsConvertEnum2StrStreamState
 * ----------------------------------------------------------------------------
 * General:
 *  Converts value of RvOaStreamState enumeration into string.
 *
 * Arguments:
 * Input:  eState - RvOaStreamState enumeration value.
 * Output: none.
 *
 * Return Value: string.
 *****************************************************************************/
const RvChar* RVCALLCONV OaUtilsConvertEnum2StrStreamState(
                                        IN RvOaStreamState  eState);

/******************************************************************************
 * OaUtilsConvertEnum2StrLogSource
 * ----------------------------------------------------------------------------
 * General:
 *  Converts value of RvOaLogSource enumeration into string.
 *
 * Arguments:
 * Input:  eState - RvOaLogSource enumeration value.
 * Output: none.
 *
 * Return Value: string.
 *****************************************************************************/
const RvChar* RVCALLCONV OaUtilsConvertEnum2StrLogSource(
                                                IN RvOaLogSource  eLogSource);

/******************************************************************************
 * OaUtilsConvertEnum2StrCodec
 * ----------------------------------------------------------------------------
 * General:
 *  Converts value of RvOaCodec enumeration to string.
 *
 * Arguments:
 * Input:  eCodec - enumeration value to be converted.
 * Output: none.
 *
 * Return Value: string.
 *****************************************************************************/
const RvChar* RVCALLCONV OaUtilsConvertEnum2StrCodec(IN RvOaCodec eCodec);

/******************************************************************************
 * OaUtilsConvertStr2EnumCodec
 * ----------------------------------------------------------------------------
 * General:
 *  Converts string to value of RvOaCodec enumeration.
 *
 * Arguments:
 * Input:  strFormatName - string to be converted.
 * Output: none.
 *
 * Return Value: enumeration value.
 *****************************************************************************/
RvOaCodec RVCALLCONV OaUtilsConvertStr2EnumCodec(IN const RvChar* strCodec);

/******************************************************************************
 * OaUtilsOaSdpMsgConstruct
 * ----------------------------------------------------------------------------
 * General:
 *  Constructs Offer-Answer SDP Message object, while freeing object resources,
 *  if the object was already in use. Constructs SDP Stack Allocator and
 *  SDP Stack Message objects in the memory, contained in the Offer-Answer SDP
 *  Message structure.
 *
 * Arguments:
 * Input:  hPool      - pool to be used while constructing Allocator.
 *                      This pool provides memory for strings, kept by SDP
 *                      Message object.
 *         pLogSource - log source to be used for log printing.
 * Output: pOaSdpMsg  - constructed Offer-Answer SDP message.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaUtilsOaSdpMsgConstruct(
                                    IN    HRPOOL        hPool,
                                    IN    RvLogSource*  pLogSource,
                                    OUT OaSdpMsg*       pSdpMsg);

/******************************************************************************
 * OaUtilsOaSdpMsgConstructParse
 * ----------------------------------------------------------------------------
 * General:
 *  Constructs Offer-Answer SDP Message object, while freeing object resources,
 *  if the object was already in use. Constructs SDP Stack Allocator and
 *  SDP Stack Message objects in the memory, contained in the Offer-Answer SDP
 *  Message structure. Parses provided SDP message in string form
 *  into the constructed message.
 *
 * Arguments:
 * Input:  hPool      - pool to be used while constructing Allocator.
 *                      This pool provides memory for strings, kept by SDP
 *                      Message object.
 *         pLogSource - log source to be used for log printing.
 *         pOaSdpMsg  - Offer-Answer SDP message.
 *         strSdpMsg  - SDP message in string form.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaUtilsOaSdpMsgConstructParse(
                                    IN HRPOOL        hPool,
                                    IN RvLogSource*  pLogSource,
                                    IN OaSdpMsg*     pOaSdpMsg,
                                    IN RvChar*       strSdpMsg);

/******************************************************************************
 * OaUtilsOaSdpMsgConstructCopy
 * ----------------------------------------------------------------------------
 * General:
 *  Constructs Offer-Answer SDP Message object, while freeing object resources,
 *  if the object was already in use. Constructs SDP Stack Allocator and
 *  SDP Stack Message objects in the memory, contained in the Offer-Answer SDP
 *  Message structure. Copies SDP message in form of SDP Stack message into
 *  the constructed message.
 *
 * Arguments:
 * Input:  hPool      - pool to be used while constructing Allocator.
 *                      This pool provides memory for strings, kept by SDP
 *                      Message object.
 *         pLogSource - log source to be used for log printing.
 *         pOaSdpMsg  - Offer-Answer SDP message.
 *         pSdpMsg    - SDP message in form of SDP Stack Message object.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaUtilsOaSdpMsgConstructCopy(
                                    IN HRPOOL        hPool,
                                    IN RvLogSource*  pLogSource,
                                    IN OaSdpMsg*     pOaSdpMsg,
                                    IN RvSdpMsg*     pSdpMsg);

/******************************************************************************
 * OaUtilsOaSdpMsgDestruct
 * ----------------------------------------------------------------------------
 * General:
 *  Destructs Offer-Answer SDP Message object:
 *  destructs contained SDP Stack message and SDP Stack allocator.
 *
 * Arguments:
 * Input:  pOaSdpMsg  - Offer-Answer SDP message.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
void RVCALLCONV OaUtilsOaSdpMsgDestruct(IN OaSdpMsg*  pOaSdpMsg);

/******************************************************************************
 * OaUtilsOaPSdpMsgConstructParse
 * ----------------------------------------------------------------------------
 * General:
 *  Constructs Offer-Answer Pool SDP Message object, while freeing object
 *  resources, if the object was already in use. Constructs SDP Stack Allocator
 *  and SDP Stack Message objects in the memory, provided by external pools.
 *  Parses provided SDP message in string form into the constructed message.
 *
 * Arguments:
 * Input:  hRaAllocators - pool to be used while allocating Allocator object.
 *                         This pool provides memory for the SDP Stack
 *                         Allocator object used by Pool SDP Message.
 *         hRaMsgs       - pool to be used while constructing SDP Message
 *                         object. This pool provides memory for the SDP Stack
 *                         Message object used by Pool SDP Message.
 *         hPool         - pool to be used while constructing Allocator.
 *                         This pool provides memory for strings, kept by SDP
 *                         Message object.
 *         pOaSdpMsg     - Offer-Answer SDP message.
 *         strSdpMsg     - SDP message in string form.
 *         pLogSource    - log source to be used for log printing.
 *         pMsgOwner     - owner of the message, which is printed into log.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaUtilsOaPSdpMsgConstructParse(
                                    IN HRA           hRaAllocators,
                                    IN HRA           hRaMsgs,
                                    IN HRPOOL        hPool,
                                    IN OaPSdpMsg*    pOaSdpMsg,
                                    IN RvChar*       strSdpMsg,
                                    IN RvLogSource*  pLogSource,
                                    IN void*         pMsgOwner);

/******************************************************************************
 * OaUtilsOaPSdpMsgConstructCopy
 * ----------------------------------------------------------------------------
 * General:
 *  Constructs Offer-Answer Pool SDP Message object, while freeing object
 *  resources, if the object was already in use. Constructs SDP Stack Allocator
 *  and SDP Stack Message objects in the memory, provided by external pools.
 *  Copies provided SDP message in form of SDP Stack Message object into
 *  the constructed message.
 *
 * Arguments:
 * Input:  hRaAllocators - pool to be used while allocating Allocator object.
 *                         This pool provides memory for the SDP Stack
 *                         Allocator object used by Pool SDP Message.
 *         hRaMsgs       - pool to be used while constructing SDP Message
 *                         object. This pool provides memory for the SDP Stack
 *                         Message object used by Pool SDP Message.
 *         hPool         - pool to be used while constructing Allocator.
 *                         This pool provides memory for strings, kept by SDP
 *                         Message object.
 *         pOaSdpMsg     - Offer-Answer SDP message.
 *         strSdpMsg     - SDP message in string form.
 *         pLogSource    - log source to be used for log printing.
 *         pMsgOwner     - owner of the message, which is printed into log.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
RvStatus RVCALLCONV OaUtilsOaPSdpMsgConstructCopy(
                                    IN HRA           hRaAllocators,
                                    IN HRA           hRaMsgs,
                                    IN HRPOOL        hPool,
                                    IN OaPSdpMsg*    pOaSdpMsg,
                                    IN RvSdpMsg*     pSdpMsg,
                                    IN RvLogSource*  pLogSource,
                                    IN void*         pMsgOwner);

/******************************************************************************
 * OaUtilsOaPSdpMsgDestruct
 * ----------------------------------------------------------------------------
 * General:
 *  Destructs Offer-Answer Pool SDP Message object:
 *  destructs contained SDP Stack Message object and SDP Stack Allocator object,
 *  frees memory, used by these objects.
 *
 * Arguments:
 * Input:  pOaSdpMsg - Offer-Answer Pool SDP message.
 * Output: none.
 *
 * Return Value: RV_OK - if successful. Other on failure.
 *****************************************************************************/
void RVCALLCONV OaUtilsOaPSdpMsgDestruct(IN OaPSdpMsg* pOaSdpMsg);

/******************************************************************************
 * OaUtilsCompareCodecNames
 * ----------------------------------------------------------------------------
 * General:
 *  Compares two strings that contain names of codecs:
 *  1. performs case insensitive comparison of strings
 *  2. ignores dots while comparing
 *
 * Arguments:
 * Input:  strName1 - the codec name.
 *         strName2 - the codec name.
 * Output: none.
 *
 * Return Value: RV_TRUE, if the strings describe same codec,
 *               RV_FALSE - otherwise.
 *****************************************************************************/
RvBool RVCALLCONV OaUtilsCompareCodecNames(IN const RvChar* strName1,
                                           IN const RvChar* strName2);

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef _OA_UTILS_H */


/*nl for linux */

