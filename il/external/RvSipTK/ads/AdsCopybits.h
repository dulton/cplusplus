/***********************************************************************************************************************

NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

************************************************************************************************************************/


/***********************************************************************************************************************
 *                                          CopyBits.h                                                                 *
 *                                                                                                                     *
 * The package handles simple bit manipulation routines.                                                               *
 *                                                                                                                     *
 *                Written by                    Version & Date                            Change                       *
 *               ------------                   ---------------                          --------                      *
 *               Sasha R                  1          28/1/96                             Fisrt Version                 *
 *                                                                                                                     *
 ***********************************************************************************************************************/


#ifndef COPYBITS_H
#define COPYBITS_H


#ifdef __cplusplus
extern "C" {
#endif


#include "RV_ADS_DEF.h"
#include "rvlog.h"


/*************************************************************************************************************************
 * BITS_GetBit
 * purpose : Get the value of specific bit inside a source buffer.
 * input   : Ptr : Pointer to the source buffer.
 *           bit : The number of the required bit.
 * output  : None.
 * return  : The bit value ( 1 or 0 ).
 *************************************************************************************************************************/
#ifndef RV_USE_MACROS
int RVAPI RVCALLCONV BITS_GetBit (void  *Ptr,
                                  RvInt32 Bit);
#else
RVAPI RVINLINE int RVCALLCONV BITS_GetBit(void  *Ptr, RvInt32 Bit)
{
    /* Bit mask vector (inside one byte) */
    const RvUint8 Mask[8]  = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
    return ((unsigned char*)(Ptr))[(Bit)/8] & Mask[(Bit)%8];
}
#endif /* #ifndef RV_USE_MACROS */


/*************************************************************************************************************************
 * BITS_SetBit
 * purpose : Set the value of specific bit inside a source buffer.
 * input   : Ptr   : Pointer to the source buffer.
 *           bit   : The number of the required bit.
 *           value : The new bit value. ( 1 or 0 )
 * output  : None.
 *************************************************************************************************************************/
#ifndef RV_USE_MACROS
void RVAPI RVCALLCONV BITS_SetBit( void  *Ptr,
                                   RvInt32 Bit,
                                   int   Value);
#else
RVAPI RVINLINE void RVCALLCONV BITS_SetBit(
                                    void  *Ptr,
                                    RvInt32 Bit,
                                    int   Value)
{
    /* Bit mask vector (inside one byte) */
    const RvUint8 Mask[8]  = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
    /* Zero bit mask vector (inside one byte) */
    const RvUint8 ZMask[8] = {(RvUint8)~0x80, (RvUint8)~0x40, (RvUint8)~0x20,
                              (RvUint8)~0x10, (RvUint8)~0x8,  (RvUint8)~0x4,
                              (RvUint8)~0x2, (RvUint8)~0x1};

    (Value == 0) ? \
    (((unsigned char*)Ptr)[Bit/8] &= ZMask[Bit%8]) : \
    (((unsigned char*)Ptr)[Bit/8] |= Mask[Bit%8]);
}
#endif /* #ifndef RV_USE_MACROS */



/**************************************************************************************************************************
 * BITS_CopyBits
 * purpose : Copy a given number of bits from a given location inside a source buffer to a  given location inside a
 *           destination buffer.
 * input   : Dest       : Pointer to the destination buffer.
 *           DestBitPos : Position ( in bits ) to which the copy should be done.
 *           Src        : Pointer to the source buffer.
 *           SrcBitPos  : Position ( in bits ) from which copy should be done.
 *           NumOfBites : The number of bits that should be copied.
 * output  : None.
 *************************************************************************************************************************/
void RVAPI RVCALLCONV BITS_CopyBits( OUT void *Dest,
                                     IN  RvInt32 DestBitPos,
                                     IN  void *Src,
                                     IN  RvInt32 SrcBitPos,
                                     IN  RvInt32 NumOfBits);


/*************************************************************************************************************************
 * BITS_SetTrueBit
 * purpose : Set the value of sepecific bit inside a source buffer only if the bit is TRUE
 * input   : Ptr   : Pointer to the source buffer.
 *           bit   : The number of the required bit.
 *           value : The new bit value. ( 1 or 0 -> 0 is ignored)
 * output  : None.
 *************************************************************************************************************************/
void RVAPI RVCALLCONV BITS_SetTrueBit( void  *Ptr,
                                       RvInt32 Bit,
                                       int   Value);


/**************************************************************************************************************************
 * BITS_CopyTrueBits
 * purpose : Copy a given number of bits from a given location inside a source buffer to a  given location inside a
 *           destination buffer. This function ignores bits which are set to FALSE
 * input   : Dest       : Pointer to the destination buffer.
 *           DestBitPos : Position ( in bits ) to which the copy should be done.
 *           Src        : Pointer to the source buffer.
 *           SrcBitPos  : Position ( in bits ) from which copy should be done.
 *           NumOfBites : The number of bits that should be copied.
 * output  : None.
 *************************************************************************************************************************/
void RVAPI RVCALLCONV BITS_CopyTrueBits( OUT void *Dest,
                                         IN  RvInt32 DestBitPos,
                                         IN  void *Src,
                                         IN  RvInt32 SrcBitPos,
                                         IN  RvInt32 NumOfBits);

#ifdef __cplusplus
}
#endif


#endif


