#
# @file
# @brief Relative path setup
#
# Copyright (c) 2008 by Spirent Communications Inc.
# All Rights Reserved.
#
# This software is confidential and proprietary to Spirent Communications Inc.
# No part of this software may be reproduced, transmitted, disclosed or used
# in violation of the Software License Agreement without the expressed
# written consent of Spirent Communications Inc.
#

import sys, os

# Find the top of the development tree (TREE_ROOT)
_dirs = os.path.dirname(os.path.abspath(__file__)).split(os.path.sep)
for i in range(len(_dirs), 1, -1):
    _path = os.path.sep.join(_dirs[:i])
    if os.access(os.path.join(_path, 'SConstruct'), os.F_OK):
        TREE_ROOT = _path
        break
else:
    raise RuntimeError, "Couldn't locate top of tree"

sys.path.append(os.path.join(TREE_ROOT, 'content', 'traffic', 'l4l7', 'il', 'videod'))
