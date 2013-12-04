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
 *                           TransportProcessingQueue.c
 *
 * This c file contains implementations for the transport event queue.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *********************************************************************************/



/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/

#include "RV_SIP_DEF.h"
#include "TransportProcessingQueue.h"
#include "AdsPqueue.h"
#include "TransportTCP.h"
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "TransportSCTP.h"
#endif /* #if (RV_NET_TYPE & RV_NET_SCTP) */
#include "TransportMsgBuilder.h"
#include "TransportCallbacks.h"
#include "_SipTransport.h"
#include "TransportConnection.h"
#include "rvansi.h"

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
#include "rvexternal.h"
#endif
/* SPIRENT_END */

/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                           MACRO DEFINITIONS                            */
/*-----------------------------------------------------------------------*/
#define GetEvTypeString(_e) (((_e)->type==INTERNAL_OBJECT_EVENT) ? "INTERNAL_EVENT" : "EXTERNAL_EVENT")


/*-----------------------------------------------------------------------*/
/*                           MODULE VARIABLES                            */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/
static RvStatus RVCALLCONV ProcessMsgReceivedEvent(
                                    TransportProcessingQueueEvent    *ev);

static RvStatus RVCALLCONV ProcessConnectedEvent(
                            IN TransportMgr*                  pTransportMgr,
                            IN TransportProcessingQueueEvent* pEv);

static RvStatus RVCALLCONV ProcessDisconnectedEvent(
                            IN TransportMgr*                  pTransportMgr,
                            IN TransportProcessingQueueEvent* pEv);

static RvStatus RVCALLCONV ProcessWriteEvent(
                            IN TransportMgr*                  pTransportMgr,
                            IN TransportProcessingQueueEvent* pEv);

static void RVCALLCONV ResendObjEvents(IN TransportMgr* pTransportMgr);

static RvStatus RVCALLCONV NotifyConnectionsOnResources(IN TransportMgr* pMgr,
                                                         IN TransportNotificationMsg msg);

static void RVCALLCONV OORListGetNextCell(IN TransportMgr*         pMgr,
                                          IN RLIST_ITEM_HANDLE*    hListElem);

/*-----------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                            */
/*-----------------------------------------------------------------------*/
/************************************************************************************
 * TransportProcessingQueueTailEvent
 * ----------------------------------------------------------------------------------
 * General: Send Event through the processing queue
 *
 * Return Value: RvStatus - RV_OK
 *                           RV_ERROR_UNKNOWN
 *                           RV_ERROR_BADPARAM
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTtransport    - Handle to transport instance
 *          ev             - event to be sent
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV TransportProcessingQueueTailEvent(
                        IN  RvSipTransportMgrHandle            hTtransport,
                        IN  TransportProcessingQueueEvent    *ev)
{
    TransportMgr        *pTransportMgr;
    RvStatus            retCode;

    if ((hTtransport == NULL) || (ev == NULL))
    {
        return RV_ERROR_BADPARAM;
    }

    pTransportMgr = (TransportMgr *)hTtransport;

    switch (ev->type) {
    case MESSAGE_RCVD_EVENT:
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
       ; // don't print the following debug to avoid overflooding log file.
#else
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportProcessingQueueTailEvent - MESSAGE_RCVD_EVENT 0x%p, buffer 0x%p",ev,
            ev->typeSpecific.msgReceived.receivedBuffer));
#endif
/* SPIRENT_END */
        break;
    case TIMER_EXPIRED_EVENT:
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
       ; // don't print the following debug to avoid overflooding log file.
#else
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportProcessingQueueTailEvent - TIMER_EXPIRED_EVENT 0x%p",ev));
#endif
/* SPIRENT_END */
        break;
    case WRITE_EVENT:
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
       ; // don't print the following debug to avoid overflooding log file.
#else
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportProcessingQueueTailEvent - WRITE_EVENT 0x%p",ev));
#endif
/* SPIRENT_END */
        break;
    case CONNECTED_EVENT:
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
       ; // don't print the following debug to avoid overflooding log file.
#else
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportProcessingQueueTailEvent - CONNECTED_EVENT 0x%p",ev));
#endif
/* SPIRENT_END */
        break;
    case DISCONNECTED_EVENT:
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
       ; // don't print the following debug to avoid overflooding log file.
#else
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportProcessingQueueTailEvent - DISCONNECTED_EVENT 0x%p",ev));
#endif
/* SPIRENT_END */
        break;
    case INTERNAL_OBJECT_EVENT:
    case EXTERNAL_OBJECT_EVENT:
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
       ; // don't print the following debug to avoid overflooding log file.
#else
        RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportProcessingQueueTailEvent - %s ev=0x%p: %s",
            GetEvTypeString(ev), ev,
            ev->typeSpecific.objEvent.strLoginfo));
#endif
/* SPIRENT_END */
        break;
    default:
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportProcessingQueueTailEvent - Unknown event 0x%p",ev));
        break;
    }

    retCode = PQUEUE_TailEvent(pTransportMgr->hProcessingQueue,(HQEVENT)ev);

    return retCode;
}


/************************************************************************************
 * TransportProcessingQueueFreeEvent
 * ----------------------------------------------------------------------------------
 * General:  Allocates and initiates message received event.
 * Return Value: RvStatus returned when thread is terminated
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTtransport        - pointer to transport info structure
 *            ev                - pointer to allocated processing event
 ***********************************************************************************/
RvStatus RVCALLCONV TransportProcessingQueueFreeEvent(
                                IN  RvSipTransportMgrHandle            hTtransport,
                                IN  TransportProcessingQueueEvent    *ev)
{
    RvStatus      retCode = RV_OK;
    TransportMgr* pTransportMgr;

    pTransportMgr = (TransportMgr *)hTtransport;

    if ((!pTransportMgr) || (pTransportMgr->hProcessingQueue == NULL))
    {
        return RV_ERROR_UNKNOWN;
    }

    if (ev == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    if (MESSAGE_RCVD_EVENT == ev->type)
    {
        ev->typeSpecific.msgReceived.hInjectedMsg = NULL;
        if (ev->typeSpecific.msgReceived.receivedBuffer != NULL)
        {
          retCode = TransportMgrFreeRcvBuffer(pTransportMgr,
                        (RvChar*)ev->typeSpecific.msgReceived.receivedBuffer);
        }
    }

    PQUEUE_FreeEvent(pTransportMgr->hProcessingQueue,(HQEVENT)ev);
    return retCode;
}

/************************************************************************************
 * TransportProcessingQueueAllocateEvent
 * ----------------------------------------------------------------------------------
 * General:  Allocates and initiates event, according to the event type.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - pointer to transport info structure
 *          hConn         - optional pointer to connection info structure
 *          type          - event type
 *          bAllocateRcvdBuff - allocate rcvdBuffer or not (for injected message
 *                        there is no need of this buffer)
 * Output:  ev            - pointer to allocated processing event
 ***********************************************************************************/
RvStatus RVCALLCONV TransportProcessingQueueAllocateEvent(
                            IN  RvSipTransportMgrHandle         hTtransport,
                            IN  RvSipTransportConnectionHandle  hConn,
                            IN  transportQueueEventType         type,
                            IN  RvBool                          bAllocateRcvdBuff,
                            OUT TransportProcessingQueueEvent   **ev)
{
    RvStatus                    retCode = RV_OK;
    TransportMgr                *pTransportMgr;

    pTransportMgr = (TransportMgr *)hTtransport;

    if ((!pTransportMgr) || (pTransportMgr->hProcessingQueue == NULL))
    {
        *ev = NULL;
        return RV_ERROR_UNKNOWN;
    }

/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
       ; // don't print the following debug to avoid overflooding log file.
#else
    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
        "TransportProcessingQueueAllocateEvent - starting to add event: (hConn=0x%x,type=%d,bAllo=%d)",
        hConn,type,bAllocateRcvdBuff));
#endif
/* SPIRENT_END */

    retCode = PQUEUE_AllocateEvent(pTransportMgr->hProcessingQueue,(HQEVENT *)ev);
    if (retCode != RV_OK)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportProcessingQueueAllocateEvent - failed to allocate from PQUEUE: (hConn=0x%x,type=%d,bAllo=%d)",
            hConn,type,bAllocateRcvdBuff));
        *ev = NULL;
        return retCode;
    }
    memset(*ev,0,sizeof(TransportProcessingQueueEvent));

    (*ev)->type = type;

    switch(type)
    {
    case MESSAGE_RCVD_EVENT:
        {
            RvChar *memBuff;

            if(bAllocateRcvdBuff == RV_TRUE)
            {
                   retCode = TransportMgrAllocateRcvBuffer(pTransportMgr,RV_FALSE,&memBuff);
                if (retCode != RV_OK)
                {
                    retCode = RV_ERROR_OUTOFRESOURCES;
                    PQUEUE_FreeEvent(pTransportMgr->hProcessingQueue,(HQEVENT)*ev);
                    *ev = NULL;
                    break;
                }
            }
            else
            {
                memBuff = NULL;
            }
            (*ev)->typeSpecific.msgReceived.bytesRecv       = 0;
            (*ev)->typeSpecific.msgReceived.receivedBuffer  = (RvUint8*)memBuff;
            (*ev)->typeSpecific.msgReceived.pTransportMgr   = pTransportMgr;
            (*ev)->typeSpecific.msgReceived.hConn           = hConn;
            (*ev)->typeSpecific.msgReceived.hInjectedMsg    = NULL;
        }
        break;
    case EXTERNAL_OBJECT_EVENT:
    case INTERNAL_OBJECT_EVENT:
    case TIMER_EXPIRED_EVENT:
    case CONNECTED_EVENT:
    case WRITE_EVENT:
    case DISCONNECTED_EVENT:
        break;
    default:
        PQUEUE_FreeEvent(pTransportMgr->hProcessingQueue,(HQEVENT)ev);
        RvLogExcep(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportProcessingQueueAllocateEvent - unsupported event %d", type));
        return RV_ERROR_UNKNOWN;
    }

    if (RV_OK != retCode)
    {
        RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "TransportProcessingQueueAllocateEvent - failed to allocate event: (hConn=0x%x,retCode=%d)",
            hConn,retCode));
    }
    return retCode;
}

/************************************************************************************
 * TransportProcessingQueueThreadDispatchEvents
 * ----------------------------------------------------------------------------------
 * General:  Main function of processing thread. Waits for events to arrive
 *           in the processing queue. Deletes the event from the queue and process
 *           specified by the event request.
 * Return Value: RvStatus returned when thread is terminated
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pContext - pointer to transport mgr info structure
 * Output:  none
 ***********************************************************************************/
void TransportProcessingQueueThreadDispatchEvents(
                                          IN RvThread *pProcThread,
                                          IN void     *pContext)
{
    TransportMgr *pTransportMgr = (TransportMgr*)pContext;

    TransportProcessingQueueDispatchEvents(pTransportMgr);
    RV_UNUSED_ARG(pProcThread);
}

/************************************************************************************
 * TransportProcessingQueueDispatchEvents
 * ----------------------------------------------------------------------------------
 * General:  Main function of processing thread. Waits for events to arrive
 *           in the processing queue. Deletes the event from the queue and process
 *           specified by the event request.
 * Return Value: RvStatus returned when thread is terminated
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr - pointer to transport info structure
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV TransportProcessingQueueDispatchEvents(IN TransportMgr *pTransportMgr)
{
    TransportProcessingQueueEvent* ev;
    RvStatus                       retCode;

    if (pTransportMgr == NULL)
    {
        return RV_ERROR_UNKNOWN;
    }

    /* PQUEUE_PopEvent suspends thread until there is event in the queue */
    while (1)
    {
        /* ASSUMPTION: pTransportMgr->hProcessingQueue can be changed after
                       Transport module construction. In addition no events
                       are expected if Transport module is being destructed.
           EFFECT: no need to lock pTransportMgr here */

        retCode = PQUEUE_PopEvent(pTransportMgr->hProcessingQueue,(HQEVENT*)&ev);
        if (retCode != RV_OK)
        {
            if (retCode!=RV_ERROR_NOT_FOUND  &&  retCode!=RV_ERROR_DESTRUCTED)
            {
                RvLogExcep(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "TransportProcessingQueueDispatchEvents - failed to pop event"));
            }
            return retCode;
        }

        if (ev == NULL)
        {
            continue;
        }

        switch (ev->type)
        {
            case MESSAGE_RCVD_EVENT:
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
       ; // don't print the following debug to avoid overflooding log files.
#else
                RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "TransportProcessingQueueDispatchEvents - MESSAGE_RCVD_EVENT 0x%p, buffer 0x%p",ev,
                    ev->typeSpecific.msgReceived.receivedBuffer));
#endif
/* SPIRENT_END */
                retCode = ProcessMsgReceivedEvent(ev);
                break;

            case CONNECTED_EVENT:
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
       ; // don't print the following debug to avoid overflooding log files.
#else
                RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "TransportProcessingQueueDispatchEvents - CONNECTED_EVENT %p",ev));
#endif
/* SPIRENT_END */
                retCode = ProcessConnectedEvent(pTransportMgr, ev);
                break;

            case DISCONNECTED_EVENT:
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
       ; // don't print the following debug to avoid overflooding log files.
#else
                RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "TransportProcessingQueueDispatchEvents - DISCONNECTED_EVENT %p",ev));
#endif
/* SPIRENT_END */
                retCode = ProcessDisconnectedEvent(pTransportMgr, ev);
                break;

            case WRITE_EVENT:
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
       ; // don't print the following debug to avoid overflooding log files.
#else
                RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                    "TransportProcessingQueueDispatchEvents - WRITE_EVENT %p",ev));
#endif
/* SPIRENT_END */
                retCode = ProcessWriteEvent(pTransportMgr, ev);
                break;

            case TIMER_EXPIRED_EVENT:
                {
                    RvSize_t numevents;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
       ; // don't print the following debug to avoid overflooding log files.
#else
                    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                        "TransportProcessingQueueDispatchEvents - TIMER_EXPIRED_EVENT 0x%p",ev))
#endif
/* SPIRENT_END */
                    RvTimerQueueService(&pTransportMgr->pSelect->tqueue, 0, &numevents, NULL, NULL);
                    RvTimerQueueControl(&pTransportMgr->pSelect->tqueue, RV_TIMERQUEUE_ENABLED);
                }
                break;

            case INTERNAL_OBJECT_EVENT:
            case EXTERNAL_OBJECT_EVENT:
                {
                    void*                            pObj;
                    RvInt32                          param1;
                    RvInt32                          param2;
                    RvSipTransportObjectEventHandler eventFunc;
                    SipTransportObjectStateEventHandler eventFuncEx;
                    RvInt32                          objUniqueIdentifier;
                    RvStatus                         rv;

                    pObj = ev->typeSpecific.objEvent.obj;
/* SPIRENT_BEGIN */
#if defined(UPDATED_BY_SPIRENT)
       ; // don't print the following debug to avoid overflooding log files.
#else
                    RvLogDebug(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                        "TransportProcessingQueueDispatchEvents - %s ev:0x%p, (%s)",
                        GetEvTypeString(ev), ev, ev->typeSpecific.objEvent.strLoginfo));
#endif
/* SPIRENT_END */
                    param1      = ev->typeSpecific.objEvent.param1;
                    param2      = ev->typeSpecific.objEvent.param2;
                    eventFunc   = ev->typeSpecific.objEvent.eventFunc;
                    eventFuncEx = ev->typeSpecific.objEvent.eventStateFunc;
                    objUniqueIdentifier = ev->typeSpecific.objEvent.objUniqueIdentifier;
                    if (eventFunc != NULL)
                    {
                        rv = eventFunc(pObj,param1); /*call the event callback*/
                        if (RV_OK != rv)
                        {
                            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                                "TransportProcessingQueueDispatchEvents - %s ev:0x%p, (%s) - eventFunc failed(rv=%d:%s)",
                                GetEvTypeString(ev), ev,
                                ev->typeSpecific.objEvent.strLoginfo,
                                rv, SipCommonStatus2Str(rv)));
                        }
                    }
                    if (eventFuncEx != NULL)
                    {
                        rv = eventFuncEx(pObj, param1, param2, objUniqueIdentifier);
                        if (RV_OK != rv)
                        {
                            RvLogError(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                                "TransportProcessingQueueDispatchEvents - %s ev:0x%p, (%s) - eventFuncEx failed(rv=%d:%s)",
                                GetEvTypeString(ev), ev,
                                ev->typeSpecific.objEvent.strLoginfo,
                                rv, SipCommonStatus2Str(rv)));
                        }
                    }
                }
                break; /* ENDOF: case INTERNAL_OBJECT_EVENT | EXTERNAL_OBJECT_EVENT */

            default:
                break;
        } /* ENDOF: switch (ev->type)  */

        PQUEUE_FreeEvent(pTransportMgr->hProcessingQueue,(HQEVENT)ev);

    } /* ENDOF: while (1)*/

    return RV_OK;
}

/************************************************************************************
 * TransportProcessingQueueResourceAvailable
 * ----------------------------------------------------------------------------------
 * General: Resource available callback:
 *          First try and resend events.
 *          Than try and re-notify connections on events that were OOR-ed before
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr        - Transport manager handler
 *          msg         - This message indicates the type of resource that was freed: PQ cell or read buffer
 * Output:  none
 ***********************************************************************************/
RvStatus TransportProcessingQueueResourceAvailable(IN TransportMgr* pMgr,
                                                  IN TransportNotificationMsg msg)
{
    RvStatus    rv = RV_OK;

    /* 1. let transport mgr send events from objects */
    if (PQUEUE_CELL_AVAILABLE == msg)
    {
        if (pMgr->timerOOR == RV_TRUE)
        {
            RvTimerQueueControl(&pMgr->pSelect->tqueue, RV_TIMERQUEUE_ENABLED);
            pMgr->timerOOR = RV_FALSE;
        }
        ResendObjEvents(pMgr);
    }

    /* 2. see if connections were stuck without resources.
          if so, let them party with the new resources */
    rv = NotifyConnectionsOnResources(pMgr, msg);
    return rv;
}

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
/************************************************************************************
 * TransportPQPrintOORlist
 * ----------------------------------------------------------------------------------
 * General: prints objects handles from the OOR list to the log.
 *          this function may be called by the application in order to have a better
 *          view of the out of resource status.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr        - Transport manager handler
 *
 * Output:  none
 ***********************************************************************************/
void RVCALLCONV TransportPQPrintOORlist(IN TransportMgr* pMgr)
{
    RvSipTransportObjEventInfo*  pEventInfo;
#define NUM_OF_OBJS_IN_LINE 20
    RvChar printBuff[NUM_OF_OBJS_IN_LINE*13]; /* memory for 12 chars per pointer + 1 for spaces */
    RvChar* pBuff = (RvChar*)printBuff;
    RvInt   numOfChars = 0;
    RvInt   i=0;

    if(pMgr->pEventToBeNotified == NULL)
    {
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
               "TransportPQPrintOORlist: The OOR list is currently empty"));
        return;
    }

    RvMutexLock(&pMgr->hObjEventListLock, pMgr->pLogMgr);

    pEventInfo    = pMgr->pEventToBeNotified;
    memset(printBuff, 0, sizeof(printBuff));

    while (NULL != pEventInfo )
    {
        while (i < NUM_OF_OBJS_IN_LINE && pEventInfo != NULL)
        {
            numOfChars = RvSprintf(pBuff, "%p ", pEventInfo->objHandle);
            pEventInfo = pEventInfo->next;
            pBuff += numOfChars;
            ++i;
        }
        /* print and reset the buffer for more prints */
        *pBuff = '\0';
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
           "TransportPQPrintOORlist: %s", printBuff));
        memset(printBuff, 0, sizeof(printBuff));
        pBuff = (RvChar*)printBuff;
        i = 0;
    }

    RvMutexUnlock(&pMgr->hObjEventListLock,pMgr->pLogMgr);
    return;
}
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/


/************************************************************************************
 * ProcessMsgReceivedEvent
 * ----------------------------------------------------------------------------------
 * General:  Function, applied by processing thread upon receiving 'message received'
 *             event from CMT. It is responsible for both processing and freeing event
 *             resources.
 * Return Value: RV_OK or return code of applied functions
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   param - pointer to transport info structure
 * Output:  none
 ***********************************************************************************/
static RvStatus RVCALLCONV ProcessMsgReceivedEvent(TransportProcessingQueueEvent    *ev)
{
    /* IMPORTANT!!!
       Assume that the Connection object is not locked.
       If you access the connection fields, lock connection Event Lock !
       Locking of persistent connection while parsing received buffer
       have bad impact on performance, since it locks Main (Select) Thread. */

    /* Entire SIP message was read by CMT thread.
       Event should contain buffer with the SIP message string. */
    RvInt32                       totalMsgSize   = ev->typeSpecific.msgReceived.bytesRecv;
    RvInt32                       sipMsgSize     = ev->typeSpecific.msgReceived.sipMsgSize;
    RvInt32                       sdpLength      = ev->typeSpecific.msgReceived.sdpLength;
    RvSipTransportLocalAddrHandle hLocalAddr     = ev->typeSpecific.msgReceived.hLocalAddr;
    SipTransportAddress           recvFromAddr   = ev->typeSpecific.msgReceived.rcvFromAddr;
    RvChar                       *buff           = (RvChar *)ev->typeSpecific.msgReceived.receivedBuffer;
    TransportMgr                 *pTransportMgr  = ev->typeSpecific.msgReceived.pTransportMgr;
    RvStatus                      retCode        = RV_OK;
    RvSipMsgHandle                hInjectedMsg   = ev->typeSpecific.msgReceived.hInjectedMsg;
    TransportConnection          *pConn          = (TransportConnection*)(ev->typeSpecific.msgReceived.hConn);
    RvSipTransport                eConnTransport = ev->typeSpecific.msgReceived.eConnTransport;
    SipTransportSigCompMsgInfo   *pSigCompInfo   = NULL;
    RvBool                        bDiscardBuffer;
    RvSipTransportConnectionAppHandle hAppConn   = ev->typeSpecific.msgReceived.hAppConn;


#ifdef RV_SIGCOMP_ON
    /* Set the SigComp info pointer if the feature is enabled */
    pSigCompInfo = &ev->typeSpecific.msgReceived.sigCompMsgInfo;
#endif /* RV_SIGCOMP_ON */

    /* The connection is in use -> remove it from the not-in-use list */
    if(pConn != NULL)
    {
        retCode = TransportConnectionRemoveFromConnectedNoOwnerListThreadSafe(
                                pTransportMgr, pConn);
        if (retCode != RV_OK)
        {
            RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                       "ProcessMsgReceivedEvent: ev:%p - RemoveFromConnectedNoOwnerList failed(%d:%s)",
                       ev, retCode, SipCommonStatus2Str(retCode)));
            return retCode;
        }
    }

    /* Expose the received buffer to the Application */
    TransportCallbackBufferReceived(
        pTransportMgr, (TransportMgrLocalAddr*)hLocalAddr, &recvFromAddr,
        pConn, buff, totalMsgSize, &bDiscardBuffer);
    if (RV_TRUE == bDiscardBuffer)
    {
        /* Free buffer */
        ev->typeSpecific.msgReceived.hInjectedMsg = NULL;
        if (ev->typeSpecific.msgReceived.receivedBuffer != NULL)
        {
            TransportMgrFreeRcvBuffer(pTransportMgr,
                (RvChar*)ev->typeSpecific.msgReceived.receivedBuffer);
        }

        /* Decrease usage counter of the connection */
        retCode = TransportConnectionDecUsageCounterThreadSafe(pTransportMgr, pConn);
        return retCode;
    }

    /* Handle received buffer */
    switch (eConnTransport)
    {
        case RVSIP_TRANSPORT_UDP:
            retCode = TransportMsgBuilderParseUdpMsg(pTransportMgr, buff,
                        totalMsgSize, hInjectedMsg, hLocalAddr, &recvFromAddr);
            break;  /* case RVSIP_TRANSPORT_UDP */

        case RVSIP_TRANSPORT_TCP:
        case RVSIP_TRANSPORT_TLS:
            {
                RvStatus usageCounterDecRetCode;
                retCode = TransportMsgBuilderParseTcpMsg(pTransportMgr,
                                                         buff,
                                                         totalMsgSize,
                                                         pConn,
                                                         hAppConn,
                                                         sipMsgSize,
                                                         sdpLength,
                                                         hLocalAddr,
                                                         &recvFromAddr,
                                                         hInjectedMsg,
                                                         pSigCompInfo);

                /* Decrease usage counter of the connection */
                usageCounterDecRetCode = TransportConnectionDecUsageCounterThreadSafe(pTransportMgr, pConn);
                retCode = (usageCounterDecRetCode == RV_OK) ? retCode : usageCounterDecRetCode;
            }
            break; /* case RVSIP_TRANSPORT_TCP, case RVSIP_TRANSPORT_TLS*/

        case RVSIP_TRANSPORT_SCTP:
            retCode = TransportMsgBuilderParseSctpMsg(pTransportMgr, pConn,
                        hAppConn, buff, totalMsgSize, hInjectedMsg, hLocalAddr,
                        &recvFromAddr);
            /* Don't decrease usage counter: it is not relevant for SCTP */
            break;  /* case RVSIP_TRANSPORT_SCTP */

        default:
            RvLogExcep(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                       "ProcessMsgReceivedEvent: ev:%p - unsupported transport type (%d)",
                       ev, eConnTransport));
            return RV_ERROR_UNKNOWN;
    }

    /* Free buffer. Is is not needed anymore - we have a Message object */
    if (NULL == hInjectedMsg)
    {
        TransportMgrFreeRcvBuffer(pTransportMgr, buff);
    }

    return retCode;
}

/******************************************************************************
 * ProcessConnectedEvent
 * ----------------------------------------------------------------------------
 * General:  handles CONNECTED event, read from TPQ(Transport Processing Queue)
 * Return Value: RV_OK or return code of applied functions
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pEv - event
 * Output:  none
 *****************************************************************************/
static RvStatus RVCALLCONV ProcessConnectedEvent(
                            IN TransportMgr*                  pTransportMgr,
                            IN TransportProcessingQueueEvent* pEv)
{
    RvStatus rv;
    TransportConnection* pConn = pEv->typeSpecific.connected.pConn;
    RvBool               bError = pEv->typeSpecific.connected.error;

    rv = TransportConnectionLockEvent(pConn);
    if (rv != RV_OK)
    {
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "ProcessConnectedEvent - failed to lock Connection %p", pConn));
        return RV_OK;
    }

    if (RV_TRUE == pConn->bClosedByLocal)
    {
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "ProcessConnectedEvent - Connection %p was closed locally", pConn));
        TransportConnectionDecUsageCounter(pConn);
        TransportConnectionUnLockEvent(pConn);
        return RV_OK;
    }

    switch (pConn->eTransport)
    {
#if (RV_NET_TYPE & RV_NET_SCTP)
        case RVSIP_TRANSPORT_SCTP:
            rv = TransportSctpConnectedQueueEvent(pConn, bError);
            TransportConnectionDecUsageCounter(pConn);
            break;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

        case RVSIP_TRANSPORT_TCP:
        case RVSIP_TRANSPORT_TLS:
            rv = TransportTcpConnectedQueueEvent(pConn, bError);
            TransportConnectionDecUsageCounter(pConn);
            break;

        default:
            RvLogExcep(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "ProcessConnectedEvent - Connection %p - unsupported type (%d)",
                pConn, pConn->eTransport));
            rv = RV_ERROR_UNKNOWN;
    }

    TransportConnectionUnLockEvent(pConn);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransportMgr);
#endif

    return rv;
}

/******************************************************************************
 * ProcessDisconnectedEvent
 * ----------------------------------------------------------------------------
 * General:  handles DISCONNECTED event, read from TPQ.
 * Return Value: RV_OK or return code of applied functions
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pEv - event
 * Output:  none
 *****************************************************************************/
static RvStatus RVCALLCONV ProcessDisconnectedEvent(
                            IN TransportMgr*                  pTransportMgr,
                            IN TransportProcessingQueueEvent* pEv)
{
    RvStatus rv;
    TransportConnection* pConn = pEv->typeSpecific.disconnected.pConn;

    rv = TransportConnectionLockEvent(pConn);
    if (rv != RV_OK)
    {
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "ProcessDisconnectedEvent - failed to lock Connection %p", pConn));
        return RV_OK;
    }

    /* If there is WRITE event in TPQ, waiting to be handled,
       postpone processing of DISCONNECT event. */
    if (RV_TRUE == pConn->bWriteEventInQueue)
    {
        TransportProcessingQueueEvent* pEvPostponed;

        rv = TransportProcessingQueueAllocateEvent(
                (RvSipTransportMgrHandle)pTransportMgr,
                (RvSipTransportConnectionHandle)pConn, DISCONNECTED_EVENT,
                RV_FALSE/*bAllocateRcvdBuff*/, &pEvPostponed);
        if (rv == RV_OK)
        {
            *pEvPostponed = *pEv;
            rv = TransportProcessingQueueTailEvent(
                    (RvSipTransportMgrHandle)pTransportMgr, pEvPostponed);
            if (rv != RV_OK)
            {
                PQUEUE_FreeEvent(pTransportMgr->hProcessingQueue,(HQEVENT)pEvPostponed);
            }
            else /* rv == RV_OK */
            {
                RvLogDebug(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
                    "ProcessDisconnectedEvent - connection %p: processing DISCONNECT_EVENT was postponed due to not handled yet WRITE_EVENT",
                    pConn));
                TransportConnectionUnLockEvent(pConn);
                return RV_OK;
            }
        }
    }

    switch (pConn->eTransport)
    {
#if (RV_NET_TYPE & RV_NET_SCTP)
        case RVSIP_TRANSPORT_SCTP:
            rv = TransportSctpDisconnectQueueEvent(pConn);
            break;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

        case RVSIP_TRANSPORT_TCP:
        case RVSIP_TRANSPORT_TLS:
            rv = TransportTcpDisconnectQueueEvent(pConn);
            break;

        default:
            RvLogExcep(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "ProcessDisconnectedEvent - Connection %p - unsupported type (%d)",
                pConn, pConn->eTransport));
            rv = RV_ERROR_UNKNOWN;
    }

    /* No decrement counter here */

    TransportConnectionUnLockEvent(pConn);
    return rv;
}

/******************************************************************************
 * ProcessWriteEvent
 * ----------------------------------------------------------------------------
 * General:  handles WRITE event, read from TPQ (Transport Processing Queue).
 * Return Value: RV_OK or return code of applied functions
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pEv - event
 * Output:  none
 *****************************************************************************/
static RvStatus RVCALLCONV ProcessWriteEvent(
                            IN TransportMgr*                  pTransportMgr,
                            IN TransportProcessingQueueEvent* pEv)
{
    RvStatus rv;
    TransportConnection* pConn = pEv->typeSpecific.write.pConn;

    rv = TransportConnectionLockEvent(pConn);
    if (rv != RV_OK)
    {
        RvLogWarning(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
            "ProcessWriteEvent - failed to lock Connection %p", pConn));
        return RV_OK;
    }

    switch (pConn->eTransport)
    {
#if (RV_NET_TYPE & RV_NET_SCTP)
        case RVSIP_TRANSPORT_SCTP:
            rv = TransportSctpWriteQueueEvent(pConn);
            TransportConnectionDecUsageCounter(pConn);
            break;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

        case RVSIP_TRANSPORT_TCP:
        case RVSIP_TRANSPORT_TLS:
            rv = TransportTcpWriteQueueEvent(pConn);
            TransportConnectionDecUsageCounter(pConn);
            break;

        default:
            RvLogExcep(pTransportMgr->pLogSrc,(pTransportMgr->pLogSrc,
                "ProcessWriteEvent - Connection %p - unsupported type (%d)",
                pConn, pConn->eTransport));
            rv = RV_ERROR_UNKNOWN;
    }

    /* No decrement counter here */

    TransportConnectionUnLockEvent(pConn);

#if (RV_LOGMASK == RV_LOGLEVEL_NONE)
	RV_UNUSED_ARG(pTransportMgr);
#endif

	return rv;
}

/***************************************************************************
 * NotifyConnectionsOnResources
 * ------------------------------------------------------------------------
 * General: Goes through the list of OOR event.
 *          for each event: if the connection is still "alive" try and re-process the event
 *          if the connection cant be locked, it means that the connection has died. we don't care.
 *          keep an eye on the events that were processed, try to avoid loops by
 *          not going with the same event twice.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr        - Transport manager handler
 *          msg         - This message indicates the type of resource that was freed: PQ cell or read buffer
 ***************************************************************************/
static RvStatus RVCALLCONV NotifyConnectionsOnResources(IN TransportMgr* pMgr,
                                                         IN TransportNotificationMsg msg)
{
    TransportConnection*    pConn           = NULL;
    RLIST_ITEM_HANDLE       hListElem       = NULL;
    RvSelectEvents          eventsToRetry   = 0;
    RvSelectEvents          processedEvents = 0;
    RvStatus                rv              = RV_OK;
    RvBool                  bUdpOOR         = RV_FALSE;

    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
        "NotifyConnectionsOnResources - Starting To Notify On OOR! (msg=%d)",msg))

    /* first checking UDP oor */
    RvMutexLock(&pMgr->hRcvBufferAllocLock,pMgr->pLogMgr);
    if (RV_TRUE == pMgr->notEnoughBuffersInPoolForUdp)
    {
        pMgr->notEnoughBuffersInPoolForUdp = RV_FALSE;
        bUdpOOR = RV_TRUE;
    }
    RvMutexUnlock(&pMgr->hRcvBufferAllocLock,pMgr->pLogMgr);

    if (RV_TRUE == bUdpOOR)
    {
        TransportMgrSelectUpdateForAllUdpAddrs(pMgr,RvSelectRead);
    }

    RvMutexLock(&pMgr->hOORListLock,pMgr->pLogMgr);
    RLIST_GetHead(pMgr->oorListPool,pMgr->oorList,&hListElem);
    RvMutexUnlock(&pMgr->hOORListLock,pMgr->pLogMgr);
    while (NULL != hListElem)
    {
        pConn = *(TransportConnection**)hListElem;
        rv = TransportConnectionLockEvent(pConn);
        if (rv != RV_OK)
        {
            RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "NotifyConnectionsOnResources - conn 0x%p, was destructed. don't care for left over OOR events",pConn))
            OORListGetNextCell(pMgr,&hListElem);
			/*Since the OORListGetNextCell is also removing the previous element from the list we need to
			  NULLIFY the oorListLocation of the pConn in order to avoid multiple removing from the list.*/
			pConn->oorListLocation = NULL;
            continue;
        }

        if (pConn->eState == RVSIP_TRANSPORT_CONN_STATE_TERMINATED)
        {
            TransportConnectionUnLockEvent(pConn);
            RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "NotifyConnectionsOnResources - conn 0x%p, was terminated. don't care for left over OOR events",pConn))
            OORListGetNextCell(pMgr,&hListElem);
			/*Since the OORListGetNextCell is also removing the previous element from the list we need to
			  NULLIFY the oorListLocation of the pConn in order to avoid multiple removing from the list.*/
			pConn->oorListLocation = NULL;
            continue;
        }

        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "NotifyConnectionsOnResources - conn 0x%p, processing OOR events 0x%p",
            pConn, pConn->oorEvents))

        /* resetting eventsToRetry */
        eventsToRetry = 0;

        switch (msg)
        {
        case PQUEUE_CELL_AVAILABLE:
            if ((pConn->oorEvents) & RvSelectClose)
            {
                eventsToRetry = RvSelectClose;
            }
            if ((pConn->oorEvents) & RvSelectConnect)
            {
                eventsToRetry = RvSelectConnect;
            }
            if ((pConn->oorEvents) & RvSelectWrite)
            {
                eventsToRetry = RvSelectWrite;
            }
            /* don't put 'break' here */
        case TRANSPORT_READ_BUFFRES_READY:
            if ((pConn->oorEvents) & RvSelectRead)
            {
                eventsToRetry |= RvSelectRead;
            }
            break;
        default:
            break;
        }

        if (processedEvents & eventsToRetry)
        {
            /* we already tried to process this event for this connection
               endless loop should be avoided */
            RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "NotifyConnectionsOnResources - conn 0x%p, processedEvents == eventsToRetry == 0x%p, exiting to avoid loop",pConn,eventsToRetry))
            TransportConnectionUnLockEvent(pConn);
            break;
        }
        if (eventsToRetry == 0)
        {

            OORListGetNextCell(pMgr,&hListElem);
            pConn->oorListLocation = NULL;
            TransportConnectionUnLockEvent(pConn);
            RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "NotifyConnectionsOnResources - conn 0x%p, eventsToRetry==0,continuing",pConn))
            continue;
        }
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "NotifyConnectionsOnResources - conn 0x%p, retrying events 0x%p",pConn, eventsToRetry))

        /* removed the handled event from list of oor events */
        pConn->oorEvents &= ~eventsToRetry;
        RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
            "NotifyConnectionsOnResources - conn 0x%p, recalling TransportTcp/SctpEventCallback() with: event 0x%p, error:%d",
             pConn, eventsToRetry,pConn->oorConnectIsError));

        /* Simulate event from net */
        switch (pConn->eTransport)
        {
            case RVSIP_TRANSPORT_TCP:
#if (RV_TLS_TYPE != RV_TLS_NONE)
            case RVSIP_TRANSPORT_TLS:
/*OOR fix recovery*/
				/*If we destructed the FD when the OOR occured we need to recreate the FD again*/
				if (RV_FALSE == pConn->bFdAllocated)
				{
					RvLogInfo(pConn->pTransportMgr->pLogSrc,(pConn->pTransportMgr->pLogSrc,
						"NotifyConnectionsOnResources - connection 0x%p: FD was destructed upon OOR event recreating new FD.", pConn));
					/*We re-setting the events to listen to read and write, the close event will be added from the flag to the TransportConnectionRegisterSelectEvents func*/
					pConn->ccEvent = RvSelectRead;
					TransportConnectionRegisterSelectEvents(
						pConn, RV_TRUE /*bAddClose */, RV_TRUE /*bFirstCall*/);
				}

                /* TLS events, stored in pConn->pTlsSession, are reset on
                select event handling, even if we fail to complete the handling
                due to OUT-OF-RESOURCES.
                Refresh the pConn->pTlsSession->tlsEvents with the TLS events
                stored in the Connection before processing of the restored
                select event */
                if (RVSIP_TRANSPORT_TLS == pConn->eTransport)
                {
                    RvTLSTranslateTLSEvents(pConn->pTlsSession,
                        pConn->eTlsEvents,
                        &pConn->pTripleLock->hLock,
                        pConn->pTransportMgr->pLogMgr,
                        &pConn->ccEvent);
                }
#endif /*#if (RV_TLS_TYPE != RV_TLS_NONE)*/

				TransportTcpEventCallback(pMgr->pSelect,&pConn->ccSelectFd,eventsToRetry,pConn->oorConnectIsError);
                break;
#if (RV_NET_TYPE & RV_NET_SCTP)
            case RVSIP_TRANSPORT_SCTP:
                TransportSctpEventCallback(pMgr->pSelect,&pConn->ccSelectFd,eventsToRetry,pConn->oorConnectIsError);
                break;
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
            default:
                RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
                    "NotifyConnectionsOnResources - Not supported Transport Type (%d)",pConn->eTransport));
                TransportConnectionUnLockEvent(pConn);
                return RV_ERROR_UNKNOWN;
        }

        if (0 == pConn->oorEvents)
        {
            RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "NotifyConnectionsOnResources - conn 0x%p, All OOR events were processed",pConn))
            OORListGetNextCell(pMgr,&hListElem);
            processedEvents = 0;
            pConn->oorListLocation = NULL;
        } /*if (0 == pConn->oorEvents)*/
        else
        {
            processedEvents |= eventsToRetry;
        }
        TransportConnectionUnLockEvent(pConn);
    } /* while (NULL != hListElem) */

    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
        "NotifyConnectionsOnResources - Done notifying on OOR!"))

    return rv;
}

/***************************************************************************
 * OORListGetNextCell
 * ------------------------------------------------------------------------
 * General: get the next cell from OOR list and removes the previous cell
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr        - transport manager handler
 ***************************************************************************/
static void RVCALLCONV OORListGetNextCell(IN TransportMgr*         pMgr,
                                          IN RLIST_ITEM_HANDLE*    hListElem)
{
	RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
              "OORListGetNextCell - about to remove from OOR list in location 0x%p",*hListElem));
    RvMutexLock(&pMgr->hOORListLock,pMgr->pLogMgr);
    RLIST_Remove(pMgr->oorListPool,pMgr->oorList,*hListElem);
    RLIST_GetHead(pMgr->oorListPool,pMgr->oorList,hListElem);
    RvMutexUnlock(&pMgr->hOORListLock,pMgr->pLogMgr);
}

/***************************************************************************
 * ResendObjEvents
 * ------------------------------------------------------------------------
 * General: Try to send again object event for which
 *            it failed earlier due to no enough resources. The function goes
 *          over the list of object event that failed before
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pMgr        - transport manager handler
 ***************************************************************************/
static void RVCALLCONV ResendObjEvents(IN TransportMgr* pMgr)
{
    RvStatus                     rv;
    RvSipTransportObjEventInfo*  pEventInfo;
    RvSipTransportObjEventInfo*  pTmpEventInfo;

    RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
               "ResendObjEvents"));

    RvMutexLock(&pMgr->hObjEventListLock, pMgr->pLogMgr);

    pEventInfo    = pMgr->pEventToBeNotified;

    while (NULL != pEventInfo )
    {
        if (NULL != pEventInfo)
        {
            RvLogDebug(pMgr->pLogSrc,(pMgr->pLogSrc,
                "ResendObjEvents - process event %p for object 0x%p",
                pEventInfo, pEventInfo->objHandle));

            pTmpEventInfo = pEventInfo->next;
            rv = SipTransportSendTerminationObjectEvent((RvSipTransportMgrHandle)pMgr,
                                            pEventInfo->objHandle,
                                            pEventInfo,
                                            pEventInfo->reason,
                                            pEventInfo->func,
                                            RV_TRUE /*bInternalEvent*/,
                                            "Resend",
                                            "OOR recovery");
            if (RV_OK != rv)
            {
                pEventInfo = NULL;
            }
            else
            {
                pMgr->pEventToBeNotified = pTmpEventInfo;
                pEventInfo = pTmpEventInfo;
            }
        }

    }

    RvMutexUnlock(&pMgr->hObjEventListLock,pMgr->pLogMgr);
}

#ifdef __cplusplus
}
#endif

