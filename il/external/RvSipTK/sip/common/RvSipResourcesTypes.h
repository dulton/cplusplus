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
*********************************************************************************
*/


/*********************************************************************************
 *                              <RvSipResourcesTypes.h>
 *
 *
 *
 *    Author                         Date
 *    ------                        ------
 *    Sarit Mekler                  Nov 2000
 *********************************************************************************/


#ifndef RV_SIP_RESOURCES_TYPES_H
#define RV_SIP_RESOURCES_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "RV_SIP_DEF.h"
#include "RV_ADS_DEF.h"


/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/


/*
 * RvSipResource
 * --------------------
 * A general resource structure used by all modules
 * numOfAllocatedElements - The total number of elements that the SIP Stack allocated.
 * currNumOfUsedElements  - The number of elements that are currently being used.
 * maxUsageOfElements     - The maximum number of elements that were used concurrently until the present.
 */
typedef struct
{
    RvUint32       numOfAllocatedElements;
    RvUint32       currNumOfUsedElements;
    RvUint32       maxUsageOfElements;
} RvSipResource;

#ifndef RV_SIP_PRIMITIVES
/*
 * RvSipCallLegResources
 * --------------------
 * The resources of the Call-leg module.
 * The structure contains the resource information for the following resources:
 * calls         - The resource information of the call-leg objects.
 * transcLists   - Every call-leg holds one list of 'transaction handle' objects
 *                 to manage all its related transactions. This is the resource
 *                 information of all the allocated transactions lists.
 * transcHandles - This is the resource information of all the 'transaction handle'
 *                 objects that are allocated by all call-legs
 * inviteLists   - Every call-leg holds one list of 'invite' objects
 *                 to manage all its invite transactions. This is the resource
 *                 information of all the allocated invite lists.
 * inviteObjects - This is the resource information of all the 'invite'
 *                 objects that are allocated by all call-legs.
 */

typedef struct
{
    RvSipResource   calls;
    RvSipResource   transcLists;
    RvSipResource   transcHandles;
    RvSipResource   inviteLists;
    RvSipResource   inviteObjects;
} RvSipCallLegResources;

/*
 * RvSipRegClientResources
 * --------------------
 * The resources of the Register-Client module.
 * The structure contains the resource information for the following resources:
 * regClients - The resource information of the register-client objects.
 */
typedef struct
{
    RvSipResource   regClients;
} RvSipRegClientResources;

/*
 * RvSipPubClientResources
 * --------------------
 * The resources of the Publish-Client module.
 * The structure contains the resource information for the following resources:
 * pubClients  - The resource information of the publish-client objects.
 */
typedef struct
{
    RvSipResource   pubClients;
} RvSipPubClientResources;

#endif /* RV_SIP_PRIMITIVES */
/*
 * RvSipTransportResources
 * --------------------
 * Defines the resources of the Transport module.
 * The structure contains the resource information for the following resources:
 * connections  - The resource information of the connection objects.
 * connectionsNoOwners - The resource information of the allocated connection which
 *                does not have owners.
 * pQueueElements - The resource information of the allocated queue elements. This
 *                queue managees all the non-synchronious actions.
 * pQueueEvents - The resource information of the allocated queue event elements. This
 *                queue manages all the non-synchronious actions.
 * readBuffers  - The resource information of the allocated read-buffers.
 * connHash     - The resource information of the connection hash table.
 * ownersHash   - The resource information of the connection owners hash table.
 * tlsSessions  - The resource information of the TLS session objects.
 * tlsEngines   - The resource information of the TLS engine objects.
 * oorEvents    - The resource information of the allocated out-of-resource event elements pool.
 *                This pool manages all the pqueue events that failed to be processed because of an
 *                out-of-resource situation.
 */
typedef struct
{
    RvSipResource   connections;
    RvSipResource   connectionsNoOwners;
    RvSipResource   pQueueElements;
    RvSipResource   pQueueEvents;
    RvSipResource   readBuffers;
    RvSipResource   connHash;
    RvSipResource   ownersHash;
    RvSipResource   tlsSessions;
    RvSipResource   tlsEngines;
    RvSipResource   oorEvents;
} RvSipTransportResources;


/*
 * RvSipTranscResources
 * --------------------
 * The resources of the Transaction module.
 * The structure contains the resource information for the following resources:
 * transactions - The resource information of the transaction objects.
 */
typedef struct
{
    RvSipResource   transactions;
} RvSipTranscResources;


/*
 * RvSipStackResources
 * --------------------
 * The resources of the SIP Stack module.
 * This structure contains the resource information of the three memory pools that are used
 * by the SIP Stack: msgPoolElements, generalPoolElements and headerPoolElements.
 * The difference between the three memory pools is in the size of memory it supplies.
 * It also contains the information of the timers that are used by the entire SIP Stack.
 */
typedef struct
{
    RvSipResource   msgPoolElements;
    RvSipResource   generalPoolElements;
    RvSipResource   headerPoolElements;
    RvSipResource   timerPool;
} RvSipStackResources;

#ifndef RV_SIP_PRIMITIVES
/*
 * RvSipSubsResources
 * --------------------
 * Defines the resource structure of the Subscription module
 * The structure contains the resource information for the following resources:
 * subscriptions - The resource information of the subscription objects.
 * notifications - The resource information of the notify objects allocated by all subscriptions.
 * notifyLists   - Each subscription holds one list of notify objects
 *                 to manage all its notify objects. This is the resource
 *                 information of all the allocated notify lists.
 */

typedef struct
{
    RvSipResource   subscriptions;
    RvSipResource   notifications;
    RvSipResource   notifyLists;
} RvSipSubsResources;
#endif

/*
 * RvSipCompartmentResources
 * --------------------
 * Defines the resource structure of the Compartment module.
 * The structure contains the resource information for the following resources:
 * compartments - The resource information of the compartment objects.
 * sigCompIdElem - The resource information of the sigCompId strings pool.
 */
typedef struct {
    RvSipResource compartments;
	RvSipResource sigCompIdElem;
} RvSipCompartmentResources;

/*
 * RvSipTransmitterResources
 * --------------------
 * Defines the resource structure of the transmitter module.
 * The structure contains the resource information for the following resources:
 * transmitters - The resource information of the transmitter objects.
 */
typedef struct {
    RvSipResource transmitters;
}RvSipTransmitterResources;

/*
 * RvSipResolverResources
 * --------------------
 * Defines the resource structure of the Resolver module.
 * The structure contains the resource information for the following resources:
 * resolvers - The resource information of the resolver objects.
 */
typedef struct {
    RvSipResource resolvers;
}RvSipResolverResources;

#ifdef RV_SIP_IMS_ON
/*
 * RvSipSecAgreeResources
 * --------------------
 * The resources of the Security-Agreement module.
 * The structure contains the resource information for the following resources:
 * secAgrees  - The resource information of the security-agreement objects.
 * ownersHash - The resource information of the security-agreement owners hash table.
 */

typedef struct
{
    RvSipResource   secAgrees;
	RvSipResource   ownersHash;
	RvSipResource   ownersLists;
	RvSipResource   ownersHandles;
} RvSipSecAgreeResources;

/*
 * RvSipSecurityResources
 * --------------------
 * The resources of the Security module.
 * The structure contains the resource information for the following resources:
 * secobjPoolElements - The resource information of the security objects.
 * ipsecsessionPoolElements - The resource information of the ip-sec session objects.
 */
typedef struct
{
    RvSipResource   secobjPoolElements;
    RvSipResource   ipsecsessionPoolElements;
} RvSipSecurityResources;
#endif /* RV_SIP_IMS_ON */

#ifdef __cplusplus
}
#endif


#endif /* END OF: #ifndef RV_SIP_RESOURCES_TYPES_H */
