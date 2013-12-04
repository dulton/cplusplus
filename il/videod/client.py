#
# @file
# @brief Video client classes
#
# Copyright (c) 2011 by Spirent Communications Inc.
# All Rights Reserved.
#
# This software is confidential and proprietary to Spirent Communications Inc.
# No part of this software may be reproduced, transmitted, disclosed or used
# in violation of the Software License Agreement without the expressed
# written consent of Spirent Communications Inc.
#

import socket, urlparse

import mps.logger as logger
from constants import *
from rtspclient import RTSPClient
import utils


# Client event types
_EVENT_CONNECTION_ATTEMPTED = 1
_EVENT_CONNECTION_SUCCESSFUL = 2
_EVENT_CONNECTION_UNSUCCESSFUL = 3
_EVENT_CONNECTION_ABORTED = 4
_EVENT_TRANSACTION_ATTEMPTED = 5
_EVENT_TRANSACTION_COMPLETED = 6
_EVENT_TRANSACTION_ABORTED = 7


class _DisconnectedState(object):
    NAME = 'DISCONNECTED'
    
    def __init__(self, client):
        self.c = client

    def start_connection(self):
        try:
            self.c._start_connection()
        except (socket.error, socket.gaierror), e:
            self.c.event_callable(_EVENT_CONNECTION_ATTEMPTED, None)
            self.c.event_callable(_EVENT_CONNECTION_UNSUCCESSFUL, None)
        else:
            self.c.event_callable(_EVENT_CONNECTION_ATTEMPTED, None)
            self.c._change_state(_ConnectWaitState)

    def stop_connection(self):
        pass

    def connection_event(self, connected):
        pass


class _ConnectWaitState(object):
    NAME = 'CONNECT_WAIT'
    
    def __init__(self, client):
        self.c = client

    def start_connection(self):
        pass
    
    def stop_connection(self):
        self.c.event_callable(_EVENT_CONNECTION_ABORTED, None)
        self.c._stop_connection()
        self.c._change_state(_DisconnectedState)

    def connection_event(self, connected):
        if connected:
            self.c.event_callable(_EVENT_CONNECTION_SUCCESSFUL, None)
            self.c._send_setup_request()
            self.c.event_callable(_EVENT_TRANSACTION_ATTEMPTED, None)
            self.c._change_state(_SetupWaitState)
        else:
            self.c.event_callable(_EVENT_CONNECTION_UNSUCCESSFUL, None)
            self.c._change_state(_DisconnectedState)
        
        
class _SetupWaitState(object):
    NAME = 'SETUP_WAIT'
    
    def __init__(self, client):
        self.c = client

    def start_connection(self):
        pass
    
    def stop_connection(self):
        self.c.event_callable(_EVENT_TRANSACTION_ABORTED, None)
        self.c._stop_connection()
        self.c._change_state(_DisconnectedState)

    def transaction_event(self, status):
        self.c.event_callable(_EVENT_TRANSACTION_COMPLETED, status)
        if status.status_category == 200:
            self.c._send_play_request()
            self.c.event_callable(_EVENT_TRANSACTION_ATTEMPTED, None)
            self.c._change_state(_PlayWaitState)
        else:
            self.c._stop_connection()
            self.c._change_state(_DisconnectedState)

    def connection_event(self, connected):
        if not connected:
            self.c.event_callable(_EVENT_TRANSACTION_ABORTED, None)
            self.c._change_state(_DisconnectedState)
    
        
class _PlayWaitState(object):
    NAME = 'PLAY_WAIT'
    
    def __init__(self, client):
        self.c = client

    def start_connection(self):
        pass
    
    def stop_connection(self):
        self.c.event_callable(_EVENT_TRANSACTION_ABORTED, None)
        self.c._stop_connection()
        self.c._change_state(_DisconnectedState)

    def transaction_event(self, status):
        self.c.event_callable(_EVENT_TRANSACTION_COMPLETED, status)
        if status.status_category == 200:
            self.c._change_state(_SessionState)
        else:
            self.c._send_teardown_request()
            self.c.event_callable(_EVENT_TRANSACTION_ATTEMPTED, None)
            self.c._change_state(_TeardownWaitState)

    def connection_event(self, connected):
        if not connected:
            self.c.event_callable(_EVENT_TRANSACTION_ABORTED, None)
            self.c._change_state(_DisconnectedState)


class _SessionState(object):
    NAME = 'SESSION'
    
    def __init__(self, client):
        self.c = client

    def start_connection(self):
        pass
    
    def stop_connection(self):
        self.c._send_teardown_request()
        self.c.event_callable(_EVENT_TRANSACTION_ATTEMPTED, None)
        self.c._change_state(_TeardownWaitState)

    def connection_event(self, connected):
        if not connected:
            self.c._change_state(_DisconnectedState)


class _TeardownWaitState(object):
    NAME = 'TEARDOWN_WAIT'
    
    def __init__(self, client):
        self.c = client

    def start_connection(self):
        pass
    
    def stop_connection(self):
        pass

    def transaction_event(self, status):
        self.c.event_callable(_EVENT_TRANSACTION_COMPLETED, status)
        self.c._stop_connection()
        self.c._change_state(_DisconnectedState)

    def connection_event(self, connected):
        if not connected:
            self.c.event_callable(_EVENT_TRANSACTION_ABORTED, None)
            self.c._change_state(_DisconnectedState)


class VideoClient(object):
    def __init__(self, port, config, if_type, if_stack, event_callable):
        self.port = port
        self.config = config
        self.addr = if_stack['addr']
        self.event_callable = event_callable
        self.enabled = True
        self.rtsp = RTSPClient()
        self.rtsp.set_connection_event_callable(self._connection_event_handler)
        self.rtsp.set_transaction_event_callable(self._transaction_event_handler)
        self.rtsp_binder = (if_stack['ifIndex'], if_stack['ifName'], ADDR_FAMILY_MAP[if_type], self.addr, config['Endpoint']['DstIpAddr'], config['Endpoint']['DstPort'], config['Profile']['L4L7Profile']['Ipv4Tos'], config['Profile']['L4L7Profile']['Ipv6TrafficClass'])
        self.state = _DisconnectedState(self)

    def enable(self, enabled):
        if not self.enabled:
            self.stop()
    
    def start(self):
        logger.log(logger.DEBUG, "Starting client on %s", self.addr)
        self.state.start_connection()
        
    def stop(self):
        logger.log(logger.DEBUG, "Stopping client on %s", self.addr)
        self.state.stop_connection()

    def _change_state(self, new_state):
        old_state_name = self.state.NAME
        self.state = new_state(self)
        logger.log(logger.DEBUG, "Changed state from %s to %s", old_state_name, self.state.NAME)
        
    def _start_connection(self):
        self.rtsp.connect(self.rtsp_binder)

    def _stop_connection(self):
        self.rtsp.close()

    def _send_setup_request(self):
        uri = urlparse.urljoin('/', self.config['ProtocolConfig']['ClipFile'])
        dst_addr = self.config['ProtocolConfig']['DstIpAddr']
        dst_addr = (dst_addr, self.addr)[utils.isUnspecifiedAddr(dst_addr)]
        dst_ports = self.config['ProtocolConfig']['DstPort']
        dst_ports = (DEFAULT_PORTS, (dst_ports, dst_ports + 1))[bool(dst_ports)]
        ttl = DEFAULT_TTL
        loop = self.config['ProtocolConfig']['Loop']
        self.rtsp.setup(uri, dst_addr, dst_ports, ttl, loop)

    def _send_play_request(self):
        self.rtsp.play()

    def _send_teardown_request(self):
        self.rtsp.teardown()
    
    def _connection_event_handler(self, connected):
        self.state.connection_event(connected)

    def _transaction_event_handler(self, status):
        self.state.transaction_event(status)


class VideoClientBlock(object):
    class StatsDriver(object):
        TABLE = 'VideoClientResults'
        COUNTED_STATUS_CODES = ( 200, 400, 403, 404, 405, 454, 459, 461 )
        NEXT_BLOCK_INDEX = 1

        def __init__(self, port, db, bll_handle):
            self.port = port
            self.db = db
            self.block_index, VideoClientBlock.StatsDriver.NEXT_BLOCK_INDEX = VideoClientBlock.StatsDriver.NEXT_BLOCK_INDEX, VideoClientBlock.StatsDriver.NEXT_BLOCK_INDEX + 1
            self.bll_handle = bll_handle
            self.table = self.db.table(self.TABLE)
            self._new_result_row()
            
        def _new_result_row(self):
            t = self.db.writeTransaction(self.port)
            row = self.table.new(t)
            row.BlockIndex = self.block_index
            row.Handle = self.bll_handle
            row.LastModified = utils.getTimeStamp()
            self.autoid = row.autoid
            t.commit()

        def _del_result_row(self):
            try:
                t = self.db.writeTransaction(self.port)
                row = self.table.find(t, self.autoid)
                self.table.delete(row)
                t.commit()
            except:
                pass
            
        def reset(self):
            self._del_result_row()
            
        def clear(self):
            self._del_result_row()
            self._new_result_row()
            
        def event_handler(self, event, data):
            try:
                t = self.db.writeTransaction(self.port)
                row = self.table.find(t, self.autoid)
                if event == _EVENT_CONNECTION_ATTEMPTED:
                    row.AttemptedConnections += 1
                elif event == _EVENT_CONNECTION_SUCCESSFUL:
                    row.SuccessfulConnections += 1
                elif event == _EVENT_CONNECTION_UNSUCCESSFUL:
                    row.UnsuccessfulConnections += 1
                elif event == _EVENT_CONNECTION_ABORTED:
                    row.AbortedConnections += 1
                elif event == _EVENT_TRANSACTION_ATTEMPTED:
                    row.AttemptedTransactions += 1
                elif event == _EVENT_TRANSACTION_COMPLETED:
                    if data.status_category == 200:
                        row.SuccessfulTransactions += 1
                    else:
                        row.UnsuccessfulTransactions += 1
                    if data.status_code in self.COUNTED_STATUS_CODES:
                        counter = 'RxResponseCode%u' % data.status_code
                        setattr(row, counter, getattr(row, counter) + 1)
                elif event == _EVENT_TRANSACTION_ABORTED:
                    row.AbortedTransactions += 1
                else:
                    raise NotImplementedError, "Unexpected client event: %u" % event
                row.LastModified = utils.getTimeStamp()
                t.commit()
            except:
                t.rollback()
        
    def __init__(self, port, config, if_block, db):
        if if_block['type'] not in ['IPv4', 'IPv6']:
            raise TypeError, "Cannot configure video client(s) on non-IP interface block"
        self.port = port
        self.config = config
        self.bll_handle = config['ProtocolConfig']['L4L7ProtocolConfigClientBase']['L4L7ProtocolConfigBase']['BllHandle']
        self.if_handle = config['Endpoint']['SrcIfHandle']
        self.stats = self.StatsDriver(port, db, self.bll_handle)
        self.clients = [VideoClient(port, config, if_block['type'], if_stack, self.stats.event_handler) for if_stack in if_block['stacks']]
        self.running = False

    def reset(self):
        self.stop()
        self.stats.reset()
            
    def enable(self, enabled):
        map(lambda x: x.enable(enabled), self.clients)
    
    def start(self):
        if not self.running:
            map(lambda x: x.start(), self.clients)
            self.running = True

    def stop(self):
        if self.running:
            map(lambda x: x.stop(), self.clients)
            self.running = False

    def clear_stats(self):
        self.stats.clear()

