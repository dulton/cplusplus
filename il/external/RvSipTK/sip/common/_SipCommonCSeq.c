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
 *                              <_SipCommonCSeq.c>
 *
 *  The file holds CSeq utils functions to be used in all stack layers.
 *    Author                         Date
 *    ------                        ------
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipMsgTypes.h"
#include "_SipCommonTypes.h"

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * SipCommonCSeqCopy
 * ------------------------------------------------------------------------
 * General: Copies one CSeq to another
 * Return Value: - 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pSrcCSeq - Pointer to the source CSeq struct
 * Output: pDstCSeq - Pointer to the dest. CSeq struct
 ***************************************************************************/
void RVCALLCONV SipCommonCSeqCopy(IN  SipCSeq *pSrcCSeq,
								  OUT SipCSeq *pDstCSeq)
{
#ifdef RV_SIP_UNSIGNED_CSEQ
	pDstCSeq->bInitialized = pSrcCSeq->bInitialized; 
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 
	pDstCSeq->val		   = pSrcCSeq->val; 
}

/***************************************************************************
 * SipCommonCSeqReset
 * ------------------------------------------------------------------------
 * General: Resets CSeq struct
 * Return Value: - 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  bZero  - Indication if the CSeq should be initialized with zero
 *					or UNDEFINED.
 * Output: pCSeq  - Pointer to the CSeq struct
 ***************************************************************************/
void RVCALLCONV SipCommonCSeqReset(OUT SipCSeq   *pCSeq)
{
#ifdef RV_SIP_UNSIGNED_CSEQ
	pCSeq->bInitialized = RV_FALSE; 
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */
	/* The CSeq value is set in any case, otherwise we might have backwards */
	/* compatibility problem when moving from UNSIGNED_CSEQ application to  */ 
	/* a non-UNSIGNED_CSEQ (especially if the call-legs are stored and      */
	/* restored) */
	pCSeq->val = UNDEFINED; 
}

/***************************************************************************
 * SipCommonCSeqSetStep
 * ------------------------------------------------------------------------
 * General: Sets the CSeq struct a value
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  cseqVal - CSeq value
 * Output: pCSeq   - Pointer to the CSeq struct
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonCSeqSetStep(IN  RvInt32  cseqVal,
										 OUT SipCSeq *pCSeq)
{
#ifdef RV_SIP_UNSIGNED_CSEQ
	pCSeq->bInitialized = RV_TRUE; 
#else /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 
	if (cseqVal < 0)
	{
		return RV_ERROR_BADPARAM; 
	}
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 
	pCSeq->val = cseqVal; 

	return RV_OK;
}

/***************************************************************************
 * SipCommonCSeqGetStep
 * ------------------------------------------------------------------------
 * General: Gets the CSeq struct a value. This function returns a cseq 
 *			value only if it's initialized and have legal value
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCSeq    - Pointer to the CSeq struct
 * Output: pCSeqVal - CSeq value
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonCSeqGetStep(IN  SipCSeq *pCSeq,
									     OUT RvInt32 *pCSeqVal)
{
#ifdef RV_SIP_UNSIGNED_CSEQ
	if (pCSeq->bInitialized == RV_FALSE)
	{
		return RV_ERROR_UNINITIALIZED;
	}
#else /* #ifdef RV_SIP_UNSIGNED_CSEQ */
	if (pCSeq->val < 0)
	{
		return RV_ERROR_UNINITIALIZED;
	}
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */

	*pCSeqVal = pCSeq->val; 

	return RV_OK;
}

/***************************************************************************
 * SipCommonCSeqIsEqual
 * ------------------------------------------------------------------------
 * General: Compares between an integer cseq value to cseq struct
 * Return Value: RV_TRUE/RV_FALSE (Comparison result)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCSeq    - Pointer to the CSeq struct
 *		   cseqVal  - CSeq integer value to be compared
 * Output: - 
 ***************************************************************************/
RvBool RVCALLCONV SipCommonCSeqIsEqual(IN  SipCSeq *pCSeq,
									   IN  RvInt32  cseqVal)
{
	RvBool bEqual; 

#ifdef RV_SIP_UNSIGNED_CSEQ
	if (pCSeq->bInitialized == RV_FALSE)
	{
		return RV_FALSE; 
	}
#else /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 
	if (pCSeq->val < 0)
	{
		return RV_FALSE;
	}
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 
	bEqual = (pCSeq->val == cseqVal) ? RV_TRUE : RV_FALSE; 

	return bEqual; 
}

/***************************************************************************
 * SipCommonCSeqPromoteStep
 * ------------------------------------------------------------------------
 * General: Promotes the cseq value and refers it as defined
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * InOut:  pCSeq    - Pointer to the CSeq struct
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonCSeqPromoteStep(INOUT SipCSeq *pCSeq)
{
#ifdef RV_SIP_UNSIGNED_CSEQ
	if (RV_FALSE == pCSeq->bInitialized)
	{
		return RV_ERROR_UNINITIALIZED;
	}
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 
	pCSeq->val++;

	return RV_OK;
}

/***************************************************************************
 * SipCommonCSeqReduceStep
 * ------------------------------------------------------------------------
 * General: Promotes the cseq value and refers it as defined
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * InOut:  pCSeq    - Pointer to the CSeq struct
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonCSeqReduceStep(INOUT SipCSeq *pCSeq)
{
#ifdef RV_SIP_UNSIGNED_CSEQ
	if (RV_FALSE == pCSeq->bInitialized)
	{
		return RV_ERROR_UNINITIALIZED;
	}
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 
	pCSeq->val--;

	return RV_OK;
}

/***************************************************************************
 * SipCommonCSeqIsIntSmaller
 * ------------------------------------------------------------------------
 * General: Checks if the given integer cseq value is smaller than the 
 *			cseq value within the cseq struct. 
 * Return Value: RV_TRUE/RV_FALSE
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  pCSeq     - Pointer to the CSeq struct.
 *		   cseqVal   - Integer cseq value to be compared. 
 *         bEqualTo  - if TRUE: checks '<='. if FALSE: checks '<'
 * Output: -
 ***************************************************************************/
RvBool RVCALLCONV SipCommonCSeqIsIntSmaller(IN  SipCSeq *pCSeq,
											IN  RvInt32  cseqVal,
                                            IN  RvBool   bEqualToo)
{
	RvBool bSmaller; 

#ifdef RV_SIP_UNSIGNED_CSEQ
	if (pCSeq->bInitialized == RV_FALSE)
	{
		bSmaller = RV_FALSE; 
	}
	else if(RV_TRUE == bEqualToo)
	{
		bSmaller = ((RvUint32) cseqVal <= (RvUint32) pCSeq->val) ? RV_TRUE : RV_FALSE; 
	}
    else
    {
        bSmaller = ((RvUint32) cseqVal < (RvUint32) pCSeq->val) ? RV_TRUE : RV_FALSE; 
    }
#else /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 
    if(RV_TRUE == bEqualToo)
    {
        bSmaller = (cseqVal <= pCSeq->val) ? RV_TRUE : RV_FALSE; 
    }
    else
    {
        bSmaller = (cseqVal < pCSeq->val) ? RV_TRUE : RV_FALSE; 
    }
	RV_UNUSED_ARG(bEqualToo)
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 

	return bSmaller;
}

#if 0 
/***************************************************************************
 * SipCommonCSeqIsDefined
 * ------------------------------------------------------------------------
 * General: Checks if the CSeq value was set before (is defined)
 * Return Value: RV_TRUE/RV_FALSE
 * ------------------------------------------------------------------------
 * Arguments:
 * InOut:  pCSeq    - Pointer to the CSeq struct
 ***************************************************************************/
RvBool RVCALLCONV SipCommonCSeqIsDefined(IN SipCSeq *pCSeq)
{
#ifdef RV_SIP_UNSIGNED_CSEQ
	return pCSeq->bInitialized; 
#else /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 
	RvBool	bInitialized = (pCSeq->val < 0) ? RV_FALSE : RV_TRUE; 

	return bInitialized; 
#endif /* #ifdef RV_SIP_UNSIGNED_CSEQ */ 
}
#endif /* 0 */
