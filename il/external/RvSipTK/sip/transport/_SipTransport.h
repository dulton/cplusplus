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
 *                              _SipTransport.h
 *
 *  This file provides headers for the transport layer of the sip stack.
 *
 *
 *    Author                         Date
 *    ------                        ------
 *  Oren Libis                    20-Nov-2000
 *********************************************************************************/

#ifndef _SIP_TRANSPORT_H
#define _SIP_TRANSPORT_H


#ifdef __cplusplus
extern "C" {
#endif


/*-----------------------------------------------------------------------*/
/*                        INCLUDE HEADER FILES                           */
/*-----------------------------------------------------------------------*/
#include "rvsemaphore.h"


#include "AdsRpool.h"
#include "_SipTransportTypes.h"
#include "TransportDNS.h"
#include "RvSipTransportTypes.h"
#include "RvSipResourcesTypes.h"
#ifdef RV_SIGCOMP_ON
#include "RvSigComp.h"
#endif /* RV_SIGCOMP_ON */
#if (RV_NET_TYPE & RV_NET_SCTP)
#include "RvSipTransportSctpTypes.h"
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
    
/*-----------------------------------------------------------------------*/
/*                          Configuration                                */
/*-----------------------------------------------------------------------*/
#if ((RV_NET_TYPE & RV_NET_SCTP) && (RV_OS_TYPE == RV_OS_TYPE_LINUX))
/* This flag enables customer to mix IPv4 and IPv6 address,
   when open IPv6 socket. Currently is used for SCTP only.
   Currently mixed addresses are supported on Linux only. */
#define TRANSPORT_SCTP_MIXEDIPV4IPV6_ON
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

/*-----------------------------------------------------------------------*/
/*                          TYPE DEFINITIONS                             */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*                          FUNCTIONS HEADERS                            */
/*-----------------------------------------------------------------------*/

/************************************************************************************
 * SipTransportConstruct
 * ----------------------------------------------------------------------------------
 * General: Construct and initialize the transport module.
 *
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportCfg        - Configuration needed for constructing the
 *                                 transport module.
 * Output:  hTransportMgr        - Handle to the transport manager.
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportConstruct(
                                   INOUT SipTransportCfg        *pTransportCfg,
                                   OUT RvSipTransportMgrHandle  *hTransportMgr);



/************************************************************************************
 * SipTransportStopProcessingThreads
 * ----------------------------------------------------------------------------------
 * General: Stops transport processing threads. Must be done before
 *          freeing allocated memory.
 *
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportHandle    - Handle to transport instance
 * Output:  none
 ***********************************************************************************/
void RVCALLCONV SipTransportStopProcessingThreads(IN RvSipTransportMgrHandle hTransportHandle);


/************************************************************************************
 * SipTransportDestruct
 * ----------------------------------------------------------------------------------
 * General: Destruct the transport layer module and release the used memory.
 *
 * Return Value: RvStatus - RV_OK, RV_ERROR_UNKNOWN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr    - Handle to the transport manager
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportDestruct(IN RvSipTransportMgrHandle hTransportMgr);



/************************************************************************************
 * SipTransportSetEventHandler
 * ----------------------------------------------------------------------------------
 * General: Set callback functions of the transport module.
 * Return Value: RvStatus - RV_OK, RV_ERROR_INVALID_HANDLE
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle to the transport manager.
 *          evHandlers      - struct with pointers to callback functions.
 *          evHandlersSize  - ev handler struct size
 *          hContext - Will be used as a parameter for the callbacks.
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportSetEventHandler(
                                        IN RvSipTransportMgrHandle hTransportMgr,
                                        IN SipTransportEvHandlers  *evHandlers,
                                        IN RvUint32               evHandlersSize,
                                        IN SipTransportCallbackContextHandle  hContext);

/******************************************************************************
 * SipTransportSendObjectEvent
 * ----------------------------------------------------------------------------
 * General: Insert an object termination event into the processing queue.
 *          Note - if you want to use an event for state, please use the 
 *          SipTransportSendStateObjectEvent function.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTransportMgr       - transport handler
 *          pObj                - Pointer to the object sending the event.
 *          pEventInfo          - empty allocated structure.
 *          eNum                - event enumeration, may be used for reason or state
 *          func                - callback function
 *          bInternal           - is the event stack or external to stack
 *          pLoggingInfo        - logging message limited to TRANSPORT_EVENT_LOG_LEN len
 *          pObjName            - the name of the object that sends the message
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportSendTerminationObjectEvent(
                        IN RvSipTransportMgrHandle             hTransportMgr,
                        IN void*                               pObj,
                        IN RvSipTransportObjEventInfo*         pEventInfo,
                        IN RvInt32                             eNum,
                        IN RvSipTransportObjectEventHandler    func,
                        IN RvBool                              bInternal,
                        IN const RvChar*                       pLoggingInfo,
                        IN const RvChar*                       pObjName);

/******************************************************************************
 * SipTransportSendObjectEventExt
 * ----------------------------------------------------------------------------
 * General: Insert a state-changed event into the processing queue.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          hTransportMgr       - transport handler
 *          pObj                - Pointer to the object sending the event.
 *          objUniqueIdentifier - the unique id of the object to terminate. when the event
 *                                pops, before handling the event we check that the object
 *                                was not reallocated, using this identifier.
 *          param1        - param, provided to the sending Object on pop event 
 *          param2        - param, provided to the sending Object on pop event 
 *          func                - callback function
 *          bInternal           - is the event stack or external to stack
 *          pLoggingInfo        - logging message limited to TRANSPORT_EVENT_LOG_LEN len
 *          pObjName            - the name of the object that sends the message
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportSendStateObjectEvent(
                        IN RvSipTransportMgrHandle             hTransportMgr,
                        IN void*                               pObj,
                        IN RvInt32                             objUniqueIdentifier,
                        IN RvInt32                             param1,
                        IN RvInt32                             param2,
                        IN SipTransportObjectStateEventHandler func,
                        IN RvBool                              bInternal,
                        IN const RvChar*                       pLoggingInfo,
                        IN const RvChar*                       pObjName);

#ifdef RV_SIGCOMP_ON
/***************************************************************************
 * SipTransportMgrSetSigCompMgrHandle
 * ------------------------------------------------------------------------
 * General: Stores the SigComp manager handle in the transport manager
 *
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr   - Handle to the stack  transport manager.
 *           hSigCompMgr     - Handle of the SigComp manager handle to be 
 *                             stored.
 * Output:   -
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportMgrSetSigCompMgrHandle(
                      IN RvSipTransportMgrHandle hTransportMgr,
                      IN RvSigCompMgrHandle      hSigCompMgr);

/***************************************************************************
 * SipTransportSetObjectOutboundSigcompId
 * ------------------------------------------------------------------------
 * General: Sets the outbound sigcomp-id string for a given object.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr			- The transport manager.
 *          pOutboundAddr			- An outbound addr structure of a stack object
 *          strOutboundSigcompId	- The outbound host to be set to this object.
 *          hObjPool				- The memory pool the object uses.
 *          hObjPage				- The memory page the object uses.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportSetObjectOutboundSigcompId(
                            IN  RvSipTransportMgrHandle     hTransportMgr,
                            IN  SipTransportOutboundAddress *pOutboundAddr,
                            IN  RvChar                      *strOutboundSigcompId,
                            IN  HRPOOL                      hObjPool,
                            IN  HPAGE                       hObjPage);

/***************************************************************************
 * SipTransportGetObjectOutboundSigcompId
 * ------------------------------------------------------------------------
 * General: Copies the used outbound sigcomp-id to given parameters.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr - The transport manager.
 *           pOutboundAddr - An outbound addr structure of a stack object
 *           strOutboundSigcompId - The outbound sigcompId used by this object.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportGetObjectOutboundSigcompId(
                            IN  RvSipTransportMgrHandle     hTransportMgr,
                            IN  SipTransportOutboundAddress *pOutboundAddr,
                            OUT RvChar                      *strOutboundSigcompId);

#endif /* RV_SIGCOMP_ON */

/***************************************************************************
 * SipTransportMgrIsSupportServerConnReuse
 * ------------------------------------------------------------------------
 * General: Return true if application is registered on pfnEvConnServerReuse.
 * Return Value: RV_TRUE  - supports connection reuse.
 *               RV_FALSE - does not support connection reuse.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - The handle to the transport manager.
 ***************************************************************************/
RvBool RVCALLCONV SipTransportMgrIsSupportServerConnReuse(
                         IN  RvSipTransportMgrHandle    hTransportMgr);

/*-----------------------------------------------------------------------
                         LOCAL ADDRESS FUNCTIONS
  -----------------------------------------------------------------------*/

/************************************************************************************
 * SipTransportLocalAddressGetDefaultAddrByType
 * ----------------------------------------------------------------------------------
 * General: Get a default local address according to transaport and address type.
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr    - Handle to transport manager
 *          eTransportType   - required transport
 *          eAddressType     - required address type.
 *          pLocalAddresses   - handle to the matching local addresses.
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetDefaultAddrByType(
                            IN RvSipTransportMgrHandle      hTransportMgr,
                            IN RvSipTransport               eTransportType,
                            IN RvSipTransportAddressType    eAddressType,
                            IN RvSipTransportLocalAddrHandle  *phLocalAddress);

/************************************************************************************
 * SipTransportLocalAddressInitDefaults
 * ----------------------------------------------------------------------------------
 * General: Initialize the local addresses structure with a default local
 *          address of each address/transport type.The default is the first
 *          address found in the local address list.
 *
 * Return Value: RvStatus.
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr    - Handle to transport manager
 *          pLocalAddresses   - A structure with all types of local addresses.
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressInitDefaults(
                            IN RvSipTransportMgrHandle      hTransportMgr,
                            IN SipTransportObjLocalAddresses       *pLocalAddresses);


/***************************************************************************
 * SipTransportLocalAddrSetHandleInStructure
 * ------------------------------------------------------------------------
 * General: Set a local address handle in a local address
 *          structure.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 * Input:   hTransportMgr    - Handle to transport manager
 *          pLocalAddresses  - The local address structure.
 *          hLocalAddr - the local address handle
 *          bSetAddrInUse - set the supplied address as the used address
 ***************************************************************************/
void RVCALLCONV SipTransportLocalAddrSetHandleInStructure(
                            IN RvSipTransportMgrHandle         hTransportMgr,
                            IN SipTransportObjLocalAddresses  *pLocalAddresses,
                            IN  RvSipTransportLocalAddrHandle  hLocalAddr,
                            IN RvBool                          bSetAddrInUse);
/***************************************************************************
 * SipTransportSetLocalAddressForAllTransports
 * ------------------------------------------------------------------------
 * General: gets a string local ip and port and set it to all handles that have
 *          a matching address.
 * Return Value: (RvStatus)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr    - Handle to transport manager
 *            pLocalAddresses  - the local address structure to use.
 *            strLocalAddress - The local address string
 *            localPort - The local port.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportSetLocalAddressForAllTransports(
                            IN  RvSipTransportMgrHandle        hTransportMgr,
                            IN  SipTransportObjLocalAddresses *pLocalAddresses,
                            IN  RvChar                        *strLocalAddress,
                            IN  RvUint16                       localPort
#if defined(UPDATED_BY_SPIRENT)
			    , IN  RvChar                      *if_name
			    , IN  RvUint16                     vdevblock
#endif
			    );


/************************************************************************************
 * SipTransportLocalAddressGetAddressByHandle
 * ----------------------------------------------------------------------------------
 * General: returns the string Ip and port for a given local address handle.
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr    - Handle to transport manager
 *          hLocalAddr       - Local address handle
 *          bConvertZeroToDefaultHost - Specifies whether to convert a zero IP to
 *                                      the default host.
 *          bAddScopeIp - Specifies whether to add the scope id to the end of the
 *                         string ip.
 * Output   localIp          - string IP of this local address
 *          localPort        - port of this local address
 *          if_name          - local interface name
 *          vdevblock           - local hardware port
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetAddressByHandle(
                            IN  RvSipTransportMgrHandle      hTransportMgr,
                            IN  RvSipTransportLocalAddrHandle  hLocalAddr,
                            IN  RvBool                      bConvertZeroToDefaultHost,
                            IN  RvBool                      bAddScopeIp,
                            OUT RvChar                      *localIp,
                            OUT RvUint16                    *localPort
#if defined(UPDATED_BY_SPIRENT)
			    , OUT RvChar                    *if_name
			    , OUT RvUint16                  *vdevblock
#endif
			    );

/************************************************************************************
 * SipTransportLocalAddressGetBufferedAddressByHandle
 * ----------------------------------------------------------------------------------
 * General: returns the strings that represents the local address. 
 *          This includes the sentBy string if exist and the IP string and port.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr       - Local address handle
 *          sentByRpoolPtr   - RPool ptr of the sent-by host.
 *          sentByPort       - sent by port
 * Output   localIp          - string IP of this local address
 *          localPort        - port of this local address
 *          
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetBufferedAddressByHandle(
                            IN  RvSipTransportLocalAddrHandle  hLocalAddr,
							IN  RPOOL_Ptr*                     sentByRpoolPtr,
							OUT RvInt32*                       sentByPort,
                            OUT RvChar                         **localIp,
                            OUT RvUint16                       *localPort);

/************************************************************************************
 * SipTransportLocalAddressGetHandleByAddress
 * ----------------------------------------------------------------------------------
 * General: Returns the handle of a local address according to given local address
 *          characteristics.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr    - Handle to transport manager
 *          eTransportType   - tcp/udp
 *          eAddressType     - ip4/ip6
 *          strLocalIp       - string IP of this local address
 *          localPort        - port of this local address
 *          if_name          - local interface name
 *          vdevblock           - local hardware port
 * Output   hLocalAddr      - local address handle
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetHandleByAddress(
                            IN  RvSipTransportMgrHandle      hTransportMgr,
                            IN RvSipTransport                eTransportType,
                            IN RvSipTransportAddressType     eAddressType,
                            IN  RvChar                      *strLocalIp,
                            IN  RvUint16                    localPort,
#if defined(UPDATED_BY_SPIRENT)
                            IN  RvChar                     *if_name,
			    IN  RvUint16                    vdevblock, 
#endif
                            OUT RvSipTransportLocalAddrHandle  *phLocalAddr);


/************************************************************************************
 * SipTransportLocalAddressGetTransportType
 * ----------------------------------------------------------------------------------
 * General: Returns the transport type of a local address
 * Return Value: RvSipTransport
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr      - Local address handle
 *          pTransportType  - Pointer to memory, where the the type will be returned
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetTransportType(
                                    IN  RvSipTransportLocalAddrHandle  hLocalAddr,
                                    OUT RvSipTransport* pTransportType);

/************************************************************************************
 * SipTransportLocalAddressGetAddrType
 * ----------------------------------------------------------------------------------
 * General: Returns the address type of a local address
 * Return Value:RvSipTransportAddressType
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr      - local address handle
 *          pAddressType    - pointer to the memory, where the type will be returned
 * Output   (-)
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetAddrType(
                            IN  RvSipTransportLocalAddrHandle  hLocalAddr,
                            OUT RvSipTransportAddressType    *pAddressType);

/************************************************************************************
 * SipTransportLocalAddressGetPort
 * ----------------------------------------------------------------------------------
 * General: Returns the transport type of a local address
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - Handle to transport manager
 *          hLocalAddr      - Local address handle
 *          pPort           - port
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetPort(
                            IN  RvSipTransportMgrHandle      hTransportMgr,
                            IN  RvSipTransportLocalAddrHandle  hLocalAddr,
                            OUT RvUint16                    *pPort);

/************************************************************************************
 * SipTransportLocalAddressGetFirstAvailAddress
 * ----------------------------------------------------------------------------------
 * General: returns the address of the first defined UDP local address.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - Handle to transport manager
 * Output:  addr            - address structure
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetFirstAvailAddress(
                            IN  RvSipTransportMgrHandle      hTransportMgr,
                            IN  SipTransportAddress*         addr);

#if(RV_NET_TYPE & RV_NET_IPV6)
/************************************************************************************
 * SipTransportLocalAddressGetScopeId
 * ----------------------------------------------------------------------------------
 * General: Returns the scope ID of an Ipv6 address
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - Handle to transport manager
 *          hLocalAddr      - Local address handle
 *          pScodeId  -       scope id
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressGetScopeId(
                            IN  RvSipTransportMgrHandle      hTransportMgr,
                            IN  RvSipTransportLocalAddrHandle  hLocalAddr,
                            OUT RvInt32                    *pScodeId);
#endif


/*-----------------------------------------------------------------------
                         OUTBOUND ADDRESS FUNCTIONS
  -----------------------------------------------------------------------*/

/************************************************************************************
 * SipTransportGetOutboundAddress
 * ----------------------------------------------------------------------------------
 * General: returns the outbound address information.
 * Return Value: host name
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - Handle to transport manager
 * Output:  strIp           - outbound address ip
 *          strHost         - outbound address host name
 *          pPort           - outbound address port
 *          peTransport     - outbound address transport
 *          peCompType      - outbound compression type
 *          strSigcompId    - outbound sigcomp-id
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportGetOutboundAddress(
                            IN  RvSipTransportMgrHandle   hTransportMgr,
                            OUT RvChar**                  strIp,
                            OUT RvChar**                  strHost,
                            OUT RvUint16*                 pPort,
                            OUT RvSipTransport*           peTransport,
                            OUT RvSipCompType*            peCompType,
							OUT RvChar**                  strSigcompId);

/***************************************************************************
 * SipTransportInitObjectOutboundAddr
 * ------------------------------------------------------------------------
 * General: Initialize an outbound addr for a stack object such as call-leg
 *          transaction or Reg-Client.
 * Returned value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr - The transport manager.
 * Input Output:  outboundAddr - An outbound addr structrue
 ***************************************************************************/
void RVCALLCONV SipTransportInitObjectOutboundAddr (
                     IN    RvSipTransportMgrHandle     hTransportMgr,
                     INOUT SipTransportOutboundAddress *outboundAddr);

/***************************************************************************
 * SipTransportGetObjectOutboundAddr
 * ------------------------------------------------------------------------
 * General: Copies the used outbound address ip and port to given parameters.
 *          First try to locate an outbound address in the object outbound
 *          address structure. If there is no outbound address in the object
 *          take the outbound address that was configured for the stack.
 *          If there is no outbound address for the stack reset the given
 *          paramters and return succsess.
 * Returned value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr - The transport manager.
 *          outboundAddr - An outbound addr structrue of a stack object
 * Output:    outboundAddrIp   -  A buffer with the outbound address IP used by the object.
 *          pOutboundProxyPort - The outbound address port
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportGetObjectOutboundAddr (
                     IN    RvSipTransportMgrHandle     hTransportMgr,
                     IN    SipTransportOutboundAddress *outboundAddr,
                       OUT   RvChar                     *outboundAddrIp,
                     OUT   RvUint16                   *pOutboundAddrPort);


/***************************************************************************
 * SipTransportSetObjectOutboundAddr
 * ------------------------------------------------------------------------
 * General: Sets the outbound address for a given object.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr - The transport manager.
 *          pOutboundAddr - An outbound addr structrue of a stack object
 *            strOutboundAddrIp   - The outbound ip to be set to this object.
 *          outboundPort  - The outbound port to be set to this object.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportSetObjectOutboundAddr(
                            IN  RvSipTransportMgrHandle     hTransportMgr,
                            IN  SipTransportOutboundAddress *pOutboundAddr,
                            IN  RvChar                     *strOutboundAddrIp,
                            IN  RvInt32                    outboundPort);


/***************************************************************************
 * SipTransportSetObjectOutboundHost
 * ------------------------------------------------------------------------
 * General: Sets the outbound address for a given object.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr - The transport manager.
 *          pOutboundAddr - An outbound addr structrue of a stack object
 *            strOutboundHost   - The outbound host to be set to this object.
 *          outboundPort  - The outbound port to be set to this object.
 *          hObjPool      - The memory pool the object uses.
 *          hObjPage      - The memory page the object uses.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportSetObjectOutboundHost(
                            IN  RvSipTransportMgrHandle     hTransportMgr,
                            IN  SipTransportOutboundAddress *pOutboundAddr,
                            IN  RvChar                     *strOutboundHost,
                            IN  RvUint16                   outboundPort,
                            IN  HRPOOL                      hObjPool,
                            IN  HPAGE                       hObjPage);

/***************************************************************************
 * SipTransportGetObjectOutboundHost
 * ------------------------------------------------------------------------
 * General: Copies the used outbound host and port to given parameters.
 *          First try to locate an outbound host in the object outbound
 *          address structure. If there is no outbound host in the object
 *          take the outbound host that was configured for the stack.
 *          If there is no outbound host for the stack reset the given
 *          paramters and return succsess.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr - The transport manager.
 *          pOutboundAddr - An outbound addr structrue of a stack object
 *            strOutboundHostName   - The outbound host used by this object.
 *          pOutboundPort  - The outbound port used by this object.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportGetObjectOutboundHost(
                            IN  RvSipTransportMgrHandle     hTransportMgr,
                            IN  SipTransportOutboundAddress *pOutboundAddr,
                              OUT RvChar                     *strOutboundHostName,
                            OUT RvUint16                   *pOutboundPort);


/***************************************************************************
 * SipTransportGetObjectOutboundTransportType
 * ------------------------------------------------------------------------
 * General: returns the transport of the outbound address the object is using.
 *          If the object does not have and outbound address, the transport is
 *          taken from the configured outbound proxy. If and outbound proxy was
 *          not configured undefined is returnes.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr - The transport manager.
 *          pOutboundAddr - An outbound addr structrue of a stack object
 * Output:    eOutboundProxyTransport   - The outbound transport type
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportGetObjectOutboundTransportType(
                            IN  RvSipTransportMgrHandle     hTransportMgr,
                            IN  SipTransportOutboundAddress *pOutboundAddr,
                            OUT RvSipTransport       *eOutboundProxyTransport);

#ifdef RV_SIGCOMP_ON

/***************************************************************************
 * SipTransportGetObjectOutboundCompressionType
 * ------------------------------------------------------------------------
 * General: returns the compression type of the outbound address the object
 *          is using. If the object does not have and outbound address, the
 *          compression type is taken from the configured outbound proxy.
 *          If and outbound proxy was not configured undefined is returned.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:    hTransportMgr - The transport manager.
 *          pOutboundAddr - An outbound addr structrue of a stack object
 * Output:    peOutboundProxyComp - The outbound compression type
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportGetObjectOutboundCompressionType(
                            IN  RvSipTransportMgrHandle      hTransportMgr,
                            IN  SipTransportOutboundAddress *pOutboundAddr,
                            OUT RvSipCompType               *peOutboundProxyComp);
#endif /* RV_SIGCOMP_ON */
/************************************************************************************
 * SipTransportUdpSendMessage
 * ----------------------------------------------------------------------------------
 * General: Send Message on udp socket after copying it to a local buffer
 *
 * Return Value: RvStatus - RvStatus
 *                           RV_ERROR_UNKNOWN
 *                           RV_ERROR_INSUFFICIENT_BUFFER
 *                           RV_ERROR_BADPARAM
 *                           RV_ERROR_TRY_AGAIN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportHandle    - Handle to transport instance
 *          hPool               - handle to the message rpool
 *          element             - rpool element that is going to be sent
 *          hLocalAddr          - The local address to send the message from.
 *          pAddress            - The address to send the message to.
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportUdpSendMessage(
                          IN RvSipTransportMgrHandle     hTransportHandle,
                          IN HRPOOL                      hPool,
                          IN HPAGE                       element,
                          IN RvSipTransportLocalAddrHandle hLocalAddr,
                          IN RvAddress*                  pAddress);

/************************************************************************************
 * SipTransportString2Address
 * ----------------------------------------------------------------------------------
 * General: make SipTransportAddress from string
 *
 * Return Value: none
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   StringIp - the source string
 *          bIsIpV4     - Defines if the address is ip4 or ip6
 * Output:  address - the address in RV_LI_AddressType format
 ***********************************************************************************/
RvStatus SipTransportString2Address(RvChar* StringIp, SipTransportAddress* address, RvBool bIsIpV4);

#if (RV_TLS_TYPE != RV_TLS_NONE)
/************************************************************************************
 * SipTransportConnectionSetHostStringName
 * ----------------------------------------------------------------------------------
 * General: sets the connection's host name string, so it can compare it to the certificate.
 *          this procedure is only done until the connection has been approved by the
 *          application. once the application approved the connection, it can no longer
 *          change the host name
 *
 * Return Value: RvStatus - RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hConnection    - the connection's handle
 *          strAddress     - the address that the connection will be connected to
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionSetHostStringName(
               IN  RvSipTransportConnectionHandle         hConnection,
               IN  RvChar*                               strAddress);
#endif /* (RV_TLS_TYPE != RV_TLS_NONE)*/

/************************************************************************************
 * SipTransportConnectionConstruct
 * ----------------------------------------------------------------------------------
 * General: Construct a new connection.
 *
 * Return Value: RvStatus - RvStatus
 *                           RV_ERROR_UNKNOWN
 *                           RV_ERROR_INSUFFICIENT_BUFFER
 *                           RV_ERROR_BADPARAM
 *                           RV_ERROR_TRY_AGAIN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportHandle    - Handle to transport instance
 *          bForceCreation      - indicates whether to look for such a connection in
 *                                the hash or to force the creation of a new connection.
 *          eConnType           - client or server
 *          pDestAddress        - The destination address
 *          hOwner              - The connection owner.
 *          pfnStateChangedEvHandler - function pointer to notify of connection state.
 *          pfnConnStatusEvHandle    - function pointer to notify of connection status.
 *          pbNewConnCreated    - Indicates whether the connection that was returned is
 *                                a new connection that was just created.
 * Output:  hConnection         - the newly created connection handle
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionConstruct(
               IN  RvSipTransportMgrHandle                hTransportMgr,
               IN  RvBool                                 bForceCreation,
               IN  RvSipTransportConnectionType           eConnType,
               IN  RvSipTransport                         eTransport,
               IN  RvSipTransportLocalAddrHandle          hLocalAddr,
               IN  SipTransportAddress                    *pDestAddress,
               IN  RvSipTransportConnectionOwnerHandle    hOwner,
               IN  RvSipTransportConnectionStateChangedEv pfnStateChangedEvHandler,
               IN  RvSipTransportConnectionStatusEv       pfnConnStatusEvHandle,
               OUT RvBool                                 *pbNewConnCreated,
               OUT RvSipTransportConnectionHandle         *hConnection);


/************************************************************************************
 * SipTransportConnectionIsUsable
 * ----------------------------------------------------------------------------------
 * General:
 *
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn - Handle to a connection to check
 *          hTransport - the transport to check
 *          hLocalAddr - the local address to check
 *          pDestAddress    - The destination address to check
 *          bCheckStateOnly - RV_TRUE - check only the connection state.
 ***********************************************************************************/
RvBool RVCALLCONV SipTransportConnectionIsUsable(
               IN  RvSipTransportConnectionHandle         hConn,
               IN  RvSipTransport                         eTransport,
               IN  RvSipTransportLocalAddrHandle            hLocalAddr,
               IN  SipTransportAddress                        *pDestAddress,
               IN  RvBool                                bCheckStateOnly);

/************************************************************************************
 * SipTransportConnectionSend
 * ----------------------------------------------------------------------------------
 * General: Send Message on a tcp connection after copying it to a local buffer
 *
 * Return Value: RvStatus - RvStatus
 *                           RV_ERROR_UNKNOWN
 *                           RV_ERROR_INSUFFICIENT_BUFFER
 *                           RV_ERROR_BADPARAM
 *                           RV_ERROR_TRY_AGAIN
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportHandle    - Handle to transport instance
 *          hConnection         -
 *          hPool               - handle to the message rpool
 *          element             - rpool element that is going to be sent
 *          hLocalAddress       - The local address to send the message from.
 *          bFreeMsg            - The connection is responsible for deleteing the message page
 *          msgType             - The message type - will be notified in the conncetion callback
 *          hOwner              - The owner of the conncetion.
 *          sctpParams          - Parameters used while sending the message over SCTP
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionSend(
                    IN RvSipTransportMgrHandle             hTransportHandle,
                    IN RvSipTransportConnectionHandle      hConnHandle,
                    IN HRPOOL                              hPool,
                    IN HPAGE                               element,
                    IN RvBool                              bFreeMsg,
                    IN SipTransportMsgType                 msgType,
                    IN RvSipTransportConnectionOwnerHandle hOwner
#if (RV_NET_TYPE & RV_NET_SCTP)
                            ,
                    IN RvSipTransportSctpMsgSendingParams  *pSctpParams
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/
                            );



RvBool RVCALLCONV SipTransportTcpEnabled(IN  RvSipTransportMgrHandle    hTransport);


/***************************************************************************
* SipTransportGetMaxDnsElements
* ------------------------------------------------------------------------
* General: return status of tcp enabled or not
* Return Value: RvUint32 - maximum number of DNS list entries
* ------------------------------------------------------------------------
* Arguments:
* Input:     hTransport - handle to the transport
*
***************************************************************************/
RvUint32 RVCALLCONV SipTransportGetMaxDnsElements(IN  RvSipTransportMgrHandle    hTransport);

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/*****************************************************************************
* SipTransportDnsListPrepare
* ---------------------------------------------------------------------------
* General: 1. Removes TLS if client does not support TLS
*          2. Removes non-TLS if transport supposed to be TSL (sips URI)
*          3. Does nothing if client supports TLS and uri was "sip"
*
* Return Value: RvStatus - RvStatus
*                           RV_ERROR_UNKNOWN
*                           RV_ERROR_BADPARAM
* ---------------------------------------------------------------------------
* Arguments:
* Input:   pTransportMgr        - pointer to transport manager
*          strHostName          - the host name we handle
*          bIsSecure            - was the URI of the message secure (eg. sips)
*          hSrvNamesList        - the list that holds the srv records
*****************************************************************************/
void RVCALLCONV SipTransportDnsListPrepare(
      IN  RvSipTransportMgrHandle   hTransportMgr,
      IN  RvChar*             strHostName,
      IN  RvSipTransportDNSListHandle hDnsList);

#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/


/************************************************************************************
 * SipTransportISIpV4
 * ----------------------------------------------------------------------------------
 * General: check if the string is an ip4 address
 *
 * Return Value: RV_TRUE - ip4 address, RV_FALSE - not ip4 address
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   StringIp - the source string
 * Output:  (-)
 ***********************************************************************************/
RvBool RVCALLCONV SipTransportISIpV4(RvChar* StringIp);

/************************************************************************************
 * SipTransportISIpV6
 * ----------------------------------------------------------------------------------
 * General: check if the string is an ip6 address
 *
 * Return Value: RV_TRUE - ip4 address, RV_FALSE - not ip4 address
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   StringIp - the source string
 * Output:  (-)
 ***********************************************************************************/
RvBool RVCALLCONV SipTransportISIpV6(RvChar* StringIp);

/***************************************************************************
 * SipTransportMgrIsMultiThread
 * ------------------------------------------------------------------------
 * General: Return true if SIP stack is configured to be multi-thread.
 * Return Value: RV_TRUE  - multi thread stack.
 *               RV_FALSE - single thread stack
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - The handle to the transport manager.
 ***************************************************************************/
RvBool RVCALLCONV SipTransportMgrIsMultiThread(IN  RvSipTransportMgrHandle    hTransportMgr);

/************************************************************************************
 * SipTransportConvertCoreAddrTypeToSipAddrType
 * ----------------------------------------------------------------------------------
 * General:
 * Return Value:
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:
 * Output:  (-)
 ***********************************************************************************/
RvSipTransportAddressType SipTransportConvertCoreAddrTypeToSipAddrType(
                        IN   RvInt       eCoreAddressType);

/***************************************************************************
 * SipTransportGetResourcesStatus
 * ------------------------------------------------------------------------
 * General: Returns a structure with the status of all resources used by
 *          the register-client module. It includes the register-client list.
 * Return Value: RV_ERROR_INVALID_HANDLE - The handle to the manager is invalid.
 *               RV_ERROR_NULLPTR    - The pointer to the resource structure is
 *                                  NULL.
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMgr       - Handle to the call-leg manager.
 * Output:     pResources - Pointer to a call-leg resource structure.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportGetResourcesStatus (
                                 IN  RvSipTransportMgrHandle   hTransport,
                                 OUT RvSipTransportResources  *pResources);

/***************************************************************************
 * SipTransportDNSListConstruct
 * ------------------------------------------------------------------------
 * General: The SipTransportDNSListConstruct function allocates and fills
 *          the TransportDNSList structure and returns handler to the
 *            TransportDNSList and appropriate error code or RV_OK.
 *            Receives as input memory pool, page and list pool where lists
 *            element and the TransportDNSList object itself will be allocated.
 * Return Value: RV_ERROR_INVALID_HANDLE - One of the input handles is invalid
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     hMemPool    - Handle of the memory pool.
 *            hMemPage    - Handle of the memory page
 *            hListsPool    - Handle of the list pool
 *            maxElementsInSingleDnsList - maximum num. of elements
 *                        in single list
 * Output:     phDnsList    - Pointer to a DNS list object handler.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportDNSListConstruct(IN  HRPOOL                           hMemPool,
                                                  IN  HPAGE                            hMemPage,
                                                  IN  RvUint32                        maxElementsInSingleDnsList,
                                                  OUT RvSipTransportDNSListHandle    *phDnsList);

/***************************************************************************
 * SipTransportDNSListDestruct
 * ------------------------------------------------------------------------
 * General: The RvSipTransportDNSListDestruct function frees all memory allocated
 *          by the TransportDNSList object, including the TransportDNSList itself.
 * Return Value: RV_ERROR_INVALID_HANDLE - One of the input handles is invalid
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     phDnsList    - Pointer to a DNS list object handler.
 * Output:     N/A
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportDNSListDestruct(IN RvSipTransportDNSListHandle    hDnsList);

/***************************************************************************
 * SipTransportDNSListClone
 * ------------------------------------------------------------------------
 * General: The SipTransportDNSListClone function copies entire original
 *          TransportDNSList object to it's clone object.
 *          Note that clone object should be constructed before by call to the
 *          SipTransportDNSListConstruct function.
 * Return Value: RV_ERROR_INVALID_HANDLE - One of the input handles is invalid
 *               RV_OK       - Resource status returned successfully.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:      hMgr               - Transport manager  
 *             hDnsListOriginal   - Original DNS list object handler.
 * Output:     hDnsListClone      - Clone DNS list object handler
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportDNSListClone(IN RvSipTransportMgrHandle        hMgr, 
                                             IN  RvSipTransportDNSListHandle   hDnsListOriginal,
                                              OUT RvSipTransportDNSListHandle  hDnsListClone);


/*****************************************************************************
 * SipTransportIdentifyDestAddress
 * ---------------------------------------------------------------------------
 * General: Identify the destination address according to the message type:
 *           Requests:  from the request-URI
 *           Responses: from the top Via header
 * Return Value: RV_OK, RV_ERROR_NULLPTR or other error
 * ---------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportHandle        - Transport handle for LOG print
 *            msgType                    - The message type (Request or Response)
 *          msgHostName                - The destination address, as specified in the message
 *            msgPort                    - The port, as specified in the message
 *            inMsgTransportType        - Original transport type as specified by message
 *          bIsSecure               - Is The URI in the message secure
 *          hMemPool                - Memory pool
 *            hMemPage                - Memory page from the memory pool where result should be saved
 *            hListsPool                - List pool where list of IP addresses should be created
 *            phDnsList                - DNS list object handle, created by previous procedure
 * Output:  phDnsList                - DNS list object handle that may be initiated by the function
 *            pAddress                - the destination address that was taken from
 *                                        the message
 *            pTransportTypeToUse        - Real transport type to be used
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportIdentifyDestAddress(
             IN  RvSipTransportMgrHandle        hTransportHandle,
             IN  RvSipMsgType                    msgType,
             IN  RvChar                        *msgHostName,
             IN  RvUint16                        msgPort,
             IN  RvSipTransport                    inMsgTransportType,
             IN  RvBool                        bIsSecure,
             IN  HRPOOL                            hMemPool,
             IN  HPAGE                            hMemPage,
             INOUT RvSipTransportDNSListHandle    *phDnsList,
             OUT SipTransportAddress            *pAddress,
             OUT RvSipTransport                    *pTransportTypeToUse);

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
*          ppPlainMsgBuf   - Pointer to the plain SIP message buffer (uncompressed)
*                            This pointer can be NULL if the original buffer is
*                            plain and doesn't require any additional processing.
***********************************************************************************/
RvStatus RVCALLCONV SipTransportMsgBuilderUdpMakeMsg(
                                  IN  RvSipTransportMgrHandle     hTransportMgr,
                                  IN  RvChar                     *pBuf,
                                  IN  RvUint32                   bufSize,
                                  IN  RvBool                      bConstructMsg,
                                  IN  RvSipMsgHandle              hConstructedMsg,
								  IN  RvSipTransport              eTransportType,
                                  OUT RvSipMsgHandle             *phMsg,
                                  OUT SipTransportSigCompMsgInfo *pSigCompMsgInfo,
                                  OUT RvChar                    **ppPlainMsgBuf);

/***************************************************************************
 * SipTransportGetLocalAddressByType
 * ------------------------------------------------------------------------
 * General: Get the local address from a local address struct
 *        . This is the address the user set using the
 *          RvSipXXXXSetLocalAddress function. If no adress was set the
 *          function will return the default UDP address.
 *          The user can use the transport type and the address type to indicate
 *          which kind of local address he wishes to get.
 * Return Value: RvStatus.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pLocalAddr - pointer to local address struct
 *          eTransportType - The transport type (UDP, TCP, SCTP, TLS).
 *          eAddressType - The address type (ip4 or ip6).
 *          hTransportMgr       - transport Mgr
 * Output:    localAddress - The local address this call-leg is using(must be longer than 48).
 *          localPort - The local port this call-leg is using.
 *          if_name - the local interface name
 *          vdevblock  - the local hardware port
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportGetLocalAddressByType(
                            IN  RvSipTransportMgrHandle        hTransportMgr,
                            IN  SipTransportObjLocalAddresses* pLocalAddr,
                            IN  RvSipTransport                 eTransportType,
                            IN  RvSipTransportAddressType      eAddressType,
                            OUT  RvChar                       *strLocalAddress,
                            OUT  RvUint16                     *pLocalPort
#if defined(UPDATED_BY_SPIRENT)
                            , OUT  RvChar                     *if_name
			    , OUT  RvUint16                   *vdevblock
#endif
			    );

/***************************************************************************
 * SipTransportGetNumOfIPv4LocalAddresses
 * ------------------------------------------------------------------------
 * General: Returns the number of local addresses that the stack listens to.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          pTransportMgr    - A pointer to the transport manager
 *          pNumberOfAddresses - The number of local addresses for which the
 *                              stack listens.
 *
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportGetNumOfIPv4LocalAddresses(
                                     IN   TransportMgr*         pTransportMgr,
                                     OUT  RvUint32*            pNumberOfAddresses);

/***************************************************************************
 * SipTransportGetIPv4LocalAddressByIndex
 * ------------------------------------------------------------------------
 * General: Retrieves the 'index'th local address. Used when the stack was
 *          initialized with IPv4 local address of 0, and therefore listens
 *          on several distinct local addresses.
 *          To know how many local addresses are available by this function
 *          call RvSipTransportGetNumOfIPv4LocalAddresses. If for example this
 *          function returns 5 then you can call
 *          SipTransportGetIPv4LocalAddressByIndex with indexes going from
 *          0 to 4.
 *          Note: The IPv4 address requires 4-BYTEs of memory. This is the
 *          same as an unsigned int (RvUint32). This function requires
 *          pLocalAddr to be a pointer to a 4-BYTE allocated memory.
 *          It can also be a pointer to RvUint32 with an appropriate casting.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:
 *          pTransportMgr    - A pointer to the transport manager
 *          index - The index for the local address to retrieve
 *          pLocalAddr  - A pointer to a 4-BYTE memory space to be filled
 *                        with the selected local address.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportGetIPv4LocalAddressByIndex(
                                     IN   TransportMgr *pTransportMgr,
                                     IN   RvUint                   index,
                                     OUT  RvUint8                  *pLocalAddr);

/************************************************************************************
 * SipTransportMgrAllocateRcvBuffer
 * ----------------------------------------------------------------------------------
 * General: Allocates receive buffer. This function gives other layers, the option
 *          to use a consecutive buffer.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - transport info instance
 * Output:  buff            - allocated buffer
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportMgrAllocateRcvBuffer(
                               IN  RvSipTransportMgrHandle   hTransportMgr,
                               OUT RvChar                     **buff,
                               OUT RvUint32                 *bufLen);

/************************************************************************************
 * SipTransportMgrFreeRcvBuffer
 * ----------------------------------------------------------------------------------
 * General: Frees allocated receive buffer
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - transport info instance
 *            buff            - allocated buffer
 * Output:  none
 ***********************************************************************************/
RvStatus RVCALLCONV SipTransportMgrFreeRcvBuffer(
                              IN  RvSipTransportMgrHandle   hTransportMgr,
                              IN  RvChar                    *buff);

#if (RV_LOGMASK != RV_LOGLEVEL_NONE)

/***************************************************************************
 * SipTransportGetCompTypeName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given compression type
 * Return Value: The string with the address compression type name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   eCompType  - The compression type as enum
 ***************************************************************************/
const RvChar*  RVCALLCONV SipTransportGetCompTypeName(
                                  IN RvSipCompType    eCompType);

/***************************************************************************
 * SipTransportGetMsgTypeName
 * ------------------------------------------------------------------------
 * General: Returns the name of a given msgType
 * Return Value: The string with the msg type name.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   eMsgType -  message type
 ***************************************************************************/
const RvChar*  RVCALLCONV SipTransportGetMsgTypeName(
                              IN SipTransportMsgType    eMsgType);


/******************************************************************************
 * SipTransportMsgBuilderPrintMsg
 * ----------------------------------------------------------------------------
 * General: log print function. Used for printing the sent buffer to the log.
 *
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr    - Handle to the transport manager.
 *          pBuf             - the buffer that was received from the network.
 *          msgDirection     - The direction of the printed message.
 * Output:  none
 *****************************************************************************/
void RVCALLCONV SipTransportMsgBuilderPrintMsg(
                    IN RvSipTransportMgrHandle           hTransportMgr,
                    IN RvChar                           *pBuf,
                    IN SipTransportMgrBuilderPrintMsgDir msgDirection);
#endif /*#if (RV_LOGMASK != RV_LOGLEVEL_NONE)*/

/******************************************************************************
 * SipTransportConvertApiTransportAddrToCoreAddress
 * ----------------------------------------------------------------------------
 * General: converts API address (RvSipTransportAddr) to Transport
 *          address (SipTransportAddress).
 *          the Transport parameter is ignored
 *          *** it is the responsibility of the caller to destruct the core
 *              address inside the transport address
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - Handle to the Transport manager
 *          pAPIAddr        - source address
 * Output:  pCoreAddr       - target address
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportConvertApiTransportAddrToCoreAddress(
                                IN  RvSipTransportMgrHandle hTransportMgr,
                                IN  RvSipTransportAddr*     pAPIAddr,
                                OUT RvAddress*              pCoreAddr);

/******************************************************************************
 * SipTransportConvertCoreAddressToApiTransportAddr
 * ----------------------------------------------------------------------------
 * General: converts Transport address (SipTransportAddress)
 *          to API address (RvSipTransportAddr) .
 *          the Transport parameter is ignored
 * Return Value: -
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pCoreAddr      - source address
 *          pAPIAddr       - target address
 * Output:  none
 *****************************************************************************/
void RVCALLCONV SipTransportConvertCoreAddressToApiTransportAddr(
                              IN  RvAddress*                pCoreAddr,
                              OUT RvSipTransportAddr*       pAPIAddr);


/******************************************************************************
 * SipTransportConvertSipTransportAddressToApiTransportAddr
 * ----------------------------------------------------------------------------
 * General: converts Transport address (SipTransportAddress)
 *          to API address (RvSipTransportAddr).
 * Return Value: -
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   pCoreAddr      - source address
 *          pAPIAddr       - target address
 * Output:  none
 *****************************************************************************/
void RVCALLCONV SipTransportConvertSipTransportAddressToApiTransportAddr(
                              IN  SipTransportAddress*      pInternalAddr,
                              OUT RvSipTransportAddr*       pAPIAddr);

/******************************************************************************
 * SipTransportConvertStrIp2TransportAddress
 * ----------------------------------------------------------------------------
 * General: parse string, representing IP address,
 *          and fills the RvSipTransportAddr structure with the IP details
 *
 * Return Value: RV_OK on success,
 *               error code, defined in rverror.h or RV_SIP_DEF.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr  - Handle to the Transport Manager object.
 *          strIP          - String, conatining IP.
 * Output:  pTransportAddr - Structure, to be filled with the IP details.
 *****************************************************************************/
RvStatus SipTransportConvertStrIp2TransportAddress(
                                   IN  RvSipTransportMgrHandle hTransportMgr,
                                   IN  RvChar*                 strIP,
                                   OUT RvSipTransportAddr*     pTransportAddr);

/******************************************************************************
 * SipTransportDnsSetEnumResult
 * purpose : sets the result on an ENUM NAPTR query
 * input   : hTransportMgr  - transport mgr
 *           pDnsData       - single record from the DNS query result
 * output  : hDnsList       - dns list to set record to
 * return  : RvStatus       - RV_OK or error
 *****************************************************************************/
RvStatus SipTransportDnsSetEnumResult(
    IN RvSipTransportMgrHandle          hTransportMgr,
    IN RvDnsData*                       pDnsData,
    INOUT RvSipTransportDNSListHandle   hDnsList);

/******************************************************************************
 * SipTransportDnsGetEnumResult
 * purpose : get the result on an ENUM NAPTR query
 * input   : hTransportMgr  - transport mgr
 *           hDnsList       - DNS list to set record to
 * output  : pEnumRes       - pointer to ENUM string
 * return  : RvStatus       - RV_OK or error
 *****************************************************************************/
RvStatus SipTransportDnsGetEnumResult(
    IN  RvSipTransportMgrHandle       hTransportMgr,
    IN  RvSipTransportDNSListHandle   hDnsList,
    OUT RvChar**                      pEnumRes);

#ifdef RV_DNS_ENHANCED_FEATURES_SUPPORT
/******************************************************************************
 * SipTransportDnsAddNAPTRRecord
 * purpose : Adds NAPTR record into the DNS list. Note that the new element
 *           is added in correct sorting order.
 * input   : hTransportMgr  - transport mgr
 *           pDnsData       - single record from the DNS query result
 *           bIsSecure      - Allow ONLY Secure resilts (i.e. TLS)
 * output  : hDnsList       - dns list to add record to
 * return  : RvStatus       - RV_OK or error
 *****************************************************************************/
RvStatus SipTransportDnsAddNAPTRRecord(
    IN RvSipTransportMgrHandle          hTransportMgr,
    IN RvDnsData*                       pDnsData,
    IN RvBool                           bIsSecure,
    INOUT RvSipTransportDNSListHandle   hDnsList);

/******************************************************************************
 * SipTransportDnsAddSRVRecord
 * purpose : Adds SRV record into the DNS list. Note that the new element
 *           is added in correct sorting order.
 * input   : hTransportMgr  - transport mgr
 *           pDnsData       - single record from the DNS query result
 *           eTransport     - transport of the added record
 * output  : hDnsList       - dns list to add record to
 * return  : RvStatus       - RV_OK or error
 *****************************************************************************/
RvStatus SipTransportDnsAddSRVRecord(
    IN RvSipTransportMgrHandle   hTransportMgr,
    IN RvDnsData*            pDnsData,
    IN RvSipTransport        eTransport,
    INOUT RvSipTransportDNSListHandle hDnsList);


/***************************************************************************
 * SipTransportDNSListGetHostElement
 * ------------------------------------------------------------------------
 * General: retrieves host element from the host elements list of the DNS
 * list object according to specified by input location.
 * Return Value: RV_OK or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr   - pointer to the transport manager
 *          pDnsList        - pointer of the DNS list object
 *          location        - starting element location
 *          pRelative       - relative host name element for 'next' or
 *                            'previous' locations
 * Output:  pHostElement    - found element
 *          pRelative       - new relative host name element for consequent
 *          call to the RvSipTransportDNSListGetHostElement function.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportDNSListGetHostElement (
    IN RvSipTransportMgrHandle   hTransportMgr,
   IN  RvSipTransportDNSListHandle   hDnsList,
   IN  RvSipListLocation                location,
   INOUT  void                          **pRelative,
   OUT RvSipTransportDNSHostNameElement *pHostElement);

#endif /*RV_DNS_ENHANCED_FEATURES_SUPPORT*/

/******************************************************************************
 * SipTransportDnsAddIPRecord
 * purpose : Adds IP record into the DNS list. Note that the new element
 *           is added in correct sorting order.
 * input   : hTransportMgr  - transport mgr
 *           pDnsData       - single record from the DNS query result
 *           eTransport     - transport of record
 *           port           - port of record
 *           
 * output  : hDnsList       - dns list to add record to
 * return  : RvStatus        - RV_OK or error
 *****************************************************************************/
RvStatus SipTransportDnsAddIPRecord(
    IN RvSipTransportMgrHandle   hTransportMgr,
    IN RvDnsData*            pDnsData,
    IN RvSipTransport        eTransport,
    IN RvUint16              port,
    INOUT RvSipTransportDNSListHandle hDnsList);

/******************************************************************************
 * SipTransportMgrCounterLocalAddrsGet
 * ----------------------------------------------------------------------------
 * General: Returns counter of the opened addresses of the required type.
 * Return Value: none
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:     hTransportMgr - the Transport Manager.
 *            eTransportType- type of Transport Protocol.
 * Output:    pV4Counter     - pointer to the memory, where the requested value
 *                             number of IPv4 addresses will be stored.
 *            pV6Counter     - pointer to the memory, where the requested value
 *                             number of IPv4 addresses will be stored.
 *****************************************************************************/
void RVCALLCONV SipTransportMgrCounterLocalAddrsGet(
                                    IN  RvSipTransportMgrHandle   hTransportMgr,
                                    IN  RvSipTransport          eTransportType,
                                    OUT RvUint32*               pV4Counter,
                                    OUT RvUint32*               pV6Counter);


 /************************************************************************************
 * SipTransportIPElementToSipTransportAddress
 * ----------------------------------------------------------------------------------
 * General: converts API address (RvSipTransportAddr) to Transport
 *          address (SipTransportAddress).
 *          the Transport parameter is ignored
 *          *** it is the responsibility of the caller to destruct the core
 *              address inside the transport address
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------------
 * Arguments:
 * Input:   pAPIAddr        - source address
 *          pCoreAddr       - target address
 * Output:  none
 ***********************************************************************************/
void RVCALLCONV SipTransportIPElementToSipTransportAddress(
                              INOUT SipTransportAddress*       pDestAddr,
                              IN RvSipTransportDNSIPElement*   pIPElement);

/***************************************************************************
 * SipTransportDNSListGetIPElement
 * ------------------------------------------------------------------------
 * General: retrieves IP address element from the DNS list objects IP
 * addresses list according to specified by input location.
 * Return Value: RV_OK, RV_ERROR_UNKNOWN or RV_ERROR_BADPARAM
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pTransportMgr    - pointer to the transport manager
 *          pDnsList        - Handle of the DNS list object
 *          location        - starting element location
 *          pRelative       - relative IP element for get next/previous
 * Output:  pIPElement      - found element
 *          pRelative       - new relative IP element for consequent
 *          call to the RvSipTransportDNSListGetIPElement function.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportDNSListGetIPElement (
    IN    RvSipTransportMgrHandle        hTransportMgr,
    IN    RvSipTransportDNSListHandle    hDnsList,
    IN    RvSipListLocation              location,
    INOUT void**                         pRelative,
    OUT   RvSipTransportDNSIPElement*    pIPElement);

/******************************************************************************
 * SipTransportConnectionAttachOwner
 * ----------------------------------------------------------------------------
 * General: Attach a new owner to the supplied connection together with a set
 *          of callback functions that will be used to notify this owner about
 *          connection states and statuses. You can use this function only if
 *          the connection is connected or in the process to be connected.
 *          You cannot attach an owner to a connection that started its
 *          disconnection process.
 *          Note: The connection will not disconnect as long as it has owners
 *          attached to it.
 * Return Value: RvStatus
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn - Handle to the connection.
 *          hOwner - The owner handle
 *          pEvHanders  - Event handlers structure for this connection owner.
 *          sizeofEvHandlers - The size of the event handler tructure.
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionAttachOwner(
            IN  RvSipTransportConnectionHandle        hConn,
            IN  RvSipTransportConnectionOwnerHandle   hOwner,
            IN  RvSipTransportConnectionEvHandlers    *pEvHandlers,
            IN  RvInt32                               sizeofEvHandlers);

/***************************************************************************
 * SipTransportConnectionHashFindByAlias
 * ------------------------------------------------------------------------
 * General: find a connection in the hash table according to given information.
 * Return Value: (-)
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:     pTransportMgr -    Handle of the transport manager
 *          eTransport    -    The requested transport (Tcp/Tls/Sctp)
 *          hLocalAddr    -    Handle to the requested local address
 *          pDestAddr     -    pointer to the requested destination address
 * Output:  phConn -           Handle to the connection found in the hash or NULL
 *                             if the connection was not found.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionHashFindByAlias(
                         IN RvSipTransportMgrHandle        hTransportMgr,
                         IN RvSipTransport                 eTransport,
                         IN RvSipTransportLocalAddrHandle  hLocalAddress,
                         IN HRPOOL                         hAliasPool,
                         IN HPAGE                          hAliasPage,
                         IN RvInt32                        aliasOffset,
                         OUT RvSipTransportConnectionHandle *phConn);

/***************************************************************************
 * SipTransportConnectionGetRemoteAddress
 * ------------------------------------------------------------------------
 * General: Copy the remote address struct from conection to the given address object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn           - the connection that its address we seek
 * Output:  pRemoteAddress  - A previously allocated space for the address.
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionGetRemoteAddress(
                        IN  RvSipTransportConnectionHandle    hConn,
                        OUT SipTransportAddress    *pRemoteAddress);

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipTransportConnectionGetTransport
 * ------------------------------------------------------------------------
 * General: Returns the transport type of this connection
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   pConn           - the connection that its address we seek
 ***************************************************************************/
RvSipTransport RVCALLCONV SipTransportConnectionGetTransport(
                                        IN  RvSipTransportConnectionHandle hConn);
#endif /* #ifdef RV_SIP_IMS_ON */

/******************************************************************************
 * SipTransportFixIpv6DestinationAddr
 * ----------------------------------------------------------------------------
 * General: Destination IPv6 address of local-link level should contain
 *          Scope ID, which tells to the Operation System, through which
 *          interface the SIP message for UDP, or SIN packet for TCP should be
 *          sent. If it the destination address has no Scope Id,
 *          this function sets it Scope ID to the value, set for Local Address.
 * Return Value: none.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr - handle of Local Address object - source of Scope ID
 *          pDestAddr  - pointer to the address to be fixed
 * Output:  none.
 *****************************************************************************/
void SipTransportFixIpv6DestinationAddr(
                            IN  RvSipTransportLocalAddrHandle hLocalAddr,
                            OUT RvAddress*                    pDestAddr);

/******************************************************************************
 * SipTransportLocalAddressGetNumIpv4Ipv6Ips
 * ----------------------------------------------------------------------------
 * General: For Local Addresses, socket of that is bound to few IPs
 *          (such as SCTP address) - get number of IPv4 and IPv6 IPs.
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr    - Handle to Local Address object
 * Output:  pNumOfIpv4    - Number of IPv4 IPs, to which the socket is bound
 *          pNumOfIpv6    - Number of IPv6 IPs, to which the socket is bound
 *****************************************************************************/
RvStatus SipTransportLocalAddressGetNumIpv4Ipv6Ips(
                            IN  RvSipTransportLocalAddrHandle hLocalAddress,
                            OUT RvUint32*                     pNumOfIpv4,
                            OUT RvUint32*                     pNumOfIpv6);

#if (RV_NET_TYPE & RV_NET_SCTP)
/***************************************************************************
 * SipTransportConnectionSetSctpMsgSendingParams
 * ------------------------------------------------------------------------
 * General: sets SCTP message sending parameters, such as stream number,
 *          into the Connection object.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hRegClient - Handle to the Reg-Client object.
 *          pParams    - Parameters to be set.
 * Output:  none.
 ***************************************************************************/
void RVCALLCONV SipTransportConnectionSetSctpMsgSendingParams(
                    IN  RvSipTransportConnectionHandle     hConn,
                    IN  RvSipTransportSctpMsgSendingParams *pParams);

/***************************************************************************
 * SipTransportConnectionGetSctpMsgSendingParams
 * ------------------------------------------------------------------------
 * General: gets SCTP message sending parameters, such as stream number,
 *          set for the Connection object.
 * Return Value: none.
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hRegClient - Handle to the Reg-Client object.
 * Output:  pParams    - Parameters to be get.
 ***************************************************************************/
void RVCALLCONV SipTransportConnectionGetSctpMsgSendingParams(
                    IN  RvSipTransportConnectionHandle     hConn,
                    OUT RvSipTransportSctpMsgSendingParams *pParams);
#endif /*#if (RV_NET_TYPE & RV_NET_SCTP)*/

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipTransportConnectionSetSecOwner
 * ----------------------------------------------------------------------------
 * General: set object, implemented in Security Module, as Security Owner of 
 *          the connection. The function is used by the Security module.
 *
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h, otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn       - Details of the address to be opened
 *          pSecOwner   - Pointer to the security owner
 *          bDisconnect - If RV_TRUE, and pSecOwner is NULL, disconnection of
 *                        the connection will be initiated.
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionSetSecOwner(
                               IN RvSipTransportConnectionHandle hConn,
                               IN void*                          pSecOwner,
                               IN RvBool                         bDisconnect);
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipTransportLocalAddressSetSecOwner
 * ----------------------------------------------------------------------------
 * General: set object, implemented in Security Module, as Security Owner of 
 *          the Local Address. The function is used by the Security module.
 *
 * Return Value: RV_OK on success,
 *               error code, defined in RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr    - Handle of the Local Address to be closed.
 *          pSecOwner     - Pointer to the Security Owner.
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressSetSecOwner(
                        IN RvSipTransportLocalAddrHandle   hLocalAddr,
                        IN void*                           pSecOwner);
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipTransportSetSecEventHandlers
 * ----------------------------------------------------------------------------
 * General: Set callback functions, implemented in the Security Module,
 *          into the Transport Manager.
 * Return Value: RV_OK on success,
 *               error code,defined in file RV_SIP_DEF.h or rverror.h otherwise
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle to the transport manager.
 *          evHandlers      - struct with pointers to callback functions.
 *          evHandlersSize  - ev handler struct size
 *          hContext - Will be used as a parameter for the callbacks.
 * Output:  none
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportSetSecEventHandlers(
                                IN RvSipTransportMgrHandle    hTransportMgr,
                                IN SipTransportSecEvHandlers* pEvHandlers);
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipTransportMgrLocalAddressAdd
 * ------------------------------------------------------------------------
 * General: add new local address,on which Stack will receive and send messages
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hTransportMgr   - handle of the Transport Manager.
 *          pAddressDetails - pointer to the memory, where the details of
 *                            the address to be added are stored
 *          bDontOpen       - if RV_TRUE, no socket will be opened,
 *                            for TCP/TLS/SCTP address - no listening
 *                            connection will be constructed.
 * Output : phLocalAddr     - pointer to the memory, where the handle of the added
 *                            address will be stored by the function.
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportMgrLocalAddressAdd(
                        IN  RvSipTransportMgrHandle         hTransportMgr,
                        IN  RvSipTransportAddr*             pAddressDetails,
                        IN  RvBool                          bDontOpen,
                        OUT RvSipTransportLocalAddrHandle*  phLocalAddr);
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipTransportMgrLocalAddressGetDetails
 * ------------------------------------------------------------------------
 * General: Returns the details of the local address, the handle of which
 *          is supplied to the function as a parameter.
 *
 * Return Value: RvStatus.
 *               RV_OK on success, error code on failure.
 *               See possible error codes in RV_SIP_DEF.h
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hLocalAddr      -  handle of the Local Address.
 * Output : pAddressDetails - pointer to the memory, where the details of
 *                            the address to be added are stored.
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportMgrLocalAddressGetDetails(
                        IN  RvSipTransportLocalAddrHandle   hLocalAddr,
                        OUT RvSipTransportAddr              *pAddressDetails);
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipTransportConnectionGetLocalAddress
 * ------------------------------------------------------------------------
 * General: retrieves handle to the local address, used by the Connection
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn          - The handle to the Connection
 * Output:  phLocalAddress - Handle to the Local Address
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionGetLocalAddress(
                        IN  RvSipTransportConnectionHandle hConn,
                        OUT RvSipTransportLocalAddrHandle* phLocalAddress);
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipTransportLocalAddressSetImpi
 * ------------------------------------------------------------------------
 * General: set IMPI into the Local Address. The IMPI is provided by
 *          the Application through Security Agreement object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn - The handle to the Connection
 *          impi  - IMPI context
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportLocalAddressSetImpi(
                        IN RvSipTransportLocalAddrHandle hLocalAddress,
                        IN void*                         impi);
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/***************************************************************************
 * SipTransportConnectionSetImpi
 * ------------------------------------------------------------------------
 * General: set IMPI into the Connection. The IMPI is provided by
 *          the Application through Security Agreement object.
 * Return Value: RvStatus
 * ------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn - The handle to the Connection
 *          impi  - IMPI context
 ***************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionSetImpi(
                        IN RvSipTransportConnectionHandle hConn,
                        IN void*                          impi);
#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipTransportConnectionOpenDontConnect
 * ----------------------------------------------------------------------------
 * General: opens client connection, bound to the specified local port.
 *          Doesn't connect it.
 * Return Value: RV_OK on success,
 *               error code, defined in rverror.h or RV_SIP_DEF.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn     - The handle to the Connection
 *          localPort - Port, to which the connection socket should be bound
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionOpenDontConnect(
                        IN RvSipTransportConnectionHandle hConn,
//SPIRENT_BEGIN
#if defined(UPDATED_BY_SPIRENT)
                        IN RvUint16                       *localPort);
#else
                        IN RvUint16                       localPort);
#endif
//SPIRENT_END

#endif /* #ifdef RV_SIP_IMS_ON */

#ifdef RV_SIP_IMS_ON
/******************************************************************************
 * SipTransportConnectionSetDestAddress
 * ----------------------------------------------------------------------------
 * General: sets destination address into connection.
 * Return Value: RV_OK on success,
 *               error code, defined in rverror.h or RV_SIP_DEF.h otherwise.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:   hConn    - The handle to the Connection
 *          pAddress - Destination common core address
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionSetDestAddress(
                        IN RvSipTransportConnectionHandle hConn,
                        IN RvAddress*                     pAddress);
#endif /* #ifdef RV_SIP_IMS_ON */

/******************************************************************************
 * SipTransportConnectionRemoveOwnerMsgs
 * ----------------------------------------------------------------------------
 * General: remove all messages of a specific owner from connection send list.
 * Return Value: void
 * ----------------------------------------------------------------------------
 * Arguments:
 * Output:     pConn - handle to the connection to be set.
 *            hOwner - handle to the connection owner.
 *****************************************************************************/
RvStatus RVCALLCONV SipTransportConnectionRemoveOwnerMsgs(
                      IN RvSipTransportConnectionHandle      hConn,
                      IN RvSipTransportConnectionOwnerHandle hOwner);

#ifdef __cplusplus
}
#endif

#endif  /*END OF: define _SIP_TRANSPORT_H*/
