
#ifndef ABMEM_POOL_H
#define ABMEM_POOL_H

#ifdef __cplusplus
extern "C" {
#endif

/* The following macro is used to define type of the type VOID * in a way that
   each VOID * type that is defined using this MACRO will be different than 
   other types that are defined by this macro ( such a way will enable the 
   compiler to warn us from Ilegal calls between two differents types even 
   though that both of them need to be defined as VOID *.This macro is used to
   define hanldes in all stack modules. */

#ifndef DECLARE_VOID_POINTER
#define DECLARE_VOID_POINTER(name) typedef struct { int unused; } name##__ ; \
                                   typedef const name##__ * name; \
                                   typedef name*  LP##name
#endif /* DECLARE_VOID_POINTER */

DECLARE_VOID_POINTER(ABHRA);
DECLARE_VOID_POINTER(ABRA_Element);

typedef struct
{
    unsigned int      numOfUsed;
    unsigned int      maxUsage;
    unsigned int      maxNumOfElements;
    unsigned int      size;
} ABRA_Resource;

typedef void (*ABRA_PrintFunc)(ABRA_Element Elem, void *Param);

ABHRA ABRA_Construct_trace(unsigned int      ElementSize,
                           unsigned int      NumOfElements,
                           const char*    Name,
                           const char* file,int line);

void ABRA_SetInitFunc(ABHRA raH, void (*ifunc)(void*));

void ABRA_Destruct(ABHRA raH);


#define ABRA_Alloc(raH,ElementPtr) ABRA_Alloc_trace(raH,ElementPtr,__FILE__,__LINE__)
int ABRA_Alloc_trace(ABHRA         raH, ABRA_Element* ElementPtr,const char* file,int line);

#define ABRA_DeAlloc(raH,ElementPtr) ABRA_DeAlloc_trace(raH,ElementPtr,__FILE__,__LINE__)
int ABRA_DeAlloc_trace(ABHRA       raH, ABRA_Element delElem,const char* file,int line);

int ABRA_GetByPointer(ABHRA         raH, ABRA_Element  ptr);

unsigned int ABRA_AllocationSize(unsigned int   ElementSize,unsigned int   NumOfElements);

ABHRA ABRA_ConstructFrom_trace(unsigned int      ElementSize,
                               unsigned int      NumOfElements,
                               const char* Name,
                               const char* file,int line);

ABRA_Element ABRA_GetElement(ABHRA       raH,int  Location);

#define ABRA_GetElementInc(raH,Location) ABRA_GetElementInc_trace(raH,Location,__FILE__,__LINE__)
ABRA_Element ABRA_GetElementInc_trace(ABHRA       raH,int  Location,const char* file,int line);

int ABRA_ElementIsVacant(ABHRA      raH,int Location);

int ABRA_ElemPtrIsVacant(ABHRA         raH, ABRA_Element  ptr);

int ABRA_GetFirstUsedElement(ABHRA raH,int StartLocation, ABRA_Element*  Element,int*    ElemLoc);

int ABRA_NumOfUsedElements(ABHRA raH);

int ABRA_NumOfFreeElements(ABHRA raH);

void ABRA_GetAllocationParams(ABHRA raH,unsigned int* ElementSize,unsigned int* NumOfElements);

int ABRA_GetResourcesStatus(ABHRA raH, ABRA_Resource*  resources);

void ABRA_ResetMaxUsageCounter(ABHRA raH);

void ABRA_Print(ABHRA   raH,void* Param,ABRA_PrintFunc func);

char* ABRA_GetName(ABHRA raH);

int   ABRA_Clear(ABHRA raH);

extern void unit_test_abmem_pool(void);
extern int (*abprintf)(const char * format,...);

#ifdef __cplusplus
}
#endif

#endif /*ABMEM_POOL_H*/


