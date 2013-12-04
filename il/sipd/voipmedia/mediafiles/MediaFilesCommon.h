/// @file
/// @brief Media Files Common defs
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _MEDIAFILES_COMMON_H_
#define _MEDIAFILES_COMMON_H_

#include <stdint.h>

#include <boost/shared_ptr.hpp>

#include "VoIPCommon.h"

#ifndef MEDIAFILES_NS
# define MEDIAFILES_NS MEDIA_NS::mediafiles
#endif

#ifndef USING_MEDIAFILES_NS
# define USING_MEDIAFILES_NS using namespace MEDIA_NS::mediafiles
#endif

#ifndef DECL_MEDIAFILES_NS
# define DECL_MEDIAFILES_NS DECL_MEDIA_NS namespace mediafiles {
# define END_DECL_MEDIAFILES_NS END_DECL_MEDIA_NS }
#endif

#ifndef DECL_CLASS_FORWARD_MEDIAFILES_NS
# define DECL_CLASS_FORWARD_MEDIAFILES_NS(CLASSNAME) DECL_MEDIAFILES_NS class CLASSNAME; END_DECL_MEDIAFILES_NS
#endif

DECL_MEDIAFILES_NS

///////////////////////////////////////////////////////////////////////////////

typedef uint32_t EncodedMediaStreamIndex;

//////////////////////////////////////////////////////////////////////////////

class MediaPacket;

typedef boost::shared_ptr<MediaPacket> MediaPacketHandler;

///////////////////////////////////////////////////////////////////////////////

typedef int CodecType;

///////////////////////////////////////////////////////////////////////////

typedef int CodecGroupType;

///////////////////////////////////////////////////////////////////////////

typedef int PayloadType;

/////////////////////////////////////////////////////////////////////////////

class MediaStreamInfo;

///////////////////////////////////////////////////////////////////////////////

class EncodedMediaFile;

typedef boost::shared_ptr<EncodedMediaFile> EncodedMediaStreamHandler;

//////////////////////////////////////////////////////////////

END_DECL_MEDIAFILES_NS

#endif //_MEDIAFILES_COMMON_H_

