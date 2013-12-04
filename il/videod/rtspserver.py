#
# @file
# @brief RTSP server implementation
#
# Copyright (c) 2011 by Spirent Communications Inc.
# All Rights Reserved.
#
# This software is confidential and proprietary to Spirent Communications Inc.
# No part of this software may be reproduced, transmitted, disclosed or used
# in violation of the Software License Agreement without the expressed
# written consent of Spirent Communications Inc.
#

import socket, asyncore, os, random, urlparse

import mps.logger as logger
from constants import *
from rtspproto import *


class RTSPServer(object):
    ALLOWED_METHODS = (METHOD_SETUP, METHOD_PLAY, METHOD_TEARDOWN)
    SESSION_ID_LENGTH = 16
    SESSION_ID_SAMPLE_STRING = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"

    class Session(object):
        def __init__(self):
            self.id = None
            self.stream_files = []
            self.loop = False
            self.saddr = None
            self.daddr = None
            self.dport = None
            self.ttl = None
            self.active = False

    def __init__(self, lib_path):
        self.lib_path = lib_path
        self.connection_event_callable = None
        self.transaction_event_callable = None
        self.session_event_callable = None
        self._init_state()

    def _init_state(self):
        self.if_index = 0
        self.if_name = None
        self.addr_family = socket.AF_INET
        self.addr = None
        self.port = 0
        self.acceptor = None
        self.handlers = {}
        self.sessions = {}

    def set_connection_event_callable(self, call):
        self.connection_event_callable = call

    def set_transaction_event_callable(self, call):
        self.transaction_event_callable = call

    def set_session_event_callable(self, call):
        self.session_event_callable = call
        
    def start(self, binder):
        self.stop()
        self.if_index, self.if_name, self.addr_family, self.addr, self.port, tos, tclass = binder
        self.acceptor = _ServerConnHandler(self._handle_accept, self._handle_request, self._handle_close)
        self.acceptor.start(binder)

    def stop(self):
        map(lambda x: x.close(), self.handlers.keys())
        self.acceptor and self.acceptor.close()
        self._init_state()

    def _handle_accept(self, handler, addr):
        logger.log(logger.DEBUG, "Accepted connection: %s", addr)
        self.handlers[handler] = addr
        self.connection_event_callable and self.connection_event_callable()
        
    def _make_status(self, req, status_code):
        status = RTSPStatus()
        status.version = VERSION
        status.status_code = status_code
        status.headers[HEADER_SERVER] = SERVER_STRING
        if HEADER_CSEQ in req.headers:
            status.headers[HEADER_CSEQ] = req.headers[HEADER_CSEQ]
        if HEADER_SESSION in req.headers:
            status.headers[HEADER_SESSION] = req.headers[HEADER_SESSION]
        return status
    
    def _new_session_id(self):
        while True:
            session_id = ''.join(random.sample(self.SESSION_ID_SAMPLE_STRING, self.SESSION_ID_LENGTH))
            if session_id not in self.sessions:
                return session_id

    def _handle_setup_request(self, handler, req):
        if HEADER_SESSION in req.headers:
            return self._make_status(req, 459)

        if HEADER_TRANSPORT not in req.headers:
            return self._make_status(req, 461)

        try:
            transport = RTSPTransport(req.headers[HEADER_TRANSPORT])
        except BadTransportValue:
            return self._make_status(req, 461)

        if transport.proto != 'UDP':
            status = self._make_status(req, 461)
            status[HEADER_X_ERROR_MSG] = 'This server only supports UDP transports'
            return status

        stream_path = os.path.normpath(self.lib_path + urlparse.urlparse(req.uri)[2])
        if not os.path.isfile(stream_path):
            return self._make_status(req, 404)
        if not os.access(stream_path, os.R_OK):
            return self._make_status(req, 403)

        transport_actual = RTSPTransport()
        dest = None
        ports = None
        ttl = None
        
        if TRANSPORT_UNICAST in transport.params:
            dest = transport.params.get(TRANSPORT_DESTINATION, handler.addr[0])
            ports = transport.params.get(TRANSPORT_CLIENT_PORT, DEFAULT_PORTS)
            ttl = int(transport.params.get(TRANSPORT_TTL, DEFAULT_TTL))
            transport_actual.decode('RTP/AVP/UDP;%s;%s=%s;%s=%u-%u' % (TRANSPORT_UNICAST,
                                                                       TRANSPORT_DESTINATION, dest,
                                                                       TRANSPORT_CLIENT_PORT, ports[0], ports[1]))
        else:
            dest = transport.params.get(TRANSPORT_DESTINATION)
            if dest is None:
                status = self._make_status(req, 461)
                status[HEADER_X_ERROR_MSG] = 'Missing multicast destination address'
                return status
            ports = transport.params.get(TRANSPORT_PORT, DEFAULT_PORTS)
            ttl = int(transport.params.get(TRANSPORT_TTL, DEFAULT_TTL))
            transport_actual.decode('RTP/AVP/UDP;%s;%s=%s;%s=%u-%u;%s=%u' % (TRANSPORT_MULTICAST,
                                                                             TRANSPORT_DESTINATION, dest,
                                                                             TRANSPORT_PORT, ports[0], ports[1],
                                                                             TRANSPORT_TTL, ttl))

        session = self.Session()
        session.id = self._new_session_id()
        session.uri = req.uri
        session.stream_path = stream_path
        try:
            session.loop = eval(req.headers.get(HEADER_X_LOOP_FLAG, 'False'))
        except:
            pass
        session.saddr = self.addr
        session.daddr = dest
        session.dport = int(ports[0])
        session.ttl = ttl
        session.active = False
        self.sessions[session.id] = session
        
        status = self._make_status(req, 200)
        status.headers[HEADER_SESSION] = session.id
        status.headers[HEADER_TRANSPORT] = transport_actual.encode()
        return status

    def _handle_play_request(self, handler, req):
        session = self.sessions.get(req.headers.get(HEADER_SESSION))
        if session is None:
            return self._make_status(req, 454)

        session.active = True
        self.session_event_callable and self.session_event_callable(session)

        status = self._make_status(req, 200)
        if HEADER_RANGE in req.headers:
            status.headers[HEADER_X_WARNING_MSG] = 'Range header not supported'
        return status
        
    def _handle_teardown_request(self, handler, req):
        session_id = req.headers.get(HEADER_SESSION)
        if session_id is None:
            return self._make_status(req, 454)
        session = self.sessions.get(session_id)
        if session:
            del self.sessions[session_id]
            session.active = False
            self.session_event_callable and self.session_event_callable(session)
        return self._make_status(req, 200)
    
    def _validate_request(self, req):
        if HEADER_CSEQ not in req.headers:
            return self._make_status(req, 400)
        if req.method not in self.ALLOWED_METHODS:
            status = self._make_status(req, 405)
            status.headers[HEADER_ALLOW] = ', '.join(self.ALLOWED_METHODS)
            return status

    REQUEST_HANDLER_MAP = { METHOD_SETUP: _handle_setup_request,
                            METHOD_PLAY: _handle_play_request,
                            METHOD_TEARDOWN: _handle_teardown_request }
    
    def _handle_request(self, handler, req):
        logger.log(logger.DEBUG, "Received request: %s %s %s", req.method, req.uri, req.headers)
        status = self._validate_request(req)
        status = status or self.REQUEST_HANDLER_MAP[req.method](self, handler, req)
        handler.send_status(status)
        self.transaction_event_callable and self.transaction_event_callable(status)
        return req.headers.get(HEADER_CONNECTION, 'Close') != 'Close'

    def _handle_close(self, handler):
        logger.log(logger.DEBUG,  "Closed connection: %s", self.handlers[handler])
        del self.handlers[handler]


class _ServerConnHandler(asyncore.dispatcher):
    READ_SIZE = 4096
    
    def __init__(self, accept_handler, request_handler, close_handler, sock=None):
        asyncore.dispatcher.__init__(self, sock)
        self.accept_handler = accept_handler
        self.request_handler = request_handler
        self.close_handler = close_handler
        if sock:
            self.proto = RTSPProtocol(RTSPRequest)

    def start(self, binder):
        if_index, if_name, addr_family, addr, port, tos, tclass = binder
        self.create_socket(addr_family, socket.SOCK_STREAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_BINDTODEVICE, if_name + chr(0))
        self.set_reuse_addr()
        if addr_family == socket.AF_INET6:
            self.socket.setsockopt(socket.IPPROTO_IPV6, 67, tclass)
            self.bind((addr, port, 0, 0))
        else:
            self.socket.setsockopt(socket.IPPROTO_IP, socket.IP_TOS, tos)
            self.bind((addr, port))
        self.listen(5)

    def send_status(self, status):
        self.send(self.proto.output(status))
        
    def readable(self):
        return True

    def writable(self):
        return False

    def handle_accept(self):
        sock, addr = self.accept()
        handler = _ServerConnHandler(self.accept_handler, self.request_handler, self.close_handler, sock)
        self.accept_handler(handler, addr)
        
    def handle_read(self):
        try:
            for req in self.proto.input(self.recv(self.READ_SIZE)):
                if not self.request_handler(self, req):
                    self.handle_close()
                    break
        except (socket.error, RTSPException):
            self.handle_close()

    def handle_close(self):
        self.close()
        self.close_handler(self)
