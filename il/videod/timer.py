#
# @file
# @brief Timer class
#
# Copyright (c) 2008 by Spirent Communications Inc.
# All Rights Reserved.
#
# This software is confidential and proprietary to Spirent Communications Inc.
# No part of this software may be reproduced, transmitted, disclosed or used
# in violation of the Software License Agreement without the expressed
# written consent of Spirent Communications Inc.
#

import mps.client


# Wrap the MPS client timer API with an easier to use class interface
class Timer(object):
    def __init__(self, interval, handler):
        self.timeout = mps.client.scheduleTimeout(interval, handler)

    def __del__(self):
        self.timeout.cancel()
