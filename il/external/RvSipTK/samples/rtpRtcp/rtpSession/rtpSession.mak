#************************************************************************
#Filename   : rtpSession.mak
#Description: sample appilcation that demonstrates simple RTP session
#************************************************************************
#      Copyright (c) 2001,2002 RADVISION Inc. and RADVISION Ltd.
#************************************************************************
#NOTICE:
#This document contains information that is confidential and proprietary
#to RADVISION Inc. and RADVISION Ltd.. No part of this document may be
#reproduced in any form whatsoever without written prior approval by
#RADVISION Inc. or RADVISION Ltd..
#
#RADVISION Inc. and RADVISION Ltd. reserve the right to revise this
#publication and make changes without obligation to notify any person of
#such revisions or changes.
#************************************************************************

# Project Name - must be unique within build tree. Actual target names will
# be based on this name (for example, on Solaris, the resulting library
# will be called libNAME.a and the excutable, if any, will be NAME.
PROJECT_NAME := rtpSession

# Project Makefile - relative path and name of this file.
# Must match path and name in main makefile.
PROJECT_MAKE := samples/rtpRtcp/rtpSession/rtpSession.mak

# Project Path = relative path from main makefile to top of source
# directory tree for this project
PROJECT_PATH := samples/rtpRtcp/rtpSession

# RTP include directory
INCLUDE_DIRS += /usr/local/include /usr/local/ssl/include include/common  include/rtpRtcp/rtp include/rtpRtcp/srtp \
		appl/rtpRtcp/include/openssl appl/rtpRtcp/testapp/rtpt/commonext samples/rtpRtcp samples/rtpRtcp/rtpSession 

# Module sub-directories - relative path from this makefile
MODULES := ..

# Build executable application for this project - yes/no
BUILD_EXECUTABLE := yes

# Build shared library - yes/no
BUILD_SHARED_LIB := no

# Libraries needed for linking executable (exclude lib prefix and extension)
EXE_LIBS_NEEDED +=  rvrtp rvcommon 

# Check compilation flags of addons

# This file must be included at the end of every project makefile
include $(PROJECT_MAKE_PATH)/project.mak

