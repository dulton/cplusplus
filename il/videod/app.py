#
# @file
# @brief Video application class
#
# Copyright (c) 2008 by Spirent Communications Inc.
# All Rights Reserved.
#
# This software is confidential and proprietary to Spirent Communications Inc.
# No part of this software may be reproduced, transmitted, disclosed or used
# in violation of the Software License Agreement without the expressed
# written consent of Spirent Communications Inc.
#

import glob, os

import mps.logger as logger
from constants import *
from client import VideoClientBlock
from server import VideoServerBlock
from statsframework.database import Database
from timer import Timer
import utils


class VideoApplication(object):
    class PortContext(object):
        def __init__(self):
            self.if_block_map = {}
            self.bll_handle_map = {}
            self.if_handle_map = {}

        def new_if_block(self, if_block):
            self.if_block_map[if_block['handle']] = if_block

        def del_if_handle(self, if_handle):
            obj_set = self.if_handle_map.get(if_handle)
            if obj_set:
                bll_handles = list(obj_set)
                map(self.del_object, bll_handles)
                del self.if_handle_map[if_handle]
            try:
                del self.if_block_map[if_handle]
            except KeyError:
                pass
            
        def new_object(self, obj):
            self.del_object(obj.bll_handle)
            self.bll_handle_map[obj.bll_handle] = obj
            obj_set = self.if_handle_map.setdefault(obj.if_handle, set())
            obj_set.add(obj.bll_handle)
            return obj.bll_handle

        def del_object(self, bll_handle):
            obj = self.bll_handle_map.get(bll_handle)
            if obj:
                obj.reset()
                self.if_handle_map[obj.if_handle].discard(bll_handle)
                del self.bll_handle_map[bll_handle]

        def gen_objects(self):
            for obj in self.bll_handle_map.values():
                yield obj
            
        def gen_objects_from_bll_handles(self, bll_handles):
            return (self.bll_handle_map.get(bll_handle) for bll_handle in bll_handles)

        def gen_objects_from_if_handles(self, if_handles):
            return (self.bll_handle_map.get(bll_handle) for bll_handle in (self.if_handle_map.get(if_handle) for if_handle in if_handles))
                
    def __init__(self, ports):
        self.ports = [self.PortContext() for port in range(ports)]
        self.db = None
        self.stats_clear_timers = {}

    def activate(self):
        self.db = self.db or Database(DB_SCHEMA_FILE, DB_SHARED_LIB, len(self.ports))

    def configClients(self, port, clients):
        port_ctxt = self.ports[port]
        handles = []
        for client in clients:
            if_block = port_ctxt.if_block_map[client['Endpoint']['SrcIfHandle']]
            bll_handle = port_ctxt.new_object(VideoClientBlock(port, client, if_block, self.db))
            logger.log(logger.DEBUG, "Configured client handle %u on port %u: %s", bll_handle, port, str(client))
            handles.append(bll_handle)
        return handles

    def deleteClients(self, port, bll_handles):
        port_ctxt = self.ports[port]
        map(port_ctxt.del_object, bll_handles)

    def updateClients(self, port, bll_handles, clients):
        self.deleteClients(port, bll_handles)
        self.configClients(port, clients)

    def configServers(self, port, servers):
        port_ctxt = self.ports[port]
        handles = []
        for server in servers:
            if_block = port_ctxt.if_block_map[server['Common']['Endpoint']['SrcIfHandle']]
            bll_handle = port_ctxt.new_object(VideoServerBlock(port, server, if_block, self.db))
            logger.log(logger.DEBUG, "Configured server handle %u on port %u: %s", bll_handle, port, str(server))
            handles.append(bll_handle)
        return handles

    def deleteServers(self, port, bll_handles):
        port_ctxt = self.ports[port]
        map(port_ctxt.del_object, bll_handles)

    def updateServers(self, port, bll_handles, servers):
        self.deleteServers(port, bll_handles)
        self.configServers(port, servers)

    def enumerateVideoClips(self, port, glob_spec):
        old_cwd = os.getcwd()
        os.chdir(VIDEO_CLIP_PATH)
        file_list = glob.glob(glob_spec or '*')
        clip_files = []
        for clip_file in file_list:
            if(os.path.isdir(clip_file)):
                continue
            else:
                clip_files.append(clip_file)        
        os.chdir(old_cwd)
        return clip_files

    def startProtocol(self, port, bll_handles):
        port_ctxt = self.ports[port]
        map(lambda x: x.start(), port_ctxt.gen_objects_from_bll_handles(bll_handles))

    def stopProtocol(self, port, bll_handles):
        port_ctxt = self.ports[port]
        map(lambda x: x.stop(), port_ctxt.gen_objects_from_bll_handles(bll_handles))

    def clearResults(self, port, bll_handles, abs_exec_time):
        rel_exec_time = 0
        if abs_exec_time:
            now = utils.getTimeStamp()
            if abs_exec_time > now:
                rel_exec_time = abs_exec_time - now
        if rel_exec_time:  # NB: rel_exec_time is in 2.5ns backplane ticks
            self.stats_clear_timers[port] = Timer(rel_exec_time / 400000000.0, lambda timeout: self.clearResultsImmediate(port, bll_handles))
        else:
            self.clearResultsImmediate(port, bll_handles)

    def clearResultsImmediate(self, port, bll_handles):
        port_ctxt = self.ports[port]
        if bll_handles:
            map(lambda x: x.clear_stats(), port_ctxt.gen_objects_from_bll_handles(bll_handles))
        else:
            map(lambda x: x.clear_stats(), port_ctxt.gen_objects())
    
    def attachInterfacesComplete(self, port, if_blocks):
        port_ctxt = self.ports[port]
        map(port_ctxt.new_if_block, if_blocks)
        
    def detachInterfacesPending(self, port, if_handles):
        port_ctxt = self.ports[port]
        map(port_ctxt.del_if_handle, if_handles)

    def deleteInterfacesPending(self, port, if_handles):
        port_ctxt = self.ports[port]
        map(port_ctxt.del_if_handle, if_handles)

    def updateInterfacesPending(self, port, if_handles):
        port_ctxt = self.ports[port]
        map(lambda x: x.enable(False), port_ctxt.gen_objects_from_if_handles(if_handles))

    def updateInterfacesComplete(self, port, if_blocks):
        port_ctxt = self.ports[port]
        if_handles = [if_block['handle'] for if_block in if_blocks]
        clients = []
        servers = []
        for obj in port_ctxt.gen_objects_from_if_handles(if_handles):
            if isinstance(obj, VideoClientBlock):
                clients.append(obj.config)
            elif isinstance(obj, VideoServerBlock):
                servers.append(obj.config)
            else:
                raise NotImplementedError
        map(port_ctxt.del_if_handle, if_handles)
        self.configClients(port, clients)
        self.configServers(port, servers)
