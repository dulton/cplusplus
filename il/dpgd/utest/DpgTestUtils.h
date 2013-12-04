/// @file
/// @brief Dpg unit test utilities
///
///  Copyright (c) 2009 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _DPG_TEST_UTILS_H_
#define _DPG_TEST_UTILS_H_
#include <vif/IfSetupCommon.h>
#include "DpgCommon.h"

///////////////////////////////////////////////////////////////////////////////

class DpgTestUtils 
{
  public:
    static void InitCommonProfile(l4l7Base_1_port_server::L4L7Profile_t* profile, const std::string& name)
    {
        profile->ProfileName = name;
        profile->Ipv4Tos = 0;
        profile->Ipv6TrafficClass = 0;
        profile->RxWindowSizeLimit = 0;
        profile->EnableDelayedAck = true;
        profile->IsDefaultProfile = false;
    }

    static void InitClientEndpoint(Dpg_1_port_server::ClientEndpointConfig_t* endpoint)
    {
        endpoint->SrcIfHandle = 0;
        endpoint->DstIf.ifDescriptors.resize(1);
        endpoint->DstIf.ifDescriptors[0].ifType = IFSETUP_NS::STC_DM_IFC_IPv4;
        endpoint->DstIf.ifDescriptors[0].indexInList = 0;
        endpoint->DstIf.Ipv4InterfaceList.resize(1);
        Dpg_1_port_server::Ipv4If_t_clear(&endpoint->DstIf.Ipv4InterfaceList[0]);
        endpoint->DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsRange = true;
        endpoint->DstIf.Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsDirectlyConnected = true;
        endpoint->DstIf.Ipv4InterfaceList[0].NetworkInterface.IfCountPerLowerIf = 1;
        endpoint->DstIf.Ipv4InterfaceList[0].NetworkInterface.TotalCount = 1;
        endpoint->DstIf.Ipv4InterfaceList[0].Address.address[0] = 127;
        endpoint->DstIf.Ipv4InterfaceList[0].Address.address[3] = 1;
        endpoint->DstIf.Ipv4InterfaceList[0].AddrStepMask.address[0] = 0xff;
        endpoint->DstIf.Ipv4InterfaceList[0].AddrStepMask.address[1] = 0xff;
        endpoint->DstIf.Ipv4InterfaceList[0].AddrStepMask.address[2] = 0xff;
        endpoint->DstIf.Ipv4InterfaceList[0].AddrStepMask.address[3] = 0xff;
        endpoint->DstIf.Ipv4InterfaceList[0].PrefixLength = 8;
    }

    static void InitServerIf(Dpg_1_port_server::NetworkInterfaceStack_t* dstIf)
    {
        dstIf->ifDescriptors.resize(1);
        dstIf->ifDescriptors[0].ifType = IFSETUP_NS::STC_DM_IFC_IPv4;
        dstIf->ifDescriptors[0].indexInList = 0;
        dstIf->Ipv4InterfaceList.resize(1);
        Dpg_1_port_server::Ipv4If_t_clear(&dstIf->Ipv4InterfaceList[0]);
        dstIf->Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsRange = true;
        dstIf->Ipv4InterfaceList[0].NetworkInterface.EmulatedIf.IsDirectlyConnected = true;
        dstIf->Ipv4InterfaceList[0].NetworkInterface.IfCountPerLowerIf = 1;
        dstIf->Ipv4InterfaceList[0].NetworkInterface.TotalCount = 1;
        dstIf->Ipv4InterfaceList[0].Address.address[0] = 127;
        dstIf->Ipv4InterfaceList[0].Address.address[3] = 1;
        dstIf->Ipv4InterfaceList[0].AddrStepMask.address[0] = 0xff;
        dstIf->Ipv4InterfaceList[0].AddrStepMask.address[1] = 0xff;
        dstIf->Ipv4InterfaceList[0].AddrStepMask.address[2] = 0xff;
        dstIf->Ipv4InterfaceList[0].AddrStepMask.address[3] = 0xff;
        dstIf->Ipv4InterfaceList[0].PrefixLength = 8;
    }
};

///////////////////////////////////////////////////////////////////////////////

#endif
