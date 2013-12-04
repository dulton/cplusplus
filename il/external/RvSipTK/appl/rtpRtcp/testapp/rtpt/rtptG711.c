/************************************************************************
 File Name     : rvg711.c
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

#include "rtptG711.h"
#include "rvtypes.h"

/*********************************************************************
*  G711 u-Law
*********************************************************************/

static const RvInt table[256] = {0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,
                               4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
                               5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
                               5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
                               6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                               6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                               6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                               6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
                               7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                               7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                               7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                               7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                               7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                               7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                               7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
                               7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7};


void rvG711PCMToULaw(const void *src, void *dst, RvInt byteLength)
{

    RvUint8        *output = (RvUint8 *)dst; 
    const RvInt16  *input  = (const RvInt16 *)src;
    const RvUint8  *end    = output + (byteLength/2);
    RvInt16         value  = 0;
    RvInt16         sign;
    RvInt16         exponent;
    RvInt16         mantissa;

    while(output != end)
    {
        value = *input; 
        
        /* Get the sample into sign-magnitude. */
        sign = (RvInt16)((value >> 8) & 0x80);		

        /* Get the magnitude */
        if(sign != 0) 
            value = (RvInt16) -value;		

        /* Clip the magnitude */
        if(value > 32635) 
          value = 32635;		

        /* Convert from 16 bit linear to ulaw. */
        value = (RvInt16) (value + 132);
        exponent = (RvInt16) table[(value >> 7) & 0xFF];
        mantissa = (RvInt16) ((value >> (exponent + 3)) & 0x0F);
        *output = (RvUint8)~(sign | (exponent << 4) | mantissa);

        /* optional CCITT trap */
        if(value == 0) 
            value = 0x02;	

        output++;
        input++;
    }
}

static const RvInt16 ULawToPCM[256] = {
   -8031,   -7775,   -7519,   -7263,   -7007,   -6751,   -6495,   -6239,
   -5983,   -5727,   -5471,   -5215,   -4959,   -4703,   -4447,   -4191,
   -3999,   -3871,   -3743,   -3615,   -3487,   -3359,   -3231,   -3103,
   -2975,   -2847,   -2719,   -2591,   -2463,   -2335,   -2207,   -2079,
   -1983,   -1919,   -1855,   -1791,   -1727,   -1663,   -1599,   -1535,
   -1471,   -1407,   -1343,   -1279,   -1215,   -1151,   -1087,   -1023,
    -975,    -943,    -911,    -879,    -847,    -815,    -783,    -751,
    -719,    -687,    -655,    -623,    -591,    -559,    -527,    -495,
    -471,    -455,    -439,    -423,    -407,    -391,    -375,    -359,
    -343,    -327,    -311,    -295,    -279,    -263,    -247,    -231,
    -219,    -211,    -203,    -195,    -187,    -179,    -171,    -163,
    -155,    -147,    -139,    -131,    -123,    -115,    -107,     -99,
     -93,     -89,     -85,     -81,     -77,     -73,     -69,     -65,
     -61,     -57,     -53,     -49,     -45,     -41,     -37,     -33,
     -30,     -28,     -26,     -24,     -22,     -20,     -18,     -16,
     -14,     -12,     -10,      -8,      -6,      -4,      -2,       0,
    8031,    7775,    7519,    7263,    7007,    6751,    6495,    6239,
    5983,    5727,    5471,    5215,    4959,    4703,    4447,    4191,
    3999,    3871,    3743,    3615,    3487,    3359,    3231,    3103,
    2975,    2847,    2719,    2591,    2463,    2335,    2207,    2079,
    1983,    1919,    1855,    1791,    1727,    1663,    1599,    1535,
    1471,    1407,    1343,    1279,    1215,    1151,    1087,    1023,
     975,     943,     911,     879,     847,     815,     783,     751,
     719,     687,     655,     623,     591,     559,     527,     495,
     471,     455,     439,     423,     407,     391,     375,     359,
     343,     327,     311,     295,     279,     263,     247,     231,
     219,     211,     203,     195,     187,     179,     171,     163,
     155,     147,     139,     131,     123,     115,     107,      99,
      93,      89,      85,      81,      77,      73,      69,      65,
      61,      57,      53,      49,      45,      41,      37,      33,
      30,      28,      26,      24,      22,      20,      18,      16,
      14,      12,      10,       8,       6,       4,       2,      0   
}; 

void rvG711ULawToPCM(const void *src, void *dst, RvInt byteLength)
{
    const RvUint8  *input  = (const RvUint8 *)src; 
    RvInt16        *output = (RvInt16 *)dst;
    const RvUint8  *end    = input + byteLength;
    
    while(input != end)
        *output++ = ULawToPCM[*input++];
}
