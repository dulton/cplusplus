#
# @file
# @brief RTSP client implementation
#
# Copyright (c) 2011 by Spirent Communications Inc.
# All Rights Reserved.
#
# This software is confidential and proprietary to Spirent Communications Inc.
# No part of this software may be reproduced, transmitted, disclosed or used
# in violation of the Software License Agreement without the expressed
# written consent of Spirent Communications Inc.
#

import asyncore, errno, os, socket

import mps.logger as logger
from constants import *
from rtspproto import *
import utils


# SO_BINDTODEVICE is missing for some reason?
try:
    socket.SO_BINDTODEVICE
except AttributeError:
    socket.SO_BINDTODEVICE = 25


class RTSPClient(object):
    def __init__(self):
        self.connection_event_callable = None
        self.transaction_event_callable = None
        self._init_state()

    def _init_state(self):
        self.handler = None
        self.next_cseq = 1
        self.last_req = None
        self.uri = None
        self.session = None

    def set_connection_event_callable(self, call):
        self.connection_event_callable = call

    def set_transaction_event_callable(self, call):
        self.transaction_event_callable = call

    def connect(self, binder):
        self.handler = _ClientConnHandler(self._handle_connect, self._handle_status)
        self.handler.start(binder)

    def close(self):
        self.handler and self.handler.close()
        self._init_state()

    def setup(self, uri, dest, ports, ttl, loop, keepalive=True):
        transport = RTSPTransport()
        transport.proto = 'UDP'
        if utils.isMulticastAddr(dest):
            transport.params[TRANSPORT_MULTICAST] = True
            transport.params[TRANSPORT_DESTINATION] = dest
            transport.params[TRANSPORT_PORT] = ports
            transport.params[TRANSPORT_TTL] = ttl
        else:
            transport.params[TRANSPORT_UNICAST] = True
            transport.params[TRANSPORT_DESTINATION] = dest
            transport.params[TRANSPORT_CLIENT_PORT] = ports
        req = RTSPRequest()
        req.method = METHOD_SETUP
        req.uri = self.uri = uri
        req.version = VERSION
        req.headers[HEADER_CSEQ], self.next_cseq = self.next_cseq, self.next_cseq + 1
        req.headers[HEADER_TRANSPORT] = transport.encode()
        if loop:
            req.headers[HEADER_X_LOOP_FLAG] = True
        if keepalive:
            req.headers[HEADER_CONNECTION] = 'Keepalive'
        self.handler.send_request(req)
        self.last_req = req
        
    def play(self, keepalive=True):
        req = RTSPRequest()
        req.method = METHOD_PLAY
        req.uri = self.uri
        req.version = VERSION
        req.headers[HEADER_CSEQ], self.next_cseq = self.next_cseq, self.next_cseq + 1
        req.headers[HEADER_SESSION] = self.session
        if keepalive:
            req.headers[HEADER_CONNECTION] = 'Keepalive'
        self.handler.send_request(req)
        self.last_req = req

    def teardown(self, keepalive=False):
        req = RTSPRequest()
        req.method = METHOD_TEARDOWN
        req.uri = self.uri
        req.version = VERSION
        req.headers[HEADER_CSEQ], self.next_cseq = self.next_cseq, self.next_cseq + 1
        req.headers[HEADER_SESSION] = self.session
        if keepalive:
            req.headers[HEADER_CONNECTION] = 'Keepalive'
        self.handler.send_request(req)
        self.last_req = req
        
    def _handle_connect(self, connected):
        self.connection_event_callable and self.connection_event_callable(connected)

    def _handle_status(self, status):
        logger.log(logger.DEBUG, "Received status: %s %s %s", status.status_code, status.reason_phrase, status.headers)
        self.session = self.session or status.headers.get(HEADER_SESSION)
        status.req, self.last_req = self.last_req, None
        self.transaction_event_callable and self.transaction_event_callable(status)


class _ClientConnHandler(asyncore.dispatcher):
    READ_SIZE = 4096
    
    def __init__(self, connect_handler, status_handler):
        asyncore.dispatcher.__init__(self)
        self.connect_handler = connect_handler
        self.status_handler = status_handler
        self.proto = RTSPProtocol(RTSPStatus)

    def start(self, binder):
        if_index, if_name, addr_family, addr, server, port, tos, tclass = binder
        self.create_socket(addr_family, socket.SOCK_STREAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_BINDTODEVICE, if_name + chr(0))
        if addr_family == socket.AF_INET6:
            self.socket.setsockopt(socket.IPPROTO_IPV6, 67, tclass)
            self.bind((addr, 0, 0, 0))
            self.connect((server, port, 0, 0))
        else:
            self.socket.setsockopt(socket.IPPROTO_IP, socket.IP_TOS, tos)
            self.bind((addr, 0))
            self.connect((server, port))

    def send_request(self, req):
        self.send(self.proto.output(req))
        
    def readable(self):
        return True

    def writable(self):
        return not self.connected

    def handle_connect(self):
        self.connect_handler(True)

    def handle_read(self):
        try:
            map(self.status_handler, self.proto.input(self.recv(self.READ_SIZE)))
        except (socket.error, RTSPException):
            self.handle_close()

    def handle_write(self):
        pass

    def handle_close(self):
        self.close()
        self.connect_handler(False)
