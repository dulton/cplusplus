#
# @file
# @brief Video daemon constants
#
# Copyright (c) 2008 by Spirent Communications Inc.
# All Rights Reserved.
#
# This software is confidential and proprietary to Spirent Communications Inc.
# No part of this software may be reproduced, transmitted, disclosed or used
# in violation of the Software License Agreement without the expressed
# written consent of Spirent Communications Inc.
#

import os, socket


# Module name
MODULE = 'videod'

# Message set name
MESSAGE_SET = 'Video_1'

# Daemon library path
VIDEO_LIB_PATH = '/usr/spirent/lib/video'

# Database schema and precompiled C API
DB_SCHEMA_FILE = os.path.join(VIDEO_LIB_PATH, 'statsdb.mco')
DB_SHARED_LIB = 'libvideostatsdb.so'

# Video clip path
VIDEO_CLIP_PATH = os.path.join(VIDEO_LIB_PATH, 'clips')

# Video cache path and size limit
VIDEO_CACHE_PATH = os.path.join('/tmp', '%s-cache' % MODULE)
MAX_VIDEO_CACHE_SIZE = 128 * 1024 * 1024

# Server string
SERVER_STRING = 'SpirentTestCenter/1.0'

# Default RTP/RTCP destination ports
DEFAULT_PORTS = (50050, 50051)

# Default TTL
DEFAULT_TTL = 127

# Socket address family map
ADDR_FAMILY_MAP = { 'IPv4': socket.AF_INET,
                    'IPv6': socket.AF_INET6 }
