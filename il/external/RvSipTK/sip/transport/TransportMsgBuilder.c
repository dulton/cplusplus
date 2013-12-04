#ifdef __cplusplus
extern "C" {
#endif

/*
*********************************************************************************
*                                                                               *
* NOTICE:                                                                       *
* This document contains information that is confidential and proprietary to    *
* RADVision LTD.. No part of this publication may be reproduced in any form     *
* whatsoever without written prior approval by RADVision LTD..                  *
*                                                                               *
* RADVision LTD. reserves the right to revise this publication and make changes *
* without obligation to notify any person of such revisions or changes.         *
* Copyright RADVision 1996.                                                     *
* Last Revision: Jan. 2000                                                      *
*********************************************************************************
*/


/*********************************************************************************
 *                              TransportMsgBuilder.c
 *
 * This c file contains implementations for the message builder module.
 * The message builder module is responsible for parsing the the raw buffer and
 * find the point that the headers end and the body begins. after that the builder
 * builds the message object and pass it to the transaction module.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *  Oren Libis                        20 Nov 2000
 *  Yaniv Saltoon                    31 Jul 2001  (TCP)
 *  Ofra Wachsman                   Jan 2003 (bad-syntax)
 *********************************************************************************/



/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"

#include <string.h>

#include "_SipTransportTypes.h"
#include "_SipMsg.h"
#include "_SipTransport.h"
#include "TransportCallbacks.h"

#include "TransportMsgBuilder.h"
#include "TransportTCP.h"
#include "TransportTLS.h"
#include "RvSipMsgTypes.h"
#include "RvSipBody.h"
#include "_SipCommonConversions.h"

#if (RV_TLS_TYPE != RV_TLS_NONE)
#include "RvSipPartyHeader.h"
#include "RvSipAddress.h"
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

#ifdef RV_SIGCOMP_ON
#include "_SipTransactionMgr.h"
#include "RvSipCompartment.h"
#ifdef PRINT_SIGCOMP_BYTE_STREAM
#include "_SipCompartment.h"
#endif /* PRINT_SIGCOMP_BYTE_STREAM */
#endif

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
void* HSTPOOL_Alloc_TRACE ( int* size, char* file, int line ); 
void  HSTPOOL_Release_TRACE ( void * buff, char* file, int line );
#define HSTPOOL_Alloc(size) HSTPOOL_Alloc_TRACE(size,__FILE__,__LINE__)
#define HSTPOOL_Release(buff) HSTPOOL_Release_TRACE(buff,__FILE__,__LINE__)
void  HSTPOOL_SetBlockSize ( void * buff, int size );
#endif /* defined(UPDATED_BY_SPIRENT_ABACUS) */ 
/* SPIRENT_END */
/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

#define SIP_MSG_BUILDER_CR   0x0D
#define SIP_MSG_BUILDER_LF   0x0A
#define SIP_MSG_BUILDER_SP   0x20
#define SIP_MSG_BUILDER_HT   0x09
#define SIP_MSG_BUILDER_B0   0x00
#define SIP_TRANSPORT_LOG_MAX_LINE_LENGTH 1000

    /* prefixes for message printing to log file */
#define PRINT_INCOMING_MSG_PREF "<-- "
#define PRINT_OUTGOING_MSG_PREF "--> "
#define PRINT_INCOMING_MSG_EXTERN_PREF "<--- "
#define PRINT_OUTGOING_MSG_EXTERN_PREF "---> "
#define PRINT_HEADER_PREF        "    "
#define PRINT_HEADER_EXTERN_PREF "     "

#ifdef RV_SIGCOMP_ON
#define SIP_MSG_BUILDER_SIGCOMP_DELIMITER   0xFF
#define SIP_MSG_BUILDER_SIGCOMP_PREFIX_MASK 0xF8      /* 0xF8 = 11111000 */
#endif /* RV_SIGCOMP_ON */

typedef struct {
    RvBool                        msgComplete;
    RvInt32                       msgSize;
    RvUint32                      msgStart;
    RvChar                       *originalBuffer;
    RvUint32                      bytesReadFromBuf;
    RvSipTransportLocalAddrHandle hLocalAddr;
    RvInt32                       plainBufSize;
    RvChar                       *pPlainBuf;
    RvChar                       *pDecompressedBuf;
    RvInt32                       decompressedBufSize;
#ifdef RV_SIGCOMP_ON
    RvBool                        bIsSigCompMsg;
#endif /* RV_SIGCOMP_ON */
} MsgBuilderStoredConnInfo;

typedef struct {
    RvChar    *pBuf;
    RvUint32   bufSize;
    RvChar    *pDecompressedBuf;
    RvUint32   decompressedBufSize;
} MsgBuilderSigCompInfo;

/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV TransportMsgBuilderTcpFindHeader(
        IN RvUint32            index,
        IN RvChar              *pBuf,
        IN TransportConnection  *conn,
        IN RvUint32                bytesForwardToCheack,
        OUT RvUint32           *headerIndex);

static RvBool RVCALLCONV TransportMsgBuilderIsEndofMsg(
                                     IN  RvChar   *pBuf,
                                     IN  RvInt32 index,
                                     IN  RvSipTransportConnectionHandle   hConn,
                                     OUT RvInt32 *sipMsgSize,
                                     IN RvUint32    bytesForwardToCheack,
                                     OUT RvInt32 *sipMsgStartBody);

static RvStatus RVCALLCONV ParseMsg(
                                     IN TransportMgr          *pTransportMgr,
                                     IN  RvUint32             sipMsgStartIndex,
                                     IN  RvUint32             sipMsgEndIndex,
                                     IN  RvChar              *pBuffer,
                                     IN  RvUint32             bufSize,
                                     IN  RvSipMsgHandle        hMsg);


static void RVCALLCONV TransportMsgBuilderSetSpaces(
                                     IN    RvUint32    index,
                                     INOUT RvChar   *pBuf);



static RvChar* RVCALLCONV TransportMsgBuilderGetSDPStart(
                                      IN RvChar      *pBuf,
                                      IN RvUint32    index,
                                      OUT RvUint32 *startBodyIndex);




static RvStatus AppendBodyToMsg(IN TransportConnection    *conn,
                                 IN RvChar                *pBuf,
                                 IN RvInt32             bufSize,
                                 OUT RvBool*             msgComplete,
                                 OUT RvUint32            *bytesReadFromBuff);


static RvStatus CopyBodyToMsg(IN TransportConnection    *conn,
                                 IN RvChar                *pBuf,
                                 IN RvInt32            bufSize,
                                 IN RvUint32           *sipMsgStartIndex,
                                 IN RvUint32           sipMsgStartBody,
                                 IN RvInt32            bytesLeftInBuffer,
                                 OUT RvBool*            msgComplete,
                                 OUT RvUint32            *bytesReadFromBuff);

static RvBool RVCALLCONV CheckPrevPacketForEndOfMsg(
                                 IN RvChar                *pBuf,
                                 IN TransportConnection *conn,
                                 OUT RvInt32           *sipMsgEndIndex,
                                 OUT RvInt32           *sipMsgStartBody);

static RvBool CheckPrevPacketForEndOfHeader(
                                 IN RvChar                *pBuf,
                                 IN TransportConnection *conn);


static RvStatus RVCALLCONV TransportMsgBuilderUdpPrepare(
                              IN RvLogSource*    regId,
                              IN RvInt32             bufSize,
                              INOUT RvChar            *pBuffer,
                              OUT RvInt32            *sipMsgStartIndex,
                              OUT RvInt32            *sipMsgEndIndex);


static RvStatus RVCALLCONV TransportMsgBuilderTcpPrepare(
                                 IN    TransportConnection *conn,
                                 IN    RvInt32            bufSize,
                                 INOUT RvChar             *pBuffer,
                                 OUT   RvUint32           *sipMsgStartIndex,
                                 OUT   RvInt32            *sipMsgEndIndex,
                                 OUT   RvUint32           *bytesReadFromBuf,
                                 OUT   RvBool             *msgComplete);


static RvBool RVCALLCONV FindContentLengthInHeader(
                                 INOUT TransportConnection *conn,
                                 IN RvChar*               buff,
                                 INOUT RvUint32           *sipStartHeader,
                                 IN RvInt32               startHeader,
                                 INOUT RvInt32            endHeader);


static RvBool RVCALLCONV FindContentLengthInHeaderPage(
                                   INOUT TransportConnection *conn,
                                 IN RvInt32 headerSize);

static RvStatus RVCALLCONV GetContentLengthFromHeader(
                                 IN RvLogSource* regId,
                                 IN RvChar*            buff,
                                 IN RvInt32            length,
                                 OUT RvInt32            *contentLength);

static RvStatus RVCALLCONV GetContentLengthFromHeaderPage(
                                   IN TransportConnection *conn,
                                 IN RvInt32  Offset,
                                 IN RvInt32  totalLength,
                                 OUT RvInt32 *contentLength);



static RvBool RVCALLCONV findColonInPage(
                                 INOUT TransportConnection *conn,
                                 IN RvInt32 offset,
                                 IN RvInt32 length,
                                 OUT RvInt32 *newOffset);

static RvStatus RVCALLCONV MsgBuilderConstructParseTcpMsg(
                                IN  TransportMgr*   pTransportMgr,
                                IN  RvChar*         pBuffer,
                                IN  RvInt32         totalMsgSize,
                                IN  RvInt32         sipMsgSize,
                                IN  RvUint32        sdpLength,
                                OUT RvSipMsgHandle* phMsg);

static RvBool InformBadSyntax(  IN  TransportMgr           *pTransportMgr,
                                IN  RvSipMsgHandle          hMsg,
                                OUT RvSipTransportBsAction *eAction);
static RvBool IsFirstOccurrence(const RvChar* psztoFind, const RvChar* pszFindInMe);

static void RVCALLCONV FreeMsgBuilderTcpResources(
                          IN    TransportConnection           *pConn,
                          IN    RvChar                       **ppDecompressedBuf,
                          IN    TransportProcessingQueueEvent *pTcpEvent);

static void RVCALLCONV RestoreConnInfo(
                          IN  TransportConnection        *pConn,
                          OUT MsgBuilderStoredConnInfo  *pConnInfo);

static void RVCALLCONV ResetConnReceivedInfo(
                          IN TransportConnection  *pConn);


static void RVCALLCONV StoreConnInfo(
                          IN  MsgBuilderStoredConnInfo  *pConnInfo,
                          OUT TransportConnection        *pConn);

static void RVCALLCONV InitConnInfo(OUT TransportConnection        *pConn);

static RvStatus RVCALLCONV PrepareTcpCompleteMsgBuffer(
                   IN    TransportConnection        *pConn,
                   IN    RvChar                     *pBuf,
                   IN    RvUint32                    bufSize,
                   OUT   SipTransportSigCompMsgInfo *pSigCompMsgInfo,
                   OUT   MsgBuilderStoredConnInfo   *pConnInfo);
static RvStatus RVCALLCONV HandleTcpCompleteMsgBuffer(
                   IN    TransportConnection        *pConn,
                   IN    RvChar                     *pBuf,
                   IN    RvUint32                   bufSize,
                   IN    SipTransportSigCompMsgInfo *pSigCompMsgInfo,
                   IN    MsgBuilderStoredConnInfo   *pConnInfo);
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static void RVCALLCONV ReportTcpCompleteMsgBuffer(
                   IN    TransportConnection           *pConn,
                   IN    TransportProcessingQueueEvent *pEv);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

#ifdef RV_SIGCOMP_ON

static RvBool RVCALLCONV IsSigCompStreamStart(IN  RvChar   *pBuf,
                                                  IN  RvInt32   bufSize);

static RvStatus RVCALLCONV DecompressCompeleteMsgBufferIfNeeded(
               IN    TransportMgr               *pTransportMgr,
			   IN    RvSipTransport              eTransport,
               INOUT MsgBuilderSigCompInfo      *pBuilderSigCompInfo,
               OUT   SipTransportSigCompMsgInfo *pSigCompMsgInfo);

static RvBool RVCALLCONV IsTcpSigCompStream(
               IN TransportConnection    *pConn,
               IN MsgBuilderSigCompInfo *pBuilderSigCompInfo);

static RvStatus RVCALLCONV HandleIncompleteSigCompDelimiter(
               IN    TransportConnection  *pConn,
               INOUT RvChar             **ppCurrBufPos,
               INOUT RvInt32             *pCurrBytesRead,
               OUT   RvBool              *pbMsgComplete);

static RvStatus RVCALLCONV HandleSigCompQuotedBytesLeft(
               IN    TransportConnection *pConn,
               IN    RvInt32             bufSizeLeft,
               INOUT RvChar            **ppCurrBufPos,
               INOUT RvInt32            *pCurrBytesRead);

static RvStatus RVCALLCONV HandleSigCompBufferLeft(
               IN    TransportConnection *pConn,
               IN    RvInt32             totalBufSize,
               INOUT RvChar            **ppCurrBufPos,
               INOUT RvInt32            *pCurrBytesRead,
               OUT   RvBool             *pbMsgComplete);

static void RVCALLCONV FreeSigCompResources(IN TransportConnection *pConn);

static RvStatus RVCALLCONV HandleSigCompCompleteMsgBuffer(
               IN    TransportConnection        *pConn,
               IN    RvInt32                    sigcompMsgBytes,
               INOUT MsgBuilderSigCompInfo      *pBuilderSigCompInfo,
               OUT   SipTransportSigCompMsgInfo *pSigCompMsgInfo);

static RvStatus RVCALLCONV HandleSigCompIncompleteMsgBuffer(
               IN    TransportConnection            *pConn,
               IN    RvChar                        *pBuf,
               IN    RvInt32                        bufSize,
               OUT   SipTransportSigCompMsgInfo     *pSigCompMsgInfo);

static RvStatus RVCALLCONV BuildTcpDecompressMsgBufferIfNeeded(
                          IN    TransportConnection         *conn,
                          INOUT MsgBuilderSigCompInfo       *pBuilderSigCompInfo,
                          OUT   SipTransportSigCompMsgInfo  *pSigCompMsgInfo,
                          OUT   RvUint32                    *pBytesReadFromBuf,
                          OUT   RvBool                       *pbMsgComplete,
                          OUT   RvBool                     *pbIsSigCompMsg);

static void convertRvTransportToSigcompTransport(RvSipTransport eTransport, RvInt *pSigcompTransport);
#endif /* RV_SIGCOMP_ON */

static void RVCALLCONV GetMsgCompressionType(IN  SipTransportSigCompMsgInfo *pSigCompMsgInfo,
											 OUT RvSipCompType				*peCompression);

static RvStatus RVCALLCONV ReportConnectionMsg(
                            IN TransportMgr*                     pTransportMgr,
                            IN RvSipMsgHandle                    hMsg,
                            IN TransportConnection*              pConn,
                            IN RvSipTransportConnectionAppHandle hAppConn,
                            IN RvSipTransportLocalAddrHandle     hLocalAddr,
                            IN SipTransportAddress*              pRecvFromAddr,
                            IN SipTransportSigCompMsgInfo*       pSigCompInfo);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
#define ASCII_CR 13
#define ASCII_LF 10
/************************************************************************************
 * TransportMsgBuilderPrintMsg
 * ----------------------------------------------------------------------------------
 * General: log print function. used only for printing the buffer to the log.
 *
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport instance
 *          pBuf             - the buffer that was received from the network.
 *          msgDirection     - The direction of the sent message.
 *          bAdditional      - Indication if the request to print a message is in
 *                             addition to the general flow (for example before
 *                             compression of after decompression)
 * Output:  none
 ***********************************************************************************/
void RVCALLCONV TransportMsgBuilderPrintMsg(
                     IN TransportMgr                     *pTransportMgr,
                     IN RvChar                           *pBuf,
                     IN SipTransportMgrBuilderPrintMsgDir msgDirection,
                     IN RvBool                            bAdditional)
{

    if(RvLogIsSelected(pTransportMgr->pLogSrc, RV_LOGLEVEL_INFO) == RV_FALSE)
    {
        /* if the INFO log filter of the TRANSPORT layer is off,
           no need to proceed with this function, since it only print in INFO */
        return;
    }

    {
        RvChar   *pHead;
        RvChar   *pTemp;
        RvChar    tempChar;
        RvChar   *strIncomingPref;
        RvUint32  incomingPrefLen;
        RvChar   *strOutgoingPref;
        RvUint32  outgoingPrefLen;
        RvChar   *strHeaderPref;
        RvUint32  headerPrefLen;

    /*    RvBool IsFilterExist;*/
        RvUint32  index = 0;
        RvUint32  sizeLeft;
        RvChar    CRLFCRLF[5]   = {ASCII_CR,ASCII_LF,ASCII_CR,ASCII_LF,0};
        RvChar    CRCR[3]       = {ASCII_CR,ASCII_CR,0};
        RvChar    LFLF[3]       = {ASCII_LF,ASCII_LF,0};
        if (bAdditional == RV_TRUE)
        {
            strIncomingPref = PRINT_INCOMING_MSG_EXTERN_PREF;
            incomingPrefLen = (RvUint32)strlen(PRINT_INCOMING_MSG_EXTERN_PREF);
            strOutgoingPref = PRINT_OUTGOING_MSG_EXTERN_PREF;
            outgoingPrefLen = (RvUint32)strlen(PRINT_OUTGOING_MSG_EXTERN_PREF);
            strHeaderPref   = PRINT_HEADER_EXTERN_PREF;
            headerPrefLen   = (RvUint32)strlen(PRINT_HEADER_EXTERN_PREF);
        }
        else
        {
            strIncomingPref = PRINT_INCOMING_MSG_PREF;
            incomingPrefLen = (RvUint32)strlen(PRINT_INCOMING_MSG_PREF);
            strOutgoingPref = PRINT_OUTGOING_MSG_PREF;
            outgoingPrefLen = (RvUint32)strlen(PRINT_OUTGOING_MSG_PREF);
            strHeaderPref   = PRINT_HEADER_PREF;
            headerPrefLen   = (RvUint32)strlen(PRINT_HEADER_PREF);
        }
        pHead = pTemp = pBuf;
        while (*pHead != '\0' && *pHead != 0x01)
        {
            while (*pTemp != 0x0D && *pTemp != 0x0A && *pTemp != '\0')
            {
                ++pTemp;
            }
            tempChar = *pTemp;
            *pTemp = '\0';
            if (index == 0 && msgDirection != SIP_TRANSPORT_MSG_BUILDER_UNDEFINED)
            {
                if (msgDirection == SIP_TRANSPORT_MSG_BUILDER_OUTGOING)
                {
                    if (strlen(pHead) > SIP_TRANSPORT_LOG_MAX_LINE_LENGTH - outgoingPrefLen)
                    {
                        RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                                   "%s   The line is too long",strOutgoingPref));

                    }
                    else
                    {
                        RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                            "%s%s",strOutgoingPref,pHead));

                    }

                }
                else
                {
                    if (strlen(pHead) > (SIP_TRANSPORT_LOG_MAX_LINE_LENGTH - incomingPrefLen))
                    {
                        RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                            "%s   The line is too long",strIncomingPref));

                    }
                    else
                    {
                        RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                                                    "%s%s",strIncomingPref,pHead));
                    }
                }
            }
            else
            {
                if (strlen(pHead) > SIP_TRANSPORT_LOG_MAX_LINE_LENGTH - headerPrefLen)
                {
                    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                              "%sThe line is too long",strHeaderPref));
                }
                else
                {
                    RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                        "%s%s",strHeaderPref,pHead));
                }

            }

            *pTemp = tempChar;
            /* Find how long is the buffer that is left until the terminating '\0'.
               Since we compare up to 4 characters, counting up to 4 is enough. */
            sizeLeft = 0;
            while (('\0' != *pTemp) &&
                   (5 > sizeLeft))
            {
                sizeLeft++;
                pTemp++;
            }
            /* pTemp will return to the beginning */
            pTemp -= sizeLeft;
            if (0 == sizeLeft)
            {
                return;
            }
            if ((4 <= sizeLeft) &&
                (0 == strncmp(pTemp, CRLFCRLF, 4)))
            {
                /* CRLF CRLF, print an empty line*/
                RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                           ""));

                /*LOG_PrintRoutine(pTransportMgr->pLogSrc, RV_INFO, "");*/
                pTemp += 4;
            }
            else if ((2 <= sizeLeft) &&
                     ((0 == strncmp(pTemp, CRCR, 2)) ||
                      (0 == strncmp(pTemp, LFLF, 2))))
            {
                /* CR CR or LF LF, print an empty line */
                RvLogInfo(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                           ""));

               /* LOG_PrintRoutine(pTransportMgr->pLogSrc, RV_INFO, "");*/
                pTemp += 2;
            }
            else
            {
                while (*pTemp == 0x0D || *pTemp == 0x0A || *pTemp == '\0')
                {
                    if (*pTemp == '\0')
                    {
                        return;
                    }
                    ++pTemp;
                }
            }
            pHead = pTemp;
            ++index;
        }
    }
}

#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/


/*-----------------------------------------------------------------------*/
/*                           STATIC FUNCTIONS                            */
/*-----------------------------------------------------------------------*/

/************************************************************************************
 * TransportMsgBuilderScanUDPMessagePosition
 * ----------------------------------------------------------------------------------
 * General: receive an index in a UDP message buffer and checks if this is the
 *          end of message, or if there is a need to set spaces instead of horizontal tabs
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - the transport manager.
 *          sipMsgStartIndex - the index of the start of the sip part
 *          sipMsgEndIndex   - the index of the end of the sip part (without SDP).
 *          pBuffer          - the parsed buffer.
 *          bufSize          - size of given buffer
 *          hMsg             - handle to the already constructed message
 ***********************************************************************************/
/*
static RvBool RVCALLCONV TransportMsgBuilderScanUDPMessagePosition(
										   IN RvChar   *pBuf,
										   IN RvInt32   index,
										   OUT RvInt32 *sipMsgEndIndex,
										   OUT RvInt32 *sipMsgStartBody)
{

	if  (pBuf[index] == SIP_MSG_BUILDER_CR &&
		 pBuf[index + 1] == SIP_MSG_BUILDER_LF)
	{
		if(pBuf[index + 2] == SIP_MSG_BUILDER_CR &&
		   pBuf[index + 3] == SIP_MSG_BUILDER_LF)
		{
			*sipMsgEndIndex = index;
			*sipMsgStartBody = index + 4;
			return RV_TRUE;
		}
		else
		{
			if(pBuf[index + 2] == SIP_MSG_BUILDER_SP ||
			 pBuf[index + 2] == SIP_MSG_BUILDER_HT)
			{
				pBuf[index] = SIP_MSG_BUILDER_SP;
				pBuf[index + 1] = SIP_MSG_BUILDER_SP;
				return RV_FALSE;
			}
		}
	}
	return RV_FALSE;
}
*/
/************************************************************************************
 * TransportMsgBuilderPrepare
 * ----------------------------------------------------------------------------------
 * General: Separate the raw buffer to sip message and sdp body. the function also
 *          enters spaces instead of CRLF if there are spaces or tabs after the CRLF.
 * Return Value: RvStatus - RV_OK, RV_ERROR_BADPARAM
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   regId            - the register id to the log
 *          bufSize          - the number of bytes that were received from the network
 *          pBuffer          - the buffer that was received.
 * Output:  pBuffer          - the corrected buffer
 *          sipMsgStartIndex - the index of the start of the sip part
 *          sipMsgEndIndex   - the index of the end of the sip part (without SDP).
 ***********************************************************************************/
static RvStatus RVCALLCONV TransportMsgBuilderUdpPrepare(
                                              IN RvLogSource*    regId,
                                              IN RvInt32         bufSize,
                                              INOUT RvChar       *pBuffer,
                                              OUT RvInt32        *sipMsgStartIndex,
                                              OUT RvInt32        *sipMsgEndIndex)
{
    RvBool      IsEndMsg;
    RvInt32     index;
    RvInt32     startIndex;
    RvInt32     startBody;
    RvInt32     byteToCheckForward;

    /* passing CR or LF characters in the beginning of the buffer */
    for (startIndex = 0; startIndex <= bufSize; startIndex++)
    {
        if (pBuffer[startIndex] != SIP_MSG_BUILDER_CR &&
            pBuffer[startIndex] != SIP_MSG_BUILDER_LF)
        {
            break;
        }
    }

    for (index = startIndex; index < (bufSize - 3); ++index)
    {
        byteToCheckForward = (bufSize-index)-1;
        /* looking for CRLF CRLF, CR CR or LF LF */
       /* IsEndMsg = TransportMsgBuilderScanUDPMessagePosition(pBuffer,index,sipMsgEndIndex,&startBody);*/
        IsEndMsg = TransportMsgBuilderIsEndofMsg(pBuffer,
                                                index,
                                                NULL,
                                                sipMsgEndIndex,
                                                byteToCheckForward,
                                                &startBody);
        if (IsEndMsg == RV_TRUE)
        {

            RvLogInfo(regId,(regId,
                "TransportMsgBuilderUdpPrepare - Message preparing was done successfully"));
            *sipMsgStartIndex = startIndex;
            return RV_OK;
        }
        /* insert spaces to the buffer in the right places */
        TransportMsgBuilderSetSpaces(index, pBuffer);
    }

    /* if we got here, the buffer does not contain empty line */
    RvLogError(regId,(regId,
               "TransportMsgBuilderUdpPrepare - Message preparing failed"));

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(regId);
#endif

    return RV_ERROR_BADPARAM;

}

/************************************************************************************
 * ParseMsg
 * ----------------------------------------------------------------------------------
 * General: build a message from the prepared buffer
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - the transport manager.
 *          sipMsgStartIndex - the index of the start of the sip part
 *          sipMsgEndIndex   - the index of the end of the sip part (without SDP).
 *          pBuffer          - the parsed buffer.
 *          bufSize          - size of given buffer
 *          hMsg             - handle to the already constructed message
 ***********************************************************************************/
static RvStatus RVCALLCONV ParseMsg(
                                             IN TransportMgr          *pTransportMgr,
                                             IN RvUint32             sipMsgStartIndex,
                                             IN RvUint32             sipMsgEndIndex,
                                             IN RvChar               *pBuffer,
                                             IN RvUint32             bufSize,
                                             IN RvSipMsgHandle        hMsg)
{
    RvStatus      status;
    RvChar        *pSDP;
    RvUint32      parsedBuf;
    RvInt32       sdpLength;
    RvInt32       contentLength;
    RvUint32      startBodyIndex;
    RvChar       tmpChar=0;

    RvSipMsgSetContentLengthHeader(hMsg,UNDEFINED);

    parsedBuf = sipMsgEndIndex + 1;

    /* remember the char */
    tmpChar = pBuffer[parsedBuf];

    pBuffer[parsedBuf] = SIP_MSG_BUILDER_B0;
    status = SipMsgParse(hMsg, &(pBuffer[sipMsgStartIndex]), (parsedBuf-sipMsgStartIndex)+1);
    
    /* replace the char we changed earlier (even if parse failed) */
    pBuffer[parsedBuf]=tmpChar;

    if (status != RV_OK)
    {
        RvLogError(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                   "ParseMsg - Message %p failed to be parsed",  hMsg));
        return status;
    }
    
    /* getting pointer to the start of the SDP - start to check the end of msg from the
        index of the end we have found earlier */
    pSDP = TransportMsgBuilderGetSDPStart(pBuffer, parsedBuf-1,&startBodyIndex);
    if  (pSDP==NULL)
    {
        RvLogError(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
            "ParseMsg - No End of message"));

        return RV_ERROR_UNKNOWN;
    }
    
    /* calculating the sdp length and content length */
    sdpLength = bufSize-startBodyIndex;
    contentLength = RvSipMsgGetContentLengthHeader(hMsg);

    /* checking if the sdp length is equal to the content length */
    if(((sdpLength == 0 && contentLength == UNDEFINED) ||
       (sdpLength == 0 && contentLength == 0) ||
       (sdpLength == contentLength)) == RV_FALSE)
    {
         RvLogWarning(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                   "ParseMsg - content length (%d) and actual sdp length(%d) are not the same",
                   contentLength,sdpLength));
    }

    /* setting the sdp body to the message if it is not zero size */
    if(sdpLength != 0)
    {
#ifndef RV_SIP_PRIMITIVES
        RvSipBodyHandle hBody;

        hBody = RvSipMsgGetBodyObject(hMsg);
        if (NULL == hBody)
        {
            status = RvSipBodyConstructInMsg(hMsg, &hBody);
            if (RV_OK != status)
            {
                RvLogInfo(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                          "ParseMsg - Failed to set body in message"));

                return status;
            }
        }
        status = RvSipBodySetBodyStr(hBody, pSDP, sdpLength);
        if (RV_OK != status)
        {
            RvLogInfo(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                      "ParseMsg - Failed to set body in message"));

            return status;
        }
#else

        status = RvSipMsgSetBody(hMsg, pSDP);
        if (status != RV_OK)
        {
            RvLogInfo(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                       "ParseMsg - Failed to set body in message"));

            return status;
        }

        /* since the above function updates the content length, if we did not get
           a content length in the message, update it back to undefined */
        if(contentLength == UNDEFINED)
        {
            RvSipMsgSetContentLengthHeader(hMsg,UNDEFINED);
        }
#endif /* #ifndef RV_SIP_PRIMITIVES */
    }
    RvLogInfo(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
               "ParseMsg - Message 0x%p was built successfully",
               hMsg));

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransportMgr);
#endif

    return RV_OK;
}


/************************************************************************************
 * TransportMsgBuilderIsEndofMsg
 * ----------------------------------------------------------------------------------
 * General: decide if we reached the end of the message by checking the combinations
 *          CRLF CRLF, CR CR, LF LF.
 * Return Value: RvBool
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pBuf            - the buffer that was received from the network
 *          index           - the current location in the buffer
 *            hConn            - the connection.
 *          bytesForwardToCheack - how many bytes to check after index.
 *                  we need it in order not to check byes after the pBuff
 *                  limits.
 *                  the MESSAGE can end with CR LF CR LF or CR CR or LF LF.
 *                  the loop that calls this function can send an index that
 *                  is equals to buffSize-1 and if we will check index+2 or +3we might crash.
 * Output:  sipMsgEndIndex  - the index of the end of the sip part (without SDP).
 *            sipMsgStartBody - where the body starts.
 ***********************************************************************************/
static RvBool RVCALLCONV TransportMsgBuilderIsEndofMsg(IN RvChar   *pBuf,
                                               IN RvInt32            index,
                                               IN  RvSipTransportConnectionHandle   hConn,
                                               OUT RvInt32 *sipMsgEndIndex,
                                               IN RvUint32    bytesForwardToCheack,
                                               OUT RvInt32 *sipMsgStartBody)

{

    if (bytesForwardToCheack>1)
    {

        /* handling the cases: CR CR X X, LF LF X X */
        if ((pBuf[index] == SIP_MSG_BUILDER_CR &&
             pBuf[index + 1] == SIP_MSG_BUILDER_CR) ||
            (pBuf[index] == SIP_MSG_BUILDER_LF &&
             pBuf[index + 1] == SIP_MSG_BUILDER_LF))
        {
            *sipMsgEndIndex = index;
            *sipMsgStartBody = index+2;
            return RV_TRUE;
        }

        if (bytesForwardToCheack>2)
        {
            /* handling the cases: CR LF CR LF */
            if  ((pBuf[index] == SIP_MSG_BUILDER_CR &&
                pBuf[index + 1] == SIP_MSG_BUILDER_LF &&
                pBuf[index + 2] == SIP_MSG_BUILDER_CR &&
                pBuf[index + 3] == SIP_MSG_BUILDER_LF) )
            {
                *sipMsgEndIndex = index ;
                *sipMsgStartBody = index + 4;
                return RV_TRUE;
            }
            /* handling the cases: X CR CR X, X LF LF X */
            else if ((pBuf[index + 1] == SIP_MSG_BUILDER_CR &&
                pBuf[index + 2] == SIP_MSG_BUILDER_CR) ||
                (pBuf[index + 1] == SIP_MSG_BUILDER_LF &&
                pBuf[index + 2] == SIP_MSG_BUILDER_LF))
            {
                *sipMsgEndIndex = index + 1;
                *sipMsgStartBody = index + 3;
                return RV_TRUE;

            }

            /* handling the cases: X X CR CR, X X LF LF */
            else if((pBuf[index + 2] == SIP_MSG_BUILDER_CR &&
                     pBuf[index + 3] == SIP_MSG_BUILDER_CR) ||
                    (pBuf[index + 2] == SIP_MSG_BUILDER_LF &&
                     pBuf[index + 3] == SIP_MSG_BUILDER_LF))
            {
                *sipMsgEndIndex = index + 2;
                *sipMsgStartBody = index + 4;
                return RV_TRUE;
            }
        }
    }
    if (hConn!=NULL)
    {
        TransportConnection* pConn = (TransportConnection*)hConn;
           RvBool    b_isEndOfMessage;
        if((index ==0) && pConn->recvInfo.hMsgReceived!=NULL_PAGE)
        {
            b_isEndOfMessage=CheckPrevPacketForEndOfMsg(
                                         pBuf, pConn, sipMsgEndIndex,
                                         sipMsgStartBody);

            return b_isEndOfMessage;
        }


    }

    return RV_FALSE;

}


/************************************************************************************
 * TransportMsgBuilderSetSpaces
 * ----------------------------------------------------------------------------------
 * General: look for end of line characters and a space or tab afterwards.
 *          if found then insert spaces instead of the end of line characters.
 *          (end of line characters - CR or LF or CRLF)
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   index       - the current location in the buffer
 *          pBuf        - the buffer that was received from the network
 * Output:  pBuf        - the corrected buffer.
 ***********************************************************************************/
static void RVCALLCONV TransportMsgBuilderSetSpaces(IN RvUint32    index,
                                           INOUT RvChar   *pBuf)
{
    /* handling the case of CRLF and afterwards SP or HT */
    if ((pBuf[index] == SIP_MSG_BUILDER_CR &&
         pBuf[index + 1] == SIP_MSG_BUILDER_LF) &&
        (pBuf[index + 2] == SIP_MSG_BUILDER_SP ||
         pBuf[index + 2] == SIP_MSG_BUILDER_HT))
    {
        pBuf[index] = SIP_MSG_BUILDER_SP;
        pBuf[index + 1] = SIP_MSG_BUILDER_SP;
    }
    /* handling the cases of CR or LF and afterwards SP or HT */
    else if ((pBuf[index] == SIP_MSG_BUILDER_CR ||
              pBuf[index] == SIP_MSG_BUILDER_LF) &&
             (pBuf[index + 1] == SIP_MSG_BUILDER_SP ||
              pBuf[index + 1] == SIP_MSG_BUILDER_HT))
    {
        pBuf[index] = SIP_MSG_BUILDER_SP;
    }

}



/************************************************************************************
 * TransportMsgBuilderGetSDPStart
 * ----------------------------------------------------------------------------------
 * General: get pointer to the start of the SDP.
 * Return Value: RvChar* - the pointer to the start of SDP
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pBuf        - the buffer that was received from the network
 *          index       - the sip msg end index (without SDP).
 * Output:  none
 ***********************************************************************************/
static RvChar * RVCALLCONV TransportMsgBuilderGetSDPStart(IN RvChar      *pBuf,
                                                           IN RvUint32    index,
                                                           OUT RvUint32 *startBodyIndex)
{


    if ((pBuf[index] == SIP_MSG_BUILDER_CR &&
        pBuf[index + 1] == SIP_MSG_BUILDER_CR) ||
        (pBuf[index] == SIP_MSG_BUILDER_LF &&
        pBuf[index + 1] == SIP_MSG_BUILDER_LF))
    {
        *startBodyIndex = index+2;
        return &pBuf[index + 2];
    }

    /* handling the cases: CR LF CR LF*/
    else if  ((pBuf[index] == SIP_MSG_BUILDER_CR &&
        pBuf[index + 1] == SIP_MSG_BUILDER_LF &&
        pBuf[index + 2] == SIP_MSG_BUILDER_CR &&
        pBuf[index + 3] == SIP_MSG_BUILDER_LF))
    {
        *startBodyIndex = index+4;
        return &pBuf[index + 4];
    }
    else
    {
        return NULL;
    }

}


/************************************************************************************
* SipTransportMsgBuilderUdpMakeMsg
* ----------------------------------------------------------------------------------
* General: separating the sip msg from the sdp, set spaces instead of CRLF and then
*          construct msg and parse it. afterwards the function set the body to the msg.
* Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
* ----------------------------------------------------------------------------------
* Arguments:
* Input:   hTransportMgr - Handle to the transport mgr.
*          pBuf          - the buffer that was received from the network
*          bufSize       - the buffer size.
*          bConstructMsg - construct a new message (in case of transport message)
*                          or is it already constructed (in case of calling to
*                          (RvSipMsgParse() API)
*          hConstructedMsg - given message in case bConstructMsg=RV_FALSE.
*          eTransportType  - transport OF the message.
* Output:  hMsg            - handle to the new message in case bConstructMsg=RV_TRUE.
*          pSigCompMsgInfo - Pointer SigComp message info. structure that is
*                            filled according to the built (OUT) message.
*          ppPlainMsgBuf   - Pointer to the plain SIP message buffer (decompressed)
*                            This pointer can be NULL if the original buffer is
*                            plain and doesn't require any additional processing.
***********************************************************************************/
RvStatus RVCALLCONV SipTransportMsgBuilderUdpMakeMsg(
                                  IN  RvSipTransportMgrHandle     hTransportMgr,
                                  IN  RvChar                     *pBuf,
                                  IN  RvUint32                    bufSize,
                                  IN  RvBool                      bConstructMsg,
                                  IN  RvSipMsgHandle              hConstructedMsg,
								  IN  RvSipTransport              eTransportType,
                                  OUT RvSipMsgHandle             *phMsg,
                                  OUT SipTransportSigCompMsgInfo *pSigCompMsgInfo,
                                  OUT RvChar                    **ppPlainMsgBuf)
{
    RvStatus      status;
    RvInt32       msgSize;
    RvInt32       msgStart;
    RvSipMsgHandle hMsgToGive;
    RvChar       *pParsedMsgBuf    = pBuf;
    RvInt32       parsedMsgBufLen  = bufSize;
    TransportMgr  *pTransportMgr =(TransportMgr*)hTransportMgr;
#ifdef RV_SIGCOMP_ON
    MsgBuilderSigCompInfo builderSigCompInfo;
#else
	RV_UNUSED_ARG(eTransportType);
#endif

/*  SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    RvChar*       SPIRENT_buffer = NULL;
    int           SPIRENT_size   = 0;
#endif
/*  SPIRENT_END */

/* SPIRENT_BEGIN */
/*
* COMMENTS:
* Modified by Armyakov
* Counting UDP incoming messages 
*/
#if defined(UPDATED_BY_SPIRENT)
        RvSipRawMessageCounterHandlersRun(SPIRENT_SIP_RAW_MESSAGE_INCOMING);
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

    /* The pointer might be NULL that disables the decompression process */
    if (ppPlainMsgBuf != NULL)
    {
        *ppPlainMsgBuf = NULL;
    }
#ifdef RV_SIGCOMP_ON
    if (pSigCompMsgInfo != NULL && ppPlainMsgBuf != NULL)
    {
        builderSigCompInfo.pBuf    = pBuf;
        builderSigCompInfo.bufSize = bufSize;
        status = DecompressCompeleteMsgBufferIfNeeded(pTransportMgr,eTransportType,
                                                      &builderSigCompInfo,
                                                      pSigCompMsgInfo);
        if (status != RV_OK)
        {
            RvLogWarning(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                "SipTransportMsgBuilderUdpMakeMsg - DecompressCompeleteMsgBufferIfNeeded() failed"));
            return status;
        }
        if (builderSigCompInfo.pDecompressedBuf != NULL)
        {
            *ppPlainMsgBuf  = builderSigCompInfo.pDecompressedBuf;
            pParsedMsgBuf   = builderSigCompInfo.pDecompressedBuf;
            parsedMsgBufLen = builderSigCompInfo.decompressedBufSize;
        }
    }
#endif

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)

    SPIRENT_buffer = (RvChar*)HSTPOOL_Alloc(&SPIRENT_size);

    if(SPIRENT_buffer == NULL)
    {
        RvLogError(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
             "SipTransportMsgBuilderUdpMakeMsg - failed to allocate hst-buffer"));
        return RV_ERROR_UNKNOWN;
    }
    if(SPIRENT_size < parsedMsgBufLen)
    {
        RvLogError(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
             "SipTransportMsgBuilderUdpMakeMsg - insufficient hst-buffer's length=%d (<msglength=%d)", 
             SPIRENT_size, parsedMsgBufLen));
        HSTPOOL_Release(SPIRENT_buffer);
        return RV_ERROR_UNKNOWN;
    }
    // copy the original received data
    memcpy(SPIRENT_buffer, pParsedMsgBuf, parsedMsgBufLen);
    pParsedMsgBuf = SPIRENT_buffer;

    // modify the data if needed
    if ( g_SIPC_Filter_Buffer ) 
    {
        parsedMsgBufLen = g_SIPC_Filter_Buffer (pParsedMsgBuf, parsedMsgBufLen , SPIRENT_size - 1 );
    }
#endif
/*  SPIRENT_END */

    /* preparing the buffer */
    status = TransportMsgBuilderUdpPrepare(pTransportMgr->pMBLogSrc,
                                           (RvInt32)parsedMsgBufLen,
                                           pParsedMsgBuf,
                                           &msgStart,
                                           &msgSize);
    if (status != RV_OK)
    {
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
        HSTPOOL_Release(SPIRENT_buffer);
#endif
/* SPIRENT_END */
        return status;
    }

    if(bConstructMsg == RV_TRUE)
    {
        status = RvSipMsgConstruct(pTransportMgr->hMsgMgr,
                                   pTransportMgr->hMsgPool,
                                   phMsg);
        if (status != RV_OK)
        {
            RvLogError(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                       "SipTransportMsgBuilderUdpMakeMsg - Message construction failed"));

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
            HSTPOOL_Release(SPIRENT_buffer);
#endif
/* SPIRENT_END */
            return status;
        }
        hMsgToGive = *phMsg;
    }
    else
    {
        hMsgToGive = hConstructedMsg;
    }

    /* building the message */
    status = ParseMsg(pTransportMgr, msgStart, msgSize, pParsedMsgBuf,
                      parsedMsgBufLen, hMsgToGive);
    if (status != RV_OK)
    {
        if(bConstructMsg == RV_TRUE)
        {
            RvLogError(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                       "SipTransportMsgBuilderUdpMakeMsg - ParseMsg failed"));
            RvSipMsgDestruct(hMsgToGive);
            *phMsg = NULL;
        }
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
        HSTPOOL_Release(SPIRENT_buffer);
#endif
/* SPIRENT_END */

        return status;
    }
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    /* we've just successfully encocoded a message --> let's attach the encoded buffer to the message. */
    if(bConstructMsg == RV_FALSE)
    {
       /* This is an old message; so, we would have to remove its existing encoded message before attaching
          a new one to it */
       RvSipMsgReleaseSPIRENTBody(hMsgToGive);
    }

    HSTPOOL_SetBlockSize   (SPIRENT_buffer, parsedMsgBufLen );
    RvSipMsgSetSPIRENTBody (hMsgToGive, SPIRENT_buffer );
#endif
/* SPIRENT_END */
    RV_UNUSED_ARG(pSigCompMsgInfo);

    return RV_OK;
}

/************************************************************************************
* TransportMsgBuilderTcpFindHeader
* ----------------------------------------------------------------------------------
* General: find if the current character along with the next ones is an end of header.
* Return Value: RvStatus - RV_OK, RV_ERROR_BADPARAM
* ----------------------------------------------------------------------------------
* Arguments:
* Input:   index         - the index in the buffer
*          pBuf          - the buffer to look into
*           conn          - the connection
*          bytesForwardToCheack - how many bytes to check after index.
*                  we need it in order not to check byes after the pBuff
*                  limits.
*                  the header can end with CR LF or CR or LF, but after that
*                   we have to check that there is no SP.
*                  the loop that calls this function can send an index that
*                  is equals to buffSize-1 and if we will check index+2 we might crash.
* Output:  headerIndex   - the index of the end of the sip part (without SDP).
***********************************************************************************/
static RvStatus RVCALLCONV TransportMsgBuilderTcpFindHeader(
                                                   IN RvUint32            index,
                                                   IN RvChar              *pBuf,
                                                   IN TransportConnection  *conn,
                                                   IN RvUint32                bytesForwardToCheack,
                                                   OUT RvUint32           *headerIndex)
{
    if (bytesForwardToCheack >1)
    {
        /* try to find the case of CRLF or CR or LF and afterwards *NOT* SP or HT */
        /* handling the cases of CRLF and afterwards SP or HT */
        if ((pBuf[index] == SIP_MSG_BUILDER_CR &&
            pBuf[index+1] == SIP_MSG_BUILDER_LF) &&
            (pBuf[index + 2] != SIP_MSG_BUILDER_SP &&
            pBuf[index + 2] != SIP_MSG_BUILDER_HT))
        {
            *headerIndex =index + 2;
            return RV_OK;
        }
    }
    /* handling the cases of CR or LF and afterwards SP or HT */
    if (((pBuf[index] == SIP_MSG_BUILDER_CR) &&
        (pBuf[index + 1] != SIP_MSG_BUILDER_SP &&
        pBuf[index + 1] != SIP_MSG_BUILDER_HT) )||
        ((pBuf[index] == SIP_MSG_BUILDER_LF) &&
        (pBuf[index + 1] != SIP_MSG_BUILDER_SP &&
        pBuf[index + 1] != SIP_MSG_BUILDER_HT)))
    {
        *headerIndex =index + 1;
        return RV_OK;
    }

    if (conn!=NULL)
    {
        if((index ==0) && conn->recvInfo.hMsgReceived!=NULL_PAGE)
        {
            if(CheckPrevPacketForEndOfHeader(pBuf,conn) == RV_TRUE)
            {
                return RV_OK;
            }
        }
    }
    return RV_ERROR_UNKNOWN;
}
/************************************************************************************
 * TransportMsgBuilderTcpPrepare
 * ----------------------------------------------------------------------------------
 * General:  separate the raw buffer to sip message and sdp body. the function
 *           enters spaces instead of CRLF if there are spaces or tabs after the CRLF.
 *            whenever the function recognize a CRLF represent header start/end , it
 *            goes over this header and tries to find within this header a content-length.
 *            the header can be in the same buffer arrived from the network and in that case
 *            the check can be made as a regular string compare.
 *            in case the header is apart, which means the last network buffer was saved
 *            into page and the first byte of the current buffer are in fact the last bytes
 *            of this header, we append the part of the buffer that complete the previous
 *            parted header to the page , and go over this header but in "page" mode to
 *            search for 'content-length' (only in case it haven't found yet).
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   conn             - the current connection.
 *          bufSize          - the number of bytes that were received from the network
 *          pBuffer          - the buffer that was received.
 * Output:
 *          sipMsgStartIndex - the index of the start of the sip part
 *          sipMsgEndIndex   - the index of the end of the sip part (without SDP).
 *           msgComplete      - boolean indicating whether full msg (headers+body)was found.
 ***********************************************************************************/
static RvStatus RVCALLCONV TransportMsgBuilderTcpPrepare(
                                         IN    TransportConnection *conn,
                                         IN    RvInt32              bufSize,
                                         INOUT RvChar              *pBuffer,
                                         OUT   RvUint32            *sipMsgStartIndex,
                                         OUT   RvInt32             *sipMsgEndIndex,
                                         OUT   RvUint32            *bytesReadFromBuf,
                                         OUT   RvBool              *msgComplete)
{
    RvBool       IsEndMsg;       /* indicates if found end of the headers*/
    RvInt32      index;       /* index in the buffer when going through it*/
    RvInt32      startIndex;  /* the index where the message is actually start- after the CRLFs*/
    RvInt32      offset;       /* used as a parameter to the RPOOL_AppendFromExternal func*/
    RvInt32      headerIndexStart; /*the index in 'pBuffer' where a header starts.*/
    RvInt32      headerIndexEnd;    /*the index in 'pBuffer' where a header ends.*/
    RvInt32     sipMsgStartBody;    /*the index in 'pBuffer' where the body starts.*/
    RvStatus       status;
    RvInt32      headerPreCRLFs;   /* how many unnecessary CRLFs were at the beginning of the sip msg*/
    RvInt32      bytesLeftInBuffer;
    RvUint32     byteToCheckForward;

    headerIndexStart  = 0;
    headerIndexEnd    = 0;
    offset            = 0;
    /* Initialize the out parameters */
    *sipMsgStartIndex = 0;
    *sipMsgEndIndex   = 0;
    *bytesReadFromBuf = 0;
    *msgComplete      = RV_FALSE;

    if (conn->recvInfo.hMsgReceived != NULL_PAGE) /*means we had previous tcp part*/
    {
        /*if the last part was only headers, continue to the section code
           dealing with the headers */
        /*check if reading headers or body*/
        if ((conn->recvInfo.bReadingBody==RV_TRUE))
        {

            status = AppendBodyToMsg(conn,
                                    pBuffer,
                                    bufSize,
                                    msgComplete,
                                    bytesReadFromBuf);
            return status;
        }
    }

    /*existing message*/
    if (conn->recvInfo.hMsgReceived != NULL_PAGE)
    {
        /*if this buffer is a continuation of last one.
        don't pass unneeded CRLF. start to check from the beginning*/
        startIndex = 0;
        headerPreCRLFs = 0;
    }
    else /*new message*/
    {
        /* passing CR or LF characters in the beginning of the buffer */
        for (startIndex = 0; startIndex <= bufSize; startIndex++)
        {
            if (pBuffer[startIndex] != SIP_MSG_BUILDER_CR &&
                pBuffer[startIndex] != SIP_MSG_BUILDER_LF)
            {
                break;
            }
        }
        /*we need to know how many CRLF were in the beginning so we won't count them
        in the msgSize*/
        headerPreCRLFs = startIndex;
    }

    *sipMsgStartIndex=startIndex;

    /*going over the buffer*/
    for (index = startIndex; index <= (bufSize - 1); ++index)
    {
        byteToCheckForward = (bufSize-index)-1;
        /*    looking for CRLF CRLF, CR CR or LF LF.
            also find where the body actually starts. */
        IsEndMsg = TransportMsgBuilderIsEndofMsg(pBuffer,
                                                index,
                                                (RvSipTransportConnectionHandle)conn,
                                                sipMsgEndIndex,
                                                byteToCheckForward,
                                                &sipMsgStartBody);



        if (IsEndMsg == RV_TRUE)
        {
            headerIndexEnd = *sipMsgEndIndex;
            /*also means we are in the last header, have to check for Content-length
                if was not found yet*/
            if (conn->recvInfo.contentLength == UNDEFINED)
            {
                if (FindContentLengthInHeader(conn,
                                                pBuffer,
                                                sipMsgStartIndex,
                                                headerIndexStart,
                                                headerIndexEnd)!= RV_TRUE)

                {
                     RvLogError(conn->pTransportMgr->pMBLogSrc,(conn->pTransportMgr->pMBLogSrc,
                               "TransportMsgBuilderTcpPrepare - End of Header - No Content-Length Found!"));
                    return RV_ERROR_UNKNOWN;
                }
            }

            /*no need further checking about content-length since we have
             just found it.
             the size of the header not includes the CRLFs found at the beginning*/
            conn->recvInfo.bodyStartIndex = conn->recvInfo.msgSize+sipMsgStartBody-headerPreCRLFs;
            conn->recvInfo.msgSize = conn->recvInfo.msgSize + (RvInt)*sipMsgEndIndex-headerPreCRLFs;

            /* change state */
            conn->recvInfo.bReadingBody =RV_TRUE;
            bytesLeftInBuffer = bufSize-sipMsgStartBody;

            status = CopyBodyToMsg(conn,
                            pBuffer,
                            bufSize,
                            sipMsgStartIndex,
                            sipMsgStartBody,
                            bytesLeftInBuffer,
                            msgComplete,
                            bytesReadFromBuf);


                return status;
        }
        /* insert spaces to the buffer in the right places */
        TransportMsgBuilderSetSpaces(index,
                                     pBuffer);
        /* try to find new header*/
        status = TransportMsgBuilderTcpFindHeader(  (RvUint32)index,
                                                    pBuffer,
                                                    conn,
                                                    byteToCheckForward,
                                                    (RvUint32*)&headerIndexEnd);
        if (status==RV_OK)
        {
            index=headerIndexEnd;

            if (conn->recvInfo.contentLength == UNDEFINED)
            {
                if (FindContentLengthInHeader(conn,
                                            pBuffer,
                                            sipMsgStartIndex,
                                            headerIndexStart,
                                            headerIndexEnd)!= RV_TRUE)
                {
                    headerIndexStart = headerIndexEnd;
                    continue;
                }
            }
            /*update the index of the last header found by a recognition of a 'good'
              CRLF*/
            headerIndexStart = headerIndexEnd;
        }
    }

    /*this message part does not contain content-length and we reached the end of it.
       The SIP msg (without the body) so far is the buffer size minus the CRLFs
       appeared in the beginning of the msg */
    conn->recvInfo.msgSize +=bufSize - headerPreCRLFs;

    /* if we got here, we have to save the message since its only part of the whole msg,
       and the body hasn't arrived yet */
    if (conn->recvInfo.msgSize != 0)
    {

        if (conn->recvInfo.hMsgReceived == NULL_PAGE)
        {
            status = RPOOL_GetPage(conn->pTransportMgr->hMsgPool,
                                0,
                                &(conn->recvInfo.hMsgReceived));

            if (status!=RV_OK)
            {
                RvLogError(conn->pTransportMgr->pMBLogSrc,(conn->pTransportMgr->pMBLogSrc,
                           "TransportMsgBuilderTcpPrepare - couldn't allocate buffer for incoming parted tcp msg."));
                return status;
            }
        }



        *bytesReadFromBuf = bufSize;
        /*copy to page only the relevant data, which means:
            1. without CRLFs (if were) in the beginning
            2. without the last part of the first header in this buffer, that was
               copied earlier when was search for content length.
        */
        if(bufSize-(*sipMsgStartIndex) > 0)
        {

            status = RPOOL_AppendFromExternal(conn->pTransportMgr->hMsgPool,
                conn->recvInfo.hMsgReceived,
                (void *)(pBuffer+*sipMsgStartIndex),
                bufSize-(*sipMsgStartIndex),
                RV_FALSE,
                &offset,
                NULL);
            if (status!=RV_OK)
            {
                RvLogError(conn->pTransportMgr->pMBLogSrc,(conn->pTransportMgr->pMBLogSrc,
                    "TransportMsgBuilderTcpPrepare - couldn't append msg to Page."));

                return status;
            }
        }
    }
    RvLogInfo(conn->pTransportMgr->pMBLogSrc,(conn->pTransportMgr->pMBLogSrc,
               "TransportMsgBuilderTcpPrepare - Message preparing came parted,waiting to next part"));
    *msgComplete = RV_FALSE;
    return RV_OK;
}

/************************************************************************************
 * findColonInPage
 * ----------------------------------------------------------------------------------
 * General: try to finds a : after the header name.
 * Return Value: RvBool - true-if content-length was found.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   conn            - pointer to connection
 *            offset    - offset in the page so we can get the pointer to start check..
 *            length  - of the header till the end of it.
 * output:    newOffset  - the pointer to the char after the ":"
 ***********************************************************************************/
static RvBool RVCALLCONV findColonInPage(INOUT TransportConnection *conn,
                                        IN RvInt32 offset,
                                        IN RvInt32 length,
                                        OUT RvInt32 *newOffset)
{
    RvInt32 unfragSize;
    RvInt32 i;
    RvChar* headerToSearch;
    RvInt32 offsetInternal;

    headerToSearch = (RvChar*)RPOOL_GetPtr(conn->pTransportMgr->hMsgPool,
                                            conn->recvInfo.hMsgReceived,
                                            offset);
    for (i=0; i<length ; i=i+unfragSize)
    {

        unfragSize= RPOOL_GetUnfragmentedSize(conn->pTransportMgr->hMsgPool,
                                                conn->recvInfo.hMsgReceived,
                                                offset);

        /*begin in the byte after the l to try find :*/
        for (offsetInternal=0; offsetInternal<=unfragSize;offsetInternal++)
        {
            if((headerToSearch[offsetInternal]!=':') &&
                (headerToSearch[offsetInternal] != SIP_MSG_BUILDER_SP))
            {
                return RV_FALSE;
            }
            else if (headerToSearch[offsetInternal] == ':')
            {
                *newOffset = i + offsetInternal+1;
                return RV_TRUE;
            }
        }
        offset+=unfragSize;
        /* update the pointer to */
        headerToSearch = (RvChar*)RPOOL_GetPtr(conn->pTransportMgr->hMsgPool,
                                                conn->recvInfo.hMsgReceived,
                                                offset);
    }
    return RV_FALSE;

}
/************************************************************************************
 * FindContentLengthInHeaderPage
 * ----------------------------------------------------------------------------------
 * General: try to finds a content-length within a header that is in page.
 *            first getting the pointer to the place where the first char of the word "Content"
 *            resides. then checks to find the whole word , only if the first char was C or l
 *            which is short header.
 * Return Value: RvBool - true-if content-length was found.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   Conn        - pointer to connection
 *
 *            headerSize    - size of the header to search in.
 * return:    RvBool.
 ***********************************************************************************/
static RvBool RVCALLCONV FindContentLengthInHeaderPage(INOUT TransportConnection *conn,
                                                        IN RvInt32 headerSize)
{
    RvChar contentLengthString[15] = "Content-Length";
    RvChar* headerToSearch;
    RvInt32 offset;
    RvInt32 offsetAfterColon;
    RvStatus status;
    RvChar   tmpChar;

    offset = conn->recvInfo.lastHeaderOffset;

    contentLengthString[14] ='\0';

    headerToSearch = (RvChar*)RPOOL_GetPtr(conn->pTransportMgr->hMsgPool,
                                            conn->recvInfo.hMsgReceived,
                                            offset);

    if (headerToSearch==NULL)
    {
        return RV_FALSE;
    }


    if ((*headerToSearch== 'l') ||(*headerToSearch == 'L'))

    {
        /* try to find ":" */
        if (findColonInPage(conn,
                            offset,
                            headerSize-1,
                            &offsetAfterColon))
        {
            /*get the content-length value*/
            status = GetContentLengthFromHeaderPage(
                                                    conn,
                                                    offsetAfterColon,
                                                    headerSize-(offsetAfterColon),
                                                    &(conn->recvInfo.contentLength));
            if (status==RV_OK)
            {
                return RV_TRUE;
            }
            else
            {
                return RV_FALSE;
            }

        }
    }
    else if ((*headerToSearch== 'c') ||(*headerToSearch == 'C'))
    {
        if(headerSize < 15) /*then it cannot be Content-Length:*/
        {
            return RV_FALSE;
        }

        headerToSearch = (RvChar*)RPOOL_GetPtr(conn->pTransportMgr->hMsgPool,
                            conn->recvInfo.hMsgReceived,
                            offset+(RvInt32)strlen("Content-Length"));
        /*save the character*/
        tmpChar = *headerToSearch;
        /* make it null for the comparison*/
        *headerToSearch='\0';

        if (RPOOL_CmpToExternal(conn->pTransportMgr->hMsgPool,
                                conn->recvInfo.hMsgReceived,
                                offset,
                                contentLengthString,
                                RV_FALSE)) /*first char is C in case of "Content-Length"*/
        {
            /*return to the original char*/
            *headerToSearch=tmpChar;
            /*find ":"*/
            if (findColonInPage(conn,
                                offset+(RvInt32)strlen("Content-Length"),
                                headerSize-1,
                                &offsetAfterColon))
            {
                status = GetContentLengthFromHeaderPage(
                                                    conn,
                                                    offset+(RvInt32)strlen("Content-Length")+offsetAfterColon,
                                                    (headerSize-(RvInt32)strlen("Content-Length"))-(offsetAfterColon),
                                                    &(conn->recvInfo.contentLength));
                if (status==RV_OK)
                {
                    return RV_TRUE;
                }
                else
                {
                    return RV_FALSE;
                }
            }
        }
        else
        {
            *headerToSearch=tmpChar;
            return RV_FALSE;
        }
    }
    else
    /* this header is not content length*/
    {
        return RV_FALSE;
    }

    return RV_FALSE;

}
/************************************************************************************
 * FindContentLengthInHeader
 * ----------------------------------------------------------------------------------
 * General: try to finds a content-length within a header.
 *            the function appends a buffer to a page and search in the header for content length.
 * Return Value: RvBool - true-if content-length was found.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   conn           - the connection.
 *          buff            - the index of the end of the sip part (without BODY).
 *
 *            startHeader        - start index of header
 *            endHeader        - end index of header
 * Output:    sipStartHeader    - how many byes were copied to the Page.
 *                              needed for not copy this amount of byes again
 *                              when finishing read all the buffer.
 ***********************************************************************************/
static RvBool RVCALLCONV FindContentLengthInHeader(INOUT TransportConnection *conn,
                                                   IN RvChar*            buff,
                                                   INOUT RvUint32        *sipStartHeader,
                                                   IN RvInt32            startHeader,
                                                   INOUT RvInt32            endHeader)
{
    RvInt32 i;
    RvStatus status;
    RvInt32 offsetInPage;
    RvInt32 headerbytesinCurrentBuffer;
    RvInt32 startHeaderPosition;
    RvInt32 contentLengthSize=(RvInt32)strlen("Content-Length");
    RvChar  tmpChar;
    RvInt32    headerSize;
    headerbytesinCurrentBuffer=endHeader;
    if ((conn->recvInfo.hMsgReceived != NULL_PAGE ) && (startHeader==0))
    /*means we had previous tcp part and still in the headers sections*/
    {
        if (endHeader>0)
        {
            /*append the part of the current buffer to the page and try to find on
          it a content length*/
            status = RPOOL_AppendFromExternal(conn->pTransportMgr->hMsgPool,
                                    conn->recvInfo.hMsgReceived,
                                    (void *) buff,
                                    endHeader,
                                    RV_FALSE,
                                    &offsetInPage,
                                    NULL);
            if(status != RV_OK)
            {
                return RV_FALSE;
            }
        }
        /*the whole header size includes the previous length saved to the page
        connection info plus the current size we got now */
        headerSize = (conn->recvInfo.msgSize - conn->recvInfo.lastHeaderOffset)+(endHeader-startHeader);
        /*need for the next time we do append not append twice this part of the message*/
        *sipStartHeader=headerbytesinCurrentBuffer;

        /*look for the "Content-Length" in page.*/
        if (FindContentLengthInHeaderPage(conn,
                                        headerSize))
        {
            return RV_TRUE;
        }
        conn->recvInfo.lastHeaderOffset += headerSize;/*(conn->recvInfo.msgSize - conn->recvInfo.lastHeaderOffset) + (endHeader - startHeader);*/

        return    RV_FALSE;
    }

    else /*find the content length within this buffer- no need to append to hPage*/
    {
        /* not considering the CRLFs in the beginning*/
        if (startHeader == 0)
        {
            startHeader    += *sipStartHeader;
        }

        conn->recvInfo.lastHeaderOffset += (endHeader - startHeader);
        if ((buff[startHeader]=='l') || (buff[startHeader]=='L') ) /*first chat is l in case of l:*/
        {
            for(i=(startHeader)+1;i<endHeader;i++)
            {    if ((buff[i] != ':') && (buff[i] != SIP_MSG_BUILDER_SP) )
                {
                    return RV_FALSE;
                }
                else if (buff[i] == ':')
                {
                    status = GetContentLengthFromHeader(
                                                    conn->pTransportMgr->pMBLogSrc,
                                                    &(buff[i+1]),
                                                    endHeader-(i+1),
                                                    &(conn->recvInfo.contentLength));
                    if (status==RV_OK)
                    {
                        return RV_TRUE;
                    }
                    else
                    {
                        return RV_FALSE;
                    }
                }
            }
        }
        else if ((buff[startHeader]== 'c') ||(buff[startHeader] == 'C'))
        {
            /*make the string  a null terminated one*/
            startHeaderPosition = startHeader;
            tmpChar = buff[startHeaderPosition+contentLengthSize];
            buff[startHeaderPosition+contentLengthSize] = '\0';

            if (RV_TRUE == IsFirstOccurrence(buff+startHeader,"Content-Length"))
            {
                /*change it back*/
                buff[startHeaderPosition+contentLengthSize]=tmpChar;

                for(i=startHeader+contentLengthSize;i<endHeader;i++)
                {
                    if ((buff[i] != ':') && (buff[i] != SIP_MSG_BUILDER_SP) )
                    {
                        return RV_FALSE;
                    }
                    else if (buff[i] == ':')
                    {
                        status = GetContentLengthFromHeader(
                                                        conn->pTransportMgr->pMBLogSrc,
                                                        &(buff[i+1]),
                                                        endHeader-(i+1),
                                                        &(conn->recvInfo.contentLength));
                        if (status==RV_OK)
                        {
                            return RV_TRUE;
                        }
                        else
                        {
                            return RV_FALSE;
                        }
                    }
                }
            }
            /*change it back*/
            buff[startHeaderPosition+contentLengthSize]=tmpChar;
        }
        /* this header is not content length */
        else
        {
            return RV_FALSE;
        }
    }
    return RV_FALSE;
}


/************************************************************************************
 * GetContentLengthFromHeader
 * ----------------------------------------------------------------------------------
 * General: get the content length value from within a header that sits inside page
 *            which is not sequential memory.
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   conn    - pointer to the connection
 *          buff    - the place in the header right after the ':'
 *          length    - the len from the byte after the ":" till the end
 *
 * Output:  contentLength - the value.
 ***********************************************************************************/
static RvStatus RVCALLCONV GetContentLengthFromHeader(
                                                  IN RvLogSource* regId,
                                                  IN RvChar*            buff,
                                                  IN RvInt32            length,
                                                  OUT RvInt32            *contentLength)
{
    RvChar contentLengthStr[20];
    RvInt32 i;
    RvInt32 digitNum;
    RvInt32 loopLimit;

    digitNum=0;

    /* the buffer is 20 characters, and we need to set NULL in it's end,
       so we limit the loop to 19 */
    loopLimit = RvMin(length, 19);

	for (i=0;i<loopLimit;i++)
    {
        if ((buff[i]!=SIP_MSG_BUILDER_SP) && (buff[i]!=SIP_MSG_BUILDER_HT))
        {
            contentLengthStr[digitNum]=buff[i];
            digitNum++;
        }
    }
    contentLengthStr[digitNum]='\0';
    *contentLength = atoi(contentLengthStr);
    if(*contentLength < 0)
    {
        *contentLength=0;
    }
    RvLogInfo(regId,(regId,
              "GetContentLengthFromHeader - Found 'Content-Length: %d '",
                *contentLength));

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(regId);
#endif

    return RV_OK;
}

/************************************************************************************
 * GetContentLengthFromHeaderPage
 * ----------------------------------------------------------------------------------
 * General: get the content length value from within a header that sits inside page
 *            which is not sequential memory.
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   conn        - pointer to the connection
 *          offset        - offset in the page right after the ':'
 *
 *          totalLength - length till the header ends.
 *
 * Output:  contentLength - the value.
 ***********************************************************************************/
static RvStatus RVCALLCONV GetContentLengthFromHeaderPage(
                                                     IN TransportConnection *conn,
                                                     IN RvInt32  offset,
                                                     IN RvInt32  totalLength,
                                                     OUT RvInt32 *contentLength)
{
    RvInt32 unfragSize;
    RvInt32 i;
    RvChar* headerToSearch;
    RvInt32 offsetInternal;
    RvChar  contentLengthStr[20];
    RvInt32 digitNum;

    unfragSize =0;
    digitNum = 0;
    headerToSearch = (RvChar*)RPOOL_GetPtr(conn->pTransportMgr->hMsgPool,
                                            conn->recvInfo.hMsgReceived,
                                            offset);
    for (i=0; i<totalLength ; i=i+unfragSize)
    {

        unfragSize= RPOOL_GetUnfragmentedSize(conn->pTransportMgr->hMsgPool,
                                                conn->recvInfo.hMsgReceived,
                                                offset);

        if (headerToSearch==NULL)
        {
            return  RV_ERROR_UNKNOWN;
        }
        /*begin in the byte after the : to try find the number*/
        for (offsetInternal=0; offsetInternal<unfragSize;offsetInternal++)
        {
            if((headerToSearch[offsetInternal] != SIP_MSG_BUILDER_SP) &&
                digitNum<19/*contentLengthStr is only 20 chars long*/)
            {
                contentLengthStr[digitNum]=headerToSearch[offsetInternal];
                digitNum++;
            }
        }
        offset+=unfragSize;
        headerToSearch = (RvChar*)RPOOL_GetPtr(conn->pTransportMgr->hMsgPool,
                                                conn->recvInfo.hMsgReceived,
                                                offset);
    }

    contentLengthStr[digitNum]='\0';
    *contentLength = atoi(contentLengthStr);
    if(*contentLength < 0)
    {
        *contentLength=0;
    }
    RvLogInfo(conn->pTransportMgr->pMBLogSrc,(conn->pTransportMgr->pMBLogSrc,
              "GetContentLengthFromHeaderPage - Found 'Content-Length: %d '",
               *contentLength));

    return RV_OK;

}


/******************************************************************************
 * TransportMsgBuilderParseUdpMsg
 * ----------------------------------------------------------------------------
 * General: The TransportMsgBuilderParseUdpMsg builds Message object, using
 *          provided buffer, pass the built Message to the upper layers and
 *          frees the buffer.
 *          Main (Select) Thread fills buffer with data received from UDP
 *          socket and generates MESSAGE_RCVD event for TPQ (Transport
 *          Processing Queue). Transport Processing Threads handle this event
 *          and call TransportMsgBuilderParseUdpMsg.
 * Return Value: RvStatus - RV_OK or returned by subsequent calls error value
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - the Transport Manager object
 *          pBuffer       - the SIP message buffer (before parsing).
 *          totalMsgSize  - number of bytes in entire SIP message.
 *          hInjectedMsg  - If there is an 'injected' msg, we should not try to
 *                          parse the UDP message, but take this message instead.
 *          hLocalAddr    - local address where the message was sent.
 *          pRecvFromAddr - address from where the message was sent.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportMsgBuilderParseUdpMsg(
                            IN TransportMgr*                 pTransportMgr,
                            IN RvChar*                       pBuffer,
                            IN RvInt32                       totalMsgSize,
                            IN RvSipMsgHandle                hInjectedMsg,
                            IN RvSipTransportLocalAddrHandle hLocalAddr,
                            IN SipTransportAddress*          pRecvFromAddr)
{
    RvStatus                      rv;
    RvSipMsgHandle                hMsg = hInjectedMsg;
    SipTransportSigCompMsgInfo   *pSigCompMsgInfo = NULL;
    RvSipTransportRcvdMsgDetails  rcvdMsgDetails;
    RvChar                       *pPlainMsgBuf;
    RvBool                        bProcessMsg;
    RvSipTransportBsAction        eBSAction;
#ifdef RV_SIGCOMP_ON
    SipTransportSigCompMsgInfo    sigCompMsgInfo;
    pSigCompMsgInfo = &sigCompMsgInfo;
    /* Initialize the SigComp info */
    sigCompMsgInfo.bExpectComparmentID = RV_FALSE;
    sigCompMsgInfo.uniqueID			   = 0;
#endif /* RV_SIGCOMP_ON */

    /* Build message, if there is no injected message to process */
    if (NULL == hMsg)
    {
        /* Prepare buffer for parsing */
        pPlainMsgBuf = NULL;
        rv = SipTransportMsgBuilderUdpMakeMsg(
            (RvSipTransportMgrHandle)pTransportMgr,pBuffer, totalMsgSize,
            RV_TRUE/*bConstructMsg*/, NULL/*hConstructedMsg*/,pRecvFromAddr->eTransport,&hMsg,
            pSigCompMsgInfo, &pPlainMsgBuf);
        if (RV_OK != rv)
        {
            if (pPlainMsgBuf != NULL)
            {
                TransportMgrFreeRcvBuffer(pTransportMgr,pPlainMsgBuf);
            }
            RvLogError(pTransportMgr->pMBLogSrc ,(pTransportMgr->pMBLogSrc ,
                "TransportMsgBuilderParseUdpMsg - failed to build message from UDP buffer (rv=%d:%s)",
                rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        /* Free plain buffer. No longer needed. We have a message object now */
        if (pPlainMsgBuf != NULL)
        {
            TransportMgrFreeRcvBuffer(pTransportMgr, pPlainMsgBuf);
        }
    }

    eBSAction = RVSIP_TRANSPORT_BS_ACTION_CONTINUE_PROCESS;
    InformBadSyntax(pTransportMgr, hMsg, &eBSAction);

    /* Receive application permission application to process the message */

    /* IMPORTANT: The bProcessMsg is OVERWRITTEN due to the call to the */
    /* extended callback, which sets a default value in it or asks the  */
    /* application. */

    /* Prepare the received message details structure */
    memset(&rcvdMsgDetails,0,sizeof(rcvdMsgDetails));
    rcvdMsgDetails.eBSAction    = eBSAction;
    rcvdMsgDetails.hLocalAddr   = hLocalAddr;
    GetMsgCompressionType(pSigCompMsgInfo, &rcvdMsgDetails.eCompression);
    TransportMgrSipAddrGetDetails(pRecvFromAddr, &rcvdMsgDetails.recvFromAddr);

    bProcessMsg = RV_TRUE;
#ifdef RV_SIP_IMS_ON
    TransportCallbackMsgReceivedExtEv(pTransportMgr, hMsg,
        &rcvdMsgDetails, pRecvFromAddr,
        ((TransportMgrLocalAddr*)hLocalAddr)->pSecOwner, &bProcessMsg);
#else
    TransportCallbackMsgReceivedExtEv(pTransportMgr, hMsg,
        &rcvdMsgDetails, NULL/*pRecvFromAddr*/,NULL/*pSecOwner*/,&bProcessMsg);
#endif /*#ifdef RV_SIP_IMS_ON*/

    if (RV_TRUE == bProcessMsg)
    {
        /* pass the message to the transaction manager */
        TransportCallbackMsgRcvdEv(pTransportMgr, hMsg, NULL/*hConn*/,
            hLocalAddr, pRecvFromAddr, eBSAction, pSigCompMsgInfo);
    }
    else
    {
        RvLogInfo(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
            "TransportMsgBuilderParseUdpMsg - received message is processed by the application"));
    }

    /* When JSR32 is used it is the responsibility of the application
    to free the msg object*/
#ifndef RV_SIP_JSR32_SUPPORT
    RvSipMsgDestruct(hMsg);
#endif /*#ifndef RV_SIP_JSR32_SUPPORT*/

    return RV_OK;
}

/******************************************************************************
 * TransportMsgBuilderParseTcpMsg
 * ----------------------------------------------------------------------------
 * General: The TransportMsgBuilderParseTcpMsg builds Message object, using
 *          provided buffer, pass the built Message to the upper layers and
 *          frees the buffer.
 *          Main (Select) Thread fills buffer with data received from TCP/TLS
 *          connection and generates MESSAGE_RCVD event for TPQ (Transport
 *          Processing Queue). Transport Processing Threads handle this event
 *          and call TransportMsgBuilderParseTcpMsg.
 * Return Value: RvStatus - RV_OK or returned by subsequent calls error value
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - the Transport Manager object
 *          pBuffer       - the SIP message buffer (before parsing).
 *          totalMsgSize  - number of bytes in entire SIP message.
 *          pConn         - pointer to the connection.
 *          hAppConn      - application handle stored in connection.
 *          sipMsgSize    - SIP part length of the message (without SDP).
 *          sdpLength     - SDP part length of the message.
 *          hLocalAddr    - local address where the message was sent.
 *          pRecvFromAddr - address from where the message was sent.
 *          hInjectedMsg  - If there is an 'injected' msg, we should not try to
 *                          parse the UDP message, but take this message instead.
 *          pSigCompInfo  - Transport SigComp information related to the message.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportMsgBuilderParseTcpMsg(
                            IN TransportMgr*                 pTransportMgr,
                            IN RvChar*                       pBuffer,
                            IN RvInt32                       totalMsgSize,
                            IN TransportConnection*          pConn,
                            IN RvSipTransportConnectionAppHandle hAppConn,
                            IN RvInt32                       sipMsgSize,
                            IN RvUint32                      sdpLength,
                            IN RvSipTransportLocalAddrHandle hLocalAddr,
                            IN SipTransportAddress*          pRecvFromAddr,
                            IN RvSipMsgHandle                hInjectedMsg,
                            IN SipTransportSigCompMsgInfo*   pSigCompInfo)
{
    /* IMPORTANT!!!
       Assume that the Connection object is not locked.
       If you access the connection fields, lock connection Event Lock !
       Locking of persistent connection while parsing received buffer
       have bad impact on performance, since it locks Main (Select) Thread. */

    RvSipMsgHandle hMsg = NULL;
    RvStatus       rv   = RV_OK;

    /* Generate Message object, if there is no injected message to process */
    if(hInjectedMsg != NULL)
    {
        hMsg = hInjectedMsg;
    }
    else
    {
        rv = MsgBuilderConstructParseTcpMsg(pTransportMgr, pBuffer,
                                            totalMsgSize, sipMsgSize,
                                            sdpLength, &hMsg);
        if(rv != RV_OK)
        {
            RvLogError(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                "TransportMsgBuilderParseTcpMsg - conn 0x%p: Failed to construct and parse new message.",pConn));
            return rv;
        }
        else
        {
            RvLogInfo(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                "TransportMsgBuilderParseTcpMsg - conn: 0x%p: Message 0x%p was built successfully", pConn, hMsg));
        }
    }

    /* Expose generated message to upper layers */
    rv = ReportConnectionMsg(pTransportMgr, hMsg, pConn, hAppConn, hLocalAddr,
                             pRecvFromAddr, pSigCompInfo);
    if(RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportMsgBuilderParseTcpMsg - failed to report Msg %p received on Connection %p(rv=%d:%s)",
            hMsg, pConn, rv, SipCommonStatus2Str(rv)));
		RvSipMsgDestruct(hMsg);
        return rv;
    }
    return RV_OK;
}

/******************************************************************************
 * TransportMsgBuilderParseSctpMsg
 * ----------------------------------------------------------------------------
 * General: The TransportMsgBuilderParseSctpMsg builds Message object, using
 *          provided buffer, pass the built Message to the upper layers and
 *          frees the buffer.
 *          Main (Select) Thread fills buffer with data received from UDP
 *          socket and generates MESSAGE_RCVD event for TPQ (Transport
 *          Processing Queue). Transport Processing Threads handle this event
 *          and call TransportMsgBuilderParseSctpMsg.
 * Return Value: RvStatus - RV_OK or returned by subsequent calls error value
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - the Transport Manager object
 *          pConn         - pointer to the connection.
 *          hAppConn      - handle stored by the application in the connection.
 *          pBuffer       - the SIP message buffer (before parsing).
 *          totalMsgSize  - number of bytes in entire SIP message.
 *          hInjectedMsg  - If there is an 'injected' msg, we should not try to
 *                          parse the UDP message, but take this message instead.
 *          hLocalAddr    - local address where the message was sent.
 *          pRecvFromAddr - address from where the message was sent.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportMsgBuilderParseSctpMsg(
                            IN TransportMgr*                     pTransportMgr,
                            IN TransportConnection*              pConn,
                            IN RvSipTransportConnectionAppHandle hAppConn,
                            IN RvChar*                           pBuffer,
                            IN RvInt32                           totalMsgSize,
                            IN RvSipMsgHandle                    hInjectedMsg,
                            IN RvSipTransportLocalAddrHandle     hLocalAddr,
                            IN SipTransportAddress*              pRecvFromAddr)
{
    /* IMPORTANT!!!
       Assume that the Connection object is not locked.
       If you access the connection fields, lock connection Event Lock !
       Locking of persistent connection while parsing received buffer
       have bad impact on performance, since it locks Main (Select) Thread. */

    RvStatus                      rv;
    RvSipMsgHandle                hMsg = NULL;
    SipTransportSigCompMsgInfo   *pSigCompInfo = NULL;
    RvChar                       *pPlainMsgBuf;
#ifdef RV_SIGCOMP_ON
    SipTransportSigCompMsgInfo    sigCompMsgInfo;
    pSigCompInfo = &sigCompMsgInfo;
    /* Initialize the SigComp info */
    sigCompMsgInfo.bExpectComparmentID = RV_FALSE;
    sigCompMsgInfo.uniqueID			   = 0;
#endif /* RV_SIGCOMP_ON */

    /* Generate Message object, if there is no injected message to process */
    if(hInjectedMsg != NULL)
    {
        hMsg = hInjectedMsg;
    }
    else
    {
        /* Prepare buffer for parsing */
        pPlainMsgBuf = NULL;
        rv = SipTransportMsgBuilderUdpMakeMsg(
            (RvSipTransportMgrHandle)pTransportMgr, pBuffer, totalMsgSize,
            RV_TRUE/*bConstructMsg*/, NULL/*hConstructedMsg*/, pRecvFromAddr->eTransport,&hMsg,
            pSigCompInfo, &pPlainMsgBuf);
        if (RV_OK != rv)
        {
            if (pPlainMsgBuf != NULL)
            {
                TransportMgrFreeRcvBuffer(pTransportMgr, pPlainMsgBuf);
            }
            RvLogError(pTransportMgr->pMBLogSrc ,(pTransportMgr->pMBLogSrc ,
                "TransportMsgBuilderParseSctpMsg - failed to build message from SCTP buffer (rv=%d:%s)",
                rv, SipCommonStatus2Str(rv)));
            return rv;
        }
        /* Free plain buffer. No longer needed. We have a message object now */
        if (pPlainMsgBuf != NULL)
        {
            TransportMgrFreeRcvBuffer(pTransportMgr, pPlainMsgBuf);
        }
    }

    /* Expose generated message to upper layers */
    rv = ReportConnectionMsg(pTransportMgr, hMsg, pConn, hAppConn, hLocalAddr,
                             pRecvFromAddr, pSigCompInfo);
    if(RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportMsgBuilderParseTcpMsg - failed to report Msg %p received on Connection %p(rv=%d:%s)",
            hMsg, pConn, rv, SipCommonStatus2Str(rv)));
		RvSipMsgDestruct(hMsg);
        return rv;
    }
    return RV_OK;
}

/******************************************************************************
 * TransportMsgBuilderAccumulateMsg
 * ----------------------------------------------------------------------------
 * General:   The TransportMsgBuilderAccumulateMsg goes through the buffer
 *            that stores data, received on bit-stream connection, and tries
 *            to recognize SIP messages in it (if more then 1).
 *            This function calls the prepare function to remove crlfs,
 *            and calls the set function which sets the body.
 *            The prepare function returns number of bytes actually was read
 *            from the buffer.
 *            Each time when single SIP message boundaries are discovered,
 *            MESSAGE_RECEIVED event is created and sent to the TPQ.
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pBuf        - the buffer that was received from the network
 *          bufSize     - the buffer size.
 *          conn           - the connection
 *          pLocalAddr  - The local address to which the message was received
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV TransportMsgBuilderAccumulateMsg(
                            IN RvChar                       *pBuf,
                            IN RvUint32                      bufSize,
                            IN TransportConnection          *conn,
                            IN RvSipTransportLocalAddrHandle hLocalAddr)
{
    RvStatus                    rv;
    RvBool                      restoreFromPreviousePoint = RV_TRUE;
    MsgBuilderStoredConnInfo    connInfo;
    SipTransportSigCompMsgInfo* pSigCompMsgInfo           = NULL;
#ifdef RV_SIGCOMP_ON
    /*  SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    SipTransportSigCompMsgInfo  sigCompMsgInfo={0,0};
#else
    SipTransportSigCompMsgInfo  sigCompMsgInfo;
#endif
/*  SPIRENT_END */

    pSigCompMsgInfo = &sigCompMsgInfo;
#endif /* RV_SIGCOMP_ON */

    memset(&connInfo,0,sizeof(connInfo));
    if ((pBuf == NULL) && (conn->pBuf != NULL))
    {
        pBuf                      = conn->pBuf;
        bufSize                   = conn->bufSize;
        restoreFromPreviousePoint = RV_TRUE;
        RestoreConnInfo(conn,&connInfo);
        connInfo.pPlainBuf        = conn->pBuf;
        connInfo.plainBufSize     = conn->bufSize;
#ifdef RV_SIGCOMP_ON
        if (connInfo.pDecompressedBuf != NULL)
        {
            connInfo.pPlainBuf    = connInfo.pDecompressedBuf;
            connInfo.plainBufSize = connInfo.decompressedBufSize;
        }
#endif /* RV_SIGCOMP_ON */
        conn->pBuf    = NULL;
        conn->bufSize = 0;
    }
    else
    {
        restoreFromPreviousePoint = RV_FALSE;
        connInfo.msgComplete      = RV_FALSE;
        connInfo.originalBuffer   = pBuf;
        connInfo.hLocalAddr       = hLocalAddr;
    }

    if (pBuf == NULL)
    {
        return RV_OK;
    }

    do {
        if (restoreFromPreviousePoint != RV_TRUE)
        {
            rv = PrepareTcpCompleteMsgBuffer(conn,
                                             pBuf,
                                             bufSize,
                                             pSigCompMsgInfo,
                                             &connInfo);
            /* It's important to check the boolean connInfo.msgComplete     */
            /* because there might be a possibility that a complete SigComp */
            /* message was received and failed to be decompressed but rest  */
            /* of the buffer is general SIP message (decompressed)          */
            if (rv != RV_OK && connInfo.msgComplete != RV_TRUE)
            {
                RvLogError(conn->pTransportMgr->pMBLogSrc,(conn->pTransportMgr->pMBLogSrc,
                        "TransportMsgBuilderAccumulateMsg - Failed to prepare complete message buf for conn %0x",conn));
                return rv;
            }
            if (connInfo.msgComplete != RV_TRUE)
            {
                return RV_OK;
            }
        }
        restoreFromPreviousePoint = RV_FALSE;
        /*check if got content-length */
        if (conn->recvInfo.contentLength == UNDEFINED)
        {
            /* if we got here, the buffer does not contain content length */
            /* This might happen when a SigComp Decompression fails or    */
            /* when a SIP complete message doesn't contain content-length  */
            RvLogWarning(conn->pTransportMgr->pMBLogSrc,(conn->pTransportMgr->pMBLogSrc,
                        "TransportMsgBuilderAccumulateMsg - Received TCP message that wasn't handled correctly"));
        }
        else
        {
/* SPIRENT_BEGIN */
/*
* COMMENTS:
* Modified by Armyakov
* Counting TCP incoming messages 
*/
#if defined(UPDATED_BY_SPIRENT)
            RvSipRawMessageCounterHandlersRun(SPIRENT_SIP_RAW_MESSAGE_INCOMING);
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
            /* Entire SIP message was accumulated */
            rv = HandleTcpCompleteMsgBuffer(conn,pBuf,bufSize,pSigCompMsgInfo,&connInfo);
            if (RV_OK != rv)
            {
                 RvLogError(conn->pTransportMgr->pMBLogSrc,(conn->pTransportMgr->pMBLogSrc,
                        "TransportMsgBuilderAccumulateMsg - Failed to handle complete message buf for conn %0x",conn));
                 return rv;
            }
        }

        if (connInfo.bytesReadFromBuf > bufSize)
        {
            RvLogExcep(conn->pTransportMgr->pMBLogSrc,(conn->pTransportMgr->pMBLogSrc,
                       "TransportMsgBuilderAccumulateMsg - Read more bytes (%d) than received buffer size (%d) for conn %0x",
                       conn,connInfo.bytesReadFromBuf,bufSize));
            bufSize = 0;
        }
        else
        {
            bufSize -= connInfo.bytesReadFromBuf;
        }


        /*update the pointer to point to the byte after the last message processed*/
        pBuf = pBuf + connInfo.bytesReadFromBuf;

        /*reinitialize parameters*/
        conn->recvInfo.bReadingBody   = RV_FALSE;
        conn->recvInfo.bodyStartIndex = 0;
        conn->recvInfo.contentLength  = UNDEFINED;
        /*free Tcp resources*/
        FreeMsgBuilderTcpResources(conn,&(connInfo.pDecompressedBuf),NULL);

        conn->recvInfo.hMsgReceived     = NULL_PAGE;
        conn->recvInfo.lastHeaderOffset = 0;
        conn->recvInfo.msgSize          = 0;
        conn->recvInfo.offsetInBody     = 0;
    }
    /*if there are bytes left it's means we have another Sip msg within
      this buffer*/
    while (bufSize > 0); /*end of do*/

    if (conn->originalBuffer != NULL)
    {
        TransportMgrFreeRcvBuffer(conn->pTransportMgr,conn->originalBuffer);
        InitConnInfo(conn);
        RvLogDebug(conn->pTransportMgr->pMBLogSrc,(conn->pTransportMgr->pMBLogSrc,
            "TransportMsgBuilderAccumulateMsg - connection 0x%p(%d) recovered from OOR",conn,conn->ccSocket));
    }
    else
    {
        RvLogDebug(conn->pTransportMgr->pMBLogSrc,(conn->pTransportMgr->pMBLogSrc,
            "TransportMsgBuilderAccumulateMsg - connection 0x%p(%d) succeed (no OOR)",conn,conn->ccSocket));
    }

    return RV_OK;
}


/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS                               */
/*-----------------------------------------------------------------------*/

/************************************************************************************
* CheckPrevPacketForEndOfMsg
* ----------------------------------------------------------------------------------
* General:    if the there was a previous part of the message came before. there is a need
* to check if the last part had , at the end of it an indication for ending the header part.
* e.g.: 1.last buffer   - XXX CR LF CR
*            current buf - LF|start of body.
*       2.last buffer   - XXX CR LF
*           current buf - CR LF|start of body.
*    in those cases we need to update the indexes.
*    1. the index that represent the start of the body.
*    2. the index that represent the last header in the message so we can search
*        for Content-Length if wasn't found yet.
*
*    Return Value: RV_Bool -
* ----------------------------------------------------------------------------------
* Arguments:
* Input:   pBuf        - the buffer that was received from the network
*          conn           - the connection
* Output:  sipMsgEndIndex    - index of the end of last header (that arrived in the
*                               previous tcp MSG.
*            sipMsgStartBody  - index to the start of the body part.
***********************************************************************************/
static RvBool RVCALLCONV CheckPrevPacketForEndOfMsg(IN  RvChar             *pBuf,
                                                     IN  TransportConnection *conn,
                                                     OUT RvInt32           *sipMsgEndIndex,
                                                     OUT RvInt32           *sipMsgStartBody)
{    /*the last 3 chars of the last saved buffer*/
    RvChar *last3Bytes_1; /* ........X--*/
    RvChar *last3Bytes_2; /* ........-X-*/
    RvChar *last3Bytes_3; /* ........--X*/

    RvInt    bytesToCheck;
    if (conn->recvInfo.msgSize<3)
        bytesToCheck = conn->recvInfo.msgSize;
    else
        bytesToCheck =3;


    last3Bytes_1 = ((RvChar*)RPOOL_GetPtr(conn->pTransportMgr->hMsgPool,
                                                conn->recvInfo.hMsgReceived,
                                                conn->recvInfo.msgSize-bytesToCheck));

    last3Bytes_2 = ((RvChar*)RPOOL_GetPtr(conn->pTransportMgr->hMsgPool,
                                            conn->recvInfo.hMsgReceived,
                                            (conn->recvInfo.msgSize-bytesToCheck)+1));

    last3Bytes_3 = ((RvChar*)RPOOL_GetPtr(conn->pTransportMgr->hMsgPool,
                                            conn->recvInfo.hMsgReceived,
                                            (conn->recvInfo.msgSize-bytesToCheck)+2));

    if(last3Bytes_1==NULL || last3Bytes_2==NULL || last3Bytes_3==NULL)
    {
        return RV_FALSE;
    }
    /* handling the case CR LF CR | LF */
    if (*last3Bytes_1==SIP_MSG_BUILDER_CR &&
        *last3Bytes_2==SIP_MSG_BUILDER_LF &&
        *last3Bytes_3==SIP_MSG_BUILDER_CR)
    {
        if ((*pBuf) == SIP_MSG_BUILDER_LF)
        {
            *sipMsgEndIndex=-3;
            *sipMsgStartBody=1;
            return RV_TRUE;
        }

    }
    /* handling the case CR LF | CR LF */
    else if (*last3Bytes_2==SIP_MSG_BUILDER_CR &&
             *last3Bytes_3==SIP_MSG_BUILDER_LF )
    {
        if ((*pBuf) == SIP_MSG_BUILDER_CR &&
            (*(pBuf+1)) == SIP_MSG_BUILDER_LF)
        {
            *sipMsgEndIndex=-2;
            *sipMsgStartBody=2;
            return RV_TRUE;
        }

    }

    else if (*last3Bytes_3==SIP_MSG_BUILDER_CR)
    {
        /* handling the case CR | CR */
        if ((*pBuf) == SIP_MSG_BUILDER_CR)
        {
            *sipMsgEndIndex=-1;
            *sipMsgStartBody=1;
            return RV_TRUE;

        }
        /* handling the case CR | LF CR LF */
        if ((*pBuf) == SIP_MSG_BUILDER_LF &&
            (*(pBuf+1)) == SIP_MSG_BUILDER_CR &&
            (*(pBuf+2)) == SIP_MSG_BUILDER_LF)
        {
            *sipMsgEndIndex=-1;
            *sipMsgStartBody=3;
            return RV_TRUE;

        }
    }
    /* handling the case LF | LF */
    else if (*last3Bytes_3==SIP_MSG_BUILDER_LF)
    {
        if ((*pBuf) == SIP_MSG_BUILDER_LF)
        {
            *sipMsgEndIndex=-1;
            *sipMsgStartBody=1;
            return RV_TRUE;

        }
    }

    return RV_FALSE;
}



/************************************************************************************
 * CheckPrevPacketForEndOfHeader
 * ----------------------------------------------------------------------------------
 * General:    if the there was a previous part of the message came before. there is a need
 * to check if the last part had , at the end of it an indication for ending of header.
 * e.g.: 1.last buffer   - XXX CR
 *            current buf - start new Header.
 *       2.last buffer   - XXX CR LF
 *           current buf - start new Header.
 *       3.last buffer   - XXX CR
 *            current buf - LF|start new Header.
 *
 *     if the answer is true then we need to return TRUe to the calling function to
 *      make her update indexes and search in this header for Content-Length if needed.
 *
 * Return Value: RV_Bool -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pBuf        - the buffer that was received from the network
 *          conn           - the connection
 * Output:  none
 ***********************************************************************************/
static RvBool CheckPrevPacketForEndOfHeader(IN RvChar             *pBuf,
                                              IN TransportConnection *conn)
{
    RvChar* last2Bytes_1; /* ........X-*/
    RvChar* last2Bytes_2; /* ........-X*/
    RvInt    bytesToCheck;

    if (conn->recvInfo.msgSize<2)
    {
        bytesToCheck = conn->recvInfo.msgSize;
    }
    else
    {
        bytesToCheck =2;
    }

    last2Bytes_1 = ((RvChar*)RPOOL_GetPtr(conn->pTransportMgr->hMsgPool,
                                                conn->recvInfo.hMsgReceived,
                                                conn->recvInfo.msgSize-bytesToCheck));

    last2Bytes_2 = ((RvChar*)RPOOL_GetPtr(conn->pTransportMgr->hMsgPool,
                                            conn->recvInfo.hMsgReceived,
                                            (conn->recvInfo.msgSize-bytesToCheck)+1));

    if(last2Bytes_1==NULL || last2Bytes_2==NULL)
    {
        return RV_FALSE;
    }
     /* handling the case CR LF|new header*/
    if (*last2Bytes_1==SIP_MSG_BUILDER_CR &&
        *last2Bytes_2==SIP_MSG_BUILDER_LF)

    {
        if (((*pBuf) != SIP_MSG_BUILDER_B0) ||
            ((*pBuf) != SIP_MSG_BUILDER_HT))
        {
            return RV_TRUE;

        }
        else
        {    /*set spaces*/
            *last2Bytes_1= SIP_MSG_BUILDER_SP;
            *last2Bytes_2 = SIP_MSG_BUILDER_SP;
        }
    }
    /* handling the case CR|new header*/
    else if (*last2Bytes_2==SIP_MSG_BUILDER_CR)
    {

        if (((*pBuf) != SIP_MSG_BUILDER_B0) ||
            ((*pBuf) != SIP_MSG_BUILDER_HT))
        {
            return RV_TRUE;

        }
        else
        {    /*set spaces*/
            *last2Bytes_2 = SIP_MSG_BUILDER_SP;
        }
    }
    /* handling the case  LF|new header*/
    else if (*last2Bytes_2==SIP_MSG_BUILDER_LF)
    {
        if (((*pBuf) != SIP_MSG_BUILDER_B0) ||
            ((*pBuf) != SIP_MSG_BUILDER_HT))
        {
            return RV_TRUE;
        }
        else
        {    /*set spaces*/
            *last2Bytes_2 = SIP_MSG_BUILDER_SP;
        }
    }

    return RV_FALSE;
}


/************************************************************************************
 * AppendBodyToMsg
 * ----------------------------------------------------------------------------------
 * General:    append body data to page
 *
 * Return Value: RV_Bool -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:    *conn,          - the connection
 *            *pBuffer       - buffer from the transport
 *            bufSize           - buffer size
 * Output:    msgComplete    - boolean indicating if we finished reading the body
 *            *bytesReadFromBuf   - how many byte was read from the buffer
 ***********************************************************************************/
static RvStatus AppendBodyToMsg(IN TransportConnection     *conn,
                               IN RvChar        *pBuffer,
                               IN RvInt32      bufSize,
                               OUT RvBool*        msgComplete,
                               OUT RvUint32    *bytesReadFromBuf)
{
    RvInt32      offset;       /* used as a parameter to the RPOOL_AppendFromExternal func*/
    RvInt32      bytesLeftToReadInBody; /*the size of the body we still needs to get*/
    RvStatus       status;

    /*just continue to read the body */
    bytesLeftToReadInBody = conn->recvInfo.contentLength - conn->recvInfo.offsetInBody;
    /*check if the message that just arrived is the rest of the previous msg.*/
    if (bufSize < bytesLeftToReadInBody)
    {
        status = RPOOL_AppendFromExternal(conn->pTransportMgr->hMsgPool,
                                          conn->recvInfo.hMsgReceived,
                                          (void *) pBuffer,
                                          bufSize,
                                          RV_FALSE,
                                          &offset,
                                          NULL);
        if(status != RV_OK)
        {
            return status;
        }
        *bytesReadFromBuf = bufSize;

        conn->recvInfo.offsetInBody+=bufSize;
        /* there are more bytes to read from the body that will probably will arrive
            in the future. the whole buffer is part of the body.
            we return an error since there are more bytes to come that are
            part of the body*/
        *msgComplete = RV_FALSE;
        return RV_OK;
    }
    else if(bufSize == bytesLeftToReadInBody)
    {
        /*just append the current msg to the page since we are in the middle
        of the body and after that msg we' will finish it*/

        status = RPOOL_AppendFromExternal(conn->pTransportMgr->hMsgPool,
                                conn->recvInfo.hMsgReceived,
                                (void *) pBuffer,
                                bufSize,
                                RV_FALSE,
                                &offset,
                                NULL);
        if(status != RV_OK)
        {
            return status;
        }
        *bytesReadFromBuf = bufSize;
        /*update the index of the current position in the body.
          the whole buffer is part of the body*/
        conn->recvInfo.offsetInBody+=bufSize;
        *msgComplete = RV_TRUE;
        return RV_OK;
    }
    else
    {
        /*the current msg contains:
        1: the last part of previous received tcp buffers.
        2: a new message that arrived within the same current tcp buffer*/
        status = RPOOL_AppendFromExternal(conn->pTransportMgr->hMsgPool,
                                conn->recvInfo.hMsgReceived,
                                (void *)pBuffer,
                                bytesLeftToReadInBody,
                                RV_FALSE,
                                &offset,
                                NULL);
        if(status != RV_OK)
        {
            return status;
        }
        *bytesReadFromBuf = bytesLeftToReadInBody;
        /*update the position in the body we have already read*/
        conn->recvInfo.offsetInBody+=bytesLeftToReadInBody;
        *msgComplete = RV_TRUE;
        return RV_OK;
    }
}

/************************************************************************************
 * CopyBodyToMsg
 * ----------------------------------------------------------------------------------
 * General:    append msg data to page
 *
 * Return Value: RV_Bool -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:    *conn,          - the connection
 *            *pBuffer       - buffer from the transport
 *            bufSize           - buffer size
 *           *sipMsgStartIndex - from where to do the append
 *           sipMsgStartBody   - index in pBuffer of start of body
 *             bytesLeftInBuffer  - how many byte left to read in the buffer
 * Output:    msgComplete    - boolean indicating if we finished reading the body
 *            *bytesReadFromBuf   - how many byte was read from the buffer
 ***********************************************************************************/
static RvStatus CopyBodyToMsg(IN TransportConnection    *conn,
                               IN RvChar                *pBuffer,
                               IN RvInt32              bufSize,
                               IN RvUint32             *sipMsgStartIndex,
                               IN RvUint32             sipMsgStartBody,
                               IN RvInt32                bytesLeftInBuffer,
                               OUT RvBool*                msgComplete,
                               OUT RvUint32            *bytesReadFromBuf)
{
    RvInt32      offset;       /* used as a parameter to the RPOOL_AppendFromExternal func*/
    RvStatus       status;

    /*try to find the body length*/
    if (bytesLeftInBuffer > conn->recvInfo.contentLength)
    {

        conn->recvInfo.offsetInBody = conn->recvInfo.contentLength;
        /*how many bytes we have already processed - in fact, includes the CRLFs
        in the beginning of the msg - if were*/
        *bytesReadFromBuf = sipMsgStartBody + conn->recvInfo.contentLength;

        if (conn->recvInfo.hMsgReceived != NULL_PAGE)
        {
            /*append to Page*/
            status = RPOOL_AppendFromExternal(conn->pTransportMgr->hMsgPool,
                                              conn->recvInfo.hMsgReceived,
                                              (void *)(pBuffer+*sipMsgStartIndex),
                                              sipMsgStartBody+conn->recvInfo.contentLength-*sipMsgStartIndex,
                                              RV_FALSE,
                                              &offset,
                                              NULL);

            if (status != RV_OK)
            {
                RvLogError(conn->pTransportMgr->pMBLogSrc,(conn->pTransportMgr->pMBLogSrc,
                           "CopyBodyToMsg - couldn't Append msg."));

                RPOOL_FreePage(conn->pTransportMgr->hMsgPool,
                    (conn->recvInfo.hMsgReceived));

                conn->recvInfo.hMsgReceived=NULL_PAGE;
                return status;
            }
        }
        /*go to parse the msg*/

        RvLogInfo(conn->pTransportMgr->pMBLogSrc,(conn->pTransportMgr->pMBLogSrc,
                 "CopyBodyToMsg - Message preparing was done successfully"));
        *msgComplete = RV_TRUE;
        return RV_OK;
    }
    else if(bytesLeftInBuffer  <= conn->recvInfo.contentLength)
    {
        conn->recvInfo.offsetInBody = bufSize-sipMsgStartBody;
        /*how many bytes we have already processed - in fact, includes the CRLFs
        in the beginning of the msg - if were*/
        *bytesReadFromBuf = bufSize;
        /* checking if the the body read so far equals to content length */
        if ((conn->recvInfo.offsetInBody != conn->recvInfo.contentLength) ||
            (conn->recvInfo.hMsgReceived!=NULL_PAGE))
        {
            if (conn->recvInfo.hMsgReceived == NULL_PAGE)
            {
                status = RPOOL_GetPage(conn->pTransportMgr->hMsgPool,
                    0,
                    &(conn->recvInfo.hMsgReceived));

                if (status!=RV_OK)
                {
                    RvLogError(conn->pTransportMgr->pMBLogSrc,(conn->pTransportMgr->pMBLogSrc,
                               "CopyBodyToMsg - couldn't allocate buffer for incoming parted tcp msg."));

                    return status;
                }
            }
            /*append to Page*/
            status = RPOOL_AppendFromExternal(conn->pTransportMgr->hMsgPool,
                                              conn->recvInfo.hMsgReceived,
                                              (void *)(pBuffer+*sipMsgStartIndex),
                                              sipMsgStartBody+conn->recvInfo.offsetInBody-*sipMsgStartIndex,
                                              RV_FALSE,
                                              &offset,
                                              NULL);/* if we got here, the buffer does not contain empty line */

            if (status != RV_OK)
            {
                RvLogError(conn->pTransportMgr->pMBLogSrc,(conn->pTransportMgr->pMBLogSrc,
                           "CopyBodyToMsg - couldn't Append msg."));

                RPOOL_FreePage(conn->pTransportMgr->hMsgPool,
                              (conn->recvInfo.hMsgReceived));

                conn->recvInfo.hMsgReceived=NULL_PAGE;
                return status;
            }

            if(bytesLeftInBuffer  < conn->recvInfo.contentLength)
            {
                /*still not the end of the sip message*/
                *msgComplete = RV_FALSE;
                return RV_OK;
            }
            else
            {
                /*go to parse*/
                *msgComplete = RV_TRUE;
                return RV_OK;
            }
        }
        *msgComplete = RV_TRUE;
        return RV_OK;
    }
    return RV_OK;
}

/************************************************************************************
 * MsgBuilderConstructParseTcpMsg
 * ----------------------------------------------------------------------------------
 * General: Construct and parse message received on TCP connection.
 * Return Value: RvStatus - RV_OK or returned by subsequent calls error value
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - Transport Manager object
 *          pBuffer       - the SIP message buffer (before parsing)
 *          totalMsgSize  - number of bytes in entire SIP message
 *          sipMsgSize    - SIP part length of the message (without SDP).
 *          sdpLength     - SDP part length of the message
 * Output:  phMsg         - constructed message.
 ***********************************************************************************/
static RvStatus RVCALLCONV MsgBuilderConstructParseTcpMsg(
                                    IN  TransportMgr*   pTransportMgr,
                                    IN  RvChar*         pBuffer,
                                    IN  RvInt32         totalMsgSize,
                                    IN  RvInt32         sipMsgSize,
                                    IN  RvUint32        sdpLength,
                                    OUT RvSipMsgHandle* phMsg)
{
    RvUint32 startBodyIndex;
    RvStatus status;
    RvChar   tmpChar=0;
    RvChar*  pSDP;
#ifndef RV_SIP_PRIMITIVES
#else
    RvInt32  contentLength;
#endif /* RV_SIP_PRIMITIVES */

/*  SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    RvChar*       SPIRENT_buffer = NULL;
    int           SPIRENT_size   = 0;
#endif
/*  SPIRENT_END */

/*  SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
    startBodyIndex=0;
#endif
/*  SPIRENT_END */

    /* constructing the message */

    status = RvSipMsgConstruct(pTransportMgr->hMsgMgr,
                               pTransportMgr->hMsgPool, phMsg);
    if (status != RV_OK)
    {
        RvLogError(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
            "MsgBuilderConstructParseTcpMsg - Message construction failed"));
        return status;
    }

    /*parse the sip message into the message object*/
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT_ABACUS)

    SPIRENT_buffer      = HSTPOOL_Alloc ( &SPIRENT_size );
    if ( SPIRENT_buffer && totalMsgSize > SPIRENT_size )
    {
       HSTPOOL_Release ( SPIRENT_buffer );
       SPIRENT_buffer       = NULL;
    }
    RvLogDebug(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
        "MsgBuilderConstructParseTcpMsg - Message %d, new SPIRENT_buffer=%p, size=%d",
        *phMsg, SPIRENT_buffer, SPIRENT_size));


    if ( SPIRENT_buffer )
    {
       typedef int (*SIPC_Filter_Buffer_FUNCPTR) ( char*, int, int );
       extern SIPC_Filter_Buffer_FUNCPTR g_SIPC_Filter_Buffer;

       int newSipMsgSize = sipMsgSize;    // will not change if no g_SIPC_Filter_Buffer set
       int oldBodySize = totalMsgSize - sipMsgSize;
       int extraSize = totalMsgSize - sipMsgSize - sdpLength;
       int newBodySize = totalMsgSize - sipMsgSize;    // will not change if no g_SIPC_Filter_Buffer set

       // I don't see more efficient way here
       // Copy sip part
       memcpy ( SPIRENT_buffer, pBuffer, sipMsgSize + 1 );      
       if ( g_SIPC_Filter_Buffer )
       {
           newSipMsgSize = g_SIPC_Filter_Buffer ( SPIRENT_buffer, sipMsgSize + 1 , SPIRENT_size - oldBodySize ) - 1;
           RvLogDebug(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                      "MsgBuilderConstructParseTcpMsg - use g_SIPC_Filter_Buffer on Sip-headers: oldSipMsgSize=%d, newSipMsgSize=%d in message=%d", 
                      sipMsgSize, newSipMsgSize, *phMsg));
       }

       
       // copy sdp part
       memcpy ( &(SPIRENT_buffer[newSipMsgSize]), &(pBuffer[sipMsgSize]), oldBodySize );
       if ( g_SIPC_Filter_Buffer )
       {
           newBodySize = g_SIPC_Filter_Buffer ( &(SPIRENT_buffer[newSipMsgSize]), oldBodySize, SPIRENT_size - newSipMsgSize );
           RvLogDebug(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                      "MsgBuilderConstructParseTcpMsg - use g_SIPC_Filter_Buffer on Sip-body: oldSipBodySize=%d, newSipBodySize=%d in message=%d", 
                      oldBodySize, newBodySize, *phMsg));
       }

       // modify parameters to represent SPIRENT_buffer instead of original pBuffer
       totalMsgSize = newSipMsgSize + newBodySize;
       sipMsgSize = newSipMsgSize;
       pBuffer = SPIRENT_buffer;
    }

#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

    tmpChar = pBuffer[sipMsgSize+1];
    pBuffer[sipMsgSize+1] = SIP_MSG_BUILDER_B0;  /*set null at the end of the sip message*/
    status = SipMsgParse(*phMsg, pBuffer, sipMsgSize+1);
    pBuffer[sipMsgSize+1] = tmpChar;             /*restore what was there before*/

    if (status != RV_OK)
    {
        RvLogError(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
            "MsgBuilderConstructParseTcpMsg - Message %d failed to be parsed",*phMsg));
        RvSipMsgDestruct(*phMsg);
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
        if ( SPIRENT_buffer )  {
            HSTPOOL_Release ( SPIRENT_buffer );
            SPIRENT_buffer = NULL;
        }
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
        return status;
    }

    /* getting pointer to the start of the SDP */
    pSDP = TransportMsgBuilderGetSDPStart(pBuffer, sipMsgSize,&startBodyIndex);

    /* setting the SDP body to the message if it is not zero size */
    if(sdpLength > 0)
    {
#ifndef RV_SIP_PRIMITIVES
        RvSipBodyHandle hBody;

        hBody = RvSipMsgGetBodyObject(*phMsg);
        if (NULL == hBody)
        {
            status = RvSipBodyConstructInMsg(*phMsg, &hBody);
            if (RV_OK != status)
            {
                RvSipMsgDestruct(*phMsg);
                RvLogInfo(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                          "MsgBuilderConstructParseTcpMsg - Failed to set body in message"));
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
                if ( SPIRENT_buffer ) {
                    HSTPOOL_Release ( SPIRENT_buffer );
                    SPIRENT_buffer = NULL;
                }
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
                return status;
            }
        }
        status = RvSipBodySetBodyStr(hBody, pSDP, totalMsgSize - startBodyIndex);
        if (RV_OK != status)
        {
            RvSipMsgDestruct(*phMsg);
            RvLogInfo(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                "MsgBuilderConstructParseTcpMsg - Failed to set body in message"));
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
            if ( SPIRENT_buffer ) { 
                HSTPOOL_Release ( SPIRENT_buffer );
                SPIRENT_buffer = NULL;
            }
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */
            return status;
        }
#else
        status = RvSipMsgSetBody(*phMsg, pSDP);
        if (status != RV_OK)
        {
            RvLogInfo(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                       "MsgBuilderConstructParseTcpMsg - Failed to set body in message"));
/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
            if ( SPIRENT_buffer ) {
                HSTPOOL_Release ( SPIRENT_buffer );
                SPIRENT_buffer = NULL;
            }
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

            return status;
        }
        /*since the above function updates the content length, if we did not get
          a content length in the message, update it back to undefined*/
        contentLength = RvSipMsgGetContentLengthHeader(*phMsg);
        if(contentLength == UNDEFINED)
        {
            RvSipMsgSetContentLengthHeader(*phMsg,UNDEFINED);
        }
        RV_UNUSED_ARG(totalMsgSize);
#endif
    }

/* SPIRENT_BEGIN */
/*
 * COMMENTS: 
 */
#if defined(UPDATED_BY_SPIRENT_ABACUS)
    if(SPIRENT_buffer)
    {
        HSTPOOL_SetBlockSize ( SPIRENT_buffer, totalMsgSize );
        status = RvSipMsgSetSPIRENTBody(*phMsg,SPIRENT_buffer);
        if(status != RV_OK)
        {
            RvLogError(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                       "MsgBuilderConstructParseTcpMsg - Failed to set SPIRENT body in message=%d", *phMsg));
            HSTPOOL_Release ( SPIRENT_buffer );
            SPIRENT_buffer = NULL;
        }
        else
        {
            RvLogDebug(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                       "MsgBuilderConstructParseTcpMsg - set SPIRENTbody=%p, totalsize=%d in message=%d", 
                       SPIRENT_buffer, totalMsgSize, *phMsg));
        }
    }
#endif /* defined(UPDATED_BY_SPIRENT) */ 
/* SPIRENT_END */

    return RV_OK;
}


/******************************************************************************
 * InformBadSyntax
 * ----------------------------------------------------------------------------
 * General:    Calls to the bad-syntax callbacks (start-line and headers)
 *
 * Return Value: RV_FALSE, if bad syntax was found, RV_TRUE otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - The transport info structure.
 *          hMsg            - The received message.
 * Output:  eAction         - Action chosen by application for further
 *                            processing of this message.
 *****************************************************************************/
static RvBool InformBadSyntax(  IN  TransportMgr           *pTransportMgr,
                                IN  RvSipMsgHandle          hMsg,
                                OUT RvSipTransportBsAction *eAction)
{
    RvSipHeaderListElemHandle hElem;
    RvSipHeaderType eHeaderType;
    RvBool bBadSyntaxWasFound = RV_FALSE;

    /* 1. check bad-syntax in start line */
    if(SipMsgGetBadSyntaxStartLine(hMsg) > UNDEFINED)
    {
        bBadSyntaxWasFound = RV_TRUE;
        /* only if we found bad-syntax, the default behavior is discard the message */
        *eAction = RVSIP_TRANSPORT_BS_ACTION_DISCARD_MSG;

        TransportCallbackBadSyntaxStartLineMsgEv(pTransportMgr, hMsg,eAction);
        if(*eAction != RVSIP_TRANSPORT_BS_ACTION_CONTINUE_PROCESS)
        {
            /* application decided to reject/discard message. no need to continue */
            return bBadSyntaxWasFound;
        }
    }
    /* 2. check bad-syntax headers */
    if(RvSipMsgGetBadSyntaxHeader(hMsg, RVSIP_FIRST_HEADER, &hElem, &eHeaderType) != NULL ||
       SipMsgIsBadSyntaxContentLength(hMsg) == RV_TRUE)
    {
        bBadSyntaxWasFound = RV_TRUE;
        /* only if we found bad-syntax, the default behavior is discard the message */
        *eAction = RVSIP_TRANSPORT_BS_ACTION_DISCARD_MSG;

        TransportCallbackBadSyntaxMsgEv(pTransportMgr, hMsg, eAction);

        if(RvSipMsgGetMsgType(hMsg) == RVSIP_MSG_RESPONSE &&
           RVSIP_TRANSPORT_BS_ACTION_REJECT_MSG == *eAction)
        {
            /* reject option on response message should treat as discard option */
            *eAction = RVSIP_TRANSPORT_BS_ACTION_DISCARD_MSG;
        }
    }
    return bBadSyntaxWasFound;
}

/************************************************************************************
 * IsFirstOccurrence
 * ----------------------------------------------------------------------------------
 * General:    Acts just like strstr() only comparison is done regardless to case
 *          both arguments to function MUST be NULL terminated
 *
 * Return Value: NULL - no matching string was found
 *               a pointer to a string - the first occurrence of a the substring
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pszFindInMe - a string that might contain the substring.
 *          psztoFind - the substring to find.
 * Output:    (-)
 ***********************************************************************************/
static RvBool IsFirstOccurrence(const RvChar* psztoFind, const RvChar* pszFindInMe)
{
    RvInt indexInFindInMe = 0;
    RvInt indexInToFind = 0;

    if (NULL == psztoFind || NULL == pszFindInMe)
    {
        return RV_FALSE;
    }
    while (0 == (psztoFind[indexInToFind] - pszFindInMe[indexInFindInMe]) % ('A'-'a'))
    {
        indexInFindInMe++;
        indexInToFind++;
        if ('\0' == psztoFind[indexInToFind])
        {
            return RV_TRUE;
        }
        if ('\0' == pszFindInMe[indexInFindInMe])
        {
            return RV_FALSE;
        }
    }
    return RV_FALSE;
}

/************************************************************************************
 * FreeMsgBuilderTcpResources
 * ----------------------------------------------------------------------------------
 * General:    Frees the resources used by the TransportMsgBuilderAccumulateMsg() in
 *          case that it was allocated.
 *
 * Return Value: Rv_Success/RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn              - Pointer to the connection.
 *          ppDecompressedBuf  - Pointer to decompressed buffer.
 *          pTcpEvent          - Pointer to the TCP Transport event.
 * Output:    (-)
 ***********************************************************************************/
static void RVCALLCONV FreeMsgBuilderTcpResources(
                          IN    TransportConnection           *pConn,
                          INOUT RvChar                       **ppDecompressedBuf,
                          IN    TransportProcessingQueueEvent *pTcpEvent)
{
    TransportMgr *pTransportMgr = pConn->pTransportMgr;

    if (pConn->recvInfo.hMsgReceived != NULL_PAGE)
    {
        RPOOL_FreePage(pTransportMgr->hMsgPool,pConn->recvInfo.hMsgReceived);
        pConn->recvInfo.hMsgReceived = NULL_PAGE;
        pConn->recvInfo.msgSize = 0;
    }
    if (*ppDecompressedBuf != NULL)
    {
        TransportMgrFreeRcvBuffer(pTransportMgr,*ppDecompressedBuf);
        *ppDecompressedBuf = NULL;
    }
    if (pTcpEvent != NULL)
    {
        TransportProcessingQueueFreeEvent((RvSipTransportMgrHandle)pTransportMgr,pTcpEvent);
    }
}

/************************************************************************************
 * RestoreConnInfo
 * ----------------------------------------------------------------------------------
 * General:    In case that the current connection is restored, due to error or
 *          invalid situation during its previous message handling the function
 *          retrieves the stored connection data.
 *
 * Return Value: void
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn       - Pointer to the connection.
 * Output:    pConnInfo   - The restored connection info.
 ***********************************************************************************/
static void RVCALLCONV RestoreConnInfo(
                          IN  TransportConnection        *pConn,
                          OUT MsgBuilderStoredConnInfo  *pConnInfo)
{
    pConnInfo->msgStart                  = pConn->msgStart;
    pConnInfo->msgSize                   = pConn->msgSize;
    pConnInfo->bytesReadFromBuf          = pConn->bytesReadFromBuf;
    pConnInfo->msgComplete               = pConn->msgComplete;
    pConnInfo->hLocalAddr                = pConn->hLocalAddr;
    pConnInfo->originalBuffer            = pConn->originalBuffer;
#ifdef RV_SIGCOMP_ON
    pConnInfo->pDecompressedBuf          = pConn->recvInfo.pDecompressedBuf;
    pConnInfo->decompressedBufSize       = pConn->recvInfo.decompressedBufSize;
#endif
    RvLogDebug(pConn->pTransportMgr->pMBLogSrc,(pConn->pTransportMgr->pMBLogSrc,
        "RestoreConnInfo - restoring connection 0x%p from OOR failure",pConn));
}

/************************************************************************************
 * StoreConnInfo
 * ----------------------------------------------------------------------------------
 * General:    Stores the current connection is restored, due to error or
 *          invalid situation during its message handling.
 *
 * Return Value: void
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConnInfo   - The restored connection info.
 * Output:  pConn       - Pointer to the connection.
 ***********************************************************************************/
static void RVCALLCONV StoreConnInfo(
                          IN  MsgBuilderStoredConnInfo  *pConnInfo,
                          OUT TransportConnection        *pConn)
{
    pConn->msgStart         = pConnInfo->msgStart;
    pConn->msgSize          = pConnInfo->msgSize;
    pConn->bytesReadFromBuf = pConnInfo->bytesReadFromBuf;
    pConn->msgComplete      = pConnInfo->msgComplete;
    pConn->hLocalAddr       = pConnInfo->hLocalAddr;
    pConn->originalBuffer   = pConnInfo->originalBuffer;
#ifdef RV_SIGCOMP_ON
    pConn->recvInfo.pDecompressedBuf    = pConnInfo->pDecompressedBuf;
    pConn->recvInfo.decompressedBufSize = pConnInfo->decompressedBufSize;
#endif
    RvLogDebug(pConn->pTransportMgr->pMBLogSrc,(pConn->pTransportMgr->pMBLogSrc,
        "StoreConnInfo - storing connection 0x%p for OOR failure",pConn));
}

/************************************************************************************
 * InitConnInfo
 * ----------------------------------------------------------------------------------
 * General:    Init the current connection data structure.
 *
 * Return Value: void
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Output:  pConn       - Pointer to the connection.
 ***********************************************************************************/
static void RVCALLCONV InitConnInfo(OUT TransportConnection        *pConn)
{
    pConn->bufSize          = 0;
    pConn->pBuf             = NULL;
    pConn->msgStart         = 0;
    pConn->msgSize          = 0;
    pConn->bytesReadFromBuf = 0;
    pConn->msgComplete      = RV_FALSE;
    pConn->originalBuffer   = NULL;

    RvLogDebug(pConn->pTransportMgr->pMBLogSrc,(pConn->pTransportMgr->pMBLogSrc,
        "InitConnInfo - Initializing connection 0x%p",pConn));
}

/************************************************************************************
 * PrepareTcpCompleteMsgBuffer
 * ----------------------------------------------------------------------------------
 * General:    Prepares a buffer that include complete message by decompressing an
 *          incoming buffer (in case of SigComp message), finding its body and
 *          calculating its length.
 *
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn               - Pointer to the connection.
 *          pBuf                - The input buffer to be manipulated.
 *          bufSize             - The size of the input buffer.
 * Output:    pSigCompMsgInfo     - Pointer to the SigComp message information structure.
 *          pConnInfo           - Pointer to the connection information structure.
 ***********************************************************************************/
static RvStatus RVCALLCONV PrepareTcpCompleteMsgBuffer(
                   IN    TransportConnection        *pConn,
                   IN    RvChar                     *pBuf,
                   IN    RvUint32                    bufSize,
                   OUT   SipTransportSigCompMsgInfo *pSigCompMsgInfo,
                   OUT   MsgBuilderStoredConnInfo   *pConnInfo)
{
    RvStatus status = RV_OK;
    RvUint32 bytesReadFromPlainBuf; /*the bytes actually read from the buffer and processed
                                       important when there is more then one sip msg in the buf
                                       and also there are CRLFs in the beginning of the sip msg*/
#ifdef RV_SIGCOMP_ON
    MsgBuilderSigCompInfo builderSigCompInfo;
#endif /* RV_SIGCOMP_ON */

#ifdef RV_SIGCOMP_ON
    builderSigCompInfo.pBuf             = pBuf;
    builderSigCompInfo.bufSize          = bufSize;
    builderSigCompInfo.pDecompressedBuf = NULL;

    status = BuildTcpDecompressMsgBufferIfNeeded(pConn,
                                                 &builderSigCompInfo,
                                                 pSigCompMsgInfo,
                                                 &pConnInfo->bytesReadFromBuf,
                                                 &pConnInfo->msgComplete,
                                                 &pConnInfo->bIsSigCompMsg);
    if (status != RV_OK)
    {
        FreeMsgBuilderTcpResources(pConn,&(builderSigCompInfo.pDecompressedBuf),NULL);
        RvLogError(pConn->pTransportMgr->pMBLogSrc,(pConn->pTransportMgr->pMBLogSrc,
            "PrepareTcpCompleteMsgBuffer - Couldn't decompress TCP message on conn %0x",pConn));

        return status;
    }

    if (pConnInfo->bIsSigCompMsg == RV_TRUE && pConnInfo->msgComplete != RV_TRUE)
    {
        return RV_OK;
    }
    /* If a message buffer was decompressed then the parsed */
    /* msg buffer pointer should point to different buffer  */
    if (pConnInfo->bIsSigCompMsg             == RV_TRUE &&
        builderSigCompInfo.pDecompressedBuf != NULL)
    {
        pConnInfo->pPlainBuf           = builderSigCompInfo.pDecompressedBuf;
        pConnInfo->plainBufSize        = builderSigCompInfo.decompressedBufSize;
        pConnInfo->pDecompressedBuf    = builderSigCompInfo.pDecompressedBuf;
        pConnInfo->decompressedBufSize = builderSigCompInfo.decompressedBufSize;
    }
    /* If there are few complete messages in the buffer - some are */
    /* compressed and some not we should set the pointer to the    */
    /* original buffer */
    else
#endif /* RV_SIGCOMP_ON */
    {
        pConnInfo->pPlainBuf           = pBuf;
        pConnInfo->plainBufSize        = bufSize;
        pConnInfo->pDecompressedBuf    = NULL;
        pConnInfo->decompressedBufSize = 0;
    }

    pConnInfo->msgSize  = 0;
    pConnInfo->msgStart = 0;

    /* preparing the buffer */
    status = TransportMsgBuilderTcpPrepare(pConn,
                                           pConnInfo->plainBufSize,
                                           pConnInfo->pPlainBuf,
                                           &pConnInfo->msgStart,
                                           &pConnInfo->msgSize,
                                           &bytesReadFromPlainBuf,
                                           &pConnInfo->msgComplete);

#ifdef RV_SIGCOMP_ON
    /* Set the bytes read from buffer value according the */
    /* the buffer type (SigComp/Plain). This setting must */
    /* be done before returning with/without error, since */
    /* there might be an invalid complete SIP message. In */
    /* this case the stack will move on to the next one.  */
    if (pConnInfo->bIsSigCompMsg != RV_TRUE)
#endif /* RV_SIGCOMP_ON */
    {
        pConnInfo->bytesReadFromBuf = bytesReadFromPlainBuf;
    }

    if (status != RV_OK)
    {
        /* if we got here, the buffer does not contain content length */
        FreeMsgBuilderTcpResources(pConn,&(pConnInfo->pDecompressedBuf),NULL);
        pConnInfo->msgSize=0;
        RvLogError(pConn->pTransportMgr->pMBLogSrc,(pConn->pTransportMgr->pMBLogSrc,
            "PrepareTcpCompleteMsgBuffer - Couldn't Analyze TCP message"));

		if (status == RV_ERROR_OUTOFRESOURCES)
		{
			RvLogError(pConn->pTransportMgr->pMBLogSrc,(pConn->pTransportMgr->pMBLogSrc,
				"PrepareTcpCompleteMsgBuffer - Received too long message, which cannot be handled. Closing pConn 0x%p",
				pConn));
			TransportConnectionDisconnect(pConn);

			/* An RV_ERROR_OUTOFRESOURCES value MUSTN'T be returned in this case - otherwise */
			/* the SIP Stack out of resource recovery mechanism (which doesn't know how to   */
			/* handle too long messages) will be turned on unjustifiably */
			return RV_ERROR_INSUFFICIENT_BUFFER;
		}
		else
		{
			return status;
		}
    }
    if (pConnInfo->msgComplete != RV_TRUE)
    {
#ifdef RV_SIGCOMP_ON
        if (pConnInfo->bIsSigCompMsg == RV_TRUE)
        {
            TransportMgrFreeRcvBuffer(pConn->pTransportMgr,pConnInfo->pDecompressedBuf);
            pConnInfo->pDecompressedBuf = NULL;
            RvLogError(pConn->pTransportMgr->pMBLogSrc,(pConn->pTransportMgr->pMBLogSrc,
                "PrepareTcpCompleteMsgBuffer - Received incomplete TCP message after decompression"));
        }
#endif /* RV_SIGCOMP_ON */
        return RV_OK;
    }

    RV_UNUSED_ARG(pSigCompMsgInfo);

    return status;
}

/************************************************************************************
 * HandleTcpCompleteMsgBuffer
 * ----------------------------------------------------------------------------------
 * General:    Handles a buffer that include complete SIP plain message by creating an
 *          notification event and queues it. In case of out of resource a set
 *          of data members is stored in the connection temporarily for future attempts.
 *
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn               - Pointer to the connection.
 *          pBuf                - The input buffer to be manipulated.
 *          bufSize             - The size of the input buffer.
 *          pSigCompMsgInfo     - Pointer to the SigComp message information structure.
 *          pConnInfo           - Pointer to the connection information structure.
 * Output:    (-)
 ***********************************************************************************/
static RvStatus RVCALLCONV HandleTcpCompleteMsgBuffer(
                   IN    TransportConnection        *pConn,
                   IN    RvChar                     *pBuf,
                   IN    RvUint32                   bufSize,
                   IN    SipTransportSigCompMsgInfo *pSigCompMsgInfo,
                   IN    MsgBuilderStoredConnInfo   *pConnInfo)
{
    RvStatus                       status;
    RvInt32                        sdpLength;
    RvInt32                        totalMsgSize; /*sip + sdp*/
    TransportProcessingQueueEvent *ev;
    RvUint8                       *memBuff;

    sdpLength                 = pConn->recvInfo.offsetInBody;

    status = TransportProcessingQueueAllocateEvent(
                (RvSipTransportMgrHandle)pConn->pTransportMgr,
                (RvSipTransportConnectionHandle)pConn,
                MESSAGE_RCVD_EVENT, RV_TRUE, &ev);
    if (status != RV_OK)
    {
        pConn->bufSize          = bufSize;
        pConn->pBuf             = pBuf;
        StoreConnInfo(pConnInfo,pConn);
        RvLogError(pConn->pTransportMgr->pMBLogSrc,(pConn->pTransportMgr->pMBLogSrc,
            "HandleTcpCompleteMsgBuffer - connection 0x%p(%d) Out Of Resource failure readBuffer: 0x%p",pConn,pConn->ccSocket,pConn->pBuf));

        return status;
    }


    memBuff = ev->typeSpecific.msgReceived.receivedBuffer;
    /*the message is in a buffer*/
    if (pConn->recvInfo.hMsgReceived == NULL_PAGE)
    {
        /*BUG totalMsgSize = (conn->recvInfo.bodyStartIndex + sdpLength - msgStart)+1; */
        totalMsgSize = (pConn->recvInfo.bodyStartIndex + sdpLength)+1; /*the +1 is for NULL*/
		if ((RvUint32)totalMsgSize >= pConn->pTransportMgr->maxBufSize)
        {
            FreeMsgBuilderTcpResources(pConn,&(pConnInfo->pDecompressedBuf),ev);
            RvLogError(pConn->pTransportMgr->pMBLogSrc,(pConn->pTransportMgr->pMBLogSrc,
                "HandleTcpCompleteMsgBuffer - Message is too big. Message received size (msgSize=%d) is to big for the given buffer 0x%p (buffSize=%d) conn=0x%p.",
                totalMsgSize, memBuff, pConn->pTransportMgr->maxBufSize, pConn));
            ResetConnReceivedInfo(pConn);
            return RV_ERROR_INSUFFICIENT_BUFFER;
        }
        memcpy(memBuff,&(pConnInfo->pPlainBuf[pConnInfo->msgStart]),totalMsgSize-1);
        memBuff[totalMsgSize-1] = '\0';
    }
    else /*the message in on the page starting at index 0 always*/
    {
        totalMsgSize = pConn->recvInfo.bodyStartIndex + pConn->recvInfo.offsetInBody+1; /*+1 for NULL at the end*/

        if ((RvUint32)totalMsgSize >= pConn->pTransportMgr->maxBufSize)

        {
            FreeMsgBuilderTcpResources(pConn,&(pConnInfo->pDecompressedBuf),ev);
            RvLogError(pConn->pTransportMgr->pMBLogSrc,(pConn->pTransportMgr->pMBLogSrc,
                "HandleTcpCompleteMsgBuffer - Message is too big. Message size  exceeds received buffer size (rbsize=%d, msgsize=%d)",
                pConn->pTransportMgr->maxBufSize,totalMsgSize));
            ResetConnReceivedInfo(pConn);
            return RV_ERROR_INSUFFICIENT_BUFFER;
        }

        status = RPOOL_CopyToExternal(pConn->pTransportMgr->hMsgPool,
            pConn->recvInfo.hMsgReceived,0,(void*)memBuff,totalMsgSize-1);
        if (status != RV_OK)
        {
            FreeMsgBuilderTcpResources(pConn,&(pConnInfo->pDecompressedBuf),ev);

            return status;
        }
        memBuff[totalMsgSize-1] = '\0';
    }

    ev->typeSpecific.msgReceived.bytesRecv      = totalMsgSize-1;
    ev->typeSpecific.msgReceived.sipMsgSize     = pConn->recvInfo.msgSize;
    ev->typeSpecific.msgReceived.sdpLength      = pConn->recvInfo.offsetInBody;
    ev->typeSpecific.msgReceived.hLocalAddr     = pConnInfo->hLocalAddr;
    ev->typeSpecific.msgReceived.hConn          = (RvSipTransportConnectionHandle)pConn;
    ev->typeSpecific.msgReceived.hAppConn       = pConn->hAppHandle;
    ev->typeSpecific.msgReceived.eConnTransport = pConn->eTransport;
    ev->typeSpecific.msgReceived.hInjectedMsg   = NULL;
    ev->typeSpecific.msgReceived.rcvFromAddr    = pConn->destAddress;
#ifdef RV_SIGCOMP_ON
    ev->typeSpecific.msgReceived.sigCompMsgInfo = *pSigCompMsgInfo;
#else
    RV_UNUSED_ARG(pSigCompMsgInfo);
#endif
#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
     ReportTcpCompleteMsgBuffer(pConn,ev);
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */

    status = TransportConnectionIncUsageCounter(pConn);
    if (status != RV_OK)
    {
        FreeMsgBuilderTcpResources(pConn,&(pConnInfo->pDecompressedBuf),ev);

        return status;
    }
    status = TransportProcessingQueueTailEvent((RvSipTransportMgrHandle)pConn->pTransportMgr,ev);
    if (status != RV_OK)
    {
        TransportConnectionDecUsageCounter(pConn);
        FreeMsgBuilderTcpResources(pConn,&(pConnInfo->pDecompressedBuf),ev);

        return status;
    }

    return status;
}

/************************************************************************************
 * ResetConnReceivedInfo
 * ----------------------------------------------------------------------------------
 * General:	resets the connection received info. This function should be used after
 *          the connection completed to process a complete message and should be ready
 *          to start processing a new incoming connection.
 *
 * Return Value: (-)
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - the connection handle
 * Output:	(-)
 ***********************************************************************************/
static void RVCALLCONV ResetConnReceivedInfo(
                          IN TransportConnection  *pConn)
{
    pConn->recvInfo.bReadingBody = RV_FALSE;
    pConn->recvInfo.bodyStartIndex=0;
    pConn->recvInfo.contentLength=-1;
    /*free recv buffer*/
    if (pConn->recvInfo.hMsgReceived != NULL_PAGE)
    {
        RPOOL_FreePage(pConn->pTransportMgr->hMsgPool,
            pConn->recvInfo.hMsgReceived);

        pConn->recvInfo.hMsgReceived=NULL_PAGE;

    }
    pConn->recvInfo.hMsgReceived=NULL_PAGE;
    pConn->recvInfo.lastHeaderOffset=0;
    pConn->recvInfo.msgSize=0;
    pConn->recvInfo.offsetInBody=0;
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/************************************************************************************
 * ReportTcpCompleteMsgBuffer
 * ----------------------------------------------------------------------------------
 * General:    Report to log about complete msg that was found in TCP stream
 *
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the connection.
 *          pEv   - The event that was allocated to handle the complete message.
 * Output:    (-)
 ***********************************************************************************/
static void RVCALLCONV ReportTcpCompleteMsgBuffer(
                   IN    TransportConnection           *pConn,
                   IN    TransportProcessingQueueEvent *pEv)
{
    RvChar     ipremote[RVSIP_TRANSPORT_LEN_STRING_IP];
    RvStatus   rv;
    RvSipTransportAddr  localAddrDetails;

    memset(&localAddrDetails,0,sizeof(RvSipTransportAddr));
    rv = TransportMgrLocalAddressGetDetails(pConn->hLocalAddr,
                                            &localAddrDetails);
    if (RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pMBLogSrc,(pConn->pTransportMgr->pMBLogSrc,
            "ReportTcpCompleteMsgBuffer(pConn=%p): TransportMgrLocalAddressGetDetails(hLocalAddr=0x%p) failed",
            pConn, pConn->hLocalAddr));
    }

    RvLogDebug(pConn->pTransportMgr->pMBLogSrc,(pConn->pTransportMgr->pMBLogSrc,
        "ReportTcpCompleteMsgBuffer - pConn 0x%p: %s message Rcvd, %s:%d<-%s:%d, size=%d",
        pConn,
        SipCommonConvertEnumToStrTransportType(pConn->eTransport),
        localAddrDetails.strIP,
        localAddrDetails.port,
        RvAddressGetString(&pConn->destAddress.addr,RVSIP_TRANSPORT_LEN_STRING_IP,ipremote),
        RvAddressGetIpPort(&pConn->destAddress.addr),
        pConn->recvInfo.msgSize));

    /* printing the buffer */
    TransportMsgBuilderPrintMsg(pConn->pTransportMgr,
        (RvChar *)pEv->typeSpecific.msgReceived.receivedBuffer,
        SIP_TRANSPORT_MSG_BUILDER_INCOMING,
        RV_FALSE);
}
#endif /* #if (RV_LOGMASK != RV_LOGLEVEL_NONE) */

#ifdef RV_SIGCOMP_ON
/************************************************************************************
 * IsSigCompStreamStart
 * ----------------------------------------------------------------------------------
 * General:    Checks if the buffer starts with SigComp message prefix. According to
 *          RFC3320 each SigComp message should start with 0xFF character.
 *
 * Return Value: RV_FALSE or RV_TRUE
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pBuf    - Pointer to stream data.
 *          bufSize - The length of the buffer.
 * Output:    (-)
 ***********************************************************************************/
static RvBool RVCALLCONV IsSigCompStreamStart(IN  RvChar   *pBuf,
                                              IN  RvInt32   bufSize)
{
    if (bufSize <= 0)
    {
        return RV_FALSE;
    }

    if ((*pBuf & SIP_MSG_BUILDER_SIGCOMP_PREFIX_MASK) ==
        SIP_MSG_BUILDER_SIGCOMP_PREFIX_MASK)
    {
        return RV_TRUE;
    }

    return RV_FALSE;
}

/************************************************************************************
 * DecompressCompeleteMsgBufferIfNeeded
 * ----------------------------------------------------------------------------------
 * General:    Builds plain message buffer by manipulating incoming UDP buffer.
 *
 * Return Value: RV_OK or RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr       - Pointer to the transport manager.
 * Input/Output:
 *          pBuilderSigCompInfo - SigComp Info. structure
 * Output:  pSigCompMsgInfo     - Pointer to SigComp message info.
 ***********************************************************************************/
static RvStatus RVCALLCONV DecompressCompeleteMsgBufferIfNeeded(
               IN    TransportMgr               *pTransportMgr,
   			   IN    RvSipTransport              eTransport,
               INOUT MsgBuilderSigCompInfo      *pBuilderSigCompInfo,
               OUT   SipTransportSigCompMsgInfo *pSigCompMsgInfo)
{
    RvStatus             rv;
    RvBool               bIsSigCompMsg;
    RvChar              *pDecompBuf   = NULL;
    RvInt32              decompBufLen = 0;
    RvUint32             uniqueID;
    RvSigCompMessageInfo sigCompMsgInfo;

    if (NULL == pSigCompMsgInfo)
    {
        return RV_ERROR_BADPARAM;
    }
    /* Initialize the SigComp message data structure */
    pSigCompMsgInfo->bExpectComparmentID = RV_FALSE;

    pBuilderSigCompInfo->pDecompressedBuf    = NULL;
    pBuilderSigCompInfo->decompressedBufSize = 0;

    /* If the received message is not SigComp message then */
    /* no buffer has to be decompressed (The buffer is     */
    /* already plain).                                     */
    bIsSigCompMsg = IsSigCompStreamStart(pBuilderSigCompInfo->pBuf,
                                         pBuilderSigCompInfo->bufSize);
    if (RV_TRUE != bIsSigCompMsg)
    {
        return RV_OK;
    }

    rv = TransportMgrAllocateRcvBuffer(pTransportMgr,RV_TRUE,&pDecompBuf);
    if (RV_OK != rv)
    {
        RvLogError(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                  "DecompressCompeleteMsgBufferIfNeeded - Failed to allocate read buffer for decompressing"));
        return rv;
    }

    decompBufLen = pTransportMgr->maxBufSize;

#ifdef PRINT_SIGCOMP_BYTE_STREAM
	SipCompartmentPrintMsgByteStream((RvUint8*)pBuilderSigCompInfo->pBuf,
									 pBuilderSigCompInfo->bufSize,
									 pTransportMgr->pMBLogSrc,
									 RV_TRUE);
#endif /* PRINT_SIGCOMP_BYTE_STREAM */

    /* Decompress the sent buffer by an external SigComp dll */
    sigCompMsgInfo.compressedMessageBuffSize = pBuilderSigCompInfo->bufSize;
    sigCompMsgInfo.pCompressedMessageBuffer  = (RvUint8*)pBuilderSigCompInfo->pBuf;
    sigCompMsgInfo.plainMessageBuffSize      = decompBufLen;
    sigCompMsgInfo.pPlainMessageBuffer       = (RvUint8*)pDecompBuf;
	convertRvTransportToSigcompTransport(eTransport, &sigCompMsgInfo.transportType);

	RvLogDebug(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
		"DecompressCompeleteMsgBufferIfNeeded - going to decompress msg buff 0x%p (size=%d) into buff 0x%p (size=%d), type=%d",
		sigCompMsgInfo.pCompressedMessageBuffer,
		sigCompMsgInfo.compressedMessageBuffSize,
		sigCompMsgInfo.pPlainMessageBuffer,
		sigCompMsgInfo.plainMessageBuffSize,
		sigCompMsgInfo.transportType));

    rv = RvSigCompDecompressMessage(pTransportMgr->hSigCompMgr,&sigCompMsgInfo,&uniqueID);

    if (RV_OK != rv)
    {
        TransportMgrFreeRcvBuffer(pTransportMgr,pDecompBuf);
        RvLogWarning(pTransportMgr->pMBLogSrc,(pTransportMgr->pMBLogSrc,
                  "DecompressCompeleteMsgBufferIfNeeded - Failed to decompress received buffer"));
        return rv;
    }

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
    RvLogInfo(pTransportMgr->pLogSrc ,(pTransportMgr->pLogSrc ,
       "DecompressCompeleteMsgBufferIfNeeded - an incoming message was decompressed successfully (plain=%d,comp=%d,uniqueID=%d):",
       sigCompMsgInfo.plainMessageBuffSize,sigCompMsgInfo.compressedMessageBuffSize,uniqueID));
    sigCompMsgInfo.pPlainMessageBuffer[sigCompMsgInfo.plainMessageBuffSize] = '\0';
    TransportMsgBuilderPrintMsg(pTransportMgr,
                                (RvChar*)(sigCompMsgInfo.pPlainMessageBuffer),
                                SIP_TRANSPORT_MSG_BUILDER_INCOMING,
                                RV_TRUE);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

    /* The unused SigComp buffer is freed and replaced with the new plain buffer */
    pSigCompMsgInfo->bExpectComparmentID     = RV_TRUE;
    pSigCompMsgInfo->uniqueID                = uniqueID;
    pBuilderSigCompInfo->pDecompressedBuf    = (RvChar*)sigCompMsgInfo.pPlainMessageBuffer;
    pBuilderSigCompInfo->decompressedBufSize = sigCompMsgInfo.plainMessageBuffSize;

    return RV_OK;
}

/************************************************************************************
 * IsTcpSigCompStream
 * ----------------------------------------------------------------------------------
 * General:    Checks if the received TCP buffer is part of SigComp stream.
 *
 * Return Value: RV_FALSE or RV_TRUE
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn               - Pointer to the connection
 *          pBuilderSigCompInfo - SigComp Info. structure
 * Output:    (-)
 ***********************************************************************************/
static RvBool RVCALLCONV IsTcpSigCompStream(
               IN TransportConnection    *pConn,
               IN MsgBuilderSigCompInfo *pBuilderSigCompInfo)
{
    if (pConn->recvInfo.hMsgReceived    != NULL_PAGE && /*means we had previous tcp part*/
        pConn->recvInfo.eMsgCompType == RVSIP_COMP_SIGCOMP)
    {
        return RV_TRUE;
    }
    else if (pConn->recvInfo.hMsgReceived == NULL_PAGE)
    {
        return IsSigCompStreamStart(pBuilderSigCompInfo->pBuf,
                                    pBuilderSigCompInfo->bufSize);
    }

    return RV_FALSE;
}

/************************************************************************************
 * HandleIncompleteSigCompDelimiter
 * ----------------------------------------------------------------------------------
 * General:    Handles the uncompleted SigComp delimiter in the previous packet by
 *          referring to the consequent byte in the received buffer.
 *          NOTE: According to RFC3320 SigComp delimiter consists of 2 bytes only.
 *                The first byte must be UNQUOTED 0xFF and the consequent byte can
 *                have range of values from 00 to 7F + The combination of
 *                0xFF FF signs the end of SigComp message.
 *
 * Return Value: RV_OK or RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn          - Pointer to the connection
 * Input/Output:
 *          ppCurrBufPos      - The current pos in the received buffer.
 *          pCurrBytesRead    - The number of bytes that were read from the total buf
 *                              until now.
 * Output:  pbMsgComplete  - Indication if SigComp end message delimiter was found.
 ***********************************************************************************/
static RvStatus RVCALLCONV HandleIncompleteSigCompDelimiter(
               IN    TransportConnection *pConn,
               INOUT RvChar             **ppCurrBufPos,
               INOUT RvInt32             *pCurrBytesRead,
               OUT   RvBool              *pbMsgComplete)
{
    RvUint8 completeByte = (RvUint8) **ppCurrBufPos;

    *pbMsgComplete = RV_FALSE;

    /* Promote the current position since the first char in the */
    /* received buffer was just handled as a completing char    */
    /* of a SigComp message */
    *pCurrBytesRead += 1;
    *ppCurrBufPos   += 1;

    /* Check if the first char creates the combination 0xFF FF with */
    /* the 0xFF from the previous TCP packet */
    if (completeByte == (RvUint8)SIP_MSG_BUILDER_SIGCOMP_DELIMITER)
    {
        *pbMsgComplete = RV_TRUE;
        return RV_OK;
    }

    /* Refer only to the values that are defined in the standard */
    if (completeByte > 0x00 &&
        completeByte < 0x80)
    {
        pConn->recvInfo.sigCompQuotedBytesLeft = (RvInt32) completeByte;
    }

    return RV_OK;
}

/************************************************************************************
 * HandleSigCompQuotedBytesLeft
 * ----------------------------------------------------------------------------------
 * General:    Handles the SigComp quoted bytes that are left from the TCP received
 *          buffer by promoting the number of the read bytes in the buffer without
 *          looking for SigComp delimiters (byte 0xFF).
 *
 * Return Value: RV_OK or RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn             - Pointer to the connection
 *          bufSizeLeft       - The left SigComp received buffer size.
 * Input/Output:
 *          ppCurrBufPos      - The current pos in the received buffer.
 *          pCurrBytesRead    - The number of bytes that were read from the total buf
 *                              until now.
 ***********************************************************************************/
static RvStatus RVCALLCONV HandleSigCompQuotedBytesLeft(
               IN    TransportConnection *pConn,
               IN    RvInt32             bufSizeLeft,
               INOUT RvChar            **ppCurrBufPos,
               INOUT RvInt32            *pCurrBytesRead)
{
    RvInt32 quotedBytesRead = 0;

#ifdef SIP_DEBUG
    if (bufSizeLeft < 0)
    {
        return RV_ERROR_BADPARAM;
    }
#endif
    /* If there are no quoted bytes left then nothing has to be done */
    if (pConn->recvInfo.sigCompQuotedBytesLeft == 0)
    {
        return RV_OK;
    }
    if (pConn->recvInfo.sigCompQuotedBytesLeft <= bufSizeLeft)
    {
        quotedBytesRead  = pConn->recvInfo.sigCompQuotedBytesLeft;
    }
    else
    {
        quotedBytesRead = bufSizeLeft;
    }
    *pCurrBytesRead += quotedBytesRead;
    *ppCurrBufPos   += quotedBytesRead;
    pConn->recvInfo.sigCompQuotedBytesLeft -= quotedBytesRead;

    return RV_OK;
}

/************************************************************************************
 * HandleSigCompQuotedBytesLeft
 * ----------------------------------------------------------------------------------
 * General:    Handles the SigComp TCP received buffer left by looking for SigComp
 *          delimiters (byte 0xFF).
 *
 * Return Value: RV_OK or RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn             - Pointer to the connection
 *          totalBufSize      - The total SigComp received buffer size.
 * Input/Output:
 *          ppCurrBufPos      - The current pos in the received buffer.
 *          pCurrBytesRead    - The number of bytes that were read from the total buf
 *                              until now.
 *          pbMsgComplete     - Indication if SigComp end message delimiter was found.
 ***********************************************************************************/
static RvStatus RVCALLCONV HandleSigCompBufferLeft(
               IN    TransportConnection *pConn,
               IN    RvInt32             totalBufSize,
               INOUT RvChar            **ppCurrBufPos,
               INOUT RvInt32            *pCurrBytesRead,
               OUT   RvBool             *pbMsgComplete)
{
    RvStatus rv;
    RvUint8   currByte;
    RvBool   bDelimiterFound;

    *pbMsgComplete = RV_FALSE;

    while (RV_FALSE == *pbMsgComplete && totalBufSize > *pCurrBytesRead)
    {
        rv = HandleSigCompQuotedBytesLeft(pConn,
                                         (totalBufSize - *pCurrBytesRead),
                                         ppCurrBufPos,
                                         pCurrBytesRead);
        if (RV_OK != rv)
        {
            return rv;
        }
        bDelimiterFound = RV_FALSE;
        /* Look for 0xFF byte that is not quoted */
        while (totalBufSize > *pCurrBytesRead && RV_FALSE == bDelimiterFound)
        {
            currByte = (RvUint8) **ppCurrBufPos;
            *ppCurrBufPos   += 1;
            *pCurrBytesRead += 1;
            if (currByte == SIP_MSG_BUILDER_SIGCOMP_DELIMITER)
            {
                bDelimiterFound = RV_TRUE;
            }
        }
        /* Handle incomplete 0xFF byte */
        if (RV_TRUE == bDelimiterFound)
        {
            if (totalBufSize > *pCurrBytesRead)
            {
                rv = HandleIncompleteSigCompDelimiter(pConn,
                                                      ppCurrBufPos,
                                                      pCurrBytesRead,
                                                      pbMsgComplete);
                if (RV_OK != rv)
                {
                    return rv;
                }
            }
            else /* must be totalBufSize == *pCurrBytesRead */
            {
                pConn->recvInfo.bSigCompDelimiterRcvd = RV_TRUE;
            }
        }
    }

    return RV_OK;
}

/************************************************************************************
 * FreeSigCompResources
 * ----------------------------------------------------------------------------------
 * General:    Frees the MsgBuilder SigComp resources. Useful in case of error in the
 *          middle of decompression process for example or when a receiving complete
 *          SigComp message.
 *
 * Return Value: -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn - Pointer to the connection
 * Output:  -
 ***********************************************************************************/
static void RVCALLCONV FreeSigCompResources(IN TransportConnection *pConn)
{
    TransportTcpRecvInfo  *pRecvInfo = &pConn->recvInfo;

    /* Free the allocated page that keep previous packet data */
    if (pRecvInfo->hMsgReceived != NULL_PAGE)
    {
        RPOOL_FreePage(pConn->pTransportMgr->hMsgPool,pRecvInfo->hMsgReceived);
    }

     /* Re-Initialize recvInfo structure for the next message */
    pRecvInfo->hMsgReceived           = NULL_PAGE;
    pRecvInfo->msgSize                = 0;
    pRecvInfo->sigCompQuotedBytesLeft = 0;
    pRecvInfo->bSigCompDelimiterRcvd  = RV_FALSE;
    pRecvInfo->eMsgCompType           = RVSIP_COMP_UNDEFINED;

}

/************************************************************************************
 * HandleSigCompCompleteMsgBuffer
 * ----------------------------------------------------------------------------------
 * General: Handles the SigComp TCP received buffer that include the termination
 *          sequence of SigComp message (0xFF FF) and completes a message.
 *
 * Return Value: RV_OK or RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn             - Pointer to the connection
 *          sigcompMsgBytes   - The number of bytes from the buffer before the
 *                              SigComp message termination.
 * Input/Output:
 *          pBuilderSigCompInfo - MsgBuilder SigComp Info. structure
 * Output:  pSigCompMsgInfo     - Pointer to SigComp message info.
 ***********************************************************************************/
static RvStatus RVCALLCONV HandleSigCompCompleteMsgBuffer(
               IN    TransportConnection        *pConn,
               IN    RvInt32                    sigcompMsgBytes,
               INOUT MsgBuilderSigCompInfo      *pBuilderSigCompInfo,
               OUT   SipTransportSigCompMsgInfo *pSigCompMsgInfo)
{
    RvStatus              rv;
    TransportMgr         *pTransportMgr    = pConn->pTransportMgr;
    RvBool                bNewBufAllocated = RV_FALSE;
    TransportTcpRecvInfo *pRecvInfo        = &pConn->recvInfo;
    MsgBuilderSigCompInfo decompBuilderInfo;

    memset(&decompBuilderInfo,0,sizeof(decompBuilderInfo));
    if (pRecvInfo->hMsgReceived != NULL_PAGE) /*means we had previous tcp part*/
    {
        bNewBufAllocated = RV_TRUE;
        /* Prepare the consecutive buffer for decompression */
        rv = TransportMgrAllocateRcvBuffer(pTransportMgr,
                                           RV_TRUE,
                                           &decompBuilderInfo.pBuf);
        if (RV_OK != rv)
        {
            RvLogError(pTransportMgr->pMBLogSrc ,(pTransportMgr->pMBLogSrc ,
                "HandleSigCompCompleteMsgBuffer - failed to allocate temporary consecutive buffer (rv=%d)",rv));
            return rv;
        }
        rv = RPOOL_CopyToExternal(pTransportMgr->hMsgPool,
                                  pRecvInfo->hMsgReceived,0,
                                  decompBuilderInfo.pBuf,pRecvInfo->msgSize);
        if (RV_OK != rv ||
            (RvUint32) (pRecvInfo->msgSize + sigcompMsgBytes) > pTransportMgr->maxBufSize)
        {
            TransportMgrFreeRcvBuffer(pTransportMgr,decompBuilderInfo.pBuf);
            RvLogError(pTransportMgr->pMBLogSrc ,(pTransportMgr->pMBLogSrc ,
                "HandleSigCompCompleteMsgBuffer - failed to prepare the consecutive buffer (stored msg size=%d,buffer msg size=%d)",
                pRecvInfo->msgSize,sigcompMsgBytes));
            return rv;
        }
        memcpy(decompBuilderInfo.pBuf+pRecvInfo->msgSize,
               pBuilderSigCompInfo->pBuf,
               sigcompMsgBytes);
        decompBuilderInfo.bufSize = pRecvInfo->msgSize + sigcompMsgBytes;
    }
    else
    {
        decompBuilderInfo.pBuf    = pBuilderSigCompInfo->pBuf;
        decompBuilderInfo.bufSize = sigcompMsgBytes;
    }

    /* Turn the SigComp stream into a SigComp message buffer (inplace)*/
    rv = RvSigCompStreamToMessage(decompBuilderInfo.bufSize,
                                  (RvUint8*)decompBuilderInfo.pBuf,
                                  &decompBuilderInfo.bufSize,
                                  (RvUint8*)decompBuilderInfo.pBuf);
    if (RV_OK != rv)
    {
        if (bNewBufAllocated == RV_TRUE)
        {
            TransportMgrFreeRcvBuffer(pTransportMgr,decompBuilderInfo.pBuf);
        }
        RvLogError(pTransportMgr->pMBLogSrc ,(pTransportMgr->pMBLogSrc ,
            "HandleSigCompCompleteMsgBuffer - failed to convert SigComp stream to message buffer (SigComp stream = 0x%p)",
            decompBuilderInfo.pBuf));
        return rv;
    }

    rv = DecompressCompeleteMsgBufferIfNeeded(pTransportMgr,
											  pConn->eTransport,
                                              &decompBuilderInfo,
                                              pSigCompMsgInfo);
    if (RV_OK != rv)
    {
        RvLogWarning(pTransportMgr->pMBLogSrc ,(pTransportMgr->pMBLogSrc ,
                "HandleSigCompCompleteMsgBuffer - failed to decompress the consecutive buffer %0x (rv=%d)",
                decompBuilderInfo.pBuf,rv));
    }

    /* Moving on to the next message to buffer. No matter if the */
    /* decompression succeeded or not */
    if (bNewBufAllocated == RV_TRUE)
    {
        TransportMgrFreeRcvBuffer(pTransportMgr,decompBuilderInfo.pBuf);
    }

    FreeSigCompResources(pConn);

    /* Setting the pointers to the decompressed buffer */
    pBuilderSigCompInfo->pDecompressedBuf    = decompBuilderInfo.pDecompressedBuf;
    pBuilderSigCompInfo->decompressedBufSize = decompBuilderInfo.decompressedBufSize;

    return rv;
}

/************************************************************************************
 * HandleSigCompIncompleteMsgBuffer
 * ----------------------------------------------------------------------------------
 * General:    Handles the SigComp TCP received buffer that DOESN'T include the
 *          termination sequence of SigComp message (0xFF FF) and DON'T
 *          completes a message.
 *
 * Return Value: RV_OK or RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn            - Pointer to the connection
 *          pBuf             - Pointer to the incomplete SigComp message buffer.
 *          bufSize          - The size of the incomplete SigComp message buffer.
 * Output:  pSigCompMsgInfo  - Pointer to SigComp message info.
 ***********************************************************************************/
static RvStatus RVCALLCONV HandleSigCompIncompleteMsgBuffer(
               IN    TransportConnection            *pConn,
               IN    RvChar                        *pBuf,
               IN    RvInt32                        bufSize,
               OUT   SipTransportSigCompMsgInfo     *pSigCompMsgInfo)
{
    RvInt32           allocationOffset;
    RPOOL_ITEM_HANDLE rpoolHandle;
    RvStatus          rv = RV_OK;
    TransportMgr*     pTransportMgr = pConn->pTransportMgr;

    /* Store the uncompleted SigComp message that was received */
    if (pConn->recvInfo.hMsgReceived == NULL_PAGE)
    {
        rv = RPOOL_GetPage(pTransportMgr->hMsgPool,0,&pConn->recvInfo.hMsgReceived);
        if (RV_OK != rv)
        {
            return rv;
        }
    }

    rv = RPOOL_AppendFromExternal(pTransportMgr->hMsgPool,
                                  pConn->recvInfo.hMsgReceived,
                                  pBuf,bufSize,RV_TRUE,
                                  &allocationOffset,&rpoolHandle);
    if (RV_OK != rv)
    {
        RPOOL_FreePage(pTransportMgr->hMsgPool,pConn->recvInfo.hMsgReceived);
        pConn->recvInfo.hMsgReceived = NULL_PAGE;
        RvLogError(pTransportMgr->pMBLogSrc ,(pTransportMgr->pMBLogSrc ,
                "HandleSigCompIncompleteMsgBuffer - failed to append the incomplete SigComp data to the hMsgReceived page(rv=%d)",rv));
        return rv;
    }

    /* Set the output variables */
    pConn->recvInfo.msgSize             += bufSize;
    pConn->recvInfo.eMsgCompType         = RVSIP_COMP_SIGCOMP;
    pSigCompMsgInfo->bExpectComparmentID = RV_FALSE;

    return RV_OK;
}

/************************************************************************************
 * BuildTcpDecompressMsgBufferIfNeeded
 * ----------------------------------------------------------------------------------
 * General:    Builds plain message buffer by manipulating incoming TCP stream.
 *
 * Return Value: RV_OK or RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn           - Pointer to the connection
 * Input/Output:
 *          pBuilderSigCompInfo - The MsgBuilder Info. structure that includes pointer
 *                                to the received buffer and pointer to the output
 *                                plain buffer that might be NULL in case that the
 *                                input buffer is not compressed at all.
 * Output:  pSigCompMsgInfo   - Pointer to SigComp message info.
 *          pBytesReadFromBuf - Pointer to the number of bytes from the input
 *                              buffer that were 'reviewed' during the decompression.
 *          pbMsgComplete     - Indication if a complete msg was decompressed.
 *          pbIsSigCompMsg       - Indication if the received is SigComp stream
 ***********************************************************************************/
static RvStatus RVCALLCONV BuildTcpDecompressMsgBufferIfNeeded(
                          IN    TransportConnection        *conn,
                          INOUT MsgBuilderSigCompInfo      *pBuilderSigCompInfo,
                          OUT   SipTransportSigCompMsgInfo *pSigCompMsgInfo,
                          OUT   RvUint32                   *pBytesReadFromBuf,
                          OUT   RvBool                     *pbMsgComplete,
                          OUT   RvBool                     *pbIsSigCompMsg)
{
    RvStatus rv;
    RvBool   bMsgComplete  = RV_FALSE;
    RvChar  *pCurrBufPos   = pBuilderSigCompInfo->pBuf;
    RvInt32  currBytesRead = 0;

    /* Initialize the output parameters */
    pBuilderSigCompInfo->pDecompressedBuf    = NULL;
    pBuilderSigCompInfo->decompressedBufSize = 0;
    *pBytesReadFromBuf                       = 0;
    *pbMsgComplete                           = RV_FALSE;
    *pbIsSigCompMsg                          = RV_FALSE;

    if (NULL == pSigCompMsgInfo)
    {
        return RV_ERROR_BADPARAM;
    }
    /* Initialize the SigComp message data structure */
    pSigCompMsgInfo->bExpectComparmentID     = RV_FALSE;

    if (pBuilderSigCompInfo->bufSize == 0)
    {
        return RV_OK;
    }

    *pbIsSigCompMsg = IsTcpSigCompStream(conn,pBuilderSigCompInfo);
    if (RV_FALSE == *pbIsSigCompMsg)
    {
        return RV_OK;
    }

    /* Check if we had previous tcp part and the last char in it */
    /* was part of the SigComp delimiter */
    if (conn->recvInfo.bSigCompDelimiterRcvd == RV_TRUE)
    {
        rv = HandleIncompleteSigCompDelimiter(conn,
                                              &pCurrBufPos,
                                              &currBytesRead,
                                              &bMsgComplete);
        if (RV_OK != rv)
        {
            FreeSigCompResources(conn);
            return rv;
        }
        conn->recvInfo.bSigCompDelimiterRcvd = RV_FALSE;
    }

    /* If the first byte in the buffer didn't complete  */
    /* a message, the rest of it is processed */
    if (bMsgComplete != RV_TRUE)
    {
        rv = HandleSigCompBufferLeft(conn,
                                     pBuilderSigCompInfo->bufSize,
                                     &pCurrBufPos,
                                     &currBytesRead,
                                     &bMsgComplete);
        if (RV_OK != rv)
        {
            FreeSigCompResources(conn);
            return rv;
        }
    }

    /* We're setting these values in order to move to the next message    */
    /* in buffer no matter if the decompression is completed successfully */
    *pbMsgComplete     = bMsgComplete;
    *pBytesReadFromBuf = currBytesRead;

    if (bMsgComplete == RV_TRUE)
    {
        rv = HandleSigCompCompleteMsgBuffer(conn,currBytesRead,
                                            pBuilderSigCompInfo,
                                            pSigCompMsgInfo);
        if (RV_OK != rv)
        {
            FreeSigCompResources(conn);
            return rv;
        }
    }
    else
    {
        rv = HandleSigCompIncompleteMsgBuffer(conn,
                                              pBuilderSigCompInfo->pBuf,
                                              currBytesRead,
                                              pSigCompMsgInfo);
        if (RV_OK != rv)
        {
            FreeSigCompResources(conn);
            return rv;
        }
    }

    return RV_OK;
}

/*************************************************************************
 * convertRvTransportToSigcompTransport
 * -----------------------------------------------------------------------
* General: convert transport type into bit mask.
*          One bit represents the transport reliability, and the 2nd bit
*          represents its type.
* Return Value: 
* ------------------------------------------------------------------------
* Arguments:
* Input:   eTransport        - The transport type.
* Output:  pSigcompTransport - The bit mask
**************************************************************************/
static void convertRvTransportToSigcompTransport(RvSipTransport eTransport, RvInt *pSigcompTransport)
{
	*pSigcompTransport = 0;
	switch (eTransport)
	{
	case RVSIP_TRANSPORT_TCP:            
	case RVSIP_TRANSPORT_TLS:
		*pSigcompTransport |= RVSIGCOMP_STREAM_TRANSPORT;
		*pSigcompTransport |= RVSIGCOMP_UNRELIABLE_TRANSPORT;/*RVSIGCOMP_RELIABLE_TRANSPORT;*/
		break;

	case RVSIP_TRANSPORT_SCTP:
		*pSigcompTransport |= RVSIGCOMP_MESSAGE_TRANSPORT;
		*pSigcompTransport |= RVSIGCOMP_UNRELIABLE_TRANSPORT;/*RVSIGCOMP_RELIABLE_TRANSPORT;*/
		break;

	default:
		*pSigcompTransport |= RVSIGCOMP_MESSAGE_TRANSPORT;
		*pSigcompTransport |= RVSIGCOMP_UNRELIABLE_TRANSPORT;
		break;
	}
}
#endif /* RV_SIGCOMP_ON */

/************************************************************************************
 * GetMsgCompressionType
 * ----------------------------------------------------------------------------------
 * General: Retrieves the compression type according to the sigcomp info
 *
 * Return Value: -
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pSigCompMsgInfo   - Pointer to SigComp message info (might be NULL in
 *								case of unsupported SigComp feature).
 * Output:  peCompression     -
 ***********************************************************************************/
static void RVCALLCONV GetMsgCompressionType(IN  SipTransportSigCompMsgInfo *pSigCompMsgInfo,
									   	     OUT RvSipCompType				*peCompression)
{
#ifdef RV_SIGCOMP_ON
	*peCompression = (pSigCompMsgInfo->bExpectComparmentID == RV_TRUE) ?
						RVSIP_COMP_SIGCOMP : RVSIP_COMP_UNDEFINED;
#else /* #ifdef RV_SIGCOMP_ON */
	*peCompression = RVSIP_COMP_UNDEFINED;

	RV_UNUSED_ARG(pSigCompMsgInfo);
#endif /* #ifdef RV_SIGCOMP_ON */
}

/******************************************************************************
 * ReportConnectionMsg
 * ----------------------------------------------------------------------------
 * General: provides the upper layers with Message object, into which was
 *          parsed buffer, received on TCP/TLS/SCTP connection.
 *          Destructs the message object after that.
 * Return Value: RvStatus - RV_OK or returned by subsequent calls error value
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - the Transport Manager object
 *          hMsg          - the Message object.
 *          pConn         - pointer to the connection.
 *          hAppConn      - application handle stored in the connection.
 *          hLocalAddr    - local address, used by the connection.
 *          pRecvFromAddr - remote address of the connection.
 *          pSigCompInfo  - SigComp information related to the message.
 * Output:  none
 *****************************************************************************/
static RvStatus RVCALLCONV ReportConnectionMsg(
                            IN TransportMgr*                     pTransportMgr,
                            IN RvSipMsgHandle                    hMsg,
                            IN TransportConnection*              pConn,
                            IN RvSipTransportConnectionAppHandle hAppConn,
                            IN RvSipTransportLocalAddrHandle     hLocalAddr,
                            IN SipTransportAddress*              pRecvFromAddr,
                            IN SipTransportSigCompMsgInfo*       pSigCompInfo)
{
    /* IMPORTANT!!!
       Assume that the Connection object is not locked.
       If you access the connection fields, lock connection Event Lock !
       Locking of persistent connection while parsing received buffer
       have bad impact on performance, since it locks Main (Select) Thread. */

    RvStatus                          rv;
    RvBool                            bProcessMsg;
	RvSipTransportBsAction            eBSAction;
	RvBool				              bBadSyntaxWasFound;
	RvSipTransportRcvdMsgDetails      rcvdMsgDetails;
#ifdef RV_SIP_IMS_ON
    void*                             pSecOwner = NULL;
#endif

    eBSAction = RVSIP_TRANSPORT_BS_ACTION_CONTINUE_PROCESS;
    bBadSyntaxWasFound = InformBadSyntax(pTransportMgr, hMsg, &eBSAction);

    TransportCallbackConnParserResult(pTransportMgr, hMsg, pConn, hAppConn,
                                      (!bBadSyntaxWasFound));

    /* Ensure, that connection was not closed by Application from
    TransportCallbackConnParserResult() callback */
    rv = TransportConnectionLockAPI(pConn);
    if(RV_OK != rv)
    {
        RvLogError(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
            "TransportMsgBuilderParseTcpMsg - failed to lock Connection 0x%p",pConn));
		RvSipMsgDestruct(hMsg);
        return rv;
    }

    if (pConn->bClosedByLocal == RV_TRUE)
    {
#if (RV_NET_TYPE & RV_NET_SCTP)
        if (RVSIP_TRANSPORT_CONN_STATE_SCTP_CONNECTED != pConn->eState  &&
            pConn->eTransport==RVSIP_TRANSPORT_SCTP)
#else
        if (RVSIP_TRANSPORT_CONN_STATE_TCP_CONNECTED != pConn->eState
#if (RV_TLS_TYPE != RV_TLS_NONE)
            ||
            (RVSIP_TRANSPORT_CONN_TLS_STATE_CONNECTED != pConn->eTlsState  &&
             pConn->eTransport==RVSIP_TRANSPORT_TLS)
#endif /* if TLS */
           )
#endif /* if SCTP else*/
        {
            TransportConnectionUnLockAPI(pConn);
            RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                "ReportConnectionMsg - Connection 0x%p was closed, message %p is discarded",
                pConn, hMsg));
            RvSipMsgDestruct(hMsg);
            return RV_OK;
        }
    }

    /* Get some Connection data when the connection is locked */
#ifdef RV_SIP_IMS_ON
    pSecOwner = pConn->pSecOwner;
#endif

    TransportConnectionUnLockAPI(pConn);


    /* Receive application permission to process the message */

	/* Prepare the received message details structure */
	memset(&rcvdMsgDetails,0,sizeof(rcvdMsgDetails));
	rcvdMsgDetails.eBSAction    = eBSAction;
	rcvdMsgDetails.hConnInfo    = (RvSipTransportConnectionHandle)pConn;
	rcvdMsgDetails.hLocalAddr   = hLocalAddr;
	GetMsgCompressionType(pSigCompInfo,&rcvdMsgDetails.eCompression);
	TransportMgrSipAddrGetDetails(pRecvFromAddr,&rcvdMsgDetails.recvFromAddr);

	/* IMPORTANT: The bProcessMsg is OVERWRITTEN due to the call to the */
	/* extended callback, which sets a default value in it or asks the  */
	/* application. */
    bProcessMsg = RV_TRUE;

#ifdef RV_SIP_IMS_ON
	TransportCallbackTcpMsgReceivedExtEv(hMsg, &rcvdMsgDetails, pRecvFromAddr,
                                         pSecOwner, &bProcessMsg);
#else
    TransportCallbackTcpMsgReceivedExtEv(hMsg,
        &rcvdMsgDetails,NULL/*pRecvFromAddr*/,NULL/*pSecOwner*/,&bProcessMsg);
#endif

    if (RV_TRUE == bProcessMsg)
    {
        if (NULL != pTransportMgr->appEvHandlers.pfnEvConnServerReuse)
        {
            rv = TransportConnectionApplyServerConnReuseThreadSafe(
                            pTransportMgr, pConn, hAppConn, hMsg);
            if(rv != RV_OK)
            {
                RvLogWarning(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "ReportConnectionMsg - Connection %p: Connection reuse failed(rv=%d:%s)",
                    pConn, rv, SipCommonStatus2Str(rv)));
            }
        }
        TransportCallbackConnectionMsgRcvdEv(pTransportMgr,
            hMsg, pConn, hLocalAddr, pRecvFromAddr, eBSAction, pSigCompInfo);
    }
    else
    {
        RvLogInfo(pConn->pTransportMgr->pMBLogSrc,(pConn->pTransportMgr->pMBLogSrc,
            "ReportConnectionMsg - received message is ignored due to application request"));
    }

	/*destruct the message unless JSR32 is used*/
#ifndef RV_SIP_JSR32_SUPPORT
	RvSipMsgDestruct(hMsg);
#endif /*#ifndef RV_SIP_JSR32_SUPPORT*/
    return RV_OK;
}

#ifdef __cplusplus
}
#endif

