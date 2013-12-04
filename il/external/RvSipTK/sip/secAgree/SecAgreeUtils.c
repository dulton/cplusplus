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
 *                              <SecAgreeUtils.c>
 *
 * This file defines utility functions for the security-agreement module.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Feb 2006
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "SecAgreeUtils.h"
#include "RvSipSecurityHeader.h"
#include "_SipMsgUtils.h"
#include "RvSipMsg.h"
#include "RvSipCSeqHeader.h"
#include "_SipTransport.h"

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                           */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                         SECURITY CLIENT FUNCTIONS                     */
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
RvChar* RVCALLCONV SecAgreeUtilsStateEnum2String(IN  RvSipSecAgreeState  eState)
{
	switch (eState)
	{
	case RVSIP_SEC_AGREE_STATE_IDLE:
		return "Idle";
	case RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_READY:
		return "Client Local List Ready";
	case RVSIP_SEC_AGREE_STATE_CLIENT_REMOTE_LIST_READY:
		return "Client Remote List Ready";
	case RVSIP_SEC_AGREE_STATE_CLIENT_HANDLING_INITIAL_MSG:
		return "Client Handling Initial Msg";
	case RVSIP_SEC_AGREE_STATE_CLIENT_LOCAL_LIST_SENT:
		return "Client Local List Sent";
	case RVSIP_SEC_AGREE_STATE_SERVER_LOCAL_LIST_READY:
		return "Server Local List Ready";
	case RVSIP_SEC_AGREE_STATE_SERVER_REMOTE_LIST_READY:
		return "Server Remote List Ready";
	case RVSIP_SEC_AGREE_STATE_CLIENT_CHOOSE_SECURITY:
		return "Client Choose Security";
	case RVSIP_SEC_AGREE_STATE_ACTIVE:
		return "Active";
	case RVSIP_SEC_AGREE_STATE_NO_AGREEMENT_REQUIRED:
		return "No Agreement Required";
	case RVSIP_SEC_AGREE_STATE_INVALID:
		return "Invalid";
	case RVSIP_SEC_AGREE_STATE_TERMINATED:
		return "Terminated";
	default:
		return "Undefined";
	}
}

/***************************************************************************
 * SecAgreeUtilsReasonEnum2String
 * ------------------------------------------------------------------------
 * General: Returns the string representation of the given reason
 * Return Value: reason as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  eReason - The enumerative reason 
 ***************************************************************************/
RvChar* RVCALLCONV SecAgreeUtilsReasonEnum2String(IN  RvSipSecAgreeReason  eReason)
{
	switch (eReason)
	{
	case RVSIP_SEC_AGREE_REASON_SECURITY_CLIENT_RCVD:
		return "Security-Client Rcvd";
	case RVSIP_SEC_AGREE_REASON_SECURITY_SERVER_RCVD:
		return "Security-Server Rcvd";
	case RVSIP_SEC_AGREE_REASON_MSG_SEND_FAILURE:
		return "Msg Send Failure";
	case RVSIP_SEC_AGREE_REASON_SECURITY_CLIENT_MISMATCH:
		return "Security-Client mismatch";
	case RVSIP_SEC_AGREE_REASON_SECURITY_VERIFY_MISMATCH:
		return "Security-Verify mismatch";
	case RVSIP_SEC_AGREE_REASON_INVALID_OR_MISSING_DIGEST:
		return "Invalid or Missing Digest";
	case RVSIP_SEC_AGREE_REASON_SECURITY_OBJ_EXPIRED:
		return "Security Obj Expired";
	default:
		return "Undefined";
	}
}

/***************************************************************************
 * SecAgreeUtilsStatusEnum2String
 * ------------------------------------------------------------------------
 * General: Returns the string representation of the given status
 * Return Value: status as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  eStatus - The enumerative status 
 ***************************************************************************/
RvChar* RVCALLCONV SecAgreeUtilsStatusEnum2String(IN  RvSipSecAgreeStatus  eStatus)
{
	switch (eStatus)
	{
	case RVSIP_SEC_AGREE_STATUS_IPSEC_LIFETIME_EXPIRED:
		return "Ipsec Lifetime Expired";
	default:
		return "Undefined";
	}
}

/***************************************************************************
 * SecAgreeUtilsRoleEnum2String
 * ------------------------------------------------------------------------
 * General: Returns the string representation of the given role
 * Return Value: role as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  eRole - The enumerative role 
 ***************************************************************************/
RvChar* RVCALLCONV SecAgreeUtilsRoleEnum2String(IN  RvSipSecAgreeRole  eRole)
{
	switch (eRole)
	{
	case RVSIP_SEC_AGREE_ROLE_CLIENT:
		return "Client";
	case RVSIP_SEC_AGREE_ROLE_SERVER:
		return "Server";
	default:
		return "Undefined";
	}
}

/***************************************************************************
 * SecAgreeUtilsMechanismEnum2String
 * ------------------------------------------------------------------------
 * General: Returns the string representation of the given security mechanism
 * Return Value: mechanism as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  eMechanism - The enumerative mechanism
 ***************************************************************************/
RvChar* RVCALLCONV SecAgreeUtilsMechanismEnum2String(IN  RvSipSecurityMechanismType  eMechanism)
{
	switch (eMechanism)
	{
	case RVSIP_SECURITY_MECHANISM_TYPE_DIGEST:
		return "Digest";
	case RVSIP_SECURITY_MECHANISM_TYPE_TLS:
		return "Tls";
	case RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_IKE:
		return "Ipsec-ike";
	case RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_MAN:
		return "Ipsec-man";
	case RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP:
		return "Ipsec-3gpp";
	default:
		return "Undefined";
	}
}

/***************************************************************************
 * SecAgreeUtilsSpecialStandardEnum2String
 * ------------------------------------------------------------------------
 * General: Returns the string representation of the given special standard enumeration
 * Return Value: mechanism as string
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  eMechanism - The enumerative mechanism
 ***************************************************************************/
RvChar* RVCALLCONV SecAgreeUtilsSpecialStandardEnum2String(IN  RvSipSecAgreeSpecialStandardType  eStandard)
{
	switch (eStandard)
	{
	case RVSIP_SEC_AGREE_SPECIAL_STANDARD_TISPAN_283003:
		return "TISPAN 283.003";
	case RVSIP_SEC_AGREE_SPECIAL_STANDARD_3GPP_24229:
		return "3GPP 24.229";
	case RVSIP_SEC_AGREE_SPECIAL_STANDARD_PKT_2:
		return "PKT 2.0";
	default:
		return "none";
	}
}

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
											 OUT RvBool                 *pbIsEmpty)

{
	RvSipCommonListElemHandle hListElem;
	void*                     hHeader;
	RvInt                     elementType;
	RvStatus                  rv;

	*pbIsEmpty = RV_TRUE;
		
	if (NULL == hList)
	{
		return RV_OK;
	}

	rv = RvSipCommonListGetElem(hList, RVSIP_FIRST_ELEMENT, NULL,
						        &elementType, &hHeader, &hListElem);
	if (RV_OK != rv)
	{
		return rv;
	}

	if (NULL != hListElem)
	{
		*pbIsEmpty = RV_FALSE;
	}

	return RV_OK;
}

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
									IN RvSipSecurityMechanismType  eSecurityType)
{
	if (eSecurityType == RVSIP_SECURITY_MECHANISM_TYPE_DIGEST)
	{
		return RV_TRUE;
	}

#if (RV_TLS_TYPE != RV_TLS_NONE)
	if (eSecurityType == RVSIP_SECURITY_MECHANISM_TYPE_TLS)
	{
		return RV_TRUE;
	}
#endif /* #if (RV_TLS_TYPE != RV_TLS_NONE) */

#if (RV_IMS_IPSEC_ENABLED == RV_YES) || (defined RV_SIP_IPSEC_NEG_ONLY)
	if (eSecurityType == RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP)
	{
		return RV_TRUE;
	}
#endif /* #if (RV_IMS_IPSEC_ENABLED == RV_YES) */
	
	return RV_FALSE;
}

/***************************************************************************
 * SecAgreeUtilsIsIpsecNegOnly
 * ------------------------------------------------------------------------
 * General: Checks whether we are configured to work with ipsec-3gpp in 
 *          security-agreement only, and not in security-object
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 ***************************************************************************/
RvBool RVCALLCONV SecAgreeUtilsIsIpsecNegOnly()
{
#ifdef RV_SIP_IPSEC_NEG_ONLY
	return RV_TRUE;
#else
	return RV_FALSE;
#endif
}

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
									IN RvSipSecurityMechanismType  eSecurityType)
{
	if (eSecurityType == RVSIP_SECURITY_MECHANISM_TYPE_TLS ||
		(eSecurityType == RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP && SecAgreeUtilsIsIpsecNegOnly() == RV_FALSE))
	{
		return RV_TRUE;
	}

	return RV_FALSE;
}

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
RvBool RVCALLCONV SecAgreeUtilsIsMethodForSecAgree(IN     RvSipMsgHandle      hMsg)
{
	RvSipMethodType       eMethodType;
	RvSipCSeqHeaderHandle hCSeqHeader;
	
	if (RvSipMsgGetMsgType(hMsg) == RVSIP_MSG_REQUEST)
	{
		/* Get method type */
		eMethodType = RvSipMsgGetRequestMethod(hMsg);
	}
	else /* RVSIP_MSG_RESPONSE */
	{
		/* Get method type */
		hCSeqHeader = RvSipMsgGetCSeqHeader(hMsg);
		eMethodType = RvSipCSeqHeaderGetMethodType(hCSeqHeader);
	}
	
	/* RFC3329 instructs not to insert security information to ACK and CANCEL requests */
	if (eMethodType == RVSIP_METHOD_ACK ||
		eMethodType == RVSIP_METHOD_CANCEL)
	{
		return RV_FALSE;
	}

	return RV_TRUE;
}

/***************************************************************************
 * SecAgreeUtilsGetCSeqStep
 * ------------------------------------------------------------------------
 * General: Retrieves the CSeq step from a message
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hMsg             - Handle to the message
 ***************************************************************************/
RvUint32 RVCALLCONV SecAgreeUtilsGetCSeqStep(IN     RvSipMsgHandle      hMsg)
{
	RvSipCSeqHeaderHandle hCSeqHeader;
	RvUint32              cseqStep;
	
	hCSeqHeader = RvSipMsgGetCSeqHeader(hMsg);
#ifdef RV_SIP_UNSIGNED_CSEQ
	RvSipCSeqHeaderGetStep(hCSeqHeader, &cseqStep);
#else /* #ifdef RV_SIP_UNSIGNED_CSEQ */
	cseqStep = (RvUint32)RvSipCSeqHeaderGetStep(hCSeqHeader);
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */

	return cseqStep;
}

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
								IN   RvSipSecurityHeaderHandle     hSecurityHeader)
{
	/* Algorithm */
	return RvSipSecurityHeaderGetAlgorithmType(hSecurityHeader);
}

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
								IN   RvSipSecurityHeaderHandle      hSecurityHeader)
{
	RvSipSecurityEncryptAlgorithmType eEAlg;

	/* Encryption Algorithm */
	eEAlg = RvSipSecurityHeaderGetEncryptAlgorithmType(hSecurityHeader);
	if (eEAlg == RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_UNDEFINED)
	{
		/* Set default value to encryption algorithm */
		eEAlg = RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_NULL;
	}
	
	return eEAlg;
}

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
								IN   RvSipSecurityHeaderHandle      hSecurityHeader)
{
	RvSipSecurityModeType eMode;

	eMode = RvSipSecurityHeaderGetModeType(hSecurityHeader);
	if (eMode == RVSIP_SECURITY_MODE_TYPE_UNDEFINED)
	{
		/* Set default value to mode */
		eMode = RVSIP_SECURITY_MODE_TYPE_TRANS;
	}

	return eMode;
}

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
								IN   RvSipSecurityHeaderHandle      hSecurityHeader)
{
	RvSipSecurityProtocolType eProt;

	eProt = RvSipSecurityHeaderGetProtocolType(hSecurityHeader);
	if (eProt == RVSIP_SECURITY_PROTOCOL_TYPE_UNDEFINED)
	{
		/* Set default value to protocol */
		eProt = RVSIP_SECURITY_PROTOCOL_TYPE_ESP;
	}

	return eProt;
}

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
									 IN     RvUint8*     pSourceCK)
{
	RvInt32   length = RVSIP_SEC_MAX_KEY_LENGTH/8;
	RvInt32   index;

	for (index = 0; index < length; index++)
	{
		pDestIK[index] = pSourceIK[index];
		pDestCK[index] = pSourceCK[index];
	}
}

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
											OUT   RvInt32*                   pPrefDecPart)
{
	RvInt32   intPart = UNDEFINED;
	RvInt32   decPart = UNDEFINED;
	RvInt32   otherIntPart = UNDEFINED;
	RvInt32   otherDecPart = UNDEFINED;
	RvStatus  rv;

	if (NULL == pIntPart || NULL == pDecPart)
	{
		/* If the caller of this function did not want to follow q-values, we handle them locally */
		pIntPart = &intPart;
		pDecPart = &decPart;
	}
	if (NULL == pOtherIntPart || NULL == pOtherDecPart)
	{
		/* If the caller of this function did not want to follow q-values, we handle them locally */
		pOtherIntPart = &otherIntPart;
		pOtherDecPart = &otherDecPart;
	}

	/* parse the q value into integer and decimal part */
	if (*pIntPart == UNDEFINED || *pDecPart == UNDEFINED)
	{
		rv = SecAgreeUtilsGetQVal(hSecHeader, pIntPart, pDecPart);
		if (RV_OK != rv)
		{
			return rv;
		}
	}

	/* parse the other q value into integer and decimal part */
	if (*pOtherIntPart == UNDEFINED || *pOtherDecPart == UNDEFINED)
	{
		rv = SecAgreeUtilsGetQVal(hSecHeader, pOtherIntPart, pOtherDecPart);
		if (RV_OK != rv)
		{
			return rv;
		}
	}

	/* Find the most preferable one */
	if (RV_TRUE == SecAgreeUtilsIsLargerQ(*pIntPart, *pDecPart, *pOtherIntPart, *pOtherDecPart))
	{
		*phPrefSecHeader = hOtherSecHeader;
		*pPrefIntPart    = *pOtherIntPart;
		*pPrefDecPart    = *pOtherDecPart;
	}
	else
	{
		*phPrefSecHeader = hSecHeader;
		*pPrefIntPart    = *pIntPart;
		*pPrefDecPart    = *pDecPart;
	}
	
	return RV_OK;
}

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
										 OUT   RvInt32*                   pDecPart)
{
/* 0 <= q <= 1 with at most 3 decimal digits */
#define Q_LEN 12 
	RvChar    qVal[Q_LEN]; 
	RvUint    len; 
	RvStatus  rv;

	/* Get the length of the q value */
	len = RvSipSecurityHeaderGetStringLength(hSecHeader, RVSIP_SECURITY_PREFERENCE);
	if (len == 0)
	{
		/* There is no preference for the header */
		*pIntPart = UNDEFINED;
		*pDecPart = UNDEFINED;
		return RV_OK;
	}

	if (len > Q_LEN)
	{
		/* The length of the q value is not supported by our parser and hence by the security-agreement */
		return RV_ERROR_UNKNOWN;
	}

	/* Get the q value */
	rv = RvSipSecurityHeaderGetStrPreference(hSecHeader, qVal, Q_LEN, &len);
	if (RV_OK != rv)
	{
		return rv;
	}

	/* Transform the string q values into decimal numbers */
	rv = SipMsgUtilsParseQValue(NULL, NULL_PAGE, UNDEFINED, qVal, pIntPart, pDecPart);
	if (RV_OK != rv)
	{
		return rv;
	}
		
	return RV_OK;
}

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
									 	   OUT RvSipSecurityHeaderHandle*        phSecurityHeader)
{
	RvStatus							rv = RV_OK;
	RvInt								elementType;
	RvSipSecurityHeaderHandle			hSecurityHeader;
	RvSipSecurityHeaderHandle			hBestSecurityHeader = NULL;
	RvSipCommonListElemHandle			hCurrElem;
	RvSipCommonListElemHandle			hNextElem;
	RvSipSecurityAlgorithmType			eCmpIpsecAlg;
	RvSipSecurityEncryptAlgorithmType	eCmpIpsecEAlg;
	RvSipSecurityProtocolType           eCmpIpsecProt;
	RvSipSecurityModeType               eCmpIpsecMode;
	RvInt32								bestIntPart  = UNDEFINED;
	RvInt32								bestDecPart  = UNDEFINED;
	RvBool								bSkipHeader  = RV_FALSE;
	RvInt								safeCounter  = 0;

	rv = RvSipCommonListGetElem(hList, RVSIP_FIRST_ELEMENT, NULL,
						        &elementType, (void**)&hSecurityHeader, &hCurrElem);
	while (rv == RV_OK && hCurrElem != NULL && safeCounter < SEC_AGREE_MAX_LOOP(hList))
	{
		if (eMechType == RvSipSecurityHeaderGetMechanismType(hSecurityHeader))
		{
			bSkipHeader = RV_FALSE;

			/* For ipsec we check that the algorithms are the equivalent as well */
			if (eMechType == RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP)
			{
				/* The algorithm, e-algorithm, mode and protocol must be equivalen */
				eCmpIpsecAlg  = SecAgreeUtilsSecurityHeaderGetAlgo(hSecurityHeader);
				eCmpIpsecEAlg = SecAgreeUtilsSecurityHeaderGetEAlgo(hSecurityHeader);
				eCmpIpsecProt = SecAgreeUtilsSecurityHeaderGetProtocol(hSecurityHeader);
				eCmpIpsecMode = SecAgreeUtilsSecurityHeaderGetMode(hSecurityHeader);
				
				if (eIpsecAlg != RVSIP_SECURITY_ALGORITHM_TYPE_UNDEFINED &&
					eIpsecAlg != eCmpIpsecAlg)
				{
					bSkipHeader = RV_TRUE;
				}
				if (eIpsecEAlg != RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_UNDEFINED &&
					eIpsecEAlg != eCmpIpsecEAlg)
				{
					bSkipHeader = RV_TRUE;
				}
				if (eIpsecMode != RVSIP_SECURITY_MODE_TYPE_UNDEFINED &&
					eIpsecMode != eCmpIpsecMode)
				{
					bSkipHeader = RV_TRUE;
				}
				if (eIpsecProtocol != RVSIP_SECURITY_PROTOCOL_TYPE_UNDEFINED &&
					eIpsecProtocol != eCmpIpsecProt)
				{
					bSkipHeader = RV_TRUE;
				}
			}

			if (bSkipHeader == RV_FALSE)
			{
				if (hBestSecurityHeader == NULL)
				{
					hBestSecurityHeader = hSecurityHeader;
				}
				else
				{
					rv = SecAgreeUtilsGetBetterHeader(hBestSecurityHeader, hSecurityHeader, 
													  &bestIntPart, &bestDecPart, NULL, NULL, 
													  &hBestSecurityHeader, &bestIntPart, &bestDecPart);
					if (RV_OK != rv)
					{
						break;
					}
				}
			}
		}

		rv = RvSipCommonListGetElem(hList, RVSIP_NEXT_ELEMENT, hCurrElem,
									&elementType, (void**)&hSecurityHeader, &hNextElem);
		hCurrElem = hNextElem;

		safeCounter += 1;
	}

	*phSecurityHeader = hBestSecurityHeader;

	return rv;
}

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
									IN RvInt32              isLargerDecPart)
{
	if (isLargerIntPart > intPart)
	{
		return RV_TRUE;
	}

	if (isLargerIntPart < intPart)
	{
		return RV_FALSE;
	}
	
	/* int parts equal */

	if (isLargerDecPart > decPart)
	{
		return RV_TRUE;
	}

	return RV_FALSE;
}

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
											  OUT RvBool                      *pbIsSingleMech)
{
	RvStatus                  rv = RV_OK;
	RvInt                     elementType;
	RvSipSecurityHeaderHandle hSecurityHeader;
	RvSipSecurityHeaderHandle hNextSecurityHeader;
	RvSipCommonListElemHandle hCurrElem;
	RvSipCommonListElemHandle hNextElem;
	RvInt                     safeCounter = 0;

	if (pbIsMech != NULL)
	{
		*pbIsMech = RV_FALSE;
	}
	if (phSecurityHeader != NULL)
	{
		*phSecurityHeader = NULL;
	}
	if (pbIsSingleMech != NULL)
	{
		*pbIsSingleMech = RV_FALSE;
	}
	
	if (NULL == hList)
	{
		return RV_OK;
	}
	
	if (eMechType == RVSIP_SECURITY_MECHANISM_TYPE_OTHER)
	{
		/* we do not monitor unsupported security mechanisms */
		return RV_OK;
	}

	if (pbIsSingleMech != NULL)
	{
		*pbIsSingleMech = RV_TRUE;
	}
	
	rv = RvSipCommonListGetElem(hList, RVSIP_FIRST_ELEMENT, NULL,
						        &elementType, (void**)&hSecurityHeader, &hCurrElem);
	while (rv == RV_OK && hSecurityHeader != NULL && safeCounter < SEC_AGREE_MAX_LOOP(hList))
	{
		rv = RvSipCommonListGetElem(hList, RVSIP_NEXT_ELEMENT, hCurrElem,
									&elementType, (void**)&hNextSecurityHeader, &hNextElem);
		
		if (eMechType == RvSipSecurityHeaderGetMechanismType(hSecurityHeader))
		{
			if (pbIsMech != NULL)
			{
				*pbIsMech = RV_TRUE;
			}
			if (phSecurityHeader != NULL)
			{
				*phSecurityHeader = hSecurityHeader;
			}
			if (pbIsSingleMech != NULL && hNextSecurityHeader != NULL)
			{
				*pbIsSingleMech = RV_FALSE;
			}
			break;
		}

		hSecurityHeader = hNextSecurityHeader;
		hCurrElem = hNextElem;

		if (pbIsSingleMech != NULL)
		{
			*pbIsSingleMech = RV_FALSE;
		}
		
		safeCounter += 1;
	}

	return rv;
}

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
											 OUT RvBool                      *pbIsLegal)
{
	RvSipSecurityHeaderHandle         hSecurityHeader;
	RvSipSecurityAlgorithmType        eIpsecAlg;
	RvSipSecurityProtocolType         eProtocol;
	RvSipSecurityModeType             eMode;
	RvStatus					      rv = RV_OK;

	*pbIsLegal = RV_TRUE;

	rv = SecAgreeUtilsIsMechanismInList(hList, RVSIP_SECURITY_MECHANISM_TYPE_IPSEC_3GPP, NULL, &hSecurityHeader, NULL);
	if (RV_OK != rv || hSecurityHeader == NULL)
	{
		/* All limitations are for ipsec */
		return rv;	
	}

	eIpsecAlg = SecAgreeUtilsSecurityHeaderGetAlgo(hSecurityHeader);
	if (eIpsecAlg == RVSIP_SECURITY_ALGORITHM_TYPE_UNDEFINED)
	{
		/* Algorithm must be determined. */
		*pbIsLegal = RV_FALSE;
	}

	eIpsecAlg = SecAgreeUtilsSecurityHeaderGetEAlgo(hSecurityHeader);
	if (eIpsecAlg == RVSIP_SECURITY_ENCRYPT_ALGORITHM_TYPE_UNDEFINED)
	{
		/* Algorithm must be determined. */
		*pbIsLegal = RV_FALSE;
	}

	eProtocol = SecAgreeUtilsSecurityHeaderGetProtocol(hSecurityHeader);
	if (eProtocol != RVSIP_SECURITY_PROTOCOL_TYPE_ESP)
	{
		/* Protocol must be esp */
		*pbIsLegal = RV_FALSE;
	}

	eMode = SecAgreeUtilsSecurityHeaderGetMode(hSecurityHeader);
	if (eMode != RVSIP_SECURITY_MODE_TYPE_TRANS)
	{
		/* Mode must be trans */
		*pbIsLegal = RV_FALSE;
	}

	return RV_OK;
}

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
RvBool RVCALLCONV SecAgreeUtilsIsGmInterfaceSupport(IN  SecAgree       *pSecAgree)
{
	if (pSecAgree->eSpecialStandard == RVSIP_SEC_AGREE_SPECIAL_STANDARD_TISPAN_283003 ||
		pSecAgree->eSpecialStandard == RVSIP_SEC_AGREE_SPECIAL_STANDARD_3GPP_24229 ||
		pSecAgree->eSpecialStandard == RVSIP_SEC_AGREE_SPECIAL_STANDARD_PKT_2)
	{
		return RV_TRUE;
	}

	return RV_FALSE;
}

/*------------------------------------------------------------------------
           S T A T I C   F U N C T I O N S
 -------------------------------------------------------------------------*/



#endif /* #ifdef RV_SIP_IMS_ON */

