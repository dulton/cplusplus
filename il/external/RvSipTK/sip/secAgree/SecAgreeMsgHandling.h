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
 *                              <SecAgreeMsgHandling.h>
 *
 * This file defines methods for handling messages: loading security infomration
 * from message, loading security information onto messages, etc.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2006
 *********************************************************************************/

#ifndef SEC_AGREE_MSG_HANDLING_H
#define SEC_AGREE_MSG_HANDLING_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "SecAgreeTypes.h"
#include "RvSipMsgTypes.h"

/*-----------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                         SECURITY AGREEMENT FUNCTIONS                  */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeMsgHandlingSetRequireHeadersToMsg
 * ------------------------------------------------------------------------
 * General: Adds Require sec-agree and Proxy-Require sec-agree to message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecurity    - Pointer to the security-agreement
 *         hMsg         - Handle to the message to write on
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingSetRequireHeadersToMsg(
									IN    SecAgree*            pSecAgree,
									IN    RvSipMsgHandle       hMsg);

/*-----------------------------------------------------------------------*/
/*                   SECURITY CLIENT FUNCTION HEADERS                    */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeMsgHandlingClientSetSecurityClientListToMsg
 * ------------------------------------------------------------------------
 * General: Adds the list of local security mechanisms as Security-Client
 *          headers to the message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient    - Pointer to the client security-agreement
 *         hMsg               - Handle to the message to write on
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingClientSetSecurityClientListToMsg(
									IN    SecAgree*            pSecAgreeClient,
									IN    RvSipMsgHandle       hMsg);

/***************************************************************************
 * SecAgreeMsgHandlingClientCleanMsg
 * ------------------------------------------------------------------------
 * General: Remove Require, Proxy-Require and Security-Client headers that
 *          were previously added to the message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecurityClient    - Pointer to the client security-agreement
 *         hMsg               - Handle to the message to write on
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingClientCleanMsg(
									IN    SecAgree*            pSecAgreeClient,
									IN    RvSipMsgHandle       hMsg);

/***************************************************************************
 * SecAgreeMsgHandlingClientSetSecurityVerifyListToMsg
 * ------------------------------------------------------------------------
 * General: Adds the list of remote security mechanisms as Security-Verify
 *          headers to the message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient    - Pointer to the client security-agreement
 *         hMsg               - Handle to the message to write on
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingClientSetSecurityVerifyListToMsg(
									IN    SecAgree*            pSecAgreeClient,
									IN    RvSipMsgHandle       hMsg);

/***************************************************************************
 * SecAgreeMsgHandlingClientIsThereSecurityServerListInMsg
 * ------------------------------------------------------------------------
 * General: Checks whether there are Security-Server headers in the message
 * Return Value: RV_TURE if there is Security-Server header in the message
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg          - Handle to the message
 * Output   phMsgListElem - Handle to the list element of the first security header,
 *                          if exists. This way when building the list we can avoid
 *                          repeating the search for a new
 *          phHeader      - Pointer to the first security header, if exists.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeMsgHandlingClientIsThereSecurityServerListInMsg(
									IN    RvSipMsgHandle             hMsg,
									OUT   RvSipHeaderListElemHandle* phMsgListElem,
									OUT   RvSipSecurityHeaderHandle* phHeader);

/***************************************************************************
 * SecAgreeMsgHandlingClientLoadSecurityServerListFromMsg
 * ------------------------------------------------------------------------
 * General: Load the list of Security-Server headers from the message
 *          into the client security-agreement object. Notice: each Security-Server
 *          header is stored in the client security-agreement object as Security-Verify
 *          header for insertions to further requests.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient    - Pointer to the client security-agreement
 *         hMsg               - Handle to the message to write on
 *         hMsgListElem       - The first Security-Server header in the message, if
 *                              previously found.
 *         hHeader            - The first Security-Server header in the message, if
 *                              previously found.
 * Output: pbIsSecAgreeServer - RV_TRUE if there are security-server headers in
 *                              the message, RV_FALSE if there aren't.
 *		   pbIsDigest         - RV_TRUE if there was digest mechanism in one of
 *                              the Security-Server headers. If yes, Proxy-Authenticate
 *                              headers will be copied from the message.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingClientLoadSecurityServerListFromMsg(
									IN    SecAgree*                   pSecAgreeClient,
									IN    RvSipMsgHandle              hMsg,
									IN    RvSipHeaderListElemHandle   hMsgListElem,
									IN    RvSipSecurityHeaderHandle   hHeader,
									OUT   RvBool*                     pbIsSecurityServer,
									OUT   RvBool*                     pbIsDigest);

/***************************************************************************
 * SecAgreeMsgHandlingClientLoadAuthenticationDataFromMsg
 * ------------------------------------------------------------------------
 * General: Stores the list of Proxy-Authenticate headers that was received
 *          on the response in the client security-agreement object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient    - Pointer to the Security-Manager
 *         hMsg               - Handle to the message to write on
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingClientLoadAuthenticationDataFromMsg(
									IN    SecAgree*               pSecAgreeClient,
									IN    RvSipMsgHandle          hMsg);

/***************************************************************************
 * SecAgreeMsgHandlingClientSetAuthorizationDataToMsg
 * ------------------------------------------------------------------------
 * General: Computes a list of Authorization headers from the Proxy-Authenticate
 *          list in the client security-agreement object, and adds it to the message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeClient    - Pointer to the client security-agreement
 *         hMsg               - Handle to the message to write on
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingClientSetAuthorizationDataToMsg(
									IN    SecAgree*               pSecAgreeClient,
									IN    RvSipMsgHandle          hMsg);

/*-----------------------------------------------------------------------*/
/*                   SECURITY SERVER FUNCTION HEADERS                    */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeMsgHandlingServerIsInvalidRequest
 * ------------------------------------------------------------------------
 * General: Checks whether the received request contains Require or Proxy-Require
 *          with sec-agree, but more than one Via. This message must be rejected
 *          with 502 (Bad Gateway).
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr	 - Pointer to the security-agreement manager
 *         hMsg          - Handle to the message
 * Output: pbIsInvalid   - RV_TRUE if there are multiple Vias and sec-agree required
 *         pbHasRequired - RV_TRUE if there were Require or Proxy-Require with
 *                         sec-agree
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerIsInvalidRequest(
							 IN  SecAgreeMgr*             pSecAgreeMgr,
							 IN  RvSipMsgHandle           hMsg,
							 OUT RvBool*                  pbIsInvalid,
							 OUT RvBool*                  pbHasRequired);

/***************************************************************************
 * SecAgreeMsgHandlingServerIsSupportedSecAgreeInMsg
 * ------------------------------------------------------------------------
 * General: Checks if there is Supported header with sec-agree in the message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMsg               - Handle to the message to write on
 * Output: pbIsSecAgree       - Boolean indication that there are Require
 *                              and Proxy-Require headers in the message
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerIsSupportedSecAgreeInMsg(
									IN    RvSipMsgHandle         hMsg,
									OUT   RvBool*                pbIsSecAgree);

/***************************************************************************
 * SecAgreeMsgHandlingServerIsRequireSecAgreeInMsg
 * ------------------------------------------------------------------------
 * General: Checks if there are Require or Proxy-Require headers with sec-agree
 *          in the message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMsg               - Handle to the message to write on
 * Output: pbIsSecAgree       - Boolean indication that there are Require
 *                              and Proxy-Require headers in the message
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerIsRequireSecAgreeInMsg(
									IN    RvSipMsgHandle         hMsg,
									OUT   RvBool*                pbIsSecAgree);

/***************************************************************************
 * SecAgreeMsgHandlingServerIsSingleVia
 * ------------------------------------------------------------------------
 * General: Checks if there is a single Via header in the message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMsg               - Handle to the message to write on
 * Output: pbIsSingleVia      - Boolean indication that there is a single Via
 *                              in the message
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerIsSingleVia(
									IN    RvSipMsgHandle         hMsg,
									OUT   RvBool*                pbIsSingleVia);

/***************************************************************************
 * SecAgreeMsgHandlingServerSetSecurityServerListToMsg
 * ------------------------------------------------------------------------
 * General: Adds the list of local security mechanisms as Security-Server
 *          headers to the message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeServer    - Pointer to the server security-agreement
 *         hMsg               - Handle to the message to write on
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerSetSecurityServerListToMsg(
									IN    SecAgree*              pSecAgreeServer,
									IN    RvSipMsgHandle         hMsg);

/***************************************************************************
 * SecAgreeMsgHandlingServerIsThereSecurityClientListInMsg
 * ------------------------------------------------------------------------
 * General: Checks whether there are Security-Client headers in the message
 * Return Value: RV_TURE if there is Security-Client header in the message
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMsg          - Handle to the message
 * Output: phMsgListElem - Handle to the list element of the first security header,
 *                         if exists. This way when building the list we can avoid
 *                         repeating the search for a new
 *         phHeader      - Pointer to the first security header, if exists.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeMsgHandlingServerIsThereSecurityClientListInMsg(
										IN    RvSipMsgHandle			 hMsg,
										OUT   RvSipHeaderListElemHandle* phMsgListElem,
										OUT   RvSipSecurityHeaderHandle* phHeader);

/***************************************************************************
 * SecAgreeMsgHandlingServerLoadSecurityClientListFromMsg
 * ------------------------------------------------------------------------
 * General: Load the list of Security-Client headers into the remote list of
 *          the server security-agreement.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeServer    - Pointer to the server security-agreement
 *         hMsg               - Handle to the message to write on
 *         hMsgListElem       - The first Security-Client header in the message, if
 *                              previously found.
 *         hHeader            - The first Security-Client header in the message, if
 *                              previously found.
 *         pbIsSecurityClient - RV_TRUE if there are security-client headers in
 *                              the message, RV_FALSE if there aren't.
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerLoadSecurityClientListFromMsg(
									IN    SecAgree*                   pSecAgreeServer,
									IN    RvSipMsgHandle              hMsg,
									IN    RvSipHeaderListElemHandle   hMsgListElem,
									IN    RvSipSecurityHeaderHandle   hHeader,
									OUT   RvBool*                     pbIsSecurityClient);

/***************************************************************************
 * SecAgreeMsgHandlingServerSetAuthenticationDataToMsg
 * ------------------------------------------------------------------------
 * General: Adds the list of Proxy-Authenticate headers held by the
 *          server security-agreement object to the message.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeServer    - Pointer to the server security-agreement
 *         hMsg               - Handle to the message to write on
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerSetAuthenticationDataToMsg(
									IN    SecAgree*               pSecAgreeServer,
									IN    RvSipMsgHandle          hMsg);

/***************************************************************************
 * SecAgreeMsgHandlingServerIsThereSecurityVerifyListInMsg
 * ------------------------------------------------------------------------
 * General: Checks whether there are Security-Verify headers in the message
 * Return Value: RV_TURE if there is Security-Verify header in the message
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMsg          - Handle to the message
 * Inout   phMsgListElem - Handle to the list element of the first security header,
 *                         if exists. This way when building the list we can avoid
 *                         repeating the search for a new
 *         ppHeader      - Pointer to the first security header, if exists.
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeMsgHandlingServerIsThereSecurityVerifyListInMsg(
										IN    RvSipMsgHandle			 hMsg,
										OUT   RvSipHeaderListElemHandle* phMsgListElem,
										OUT   RvSipSecurityHeaderHandle* phHeader);

/***************************************************************************
 * SecAgreeMsgHandlingServerCheckSecurityVerifyList
 * ------------------------------------------------------------------------
 * General: Compare the received list of Security-Verify headers to the local
 *          list of security headers in the server security-agreement object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeServer    - Pointer to the server security-agreement
 *         hMsg               - Handle to the message to compare to
 *         hMsgListElem       - Handle to the list element of the first security header,
 *                              if exists.
 *         hHeader            - Handle to the first security header, if exists.
 *         pbIsTispan		  - Indicates whether this is a TISPAN comparison. If it is,
 *							    compares the old list of local security to the received
 *							    Security-Verify list. If it is not, compares the local
 *							    security list that was set to this security agreement 
 *							    to the received Security-Verify list.
 * Output: pbAreEqual         - RV_TRUE if the two lists are equal, RV_FALSE
 *                              if not equal.
 *         pbHasMsgList       - RV_TRUE if there are Security-Verify headers in the message
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerCheckSecurityVerifyList(
									IN    SecAgree*                  pSecAgreeServer,
									IN    RvSipMsgHandle             hMsg,
									IN    RvSipHeaderListElemHandle  hMsgListElem,
									IN    RvSipSecurityHeaderHandle  hHeader,
									IN    RvBool                     bIsTispan,
									OUT   RvBool*                    pbAreEqual,
									OUT   RvBool*                    pbHasMsgList);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
RvStatus RVCALLCONV SecAgreeServerCheckSecurityVerifyList(
									IN    SecAgree*                  pSecAgree1,
									IN    SecAgree*                  pSecAgree2,
									OUT   RvBool*                    pbAreEqual);
#endif
/*SPIRENT_END */

/***************************************************************************
 * SecAgreeMsgHandlingServerCheckSecurityClientList
 * ------------------------------------------------------------------------
 * General: Compare the received list of Security-Client headers to the remote
 *          list of security headers in the server security-agreement object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeServer    - Pointer to the server security-agreement
 *         hMsg               - Handle to the message to compare to
 *         hMsgListElem       - Handle to the list element of the first security header,
 *                              if exists.
 *         hHeader            - Handle to the first security header, if exists.
 * Output: pbAreEqual         - RV_TRUE if the two lists are equal, RV_FALSE
 *                              if not equal.
 *         pbHasMsgList       - RV_TRUE if there are Security-Verify headers in the message
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeMsgHandlingServerCheckSecurityClientList(
									IN    SecAgree*					 pSecAgreeServer,
									IN    RvSipMsgHandle			 hMsg,
									IN    RvSipHeaderListElemHandle  hMsgListElem,
									IN    RvSipSecurityHeaderHandle  hHeader,
									OUT   RvBool*					 pbAreEqual,
									OUT   RvBool*                    pbHasMsgList);

#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef SEC_AGREE_MSG_HANDLING_H */

