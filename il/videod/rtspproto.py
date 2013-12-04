#
# @file
# @brief RTSP protocol implementation
#
# Copyright (c) 2008 by Spirent Communications Inc.
# All Rights Reserved.
#
# This software is confidential and proprietary to Spirent Communications Inc.
# No part of this software may be reproduced, transmitted, disclosed or used
# in violation of the Software License Agreement without the expressed
# written consent of Spirent Communications Inc.
#

import re


# Protocol constants
VERSION = "1.0"
PORT_NUMBER = 554

# Method names
METHOD_ANNOUNCE = 'ANNOUNCE'
METHOD_DESCRIBE = 'DESCRIBE'
METHOD_GET_PARAMETER = 'GET_PARAMETER'
METHOD_OPTIONS = 'OPTIONS'
METHOD_PAUSE = 'PAUSE'
METHOD_PLAY = 'PLAY'
METHOD_RECORD = 'RECORD'
METHOD_REDIRECT = 'REDIRECT'
METHOD_SETUP = 'SETUP'
METHOD_SET_PARAMETER = 'SET_PARAMETER'
METHOD_TEARDOWN = 'TEARDOWN'

# Important header fields
HEADER_ALLOW = 'Allow'
HEADER_CONNECTION = 'Connection'
HEADER_CONTENT_LENGTH = 'Content-Length'
HEADER_CSEQ = 'CSeq'
HEADER_RANGE = 'Range'
HEADER_SERVER = 'Server'
HEADER_SESSION = 'Session'
HEADER_TRANSPORT = 'Transport'
HEADER_X_LOOP_FLAG = 'X-Loop-Flag'
HEADER_X_ERROR_MSG = 'X-Error-Msg'
HEADER_X_WARNING_MSG = 'X-Warning-Msg'

# Transport parameters
TRANSPORT_MULTICAST = 'multicast'
TRANSPORT_UNICAST = 'unicast'
TRANSPORT_SOURCE = 'source'
TRANSPORT_DESTINATION = 'destination'
TRANSPORT_PORT = 'port'
TRANSPORT_CLIENT_PORT = 'client_port'
TRANSPORT_SERVER_PORT = 'server_port'
TRANSPORT_TTL = 'ttl'

# Connection states
_STATE_IDLE = "Idle"
_STATE_HEADER = "Header"
_STATE_BODY = "Body"

# Pre-compiled regular expressions
_RE_REQUEST_LINE = re.compile(r'^(\w+)\s+([^ ]+)\s+RTSP/(\d+\.\d+)')
_RE_STATUS_LINE = re.compile(r'^RTSP/(\d+\.\d+)\s+(\d+)\s+(.*)')
_RE_HEADER_LINE = re.compile(r'^([\w\-]+)\s*:\s*(.*)|\s+(.*)')
_RE_TRANSPORT_FIELD = re.compile(r'^RTP/AVP(/TCP|/UDP)?(.*)')
_RE_TRANSPORT_PARAM = re.compile(r'^(\w+)(?:=(.*))?')


class RTSPRequest(object):
    def __init__(self):
        self.method = None
        self.uri = None
        self.version = None
        self.headers = {}
        self.body = ''


class RTSPStatus(object):
    def __init__(self):
        self.version = None
        self.status_code = None
        self.reason_phrase = None
        self.headers = {}
        self.body = ''


class RTSPTransport(object):
    PORT_PAIR_NAMES = (TRANSPORT_PORT, TRANSPORT_CLIENT_PORT)
    
    def __init__(self, value=None):
        self.proto = None
        self.params = {}
        if value is not None:
            self.decode(value)

    def decode(self, value):
        try:
            proto, params = _RE_TRANSPORT_FIELD.findall(value)[0]
            self.proto = ('TCP' if proto == '/TCP' else 'UDP')
            for param in params.split(';'):
                if param:
                    name, value = _RE_TRANSPORT_PARAM.findall(param)[0]
                    if name in self.PORT_PAIR_NAMES:
                        self.params[name] = [int(x) for x in value.split('-')]
                    else:
                        self.params[name] = value or None
        except (IndexError, ValueError):
            raise BadTransportValue(value)

    def encode(self):
        retval = 'RTP/AVP/' + self.proto
        for name, value in self.params.items():
            if name in self.PORT_PAIR_NAMES:
                retval += ';%s=%u-%u' % (name, value[0], value[1])
            elif value:
                retval += ';%s=%s' % (name, value)
            else:
                retval += ';%s' % (name)
        return retval


class RTSPProtocol(object):
    ALLOWED_MESSAGE_TYPES = (RTSPRequest, RTSPStatus)
    
    def __init__(self, default_message_type):
        if default_message_type not in self.ALLOWED_MESSAGE_TYPES:
            raise TypeError, "RTSPProtocol only supports %s message types" % self.ALLOWED_MESSAGE_TYPES
        self.default_message_type = default_message_type
        self.reset()

    def reset(self):
        self.state = _STATE_IDLE
        self.buffer = _LineBuffer()
        self.message = None
        self.last_field_name = None
        self.body_remaining = 0

    def input(self, buffer, message_type=None):
        self.buffer.append(buffer)
        msgs = []
        remaining = None
        while remaining != self.buffer.remaining():
            remaining = self.buffer.remaining()
            # Create a new message as needed
            if self.message is None:
                if message_type is not None:
                    if message_type not in self.ALLOWED_MESSAGE_TYPES:
                        raise TypeError, "RTSPProtocol only supports %s message types" % self.ALLOWED_MESSAGE_TYPES
                    self.message = message_type()
                else:
                    self.message = self.default_message_type()
            # Process header lines first
            complete = False
            if not self.body_remaining:
                for line in self.buffer:
                    if self.INPUT_STATE_MAP[self.state](self, line):
                        complete = True
                        break
            # Process body
            if not complete and self.body_remaining:
                complete = self.INPUT_STATE_MAP[self.state](self, self.buffer.release())
            # Stash completed messages for return to caller
            if complete:
                self._change_state(_STATE_IDLE)
                msg, self.message = self.message, None
                self.last_field_name = None
                self.body_remaining = 0
                msgs.append(msg)
        return msgs
    
    def output(self, message):
        # Auto-update Content-Length header if it is required
        content_length = len(message.body)
        if content_length and HEADER_CONTENT_LENGTH not in message.headers:
            message.headers[HEADER_CONTENT_LENGTH] = content_length
        # Build request/status line
        first_line = None
        if isinstance(message, RTSPRequest):
            first_line = '%s %s RTSP/%s\r\n' % (message.method, message.uri, str(message.version))
        elif isinstance(message, RTSPStatus):
            reason_phrase = _REASON_PHRASE_MAP.get(message.status_code, "Unknown")
            first_line = 'RTSP/%s %u %s\r\n' % (str(message.version), message.status_code, reason_phrase)
        else:
            raise TypeError, "RTSPProtocol can only output RTSPRequest or RTSPStatus objects"
        return first_line + ''.join((('%s: %s\r\n' % (field_name, str(field_value))) for field_name, field_value in message.headers.items())) + '\r\n' + message.body

    def _change_state(self, new_state):
        self.state = new_state
        
    def _input_idle_state(self, line):
        # Blank lines preceeding the request line SHOULD be tolerated (RFC 2616, Section 4.1)
        line = line.rstrip("\r\n")
        if not len(line):
            return False
        if isinstance(self.message, RTSPRequest):
            # Parse request line and store method, uri and version in message object
            try:
                self.message.method, self.message.uri, self.message.version = _RE_REQUEST_LINE.findall(line)[0]
            except IndexError:
                raise BadRequestLine(line)
        else:
            # Parse status line and store version, status code and reason phrase in message object
            try:
                self.message.version, status_code, self.message.reason_phrase = _RE_STATUS_LINE.findall(line)[0]
                self.message.status_code = int(status_code)
                self.message.status_category = (self.message.status_code / 100) * 100
            except IndexError:
                raise BadStatusLine(line)
        # Transition to _STATE_HEADER
        self._change_state(_STATE_HEADER)
        return False
    
    def _input_header_state(self, line):
        # Blank lines signal the end of headers, transition to _STATE_BODY if necessary
        line = line.rstrip("\r\n")
        if not len(line):
            self.body_remaining = int(self.message.headers.get(HEADER_CONTENT_LENGTH, 0))
            if self.body_remaining:
                self._change_state(_STATE_BODY)
                return False
            else:
                return True
        # Parse header line and update field values in request object
        try:
            field_name, field_value, field_cont_value = _RE_HEADER_LINE.findall(line)[0]
            if field_name:
                self.message.headers[field_name] = field_value
                self.last_field_name = field_name
            else:
                self.message.headers[self.last_field_name] += ' ' + field_cont_value
        except (IndexError, KeyError):
            raise BadHeaderLine(line)
        return False
        
    def _input_body_state(self, buffer):
        # Append buffer to body
        if len(buffer) > self.body_remaining:
            self.message.body += buffer[:self.body_remaining]
            self.body_remaining = 0
        else:
            self.message.body += buffer
            self.body_remaining -= len(buffer)
        return not self.body_remaining
    
    INPUT_STATE_MAP = { _STATE_IDLE: _input_idle_state,
                        _STATE_HEADER: _input_header_state,
                        _STATE_BODY: _input_body_state }


class _LineBuffer(object):
    def __init__(self):
        self.buffer = ''
        
    def __iter__(self):
        return self

    def append(self, buffer):
        self.buffer += buffer

    def remaining(self):
        return len(self.buffer)
    
    def release(self):
        retval = self.buffer
        self.buffer = ''
        return retval
    
    def next(self):
        pos = self.buffer.find('\n') + 1
        if not pos:
            raise StopIteration
        retval, self.buffer = self.buffer[:pos], self.buffer[pos:]
        return retval


class RTSPException(Exception):
    # Subclasses that define an __init__ must call Exception.__init__
    # or define self.args.  Otherwise, str() will fail.
    pass


class BadRequestLine(RTSPException):
    def __init__(self, line):
        self.args = line,
        self.line = line


class BadStatusLine(RTSPException):
    def __init__(self, line):
        self.args = line,
        self.line = line


class BadHeaderLine(RTSPException):
    def __init__(self, line):
        self.args = line,
        self.line = line


class BadTransportValue(RTSPException):
    def __init__(self, value):
        self.args = value,
        self.value = value


# Map of status codes to reason phrases
_REASON_PHRASE_MAP = {
    100: "Continue",
    200: "OK",
    201: "Created",
    250: "Low on Storage Space",
    300: "Multiple Choices",
    301: "Moved Permanently",
    302: "Moved Temporarily",
    303: "See Other",
    304: "Not Modified",
    305: "Use Proxy",
    400: "Bad Request",
    401: "Unauthorized",
    402: "Payment Required",
    403: "Forbidden",
    404: "Not Found",
    405: "Method Not Allowed",
    406: "Not Acceptable",
    407: "Proxy Authentication Required",
    408: "Request Time-out",
    410: "Gone",
    411: "Length Required",
    412: "Precondition Failed",
    413: "Request Entity Too Large",
    414: "Request-URI Too Large",
    415: "Unsupported Media Type",
    451: "Parameter Not Understood",
    452: "Conference Not Found",
    453: "Not Enough Bandwidth",
    454: "Session Not Found",
    455: "Method Not Valid in This State",
    456: "Header Field Not Valid for Resource",
    457: "Invalid Range",
    458: "Parameter Is Read-Only",
    459: "Aggregate operation not allowed",
    460: "Only aggregate operation allowed",
    461: "Unsupported transport",
    462: "Destination unreachable",
    500: "Internal Server Error",
    501: "Not Implemented",
    502: "Bad Gateway",
    503: "Service Unavailable",
    504: "Gateway Time-out",
    505: "RTSP Version not supported",
    551: "Option not supported"
    }
