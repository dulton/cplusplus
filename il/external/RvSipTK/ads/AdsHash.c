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
 * use HASH functionality.                                                                                               *
 * The HASH structure is implemented above the ra data structure ( as defined in ra.h ).                           *
 * The HASH module provides its user the ability to define the hash table size , the max number of user element that     *
 * will be included in it.                                                                                               *
 * The user also defined a hash routine that will operate on several parameters/keys ( according to user definition )    *
 * to provide one key , the final hash routine ( on the final key ) is always a mod.                                     *
 *                                                                                                                       *
 *                Written by                    Version & Date                            Change                         *
 *               ------------                   ---------------                          --------                        *
 *                Ayelet Back                      NG 3.0                                                                *
 *                                                                                                                       *
 *************************************************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                             */
/*-------------------------------------------------------------------------*/
#include "RV_ADS_DEF.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "AdsRa.h"
#include "AdsHash.h"
#include "rvmemory.h"

/*-------------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                               */
/*-------------------------------------------------------------------------*/

/* Description of an HASH table
   Includes the following fields :
   NumOfUserElements  : The max number of used elements allocated for the hash table.
   UserRecSize        : Size of the user record that we want to save in bytes.
   UserParamsSize     : Size of the key params structure in bytes.
   HashTableBuffer    : Buffer for the table records of the keys.
   UserRAHandle       : Handle for the RA allocation for user data. ( the elements themselves )
   HashFunc           : Pointer to the hash function provided at construction time.
   KeysSize           : Keys resources values.
   LogRegisterId      : ID provided at registration time to the log module */

typedef struct {
    RvUint             NumOfUserElements;
    RvUint             UserRecSize;
    RvUint             UserParamsSize;
    RvUint8*            HashTableBuffer;
    HRA                 UserRAHandle;
    HASH_func           HashFunc;
    RV_Resource         KeysSize;
    RvLogSource         logSource;
	RvLogSource         *pLogSource;
} HASH_TableType;


/* The open hash is implemented using two types of records that are allocated in the ra.
   First type is the hash table entry ( which mainly contain a pointers to the list of
   user elements that are mapped to the same hash entry ).
   The second type of ra record is user record along with management record that used to manage the list of elements of
   the same type */

/* The hash tabel entry contains the following fields :
   FirstElementPtr : A pointer ( RA pointer ) of the first element in the list of elements that were mapped to this entry. */



typedef struct {
  RA_Element FirstElementPtr;
} HASH_TableEntryType;




/* This structure will be included in any user element and serve for pointing to the next and prev
   elements in the list of elements that were mapped to the same entry ( collision )
   Fields :
   Next,Prev      : pointers to the next and prev user elemnts that were mapped to the same place
                    in the hash table.
   EntryNum       : The entry num in the hash table to which the element was mapped ( collision list ) */
typedef struct HASH_ListElement_tag HASH_ListElement;
struct HASH_ListElement_tag {
  HASH_ListElement* next;
  HASH_ListElement* prev;
  int EntryNum;
};



/*-------------------------------------------------------------------------*/
/*                          MACRO DEFINITIONS                              */
/*-------------------------------------------------------------------------*/

#define HASH_ENTRY(hash,i) \
    ((HASH_TableEntryType *)( &(hash)->HashTableBuffer[sizeof(HASH_TableEntryType)*(i)] ))


/*-------------------------------------------------------------------------*/
/*                          FUNCTIONS IMPLEMENTATION                       */
/*-------------------------------------------------------------------------*/

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
HASH_HANDLE RVAPI RVCALLCONV HASH_Construct_trace (IN RvUint            HashTableSize,
                                             IN RvUint            NumOfUserElements,
                                             IN HASH_func          hfunc,
                                             IN RvUint            UserElementRecSize,
                                             IN RvUint            UserKeyParamsSize,
                                             IN RvLogMgr	       *pLogMgr,
                                             IN const char*        Name,
                                             const char* file, int line)
#else
HASH_HANDLE RVAPI RVCALLCONV HASH_Construct (IN RvUint            HashTableSize,
                                             IN RvUint            NumOfUserElements,
                                             IN HASH_func          hfunc,
                                             IN RvUint            UserElementRecSize,
                                             IN RvUint            UserKeyParamsSize,
 					                	     IN RvLogMgr	       *pLogMgr,
                                             IN const char*        Name)
#endif
/* SPIRENT_END */
{
    void                *ptr;
    HASH_TableType      *hHandle;
    RvUint              i;
    HASH_TableEntryType *HashEntry;
	RvStatus            crv;
    
    crv = RV_OK;

    if (RV_OK != RvMemoryAlloc(NULL, sizeof(HASH_TableType), pLogMgr, (void*)&ptr))
       return NULL;

    memset(ptr, 0, sizeof(HASH_TableType));

    /* save the internal hash parameters */
    hHandle = (HASH_TableType *)ptr;
    hHandle->NumOfUserElements = NumOfUserElements;
    hHandle->UserRecSize       = UserElementRecSize;
    hHandle->UserParamsSize    = UserKeyParamsSize;
    hHandle->HashFunc          = hfunc;

    hHandle->KeysSize.maxNumOfElements  = HashTableSize;
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    if(pLogMgr != NULL)
	{
		crv = RvLogSourceConstruct(pLogMgr,&hHandle->logSource,ADS_HASH,Name);
		if(crv != RV_OK)
		{
			RvMemoryFree(hHandle,pLogMgr);
			return NULL;
		}
		hHandle->pLogSource = &hHandle->logSource;
	}
	else
	{
		hHandle->pLogSource = NULL;
	}
#else
    hHandle->pLogSource = NULL;
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

    if (RV_OK != RvMemoryAlloc(NULL, sizeof(HASH_TableEntryType) * HashTableSize, NULL, (void*)&hHandle->HashTableBuffer))
    {
 	    RvLogError(hHandle->pLogSource, (hHandle->pLogSource,
		         "HASH_Construct - failed to allocate memory"));

		if(hHandle->pLogSource != NULL)
        {
            RvLogSourceDestruct(hHandle->pLogSource);
        }
        RvMemoryFree(hHandle ,NULL);
        return NULL;
    }
    
/* SPIRENT_BEGIN */
/*
 * COMMENTS:
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    if ( !(hHandle->UserRAHandle = RA_Construct_trace (
                         UserElementRecSize + UserKeyParamsSize + sizeof(HASH_ListElement),
                         NumOfUserElements,
                         NULL,
                         pLogMgr,
                         Name, file, line)))
#else
    hHandle->UserRAHandle = RA_Construct (
                         UserElementRecSize + UserKeyParamsSize + sizeof(HASH_ListElement),
                         NumOfUserElements,
                         NULL,
                         pLogMgr,
                         Name);
    if (hHandle->UserRAHandle == NULL)
#endif
/* SPIRENT_END */
    {
 	    RvLogError(hHandle->pLogSource, (hHandle->pLogSource,
		         "HASH_Construct - construct RA"));

        if(hHandle->pLogSource != NULL)
        {
            RvLogSourceDestruct(hHandle->pLogSource);
        }
        RvMemoryFree( hHandle->HashTableBuffer,NULL );
        RvMemoryFree( hHandle ,NULL);
        return NULL;
    }
    
    /* Initialize the hash table entries */
    for ( i = 0 ; i < HashTableSize ; i++ )
    {
        HashEntry = HASH_ENTRY(hHandle, i);
        HashEntry->FirstElementPtr = NULL;
    }
    
 	RvLogLeave(hHandle->pLogSource, (hHandle->pLogSource,
		     "HASH_Construct (HashTableSize=%u, NumOfUserElements=%u,UserElementRecSize=%u,UserKeyParamsSize=%u)=0x%p",
             hHandle,HashTableSize, NumOfUserElements,UserElementRecSize,UserKeyParamsSize));
    return (HASH_HANDLE)hHandle;
}



/***********************************************************************************************************************
* HASH_Destruct
* purpose : Destruct a given hash table. All memory is freed.
* input   : hHash - Handle to the hash table that should be desructed.
* output  : None.
***********************************************************************************************************************/
void RVAPI RVCALLCONV HASH_Destruct (IN HASH_HANDLE hHash)
{
    HASH_TableType *pHashTable;

    pHashTable = ( HASH_TableType *)hHash;
	if(pHashTable->pLogSource != NULL)
    {
 	    RvLogInfo(pHashTable->pLogSource, (pHashTable->pLogSource,
		         "HASH_Destruct (hHash=0x%p)",hHash));
        RvLogSourceDestruct(pHashTable->pLogSource);
    }
    RvMemoryFree ( pHashTable->HashTableBuffer ,NULL);
	RA_Destruct ( pHashTable->UserRAHandle );
    RvMemoryFree( pHashTable ,NULL);
}




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
 * output  : pbInserted        - A boolean indication if the element was inserted ( or not - in case a
 *                                search was done and it is already exist in the hash table ).
 *           ElementPtr          - Pointer to the record that was allocated to this user element.
 * return  : RV_OK - In case of insersion success ( or no insersion if the elemnt is already exist ).
 *           RV_ERROR_OUTOFRESOURCES - If there is no more available user element records.
 *************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV HASH_InsertElement (IN HASH_HANDLE        hHash,
                                               IN void               *Params,
                                               IN void               *UserElement,
                                               IN RvBool            bLookup,
                                               OUT RvBool            *pbInserted,
                                               OUT void               **ElementPtr,
                                               IN HASH_CompareFunc   CompareFunc)
{
   RvInt                Key;
   RvBool               Continue = RV_TRUE;
   HASH_TableEntryType *HashEntry;
   RA_Element          Tmp;
   HASH_ListElement    ListRec;
   HASH_ListElement    *ListRec1;
   HASH_TableType      *pHashTable;

   pHashTable = (HASH_TableType *)hHash;

   RvLogEnter(pHashTable->pLogSource, (pHashTable->pLogSource,
		     "HASH_InsertElement (hHash=0x%p,Params=0x%p,UserElement=0x%p,bLookup=%d,pbInserted=0x%p,ElementPtr=0x%p)",
             hHash,Params,UserElement,bLookup,pbInserted,ElementPtr));
   

   if (bLookup)
   {
       if ( HASH_FindElement (hHash, Params, CompareFunc, ElementPtr) )
       {
          *pbInserted = RV_FALSE;
          Continue     = RV_FALSE;
       }
   }

    if ( Continue )
    {
        /* We know here that this is a new record */
        Key = pHashTable->HashFunc( Params );
        Key = Key % pHashTable->KeysSize.maxNumOfElements;
        HashEntry = HASH_ENTRY(pHashTable, Key);

        if ( RA_Alloc ( pHashTable->UserRAHandle ,
                        (RA_Element *)ElementPtr) != RV_OK )
        {
           	RvLogError(pHashTable->pLogSource, (pHashTable->pLogSource,
                        "HASH_InsertElement - hHash=0x%p: no more user elements are available", hHash));
            return RV_ERROR_OUTOFRESOURCES;
        }

        Tmp = (RA_Element)HashEntry->FirstElementPtr;
        if (Tmp != NULL)
        {
            /* Key already in use... */
            ListRec1 = (HASH_ListElement *)Tmp;
            ListRec1->prev = (HASH_ListElement *)*ElementPtr;
        }
        else
        {
            pHashTable->KeysSize.numOfUsed++;
            if (pHashTable->KeysSize.numOfUsed > pHashTable->KeysSize.maxUsage)
                pHashTable->KeysSize.maxUsage = pHashTable->KeysSize.numOfUsed;
        }

        /* Fill the user element */
        HashEntry->FirstElementPtr = (RA_Element)*ElementPtr;
        ListRec.prev     = NULL;
        ListRec.next     = (HASH_ListElement *)Tmp;
        ListRec.EntryNum = Key;

        /* Copy to the user elemnet the following stuctures :
           - List element with the pointers to the previous and next elemnt
           - Key params that the element was inserted according to
           - User data (at last..) */
        memcpy ( *ElementPtr , &ListRec , sizeof(HASH_ListElement));
        memcpy ( &((char *)*ElementPtr)[sizeof(HASH_ListElement)] , Params , pHashTable->UserParamsSize );
        memcpy ( &((char *)*ElementPtr)[sizeof(HASH_ListElement) + pHashTable->UserParamsSize] ,
                UserElement , pHashTable->UserRecSize );

        *pbInserted = RV_TRUE;
    }
    RvLogLeave(pHashTable->pLogSource, (pHashTable->pLogSource,
                "HASH_InsertElement - hHash=0x%p: *pbInserted=%d", hHash,*pbInserted));
    return RV_OK;
}



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
 * output  : ElementId - A pointer to the user element record that was found.
 * reurn   : TRUE  : If the searched element was found.
 *           FALSE : If not.
 *************************************************************************************************************************/
RvBool RVAPI RVCALLCONV HASH_FindElement (IN  HASH_HANDLE      hHash,
                                           IN  void             *Params,
                                           IN  HASH_CompareFunc CompareFunc,
                                           OUT void             **ElementPtr)
{
    RvInt                 Key;
    HASH_TableEntryType *HashEntry;
    RA_Element          ElemPtr;
    RvBool             Found = RV_FALSE;
    HASH_ListElement    ListRec;
    HASH_TableType      *pHashTable;
    
    pHashTable = (HASH_TableType *)hHash;
    *ElementPtr = NULL;
    
    Key = pHashTable->HashFunc ( Params );
    Key = Key % pHashTable->KeysSize.maxNumOfElements;
    
    RvLogEnter(pHashTable->pLogSource, (pHashTable->pLogSource,
        "HASH_FindElement (hHash=0x%p,Params=0x%p,ElementPtr=0x%p)",
        hHash,Params,ElementPtr));
    
    HashEntry = HASH_ENTRY(pHashTable, Key);
    
    if ( HashEntry->FirstElementPtr != NULL )
    {
        /* check all the elements of the relevant hash entry */
        ElemPtr = (RA_Element) HashEntry->FirstElementPtr;
        while ( ( ElemPtr != NULL ) && (!Found))
        {
#ifdef HASH_DEBUG
            RvUint32 userDataPrefix;
            memcpy(&userDataPrefix,
                   &((char*)ElemPtr)[sizeof(ListRec) + pHashTable->UserParamsSize],
                   sizeof(RvUint32));
            RvLogDebug(pHashTable->pLogSource, (pHashTable->pLogSource,
                "HASH_FindElement(hHash=0x%p): check element with the first 4 bytes of user data 0x%p",
                hHash, userDataPrefix));
#endif /*#ifdef HASH_DEBUG*/

            if ( CompareFunc ( Params , &((char *)ElemPtr)[sizeof(ListRec)] ) )
            {
                Found = RV_TRUE;
                *ElementPtr = (void *)ElemPtr;
            }
            else
            {
                memcpy ( &ListRec , ElemPtr , sizeof(ListRec));
                ElemPtr =(RA_Element) ListRec.next;
            }
        }
    }
    RvLogLeave(pHashTable->pLogSource, (pHashTable->pLogSource,
        "HASH_FindElement (hHash=0x%p,Params=0x%p,ElementPtr=0x%p)=%d",
        hHash,Params,ElementPtr,Found));
    
    return Found;
}


/************************************************************************************************************************
 * HASH_GetUserElement
 * purpose : Return the a user element from the hash table according to the user record elemnt ID.
 * input   : hHash       - Pointer to the hash table.
 *           ElementPtr   - The position pointer of the user element that should be read.
 * output  : UserElement - The read user element
 * return  : RV_OK  - In case of success.
 *           RV_ERROR_UNKNOWN   - No such element.
 ************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV HASH_GetUserElement (IN HASH_HANDLE hHash,
                                                IN void        *ElementPtr,
                                                OUT void       *UserElement)
{
    HASH_TableType  *pHashTable;

	void            *RAElement = ElementPtr;

    pHashTable = (HASH_TableType *)hHash;

    RvLogEnter(pHashTable->pLogSource, (pHashTable->pLogSource,
        "HASH_GetUserElement (hHash=0x%p,ElementPtr=0x%p,UserElement=0x%p)",
        hHash,ElementPtr,UserElement));


    if (NULL == RAElement)
    {
        RvLogExcep(pHashTable->pLogSource, (pHashTable->pLogSource,
            "HASH_GetUserElement - (hHash=0x%p) , RAGet failed",hHash));
        return RV_ERROR_UNKNOWN;
    }

    memcpy ( UserElement ,
             &((char *)RAElement)[sizeof(HASH_ListElement) + pHashTable->UserParamsSize] ,
             pHashTable->UserRecSize );
    RvLogLeave(pHashTable->pLogSource, (pHashTable->pLogSource,
        "HASH_GetUserElement (hHash=0x%p,ElementPtr=0x%p)=%d",
        hHash,ElementPtr,RV_OK));

    return RV_OK;

}

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
                                                IN void        *UserElement)
{
    void             *RAElement = ElementPtr;
    HASH_TableType   *pHashTable;

    pHashTable = (HASH_TableType *)hHash;

    RvLogEnter(pHashTable->pLogSource, (pHashTable->pLogSource,
        "HASH_SetUserElement (hHash=0x%p,ElementPtr=0x%p,UserElement=0x%p)",
        hHash,ElementPtr,UserElement));

    if (NULL == RAElement)
    {
        RvLogExcep(pHashTable->pLogSource, (pHashTable->pLogSource,
            "HASH_SetUserElement - (hHash=0x%p) , RAGet failed",hHash));
        return RV_ERROR_UNKNOWN;
    }

    memcpy ( &((char *)RAElement)[sizeof(HASH_ListElement) + pHashTable->UserParamsSize] ,
             UserElement ,
             pHashTable->UserRecSize );

    RvLogLeave(pHashTable->pLogSource, (pHashTable->pLogSource,
        "HASH_SetUserElement (hHash=0x%p,ElementPtr=0x%p)=%d",
        hHash,ElementPtr,RV_OK));
    return RV_OK;
}



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
                                              IN void        *ElementPtr)
{
    HASH_ListElement    *NextListRec;
    HASH_ListElement    *PrevListRec;
    HASH_ListElement    *ListRec = (HASH_ListElement    *)ElementPtr;
    HASH_TableEntryType *HashEntry;
    HASH_TableType      *pHashTable;

    pHashTable = (HASH_TableType *)hHash;

    RvLogEnter(pHashTable->pLogSource, (pHashTable->pLogSource,
        "HASH_RemoveElement (hHash=0x%p,ElementPtr=0x%p)",
        hHash,ElementPtr));

    if ( ListRec->prev != NULL )
    {
        /* We've got previous elements in here... */
        PrevListRec = (HASH_ListElement *)ListRec->prev;
        PrevListRec->next = ListRec->next;
    }
    else
    {
       HashEntry = HASH_ENTRY(pHashTable, ListRec->EntryNum);
       HashEntry->FirstElementPtr = (RA_Element)ListRec->next;

       /* Check if this thing is empty for the resources */
       if (ListRec->next == NULL)
	   {
           pHashTable->KeysSize.numOfUsed--;
	   }
    }


    if ( ListRec->next != NULL )
    {
        NextListRec = (HASH_ListElement *)ListRec->next;
        NextListRec->prev = ListRec->prev;
    }

    RA_DeAlloc (pHashTable->UserRAHandle , (RA_Element)ElementPtr);
    RvLogLeave(pHashTable->pLogSource, (pHashTable->pLogSource,
        "HASH_RemoveElement (hHash=0x%p,ElementPtr=0x%p)=%d",
        hHash,ElementPtr,RV_OK));

    return RV_OK;
}


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
RvStatus RVAPI RVCALLCONV HASH_GetUserKeyElement (IN  HASH_HANDLE hHash,
                                                   IN  void        *ElementPtr,
                                                   OUT void        *KeyUserElement)
{
    void            *RAElement = ElementPtr;
    HASH_TableType  *pHashTable;
	RvInt32         ElementId;


    pHashTable = (HASH_TableType *)hHash;

	ElementId = RA_GetByPointer(pHashTable->UserRAHandle,(RA_Element)RAElement);

    if (RA_ElementIsVacant (pHashTable->UserRAHandle, ElementId))
    {
        /* element not in use */
        KeyUserElement = NULL;
        return RV_ERROR_BADPARAM;
    }
    else
    {

        if (NULL == RAElement)
        {
		  	RvLogExcep(pHashTable->pLogSource, (pHashTable->pLogSource,
                        "HASH_GetUserKeyElement , RAGet failed"));
            return RV_ERROR_UNKNOWN;
        }

        memcpy (KeyUserElement ,
                &((char *)RAElement)[sizeof(HASH_ListElement)] ,
                pHashTable->UserParamsSize );
        return RV_OK;
    }
}


/************************************************************************************************************************
 * HASH_GetResourcesStatus
 * purpose : Return information about the resource allocation of this HASH
 * input   : hHash      - Pointer to the array.
 * output  : resources  - Includes the following information:
 * return  : RV_OK         - In case of success.
 *           RV_ERROR_BADPARAM   - Invalid hHash handle (NULL)
 ************************************************************************************************************************/
RvStatus RVAPI RVCALLCONV HASH_GetResourcesStatus(IN  HASH_HANDLE          hHash,
                                                   OUT RV_HASH_Resource*    resources)
{
    HASH_TableType  *pHashTable;

    if (hHash == NULL)
        return RV_ERROR_BADPARAM;

    pHashTable = (HASH_TableType *)hHash;

    RA_GetResourcesStatus(pHashTable->UserRAHandle, &resources->elements);
    memcpy(&resources->keys, &pHashTable->KeysSize, sizeof(RV_Resource));

    return RV_OK;
}



#ifdef __cplusplus
}
#endif


