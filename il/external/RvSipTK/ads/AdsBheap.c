

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************************************************

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

************************************************************************************************************************/

/********************************************************************************************
 *                                bheap.c

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

/*-------------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                             */
/*-------------------------------------------------------------------------*/

#include "RV_ADS_DEF.h"
#include <stdlib.h>
#include "AdsBheap.h"
#include "rvmemory.h"

/*-------------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                               */
/*-------------------------------------------------------------------------*/

/********************************************************************************************
 * Description of BHEAP_Header
 * This structure holds the information for a HBHEAP handle
 * max      - Maximum number of elements in heap
 * cur      - Current number of elements in heap
 * log      - Log filter to use
 * compare  - Comparison function on nodes
 * update   - Update function to use
 * context  - Context to use for comparison function
 ********************************************************************************************/
typedef struct {
    RvInt32            max;
    RvInt32            cur;
    BHEAP_COMPARE_FUNC  compare;
    BHEAP_UPDATE_FUNC   update;
	RvLogMgr*           pLogMgr;
    void*               context;
} BHEAP_Header;


/********************************************************************************************
 *                                                                                          *
 *                                  Private macros                                          *
 *                                                                                          *
 ********************************************************************************************/
#define BHEAP_NODE(heap, id) \
    ((BHEAP_Node *)(((char *)(heap)) + sizeof(BHEAP_Header)))[id]

#define BHEAP_PARENT(id)    ((RvInt32)((id-1) / 2))
#define BHEAP_LEFT(id)      (((id+1) << 1) - 1)
#define BHEAP_RIGHT(id)     ((id+1) << 1)





/********************************************************************************************
 *                                                                                          *
 *                                  Private functions                                       *
 *                                                                                          *
 ********************************************************************************************/


/********************************************************************************************
 * bheap_Set
 * purpose : Set an element inside the heap and make sure the update function is called
 *           o(log(1))
 * input   : heap   - Binary heap structure
 *           id     - Id of element to set
 *           data   - Data to insert into element
 * output  : none
 * return  : none
 ********************************************************************************************/
void RVCALLCONV bheap_Set(IN BHEAP_Header*  heap,
                          IN RvInt32       id,
                          IN BHEAP_Node     data)
{
    BHEAP_NODE(heap, id) = data;

    /* Send the update callback if necessary */
    if (heap->update != NULL)
        heap->update(data, id);
}


/********************************************************************************************
 * bheap_Swap
 * purpose : Swap 2 elements
 *           o(log(1))
 * input   : heap   - Binary heap structure
 *           id1    - Id of first element to swap
 *           id2    - Id of second element to swap
 * output  : none
 * return  : none
 ********************************************************************************************/
void RVCALLCONV bheap_Swap(IN BHEAP_Header*  heap,
                           IN RvInt32       id1,
                           IN RvInt32       id2)
{
    BHEAP_Node tempNode;

    tempNode = BHEAP_NODE(heap, id1);
    bheap_Set(heap, id1, BHEAP_NODE(heap, id2));
    bheap_Set(heap, id2, tempNode);
}


/********************************************************************************************
 * bheap_Heapify
 * purpose : Heapify the heap.
 *           This function ensures that the heap is well arranged
 *           o(log(n))
 * input   : heap   - Binary heap structure
 *           id     - Element to start heapifying from
 * output  : none
 * return  : none
 ********************************************************************************************/
void RVCALLCONV bheap_Heapify(IN BHEAP_Header*  heap,
                              IN RvInt32       id)
{
    RvInt32    smallest, leftId, rightId, i;
    BHEAP_Node  node, left, right, smallestNode;

    i = id;
    node = BHEAP_NODE(heap, id);
    smallest = id;
    leftId = BHEAP_LEFT(id);

    while (leftId < heap->cur)
    {
        left = BHEAP_NODE(heap, leftId);
        rightId = BHEAP_RIGHT(i);
        right = BHEAP_NODE(heap, rightId);

        /* Find out who's the smallest node */
        if (heap->compare(left, node, heap->context) < 0)
        {
            /* Left son is smaller than current node */
            smallest = leftId;
            smallestNode = left;
        }
        else
        {
            /* Current node is smaller than left son */
            smallest = i;
            smallestNode = node;
        }
        if ((rightId < heap->cur) && (heap->compare(right, smallestNode, heap->context) < 0))
        {
            /* Right is the smallest node */
            smallest = rightId;
            smallestNode = right;
        }

        /* See if we're done */
        if (smallest == i) return;

        /* Swap and move on */
        bheap_Swap(heap, i, smallest);
        i = smallest;
        leftId = BHEAP_LEFT(i);
    }
}


/********************************************************************************************
 *                                                                                          *
 *                                  Public functions                                        *
 *                                                                                          *
 ********************************************************************************************/

/********************************************************************************************
 * BHEAP_Construct
 * purpose : Construct a binary heap structure
 * input   : numOfElements  - Number of elements to use for the heap
 *           pLogMgr        - log manager
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
HBHEAP RVAPI RVCALLCONV BHEAP_Construct_trace(IN RvInt32             numOfElements,
										IN RvLogMgr	 	        *pLogMgr,
                                        IN BHEAP_COMPARE_FUNC   compareFunc,
                                        IN BHEAP_UPDATE_FUNC    updateFunc,
                                        IN void*                context,
                                        const char* file, int line)
#else
HBHEAP RVAPI RVCALLCONV BHEAP_Construct(IN RvInt32             numOfElements,
										IN RvLogMgr	 	        *pLogMgr,
                                        IN BHEAP_COMPARE_FUNC   compareFunc,
                                        IN BHEAP_UPDATE_FUNC    updateFunc,
                                        IN void*                context)
#endif
/* SPIRENT_END */
{
    BHEAP_Header* heap;

    /* Allocate binary heap */
    if (RV_OK != RvMemoryAlloc(NULL, sizeof(BHEAP_Header) + sizeof(BHEAP_Node) * numOfElements, pLogMgr, (void*)&heap))
        return NULL;

    /* Set all parameters */
    heap->max = numOfElements;
    heap->cur = 0;
    heap->compare = compareFunc;
    heap->update = updateFunc;
    heap->context = context;
	heap->pLogMgr = pLogMgr;

    return (HBHEAP)heap;
}


/********************************************************************************************
 * BHEAP_Destruct
 * purpose : Destruct a binary heap structure
 * input   : heapH  - Binary heap handle
 * output  : none
 * return  : none
 ********************************************************************************************/
void RVAPI RVCALLCONV BHEAP_Destruct(IN HBHEAP  heapH)
 {
     BHEAP_Header* heap    = (BHEAP_Header *)heapH;
	 RvLogMgr*     pLogMgr = NULL;

     if (heap == NULL)
	 {
		 return;
	 }
	 pLogMgr = heap->pLogMgr;
     RvMemoryFree((void *)heap,pLogMgr);
 }


/********************************************************************************************
 * BHEAP_Clear
 * purpose : Clear the binary heap from any previous elements
 * input   : heapH  - Binary heap handle
 * output  : none
 * return  : none
 ********************************************************************************************/
void RVAPI RVCALLCONV BHEAP_Clear(IN HBHEAP heapH)
{
    BHEAP_Header* heap = (BHEAP_Header *)heapH;

    if (heap == NULL) return;

    heap->cur = 0;
}


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
                                        IN BHEAP_Node   data)
{
    BHEAP_Header*   heap = (BHEAP_Header *)heapH;
    BHEAP_Node      parent;
    RvInt32        i, parentId;

    /* Make sure we've got enough space */
    if (heap->cur == heap->max) return RV_ERROR_OUTOFRESOURCES;

    /* Insert this element as last in heap */
    i = heap->cur;
    bheap_Set(heap, i, data);

    /* Make sure integrity of binary heap is ok */
    while (i > 0)
    {
        parentId = BHEAP_PARENT(i);
        parent = BHEAP_NODE(heap, parentId);
        if (heap->compare(parent, data, heap->context) < 0)
            break;

        bheap_Swap(heap, i, parentId);
        i = parentId;
    }

    heap->cur++;
    return RV_OK;
}


/********************************************************************************************
 * BHEAP_ExtractTop
 * purpose : Return the head of the binary heap
 *           o(log(n))
 * input   : heapH - Binary heap to extract from
 * output  : none
 * return  : Node information
 *           NULL on failure
 ********************************************************************************************/
BHEAP_Node RVAPI RVCALLCONV BHEAP_ExtractTop(IN  HBHEAP      heapH)
{
    BHEAP_Header* heap = (BHEAP_Header *)heapH;
    BHEAP_Node top;

    /* Make sure we've got elements... */
    if (heap->cur == 0) return NULL;

    /* Extract the top */
    top = BHEAP_NODE(heap, 0);

    heap->cur--;

    /* Get the last element in heap to be the first one */
    if (heap->cur > 0)
    {
        bheap_Set(heap, 0, BHEAP_NODE(heap, heap->cur));

        /* Heapify to correct discrepancies */
        bheap_Heapify(heap, 0);
    }

    return top;
}


/********************************************************************************************
 * BHEAP_PeekTop
 * purpose : Check the value at the top of the heap, without extracting it
 *           o(1)
 * input   : heapH  - Binary heap to extract from
 * output  : none
 * return  : Node information
 *           NULL on failure
 ********************************************************************************************/
BHEAP_Node RVAPI RVCALLCONV BHEAP_PeekTop(IN  HBHEAP      heapH)
{
    BHEAP_Header* heap = (BHEAP_Header *)heapH;

    if (heap == NULL) return NULL;
    if (heap->cur == 0) return NULL;

    return BHEAP_NODE(heap, 0);
}


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
RvStatus RVAPI RVCALLCONV BHEAP_DeleteNode(IN HBHEAP   heapH,
                                            IN RvInt32 nodeId)
{
    BHEAP_Header*   heap = (BHEAP_Header *)heapH;
    RvInt32        i, parentId;


    /* Make sure node id is withing range */
    if ((nodeId < 0) || (nodeId >= heap->cur)) return RV_ERROR_UNKNOWN;

    i = nodeId;

    /* Remove the element and move all parents one level down */
    while (i > 0)
    {
        parentId = BHEAP_PARENT(i);
        bheap_Set(heap, i, BHEAP_NODE(heap, parentId));
        i = parentId;
    }

    /* Remove the head */
    if (BHEAP_ExtractTop(heapH) != NULL)
        return RV_OK;
    else
        return RV_ERROR_UNKNOWN;
}


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
                                             IN BHEAP_Node   data)
{
    BHEAP_Header* heap = (BHEAP_Header *)heapH;
    BHEAP_Node parent, tempNode;
    RvInt32 i, parentId;

    /* Make sure node id is withing range */
    if ((nodeId < 0) || (nodeId >= heap->cur)) return RV_ERROR_UNKNOWN;

    i = nodeId;
    tempNode = BHEAP_NODE(heap, i);

    /* Make sure the update didn't move the node upwards... */
    while (i > 0)
    {
        parentId = BHEAP_PARENT(i);
        parent = BHEAP_NODE(heap, parentId);

        if (heap->compare(parent, tempNode, heap->context) < 0) break;

        bheap_Swap(heap, parentId, i);
        i = parentId;
        tempNode = parent;
    }

    /* Fix integrity downwards */
    bheap_Heapify(heap, i);
    RV_UNUSED_ARG(data);
    return RV_OK;
}


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
                                          IN RvInt32   id)
{
    BHEAP_Header* heap = (BHEAP_Header *)heapH;

    if (heap == NULL) return NULL;
    if (heap->cur <= id) return NULL;

    return BHEAP_NODE(heap, id);
}



#ifdef __cplusplus
}
#endif


