#!/bin/sh
#
# Don't edit the following line for startup level
# startlevel 30
# @MessageSets : Dpg_1
# Description:
# dpgd startup script
#

#-------------------------------------
# Variable initialization
#-------------------------------------
. /etc/init.d/spirent/spirent_functions

APPNAME=/usr/spirent/bin/dpgd
FIFONAME=`get_fifo_name $APPNAME`
NUM_PORTS=`cat $HWINFO_INI | grep NPORTS_PER_GROUP | awk -F = '{ print $2}'`
#NUM_CORES=`get_num_cores`
#NUM_IO_WORKERS=`expr \( $NUM_CORES / $NUM_PORTS \) + 1`
# Removed to get DPG working on multicore boards
NUM_IO_WORKERS=1
ARGS="-n $NUM_IO_WORKERS -s $FIFONAME"
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

