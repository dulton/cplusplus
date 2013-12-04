/************************************************************************
 File Name	   : RtpProfileRfc3550.c
 Description   : scope: Private
                 implementation of RtpProfilePlugin for RTP (RFC 3550)
*************************************************************************/
/***********************************************************************
        Copyright (c) 2005 RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Ltd.. No part of this document may be reproduced in any
form whatsoever without written prior approval by RADVISION Ltd..

RADVISION Ltd. reserve the right to revise this publication and make
changes without obligation to notify any person of such revisions or
changes.
***********************************************************************/

#include "bitfield.h"
#include "rtputil.h"
#include "RtcpTypes.h"
#include "RtcpProfileRfc3550.h"
#include "rvrtplogger.h"
#include "rtcp.h"

#if(RV_LOGMASK != RV_LOGLEVEL_NONE)   
static  RvLogSource*     localLogSource = NULL;
#define RTP_SOURCE      (localLogSource!=NULL?localLogSource:(localLogSource=rtpGetSource(RVRTP_RTCP_MODULE)))
#define rvLogPtr        (localLogSource!=NULL?localLogSource:(localLogSource=rtpGetSource(RVRTP_RTCP_MODULE)))
static  RvRtpLogger      rtpLogManager = NULL;
#define logMgr          (RvRtpGetLogManager(&rtpLogManager),((RvLogMgr*)rtpLogManager))
#else
#define logMgr          (NULL)
#define rvLogPtr        (NULL)
#endif
#include "rtpLogFuncs.h"
#undef FUNC_NAME
#define FUNC_NAME(name) "RtcpProfileRfc3550" #name


#ifdef __cplusplus
extern "C" {
#endif

#define RV_READ2BYTES(a,b)   ((b)[0]=(a)[0],(b)[1]=(a)[1])
#define RV_WRITE2BYTES(a,b)  ((a)[0]=(b)[0],(a)[1]=(b)[1])
#define RV_READ4BYTES(a,b)   ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
#define RV_WRITE4BYTES(a,b)  ((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
    
#define  rvDataBufferReadAtUint16(buf, i, s)            (RV_READ2BYTES(buf + (i),(RvUint8*)(s)),*((RvUint16*)s) = (RvInt16 )RvConvertNetworkToHost16(*((RvUint16*)s)))
#define  rvDataBufferWriteAtUint16(buf, i, s)           {RvUint16 t = (RvInt16 )RvConvertNetworkToHost16((RvUint16)(s)); RV_WRITE2BYTES(buf + (i), (RvUint8*)&t);}
#define  rvDataBufferReadAtInt16(buf, i, s)             (RV_READ2BYTES(buf + (i),(RvUint8*)(s)),*((RvUint16*)s) = (RvInt16 )RvConvertNetworkToHost16(*((RvUint16*)s)))
#define  rvDataBufferWriteAtInt16(buf, i, s)            {RvUint16 t = (RvInt16 )RvConvertNetworkToHost16((RvUint16)(s)); RV_WRITE2BYTES(buf + (i), (RvUint8*)&t);}
#define  rvDataBufferReadAtUint32(buf, i, l)            (RV_READ4BYTES(buf + (i),(RvUint8*)(l)),*((RvUint32*)l) = RvConvertNetworkToHost32(*((RvUint32*)l)))
#define  rvDataBufferWriteAtUint32(buf, i, l)           {RvUint32 t = RvConvertNetworkToHost32((RvUint32)(l)); RV_WRITE4BYTES(buf + (i), (RvUint8*)&t);}
#define  rvDataBufferReadAtInt32(buf, i, l)             (RV_READ4BYTES(buf + (i),(RvUint8*)(l)),*((RvUint32*)l) = RvConvertNetworkToHost32(*((RvUint32*)l)))
#define  rvDataBufferWriteAtInt32(buf, i, l)            {RvUint32 t = RvConvertNetworkToHost32((RvUint32)(l)); RV_WRITE4BYTES(buf + (i), (RvUint8*)&t);}
    
#define RV_FREAD2BYTES(a,b)   ((b)[0] = (*(a)++),(b)[1] = (*(a)++))
#define RV_FWRITE2BYTES(a,b)  ((*--(a))=(b)[1],(*--(a))=(b)[0])
#define RV_FREAD4BYTES(a,b)   ((b)[0]=(*(a)++),(b)[1]=(*(a)++),(b)[2]=(*(a)++),(b)[3]=(*(a)++))
#define RV_FWRITE4BYTES(a,b)  ((*--(a))=(b)[3],(*--(a))=(b)[2],(*--(a))=(b)[1],(*--(a))=(b)[0])
    
#define  rvDataBufferWriteUint32(buf, l)                {RvUint32 t = RvConvertNetworkToHost32((RvUint32)(l)); RV_FWRITE4BYTES(buf, (RvUint8*)&t);}


/************************************************************************************
 * RtpProfileRfc3550RtcpRawReceive
 * description: this function is called, when plugin have to receive 
 *              the RTCP compound packet filled buffer.
 * input: hRTCP            - Handle of the XRTCP session.
 *        bytesToReceive   - The maximal length to receive.
 * output:
 *        buf              - Pointer to buffer for receiving a message.
 *        bytesReceivedPtr - pointer to received RTCP size
 *        remoteAddressPtr - Pointer to the remote address 
 * return value:  If no error occurs, the function returns the non-neagtive value.
 *                Otherwise, it returns a negative value.
 ***********************************************************************************/
RvStatus  RtpProfileRfc3550RtcpRawReceive(
        struct __RtpProfilePlugin* plugin,
        IN    RvRtcpSession hRTCP,
        INOUT RvRtpBuffer* buf,
        IN    RvSize_t bytesToReceive,
        OUT   RvSize_t*   bytesReceivedPtr,
        OUT   RvAddress*  remoteAddressPtr)
{
    RvStatus res = RV_ERROR_UNKNOWN;
    RvSize_t bytesReceived=0;
    rtcpSession *s = (rtcpSession *)hRTCP;
    
    RV_UNUSED_ARG(plugin);
    RTPLOG_ENTER(RtcpRawReceive);
    res = RvSocketReceiveBuffer(&s->socket, buf->buffer, bytesToReceive, logMgr, &bytesReceived, remoteAddressPtr);
    if (res == RV_OK)
    {
        /* If the encryption interface is installed, decrypt the data */
        if((s->encryptionPlugInPtr != NULL)&&
            (!RvRtpEncryptionModeIsPlainTextRtcp(s->encryptionMode)))
        {
            RvRtpEncryptionData              encryptionData;
            RvKey                             decryptionKey;

            RvKeyConstruct(&decryptionKey);
            RvRtpEncryptionDataConstruct(&encryptionData);

            RvRtpEncryptionDataSetIsRtp(&encryptionData, RV_FALSE);
            RvRtpEncryptionDataSetIsEncrypting(&encryptionData, RV_FALSE);
            RvRtpEncryptionDataSetMode(&encryptionData, s->encryptionMode);
            RvRtpEncryptionDataSetRtpHeader(&encryptionData, NULL);
            RvRtpEncryptionDataSetLocalAddress(&encryptionData, NULL);
            RvRtpEncryptionDataSetRemoteAddress(&encryptionData, (RvNetAddress*)remoteAddressPtr);

            if(s->encryptionKeyPlugInPtr != NULL)
            {
                RvRtpEncryptionKeyPlugInGetKey(s->encryptionKeyPlugInPtr, &encryptionData, &decryptionKey);
            }

            RvRtpEncryptionPlugInDecrypt(s->encryptionPlugInPtr,
                buf->buffer,
                buf->buffer,
                bytesReceived,
                &encryptionData, &decryptionKey);

            /* Remove the 32-bit random number attached to header */
            /*          rvDataBufferSkip(bufferPtr,4);*/
            *buf = buffCreate(buf->buffer+4, MAXRTCPPACKET-4);
            bytesReceived -=4;
            *bytesReceivedPtr = bytesReceived;
            RvRtpEncryptionDataDestruct(&encryptionData);
            RvKeyDestruct(&decryptionKey);
        }
        else
        {
            *bytesReceivedPtr = bytesReceived;
        }
    }
    RTPLOG_LEAVE(RtcpRawReceive);
    return res;
}

/************************************************************************************
 * RtpProfileRfc3550RtcpRawSend
 * description: this function is called, when plugin have to send
 *              the RTCP compound packet filled buffer.
 * input: plugin           - pointer to this plugin 
 *        hRTCP            - Handle of the RTCP session.
 *        bufPtr           - Pointer to buffer containing the RCTP packet.
 *        DataLen          - Length in bytes of RTCP data buf.
 *        DataCapacity     - data capacity of RTCP buffer
 *        sentLenPtr       - for bandwith calculation only
 * output: none.
 * return value:  If no error occurs, the function returns the non-neagtive value.
 *                Otherwise, it returns a negative value.
 ***********************************************************************************/
RvStatus  RtpProfileRfc3550RtcpRawSend(
        struct __RtpProfilePlugin* plugin,                                   
        IN    RvRtcpSession hRTCP,
        IN    RvUint8*      bufPtr,
        IN    RvInt32       DataLen,
        IN    RvInt32       DataCapacity,
        INOUT RvUint32*     sentLenPtr)
{
    RvStatus status = RV_OK;
    rtcpSession          *thisPtr = (rtcpSession *)hRTCP;
    RvRtpEncryptionData   encryptionData;
    RvKey        encryptionKey, decryptionKey;
    RvSize_t      BytesSent = 0;
    RvUint8      *bufferPtr = (RvUint8 *)bufPtr;
    RvUint8       workBuffer[RTCP_WORKER_BUFFER_SIZE];
    RvAddress    *destAddress = NULL;

    RV_UNUSED_ARG(plugin);
    
    RTPLOG_ENTER(RtcpRawSend);
   /* Check to see if sending has been disabled, if yes, abort. */
    if(thisPtr == NULL || (!thisPtr->isAllocated) ||thisPtr->rtpSession == NULL)
    {
        RTPLOG_ERROR_LEAVE(RtcpRawSend, "NULL RTP/RTCP session handles or RTCP session is not allocated");
        return RV_ERROR_UNKNOWN;
    }
    RvKeyConstruct(&encryptionKey);
    RvKeyConstruct(&decryptionKey);
    RvRtpEncryptionDataConstruct(&encryptionData);

    /* If encryption interface is installed, Encrypt the data and add required padding */
    if((thisPtr->encryptionPlugInPtr != NULL) &&
       (!RvRtpEncryptionModeIsPlainTextRtcp(thisPtr->encryptionMode)))
    {
        RvInt32       blockSize   = 0;
        RvInt32       paddingSize = 0;

        /* Setup the Encryption data */
        RvRtpEncryptionDataSetIsRtp(&encryptionData, RV_FALSE);
        RvRtpEncryptionDataSetIsEncrypting(&encryptionData, RV_TRUE);
        RvRtpEncryptionDataSetMode(&encryptionData, thisPtr->encryptionMode);
        RvRtpEncryptionDataSetRtpHeader(&encryptionData, NULL);
        RvRtpEncryptionDataSetLocalAddress(&encryptionData, (RvNetAddress*)&rvRtcpInstance.localAddress);

        /* Determin the required padding, if encryption is installed. */
        blockSize = RvRtpEncryptionPlugInGetBlockSize(thisPtr->encryptionPlugInPtr);
        paddingSize = blockSize - (DataLen + 4)%blockSize;

        if(paddingSize == blockSize)
            paddingSize = 0;

        /* Add required padding to fill out block size */
        if(paddingSize > 0)
        {
            if(DataCapacity >= paddingSize)
            {
                RvUint32  totalLength = DataLen;
                RvUint32  index           = 0;
                RvUint32  lastHeaderIndex = 0;
                RvUint32  header          = 0;
                RvUint32  length          = 0;

                /* Find the last header in the packet */
                while(index < totalLength)
                {
                    lastHeaderIndex = index;
                    rvDataBufferReadAtUint32(bufferPtr, lastHeaderIndex , &header);
                    index  += ((header & 0x0000FFFF) + 1)*4;
                }

                /* Set the padding bit in the last header and adjust
                   the last header's length field */
                header |= 0x20000000;
//                length = (header & 0x0000FFFF) + paddingSize/4;
                length = (totalLength - lastHeaderIndex + paddingSize)/4 - 1;
                header = (header & 0xFFFF0000) + (length & 0x0000FFFF);
                {
                   RvSize_t bitOffset =0;
                   rvBitfieldWrite(bufferPtr + lastHeaderIndex, &bitOffset, 32, header | 0x20000000);
                }
                /* Add the padding to the buffer */
/*                rvDataBufferRewindBack(bufferPtr, paddingSize - 1);*/
                DataLen += (paddingSize - 1);

                /* Write the amount of padding as the last byte */
/*                rvDataBufferWriteBackUint8(bufferPtr, paddingSize);*/
                bufferPtr[DataLen] = (RvUint8)paddingSize;
                DataLen++;
            }
            else
            {
                RTPLOG_ERROR_LEAVE(RtcpRawSend, "Not enough room at end of RTCP buffer to add required padding.");
                return -1;
            }
        }

        /* Add the 32-bit random number to header ???*/
        {
            RvRandom randVal;
            RvRandomGeneratorGetValue(&rvRtcpInstance.randomGenerator, &randVal);
            rvDataBufferWriteUint32(bufferPtr, (RvUint32) randVal); /*???*/
            DataLen +=4;
        }

        /* Make sure the work buffer is large enough for the current packet */
        if(sizeof(workBuffer) < (RvUint32)DataLen)
        {
            RTPLOG_ERROR_LEAVE(RtcpRawSend, "Not enough room for buffer.");   
            return -2;
        }
        /* Set up the work buffer data position */
/*        rvDataBufferSetDataPosition(&thisPtr->workBuffer,0,rvDataBufferGetLength(bufferPtr));*/
    }

    /* Send packet to all remote addresses */
    destAddress = RvRtpAddressListGetNext(&thisPtr->addressList, NULL);
    while(destAddress != NULL)
    {
        if((thisPtr->encryptionPlugInPtr != NULL) &&
            (!RvRtpEncryptionModeIsPlainTextRtcp(thisPtr->encryptionMode)))
        {
            /* Set the remote address in the encryption data */

            RvRtpEncryptionDataSetRemoteAddress(&encryptionData, (RvNetAddress*)destAddress);

            if(thisPtr->encryptionKeyPlugInPtr != NULL)
            {
                RvRtpEncryptionKeyPlugInGetKey(thisPtr->encryptionKeyPlugInPtr, &encryptionData, &encryptionKey);
            }

            /* Encrypt the buffer */
            RvRtpEncryptionPlugInEncrypt(thisPtr->encryptionPlugInPtr,
                bufferPtr,
                workBuffer,
                DataLen, /* -4? */
                &encryptionData,
                &encryptionKey);


            RvSocketSendBuffer(&thisPtr->socket,
                workBuffer,
                DataLen,
                destAddress,
                logMgr, &BytesSent);

        }
        else
        {

            RvSocketSendBuffer(&thisPtr->socket,
                bufferPtr,
                DataLen,
                destAddress,
                logMgr, &BytesSent);
        }
        if (sentLenPtr!=NULL)
           *sentLenPtr = BytesSent;
        destAddress = RvRtpAddressListGetNext(&thisPtr->addressList, destAddress);
    }
    RvRtpEncryptionDataDestruct(&encryptionData);
    RvKeyDestruct(&encryptionKey);
    RvKeyDestruct(&decryptionKey);
    RTPLOG_LEAVE(RtcpRawSend);
    return status;
}



#ifdef __cplusplus
}
#endif

