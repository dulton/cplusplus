/************************************************************************************************************************
NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

************************************************************************************************************************/



/*********************************************************************************
 *                              <RvSipMid.h>
 *
 * This file contains all API functions relevant to intrer acting with core lever 
 * functionality, including select poll and timers.
 *
 * select/poll 
 * ------------------------
 * Enables an application to register event for select loop or poll queries.
 * Enables an application to perform a select loop or a poll loop
 *
 * timers
 * -----------------
 * Enables an application to set and release timers.
 * Timer can be set on threads that ran one of RADVISION toolkits only, as only those 
 * threads use the RADVISION select() loop.
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                   March 2004
 *********************************************************************************/


#ifndef RV_SIP_MID_H
#define RV_SIP_MID_H

#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RvSipMidTypes.h"
    
/*-------------------------------------------------------------------------*/
/*                           MODULE FUNCTIONS                              */
/*-------------------------------------------------------------------------*/

/************************************************************************************
 * RvSipMidInit
 * ----------------------------------------------------------------------------------
 * General: Starts the Mid layer services. before calling to any of the function 
 *          of this module, the application must call this function
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   -
 * Output:  -
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidInit(void);

/************************************************************************************
 * RvSipMidEnd
 * ----------------------------------------------------------------------------------
 * General: Ends the Mid layer services. 
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   -
 * Output:  -
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidEnd(void);

/************************************************************************************
 * RvSipMidConstruct
 * ----------------------------------------------------------------------------------
 * General: Constructs and initializes the middle layer. This function allocates the
 *          required memory and constructs middle layer objects acording to the given 
 *          configuration. The function returns a handle to the middle layer manager. 
 *          You need this handle in order to use the middle layer APIs.
 *          when finishig using the middle layer call RvSipMidPrepareDestruct() if 
 *          needed and RvSipMidDesstruct() to free all resources.
 *          allocated resources
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidCfg     - Structure containing middle layer configuration parameters.
 *          sizeOfCfg   - The size of the configuration structure.
 * Output:  phMidMgr     - Handle to the middle layer manager.
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidConstruct(
                                         IN    RvInt32               sizeOfCfg,
                                         IN    RvSipMidCfg*          pMidCfg,
                                         OUT   RvSipMidMgrHandle*    phMidMgr);

/************************************************************************************
 * RvSipMidPrepareDestruct
 * ----------------------------------------------------------------------------------
 * General: This function will free all application resources. after calling this 
 *          function, allow enough time for other threads (if there are any) to finish 
 *          any calls to middle functions and call RvSipMidDestruct().
 *          If the application is sure that no other threads wait tp preform actions 
 *          on the mid layer, it can call RvSipMidDestruct() directly.
 *          After calling this function the application is not allowed to set timers 
 *          or register on select events
 *          All Application timers will be released.
 *          All select registrations will be removed.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hMidMgr     - Handle to the middle layer manager
 * Output:
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidPrepareDestruct(IN RvSipMidMgrHandle    hMidMgr);

/************************************************************************************
 * RvSipMidDestruct
 * ----------------------------------------------------------------------------------
 * General: This function will free all manager resources. after calling this 
 *          function the application is not allowed to call on file descriptors 
 *          or set timers.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hMidMgr     - Handle to the middle layer manager
 * Output:
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidDestruct(IN RvSipMidMgrHandle    hMidMgr);
    
/************************************************************************************
 * RvSipMidSetLog
 * ----------------------------------------------------------------------------------
 * General: Sets a log handle to the mid layer. Use this function if mid layed was 
 *          initiated before the stack.
 *          Use RvSipStackGetLogHandle() to get the log handle from the SIP Toolkit.
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hMidMgr   -- handle to the middle layer manager.
 *          hLog      -- log handle
 * Output:  -
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidSetLog(IN RvSipMidMgrHandle     hMidMgr,
                                         IN RV_LOG_Handle         hLog);

/***************************************************************************
 * RvSipMidMemAlloc
 * ------------------------------------------------------------------------
 * General: Allocates memory for application use.
 *          This function can be called only after core services were initialized.
 * Return Value: Pointer to newly allocated memory if successful.
 *               NULL o/w.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     size   - Allocation size in bytes.
 ***************************************************************************/
RVAPI void* RVCALLCONV RvSipMidMemAlloc(IN RvInt32 size);


/***************************************************************************
 * RvSipMidMemFree
 * ------------------------------------------------------------------------
 * General: Frees memory allocated by RvSipMemAlloc().
 *          Only memory allocation that were done using RvSipMemAlloc() should
 *          be freed with this function.
 * Return Value: (-).
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     memptr     - memory to free.
 ***************************************************************************/
RVAPI void RVCALLCONV RvSipMidMemFree(IN void* memptr);


/***************************************************************************
 * RvSipMidTimeInMilliGet
 * ------------------------------------------------------------------------
 * General: Gets a timestamp value in milliseconds.
 * Return Value: The timestamp value in milliseconds.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     
 ***************************************************************************/
RVAPI RvUint32 RVCALLCONV RvSipMidTimeInMilliGet(void);

/***************************************************************************
 * RvSipMidTimeInSecondsGet
 * ------------------------------------------------------------------------
 * General: Gets a timestamp value in seconds.
 * Return Value: The timestamp value in seconds.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     
 ***************************************************************************/
RVAPI RvUint32 RVCALLCONV RvSipMidTimeInSecondsGet(void);

/************************************************************************************
 * RvSipMidTimerSet
 * ----------------------------------------------------------------------------------
 * General: Sets a new timer.
 *          when a timer expires, the resources it consumes will be released automaticlly.
 *          it is forbidden to call RvSipMidTimerReset() in the timer callback.
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hMidMgr     -- handle to the middle layer manager.
 *          miliTimeOut -- experation specified in miliseconds
 *          cb          -- application callback
 *          ctx         -- context to be called when timer expires
 * Output:  phTimer     -- a newly allocated timer
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidTimerSet(IN RvSipMidMgrHandle       hMidMgr,
                                    IN RvUint32                 miliTimeOut,
                                    IN RvSipMidTimerExpEv       cb,
                                    IN void*                    ctx,
                                    OUT RvSipMidTimerHandle*    phTimer);

/************************************************************************************
 * RvSipMidTimerReset
 * ----------------------------------------------------------------------------------
 * General: releases a timer
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidMgr   -- pointer to the middle layer manager.
 *          hTimer    -- handle of timer to delete
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidTimerReset(IN RvSipMidMgrHandle       hMidMgr,
                                             IN RvSipMidTimerHandle    hTimer);

/************************************************************************************
 * RvSipMidTimerGetNextExpiration
 * ----------------------------------------------------------------------------------
 * General: get the time left untill the next timer expiration 
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidMgr         -- pointer to the middle layer manager.
 * Output:  pExpirationTime -- time left untill the next timer expiration
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidTimerGetNextExpiration(IN RvSipMidMgrHandle       hMidMgr,
                                                         OUT RvUint32*              pExpirationTime);

/************************************************************************************
 * RvSipMidTimerGetTimerExpiration
 * ----------------------------------------------------------------------------------
 * General: get the time left untill the timer expiration 
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pMidMgr         -- pointer to the middle layer manager.
 *          hTimer          -- handle of timer to get info on
 * Output:  pExpirationTime -- time left untill the timer expiration
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidTimerGetTimerExpiration(IN RvSipMidMgrHandle       hMidMgr,
                                                          IN RvSipMidTimerHandle    hTimer,
                                                          OUT RvUint32*              pExpirationTime);

/************************************************************************************
 * RvSipMidSelectCallOn
 * ----------------------------------------------------------------------------------
 * General: registers a file descriptor to the select loop. You can register to 
 *          listen on read or write events, and provide a callback that will be called 
 *          when the select exits due to activity on that file descriptor
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hMidMgr   -- handle to the middle layer manager.
 *          fd        -- OS file descriptor
 *          events    -- read/write
 *          pCallBack -- user callback
 *          ctx       -- usr context
 * Output:  
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidSelectCallOn(IN RvSipMidMgrHandle    hMidMgr,
                                               IN RvInt32              fd,
                                               IN RvSipMidSelectEvent  events,
                                               IN RvSipMidSelectEv     pCallBack,
                                               IN void*                ctx);

/******************************************************************************
 * RvSipMidSelectSetMaxDescs
 * ------------------------------------------------------------------------
 * General: Set the amount of file descriptors that the Select module can handle in a single
 *          select engine. This is also the value of the highest file descriptor possible.
 *          This function MUST be called before stack/middle layer initialization
 * Return Value: RV_OK on success, error code otherwise
 *               (see RV_SIP_DEF.h, rverror.h)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  maxDescs - Maximum value of file descriptor possible
 * Output: - 
 *****************************************************************************/
RVAPI RvStatus RvSipMidSelectSetMaxDescs(IN RvUint32 maxDescs);

/******************************************************************************
 * RvSipMidSelectGetMaxDesc
 * ------------------------------------------------------------------------
 * General: Get the current value used as the maximum value for a file descriptor 
 *          by the select procedures.
 * Return Value: RV_OK on success, error code otherwise
 *               (see RV_SIP_DEF.h, rverror.h)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  -
 * Output: pMaxFds - place to store maximun number of file descriptors
 *****************************************************************************/
RVAPI RvStatus RvSipMidSelectGetMaxDesc(OUT RvUint32 *pMaxFds);

#if defined(FD_SETSIZE)
#if (RV_SELECT_TYPE == RV_SELECT_SELECT)
/******************************************************************************
 * RvSipMidSelectGetEventsRegistration
 * ------------------------------------------------------------------------
 * General: Get Select file descriptors for select operation
 *          NOTE: 1. This function cann be called only in the thread that 
 *                   initiated the stack.
 *                2. This function resets the fd sets that was given to it.
 *                   if you intent to add file descriptors to the fd sets do 
 *                   so AFTER calling this function
 *
 * Return Value: RV_OK on success, error code otherwise
 *               (see RV_SIP_DEF.h, rverror.h)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMidMgr         - handle to the middle layer manager
 *         fdSetLen        - the length of the file descriptors set
 * Output: pMaxfd          - maximum number of file descriptors
 *         rdSet           - pointer to read file descriptor set
 *         wrSet           - pointer to write file descriptor set
 *         exSet           - pointer to exeption file descriptor set (reserved for future use)
 *         pTimeOut        - time out the stack want to give to select. (-1 means ingfinite)      
 *****************************************************************************/
RVAPI RvStatus RvSipMidSelectGetEventsRegistration(IN  RvSipMidMgrHandle               hMidMgr,
                                                    IN  RvInt                           fdSetLen,
                                                    OUT RvInt*                          pMaxfd,
                                                    OUT fd_set*                         rdSet,
                                                    OUT fd_set*                         wrSet,
                                                    OUT fd_set*                         exSet,
                                                    OUT RvUint32*                       pTimeOut);

/******************************************************************************
 * RvSipMidSelectEventsHandling
 * ------------------------------------------------------------------------
 * General: Handle events for sockets opened by Stack
 *          NOTE: this function can be called only in the thread that 
 *          initiated the stack
 * Return Value: RV_OK on success, error code otherwise
 *               (see RV_SIP_DEF.h, rverror.h)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMidMgr   - handle to the middle layer manager
 *         rdSet     - not in use since SIP TK 4.5 GA3
 *         wrSet     - not in use since SIP TK 4.5 GA3
 *         exSet     - not in use since SIP TK 4.5 GA3
 *         numFds    - not in use since SIP TK 4.5 GA3
 *         numEvents - not in use since SIP TK 4.5 GA3
 *****************************************************************************/
RVAPI RvStatus RvSipMidSelectEventsHandling(IN RvSipMidMgrHandle  hMidMgr,
                                            IN fd_set*            rdSet,
                                            IN fd_set*            wrSet,
                                            IN fd_set*            exSet,
                                            IN RvInt              numFds,
                                            IN RvInt              numEvents);
#endif /*(RV_SELECT_TYPE == RV_SELECT_SELECT)*/
#endif  /* #if defined(FD_SETSIZE)*/

#if (RV_SELECT_TYPE == RV_SELECT_POLL)
struct pollfd; /* This line is here to remove warnings from the compilation process */

/******************************************************************************
 * RvSipMidPollGetEventsRegistration
 * ------------------------------------------------------------------------
 * General: Get Select file descriptors for select operation
 *          NOTE: 1. This function cann be called only in the thread that 
 *                   initiated the stack.
 *                2. This function resets the fd sets that was given to it.
 *                   if you intent to add file descriptors to the fd sets do 
 *                   so AFTER calling this function
 *
 * Return Value: RV_OK on success, error code otherwise
 *               (see RV_SIP_DEF.h, rverror.h)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMidMgr         - handle to the middle layer manager
 *         len             - the length of the poll fd set
 * Output: pollFdSet       - pointer poll fd set to poll 
 *         pNum            - the number of file descriptors on the poll set      
 *         pTimeOut        - time out the stack want to give to poll. (-1 means ingfinite)      
 *****************************************************************************/
RVAPI RvStatus RvSipMidPollGetEventsRegistration(IN RvSipMidMgrHandle               hMidMgr,
                                                IN  RvInt           len,
                                                OUT struct pollfd*  pollFdSet,
                                                OUT RvInt*          pNum,
                                                OUT RvUint32*       pTimeOut);

/******************************************************************************
 * RvSipMidPollEventsHandling
 * ------------------------------------------------------------------------
 * General: Handle events gotten from poll procedure
 *          NOTE: this function can be called only in the thread that 
 *          initiated the stack
 *
 * Return Value: RV_OK on success, error code otherwise
 *               (see RV_SIP_DEF.h, rverror.h)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:  hMidMgr         - handle to the middle layer manager
 *         num             - number of file descriptors
 *         pollFdSet       - pointer poll fd set from poll 
 * Output: 
 *           
 *****************************************************************************/
RVAPI RvStatus RvSipMidPollEventsHandling(IN RvSipMidMgrHandle  hMidMgr,
                                            IN struct pollfd*     pollFdSet,
                                            IN RvInt              num,
                                            IN RvInt              numEvents);
#endif  /* (RV_SELECT_TYPE == RV_SELECT_POLL) */


/************************************************************************************
 * RvSipMidMainThreadClean
 * ----------------------------------------------------------------------------------
 * General: Releases all resources taken by the main thread.
 *          The application should call this function if it wishes to clean all the 
 *          resources taken by SIP Stack main thread.This function should be called 
 *          only after ALL RADVISION's toolkits have been destructed and before
 *          calling to RvSipMidEnd().
 *          In general a good function call sequence that will clear all resources
 *          will be:
 *          1. RvSipMidInit()
 *          2. RvSipStackConstruct()
 *          ......
 *          3. RvSipStackDestruct()
 *          4. RvSipMidMainThreadClean()
 *          5. RvSipMidEnd().
 *
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   -
 * Output:  -
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidMainThreadClean(void);

/***************************************************************************
 * RvSipMidEncodeB64 
 * ------------------------------------------------------------------------
 * General: Performs a 64 bit encoding operation of a given number of bytes 
 *          in a given buffer.
 * Return Value: The number of used bytes in the 'outTxt' buffer or -1 if 
 *          the function fails.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    inTxt - the buffer to be encoded.
 *           inLen - the length of buffer to be encoded
 *           outTxt - an encoding destination buffer
 *           outLen - the size of 'outTxt' buffer
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipMidEncodeB64 (IN RvUint8* inTxt,   
                                             IN RvInt    inLen,      
                                             INOUT RvUint8* outTxt, 
                                             IN RvInt    outLen);
/***************************************************************************
 * RvSipMidDecodeB64 
 * ------------------------------------------------------------------------
 * General: Performs a 64 bit decoding operation of a given number of bytes 
 *          in a given buffer..
 * Return Value: The number of used bytes in the 'outTxt' buffer or -1 if 
 *        the function fails
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    inTxt - the buffer to be decoded.
 *           inLen - the length of buffer to be decoded
 *           outTxt - a decoding destination buffer.
 *           outLen - the size of the 'outTxt' buffer
 ***************************************************************************/
RVAPI RvInt32 RVCALLCONV RvSipMidDecodeB64(IN RvUint8* inTxt,  
                                            IN RvInt    inLen,     
                                            INOUT RvUint8* outTxt, 
                                            IN RvInt    outLen);

/******************************************************************************
 * RvSipMidTlsSetLockingCallback
 * ----------------------------------------------------------------------------
 * General: The RADVISION SIP Stack uses OpenSSL library in order to provide
 *          the application with the TLS ability. OpenSSL library forces
 *          the modules run above it to manage locks. That means, the OpenSSL
 *          asks the modules to lock or unlock locks on behalf of the OpenSSL.
 *          This request is performed using special callback, which should be
 *          implemented in the modules, and which is called by the OpenSSL.
 *          If the callback is not set into the OpenSSL library, OpenSSL
 *          doesn't lock shared objects. This means, no multithread safety is
 *          provided.
 *              By default the RADVISION SIP Stack sets the callback into
 *          the OpenSSL library on construction and removes it on destruction.
 *          See RV_TLS_AUTO_SET_OS_CALLBACKS macro in rvusrconfig.h.
 *          It is defined as RV_YES by default.
 *              But if the Application has another modules in addition to
 *          the RADVISION SIP Stack that accesses the OpenSSL library,
 *          it should define the RV_TLS_AUTO_SET_OS_CALLBACKS macro as RV_NO.
 *          Now the Application can implements it's own callback, or it can use
 *          the RADVISION Stacks's implementation.
 *          Pay attention, the RADVISION implementation will not be available
 *          after the RADVISION Stack is destructed.
 *              To activate or to stop the RADVISION SIP Stack implementation
 *          of the locking callback, the RvSipMidTlsSetLockingCallback()
 *          function should be called. It just sets the callback into
 *          the OpenSSL library or remove it.
 * Return Value: RV_OK - on success
 *               RV_ERROR_NOTSUPPORTED   - if TLS is not supported
 *               RV_ERROR_ILLEGAL_ACTION - if callback is set automatically
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:    bSet - If RV_TRUE the callback will be set.
 *                  Otherwise the callback will be removed.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidTlsSetLockingCallback(RvBool bSet);

/******************************************************************************
 * RvSipMidTlsSetThreadIdCallback
 * ----------------------------------------------------------------------------
 * General: The RADVISION SIP Stack uses OpenSSL library in order to provide
 *          the application with the TLS ability. To ensure multithread safety,
 *          every time the OpenSSL needs to know ID of the current thread,
 *          it asks modules run above for this ID.
 *          This request is performed using special callback, which should be
 *          implemented in the modules, and which is called by the OpenSSL.
 *          If the callback is not set into the OpenSSL library, OpenSSL
 *          doesn't know the thread ID. As a result, no multithread safety can
 *          be ensured.
 *              By default the RADVISION SIP Stack sets the callback into
 *          the OpenSSL library on construction and removes it on destruction.
 *          See RV_TLS_AUTO_SET_OS_CALLBACKS macro in rvusrconfig.h.
 *          It is defined as RV_YES by default.
 *              But if the Application has another modules in addition to
 *          the RADVISION SIP Stack that accesses the OpenSSL library,
 *          it should define the RV_TLS_AUTO_SET_OS_CALLBACKS macro as RV_NO.
 *          Now the Application can implements it's own callback, or it can use
 *          the RADVISION Stacks's implementation.
 *          Pay attention, the RADVISION implementation will not be available
 *          after the RADVISION Stack is destructed.
 *              To activate or to stop the RADVISION SIP Stack implementation
 *          of the Thread ID callback, the RvSipMidTlsSetThreadIdCallback()
 *          function should be called. It just sets the callback into
 *          the OpenSSL library or remove it.
 * Return Value: RV_OK - on success
 *               RV_ERROR_NOTSUPPORTED   - if TLS is not supported
 *               RV_ERROR_ILLEGAL_ACTION - if callback is set automatically
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:    bSet - If RV_TRUE the callback will be set.
 *                  Otherwise the callback will be removed.
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidTlsSetThreadIdCallback(RvBool bSet);

/******************************************************************************
 * RvSipMidAttachThread
 * ----------------------------------------------------------------------------
 * General: Indicate that a thread created by the application is about to use
 *          the SIP toolkit's API functions. 
 *          The function should be called within the attached thread.
 *			Without a call to this function, calls to API functions might cause a crash.
 *          On thread destruction, application must call RvSipMidDetachThread()
 *          in order to remove any memory allocated by the toolkit for this thread.
 *
 * Return Value: Returns RvStatus
 *               RV_ERROR_ILLEGAL_ACTION - if the thread was already attached.
 *               RV_OK  - if successful
 * ----------------------------------------------------------------------------
 * Arguments: 
 * Input:   hMidMgr    - handle to the middle layer manager.
 *			threadName - Name to give this thread in the logs
 * Output:  
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidAttachThread(IN RvSipMidMgrHandle    hMidMgr,
											   IN const RvChar         *threadName);

/******************************************************************************
 * RvSipMidDetachThread
 * ----------------------------------------------------------------------------
 * General: Indicate that a thread created by the user is being shut down.
 *          This function should be called to remove any memory allocated by
 *          the toolkit for this thread (memory which was allocated when calling
 *          to RvSipMidAttachThread()).
 *          The function should be called within the thread to detach.
 *
 * Return Value: RV_OK  - if successful.
 *               Other on failure
 * ----------------------------------------------------------------------------
 * Arguments: 
 * Input:   hMidMgr    - handle to the middle layer manager.
 * Output:  
 *****************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidDetachThread(IN RvSipMidMgrHandle    hMidMgr);

/************************************************************************************
 * RvSipMidGetResources
 * ----------------------------------------------------------------------------------
 * General: Gets the status of resources used by the Mid layer. 
 *
 * Return Value: Returns RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hMidMgr    - handle to the middle layer manager.
 *          pResources - The resources in use by the Mid layer.
 * Output:  none
 ***********************************************************************************/
RVAPI RvStatus RVCALLCONV RvSipMidGetResources(
                                              IN    RvSipMidMgrHandle   hMidMgr,
                                              INOUT RvSipMidResources  *pResources);

#ifdef __cplusplus
}
#endif

#endif /*RV_SIP_MID_H*/
