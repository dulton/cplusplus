/************************************************************************
 File Name     : rvtonegenerators.c
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

#include "rtptToneGenerators.h"
#include <stdlib.h>



/***********************************************************************
 * RvToneGenerator
 ***********************************************************************/
void rvToneGeneratorGenerate(RvToneGenerator *thisPtr, RvInt16 *buffer, RvInt length)
{
    thisPtr->Generate(thisPtr->generatorPtr,buffer,length);
}

void rvToneGeneratorSumGenerate(RvToneGenerator *thisPtr, RvInt16 *buffer, RvInt length)
{
    thisPtr->SumGenerate(thisPtr->generatorPtr,buffer,length);
}

/***********************************************************************
 * RvDCWave
 ***********************************************************************/
void rvDCWaveInitialize(RvDCWave *thisPtr, RvUint32 sampleRate, RvUint32 frequency, RvInt16 magnitude)
{
    thisPtr->value = magnitude; 
	RV_UNUSED_ARG(sampleRate);
	RV_UNUSED_ARG(frequency);
}

void rvDCWaveGenerate(RvDCWave *thisPtr, RvInt16 *buffer, RvInt length)
{
    RvInt16 *end = buffer + length;
    
    while(buffer != end)
        *buffer++ = thisPtr->value;
}

void rvDCWaveSumGenerate(RvDCWave *thisPtr, RvInt16 *buffer, RvInt length)
{
    RvInt16 *end = buffer + length;
    
    while(buffer != end)
    {
        *buffer = (RvInt16)(*buffer + thisPtr->value);
        buffer++;
    }
}

void rvDCWaveGetToneGenerator(RvDCWave *thisPtr, RvToneGenerator *generatorPtr)
{
    generatorPtr->generatorPtr = thisPtr;
    generatorPtr->Generate     = (void (*)(void *, RvInt16 *, RvInt))rvDCWaveGenerate;
    generatorPtr->SumGenerate  = (void (*)(void *, RvInt16 *, RvInt))rvDCWaveSumGenerate;
}

/***********************************************************************
 * RvSawtoothWave
 ***********************************************************************/
void rvSawtoothWaveInitialize(RvSawtoothWave *thisPtr, RvUint32 sampleRate, RvUint32 frequency, RvInt16 magnitude)
{
    thisPtr->magnitude  = magnitude;
    thisPtr->step       = (RvInt16)(((RvUint32)magnitude*frequency)/sampleRate);
    thisPtr->value      = 0;
}


void rvSawtoothWaveGenerate(RvSawtoothWave *thisPtr, RvInt16 *buffer, RvInt length)
{
    RvInt16 *end = buffer + length;
    
    while(buffer != end)
    {
        *buffer++ = thisPtr->value;
        
        thisPtr->value = (RvInt16)(thisPtr->value + thisPtr->step);
        if(thisPtr->value > thisPtr->magnitude)
            thisPtr->value = (RvInt16)(thisPtr->value - (thisPtr->magnitude*2));
    }
}

void rvSawtoothWaveSumGenerate(RvSawtoothWave *thisPtr, RvInt16 *buffer, RvInt length)
{
    RvInt16 *end = buffer + length;
    
    while(buffer != end)
    {
        *buffer = (RvInt16)(*buffer + thisPtr->value);
        buffer++;
        thisPtr->value = (RvInt16)(thisPtr->value + thisPtr->step);
        if(thisPtr->value > thisPtr->magnitude)
            thisPtr->value = (RvInt16)(thisPtr->value - (thisPtr->magnitude*2));
    }
}

void rvSawtoothWaveGetToneGenerator(RvSawtoothWave *thisPtr, RvToneGenerator *generatorPtr)
{
    generatorPtr->generatorPtr = thisPtr;
    generatorPtr->Generate     = (void (*)(void *, RvInt16 *, RvInt))rvSawtoothWaveGenerate;
    generatorPtr->SumGenerate  = (void (*)(void *, RvInt16 *, RvInt))rvSawtoothWaveSumGenerate;
}

/***********************************************************************
 * RvSquareWave
 ***********************************************************************/
void rvSquareWaveInitialize(RvSquareWave *thisPtr, RvUint32 sampleRate, RvUint32 frequency, RvInt16 magnitude)
{
    thisPtr->width     = (RvUint16)(frequency/sampleRate);
    thisPtr->count     = 0;
    thisPtr->value     = (RvInt16) -magnitude;
}

void rvSquareWaveGenerate(RvSquareWave *thisPtr, RvInt16 *buffer, RvInt length)
{
    RvInt16 *end = buffer + length;
    
    while(buffer != end)
    {
        *buffer++ = thisPtr->value;

        if(++thisPtr->count == thisPtr->width)
        {
            thisPtr->count = 0;
            thisPtr->value = (RvInt16)-thisPtr->value;
        }
    }
}

void rvSquareWaveSumGenerate(RvSquareWave *thisPtr, RvInt16 *buffer, RvInt length)
{
    RvInt16 *end = buffer + length;
    
    while(buffer != end)
    {
        *buffer = (RvInt16)(*buffer + thisPtr->value);
        buffer++;
        if(++thisPtr->count == thisPtr->width)
        {
            thisPtr->count = 0;
            thisPtr->value = (RvInt16)-thisPtr->value;
        }
    }
}

void rvSquareWaveGetToneGenerator(RvSquareWave *thisPtr, RvToneGenerator *generatorPtr)
{
    generatorPtr->generatorPtr = thisPtr;
    generatorPtr->Generate     = (void (*)(void *, RvInt16 *, RvInt))rvSquareWaveGenerate;
    generatorPtr->SumGenerate  = (void (*)(void *, RvInt16 *, RvInt))rvSquareWaveSumGenerate;
}

/***********************************************************************
 * RvNoise
 ***********************************************************************/
#if 0
void rvNoiseInitialize(RvNoise *thisPtr, RvUint32 sampleRate, RvUint32 frequency, RvInt16 magnitude)
{
    thisPtr->magnitude  = magnitude;
    srand(rand());
}

void rvNoiseGenerate(RvNoise *thisPtr, RvInt16 *buffer, RvInt length)
{
    RvInt16 *end = buffer + length;
    
    while(buffer != end)
        *buffer++ = rand()%thisPtr->magnitude;
}

void rvNoiseSumGenerate(RvNoise *thisPtr, RvInt16 *buffer, RvInt length)
{
    RvInt16 *end = buffer + length;
    
    while(buffer != end)
        *buffer++ += rand()%thisPtr->magnitude;
}

void rvNoiseGetToneGenerator(RvNoise *thisPtr, RvToneGenerator *generatorPtr)
{
    generatorPtr->generatorPtr = thisPtr;
    generatorPtr->Generate     = (void (*)(void *, RvInt16 *, RvInt))rvNoiseGenerate;
    generatorPtr->SumGenerate  = (void (*)(void *, RvInt16 *, RvInt))rvNoiseSumGenerate;
}
#endif

