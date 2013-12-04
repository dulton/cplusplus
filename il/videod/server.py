#
# @file
# @brief Video server classes
#
# Copyright (c) 2011 by Spirent Communications Inc.
# All Rights Reserved.
#
# This software is confidential and proprietary to Spirent Communications Inc.
# No part of this software may be reproduced, transmitted, disclosed or used
# in violation of the Software License Agreement without the expressed
# written consent of Spirent Communications Inc.
#

import os, socket

import mps.idl
import mps.logger as logger
from constants import *
from rtspserver import RTSPServer
from cache import TmpFileCache, CacheSizeLimitPolicy
from streamer import Streamer, StreamerParams
import utils


# Video_1 IDL (for access to enum values)
_VIDEO_1 = mps.idl.makeMessageSet(MESSAGE_SET)

# Server event types
_EVENT_CONNECTION = 1
_EVENT_TRANSACTION = 2
_EVENT_SESSION = 3

# Shared tmp file cache
_CACHE = TmpFileCache(VIDEO_CACHE_PATH, CacheSizeLimitPolicy(MAX_VIDEO_CACHE_SIZE))


class VideoServer(object):
    class Stream(object):
        def __init__(self, streamer_params, exit_callable=None):
            self.streamer = None
            self.streamer_params = streamer_params
            self.exit_callable = exit_callable

        def start(self):
            if not self.streamer:
                self.streamer = Streamer(self.streamer_params)
                self.streamer.set_exit_callable(lambda status: self.exit_callable(self))

        def stop(self):
            self.streamer.close()
            self.streamer = None

    STREAMER_PROTO_MAP = { (_VIDEO_1.TransportTypes_UDP, socket.AF_INET): StreamerParams.PROTO_UDP4,
                           (_VIDEO_1.TransportTypes_RTP, socket.AF_INET): StreamerParams.PROTO_RTP4,
                           (_VIDEO_1.TransportTypes_UDP, socket.AF_INET6): StreamerParams.PROTO_UDP6,
                           (_VIDEO_1.TransportTypes_RTP, socket.AF_INET6): StreamerParams.PROTO_RTP6 }
                           
    def __init__(self, port, config, if_type, if_stack, event_callable):
        self.port = port
        self.config = config
        self.if_index = if_stack['ifIndex']
        self.if_name = if_stack['ifName']
        self.addr_family = ADDR_FAMILY_MAP[if_type]
        self.addr = if_stack['addr']
        self.tos = config['Common']['Profile']['L4L7Profile']['Ipv4Tos']
        self.tclass = config['Common']['Profile']['L4L7Profile']['Ipv6TrafficClass']
        self.event_callable = event_callable
        self.enabled = True
        self.rtsp = None
        self.rtsp_streams = {}
        self.static_streams = []

        clip_files = config['ProtocolConfig']['ClipFiles']
        if not clip_files:
            raise RuntimeError, "Cannot configure server without clip files"

        clip_files = map(lambda x: x.strip(), clip_files)
        map(_CACHE.insert, (os.path.join(VIDEO_CLIP_PATH, x) for x in clip_files))
        map(lambda file: os.chmod(file, 0644), (os.path.join(VIDEO_CACHE_PATH, x) for x in clip_files))

        if config['ProtocolConfig']['RtspEnabled']:
            self.rtsp = RTSPServer(_CACHE.path)
            self.rtsp.set_connection_event_callable(self._connection_event_handler)
            self.rtsp.set_transaction_event_callable(self._transaction_event_handler)
            self.rtsp.set_session_event_callable(self._session_event_handler)

        for stream in config['StaticStreams']:
            streamer_params = StreamerParams()
            streamer_params.proto = self.STREAMER_PROTO_MAP[(stream['Transport'], utils.getAddrFamily(stream['DstIpAddr']))]
            streamer_params.stream_files = clip_files
            streamer_params.loop = stream['Loop']
            streamer_params.ifname = if_stack['ifName']
            streamer_params.saddr = self.addr
            streamer_params.daddr = stream['DstIpAddr']
            streamer_params.dport = stream['DstPort']
            streamer_params.tclass = self.tclass
            streamer_params.tos = self.tos
            self.static_streams.append(self.Stream(streamer_params))

    def reset(self):
        map(_CACHE.remove, self.config['ProtocolConfig']['ClipFiles'])
        
    def enable(self, enabled):
        if not self.enabled:
            self.stop()
    
    def start(self):
        logger.log(logger.DEBUG, "Starting server on %s", self.addr)
        self.rtsp and self.rtsp.start((self.if_index, self.if_name, self.addr_family, self.addr, self.config['ProtocolConfig']['RtspPortNum'], self.tos, self.tclass))
        map(lambda x: x.start(), self.static_streams)
        
    def stop(self):
        logger.log(logger.DEBUG, "Stopping server on %s", self.addr)
        self.rtsp and self.rtsp.stop()
        map(lambda x: x.stop(), self.rtsp_streams.values())
        self.rtsp_streams = {}
        map(lambda x: x.stop(), self.static_streams)

    def _connection_event_handler(self):
        self.event_callable(_EVENT_CONNECTION, None)

    def _transaction_event_handler(self, status):
        self.event_callable(_EVENT_TRANSACTION, status)

    def _session_event_handler(self, session):
        if session.active:
            streamer_params = StreamerParams()
            streamer_params.proto = (StreamerParams.PROTO_RTP4 if self.addr_family == socket.AF_INET else StreamerParams.PROTO_RTP6)
            streamer_params.stream_files = [ session.stream_path ]
            streamer_params.loop = session.loop
            streamer_params.ifname = self.if_name
            streamer_params.saddr = self.addr
            streamer_params.daddr = session.daddr
            streamer_params.dport = session.dport
            streamer_params.tclass = self.tclass
            streamer_params.tos = self.tos
            stream = self.Stream(streamer_params, self._streamer_exit_handler)
            stream.session = session
            stream.start()
            self.rtsp_streams[session.id] = stream
        else:
            stream = self.rtsp_streams.get(session.id)
            if stream:
                stream.stop()
                del self.rtsp_streams[session.id]
        self.event_callable(_EVENT_SESSION, session)
    
    def _streamer_exit_handler(self, stream):
        session = stream.session
        if session.id in self.rtsp_streams:
            del self.rtsp_streams[session.id]
            session.active = False
            self.event_callable(_EVENT_SESSION, session)


class VideoServerBlock(object):
    class StatsDriver(object):
        RESULTS_TABLE = 'VideoServerResults'
        SESSION_EVENTS_TABLE = 'VideoServerSessionEvents'
        COUNTED_STATUS_CODES = ( 200, 400, 403, 404, 405, 454, 459, 461 )
        NEXT_BLOCK_INDEX = 1
        
        def __init__(self, port, db, bll_handle):
            self.port = port
            self.db = db
            self.block_index, VideoServerBlock.StatsDriver.NEXT_BLOCK_INDEX = VideoServerBlock.StatsDriver.NEXT_BLOCK_INDEX, VideoServerBlock.StatsDriver.NEXT_BLOCK_INDEX + 1
            self.bll_handle = bll_handle
            self.results_table = self.db.table(self.RESULTS_TABLE)
            self.session_events_table = self.db.table(self.SESSION_EVENTS_TABLE)
            self.enable_event_recording = False
            self._new_result_row()
            
        def _new_result_row(self):
            t = self.db.writeTransaction(self.port)
            row = self.results_table.new(t)
            row.BlockIndex = self.block_index
            row.Handle = self.bll_handle
            row.LastModified = utils.getTimeStamp()
            self.autoid = row.autoid
            t.commit()

        def _del_result_row(self):
            try:
                t = self.db.writeTransaction(self.port)
                row = self.results_table.find(t, self.autoid)
                self.results_table.delete(row)
                t.commit()
            except:
                pass
        
        def reset(self):
            self._del_result_row()
            
        def clear(self):
            self._del_result_row()
            self._new_result_row()
            if self.enable_event_recording:
                self.db.doSql(self.port, ['delete from %s where Handle = %u' % (self.SESSION_EVENTS_TABLE, self.bll_handle)])
            
        def event_handler(self, event, data):
            if event == _EVENT_CONNECTION:
                try:
                    t = self.db.writeTransaction(self.port)
                    row = self.results_table.find(t, self.autoid)
                    row.TotalConnections += 1
                    row.LastModified = utils.getTimeStamp()
                    t.commit()
                except:
                    pass
            elif event == _EVENT_TRANSACTION:
                try:
                    t = self.db.writeTransaction(self.port)
                    row = self.results_table.find(t, self.autoid)
                    if ((data.status_code / 100) * 100) == 200:
                        row.SuccessfulTransactions += 1
                    else:
                        row.UnsuccessfulTransactions += 1
                    if data.status_code in self.COUNTED_STATUS_CODES:
                        counter = 'TxResponseCode%u' % data.status_code
                        setattr(row, counter, getattr(row, counter) + 1)
                    row.LastModified = utils.getTimeStamp()
                    t.commit()
                except:
                    pass
            elif event == _EVENT_SESSION:
                if self.enable_event_recording:
                    try:
                        t = self.db.writeTransaction(self.port)
                        row = self.session_events_table.new(t)
                        row.EventTimestamp = utils.getTimeStamp()
                        row.Handle = self.bll_handle
                        row.SrcAddress = data.saddr
                        row.DstAddress = data.daddr
                        row.DstPort = data.dport
                        row.ClipFile = os.path.basename(data.uri)
                        row.Playing = (1 if data.active else 0)
                        t.commit()
                    except:
                        pass
            else:
                raise NotImplementedError, "Unexpected server event: %u" % event
        
    def __init__(self, port, config, if_block, db):
        if if_block['type'] not in ['IPv4', 'IPv6']:
            raise TypeError, "Cannot configure video server(s) on non-IP interface block"
        self.port = port
        self.config = config
        self.bll_handle = config['ProtocolConfig']['L4L7ProtocolConfigServerBase']['L4L7ProtocolConfigBase']['BllHandle']
        self.if_handle = config['Common']['Endpoint']['SrcIfHandle']
        self.stats = self.StatsDriver(port, db, self.bll_handle)
        self.stats.enable_event_recording = config['ProtocolConfig']['EnableEventRecording']
        self.servers = [VideoServer(port, config, if_block['type'], if_stack, self.stats.event_handler) for if_stack in if_block['stacks']]
        self.running = False

    def reset(self):
        self.stop()
        map(lambda x: x.reset(), self.servers)
        self.stats.reset()
            
    def enable(self, enabled):
        map(lambda x: x.enable(enabled), self.servers)
    
    def start(self):
        if not self.running:
            map(lambda x: x.start(), self.servers)
            self.running = True

    def stop(self):
        if self.running:
            map(lambda x: x.stop(), self.servers)
            self.running = False

    def clear_stats(self):
        self.stats.clear()

