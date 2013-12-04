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
 *                              <SecAgreeUtils.h>
 *
 * This file defines utility functions for the security-agreement module.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2006
 *********************************************************************************/

#ifndef SEC_AGREE_UTILS_H
#define SEC_AGREE_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "RvSipSecAgreeTypes.h"
#include "SecAgreeTypes.h"
    
/*-----------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeUtilsStateEnum2String
 * ------------------------------------------------------------------------
 * General: Returns the string representation of the given state
 * Return Value: state as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  eState - The enumerative state 
 ***************************************************************************/
RvChar* RVCALLCONV SecAgreeUtilsStateEnum2String(IN  RvSipSecAgreeState  eState);

/***************************************************************************
 * SecAgreeUtilsReasonEnum2String
 * ------------------------------------------------------------------------
 * General: Returns the string representation of the given reason
 * Return Value: reason as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  eReason - The enumerative reason 
 ***************************************************************************/
RvChar* RVCALLCONV SecAgreeUtilsReasonEnum2String(IN  RvSipSecAgreeReason  eReason);

/***************************************************************************
 * SecAgreeUtilsStatusEnum2String
 * ------------------------------------------------------------------------
 * General: Returns the string representation of the given status
 * Return Value: status as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  eStatus - The enumerative status 
 ***************************************************************************/
RvChar* RVCALLCONV SecAgreeUtilsStatusEnum2String(IN  RvSipSecAgreeStatus  eStatus);

/***************************************************************************
 * SecAgreeUtilsRoleEnum2String
 * ------------------------------------------------------------------------
 * General: Returns the string representation of the given role
 * Return Value: role as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  eRole - The enumerative role 
 ***************************************************************************/
RvChar* RVCALLCONV SecAgreeUtilsRoleEnum2String(IN  RvSipSecAgreeRole  eRole);

/***************************************************************************
 * SecAgreeUtilsMechanismEnum2String
 * ------------------------------------------------------------------------
 * General: Returns the string representation of the given security mechanism
 * Return Value: mechanism as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  eMechanism - The enumerative mechanism
 ***************************************************************************/
RvChar* RVCALLCONV SecAgreeUtilsMechanismEnum2String(IN  RvSipSecurityMechanismType  eMechanism);

/***************************************************************************
 * SecAgreeUtilsSpecialStandardEnum2String
 * ------------------------------------------------------------------------
 * General: Returns the string representation of the given special standard enumeration
 * Return Value: mechanism as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  eMechanism - The enumerative mechanism
 ***************************************************************************/
RvChar* RVCALLCONV SecAgreeUtilsSpecialStandardEnum2String(IN  RvSipSecAgreeSpecialStandardType  eStandard);

/***************************************************************************
 * SecAgreeUtilsIsListEmpty
 * ------------------------------------------------------------------------
 * General: Checks whether the given list is empty
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hList     - Handle to the list of security mechanisms
 * Output:  pbIsEmpty - Boolean indication to whether the list is empty
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeUtilsIsListEmpty(IN  RvSipCommonListHandle   hList,
											 OUT RvBool                 *pbIsEmpty);

/***************************************************************************
 * SecAgreeUtilsIsSupportedMechanism
 * ------------------------------------------------------------------------
 * General: Checks whether the given security mechanism is supported by the
 *          stack
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  eSecurityType - The security mechanism to check
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeUtilsIsSupportedMechanism(
									IN RvSipSecurityMechanismType  eSecurityType);

/***************************************************************************
 * SecAgreeUtilsIsIpsecNegOnly
 * ------------------------------------------------------------------------
 * General: Checks whether we are configured to work with ipsec-3gpp in 
 *          security-agreement only, and not in security-object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeUtilsIsIpsecNegOnly();

/***************************************************************************
 * SecAgreeUtilsIsMechanismHandledBySecObj
 * ------------------------------------------------------------------------
 * General: Checks whether the given security mechanism is handled by the
 *          security object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  eSecurityType - The security mechanism to check
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeUtilsIsMechanismHandledBySecObj(
									IN RvSipSecurityMechanismType  eSecurityType);

/***************************************************************************
 * SecAgreeUtilsIsMethodForSecAgree
 * ------------------------------------------------------------------------
 * General: Checks the method of the message to determine whether is should 
 *          contain security-agreement information
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg             - Handle to the message
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeUtilsIsMethodForSecAgree(IN     RvSipMsgHandle      hMsg);

/***************************************************************************
 * SecAgreeUtilsGetCSeqStep
 * ------------------------------------------------------------------------
 * General: Retrieves the CSeq step from a message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg             - Handle to the message
 ***************************************************************************/
RvUint32 RVCALLCONV SecAgreeUtilsGetCSeqStep(IN     RvSipMsgHandle      hMsg);

/***************************************************************************
 * SecAgreeUtilsSecurityHeaderGetAlgo
 * ------------------------------------------------------------------------
 * General: Retrieves the algorithm and encryption-algorithm from a Security
 *          header
 * Return Value: The algorithm enumeration
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecurityHeader  - Handle to the Security header
 ***************************************************************************/
RvSipSecurityAlgorithmType RVCALLCONV SecAgreeUtilsSecurityHeaderGetAlgo(
								IN   RvSipSecurityHeaderHandle     hSecurityHeader);

/***************************************************************************
 * SecAgreeUtilsSecurityHeaderGetEAlgo
 * ------------------------------------------------------------------------
 * General: Retrieves the algorithm and encryption-algorithm from a Security
 *          header
 * Return Value: The encryption algorithm enumeration
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecurityHeader  - Handle to the Security header
 ***************************************************************************/
RvSipSecurityEncryptAlgorithmType RVCALLCONV SecAgreeUtilsSecurityHeaderGetEAlgo(
								IN   RvSipSecurityHeaderHandle      hSecurityHeader);

/***************************************************************************
 * SecAgreeUtilsSecurityHeaderGetMode
 * ------------------------------------------------------------------------
 * General: Retrieves the mode parameter from a Security header
 * Return Value: The mode enumeration
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecurityHeader  - Handle to the Security header
 ***************************************************************************/
RvSipSecurityModeType RVCALLCONV SecAgreeUtilsSecurityHeaderGetMode(
								IN   RvSipSecurityHeaderHandle      hSecurityHeader);

/***************************************************************************
 * SecAgreeUtilsSecurityHeaderGetProtocol
 * ------------------------------------------------------------------------
 * General: Retrieves the protocol parameter from a Security header
 * Return Value: The protocol enumeration
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecurityHeader  - Handle to the Security header
 ***************************************************************************/
RvSipSecurityProtocolType RVCALLCONV SecAgreeUtilsSecurityHeaderGetProtocol(
								IN   RvSipSecurityHeaderHandle      hSecurityHeader);

/***************************************************************************
 * SecAgreeUtilsCopyKey
 * ------------------------------------------------------------------------
 * General: Copies IK and CK arrays.
 *          Notice: this function always copies RVSIP_SEC_MAX_KEY_LENGTH/8
 *          entries.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Inout:   pDestIK   - destination IK array
 *          pDestCK   - destination CK array
 * Input:   pSourceIK - Source IK array
 *          pSourceCK - Source CK array
 ***************************************************************************/
void RVCALLCONV SecAgreeUtilsCopyKey(INOUT  RvUint8*     pDestIK,
									 INOUT  RvUint8*     pDestCK,
									 IN     RvUint8*     pSourceIK,
									 IN     RvUint8*     pSourceCK);

/***************************************************************************
 * SecAgreeUtilsGetBetterHeader
 * ------------------------------------------------------------------------
 * General: Compares the q value of the two headers to determine the most
 *          preferable
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecHeader      - First security header to compare
 *          hOtherSecHeader - Second security header to compare
 * Inout:   pIntPart        - The int part of the q-val of the first security header
 *          pDecPart        - The dec part of the q-val of the first security header
 *			pOtherIntPart   - The int part of the q-val of the second security header
 *          pOtherDecPart   - The dec part of the q-val of the second security header     
 * Output:  phPrefSecHeader - The most preferable security header
 *          pPrefIntPart    - The int part of the q-val of the chosen security header
 *          pPrefDecPart    - The dec part of the q-val of the chosen security header  
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeUtilsGetBetterHeader(
											IN    RvSipSecurityHeaderHandle  hSecHeader,
											IN    RvSipSecurityHeaderHandle  hOtherSecHeader,
											INOUT RvInt32*                   pIntPart,
											INOUT RvInt32*                   pDecPart,
									        INOUT RvInt32*                   pOtherIntPart,
											INOUT RvInt32*                   pOtherDecPart,
									        OUT   RvSipSecurityHeaderHandle* phPrefSecHeader,
											OUT   RvInt32*                   pPrefIntPart,
											OUT   RvInt32*                   pPrefDecPart);

/***************************************************************************
 * SecAgreeUtilsGetBetterHeader
 * ------------------------------------------------------------------------
 * General: Compares the q value of the two headers to determine the most
 *          preferable
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hSecHeader  - First security header to compare
 * Output:  pIntPart    - The int part of the q-val
 *          pDecPart    - The dec part of the q-val
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeUtilsGetQVal(IN    RvSipSecurityHeaderHandle  hSecHeader,
										 OUT   RvInt32*                   pIntPart,
										 OUT   RvInt32*                   pDecPart);

/***************************************************************************
 * SecAgreeUtilsFindBestHeader
 * ------------------------------------------------------------------------
 * General: Among all security headers with the given mechanism, find the
 *          most preferable one.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hList            - Handle to the list of security mechanisms
 *          eMechType        - The mechanism to look for
 *          eIpsecAlg        - The ipsec algorithm to consider
 *          eIpsecEAlg       - The ipsec encryption algorithm to consider
 *          eIpsecMode       - The ipsec mode to consider
 *          eIpsecProtocol   - The ipsec protocol to consider
 * Output:  phSecurityHeader - The most preferable header
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeUtilsFindBestHeader(
										   IN  RvSipCommonListHandle             hList,
										   IN  RvSipSecurityMechanismType        eMechType,
										   IN  RvSipSecurityAlgorithmType        eIpsecAlg,
										   IN  RvSipSecurityEncryptAlgorithmType eIpsecEAlg,
										   IN  RvSipSecurityModeType             eIpsecMode,
										   IN  RvSipSecurityProtocolType         eIpsecProtocol,
									 	   OUT RvSipSecurityHeaderHandle*        phSecurityHeader);

/***************************************************************************
 * SecAgreeUtilsIsLargerQ
 * ------------------------------------------------------------------------
 * General: Determines whether the second q-value is larger than the first
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   intPart           - The int part of the q-val 
 *          decPart           - The dec part of the q-val 
 *			isLargerIntPart   - The int part of the q-val to check if larger
 *          isLargerDecPart   - The dec part of the q-val to check if larger     
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeUtilsIsLargerQ(
									IN RvInt32              intPart,
									IN RvInt32              decPart,
									IN RvInt32              isLargerIntPart,
									IN RvInt32              isLargerDecPart);

/***************************************************************************
 * SecAgreeUtilsIsMechanismInList
 * ------------------------------------------------------------------------
 * General: Checks whether the given mechanism appears in the given list of 
 *          security mechanisms
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hList            - Handle to the list of security mechanisms
 * Output:  pbIsMech         - Boolean indication to whether this appears in the
 *                             mechanisms list
 *          phSecurityHeader - Pointer to the security header with the given 
 *                             mechanism
 *          pbIsSingleMech   - Boolean indication to whether this mechanism is
 *                             the only mechanism that appears in the list
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeUtilsIsMechanismInList(
											  IN  RvSipCommonListHandle        hList,
											  IN  RvSipSecurityMechanismType   eMechType,
									 	      OUT RvBool                      *pbIsMech,
											  OUT RvSipSecurityHeaderHandle   *phSecurityHeader,
											  OUT RvBool                      *pbIsSingleMech);

/***************************************************************************
 * SecAgreeUtilsIsListLegal
 * ------------------------------------------------------------------------
 * General: Checks whether the list of security mechanisms has legal
 *          parameters
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hList      - Handle to the list of security mechanisms
 * Output:  pbIsLegal  - Boolean indication to whether the list is legal
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeUtilsIsListLegal(IN  RvSipCommonListHandle        hList,
											 OUT RvBool                      *pbIsLegal);

/***************************************************************************
 * SecAgreeUtilsIsGmInterfaceSupport
 * ------------------------------------------------------------------------
 * General: Checks whether the security-agreement supports the Gm interface,
 *          i.e, whether it was set to support special standard 3GPP 24.229
 *          or special standard TISPAN ES 283.003
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pSecAgree  - The security-agreement object
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeUtilsIsGmInterfaceSupport(IN  SecAgree       *pSecAgree);

#endif /* #ifdef RV_SIP_IMS_ON */


#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SEC_AGREE_UTILS_H*/

