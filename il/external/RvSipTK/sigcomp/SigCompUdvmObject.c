/***********************************************************************
Filename   : SigCompUdvmObject.c
Description: Contains implementation of UDVM
************************************************************************
      Copyright (c) 2001,2002 RADVISION Inc. and RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Inc. and RADVISION Ltd.. No part of this document may be
reproduced in any form whatsoever without written prior approval by
RADVISION Inc. or RADVISION Ltd..

RADVISION Inc. and RADVISION Ltd. reserve the right to revise this
publication and make changes without obligation to notify any person of
such revisions or changes.
***********************************************************************/


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "SigCompUdvmObject.h"
#include "SigCompCommon.h"
#include "SigCompStateHandlerObject.h"
#include "SigCompMgrObject.h"
#include "rvassert.h"
#include "rvansi.h"

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/
#define MAX_FIXED_OPERANDS 7
#define MAX_OPERANDS       7

#define RV_SIGCOMP_UDVM_DEBUG 0

#if RV_SIGCOMP_UDVM_DEBUG
/* Useful for debugging, works only for single-threaded app. Global variable 
 * gLastPC 'remembers' previous value of program counter, gPC is set to the new value. 
 * Data breakpoint may be set on gPC or gLastPC.
 */
RvUint16 gPC = 0;
RvUint16 gLastPC = 0;
#define SETPC(x) gLastPC = gPC, gPC = (x)
#else
#define SETPC(x) x
#endif

/*lint -esym(578, pMem, memSize)*/

/*lint -esym(613, pSelfUdvm) pSelfUdvm (pointer to UDVM machine is never 0 */
/*lint -esym(613, pDecoder) pDecoder is never 0 */

/*
#define UDVM_INSTR_LOG 
*/
/*-----------------------------------------------------------------------*/
/*                           HELPER FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

#define HandleTraverseCopy(_pUdvm,_src,_pDest,_len,_pRv)\
{\
	RvUint8		*pMem = _pUdvm->pUdvmMemory; \
	UdvmAddr	 bcl;\
	UdvmAddr	 bcr;\
	*_pRv = RV_OK;\
	BCL(bcl);\
	BCR(bcr);\
	if (bcl < bcr &&\
		(RvUint32)(_src + _len)    < (RvUint32) bcr &&\
		(RvUint32)(*_pDest + _len) < (RvUint32) bcr)\
	{\
		optMemcpy(pMem+*_pDest,pMem+_src,_len);\
		*_pDest = (UdvmAddr) (*_pDest + _len);\
	}\
	else\
	{\
		CyclicBuffer cbDest;\
		CyclicBuffer cbSrc;\
		CyclicBufferConstruct(&cbSrc, (_pUdvm), _src, _len);\
		CyclicBufferConstruct(&cbDest, (_pUdvm), *_pDest, 0);\
		*_pRv = CyclicBufferTraverse(&cbSrc, NULL, &cbDest,UDVM_CYCLIC_BUF_COPY_TO_CYCLIC_TRAVERSER);\
		*_pDest = cbDest.start;\
	}\
}

#define CompareInstrMacro(_pSelfUdvm,_opcode,_pDecoder,_runningStatus,_rv) \
{\
    RvUint16 *operands  = (_pDecoder)->operands;\
    RvUint16 val1       = *operands++;\
    RvUint16 val2       = *operands++;\
    if(val1 < val2)\
    {\
        SETPC((_pSelfUdvm)->pc = operands[0]);\
    } else if(val1 == val2)\
    {\
        SETPC((_pSelfUdvm)->pc = operands[1]);\
    } else\
    {\
        SETPC((_pSelfUdvm)->pc = operands[2]);\
    }\
    _rv = RV_OK;\
}

#define CopyLiteralInstrMacro(_pSelfUdvm,_opcode,_pDecoder,_runningStatus,_rv) \
{\
    RvUint8  *pMem   = (_pSelfUdvm)->pUdvmMemory;\
    RvInt32  memSize = (RvInt32)(_pSelfUdvm)->udvmMemorySize;\
    RvUint16 *operands  = (_pDecoder)->operands;\
    UdvmAddr position   = *operands++;\
    RvUint16 length     = *operands++;\
    RvUint16 destinationRef = *operands++;\
    UdvmAddr destination;\
    CyclicBuffer cbDest;\
    CyclicBuffer cbSrc;\
    SAFE_READ16(destinationRef, destination);    \
    CyclicBufferConstruct(&cbSrc, (_pSelfUdvm), position, length);\
    CyclicBufferConstruct(&cbDest, (_pSelfUdvm), destination, 0);  \
    _rv = CyclicBufferTraverse(&cbSrc, NULL, &cbDest, UDVM_CYCLIC_BUF_COPY_TO_CYCLIC_TRAVERSER);\
    if(_rv == RV_OK)\
    {\
        SAFE_WRITE16(destinationRef, cbDest.start);\
        (_pSelfUdvm)->totalCycles += length;    \
    }\
}


#define OutputInstrMacro(_pSelfUdvm,_opcode,_pDecoder,_runningStatus,_rv) \
{\
    RvUint16     *operands  = (_pDecoder)->operands;\
    UdvmAddr     start      = operands[0];\
    UdvmAddr     length     = operands[1];\
    CyclicBuffer cb;\
    CyclicBufferConstruct(&cb, (_pSelfUdvm), start, length);\
    _rv = CyclicBufferTraverse(&cb, \
        NULL,\
        (_pSelfUdvm), UDVM_CYCLIC_BUF_WRITE_BYTES);    \
    if(_rv == RV_OK)\
    {\
        (_pSelfUdvm)->totalCycles += length;\
    }\
}
#define LoadInstrMacro(_pSelfUdvm,_opcode,_pDecoder,_runningStatus,_rv) \
{\
    RvUint8  *pMem   = (_pSelfUdvm)->pUdvmMemory;\
    RvInt32  memSize = (RvInt32)(_pSelfUdvm)->udvmMemorySize;\
    RvUint16 *operands = (_pDecoder)->operands;\
    RvUint16 addr = operands[0];\
    RvUint16 val = operands[1];\
    SAFE_WRITE16(addr, val);\
    _rv = RV_OK;\
} 

#define JumpInstrMacro(_pSelfUdvm,_opcode,_pDecoder,_runningStatus,_rv) \
{\
    SETPC((_pSelfUdvm)->pc = (_pDecoder)->operands[0]);\
    _rv = RV_OK;\
}

#define AndInstrMacro(_pSelfUdvm,_opcode,_pDecoder,_runningStatus,_rv) \
{\
    RvUint8  *pMem      = (_pSelfUdvm)->pUdvmMemory;\
    RvInt32  memSize    = (RvInt32)(_pSelfUdvm)->udvmMemorySize;\
    RvUint16 *operands  = (_pDecoder)->operands;\
    UdvmAddr op1Ref     = *operands++;\
    RvUint16 op2        = *operands++;\
    RvUint16 op1;\
    SAFE_READ16(op1Ref, op1);\
    op1 &= op2;\
    WRITE16(op1Ref, op1);\
    _rv = RV_OK;\
}

#define OrInstrMacro(_pSelfUdvm,_opcode,_pDecoder,_runningStatus,_rv) \
{\
    RvUint8  *pMem   = (_pSelfUdvm)->pUdvmMemory;\
    RvInt32  memSize = (RvInt32)(_pSelfUdvm)->udvmMemorySize;\
    RvUint16 *operands = (_pDecoder)->operands;\
    UdvmAddr op1Ref = *operands++;\
    RvUint16 op2 = *operands++;\
    RvUint16 op1;\
    SAFE_READ16(op1Ref, op1);\
    op1 |= op2;\
    WRITE16(op1Ref, op1);\
    _rv = RV_OK;\
}

#define NotInstrMacro(_pSelfUdvm,_opcode,_pDecoder,_runningStatus,_rv) \
{\
    RvUint8  *pMem   = (_pSelfUdvm)->pUdvmMemory;\
    RvInt32  memSize = (RvInt32)(_pSelfUdvm)->udvmMemorySize;\
    UdvmAddr op1Ref  = (_pDecoder)->operands[0];\
    RvUint16 op1;\
    SAFE_READ16(op1Ref, op1);\
    op1 = (RvUint16) (~op1);\
    WRITE16(op1Ref, op1);\
    _rv = RV_OK;\
}

#define LshiftInstrMacro(_pSelfUdvm,_opcode,_pDecoder,_runningStatus,_rv) \
{\
    RvUint8  *pMem      = (_pSelfUdvm)->pUdvmMemory;\
    RvInt32  memSize    = (RvInt32)(_pSelfUdvm)->udvmMemorySize;\
    RvUint16 *operands  = (_pDecoder)->operands;\
    UdvmAddr op1Ref     = *operands++;\
    RvUint16 op2        = *operands++;\
    RvUint16 op1;\
    SAFE_READ16(op1Ref, op1);\
    op1 <<= op2;\
    WRITE16(op1Ref, op1);\
    _rv = RV_OK;\
}

#define RshiftInstrMacro(_pSelfUdvm,_opcode,_pDecoder,_runningStatus,_rv) \
{\
    RvUint8  *pMem      = (_pSelfUdvm)->pUdvmMemory;\
    RvInt32  memSize    = (RvInt32)(_pSelfUdvm)->udvmMemorySize;\
    RvUint16 *operands  = (_pDecoder)->operands;\
    UdvmAddr op1Ref     = *operands++;\
    RvUint16 op2        = *operands++;\
    RvUint16 op1;\
    SAFE_READ16(op1Ref, op1);\
    op1 >>= op2;\
    WRITE16(op1Ref, op1);\
    _rv = RV_OK;\
}

#define AddInstrMacro(_pSelfUdvm,_opcode,_pDecoder,_runningStatus,_rv) \
{\
    RvUint8  *pMem      = (_pSelfUdvm)->pUdvmMemory;\
    RvInt32  memSize    = (RvInt32)(_pSelfUdvm)->udvmMemorySize;\
    RvUint16 *operands  = (_pDecoder)->operands;\
    UdvmAddr op1Ref     = *operands++;\
    RvUint16 op2        = *operands++;\
    RvUint16 op1;\
    SAFE_READ16(op1Ref, op1);\
    op1 = (RvUint16) (op1 + op2);\
    WRITE16(op1Ref, op1);\
    _rv = RV_OK;\
}
 

#define optMemcpy(_dst,_src,_len) \
    {\
	RvUint8* _d = _dst;\
	RvUint8* _s = _src;\
	RvUint   _l = _len;\
	switch (_l) {\
		case 8:\
		*(_d)++ = *(_s)++;\
		case 7:\
		*(_d)++ = *(_s)++;\
        case 6:\
		*(_d)++ = *(_s)++;\
        case 5:\
		*(_d)++ = *(_s)++;\
        case 4:\
		*(_d)++ = *(_s)++;\
        case 3:\
		*(_d)++ = *(_s)++;\
        case 2:\
		*(_d)++ = *(_s)++;\
        case 1:\
		*(_d)++ = *(_s)++;\
		break;\
        default:\
		while (_l--) { *(_d)++ = *(_s)++; }\
        }\
    }


#define UdvmReadBytes(_udvmMachine,_pBuf,_bufSize,_rv) \
{\
    SigCompPerUdvmBlock *_pub = RV_GET_STRUCT(SigCompPerUdvmBlock, udvmMachine, (_udvmMachine));\
    SigCompInputStream *_istream = &_pub->pDispatcher->inputStream;\
    if (_istream->pCur + (_bufSize) > _istream->pLast)\
        _rv = RV_ERROR_UNKNOWN;\
    else {\
        optMemcpy((_pBuf), _istream->pCur, (_bufSize));\
        _istream->pCur += (_bufSize);\
        _rv = RV_OK;\
    }\
}

#define UdvmWriteBytesMacro(_udvmMachine,_pBuf,_bufSize,_rv) \
{\
    SigCompPerUdvmBlock *_pub = RV_GET_STRUCT(SigCompPerUdvmBlock, udvmMachine, (_udvmMachine));\
    SigCompOutputStream *_ostream = &_pub->pDispatcher->outputStream;\
    if(_ostream->pCur + (_bufSize) > _ostream->outputArea + _ostream->outputAreaSize)\
    {\
        _rv = RV_ERROR_UNKNOWN;\
    }\
    else {\
        _ostream->bOutputEncountered = 1;\
        optMemcpy(_ostream->pCur, (_pBuf), (_bufSize));\
        _ostream->pCur += (_bufSize);\
        _rv = RV_OK;\
    }\
}

#define UdvmRequire(_udvmMachine,_reqSize,_rv) \
{\
    SigCompPerUdvmBlock *_pub = /*lint -e{613} */RV_GET_STRUCT(SigCompPerUdvmBlock, udvmMachine, (_udvmMachine));\
    SigCompInputStream *_istream = &_pub->pDispatcher->inputStream;\
    if(_istream->pCur + (_reqSize) > _istream->pLast)\
        _rv = RV_ERROR_UNKNOWN;\
    else\
        _rv = RV_OK;\
}

/*-----------------------------------------------------------------------*/
/*                           HELPER MACROS                               */
/*-----------------------------------------------------------------------*/

/* READ16, WRITE16, SAFE_READ16, SAFE_WRITE16, READ8, SAFE_READ8
 * This family of macros reads/writes UDVM memory
 * xxx16 macros manipulate 16-bits numbers,
 * xxx8  macros manipulate 8-bits numbers
 *
 * SAFExxx macros check memory boundaries and in case of memory access
 *  violation raises the appropriate flag and return with error from the calling
 *  function
 *
 * Precondition for using these macros:
 *  pMem should point to the start of UDVM memory,
 *  pSelfUdvm should point to the SigCompUdvm object
 */

#define READ16(addr,res) \
    (res) = (RvUint16)((*(pMem + (addr)) << 8) + *((pMem + (addr) + 1)));

#define WRITE16(addr, val) \
{\
    *(pMem + (addr)) = (RvUint8)((val) >> 8);\
    *(pMem + (addr) + 1) = (RvUint8)((val) & 0x00ff);\
}

#define SAFE_READ16(addr, lv) \
if((addr) + 1 >= memSize) { \
        pSelfUdvm->reason = UDVM_MEM_ACCESS;\
        return RV_ERROR_UNKNOWN; \
}\
READ16(addr,lv)


#define SAFE_WRITE16(addr, val) \
if((addr) + 1 >= memSize) { \
    pSelfUdvm->reason = UDVM_MEM_ACCESS; \
    return RV_ERROR_UNKNOWN; \
}\
WRITE16(addr, val)

#define READ8(addr) \
*(pMem + (addr))

#define SAFE_READ8(addr, lv) \
if(addr >= memSize) return RV_ERROR_UNKNOWN; \
lv = READ8(addr)


/* Maps UDVM address to pointer to the real memory */
#define ADDR_MEM(addr) (pMem + (addr))

#define DECODE_FINISHED 1000

/* Log macros 
 * Logging each and every Udvm instruction creates a lot of output, slowes down
 * process execution and clutters log files, so it should be used carefully
 */

#ifdef UDVM_INSTR_LOG
#define UdvmInstrLog(x, y) RvLogDebug(x, y)
#else
#define UdvmInstrLog(x, y)
#endif


/*-----------------------------------------------------------------------*/
/*                           MODULE STRUCTURES                           */
/*-----------------------------------------------------------------------*/
typedef RvUint16 UdvmAddr;

typedef enum
{
    UDVM_REFERENCE_OPERAND_DECODER = 0,
    UDVM_MULTITYPE_OPERAND_DECODER = 1,
    UDVM_ADDRESS_OPERAND_DECODER   = 2,
    UDVM_LITERAL_OPERAND_DECODER   = 3
                
} OperandDecoderEnum;

typedef enum 
{
    UDVM_CYCLIC_BUF_COPY_FROM_LINEAR_TRAVERSER	= 0,
    UDVM_CYCLIC_BUF_SHA1_TRAVERSER				= 1,
    UDVM_CYCLIC_BUF_COMPARE_STATES_TRAVERSER	= 2,
    UDVM_CYCLIC_BUF_COPY_TO_LINEAR_TRAVERSER	= 3,
    UDVM_CYCLIC_BUF_COPY_TO_CYCLIC_TRAVERSER	= 4,
    UDVM_CYCLIC_BUF_WRITE_BYTES					= 5,
    UDVM_CYCLIC_BUF_MEMSET_TRAVERSER			= 6,
    UDVM_CYCLIC_BUF_CRC_TRAVERSER				= 7,
    UDVM_CYCLIC_BUF_INPUT_BYTES_TRAVERSER		= 8,
    UDVM_CYCLIC_BUF_TEST_TRAVERSER				= 9
} CyclicBufferCallbackEnum;


/* UDVM opcodes */
typedef enum
{
    DECOMPRESSION_FAILURE = 0,  /* 1 */
    AND = 1,                    /* 1 */
    OR = 2,                     /* 1 */
    NOT = 3,                    /* 1 */
    LSHIFT = 4,                 /* 1 */
    RSHIFT = 5,                 /* 1 */
    ADD = 6,                    /* 1 */
    SUBTRACT = 7,               /* 1 */
    MULTIPLY = 8,               /* 1 */
    DIVIDE = 9,                 /* 1 */
    REMAINDER = 10,             /* 1 */
    SORT_ASCENDING = 11,        /* 1 + k * (ceiling(log2(k)) + n) */
    SORT_DESCENDING = 12,       /* 1 + k * (ceiling(log2(k)) + n) */
    SHA_1 = 13,                 /* 1 + length */
    LOAD = 14,                  /* 1 */
    MULTILOAD = 15,             /* 1 + n */
    PUSH = 16,                  /* 1 */
    POP = 17,                   /* 1 */
    COPY = 18,                  /* 1 + length */
    COPY_LITERAL = 19,          /* 1 + length */
    COPY_OFFSET = 20,           /* 1 + length */
    MEMSET = 21,                /* 1 + length */
    JUMP = 22,                  /* 1 */
    COMPARE = 23,               /* 1 */
    CALL = 24,                  /* 1 */
    RETURN = 25,                /* 1 */
    SWITCH = 26,                /* 1 + n */
    CRC = 27,                   /* 1 + length */
    INPUT_BYTES = 28,           /* 1 + length */
    INPUT_BITS = 29,            /* 1 */
    INPUT_HUFFMAN = 30,         /* 1 + n */
    STATE_ACCESS = 31,          /* 1 + state_length */
    STATE_CREATE = 32,          /* 1 + state_length */
    STATE_FREE = 33,            /* 1 */
    OUTPUT = 34,                /* 1 + output_length */
    END_MESSAGE = 35           /* 1 + state_length */

#ifdef UDVM_DEBUG_SUPPORT
    /* Special debugging support commands */
    , BREAK,
    BREAKPOINT,
    WATCHPOINT,
    TRACE
#endif
} UdvmOpcode;

/**********************************************************************
* OperandsDecoder
* ------------------------------------------------------------------------
* Description:
*   Operands decoder object fields.
*   Operands decoder is responsible for decoding operands for given
*   instruction.
*************************************************************************/

typedef struct
{
    RvUint16 operands[MAX_OPERANDS]; /* decoded operands */
    RvUint16 nVarGroupsNumber;       /* number of variable operands groups */

    /* points to the instruction description */
    struct _UdvmInstructionDescription *instrDescr;

    UdvmAddr nextOperandAddr;
} OperandsDecoder;



typedef RvStatus(*OperandDecoder) (IN  SigCompUdvm *pSelfUdvm,
                                   IN  UdvmAddr    addr,
                                   OUT RvUint16   *pOpSize,
                                   OUT RvUint16   *pRes);

typedef RvStatus(*Instruction) (IN  SigCompUdvm       *pSelfUdvm,
                                IN  UdvmOpcode        opcode,
                                IN  OperandsDecoder   *pDecoder,
                                OUT SigCompUdvmStatus *pRunningStatus);

/**********************************************************************
* UdvmInstructionDescription
* ------------------------------------------------------------------------
* Description:
* Describes single Udvm instruction
* some fields in this structure (opcode, instr, instrName)
* are initialized manually, others are computed by InitUdvmInstructions function
*
*
*************************************************************************/

typedef struct _UdvmInstructionDescription
{
    UdvmOpcode opcode;
#if defined(UDVM_DEBUG_SUPPORT) || (RV_LOGMASK != RV_LOGLEVEL_NONE)
    RvChar instrName[30];
#endif
    Instruction instr;          /* pointer to the function that implements this
                                 * instruction
                                 */

    RvChar *opspec;         /* operands specification codes */

    /* instuction is flow control instruction: JUMP,
     * CALL, etc
     * Some instuctions such as various input instructions
     * also considered as flow control
     */
    RvBool bFlowControl;

    /* number of fixed operands in the instruction */
    RvUint nFixedOperands;

    /* variable operands are organized in groups.
     * This is the number of operands in each group
     */
    RvUint nVarOperands;

    /* each entry in this array holds a pointer to the appropriate decoder
     * function. Each decoder function knows how to decode operand of some type.
     * First 'nFixedOperands' entries in this array are allocated to the
     *  decoders for fixed operands.
     * Last 'nVarOperands' entries are allocated for variable operands decoders
     */
    OperandDecoderEnum opDecoders[MAX_FIXED_OPERANDS];
} UdvmInstructionDescription;

/**********************************************************************
* CyclicBuffer
* ------------------------------------------------------------------------
* Description:
*   CyclicBuffer object
*   There are quite a few various operations that have to obey byte coping
*   rules. Cyclic buffer provides infrastructure for implementing these
*   instructions by the means of 'visitor' pattern.
*   Cyclic buffer is first constructed and then traversed using externally
*   provided traverser callback function
*************************************************************************/

typedef struct
{
    /*
     *  Saved byte_copy_left and byte_copy_right registers
     */
    UdvmAddr bcl;
    UdvmAddr bcr;

    /*
     *  Start address of the traversal
     */
    UdvmAddr start;
    /*
     *  Number of bytes to traverse
     */
    RvUint16 nBytes;
    /*
     *  Udvm memory size
     */
    RvUint32 memSize;
    /*
     *  Pointer to the UDVM memory
     */
    RvUint8 *pMem;
} CyclicBuffer;




#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

/* Not every instruction uses all arguments, so this macro
 * should be used to avoid 'unused argument' warnings in instructions
 * implementation
 */

#define USE_INSTR_ARGS RV_UNUSED_ARG(pSelfUdvm); RV_UNUSED_ARG(opcode);\
 RV_UNUSED_ARG(pDecoder); RV_UNUSED_ARG(runningStatus)

static RvStatus DecompressionFailureInstr(SigCompUdvm       *pSelfUdvm,
                                          UdvmOpcode        opcode,
                                          OperandsDecoder   *pDecoder,
                                          SigCompUdvmStatus *runningStatus);

static RvStatus AndInstr(SigCompUdvm        *pSelfUdvm,
                         UdvmOpcode         opcode,
                         OperandsDecoder    *pDecoder,
                         SigCompUdvmStatus  *runningStatus);

static RvStatus OrInstr(SigCompUdvm         *pSelfUdvm,
                        UdvmOpcode          opcode,
                        OperandsDecoder     *pDecoder,
                        SigCompUdvmStatus   *runningStatus);

static RvStatus NotInstr(SigCompUdvm        *pSelfUdvm,
                         UdvmOpcode         opcode,
                         OperandsDecoder    *pDecoder,
                         SigCompUdvmStatus  *runningStatus);

static RvStatus LshiftInstr(SigCompUdvm       *pSelfUdvm,
                            UdvmOpcode        opcode,
                            OperandsDecoder   *pDecoder,
                            SigCompUdvmStatus *runningStatus);

static RvStatus RshiftInstr(SigCompUdvm       *pSelfUdvm,
                            UdvmOpcode        opcode,
                            OperandsDecoder   *pDecoder,
                            SigCompUdvmStatus *runningStatus);

static RvStatus AddInstr(SigCompUdvm       *pSelfUdvm,
                         UdvmOpcode        opcode,
                         OperandsDecoder   *pDecoder,
                         SigCompUdvmStatus *runningStatus);

static RvStatus SubtractInstr(SigCompUdvm       *pSelfUdvm,
                              UdvmOpcode        opcode,
                              OperandsDecoder   *pDecoder,
                              SigCompUdvmStatus *runningStatus);

static RvStatus MultiplyInstr(SigCompUdvm       *pSelfUdvm,
                              UdvmOpcode        opcode,
                              OperandsDecoder   *pDecoder,
                              SigCompUdvmStatus *runningStatus);

static RvStatus DivideInstr(SigCompUdvm         *pSelfUdvm,
                            UdvmOpcode          opcode,
                            OperandsDecoder     *pDecoder,
                            SigCompUdvmStatus   *runningStatus);

static RvStatus RemainderInstr(SigCompUdvm       *pSelfUdvm,
                               UdvmOpcode        opcode,
                               OperandsDecoder   *pDecoder,
                               SigCompUdvmStatus *runningStatus);

static RvStatus SortInstr(SigCompUdvm       *pSelfUdvm,
                          UdvmOpcode        opcode,
                          OperandsDecoder   *pDecoder,
                          SigCompUdvmStatus *runningStatus);

static RvStatus Sha1Instr(SigCompUdvm       *pSelfUdvm,
                          UdvmOpcode        opcode,
                          OperandsDecoder   *pDecoder,
                          SigCompUdvmStatus *runningStatus);

static RvStatus LoadInstr(SigCompUdvm       *pSelfUdvm,
                          UdvmOpcode        opcode,
                          OperandsDecoder   *pDecoder,
                          SigCompUdvmStatus *runningStatus);

static RvStatus MultiloadInstr(SigCompUdvm       *pSelfUdvm,
                               UdvmOpcode        opcode,
                               OperandsDecoder   *pDecoder,
                               SigCompUdvmStatus *runningStatus);

static RvStatus PushInstr(SigCompUdvm       *pSelfUdvm,
                          UdvmOpcode        opcode,
                          OperandsDecoder   *pDecoder,
                          SigCompUdvmStatus *runningStatus);

static RvStatus PopInstr(SigCompUdvm        *pSelfUdvm,
                         UdvmOpcode         opcode,
                         OperandsDecoder    *pDecoder,
                         SigCompUdvmStatus  *runningStatus);

static RvStatus CopyInstr(SigCompUdvm       *pSelfUdvm,
                          UdvmOpcode        opcode,
                          OperandsDecoder   *pDecoder,
                          SigCompUdvmStatus *runningStatus);

static RvStatus CopyLiteralInstr(SigCompUdvm        *pSelfUdvm,
                                 UdvmOpcode         opcode,
                                 OperandsDecoder    *pDecoder,
                                 SigCompUdvmStatus  *runningStatus);

static RvStatus CopyOffsetInstr(SigCompUdvm         *pSelfUdvm,
                                UdvmOpcode          opcode,
                                OperandsDecoder     *pDecoder,
                                SigCompUdvmStatus   *runningStatus);

static RvStatus MemsetInstr(SigCompUdvm         *pSelfUdvm,
                            UdvmOpcode          opcode,
                            OperandsDecoder     *pDecoder,
                            SigCompUdvmStatus   *runningStatus);

static RvStatus JumpInstr(SigCompUdvm       *pSelfUdvm,
                          UdvmOpcode        opcode,
                          OperandsDecoder   *pDecoder,
                          SigCompUdvmStatus *runningStatus);

static RvStatus CompareInstr(SigCompUdvm        *pSelfUdvm,
                             UdvmOpcode         opcode,
                             OperandsDecoder    *pDecoder,
                             SigCompUdvmStatus  *runningStatus);

static RvStatus CallInstr(SigCompUdvm       *pSelfUdvm,
                          UdvmOpcode        opcode,
                          OperandsDecoder   *pDecoder,
                          SigCompUdvmStatus *runningStatus);

static RvStatus ReturnInstr(SigCompUdvm         *pSelfUdvm,
                            UdvmOpcode          opcode,
                            OperandsDecoder     *pDecoder,
                            SigCompUdvmStatus   *runningStatus);

static RvStatus SwitchInstr(SigCompUdvm         *pSelfUdvm,
                            UdvmOpcode          opcode,
                            OperandsDecoder     *pDecoder,
                            SigCompUdvmStatus   *runningStatus);

static RvStatus CrcInstr(SigCompUdvm        *pSelfUdvm,
                         UdvmOpcode         opcode,
                         OperandsDecoder    *pDecoder,
                         SigCompUdvmStatus  *runningStatus);

static RvStatus InputBytesInstr(SigCompUdvm         *pSelfUdvm,
                                UdvmOpcode          opcode,
                                OperandsDecoder     *pDecoder,
                                SigCompUdvmStatus   *runningStatus);

static RvStatus InputBitsInstr(SigCompUdvm       *pSelfUdvm,
                               UdvmOpcode        opcode,
                               OperandsDecoder   *pDecoder,
                               SigCompUdvmStatus *runningStatus);

static RvStatus InputHuffmanInstr(SigCompUdvm       *pSelfUdvm,
                                  UdvmOpcode        opcode,
                                  OperandsDecoder   *pDecoder,
                                  SigCompUdvmStatus *runningStatus);

static RvStatus StateAccessInstr(SigCompUdvm        *pSelfUdvm,
                                 UdvmOpcode         opcode,
                                 OperandsDecoder    *pDecoder,
                                 SigCompUdvmStatus  *runningStatus);

static RvStatus StateCreateInstr(SigCompUdvm        *pSelfUdvm,
                                 UdvmOpcode         opcode,
                                 OperandsDecoder    *pDecoder,
                                 SigCompUdvmStatus  *runningStatus);

static RvStatus StateFreeInstr(SigCompUdvm       *pSelfUdvm,
                               UdvmOpcode        opcode,
                               OperandsDecoder   *pDecoder,
                               SigCompUdvmStatus *runningStatus);

static RvStatus OutputInstr(SigCompUdvm         *pSelfUdvm,
                            UdvmOpcode          opcode,
                            OperandsDecoder     *pDecoder,
                            SigCompUdvmStatus   *runningStatus);

static RvStatus EndMessageInstr(SigCompUdvm         *pSelfUdvm,
                                UdvmOpcode          opcode,
                                OperandsDecoder     *pDecoder,
                                SigCompUdvmStatus   *runningStatus);

#ifdef UDVM_DEBUG_SUPPORT

static RvStatus BreakInstr(SigCompUdvm       *pSelfUdvm,
                           UdvmOpcode        opcode,
                           OperandsDecoder   *pDecoder,
                           SigCompUdvmStatus *runningStatus);

static RvStatus BreakpointInstr(SigCompUdvm         *pSelfUdvm,
                                UdvmOpcode          opcode,
                                OperandsDecoder     *pDecoder,
                                SigCompUdvmStatus   *runningStatus);

static RvStatus TraceInstr(SigCompUdvm       *pSelfUdvm,
                           UdvmOpcode        opcode,
                           OperandsDecoder   *pDecoder,
                           SigCompUdvmStatus *runningStatus);

static RvStatus WatchpointInstr(SigCompUdvm         *pSelfUdvm,
                                UdvmOpcode          opcode,
                                OperandsDecoder     *pDecoder,
                                SigCompUdvmStatus   *runningStatus);

static void TreatWatchpoint(SigCompUdvm           *pSelfUdvm,
                            SigCompUdvmWatchpoint *pWatch);

static void DefaultOutputDebugString(void       *cookie,
                                     const RvChar *msg);

#endif /* UDVM_DEBUG_SUPPORT */

static void FillPostmortem(SigCompUdvm           *pSelfUdvm,
                           SigCompUdvmPostmortem *pPostmortem);


static RvStatus ExecuteInstruction(SigCompUdvm       *pSelfUdvm,
                                   SigCompUdvmStatus *pRunningStatus);

static RvStatus InitMachine(IN SigCompUdvm        *pSelfUdvm,
                            IN RvSigCompMgrHandle hMgr,
                            IN SigCompUdvmParameters *pParameters,
                            IN RvUint32           partialStateIdLen,
                            IN RvUint16           stateLen,
                            IN RvUint8            *pCode,
                            IN RvUint16           codeLen,
                            UdvmAddr              codeDestination,
                            UdvmAddr              startAddr);

static void UdvmForwardStateRequests(
    IN SigCompUdvm                  *pSelfUdvm,
    IN SigCompCompartmentHandlerMgr *pCompartmentHandler,
    IN SigCompCompartment           *pCompartment);

static void UdvmForwardRequestedFeedback(
    IN SigCompUdvm                  *pSelfUdvm,
    IN SigCompCompartmentHandlerMgr *pCompartmentHandler,
    IN SigCompCompartment           *pCompartment);

static void UdvmForwardReturnedParameters(
    IN SigCompUdvm                *pSelfUdvm,
    IN SigCompCompartmentHandlerMgr *pCompartmentHandler,
    IN SigCompCompartment *pCompartment);



#define CopyToLinearTraverserMacro(_cookie,_mem,_size,_rv) \
{\
    RvUint8 **ppDest = (RvUint8 **) (_cookie);\
    optMemcpy(*ppDest, (_mem), (_size));\
    *ppDest += (_size);\
    _rv = RV_OK;\
}


#define CopyFromLinearTraverserMacro(_cookie,_mem,_size,_rv) \
{\
    RvUint8 **_ppDest = (RvUint8 **) (_cookie);\
    optMemcpy((_mem), *_ppDest, (_size));\
    *_ppDest += (_size);\
    _rv = RV_OK;\
}


#define CopyToCyclicTraverserMacro(_cookie,_mem,_size,_rv) \
{\
    CyclicBuffer *_pCb = (CyclicBuffer *) (_cookie);\
    _pCb->nBytes = (RvUint16) (_pCb->nBytes + (_size));\
    _rv = CyclicBufferTraverse(_pCb, NULL, &(_mem), UDVM_CYCLIC_BUF_COPY_FROM_LINEAR_TRAVERSER);\
}

#define InputBytesTraverserMacro(_cookie,_buf,_bufsize,_rv) \
	UdvmReadBytes((SigCompUdvm *) (_cookie), (_buf), (_bufsize),(_rv))



static void InitUdvmInstructions(void);

static void InitReverseBits(void);

#if !defined(UDVM_DEBUG_SUPPORT) && (RV_LOGMASK == RV_LOGLEVEL_NONE)

/* Inhibit 'type mismatch' warning for operand decoder enumeration */
#define DEFOP(instr, func, spec) /*lint -e{64}*/{instr, func, spec, 0, 0, 0, {0}}

/* Flow control instruction */
#define FDEFOP(instr, func, spec) /*lint -e{64}*/{instr, func, spec, 1, 0, 0, {0}}

#else /* UDVM_DEBUG_SUPPORT defined */

#define DEFOP(instr, func, spec) /*lint -e{64}*/{instr, #instr, func, spec, 0, 0, 0, {0}}

/* Flow control instruction */
#define FDEFOP(instr, func, spec) /*lint -e{64}*/{instr, #instr, func, spec, 1, 0, 0, {0}}

#endif /* UDVM_DEBUG_SUPPORT */

/*
 *  Static array holding instruction descriptions for each UDVM instruction
 *  Array is indexed by UDVM opcode, e.g. if 'opcode' is UDVM opcode then
 *  gUdvmInstructions[opcode] holds description for this instruction
 */
static UdvmInstructionDescription gUdvmInstructions[] = {
    /* 00 */ DEFOP(DECOMPRESSION_FAILURE, DecompressionFailureInstr, ""),
    /* 01 */ DEFOP(AND, AndInstr, "$%"),
    /* 02 */ DEFOP(OR, OrInstr, "$%"),
    /* 03 */ DEFOP(NOT, NotInstr, "$"),
    /* 04 */ DEFOP(LSHIFT, LshiftInstr, "$%"),
    /* 05 */ DEFOP(RSHIFT, RshiftInstr, "$%"),
    /* 06 */ DEFOP(ADD, AddInstr, "$%"),
    /* 07 */ DEFOP(SUBTRACT, SubtractInstr, "$%"),
    /* 08 */ DEFOP(MULTIPLY, MultiplyInstr, "$%"),
    /* 09 */ DEFOP(DIVIDE, DivideInstr, "$%"),
    /* 10 */ DEFOP(REMAINDER, RemainderInstr, "$%"),
    /* 11 */ DEFOP(SORT_ASCENDING, SortInstr, "%%%"),
    /* 12 */ DEFOP(SORT_DESCENDING, SortInstr, "%%%"),
    /* 13 */ DEFOP(SHA_1, Sha1Instr, "%%%"),
    /* 14 */ DEFOP(LOAD, LoadInstr, "%%"),
    /* 15 */ DEFOP(MULTILOAD, MultiloadInstr, "%#;%"),
    /* 16 */ DEFOP(PUSH, PushInstr, "%"),
    /* 17 */ DEFOP(POP, PopInstr, "%"),
    /* 18 */ DEFOP(COPY, CopyInstr, "%%%"),
    /* 19 */ DEFOP(COPY_LITERAL, CopyLiteralInstr, "%%$"),
    /* 20 */ DEFOP(COPY_OFFSET, CopyOffsetInstr, "%%$"),
    /* 21 */ DEFOP(MEMSET, MemsetInstr, "%%%%"),
    /* 22 */ FDEFOP(JUMP, JumpInstr, "@"),
    /* 23 */ FDEFOP(COMPARE, CompareInstr, "%%@@@"),
    /* 24 */ FDEFOP(CALL, CallInstr, "@"),
    /* 25 */ FDEFOP(RETURN, ReturnInstr, ""),
    /* 26 */ FDEFOP(SWITCH, SwitchInstr, "#%;@"),
    /* 27 */ FDEFOP(CRC, CrcInstr, "%%%@"),
    /* 28 */ FDEFOP(INPUT_BYTES, InputBytesInstr, "%%@"),
    /* 29 */ FDEFOP(INPUT_BITS, InputBitsInstr, "%%@"),
    /* 30 */ FDEFOP(INPUT_HUFFMAN, InputHuffmanInstr, "%@#;%%%%"),
    /* 31 */ FDEFOP(STATE_ACCESS, StateAccessInstr, "%%%%%%"),
    /* 32 */ DEFOP(STATE_CREATE, StateCreateInstr, "%%%%%"),
    /* 33 */ DEFOP(STATE_FREE, StateFreeInstr, "%%"),
    /* 34 */ DEFOP(OUTPUT, OutputInstr, "%%"),
    /* 35 */ FDEFOP(END_MESSAGE, EndMessageInstr, "%%%%%%%"),
    
#ifdef UDVM_DEBUG_SUPPORT
    /* 36 */ DEFOP(BREAK, BreakInstr, ""),
    /* 37 */ DEFOP(BREAKPOINT, BreakpointInstr, "#;%"),
    /* 38 */ DEFOP(WATCHPOINT, WatchpointInstr, "#;%%%%"),
    /* 39 */ DEFOP(TRACE, TraceInstr, "%")
#endif                          /* UDVM_DEBUG_SUPPORT */
};

/* indicates whether UDVM instruction description array was initialized */
static RvBool gUdvmInstructionsInited = 0;
RvUint8 gReversedBytes[256];      /* table of reversed bits bytes, e.g at gReverseBytes[i] is reversebits(i) */

const UdvmAddr cUDVMMemorySizeAddr       = 0;
const UdvmAddr cCyclesPerBitAddr         = 2;
const UdvmAddr cSigCompVersionAddr       = 4;
const UdvmAddr cPartialStateIdLengthAddr = 6;
const UdvmAddr cStateLengthAddr          = 8;
const UdvmAddr cReservedAreaStartAddr    = 10;
const RvUint16 cReservedAreaLength       = 22;


/*
 *   0             7 8            15
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |        byte_copy_left         |  64 - 65
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |        byte_copy_right        |  66 - 67
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |        input_bit_order        |  68 - 69
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |        stack_location         |  70 - 71
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

const UdvmAddr rBCL = 64;
const UdvmAddr rBCR = 66;
const UdvmAddr rIBO = 68;
const UdvmAddr rSP  = 70;

#ifdef UDVM_INSTR_LOG
static RvChar *gOperandsFormat[] = 
{
    /* DECOMPRESSION-FAILURE operands */
    /* AND operands */
    "    operand_1 = %hu",
    "    operand_2 = %hu",
    /* OR operands */
    "    operand_1 = %hu",
    "    operand_2 = %hu",
    /* NOT operands */
    "    operand_1 = %hu",
    /* LSHIFT operands */
    "    operand_1 = %hu",
    "    operand_2 = %hu",
    /* RSHIFT operands */
    "    operand_1 = %hu",
    "    operand_2 = %hu",
    /* ADD operands */
    "    operand_1 = %hu",
    "    operand_2 = %hu",
    /* SUBTRACT operands */
    "    operand_1 = %hu",
    "    operand_2 = %hu",
    /* MULTIPLY operands */
    "    operand_1 = %hu",
    "    operand_2 = %hu",
    /* DIVIDE operands */
    "    operand_1 = %hu",
    "    operand_2 = %hu",
    /* REMAINDER operands */
    "    operand_1 = %hu",
    "    operand_2 = %hu",
    /* SORT-ASCENDING operands */
    "    start = %hu",
    "    n = %hu",
    "    k = %hu",
    /* SORT-DESCENDING operands */
    "    start = %hu",
    "    n = %hu",
    "    k = %hu",
    /* SHA-1 operands */
    "    position = %hu",
    "    length = %hu",
    "    destination = %hu",
    /* LOAD operands */
    "    address = %hu",
    "    value = %hu",
    /* MULTILOAD operands */
    "    address = %hu",
    "    n = %hu",
    "    value = %hu",
    /* PUSH operands */
    "    value = %hu",
    /* POP operands */
    "    address = %hu",
    /* COPY operands */
    "    position = %hu",
    "    length = %hu",
    "    destination = %hu",
    /* COPY-LITERAL operands */
    "    position = %hu",
    "    length = %hu",
    "    destination = %hu",
    /* COPY-OFFSET operands */
    "    offset = %hu",
    "    length = %hu",
    "    destination = %hu",
    /* MEMSET operands */
    "    address = %hu",
    "    length = %hu",
    "    start_value = %hu",
    "    offset = %hu",
    /* JUMP operands */
    "    address = %hu",
    /* COMPARE operands */
    "    value_1 = %hu",
    "    value_2 = %hu",
    "    address_1 = %hu",
    "    address_2 = %hu",
    "    address_3 = %hu",
    /* CALL operands */
    "    address = %hu",
    /* SWITCH operands */
    "    n = %hu",
    "    j = %hu",
    "    address = %hu",
    /* CRC operands */
    "    value = %hu",
    "    position = %hu",
    "    length = %hu",
    "    address = %hu",
    /* INPUT-BYTES operands */
    "    length = %hu",
    "    destination = %hu",
    "    address = %hu",
    /* INPUT-BITS operands */
    "    length = %hu",
    "    destination = %hu",
    "    address = %hu",
    /* INPUT-HUFFMAN operands */
    "    destination = %hu",
    "    address = %hu",
    "    n = %hu",
    "    bits = %hu",
    "    lower_bound = %hu",
    "    upper_bound = %hu",
    "    uncompressed = %hu",
    /* STATE-ACCESS operands */
    "    partial_identifier_start = %hu",
    "    partial_identifier_length = %hu",
    "    state_begin = %hu",
    "    state_length = %hu",
    "    state_address = %hu",
    "    state_instruction = %hu",
    /* STATE-CREATE operands */
    "    state_length = %hu",
    "    state_address = %hu",
    "    state_instruction = %hu",
    "    minimum_access_length = %hu",
    "    state_retention_priority = %hu",
    /* STATE-FREE operands */
    "    partial_identifier_start = %hu",
    "    partial_identifier_length = %hu",
    /* OUTPUT operands */
    "    output_start = %hu",
    "    output_length = %hu",
    /* END-MESSAGE operands */
    "    requested_feedback_location = %hu",
    "    returned_parameters_location = %hu",
    "    state_length = %hu",
    "    state_address = %hu",
    "    state_instruction = %hu",
    "    minimum_access_length = %hu",
    "    state_retention_priority = %hu"
};

RvUint gOperandFormatOffsets[] = {
    0, 0, 2, 4, 5, 7, 9, 11, 13, 15, 17, 19, 22, 25, 28, 30, 33, 34, 35,
    38, 41, 44, 48, 49, 54, 55, 55, 58, 62, 65, 68, 75, 81, 86, 88, 90, 
};
#endif

#define BCL(rs) READ16(rBCL,rs)
#define BCR(rs) READ16(rBCR,rs)
#define IBO(rs) READ16(rIBO,rs)
#define SL(rs)  READ16(rSP,rs)



/**********************************************************************
* SigCompUdvmCreateFromState
* ------------------------------------------------------------------------
* Description:
*  Creates fresh instance of UDVM machine from the previously saved state
*  given by partial state identifier and prepare it for running
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*  pSelfUdvm - points to this instance of UDVM machine
*  pPartialState - point to the partial state identifier
*  partialStateLength - size of the memory area pointed by pPartialState
*
* OUTPUT: none
*
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successful, error code if failed
*
*************************************************************************/
RvStatus SigCompUdvmCreateFromState(IN SigCompUdvm           *pSelfUdvm,
                                    IN RvSigCompMgrHandle    hMgr,
                                    IN SigCompUdvmParameters *pParameters,
                                    IN RvUint8               *pStateId,
                                    IN RvUint                partialIdLength)
{
    SigCompStateHandlerMgr  *pStateHandler;
    SigCompState            *pState;
    RvStatus                rv;

    pStateHandler = &((SigCompMgr *)hMgr)->stateHandlerMgr;

    RvLogDebug(pStateHandler->pLogSrc,(pStateHandler->pLogSrc,
        "SigCompUdvmCreateFromState - trying to get state (statePartialIdLen=%d stateID=0x%X 0x%X 0x%X)",
        partialIdLength,
        pStateId[0],
        pStateId[1],
        pStateId[2]));
    
    rv = SigCompStateHandlerGetState(pStateHandler,
                                     pStateId,
                                     partialIdLength,
                                     RV_FALSE, /* don't ignore miminumAccessLength */
                                     &pState);

    if(rv != RV_OK)
    {
        pSelfUdvm->reason = UDVM_STATE_ACCESS;
        RvLogDebug(pStateHandler->pLogSrc,(pStateHandler->pLogSrc,
            "SigCompUdvmCreateFromState - failed to get state (rv=%d statePartialIdLen=%d stateID=0x%X 0x%X 0x%X)",
            rv,
            partialIdLength,
            pStateId[0],
            pStateId[1],
            pStateId[2]));
        return rv;
    }
    RvLogDebug(pStateHandler->pLogSrc,(pStateHandler->pLogSrc,
        "SigCompUdvmCreateFromState - using state (pState=%p stateID=0x%X 0x%X 0x%X)",
        pState,
        pStateId[0],
        pStateId[1],
        pStateId[2]));
    
    rv = InitMachine(pSelfUdvm, 
                     hMgr, 
                     pParameters, 
                     partialIdLength,
                     pState->stateDataSize, 
                     pState->pData, 
                     pState->stateDataSize,
                     pState->stateAddress, 
                     pState->stateInstruction);

    if(rv != RV_OK)
    {
        RvLogError(pStateHandler->pLogSrc,(pStateHandler->pLogSrc,
            "SigCompUdvmCreateFromState - failed to init udvm machine using state (pState=%p rv=%d)",
            pState,
            rv));
    }

    return rv;
}

/**********************************************************************
* SigCompUdvmCreateFromCode
* ------------------------------------------------------------------------
* Description:
*  Creates fresh instance of UDVM machine from the code sent inside SigComp
*  message and prepare it to run
*
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelfUdvm - points to the instance of SigCompUdvm object
*   pCode - points to the start of code buffer
*   codeLen - size of the code buffer
*   destination - start address in UDVM memory for code loading
*
* OUTPUT: none
*
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successful, error code if failed
*
*************************************************************************/
RvStatus SigCompUdvmCreateFromCode(IN SigCompUdvm           *pSelfUdvm,
                                   IN RvSigCompMgrHandle    hMgr,
                                   IN SigCompUdvmParameters *pParameters,
                                   IN RvUint8               *pCode,
                                   IN RvUint32              codeLen,
                                   IN RvUint32              destination)
{

    return InitMachine(pSelfUdvm, hMgr, pParameters, 0, 0, pCode, (RvUint16) codeLen,
                       (UdvmAddr) destination, (UdvmAddr) destination);
}

/**********************************************************************
* SigCompUdvmRun
* ------------------------------------------------------------------------
* Description:
*
* Run UDVM for the specified number of cycles. Cycles quota may be
*  enlarged by consuming additional input
*
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelfUdvm - points to the instance of SigCompUdvm object
*   nCycles - initial cycles quota
*
* OUTPUT:
*   pStatus - returns the status of UDVM:
*     UDVM_RUNNING - end-message instruction wasn't encountered
*        (UDVM needs more cycles to decompress this message)
*     UDVM_OK - udvm finished, no state
*      UDVM_WAITING,
*
* OUTPUT: none
*
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successful, error code if failed
*
*************************************************************************/

RvStatus SigCompUdvmRun(IN  SigCompUdvm       *pSelfUdvm,
                        IN  RvInt32           nCycles,
                        OUT SigCompUdvmStatus *pStatus)
{

    return SigCompUdvmRunPostmortem(pSelfUdvm, nCycles, pStatus, 0);
}


/**********************************************************************
* SigCompUdvmRunPostmortem
* ------------------------------------------------------------------------
* Description:
*
* Run UDVM for the specified number of cycles. Cycles quota may be
*  enlarged by consuming additional input
*
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelfUdvm - points to the instance of SigCompUdvm object
*   nCycles - number of cycles to run
*    (-1 run till the end or till cycles per bit exhausted)
*
* OUTPUT:
*   pPostmortem - pointer to SigCompPostmortem structure to be filled
*     with postmortem info
*   pStatus - returns the status of UDVM:
*     UDVM_RUNNING - end-message instruction wasn't encountered
*        (UDVM needs more cycles to decompress this message)
*     UDVM_OK - udvm finished, no state
*     UDVM_WAITING - udvm is waiting for valid compartment id to finish

*
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successful, error code if failed
*
*************************************************************************/

RvStatus SigCompUdvmRunPostmortem(IN  SigCompUdvm           *pSelfUdvm,
                                  IN  RvInt32               nCycles,
                                  OUT SigCompUdvmStatus     *pStatus,
                                  OUT SigCompUdvmPostmortem *pPostmortem)
{

    RvStatus rv;

    pSelfUdvm->targetCycles = pSelfUdvm->totalCycles + nCycles;

    *pStatus = UDVM_RUNNING;
    rv = RV_OK;
    pSelfUdvm->reason = UDVM_OK;

    while(pSelfUdvm->totalCycles < pSelfUdvm->targetCycles)
    {
        if(rv != RV_OK)
        {
            /* If failure reason wasn't set, set it to UNKNOWN reason */
            if(pSelfUdvm->reason == UDVM_OK)
            {
                pSelfUdvm->reason = UDVM_UNKNOWN;
            }
            break;
        }

#ifdef UDVM_DEBUG_SUPPORT
        {
            int i;
            SigCompUdvmBreakpoint *curb = pSelfUdvm->breakPoints;
            SigCompUdvmWatchpoint *curw = pSelfUdvm->watchPoints;
            int nBreaks = pSelfUdvm->nBreakPoints;
            int nWatches = pSelfUdvm->nWatchPoints;
            UdvmAddr pc = pSelfUdvm->pc;
            
            for(i = 0; i < nBreaks; i++, curb++)
            {

                if(curb->bEnabled && curb->pc == pc)
                {
                    *pStatus = UDVM_BREAK;
                    break;
                }
            }

            for(i = 0; i < nWatches; i++, curw++)
            {
                if(curw->bEnabled && (curw->pc == pc || curw->pc == 0))
                {
                    TreatWatchpoint(pSelfUdvm, curw);
                }

            }
        }

        if(*pStatus == UDVM_BREAK)
        {
            *pStatus = UDVM_RUNNING;
            DebugBreak();
        }
#endif
        if(*pStatus != UDVM_RUNNING)
        {
            break;
        }

        rv = ExecuteInstruction(pSelfUdvm, pStatus);
        pSelfUdvm->totalCycles++;
    }

    if((pSelfUdvm->totalCycles > pSelfUdvm->targetCycles) ||
        (*pStatus == UDVM_RUNNING && pSelfUdvm->totalCycles == pSelfUdvm->targetCycles))
    {
        pSelfUdvm->reason = UDVM_CPB_EXCEEDED;
        RvLogWarning(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
            "SigCompUdvmRunPostmortem - allocated cycles were exceeded"));
    }

    if(pPostmortem != 0)
    {
        FillPostmortem(pSelfUdvm, pPostmortem);

    }
    
    RvLogInfo(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
        "SigCompUdvmRunPostmortem - exit with rv=%d reason=%d",
        rv, pSelfUdvm->reason));
    return rv;
} /* end of SigCompUdvmRunPostmortem */

/**********************************************************************
* SigCompUdvmFinish
* ------------------------------------------------------------------------
* Description:
*    Finalize UDVM run. This function will perform all actions related to the
*    second part of End-Message instruction such as:
*     1. Freeing and saving states
*     2. Forwarding returned parameters:
*          * cycles-per-bit of the peer endpoint
*          * decompression_memory_size of the peer endpoint
*          * state_memory_size of the peer
*          * list of partial state IDs that are locally available at the peer
*            endpoint
*     3. Forwarding requested feedback specification:
*          S (state not needed) and I (local state items not needed) bits and
*          requested feedback item that will traverse as is in the opposite
*          direction in the next compressed message to this peer
*
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelfUdvm - points to the instance of the SigCompUdvm object
*   pCompartment - pointer to compartment
*
* OUTPUT: none
*
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successful, error code if failed
*
*************************************************************************/



RvStatus SigCompUdvmFinish(IN SigCompUdvm                   *pSelfUdvm,
                           IN SigCompCompartmentHandlerMgr  *pCompartmentHandler,
                           IN SigCompCompartment            *pCompartment)
{

    RvLogEnter(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
		"SigCompUdvmFinish(udvm=%p,comp=%p)",pSelfUdvm,pCompartment));

    UdvmForwardReturnedParameters(
        pSelfUdvm,
        pCompartmentHandler,
        pCompartment);

    UdvmForwardRequestedFeedback(
        pSelfUdvm,
        pCompartmentHandler,
        pCompartment);


    UdvmForwardStateRequests(
        pSelfUdvm,
        pCompartmentHandler,
        pCompartment);

    RvLogLeave(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
		"SigCompUdvmFinish(udvm=%p,comp=%p)",pSelfUdvm,pCompartment));

    return RV_OK;
}


/**********************************************************************
* SigCompUdvmDestruct
* ------------------------------------------------------------------------
* Description:
*   Destructs UDVM machine given by pSelfUdvm pointer
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelfUdvm - points to the instance of SigCompUdvm object
*
* OUTPUT: none
*
* ------------------------------------------------------------------------
* Returns:
*   void
*
*************************************************************************/
void SigCompUdvmDestruct(IN SigCompUdvm *pSelfUdvm)
{
	RV_UNUSED_ARG(pSelfUdvm);
    return;
}

/**********************************************************************
* InitMachine
* ------------------------------------------------------------------------
* Description:
* Initialize UDVM structure, loads code into udvm memory, initialize
* some parameters in the udvm memory and set program counter to the first
* instruction for execution.
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelfUdvm              - points to the instance of object
*   pParameters       - some inititalization
*   partialStateIdLen - length of the partial state id
*   stateLen          - length of the state itself
*   pCode             - points to the buffer containing the code
*   codeLen           - size of the code
*   codeDestination   - UDVM memory address to load the code
*   startAddr         - UDVM starting address
*
* OUTPUT: none
*
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successful, error code if failed
*
*************************************************************************/

static RvStatus InitMachine(
    IN SigCompUdvm            *pSelfUdvm,
    IN RvSigCompMgrHandle     hMgr,
    IN SigCompUdvmParameters  *pParameters,
    IN RvUint32               partialStateIdLen,
    IN RvUint16               stateLen,
    IN RvUint8                *pCode,
    IN RvUint16               codeLen,
    IN UdvmAddr               codeDestination,
    IN UdvmAddr               startAddr)
{
    RvUint8 *pMem;
    RvInt32 memSize;
    RvInt32 lastAddr = codeDestination + codeLen;

    if(pSelfUdvm == 0) {
        return RV_ERROR_NULLPTR;
    }

    pMem = pSelfUdvm->pUdvmMemory;
    memSize = (RvInt32)pSelfUdvm->udvmMemorySize;

    InitUdvmInstructions();

    pSelfUdvm->hManager = hMgr;

	if(hMgr == 0)
	{
		pSelfUdvm->pLogSource = 0;
	} else
	{
		SigCompMgr *pMgr = (SigCompMgr *)hMgr;
        pSelfUdvm->pLogSource = pMgr->pLogSource;
	}

    pSelfUdvm->pUdvmMemory    = pParameters->pUdvmMemory;
    pSelfUdvm->udvmMemorySize = pParameters->udvmMemorySize;
    pSelfUdvm->cyclesPerBit   = (RvUint16)pParameters->cyclesPerBit;

    pMem = pSelfUdvm->pUdvmMemory;
    memSize = pSelfUdvm->udvmMemorySize;

    if(lastAddr > memSize) {
        return RV_ERROR_OUTOFRANGE;
    }

    memcpy(ADDR_MEM(codeDestination), pCode, codeLen);

    WRITE16(cUDVMMemorySizeAddr, memSize);
    WRITE16(cCyclesPerBitAddr, pSelfUdvm->cyclesPerBit);
    WRITE16(cSigCompVersionAddr, 1);  /*lint !e572*/
    WRITE16(cPartialStateIdLengthAddr, partialStateIdLen);
    WRITE16(cStateLengthAddr, stateLen);

    memset(ADDR_MEM(10), 0, 22);
    memset(ADDR_MEM(lastAddr), 0, memSize - lastAddr);
    if(codeDestination > 32) {
        memset(ADDR_MEM(32), 0, codeDestination - 32);
    }

   
    pSelfUdvm->nextFreeState              = 0;
    pSelfUdvm->nextSaveState              = 0;
    pSelfUdvm->pFirstStateRequest         = 0;
    pSelfUdvm->pLastStateRequest          = 0;
    pSelfUdvm->totalCycles                = 0;
    pSelfUdvm->totalInstructions          = 0;
    pSelfUdvm->requestedFeedbackLocation  = 0;
    pSelfUdvm->returnedParametersLocation = 0;
    SETPC(pSelfUdvm->pc = startAddr);

    /* Bits buffer initialization */
    pSelfUdvm->curBits    = 0;
    pSelfUdvm->bitsBuffer = 0;
    pSelfUdvm->pbit       = 0;

#ifdef UDVM_DEBUG_SUPPORT
    pSelfUdvm->nBreakPoints = 0;
    pSelfUdvm->nWatchPoints = 0;
    pSelfUdvm->bTraceOn     = 0;
    pSelfUdvm->outputDebugStringCb = DefaultOutputDebugString;
    pSelfUdvm->outputDebugStringCookie = 0;
#endif
    return RV_OK;
}

/************************************************************************
* FillPostmortem
* ------------------------------------------------------------------------
* Description:
*   Fills postmortem information
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelfUdvm - points to the instance of SigCompUdvm object
*
* OUTPUT:
*   pPostmortem - points to the structure to be filled
* ------------------------------------------------------------------------
* Returns:
*
*************************************************************************/

static void FillPostmortem(IN  SigCompUdvm           *pSelfUdvm,
                           OUT SigCompUdvmPostmortem *pPostmortem)
{
    pPostmortem->failureReason = pSelfUdvm->reason;
    pPostmortem->pc = pSelfUdvm->pc;
    pPostmortem->totalCycles = pSelfUdvm->totalCycles;
    pPostmortem->totalInstructions = pSelfUdvm->totalInstructions;
    if(pSelfUdvm->pc < pSelfUdvm->udvmMemorySize)
    {
        pPostmortem->lastInstruction = pSelfUdvm->pUdvmMemory[pSelfUdvm->pc];
    } else
    {
        pPostmortem->lastInstruction = 0xff;
    }

}


 /*
    Bytecode:                       Operand value:      Range:

    00nnnnnn                        N                   0 - 63
    01nnnnnn                        memory[2 * N]       0 - 65535
    1000011n                        2 ^ (N + 6)        64 , 128
    10001nnn                        2 ^ (N + 8)    256 , ... , 32768
    111nnnnn                        N + 65504       65504 - 65535
    1001nnnn nnnnnnnn               N + 61440       61440 - 65535
    101nnnnn nnnnnnnn               N                   0 - 8191
    110nnnnn nnnnnnnn               memory[N]           0 - 65535
    10000000 nnnnnnnn nnnnnnnn      N                   0 - 65535
    10000001 nnnnnnnn nnnnnnnn      memory[N]           0 - 65535
  */

const RvUint8 bit2mask = 0xc0;
const RvUint8 bit3mask = 0xe0;
const RvUint8 bit4mask = 0xf0;
const RvUint8 bit5mask = 0xf8;
const RvUint8 bit7mask = 0xfe;

const RvUint8 mode00 = 0;
const RvUint8 mode01 = 0x40;
const RvUint8 mode1000011 = 0x86;
const RvUint8 mode10001 = 0x88;
const RvUint8 mode111 = 0xe0;
const RvUint8 mode1001 = 0x90;
const RvUint8 mode101 = 0xa0;
const RvUint8 mode110 = 0xc0;
const RvUint8 mode10000000 = 0x80;
const RvUint8 mode10000001 = 0x81;

/*
 *  Operand decoders
 *  There are 4 decoder functions - one for each type of UDVM operands:
 *   multitype, literal, reference and address.
 *  Those functions are called indirectly by OperandsDecoder object
 *  methods
 */

/**********************************************************************
* DecodeMultitypeOperand
* ------------------------------------------------------------------------
* Description:
* Decodes multitype operand starting at address 'addr'
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelfUdvm - points to the instance of SigCompUdvm object
*   addr  - starting address of operand in the UDVM memory
*
* OUTPUT:
*   pOpSize - size of the returned operand in bytes
*   pRes    - value of the operand
*
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successful, error code if failed
*
*************************************************************************/

#define DecodeMultitypeOperandMacro(ret, pSelfUdvm, addr, pOpSize, pRes)\
{\
    RvUint8 *pMem = pSelfUdvm->pUdvmMemory;\
    RvInt32 memSize = (RvInt32)pSelfUdvm->udvmMemorySize;\
    RvUint8 curval;\
	RvUint16 x;\
\
    *pOpSize = 0;\
    SAFE_READ8(addr, curval);\
\
    /* check 00nnnnnn */\
    if(curval < 64)\
    {\
        *pRes = curval;\
        *pOpSize = 1;\
        ret = RV_OK;\
    }\
    else if ((x = (RvUint16)(curval - 64)) < 64) /* check 01nnnnnn */\
	{\
        UdvmAddr opaddr = (UdvmAddr)(x << 1);\
        SAFE_READ16(opaddr, *pRes);\
        *pOpSize = 1;\
        ret = RV_OK;\
    }\
    else {\
		x = (RvUint16)(curval - 128); /* Get rid of the preceding 1 in 1xxxxxxx */\
		if (x == 0) /* check 10000000 nnnnnnnn nnnnnnnn */\
		{\
			UdvmAddr opaddr = (RvUint16)(addr + 1);\
			SAFE_READ16(opaddr, *pRes);\
			*pOpSize = 3;\
			ret = RV_OK;\
		}\
		else if (x == 1) /* check 10000001 nnnnnnnn nnnnnnnn */\
		{\
			UdvmAddr opaddr = (RvUint16)(addr + 1);\
			SAFE_READ16(opaddr, opaddr);\
			SAFE_READ16(opaddr, *pRes);\
			*pOpSize = 3;\
			ret = RV_OK;\
		}\
		else if ((x = (RvUint16)(curval - 0x86)) < 2) /* check 1000011n */\
		{\
			*pRes = (RvUint16)((x) ? 128 : 64);\
			*pOpSize = 1;\
			ret = RV_OK;\
		}\
		else if ((x = (RvUint16)(curval - 0x88)) < 8) /* check 10001nnn */\
		{\
			*pRes = (RvUint16)(1 << (x + 8)); /*lint !e504*/\
			*pOpSize = 1;\
			ret = RV_OK;\
		}\
        else if ((x = (RvUint16)(curval - 0x90)) < 16) /* check 1001nnnn nnnnnnnn */\
		{\
			UdvmAddr opaddr = (RvUint16)(addr + 1);\
			RvUint8 lsb;\
			SAFE_READ8(opaddr, lsb);\
			*pRes = (RvUint16)((x << 8) + lsb + 61440);\
			*pOpSize = 2;\
			ret = RV_OK;\
		}\
        else if ((x = (RvUint16)(curval - 0xa0)) < 32) /* check 101nnnnn nnnnnnnn */\
		{\
			UdvmAddr opaddr = (RvUint16)(addr + 1);\
			RvUint8 lsb;\
			SAFE_READ8(opaddr, lsb);\
			*pRes = (RvUint16)((x << 8) + lsb);\
			*pOpSize = 2;\
			ret = RV_OK;\
		}\
		else if ((x = (RvUint16)(curval - 0xc0)) < 32) /* check 110nnnnn nnnnnnnn */\
		{\
			UdvmAddr opaddr = (RvUint16)(addr + 1);\
			RvUint8 lsb;\
			SAFE_READ8(opaddr, lsb);\
			opaddr = (RvUint16)((x << 8) + lsb);\
			SAFE_READ16(opaddr, *pRes);\
			*pOpSize = 2;\
			ret = RV_OK;\
		}\
		else /* left 111nnnnn */\
		{\
			*pRes    = (RvUint16)(curval - 0xe0 + 65504);\
			*pOpSize = 1;\
			ret = RV_OK;\
		}\
	}\
\
}

/*
Bytecode:                       Operand value:      Range:

0nnnnnnn                        N                   0 - 127
10nnnnnn nnnnnnnn               N                   0 - 16383
11000000 nnnnnnnn nnnnnnnn      N                   0 - 65535
*/

const RvUint8 mode10 = 0x80;
const RvUint8 mode11000000 = 0xc0;

/**********************************************************************
* DecodeLiteralOperand
* ------------------------------------------------------------------------
* Description:
* Decodes literal operand starting at address 'addr'
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelfUdvm - points to the instance of SigCompUdvm object
*   addr  - starting address of operand in the UDVM memory
*
* OUTPUT:
*   pOpSize - size of the returned operand in bytes
*   pRes    - value of the operand
*
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successful, error code if failed
*
*************************************************************************/

#define DecodeLiteralOperandMacro(ret,pSelfUdvm,addr,pOpSize,pRes) \
{\
    RvUint8 *pMem = pSelfUdvm->pUdvmMemory;\
    RvInt32 memSize = (RvInt32)pSelfUdvm->udvmMemorySize;\
    RvUint8 curval;\
\
    *pOpSize = 0;\
    SAFE_READ8(addr, curval);\
\
    if(curval < 128)\
    {\
        *pRes = curval;\
        *pOpSize = 1;\
        ret = RV_OK;\
    }\
    else if((curval & bit2mask) == mode10)\
    {\
        RvUint8 lsb;\
\
        SAFE_READ8((addr+1), lsb);\
        *pRes = (RvUint16)(((curval & ~bit2mask) << 8) + lsb);\
        *pOpSize = 2;\
        ret = RV_OK;\
    }\
    else if(curval == mode11000000)\
    {\
        SAFE_READ16((addr+1), *pRes);\
        *pOpSize = 3;\
        ret = RV_OK;\
    }\
    else \
        ret = RV_ERROR_UNKNOWN;\
} 

/**********************************************************************
* DecodeReferenceOperand
* ------------------------------------------------------------------------
* Description:
* Decodes reference operand starting at address 'addr'
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelfUdvm - points to the instance of SigCompUdvm object
*   addr  - starting address of operand in the UDVM memory
*
* OUTPUT:
*   pOpSize - size of the returned operand in bytes
*   pRes    - value of the operand
*
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successful, error code if failed
*
*************************************************************************/
#define DecodeReferenceOperandMacro(rv,pSelfUdvm,addr,pOpSize,pRes)\
{\
    RvUint8 *pMem   = pSelfUdvm->pUdvmMemory;\
    RvInt32  memSize = (RvInt32)pSelfUdvm->udvmMemorySize;\
    RvUint8  curval;\
\
    *pOpSize = 0;\
    SAFE_READ8(addr, curval);\
\
    if(curval < 128)\
    {\
        *pRes = (RvUint16)(curval << 1);\
        *pOpSize = 1;\
        rv = RV_OK;\
    }\
	else if((curval & bit2mask) == mode10)\
    {\
        RvUint8 lsb;\
\
        SAFE_READ8((addr+1), lsb);\
        *pRes = (RvUint16)((((curval & ~bit2mask) << 8) + lsb) << 1);\
        *pOpSize = 2;\
        rv = RV_OK;\
    }\
	else if(curval == mode11000000)\
    {\
        SAFE_READ16((addr+1), *pRes);\
        *pOpSize = 3;\
        rv = RV_OK;\
    }\
	else \
	{\
		rv = RV_ERROR_UNKNOWN;\
	}\
}

/**********************************************************************
* DecodeAddressOperand
* ------------------------------------------------------------------------
* Description:
* Decodes address operand starting at address 'addr'
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelfUdvm - points to the instance of SigCompUdvm object
*   addr  - starting address of operand in the UDVM memory
*
* OUTPUT:
*   pOpSize - size of the returned operand in bytes
*   pRes    - value of the operand
*
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successful, error code if failed
*
*************************************************************************/
#define DecodeAddressOperandMacro(ret,pSelfUdvm,addr,pOpSize,pRes)  \
{\
    RvUint16 displacement=0;\
\
    DecodeMultitypeOperandMacro(ret,pSelfUdvm, addr, pOpSize, &displacement);\
    if(ret == RV_OK)\
    {\
        *pRes = (RvUint16)(pSelfUdvm->pc + displacement);\
    }\
}

/************************************************************************
* ReverseBits8, ReverseBits16
* ------------------------------------------------------------------------
* Description:
* reverse bits in the given number
* ReverseBits8  - 8 bits version
* ReverseBits16 - 16 bits version
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   x - number to reverse bits
*
* OUTPUT: none
*
* ------------------------------------------------------------------------
* Returns:
*  Reversed bit version of x
*************************************************************************/

static RvUint8 ReverseBits8(RvUint8 x)
{
    RvUint8 y;

    y = (RvUint8)(((x & 0x55) << 1) | ((x & 0xaa) >> 1));
    y = (RvUint8)(((y & 0x33) << 2) | ((y & 0xcc) >> 2));
    y = (RvUint8)(((y & 0x0f) << 4) | ((y & 0xf0) >> 4));

    return (y);
}

static RvUint16 ReverseBits16(RvUint16 x)
{
    RvUint16 y;

    y = (RvUint16)(((x & 0x5555) << 1) | ((x & 0xaaaa) >> 1));
    y = (RvUint16)(((y & 0x3333) << 2) | ((y & 0xcccc) >> 2));
    y = (RvUint16)(((y & 0x0f0f) << 4) | ((y & 0xf0f0) >> 4));
    y = (RvUint16)(((y & 0x00ff) << 8) | ((y & 0xff00) >> 8));
    return (y);
}

static void InitReverseBits()
{
    int i;

    for(i = 0; i < 256; i++)
    {
        gReversedBytes[i] = ReverseBits8((RvUint8) i);
    }
}

/************************************************************************
*
* ------------------------------------------------------------------------
* Description:
*   Initialize UDVM instruction descriptions array
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT: none
*
* OUTPUT: none
*
* ------------------------------------------------------------------------
* Returns: none
*
*************************************************************************/

static void InitUdvmInstructions(void)
{
    RvUint i;
    UdvmInstructionDescription *pCur;

    if(gUdvmInstructionsInited)
    {
        return;
    }

    InitReverseBits();

    for(i = 0, pCur = gUdvmInstructions; i < ARRAY_SIZE(gUdvmInstructions);
        pCur++, i++)
    {
        RvChar *pOpSpec = pCur->opspec;
        RvChar curSpec;
        RvUint *pCurNops = &pCur->nFixedOperands;
        OperandDecoderEnum *pCurDecoder = pCur->opDecoders;

        pCur->nVarOperands = 0;
        pCur->nFixedOperands = 0;


        for(curSpec = *pOpSpec++; curSpec; curSpec = *pOpSpec++)
        {
            switch (curSpec) 
            {
            case '$':
                *pCurDecoder = UDVM_REFERENCE_OPERAND_DECODER;
                break;

            case '%':
                *pCurDecoder = UDVM_MULTITYPE_OPERAND_DECODER;
                break;

            case '@':
                *pCurDecoder = UDVM_ADDRESS_OPERAND_DECODER;
                break;

            case '#':
                *pCurDecoder = UDVM_LITERAL_OPERAND_DECODER;
                break;

            case ';':
                pCurNops = &pCur->nVarOperands;
                break;

            default:
                /* Shouldn't happen */
                /*lint -e527*/
                RvAssert(0);
                return;
                /*lint +e527*/
            }

            if(curSpec != ';')
            {
                pCurDecoder++;
                (*pCurNops)++;
            }

            /* Shouldn't happen */
            RvAssert(pCur->nVarOperands + pCur->nFixedOperands <=
                     ARRAY_SIZE(pCur->opDecoders));
        }

    }

    gUdvmInstructionsInited = 1;
}

/**********************************************************************
* OperandsDecoderDecodeStart
* ------------------------------------------------------------------------
* Description:
*   Decodes fixed operands and places them in the pSelfUdvm->operands array
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelfUdvm        - points to the instance of OperandsDecoder object
*   pUdvm        - points to the associated UDVM machine
*   pDescr       - point to the instruction description structure
*   operandStart - starting address in UDVM memory of the first
*                  instruction operand
* OUTPUT:
*
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successful, error code if failed
*
*************************************************************************/


/**********************************************************************
* OperandsDecoderDecodeNext
* ------------------------------------------------------------------------
* Description:
*   Decodes the next group of variable operands and places them in the
*   pSelfUdvm->operands array
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelfUdvm        - points to the instance of OperandsDecoder object
*   pUdvm        - points to the associated UDVM machine
*
* OUTPUT: none
*
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successful, error code if failed
*
*************************************************************************/
static RvStatus OperandsDecoderDecodeNext(OperandsDecoder *pDecoder,
                                          SigCompUdvm     *pUdvm)
{
    RvUint32 i;
    UdvmInstructionDescription *pDescr = pDecoder->instrDescr;
    RvUint nVars = pDescr->nVarOperands;
    UdvmAddr curAddr = pDecoder->nextOperandAddr;
    OperandDecoderEnum *pCurDecoder = &pDescr->opDecoders[pDescr->nFixedOperands];
    RvStatus rv = RV_OK;
    RvUint16 *pCurOperand = pDecoder->operands;

    if(pDecoder->nVarGroupsNumber == 0 || nVars == 0)
    {
        return DECODE_FINISHED;
    }


    for(i = 0; i < nVars; i++, pCurDecoder++, pCurOperand++)
    {
        RvUint16 opSize = 0;
        SigCompUdvm  *pSelfUdvm = pUdvm;
        
        switch (*pCurDecoder)
        {
        case UDVM_MULTITYPE_OPERAND_DECODER:
            DecodeMultitypeOperandMacro(rv,pUdvm,curAddr,(&opSize),pCurOperand);
            break;
        case UDVM_ADDRESS_OPERAND_DECODER:
            DecodeAddressOperandMacro(rv,pUdvm,curAddr,(&opSize),pCurOperand);
            break;
        case UDVM_REFERENCE_OPERAND_DECODER:
            DecodeReferenceOperandMacro(rv,pUdvm,curAddr,(&opSize),pCurOperand);
            break;
        case UDVM_LITERAL_OPERAND_DECODER:
            DecodeLiteralOperandMacro(rv,pUdvm,curAddr,(&opSize),pCurOperand);
            break;
        }


        if(rv != RV_OK)
		{
            return rv;
		}

		UdvmInstrLog(pUdvm->pLogSource, (pUdvm->pLogSource, 
			gOperandsFormat[gOperandFormatOffsets[pDescr->opcode] + pDescr->nFixedOperands + i],
			*pCurOperand));
		
        curAddr = (RvUint16) (curAddr + opSize);
    }

    pDecoder->nextOperandAddr = curAddr;
    pDecoder->nVarGroupsNumber--;
    return RV_OK;
}


static RvStatus OperandsDecoderSkipRest(OperandsDecoder *pDecoder, SigCompUdvm *pUdvm)
{
    RvStatus rv = RV_ERROR_UNKNOWN;

    for (;;)
    {
        rv = OperandsDecoderDecodeNext(pDecoder, pUdvm);
        if(rv == DECODE_FINISHED)
        {
            return RV_OK;
        }

        if(rv != RV_OK)
        {
            break;
        }
    }  


    return rv;
}


/*
 *  Cyclic buffer related methods
 */

/**********************************************************************
* CyclicBufferGetNext
* ------------------------------------------------------------------------
* Description:
*   Computes the next contiguous area in the
* ------------------------------------------------------------------------
* Arguments:
*
* INPUT:
*   pSelfUdvm - points to the instance of object
*
* OUTPUT:
*
* ------------------------------------------------------------------------
* Returns:
*   RV_OK if successful, error code if failed
*
*************************************************************************/

#define CyclicBufferGetNext(_pSelfUdvm,_pStart,_pSize,_rv) \
{\
    UdvmAddr bcl      = (_pSelfUdvm)->bcl;\
    UdvmAddr bcr      = (_pSelfUdvm)->bcr;\
    UdvmAddr start    = (_pSelfUdvm)->start;\
    RvUint32 memSize  = (_pSelfUdvm)->memSize;\
    RvUint16 nBytes   = (_pSelfUdvm)->nBytes;\
    RvInt32 dist     =  bcr - start;\
    RvUint16 curBytes;\
    _rv = RV_OK;\
    if(dist <= 0)\
        dist += MAX_UDVM_MEMORY_SIZE;\
    curBytes = (RvUint16) (dist < nBytes ? dist : nBytes);\
    if((RvUint32) (start + curBytes) > memSize)\
    {\
        if(memSize != MAX_UDVM_MEMORY_SIZE) \
            _rv = RV_ERROR_UNKNOWN;\
        else\
            curBytes = (RvUint16)(MAX_UDVM_MEMORY_SIZE - start); \
    }\
    if (_rv == RV_OK)\
    {\
        *(_pStart) = start;\
        *(_pSize) = curBytes;\
        if((start = (RvUint16) (start + curBytes)) == bcr)\
            (_pSelfUdvm)->start = bcl;\
        else\
            (_pSelfUdvm)->start = start;\
        (_pSelfUdvm)->nBytes = (RvUint16) ((_pSelfUdvm)->nBytes - curBytes);\
    }\
}
#define CyclicBufferConstruct(_pSelfUdvm,_pUdvm,_start,_nBytes)  \
{\
    RvUint8  *pMem   = (_pUdvm)->pUdvmMemory;\
    BCL((_pSelfUdvm)->bcl);\
    BCR((_pSelfUdvm)->bcr);\
    (_pSelfUdvm)->memSize  = (_pUdvm)->udvmMemorySize;\
    (_pSelfUdvm)->start    = (_start);\
    (_pSelfUdvm)->nBytes   = (_nBytes);\
    (_pSelfUdvm)->pMem     = pMem;\
} 

/* Performs calculation of starting address for COPY-OFFSET instruction 
 * This calculation is defined in RFC3320:
 *
 * To derive the value of the position operand, starting at the memory
 * address specified by destination, the UDVM counts backwards a total
 * of offset memory addresses.
 *
 *  If the memory address specified in byte_copy_left is reached, the
 *  next memory address is taken to be (byte_copy_right - 1) modulo 2^16.
 *
 *  
 */
static void CyclicBufferComputeStart(CyclicBuffer   *pCB,
                                     RvUint16       destination,
                                     RvUint16       offset)
{
    RvUint16 distFromBcl;
    RvUint16 rem;
    RvUint16 cyclicBufferSize;

    cyclicBufferSize = (RvUint16) (pCB->bcr - pCB->bcl);

    /* If there is no cyclic buffer defined - just count backwards 
     * 'offset' steps
     */
    if(cyclicBufferSize == 0) 
    {
        pCB->start = (RvUint16) (destination - offset);
        return;
    }

    /* Otherwise, there is a cyclic buffer. Pay attention: it doesn't matter
     * whether bcl < bcr or bcl > bcr. All our calcullations are module 2^16
     * At this stage we don't check validity of the result also
     */

    distFromBcl = (RvUint16) (destination - pCB->bcl);

    /* The 'magic' starts when we reach 'bcl', so if offset is less or equal
     * than distance from bcl - no 'magic' occurs
     */
    if(distFromBcl >= offset) 
    {
        pCB->start = (RvUint16) (destination - offset);
        return;
    }

    /* We're get here if offset > distFromBcl */

    offset = (RvUint16) (offset - distFromBcl); /* This is the part of offset that will be 'spent' inside cyclic buffer */
    rem = (RvUint16) (offset % cyclicBufferSize);
    pCB->start = (RvUint16) ((rem == 0) ? pCB->bcl : pCB->bcr - rem);
}



typedef void (*CyclicBufferTraverseCb) (void     *cookie,
                                        RvUint8  *mem,
                                        RvUint16 size,
                                        RvStatus* rv);

static RvStatus CyclicBufferTraverse(CyclicBuffer            *pCB,
                                     CyclicBufferTraverseCb   cb,
                                     void                    *cookie,
                                     CyclicBufferCallbackEnum cbNum)
{
	RvStatus rv = RV_OK;

    for (;;)
    {
        UdvmAddr curAddr = 0;
        RvUint16 curSize = 0;
        RvUint8  *pCur;

        CyclicBufferGetNext(pCB, &curAddr, &curSize, rv);
        if(rv != RV_OK)
        {
			break;
        }

        if(curSize == 0)
        {
            return RV_OK;
        }

        pCur = pCB->pMem + curAddr;


        switch (cbNum)
        {
        case UDVM_CYCLIC_BUF_COPY_TO_LINEAR_TRAVERSER:
            CopyToLinearTraverserMacro(cookie,pCur,curSize,rv);
            break;
        case UDVM_CYCLIC_BUF_COPY_FROM_LINEAR_TRAVERSER:
            CopyFromLinearTraverserMacro(cookie,pCur,curSize, rv);
            break;
        case UDVM_CYCLIC_BUF_COPY_TO_CYCLIC_TRAVERSER:
            CopyToCyclicTraverserMacro(cookie,pCur,curSize,rv);
            break;
        case UDVM_CYCLIC_BUF_INPUT_BYTES_TRAVERSER:
            InputBytesTraverserMacro(cookie,pCur,curSize, rv);
            break;
        case UDVM_CYCLIC_BUF_WRITE_BYTES:
            UdvmWriteBytesMacro(cookie,pCur,curSize, rv);
            break;
        default:
            (*cb) (cookie, pCur, curSize,&rv);            
        }

        if(rv != RV_OK)
        {
			break;
        }

    } 


    return rv;
}






  /* Treat returned parameters

    0   1   2   3   4   5   6   7
    +---+---+---+---+---+---+---+---+
    |  cpb  |    dms    |    sms    |  returned_parameters_location
    +---+---+---+---+---+---+---+---+
    |        SigComp_version        |
    +---+---+---+---+---+---+---+---+
    | length_of_partial_state_ID_1  |
    +---+---+---+---+---+---+---+---+
    |                               |
    :  partial_state_identifier_1   :
    |                               |
    +---+---+---+---+---+---+---+---+
    :               :
    +---+---+---+---+---+---+---+---+
    | length_of_partial_state_ID_n  |
    +---+---+---+---+---+---+---+---+
    |                               |
    :  partial_state_identifier_n   :
    |                               |
    +---+---+---+---+---+---+---+---+
    */

static void UdvmForwardReturnedParameters(
                        IN SigCompUdvm                  *pSelfUdvm,
                        IN SigCompCompartmentHandlerMgr *pCompartmentHandler,
                        IN SigCompCompartment           *pCompartment)
{
    RvUint32 localStatesSize = 0;
    RvUint32 returnedSize = 0;
    RvUint8 *pLocalStates = 0;
    RvUint8 *pLocalStatesDest;
    RvUint8 *retParams = pSelfUdvm->pUdvmMemory + pSelfUdvm->returnedParametersLocation;
    RvUint8 *curPtr = retParams;
    RvUint8 *endMem = pSelfUdvm->pUdvmMemory + pSelfUdvm->udvmMemorySize;
    RvStatus rv;

    RvLogEnter(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
		"UdvmForwardReturnedParameters(udvm=%p,comp=%p)",pSelfUdvm,pCompartment));
    
    if(pSelfUdvm->returnedParametersLocation == 0)
    {
        RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
            "UdvmForwardReturnedParameters - returnedParametersLocation == 0"));
        return;
    }

    pLocalStates = curPtr = retParams + 2;

    while(*curPtr >= 6 && *curPtr <= 20 && curPtr < endMem)
    {
        curPtr += *curPtr + 1;
    }

    /* Illegal returned parameters format */
    if(curPtr >= endMem)
    {
        RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
            "UdvmForwardReturnedParameters - illegal returned parameters format"));
        return;
    }

    returnedSize = localStatesSize = (RvUint32)(curPtr - pLocalStates);

    rv = SigCompCompartmentHandlerPeerEPCapabilitiesUpdate( pCompartmentHandler,
                                                            pCompartment,
                                                            *retParams,
                                                            *(retParams + 1),
                                                            &returnedSize,
                                                            &pLocalStatesDest);

    /* localStatesSize == 0 */
    if(rv != RV_OK || returnedSize == 0 || pLocalStatesDest == 0)
    {
        RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
            "UdvmForwardReturnedParameters - failure when updating peer EP capabilities (rv=%d)",
            rv));
        return;
    }

    RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
        "UdvmForwardReturnedParameters - peer EP capabilities updated"));
    
    /* Compute new localStatesSize that will be <= returnedSize and
    * will cover integral number of state IDs
    */
    if(returnedSize < localStatesSize)
    {
        curPtr = pLocalStates;
        endMem = pLocalStates + returnedSize;

        while(*curPtr >= 6 && *curPtr <= 20 &&
            (curPtr + *curPtr + 1) < endMem)
        {
            curPtr += *curPtr + 1;
        }

        localStatesSize = (RvUint32)(curPtr - pLocalStates);
    }

    memcpy(pLocalStatesDest, pLocalStates, localStatesSize);
    RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
        "UdvmForwardReturnedParameters - finished"));
    RvLogLeave(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
		"UdvmForwardReturnedParameters(udvm=%p,comp=%p)",pSelfUdvm,pCompartment));
}


/*
0   1   2   3   4   5   6   7
+---+---+---+---+---+---+---+---+
|     reserved      | Q | S | I |  requested_feedback_location
+---+---+---+---+---+---+---+---+
|                               |
:    requested feedback item    :  if Q = 1
|                               |
+---+---+---+---+---+---+---+---+
*/

const RvUint8 cSbitMask = 2;
const RvUint8 cIbitMask = 1;
const RvUint8 cQbitMask = 4;

static void UdvmForwardRequestedFeedback(
                        IN SigCompUdvm                  *pSelfUdvm,
                        IN SigCompCompartmentHandlerMgr *pCompartmentHandler,
                        IN SigCompCompartment           *pCompartment)
{
    RvUint8  *pRfp;
    RvUint8  *pEndMem;
    RvBool   bRemoteStatesNeeded;
    RvBool   bLocalStatesNeeded;
    RvUint8  qsi;
    RvUint16 requestedFeedbackItemSize = 0;
    RvUint8  *pRequestedFeedbackItem   = 0;
    RvStatus rv;

    RvLogEnter(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,"UdvmForwardRequestedFeedback"));

    if(pSelfUdvm->requestedFeedbackLocation == 0)
    {
        RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
            "UdvmForwardRequestedFeedback - no requested feedback supplied"));
        return;
    }

    pRfp = pSelfUdvm->pUdvmMemory + pSelfUdvm->requestedFeedbackLocation;
    pEndMem = pSelfUdvm->pUdvmMemory + pSelfUdvm->udvmMemorySize;

    /* Illegal requested feedback format */
    if(pRfp >= pEndMem)
    {
            RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
              "UdvmForwardRequestedFeedback - illegal requested feedback format (0)"));
        return;
    }

    qsi = *pRfp;

    bRemoteStatesNeeded = (qsi & cSbitMask) == 0;
    bLocalStatesNeeded = (qsi & cIbitMask) == 0;

    if(qsi & cQbitMask)
    {
        pRfp++;
        /* Illegal requested feedback format */
        if(pRfp >= pEndMem)
        {
            RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
                "UdvmForwardRequestedFeedback - illegal requested feedback format (1)"));
            return;
        }

        if(*pRfp <= 127)
        {
            requestedFeedbackItemSize = 1;
            pRequestedFeedbackItem = pRfp;
        } 
        else
        {
            requestedFeedbackItemSize = (RvUint16) (*pRfp & 0x7f);

            /* Illegal requested feedback format */
            if(pRfp + requestedFeedbackItemSize + 1 >= pEndMem)
            {
                RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
                    "UdvmForwardRequestedFeedback - illegal requested feedback format (2)"));
                return;
            }

            pRequestedFeedbackItem = pRfp + 1;
        }

    }

    rv = SigCompCompartmentHandlerForwardRequestedFeedback(pCompartmentHandler,
                                                            pCompartment,
                                                            bRemoteStatesNeeded,
                                                            bLocalStatesNeeded,
                                                            requestedFeedbackItemSize,
                                                            pRequestedFeedbackItem);
    if (rv != RV_OK) 
    {
    RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
            "UdvmForwardRequestedFeedback - failure on return from SigCompCompartmentHandlerForwardRequestedFeedback (rv=%d)",
            rv));
    }

    RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
        "UdvmForwardRequestedFeedback - finished"));
    RvLogLeave(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,"UdvmForwardRequestedFeedback"));
}

static void Sha1Traverser(void     *cookie,
                          RvUint8  *buf,
                          RvUint16 bufSize,
						  RvStatus *pRv);


/*
    calculates a 20-byte SHA-1 hash [RFC-3174] over the
    byte string formed by concatenating the state_length,
    state_address, state_instruction, minimum_access_length and
    state_value (in the order given).  This is the state_identifier.
*/

static RvStatus UdvmComputeStateId(IN  SigCompUdvm *pSelfUdvm,
                                   IN  RvUint16    stateLength,
                                   IN  RvUint16    stateAddress,
                                   IN  RvUint16    stateInstruction,
                                   IN  RvUint16    minAccessLength,
                                   OUT RvUint8     stateId[RVSIGCOMP_SHA1_HASH_SIZE])
{
    RvStatus			 rv;
    RvSigCompSHA1Context sha1;
    CyclicBuffer		 cb;
    RvUint8				 pMem[8];

    WRITE16(0, stateLength);
    WRITE16(2, stateAddress);
    WRITE16(4, stateInstruction);
    WRITE16(6, minAccessLength);

    if (RvSigCompSHA1Reset(&sha1) != 0) 
    {
        /* error in setting SHA1 */
        return RV_ERROR_UNKNOWN;
    }
    if (RvSigCompSHA1Input(&sha1, pMem, sizeof(pMem)) != 0) 
    {
        /* error in input to SHA1 */
        return RV_ERROR_UNKNOWN;
    }
    
    CyclicBufferConstruct(&cb, pSelfUdvm, stateAddress, stateLength);

    rv = CyclicBufferTraverse(&cb, Sha1Traverser, &sha1, UDVM_CYCLIC_BUF_SHA1_TRAVERSER);
    if(rv != RV_OK)
    {
        return rv;
    }

    if (RvSigCompSHA1Result(&sha1, stateId) != 0) 
    {
        /* error in getting SHA1 result */
        return RV_ERROR_UNKNOWN;
    }
    return RV_OK;
}


static void CompareStatesTraverser(void *ctx, RvUint8 *pMem, RvUint16 size,RvStatus *pRv)
{
    RvUint8 **pLinear = (RvUint8 **)ctx;
    RvInt cmpRes = memcmp(*pLinear, pMem, size);
    *pLinear += size;
    *pRv = (cmpRes == 0) ? RV_OK : 1;
}


static RvBool CompareStates(void *ctx, RvUint8 *pLinear, RvUint size)
{
    CyclicBuffer *pCb = (CyclicBuffer *)ctx;
    RvStatus rv = CyclicBufferTraverse(pCb, CompareStatesTraverser, &pLinear, UDVM_CYCLIC_BUF_COMPARE_STATES_TRAVERSER);
	RV_UNUSED_ARG(size);
    return rv == RV_OK;
}



static void UdvmForwardStateRequests(
                        IN SigCompUdvm                  *pSelfUdvm,
                        IN SigCompCompartmentHandlerMgr *pCompartmentHandler,
                        IN SigCompCompartment           *pCompartment)
{
    SigCompStateRequest *pCurRequest = pSelfUdvm->pFirstStateRequest;
    RvStatus rv;

    RvLogEnter(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,"UdvmForwardStateRequests"));
    
    for(; pCurRequest; pCurRequest = pCurRequest->pNext)
    {
        if(pCurRequest->requestType == SAVE_STATE_REQUEST)
        {
            SigCompSaveStateRequest *pSaveState = (SigCompSaveStateRequest *)pCurRequest;

            RvUint8 stateId[RVSIGCOMP_STATE_ID_LENGTH];
            RvUint8 *pStateVal = 0;
            CyclicBuffer cb;
            RvUint16 stateLength      = pSaveState->stateLength;
            RvUint16 stateAddress     = pSaveState->stateAddress;
            RvUint16 stateInstruction = pSaveState->stateInstruction;
            RvUint16 minAccessLength  = pSaveState->minimumAccessLength;
            RvUint16 statePriority    = pSaveState->stateRetentionPriority;

                RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
                "UdvmForwardStateRequests - process a request to save state"));

            if((RvUint32)(stateLength + 64) > pCompartment->localSMS)
            {
                stateLength = (RvUint16) (pCompartment->localSMS - 64);
                if(pCompartment->localSMS < 64)
                {
                    continue;
                }
            }

            rv = UdvmComputeStateId(pSelfUdvm, 
                                    stateLength, 
                                    stateAddress, 
                                    stateInstruction,
                                    minAccessLength, 
                                    stateId);

            if(rv != RV_OK)
            {
                RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
                    "UdvmForwardStateRequests - problem in UdvmComputeStateId (rv=%d)",
                    rv));
                continue;
            }

            CyclicBufferConstruct(&cb, pSelfUdvm, stateAddress, stateLength);

            rv = SigCompCompartmentHandlerAddStateToCompartment(pCompartmentHandler,
                                                                pCompartment,
                                                                stateId,
                                                                statePriority,
                                                                minAccessLength,
                                                                stateAddress,
                                                                stateInstruction,
                                                                stateLength,
                                                                CompareStates,
                                                                (void *)&cb,
                                                                &pStateVal);

            if(pStateVal == 0 || rv != RV_OK)
            {
                RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
                    "UdvmForwardStateRequests - state not added to compartment (rv=%d)", rv));
                continue;
            }

            CyclicBufferConstruct(&cb, pSelfUdvm, stateAddress, stateLength);
            rv = CyclicBufferTraverse(&cb, NULL, &pStateVal, UDVM_CYCLIC_BUF_COPY_TO_LINEAR_TRAVERSER);
            if(rv != RV_OK)
            {
                RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
                    "UdvmForwardStateRequests - failure in CyclicBufferTraverse (1)"));
                continue;
            }
        } 
        else
        {
            SigCompFreeStateRequest *pFreeRequest =
                (SigCompFreeStateRequest *)pCurRequest;
            RvUint8 stateId[RVSIGCOMP_STATE_ID_LENGTH];
            RvUint8 *pStateId = &stateId[0];
            CyclicBuffer cb;
            UdvmAddr partialIdStart = pFreeRequest->partialIdentifierStart;
            RvUint16 partialIdLength = pFreeRequest->partialIdentifierLength;

            CyclicBufferConstruct(&cb, pSelfUdvm, partialIdStart, partialIdLength);
            rv = CyclicBufferTraverse(&cb, NULL, &pStateId, UDVM_CYCLIC_BUF_COPY_TO_LINEAR_TRAVERSER);
            
            if(rv != RV_OK)
            {
                RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
                    "UdvmForwardStateRequests - failure in CyclicBufferTraverse (2)"));
                continue;
            }

            rv = SigCompCompartmentHandlerRemoveStateFromCompartment(pCompartmentHandler,
                                                                     pCompartment,
                                                                     stateId,
                                                                     partialIdLength);
            if (rv != RV_OK) 
            {
                RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
                    "UdvmForwardStateRequests - failed to remove state from compartment (hCmpartment=%p stateID=0x%X 0x%X 0x%X rv=%d)",
                    pCompartment,
                    stateId[0],
                    stateId[1],
                    stateId[2],
                    rv));
            }
        }
    }

    RvLogDebug(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
        "UdvmForwardStateRequests - finished"));
    RvLogLeave(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,"UdvmForwardStateRequests"));
}



static RvStatus ExecuteInstruction(SigCompUdvm       *pSelfUdvm,
                                   SigCompUdvmStatus *pRunningStatus)
{

    RvUint8 *pMem   = pSelfUdvm->pUdvmMemory;
    RvInt32 memSize = (RvInt32)pSelfUdvm->udvmMemorySize;
    UdvmOpcode opcode;
    OperandsDecoder operandsDecoder;
    UdvmInstructionDescription *descr;
    RvStatus rv;
    UdvmAddr pc;
    RvUint8 topcode;

    SAFE_READ8(pSelfUdvm->pc, topcode);
    opcode = (UdvmOpcode)topcode;

    if(opcode >= ARRAY_SIZE(gUdvmInstructions))
    {
        RvLogWarning(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
            "ExecuteInstruction - unexpected opcode (opcode=0x%.2X)",
            opcode));
        return RV_ERROR_UNKNOWN;
    }

    pc = pSelfUdvm->pc;
    descr = &gUdvmInstructions[opcode];

	UdvmInstrLog(pSelfUdvm->pLogSource, (pSelfUdvm->pLogSource, "<%s: %d, %d>", 
		descr->instrName, pSelfUdvm->totalInstructions, pSelfUdvm->totalCycles));
	
    {
        UdvmInstructionDescription *pDescr = descr;
        RvUint32 i;
        /*SigCompUdvm  *pUdvm = pSelfUdvm;*/
        RvUint32 nFixed = pDescr->nFixedOperands;
        OperandDecoderEnum *pCurDecoder = pDescr->opDecoders;
        RvUint16 *pCurOperand = operandsDecoder.operands;
        UdvmAddr curAddr = (RvUint16) (pc + 1);
        /*SigCompUdvm  *pSelfUdvm = pUdvm;*/
        
        rv = RV_OK;

        for(i = 0; i < nFixed; i++, pCurDecoder++, pCurOperand++)
        {
            RvUint16 opSize = 0;

            switch (*pCurDecoder)
            {
            case UDVM_MULTITYPE_OPERAND_DECODER:
                DecodeMultitypeOperandMacro(rv,pSelfUdvm,curAddr,(&opSize),pCurOperand);
                break;
            case UDVM_ADDRESS_OPERAND_DECODER:
                DecodeAddressOperandMacro(rv,pSelfUdvm,curAddr,(&opSize),pCurOperand);
                break;
            case UDVM_REFERENCE_OPERAND_DECODER:
                DecodeReferenceOperandMacro(rv,pSelfUdvm,curAddr,(&opSize),pCurOperand);
                break;
            case UDVM_LITERAL_OPERAND_DECODER:
                DecodeLiteralOperandMacro(rv,pSelfUdvm,curAddr,(&opSize),pCurOperand);
                break;
            }

            
            if(rv != RV_OK)
                break;

		    UdvmInstrLog(pUdvm->pLogSource, (pUdvm->pLogSource, 
			    gOperandsFormat[gOperandFormatOffsets[pDescr->opcode] + i],
			    *pCurOperand));
		    
            /* If instruction is variable operands instruction then it's
             * literal operand gives the number of groups
             */
            if(pDescr->nVarOperands > 0 && *pCurDecoder == UDVM_LITERAL_OPERAND_DECODER)
            {
                operandsDecoder.nVarGroupsNumber = *pCurOperand;
            }
            curAddr = (RvUint16) (curAddr + opSize);
        }
 
        if (rv == RV_OK)
        {
            operandsDecoder.nextOperandAddr = curAddr;
            operandsDecoder.instrDescr = pDescr;
        }
    }
    
    if(rv != RV_OK)
    {
        RvLogWarning(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
            "ExecuteInstruction - failure returning from OperandsDecoderDecodeStart (rv=%d)",
            rv));
        return rv;
    }

    switch (opcode)
    {
    case COMPARE:
        CompareInstrMacro(pSelfUdvm, opcode, &operandsDecoder, pRunningStatus,rv)
        break;
    case COPY_LITERAL:
        CopyLiteralInstrMacro(pSelfUdvm, opcode, &operandsDecoder, pRunningStatus,rv)
        break;
    case OUTPUT:
        OutputInstrMacro(pSelfUdvm, opcode, &operandsDecoder, pRunningStatus,rv)
        break;
    case LOAD:
        LoadInstrMacro(pSelfUdvm, opcode, &operandsDecoder, pRunningStatus,rv)
        break;
    case JUMP:
        JumpInstrMacro(pSelfUdvm, opcode, &operandsDecoder, pRunningStatus,rv)
        break;
    case AND:
        AndInstrMacro(pSelfUdvm, opcode, &operandsDecoder, pRunningStatus,rv)
        break;
    case OR:
        OrInstrMacro(pSelfUdvm, opcode, &operandsDecoder, pRunningStatus,rv)
        break;
    case NOT:
        NotInstrMacro(pSelfUdvm, opcode, &operandsDecoder, pRunningStatus,rv)
        break;
    case LSHIFT:
        LshiftInstrMacro(pSelfUdvm, opcode, &operandsDecoder, pRunningStatus,rv)
        break;
    case RSHIFT:
        RshiftInstrMacro(pSelfUdvm, opcode, &operandsDecoder, pRunningStatus,rv)
        break;
    case ADD:
        AddInstrMacro(pSelfUdvm, opcode, &operandsDecoder, pRunningStatus,rv)
        break;
    default:
        rv = (*descr->instr) (pSelfUdvm, opcode, &operandsDecoder, pRunningStatus);        
    }

    if(rv < 0)
    {
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
        RvLogWarning(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
            "ExecuteInstruction - failure executing instruction (%s)",
            descr->instrName));
#else
        RvLogWarning(pSelfUdvm->pLogSource,(pSelfUdvm->pLogSource,
            "ExecuteInstruction - failure executing instruction"));
#endif
        return rv;
    }

    if(!descr->bFlowControl)
    {
        SETPC(pSelfUdvm->pc = operandsDecoder.nextOperandAddr);
    }

    pSelfUdvm->totalInstructions++;
    return rv;
} /* end of ExecuteInstruction */


static RvStatus DecompressionFailureInstr(SigCompUdvm       *pSelfUdvm,
                                          UdvmOpcode        opcode,
                                          OperandsDecoder   *pDecoder,
                                          SigCompUdvmStatus *runningStatus)
{
	USE_INSTR_ARGS;
    return RV_ERROR_UNKNOWN;
}


static RvStatus AndInstr(SigCompUdvm        *pSelfUdvm,
                         UdvmOpcode         opcode,
                         OperandsDecoder    *pDecoder,
                         SigCompUdvmStatus  *runningStatus)
{
    RvUint8  *pMem;
    RvInt32  memSize    = (RvInt32)pSelfUdvm->udvmMemorySize;
    RvUint16 *operands  = pDecoder->operands;
    UdvmAddr op1Ref     = *operands++;
    RvUint16 op2        = *operands++;
    RvUint16 op1;

	USE_INSTR_ARGS;

    pMem = pSelfUdvm->pUdvmMemory;
	
    SAFE_READ16(op1Ref, op1);
    op1 &= op2;
    WRITE16(op1Ref, op1);
    return RV_OK;
}


static RvStatus OrInstr(SigCompUdvm         *pSelfUdvm,
                        UdvmOpcode          opcode,
                        OperandsDecoder     *pDecoder,
                        SigCompUdvmStatus   *runningStatus)
{
    RvUint8  *pMem;
    RvInt32  memSize = (RvInt32)pSelfUdvm->udvmMemorySize;
    RvUint16 *operands = pDecoder->operands;
    UdvmAddr op1Ref = *operands++;
    RvUint16 op2 = *operands++;
    RvUint16 op1;

	USE_INSTR_ARGS;
	
    pMem = pSelfUdvm->pUdvmMemory;
    SAFE_READ16(op1Ref, op1);
    op1 |= op2;
    WRITE16(op1Ref, op1);
    return RV_OK;
}


static RvStatus NotInstr(SigCompUdvm        *pSelfUdvm,
                         UdvmOpcode         opcode,
                         OperandsDecoder    *pDecoder,
                         SigCompUdvmStatus  *runningStatus)
{
    RvUint8  *pMem;
    RvInt32  memSize = (RvInt32)pSelfUdvm->udvmMemorySize;
    UdvmAddr op1Ref;
    RvUint16 op1;

	USE_INSTR_ARGS;
	
    pMem    = pSelfUdvm->pUdvmMemory;
    op1Ref  = pDecoder->operands[0];

    SAFE_READ16(op1Ref, op1);
    op1 = (RvUint16) (~op1);
    WRITE16(op1Ref, op1);
    return RV_OK;
}

static RvStatus LshiftInstr(SigCompUdvm         *pSelfUdvm,
                            UdvmOpcode          opcode,
                            OperandsDecoder     *pDecoder,
                            SigCompUdvmStatus   *runningStatus)
{
    RvUint8  *pMem;
    RvInt32  memSize;
    RvUint16 *operands  = pDecoder->operands;
    UdvmAddr op1Ref     = *operands++;
    RvUint16 op2        = *operands++;
    RvUint16 op1;

	USE_INSTR_ARGS;
	
    pMem    = pSelfUdvm->pUdvmMemory;
    memSize = (RvInt32)pSelfUdvm->udvmMemorySize;

    SAFE_READ16(op1Ref, op1);
    op1 <<= op2;
    WRITE16(op1Ref, op1);
    return RV_OK;
}


static RvStatus RshiftInstr(SigCompUdvm         *pSelfUdvm,
                            UdvmOpcode          opcode,
                            OperandsDecoder     *pDecoder,
                            SigCompUdvmStatus   *runningStatus)
{
    RvUint8  *pMem;
    RvInt32  memSize;
    RvUint16 *operands  = pDecoder->operands;
    UdvmAddr op1Ref     = *operands++;
    RvUint16 op2        = *operands++;
    RvUint16 op1;

	USE_INSTR_ARGS;
	
    pMem = pSelfUdvm->pUdvmMemory;
    memSize = (RvInt32)pSelfUdvm->udvmMemorySize;

    SAFE_READ16(op1Ref, op1);
    op1 >>= op2;
    WRITE16(op1Ref, op1);
    return RV_OK;
}


static RvStatus AddInstr(SigCompUdvm        *pSelfUdvm,
                         UdvmOpcode         opcode,
                         OperandsDecoder    *pDecoder,
                         SigCompUdvmStatus  *runningStatus)
{
    RvUint8  *pMem;
    RvInt32   memSize;
    RvUint16 *operands  = pDecoder->operands;
    UdvmAddr op1Ref     = *operands++;
    RvUint16 op2        = *operands++;
    RvUint16 op1;

	USE_INSTR_ARGS;
	
    pMem = pSelfUdvm->pUdvmMemory;
    memSize = (RvInt32)pSelfUdvm->udvmMemorySize;

    SAFE_READ16(op1Ref, op1);
    op1 = (RvUint16) (op1 + op2);
    WRITE16(op1Ref, op1);
    return RV_OK;
}


static RvStatus SubtractInstr(SigCompUdvm       *pSelfUdvm,
                              UdvmOpcode        opcode,
                              OperandsDecoder   *pDecoder,
                              SigCompUdvmStatus *runningStatus)
{
    RvUint8  *pMem;
    RvInt32  memSize;
    RvUint16 *operands  = pDecoder->operands;
    UdvmAddr op1Ref     = *operands++;
    RvUint16 op2        = *operands++;
    RvUint16 op1;

	USE_INSTR_ARGS;
	
    pMem    = pSelfUdvm->pUdvmMemory;
    memSize = (RvInt32)pSelfUdvm->udvmMemorySize;

    SAFE_READ16(op1Ref, op1);
    op1 = (RvUint16) (op1 - op2);
    WRITE16(op1Ref, op1);
    return RV_OK;
}


static RvStatus MultiplyInstr(SigCompUdvm       *pSelfUdvm,
                              UdvmOpcode        opcode,
                              OperandsDecoder   *pDecoder,
                              SigCompUdvmStatus *runningStatus)
{
    RvUint8  *pMem;
    RvInt32  memSize;
    RvUint16 *operands  = pDecoder->operands;
    UdvmAddr op1Ref     = *operands++;
    RvUint16 op2        = *operands++;
    RvUint16 op1;

	USE_INSTR_ARGS;
	
    pMem    = pSelfUdvm->pUdvmMemory;
    memSize = (RvInt32)pSelfUdvm->udvmMemorySize;
    
    SAFE_READ16(op1Ref, op1);
    op1 = (RvUint16) (op1 * op2);
    WRITE16(op1Ref, op1);
    return RV_OK;
}


static RvStatus DivideInstr(SigCompUdvm         *pSelfUdvm,
                            UdvmOpcode          opcode,
                            OperandsDecoder     *pDecoder,
                            SigCompUdvmStatus   *runningStatus)
{
    RvUint8  *pMem;
    RvInt32  memSize;
    RvUint16 *operands  = pDecoder->operands;
    UdvmAddr op1Ref     = *operands++;
    RvUint16 op2        = *operands++;
    RvUint16 op1;

	USE_INSTR_ARGS;
	
    pMem    = pSelfUdvm->pUdvmMemory;
    memSize = (RvInt32)pSelfUdvm->udvmMemorySize;
    
    SAFE_READ16(op1Ref, op1);
    if(op2 == 0)
    {
        pSelfUdvm->reason = UDVM_ZERO_DIVISION;
        return RV_ERROR_UNKNOWN;
    }
    op1 = (RvUint16) (op1 / op2);
    WRITE16(op1Ref, op1);
    return RV_OK;
}


static RvStatus RemainderInstr(SigCompUdvm *pSelfUdvm,
                               UdvmOpcode opcode,
                               OperandsDecoder *pDecoder,
                               SigCompUdvmStatus *runningStatus)
{
    RvUint8  *pMem;
    RvInt32  memSize;
    RvUint16 *operands = pDecoder->operands;
    UdvmAddr op1Ref = *operands++;
    RvUint16 op2 = *operands++;
    RvUint16 op1;

	USE_INSTR_ARGS;
	
    pMem = pSelfUdvm->pUdvmMemory;
    memSize = (RvInt32)pSelfUdvm->udvmMemorySize;

    SAFE_READ16(op1Ref, op1);
    if(op2 == 0)
    {
        pSelfUdvm->reason = UDVM_ZERO_DIVISION;
        return RV_ERROR_UNKNOWN;
    }
    op1 %= op2;
    WRITE16(op1Ref, op1);
    return RV_OK;
}


typedef struct
{
    RvUint16 *firstList;
    RvUint16 nLists;
    RvUint16 listSize;
    RvInt    mode;      /* if ascending mode == 1, descending mode == -1 */
} UdvmSortingContext;

static void SwapEntries(UdvmSortingContext *pCtx, RvUint16 *p1, RvUint16 *p2)
{
    int n = pCtx->nLists;
    int listSize = pCtx->listSize;

    while(n--)
    {
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;

        p1 += listSize;
        p2 += listSize;
    }

}

static int Compare(UdvmSortingContext *ctx, RvUint16 *p1, RvUint16 *p2)
{
   RvUint8 *pc1 = (RvUint8 *)p1;
   RvUint8 *pc2 = (RvUint8 *)p2;

   RvUint16 v1 = (RvUint16) ((*pc1 << 8) + *(pc1 + 1));
   RvUint16 v2 = (RvUint16) ((*pc2 << 8) + *(pc2 + 1));

   return ctx->mode * (v1 - v2);
}

static int Log2(RvUint16 n)
{
    int i = 0;
    RvUint16 n1 = n;

    while(n1 >>= 1)
    {
        i++;
    }

    i += ((1 << i) == n) ? 0 : 1;
    return i;
}

static void UdvmReverse(UdvmSortingContext *ctx,
                        RvUint16 *first,
                        RvUint16 *last)
{
    RvUint32 len = (RvUint32)((last) - (first));

    while(len > 1)
    {
        (last--);
        SwapEntries(ctx, first, last);
        (first++);
        len -= 2;
    }

}

static RvUint16* UdvmRotate(UdvmSortingContext* ctx,
                            RvUint16 *first,
                            RvUint16 *middle,
                            RvUint16 *last)
{
    if(((middle) - (first)) == 0)
    {
        return last;
    }

    if(((middle) - (last)) == 0)
    {
        return first;
    }

    UdvmReverse(ctx, first, middle);
    UdvmReverse(ctx, middle, last);

    while(((middle) - (first)) != 0 && ((last) - (middle)) != 0)
    {
        (last--);
        SwapEntries(ctx, first, last);
        (first++);
    }

    if(((middle) - (first)) == 0)
    {
        UdvmReverse(ctx, middle, last);
        return last;
    }

    UdvmReverse(ctx, first, middle);
    return first;
}



static RvUint16* UdvmLowerBound(UdvmSortingContext*  ctx,
                             RvUint16 *first,
                             RvUint16 *last,
                             RvUint16 *val)
{
    RvUint32 len = (RvUint32)((last) - (first));
    RvUint32 half;
    RvUint16 *middle;
    int rv;

    while(len > 0)
    {
        half = len >> 1;
        middle = (first + half);
        rv = Compare(ctx, middle, val);

        if(rv < 0)
        {
            first = (middle + 1);
            len -= half + 1;
            continue;
        }

        len = half;

    }

    return first;
}

static RvUint16* UdvmUpperBound(UdvmSortingContext *ctx,
                             RvUint16 *first,
                             RvUint16 *last,
                             RvUint16 *val)
{
    RvUint32 len = (RvUint32)((last) - (first));
    RvUint32 half;
    RvUint16 *middle;
    int rv;

    while(len > 0)
    {
        half = len >> 1;
        middle = (first + half);
        rv = Compare(ctx, middle, val);

        if(rv > 0)
        {
            len = half;
            continue;
        }

        first = (middle + 1);
        len -= half + 1;

    }

    return first;
}

static void UdvmMergeWithoutBuffer(
                            UdvmSortingContext  *ctx,
                            RvUint16            *first,
                            RvUint16            *middle,
                            RvUint16            *last,
                            RvUint32            len1,
                            RvUint32            len2)
{
    if(len1 == 0 || len2 == 0)
    {
        return;
    }

    if(len1 + len2 == 2)
    {
        if(Compare(ctx, middle, first) < 0)
        {
            SwapEntries(ctx, first, middle);
        }

        return;
    }

    {
        RvUint16 *firstCut = first;
        RvUint16 *secondCut = middle;
        RvUint16 *newMiddle;
        RvUint32 len11 = 0;
        RvUint32 len22 = 0;

        if(len1 > len2)
        {
            len11 = len1 / 2;
            (firstCut += len11);
            secondCut = UdvmLowerBound(ctx, middle, last, firstCut);
            len22 += (RvUint32)((secondCut) - (middle));
        } else
        {
            len22 = len2 / 2;
            (secondCut += len22);
            firstCut = UdvmUpperBound(ctx, first, middle, secondCut);
            len11 = (RvUint32)((firstCut) - (first));
        }

        newMiddle = UdvmRotate(ctx, firstCut, middle, secondCut);
        UdvmMergeWithoutBuffer(ctx, first, firstCut, newMiddle, len11, len22);
        UdvmMergeWithoutBuffer(ctx, newMiddle, secondCut, last, len1 - len11,
            len2 - len22);
    }
}


void UdvmInplaceStableSort(UdvmSortingContext *ctx,
                           RvUint16           *first,
                           RvUint16           *last)
{
    RvUint16 *middle;

    RvUint32 len = (RvUint32)((last) - (first));

    if(len <= 2)
    {
        (last--);
        if(Compare(ctx, first, last) > 0)
        {
            SwapEntries(ctx, first, last);
            return;
        }

        return;

    }

    middle = (first + ((last) - (first)) / 2) ;

    UdvmInplaceStableSort(ctx, first, middle);
    UdvmInplaceStableSort(ctx, middle, last);

    UdvmMergeWithoutBuffer(ctx, first, middle, last,
                         (RvUint32)((middle) - (first)),
                         (RvUint32)((last) - (middle)));
}


/*
SORT-ASCENDING (%start, %n, %k)
*/

static RvStatus SortInstr(SigCompUdvm       *pSelfUdvm,
                          UdvmOpcode        opcode,
                          OperandsDecoder   *pDecoder,
                          SigCompUdvmStatus *runningStatus)
{
    RvUint16 *operands  = pDecoder->operands;
    UdvmAddr start      = *operands++;
    RvUint16 nLists     = *operands++;
    RvUint16 listSize   = *operands++;
    UdvmSortingContext ctx;

	USE_INSTR_ARGS;
	
    if((RvUint32)(start + nLists * listSize) > pSelfUdvm->udvmMemorySize)
    {
        pSelfUdvm->reason = UDVM_MEM_ACCESS;
        return RV_ERROR_UNKNOWN;
    }

    ctx.firstList = (RvUint16 *)(pSelfUdvm->pUdvmMemory + start);
    ctx.nLists = nLists;
    ctx.listSize = listSize;
    ctx.mode = (opcode == SORT_ASCENDING) ? 1 : -1;

    UdvmInplaceStableSort(&ctx, ctx.firstList, ctx.firstList + ctx.listSize);
    pSelfUdvm->totalCycles += listSize * (Log2(listSize) + nLists);

    return RV_OK;
}


static void Sha1Traverser(void *cookie,
                              RvUint8 * buf,
                              RvUint16 bufSize,
							  RvStatus *pRv)
{
    RvSigCompSHA1Context *ctx = (RvSigCompSHA1Context *) cookie;

    if(RvSigCompSHA1Input(ctx, buf, bufSize) != 0)
    {
        /* error in input to SHA1 */
        *pRv = RV_ERROR_UNKNOWN;
    }
    
    *pRv = RV_OK;
}


static RvStatus UdvmSha1(SigCompUdvm *pSelfUdvm,
                         UdvmAddr    position,
                         RvUint16    length,
                         UdvmAddr    destination)
{
    CyclicBuffer		  cb;
    RvSigCompSHA1Context  sha1;
    RvStatus			  rv;
    RvUint8				  digest[RVSIGCOMP_SHA1_HASH_SIZE];
	RvUint8				 *pDigest = digest; /* To be forwarded as by-reference parameter to a macro */

    CyclicBufferConstruct(&cb, pSelfUdvm, position, length);
    
    if (RvSigCompSHA1Reset(&sha1) != 0) 
    {
        /* error in starting SHA1 */
        return RV_ERROR_UNKNOWN;
    }
    
    rv = CyclicBufferTraverse(&cb, Sha1Traverser, &sha1, UDVM_CYCLIC_BUF_SHA1_TRAVERSER);
    if(rv != RV_OK)
    {
        return rv;
        
    }
    
    if (RvSigCompSHA1Result(&sha1, digest) != 0) 
    {
        /* error in calculating SHA1 */
        return RV_ERROR_UNKNOWN;
    }
    
    CyclicBufferConstruct(&cb, pSelfUdvm, destination, sizeof(digest));
	rv = CyclicBufferTraverse(&cb, NULL, &pDigest,
			UDVM_CYCLIC_BUF_COPY_FROM_LINEAR_TRAVERSER);
    
    return rv;
}

static RvStatus Sha1Instr(SigCompUdvm       *pSelfUdvm,
                          UdvmOpcode        opcode,
                          OperandsDecoder   *pDecoder,
                          SigCompUdvmStatus *runningStatus)
{
    RvUint16 *operands   = pDecoder->operands;
    UdvmAddr position    = *operands++;
    RvUint16 length      = *operands++;
    UdvmAddr destination = *operands++;

	
    RvStatus rv = UdvmSha1(pSelfUdvm, position, length, destination);

	USE_INSTR_ARGS;
	
    if(rv == RV_OK)
    {
        pSelfUdvm->totalCycles += length;
    }

    return rv;
}


static RvStatus LoadInstr(SigCompUdvm       *pSelfUdvm,
                          UdvmOpcode        opcode,
                          OperandsDecoder   *pDecoder,
                          SigCompUdvmStatus *runningStatus)
{
    RvUint8  *pMem;
    RvInt32  memSize;
    RvUint16 *operands = pDecoder->operands;
    RvUint16 addr = operands[0];
    RvUint16 val = operands[1];

	USE_INSTR_ARGS;

    pMem    = pSelfUdvm->pUdvmMemory;
    memSize = (RvInt32)pSelfUdvm->udvmMemorySize;

    SAFE_WRITE16(addr, val);
    return RV_OK;
}



static RvStatus MultiloadInstr(SigCompUdvm       *pSelfUdvm,
                               UdvmOpcode        opcode,
                               OperandsDecoder   *pDecoder,
                               SigCompUdvmStatus *runningStatus)
{
    RvUint8  *pMem;
    RvInt32  memSize;
    RvUint16 *operands  = pDecoder->operands;
    UdvmAddr curAddr    = operands[0];
    RvUint16 n          = operands[1];
    RvStatus rv;
    UdvmAddr instrStart;
    UdvmAddr instrEnd;
    UdvmAddr writeStart = curAddr;
    UdvmAddr writeEnd   = (UdvmAddr) (curAddr + 2 * n - 1);

	USE_INSTR_ARGS;

    pMem       = pSelfUdvm->pUdvmMemory;
    memSize    = (RvInt32)pSelfUdvm->udvmMemorySize;
    instrStart = pSelfUdvm->pc;

    while((rv = OperandsDecoderDecodeNext(pDecoder, pSelfUdvm)) == RV_OK)
    {
        RvUint16 curVal = operands[0];

        SAFE_WRITE16(curAddr, curVal);
        curAddr += 2;
    }

    instrEnd = (RvUint16) (pDecoder->nextOperandAddr - 1);

    if((writeStart >= instrStart && writeStart <= instrEnd) ||
        (writeEnd >= instrStart && writeEnd <= instrEnd))
    {
        pSelfUdvm->reason = UDVM_MEM_ACCESS;
        return RV_ERROR_UNKNOWN;
    }

    if(rv == DECODE_FINISHED)
    {
        pSelfUdvm->totalCycles += n;
        return RV_OK;
    }

    return rv;
}

static RvStatus UdvmPush(SigCompUdvm *pSelfUdvm,
                         RvUint16    val)
{
    RvUint8  *pMem   = pSelfUdvm->pUdvmMemory;
    RvInt32  memSize = (RvInt32)pSelfUdvm->udvmMemorySize;
    UdvmAddr stackLocation;
    RvUint16 stackFill;
    UdvmAddr addr;

	SL(stackLocation)
	
	SAFE_READ16(stackLocation, stackFill);
    addr = (RvUint16) (stackLocation + stackFill * 2 + 2);
    SAFE_WRITE16(addr, val);
    stackFill++;
    WRITE16(stackLocation, stackFill);
    return RV_OK;
}

static RvStatus UdvmPop(SigCompUdvm *pSelfUdvm,
                        RvUint16 *pVal)
{
    RvUint8  *pMem   = pSelfUdvm->pUdvmMemory;
    RvInt32  memSize = (RvInt32)pSelfUdvm->udvmMemorySize;
    UdvmAddr stackLocation;
    RvUint16 stackFill;
    UdvmAddr addr;

	SL(stackLocation)

    SAFE_READ16(stackLocation, stackFill);
    if(stackFill == 0)
    {
        pSelfUdvm->reason = UDVM_STACK_UNDERFLOW;
        return RV_ERROR_UNKNOWN;
    }
    --stackFill;
    addr = (RvUint16) (stackLocation + stackFill * 2 + 2);
    SAFE_READ16(addr, *pVal);
    WRITE16(stackLocation, stackFill);

    return RV_OK;
}

static RvStatus PushInstr(SigCompUdvm       *pSelfUdvm,
                          UdvmOpcode        opcode,
                          OperandsDecoder   *pDecoder,
                          SigCompUdvmStatus *runningStatus)
{
   	USE_INSTR_ARGS;
	
    return UdvmPush(pSelfUdvm, pDecoder->operands[0]);
}


static RvStatus PopInstr(SigCompUdvm        *pSelfUdvm,
                         UdvmOpcode         opcode,
                         OperandsDecoder    *pDecoder,
                         SigCompUdvmStatus  *runningStatus)
{
    RvUint8  *pMem;
    RvInt32  memSize;
    RvUint16 popedVal;
    UdvmAddr addr;
    RvStatus rv;

	USE_INSTR_ARGS;
	
    pMem    = pSelfUdvm->pUdvmMemory;
    memSize = (RvInt32)pSelfUdvm->udvmMemorySize;
    addr    = pDecoder->operands[0];

    rv = UdvmPop(pSelfUdvm, &popedVal);
    if(rv != RV_OK)
    {
        return rv;
    }

    SAFE_WRITE16(addr, popedVal);
    return RV_OK;
}


static RvStatus CopyInstr(SigCompUdvm       *pSelfUdvm,
                          UdvmOpcode        opcode,
                          OperandsDecoder   *pDecoder,
                          SigCompUdvmStatus *runningStatus)
{
    RvUint16 *operands   = pDecoder->operands;
    UdvmAddr position    = *operands++;
    RvUint16 length      = *operands++;
    UdvmAddr destination = *operands++;

    RvStatus rv;

	USE_INSTR_ARGS;
	
	HandleTraverseCopy(pSelfUdvm,position,&destination,length,&rv); 
    if(rv == RV_OK)
    {
        pSelfUdvm->totalCycles += length;
    }

    return rv;
}

/*
 *  COPY-LITERAL (%position, %length, $destination)
 */

static RvStatus CopyLiteralInstr(SigCompUdvm        *pSelfUdvm,
                                 UdvmOpcode         opcode,
                                 OperandsDecoder    *pDecoder,
                                 SigCompUdvmStatus  *runningStatus)
{
    RvUint8  *pMem;
    RvInt32  memSize;
    RvUint16 *operands  = pDecoder->operands;
    UdvmAddr position   = *operands++;
    RvUint16 length     = *operands++;
    RvUint16 destinationRef = *operands++;
    UdvmAddr destination;
    RvStatus rv;

	USE_INSTR_ARGS;
	
    pMem    = pSelfUdvm->pUdvmMemory;
    memSize = (RvInt32)pSelfUdvm->udvmMemorySize;

    SAFE_READ16(destinationRef, destination);

	HandleTraverseCopy(pSelfUdvm,position,&destination,length,&rv); 
    if(rv != RV_OK)
    {
        return rv;
    }

    SAFE_WRITE16(destinationRef, destination);
    pSelfUdvm->totalCycles += length;

    return RV_OK;
}


/*
 *  COPY-OFFSET (%offset, %length, $destination)
 */
static RvStatus CopyOffsetInstr(SigCompUdvm       *pSelfUdvm,
                                UdvmOpcode        opcode,
                                OperandsDecoder   *pDecoder,
                                SigCompUdvmStatus *runningStatus)
{
    RvUint8  *pMem;
    RvInt32  memSize;
    RvUint16 *operands  = pDecoder->operands;
    RvUint16 offset     = *operands++;
    RvUint16 length     = *operands++;
    UdvmAddr destinationRef = *operands++;
    UdvmAddr destination;
    RvStatus rv;
    CyclicBuffer cbSrc;
    CyclicBuffer cbDest;

	USE_INSTR_ARGS;
	
    pMem    = pSelfUdvm->pUdvmMemory;
    memSize = (RvInt32)pSelfUdvm->udvmMemorySize;

    SAFE_READ16(destinationRef, destination);

    CyclicBufferConstruct(&cbSrc, pSelfUdvm, 0, length);
    CyclicBufferConstruct(&cbDest, pSelfUdvm, destination, 0);
    CyclicBufferComputeStart(&cbSrc, destination, offset);

    rv = CyclicBufferTraverse(&cbSrc, NULL, &cbDest, UDVM_CYCLIC_BUF_COPY_TO_CYCLIC_TRAVERSER);
    if(rv != RV_OK)
    {
        return rv;
    }

    SAFE_WRITE16(destinationRef, cbDest.start);
    pSelfUdvm->totalCycles += length;
    return RV_OK;
}


typedef struct
{
    RvUint8 nextVal;
    RvUint8 offset;
} MemsetTraverserCtx;

static void MemsetTraverser(void     *cookie,
                                RvUint8  *buf,
                                RvUint16 bufsize,
								RvStatus *pRv)
{
    MemsetTraverserCtx *pCtx = (MemsetTraverserCtx *) cookie;
    int i;

    for(i = 0; i < bufsize; i++)
    {
        *buf++ = pCtx->nextVal;
        pCtx->nextVal = (RvUint8) (pCtx->nextVal + pCtx->offset);
    }

    *pRv = RV_OK;

}

static RvStatus MemsetInstr(SigCompUdvm       *pSelfUdvm,
                            UdvmOpcode        opcode,
                            OperandsDecoder   *pDecoder,
                            SigCompUdvmStatus *runningStatus)
{
    RvUint16 *operands  = pDecoder->operands;
    RvUint16 address    = operands[0];
    RvUint16 length     = operands[1];
    RvUint8  cur        = (RvUint8) operands[2];
    RvUint8  offset     = (RvUint8) operands[3];
    RvStatus rv;
    MemsetTraverserCtx ctx;
    CyclicBuffer cb;

	USE_INSTR_ARGS;
	
    ctx.nextVal = cur;
    ctx.offset = offset;


    CyclicBufferConstruct(&cb, pSelfUdvm, address, length);
    rv = CyclicBufferTraverse(&cb, MemsetTraverser, &ctx, UDVM_CYCLIC_BUF_MEMSET_TRAVERSER);

    pSelfUdvm->totalCycles += length;
    return rv;
}


static RvStatus JumpInstr(SigCompUdvm       *pSelfUdvm,
                          UdvmOpcode        opcode,
                          OperandsDecoder   *pDecoder,
                          SigCompUdvmStatus *runningStatus)
{

	USE_INSTR_ARGS;
	
    SETPC(pSelfUdvm->pc = pDecoder->operands[0]);
    return RV_OK;
}

/* COMPARE (%value_1, %value_2, @address_less, @address_equal, @address_greater) */

static RvStatus CompareInstr(SigCompUdvm        *pSelfUdvm,
                             UdvmOpcode         opcode,
                             OperandsDecoder    *pDecoder,
                             SigCompUdvmStatus  *runningStatus)
{
    RvUint16 *operands  = pDecoder->operands;
    RvUint16 val1       = *operands++;
    RvUint16 val2       = *operands++;

	USE_INSTR_ARGS;
	
	if(val1 < val2)
    {
        SETPC(pSelfUdvm->pc = operands[0]);
    } else if(val1 == val2)
    {
        SETPC(pSelfUdvm->pc = operands[1]);
    } else
    {
        SETPC(pSelfUdvm->pc = operands[2]);
    }

    return RV_OK;
}

/*
 * CALL (@address)
 */

static RvStatus CallInstr(SigCompUdvm       *pSelfUdvm,
                          UdvmOpcode        opcode,
                          OperandsDecoder   *pDecoder,
                          SigCompUdvmStatus *runningStatus)
{

    RvStatus rv;
    UdvmAddr returnAddr;
    UdvmAddr jumpAddr;

	USE_INSTR_ARGS;

    returnAddr = pDecoder->nextOperandAddr;
    jumpAddr = pDecoder->operands[0];

    rv = UdvmPush(pSelfUdvm, returnAddr);
    if(rv != RV_OK)
    {
        return rv;
    }

    SETPC(pSelfUdvm->pc = jumpAddr);
    return RV_OK;
}


static RvStatus ReturnInstr(SigCompUdvm       *pSelfUdvm,
                            UdvmOpcode        opcode,
                            OperandsDecoder   *pDecoder,
                            SigCompUdvmStatus *runningStatus)
{
    UdvmAddr jumpAddr;
    RvStatus rv;

   	USE_INSTR_ARGS;
	
	rv = UdvmPop(pSelfUdvm, &jumpAddr);

    if(rv != RV_OK)
    {
        return rv;
    }

    SETPC(pSelfUdvm->pc = jumpAddr);
    return RV_OK;
}

/*
 *   SWITCH (#n, %j, @address_0, @address_1, ... , @address_n-1)
 */

static RvStatus SwitchInstr(SigCompUdvm       *pSelfUdvm,
                            UdvmOpcode        opcode,
                            OperandsDecoder   *pDecoder,
                            SigCompUdvmStatus *runningStatus)
{
    RvUint16 *operands  = pDecoder->operands;
    RvUint16 n          = *operands++;
    RvUint   j          = *operands++;

   	USE_INSTR_ARGS;
	
	if(j >= n)
    {
        
        pSelfUdvm->reason = UDVM_ILLEGAL_OPERAND;
        return RV_ERROR_UNKNOWN;
    }


    do
    {
        OperandsDecoderDecodeNext(pDecoder, pSelfUdvm); /*lint !e534*/
        /* if decoding failed, then skip to next ... */
    } while(j--);

    
    SETPC(pSelfUdvm->pc = pDecoder->operands[0]);
    
    pSelfUdvm->totalCycles += n;
    return RV_OK;
}


/* CRC (%value, %position, %length, @address)
 */

static void CrcTraverser(void *cookie, RvUint8 *mem, RvUint16 size,RvStatus *pRv)
{
    *pRv = SigCompCrc16Calc(mem, size, (RvUint16 *)cookie);
}


#define INITFCS16    0xffff  /* Initial FCS value */

static RvStatus CrcInstr(SigCompUdvm        *pSelfUdvm,
                         UdvmOpcode         opcode,
                         OperandsDecoder    *pDecoder,
                         SigCompUdvmStatus  *runningStatus)
{
    RvUint16 *operands = pDecoder->operands;
    RvUint16 value    = *operands++;
    UdvmAddr position = *operands++;
    RvUint16 length   = *operands++;
    UdvmAddr jumpAddr = *operands;
    CyclicBuffer cb;
    RvStatus rv;
    RvUint16 crc = INITFCS16;

	USE_INSTR_ARGS;
	
	CyclicBufferConstruct(&cb, pSelfUdvm, position, length);
    rv = CyclicBufferTraverse(&cb, CrcTraverser, &crc, UDVM_CYCLIC_BUF_CRC_TRAVERSER);

    if(rv != RV_OK)
    {
        return rv;
    }


    
    SETPC(pSelfUdvm->pc = (RvUint16) ((value == crc) ? pDecoder->nextOperandAddr : jumpAddr));
    
    pSelfUdvm->totalCycles += length;
    return RV_OK;
}


static RvStatus InputBytesInstr(SigCompUdvm       *pSelfUdvm,
                                UdvmOpcode        opcode,
                                OperandsDecoder   *pDecoder,
                                SigCompUdvmStatus *runningStatus)
{
    RvUint16 *operands  = pDecoder->operands;
    RvUint16 length     = operands[0];
    UdvmAddr start      = operands[1];
    CyclicBuffer cb;
    RvStatus rv;

	USE_INSTR_ARGS;
	
    /* Get rid of partial bits from previous
    * INPUT-BITS, INPUT-HUFFMAN instruction
    */

    pSelfUdvm->curBits = 0;

	/* No input requested */
    if(length == 0)
    {
        
        SETPC(pSelfUdvm->pc = pDecoder->nextOperandAddr);
        return RV_OK;
    }

    /* Pay attention that number of cycles used is increased, even
     * that eventually this instruction may fail
     */
    pSelfUdvm->totalCycles  += length;

    /*
     *  Check if there is enough input
     */
    UdvmRequire(pSelfUdvm, length, rv);

    if(rv != RV_OK)
    {
        
        SETPC(pSelfUdvm->pc = operands[2]);
        return RV_OK;
    }

    CyclicBufferConstruct(&cb, pSelfUdvm, start, length);
    rv = CyclicBufferTraverse(&cb, NULL, pSelfUdvm, UDVM_CYCLIC_BUF_INPUT_BYTES_TRAVERSER);

    if(rv != RV_OK)
    {
        return rv;
    }

    SETPC(pSelfUdvm->pc = pDecoder->nextOperandAddr);
    pSelfUdvm->targetCycles += 8 * length *pSelfUdvm->cyclesPerBit;
    /*lint +e613*/
    return RV_OK;
}


static RvStatus UdvmInputBits(SigCompUdvm *pSelfUdvm,
                              RvUint      nBitsIn,
                              RvBool      fhflag,
                              RvUint16    *pRet,
                              RvUint      *nBitsOut)
{
    RvUint8  *pMem   = pSelfUdvm->pUdvmMemory;

    RvUint16 ibo;
    RvBool   pbit;
    RvUint   curBits;
    RvInt    newBits;
    RvUint32 bitbuf;
    RvUint16 res;

    if(nBitsIn > 16)
    {
        pSelfUdvm->reason = UDVM_ILLEGAL_OPERAND;
        return RV_ERROR_UNKNOWN;
    }

    IBO(ibo);
    pbit = ibo & 1;

    /* If pbit was changed since last call, discard the byte fraction that
     * may be hold in buffer
     */
    if(pbit != pSelfUdvm->pbit)
    {
        pSelfUdvm->curBits = 0;
        pSelfUdvm->bitsBuffer = 0;
        pSelfUdvm->pbit = pbit;
    }
    
    if(nBitsIn == 0)
    {
        *nBitsOut = 0;
        return RV_OK;
    }

    bitbuf = pSelfUdvm->bitsBuffer;
    curBits = pSelfUdvm->curBits;
    newBits = nBitsIn - curBits;

    if(newBits > 0)
    {
        RvUint8 buf[2];
        RvUint  nBytes = (newBits <= 8) ? 1 : 2;
        RvStatus rv;

        buf[1] = 0;
        UdvmReadBytes(pSelfUdvm, buf, nBytes,rv);
        if(rv != RV_OK)
        {
            *nBitsOut = 0;
            return RV_OK;
        }

        if(pbit)
        {
            buf[0] = gReversedBytes[buf[0]];
            buf[1] = gReversedBytes[buf[1]];
        }

        /* push new bits into the bits buffer */
        bitbuf |= buf[0] << (24 - curBits) | buf[1] << (16 - curBits);

        curBits += nBytes * 8;
    }

    res = (RvUint16) (bitbuf >> (32 - nBitsIn));
    curBits -= nBitsIn;
    bitbuf <<= nBitsIn;
    bitbuf = (bitbuf >> (32 - curBits)) << (32 - curBits); 
    if(fhflag)
    {
        /* Now the resulting nBitsIn bits have to be reversed */
        res = (RvUint16) (ReverseBits16(res) >> (16 - nBitsIn));

    }

    *nBitsOut = nBitsIn;
    *pRet = res;
    pSelfUdvm->curBits = curBits;
    pSelfUdvm->bitsBuffer = bitbuf;

    return RV_OK;
}

/*
 *  INPUT-BITS (%length, %destination, @address)
 */
static RvStatus InputBitsInstr(SigCompUdvm      *pSelfUdvm,
                               UdvmOpcode       opcode,
                               OperandsDecoder  *pDecoder,
                               SigCompUdvmStatus *runningStatus)
{
    RvUint8  *pMem;
    RvInt32  memSize;
    RvUint16 *operands = pDecoder->operands;
    RvUint16 length = *operands++;
    UdvmAddr destination = *operands++;
    RvStatus rv;
    RvUint   nBitsOut;
    RvUint16 ibo;
    RvUint16 res;

   	USE_INSTR_ARGS;
	
    pMem    = pSelfUdvm->pUdvmMemory;
    memSize = (RvInt32)pSelfUdvm->udvmMemorySize;

    if(length == 0)
    {
        SETPC(pSelfUdvm->pc = pDecoder->nextOperandAddr);
        return RV_OK;
    }

    IBO(ibo);

    rv = UdvmInputBits(pSelfUdvm, length, ibo & 4, &res, &nBitsOut);
    if(rv != RV_OK)
    {
        return rv;
    }

    /* Not enough data, jump to the address specified by the third operand */
    if(nBitsOut != length)
    {
        SETPC(pSelfUdvm->pc = *operands);
        return RV_OK;
    }

    SAFE_WRITE16(destination, res);
    SETPC(pSelfUdvm->pc = pDecoder->nextOperandAddr);
    pSelfUdvm->targetCycles += length *pSelfUdvm->cyclesPerBit;

    return RV_OK;
    /*lint +e613*/
}


static RvStatus InputHuffmanInstr(SigCompUdvm       *pSelfUdvm,
                                  UdvmOpcode        opcode,
                                  OperandsDecoder   *pDecoder,
                                  SigCompUdvmStatus *runningStatus)
{
    RvUint8  *pMem;
    RvInt32  memSize;
    RvUint16 *operands = pDecoder->operands;
    UdvmAddr destination = *operands++;
    UdvmAddr jumpAddr = *operands++;
    RvUint16 n = *operands++;
    RvStatus rv;
    RvUint16 h          = 0;
    RvUint   totalBits  = 0;
    RvUint16 ibo;
    RvBool   hbit;
    RvUint16 curBits;
    RvUint16 curLowerBound;
    RvUint16 curUpperBound;
    RvUint16 curUncompressed;
    RvUint16 curVal;
    RvUint   outBits;

   	USE_INSTR_ARGS;
	
    pMem    = pSelfUdvm->pUdvmMemory;
    memSize = (RvInt32)pSelfUdvm->udvmMemorySize;

    IBO(ibo);
	hbit = ibo & 2;

    pSelfUdvm->totalCycles += n;

    do
    {
        rv = OperandsDecoderDecodeNext(pDecoder, pSelfUdvm);
        /* all groups where traversed and match wasn't found - report error */
        if(rv == DECODE_FINISHED)
        {
            pSelfUdvm->reason = UDVM_HUFFMAN_MATCH;
            return RV_ERROR_UNKNOWN;
        }

        if(rv != RV_OK)
        {
            return rv;
        }

        operands = pDecoder->operands;
        curBits = *operands++;
        curLowerBound = *operands++;
        curUpperBound = *operands++;
        curUncompressed = *operands++;

        if(curBits == 0)
        {
            continue;
        }

        totalBits += curBits;

        if(totalBits > 16)
        {
            pSelfUdvm->reason = UDVM_HUFFMAN_BITS;
            return RV_ERROR_UNKNOWN;
        }

        rv = UdvmInputBits(pSelfUdvm, curBits, hbit, &curVal, &outBits);
        if(rv != RV_OK)
        {
            return rv;
        }
		/* Increase the available UDVM cycles according to RFC3320 chapter 8.6    */
		/* The number of available UDVM cycles is increased by n * cycles_per_bit */
		pSelfUdvm->targetCycles += curBits *pSelfUdvm->cyclesPerBit;

        if(outBits != curBits)
        {
            SETPC(pSelfUdvm->pc = jumpAddr);
            return RV_OK;
        }

        h = (RvUint16) ((h << curBits) + curVal);

    } while(h < curLowerBound || h > curUpperBound);

    h = (RvUint16) (h + curUncompressed - curLowerBound);
    SAFE_WRITE16(destination, h);
    rv = OperandsDecoderSkipRest(pDecoder, pSelfUdvm);
    if(rv != RV_OK)
    {
        return rv;
    }

    SETPC(pSelfUdvm->pc = pDecoder->nextOperandAddr);
    return RV_OK;
    /*lint +e613*/
}

/*
STATE-ACCESS (%partial_identifier_start, %partial_identifier_length,
              %state_begin, %state_length, %state_address, %state_instruction)
*/

static RvStatus StateAccessInstr(SigCompUdvm        *pSelfUdvm,
                                 UdvmOpcode         opcode,
                                 OperandsDecoder    *pDecoder,
                                 SigCompUdvmStatus  *runningStatus)
{
    RvUint8  *pMem;
    RvInt32  memSize;
    RvUint16 *operands        = pDecoder->operands;
    UdvmAddr partialIdStart   = *operands++;
    RvUint16 partialIdLength  = *operands++;
    RvUint16 stateBegin       = *operands++;
    RvUint16 stateLength      = *operands++;
    UdvmAddr stateAddress     = *operands++;
    UdvmAddr stateInstruction = *operands++;
    RvStatus rv;
    SigCompStateHandlerMgr *pStateHandler;
    SigCompState *pState = 0;
    RvUint8      *pStateId;
    RvUint8      *pStateBegin;
    CyclicBuffer cb;

	USE_INSTR_ARGS;
    
    pMem    = pSelfUdvm->pUdvmMemory;
    memSize = (RvInt32)pSelfUdvm->udvmMemorySize;

    if(partialIdLength < 6 || partialIdLength > 20)
    {
        pSelfUdvm->reason = UDVM_ILLEGAL_OPERAND;
        return RV_ERROR_UNKNOWN;
    }

    if(partialIdStart + partialIdLength > memSize)
    {
        pSelfUdvm->reason = UDVM_MEM_ACCESS;
        return RV_ERROR_UNKNOWN;
    }

    pStateId = pMem + partialIdStart;

    pStateHandler = &((SigCompMgr *)pSelfUdvm->hManager)->stateHandlerMgr;
    rv = SigCompStateHandlerGetState(pStateHandler,
                                    pStateId,
                                    partialIdLength,
                                    RV_FALSE, /* don't ignore miminumAccessLength */
                                    &pState);

    if(rv != RV_OK)
	{
		pSelfUdvm->reason = UDVM_STATE_ACCESS;
		return rv;
	}
	
	if(stateAddress == 0)
	{
		stateAddress = pState->stateAddress;
	}
	
	if(stateInstruction == 0)
	{
		stateInstruction = pState->stateInstruction;
	}
	
	if(stateLength == 0)
	{
		stateLength = pState->stateDataSize;
	}
	
	if((stateBegin + stateLength > pState->stateDataSize) ||
		(stateBegin != 0 && stateLength == 0))
	{
		pSelfUdvm->reason = UDVM_ILLEGAL_OPERAND;
		return RV_ERROR_UNKNOWN;
	}
	
	pStateBegin = pState->pData + stateBegin;
	
	CyclicBufferConstruct(&cb, pSelfUdvm, stateAddress, stateLength);
    rv = CyclicBufferTraverse(&cb, NULL, &pStateBegin, UDVM_CYCLIC_BUF_COPY_FROM_LINEAR_TRAVERSER);
	
	if(rv != RV_OK)
	{
		return rv;
	}
	
	pSelfUdvm->totalCycles += stateLength;
	if(stateInstruction != 0)
	{
		SETPC(pSelfUdvm->pc = stateInstruction);
	} else
	{
		SETPC(pSelfUdvm->pc = pDecoder->nextOperandAddr);
	}

    return rv;
}

static void UdvmAddStateRequest(SigCompUdvm *pSelfUdvm,
                                SigCompStateRequest *pRequest,
                                SigCompStateRequestType requestType)
{
    pRequest->requestType = requestType;
    pRequest->pNext = 0;
    if(pSelfUdvm->pFirstStateRequest == 0)
    {
        pSelfUdvm->pFirstStateRequest = pRequest;
        pSelfUdvm->pLastStateRequest = pRequest;
        return;
    }

    pSelfUdvm->pLastStateRequest->pNext = pRequest;
    pSelfUdvm->pLastStateRequest = pRequest;
}

/* Adds new save state request  to the list of save state requests */
static RvStatus UdvmAddSaveStateRequest(SigCompUdvm *pSelfUdvm,
                                        RvUint16 stateLength,
                                        UdvmAddr stateAddress,
                                        UdvmAddr stateInstruction,
                                        RvUint16 minimumAccessLength,
                                        RvUint16 stateRetentionPriority)
{
    RvInt nextSaveState = pSelfUdvm->nextSaveState;
    SigCompSaveStateRequest *pSaveState;

    if((minimumAccessLength > 20 || minimumAccessLength < 6) ||
       stateRetentionPriority == 65535 || nextSaveState >= MAX_SAVE_STATES)
    {
        pSelfUdvm->reason = UDVM_STATE;
        return RV_ERROR_UNKNOWN;
    }


    pSaveState = &pSelfUdvm->saveStateRequestList[nextSaveState];
    pSelfUdvm->nextSaveState++;
    pSaveState->minimumAccessLength = minimumAccessLength;
    pSaveState->stateAddress = stateAddress;
    pSaveState->stateInstruction = stateInstruction;
    pSaveState->stateLength = stateLength;
    pSaveState->stateRetentionPriority = stateRetentionPriority;
    UdvmAddStateRequest(pSelfUdvm, (SigCompStateRequest *) pSaveState,
                        SAVE_STATE_REQUEST);

    return RV_OK;
}

/* Adds new free state request  to the list of free state requests */
static RvStatus UdvmAddFreeStateRequest(SigCompUdvm *pSelfUdvm,
                                        UdvmAddr partialIdStart,
                                        RvUint16 partialIdLength)
{
    RvInt nextFreeState = pSelfUdvm->nextFreeState;
    SigCompFreeStateRequest *pFreeState;

    if(nextFreeState >= MAX_FREE_STATES || partialIdLength > 20 ||
       partialIdLength < 6)
    {
        return RV_ERROR_UNKNOWN;

    }

    pFreeState = &pSelfUdvm->freeStateRequestList[nextFreeState];
    pSelfUdvm->nextFreeState++;
    pFreeState->partialIdentifierLength = partialIdLength;
    pFreeState->partialIdentifierStart = partialIdStart;
    UdvmAddStateRequest(pSelfUdvm, (SigCompStateRequest *) pFreeState,
                        FREE_STATE_REQUEST);
    return RV_OK;
}

/*
STATE-CREATE (%state_length, %state_address, %state_instruction,
              %minimum_access_length, %state_retention_priority)
*/

static RvStatus StateCreateInstr(SigCompUdvm *pSelfUdvm,
                                 UdvmOpcode opcode,
                                 OperandsDecoder *pDecoder,
                                 SigCompUdvmStatus *runningStatus)
{
    RvUint16 *operands              = pDecoder->operands;
    RvUint16 stateLength            = *operands++;
    UdvmAddr stateAddress           = *operands++;
    UdvmAddr stateInstruction       = *operands++;
    RvUint16 minimumAccessLength    = *operands++;
    RvUint16 stateRetentionPriority = *operands++;

	
	RvStatus rv = UdvmAddSaveStateRequest(pSelfUdvm, stateLength, stateAddress,
                                   stateInstruction, minimumAccessLength,
                                   stateRetentionPriority);
	USE_INSTR_ARGS;
	
    if(rv == RV_OK)
    {
        
        pSelfUdvm->totalCycles += stateLength;
    }

    return rv;
}


static RvStatus StateFreeInstr(SigCompUdvm       *pSelfUdvm,
                               UdvmOpcode        opcode,
                               OperandsDecoder   *pDecoder,
                               SigCompUdvmStatus *runningStatus)
{
    RvUint16 *operands = pDecoder->operands;
    UdvmAddr partialIdStart = operands[0];
    RvUint16 partialIdLength = operands[1];

	USE_INSTR_ARGS;
	
	return UdvmAddFreeStateRequest(pSelfUdvm, partialIdStart, partialIdLength);
}


static RvStatus OutputInstr(SigCompUdvm *pSelfUdvm,
                            UdvmOpcode opcode,
                            OperandsDecoder *pDecoder,
                            SigCompUdvmStatus *runningStatus)
{
    RvUint16     *operands  = pDecoder->operands;
    UdvmAddr     start      = operands[0];
    UdvmAddr     length     = operands[1];
    CyclicBuffer cb;
    RvStatus     rv;

   	USE_INSTR_ARGS;
	
	CyclicBufferConstruct(&cb, pSelfUdvm, start, length);
    rv = CyclicBufferTraverse(&cb, 
                             /*lint -e(611) This cast is OK*/
                             (CyclicBufferTraverseCb)NULL,
                             pSelfUdvm,
							 UDVM_CYCLIC_BUF_WRITE_BYTES);

    if(rv == RV_OK)
    {
        
        pSelfUdvm->totalCycles += length;
    }

    return rv;
}

/*
END-MESSAGE (%requested_feedback_location,
             %returned_parameters_location, %state_length, %state_address,
             %state_instruction, %minimum_access_length,
             %state_retention_priority)
*/

static RvStatus EndMessageInstr(SigCompUdvm         *pSelfUdvm,
                                UdvmOpcode          opcode,
                                OperandsDecoder     *pDecoder,
                                SigCompUdvmStatus   *runningStatus)
{
    RvUint16 *operands                  = pDecoder->operands;
    UdvmAddr requestedFeedbackLocation  = *operands++;
    UdvmAddr returnedParametersLocation = *operands++;
    RvUint16 stateLength                = *operands++;
    UdvmAddr stateAddress               = *operands++;
    UdvmAddr stateInstruction           = *operands++;
    RvUint16 minimumAccessLength        = *operands++;
    RvUint16 stateRetentionPriority     = *operands++;
    RvStatus rv;

	USE_INSTR_ARGS;
	
    /*lint -e{613} runningStatus never 0*/
    *runningStatus = UDVM_FINISHED;

    pSelfUdvm->requestedFeedbackLocation = requestedFeedbackLocation;
    pSelfUdvm->returnedParametersLocation = returnedParametersLocation;

    /* Here minimum_access_length violation or state_retention_priority
     * violations doesn't cause decompression failure
     */
    if(stateLength != 0 && minimumAccessLength <= 20 &&
        minimumAccessLength >= 6 && stateRetentionPriority < 65535)
    {
      rv = UdvmAddSaveStateRequest(pSelfUdvm, stateLength, stateAddress,
            stateInstruction, minimumAccessLength, stateRetentionPriority);
      if(rv != RV_OK)
      {
          return rv;
      }

    }

    if(requestedFeedbackLocation != 0 || returnedParametersLocation != 0 ||
        pSelfUdvm->pFirstStateRequest != 0)
    {
        /*lint -e{613} runningStatus never 0*/
        *runningStatus = UDVM_WAITING;
    }

    pSelfUdvm->totalCycles += stateLength;
    /*lint +e613*/
    return RV_OK;
}


#ifdef UDVM_DEBUG_SUPPORT
#include <stdio.h>
#include <stdarg.h>

static void EmptyOutputDebugString(void *cookie,
                                   const RvChar *msg)
{
}


void SigCompUdvmSetOutputDebugStringCb(SigCompUdvm *pSelfUdvm,
                                       OutputDebugStringCb cb,
                                       void *cookie)
{
    pSelfUdvm->outputDebugStringCb = cb == 0 ? EmptyOutputDebugString : cb;
    pSelfUdvm->outputDebugStringCookie = cookie;
}

static void DefaultOutputDebugString(void *cookie,
                                     const RvChar *msg)
{
    OutputDebugString(msg);
}

//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
#if defined(dprintf)
#undef dprintf
#endif
#endif
//SPIRENT_END

static void dprintf(SigCompUdvm *pSelfUdvm,
                    const RvChar *format,
                    ...)
{
    RvChar buff[1024];
    va_list args;

    va_start(args, format);

    /* return code not relevant */
    RvVsprintf(buff, format, args); /*lint !e534*/

    va_end(args);
    pSelfUdvm->outputDebugStringCb(pSelfUdvm->outputDebugStringCookie, buff);
}

static void TreatWatchpoint(SigCompUdvm *pSelfUdvm,
                            SigCompUdvmWatchpoint * pWatch)
{
    RvUint8  *pMem   = pSelfUdvm->pUdvmMemory;
    RvInt32  memSize = (RvInt32)pSelfUdvm->udvmMemorySize;
    UdvmAddr start = pWatch->startAddr;
    RvUint16 size = pWatch->size;
    RvInt32  i, j;
    RvUint8  *pCurChar;

    if(start + size > memSize)
    {
        size = (RvUint16)memSize - start;
    }

    dprintf(pSelfUdvm, "\nWatchpoint: %hu\n", pWatch->index);

    pCurChar = pMem + start;
    i = 0;
    while(i < size)
    {
        for(j = 0; j < 16 && i < size; i++, j++)
        {
            dprintf(pSelfUdvm, "%2.2x ", (unsigned int) *pCurChar++);
        }

        dprintf(pSelfUdvm, "\n");
    }

}


static RvStatus BreakInstr(SigCompUdvm       *pSelfUdvm,
                           UdvmOpcode        opcode,
                           OperandsDecoder   *pDecoder,
                           SigCompUdvmStatus *runningStatus)
{
    *runningStatus = UDVM_BREAK;
    return RV_OK;
}


static RvStatus BreakpointInstr(SigCompUdvm         *pSelfUdvm,
                                UdvmOpcode          opcode,
                                OperandsDecoder     *pDecoder,
                                SigCompUdvmStatus   *runningStatus)
{
    RvStatus                rv;
    SigCompUdvmBreakpoint   *curb           = pSelfUdvm->breakPoints;
    int                     nBreakPoints    = pSelfUdvm->nBreakPoints;
    int                     maxBreakpoints  = ARRAY_SIZE(pSelfUdvm->breakPoints);
    RvUint16                *operands       = pDecoder->operands;

    while((rv = OperandsDecoderDecodeNext(pDecoder, pSelfUdvm)) == RV_OK)
    {
        if(nBreakPoints == maxBreakpoints)
            continue;

        curb->bEnabled = 1;
        curb->pc = operands[0];
        curb++;
        nBreakPoints++;
    }

    if(rv == DECODE_FINISHED)
    {
        pSelfUdvm->nBreakPoints = nBreakPoints;
        return RV_OK;
    }

    return rv;
}


static RvStatus TraceInstr(SigCompUdvm          *pSelfUdvm,
                           UdvmOpcode           opcode,
                           OperandsDecoder      *pDecoder,
                           SigCompUdvmStatus    *runningStatus)
{
    pSelfUdvm->bTraceOn = pDecoder->operands[0];
    RV_UNUSED_ARG(opcode);
    RV_UNUSED_ARG(runningStatus);
    return RV_OK;
}


static RvStatus WatchpointInstr(SigCompUdvm         *pSelfUdvm,
                                UdvmOpcode          opcode,
                                OperandsDecoder     *pDecoder,
                                SigCompUdvmStatus   *runningStatus)
{
    RvStatus                rv;
    SigCompUdvmWatchpoint   *curw           = pSelfUdvm->watchPoints;
    int                     nWatchPoints    = pSelfUdvm->nWatchPoints;
    int                     maxWatchpoints  = ARRAY_SIZE(pSelfUdvm->watchPoints);
    RvUint16                *operands       = pDecoder->operands;

    while((rv = OperandsDecoderDecodeNext(pDecoder, pSelfUdvm)) == RV_OK)
    {
        RvUint16 index  = operands[0];
        UdvmAddr pc     = operands[1];
        UdvmAddr start  = operands[2];
        RvUint16 size   = operands[3];

        if(nWatchPoints == maxWatchpoints)
            continue;
        curw->bEnabled  = 1;
        curw->index     = index;
        curw->pc        = pc;
        curw->startAddr = start;
        curw->size      = size;
        curw++;
        nWatchPoints++;
    }

    if(rv == DECODE_FINISHED)
    {
        pSelfUdvm->nWatchPoints = nWatchPoints;
        return RV_OK;
    }

    return rv;
}

#endif


/* ==================== Some testing stuff ==========================*/

#ifdef RV_DEBUG
#define TEST_CYCLIC_BUFFER 1
#else
#define TEST_CYCLIC_BUFFER 0
#endif

#if TEST_CYCLIC_BUFFER

#include <stdio.h>

void CBT_InitUdvm(SigCompUdvm *pSelfUdvm, RvUint8 *mem, RvUint32 memSize, UdvmAddr bcl, UdvmAddr bcr) {
    RvUint8 *pMem = mem;

    pSelfUdvm->pUdvmMemory = pMem;
    pSelfUdvm->udvmMemorySize = memSize;

    WRITE16(rBCL, bcl);
    WRITE16(rBCR, bcr);
}

typedef struct {
    RvUint8 *memStart;
    RvInt idx;
} CBTCtx;

void CBT_Traverser(void *cookie, RvUint8 *mem, RvUint16 size,RvStatus *pRv) {
    CBTCtx *ctx = (CBTCtx *)cookie;

    printf("  Iter #%d starts at %d, size %d\n", ctx->idx, (RvInt32)(mem - ctx->memStart), size);
    ctx->idx++;
    *pRv = RV_OK;
}


RVAPI
void CBT_Test(RvUint32 memsize, RvUint16 bcl, RvUint16 bcr, RvUint16 start, RvUint16 size) {
    CyclicBuffer cb;
    CBTCtx ctx;
    RvUint8 mem[256]; /* the size shouldn't matter at all - test doesn't write anything */
    SigCompUdvm udvm;

    ctx.idx = 0;
    ctx.memStart = mem;

    CBT_InitUdvm(&udvm, mem, memsize, bcl, bcr);
    CyclicBufferConstruct(&cb, &udvm, start, size);

    printf("Starting traverse: memsize = %d, bcl = %d, bcr = %d, start = %d, size = %d\n", 
        memsize, bcl, bcr, start, size);
    CyclicBufferTraverse(&cb, CBT_Traverser, &ctx,UDVM_CYCLIC_BUF_TEST_TRAVERSER);
}



#endif /* #if TEST_CYCLIC_BUFFER */ 

