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
 *                              <SecAgreeSecurityData.h>
 *
 * This file defines the security-agreement-data object, attributes and interface functions.
 * The security-agreement-data object is used to encapsulate all security-agreement data,
 * negotiated as defined in RFC 3329.
 *
 *    Author                         Date
 *    ------                        ------
 *    Tamar Barzuza                 Jan 2006
 *********************************************************************************/

#ifndef SEC_AGREE_DATA_H
#define SEC_AGREE_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#ifdef RV_SIP_IMS_ON

#include "SecAgreeTypes.h"
#include "RvSipSecurityHeader.h"

/*-----------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/* SecAgreeSecurityDataListType
 * --------------------------------------------------------------------------
 * Indicates the type of list to refer to at the security-data structure:
 * SEC_AGREE_SEC_DATA_LIST_LOCAL  - The local list of security mechanisms
 * SEC_AGREE_SEC_DATA_LIST_REMOTE - The remote list of security mechanisms
 * SEC_AGREE_SEC_DATA_LIST_TISPAN_LOCAL - The local list that is copied from
 *                                        previous security-agreement via
 *										  RvSipSecAgreeCopy() and is compared
 *										  according to TISPAN with the received
 *                                        Security-Verify list
 */
typedef enum
{
	SEC_AGREE_SEC_DATA_LIST_UNDEFINED = -1,
	SEC_AGREE_SEC_DATA_LIST_LOCAL,
	SEC_AGREE_SEC_DATA_LIST_REMOTE,
	SEC_AGREE_SEC_DATA_LIST_TISPAN_LOCAL

} SecAgreeSecurityDataListType;

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/***************************************************************************
 * SecAgreeSecurityDataInitialize
 * ------------------------------------------------------------------------
 * General: Initialize a new Security-Agreement-Data object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr  - Pointer to the Security-Mgr
 *         pSecAgreeSecurityData - Pointer to the new Security-Agreement-Data
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeSecurityDataInitialize(
									IN    SecAgreeMgr*    pSecAgreeMgr,
									INOUT SecAgreeSecurityData*   pSecAgreeSecurityData);

/***************************************************************************
 * SecAgreeSecurityDataDestruct
 * ------------------------------------------------------------------------
 * General: Destructs a Security-Agreement-Data object.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pSecAgreeMgr  - Pointer to the security-agreement manager
 *            pSecAgreeSecurityData - Pointer to the Security-Agreement-Data to terminate.
 *            eRole         - Indication whether this is client or server security
 *                            agreement
 ***************************************************************************/
void RVCALLCONV SecAgreeSecurityDataDestruct(
                                 IN SecAgreeMgr*       pSecAgreeMgr,
								 IN SecAgreeSecurityData*      pSecAgreeSecurityData,
								 IN RvSipSecAgreeRole  eRole);

/***************************************************************************
 * SecAgreeSecurityDataCopy
 * ------------------------------------------------------------------------
 * General: Copies local information from source security-agreement data to
 *          destination security-agreement data. The fields being copied here are:
 *          1. Local security list.
 *          2. Auth header for server security-agreement only.
 *          Notice: Not all fields are being copied here. For example, the
 *          remote security list is not copied.
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr        - Pointer to the security-agreement manager
 *         pSourceSecAgreeSecurityData - Pointer to the source security-agreement-data
 *         pDestSecAgreeSecurityData   - Pointer to the destination security-agreement-data
 *         eRole               - The role of this security-agreement
 *         hPage               - The memory page to be used by the destination
 *                               security-agreement-data
 *         pbResetRemote       - Indicates in case of failure whether the remote list should be 
 *                               reset
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeSecurityDataCopy(
									 IN    SecAgreeMgr*						pSecAgreeMgr,
                                     IN    SecAgree*						pSourceSecAgree,
									 IN    SecAgreeSecurityData*			pSourceSecAgreeSecurityData,
									 IN    SecAgreeSecurityData*			pDestSecAgreeSecurityData,
									 IN    RvSipSecAgreeRole				eRole,
									 IN    HPAGE							hPage,
									 OUT   RvBool						   *pbResetRemote);

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
RvStatus RVCALLCONV SecAgreeSecurityDataSetPortC(
									 IN    SecAgreeSecurityData*			pSecAgreeSecurityData,
									 IN    int portC);
#endif
/* SPIRENT_END */

/***************************************************************************
 * SecAgreeSecurityDataResetData
 * ------------------------------------------------------------------------
 * General: Resets the security data structure in case of failure in the 
 *          middle of copy
 *
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecurityData - The security-data to reset
 *         eRole         - The role of this security-agreement
 *         bIsLocal      - Indication whether to reset local data or remote
 ***************************************************************************/
void RVCALLCONV SecAgreeSecurityDataResetData(
									 IN    SecAgreeSecurityData*		pSecurityData,
									 IN    RvSipSecAgreeRole			eRole,
									 IN    SecAgreeSecurityDataListType eListType);

/***************************************************************************
 * SecAgreeSecurityDataIsLocalSecObj
 * ------------------------------------------------------------------------
 * General: Indicates whether the local list requires the creation of a 
 *          security object, i.e. contains ipsec or tls.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr    - Pointer to the Security-Mgr
 * Output: pbIsLocalSecObj - Boolean indication to whether there should be a 
 *                           security object
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeSecurityDataIsLocalSecObj(
							 IN    SecAgreeSecurityData*    pSecAgreeSecurityData,
							 OUT   RvBool*                  pbIsLocalSecObj);

/***************************************************************************
 * SecAgreeSecurityDataConstructSecurityList
 * ------------------------------------------------------------------------
 * General: Constructs a security list of this Security-Agreement-Data object
 *          according to the list type given in eListType
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr  - Pointer to the Security-Mgr
 *         hPage         - Handle to the memory page to construct the list on
 *         pSecAgreeSecurityData - Pointer to the Security-Agreement-Data
 *         eListType     - The type of the list to construct: local, remote
 *                         or local for TISPAN support
 * Output: phList        - Pointer to the newly constructed list within the
 *                         security-data structure
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeSecurityDataConstructSecurityList(
							 IN    SecAgreeMgr*					 pSecAgreeMgr,
							 IN    HPAGE						 hPage,
                             IN    SecAgreeSecurityData*		 pSecAgreeSecurityData,
							 IN    SecAgreeSecurityDataListType  eListType,
							 OUT   RvSipCommonListHandle        *phList);

/***************************************************************************
 * SecAgreeSecurityDataDestructSecurityList
 * ------------------------------------------------------------------------
 * General: Destructs a security list of this Security-Agreement-Data object
 *          according to the list type given in eListType
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeSecurityData - Pointer to the Security-Agreement-Data
 *         eListType			 - The type of the list to construct: local, remote
 *								   or local for TISPAN support
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeSecurityDataDestructSecurityList(
										IN SecAgreeSecurityData*		pSecAgreeSecurityData,
										IN SecAgreeSecurityDataListType eListType);

/***************************************************************************
 * SecAgreeSecurityDataDestructAuthData
 * ------------------------------------------------------------------------
 * General: Destructs the local authentication data
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr  - Pointer to the security-agreement manager
 *         pSecAgreeSecurityData - Pointer to the Security-Agreement-Data
 *         eRole         - The role of the security-agreement
 ***************************************************************************/
void RVCALLCONV SecAgreeSecurityDataDestructAuthData(
										IN SecAgreeMgr*      pSecAgreeMgr,
									    IN SecAgreeSecurityData*     pSecAgreeSecurityData,
										IN RvSipSecAgreeRole eRole);

/***************************************************************************
 * SecAgreeSecurityDataPushSecurityHeader
 * ------------------------------------------------------------------------
 * General: Push Security header to the list of security headers
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr        - Pointer to the Security-Mgr
 *         hPage               - The memory page to construct the header on (if needed)
 *         pSecAgreeSecurityData       - Pointer to the Security-Agreement-Data
 *         bIsLocal            - RV_TRUE if the header should be pushed to local security
 *                               list and RV_FALSE if it should be pushed to remote security
 *                               list.
 *         hSecurityHeader     - The security header to push
 *         eLocation		   - Inserted element location (first, last, etc).
 *         hPos				   - Current list position, relevant in case that location is next or prev.
 *         eListType           - Indicates the type of list to push the security mechanism
 *                               to: local, remote or local that is specific for TISPAN comparison
 * Inout:  pNewPos             - The location of the object that was pushed.
 * Output: phNewSecurityHeader - The new security header that was actually pushed to the
 *                               list (if relevant).
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeSecurityDataPushSecurityHeader(
									IN     SecAgreeMgr*					pSecAgreeMgr,
									IN     HPAGE						hPage,
									IN     SecAgreeSecurityData*		pSecAgreeSecurityData,
									IN     RvSipSecurityHeaderHandle	hSecurityHeader,
									IN     RvSipListLocation			eLocation,
									IN     RvSipCommonListElemHandle	hPos,
									IN     SecAgreeSecurityDataListType	eListType,
									INOUT  RvSipCommonListElemHandle*	phNewPos,
									OUT    RvSipSecurityHeaderHandle*	phNewSecurityHeader);

/***************************************************************************
 * SecAgreeSecurityDataSetProxyAuth
 * ------------------------------------------------------------------------
 * General: Set Proxy-Authenticate header to the server authentication info.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSecAgreeMgr        - Pointer to the Security-Mgr
 *         hPage               - The memory page to construct the header on (if needed)
 *         pSecAgreeSecurityData       - Pointer to the Security-Agreement-Data
 *         hProxyAuthHeader    - The Proxy-Authenticate header to push
 ***************************************************************************/
RvStatus RVCALLCONV SecAgreeSecurityDataSetProxyAuth(
									IN    SecAgreeMgr*				      pSecAgreeMgr,
									IN    HPAGE                           hPage,
									IN    SecAgreeSecurityData*				      pSecAgreeSecurityData,
									IN    RvSipAuthenticationHeaderHandle hProxyAuthHeader);

#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif

#endif /* END OF: #ifndef SEC_AGREE_DATA_H*/

