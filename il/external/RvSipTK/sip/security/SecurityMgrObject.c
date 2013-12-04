/*
*******************************************************************************
*                                                                             *
* NOTICE:                                                                     *
* This document contains information that is confidential and proprietary to  *
* RADVision LTD.. No part of this publication may be reproduced in any form   *
* whatsoever without written prior approval by RADVision LTD..                *
*                                                                             *
* RADVision LTD. reserves the right to revise this publication and make       *
* changes without obligation to notify any person of such revisions or changes*
*******************************************************************************
*/

/******************************************************************************
 *                              <SecurityMgrObject.c>
 *
 * The SecurityMgrObject.c file implements the Security Object object.
 *
 *
 *    Author                        Date
 *    ------------                  ------------
 *    Igor                          Februar 2006
 *****************************************************************************/

/*---------------------------------------------------------------------------*/
/*                            INCLUDE HEADER FILES                           */
/*---------------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonUtils.h"

#ifdef RV_SIP_IMS_ON

#include "SecurityObject.h"
#include "SecurityMgrObject.h"

/*---------------------------------------------------------------------------*/
/*                             TYPE DEFINITIONS                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                          STATIC FUNCTIONS PROTOTYPES                      */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                             MODULE FUNCTIONS                              */
/*---------------------------------------------------------------------------*/

/******************************************************************************
 * SecMgrCreateSecObj
 * ----------------------------------------------------------------------------
 * General: Allocates a Security Object.
 *
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr       - Pointer to the Security Manager
 *          hAppSecObj - Application handle to the allocated Security Object
 * Output:  phSecObj   - Pointer to the allocated Security Object
*****************************************************************************/
RvStatus SecMgrCreateSecObj(IN  SecMgr*              pMgr,
                            IN  RvSipAppSecObjHandle hAppSecObj,
                            OUT RvSipSecObjHandle*   phSecObj)
{
    RvStatus            rv;
    SecObj*             pSecObj;
    RLIST_ITEM_HANDLE   listItem;

    rv = RLIST_InsertTail(pMgr->hSecObjPool, pMgr->hSecObjList, &listItem);
    if(RV_OK != rv)
    {
        *phSecObj = NULL;
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SecMgrCreateSecObj(pMgr=%p) - Failed to allocate new Security Object(rv=%d:%s)",
            pMgr, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    pSecObj = (SecObj*)listItem;

    rv = SecObjInitialize(pSecObj,pMgr);
    if(RV_OK != rv)
    {
        RLIST_Remove(pMgr->hSecObjPool, pMgr->hSecObjList, listItem);
        *phSecObj = NULL;
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
            "SecMgrCreateSecObj(pMgr=%p) - Failed to initialize SecObj %p. Remove it.(rv=%d:%s)",
            pMgr, pSecObj, rv, SipCommonStatus2Str(rv)));
        return rv;
    }

    pSecObj->hAppSecObj = hAppSecObj;

    RvLogInfo(pMgr->pLogSrc,(pMgr->pLogSrc,
        "SecMgrCreateSecObj(pMgr=%p) - SecObj %p was created",
        pMgr, pSecObj));

    *phSecObj = (RvSipSecObjHandle)pSecObj;

    return RV_OK;
}

/*---------------------------------------------------------------------------*/
/*                              STATIC FUNCTIONS                             */
/*---------------------------------------------------------------------------*/

#endif /*#ifdef RV_SIP_IMS_ON*/

