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
 *                              <_SipCommonCSeq.h>
 *
 *  The file holds CSeq utils functions to be used in all stack layers.
 *    Author                         Date
 *    ------                        ------
 *********************************************************************************/
#ifndef SIP_COMMON_CSEQ_H
#define SIP_COMMON_CSEQ_H

#ifdef __cplusplus
extern "C" {
#endif
/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
	
/*-----------------------------------------------------------------------*/
/*                                 MACROS                               */
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
								  OUT SipCSeq *pDstCSeq);

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
void RVCALLCONV SipCommonCSeqReset(OUT SipCSeq   *pCSeq);

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
										 OUT SipCSeq *pCSeq);

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
									     OUT RvInt32 *pCSeqVal);

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
									   IN  RvInt32  cseqVal);

/***************************************************************************
 * SipCommonCSeqPromoteStep
 * ------------------------------------------------------------------------
 * General: Promotes the cseq value and refers it as defined
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * InOut:  pCSeq    - Pointer to the CSeq struct
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonCSeqPromoteStep(INOUT SipCSeq *pCSeq);

/***************************************************************************
 * SipCommonCSeqReduceStep
 * ------------------------------------------------------------------------
 * General: Promotes the cseq value and refers it as defined
 * Return Value: -
 * ------------------------------------------------------------------------
 * Arguments:
 * InOut:  pCSeq    - Pointer to the CSeq struct
 ***************************************************************************/
RvStatus RVCALLCONV SipCommonCSeqReduceStep(INOUT SipCSeq *pCSeq);

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
                                            IN  RvBool   bEqualToo);

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef SIP_COMMON_CSEQ_H */
