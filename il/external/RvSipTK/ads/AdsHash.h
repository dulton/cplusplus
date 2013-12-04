
/*******************************************************************************************************************

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

********************************************************************************************************************/


/*************************************************************************************************************************
 *                                              HASH.h                                                                   *
 *                                                                                                                       *
 * The HASH module contains generic implementation of open hash table for the use of other stack module that need to     *
 * use this moudle.                                                                                                      *
 * The HASH structure is implemented above the ra data structure ( as defined in ra.h ).                                 *
 * The HASH module provides its user the ability to define the hash table size , the max number of user element that     *
 * will be included in it.                                                                                               *
 * The user also defined a hash routine that will operate on several parameters/keys ( according to user definition )    *
 * to provide one key , the final hash routine ( on the final key ) is always a mod.                                     *
 *                                                                                                                       *
 *                Written by                    Version & Date                            Change                         *
 *               ------------                   ---------------                          --------                        *
 *                  Ayelet B                       NG 3.0                                                                *
 *                                                                                                                       *
 *************************************************************************************************************************/


#ifndef HASH_H
#define HASH_H


#ifdef __cplusplus
extern "C" {
#endif

#include "RV_ADS_DEF.h"
#include "AdsRa.h"
#include "rvlog.h"



/* Declaration of an handle to a hash table. */
RV_DECLARE_HANDLE ( HASH_HANDLE );


/* Prototype of the Hash function provided at construction time. The function is used in order
   to convert all keys of hash to one number that will be used in the mod operation to find
   the hash entry */
typedef RvUint  (*HASH_func)( void *Param );

/* Prototype for a compare function used to search for a given element in the hash table */
typedef RvBool (*HASH_CompareFunc) (IN void *Param1 , IN void *Param2);


#define HASH_DEFAULT_TABLE_SIZE(userElem) (userElem * 2 + 1)



/********************************************************************************************
 * Definition of RV_HASH_Resource:
 * keys     - Resource information about the keys being used
 * elements - Resource information about the elements being stored in the hash table
 ********************************************************************************************/
typedef struct
{
    RV_Resource keys;
    RV_Resource elements;
} RV_HASH_Resource;




/*************************************************************************************************************
 * HASH_Construct
 * purpose : The routine is used to build a hash table.
 *           It gets the hash table specifications ( as described in detailed in the parameter list )
 *           and using ra it allocates the memory and built the required hash table.
 *           Note - No partial allocation is done , if there is no enough memory to do all allocations
 *           nothing is allocated and a NULL pointer is returned.
 * input   : HashTableSize      - The hash table size , this parameter referes to the size of the table
 *                                and not to the number of user records in it ( actually it is the
 *                                number that will use in the mod routine ).
 *           NumOfUserElements  - The max number of user elements that this hash table should support.
 *           hfunc              - The hash function ( perfrom on key patameters before the modul function.
 *           UserElementRecSize - The size of the user element structure in bytes.
 *           UserKeyParamsSize  - The size of the user key params structure in bytes.
 *           pLogMgr       : pointer to a log manager.
 * output  : None.
 * return  : Handle to the allocated HASH table ( NULL if there was a resource allocation problem ).
 ************************************************************************************************************************/
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
#define HASH_Construct(HashTableSize,NumOfUserElements,hfunc,UserElementRecSize,UserKeyParamsSize,LogHandle,Name) HASH_Construct_trace(HashTableSize,NumOfUserElements,hfunc,UserElementRecSize,UserKeyParamsSize,LogHandle,Name,__FILE__,__LINE__)

HASH_HANDLE RVAPI RVCALLCONV HASH_Construct_trace (IN RvUint            HashTableSize,
                                             IN RvUint            NumOfUserElements,
                                             IN HASH_func          hfunc,
                                             IN RvUint            UserElementRecSize,
                                             IN RvUint            UserKeyParamsSize,
                                             IN RvLogMgr	       *pLogMgr,
                                             IN const char*        Name,
                                             const char* file, int line);
#else
HASH_HANDLE RVAPI RVCALLCONV HASH_Construct (IN RvUint        HashTableSize ,
                                             IN RvUint        NumOfUserElements,
                                             IN HASH_func      hfunc,
                                             IN RvUint        UserElementRecSize,
                                             IN RvUint        UserKeyParamsSize,
 					                	     IN RvLogMgr	  *pLogMgr,
                                             IN const char*    Name);
#endif
/* SPIRENT_END */


/***********************************************************************************************************************
 * HASH_Destruct
 * purpose : Destruct a given hash table. All memory is freed.
 * input   : hHash - Handle to the hash table that should be desructed.
 * output  : None.
 ***********************************************************************************************************************/
void RVAPI RVCALLCONV HASH_Destruct (IN HASH_HANDLE hHash);



/************************************************************************************************************************
 * HASH_GetCurrUsedElements
 * purpose : Return the number if used user elements in a hash table.
 * input   : hHash - Handle to the hash table.
 * output  : None.
 * return  : The number of used user elements.
 ************************************************************************************************************************/
/*int RVAPI RVCALLCONV HASH_GetCurrUsedElements ( HASH_HANDLE hHash );*/


/*************************************************************************************************************************
 * HASH_InsertElement
 * purpose : Add a new user element to a given hash table.
 * input   : hHash              - handle to the hash table to which the element should be entered.
 *           Params             - Pointer to the structure of the key parameters.
 *           UserElement        - Pointer to the user element that should be inserted to the hash table.
 *           bLookup            - A boolean indication, if true a search is done before the insersion
 *                                to check that the element is not already inserted to the table ( and
 *                                then nothing is done ) , If false no search is done before the insersion.
 *           CompareFunc        - In case a search before insersion is needed the user should porvide a
 *                                comparison function.
 * output  : WasInserted        - A boolean indication if the element was inserted ( or not - in case a
 *                                search was done and it is already exist in the hash table ).
 *           ElementPtr          - A pointer of the record that was allocated to this user element.
 * return  : RV_OK - In case of insersion success ( or no insersion if the elemnt is already exist ).
 *           RV_ERROR_OUTOFRESOURCES - If there is no more available user element records.
 *************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV HASH_InsertElement (IN HASH_HANDLE        hHash,
                                               IN void               *Params,
                                               IN void               *UserElement,
                                               IN RvBool            bLookup,
                                               OUT RvBool           *pbInserted,
                                               OUT void              **ElementPtr,
                                               IN HASH_CompareFunc   CompareFunc);



/************************************************************************************************************************
 * HASH_FindElement
 * purpose : The routine is used to find the location of an element in the hash table acoording to its key params.
 * input   : hHash   - Handle to the hash table.
 *           Params  - The parameters ( key parameters ) that should be used. What is done is that the hash routine that
 *                     is given in construction time is activated to get one key from the set of parameters , and the
 *                     search is done on the hash table using this key ( assuming the hash function is mod ).
 *                     Note that this key is saved along with the user elemnt record in the hash table and is used to identify
 *                     the searched record.
 *           CompareFunc - Pointer to the user function ( hash module user ) that make the compare between the hash table
 *                         element and the new params.
 * output  : ElementId - A pointer to the user element record that was found..
 * reurn   : TRUE  : If the searched element was found.
 *           FALSE : If not.
 *************************************************************************************************************************/
RvBool RVAPI RVCALLCONV HASH_FindElement (IN  HASH_HANDLE      hHash,
                                           IN  void             *Params,
                                           IN  HASH_CompareFunc CompareFunc,
                                           OUT void             **ElementPtr);



/************************************************************************************************************************
 * HASH_GetUserElement
 * purpose : Return the user element from the hash table according to the user record elemnt ID.
 * input   : hHash       - Pointer to the hash table.
 *           ElementPtr  - The position pointer of the user element that should be read.
 * output  : UserElement - The read user element
 * return  : RV_OK  - In case of success.
 *           RV_ERROR_UNKNOWN     - No such element.
 ************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV HASH_GetUserElement (IN HASH_HANDLE hHash,
                                                IN void        *ElementPtr,
                                                OUT void       *UserElement);


/************************************************************************************************************************
 * HASH_SetUserElement
 * purpose : Set a user element record to a given location/ID in a given Hash table.
 * input   : hHash       - Pointer to the hash table.
 *           ElementId   - The position pointer of the user element that should be written.
 *           UserElement - The user element that should be written.
 * output  : None.
 * return  : RV_OK  - In case of success.
 *           RV_ERROR_UNKNOWN     - No such element.
 ************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV HASH_SetUserElement (IN HASH_HANDLE hHash,
                                                IN void        *ElementPtr,
                                                IN void        *UserElement);


/*************************************************************************************************************************
 * HASH_RemoveElement
 * purpose : Remove an element from the hash table.
 * input   : hHash     - Pointer to the hash table.
 *           ElementPtr - The location pointer of the element that should be removed.
 * output  : None.
 * return  : RV_OK  - In case of success.
 *           RV_ERROR_UNKNOWN     - No such element.
 ************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV HASH_RemoveElement (IN HASH_HANDLE hHash,
                                               IN void        *ElementPtr);


/**************************************************************************************************************************
 * HASH_GetNumOfFreeUserElements
 * purpose : Return the number of free user elements records ( free for allocation and insersion ) for a given hash table.
 * input   : hHash - Pointer to the hash table.
 * output  : None.
 * return  : The number of free user element records.
 ************************************************************************************************************************/
/*int RVAPI RVCALLCONV HASH_GetNumOfFreeUserElements ( HASH_HANDLE hHash );*/


/**************************************************************************************************************************
 * HASH_GetTotalNumOfUserElements
 * purpose : Return the total numner of allocated user elements records (used or free).
 * input   : hHash - Pointer to the hash table.
 * output  : None.
 * return  : The number of free user element records.
 ************************************************************************************************************************/
/*int RVAPI RVCALLCONV HASH_GetTotalNumOfUserElements ( HASH_HANDLE hHash );*/


/************************************************************************************************************************
 * HASH_GetUserKeyElement
 * purpose : Return the key of user element from the hash table according to the user record
 *           elemnt pointer.
 * input   : hHash       - Pointer to the hash table.
 *           ElementPtr   - The position pointer of the user element that should be read.
 * output  : KeyUserElement - The key user element.
 * return  : RV_OK          - In case of success.
 *           RV_ERROR_BADPARAM - Element not in use.
 *           RV_ERROR_UNKNOWN             - No such element.
 ************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV HASH_GetUserKeyElement (IN HASH_HANDLE hHash,
                                                   IN void        *ElementPtr,
                                                   OUT void       *KeyUserElement);


/************************************************************************************************************************
 * HASH_GetResourcesStatus
 * purpose : Return information about the resource allocation of this HASH
 * input   : hHash      - Pointer to the array.
 * output  : resources  - Includes the following information:
 * return  : RV_OK         - In case of success.
 *           RV_ERROR_BADPARAM   - Invalid hHash handle (NULL)
 ************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV HASH_GetResourcesStatus(IN  HASH_HANDLE          hHash,
                                                   OUT RV_HASH_Resource*    resources);



#ifdef __cplusplus
}
#endif


#endif


