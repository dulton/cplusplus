#!/usr/bin/env python
#
# @file
# @brief Streaming Video Daemon (videod)
#
# Copyright (c) 2008 by Spirent Communications Inc.
# All Rights Reserved.
#
# This software is confidential and proprietary to Spirent Communications Inc.
# No part of this software may be reproduced, transmitted, disclosed or used
# in violation of the Software License Agreement without the expressed
# written consent of Spirent Communications Inc.
#

import sys, getopt, traceback, gc

import mps.client
import mps.logger as logger
import prctl
from constants import *
from app import VideoApplication
from video_1 import Video1Handlers
import utils


class Usage(Exception):
    def __init__(self, msg):
        self.args = msg


class Params(object):
    def __init__ (self):
        self.startup_fifo = None
        self.redirect_to = None
        self.log_level = logger.ERROR
        self.tcp_port = mps.conn.DEFAULT_MPS_PORT

        opt_chars = 'hs:f:l:p:'
        usage_msg = 'usage: ' + sys.argv[0] + ' [-' + opt_chars + """]
        \t-h prints this help
        \t-s startup fifo name
        \t-l log level
        \t-f trace file (use \"stdout\" for console)
        \t-p mps server port number
        """

        try:
            opts, args = getopt.getopt(sys.argv[1:], opt_chars)
        except getopt.error, msg:
            raise Usage(msg)

        for optname, optvalue in opts:
            if optname == '-h':
                print usage_msg
                sys.exit(1)

            if optname == '-s':
                self.startup_fifo = optvalue

            if optname == '-l':
                self.log_level = self.translate_log_level(int(optvalue))

            if optname == '-f':
                self.redirect_to = optvalue

            if optname == '-p':
                self.tcp_port = int(optvalue)

    def translate_log_level(self, level):
        level_map = {
            7: logger.DEBUG,
            6: logger.INFO,
            5: logger.INFO,
            4: logger.WARNING,
            3: logger.ERROR,
            2: logger.CRITICAL,
            1: logger.FATAL }
        try:
            return level_map[level]
        except KeyError:
            return logger.DEBUG

def init_logger(name, params):
    redirect_stdout = (params.redirect_to == 'stdout')
    redirect_file = (params.redirect_to, None)[redirect_stdout]
    logger.init(name, redirect_stdout, redirect_file)
    logger.setLevel(params.log_level)


def signal_startup(params):
    if params.startup_fifo:
        f = open(params.startup_fifo, 'w')
        print >>f, "OK",


# Parse command line params
params = Params()

# Initialize logger
init_logger(MODULE, params)

# Set process name
prctl.setName(MODULE)

# Join the MPS network
logger.log(logger.INFO, "Starting MPS")
if not mps.client.start(MODULE, params.tcp_port):
    raise SystemExit

# Create an application instance, Video_1 message set server and register with the MPS client
app = VideoApplication(utils.getPortCount())
video_1 = Video1Handlers(app)
if not mps.client.registerMessageSetServer(video_1):
    raise SystemExit

# Force garbage collection post-initialization
gc.collect()

# Signal successful daemon startup
signal_startup(params)
logger.log(logger.INFO, "Signaled startup, entering main loop")

# This is the main loop of the daemon -- it runs forever or until we interrupt the daemon (when running interactively)
while True:
    try:
        mps.client.run()
    except KeyboardInterrupt:
        break
    except:
        traceback.print_exc()

# Shutdown MPS
logger.log(logger.INFO, "Stopping MPS")
mps.client.stop()
