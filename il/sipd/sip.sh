#!/bin/sh
#
# Don't edit the following line for startup level
# startlevel 30
# @MessageSets : Sip_1
# Description:
# sipd startup script
#

#-------------------------------------
# Variable initialization
#-------------------------------------
. /etc/init.d/spirent/spirent_functions

APPNAME=/usr/spirent/bin/sipd
FIFONAME=`get_fifo_name $APPNAME`
ARGS="-s $FIFONAME"
NICE=0
EXITCODE=0

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


#-------------------------------------
# Daemon start/stop routine
#-------------------------------------
run_start_stop $1 $APPNAME $FIFONAME $NICE

exit $EXITCODE
