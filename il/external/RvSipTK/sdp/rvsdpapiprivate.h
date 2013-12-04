/******************************************************************************
  Filename    : rvsdpapiprivate.h
  Description : definition of API functions that are used directly by library user
                but via macros.
  ******************************************************************************
  Copyright (c) 2005 RADVision Inc.
  ************************************************************************
  NOTICE:
  This document contains information that is proprietary to RADVision LTD.
  No part of this publication may be reproduced in any form whatsoever
  without written prior approval by RADVision LTD..
  
    RADVision LTD. reserves the right to revise this publication and make
    changes without obligation to notify any person of such revisions or
    changes.
 Author:Rafi Kiel
******************************************************************************/

#ifndef _rvsdpapiprivate_h_
#define _rvsdpapiprivate_h_

#ifdef __cplusplus
extern "C" {
#endif
    

#include "rvsdpenums.h"
#include "rvsdplist.h"
#include "rvsdpstrings.h"
#include  "rvsdpdatastruct.h"

#if (RV_OS_TYPE == RV_OS_TYPE_NETBSD || RV_OS_TYPE == RV_OS_TYPE_MAC)
#include <stdlib.h>
#endif
    

#define RV_SDP_KEY_SET_USED(_k)     (_k)->iKeyIsUsed = 456234
#define RV_SDP_KEY_SET_NOT_USED(_k) (_k)->iKeyIsUsed = 0
#define RV_SDP_KEY_IS_USED(_k)      ((_k)->iKeyIsUsed == 456234)

#define RV_SDP_ORIGIN_SET_USED(_o)     (_o)->iOriginIsUsed = 456234
#define RV_SDP_ORIGIN_SET_NOT_USED(_o) (_o)->iOriginIsUsed = 0
#define RV_SDP_ORIGIN_IS_USED(_o)      ((_o)->iOriginIsUsed == 456234)

#define RV_SDP_EMPTY_STRING(_x)		(((_x) == NULL) ? (const char*)"" : (const char*)(_x))
#define RV_SDP_STRING_NOT_SET(_x)  ((((_x) == NULL) || (*(_x) == 0)) ? "NotSet" : (_x))

#define RV_SDP_STR_2_NUM(_x) (RV_TRUE == rvSdpIsInteger((_x)) ? atoi((_x)) : RV_SDP_UNDEFINED)
/*#define RV_SDP_STR_2_NUM(_x) (RV_TRUE == RV_SDP_IS_DIGIT((_x)) ? atoi((_x)) : RV_SDP_UNDEFINED)*/


#define RV_SDP_PORT_NOTSET        -1 /* for '-' values */
#define RV_SDP_PORT_IGNORE        -2 /* for '-' values */
#define RV_SDP_PORT_ANY           -3 /* for '$' values */
#define RV_SDP_PORT_ATM           -4 /* when VcId supported */

/* for backward compatibility */
#define RV_SDPPORT_IGNORE        RV_SDP_PORT_IGNORE
#define RV_SDPPORT_ANY           RV_SDP_PORT_ANY
#define RV_SDPPORT_ATM           RV_SDP_PORT_ATM
    

struct _RvSdpAttribute;
struct _RvSdpMsg;
struct _RvSdpParserData;
struct _RvSdpCommonFields;
struct _RvSdpMediaDescr;
struct _RvSdpSessionTime;
struct _RvSdpRtpMap;
struct _RvSdpInformation;

/***************************************************************************
 * rvSdpSpecAttrGetValue
 * ------------------------------------------------------------------------
 * General: 
 *      Gets the value of a special attribute;
 *      The value is first calculated using the appropriate callback 
 *      (iGetValueFunc) in RvSdpSpecAttributeData structure and then stored 
 *      in iAttrValue field of the RvSdpAttribute instance.
 *      The iAttrValue is returned 
 *          
 * Return Value: 
 *      The value of iAttrValue of RvSdpAttribute is returned.
 * ------------------------------------------------------------------------
 * Arguments:
 *      attr - pointer to RvSdpAttribute instance.
 ***************************************************************************/
RVSDPCOREAPI const char* rvSdpSpecAttrGetValue(
			struct _RvSdpAttribute *attr);

/***************************************************************************
 * rvSdpSetTextField
 * ------------------------------------------------------------------------
 * General: 
 *      Allocate and copy string.
 *      If obj points to RvSdpMsg instance the allocation will be performed 
 *      in message strings buffer and the pointer within the strings buffer 
 *      will be returned. Otherwise obj must point to RvAlloc allocator which
 *      will be used for memory allocation.
 *          
 * Return Value: 
 *      Returns RV_SDPSTATUS_OK if the function succeeds, or an error code if the 
 *      function fails.
 * ------------------------------------------------------------------------
 * Arguments:
 *      str - will point to allocated memory.
 *      obj - points to RvSdpMsg or RvAlloc instance.
 *      value - string to copy.
 ***************************************************************************/
RVSDPCOREAPI RvSdpStatus rvSdpSetTextField(
            RvSdpString* str,
            void *obj,
            const char* value);

/***************************************************************************
 * rvSdpUnsetTextField
 * ------------------------------------------------------------------------
 * General: 
 *      Deallocates the string within the RvSdpMessage strings buffer or
 *      using the allocator.
 *          
 * Return Value: 
 *      None.
 * ------------------------------------------------------------------------
 * Arguments:
 *      str - pointer to memory to be freed.
 *      obj - points to RvSdpMsg or RvAlloc instance.
 ***************************************************************************/
RVSDPCOREAPI void rvSdpUnsetTextField(
            RvSdpString* str,
            void *obj);


/***************************************************************************
 * rvSdpAddTextToArray
 * ------------------------------------------------------------------------
 * General: 
 *      Allocates new string (in RvSdpMsg strings buffer or using the alloctor)
 *      and adds the allocated string as the last entry in strings array.
 *          
 * Return Value: 
 *      Returns RV_SDPSTATUS_OK if the function succeeds, or an error code if the 
 *      function fails.
 * ------------------------------------------------------------------------
 * Arguments:
 *      num - the current lenght of strings  array will be increased by one.
 *      maxNum - the max size of strings array.
 *      txtArr - the array of strings.
 *      obj - points to RvSdpMsg or RvAlloc instance.
 *      val - the string to copy to strinsg array.
 ***************************************************************************/
RVSDPCOREAPI RvSdpStatus rvSdpAddTextToArray(
            RvUint16* num,
            int maxNum,
            char** txtArr,
            void* obj,
            const char* val);

/***************************************************************************
 * rvSdpRemoveTextFromArray
 * ------------------------------------------------------------------------
 * General: 
 *      Removes and deallocates the string in the array of strings. The 
 *      de-allocation is performed in RvSdpMsg strings buffer or using the
 *      allocator. If the removed string is not the last entry in the array
 *      all next entries are moved backwards to keep the array continuous.
 *          
 * Return Value: 
 *      None.
 * ------------------------------------------------------------------------
 * Arguments:
 *      num - the current lenght of strings  array will be decreased by one.
 *      txtArr - the array of strings.
 *      obj - points to RvSdpMsg or RvAlloc instance.
 *      index - the index of array entry to remove.
 ***************************************************************************/
RVSDPCOREAPI void rvSdpRemoveTextFromArray(
            RvUint16* num,
            char** txtArr,
            void* obj,
            RvSize_t index);

/***************************************************************************
 * rvSdpRemoveTextFromArray
 * ------------------------------------------------------------------------
 * General: 
 *      Deallocates all strings in the array of strings. The 
 *      de-allocation is performed in RvSdpMsg strings buffer or using the
 *      allocator.
 *          
 * Return Value: 
 *      None.
 * ------------------------------------------------------------------------
 * Arguments:
 *      num - the current lenght of strings array will be set to zero.
 *      txtArr - the array of strings.
 *      obj - points to RvSdpMsg or RvAlloc instance.
 ***************************************************************************/
RVSDPCOREAPI void rvSdpClearTxtArray(
            RvUint16* num,
            char** txtArr,
            void* obj); 

/***************************************************************************
 * RvSdpMsgConstruct2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs the instance of RvSdpMsg, initializes all internal fields,
 *      allocates memory for the strings buffer and pools of reusable objects 
 *          
 * Return Value: 
 *      valid RvSdpMsg pointer on success or NULL on failure
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - pointer to RvSdpMsg instance to be constructed, if the value is 
 *            NULL the instance will be allocated within the function.
 *      a  - allocator to be used for memory allocations, if allocator is NULL
 *           the default allocator is used.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpMsg* rvSdpMsgConstruct2(
            struct _RvSdpMsg* msg, 
            RvAlloc* a);

/***************************************************************************
 * rvSdpMsgCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpMsg from 'src' to 'dest'. The destination
 *      object can be constructed depending on the value of 'dontConstruct' 
 *      argument. If the destination object is NULL pointer, the destination
 *      object will be allocated within the function.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpMsg object, or NULL if the function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpMsg object or NULL. 
 *      src - a pointer to the source object
 *      a  - allocator to be used for memory allocations, if allocator is NULL
 *           the default allocator is used.
 *      dontConstruct - whether the 'dest' should be constructed.
 *      sessDescrOnly - if RV_TRUE, only session description fields will be
 *                       copied, otherwise - media descriptors will be copied
 *                       also.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpMsg* rvSdpMsgCopy2(
            struct _RvSdpMsg* dest,
            const struct _RvSdpMsg* src,
            RvAlloc* a,
            RvBool   dontConstruct,
            RvBool   sessDescrOnly);

/***************************************************************************
 * rvSdpMsgSetVersion2
 * ------------------------------------------------------------------------
 * General: 
 *      Sets the version field of the SDP message.
 *          
 * Return Value: 
 *      Returns RV_SDPSTATUS_OK if the function succeeds, or an error code if the 
 *      function fails.
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - a pointer to the RvSdpMsg object.
 *      version - the new version value.
 ***************************************************************************/
RVCOREAPI RvSdpStatus rvSdpMsgSetVersion2(
            struct _RvSdpMsg* msg,
            const char* version);

/***************************************************************************
 * rvSdpMsgSetURI2
 * ------------------------------------------------------------------------
 * General: 
 *      Sets the URI field of the SDP message.
 *          
 * Return Value: 
 *      Returns RV_SDPSTATUS_OK if the function succeeds, or an error code if the 
 *      function fails.
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - a pointer to the RvSdpMsg object.
 *      uri - the new URI value.
 *      badSyntax - whether the URI value is proprietary formatted.
 ***************************************************************************/
RVSDPCOREAPI RvSdpStatus rvSdpMsgSetURI2(
            struct _RvSdpMsg* msg,
            const char* uri,
            RvBool badSyntax);

/***************************************************************************
 * rvSdpSetSdpInformation
 * ------------------------------------------------------------------------
 * General: 
 *      Sets the information field of the SDP message or media descriptor.
 *          
 * Return Value: 
 *      Returns RV_SDPSTATUS_OK if the function succeeds, or an error code if the 
 *      function fails.
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - the RvSdpInformation object of message or media.
 *      info - the new information value.
 *      msg - a pointer to the RvSdpMsg object.
 ***************************************************************************/
RVSDPCOREAPI RvSdpStatus rvSdpSetSdpInformation(
            struct _RvSdpInformation* dest, 
            const char* info, 
            struct _RvSdpMsg *msg);

/***************************************************************************
 * rvSdpDestroySdpInformation
 * ------------------------------------------------------------------------
 * General: 
 *      Destroys the information field of the SDP message or media descriptor.
 *          
 * Return Value: 
 *      None
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - the RvSdpInformation object of message or media.
 *      msg - a pointer to the RvSdpMsg object.
 ***************************************************************************/
RVSDPCOREAPI void rvSdpDestroySdpInformation(
            struct _RvSdpInformation* dest, 
            struct _RvSdpMsg* msg);


/***************************************************************************
 * rvSdpMediaDescrConstructEx
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs the RvSdpMediaDescr object.
 *          
 * Return Value: 
 *      A pointer to the constructed object, or NULL if the function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - pointer to valid RvSdpMsg object or NULL. If the parameter is not
 *            NULL and 'descr' is NULL the media descriptor will be allocated from
 *            the 'msg' pool of media descriptors. If 'msg' is not NULL the constructed
 *            media will be appended to 'msg' list of media descriptors. If the 'msg'
 *            is NULL the 'a' allocator will be used.
 *      descr - a pointer to valid RvSdpMediaDescr object or NULL.
 *      mediaType - the type of media.
 *      port - the media's port or special MEGACO values RV_SDP_PORT_IGNORE/RV_SDP_PORT_ANY/RV_SDP_PORT_ATM.
 *      protocol - the media's protocol.
 *      badSyn - the proprietary formatted media field or NULL if standard media is 
 *               constructed.
 *      a  - allocator to be used for memory allocations, if allocator is NULL
 *           the default allocator is used.
 *      dontConstruct - if set to RV_TRUE the 'descr' must point to valid & constructed
 *                      RvSdpMediaDescr object. This parameter (with RV_TRUE value) 
 *                      is used when objects are copied without construction.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpMediaDescr* rvSdpMediaDescrConstructEx(
            struct _RvSdpMsg* msg,
            struct _RvSdpMediaDescr* descr,
            RvSdpMediaType mediaType,
            RvInt32 port,
            RvSdpProtocol protocol,
            const char* badSyn,
            RvAlloc* a,
            RvBool dontConstruct);

/***************************************************************************
 * rvSdpMediaDescrCopyEx
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpMediaDescr from 'src' to 'dest'. 
 *      If the destination object is NULL pointer, the destination
 *      object will be allocated & constructed within the function.
 *      The destination object will or willl not be constructed depending
 *      on 'dontConstruct' value. If 'dontConstruct' is true the 'dest'
 *      must point to valid & constructed object.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpMediaDescr object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpMediaDescr object or NULL. 
 *      src - a pointer to the source object.
 *      obj - the RvSdpMsg instance that will own the destination object or the
 *            allocator pointer.
 *      dontConstruct - whether the destination object has to be constructed
 *                      prior to copy.
 *      mediaOnly - if set to true only fields regarding media line itself will
 *                  be copied, otherwise also object owned by media descriptors
 *                  will be copied as well.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpMediaDescr* rvSdpMediaDescrCopyEx(
            struct _RvSdpMediaDescr* dest, 
            const struct _RvSdpMediaDescr* src, 
            void* obj,
            RvBool dontConstruct,
            RvBool mediaOnly);

/***************************************************************************
 * rvSdpSessionTimeConstruct2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs the RvSdpSessionTime object.
 *          
 * Return Value: 
 *      A pointer to the constructed object, or NULL if the function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - pointer to valid RvSdpMsg object or NULL. If the parameter is not
 *            NULL and 'sessTime' is NULL the sesion time will be allocated from
 *            the 'msg' pool of session time objectss. If 'msg' is not NULL 
 *            the constructed session time  will be appended to 'msg' list of  
 *            session times.
 *            If the 'msg' is NULL the 'a' allocator will be used.
 *      sessTime - a pointer to valid RvSdpSessionTime object or NULL.
 *      start - the start time of the session.
 *      end - the end time of the session.
 *      badSyn - the proprietary formatted session time field or NULL if standard 
 *               session time is constructed.
 *      a  - allocator to be used for memory allocations, if allocator is NULL
 *           the default allocator is used.
 *      dontConstruct - if set to RV_TRUE the 'sessTime' must point to valid & constructed
 *                      RvSdpSessionTime object. This parameter (with RV_TRUE value) 
 *                      is used when objects are copied without construction.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpSessionTime* rvSdpSessionTimeConstruct2(
            struct _RvSdpMsg* msg,
            struct _RvSdpSessionTime* sessTime,
            RvUint32 start,
            RvUint32 end,
            const char* badSyn,
            RvAlloc* a,
            RvBool dontConstruct);

/***************************************************************************
 * rvSdpSessionTimeCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpSessionTime from 'src' to 'dest'. 
 *      If the destination object is NULL pointer, the destination
 *      object will be allocated & constructed within the function.
 *      The destination object will or willl not be constructed depending
 *      on 'dontConstruct' value. If 'dontConstruct' is true the 'dest'
 *      must point to valid & constructed object.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpSessionTime object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpSessionTime object or NULL. 
 *      src - a pointer to the source object
 *      obj - the RvSdpMsg instance that will own the destination object
 *            or pointer to allocator.
 *      dontConstruct - whether the destination object has to be constructed
 *                      prior to copy.
 *      copyRepeatIn - if set to true the repeat intervals owned by 'src' will
 *                     be copied as well.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpSessionTime* rvSdpSessionTimeCopy2(
            struct _RvSdpSessionTime* dst,
            const struct _RvSdpSessionTime* src,
            void* obj, 
            RvBool dontConstruct,
            RvBool copyRepeatIn);

/***************************************************************************
 * rvSdpAddSessionTime2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs and  adds the session time to the message.
 *          
 * Return Value: 
 *      A pointer to the added RvSdpSessionTime object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - the pointer to RvSdpMsg object where the 
 *            addition will be performed.
 *      start - the start time of the session.
 *      end - the end time of the session.
 *      badSyn - the proprietary formatted session time field or NULL if standard 
 *               session time is added.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpSessionTime* rvSdpMsgAddSessionTime2(
            struct _RvSdpMsg* msg,
            RvUint32 start,         
            RvUint32 end, 
            const char* badSyn);

 /***************************************************************************
 * rvSdpSessionTimeAddRepeatInterval2
 * ------------------------------------------------------------------------
 * General: 
 *      Adds new repeat interval to the session time.
 *          
 * Return Value: 
 *      The pointer to added RvSdpRepeatInterval object or NULL if the function fails.
 * ------------------------------------------------------------------------
 * Arguments:
 *      session - a pointer to the RvSdpSessionTime object.
 *      time - the time length of the repeat times.
 *      t_units - the time units of the repeat times.
 *      duration - the length of the active duration.
 *      d_units - the time units of the active duration.
 *      badSyn - the proprietary formatted repeat interval field or NULL if standard 
 *               repeat interval is added.
 **************************************************************************/
RVSDPCOREAPI struct _RvSdpRepeatInterval* rvSdpSessionTimeAddRepeatInterval2(
            struct _RvSdpRepeatInterval* interv,
            struct _RvSdpSessionTime* session,
            RvUint32 time,
            RvSdpTimeUnit t_units,
            RvUint32 duration ,
            RvSdpTimeUnit d_units,
            const char* badSyn);

/***************************************************************************
 * rvSdpTimeZoneAdjustConstruct2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs the RvSdpTimeZoneAdjust object.
 *          
 * Return Value: 
 *      A pointer to the constructed object, or NULL if the function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - pointer to valid RvSdpMsg object or NULL. If the parameter is not
 *            NULL and 'timeZone' is NULL the time zone will be allocated from
 *            the 'msg' pool. If 'msg' is not NULL the constructed
 *            object will be appended to 'msg' list of time zones. If the 'msg'
 *            is NULL the 'a' allocator will be used.
 *      timeZone - a pointer to valid RvSdpTimeZoneAdjust object or NULL.
 *      t - the time of time shift
 *      offsetTime - the lehgth of the time shift
 *      offsetUnits - the units  of 'offsetTime'
 *      a  - allocator to be used for memory allocations, if allocator is NULL
 *           the default allocator is used.
 *      dontConstruct - if set to RV_TRUE the 'timeZone' must point to valid & constructed
 *                      RvSdpTimeZoneAdjust object. This parameter (with RV_TRUE value) 
 *                      is used when objects are copied without construction.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpTimeZoneAdjust* rvSdpTimeZoneAdjustConstruct2(
            struct _RvSdpMsg* msg,
            struct _RvSdpTimeZoneAdjust* timeZone,
            RvUint32 t,
            RvInt32 offsetTime,
            RvSdpTimeUnit offsetUnits,
            RvAlloc* a,
            RvBool dontConstruct);


/***************************************************************************
 * rvSdpTimeZoneAdjustCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpTimeZoneAdjust from 'src' to 'dest'. 
 *      If the destination object is NULL pointer, the destination
 *      object will be allocated & constructed within the function.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpTimeZoneAdjust object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpTimeZoneAdjust object or NULL. 
 *      src - a pointer to the source object
 *      msg - the RvSdpMsg instance that will own the destination object.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpTimeZoneAdjust* rvSdpTimeZoneAdjustCopy2(
            struct _RvSdpTimeZoneAdjust* dst,
            const struct _RvSdpTimeZoneAdjust* src,
            struct _RvSdpMsg* msg);


/***************************************************************************
 * rvSdpRepeatIntervalConstruct2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs the RvSdpRepeatInterval object.
 *          
 * Return Value: 
 *      A pointer to the constructed object, or NULL if the function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - pointer to valid RvSdpMsg object or NULL. If the parameter is not
 *            NULL and 'interv' is NULL the repeat interval will be allocated from
 *            the 'msg' pool of repeat intervals. If 'msg' is not NULL the constructed
 *            repeat interv will be appended to 'msg' list of repeat interval. If the 
 *            'msg' is NULL the 'a' allocator will be used.
 *      interv - a pointer to valid RvSdpRepeatInterval object or NULL.
 *      time - the length of repeat interval.
 *      t_units - the units of repeat interval length.
 *      duration - the length of session active duration.
 *      d_units - the units of session active duration length.
 *      badSyn - the proprietary formatted repeat interval field or NULL if 
 *               standard repeat interval is constructed.
 *      a  - allocator to be used for memory allocations, if allocator is NULL
 *           the default allocator is used.
 *      dontConstruct - if set to RV_TRUE the 'interv' must point to valid & constructed
 *                      RvSdpRepeatInterval object. This parameter (with RV_TRUE value) 
 *                      is used when objects are copied without construction.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpRepeatInterval* rvSdpRepeatIntervalConstruct2(
            struct _RvSdpMsg* msg,
            struct _RvSdpRepeatInterval* interv,                                                                   
            RvUint32 time,
            RvSdpTimeUnit t_units,
            RvUint32 duration,
            RvSdpTimeUnit d_units,
            const char* badSyntax,
            RvAlloc* a,
            RvBool dontConstruct);

/***************************************************************************
 * rvSdpRepeatIntervalCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpRepeatInterval from 'src' to 'dest'. 
 *      If the destination object is NULL pointer, the destination
 *      object will be allocated & constructed within the function.
 *      The destination object will or willl not be constructed depending
 *      on 'dontConstruct' value. If 'dontConstruct' is true the 'dest'
 *      must point to valid & constructed object.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpRepeatInterval object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpRepeatInterval object or NULL. 
 *      src - a pointer to the source object
 *      msg - the RvSdpMsg instance that will own the destination object.
 *      dontConstruct - whether the destination object has to be constructed
 *                      prior to copy.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpRepeatInterval* rvSdpRepeatIntervalCopy2(
            struct _RvSdpRepeatInterval* dst,
            const struct _RvSdpRepeatInterval* src,
            struct _RvSdpMsg* msg,
            RvBool dontConstruct);


/***************************************************************************
 * rvSdpKeyConstruct2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs the RvSdpKey object.
 *          
 * Return Value: 
 *      A pointer to the constructed object, or NULL if the function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - pointer to valid RvSdpMsg object or NULL. 
 *            If NULL the 'a' allocator will be used.
 *      key - a pointer to valid RvSdpKey object.
 *      type - the encryption method type.
 *      data - the encryption data.
 *      badSyn - the proprietary formatted key field or NULL if standard key is 
 *               constructed.
 *      a  - allocator to be used for memory allocations, if allocator is NULL
 *           the default allocator is used.
 *      dontConstruct - if set to RV_TRUE the 'key' must point to valid & constructed
 *                      RvSdpKey object. This parameter (with RV_TRUE value) 
 *                      is used when objects are copied without construction.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpKey* rvSdpKeyConstruct2(
            struct _RvSdpMsg* msg,
            struct _RvSdpKey* key,
            RvSdpEncrMethod type,
            const char* data,
            const char* badSyn,
            RvAlloc* a,
            RvBool dontConstruct);

/***************************************************************************
 * rvSdpKeyCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpKey from 'src' to 'dest'. 
 *      The destination object will or willl not be constructed depending
 *      on 'dontConstruct' value. If 'dontConstruct' is true the 'dest'
 *      must point to constructed object.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpKey object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpKey object. 
 *      src - a pointer to the source object.
 *      obj - the RvSdpMsg instance that will own the destination object
 *            or pointer to allocator.
 *      dontConstruct - whether the destination object has to be constructed
 *                      prior to copy.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpKey* rvSdpKeyCopy2(
            struct _RvSdpKey* dest, 
            const struct _RvSdpKey* src, 
            void* obj,
            RvBool dontConstruct);

/***************************************************************************
 * rvSdpKeyCompare
 * ------------------------------------------------------------------------
 * General: 
 *      Compare two RvSdpKey objects. 
 *          
 * Return Value: 
 *      RV_TRUE is they are equal.
 * ------------------------------------------------------------------------
 * Arguments:
 *      k1 - RvSdpKey object
 *      k2 - RvSdpKey object
 ***************************************************************************/
RVSDPCOREAPI RvBool rvSdpKeyCompare(
            const struct _RvSdpKey* k1, 
            const struct _RvSdpKey* k2);

/***************************************************************************
 * rvSdpBandwidthConstruct2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs the RvSdpBandwidth object.
 *          
 * Return Value: 
 *      A pointer to the constructed object, or NULL if the function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - pointer to valid RvSdpMsg object or NULL. If the parameter is not
 *            NULL and 'bw' is NULL the bandwidth will be allocated from
 *            the 'msg' pool of bandwidths. If 'msg' is not NULL the constructed
 *            bandwidth will be appended to 'msg' list of bandwidths. If the 'msg'
 *            is NULL the 'a' allocator will be used.
 *      bw - a pointer to valid RvSdpBandwidth object or NULL.
 *      type - the bandwidth type name.
 *      value - the bandwith value (in Kbs).
 *      badSyn - the proprietary formatted bandwidth field or NULL if standard bandwidth
 *               is constructed.
 *      a  - allocator to be used for memory allocations, if allocator is NULL
 *           the default allocator is used.
 *      dontConstruct - if set to RV_TRUE the 'bw' must point to valid & constructed
 *                      RvSdpBandwidth object. This parameter (with RV_TRUE value) 
 *                      is used when objects are copied without construction.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpBandwidth* rvSdpBandwidthConstruct2(
            struct _RvSdpMsg* msg,
            struct _RvSdpBandwidth* bw,
            const char* type,
            RvUint32 value,
            const char* badSyn,                
            RvAlloc* a,
            RvBool dontConstruct);

/***************************************************************************
 * rvSdpBandwidthCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpBandwidth from 'src' to 'dest'. 
 *      If the destination object is NULL pointer, the destination
 *      object will be allocated & constructed within the function.
 *      The destination object will or willl not be constructed depending
 *      on 'dontConstruct' value. If 'dontConstruct' is true the 'dest'
 *      must point to valid & constructed object.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpBandwidth object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpBandwidth object or NULL. 
 *      src - a pointer to the source object
 *      obj - the RvSdpMsg instance that will own the destination object
 *            or pointer to allocator.
 *      dontConstruct - whether the destination object has to be constructed
 *                      prior to copy.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpBandwidth* rvSdpBandwidthCopy2(
            struct _RvSdpBandwidth* dest, 
            const struct _RvSdpBandwidth* src, 
            void* obj,
            RvBool dontConstruct);

/***************************************************************************
 * rvSdpBandwidthCompare
 * ------------------------------------------------------------------------
 * General: 
 *      Compares two RvSdpBandwidth objects.
 *
 * Return Value: 
 *      RV_TRUE if they are equal
 * ------------------------------------------------------------------------
 * Arguments:
 *      bw1 - RvSdpBandwidth object 
 *      bw2 - RvSdpBandwidth object
 ***************************************************************************/
RVSDPCOREAPI RvBool rvSdpBandwidthCompare(
            const struct _RvSdpBandwidth* bw1, 
            const struct _RvSdpBandwidth* bw2);

/***************************************************************************
 * rvSdpAddBandwidth2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs and  adds the bandwidth to the message or media.
 *          
 * Return Value: 
 *      A pointer to the added RvSdpBandwidth object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      obj - the pointer to RvSdpMsg or RvSdpCommonFields object where the 
 *            addition will be performed.
 *      type - the bandwidth type name.
 *      value - the bandwith value (in Kbs).
 *      badSyn - the proprietary formatted bandwidth field or NULL if standard 
 *               bandwidth is added.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpBandwidth* rvSdpAddBandwidth2(
            void* obj,
            const char* type,
            int value,
            const char* badSyn);


/***************************************************************************
 * rvSdpConnectionConstruct2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs the RvSdpConnection object.
 *          
 * Return Value: 
 *      A pointer to the constructed object, or NULL if the function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - pointer to valid RvSdpMsg object or NULL. If the parameter is not
 *            NULL and 'conn' is NULL the connection will be allocated from
 *            the 'msg' pool of connections. If 'msg' is not NULL the constructed
 *            connection will be appended to 'msg' list of connections. If the 'msg'
 *            is NULL the 'a' allocator will be used.
 *      conn - a pointer to valid RvSdpConnection object or NULL.
 *      nettype - the network type.
 *      addrtype - the address type.
 *      addr - the connection address.
 *      badSyn - the proprietary formatted connection field or NULL if standard 
 *               connection is constructed.
 *      a  - allocator to be used for memory allocations, if allocator is NULL
 *           the default allocator is used.
 *      dontConstruct - if set to RV_TRUE the 'conn' must point to valid & constructed
 *                      RvSdpConnection object. This parameter (with RV_TRUE value) 
 *                      is used when objects are copied without construction.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpConnection* rvSdpConnectionConstruct2(
            struct _RvSdpMsg* msg,
            struct _RvSdpConnection* conn,
            RvSdpNetType nettype,
            RvSdpAddrType addrtype,
            const char* address,
            const char* badSyn,
            RvAlloc* a,
            RvBool dontConstruct);

/***************************************************************************
 * rvSdpConnectionCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpConnection from 'src' to 'dest'. 
 *      If the destination object is NULL pointer, the destination
 *      object will be allocated & constructed within the function.
 *      The destination object will or willl not be constructed depending
 *      on 'dontConstruct' value. If 'dontConstruct' is true the 'dest'
 *      must point to valid & constructed object.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpConnection object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpConnection object or NULL. 
 *      src - a pointer to the source object
 *      obj - the RvSdpMsg instance that will own the destination object
 *            or pointer to allocator.
 *      dontConstruct - whether the destination object has to be constructed
 *                      prior to copy.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpConnection* rvSdpConnectionCopy2(
            struct _RvSdpConnection* dest, 
            const struct _RvSdpConnection* src, 
            void* obj,
            RvBool dontConstruct);

/***************************************************************************
 * rvSdpConnectionCompare
 * ------------------------------------------------------------------------
 * General: 
 *      Compares two RvSdpConnection objects.
 *
 * Return Value: 
 *      RV_TRUE id objects are equal
 * ------------------------------------------------------------------------
 * Arguments:
 *      c1 - RvSdpConnection object
 *      c2 - RvSdpConnection object
 ***************************************************************************/
RVSDPCOREAPI RvBool rvSdpConnectionCompare(
            const struct _RvSdpConnection* c1, 
            const struct _RvSdpConnection* c2);

/***************************************************************************
 * rvSdpAddConnection2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs and  adds the connection to the message or media.
 *          
 * Return Value: 
 *      A pointer to the added RvSdpConnection object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      obj - the pointer to RvSdpMsg or RvSdpCommonFields object where the 
 *            addition will be performed.
 *      nettype - the network type.
 *      addrtype - the address type.
 *      addr - the connection address.
 *      badSyn - the proprietary formatted connection field or NULL if standard 
 *               connection is added.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpConnection* rvSdpAddConnection2(
            void* obj,
            RvSdpNetType nettype,
            RvSdpAddrType addrtype,
            const char* addr,
            const char* badSyn);

/***************************************************************************
 * rvSdpRtpMapConstruct2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs the RvSdpRtpMap object.
 *          
 * Return Value: 
 *      A pointer to the constructed object, or NULL if the function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - pointer to valid RvSdpMsg object or NULL. If the parameter is not
 *            NULL and 'rtpMap' is NULL the RTP map will be allocated from
 *            the 'msg' pool of RTP maps. If 'msg' is not NULL the constructed
 *            RTP map will be appended to 'msg' list of attributes. If the 'msg'
 *            is NULL the 'a' allocator will be used.
 *      rtpMap - a pointer to valid RvSdpRtpMap object or NULL.
 *      payload - the RTP map payload number.
 *      encoding_name - the RTP map encoding name.
 *      rate - the RTP map rate value.
 *      badSyn - the proprietary formatted RTP map field or NULL if standard 
 *               RTP map is constructed.
 *      a  - allocator to be used for memory allocations, if allocator is NULL
 *           the default allocator is used.
 *      dontConstruct - if set to RV_TRUE the 'rtpMap' must point to valid & constructed
 *                      RvSdpRtpMap object. This parameter (with RV_TRUE value) 
 *                      is used when objects are copied without construction.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpRtpMap* rvSdpRtpMapConstruct2(
            struct _RvSdpMsg* msg,
            struct _RvSdpRtpMap* rtpMap,
            int payload,
            const char* encoding_name,
            int rate,
            const char* badSyn,
            RvAlloc* alloc);

/***************************************************************************
 * rvSdpRtpMapCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpRtpMap from 'src' to 'dest'. 
 *      If the destination object is NULL pointer, the destination
 *      object will be allocated & constructed within the function.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpRtpMap object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpRtpMap object or NULL. 
 *      src - a pointer to the source object
 *      obj - the RvSdpMsg instance that will own the destination object or
 *            allocator pointer.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpRtpMap* rvSdpRtpMapCopy2(
            struct _RvSdpRtpMap* dest, 
            const struct _RvSdpRtpMap* src, 
            void* obj);

/***************************************************************************
 * rvSdpRtpMapGetValue
 * ------------------------------------------------------------------------
 * General: 
 *      Prints the textual value of RTP map attribute  into provided 
 *      buffer.
 *          
 * Return Value: 
 *      Returns the buffer pointer on success or NULL if fails.
 * ------------------------------------------------------------------------
 * Arguments:
 *          rtpMap - instance of RvSdpRtpMap.
 *          txt - the buffer for the value.
 ***************************************************************************/
RVSDPCOREAPI char* rvSdpRtpMapGetValue(
            struct _RvSdpRtpMap* rtpMap, 
            char* txt);

/***************************************************************************
 * rvSdpAddRtpMap2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs and  adds the RTP map atribute to the message or media.
 *          
 * Return Value: 
 *      A pointer to the added RvSdpRtpMap object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *          msg - message where addition will be performed.
 *          commF - common fields pointer of message or media.
 *          payload - an RTP dynamic payload number.
 *          encoding_name - the name of the codec.
 *          rate - the clock rate.
 *          badSyn - proprietary syntax RTP map attribute or NULL if
 *                   the  valid RTP map attribute is added.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpRtpMap* rvSdpAddRtpMap2(
            struct _RvSdpMsg* msg,            
            struct _RvSdpCommonFields* commF, 
            int payload,                
            const char* encoding_name, 
            int rate, 
            const char *badSyn);       

/***************************************************************************
 * rvSdpOriginConstruct2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs the RvSdpOrigin object.
 *          
 * Return Value: 
 *      A pointer to the constructed object, or NULL if the function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - pointer to valid RvSdpMsg object.
 *      origin - a pointer to valid RvSdpOrigin object.
 *      username - the origin field user name.
 *      session_id - the origin field session ID.
 *      version - the origin field version.
 *      nettype - the network type.
 *      addrtype - the address type.
 *      addr - the connection address.
 *      badSyn - the proprietary formatted origin field or NULL if standard 
 *               origin is constructed.
 *      a  - allocator to be used for memory allocations, if allocator is NULL
 *           the default allocator is used.
 *      dontConstruct - if set to RV_TRUE the 'origin' must point to valid & constructed
 *                      RvSdpOrigin object. This parameter (with RV_TRUE value) 
 *                      is used when objects are copied without construction.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpOrigin* rvSdpOriginConstruct2(
            struct _RvSdpMsg *msg,
            struct _RvSdpOrigin *origin,
            const char* username,
            const char* session_id,
            const char* version,
            RvSdpNetType nettype,
            RvSdpAddrType addrtype,
            const char* address,
            const char* badSyn,
            RvAlloc* a,
            RvBool dontConstruct);

/***************************************************************************
 * rvSdpOriginCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpOrigin from 'src' to 'dest'. 
 *      The destination object will or willl not be constructed depending
 *      on 'dontConstruct' value. If 'dontConstruct' is true the 'dest'
 *      must point to constructed object.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpOrigin object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpOrigin object. 
 *      src - a pointer to the source object.
 *      obj - the RvSdpMsg instance that will own the destination object
 *            or pointer to allocator.
 *      dontConstruct - whether the destination object has to be constructed
 *                      prior to copy.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpOrigin* rvSdpOriginCopy2(
            struct _RvSdpOrigin* dest,
            const struct _RvSdpOrigin* src,
            void* obj,
            RvBool dontConstruct);

/***************************************************************************
 * rvSdpEmailConstruct2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs the RvSdpEmail object.
 *          
 * Return Value: 
 *      A pointer to the constructed object, or NULL if the function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - pointer to valid RvSdpMsg object or NULL. If the parameter is not
 *            NULL and 'email' is NULL the email will be allocated from
 *            the 'msg' pool of emails. If 'msg' is not NULL the constructed
 *            email will be appended to 'msg' list of emails. If the 'msg'
 *            is NULL the 'a' allocator will be used.
 *      email - a pointer to valid RvSdpEmail object or NULL.
 *      address - the email address.
 *      text - optional email text.
 *      badSyn - the proprietary formatted email field or NULL if standard 
 *               email is constructed.
 *      a  - allocator to be used for memory allocations, if allocator is NULL
 *           the default allocator is used.
 *      dontConstruct - if set to RV_TRUE the 'email' must point to valid & constructed
 *                      RvSdpEmail object. This parameter (with RV_TRUE value) 
 *                      is used when objects are copied without construction.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpEmail* rvSdpEmailConstruct2(
            struct _RvSdpMsg* msg,
            struct _RvSdpEmail* email,
            const char* address,
            const char* text,
            const char* badSyn,
            RvAlloc* a,
            RvBool dontConstruct);

/***************************************************************************
 * rvSdpEmailCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpEmail from 'src' to 'dest'. 
 *      If the destination object is NULL pointer, the destination
 *      object will be allocated & constructed within the function.
 *      The destination object will or willl not be constructed depending
 *      on 'dontConstruct' value. If 'dontConstruct' is true the 'dest'
 *      must point to valid & constructed object.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpEmail object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpEmail object or NULL. 
 *      src - a pointer to the source object
 *      obj - the RvSdpMsg instance that will own the destination object
 *            or pointer to allocator.
 *      dontConstruct - whether the destination object has to be constructed
 *                      prior to copy.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpEmail* rvSdpEmailCopy2(
            struct _RvSdpEmail* dest, 
            const struct _RvSdpEmail* src, 
            void* obj,
            RvBool dontConstruct);

/***************************************************************************
 * rvSdpAddEmail2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs and  adds the email to the message object.
 *          
 * Return Value: 
 *      A pointer to the added RvSdpEmail object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - the pointer to RvSdpMsg object where the 
 *            addition will be performed.
 *      address - the email address.
 *      text - optional email text.
 *      badSyn - the proprietary formatted email field or NULL if standard 
 *               email is added.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpEmail* rvSdpMsgAddEmail2(
            struct _RvSdpMsg* msg,
            const char* address,
            const char* text,
            const char* badSyn);


/***************************************************************************
 * rvSdpPhoneConstruct2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs the RvSdpPhone object.
 *          
 * Return Value: 
 *      A pointer to the constructed object, or NULL if the function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - pointer to valid RvSdpMsg object or NULL. If the parameter is not
 *            NULL and 'phone' is NULL the phone will be allocated from
 *            the 'msg' pool of phones. If 'msg' is not NULL the constructed
 *            phone will be appended to 'msg' list of phones. If the 'msg'
 *            is NULL the 'a' allocator will be used.
 *      phone - a pointer to valid RvSdpPhone object or NULL.
 *      number - the phone number.
 *      text - optional phone text.
 *      badSyn - the proprietary formatted phone field or NULL if standard 
 *               phone is constructed.
 *      a  - allocator to be used for memory allocations, if allocator is NULL
 *           the default allocator is used.
 *      dontConstruct - if set to RV_TRUE the 'phone' must point to valid & constructed
 *                      RvSdpPhone object. This parameter (with RV_TRUE value) 
 *                      is used when objects are copied without construction.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpPhone* rvSdpPhoneConstruct2(
            struct _RvSdpMsg* msg,
            struct _RvSdpPhone* phone,
            const char* number,
            const char* text,
            const char* badSyn,
            RvAlloc* alloc,
            RvBool dontConstruct);

/***************************************************************************
 * rvSdpPhoneCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpPhone from 'src' to 'dest'. 
 *      If the destination object is NULL pointer, the destination
 *      object will be allocated & constructed within the function.
 *      The destination object will or willl not be constructed depending
 *      on 'dontConstruct' value. If 'dontConstruct' is true the 'dest'
 *      must point to valid & constructed object.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpPhone object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpPhone object or NULL. 
 *      src - a pointer to the source object
 *      obj - the RvSdpMsg instance that will own the destination object
 *            or pointer to allocator.
 *      dontConstruct - whether the destination object has to be constructed
 *                      prior to copy.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpPhone* rvSdpPhoneCopy2(
            struct _RvSdpPhone* dst,
            const struct _RvSdpPhone* src,
            void* obj,
            RvBool dontConstruct);

/***************************************************************************
 * rvSdpAddPhone2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs and  adds the phone to the message object.
 *          
 * Return Value: 
 *      A pointer to the added RvSdpPhone object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - the pointer to RvSdpMsg object where the 
 *            addition will be performed.
 *      number - the phone number.
 *      text - optional phone text.
 *      badSyn - the proprietary formatted phone field or NULL if standard 
 *               phone is added.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpPhone* rvSdpMsgAddPhone2(
            struct _RvSdpMsg* msg,
            const char* number,
            const char* text,
            const char* badSyn);


/***************************************************************************
 * rvSdpOtherConstruct2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs the RvSdpOther object.
 *          
 * Return Value: 
 *      A pointer to the constructed object, or NULL if the function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - pointer to valid RvSdpMsg object or NULL. If the parameter is not
 *            NULL and 'oth' is NULL the 'other' will be allocated from
 *            the 'msg' pool of 'other's. If 'msg' is not NULL the constructed
 *            'other' will be appended to 'msg' list of 'other's. If the 'msg'
 *            is NULL the 'a' allocator will be used.
 *      oth - a pointer to valid RvSdpOther object or NULL.
 *      tag - the tag letter of the line.
 *      value - the text of the line.
 *      a  - allocator to be used for memory allocations, if allocator is NULL
 *           the default allocator is used.
 *      dontConstruct - if set to RV_TRUE the 'oth' must point to valid & constructed
 *                      RvSdpOther object. This parameter (with RV_TRUE value) 
 *                      is used when objects are copied without construction.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpOther* rvSdpOtherConstruct2(
            struct _RvSdpMsg* msg,
            struct _RvSdpOther* oth,
            const char tag,
            const char* value,
            RvAlloc* a,
            RvBool dontConstruct);

/***************************************************************************
 * rvSdpOtherCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpOther from 'src' to 'dest'. 
 *      If the destination object is NULL pointer, the destination
 *      object will be allocated & constructed within the function.
 *      The destination object will or willl not be constructed depending
 *      on 'dontConstruct' value. If 'dontConstruct' is true the 'dest'
 *      must point to valid & constructed object.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpOther object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpOther object or NULL. 
 *      src - a pointer to the source object
 *      obj - the RvSdpMsg instance that will own the destination object
 *            or pointer to allocator.
 *      dontConstruct - whether the destination object has to be constructed
 *                      prior to copy.
 ***************************************************************************/
#ifndef RV_SDP_USE_MACROS
RVSDPCOREAPI struct _RvSdpOther* rvSdpOtherCopy2(
            struct _RvSdpOther* dest, 
            const struct _RvSdpOther* src, 
            void* obj,
            RvBool dontConstruct);
#else /*RV_SDP_USE_MACROS*/
#define rvSdpOtherCopy2(_dest,_src,_obj,_dontConstruct) \
            rvSdpOtherConstruct2(NULL,(_dest),(_src)->iOtherTag,\
                                 (_src)->iValue,(RvAlloc*)(_obj),(_dontConstruct))
#endif /*RV_SDP_USE_MACROS*/

/***************************************************************************
 * rvSdpOtherCompare
 * ------------------------------------------------------------------------
 * General: 
 *      Compare two _RvSdpOther objects.
 *          
 * Return Value: 
 *      RV_TRUE if the 'other' are equal
 * ------------------------------------------------------------------------
 * Arguments:
 *      o1 - _RvSdpOther object.
 *      o2 - _RvSdpOther object. 
 ***************************************************************************/
RVSDPCOREAPI RvBool rvSdpOtherCompare(
            struct _RvSdpOther* o1,
            struct _RvSdpOther* o2);

/***************************************************************************
 * rvSdpAddOther
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs and  adds the 'other' object to message or media descriptor.
 *          
 * Return Value: 
 *      A pointer to the added RvSdpOther object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - the pointer to RvSdpMsg object.
 *      commF - the pointer to RvSdpCommonFields of message or media descriptor 
 *              where 'other' object has to be added.
 *      tag - the tag letter of the 'other' object.
 *      value - the value (after '=' sign) of the 'other' object.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpOther* rvSdpAddOther(
            struct _RvSdpMsg* msg,
            struct _RvSdpCommonFields* commF,
            const char tag,     
            const char *value);

/***************************************************************************
 * rvSdpAttributeConstruct2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs the RvSdpAttribute object.
 *          
 * Return Value: 
 *      A pointer to the constructed object, or NULL if the function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - pointer to valid RvSdpMsg object or NULL. If the parameter is not
 *            NULL and 'attr' is NULL the attribute will be allocated from
 *            the 'msg' pool of attributes. If 'msg' is not NULL the constructed
 *            attribute will be appended to 'msg' list of attributes. If the 'msg'
 *            is NULL the 'a' allocator will be used.
 *      attr - a pointer to valid RvSdpAttribute object or NULL.
 *      name - the name of the attribute.
 *      value - the value of the attribute.
 *      a  - allocator to be used for memory allocations, if allocator is NULL
 *           the default allocator is used.
 *      dontConstruct - if set to RV_TRUE the 'attr' must point to valid & constructed
 *                      RvSdpAttribute object. This parameter (with RV_TRUE value) 
 *                      is used when objects are copied without construction.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpAttribute* rvSdpAttributeConstruct2(
            struct _RvSdpMsg* msg,
            struct _RvSdpAttribute* attr,
            const char* name,
            const char* value,
            RvAlloc* a,
            RvBool dontConstruct);

/***************************************************************************
 * rvSdpAttributeCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpAttribute from 'src' to 'dest'. 
 *      If the destination object is NULL pointer, the destination
 *      object will be allocated & constructed within the function.
 *      The destination object will or willl not be constructed depending
 *      on 'dontConstruct' value. If 'dontConstruct' is true the 'dest'
 *      must point to valid & constructed object.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpAttribute object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpAttribute object or NULL. 
 *      src - a pointer to the source object
 *      dstMsg - the RvSdpMsg instance that will own the destination object
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpAttribute* rvSdpAttributeCopy2(
            struct _RvSdpAttribute* dest, 
            const struct _RvSdpAttribute* src,
            struct _RvSdpMsg* dstMsg);


/***************************************************************************
 * rvSdpAddAttr2
 * ------------------------------------------------------------------------
 * General: 
 *      Adds the generic atribute to the message or media.
 *      If the 'attr' is not NULL it must point to constructed RvSdpAttribute
 *      whose name and value will be set. Otherwise the attribute will be 
 *      constructed.
 *          
 * Return Value: 
 *      A pointer to the added RvSdpAttribute object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *          msg - message where addition will be performed.
 *          commF - common fields pointer of message or media.
 *          attr - the pointer to constructed RvSdpAttribute or NULL.
 *          name - the generic attribute name.
 *          value - the generic attribute value.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpAttribute* rvSdpAddAttr2(
            struct _RvSdpMsg* msg,
            struct _RvSdpCommonFields *commF,
            struct _RvSdpAttribute* attr, 
            const char* name,
            const char* value);

/*
 *	Key-Mgmt attribute functions
 */

#ifdef RV_SDP_KEY_MGMT_ATTR

struct _RvSdpKeyMgmtAttr;
/***************************************************************************
 * rvSdpKeyMgmtDestruct
 * ------------------------------------------------------------------------
 * General: 
 *      Destroys the instance of RvSdpKeyMgmtAttr.
 *          
 * Return Value: 
 *      None.
 * ------------------------------------------------------------------------
 * Arguments:
 *      keyM - object to be destroyed.
 ***************************************************************************/
RVSDPCOREAPI void rvSdpKeyMgmtDestruct(
            struct _RvSdpKeyMgmtAttr* keym);

/***************************************************************************
 * rvSdpKeyMgmtAttrCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpKeyMgmtAttr from 'src' to 'dest'. 
 *      If the destination object is NULL pointer, the destination
 *      object will be allocated & constructed within the function.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpKeyMgmtAttr object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpKeyMgmtAttr object or NULL. 
 *      src - a pointer to the source object
 *      obj - the RvSdpMsg instance that will own the destination object
 *            or pointer to allocator.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpKeyMgmtAttr* rvSdpKeyMgmtCopy2(
            struct _RvSdpKeyMgmtAttr* dest, 
            const struct _RvSdpKeyMgmtAttr* src, 
            void* obj);

/***************************************************************************
 * rvSdpAddKeyMgmt2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs and  adds the key-mgmt atribute to the message or media.
 *          
 * Return Value: 
 *      A pointer to the added RvSdpKeyMgmtAttr object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *          msg - message where addition will be performed.
 *          commF - common fields pointer of message or media.
 *          prtclId - protocol id value.
 *          keyData - the key data value.
 *          badSyn - proprietary syntax key-mgmt attribute.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpKeyMgmtAttr* rvSdpAddKeyMgmt2(
            struct _RvSdpMsg* msg,
            struct _RvSdpCommonFields* commF,
            const char* prtclId,
            const char* keyData,
            const char* badSyn);        

/***************************************************************************
 * rvSdpKeyMgmtGetValue
 * ------------------------------------------------------------------------
 * General: 
 *      Prints the textual value of key management attribute  into provided 
 *      buffer.
 *          
 * Return Value: 
 *      Returns the buffer pointer on success or NULL if fails.
 * ------------------------------------------------------------------------
 * Arguments:
 *          km - instance of RvSdpKeyMgmtAttr.
 *          txt - the buffer for the value.
 ***************************************************************************/
RVSDPCOREAPI char* rvSdpKeyMgmtGetValue(
            struct _RvSdpKeyMgmtAttr* km,
            char *txt);
#endif /*RV_SDP_KEY_MGMT_ATTR*/

/*
 *	Crypto attribute functions
 */

#ifdef RV_SDP_CRYPTO_ATTR

struct _RvSdpCryptoAttr;

/***************************************************************************
 * rvSdpCryptoDestruct
 * ------------------------------------------------------------------------
 * General: 
 *      Destroys the instance of RvSdpCryptoAttr.
 *          
 * Return Value: 
 *      None.
 * ------------------------------------------------------------------------
 * Arguments:
 *      crypto - object to be destroyed.
 ***************************************************************************/
RVSDPCOREAPI void rvSdpCryptoDestruct(
            struct _RvSdpCryptoAttr* crypto);

/***************************************************************************
 * rvSdpCryptoCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpCryptoAttr from 'src' to 'dest'. 
 *      If the destination object is NULL pointer, the destination
 *      object will be allocated & constructed within the function.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpCryptoAttr object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpCryptoAttr object or NULL. 
 *      src - a pointer to the source object
 *      obj - the RvSdpMsg instance that will own the destination object
 *            or pointer to allocator.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpCryptoAttr* rvSdpCryptoCopy2(
            struct _RvSdpCryptoAttr* dest, 
            const struct _RvSdpCryptoAttr* src, 
            void* obj);

/***************************************************************************
 * rvSdpAddCrypto2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs and  adds the crypto atribute to the message or media.
 *          
 * Return Value: 
 *      A pointer to the added RvSdpCryptoAttr object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *          msg - message where addition will be performed.
 *          commF - common fields pointer of message or media.
 *          tag - the tag value.
 *          suite - the crypto suite.
 *          badSyn - proprietary syntax crypto attribute.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpCryptoAttr* rvSdpAddCrypto2(
            struct _RvSdpMsg* msg,
            struct _RvSdpCommonFields* commF,
            RvUint tag,
            const char* suite,
            const char* badSyn);

/***************************************************************************
 * rvSdpCryptoGetValue
 * ------------------------------------------------------------------------
 * General: 
 *      Prints the textual value of crypto attribute  into provided 
 *      buffer.
 *          
 * Return Value: 
 *      Returns the buffer pointer on success or NULL if fails.
 * ------------------------------------------------------------------------
 * Arguments:
 *          crypto - instance of RvSdpCryptoAttr.
 *          txt - the buffer for the value.
 ***************************************************************************/
RVSDPCOREAPI char* rvSdpCryptoGetValue(
            struct _RvSdpCryptoAttr* crypto,
            char *txt);

#endif /*RV_SDP_CRYPTO_ATTR*/

/***************************************************************************
 * rvSdpGetNumOfSpecialAttr
 * ------------------------------------------------------------------------
 * General: 
 *      Gets  the number of special attributes of specific type in message 
 *      or media descrtiptor.
 *          
 * Return Value: 
 *      Returns the number of special attributes  of specific type in message
 *      or media descriptor.
 * ------------------------------------------------------------------------
 * Arguments:
 *          commF - common fields pointer of message or media where number of 
 *                  special attributes is needed.
 *          specAttrType - the type of special attribute is question.
 ***************************************************************************/
RVSDPCOREAPI RvSize_t rvSdpGetNumOfSpecialAttr(
            const struct _RvSdpCommonFields* commF,
            RvSdpFieldTypes specAttrType);

/***************************************************************************
 * rvSdpGetFirstSpecialAttr
 * ------------------------------------------------------------------------
 * General: 
 *      Returns the first special attribute object of specific type defined 
 *      in the message or  media descriptor. 
 *      Also sets the list iterator for the further use.
 *          
 * Return Value: 
 *      Pointer to the RvSdpAttribute  object or the NULL pointer if there are no
 *      special attributes of given type defined in the message or media descriptor.
 * ------------------------------------------------------------------------
 * Arguments:
 *          commF - common fields pointer of message or media descriptor.
 *          iter - pointer to RvSdpListIter to be used for subsequent 
 *                 rvSdpGetNextSpecialAttr calls.
 *          specAttrType - the type of special attribute is question.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpAttribute* rvSdpGetFirstSpecialAttr(
            struct _RvSdpCommonFields* commF, 
            RvSdpListIter* iter,
            RvSdpFieldTypes specAttrType);

/***************************************************************************
 * rvSdpGetNextSpecialAttr
 * ------------------------------------------------------------------------
 * General: 
 *      Returns the next special attribute object of specific type defined 
 *      in the message or  media descriptor. 
 *      Also modifies the list iterator for the further use.
 *          
 * Return Value: 
 *      Pointer to the RvSdpAttribute  object or the NULL pointer if there are no
 *      more special attributes of given type defined in the message or media 
 *      descriptor.
 * ------------------------------------------------------------------------
 * Arguments:
 *          iter - pointer to RvSdpListIter to be used for subsequent 
 *                 rvSdpGetNextSpecialAttr calls.
 *          specAttrType - the type of special attribute is question.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpAttribute* rvSdpGetNextSpecialAttr(
            RvSdpListIter* iter,
            RvSdpFieldTypes specAttrType);

/***************************************************************************
 * rvSdpGetSpecialAttr
 * ------------------------------------------------------------------------
 * General: 
 *      Returns the special attribute object of specific type defined 
 *      in the message or media descriptor by zero-based index. 
 *          
 * Return Value: 
 *      Pointer to the RvSdpAttribute  object.
 * ------------------------------------------------------------------------
 * Arguments:
 *          commF - common fields pointer of message or media descriptor.
 *          index - zero-based index of special attribute. 
 *          specAttrType - the type of special attribute is question.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpAttribute* rvSdpGetSpecialAttr(
            struct _RvSdpCommonFields* commF, 
            RvSize_t index,
            RvSdpFieldTypes specAttrType);

/***************************************************************************
 * rvSdpRemoveSpecialAttr
 * ------------------------------------------------------------------------
 * General: 
 *      Removes and destroys the special attribute object of specific type defined 
 *      in the message or media descriptor by zero-based index. 
 *          
 * Return Value: 
 *      None.
 * ------------------------------------------------------------------------
 * Arguments:
 *          commF - common fields pointer of message or media descriptor.
 *          index - zero-based index of special attribute. 
 *          specAttrType - the type of special attribute is question.
 ***************************************************************************/
RVSDPCOREAPI void rvSdpRemoveSpecialAttr(
            struct _RvSdpCommonFields* commF, 
            RvSize_t index,
            RvSdpFieldTypes specAttrType);

/***************************************************************************
 * rvSdpClearSpecialAttr
 * ------------------------------------------------------------------------
 * General: 
 *      Removes and destroys all special attribute object of specific type defined 
 *      in the message or media descriptor. 
 *          
 * Return Value: 
 *      None.
 * ------------------------------------------------------------------------
 * Arguments:
 *          commF - common fields pointer of message or media descriptor.
 *          specAttrType - the type of special attributes is question.
 ***************************************************************************/
RVSDPCOREAPI void rvSdpClearSpecialAttr(
            struct _RvSdpCommonFields* commF,
            RvSdpFieldTypes specAttrType);

/***************************************************************************
 * rvSdpSpecialAttrSetValue
 * ------------------------------------------------------------------------
 * General: 
 *      For special attributes allowing multiple appearances adds another
 *      instance of given type special attribute with given name.
 *      For special attributes with at most single appearance modifies the
 *      value of attribute if it was set before or creates the attribute 
 *      if it was not set before.
 *          
 * Return Value: 
 *      Returns the pointer to the RvSdpAttribute of created/modified object.
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - the RvSdpMsg where change happens.
 *      commF - common fields pointer of message or media descriptor.
 *      specAttrType - the type of special attributes is question.
 *      value - the desired value of special attribute.
 *      isMultiple - whether the special attribute allows multiple appearances.
 ***************************************************************************/
RVSDPCOREAPI struct _RvSdpAttribute* rvSdpSpecialAttrSetValue(
			struct _RvSdpMsg *msg, 
			struct _RvSdpCommonFields* commF,
			RvSdpFieldTypes specAttrType, 
			const char* value, 
			RvBool isMultiple);

										 
/***************************************************************************
 * rvSdpSetConnectionMode
 * ------------------------------------------------------------------------
 * General: 
 *      Sets/modifies/removes the connection mode of the message or media
 *      descriptor.
 *      If the SDP_FIELDTYPE_CONNECTION_MODE special attribute was not set
 *      in a message or media it will be added. In case the 'mode' is 
 *      RV_SDPCONNECTMODE_NOTSET the special attribute will be removed.
 *          
 * Return Value: 
 *      Returns RV_SDPSTATUS_OK if the function succeeds, or an error code if the 
 *      function fails.
 * ------------------------------------------------------------------------
 * Arguments:
 *      msg - the RvSdpMsg instance.
 *      commF - the RvSdpCommonFields instance of message or media.
 *      mode - the desired connection mode.
 ***************************************************************************/
RVSDPCOREAPI RvSdpStatus rvSdpSetConnectionMode(
            struct _RvSdpMsg* msg,
            struct _RvSdpCommonFields* commF, 
            RvSdpConnectionMode mode);

/***************************************************************************
 * rvSdpGetConnectionMode
 * ------------------------------------------------------------------------
 * General: 
 *      Gets the connection mode of the message or media
 *      descriptor.
 *          
 * Return Value: 
 *      Returns the connection mode or RV_SDPCONNECTMODE_NOTSET if the 
 *      SDP_FIELDTYPE_CONNECTION_MODE special attribute is not set in the message
 *      or media descriptor.
 * ------------------------------------------------------------------------
 * Arguments:
 *      commF - the RvSdpCommonFields instance of message or media.
 ***************************************************************************/
RVSDPCOREAPI RvSdpConnectionMode rvSdpGetConnectionMode(
            const struct _RvSdpCommonFields* commF);



/*---------------------------*/
/* GROUP attribute (a=group) */
/*---------------------------*/

#ifdef RV_SDP_MEDIA_GROUPING_ATTR
/***************************************************************************
 * rvSdpMediaGroupAttrConstruct2
 * ------------------------------------------------------------------------
 * General: 
 *	Allocates and constructs the object using RvAlloc allocator.
 *  Returns the constructed object or NULL if it was failed.
 .
 * Return Value: 
 *      A pointer to the new added RvSdpMediaGroupAttr object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *          obj - points to RvSdpMsg or RvAlloc instance.
 *          eSemantics - the semantics enum.
 *          badSyn - proprietary syntax Media-Group attribute or NULL.
 ***************************************************************************/
RVSDPCOREAPI RvSdpMediaGroupAttr* rvSdpMediaGroupAttrConstruct2(
            void*                   obj,        
            const char*             semantics, 
            const char*             badSyn);     

/***************************************************************************
 * rvSdpAddMediaGroupAttr2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs and  adds the Media-Group atribute to media descr.
 *          
 * Return Value: 
 *      A pointer to the added RvSdpMediaGroupAttr object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *          msg - message where addition will be performed.
 *          commF - common fields pointer of message or media.
 *          semantics - the semantics string.
 *          badSyn - proprietary syntax key-mgmt attribute.
 ***************************************************************************/
RVSDPCOREAPI RvSdpMediaGroupAttr* rvSdpAddMediaGroupAttr2(
                                   RvSdpMsg  *             msg,  
                                   RvSdpCommonFields*      commF,
                                   const char*             semantics,
                                   const char*             badSyn);


/***************************************************************************
 * rvSdpMediaGroupAttrCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpMediaGroupAttr from 'src' to 'dest'. 
 *      If the destination object is NULL pointer, the destination
 *      object will be allocated & constructed within the function.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpMediaGroupAttr object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpMediaGroupAttr object or NULL. 
 *      src - a pointer to the source object
 *      obj - the RvSdpMsg instance that will own the destination object
 *            or pointer to allocator.
 ***************************************************************************/
RVSDPCOREAPI RvSdpMediaGroupAttr* rvSdpMediaGroupAttrCopy2(
                                              RvSdpMediaGroupAttr      * dest, 
                                              const RvSdpMediaGroupAttr* src, 
                                              void                  * obj);


/***************************************************************************
 * rvSdpMediaGroupAttrDestruct
 * ------------------------------------------------------------------------
 * General: Destroys the instance of RvSdpMediaGrouptAttr.
 *          
 * Return Value:  None.
 *     
 * ------------------------------------------------------------------------
 * Arguments:
 *      mediaGroupAttr - object to be destroyed.
 ***************************************************************************/
RVSDPCOREAPI void rvSdpMediaGroupAttrDestruct(RvSdpMediaGroupAttr* mediaGroupAttr);


/***************************************************************************
 * rvSdpMediaGroupAttrGetValue
 * ------------------------------------------------------------------------
 * General: 
 *      Prints the textual value of Media Group attribute  into provided 
 *      buffer (all the fields values following "a=group:_________________ ".
 *          
 * Return Value: 
 *      Returns the buffer pointer on success or NULL if fails.
 * ------------------------------------------------------------------------
 * Arguments:
 *          mediaGroupAttr - instance of RvSdpMediaGroupAttr.
 *          txtVal - the buffer for the value.
 ***************************************************************************/
RVSDPCOREAPI char* rvSdpMediaGroupAttrGetValue(RvSdpMediaGroupAttr* mediaGroupAttr, char *txtVal);

#endif /*RV_SDP_MEDIA_GROUPING_ATTR */




#ifdef RV_SDP_PRECONDITIONS_ATTR
/***************************************************************************
 * rvSdpPreconditionAttrConstruct2
 * ------------------------------------------------------------------------
 * General: 
 *	Allocates and constructs the object using RvAlloc allocator.
 *  Returns the constructed object or NULL if it was failed.
 *  Precondition attribute represents 3 precond attributes: 
 *     1.'a=curr'  (current)
 *     2.'a=des'   (desired)
 *     3.'a=conf'  (confirmed)
 *  The syntax is: 'a=curr:preconditionType  statusType directionTag'
 *                 'a=des :preconditionType  strengthTag statusType directionTag'
 *                 'a=conf:preconditionType  statusType directionTag
 *
 *  Notice- that only the "des" attr contains the 'strengthTag'.for the others it's null.
 *  This attribute can apear only in media-descr level multiple appearence.  
 *  
 * Return Value: 
 *      A pointer to the new added RvSdpPreconditionAttr object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *          obj - points to RvSdpMsg or RvAlloc instance.
 *          precondTypeStr      - the precondition-type str.
 *          precondStatusStr    - the precondition-status str.
 *          precondStrengthStr  - the precondition-strength str.
 *          precondDirectionStr - the precondition-direction str.
 *          badSyn              - proprietary syntax precondition attribute or NULL.
 ***************************************************************************/
RVSDPCOREAPI RvSdpPreconditionAttr* rvSdpPreconditionAttrConstruct2(
                             void*                   obj,        
                             const char*             precondTypeStr, 
                             const char*             precondStatusStr,
                             const char*             precondStrengthStr,
                             const char*             precondDirectionStr,
                             const char*             badSyn);


/***************************************************************************
 * rvSdpAddPreconditionAttr2
 * ------------------------------------------------------------------------
 * General: 
 *      Constructs and  adds the Precondition atribute to media descr obj.
 *      (can be added only at media-descr level)   
 * Return Value: 
 *      A pointer to the added RvSdpPreconditionAttr object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *          descr - media where precond att will be added.
 *          commF - common fields pointer of message or media.
 *          precondName  - the precondition-name str.("curr"/"des"/"conf")
 *          typeStr      - the precondition-type str.
 *          statusStr    - the precondition-status str.
 *          strengthStr  - the precondition-strength str.
 *          directionStr - the precondition-direction str.
 *          badSyn - proprietary syntax Media-Group attribute.
 ***************************************************************************/
RVSDPCOREAPI RvSdpPreconditionAttr* rvSdpAddPreconditionAttr2(
                                   RvSdpMediaDescr  *      descr,  
                                   RvSdpCommonFields*      commF,
                                   const char*             precondName, 
                                   const char*             typeStr, 
                                   const char*             statusStr,
                                   const char*             strengthStr,
                                   const char*             directionStr,
                                   const char*             badSyn);

/***************************************************************************
 * rvSdpPreconditionAttrCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpPreconditionAttr from 'src' to 'dest'. 
 *      If the destination object is NULL pointer, the destination
 *      object will be allocated & constructed within the function.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpPreconditionAttr object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpPreconditionAttr object or NULL. 
 *      src - a pointer to the source object
 *      obj - the RvSdpMsg instance that will own the destination object
 *            or pointer to allocator.
 ***************************************************************************/
RVSDPCOREAPI RvSdpPreconditionAttr* rvSdpPreconditionAttrCopy2(
                                       RvSdpPreconditionAttr      * dest, 
                                       const RvSdpPreconditionAttr* src, 
                                       void                  * obj);

/***************************************************************************
 * rvSdpPreconditionAttrDestruct
 * ------------------------------------------------------------------------
 * General: 
 *      Destroys the instance of RvSdpPreconditiontAttr.
 *          
 * Return Value: 
 *      None.
 * ------------------------------------------------------------------------
 * Arguments:
 *      precondAttr - precond object that should be destructed.
 ***************************************************************************/
RVSDPCOREAPI void rvSdpPreconditionAttrDestruct(RvSdpPreconditionAttr* precondAttr);


/***************************************************************************
 * rvSdpPreconditionAttrGetValue
 * ------------------------------------------------------------------------
 * General: 
 *      Prints the textual value of precondition attribute  into provided 
 *      buffer (prints all the fields values following one of the precondition names 
 *      1. For example for: "a=curr:____________________".
 *         value can be:  "qos e2e send", 
 *      2. For example for: "a=des:_______________________" 
 *         value  can be :"qos e2e optional sendrecv"
 * Return Value: 
 *      Returns the buffer pointer on success or NULL if fails.
 * ------------------------------------------------------------------------
 * Arguments:
 *          precondAttr - instance of RvSdpPreconditionAttr.
 *          txtVal      - the buffer for the precond value.
 ***************************************************************************/
RVSDPCOREAPI char* rvSdpPreconditionAttrGetValue(RvSdpPreconditionAttr* precondAttr, 
                                                 char                 * txtVal);


/***************************************************************************
 * rvSdpPreconditionAttrReshuffle
 * ------------------------------------------------------------------------
 * General: 
 *      Called during message strings buffer reshuffle.
 *      Copies all string fields of Media-Group into new '*ptr' while
 *      increasing '*ptr' value by the length of copied strings. 
 *      reshuffle- is called when 
 * Return Value: void
 * ------------------------------------------------------------------------
 * Arguments:
 *      PreconditionAttr - a pointer to the RvSdpPreconditionAttr object.
 *      ptr            - new pointer to the new string buffer (after reshuffling)
 *
 ***************************************************************************/
void rvSdpPreconditionAttrReshuffle(RvSdpPreconditionAttr* PrecondAttr, char** ptr);

/***************************************************************************
 * rvSdpPreconditionAttrParsing
 * ------------------------------------------------------------------------
 * General: 
 *	   Special attribute parse function for the precondition attribute. 
 *     Parses precondition attribute field and constructs the RvSdpPreconditionAttr object.
 *     The constructed object is added to current context media. 
 *     (precondition attr can appear only in the media-descr level.
 *
 *     An Example to precondition attribute that should be parsed may look like:    
 *          "a=curr:qos e2e send"                or:  
 *          "a=des:qos remote optional sendrecv" or:
 *          "a=conf:qos local recv "
 *
 * Return Value: 
 *     If parsing or construction fails the correspondent status is returned.
 * ------------------------------------------------------------------------
 * Arguments:
 *  specAttrData      - the special attribute data.
 *  createAsBadSyntax - if set to RV_TRUE the bad syntax 
 *                      precondition attribute will be created.
 *  parserData        - a pointer to the parser data instance.
 *  commonFields      - the current context common fields instance,
                        here the special attribute will be added
 *  name - the Precondition Attr name.
 *  val  - the Precondition Attr val.
 ***************************************************************************/
RvSdpSpecAttrParseSts rvSdpPreconditionAttrParsing(
				const RvSdpSpecAttributeData* specAttrData, /* the special attribute data */
                RvBool              createAsBadSyntax, /* if set to RV_TRUE the bad 
                                                          syntax precondition attribute  
                                                          will be created */
                RvSdpParserData   * parserData,   /* the parser data instance */        
                RvSdpCommonFields * commonFields, /* the current context common fields instance,
                                                     here the special attribute will be added */
				char              * name,      /* the attribute name */
                char              * val,       /* the attribute value to parse */
                REPLACING_DECL)        /* used for zero-substitutions in input buffer */;


#endif /* RV_SDP_PRECONDITIONS_ATTR */


#ifdef RV_SDP_RTCP_ATTR

/***************************************************************************
 * rvSdpRtcpAttrCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the instance of RvSdpRtcpAttr from 'src' to 'dest'. 
 *      If the destination object is NULL pointer, the destination
 *      object will be allocated & constructed within the function.
 *          
 * Return Value: 
 *      A pointer to the input RvSdpRtcpAttr object, or NULL if the 
 *      function fails
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - pointer to valid RvSdpRtcpAttr object or NULL. 
 *      src - a pointer to the source object
 *      obj - the RvSdpMsg instance that will own the destination object
 *            or pointer to allocator.
 ***************************************************************************/
RVSDPCOREAPI RvSdpRtcp* rvSdpRtcpAttrCopy2(
        RvSdpRtcp      *dst, 
        const RvSdpRtcp *src, 
        void *obj);


/***************************************************************************
 * rvSdpRtcpAttrDestruct
 * ------------------------------------------------------------------------
 * General: Destroys the instance of RvSdpRtcp.
 *          
 * Return Value:  None.
 *     
 * ------------------------------------------------------------------------
 * Arguments:
 *      rtcp - object to be destroyed.
 ***************************************************************************/
RVSDPCOREAPI void rvSdpRtcpAttrDestruct(RvSdpRtcp* rtcp);


/***************************************************************************
 * rvSdpRtcpAttrGetValue
 * ------------------------------------------------------------------------
 * General: 
 *      Prints the textual value of rtcp attribute  into provided 
 *      buffer (all the fields values following "a=rtcp:_________________ ".
 *          
 * Return Value: 
 *      Returns the buffer pointer on success or NULL if fails.
 * ------------------------------------------------------------------------
 * Arguments:
 *          rtcp - instance of RvSdpRtcp.
 *          txtVal - the buffer for the value.
 ***************************************************************************/
RVSDPCOREAPI char* rvSdpRtcpAttrGetValue(
    RvSdpRtcp* rtcp, 
    char *txtVal);

#endif /*#ifdef RV_SDP_RTCP_ATTR */



/***************************************************************************
 * rvSdpCommonFieldCopy2
 * ------------------------------------------------------------------------
 * General: 
 *      Copies the contents of 'src' RvSdpCommonFields instance to the 'dest'
 *      RvSdpCommonFields instance.
 *          
 * Return Value: 
 *      Returns RV_SDPSTATUS_OK if the function succeeds, or an error code if the 
 *      function fails.
 * ------------------------------------------------------------------------
 * Arguments:
 *      dest - the destination RvSdpCommonFields instance.
 *      src - the source RvSdpCommonFields instance.
 *      obj - the RvSdpMsg pointer owning common fields object of message or 
 *            media descriptor OR the RvAlloc allocator.
 ***************************************************************************/
RVSDPCOREAPI RvSdpStatus rvSdpCommonFieldCopy2(
            struct _RvSdpCommonFields* dest, 
            const struct _RvSdpCommonFields* src, 
            void* obj);             

/***************************************************************************
 * rvSdpCommonFieldCompare
 * ------------------------------------------------------------------------
 * General: 
 *      Compares common fields.
 *          
 * Return Value:
 *      RV_TRUE, if common fields 1 represent exact copy of common fields 2.
 * ------------------------------------------------------------------------
 * Arguments:
 *      commonFields1 - RvSdpCommonFields instance.
 *      commonFields2 - RvSdpCommonFields instance.
 ***************************************************************************/
RVSDPCOREAPI RvBool rvSdpCommonFieldCompare(
            const struct _RvSdpCommonFields* commonFields1,
            const struct _RvSdpCommonFields* commonFields2);

/***************************************************************************
 * rvSdpAttributeCompare
 * ------------------------------------------------------------------------
 * General: 
 *      Compares two RvSdpAttribute objects.
 *          
 * Return Value:
 *      RV_TRUE, if attributes are equal.
 * ------------------------------------------------------------------------
 * Arguments:
 *      a1 - RvSdpAttribute object.
 *      a2 - RvSdpAttribute object.
 ***************************************************************************/
RVSDPCOREAPI RvBool rvSdpAttributeCompare(
            const RvSdpAttribute* a1,
            const RvSdpAttribute* a2);

/* sets default SDP allocator */
RVSDPCOREAPI void rvSdpSetDefaultAllocator(RvAlloc* a);

/* 
 * Enum Txt->Value and Value->Txt functions 
 */

/* connection mode */
RVSDPCOREAPI const char* rvSdpConnModeVal2Txt(
            RvSdpConnectionMode m);
RVSDPCOREAPI RvSdpConnectionMode rvSdpConnModeTxt2Val(
            const char* txt);

/* network type */
RVSDPCOREAPI RvSdpNetType rvSdpNetTypeTxt2Val(
            char* p);
RVSDPCOREAPI const char* rvSdpNetTypeVal2Txt(
            RvSdpNetType t);

/* address type */
RVSDPCOREAPI RvSdpAddrType rvSdpAddrTypeTxt2Val(
            char* p);
RVSDPCOREAPI const char* rvSdpAddrTypeVal2Txt(
            RvSdpAddrType t);

/* encryption type */
RVSDPCOREAPI RvSdpEncrMethod rvSdpKeyTypeTxt2Val(
            const char *txt);
RVSDPCOREAPI const char* rvSdpKeyTypeVal2Txt(
            RvSdpEncrMethod m);

/* media descriptor protocol type */
RVSDPCOREAPI RvSdpProtocol rvSdpMediaProtoTxt2Val(
            const char* protoTxt);
RVSDPCOREAPI const char* rvSdpMediaProtoVal2Txt(
            RvSdpProtocol p);

#ifdef RV_SDP_MEGACO_ENABLED
/* vcid type */
RVSDPCOREAPI RvSdpVcId rvSdpVcidTypeTxt2Val(const char* txt);
RVSDPCOREAPI const char* rvSdpVcidTypeTypeVal2Txt(RvSdpVcId id);
#endif /*RV_SDP_MEGACO_ENABLED*/


/* media descriptor type */
RVSDPCOREAPI RvSdpMediaType rvSdpMediaTypeTxt2Val(
            const char* mediaTxt);
RVSDPCOREAPI const char* rvSdpMediaTypeVal2Txt(
            RvSdpMediaType m);

/* key management protocol type */
#ifdef RV_SDP_KEY_MGMT_ATTR
struct _RvSdpKeyMgmtAttr;
RVSDPCOREAPI RvSdpKeyMgmtPrtclType rvSdpKeyMgmtPrtclTypeTxt2Val(
            const char* KeyMgmtPrtclTxt);
RVSDPCOREAPI const char* rvSdpKeyMgmtPrtclTypeVal2Txt(
            RvSdpKeyMgmtPrtclType m);
#endif /*RV_SDP_KEY_MGMT_ATTR*/


/* media group semantics type */
#ifdef RV_SDP_MEDIA_GROUPING_ATTR
struct _RvSdpMediaGroupAttr;
RVSDPCOREAPI RvSdpGroupSemanticsType rvSdpGroupSemanticTypeTxt2Val(const char* semanticTxt);
RVSDPCOREAPI const char* rvSdpGroupSemanticTypeVal2Txt(RvSdpGroupSemanticsType eSemanticVal);
#endif /*RV_SDP_MEDIA_GROUPING_ATTR*/



/* Preconditions */
#ifdef RV_SDP_PRECONDITIONS_ATTR
struct _RvSdpPreconditionAttr;
RVSDPCOREAPI const char* rvSdpPreconditionNameVal2Txt(RvSdpPrecondName ePrecondName);
RVSDPCOREAPI RvSdpPrecondName rvSdpPreconditionNameTxt2Val(const char* nameTxt);

RVSDPCOREAPI RvSdpPreconditionType rvSdpPreconditionTypeTxt2Val(const char* precondTypeTxt);
RVSDPCOREAPI const char*   rvSdpPreconditionTypeVal2Txt(RvSdpPreconditionType ePrecondType);

RVSDPCOREAPI RvSdpPreconditionStatusType rvSdpPreconditionStatusTxt2Val(const char* precStatusTxt);
RVSDPCOREAPI const char*   rvSdpPreconditionStatusVal2Txt(RvSdpPreconditionStatusType ePrecStatus);

RVSDPCOREAPI RvSdpPreconditionStrengthTag rvSdpPreconditionStrengthTxt2Val(const char* strengthTxt);
RVSDPCOREAPI const char*   rvSdpPreconditionStrengthVal2Txt(RvSdpPreconditionStrengthTag eStrength);

RVSDPCOREAPI RvSdpPreconditionDirectionTag rvSdpPreconditionDirectionTxt2Val(const char* directionTxt);
RVSDPCOREAPI const char*   rvSdpPreconditionDirectionVal2Txt(RvSdpPreconditionDirectionTag eDirection);
#endif /*RV_SDP_PRECONDITIONS_ATTR*/




#ifdef __cplusplus
}
#endif
    

#endif /*_rvsdpapiprivate_h_*/


