/************************************************************************
 File Name     : rvtonegenerators.h
 Description   :
*************************************************************************
 Copyright (c)  2001 , RADVision, Inc. All rights reserved.
*************************************************************************
 NOTICE:
 This document contains information that is proprietary to RADVision Inc. 
 No part of this publication may be reproduced in any form whatsoever
 without written prior approval by RADVision Inc. 
 
 RADVision Inc. reserves the right to revise this publication and make
 changes without obligation to notify any person of such revisions or
 changes.
*************************************************************************
 $Revision: #1 $
 $Date: 2011/08/05 $
 $Author: songkamongkol $
************************************************************************/

#if !defined(RVTONEGENERATORS_H)
#define RVTONEGENERATORS_H

#include "rvtypes.h"

#ifdef __cplusplus
extern "C" {
#endif


/***********************************************************************
 * RvToneGenerator
 ***********************************************************************/
typedef struct
{
    void *generatorPtr;
    void (* Generate)(void *, RvInt16 *, RvInt); 
    void (* SumGenerate)(void *, RvInt16 *, RvInt ); 
} RvToneGenerator;

void rvToneGeneratorGenerate(RvToneGenerator *thisPtr, RvInt16 *buffer, RvInt length);
void rvToneGeneratorSumGenerate(RvToneGenerator *thisPtr, RvInt16 *buffer, RvInt length);


/***********************************************************************
 * RvDCWave
 ***********************************************************************/
typedef struct
{
    RvInt16  value; 
} RvDCWave;

void rvDCWaveInitialize(RvDCWave *thisPtr, RvUint32 sampleRate, RvUint32 frequency, RvInt16 magnitude);
void rvDCWaveGenerate(RvDCWave *thisPtr, RvInt16 *buffer, RvInt length);
void rvDCWaveSumGenerate(RvDCWave *thisPtr, RvInt16 *buffer, RvInt length);
void rvDCWaveGetToneGenerator(RvDCWave *thisPtr, RvToneGenerator *generatorPtr);


/***********************************************************************
 * RvSawtoothWave
 ***********************************************************************/
typedef struct
{
    RvUint16 step; 
    RvInt16  value; 
    RvInt16  magnitude; 
} RvSawtoothWave;

void rvSawtoothWaveInitialize(RvSawtoothWave *thisPtr, RvUint32 sampleRate, RvUint32 frequency, RvInt16 magnitude);
void rvSawtoothWaveGenerate(RvSawtoothWave *thisPtr, RvInt16 *buffer, RvInt length);
void rvSawtoothWaveSumGenerate(RvSawtoothWave *thisPtr, RvInt16 *buffer, RvInt length);
void rvSawtoothWaveGetToneGenerator(RvSawtoothWave *thisPtr, RvToneGenerator *generatorPtr);


/***********************************************************************
 * RvSquareWave
 ***********************************************************************/
typedef struct
{
    RvUint16 count; 
    RvUint16 width; 
    RvInt16  value;
} RvSquareWave;

void rvSquareWaveInitialize(RvSquareWave *thisPtr, RvUint32 sampleRate, RvUint32 frequency, RvInt16 magnitude);
void rvSquareWaveGenerate(RvSquareWave *thisPtr, RvInt16 *buffer, RvInt length);
void rvSquareWaveSumGenerate(RvSquareWave *thisPtr, RvInt16 *buffer, RvInt length);
void rvSquareWaveGetToneGenerator(RvSquareWave *thisPtr, RvToneGenerator *generatorPtr);

/***********************************************************************
 * RvNoise
 ***********************************************************************/
typedef struct
{
    RvUint16 magnitude; 
} RvNoise;

void rvNoiseInitialize(RvNoise *thisPtr, RvUint32 sampleRate, RvUint32 frequency, RvInt16 magnitude);
void rvNoiseGenerate(RvNoise *thisPtr, RvInt16 *buffer, RvInt length);
void rvNoiseSumGenerate(RvNoise *thisPtr, RvInt16 *buffer, RvInt length);
void rvNoiseGetToneGenerator(RvNoise *thisPtr, RvToneGenerator *generatorPtr);




#ifdef __cplusplus
}
#endif

#endif  /* Include guard */

