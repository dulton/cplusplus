/// @file
/// @brief ReferenceHolder declaration
///
///  Copyright (c) 2011 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///
///  Uses RAII to raise a reference count while this object is instantiated

#ifndef _REFERENCE_HOLDER_H_
#define _REFERENCE_HOLDER_H_

#include <engine/EngineCommon.h>

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

template<class T>
class ReferenceHolder
{
  public:
    ReferenceHolder(T& object) : 
        mObject(object)
    {
        mObject.AddReference(); 
    }

    ~ReferenceHolder()
    {
        mObject.RemoveReference();
    }

  private:
    T& mObject;
};

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#endif
