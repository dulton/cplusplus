##############################################################################################
#
# Notice:
# This document contains information that is proprietary to RADVISION LTD..
# No part of this publication may be reproduced in any form whatsoever without
# written prior approval by RADVISION LTD..
#
# RADVISION LTD. reserves the right to revise this publication and make changes
# without obligation to notify any person of such revisions or changes.
#
##############################################################################################

##############################################################################################
#                                 RTPT_help.tcl
#
#   Help menu handling.
#   This file holds all the GUI procedures for the Help menu. This includes the script help
#   window and the about window.
#
#
#       Written by                        Version & Date                        Change
#      ------------                       ---------------                      --------
#  Tsahi Levent-Levi                        5-May-2005
#
##############################################################################################



# Set the server address we want to connect to.
proc bcast:setServer {} {
    global app
    set base .bcast

    toplevel $base -class Toplevel
    wm focusmodel $base passive
    wm geometry $base 239x94+222+269; update
    wm overrideredirect $base 0
    wm resizable $base 0 0
    wm deiconify $base
    wm title $base "Broadcast Server"
    label $base.addrLab -padx 1 -text address
    entry $base.addr -background white -textvariable app(bcastaddr)

    grid columnconf $base 1 -weight 1
    grid $base.addrLab -in $base -column 0 -row 0 -columnspan 1 -rowspan 1 -padx 5
    grid $base.addr -in $base -column 1 -row 0 -columnspan 1 -rowspan 1 -padx 5 -sticky ew
}


# Connect to the broadcaster.
# serverAddr   - Server address to connect to
proc bcast:connect {serverAddr} {
    global tmp app

    catch {
        set remoteAddress [string map { ":" " " } $serverAddr]

        # open a socket to the remote address
        set sock [socket [lindex $remoteAddress 0] [lindex $remoteAddress 1]]
        fconfigure $sock -buffering line
        fconfigure $sock -blocking 0

        fileevent $sock readable bcast:incomingData

        test:Log "Connected from broadcaster"

        set tmp(bcast,sock) $sock
    }
}


# Disconnect from the broadcaster.
proc bcast:disconnect {} {
    global tmp

    close $tmp(bcast,sock)
    set tmp(bcast,sock) ""

    test:Log "Got disconnected from broadcaster"
}


# Deal with incoming data from the broadcaster.
proc bcast:incomingData {} {
    global tmp app
    if {[eof $tmp(bcast,sock)]} {
        # got disconnected from the server
        bcast:disconnect
        return
    }

    set str [gets $tmp(bcast,sock)]
    if {$str != ""} {
        SessionSRTP.MessageDispatcher [lindex $str 0] "[lrange $str 1 end]"
#        test:Log "RCV: $str"
    }
}


# Send a message to the broadcaster
proc bcast:send {msg} {
    global tmp
    puts $tmp(bcast,sock) $msg

#   tk_messageBox -message "tmp(bcast,sock) = $tmp(bcast,sock)"

}

