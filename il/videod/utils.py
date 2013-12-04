#
# @file
# @brief Utility functions
#
# Copyright (c) 2008 by Spirent Communications Inc.
# All Rights Reserved.
#
# This software is confidential and proprietary to Spirent Communications Inc.
# No part of this software may be reproduced, transmitted, disclosed or used
# in violation of the Software License Agreement without the expressed
# written consent of Spirent Communications Inc.
#

import socket


# Wrap Hal functions for retrieving port count and backplane timestamp
# If Hal is unavailable, provide surrogates for unit testing
try:
    import Hal
    getPortCount = Hal.HwInfo().getLogPortCount
    getTimeStamp = Hal.getTimeStamp
except ImportError:
    import time
    getPortCount = lambda: 1
    getTimeStamp = time.time


def getAddrFamily(addr_str):
    return socket.getaddrinfo(addr_str, 0)[0][0]


def isUnspecifiedAddr(addr_str):
    if not addr_str:
        return True
    addr_family = socket.getaddrinfo(addr_str, 0)[0][0]
    if addr_family not in (socket.AF_INET, socket.AF_INET6):
        raise NotImplementedError
    addr = socket.inet_pton(addr_family, addr_str)
    if addr_family == socket.AF_INET:
        return addr == '\x00' * 4
    else:
        return addr == '\x00' * 16


def isMulticastAddr(addr_str):
    addr_family = socket.getaddrinfo(addr_str, 0)[0][0]
    if addr_family not in (socket.AF_INET, socket.AF_INET6):
        raise NotImplementedError
    addr = socket.inet_pton(addr_family, addr_str)
    if addr_family == socket.AF_INET:
        return ord(addr[0]) & 0xf0 == 0xe0
    else:
        return ord(addr[0]) == 0xff
