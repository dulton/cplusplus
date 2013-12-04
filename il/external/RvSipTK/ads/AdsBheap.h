/*******************************************************************************************************************

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

************************************************************************************************************************/

/********************************************************************************************
 *                                bheap.h
 *
 * Binary Heap package
 *
 * This package is responsible for managing binary heaps.
 * Each element holds a pointer to the actual value, and nothing more.
 * On each setting of element position, an update function is called to allow users access to
 * elements which are not at the top of the heap.
 *
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *     Tsahi Levent-Levi                  11-May-2000
 *
 ********************************************************************************************/




#ifndef RV_BHEAP_H
#define RV_BHEAP_H

#ifdef __cplusplus
extern "C" {
#endif



#include "RV_ADS_DEF.h"
#include "rvlog.h"

/* Binary heap instance handle */
RV_DECLARE_HANDLE(HBHEAP);

/* Binary heap element pointer declaration */
typedef void* BHEAP_Node;



/********************************************************************************************
 * BHEAP_COMPARE_FUNC
 * purpose : Comparison function used
 * input   : elem1          - First element
 *           elem2          - Second element
 *           context        - Context to use. This value is given by the user when calling
 *                            BHEAP_Construct
 * output  : none
 * return  : Value Relationship of elem1 to elem2
 *           < 0 elem1 less than elem2
 *             0 elem1 identical to elem2
 *           > 0 elem1 greater than elem2
 ********************************************************************************************/
typedef int (*BHEAP_COMPARE_FUNC)(IN BHEAP_Node elem1,
                                  IN BHEAP_Node elem2,
                                  IN void*      context);


/********************************************************************************************
 * BHEAP_UPDATE_FUNC
 * purpose : Update function to use. This function is called whenever the ID of an element
 *           is changed inside the heap.
 * input   : elem           - Updated element
 *           newId          - New ID for the element
 * output  : none
 * return  : none
 ********************************************************************************************/
typedef void (*BHEAP_UPDATE_FUNC)(IN BHEAP_Node elem,
                                  IN RvInt32   newId);







/********************************************************************************************
 * BHEAP_Construct
 * purpose : Construct a binary heap structure
 * input   : numOfElements  - Number of elements to use for the heap
 *           logHandle      - LOG instance handle to use
 *           compareFunc    - Comparison function to use on elements
 *           updateFunc     - Update function to use
 *           context        - Context used on every call to the comparison function
 * output  : none
 * return  : BHEAP instance handle
 *           NULL if failed.
 ********************************************************************************************/
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
#define BHEAP_Construct(numOfElements,logHandle,compareFunc,updateFunc,context) BHEAP_Construct_trace(numOfElements,logHandle,compareFunc,updateFunc,context,__FILE__,__LINE__)

HBHEAP RVAPI RVCALLCONV BHEAP_Construct_trace(IN RvInt32             numOfElements,
										IN RvLogMgr	 	        *pLogMgr,
                                        IN BHEAP_COMPARE_FUNC   compareFunc,
                                        IN BHEAP_UPDATE_FUNC    updateFunc,
                                        IN void*                context,
                                        const char* file, int line);
#else
HBHEAP RVAPI RVCALLCONV BHEAP_Construct(IN RvInt32             numOfElements,
										IN RvLogMgr	 	        *pLogMgr,
                                        IN BHEAP_COMPARE_FUNC   compareFunc,
                                        IN BHEAP_UPDATE_FUNC    updateFunc,
                                        IN void*                context);
#endif
/* SPIRENT_END */


/********************************************************************************************
 * BHEAP_Destruct
 * purpose : Destruct a binary heap structure
 * input   : heapH  - Binary heap handle
 * output  : none
 * return  : none
 ********************************************************************************************/
void RVAPI RVCALLCONV BHEAP_Destruct(IN HBHEAP  heapH);


/********************************************************************************************
 * BHEAP_Clear
 * purpose : Clear the binary heap from any previous elements
 * input   : heapH  - Binary heap handle
 * output  : none
 * return  : none
 ********************************************************************************************/
void RVAPI RVCALLCONV BHEAP_Clear(IN HBHEAP heapH);


/********************************************************************************************
 * BHEAP_Insert
 * purpose : Insert an element into the binary heap
 *           o(log(n))
 * input   : heapH  - Binary heap to use
 *           data   - Actual information to insert
 * output  : none
 * return  : RV_OK on success
 *           RV_ERROR_OUTOFRESOURCES on failure
 ********************************************************************************************/
RvStatus RVAPI RVCALLCONV BHEAP_Insert(IN HBHEAP       heapH,
                                        IN BHEAP_Node   data);


/********************************************************************************************
 * BHEAP_ExtractTop
 * purpose : Return the head of the binary heap
 *           o(log(n))
 * input   : heapH - Binary heap to extract from
 * output  : none
 * return  : Node information
 *           NULL on failure
 ********************************************************************************************/
BHEAP_Node RVAPI RVCALLCONV BHEAP_ExtractTop(IN  HBHEAP      heapH);


/********************************************************************************************
 * BHEAP_PeekTop
 * purpose : Check the value at the top of the heap, without extracting it
 *           o(1)
 * input   : heapH  - Binary heap to extract from
 * output  : none
 * return  : Node information
 *           NULL on failure
 ********************************************************************************************/
BHEAP_Node RVAPI RVCALLCONV BHEAP_PeekTop(IN  HBHEAP      heapH);



/********************************************************************************************
 * BHEAP_DeleteNode
 * purpose : Delete a node for a binary heap
 *           o(log(n))
 * input   : heapH  - Binary heap handle
 *           nodeId - Node in heap to delete
 * output  : none
 * return  : RV_OK on success
 *           RV_ERROR_UNKNOWN on failure
 ********************************************************************************************/
RvStatus RVAPI RVCALLCONV BHEAP_DeleteNode (IN HBHEAP      heapH,
                                             IN RvInt32    nodeId);


/********************************************************************************************
 * BHEAP_UpdateNode
 * purpose : Update the contents of a node inside a binary heap
 *           This function will conduct a HEAPIFY call and should be called before any other
 *           changes are done on the heap.
 *           o(log(n))
 * input   : heapH  - Binary heap handle
 *           nodeId - Node in heap to update
 *           data   - Actual information to update
 * output  : none
 * return  : RV_OK on success
 *           RV_ERROR_UNKNOWN on failure
 ********************************************************************************************/
RvStatus RVAPI RVCALLCONV BHEAP_UpdateNode (IN HBHEAP       heapH,
                                             IN RvInt32     nodeId,
                                             IN BHEAP_Node   data);


/********************************************************************************************
 * BHEAP_GetNode
 * purpose : Return a node by its handle.
 *           o(log(1))
 * input   : heapH  - Binary heap handle
 *           id     - Node in heap
 * output  : none
 * return  : BHEAP_Node for the ID
 *           NULL on failure
 ********************************************************************************************/
BHEAP_Node RVAPI RVCALLCONV BHEAP_GetNode(IN HBHEAP     heapH,
                                          IN RvInt32   id);



#ifdef __cplusplus
}
#endif

#endif
