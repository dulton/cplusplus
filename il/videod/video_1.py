#
# @file
# @brief Video_1 message set implementation
#
# Copyright (c) 2008 by Spirent Communications Inc.
# All Rights Reserved.
#
# This software is confidential and proprietary to Spirent Communications Inc.
# No part of this software may be reproduced, transmitted, disclosed or used
# in violation of the Software License Agreement without the expressed
# written consent of Spirent Communications Inc.
#

from mps.handlers import MessageSetHandlers
import ifmgrclient
import ifmgrclient.dispatcher
import ifmgrclient.phxrpc
import statsframework.phxrpc
from constants import *


class Video1Handlers(MessageSetHandlers):
    def __init__(self, app):
        MessageSetHandlers.__init__(self, MESSAGE_SET)
        self.app = app
        observers = { 'attachComplete': self.app.attachInterfacesComplete,
                      'detachPending': self.app.detachInterfacesPending,
                      'deletePending': self.app.deleteInterfacesPending,
                      'updatePending': self.app.updateInterfacesPending,
                      'updateComplete': self.app.updateInterfacesComplete }
        if not ifmgrclient.open('videod', observers):
            raise RuntimeError, "Couldn't open interface manager client connection"
        self.ifmgrclient_dispatcher = ifmgrclient.dispatcher.Dispatcher()

    def AttachInterfacesHandler(self, params, port):
        self.app.activate()
        return ifmgrclient.phxrpc.AttachInterfacesHandler(self, params, port)

    def DetachInterfacesHandler(self, params, port):
        self.app.activate()
        return ifmgrclient.phxrpc.DetachInterfacesHandler(self, params, port)

    def DoSQLHandler(self, params, port):
        self.app.activate()
        return statsframework.phxrpc.DoSQLHandler(self, params, port)

    def DoSQLFasterHandler(self, params, port):
        self.app.activate()
        return statsframework.phxrpc.DoSQLFasterHandler(self, params, port)

    def RequestDoSQLNotificationHandler(self, params, port):
        self.app.activate()
        return statsframework.phxrpc.RequestDoSQLNotificationHandler(self, params, port)

    def RequestDoSQLNotificationFasterHandler(self, params, port):
        self.app.activate()
        return statsframework.phxrpc.RequestDoSQLNotificationFasterHandler(self, params, port)

    def DeleteDoSQLNotificationHandler(self, params, port):
        self.app.activate()
        return statsframework.phxrpc.DeleteDoSQLNotificationHandler(self, params, port)
    
    def DeleteDoSQLNotificationFasterHandler(self, params, port):
        self.app.activate()
        return statsframework.phxrpc.DeleteDoSQLNotificationHandler(self, params, port)
    
    def ConfigureVideoClientsHandler(self, params, port):
        self.app.activate()
        return { 'Handles': self.app.configClients(port, params['Clients']) }

    def UpdateVideoClientsHandler(self, params, port):
        self.app.activate()
        self.app.updateClients(port, params['Handles'], params['Clients'])
        return { }

    def DeleteVideoClientsHandler(self, params, port):
        self.app.activate()
        self.app.deleteClients(port, params['Handles'])
        return { }

    def ConfigureVideoServersHandler(self, params, port):
        self.app.activate()
        return { 'Handles': self.app.configServers(port, params['Servers']) }

    def UpdateVideoServersHandler(self, params, port):
        self.app.activate()
        self.app.updateServers(port, params['Handles'], params['Servers'])
        return { }

    def DeleteVideoServersHandler(self, params, port):
        self.app.activate()
        self.app.deleteServers(port, params['Handles'])
        return { }

    def EnumerateVideoClipsHandler(self, params, port):
        self.app.activate()
        return { 'ClipFiles': self.app.enumerateVideoClips(port, params['GlobSpec']) }

    def ResetProtocolHandler(self, params, port):
        self.app.activate()
        self.app.stopProtocol(port, params['ProtocolHandles'])
        return { }

    def StartProtocolHandler(self, params, port):
        self.app.activate()
        self.app.startProtocol(port, params['ProtocolHandles'])
        return { }

    def StopProtocolHandler(self, params, port):
        self.app.activate()
        self.app.stopProtocol(port, params['ProtocolHandles'])
        return { }

    def ClearResultsHandler(self, params, port):
        self.app.activate()
        self.app.clearResults(port, params['ProtocolHandles'], params['AbsExecTime'])
        return { }
