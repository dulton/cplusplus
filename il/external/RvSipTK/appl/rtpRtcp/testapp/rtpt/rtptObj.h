#ifndef _RV_RTPEPOBJ_H_
#define _RV_RTPEPOBJ_H_

#include "rtptSession.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RTP_TEST_APPLICATION_SESSIONS_MAX (50)
/*typedef struct rtptObj_Tag rtptObj;*/
	
typedef enum
{
	RtptResourceCall,           /* Call object */
	RtptResourceChannel,        /* Channel object */
    RtptResourceCompletionService,
    RtptResourceInConnection,
    RtptResourceOutConnection
} RtptResourceType;

/******************************************************************************
* rtptMallocEv
* ----------------------------------------------------------------------------
* General: Dynamic allocation function to call (the equivalent of malloc()).
*
* Return Value: Pointer to allocated memory on success, NULL on failure.
* ----------------------------------------------------------------------------
* Arguments:
* Input:  rtpt         - Endpoint object to use.
*         size         - Size of allocation requested.
* Output: None.
*****************************************************************************/
typedef void* (*rtptMallocEv)(
	IN rtptObj              *rtpt,
	IN RvSize_t             size);


/******************************************************************************
* rtptFreeEv
* ----------------------------------------------------------------------------
* General: Request user to allocate a resource identifier for an object.
*
* Return Value: RV_OK on success, other on failure.
* ----------------------------------------------------------------------------
* Arguments:
* Input:  rtpt         - Endpoint object to use.
*         ptr          - Pointer of memory to free.
* Output: None.
*****************************************************************************/
typedef RvStatus (*rtptFreeEv)(
	IN rtptObj              *rtpt,
	IN void                 *ptr);


/******************************************************************************
* rtptAllocateResourceIdEv
* ----------------------------------------------------------------------------
* General: Request user to allocate a resource identifier for an object.
*
* Return Value: Allocated resource on success, negative value on failure.
* ----------------------------------------------------------------------------
* Arguments:
* Input:  rtpt         - Endpoint object to use.
*         resourceType - Type of resource we need id for.
* Output: None.
*****************************************************************************/
typedef RvInt32 (*rtptAllocateResourceIdEv)(
    IN rtptObj          *rtpt,
    IN RtptResourceType resourceType);

/******************************************************************************
 * rtptLogEv
 * ----------------------------------------------------------------------------
 * General: Indication of a message that can be logged somewhere.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  rtpt             - Endpoint object to use.
 *         call             - Call this event belongs to (NULL if none).
 *         logMessage       - Log message.
 * Output: None.
 *****************************************************************************/
typedef void (*rtptLogEv)(
    IN rtptObj              *rtpt,
    IN rtptSessionObj       *call,
    IN const RvChar         *logMessage);
    
/******************************************************************************
 * rtptEventIndication_CB
 * ----------------------------------------------------------------------------
 * General: Indication of a message being sent or received by the endpoint.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  term             - Terminal object to use.
 *         eventType        - Type of the event indicated.
 *         call             - Call this event belongs to (NULL if none).
 *         eventStr         - Event string.
 * Output: None.
 *****************************************************************************/
typedef void (*rtptEventIndication_CB)(
    IN rtptObj        *rtpt,
    IN const RvChar *eventType,
    IN rtptSessionObj *session,
    IN RvChar *msg);

/* List of this module's callbacks */
typedef struct
{
    rtptMallocEv                  rtptMalloc;
    rtptFreeEv                    rtptFree;
    rtptAllocateResourceIdEv      rtptAllocateResourceId;
    rtptLogEv                     rtptLog;
  
    rtptEventIndication_CB        rtptEventIndication;
} rtptCallbacks;	



	
typedef struct rtptObj_Tag
{
   rtptSessionObj          session[RTP_TEST_APPLICATION_SESSIONS_MAX];  /* RTP sessions db */
   rtptEncryptors          encryptors;
   rtptCallbacks           cb; /* Callbacks implemented */

   /* Logger */
   RvRtpLogger             rtptLogger;
   RvBool                  appLogSourceInited; /* indicator of inited RTPT log source */
   RvLogSource             appLogSource;       /* log source of RTPT module */
   RvBool                  resetError; /* Do we need to reset the error string? */
   RvChar                  lastError[256]; /* Last error string we know of */

   /* Resources */
   RvUint32                curAllocs;
   RvUint32                maxAllocs;
   RvUint32                curMemory;
   RvUint32                maxMemory;  

} rtptObj_t;

/********************************************************************************************
 * rtptStackEnd
 * purpose : ends working with the RTPT object
 * input   : pointer to rtptObj
 * output  : none
 * return  : RV_OK
 ********************************************************************************************/
RvStatus rtptStackEnd (IN rtptObj       *ta);

/********************************************************************************************
 * rtptStackInit
 * purpose : initializes the test application RTPT object
 * input   : pointer to rtptObj
 * output  : none
 * return  : RV_OK
 ********************************************************************************************/
RvStatus rtptStackInit(IN rtptObj       *ta);

/********************************************************************************************
 * rtptGetLogSource
 * purpose : Accessor to the TA log source
 * input   : none
 * output  : none
 * return  : pointer to the application log source
 ********************************************************************************************/
RvLogSource* rtptGetLogSource(rtptObj* rtptPtr);

#ifdef __cplusplus
}
#endif



#endif /* _RV_RTPEPOBJ_H_ */

