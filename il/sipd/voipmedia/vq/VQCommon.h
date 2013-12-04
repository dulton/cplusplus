/// @file
/// @brief VQCommon header
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _VQCOMMON_H_
#define _VQCOMMON_H_

#include "VoIPCommon.h"

////////////////////////////////////////////////////////////////////////

#ifndef VQSTREAM_NS
# define VQSTREAM_NS MEDIA_NS::vq
#endif

#ifndef USING_VQSTREAM_NS
# define USING_VQSTREAM_NS using namespace MEDIA_NS::vq
#endif

#ifndef DECL_VQSTREAM_NS
# define DECL_VQSTREAM_NS DECL_MEDIA_NS namespace vq {
# define END_DECL_VQSTREAM_NS END_DECL_MEDIA_NS }
#endif

#ifndef DECL_CLASS_FORWARD_VQSTREAM_NS
# define DECL_CLASS_FORWARD_VQSTREAM_NS(CLASSNAME) DECL_VQSTREAM_NS class CLASSNAME; END_DECL_VQSTREAM_NS
#endif

DECL_VQSTREAM_NS

enum VQMediaType {
  VQ_MEDIA_UNDEFINED=0,
  VQ_VOICE,
  VQ_VIDEO,
  VQ_AUDIOHD,
  VQ_MEDIA_TYPE_TOTAL
};

END_DECL_VQSTREAM_NS

//////////////////////////////////////////////////////////////

#endif //_VQCOMMON_H_

