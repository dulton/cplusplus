#
# @file
# @brief Temporary file cache class
#
# Copyright (c) 2008 by Spirent Communications Inc.
# All Rights Reserved.
#
# This software is confidential and proprietary to Spirent Communications Inc.
# No part of this software may be reproduced, transmitted, disclosed or used
# in violation of the Software License Agreement without the expressed
# written consent of Spirent Communications Inc.
#

import os, shutil


class TmpFileCache(object):
    def __init__(self, path, policy=None):
        self.path = os.path.normpath(path)
        if os.access(self.path, os.F_OK):
            shutil.rmtree(self.path, True)
            try:
                os.rmdir(self.path)
            except OSError:
                pass
        os.mkdir(self.path)
        self.files = {}
        self.policy = policy

    def __del__(self):
        map(self._remove_file, self.files.keys())
        shutil.rmtree(self.path, True)
        try:
            os.rmdir(self.path)
        except OSError:
            pass

    def insert(self, src):
        file = os.path.basename(src)
        if file not in self.files:
            self.policy and self.policy.insert(src, file)
            shutil.copy(src, self.path)
            self.files[file] = 1
        else:
            self.files[file] += 1
            
    def remove(self, files):
        try:
            map(self._remove_file, files)
        except TypeError:
            self._remove_file(files)

    def _remove_file(self, file):
        try:
            if self.files[file] == 1:
                self.policy and self.policy.remove(file)
                try:
                    os.unlink(os.path.join(self.path, file))
                except OSError:
                    pass
                del self.files[file]
            else:
                self.files[file] -= 1
        except KeyError:
            pass


class CacheSizeLimitPolicy(object):
    def __init__(self, limit):
        self.limit = limit
        self.usage = 0
        self.sizes = {}

    def insert(self, src, file):
        statinfo = os.stat(src)
        free = self.limit - self.usage
        if statinfo.st_size <= free:
            self.usage += statinfo.st_size
            self.sizes[file] = statinfo.st_size
            return True
        else:
            raise RuntimeError, "Temporary file cache cannot accept '%s': %u bytes required, %u bytes free" % (file, statinfo.st_size, free)

    def remove(self, file):
        self.usage -= self.sizes[file]
        del self.sizes[file]
