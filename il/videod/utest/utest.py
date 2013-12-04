#
# @file
# @brief videod unit test package
#
# Copyright (c) 2008-2013 by Spirent Communications Inc.
# All Rights Reserved.
#
# This software is confidential and proprietary to Spirent Communications Inc.
# No part of this software may be reproduced, transmitted, disclosed or used
# in violation of the Software License Agreement without the expressed
# written consent of Spirent Communications Inc.
#

from optparse import OptionParser
import unittest
import sys

import TestProtocol

parser = OptionParser()
parser.add_option('-v', '--verbose', action='store_const', const=2, dest='verbosity', default=1)
(options, args) = parser.parse_args()

PKG_SUITE = unittest.TestSuite()
PKG_SUITE.addTest(TestProtocol.suite())

result = unittest.TextTestRunner(verbosity=options.verbosity).run(PKG_SUITE)
sys.exit([-1, 0][result.wasSuccessful()])
