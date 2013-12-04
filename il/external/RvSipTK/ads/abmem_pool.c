#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
	
//#include <abmem.h>	

#include <abmem_pool.h>
#define abmem_malloc(size,file,line) malloc((size))
//#define abmem_calloc(nmemb,size,file,line) calloc((nmemb),(size))
#define abmem_realloc(ptr,size,file,line) realloc((ptr),(size))
#define abmem_strdup(s,file,line) strdup((s))
//#define abmem_strndup(s,n,file,line) strndup((s),(n))
#define abmem_free(ptr,file,line) free((ptr))


/* Definition of minimum and maximum macros */
#ifndef RV_MIN
#define RV_MIN(a,b) (((a)<(b))?(a):(b))
#endif /* RV_MIN */

#ifndef RV_MAX
#define RV_MAX(a,b) (((a)>(b))?(a):(b))
#endif /* RV_MAX */

#define ABRA_LOCK(ra)
#define ABRA_UNLOCK(ra)

//extern int dprintf_T(const char * format,...);
//#define dprintf dprintf_T

//int (*abprintf)(const char * format,...)=&dprintf;
#define abprintf(...) 

static int print_memory_warning=1;

//define ABMEM_POOL_DEBUG

struct _ABRAI_Header {

    char* file;
    int line;

    ABRA_Element			FirstVacantElement;
    ABRA_Element			LastVacantElement;
    unsigned int			MaxNumOfElements;
    unsigned int			CurrNumOfElements;
    ABRA_Element*		   ArrayLocation;
    unsigned int			UsedNumOfElements;
    unsigned int			SizeOfElement;
    unsigned int        ElementAllocSize;
    unsigned int			MaxUsedNumOfElements;

    char                Name[32];
    int                 FreeAllocation;

    void                (*initFunc)(void* arg);
    char*               prev_page_next;

};

typedef struct _ABRAI_Header ABRAI_Header;

/* The following structure represents the way the software sees free nodes ( in order to keep a free list ) */
struct _ABRAI_VacantNode {

   void*       unused; /* This unused pointer is here to protect the first parameter people
                           will put in structs inside RA even when the element is vacant.*/

   void*       unused2; /* min size is 2*sizeof(void*) */

};

typedef struct _ABRAI_VacantNode ABRAI_VacantNode;

#ifdef ABMEM_POOL_DEBUG
typedef struct _DebugInfo {
   const char* file;
   int line;
   unsigned long hash;
} DebugInfo;
#endif

/* The minumum size of an element is the array*/
#define ABRAI_ElementMinSize (sizeof(ABRAI_VacantNode))

/* Page size, number of records */
#define PAGE_SIZE (32)

/* The following macros provide quick access .calculation to RA elements */

/* Definition of pointer size */
#define ABRAI_ALIGN sizeof(void*)

/* Index of every node is stored after it's end: */
#define GET_NODE_INDEX(ra,el) (*((unsigned int*)(((char*)(el))+((ABRAI_Header*)ra)->SizeOfElement)) & 0x7fffffff)

/* Index of every node is stored after it's end: */
#define SET_NEW_NODE_INDEX(ra,el,index) {*((unsigned int*)(((char*)(el))+((ABRAI_Header*)ra)->SizeOfElement)) = ((unsigned int)(index)) & 0x7fffffff;}

/* Size calculation for an element - we must use it for unix systems */
static inline unsigned int ABRAI_ALIGN_SIZE(unsigned int n) {
   return ((((n) + 3) / 4) * 4);
}

/* Calculate the full size of the element */ 
static inline unsigned int ABRAI_ELEM_ALLOC_SIZE(ABRAI_Header* ra) {
#ifndef ABMEM_POOL_DEBUG
   return ABRAI_ALIGN_SIZE(ra->SizeOfElement + sizeof(unsigned int) + sizeof(void*));
#else
   return ABRAI_ALIGN_SIZE(ra->SizeOfElement + sizeof(unsigned int) + sizeof(void*) + sizeof(DebugInfo));
#endif
}

/* Calculate the pointer to the location of element i in a given ra */ 
#define GET_ELEM_DATA(ra,i) ((ABRA_Element)((char*)(((ABRAI_Header*)(ra))->ArrayLocation[(i)/(PAGE_SIZE)])+(((ABRAI_Header*)(ra))->ElementAllocSize)*((i)%(PAGE_SIZE))))

/* Get next vacant node */
#define NEXT_VACANT_ELEMENT_POINTER(ra,el) (((ABRA_Element*)(((char*)(el))+((ABRAI_Header*)ra)->SizeOfElement+sizeof(unsigned int))))

/* Busy bit of every node is stored after it's end, immediately: */
#define GET_NODE_BIT(ra,el) ((*((unsigned int*)(((char*)(el))+((ABRAI_Header*)ra)->SizeOfElement)) & 0x80000000)>0)

/* Busy bit of every node is stored after it's end, immediately: */
#define GET_NODE_BIT_BY_INDEX(ra,index) GET_NODE_BIT((ra),GET_ELEM_DATA((ra),(index)))

/* Busy bit of every node is stored after it's end, immediately: */
#define SET_NODE_BIT_TRUE(ra,el) {\
\
   unsigned int* __cptr__=(unsigned int*)(((char*)(el))+((ABRAI_Header*)ra)->SizeOfElement);\
\
   *__cptr__ |= 0x80000000;\
}

/* Busy bit of every node is stored after it's end, immediately: */
#define SET_NODE_BIT_FALSE(ra,el) {\
\
   unsigned int* __cptr__=(unsigned int*)(((char*)(el))+((ABRAI_Header*)ra)->SizeOfElement);\
\
   *__cptr__ &= 0x7fffffff;\
}

#ifdef ABMEM_POOL_DEBUG

#define GET_DEBUG_INFO_POINTER(ra,el) (((DebugInfo*)(((char*)(el))+((ABRAI_Header*)ra)->SizeOfElement+sizeof(unsigned int)+sizeof(void*))))

static inline unsigned long hash(void *bin,int len)
{
   unsigned long hash = 5381;
   const char* binstr=(const char*)bin;
   int c;
   
   if(binstr) {
      while (len--) {
         c = *binstr++;
         hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
      }
   }
   
   return hash;
}

static inline void setDebugInfo(ABRAI_Header* ra,ABRA_Element elem,const char* file,int line) {
   if(ra && elem) {
      DebugInfo *dip=GET_DEBUG_INFO_POINTER(ra,elem);
      if(dip) {
         dip->file=file;
         dip->line=line;
         dip->hash=hash((void*)elem,ra->SizeOfElement);
      }
   }
}

static inline int checkDebugInfo(ABRAI_Header* ra,ABRA_Element elem,const char* file,int line) {
   int ret=0;
   if(ra && elem) {
      DebugInfo *dip=GET_DEBUG_INFO_POINTER(ra,elem);
      if(dip) {
         unsigned long newhash=hash((void*)elem,ra->SizeOfElement);
         if(dip->hash!=newhash) {
            ret=-1;
            if(print_memory_warning) {
               abprintf("AbmemPool Test: %s ACCESS ERROR:\n hashs are not the same for element 0x%x: old %d, new %d.\n Memory frozen in %s/%d, current check place %s/%d\n",
                  ra->Name,(int)elem,dip->hash,newhash,dip->file,dip->line,file,line);
            }
         }
      }
   }
   return ret;
}

#endif

static inline ABRA_Element GET_NEXT_USED(ABRAI_Header* ra,ABRA_Element elem,int *i) {
   
   if(ra && elem && i) {

      int totsize=ra->CurrNumOfElements;
      int elemsize=ra->SizeOfElement;
      int allocsize=ra->ElementAllocSize;
      int index=*i;
      
      ABRA_LOCK(ra);
      
      {
         while(elem && index<totsize) {
            if( (*((unsigned int*)(((char*)elem)+elemsize))   & 0x80000000) > 0 ) {
               *i=index;
               break;
            } else {
               if((++index)>=totsize) {
                  elem=NULL;
                  break;
               }
               if(index%PAGE_SIZE==0) elem=GET_ELEM_DATA(ra,index);
               else elem=(ABRA_Element)((char*)elem+allocsize);
            }
         }
      }

      ABRA_UNLOCK(ra);
   }

   return elem;
}

/************************************************************************************************************************
 * Routines
 ************************************************************************************************************************/

int ABRA_GetByPointer(ABHRA         raH,
                      ABRA_Element  ptr)
{
   ABRAI_Header *ra = (ABRAI_Header *)raH;
   int position=-1;
   
   ABRA_LOCK(ra);
   
   if(ra && ptr) {
      position=GET_NODE_INDEX(ra,ptr);
   }
   
   ABRA_UNLOCK(ra);

   return position;
}

int ABRA_ElemPtrIsVacant(ABHRA         raH,  ABRA_Element  ptr)
{
   ABRAI_Header *ra = (ABRAI_Header *)raH;
   int position;
   
   int  isVacant=0;
   
   ABRA_LOCK(ra);
   
   if(ra && ptr) {
      /* Calculate the element */
      isVacant =  !GET_NODE_BIT(ra,ptr);

#ifdef ABMEM_POOL_DEBUG
      {
         int index=0;
         
         index=GET_NODE_INDEX(ra,ptr);
         if(index<0 || index>=ra->CurrNumOfElements) {
            abprintf("Wrong integrity check 7 (%d): %s: %s : %d\n",index,__FUNCTION__,__FILE__,__LINE__);
         }
         if(GET_NODE_BIT_BY_INDEX(ra,index) != (!isVacant)) {
            abprintf("Wrong integrity check 8 (%d): %s: %s : %d\n",index,__FUNCTION__,__FILE__,__LINE__);
         }
      }
#endif

   }
   
   ABRA_UNLOCK(ra);
   
   return isVacant;
}

int   ABRA_Clear(ABHRA raH)
{
   ABRAI_Header *ra = (ABRAI_Header *)raH;
   
   if(ra->ArrayLocation) {
      {
         int i=0;
         for(i=0;i<(int)((ra->CurrNumOfElements)/PAGE_SIZE);i++) {
            if(ra->ArrayLocation[i]) {
               abmem_free((void*)(ra->ArrayLocation[i]),ra->file,ra->line);
               ra->ArrayLocation[i]=NULL;
            }
         }
      }
      abmem_free(ra->ArrayLocation,ra->file,ra->line);
      ra->ArrayLocation=NULL;
   }
   
   ra->CurrNumOfElements = 0;
   
   if(!(ra->ArrayLocation = (ABRA_Element *)abmem_malloc(ABRAI_ALIGN, ra->file,ra->line))) {
      return -1;
   }
   
   memset(ra->ArrayLocation, 0,ABRAI_ALIGN);
   
   ra->UsedNumOfElements = 0;
   ra->MaxUsedNumOfElements = 0;
   
   ra->FirstVacantElement = NULL;
   ra->LastVacantElement = NULL;

   ra->initFunc=NULL;
   
   return 0;
}

ABHRA ABRA_ConstructFrom_trace(unsigned int      ElementSize,
                               unsigned int      NumOfElements,
                               const char* Name,
                               const char* file,int line)
{
    ABRAI_Header* ra = NULL;
    unsigned int ActualElementSize = ABRAI_ALIGN_SIZE(RV_MAX(ABRAI_ElementMinSize, ElementSize));

    if(!Name) Name="";

    if (ActualElementSize < ABRAI_ElementMinSize) {
        ActualElementSize = ABRAI_ElementMinSize;
    }

    if (!(ra = (ABRAI_Header *)abmem_malloc(sizeof(ABRAI_Header), file,line))) {
       return NULL;
    }
    
    memset(ra, 0, sizeof(ABRAI_Header));
    
    ra->file=abmem_strdup(file,file,line);
    ra->line=line;
    ra->SizeOfElement    = ActualElementSize;
    ra->ElementAllocSize = ABRAI_ELEM_ALLOC_SIZE(ra);
    ra->FreeAllocation   = 1;
    strncpy((char *)ra->Name, Name, sizeof(ra->Name) - 1);
    
    ra->MaxNumOfElements = NumOfElements;

    ra->CurrNumOfElements = 0;
    
    if(!(ra->ArrayLocation = (ABRA_Element *)abmem_malloc(ABRAI_ALIGN, file,line))) {
       return NULL;
    }
    
    memset(ra->ArrayLocation, 0,ABRAI_ALIGN);
    
    ra->UsedNumOfElements = 0;
    ra->MaxUsedNumOfElements = 0;
    
    ra->FirstVacantElement = NULL;
    ra->LastVacantElement = NULL;

    ra->initFunc=NULL;
    ra->prev_page_next = NULL;

    return (ABHRA)ra;
}

ABHRA ABRA_Construct_trace(unsigned int      ElementSize,
                           unsigned int      NumOfElements,
                           const char*    Name,
                           const char* file,int line)
{
    ABRAI_Header* ra = (ABRAI_Header *)ABRA_ConstructFrom_trace(ElementSize, NumOfElements, Name,file,line);

    return (ABHRA)ra;
}

void ABRA_SetInitFunc(ABHRA raH, void (*ifunc)(void*))
{
   ABRAI_Header *ra = (ABRAI_Header *)raH;
   
   if(ra) {
      ra->initFunc=ifunc;
   }
}

void ABRA_Destruct(ABHRA raH)
{
   ABRAI_Header *ra = (ABRAI_Header *)raH;
   
   if (ra != NULL)
   {
      if (ra->FreeAllocation) {
         if(ra->ArrayLocation) {
            {
               int i=0;
               for(i=0;i<(int)((ra->CurrNumOfElements)/PAGE_SIZE);i++) {
                  if(ra->ArrayLocation[i]) {
                     abmem_free((void*)(ra->ArrayLocation[i]),ra->file,ra->line);
                     ra->ArrayLocation[i]=NULL;
                  }
               }
            }
            abmem_free(ra->ArrayLocation,ra->file,ra->line);
            ra->ArrayLocation=NULL;
         }
         if(ra->file) free(ra->file);
         free(ra);
      }
   }
   
}

static int ABRA_Inc_trace(ABHRA raH,const char* file,int line) {

   ABRAI_Header *ra = (ABRAI_Header *)raH;

   int i=0;
   int base=ra->CurrNumOfElements;
   int location_base=(int)(base/PAGE_SIZE);
   int alloc_size=ra->ElementAllocSize;
   char* cptr=NULL;
   int index=0;
   int elementSize=ra->SizeOfElement;
   int totallocsize=PAGE_SIZE*alloc_size;
   int normElems=PAGE_SIZE-1;

   ABRA_LOCK(ra);
   
   ra->ArrayLocation=(ABRA_Element*)abmem_realloc(ra->ArrayLocation,
      (location_base+1)*sizeof(ABRA_Element),
      ra->file,
      ra->line);
   
   cptr = abmem_malloc(totallocsize,ra->file,ra->line);
   memset(cptr,0,totallocsize);
   
   ra->ArrayLocation[location_base]= (ABRA_Element)cptr;
   
   ra->CurrNumOfElements+=PAGE_SIZE;
   
   index=base;
   
   for(i=0;i<normElems;i++) {

      char* next=cptr+alloc_size;
      char* indexptr=cptr+elementSize;

      *((unsigned int*)indexptr) = ((unsigned int)(index++)) & 0x7fffffff;

#ifdef ABMEM_POOL_DEBUG
      setDebugInfo(ra,(ABRA_Element)cptr,file,line);
#endif
      
      *(NEXT_VACANT_ELEMENT_POINTER(ra,cptr)) = (ABRA_Element)next;

      if(ra->initFunc) (*(ra->initFunc))(cptr);

      cptr=next;
   }

   //Last element:
   *((unsigned int*)(cptr+elementSize)) = ((unsigned int)(index)) & 0x7fffffff;
   if(ra->prev_page_next)
       *(NEXT_VACANT_ELEMENT_POINTER(ra,ra->prev_page_next)) = (ABRA_Element)(ra->ArrayLocation[location_base]);
   ra->prev_page_next = cptr;

   *(NEXT_VACANT_ELEMENT_POINTER(ra,cptr)) = NULL;

   if(ra->initFunc) (*(ra->initFunc))(cptr);

#ifdef ABMEM_POOL_DEBUG
   setDebugInfo(ra,(ABRA_Element)cptr,file,line);
#endif
 
   if(ra->FirstVacantElement==NULL) ra->FirstVacantElement = GET_ELEM_DATA(ra, base+1);   
   ra->LastVacantElement = GET_ELEM_DATA(ra,ra->CurrNumOfElements-1);

   ABRA_UNLOCK(ra);

   return 0;
}

int ABRA_Alloc_trace(ABHRA         raH,ABRA_Element* ElementPtr,const char* file, int line)
{
   ABRAI_Header *ra = (ABRAI_Header *)raH;
   ABRA_Element       allocatedElement;
   int status=0;

   ABRA_LOCK(ra);

   if(ra->UsedNumOfElements>=ra->MaxNumOfElements) 
   {
      abprintf("Warning: Too many elements in pool %s (created in %s:%d): %d (max - %d). Allocation in %s:%d\n",
         ra->Name,ra->file,ra->line,ra->UsedNumOfElements,ra->MaxNumOfElements,file,line);
      status=-1;
      if(ElementPtr) {
         *ElementPtr=NULL;
      }
   }
   else if (ra->FirstVacantElement == NULL) {
      
      int i=0;
      int base=ra->CurrNumOfElements;
      int location_base=(int)(base/PAGE_SIZE);
      int alloc_size=ra->ElementAllocSize;
      char* cptr=NULL;
      int index=0;
      int elementSize=ra->SizeOfElement;
      int totallocsize=PAGE_SIZE*alloc_size;
      int normElems=PAGE_SIZE-1;
      
      ra->ArrayLocation=(ABRA_Element*)abmem_realloc(ra->ArrayLocation,
         (location_base+1)*sizeof(ABRA_Element),
         ra->file,
         ra->line);
      
      cptr = abmem_malloc(totallocsize,ra->file,ra->line);
      memset(cptr,0,totallocsize);
      
      ra->ArrayLocation[location_base]= (ABRA_Element)cptr;
      
      ra->CurrNumOfElements+=PAGE_SIZE;
      
      index=base;
      
      for(i=0;i<normElems;i++) {
         
         char* next=cptr+alloc_size;
         char* indexptr=cptr+elementSize;
         
         *((unsigned int*)indexptr) = ((unsigned int)(index++)) & 0x7fffffff;
         
#ifdef ABMEM_POOL_DEBUG
         setDebugInfo(ra,(ABRA_Element)cptr,file,line);
#endif
         
         *(NEXT_VACANT_ELEMENT_POINTER(ra,cptr)) = (ABRA_Element)next;

         if(ra->initFunc) (*(ra->initFunc))(cptr);
         
         cptr=next;
      }
      
      //Last element:
      *((unsigned int*)(cptr+elementSize)) = ((unsigned int)(index)) & 0x7fffffff;
      
#ifdef ABMEM_POOL_DEBUG
      setDebugInfo(ra,(ABRA_Element)cptr,file,line);
#endif
      
      *(NEXT_VACANT_ELEMENT_POINTER(ra,cptr)) = NULL;

      if(ra->initFunc) (*(ra->initFunc))(cptr);
      
      if(ElementPtr) {
         allocatedElement=ra->ArrayLocation[location_base];
         *ElementPtr=allocatedElement;
         ra->UsedNumOfElements++;
         if (ra->UsedNumOfElements > ra->MaxUsedNumOfElements) {
            ra->MaxUsedNumOfElements=ra->UsedNumOfElements;
         }
         ra->FirstVacantElement = GET_ELEM_DATA(ra, base+1);
         SET_NODE_BIT_TRUE(ra,allocatedElement);
      } else {
         ra->FirstVacantElement = ra->ArrayLocation[location_base];
      }
      
      ra->LastVacantElement = GET_ELEM_DATA(ra,ra->CurrNumOfElements-1);
      
   } else if(ElementPtr) {
      
      ra->UsedNumOfElements++;
      
      allocatedElement = ra->FirstVacantElement;
      
      /* Get the element from list of vacant elements and fix that list */
      ra->FirstVacantElement = *(NEXT_VACANT_ELEMENT_POINTER(ra,allocatedElement));
      if (ra->FirstVacantElement == NULL) {
         ra->LastVacantElement = NULL;
      }

#ifdef ABMEM_POOL_DEBUG
      {
         int index=0;
         
         if(GET_NODE_BIT(ra,allocatedElement)) {
            abprintf("Wrong integrity check 1: %s: %s : %d\n",__FUNCTION__,__FILE__,__LINE__);
            status=-1;
         }
         index=GET_NODE_INDEX(ra,allocatedElement);
         if(index<0 || index>=ra->CurrNumOfElements) {
            abprintf("Wrong integrity check 2 (%d): %s: %s : %d\n",index,__FUNCTION__,__FILE__,__LINE__);
            status=-1;
         }
         if(GET_NODE_BIT_BY_INDEX(ra,index)) {
            abprintf("Wrong integrity check 3 (%d): %s: %s : %d\n",index,__FUNCTION__,__FILE__,__LINE__);
            status=-1;
         }
      }
#endif
      
      SET_NODE_BIT_TRUE(ra, allocatedElement); /* make it occupied */
      
#ifdef ABMEM_POOL_DEBUG
      if(checkDebugInfo(ra,allocatedElement,file,line)!=0) {
         status=-1;
      }
#endif
      
      /* Set statistical information */ 
      if (ra->UsedNumOfElements > ra->MaxUsedNumOfElements) {
         ra->MaxUsedNumOfElements=ra->UsedNumOfElements;
      }
      
      *ElementPtr = allocatedElement;
   }

   ABRA_UNLOCK(ra);

   return status;
}

int ABRA_DeAlloc_trace(ABHRA raH, ABRA_Element delElem, const char* file, int line)
{
   ABRAI_Header *ra       = (ABRAI_Header *)raH;
   int    location = 0;
   int status=0;
   if (!ra || (delElem == NULL)) return -1;

   ABRA_LOCK(ra);
   
   location = ABRA_GetByPointer(raH, delElem);

   if(location>=0 && (unsigned int)location<ra->CurrNumOfElements && GET_NODE_BIT(ra,delElem)) {

#ifdef ABMEM_POOL_DEBUG
      {
         int index=0;
         
         index=GET_NODE_INDEX(ra,delElem);
         if(index<0 || index>=ra->CurrNumOfElements) {
            abprintf("Wrong integrity check 5 (%d): %s: %s : %d\n",index,__FUNCTION__,__FILE__,__LINE__);
            status=-1;
         }
         if(!GET_NODE_BIT_BY_INDEX(ra,index)) {
            abprintf("Wrong integrity check 6 (%d): %s: %s : %d\n",index,__FUNCTION__,__FILE__,__LINE__);
            status=-1;
         }
      }
#endif

      ra->UsedNumOfElements--;

	  *(NEXT_VACANT_ELEMENT_POINTER(ra,delElem)) = ra->FirstVacantElement;

#ifdef ABMEM_POOL_DEBUG
     setDebugInfo(ra,delElem,file,line);
#endif
     
	  ra->FirstVacantElement=delElem;

	  if(ra->LastVacantElement==NULL) {
		  ra->LastVacantElement=delElem;
	  } 
      
      /* Set this element as a vacant one in the bits vector */
      SET_NODE_BIT_FALSE(ra, delElem);
      
   } else {
      if(print_memory_warning) {
         abprintf("WRONG pointer to release in the pool(%s,%d): 0x%x, location %d, busy %d\n",
            ra->file,ra->line,(int)delElem,location,GET_NODE_BIT(ra,delElem));
      }
      status=-1;
   }
   
   ABRA_UNLOCK(ra);
   return status;
}

int ABRA_GetFirstUsedElement(ABHRA          raH,
                             int          StartLocation,
                             ABRA_Element*  Element,
                             int*    ElemLoc)
{
   ABRAI_Header  *ra = (ABRAI_Header *)raH;
   ABRA_Element elem=NULL;

   if(StartLocation<0) StartLocation=0;

   elem=GET_NEXT_USED(ra,GET_ELEM_DATA(ra,StartLocation),&StartLocation);

   if(elem) {
     *ElemLoc = StartLocation;
     *Element = elem;
   }

   if (elem)
      return ( 0 );
   else
      return ( -1 );
}

int ABRA_NumOfUsedElements(ABHRA raH)
{
    ABRAI_Header *ra = (ABRAI_Header *)raH;

    if (!raH)
    {
        return -1;
    }

    return ra->UsedNumOfElements;
}

int ABRA_NumOfFreeElements(ABHRA raH)
{
    ABRAI_Header *ra = (ABRAI_Header *)raH;

    if (!raH)
    {
        return -1;
    }

    return ra->MaxNumOfElements - ra->UsedNumOfElements;
}

int ABRA_ElementIsVacant(ABHRA      raH,
                         int Location)
{
   ABRAI_Header *ra = (ABRAI_Header *)raH;
   int		bIsBitVacant = 1;
   ABRA_LOCK(ra);	
   if(Location>=0 && (unsigned int)Location<ra->CurrNumOfElements) {
      bIsBitVacant=!GET_NODE_BIT_BY_INDEX(ra,Location);
   }
   ABRA_UNLOCK(ra);
   return bIsBitVacant;
}

ABRA_Element ABRA_GetElement(ABHRA       raH,
                             int  Location)
{
   ABRAI_Header *ra = (ABRAI_Header *)raH;
   if((Location>=0)&&((unsigned int)Location<ra->MaxNumOfElements)) return GET_ELEM_DATA(ra, Location);
   else return NULL;
}

ABRA_Element ABRA_GetElementInc_trace(ABHRA       raH,int  Location, const char* file, int line)
{
   ABRAI_Header *ra = (ABRAI_Header *)raH;
   if(Location>=0 && (unsigned int)Location<ra->MaxNumOfElements) {
     while(((unsigned int)Location>=ra->CurrNumOfElements) && (ra->CurrNumOfElements<ra->MaxNumOfElements)) {
         if(ABRA_Inc_trace(raH,file,line)!=0) {
            return NULL;
         }
      }
     if((unsigned int)Location>=ra->CurrNumOfElements) return NULL;
      return GET_ELEM_DATA(ra, Location);
   }
   else return NULL;
}

unsigned int ABRA_AllocationSize(unsigned int   ElementSize,
                                 unsigned int   NumOfElements) {
   return sizeof(ABRAI_Header);
}

void  ABRA_GetAllocationParams(ABHRA        raH,
                               unsigned int* ElementSize,
                               unsigned int* NumOfElements)
{
    ABRAI_Header *ra = (ABRAI_Header *)raH;

    if (!raH)
    {
        if (ElementSize != NULL) *ElementSize = 0;
        if (NumOfElements != NULL) *NumOfElements = 0;
        return;
    }

    if (ElementSize != NULL)
        *ElementSize = ra->SizeOfElement;

    if (NumOfElements != NULL)
        *NumOfElements = ra->CurrNumOfElements;
}

int ABRA_GetResourcesStatus(ABHRA raH, ABRA_Resource*  resources) {

   ABRAI_Header *ra = (ABRAI_Header *)raH;

   ABRA_LOCK(ra);

   resources->numOfUsed        = ra->UsedNumOfElements;
   resources->maxUsage         = ra->MaxUsedNumOfElements;
   resources->maxNumOfElements = ra->MaxNumOfElements;
   resources->size             = ra->SizeOfElement;

   ABRA_UNLOCK(ra);

   return 0;
}

void ABRA_ResetMaxUsageCounter(ABHRA raH) {
   ABRAI_Header *ra = (ABRAI_Header *)raH;
   ra->MaxUsedNumOfElements=0;
}

void ABRA_Print(ABHRA   raH,void* Param,ABRA_PrintFunc func) {
   
   ABRAI_Header *ra = (ABRAI_Header *)raH;
   unsigned int i;
   
   if (ra && func) {   
      for (i=0; i < ra->CurrNumOfElements; i++) {
         if (!ABRA_ElementIsVacant( raH, i)) {
            func( GET_ELEM_DATA(ra, i), Param );
         }
      }
   }
}

char* ABRA_GetName(ABHRA raH) {

   ABRAI_Header *ra = (ABRAI_Header *)raH;

   if(ra) return ra->Name;

   return "";
}

/* Unit Test: */

static int checkResources(ABHRA raH,
                          int UsedNumOfElements,
                          int MaxUsedNumOfElements,
                          int CurrNumOfElements,
                          const char* s) {

   ABRAI_Header *ra = (ABRAI_Header *)raH;

   if(ra->CurrNumOfElements!=(unsigned int)CurrNumOfElements) {
      printf("ABHRA check resources failed: 111: %s: %d:%d\n",s,ra->CurrNumOfElements,CurrNumOfElements);
      return -1;
   }

   if(ra->UsedNumOfElements!=(unsigned int)UsedNumOfElements) {
      printf("ABHRA check resources failed: 222: %s: %d:%d\n",s,ra->UsedNumOfElements,UsedNumOfElements);
      return -1;
   }

   if(ra->MaxUsedNumOfElements!=(unsigned int)MaxUsedNumOfElements) {
      printf("ABHRA check resources failed: 333: %s: %d:%d\n",s,ra->MaxUsedNumOfElements,MaxUsedNumOfElements);
      return -1;
   }

   return 0;
}

static int unit_test_over_pool(ABHRA ra,int len) {

#define elemnum (45678)

   int i=0;
   int p=0;
   ABRA_Element elements[elemnum];

   for(i=0;i<100;i++) {
      if(ABRA_Inc_trace(ra,__FILE__,__LINE__)!=0) {
         abprintf("AbmemPool Test failed 111\n");
         return -1;
      }
   }

   if(checkResources(ra,0,0,100*PAGE_SIZE,"222")) {
      return -1;
   }

   ABRA_Clear(ra);

   for(i=0;i<elemnum;i++) {
      if(ABRA_Alloc(ra,&(elements[i]))!=0) {
         abprintf("AbmemPool Test failed 333\n");
         return -1;
      }
      p=ABRA_GetByPointer(ra,elements[i]);
      if(p!=i) {
         abprintf("AbmemPool Test failed 444:%d:%d\n",i,p);
         return -1;
      }
      if(ABRA_ElemPtrIsVacant(ra,elements[i])) {
         abprintf("AbmemPool Test failed 555:%d\n",i);
         return -1;
      }

      if(checkResources(ra,i+1,i+1,(((int)(i/PAGE_SIZE))+1)*PAGE_SIZE,"666")) {
         return -1;
      }
   }

   for(i=0;i<elemnum;i++) {
      if(ABRA_DeAlloc(ra,elements[i])!=0) {
         abprintf("AbmemPool Test failed 777\n");
         return -1;
      }
      if(!ABRA_ElemPtrIsVacant(ra,elements[i])) {
         abprintf("AbmemPool Test failed 888:%d\n",i);
         return -1;
      }

      if(checkResources(ra,elemnum-i-1,elemnum,(((int)(elemnum/PAGE_SIZE))+1)*PAGE_SIZE,"999")) {
         return -1;
      }
   }

   for(i=0;i<elemnum/2;i++) {
      if(ABRA_Alloc(ra,&(elements[2*i]))!=0) {
         abprintf("AbmemPool Test failed 101010\n");
         return -1;
      }
      if(ABRA_Alloc(ra,&(elements[2*i+1]))!=0) {
         abprintf("AbmemPool Test failed 111111\n");
         return -1;
      }
      if(ABRA_ElemPtrIsVacant(ra,elements[2*i])) {
         abprintf("AbmemPool Test failed 121212:%d\n",i);
         return -1;
      }
      if(ABRA_ElemPtrIsVacant(ra,elements[2*i+1])) {
         abprintf("AbmemPool Test failed 121212:%d\n",i);
         return -1;
      }
      if(ABRA_DeAlloc(ra,elements[2*i+1])!=0) {
         abprintf("AbmemPool Test failed 131313\n");
         return -1;
      }
      if(ABRA_DeAlloc(ra,elements[2*i+1])==0) {
         abprintf("AbmemPool Test failed 131313.111\n");
         return -1;
      }
      if(checkResources(ra,i+1,elemnum,(((int)(elemnum/PAGE_SIZE))+1)*PAGE_SIZE,"141414")) {
         return -1;
      }
   }

#ifdef ABMEM_POOL_DEBUG
   //Memory test:
   ABRA_Clear(ra);

   for(i=0;i<PAGE_SIZE;i++) {
      if(ABRA_Alloc(ra,&(elements[i]))!=0) {
         abprintf("AbmemPool Test failed 151515 (%d)\n",i);
         return -1;
      }
   }

   for(i=0;i<PAGE_SIZE;i++) {
      if(ABRA_DeAlloc(ra,elements[i])!=0) {
         abprintf("AbmemPool Test failed 161616 (%d)\n",i);
         return -1;
      }
   }

   for(i=0;i<PAGE_SIZE;i++) {
      memset((void*)(elements[i]),7,len);
   }
   
   for(i=0;i<PAGE_SIZE;i++) {
      if(ABRA_Alloc(ra,&(elements[i]))==0) {
         abprintf("AbmemPool Test failed 171717 (%d)\n",i);
         return -1;
      }
   }

#endif

   return 0;
}

void unit_test_abmem_pool(void) {

   int i=0;
   int i357=10;
   int i3570=100;

   print_memory_warning=0;

   abprintf("ABMEM POOL UNIT TEST START: \n");

   for(i=0;i<357;i++) {

      abprintf("ABMEM POOL UNIT TEST CYCLE %d START:\n",i);

      {
         ABHRA ra357 = ABRA_ConstructFrom_trace(i357,
            10000000,
            "ra357",
            __FILE__,__LINE__);
         
         ABHRA ra3570 = ABRA_ConstructFrom_trace(i3570,
            10000000,
            "ra3570",
            __FILE__,__LINE__);
         
         if(unit_test_over_pool(ra357,i357)!=0) {
            ABRA_Clear(ra357);
            ABRA_Clear(ra3570);
            ABRA_Destruct(ra357);
            ABRA_Destruct(ra3570);
            break;
         }
         if(unit_test_over_pool(ra3570,i3570)!=0) {
            ABRA_Clear(ra357);
            ABRA_Clear(ra3570);
            ABRA_Destruct(ra357);
            ABRA_Destruct(ra3570);
            break;
         }
         
         ABRA_Clear(ra357);
         ABRA_Clear(ra3570);
         
         ABRA_Destruct(ra357);
         ABRA_Destruct(ra3570);
         
         i357+=1;
         i3570+=10;
      }

      abprintf("ABMEM POOL UNIT TEST CYCLE %d STOP!\n",i);
   }

   abprintf("ABMEM POOL UNIT TEST STOP! \n");

   print_memory_warning=1;
}

#ifdef __cplusplus
}
#endif
