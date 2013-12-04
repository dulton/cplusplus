#
# @file
# @brief Streamer class
#
# Copyright (c) 2011 by Spirent Communications Inc.
# All Rights Reserved.
#
# This software is confidential and proprietary to Spirent Communications Inc.
# No part of this software may be reproduced, transmitted, disclosed or used
# in violation of the Software License Agreement without the expressed
# written consent of Spirent Communications Inc.
#

import asyncore, popen2, os

import mps.logger as logger
from constants import *
import utils


class StreamerParams(object):
    __slots__ = [ 'proto', 'stream_files', 'loop', 'ifname', 'saddr', 'daddr', 'dport', 'tclass', 'tos', 'ttl' ]

    PROTO_UDP4 = 1
    PROTO_RTP4 = 2
    PROTO_UDP6 = 3
    PROTO_RTP6 = 4
    
    def __init__(self):
        self.proto = None
        self.stream_files = []
        self.loop = False
        self.ifname = None
        self.saddr = None
        self.daddr = None
        self.dport = None
        self.tclass = -1
        self.tos = -1
        self.ttl = DEFAULT_TTL
        
    
class Streamer(asyncore.file_dispatcher):
    STATIC_PARAMS = ['--plugin-path /usr/spirent/lib/vlc', '--intf dummy', '--sout-keep', '--sout-ts-pid-video=68', '--sout-ts-pid-audio=69']
    
    def __init__(self, params):
        if not params.stream_files:
            raise ValueError, "Params must specify at least one stream file"
        if params.saddr is None or params.daddr is None or params.dport is None:
            raise ValueError, "Params must fully specify source/destination address and destiantion port"
        params = self.STATIC_PARAMS + self.build_generic_params(params) + self.PROTO_PARAM_BUILDERS[params.proto](self, params)
        # On USS we must run VLC as a non-root user or it will fork 
        # and fork doesn't work right under USS (grep for rootwrapper)
        # We have to use an absolute path because the PATH gets screwed up
        # by the one-off su on Iceboy/ProfY using ENV_PATH in /etc/login.defs
        cmd = '/bin/su -c "/usr/spirent/bin/vlc ' + ' '.join(params) + '" bin'
        self.p = popen2.Popen4(cmd)
        logger.log(logger.DEBUG, "Started subprocess %u with command line [%s]" % (self.p.pid, cmd))
        asyncore.file_dispatcher.__init__(self, self.p.fromchild.fileno())
        self.exit_callable = None

    def set_exit_callable(self, callable):
        self.exit_callable = callable

    def close(self):
        self.del_channel()
        self.socket = None
        if self.p and self.p.poll() == -1:
            os.kill(self.p.pid, 9)
            self.p.wait()

    def build_generic_params(self, params):
        stream_file_paths = [os.path.join(VIDEO_CACHE_PATH, stream_file) for stream_file in params.stream_files]
        bad_stream_files = [params.stream_files[x[0]] for x in enumerate(stream_file_paths) if not os.access(x[1], os.R_OK)]
        if bad_stream_files:
            raise IOError, "One or more stream files are unreadable: %s" % bad_stream_files
        return stream_file_paths + ([], ['--loop'])[params.loop]

    def build_udp4_params(self, params):
        return [ '--ifname %s' % params.ifname, '--miface-addr %s' % params.saddr, "--sout '#standard{mux=ts,dst=%s:%u,access=udp}'" % (params.daddr, params.dport), '--ttl %u' % params.ttl, '--tos %u' % params.tos ]

    def build_rtp4_params(self, params):
        return [ '--ifname %s' % params.ifname, '--miface-addr %s' % params.saddr, "--sout '#rtp{mux=ts,dst=%s,port=%u}'" % (params.daddr, params.dport), '--ttl %u' % params.ttl, '--tos %u' % params.tos ]

    def build_udp6_params(self, params):
        param_string = [ '--ifname %s' % params.ifname, '--miface %s' % params.ifname, "--sout '#standard{mux=ts,dst=[%s]:%u,access=udp}'" % (params.daddr, params.dport), '--ttl %u' % params.ttl, '--tclass %u' % params.tclass ]
        # RFC 3542 says mcast IPv6 should use 1280
        if (utils.isMulticastAddr(params.daddr)):
            param_string.insert(0, '--mtu 1280')
        return param_string

    def build_rtp6_params(self, params):
        param_string = [ '--ifname %s' % params.ifname, '--miface %s' % params.ifname, "--sout '#rtp{mux=ts,dst=[%s],port=%u}'" % (params.daddr, params.dport), '--ttl %u' % params.ttl, '--tclass %u' % params.tclass ]
        # RFC 3542 says mcast IPv6 should use 1280
        if (utils.isMulticastAddr(params.daddr)):
            param_string.insert(0, '--mtu 1280')
        return param_string

    PROTO_PARAM_BUILDERS = { StreamerParams.PROTO_UDP4: build_udp4_params,
                             StreamerParams.PROTO_RTP4: build_rtp4_params,
                             StreamerParams.PROTO_UDP6: build_udp6_params,
                             StreamerParams.PROTO_RTP6: build_rtp6_params }

    def readable(self):
        return True

    def writable(self):
        return False

    def handle_read(self):
        status = self.p.poll()
        if status == -1:
            try:
                for line in self.p.fromchild:
                    logger.log(logger.DEBUG, line.strip('\r\n'))
            except IOError, e:
                pass
        else:
            logger.log(logger.DEBUG, "Subprocess exited with status = %d" % status)
            self.exit_callable and self.exit_callable(status)
            self.del_channel()
            self.socket = None


