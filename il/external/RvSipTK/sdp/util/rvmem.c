#if (0)
************************************************************************
Filename:
Description:
************************************************************************
                Copyright (c) 1999 RADVision Inc.
************************************************************************
NOTICE:
This document contains information that is proprietary to RADVision LTD.
No part of this publication may be reproduced in any form whatsoever
without written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make
changes without obligation to notify any person of such revisions or
changes.
************************************************************************
************************************************************************
$Revision: #1 $
$Date: 2011/08/05 $
$Author: songkamongkol $
************************************************************************
#endif

#include <string.h>
#include "rvplatform.h"
#include "rvtypes.h"
#include "rvmem.h"

#if defined(RV_MEM_ANSI)

#include <stdlib.h>

#if defined(RV_MEMTRACE_ON)

#include <stdio.h>
#include "rvmutex.h"
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#include <stdio.h>
#include "rvsem.h"
#if defined(UPDATED_BY_SPIRENT_ABACUS)
int dtprintf(const char* format,...);
#include "abmem/abmem.h"
#endif
#endif
/* SPIRENT_END */

static RvInt rvMemTraceMemOut = 0;
static RvInt rvMemTracePeakMem = 0;
static RvMutex rvMemTraceMutex;
static FILE *rvMemTraceLog = NULL;
static char rvMemTraceCheckPtName[50];
static RvInt rvMemTraceCheckPtStart;

#if defined(RV_OS_WIN32) && defined(RV_DEBUG_ON)
#define RV_CALLSTACKTRACE_ON
#endif
#define RV_CALLSTACKTRACE_SIZE 8

typedef struct RvMemTraceRecord_
{
	struct RvMemTraceRecord_ *prev;
	struct RvMemTraceRecord_ *next;
	RvInt size;
#ifdef RV_CALLSTACKTRACE_ON
	void *callStack[RV_CALLSTACKTRACE_SIZE];
#endif
} RvMemTraceRecord;

static RvMemTraceRecord  rvMemTraceFirst = {NULL, NULL, 0};
static RvMemTraceRecord *rvMemTraceHead = &rvMemTraceFirst;
static RvMemTraceRecord *rvMemTraceTail = &rvMemTraceFirst;

void rvMemTraceConstruct(void)
{
	rvMutexConstruct(&rvMemTraceMutex);
	rvMemTraceLog = fopen("rvmem.log", "w+");
}

void rvMemTraceCheckpoint(const char *checkPtName)
{
	if(rvMemTraceLog != NULL)
		fprintf(rvMemTraceLog, "%-26s %-10u\n", checkPtName, rvMemTraceMemOut);
}

void rvMemTraceStart(const char *checkPtName)
{
	rvMutexLock(&rvMemTraceMutex);
	strcpy(rvMemTraceCheckPtName, checkPtName);
	rvMemTraceCheckPtStart = rvMemTraceMemOut;
	if(rvMemTraceLog != NULL)
		fprintf(rvMemTraceLog, "Start %-20s %-10u\n", checkPtName, rvMemTraceMemOut);
	rvMutexUnlock(&rvMemTraceMutex);
}

void rvMemTraceStop(const char *checkPtName)
{
	rvMutexLock(&rvMemTraceMutex);
	if(rvMemTraceLog != NULL)
	{
		fprintf(rvMemTraceLog, "Stop  %-20s %-10d max: %-10d", checkPtName,
			rvMemTraceMemOut, rvMemTracePeakMem);
		if(!strcmp(checkPtName, rvMemTraceCheckPtName))
			fprintf(rvMemTraceLog, " diff: %-10d\n", rvMemTraceMemOut - rvMemTraceCheckPtStart);
		else
			fprintf(rvMemTraceLog, "\n");
	}
	rvMutexUnlock(&rvMemTraceMutex);
}

void rvMemTraceDestruct(void)
{
	rvMutexDestruct(&rvMemTraceMutex);

	if(rvMemTraceLog != NULL)
	{
		fprintf(rvMemTraceLog, "\nUnreleased memory: %d, Peak memory used: %d\n",
			rvMemTraceMemOut, rvMemTracePeakMem);
		fclose(rvMemTraceLog);
	}

	/* Dump unreleased blocks */
	if(rvMemTraceMemOut)
	{
		FILE *fp = fopen("rvmem_unreleased.log", "w+");
		if(fp != NULL)
		{
			RvMemTraceRecord *record;
			fprintf(fp, "Unreleased blocks:\n\n");
			for(record = rvMemTraceHead->next; record != NULL; record = record->next)
			{
				unsigned char *cp = (unsigned char *)(record + 1);
				const RvInt bytesPerLine = 16;
				RvInt lineStart;

				fprintf(fp, "Address = %p, size = %d\n", cp, record->size);
#ifdef RV_CALLSTACKTRACE_ON
				{
				RvInt i;
				fprintf(fp, "Call Stack =");
				for(i=0; i<RV_CALLSTACKTRACE_SIZE && record->callStack[i] != NULL; ++i)
					fprintf(fp, " %p", record->callStack[i]);
				fputc('\n', fp);
				}
#endif
				fprintf(fp, "Contents =\n");

				for(lineStart = 0; lineStart < record->size; lineStart += bytesPerLine)
				{
					RvInt lineEnd = lineStart + bytesPerLine, i;
					for(i = lineStart; i < lineEnd; ++i)
					{
						if(i < record->size)
							fprintf(fp, "%02X ", (RvInt)cp[i]);
						else
							fprintf(fp, "   ");
					}
					for(i = lineStart; i < lineEnd && i < record->size; ++i)
						fputc(isprint(cp[i]) ? cp[i] : '.', fp);
					fputc('\n', fp);
				}

				fputc('\n', fp);
			}
			fclose(fp);
		}
	}
}

#ifdef RV_CALLSTACKTRACE_ON

/* non-portable code to read call stack information */

void *getFirstFrame(void *addressOfFirstParam)
{
	return (void **)addressOfFirstParam - 2;
}

void *getNextFrame(void *frame)
{
	return *(void **)frame;
}

void *getReturnAddress(void *frame)
{
	return *((void **)frame + 1);
}

#endif

void rvMemTraceSetRecord(RvSizeT n, RvMemTraceRecord *record)
{
#ifdef RV_CALLSTACKTRACE_ON
	void *frame = getFirstFrame(&n);
	void **writePtr = record->callStack;
	void **endPtr = writePtr + RV_CALLSTACKTRACE_SIZE;

	/* skip 2 frames. rvMemAlloc_ & rvMemAlloc will always be the
	first two functions in the call stack, so we don't need to save
	them in the record. */
	frame = getNextFrame(frame);
	frame = getNextFrame(frame);

	while(frame != NULL && writePtr < endPtr)
	{
		*writePtr++ = getReturnAddress(frame);
		frame = getNextFrame(frame);
	}
#endif

	record->size = n;
	record->next = NULL;
	record->prev = rvMemTraceTail;
	rvMemTraceTail->next = record;
	rvMemTraceTail = record;
}

void *rvMemAlloc_(RvSizeT n)
{
	RvMemTraceRecord *record = (RvMemTraceRecord *)malloc(sizeof(RvMemTraceRecord) + n);

	rvMutexLock(&rvMemTraceMutex);
	rvMemTraceMemOut += n;
	if(rvMemTraceMemOut > rvMemTracePeakMem)
		rvMemTracePeakMem = rvMemTraceMemOut;
	rvMemTraceSetRecord(n, record);
	rvMutexUnlock(&rvMemTraceMutex);

	return record + 1;
}

void rvMemFree_(void* p)
{
	RvMemTraceRecord *record = (RvMemTraceRecord *)p - 1;

	rvMutexLock(&rvMemTraceMutex);

	rvMemTraceMemOut -= record->size;

	/* Free record from linked list */
	record->prev->next = record->next;
	if(record->next != NULL)
		record->next->prev = record->prev;
	else
		rvMemTraceTail = record->prev;

	free(record);

	rvMutexUnlock(&rvMemTraceMutex);
}

RvBool rvMemInit_(void)
{
	return rvTrue;
}

void rvMemEnd_(void)
{
}

RvInt rvMemTraceGetMemOut()
{
	return rvMemTraceMemOut;
}

RvInt rvMemTraceGetPeakMemUsage()
{
	return rvMemTracePeakMem;
}

#else

/* RV_MEMTRACE_ON not defined */
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#include <stdio.h>
#include "rvsem.h"

#endif
/* SPIRENT_END */

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#include "rvexternal.h"
#endif
/* SPIRENT_END */

#define rvMemAlloc_(n)      (malloc(n))
#define rvMemFree_(p)       (free(p))

RvBool rvMemInit_(void)
{
	return rvTrue;
}

void rvMemEnd_(void)
{
}

#endif

#elif defined(RV_MEM_OSE)
#include "ose.h"
#include "heapapi.h"

#define rvMemAlloc_(n)  (heap_alloc_shared((n), (__FILE__), (__LINE__)))
#define rvMemFree_(p)  (heap_free_shared(p))
RvBool rvMemInit_(void)
{
	return rvTrue;
}

void rvMemEnd_(void)
{
}
#elif defined(RV_MEM_NUCLEUS)
#include "nucleus.h"
extern NU_MEMORY_POOL System_Memory;

void *rvMemAlloc_(RvSizeT n)
{
	RvInt status;
	void *ptr;

	status = NU_Allocate_Memory(&System_Memory, &ptr, n, NU_NO_SUSPEND);
	if(status != NU_SUCCESS) return NULL;
	return ptr;
}

#define rvMemFree_(p)  (NU_Deallocate_Memory(p))

RvBool rvMemInit_(void)
{
	return rvTrue;
}

void rvMemEnd_(void)
{
}
#elif defined(RV_MEM_PSOS)
#include <psos.h>
#include "rvpool.h"
#include "rvalloc.h"

/* Which psos memory region to use */
#define PSOS_MEM_REGION 0

/* Smallest sub-allocation buffer pool to create. Must be a power */
/* of 2 >= 4 as per pSOS */
#define RVMEM_MINBUFSIZE 32

static RvPool *mempools;
static RvInt numpools;
static unsigned long region_unit_size; /* pSOS forces size to power of 2 >= 16 */
static RvAlloc pSOSalloc;

static void *rvMempSOSrnAlloc(void *rnid, RvSizeT size) {
	void *result;
	if(rn_getseg((unsigned long)rnid, size, RN_NOWAIT, 0, &result) != 0)
		return NULL;
	return result;
}

static void rvMempSOSrnFree(void *rnid, RvSizeT size, void *ptr){
	rn_retseg((unsigned long)rnid, ptr);
}

void *rvMemAlloc_(RvSizeT request_size)
{
	RvInt pool;
	RvInt *poolptr;
	unsigned long bufsize;
	RvSizeT required_size;
	char *result;

	required_size = request_size + sizeof(pool);
	bufsize = RVMEM_MINBUFSIZE;
	for(pool = 0; pool < numpools; pool++) {
		if(required_size <= bufsize) {
			result = (char *)rvPoolAllocate(&mempools[pool]);
			if(result == NULL)
				return NULL;
			poolptr = (RvInt *)result;
			*poolptr = pool; /* save pool number for memfree */
			result = result + sizeof(*poolptr);
			return (void *)result;
		}
		bufsize = bufsize * 2;
	}
	/* Go directly to pSOS memory manager */
	if(rn_getseg(PSOS_MEM_REGION, required_size, RN_NOWAIT, 0, &result) != 0)
		return NULL;
	poolptr = (RvInt *)result;
	*poolptr = -1; /* store -1 as pool number to indicate direct allocation */
	result = result + sizeof(*poolptr);
	return (void *)result;
}

void *rvMemFree_(void *p)
{
	RvInt *poolptr;
	char *result;

	if(p == NULL)
		return;
	result = (char *)p - sizeof(*poolptr);
	poolptr = (RvInt *)result;
	if(*poolptr == -1) {
		/* memory allocated directly from pSOS */
		rn_retseg(PSOS_MEM_REGION, result);
		return;
	}
	rvPoolDeallocate(&mempools[*poolptr], result);
}

RvBool rvMemInit_(void)
{
	struct rninfo rbuf;
	unsigned long bufsize;
	RvInt pool;

	mempools = NULL;
	numpools = 0;
	region_unit_size = 0;
	rvAllocConstruct(&pSOSalloc, PSOS_MEM_REGION, ~0U, rvMempSOSrnAlloc, rvMempSOSrnFree);

	if(rn_info(PSOS_MEM_REGION, &rbuf) != 0)
		return rvFalse;
	region_unit_size = rbuf.unit_size;

	/* figure out number of buffer pools to use */
	bufsize = region_unit_size;
	for(numpools = 0; bufsize > RVMEM_MINBUFSIZE; numpools++) {
		bufsize = bufsize / 2;
	}

	/* special case, we'll just send calls directly to pSOS */
	if(numpools == 0)
		return rvTrue;

	/* Initialize memory pools */
	if(rn_getseg(PSOS_MEM_REGION, numpools * sizeof(RvPool), RN_NOWAIT, 0, (void **)&mempools) != 0) {
		return rvFalse;
	}
	bufsize = RVMEM_MINBUFSIZE;
	for(pool = 0; pool < numpools; pool++) {
		rvPoolConstructEx1(&mempools[pool], bufsize, region_unit_size * 2, NULL, NULL, NULL, &pSOSalloc);
		bufsize = bufsize * 2;
	}
	return rvTrue;
}

void rvMemEnd_(void)
{
	RvInt pool;

	if(numpools == 0)
		return;
	for(pool = 0; pool < numpools; pool++)
		rvPoolDestruct(&mempools[pool]);
	numpools = 0;
	rn_retseg(PSOS_MEM_REGION, mempools);
	mempools = NULL;
}
#endif

static RvMemHandler rvMemHandler = NULL;

/*$
{function:
	{name: rvMemAlloc}
    {superpackage: RvUtil}
	{include: rvmem.h}
	{description:
		{p:
			Allocate memory from the "default" dynamic heap.  The "default"
			dynamic heap is the standard C library functions malloc/free
			on most OS's.
		}

	}
	{proto: void* rvMemAlloc(RvSizeT n);}
	{params:
		{param: {n: n} {d: The number of bytes to allocate.}}
	}
	{returns:
		A pointer to a suitably sized and aligned block of memory or
		NULL if memory exhaustion occurs.
	}
}
$*/

#define SITQ_MEMPOOL

#ifdef SITQ_MEMPOOL
/* Sitq was here                                            */
/* OK looks like malloc is too slow for us                  */
/* Scaramouche,scaramouche will you do the Fandango         */

// ---------------- Begining of TMemPool operations --------------------------

typedef enum 
{ 
   MEM_TYPE_32, 
   MEM_TYPE_127, 
   MEM_TYPE_255, 
   MEM_TYPE_511, 
   MEM_TYPE_1047, 
   MEM_TYPE_STANDARD,
   MEM_TYPE_TOTAL
} enumMemType;

typedef struct
{
   unsigned long MaGiC_Number;
   void * next;
} TMemHeader;

typedef struct
{
   unsigned char * aMem;
   void * prev;
} TMemRecord;

typedef struct 
{
   unsigned int MEM_INCREMENT;
   unsigned long MAGIC_NUMBER;
   unsigned int MEM_SIZE;

   TMemRecord * pMem;
   void * pHead;
   void * pTail;

   unsigned int mem_num  ;
   unsigned int max_alloc;
   unsigned int mem_alloc;
   unsigned int mem_allocs;
   unsigned int mem_frees;
} TMemPoolEach;

typedef struct 
{
   TMemPoolEach mem[MEM_TYPE_TOTAL];

   unsigned long MEM_MAGIC_BITS;
   unsigned long MEM_TYPE_BITS;
} TMemPool;

static TMemPool* memPool = NULL;

static RvSem   mem_alloc_semaphore;

static RvBool MemPoolInit(void)
{
   // Initialize memory pool for the first time.

   int ii;

   rvSemConstruct (&mem_alloc_semaphore, 1);

   memPool = (TMemPool*)malloc( sizeof(TMemPool) );
   memset( memPool, 0, sizeof(TMemPool) );

   memPool->mem[MEM_TYPE_32].MEM_INCREMENT   =   1000;
   memPool->mem[MEM_TYPE_127].MEM_INCREMENT  =   500;
   memPool->mem[MEM_TYPE_255].MEM_INCREMENT  =   500;
   memPool->mem[MEM_TYPE_511].MEM_INCREMENT  =   100;
   memPool->mem[MEM_TYPE_1047].MEM_INCREMENT =   100;
   memPool->mem[MEM_TYPE_STANDARD].MEM_INCREMENT = 0;

   memPool->mem[MEM_TYPE_32].MEM_SIZE        = sizeof(TMemHeader) + 32;
   memPool->mem[MEM_TYPE_127].MEM_SIZE       = sizeof(TMemHeader) + 128;
   memPool->mem[MEM_TYPE_255].MEM_SIZE       = sizeof(TMemHeader) + 256;
   memPool->mem[MEM_TYPE_511].MEM_SIZE       = sizeof(TMemHeader) + 512;
   memPool->mem[MEM_TYPE_1047].MEM_SIZE      = sizeof(TMemHeader) + 1048;
   memPool->mem[MEM_TYPE_STANDARD].MEM_SIZE  = 0;

   memPool->MEM_TYPE_BITS  = 0xFF;
   memPool->MEM_MAGIC_BITS = 0x89ABCD00;
   for( ii = 0; ii < MEM_TYPE_TOTAL; ii++ )
      memPool->mem[ii].MAGIC_NUMBER = memPool->MEM_MAGIC_BITS + ii;

   return rvTrue;
}

static RvBool MemPoolAllocMore( int type )
{
   // Allocate more memory for specified pool type. Increase count MEM_INCREMENT.  

   unsigned int ii;
   unsigned char* mPtr;
   unsigned char * allocPtr;
   TMemRecord * record;

   TMemPoolEach * ptr = &(memPool->mem[type]);

   // Allocate space for new memory and initialize it.
   allocPtr = (unsigned char*)malloc( ptr->MEM_INCREMENT * ptr->MEM_SIZE );
   if( !allocPtr )
   {
      printf("%s: Wrong memPool type %d\n", __FUNCTION__, type);
      return rvFalse;
   }

   if( (!ptr->pHead) && (!ptr->pTail) )
   {  // First time allocate memory. Initialize to proper value.
      ptr->pHead = allocPtr;
      ptr->pTail = allocPtr;
   }

   for( ii = 0; ii < ptr->MEM_INCREMENT; ii++ )
   {
      mPtr = allocPtr + ii * ptr->MEM_SIZE;
      ((TMemHeader*)mPtr)->MaGiC_Number = ptr->MAGIC_NUMBER;
      ((TMemHeader*)ptr->pTail)->next = (void *)mPtr;
      ptr->pTail = (void *)mPtr;
   }

   // Allocate space for pMem and link it properly for later destruction of memory pool.
   record = (TMemRecord *)malloc( sizeof(TMemRecord) ) ;
   if( !record )
   {
      printf( "%s: Error allocating memory for TMemRecord.\n", __FUNCTION__ ); 
      free( allocPtr );
      return rvFalse;
   }

   record->aMem = allocPtr;
   record->prev = (void *)ptr->pMem;
   ptr->pMem = record;

   ptr->mem_num += ptr->MEM_INCREMENT;

   return rvTrue;
}

static void* MemPoolAlloc(size_t n) 
{ 

   // Get pointer for size n. For standard, do malloc. Otherwise, get pointer from memory pool.

   void *allocated = NULL;
   int type;
   TMemPoolEach * ptr;

   if( !memPool )
   {
      printf( "%s: Null memPool\n", __FUNCTION__ );
      return allocated;
   }

   rvSemWait (&mem_alloc_semaphore);

   if ( n <= 31 ) // if n <= 32
      type = MEM_TYPE_32;
   else if ( !(n & (~127) ) ) // if n <= 127
      type = MEM_TYPE_127;
   else if ( !(n & (~255) ) ) // if n <= 255
      type = MEM_TYPE_255;
   else if ( !(n & (~511) ) ) // if n <= 511
      type = MEM_TYPE_511;
   else if ( n <= 1047 )  // if n <= 1047
      type = MEM_TYPE_1047;
   else
      type = MEM_TYPE_STANDARD;

   ptr = &(memPool->mem[type]);

   if( type == MEM_TYPE_STANDARD )
   {
             // allocate additional memory for TMemHeader
      allocated = (void*)malloc ( n + sizeof(TMemHeader) );
      ((TMemHeader*)allocated)->MaGiC_Number = ptr->MAGIC_NUMBER;
      allocated = (void*)((char *)allocated + sizeof(TMemHeader));

   }
   else    // Get pointer from memory pool.
   {
      if( ptr->pHead == ptr->pTail )
         if( !MemPoolAllocMore(type) )
         {
            rvSemPost (&mem_alloc_semaphore);
            return NULL;
         }

      allocated = (void*)((char *)ptr->pHead + sizeof(TMemHeader));
      ptr->pHead = ((TMemHeader*)ptr->pHead)->next;
   }

   ptr->mem_alloc++;
   ptr->mem_allocs++;
   if( ptr->mem_alloc > ptr->max_alloc )
       ptr->max_alloc = ptr->mem_alloc;

   rvSemPost (&mem_alloc_semaphore);

	return allocated;
}

static void MemPoolFree(void* p) 
{ 
   // Free the pointer. For standard, do free. Otherwise, return pointer to pool.
   
   unsigned long type = MEM_TYPE_STANDARD;
   TMemPoolEach* ptr;

   void * m_pointer = (void *)((char *)p - sizeof(TMemHeader));

   if( !memPool )
   {
      dtprintf( "%s: Null memPool\n", __FUNCTION__ );
      return;
   }

   rvSemWait (&mem_alloc_semaphore);

   // Find memory type.
   if( (((TMemHeader*)m_pointer)->MaGiC_Number & ~memPool->MEM_TYPE_BITS) == memPool->MEM_MAGIC_BITS ) 
   {
      type = ((TMemHeader*)m_pointer)->MaGiC_Number & memPool->MEM_TYPE_BITS;
      if( type >= MEM_TYPE_TOTAL ) 
         type = MEM_TYPE_STANDARD;
   }

   ptr = &(memPool->mem[type]);

   if( type == MEM_TYPE_STANDARD )
   {
      // free memory including TMemHeader
      free(m_pointer);
   }

   else 
   {  // return pointer to pool.
      if ( ptr->mem_alloc <= 0 )
         dtprintf("Assertion in MemPoolFree %s %d\n",__FILE__,__LINE__);

      ((TMemHeader *)ptr->pTail)->next = m_pointer;
      ptr->pTail = m_pointer;
   }

   ptr->mem_alloc--;
   ptr->mem_frees++;

   rvSemPost (&mem_alloc_semaphore);
}

static void MemPoolDestruct( void )
{
   // Free the whole memory pool.

   int ii;
   TMemRecord * record;
   TMemRecord * recordPrev;

/*
// check if memPool is all free
   {
      void MemPoolShow( void );
      MemPoolShow();
   }
*/
   if( memPool )
   {
      for( ii = 0; ii <  MEM_TYPE_TOTAL; ii++ )
      {
         record = memPool->mem[ii].pMem;

         while( record )
         {
            recordPrev = (TMemRecord *)record->prev;

            if( record->aMem ) free( record->aMem ); 
            free( record );

            record = recordPrev;
         }
      }

      free( memPool );
      memPool = NULL;
   }

   rvSemWait (&mem_alloc_semaphore);
   rvSemDestruct (&mem_alloc_semaphore);

}

static unsigned long MemPoolSize(void)
{  
   // Calculate total size of memory pool.

   int ii;
   unsigned long size = 0;
   TMemPoolEach * ptr;

   if( !memPool )
   {
      printf( "%s: Null memPool\n", __FUNCTION__ );
      return 0;
   }

   for( ii = 0; ii <  MEM_TYPE_TOTAL; ii++ )
   {
      ptr = &(memPool->mem[ii]);
      size += ptr->mem_num * ptr->MEM_SIZE;
   }

   return size;
}

void MemPoolShow( void )
{
   // Memory pool statistics.

   int ii;
   TMemPoolEach * ptr;

   if( !memPool )
   {
      printf( "%s: Null memPool\n", __FUNCTION__ );
      return;
   }

   printf(   "   size   total   alloc    free  max_usage    allocs     frees\n" );

   for( ii = 0; ii < MEM_TYPE_TOTAL - 1; ii++ )
   {
      ptr = &(memPool->mem[ii]);
      printf("  %5d  %6d  %6d  %6d  %6d    %9d %9d\n", 
         ptr->MEM_SIZE - sizeof(TMemHeader), ptr->mem_num, ptr->mem_alloc,
         ptr->mem_num - ptr->mem_alloc, ptr->max_alloc, ptr->mem_allocs, ptr->mem_frees );
   }

   ptr = &(memPool->mem[MEM_TYPE_STANDARD]);
   printf("standard     %11d    %11d    %9d %9d\n",
      ptr->mem_alloc, ptr->max_alloc, ptr->mem_allocs, ptr->mem_frees );
/*
// print a few unreleazed memory chunks
   {
      static void MemPoolPrint (int size, int bytes);
      MemPoolPrint(32,32);
   }
*/
}

static void MemPoolPrint (int size, int bytes)
{
   int numPrint;
   unsigned int base;
   unsigned int length;
   unsigned int i, j;
   int type;
   int toPrint;
   TMemPoolEach * ptr;
   TMemRecord * memPtr;
   void * freePtr;
   unsigned char * allocated;

   if( !memPool )
   {
      dtprintf( "%s: Null memPool\n", __FUNCTION__ );
      return;
   }

   if (size == 0) size = 32;
   length = (bytes > size || bytes == 0) ? size : bytes;

   switch (size)
   {
   case 32:    type = MEM_TYPE_32;  break;
   case 127:   type = MEM_TYPE_127; break;
   case 255:   type = MEM_TYPE_255; break;
   case 511:   type = MEM_TYPE_511; break;
   case 1047:  type = MEM_TYPE_1047;break;
   default:
               printf ("wrong size %d\n", size);
               return;
   }

   ptr = &(memPool->mem[type]);
   numPrint = ptr->mem_alloc;

   // Get the last memory page
   memPtr = ptr->pMem;
   base = ptr->mem_num - ptr->MEM_INCREMENT;

   while( memPtr )
   {
      for (i = 0; numPrint > 0 && i < ptr->MEM_INCREMENT; i++)
      {
         allocated = (unsigned char *)memPtr->aMem + i * ptr->MEM_SIZE;

         toPrint = 1;
         freePtr = ptr->pHead;
         while( freePtr != ptr->pTail )
         {
            if ( freePtr == allocated )
            {
               toPrint = 0; 
               break; 
            }

            freePtr = ((TMemHeader *)freePtr)->next;
         }
         // check the last element in freePtr list
         if( freePtr == ptr->pTail )
         {
            if ( freePtr == allocated )
               toPrint = 0;
         }

         if (toPrint)
         {
            printf ("size %d elem %d at 0x%x 0x:", size, base + i, (int)allocated);
            for (j = 0; j < length + sizeof(TMemHeader); j++)
            {
               int byte = *(allocated + j);
               if (byte >= ' ')
                  printf (" %02x[%c]", byte, byte);
               else
                  printf (" %02x", byte);
            }
            printf ("\n");
            --numPrint;
         }
      }

      memPtr = (TMemRecord *)memPtr->prev;
      base -= ptr->MEM_INCREMENT;
   }
}

// ---------------- End of TMemPool operations --------------------------

#endif

void sMemShow( void )
{
   // Memory pool statistics.

#ifdef SITQ_MEMPOOL
   MemPoolShow();
#endif

   return;
}
/*
void* rvMemAlloc(RvSizeT n) 
{ 
	void* p;

#ifdef SITQ_MEMPOOL
   p = MemPoolAlloc( n );
#else
	do {
		p = rvMemAlloc_(n);
	} while ((p == NULL) && (rvMemHandler) && (rvMemHandler(n)));
#endif

	return p;
}
*/
/*$
{function:
	{name: rvMemFree}
    {superpackage: RvUtil}
	{include: rvmem.h}
	{description:
		{p:
			Deallocate a block of memory pointed to by p.  The memory
			must have been previously allocated with rvMemAlloc.
		}
	}
	{proto: void rvMemFree(void* p);}
	{params:
		{param: {n: p} {d: A pointer to the memory block to deallocate.}}
	}
}
$*/
/*
void rvMemFree(void* p) 
{ 
#ifdef SITQ_MEMPOOL
   MemPoolFree( p );   
#else
   rvMemFree_(p); 
#endif
}
*/
/*
void* rvMemCalloc(RvSizeT num, RvSizeT size) {
	void* p;
	RvSizeT total;

	total = num * size;
	p = rvMemAlloc(total);
	if(p != NULL)
		memset(p, 0, total);
	return p;
}
*/
/*$
{function scope="protected":
	{name: rvMemInit}
    {superpackage: RvUtil}
	{include: rvmem.h}
	{description:
		{p:
			Initializes memory allocation routines. Must be called
			before any allocations.
		}
	}
	{proto: RvBool rvMemInit(void);}
	{returns:
	        rvTrue if successfull, rvFalse if initialization failed.
	}
}
$*/
/*
RvBool rvMemInit(void)
{
   printf("Memory manager initialized enter\n");

#ifdef SITQ_MEMPOOL
   printf("Memory manager initialized SITQ enter\n");
   MemPoolInit();
#endif
   printf("Memory manager initialized\n");
	return rvMemInit_();
}
*/
/*$
{function scope="protected":
	{name: rvMemEnd}
    {superpackage: RvUtil}
	{include: rvmem.h}
	{description:
		{p:
			Cleans up memory allocation routines. Must be called
			before system exits.
		}
	}
	{proto: void rvMemEnd(void);}
}
$*/
/*
void rvMemEnd(void)
{
#ifdef SITQ_MEMPOOL
   MemPoolDestruct();
#endif
   printf("Memory manager stopped\n");
	rvMemEnd_();
}
*/
/*$
{function:
	{name: rvMemGetHandler}
    {superpackage: RvUtil}
	{include: rvmem.h}
	{description:
		{p:
			Get the "handler" for the dynamic heap.  The handler is a function
			pointer, that is called when the heap is exhausted.  The handler
			can log the event, initiate a safe-shutdown, or take action to
			collect enough memory to satisfy the rvMemAlloc that caused the
			heap exhaustion.
		}
	}
	{proto: RvMemHandler rvMemGetHandler(void);}
	{returns:
		A pointer to the "handler" function or NULL if not set.
	}
    {see_also:
        {n: RvMemHandler}
    }
}
$*/
RvMemHandler rvMemGetHandler(void) {
	return rvMemHandler;
}

/*$
{function:
	{name: rvMemSetHandler}
    {superpackage: RvUtil}
	{include: rvmem.h}
	{description:
		{p:
			Set the "handler" for the dynamic heap.  The handler is a function
			pointer, that is called when the heap is exhausted.  The handler
			can log the event, initiate a safe-shutdown, or take action to
			collect enough memory to satisfy the rvMemAlloc that caused the
			heap exhaustion.
		}
	}
	{proto: RvMemHandler rvMemSetHandler(RvMemHandler h);}
	{params:
		{param: {n: h} {d: A pointer to the new "handler" function.}}
	}

	{returns:
		A pointer to the previous "handler" function or NULL if not set.
	}
    {see_also:
        {n: RvBool RvMemHandler(RvSizeT n);}
    }
}
$*/
RvMemHandler rvMemSetHandler(RvMemHandler h) {
	RvMemHandler old = rvMemHandler;
	rvMemHandler = h;
	return old;
}
