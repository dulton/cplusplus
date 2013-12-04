#
# @file
# @brief RTSP protocol unit tests
#
# Copyright (c) 2008 by Spirent Communications Inc.
# All Rights Reserved.
#
# This software is confidential and proprietary to Spirent Communications Inc.
# No part of this software may be reproduced, transmitted, disclosed or used
# in violation of the Software License Agreement without the expressed
# written consent of Spirent Communications Inc.
#

import unittest, os, socket, struct
import RelativePathSetup

from rtspproto import *


class TestProtocol(unittest.TestCase):
    """
    A test case for rtspproto classes.
    """
    def setUp(self):
        pass
    
    def tearDown(self):
        pass
    
    def testProtocol(self):
        # Test a variety of bogus inputs
        s = RTSPProtocol(RTSPRequest)
        self.failUnless(s.input('') == [])
        self.failUnlessRaises(BadRequestLine, lambda: s.input('blah blah\r\n'))
        self.failUnlessRaises(BadRequestLine, lambda: s.reset() or s.input('SETUP / RTSP\r\n'))
        self.failUnlessRaises(BadHeaderLine, lambda: s.reset() or s.input('SETUP / RTSP/1.0\r\nfoo\r\n\r\n'))

        c = RTSPProtocol(RTSPStatus)
        self.failUnless(c.input('') == [])
        self.failUnlessRaises(BadStatusLine, lambda: c.input('blah blah\r\n'))
        self.failUnlessRaises(BadStatusLine, lambda: c.reset() or c.input('RTSP/1.0 200\r\n'))
        self.failUnlessRaises(BadHeaderLine, lambda: c.reset() or c.input('RTSP/1.0 200 OK\r\nfoo\r\n\r\n'))

        # Test valid inputs
        msgs = s.reset() or s.input('SETUP / RTSP/1.0\r\nContent-Length: 8\r\n\r\nfoobar\r\n')
        self.failUnless(len(msgs) == 1)
        self.failUnless(msgs[0].method == 'SETUP')
        self.failUnless(msgs[0].uri == '/')
        self.failUnless(msgs[0].version == '1.0')
        self.failUnless(msgs[0].headers == { HEADER_CONTENT_LENGTH: '8' })
        self.failUnless(msgs[0].body == 'foobar\r\n')

        msgs = c.reset() or c.input('RTSP/1.0 200 OK\r\nContent-Length: 8\r\n\r\nfoobar\r\n')
        self.failUnless(len(msgs) == 1)
        self.failUnless(msgs[0].version == '1.0')
        self.failUnless(msgs[0].status_code == 200)
        self.failUnless(msgs[0].reason_phrase == 'OK')
        self.failUnless(msgs[0].headers == { HEADER_CONTENT_LENGTH: '8' })
        self.failUnless(msgs[0].body == 'foobar\r\n')

        # Test pipelined inputs
        msgs = s.reset() or s.input('SETUP / RTSP/1.0\r\nContent-Length: 8\r\n\r\nfoobar\r\nTEARDOWN / RTSP/1.0\r\nContent-Length: 7\r\n\r\nblatz\r\n')
        self.failUnless(len(msgs) == 2)
        self.failUnless(msgs[0].method == 'SETUP')
        self.failUnless(msgs[0].uri == '/')
        self.failUnless(msgs[0].version == '1.0')
        self.failUnless(msgs[0].headers == { HEADER_CONTENT_LENGTH: '8' })
        self.failUnless(msgs[0].body == 'foobar\r\n')
        self.failUnless(msgs[1].method == 'TEARDOWN')
        self.failUnless(msgs[1].uri == '/')
        self.failUnless(msgs[1].version == '1.0')
        self.failUnless(msgs[1].headers == { HEADER_CONTENT_LENGTH: '7' })
        self.failUnless(msgs[1].body == 'blatz\r\n')

        msgs = c.reset() or c.input('RTSP/1.0 200 OK\r\nContent-Length: 8\r\n\r\nfoobar\r\nRTSP/1.0 404 Not Found\r\nContent-Length: 7\r\n\r\nblatz\r\n')
        self.failUnless(len(msgs) == 2)
        self.failUnless(msgs[0].version == '1.0')
        self.failUnless(msgs[0].status_code == 200)
        self.failUnless(msgs[0].reason_phrase == 'OK')
        self.failUnless(msgs[0].headers == { HEADER_CONTENT_LENGTH: '8' })
        self.failUnless(msgs[0].body == 'foobar\r\n')
        self.failUnless(msgs[1].version == '1.0')
        self.failUnless(msgs[1].status_code == 404)
        self.failUnless(msgs[1].reason_phrase == 'Not Found')
        self.failUnless(msgs[1].headers == { HEADER_CONTENT_LENGTH: '7' })
        self.failUnless(msgs[1].body == 'blatz\r\n')
        
    def testTransport(self):
        # Test a variety of bogus inputs
        self.failUnlessRaises(BadTransportValue, lambda: RTSPTransport(''))
        self.failUnlessRaises(BadTransportValue, lambda: RTSPTransport('foo'))

        # Test valid inputs
        t = RTSPTransport('RTP/AVP')
        self.failUnless(t)
        self.failUnless(t.proto == 'UDP')
        self.failUnless(not len(t.params))
        
        t = RTSPTransport('RTP/AVP;multicast')
        self.failUnless(t)
        self.failUnless(t.proto == 'UDP')
        self.failUnless(len(t.params) == 1)
        self.failUnless(t.params['multicast'] == None)

        t = RTSPTransport('RTP/AVP;port=3456')
        self.failUnless(t)
        self.failUnless(t.proto == 'UDP')
        self.failUnless(len(t.params) == 1)
        self.failUnless(t.params['port'] == [3456])
        
        t = RTSPTransport('RTP/AVP/TCP;client_port=3456-3457;mode="PLAY"')
        self.failUnless(t)
        self.failUnless(t.proto == 'TCP')
        self.failUnless(len(t.params) == 2)
        self.failUnless(t.params['client_port'] == [3456, 3457])
        self.failUnless(t.params['mode'] == '"PLAY"')

    
def suite():
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(TestProtocol))
    return suite

if __name__ == '__main__':
    unittest.TextTestRunner(verbosity=2).run(suite())
