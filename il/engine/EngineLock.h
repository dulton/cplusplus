/// @file
/// @brief Wrapper for locking (currently uses ACE)
///
///  Copyright (c) 2010 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _L4L7_ENGINE_LOCK_H_
#define _L4L7_ENGINE_LOCK_H_

#include <ace/Guard_T.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <ace/Thread_Mutex.h>

DECL_L4L7_ENGINE_NS

///////////////////////////////////////////////////////////////////////////////

typedef ACE_Recursive_Thread_Mutex RecursiveThreadMutex;
typedef ACE_Thread_Mutex ThreadMutex;

///////////////////////////////////////////////////////////////////////////////

END_DECL_L4L7_ENGINE_NS

#define ENGINE_GUARD(MUTEX, OBJ, LOCK) ACE_GUARD(MUTEX, OBJ, LOCK)
#define ENGINE_GUARD_RETURN(MUTEX, OBJ, LOCK, RETURN) ACE_GUARD_RETURN(MUTEX, OBJ, LOCK, RETURN)

#endif
