#!/bin/sh
#
# Don't edit the following line for startup level
# startlevel 30
# @MessageSets : XmppvJ_1
# Description:
# xmppd startup script
#

#-------------------------------------
# Variable initialization
#-------------------------------------
. /etc/init.d/spirent/spirent_functions

APPNAME=/usr/spirent/bin/xmppd
FIFONAME=`get_fifo_name $APPNAME`
NUM_PORTS=`cat $HWINFO_INI | grep NPORTS_PER_GROUP | awk -F = '{ print $2}'`
NUM_CORES=`get_num_cores`
NUM_IO_WORKERS=`expr \( $NUM_CORES / $NUM_PORTS \) + 1`
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
#prepare capabilities directory
#-------------------------------------

if [ ! -d "/mnt/spirent/ccpu/lib/video/clips/xmpp" ];then
	if [ -f "/mnt/spirent/ccpu/lib/video/clips/xmpp" ];then
		/bin/rm -f /mnt/spirent/ccpu/lib/video/clips/xmpp > /dev/null 2>&1
	fi
	/bin/mkdir -p /mnt/spirent/ccpu/lib/video/clips/xmpp > /dev/null 2>&1
	if [ ! -f "/mnt/spirent/ccpu/lib/video/clips/xmpp/default_stanza_format.txt" ];then
		/bin/cp /usr/spirent/lib/xmpp/default_stanza_format.txt /mnt/spirent/ccpu/lib/video/clips/xmpp/default_stanza_format.txt > /dev/null 2>&1
	fi
	
fi



#-------------------------------------
# Daemon start/stop routine
#-------------------------------------
run_start_stop $1 $APPNAME $FIFONAME $NICE

exit $EXITCODE
