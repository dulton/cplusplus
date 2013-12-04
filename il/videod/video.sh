#!/bin/sh
#
# Don't edit the following line for startup level
# startlevel 30
# @MessageSets : Video_1
# Description:
# videod startup script
#

#-------------------------------------
# Variable initialization
#-------------------------------------
. /etc/init.d/spirent/spirent_functions

APPNAME=/usr/spirent/bin/videod
FIFONAME=`get_fifo_name $APPNAME`
ARGS="-s $FIFONAME"
NICE=0
EXITCODE=0

#Use the system library files rather than staging files for STC Anywhere.
if [ -e /mnt/spirent/chassis/conf/stca.ini ]; then
    IS64BIT=`uname -m | grep 64`
    if [ $IS64BIT ]; then
        export LD_LIBRARY_PATH=/lib64:$LD_LIBRARY_PATH
    else
        export LD_LIBRARY_PATH=/lib:$LD_LIBRARY_PATH
    fi
fi

#
# ccpu instance given, must be Grande sfpga
#
if [ "$2" != "" ]; then
   if [ -e "/mnt/spirent/ccpu$2/conf/mps_port" ]; then
      MPS_PORT=`cat /mnt/spirent/ccpu$2/conf/mps_port`
      ARGS="$ARGS -p $MPS_PORT"
      NICE=19
   fi
fi

# This hangs on x86 if you don't leave a gap between daemons
sleep 2

#-------------------------------------
# Daemon start/stop routine
#-------------------------------------
run_start_stop $1 $APPNAME $FIFONAME $NICE

exit $EXITCODE
