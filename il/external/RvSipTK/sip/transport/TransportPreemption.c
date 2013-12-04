/************************************************************************************************************************
NOTICE:
This document contains information that is proprietary to RADVision LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVision LTD..

RADVision LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

************************************************************************************************************************/



/*********************************************************************************
 *                              TransportPreemption.h
 *
 * provides preemptions calls and convertions
 *
 *    Author                         Date
 *    ------                        ------
 *    Udi Tir0sh                    6.7.2004
 *********************************************************************************/

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "_SipCommonConversions.h"
#include "TransportMgrObject.h"
#include "TransportPreemption.h"
#include "_SipCommonCore.h"
#include "TransportProcessingQueue.h"

/*-----------------------------------------------------------------------*/
/*                                MACROES                                */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                        STATIC FUNCTIONS PROTOTYPES                    */
/*-----------------------------------------------------------------------*/

static void TransportPreemptionSelectPreemptionDispatchEventsEvHandler(
											   IN RvSelectEngine    *selectEngine,
  										       IN RvUint8            preemptEv,
											   IN void              *context);

static void TransportPreemptionSelectPreemptionReadBuffersReadyEvHandler(
											   IN RvSelectEngine    *selectEngine,
											   IN RvUint8            preemptEv,
											   IN void              *context);


static void TransportPreemptionSelectPreemptionCellAvailableEvHandler(
											   IN RvSelectEngine    *selectEngine,
											   IN RvUint8            preemptEv,
											   IN void              *context);


/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/
/***************************************************************************
 * TransportPreemptionDestruct
 * ------------------------------------------------------------------------
 * General:  Creates the preemption callback and initializes the thread 
 *           variable for the preemption users array
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr    - transport manager.
 ***************************************************************************/
void RVCALLCONV  TransportPreemptionDestruct(IN    TransportMgr*    pMgr)
{
    RvSelectSetPreemptionCbEx(pMgr->pSelect,
                              NULL,
                              NULL,
                              &pMgr->preemptionDispatchEvents);
    pMgr->preemptionDispatchEvents = 0;

    RvSelectSetPreemptionCbEx(pMgr->pSelect,
                              NULL,
                              NULL,
                              &pMgr->preemptionReadBuffersReady);
    pMgr->preemptionReadBuffersReady = 0;

    RvSelectSetPreemptionCbEx(pMgr->pSelect,
                              NULL,
                              NULL,
                              &pMgr->preemptionCellAvailable);
    pMgr->preemptionCellAvailable = 0;
}

/***************************************************************************
 * TransportPreemptionConstruct
 * ------------------------------------------------------------------------
 * General:  Registers the callbacks for the different preemption events
 * Return Value: 
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pMgr    - transport manager.
 ***************************************************************************/
RvStatus RVCALLCONV  TransportPreemptionConstruct(IN    TransportMgr*    pMgr)
{

  	/*register the 3 preemption callbacks*/

    RvStatus            crv = RV_OK;
	
    crv = RvSelectSetPreemptionCbEx(pMgr->pSelect,
									TransportPreemptionSelectPreemptionDispatchEventsEvHandler,
									(void*)pMgr, &pMgr->preemptionDispatchEvents);
	
    if (RV_OK != crv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
			"TransportPreemptionConstruct - Failed to set preemption callback (%d)", crv))
			return crv;
    }
	
    crv = RvSelectSetPreemptionCbEx(pMgr->pSelect,
									TransportPreemptionSelectPreemptionReadBuffersReadyEvHandler,
									(void*)pMgr, &pMgr->preemptionReadBuffersReady);
	
    if (RV_OK != crv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
			"TransportPreemptionConstruct - Failed to set preemption callback (%d)", crv))
			return crv;
    }
	
    crv = RvSelectSetPreemptionCbEx(pMgr->pSelect,
								    TransportPreemptionSelectPreemptionCellAvailableEvHandler,
									(void*)pMgr, &pMgr->preemptionCellAvailable);
	
    if (RV_OK != crv)
    {
        RvLogError(pMgr->pLogSrc,(pMgr->pLogSrc,
			"TransportPreemptionConstruct - Failed to set preemption callback (%d)", crv))
    }
    return CRV2RV(crv);;
	
}


/*-----------------------------------------------------------------------*/
/*                             STATIC FUNCTIONS                          */
/*-----------------------------------------------------------------------*/
/************************************************************************************
 * TransportPreemptionSelectPreemptionDispatchEventsEvHandler
 * ----------------------------------------------------------------------------------
 * General:  this callback is invoked for a message waiting event
 *
 * ----------------------------------------------------------------------------------
 * Arguments:
 * input   : selectEngine    - Events engine
 *           preemptionEvent - Preemption Event
 *           context         - predefined user context
 * Output:  none
 ***********************************************************************************/
static void TransportPreemptionSelectPreemptionDispatchEventsEvHandler(
												IN RvSelectEngine    *selectEngine,
												IN RvUint8            preemptEv,
												IN void              *context)

{
	TransportMgr*       pMgr    = (TransportMgr*)context;
	TransportProcessingQueueDispatchEvents(pMgr);
    RV_UNUSED_ARG(selectEngine);
    RV_UNUSED_ARG(preemptEv);

}




/************************************************************************************
 * TransportPreemptionSelectPreemptionReadBuffersReadyEvHandler
 * ----------------------------------------------------------------------------------
 * General:  this callback is invoked for a read buffers ready event
 *
 * ----------------------------------------------------------------------------------
 * Arguments:
 * input   : selectEngine    - Events engine
 *           preemptionEvent - Preemption Event
 *           context         - predefined user context
 * Output:  none
 ***********************************************************************************/
static void TransportPreemptionSelectPreemptionReadBuffersReadyEvHandler(
												IN RvSelectEngine    *selectEngine,
												IN RvUint8            preemptEv,
												IN void              *context)

{
	TransportMgr*       pMgr    = (TransportMgr*)context;
	TransportProcessingQueueResourceAvailable(pMgr,TRANSPORT_READ_BUFFRES_READY);
    RV_UNUSED_ARG(selectEngine);
    RV_UNUSED_ARG(preemptEv);

}


/************************************************************************************
 * TransportPreemptionSelectPreemptionCellAvailableEvHandler
 * ----------------------------------------------------------------------------------
 * General:  this callback is invoked for a read buffers ready event
 *
 * ----------------------------------------------------------------------------------
 * Arguments:
 * input   : selectEngine    - Events engine
 *           preemptionEvent - Preemption Event
 *           context         - predefined user context
 * Output:  none
 ***********************************************************************************/
static void TransportPreemptionSelectPreemptionCellAvailableEvHandler(
												IN RvSelectEngine    *selectEngine,
												IN RvUint8            preemptEv,
												IN void              *context)

{
	TransportMgr*       pMgr    = (TransportMgr*)context;
	TransportProcessingQueueResourceAvailable(pMgr,PQUEUE_CELL_AVAILABLE);
    RV_UNUSED_ARG(selectEngine);
    RV_UNUSED_ARG(preemptEv);

}


