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
#                                 RTPT_test.tcl
#
# This file contains the main window GUI along with some of its procedures, which are not
# related to any other specific window (such as calls, tools, etc).
#
#
#
#       Written by                        Version & Date                        Change
#      ------------                       ---------------                      --------
#  Oren Libis & Ran Arad                    04-Mar-2000
#
##############################################################################################



##############################################################################################
#
#   MAIN WINDOW operations
#
##############################################################################################


# test:create
# This procedure creates the main window of the test application
# input  : none
# output : none
# return : none
proc test:create {} {
    global tmp app

    ###################
    # CREATING WIDGETS
    ###################
    toplevel .test -class Toplevel -menu .test.main
    wm iconify .test
    wm focusmodel .test passive
    wm geometry .test $app(test,size)
    wm overrideredirect .test 0
    wm resizable .test 1 1
    wm title .test $tmp(version)
    wm protocol .test WM_DELETE_WINDOW {
        set app(test,size) [winfo geometry .test]
        test.Quit
    }

    # Create the menu for this window
    test:createMenu

    # Tool buttons
    frame .test.line1 -relief sunken -border 1 -height 2
    frame .test.tools
    button .test.tools.log -relief flat -border 1 -command {Window toggle .log} -image LogBut -state disabled -tooltip "Stack Log"
    button .test.tools.config -relief flat -border 1 -command {config:open} -image ConfigBut -tooltip "Open Configuration File"
    button .test.tools.logFile -relief flat -border 1 -command {logfile:open} -image LogFileBut -tooltip "Open Log File"
    button .test.tools.status -relief flat -border 1 -command {Window toggle .status} -image StatusBut -tooltip "Resource Status (Ctrl-s)"
    button .test.tools.raise -relief flat -border 1 -command {focus .dummy} -image RaiseBut -tooltip "Raise Windows"
    label  .test.tools.marker1 -relief sunken -border 1 -padx 0
    button .test.tools.execute -relief flat -border 1 -command {script:load} -image ExecuteBut -tooltip "Execute an Advanced Script"
    button .test.tools.stop -relief flat -border 1 -command {script:stop} -image StopBut -tooltip "Stop executing a running script"
    set tools {{log config status raise logFile} {execute stop}}

    # top bar
    image create photo topbar -format gif -data $tmp(topbarFade)
    label .test.tools.topbar -image topbar -borderwidth 0 -anchor e
    frame .test.line2 -relief sunken -border 1 -height 2

    # Sessions listbox
    proc callYviewScrl {args} {
        eval ".test.sess.list yview $args"
    }

    frame .test.sess -borderwidth 2 -relief groove
    listbox .test.sess.list -background White -selectmode single -exportselection 0 -width 1 -height 3 \
        -yscrollcommand {.test.sess.scrl set}
    scrollbar .test.sess.scrl -command {callYviewScrl}

    # Addresses listbox
    frame .test.addrs -borderwidth 2 -relief groove
    address:createAddressesInfo .test.addrs 1

    # Tabs Section
    set tmp(test,otherTabs) { 1 2 3 4 }

    set tabs [
        notebook:create test .test {
            { "Basic"   test:basicTab  }
            { "Set"     test:setTab   }
            { "SRTP"    test:srtpTab   }
            { "Conf"    test:confTab   }
        }
    ]

    # Messages
    frame .test.msg -borderwidth 2 -relief groove
    listbox .test.msg.list -selectmode single -exportselection 0 -height 1 -background White \
        -yscrollcommand {.test.msg.yscrl set} -xscrollcommand {.test.msg.xscrl set}
    scrollbar .test.msg.xscrl -orient horizontal -command {.test.msg.list xview}
    scrollbar .test.msg.yscrl -command {.test.msg.list yview}
    button .test.msg.clear -borderwidth 2 -command {.test.msg.list delete 0 end} \
        -highlightthickness 0 -padx 0 -pady 0 -image bmpClear -tooltip "Clear message box (Ctrl-m)"

    # Status bar
    frame .test.status -borderwidth 0 -relief flat
    label .test.status.sess -borderwidth 1 -relief sunken -anchor w -tooltip "Number of current connected sessions, Number of total sessionss connected"
    label .test.status.stack -borderwidth 1 -relief sunken -width 10 -tooltip "Stack's current execution status"
    label .test.status.mode -borderwidth 1 -relief sunken -width 7 -tooltip "Mode of the application: Normal mode, or Script mode"

    frame .test.status.log -borderwidth 1 -relief sunken -width 16
    label .test.status.log.data -borderwidth 0 -relief flat -width 15 -tooltip "Errors found while running the stack"
    button .test.status.log.reset -borderwidth 2 -command Log:Reset \
        -highlightthickness 0 -padx 0 -pady 0 -image bmpClear -tooltip "Reset error counters"

    label .test.status.timer -borderwidth 1 -relief sunken -padx 3 -tooltip "Time passed since beginning of execution (in minutes)"

    # Manual frame
    frame .test.manual -relief groove -borderwidth 0

    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf .test 0 -weight 1 -minsize 30
    grid columnconf .test 1 -weight 8
    grid columnconf .test 2 -weight 0 -minsize 300
    grid rowconf .test 3 -weight 0 -minsize 6
    grid rowconf .test 4 -weight 1 -minsize 210
    grid rowconf .test 5 -weight 0
    grid rowconf .test 6 -weight 0
    grid rowconf .test 7 -weight 0 -minsize 5
    grid rowconf .test 8 -weight 100

    # Displaying the tool buttons
    grid .test.line1 -in .test -column 0 -row 0 -columnspan 4 -sticky new -pady 1
    grid .test.tools -in .test -column 0 -row 0 -columnspan 4 -sticky ew -pady 4
    set first 1
    foreach toolGroup $tools {
        foreach tool $toolGroup {
#            pack .test.tools.$tool -in .test.tools -ipadx 2 -side left
        }
        if {$first == 1} {
#            pack .test.tools.marker1 -in .test.tools -pady 2 -padx 1 -side left -fill y
            set first 0
        }
    }

    # top bar
    pack .test.tools.topbar -in .test.tools -side right
    grid .test.line2 -in .test -column 0 -row 0 -columnspan 4 -sticky sew -pady 1

    # Sessions
    grid .test.sess -in .test -column 0 -row 4 -sticky nesw -padx 2
    grid columnconf .test.sess 0 -weight 1
    grid rowconf .test.sess 0 -weight 0 -minsize 3
    grid rowconf .test.sess 1 -weight 1
    grid .test.sess.list  -in .test.sess -column 0 -row 1 -sticky nesw -pady 2
    grid .test.sess.scrl  -in .test.sess -column 1 -row 1 -sticky ns -pady 2

    # Addresses
    grid .test.addrs -in .test -column 1 -row 4 -sticky nesw -padx 2

    # Tabs Section
    grid $tabs -in .test -column 2 -row 4 -rowspan 3 -sticky nesw -pady 2

    # Messages
    grid .test.msg -in .test -column 0 -row 8 -columnspan 4 -sticky nesw -pady 4 -padx 2
    grid columnconf .test.msg 0 -weight 1
    grid rowconf .test.msg 0 -weight 0 -minsize 5
    grid rowconf .test.msg 1 -weight 1
    grid .test.msg.list -in .test.msg -column 0 -row 1 -sticky nesw -padx 1 -pady 1
    grid .test.msg.xscrl -in .test.msg -column 0 -row 2 -sticky we
    grid .test.msg.yscrl -in .test.msg -column 1 -row 1 -sticky ns
    grid .test.msg.clear -in .test.msg -column 1 -row 2 -sticky news

    # Status bar
    grid .test.status -in .test -column 0 -row 9 -columnspan 4 -sticky ew -padx 1
    grid columnconf .test.status 0 -weight 1
    grid .test.status.sess -in .test.status -column 0 -row 0 -sticky ew -padx 1
    grid .test.status.stack -in .test.status -column 4 -row 0 -sticky ew -padx 1
    grid .test.status.mode -in .test.status -column 5 -row 0 -sticky ew -padx 1

    grid .test.status.log -in .test.status -column 6 -row 0 -padx 1
    grid .test.status.log.data -in .test.status.log -column 0 -row 0 -sticky ew
    grid .test.status.log.reset -in .test.status.log -column 1 -row 0
    grid .test.status.timer -in .test.status -column 7 -row 0 -sticky ew -padx 1

    ###########
    # BINDINGS
    ###########
    bind .test <Control-Key-m> {.test.msg.clear invoke}
    bind .test <Control-Key-s> {.test.tools.status invoke}
    bind .test <Control-Key-l> {.test.tools.logFile invoke}
    bind .test <Control-Key-R> {LogFile:Reset}
    bind .test.sess.list <Double-Button-1> {call:doubleclickedCall}
    bind .test.msg.list <<ListboxSelect>> {.test.msg.list selection clear 0 end}

    foreach toolGroup $tools {
        foreach tool $toolGroup {
            bindtags .test.tools.$tool "[bindtags .test.tools.$tool] toolbar"
        }
    }

    ########
    # OTHER
    ########
    bind .test.sess.list <<ListboxSelect>> {+
        set item [selectedItem .test.sess.list]
        if {$item != ""} {Address.DisplayAddressList $item}
        srtpSetBut $item
    }
    srtpSetBut ""
    test:SetSessions 0 0
    test:updateTimer
#    trace variable tmp(script,running) w test:refreshMenu
#    set tmp(script,running) 0

    placeHeader .test.sess "Sessions"
    placeHeader .test.msg "Messages"
    after idle {wm geometry .test $app(test,size)}
}

proc test:basicTab {base tabButton} {
    global tmp app
    set tmp(basicTab) $base

    # Session buttons

    frame $base.open -borderwidth 0
    button $base.open.but -text "Open" -width 6 -pady 1 \
        -command {global tmp; Session.Open $tmp(sess,localAddr)} -tooltip "Open a session"
    histEnt:create $base.open $base.open.but sess,localAddr "Local address for the session (Return to open)"

    frame $base.add -borderwidth 0
    button $base.add.but -text "Add" -width 6 -pady 1 \
        -command {global tmp; Session.AddRemoteAddr [selectedItem .test.sess.list] $tmp(sess,remoteAddr)} -tooltip "Add a remote address to the session"
    histEnt:create $base.add $base.add.but sess,remoteAddr "Remote address for the session (Return to add)"

    frame $base.mcast -borderwidth 0
    button $base.mcast.but -text "MCast" -width 6 -pady 1 \
        -command {global tmp; Session.ListenMulticast [selectedItem .test.sess.list] $tmp(sess,mcastAddr)} -tooltip "Set a multicast address to the session"
    histEnt:create $base.mcast $base.mcast.but sess,mcastAddr "Multicast address for the session (Return to set)"

    button $base.remove -text "Remove" -width 6 -highlightthickness 0 -padx 0 -pady 0 \
        -command {Session.RemoveRemoteAddr [selectedItem .test.sess.list] [.test.addrs.outList curselection]} -tooltip "Remove the selected remote address"
    button $base.remAll -text "Remove All" -width 6 -highlightthickness 0 -padx 0 -pady 0 \
        -command {Session.RemoveAllRemoteAddrs [selectedItem .test.sess.list]} -tooltip "Remove all remote addresses"
    button $base.close -text "Close" -width 6 -highlightthickness 0 -padx 0 -pady 0 -height 0\
        -command {Session.Close [selectedItem .test.sess.list]} -tooltip "Close the selected session"

    button $base.read -text "Read" -width 6 -highlightthickness 0 -padx 0 -pady 0 \
        -command {global tmp; Session.StartRead [selectedItem .test.sess.list] $tmp(sess,readBlocked)} -tooltip "Add a remote address to the session"
    checkbutton $base.blocked -text "Blocked" -variable tmp(sess,readBlocked) -tooltip "Set blocked read"

    button $base.sendSR -text "Send SR" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {Session.SendSR [selectedItem .test.sess.list]} -tooltip "Send SR on the selected session"
    button $base.sendRR -text "Send RR" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {Session.SendRR [selectedItem .test.sess.list]} -tooltip "Send RR on the selected session"
    button $base.sendAPP -text "Send APP" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {Session.SendAPP [selectedItem .test.sess.list]} -tooltip "Send APP on the selected session"
    button $base.sendBye -text "Send Bye" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {Session.SendBye [selectedItem .test.sess.list]  $tmp(sess,byeReason)} \
        -tooltip "Send Bye on the selected session (with reason)"
    button $base.shutdown -text "Shutdown" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {Session.Shutdown [selectedItem .test.sess.list]  $tmp(sess,byeReason)} \
        -tooltip "Shutdown the selected session (with reason)"
    entry $base.reasonEnt -textvariable tmp(sess,byeReason) -width 12 -tooltip "Bye/Shutdown Reason"

    frame $base.force -borderwidth 0
    checkbutton $base.force.ssrcCb -text "Force SSRC" -variable app(force,bSsrc) -tooltip "Force use of given SSRC"
    checkbutton $base.force.psnCb -text "Force PSN" -variable app(force,bPsn) -tooltip "Force use of given Packet Sequence Number"
    entry $base.force.ssrcEnt -textvariable app(force,ssrc) -tooltip "SSRC"
    entry $base.force.psnEnt -textvariable app(force,psn) -tooltip "Packet Sequence Number"

    ########
    # GRID #
    ########

    grid rowconf $base 0 -weight 0
    grid rowconf $base 1 -weight 0
    grid rowconf $base 2 -weight 0
    grid rowconf $base 3 -weight 0
    grid rowconf $base 4 -weight 0
    grid rowconf $base 5 -weight 0
    grid rowconf $base 6 -weight 0
    grid rowconf $base 7 -weight 1
    grid columnconf $base 0 -weight 1
    grid columnconf $base 1 -weight 1
    grid columnconf $base 2 -weight 1

    # Session buttons
    grid $base.open -in $base -column 0 -row 0 -columnspan 3 -sticky nesw -padx 0
    grid rowconf $base.open 0 -weight 0 -minsize 0
    grid columnconf $base.open 1 -weight 1
    grid $base.open.but -in $base.open -column 0 -row 1 -padx 2
    grid $base.open.histEnt -in $base.open -column 1 -row 1 -sticky ew -padx 3 -ipady 2 -pady 3

    grid $base.add -in $base -column 0 -row 1 -columnspan 3 -sticky nesw -padx 0
    grid rowconf $base.add 0 -weight 0 -minsize 0
    grid columnconf $base.add 1 -weight 1
    grid $base.add.but -in $base.add -column 0 -row 1 -padx 2
    grid $base.add.histEnt -in $base.add -column 1 -row 1 -sticky ew -padx 3 -ipady 2 -pady 3

    grid $base.mcast -in $base -column 0 -row 2 -columnspan 3 -sticky nesw -padx 0
    grid rowconf $base.mcast 0 -weight 0 -minsize 0
    grid columnconf $base.mcast 1 -weight 1
    grid $base.mcast.but -in $base.mcast -column 0 -row 1 -padx 2
    grid $base.mcast.histEnt -in $base.mcast -column 1 -row 1 -sticky ew -padx 3 -ipady 2 -pady 3

    grid $base.remove -in $base -column 0 -row 3 -sticky nesw -padx 2 -pady 2
    grid $base.remAll -in $base -column 1 -row 3 -sticky nesw -padx 2 -pady 2
    grid $base.close -in $base -column 2 -row 3 -sticky nesw -padx 2 -pady 2

    grid $base.read -in $base -column 0 -row 4 -sticky nesw -padx 2 -pady 2
    grid $base.blocked -in $base -column 1 -columnspan 2 -row 4 -sticky w -padx 2 -pady 2

    grid $base.sendSR -in $base -column 0 -row 5 -sticky nesw -padx 2 -pady 2
    grid $base.sendRR -in $base -column 1 -row 5 -sticky nesw -padx 2 -pady 2
    grid $base.sendAPP -in $base -column 2 -row 5 -sticky nesw -padx 2 -pady 2
    grid $base.sendBye -in $base -column 0 -row 6 -sticky nesw -padx 2 -pady 2
    grid $base.shutdown -in $base -column 1 -row 6 -sticky nesw -padx 2 -pady 2
    grid $base.reasonEnt -in $base -column 2 -row 6 -sticky nesw -padx 2 -pady 2

    grid $base.force -in $base -column 0 -row 7 -columnspan 2 -sticky nesw -padx 0
    grid rowconf $base.force 0 -weight 0 -minsize 0
    grid columnconf $base.force 2 -weight 1
    grid $base.force.ssrcCb -in $base.force -column 0 -row 0 -padx 2 -sticky w
    grid $base.force.psnCb -in $base.force -column 0 -row 1 -padx 2 -sticky w
    grid $base.force.ssrcEnt -in $base.force -column 1 -row 0 -padx 2 -sticky w
    grid $base.force.psnEnt -in $base.force -column 1 -row 1 -padx 2 -sticky w

    ########
    # BIND #
    ########

    bind $base.open.histEnt.name <Return> "$base.open.but invoke"
    bind $base.add.histEnt.name <Return> "$base.add.but invoke"

    bind .test.sess.list <<ListboxSelect>> "+
            sess:SetButtonsState $base"

    sess:SetButtonsState $base
}


proc test:setTab {base tabButton} {
    global tmp app
    set tmp(setTab) $base

    ###################
    # CREATING WIDGETS
    ###################

    button $base.payload -text "Set Payload" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {Session.SetPayload [selectedItem .test.sess.list] $tmp(sess,payload)} \
        -tooltip "Set the selected session's payload"
    entry $base.payloadEnt -textvariable tmp(sess,payload) -width 6 -tooltip "Payload"
    button $base.bw -text "Set Bandwidth" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {Session.SetBandwidth [selectedItem .test.sess.list] $tmp(sess,bw)} \
        -tooltip "Set the selected session's bandwidth"
    entry $base.bwEnt -textvariable tmp(sess,bw) -width 6 -tooltip "Bandwidth"

    button $base.ttl -text "Set TTL" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {Session.SetMulticastTtl [selectedItem .test.sess.list] $tmp(sess,rtpTtl) $tmp(sess,rtcpTtl)} \
        -tooltip "Set the selected session's multicast TTL"
    entry $base.rtpTtlEnt -textvariable tmp(sess,rtpTtl) -width 6 -tooltip "RTP TTL"
    entry $base.rtcpTtlEnt -textvariable tmp(sess,rtcpTtl) -width 6 -tooltip "RTCP TTL"

    button $base.tos -text "Set TOS" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {Session.SetTos [selectedItem .test.sess.list] $tmp(sess,rtpTos) $tmp(sess,rtcpTos)} \
        -tooltip "Set the selected session's Type Of Service"
    entry $base.rtpTosEnt -textvariable tmp(sess,rtpTos) -width 6 -tooltip "RTP TOS"
    entry $base.rtcpTosEnt -textvariable tmp(sess,rtcpTos) -width 6 -tooltip "RTCP TOS"

    button $base.enc -text "Set Encryption" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {Session.SetEncryption [selectedItem .test.sess.list] $tmp(sess,encMode) $tmp(sess,eKey) $tmp(sess,dKey)} \
        -tooltip "Set the selected session's encryption type and keys"
    menubutton $base.encType -borderwidth 1 -indicatoron 1  -width 1 -tooltip "Select encoding type" \
            -menu $base.encType.01 -padx 0 -pady 0 -relief raised -text "DES" -textvariable tmp(sess,encMode)
    menu $base.encType.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach val { "DES" "3DES" } lab { "DES Encryption" "Triple DES Encryption" } {
        $base.encType.01 add radiobutton -indicatoron 0 -label $lab -value $val -variable tmp(sess,encMode)
    }
    entry $base.eKeyEnt -textvariable tmp(sess,eKey) -width 6 -tooltip "Encryption Key"
    entry $base.dKeyEnt -textvariable tmp(sess,dKey) -width 6 -tooltip "Decription Key"

    ###################
    # SETTING GEOMETRY
    ###################

    grid rowconf $base 0 -weight 0
    grid rowconf $base 1 -weight 0
    grid rowconf $base 2 -weight 0
    grid rowconf $base 3 -weight 0 -minsize 17
    grid rowconf $base 4 -weight 0
    grid rowconf $base 5 -weight 0
    grid rowconf $base 6 -weight 0
    grid rowconf $base 7 -weight 0
    grid rowconf $base 8 -weight 0
    grid rowconf $base 9 -weight 1
    grid columnconf $base 0 -weight 0
    grid columnconf $base 1 -weight 0
    grid columnconf $base 2 -weight 0
    grid columnconf $base 3 -weight 0
    grid columnconf $base 4 -weight 0
    grid columnconf $base 5 -weight 0
    grid columnconf $base 6 -weight 0
    grid columnconf $base 7 -weight 0
    grid columnconf $base 8 -weight 0
    grid columnconf $base 9 -weight 1

    grid $base.payload -in $base -column 0 -columnspan 2 -row 1 -sticky nesw -padx 2 -pady 2
    grid $base.payloadEnt -in $base -column 2 -ipady 1 -row 1 -sticky w -padx 2 -pady 2

    grid $base.bw -in $base -column 0 -columnspan 2 -row 2 -sticky nesw -padx 2 -pady 2
    grid $base.bwEnt -in $base -column 2 -ipady 1 -row 2 -sticky w -padx 2 -pady 2

    grid $base.ttl -in $base -column 0 -columnspan 2 -row 3 -sticky nesw -padx 2 -pady 2
    grid $base.rtpTtlEnt -in $base -column 2 -ipady 1 -row 3 -sticky w -padx 2 -pady 2
    grid $base.rtcpTtlEnt -in $base -column 3 -ipady 1 -row 3 -sticky w -padx 2 -pady 2

    grid $base.tos -in $base -column 0 -columnspan 2 -row 4 -sticky nesw -padx 2 -pady 2
    grid $base.rtpTosEnt -in $base -column 2 -ipady 1 -row 4 -sticky w -padx 2 -pady 2
    grid $base.rtcpTosEnt -in $base -column 3 -ipady 1 -row 4 -sticky w -padx 2 -pady 2

    grid $base.enc -in $base -column 0 -columnspan 2 -row 5 -sticky nesw -padx 2 -pady 2
    grid $base.encType -in $base -column 2 -columnspan 2 -ipady 2 -row 5 -sticky ew -padx 2 -pady 2
    grid $base.eKeyEnt -in $base -column 4 -ipady 2 -row 5 -sticky w -padx 2 -pady 2
    grid $base.dKeyEnt -in $base -column 5 -ipady 2 -row 5 -sticky w -padx 2 -pady 2

    ########
    # OTHER
    ########
}

proc srtpSetBut {selected} {
    global tmp

    set butList "$tmp(srtpTab).buttons.construct $tmp(srtpTab).buttons.constActiv"
    set invButList "$tmp(srtpTab).buttons.activate $tmp(srtpTab).buttons.deactivate $tmp(srtpTab).buttons.notify
        $tmp(srtpTab).mk.add $tmp(srtpTab).rs.add $tmp(srtpTab).cirp.sendIndexes $tmp(confTab).mk.set $tmp(confTab).kd.set
        $tmp(confTab).pref.set $tmp(confTab).encr.set $tmp(confTab).auth.set $tmp(confTab).ks.set $tmp(confTab).rl.set $tmp(confTab).his.set
        $tmp(confTab).keyPool.set $tmp(confTab).streamPool.set $tmp(confTab).contextPool.set $tmp(confTab).keyHash.set
        $tmp(confTab).sourceHash.set $tmp(confTab).destHash.set"

    if {$selected == ""} {
        foreach but $butList {$but configure -state disabled}
        foreach but $invButList {$but configure -state disabled}
    } elseif {[string index $selected end] == "S"} {
        foreach but $butList {$but configure -state disabled}
        foreach but $invButList {$but configure -state normal}
    } else {
        foreach but $invButList {$but configure -state disabled}
        foreach but $butList {$but configure -state normal}
    }
    srtpVerifySelection $tmp(srtpTab).mk.list
    srtpVerifySelection $tmp(srtpTab).rs.list
}

proc srtpVerifySelection {listName} {
    global tmp

    set sessionId [selectedItem .test.sess.list]

    foreach selIndex [$listName curselection] {
        set entryId [lindex [$listName get $selIndex] 0]
        if {[string compare $sessionId $entryId] != 0} {
            $listName selection clear $selIndex
        }
    }
    if {[string equal $listName $tmp(srtpTab).mk.list]} {
        if {[$listName curselection] == ""} {
            $tmp(srtpTab).dc.setdst configure -state disabled
        } else {
            $tmp(srtpTab).dc.setdst configure -state normal
        }
    }
    if {[string equal $listName $tmp(srtpTab).rs.list]} {
        if {[$listName curselection] == ""} {
            $tmp(srtpTab).dc.setsrc configure -state disabled
        } else {
            if {[$tmp(srtpTab).mk.list curselection] == ""} {
                $tmp(srtpTab).dc.setsrc configure -state disabled
            } else {
                $tmp(srtpTab).dc.setsrc configure -state normal
            }
        }
    }
}

proc srtpCleanSession {sessionId} {
    global tmp

    if {$sessionId == ""} {
        set sessionId [selectedItem .test.sess.list]
    }

    for {set index [$tmp(srtpTab).mk.list size]} {$index >= 0} {incr index -1} {
        set entryId [lindex [$tmp(srtpTab).mk.list get $index] 0]
        if {[string compare $sessionId $entryId] == 0} {
            $tmp(srtpTab).mk.list delete $index
        }
    }
    for {set index [$tmp(srtpTab).rs.list size]} {$index >= 0} {incr index -1} {
        set entryId [lindex [$tmp(srtpTab).rs.list get $index] 0]
        if {[string compare $sessionId $entryId] == 0} {
            $tmp(srtpTab).rs.list delete $index
        }
    }
}

proc test:srtpTab {base tabButton} {
    global tmp app
    set tmp(srtpTab) $base

    ###################
    # CREATING WIDGETS
    ###################

    frame $base.buttons -borderwidth 0
    button $base.buttons.construct -text "Construct" -tooltip "Construct SRTP" -width 10 \
        -command {SessionSRTP.Construct [selectedItem .test.sess.list]}
    button $base.buttons.activate -text "Activate" -tooltip "Activate SRTP" -width 10 \
        -command {SessionSRTP.Init [selectedItem .test.sess.list]}
    button $base.buttons.constActiv -text "+" -tooltip "Activate SRTP" -width 2 \
        -command "$base.buttons.construct invoke; $base.buttons.activate invoke"
    button $base.buttons.deactivate -text "Deactivate" -tooltip "Deactivate SRTP" -width 10 \
        -command {
            srtpCleanSession ""
            SessionSRTP.Destruct [selectedItem .test.sess.list]
        }
    button $base.buttons.notify -text "Notify" -tooltip "Send local session parameters to the remote pary" -width 10 \
        -command {SessionSRTP.Notify [selectedItem .test.sess.list]}

    # Master Key
    #############
    frame $base.mk -borderwidth 0
    entry $base.mk.idn -borderwidth 1 -width 1 -textvariable tmp(srtp,mkidn) -tooltip "Master Key Identifier"
    entry $base.mk.ey -borderwidth 1 -width 1 -textvariable tmp(srtp,mkey) -tooltip "Master Key"
    entry $base.mk.salt -borderwidth 1 -width 1 -textvariable tmp(srtp,mksalt) -tooltip "Master Key Salt"
    button $base.mk.add -highlightthickness 0 -padx 0 -pady 0 -tooltip "Add Master Key" -image sunkDown \
        -command {
            set sessionId [selectedItem .test.sess.list]
            SessionSRTP.AddMK $sessionId $tmp(srtp,mkidn) $tmp(srtp,mkey) $tmp(srtp,mksalt)
            $tmp(srtpTab).mk.list insert end "{$sessionId} {$tmp(srtp,mkidn)}"
        }
    button $base.mk.del -highlightthickness 0 -padx 0 -pady 0 -tooltip "Delete selected Master Key" -image sunkSlash \
        -command {
            set delAliases [$tmp(srtpTab).mk.list curselection]
            set size [llength $delAliases]
            for {set index [incr size -1]} {$index >= 0} {incr index -1} {
                set entry [$tmp(srtpTab).mk.list get [lindex $delAliases $index]]
                SessionSRTP.DelMK [lindex $entry 0] [lindex $entry 1]
                $tmp(srtpTab).mk.list delete [lindex $delAliases $index]
            }
        }
    button $base.mk.clr -highlightthickness 0 -padx 0 -pady 0 -tooltip "Clear Master Keys for selected session" -image sunkX \
        -command {
            set sessionId [selectedItem .test.sess.list]
            SessionSRTP.DelAllMK $sessionId
            for {set index [$tmp(srtpTab).mk.list size]} {$index >= 0} {incr index -1} {
                set entryId [lindex [$tmp(srtpTab).mk.list get $index] 0]
                if {[string compare $sessionId $entryId] == 0} {
                    $tmp(srtpTab).mk.list delete $index
                }
            }
        }
    scrollbar $base.mk.scrl -command "$base.mk.list yview"
    listbox $base.mk.list -borderwidth 1 -background White -yscrollcommand "$base.mk.scrl set" \
        -selectmode multiple -height 1 -width 1

    # Remote Source
    ################
    frame $base.rs -borderwidth 0
    menubutton $base.rs.type -borderwidth 1 -indicatoron 1  -width 1 -tooltip "Remote Source Type" \
            -menu $base.rs.type.01 -padx 0 -pady 0 -relief raised -text "both" -textvariable tmp(srtp,rstype)
    menu $base.rs.type.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach value { "both" "rtp" "rtcp" } {
        $base.rs.type.01 add radiobutton -indicatoron 0 -label $value -value $value -variable tmp(srtp,rstype)
    }
    entry $base.rs.client -borderwidth 1 -width 1 -textvariable tmp(srtp,rsclient) -tooltip "Remote Source Client"
    entry $base.rs.sess -borderwidth 1 -width 1 -textvariable tmp(srtp,rssess) -tooltip "Remote Source Session"
    button $base.rs.add -highlightthickness 0 -padx 0 -pady 0 -tooltip "Add Remote Source" -image sunkDown \
        -command {
            set sessionId [selectedItem .test.sess.list]
            SessionSRTP.AddRemoteSrc $sessionId $tmp(srtp,rstype) $tmp(srtp,rsclient) $tmp(srtp,rssess)
            $tmp(srtpTab).rs.list insert end "{$sessionId} {$tmp(srtp,rstype) $tmp(srtp,rsclient) $tmp(srtp,rssess)}"
        }
    button $base.rs.del -highlightthickness 0 -padx 0 -pady 0 -tooltip "Delete selected Remote Source" -image sunkSlash \
        -command {
            set delAliases [$tmp(srtpTab).rs.list curselection]
            set size [llength $delAliases]
            for {set index [incr size -1]} {$index >= 0} {incr index -1} {
                set entry [$tmp(srtpTab).rs.list get [lindex $delAliases $index]]
                SessionSRTP.DelRemoteSrc [lindex $entry 0] [lindex $entry 1]
                $tmp(srtpTab).rs.list delete [lindex $delAliases $index]
            }
        }
    button $base.rs.clr -highlightthickness 0 -padx 0 -pady 0 -tooltip "Clear Remote Sources for selected session" -image sunkX \
        -command {
            set sessionId [selectedItem .test.sess.list]
            SessionSRTP.DelAllRemoteSrc $sessionId
            for {set index [$tmp(srtpTab).rs.list size]} {$index >= 0} {incr index -1} {
                set entryId [lindex [$tmp(srtpTab).rs.list get $index] 0]
                if {[string compare $sessionId $entryId] == 0} {
                    $tmp(srtpTab).rs.list delete $index
                }
            }
        }
    scrollbar $base.rs.scrl -command "$base.rs.list yview"
    listbox $base.rs.list -borderwidth 1 -background White -yscrollcommand "$base.rs.scrl set" \
        -selectmode single -height 1 -width 1

    # Destination and Source Context
    #################################
    frame $base.dc -borderwidth 0
    menubutton $base.dc.type -borderwidth 1 -indicatoron 1  -width 4 -tooltip "Destination Context type" \
            -menu $base.dc.type.01 -padx 0 -pady 0 -relief raised -text "both" -textvariable tmp(srtp,dctype)
    menu $base.dc.type.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    set tmp(srtp,dcportincr) 0
    $base.dc.type.01 add radiobutton -indicatoron 0 -label "both" -value "both" -variable tmp(srtp,dctype) \
        -command {if {$tmp(srtp,dcport) != ""} {incr tmp(srtp,dcport) $tmp(srtp,dcportincr)}; set tmp(srtp,dcportincr) 0}
    $base.dc.type.01 add radiobutton -indicatoron 0 -label "rtp"  -value "rtp"  -variable tmp(srtp,dctype) \
        -command {if {$tmp(srtp,dcport) != ""} {incr tmp(srtp,dcport) $tmp(srtp,dcportincr)}; set tmp(srtp,dcportincr) 0}
    $base.dc.type.01 add radiobutton -indicatoron 0 -label "rtcp" -value "rtcp" -variable tmp(srtp,dctype) \
        -command {if {$tmp(srtp,dcport) != ""} {incr tmp(srtp,dcport)}; set tmp(srtp,dcportincr) -1}
    entry $base.dc.ip -borderwidth 1 -width 7 -textvariable tmp(srtp,dcip) -tooltip "Destination Context IP address"
    entry $base.dc.port -borderwidth 1 -width 4 -textvariable tmp(srtp,dcport) -tooltip "Destination Context port"
    entry $base.dc.index -borderwidth 1 -width 4 -textvariable tmp(srtp,dcindex) -tooltip "Destination Context index (default zero)"
    checkbutton $base.dc.trigger -text "Trigger" -variable tmp(sess,dctrigger) -tooltip "Trigger Destination Context"
    button $base.dc.setdst -text "DC Set" -tooltip "Set Destination Context (needs Master Key selected)" -width 10 \
        -command {
            set mki [lindex [$tmp(srtpTab).mk.list get [lindex [$tmp(srtpTab).mk.list curselection] 0]] 1]
            SessionSRTP.SetDestContext [selectedItem .test.sess.list] $tmp(srtp,dcip) $tmp(srtp,dcport) $tmp(srtp,dctype) $tmp(srtp,dcindex) $tmp(sess,dctrigger) $mki
            set port [expr {$tmp(srtp,dcport) + $tmp(srtp,dcportincr)}]
            set tmp(sess,remoteAddr) "$tmp(srtp,dcip):$port"
        }
    button $base.dc.setsrc -text "SC Set" -tooltip "Set Source Context (needs Master Key and Remote Source selected)" -width 10 \
        -command {
            set mki [lindex [$tmp(srtpTab).mk.list get [lindex [$tmp(srtpTab).mk.list curselection] 0]] 1]
            set rs  [lindex [$tmp(srtpTab).rs.list get [lindex [$tmp(srtpTab).rs.list curselection] 0]] 1]
            SessionSRTP.SetSourceContext [selectedItem .test.sess.list] $tmp(srtp,dcindex) $tmp(sess,dctrigger) $mki $rs
        }
    set tmp(srtp,dcindex) 0

    # Current Indexies for Remote Party (CIRP)
    ###########################################
    frame  $base.cirp          -borderwidth 0
    entry  $base.cirp.ip       -borderwidth 1 -width 20 -textvariable tmp(srtp,cirpIp)       -tooltip "Destination IP address"
    entry  $base.cirp.portRtp  -borderwidth 1 -width  5 -textvariable tmp(srtp,cirpPortRtp)  -tooltip "Destination RTP Port"
    entry  $base.cirp.portRtcp -borderwidth 1 -width  5 -textvariable tmp(srtp,cirpPortRtcp) -tooltip "Destination RTCP Port"
    button $base.cirp.sendIndexes -text "Send Indexes" -tooltip "Send Current Indexes to the Remote Session" \
        -command {
            SessionSRTP.SendIndexes [selectedItem .test.sess.list] $tmp(srtp,cirpIp) $tmp(srtp,cirpPortRtp) $tmp(srtp,cirpPortRtcp)
            set tmp(sess,remoteAddr) "$tmp(srtp,cirpIp):$tmp(srtp,cirpPortRtp)"
    }
    ###################
    # SETTING GEOMETRY
    ###################

    grid rowconf $base 0 -weight 0
    grid rowconf $base 1 -weight 0
    grid rowconf $base 2 -weight 0
    grid rowconf $base 3 -weight 0
    grid rowconf $base 4 -weight 0
    grid rowconf $base 5 -weight 0
    grid rowconf $base 6 -weight 0
    grid rowconf $base 7 -weight 1
    grid columnconf $base 0 -weight 1

    grid $base.buttons -in $base -column 0 -row 0 -sticky nesw -padx 2 -pady 2
    grid $base.buttons.construct  -in $base.buttons -column 0 -row 0 -sticky nesw -padx 2 -pady 2
    grid $base.buttons.constActiv -in $base.buttons -column 0 -row 0 -sticky n    -padx 2 -pady 1 -columnspan 2
    grid $base.buttons.activate   -in $base.buttons -column 1 -row 0 -sticky nesw -padx 2 -pady 2
    grid $base.buttons.deactivate -in $base.buttons -column 2 -row 0 -sticky nesw -padx 2 -pady 2
    grid $base.buttons.notify     -in $base.buttons -column 3 -row 0 -sticky nesw -padx 2 -pady 2
    grid columnconf $base.buttons 0 -weight 1
    grid columnconf $base.buttons 1 -weight 1
    grid columnconf $base.buttons 2 -weight 1
    grid columnconf $base.buttons 3 -weight 1

    grid $base.mk -in $base -column 0 -row 1 -sticky nesw -padx 2 -pady 2
    grid $base.mk.idn        -in $base.mk -column 0 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.mk.ey         -in $base.mk -column 1 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.mk.salt       -in $base.mk -column 2 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.mk.add        -in $base.mk -column 3 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.mk.del        -in $base.mk -column 4 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.mk.clr        -in $base.mk -column 5 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.mk.list       -in $base.mk -column 0 -row 1 -sticky nesw -padx 1 -pady 2 -columnspan 5
    grid $base.mk.scrl       -in $base.mk -column 5 -row 1 -sticky ns   -padx 1 -pady 2
    grid columnconf $base.mk 0 -weight 1
    grid columnconf $base.mk 1 -weight 1
    grid columnconf $base.mk 2 -weight 1
    grid columnconf $base.mk 3 -weight 0
    grid columnconf $base.mk 4 -weight 0
    grid columnconf $base.mk 5 -weight 0
    grid columnconf $base.mk 6 -weight 0

    grid $base.rs -in $base -column 0 -row 2 -sticky nesw -padx 2 -pady 2
    grid $base.rs.type       -in $base.rs -column 0 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.rs.client     -in $base.rs -column 1 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.rs.sess       -in $base.rs -column 2 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.rs.add        -in $base.rs -column 3 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.rs.del        -in $base.rs -column 4 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.rs.clr        -in $base.rs -column 5 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.rs.list       -in $base.rs -column 0 -row 1 -sticky nesw -padx 1 -pady 2 -columnspan 5
    grid $base.rs.scrl       -in $base.rs -column 5 -row 1 -sticky ns   -padx 1 -pady 2
    grid columnconf $base.rs 0 -weight 5
    grid columnconf $base.rs 1 -weight 8
    grid columnconf $base.rs 2 -weight 8
    grid columnconf $base.rs 3 -weight 0
    grid columnconf $base.rs 4 -weight 0
    grid columnconf $base.rs 5 -weight 0
    grid columnconf $base.rs 6 -weight 0

    grid $base.dc -in $base -column 0 -row 3 -sticky nesw -padx 2 -pady 2
    grid $base.dc.type       -in $base.dc -column 0 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.dc.ip         -in $base.dc -column 1 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.dc.port       -in $base.dc -column 2 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.dc.index      -in $base.dc -column 3 -row 0 -sticky ns   -padx 1 -pady 2
    grid $base.dc.trigger    -in $base.dc -column 4 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.dc.setdst     -in $base.dc -column 5 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.dc.setsrc     -in $base.dc -column 6 -row 0 -sticky nesw -padx 1 -pady 2
    grid columnconf $base.dc 0 -weight 0
    grid columnconf $base.dc 1 -weight 8
    grid columnconf $base.dc 2 -weight 0
    grid columnconf $base.dc 3 -weight 0
    grid columnconf $base.dc 4 -weight 0
    grid columnconf $base.dc 5 -weight 0


    grid $base.cirp              -in $base -column 0 -row 4 -sticky nesw -padx 2 -pady 2
    grid $base.cirp.ip           -in $base.cirp -column 0 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.cirp.portRtp      -in $base.cirp -column 1 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.cirp.portRtcp     -in $base.cirp -column 2 -row 0 -sticky nesw -padx 1 -pady 2
    grid $base.cirp.sendIndexes  -in $base.cirp -column 3 -row 0 -sticky ns   -padx 1 -pady 2
    grid columnconf $base.cirp 0 -weight 2
    grid columnconf $base.cirp 1 -weight 1
    grid columnconf $base.cirp 2 -weight 1
    grid columnconf $base.cirp 3 -weight 0

    ########
    # OTHER
    ########

    bind $base.mk.list <<ListboxSelect>> {+ srtpVerifySelection %W }
    bind $base.rs.list <<ListboxSelect>> {+ srtpVerifySelection %W }
}


proc test:confTab {base tabButton} {
    global tmp app
    set tmp(confTab) $base

    ###################
    # CREATING WIDGETS
    ###################

    if {$app(sess,curConf) == ""} {
        test:setConfVals "default" ""
    } else {
        test:setConfVals "load" $app(sess,curConf)
    }

    frame $base.store -borderwidth 0
    menubutton $base.store.confs -borderwidth 1 -indicatoron 1  -width 10 -tooltip "Select type of configuration to set" \
            -menu $base.store.confs.01 -padx 0 -pady 0 -relief raised -text "" -textvariable app(sess,curConf)
    menu $base.store.confs.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    $base.store.confs.01 add radiobutton -indicatoron 0 -label "default" -value "" -variable app(sess,curConf) \
        -command {test:setConfVals "default" ""; set tmp(sess,curConf) ""; set app(sess,curConf) ""}
    foreach name $app(conf,names) {
        $base.store.confs.01 add radiobutton -indicatoron 0 -label $name -value $name -variable app(sess,curConf) \
            -command "test:setConfVals load $name; set tmp(sess,curConf) $name"
    }
    button $base.store.dl -text "Delete" -width 6 -highlightthickness 0 -padx 0 -pady 0 -tooltip "Delete selected configuration" \
        -command {
            if {$app(sess,curConf) != ""} {
                set index [lsearch $app(conf,names) $app(sess,curConf)]
                set app(conf,names) [lreplace $app(conf,names) $index $index]
                incr index
                $tmp(confTab).store.confs.01 delete $index
                test:setConfVals "delete" $app(sess,curConf)
                set app(sess,curConf) ""
                test:setConfVals "default" ""
            }
        }
    entry $base.store.name -textvariable tmp(sess,curConf) -width 10 -tooltip "Configuration name to save"
    button $base.store.sv -text "Save" -width 6 -highlightthickness 0 -padx 0 -pady 0  -tooltip "Save current configuration" \
        -command {
            if {$tmp(sess,curConf) != ""} {
                test:setConfVals "save" $tmp(sess,curConf)
                if {[lsearch $app(conf,names) $tmp(sess,curConf)] == -1} {
                    lappend app(conf,names) $tmp(sess,curConf)
                    $tmp(confTab).store.confs.01 add radiobutton -indicatoron 0 -label $tmp(sess,curConf) -value $tmp(sess,curConf) -variable app(sess,curConf) \
                        -command "test:setConfVals load $tmp(sess,curConf); set tmp(sess,curConf) $tmp(sess,curConf)"
                }
                set app(sess,curConf) $tmp(sess,curConf)
            }
        }

    frame $base.select -borderwidth 0
    label $base.select.lab -text "Configure:"
    menubutton $base.select.m -borderwidth 1 -indicatoron 1  -width 10 -tooltip "Select type of configuration to set" \
            -menu $base.select.m.01 -padx 0 -pady 0 -relief raised -text "" -textvariable app(conf,select)
    menu $base.select.m.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach val { mk kd pref encr auth ks rl his } lab { "Master Key Sizes" "Key Deriviation" "Prefix Length" \
            "Encryption" "Authentication" "Key Sizes" "Replay List Size" "History Size"} {
        $base.select.m.01 add radiobutton -indicatoron 0 -label $lab -value $val -variable app(conf,select)\
            -command {
                if {$tmp(conf,select) != ""} {grid remove $tmp(confTab).$tmp(conf,select)}
                set tmp(conf,select) $app(conf,select)
                grid $tmp(confTab).$app(conf,select) -in $tmp(confTab) -column 0 -row 1 -sticky news -padx 2 -pady 2}
    }
    $base.select.m.01 add separator
    foreach val { keyPool streamPool contextPool keyHash sourceHash destHash } \
        lab { "Master Key Pool" "Stream Pool" "Context Pool" "Key Hash" "Source Hash" "Destination Hash"} {
        $base.select.m.01 add radiobutton -indicatoron 0 -label $lab -value $val -variable app(conf,select)\
            -command {
                if {$tmp(conf,select) != ""} {grid remove $tmp(confTab).$tmp(conf,select)}
                set tmp(conf,select) $app(conf,select)
                grid $tmp(confTab).$app(conf,select) -in $tmp(confTab) -column 0 -row 1 -sticky news -padx 2 -pady 2}
    }

    frame $base.mk -borderwidth 0
    button $base.mk.set -text "Set MK Size" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {SessionSRTP.SetMasterKeySizes [selectedItem .test.sess.list] $tmp(sess,mkiSize) $tmp(sess,keySize) $tmp(sess,saltSize)} \
        -tooltip "Set SRTP Master Key sizes for the session"
    label $base.mk.mkiSizel -text "MKI Size:" -width 8
    entry $base.mk.mkiSize -textvariable tmp(sess,mkiSize) -width 6 -tooltip "MKI Size"
    label $base.mk.keySizel -text "Key Size:" -width 8
    entry $base.mk.keySize -textvariable tmp(sess,keySize) -width 6 -tooltip "Key Size"
    label $base.mk.saltSizel -text "Salt Size:" -width 8
    entry $base.mk.saltSize -textvariable tmp(sess,saltSize) -width 6 -tooltip "Salt Size"

    frame $base.kd -borderwidth 0
    button $base.kd.set -text "Set Derivation" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {SessionSRTP.KeyDerivation [selectedItem .test.sess.list] $tmp(sess,kdAlg) $tmp(sess,kdRate)} \
        -tooltip "Set SRTP key derivation algorithm and rate"
    menubutton $base.kd.alg -borderwidth 1 -indicatoron 1  -width 12 -tooltip "Select key derivation algorithm" \
            -menu $base.kd.alg.01 -padx 0 -pady 0 -relief raised -text "NONE" -textvariable tmp(sess,kdAlg)
    menu $base.kd.alg.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach val { "NONE" "AESCM" } lab { "NONE" "AESCM" } {
        $base.kd.alg.01 add radiobutton -indicatoron 0 -label $lab -value $val -variable tmp(sess,kdAlg)
    }
    label $base.kd.ratel -text "Rate:" -width 8
    entry $base.kd.rate -textvariable tmp(sess,kdRate) -width 6 -tooltip "Key derivation rate"

    frame $base.pref -borderwidth 0
    button $base.pref.set -text "Set Prefix" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {SessionSRTP.PrefixLength [selectedItem .test.sess.list] $tmp(sess,prefLen)} \
        -tooltip "Set SRTP prefix length"
    label $base.pref.lenl -text "Length:" -width 8
    entry $base.pref.len -textvariable tmp(sess,prefLen) -width 6 -tooltip "Prefix length"

    frame $base.encr -borderwidth 0
    button $base.encr.set -text "Set Encryption" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {SessionSRTP.SetEncryption [selectedItem .test.sess.list] $tmp(sess,encrType) $tmp(sess,encrAlg) $tmp(sess,encrUseMki)} \
        -tooltip "Set SRTP encryption"
    menubutton $base.encr.type -borderwidth 1 -indicatoron 1  -width 12 -tooltip "Select media type for which to se" \
            -menu $base.encr.type.01 -padx 0 -pady 0 -relief raised -text "both" -textvariable tmp(sess,encrType)
    menu $base.encr.type.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach val { "both" "rtp" "rtcp" } lab { "both" "rtp" "rtcp" } {
        $base.encr.type.01 add radiobutton -indicatoron 0 -label $lab -value $val -variable tmp(sess,encrType)
    }
    menubutton $base.encr.alg -borderwidth 1 -indicatoron 1  -width 12 -tooltip "Select encryption algorithm" \
            -menu $base.encr.alg.01 -padx 0 -pady 0 -relief raised -text "NONE" -textvariable tmp(sess,encrAlg)
    menu $base.encr.alg.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach val { "NONE" "AESCM" "AESF8" } lab { "NONE" "AESCM" "AESF8" } {
        $base.encr.alg.01 add radiobutton -indicatoron 0 -label $lab -value $val -variable tmp(sess,encrAlg)
    }
    checkbutton $base.encr.mki -text "Use MKI" -variable tmp(sess,encrUseMki)  -tooltip "Use MKI"

    frame $base.auth -borderwidth 0
    button $base.auth.set -text "Set Auth." -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {SessionSRTP.SetAuthentication [selectedItem .test.sess.list] $tmp(sess,authType) $tmp(sess,authAlg) $tmp(sess,tagSize)} \
        -tooltip "Set SRTP authentication"
    menubutton $base.auth.type -borderwidth 1 -indicatoron 1  -width 12 -tooltip "Select media type for which to set" \
            -menu $base.auth.type.01 -padx 0 -pady 0 -relief raised -text "both" -textvariable tmp(sess,authType)
    menu $base.auth.type.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach val { "both" "rtp" "rtcp" } lab { "both" "rtp" "rtcp" } {
        $base.auth.type.01 add radiobutton -indicatoron 0 -label $lab -value $val -variable tmp(sess,authType)
    }
    menubutton $base.auth.alg -borderwidth 1 -indicatoron 1  -width 12 -tooltip "Select authentication algorithm" \
            -menu $base.auth.alg.01 -padx 0 -pady 0 -relief raised -text "NONE" -textvariable tmp(sess,authAlg)
    menu $base.auth.alg.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach val { "NONE" "HMACSHA1" } lab { "NONE" "HMACSHA1" } {
        $base.auth.alg.01 add radiobutton -indicatoron 0 -label $lab -value $val -variable tmp(sess,authAlg)
    }
    label $base.auth.tagSizel -text "Tag Size:" -width 8
    entry $base.auth.tagSize -textvariable tmp(sess,tagSize) -width 6 -tooltip "Tag Size"

    frame $base.ks -borderwidth 0
    button $base.ks.set -text "Set Key Size" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {SessionSRTP.SetKeySizes [selectedItem .test.sess.list] $tmp(sess,ksType) $tmp(sess,ksEncSize) $tmp(sess,ksAutSize) $tmp(sess,ksSalSize)} \
        -tooltip "Set SRTP key sizes"
    menubutton $base.ks.type -borderwidth 1 -indicatoron 1  -width 12 -tooltip "Select media type for which to set" \
            -menu $base.ks.type.01 -padx 0 -pady 0 -relief raised -text "both" -textvariable tmp(sess,ksType)
    menu $base.ks.type.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach val { "both" "rtp" "rtcp" } lab { "both" "rtp" "rtcp" } {
        $base.ks.type.01 add radiobutton -indicatoron 0 -label $lab -value $val -variable tmp(sess,ksType)
    }
    label $base.ks.encl -text "EK Size:" -width 8
    entry $base.ks.enc -textvariable tmp(sess,ksEncSize) -width 6 -tooltip "Encrypt Key Size"
    label $base.ks.autl -text "AK Size:" -width 8
    entry $base.ks.aut -textvariable tmp(sess,ksAutSize) -width 6 -tooltip "Authentication Key Size"
    label $base.ks.sall -text "SK Size:" -width 8
    entry $base.ks.sal -textvariable tmp(sess,ksSalSize) -width 6 -tooltip "Salt Key Size"

    frame $base.rl -borderwidth 0
    button $base.rl.set -text "Set Replay" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {SessionSRTP.ReplayListSizes [selectedItem .test.sess.list] $tmp(sess,rlType) $tmp(sess,rlSize)} \
        -tooltip "Set SRTP replay list size"
    menubutton $base.rl.type -borderwidth 1 -indicatoron 1  -width 12 -tooltip "Select media type for which to set" \
            -menu $base.rl.type.01 -padx 0 -pady 0 -relief raised -text "both" -textvariable tmp(sess,rlType)
    menu $base.rl.type.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach val { "both" "rtp" "rtcp" } lab { "both" "rtp" "rtcp" } {
        $base.rl.type.01 add radiobutton -indicatoron 0 -label $lab -value $val -variable tmp(sess,rlType)
    }
    label $base.rl.sizel -text "List Size:" -width 8
    entry $base.rl.size -textvariable tmp(sess,rlSize) -width 6 -tooltip "Replay list size"

    frame $base.his -borderwidth 0
    button $base.his.set -text "Set History" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {SessionSRTP.SetHistory [selectedItem .test.sess.list] $tmp(sess,hisType) $tmp(sess,hisSize)} \
        -tooltip "Set SRTP history size"
    menubutton $base.his.type -borderwidth 1 -indicatoron 1  -width 12 -tooltip "Select media type for which to set" \
            -menu $base.his.type.01 -padx 0 -pady 0 -relief raised -text "both" -textvariable tmp(sess,hisType)
    menu $base.his.type.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach val { "both" "rtp" "rtcp" } lab { "both" "rtp" "rtcp" } {
        $base.his.type.01 add radiobutton -indicatoron 0 -label $lab -value $val -variable tmp(sess,hisType)
    }
    label $base.his.sizel -text "Size:" -width 8
    entry $base.his.size -textvariable tmp(sess,hisSize) -width 6 -tooltip "History size"

    frame $base.keyPool -borderwidth 0
    button $base.keyPool.set -text "Set Key Pool" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {SessionSRTP.SetKeyPool [selectedItem .test.sess.list] \
        $tmp(sess,keyPoolType) $tmp(sess,keyPoolPageItems) $tmp(sess,keyPoolPageSize) \
        $tmp(sess,keyPoolMaxItems)  $tmp(sess,keyPoolMinItems) $tmp(sess,keyPoolFreeLevel)} \
        -tooltip "Set SRTP Master Key Pool Configuration"
    menubutton $base.keyPool.type -borderwidth 1 -indicatoron 1  -width 12 -tooltip "Select Pool type for which to set" \
         -menu $base.keyPool.type.01 -padx 0 -pady 0 -relief raised -text "Type" -textvariable tmp(sess,keyPoolType)
    menu $base.keyPool.type.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach val { "EXPANDING" "FIXED" "DYNAMIC" } lab { "EXPANDING" "FIXED" "DYNAMIC" } {
        $base.keyPool.type.01 add radiobutton -indicatoron 0 -label $lab -value $val -variable tmp(sess,keyPoolType)
    }
    label $base.keyPool.pageItemsl -text "Page Items:" -width 8
    entry $base.keyPool.pageItems -textvariable tmp(sess,keyPoolPageItems) -width 6 -tooltip "Page Items"
    label $base.keyPool.pageSizel -text "Page Size:" -width 8
    entry $base.keyPool.pageSize  -textvariable tmp(sess,keyPoolPageSize)  -width 6 -tooltip "Page Size"
    label $base.keyPool.maxItemsl -text "Max Items:" -width 8
    entry $base.keyPool.maxItems  -textvariable tmp(sess,keyPoolMaxItems)  -width 6 -tooltip "Max Items"
    label $base.keyPool.minItemsl -text "Min Items:" -width 8
    entry $base.keyPool.minItems  -textvariable tmp(sess,keyPoolMinItems)  -width 6 -tooltip "Min Items"
    label $base.keyPool.freeLevell -text "Free Level:" -width 8
    entry $base.keyPool.freeLevel -textvariable tmp(sess,keyPoolFreeLevel) -width 6 -tooltip "Free Level"

    frame $base.streamPool -borderwidth 0
    button $base.streamPool.set -text "Set Stream Pool" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {SessionSRTP.SetStreamPool [selectedItem .test.sess.list] \
        $tmp(sess,streamPoolType) $tmp(sess,streamPoolPageItems) $tmp(sess,streamPoolPageSize) \
        $tmp(sess,streamPoolMaxItems)  $tmp(sess,streamPoolMinItems) $tmp(sess,streamPoolFreeLevel)} \
        -tooltip "Set SRTP Stream Pool Configuration"
    menubutton $base.streamPool.type -borderwidth 1 -indicatoron 1  -width 12 -tooltip "Select Pool type for which to set" \
         -menu $base.streamPool.type.01 -padx 0 -pady 0 -relief raised -text "Type" -textvariable tmp(sess,streamPoolType)
    menu $base.streamPool.type.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach val { "EXPANDING" "FIXED" "DYNAMIC" } lab { "EXPANDING" "FIXED" "DYNAMIC" } {
        $base.streamPool.type.01 add radiobutton -indicatoron 0 -label $lab -value $val -variable tmp(sess,streamPoolType)
    }
    label $base.streamPool.pageItemsl -text "Page Items:" -width 8
    entry $base.streamPool.pageItems -textvariable tmp(sess,streamPoolPageItems) -width 6 -tooltip "Page Items"
    label $base.streamPool.pageSizel -text "Page Size:" -width 8
    entry $base.streamPool.pageSize  -textvariable tmp(sess,streamPoolPageSize)  -width 6 -tooltip "Page Size"
    label $base.streamPool.maxItemsl -text "Max Items:" -width 8
    entry $base.streamPool.maxItems  -textvariable tmp(sess,streamPoolMaxItems)  -width 6 -tooltip "Max Items"
    label $base.streamPool.minItemsl -text "Min Items:" -width 8
    entry $base.streamPool.minItems  -textvariable tmp(sess,streamPoolMinItems)  -width 6 -tooltip "Min Items"
    label $base.streamPool.freeLevell -text "Free Level:" -width 8
    entry $base.streamPool.freeLevel -textvariable tmp(sess,streamPoolFreeLevel) -width 6 -tooltip "Free Level"

    frame $base.contextPool -borderwidth 0
    button $base.contextPool.set -text "Set Context Pool" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {SessionSRTP.SetContextPool [selectedItem .test.sess.list] \
        $tmp(sess,contextPoolType) $tmp(sess,contextPoolPageItems) $tmp(sess,contextPoolPageSize) \
        $tmp(sess,contextPoolMaxItems)  $tmp(sess,contextPoolMinItems) $tmp(sess,contextPoolFreeLevel)} \
        -tooltip "Set SRTP Context Pool Configuration"
    menubutton $base.contextPool.type -borderwidth 1 -indicatoron 1  -width 12 -tooltip "Select Pool type for which to set" \
         -menu $base.contextPool.type.01 -padx 0 -pady 0 -relief raised -text "Type" -textvariable tmp(sess,contextPoolType)
    menu $base.contextPool.type.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach val { "EXPANDING" "FIXED" "DYNAMIC" } lab { "EXPANDING" "FIXED" "DYNAMIC" } {
        $base.contextPool.type.01 add radiobutton -indicatoron 0 -label $lab -value $val -variable tmp(sess,contextPoolType)
    }
    label $base.contextPool.pageItemsl -text "Page Items:" -width 8
    entry $base.contextPool.pageItems -textvariable tmp(sess,contextPoolPageItems) -width 6 -tooltip "Page Items"
    label $base.contextPool.pageSizel -text "Page Size:" -width 8
    entry $base.contextPool.pageSize  -textvariable tmp(sess,contextPoolPageSize)  -width 6 -tooltip "Page Size"
    label $base.contextPool.maxItemsl -text "Max Items:" -width 8
    entry $base.contextPool.maxItems  -textvariable tmp(sess,contextPoolMaxItems)  -width 6 -tooltip "Max Items"
    label $base.contextPool.minItemsl -text "Min Items:" -width 8
    entry $base.contextPool.minItems  -textvariable tmp(sess,contextPoolMinItems)  -width 6 -tooltip "Min Items"
    label $base.contextPool.freeLevell -text "Free Level:" -width 8
    entry $base.contextPool.freeLevel -textvariable tmp(sess,contextPoolFreeLevel) -width 6 -tooltip "Free Level"

    frame $base.keyHash -borderwidth 0
    button $base.keyHash.set -text "Set Key Hash" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {SessionSRTP.SetKeyHash [selectedItem .test.sess.list] $tmp(sess,keyHashType) $tmp(sess,keyHashStartSize)} \
        -tooltip "Set SRTP Set Key Hash Configuration"
    menubutton $base.keyHash.type -borderwidth 1 -indicatoron 1  -width 12 -tooltip "Select hash type for which to set" \
            -menu $base.keyHash.type.01 -padx 0 -pady 0 -relief raised -text "Type" -textvariable tmp(sess,keyHashType)
    menu $base.keyHash.type.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach val { "EXPANDING" "FIXED" "DYNAMIC" } lab { "EXPANDING" "FIXED" "DYNAMIC" } {
        $base.keyHash.type.01 add radiobutton -indicatoron 0 -label $lab -value $val -variable tmp(sess,keyHashType)
    }
    label $base.keyHash.startSizel -text "Start Size:" -width 8
    entry $base.keyHash.startSize -textvariable tmp(sess,keyHashStartSize) -width 6 -tooltip "Start Size"


    frame $base.sourceHash -borderwidth 0
    button $base.sourceHash.set -text "Set Source Hash" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {SessionSRTP.SetSourceHash [selectedItem .test.sess.list] $tmp(sess,sourceHashType) $tmp(sess,sourceHashStartSize)} \
        -tooltip "Set SRTP Set Source Hash Configuration"
    menubutton $base.sourceHash.type -borderwidth 1 -indicatoron 1  -width 12 -tooltip "Select hash type for which to set" \
            -menu $base.sourceHash.type.01 -padx 0 -pady 0 -relief raised -text "Type" -textvariable tmp(sess,sourceHashType)
    menu $base.sourceHash.type.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach val { "EXPANDING" "FIXED" "DYNAMIC" } lab { "EXPANDING" "FIXED" "DYNAMIC" } {
        $base.sourceHash.type.01 add radiobutton -indicatoron 0 -label $lab -value $val -variable tmp(sess,sourceHashType)
    }
    label $base.sourceHash.startSizel -text "Start Size:" -width 8
    entry $base.sourceHash.startSize -textvariable tmp(sess,sourceHashStartSize) -width 6 -tooltip "Start Size"

    frame $base.destHash -borderwidth 0
    button $base.destHash.set -text "Set Dest Hash" -width 12 -highlightthickness 0 -padx 0 -pady 0 \
        -command {SessionSRTP.SetDestHash [selectedItem .test.sess.list] $tmp(sess,destHashType) $tmp(sess,destHashStartSize)} \
        -tooltip "Set SRTP Set Destination Hash Configuration"
    menubutton $base.destHash.type -borderwidth 1 -indicatoron 1  -width 12 -tooltip "Select hash type for which to set" \
            -menu $base.destHash.type.01 -padx 0 -pady 0 -relief raised -text "Type" -textvariable tmp(sess,destHashType)
    menu $base.destHash.type.01 -activeborderwidth 1 -borderwidth 1 -tearoff 0
    foreach val { "EXPANDING" "FIXED" "DYNAMIC" } lab { "EXPANDING" "FIXED" "DYNAMIC" } {
        $base.destHash.type.01 add radiobutton -indicatoron 0 -label $lab -value $val -variable tmp(sess,destHashType)
    }
    label $base.destHash.startSizel -text "Start Size:" -width 8
    entry $base.destHash.startSize -textvariable tmp(sess,destHashStartSize) -width 6 -tooltip "Start Size"

    ###################
    # SETTING GEOMETRY
    ###################

    grid rowconf $base 0 -weight 0
    grid rowconf $base 1 -weight 0
    grid rowconf $base 2 -weight 0
    grid rowconf $base 3 -weight 0
    grid rowconf $base 4 -weight 0
    grid rowconf $base 5 -weight 1
    grid columnconf $base 1 -weight 1

    grid $base.store    -in $base       -column 1 -row 0 -sticky nse -padx 2 -pady 2
    grid $base.store.confs -in $base.store -column 0 -row 0 -sticky ew -padx 2 -pady 2 -ipady 1
    grid $base.store.dl    -in $base.store -column 1 -row 0 -sticky ew -padx 2 -pady 2
    grid $base.store.name  -in $base.store -column 0 -row 1 -sticky ew -padx 2 -pady 2
    grid $base.store.sv    -in $base.store -column 1 -row 1 -sticky ew -padx 2 -pady 2
    grid columnconf $base.store 0 -weight 1

    grid $base.select     -in $base        -column 0 -row 0 -sticky nsew -padx 2 -pady 2
    grid $base.select.lab -in $base.select -column 0 -row 0 -sticky ew -padx 2 -pady 2
    grid $base.select.m   -in $base.select -column 1 -row 0 -sticky ew -padx 2 -pady 2 -ipady 1
    grid columnconf $base.select 10 -weight 1

    grid $base.mk.set       -in $base.mk -column 0 -columnspan 2 -row 0 -sticky nsew -padx 2 -pady 2
    grid $base.mk.mkiSizel  -in $base.mk -column 0 -ipady 1 -row 1 -sticky ew -padx 2 -pady 2
    grid $base.mk.mkiSize   -in $base.mk -column 1 -ipady 1 -row 1 -sticky ew -padx 2 -pady 2
    grid $base.mk.keySizel  -in $base.mk -column 0 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.mk.keySize   -in $base.mk -column 1 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.mk.saltSizel -in $base.mk -column 0 -ipady 1 -row 3 -sticky ew -padx 2 -pady 2
    grid $base.mk.saltSize  -in $base.mk -column 1 -ipady 1 -row 3 -sticky ew -padx 2 -pady 2

    grid $base.kd.set     -in $base.kd -column 0 -columnspan 2 -row 0 -sticky nsew -padx 2 -pady 2
    grid $base.kd.alg     -in $base.kd -column 0 -columnspan 2 -ipady 1 -row 1 -sticky ew -padx 2 -pady 2
    grid $base.kd.ratel -in $base.kd -column 0 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.kd.rate    -in $base.kd -column 1 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2

    grid $base.pref.set  -in $base.pref -column 0 -columnspan 2 -row 0 -sticky nesw -padx 2 -pady 2
    grid $base.pref.lenl -in $base.pref -column 0 -ipady 1 -row 1 -sticky ew -padx 2 -pady 2
    grid $base.pref.len  -in $base.pref -column 1 -ipady 1 -row 1 -sticky ew -padx 2 -pady 2

    grid $base.encr.set  -in $base.encr -column 0 -row 0 -sticky nesw -padx 2 -pady 2
    grid $base.encr.type -in $base.encr -column 0 -ipady 1 -row 1 -sticky ew -padx 2 -pady 2
    grid $base.encr.alg  -in $base.encr -column 0 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.encr.mki  -in $base.encr -column 0 -ipady 1 -row 3 -sticky w  -padx 2 -pady 2

    grid $base.auth.set      -in $base.auth -column 0 -columnspan 2 -row 0 -sticky nesw -padx 2 -pady 2
    grid $base.auth.type     -in $base.auth -column 0 -columnspan 2 -ipady 1 -row 1 -sticky ew -padx 2 -pady 2
    grid $base.auth.alg      -in $base.auth -column 0 -columnspan 2 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.auth.tagSizel -in $base.auth -column 0 -ipady 1 -row 3 -sticky we -padx 2 -pady 2
    grid $base.auth.tagSize  -in $base.auth -column 1 -ipady 1 -row 3 -sticky we -padx 2 -pady 2

    grid $base.ks.set  -in $base.ks -column 0 -columnspan 2 -row 0 -sticky nesw -padx 2 -pady 2
    grid $base.ks.type -in $base.ks -column 0 -columnspan 2 -ipady 1 -row 1 -sticky ew -padx 2 -pady 2
    grid $base.ks.encl -in $base.ks -column 0 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.ks.enc  -in $base.ks -column 1 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.ks.autl -in $base.ks -column 0 -ipady 1 -row 3 -sticky ew -padx 2 -pady 2
    grid $base.ks.aut  -in $base.ks -column 1 -ipady 1 -row 3 -sticky ew -padx 2 -pady 2
    grid $base.ks.sall -in $base.ks -column 0 -ipady 1 -row 4 -sticky ew -padx 2 -pady 2
    grid $base.ks.sal  -in $base.ks -column 1 -ipady 1 -row 4 -sticky ew -padx 2 -pady 2

    grid $base.rl.set   -in $base.rl -column 0 -columnspan 2 -row 0 -sticky nesw -padx 2 -pady 2
    grid $base.rl.type  -in $base.rl -column 0 -columnspan 2 -ipady 1 -row 1 -sticky ew -padx 2 -pady 2
    grid $base.rl.size  -in $base.rl -column 0 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.rl.sizel -in $base.rl -column 1 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2

    grid $base.his.set   -in $base.his -column 0 -columnspan 2 -row 0 -sticky nesw -padx 2 -pady 2
    grid $base.his.type  -in $base.his -column 0 -columnspan 2 -ipady 1 -row 1 -sticky ew -padx 2 -pady 2
    grid $base.his.sizel -in $base.his -column 0 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.his.size  -in $base.his -column 1 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2

    grid $base.keyPool.set       -in $base.keyPool -column 0 -columnspan 2 -row 0 -sticky nesw -padx 2 -pady 2
    grid $base.keyPool.type      -in $base.keyPool -column 0 -columnspan 2 -ipady 1 -row 1 -sticky ew -padx 2 -pady 2
    grid $base.keyPool.pageItemsl -in $base.keyPool -column 0 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.keyPool.pageItems  -in $base.keyPool -column 1 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.keyPool.pageSizel  -in $base.keyPool -column 0 -ipady 1 -row 3 -sticky ew -padx 2 -pady 2
    grid $base.keyPool.pageSize   -in $base.keyPool -column 1 -ipady 1 -row 3 -sticky ew -padx 2 -pady 2
    grid $base.keyPool.maxItemsl  -in $base.keyPool -column 0 -ipady 1 -row 4 -sticky ew -padx 2 -pady 2
    grid $base.keyPool.maxItems   -in $base.keyPool -column 1 -ipady 1 -row 4 -sticky ew -padx 2 -pady 2
    grid $base.keyPool.minItemsl  -in $base.keyPool -column 0 -ipady 1 -row 5 -sticky ew -padx 2 -pady 2
    grid $base.keyPool.minItems   -in $base.keyPool -column 1 -ipady 1 -row 5 -sticky ew -padx 2 -pady 2
    grid $base.keyPool.freeLevell -in $base.keyPool -column 0 -ipady 1 -row 6 -sticky ew -padx 2 -pady 2
    grid $base.keyPool.freeLevel  -in $base.keyPool -column 1 -ipady 1 -row 6 -sticky ew -padx 2 -pady 2

    grid $base.streamPool.set       -in $base.streamPool -column 0 -columnspan 2 -row 0 -sticky nesw -padx 2 -pady 2
    grid $base.streamPool.type      -in $base.streamPool -column 0 -columnspan 2 -ipady 1 -row 1 -sticky ew -padx 2 -pady 2
    grid $base.streamPool.pageItemsl -in $base.streamPool -column 0 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.streamPool.pageItems  -in $base.streamPool -column 1 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.streamPool.pageSizel  -in $base.streamPool -column 0 -ipady 1 -row 3 -sticky ew -padx 2 -pady 2
    grid $base.streamPool.pageSize   -in $base.streamPool -column 1 -ipady 1 -row 3 -sticky ew -padx 2 -pady 2
    grid $base.streamPool.maxItemsl  -in $base.streamPool -column 0 -ipady 1 -row 4 -sticky ew -padx 2 -pady 2
    grid $base.streamPool.maxItems   -in $base.streamPool -column 1 -ipady 1 -row 4 -sticky ew -padx 2 -pady 2
    grid $base.streamPool.minItemsl  -in $base.streamPool -column 0 -ipady 1 -row 5 -sticky ew -padx 2 -pady 2
    grid $base.streamPool.minItems   -in $base.streamPool -column 1 -ipady 1 -row 5 -sticky ew -padx 2 -pady 2
    grid $base.streamPool.freeLevell -in $base.streamPool -column 0 -ipady 1 -row 6 -sticky ew -padx 2 -pady 2
    grid $base.streamPool.freeLevel  -in $base.streamPool -column 1 -ipady 1 -row 6 -sticky ew -padx 2 -pady 2

    grid $base.contextPool.set        -in $base.contextPool -column 0 -columnspan 2 -row 0 -sticky nesw -padx 2 -pady 2
    grid $base.contextPool.type       -in $base.contextPool -column 0 -columnspan 2 -ipady 1 -row 1 -sticky ew -padx 2 -pady 2
    grid $base.contextPool.pageItemsl -in $base.contextPool -column 0 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.contextPool.pageItems  -in $base.contextPool -column 1 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.contextPool.pageSizel  -in $base.contextPool -column 0 -ipady 1 -row 3 -sticky ew -padx 2 -pady 2
    grid $base.contextPool.pageSize   -in $base.contextPool -column 1 -ipady 1 -row 3 -sticky ew -padx 2 -pady 2
    grid $base.contextPool.maxItemsl  -in $base.contextPool -column 0 -ipady 1 -row 4 -sticky ew -padx 2 -pady 2
    grid $base.contextPool.maxItems   -in $base.contextPool -column 1 -ipady 1 -row 4 -sticky ew -padx 2 -pady 2
    grid $base.contextPool.minItemsl  -in $base.contextPool -column 0 -ipady 1 -row 5 -sticky ew -padx 2 -pady 2
    grid $base.contextPool.minItems   -in $base.contextPool -column 1 -ipady 1 -row 5 -sticky ew -padx 2 -pady 2
    grid $base.contextPool.freeLevell -in $base.contextPool -column 0 -ipady 1 -row 6 -sticky ew -padx 2 -pady 2
    grid $base.contextPool.freeLevel  -in $base.contextPool -column 1 -ipady 1 -row 6 -sticky ew -padx 2 -pady 2

    grid $base.keyHash.set        -in $base.keyHash -column 0 -columnspan 2 -row 0 -sticky nesw -padx 2 -pady 2
    grid $base.keyHash.type       -in $base.keyHash -column 0 -columnspan 2 -ipady 1 -row 1 -sticky ew -padx 2 -pady 2
    grid $base.keyHash.startSizel -in $base.keyHash -column 0 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.keyHash.startSize  -in $base.keyHash -column 1 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2

    grid $base.sourceHash.set        -in $base.sourceHash -column 0 -columnspan 2 -row 0 -sticky nesw -padx 2 -pady 2
    grid $base.sourceHash.type       -in $base.sourceHash -column 0 -columnspan 2 -ipady 1 -row 1 -sticky ew -padx 2 -pady 2
    grid $base.sourceHash.startSizel -in $base.sourceHash -column 0 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.sourceHash.startSize  -in $base.sourceHash -column 1 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2

    grid $base.destHash.set        -in $base.destHash -column 0 -columnspan 2 -row 0 -sticky nesw -padx 2 -pady 2
    grid $base.destHash.type       -in $base.destHash -column 0 -columnspan 2 -ipady 1 -row 1 -sticky ew -padx 2 -pady 2
    grid $base.destHash.startSizel -in $base.destHash -column 0 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2
    grid $base.destHash.startSize  -in $base.destHash -column 1 -ipady 1 -row 2 -sticky ew -padx 2 -pady 2

    if {$app(conf,select) != ""} {
        grid $base.$app(conf,select) -in $base -column 0 -row 1 -sticky news -padx 2 -pady 2
        set tmp(conf,select) $app(conf,select)
    }

    ########
    # OTHER
    ########

 #tk_messageBox -message "03 - tmp(sess,prefLen)=$tmp(sess,prefLen)"
}

proc test:setConfVals {action name} {
    global tmp app

    set valTypes { mkiSize keySize saltSize kdAlg kdRate prefLen encrAlg encrUseMki authAlg tagSize ksEncSize \
        ksAutSize ksSalSize rlSize hisSize keyPoolType keyPoolPageItems keyPoolPageSize keyPoolMaxItems keyPoolMinItems \
        keyPoolFreeLevel streamPoolType streamPoolPageItems streamPoolPageSize streamPoolMaxItems streamPoolMinItems \
        streamPoolFreeLevel contextPoolType contextPoolPageItems contextPoolPageSize contextPoolMaxItems contextPoolMinItems \
        contextPoolFreeLevel keyHashType keyHashStartSize sourceHashType sourceHashStartSize destHashType destHashStartSize }

    set defaults { 4 16 14 "AESCM" 0 0 "AESCM" "1" "HMACSHA1" 10 16 20 14 64 65536 "EXPANDING" 10 0 0 0 \
        0 "EXPANDING" 20 0 0 0 0 "EXPANDING" 40 0 0 0 0 "EXPANDING" 17 "EXPANDING" 17 "EXPANDING" 17 }

    if {$action == "default"} {
        # Set configuration defaults
        foreach valType $valTypes val $defaults {
            set tmp(sess,$valType) $val
        }
    }
    if {$action == "save"} {
        # saving variables configuration
        foreach valType $valTypes {
            set app(sessConf,$name,$valType) $tmp(sess,$valType)
        }
    }
    if {$action == "load"} {
        # restoring variables from saved configuration
        foreach valType $valTypes {
            set tmp(sess,$valType) $app(sessConf,$name,$valType)
        }
    }
    if {$action == "delete"} {
        # deleting a configuration
        foreach valType $valTypes {
            array unset app sessConf,$name,$valType
        }
    }
}

# test:createMenu
# Creates the menu for the main window
# input  : none
# output : none
# return : none
proc test:createMenu {} {
    global tmp app

    set menus {tools broadcaster help}
    if {$tmp(recorder)} {
        set menus [linsert $menus 2 record]
        set m(record) [record:getmenu]
    }

    # Make sure we've got all the menus here
#        { "Config"          0 {}          config:open}
#        { "Status"          0 Ctrl+S      "Window open .status"}
#        { "Log File"        4 {}          logfile:open}
#        {separator}
#        { "Modules..."      0 {}          "disableModule:create"}
#        { "Start stack"     3 {}          "api:cm:Start"}
#        { "Stop stack"      3 {}          "api:cm:Stop"}
#        { "Restart"         0 {}          "test.Restart"}
    set m(tools) {
        { "Exit"            0 {}          "test.Quit"}
    }
    set m(broadcaster) {
        { "Set server"      0 {}            "bcast:setServer"}
        { "Connect"         0 {}            "bcast:connect $app(bcastaddr)"}
    }
#    set m(advanced) {
#        { "Console input"   0 {}          "Window open .console"}
#        {separator}
#        { "Execute script"  0 {}          script:load}
#        { "Stop script"     0 {}          script:stop}
#        { "Describe script" 0 {}          script:describe}
#        { "Edit script"     0 {}          script:edit}
#    }
    set m(help) {
        { "About"           0 {}          "Window open .about"}
    }

    # Create the main menu and all of the sub menus from the array variable m
    menu .test.main -tearoff 0
    foreach submenu $menus {
        .test.main add cascade -label [string toupper $submenu 0 0] \
            -menu .test.main.$submenu -underline 0
        menu .test.main.$submenu -tearoff 0

        foreach item $m($submenu) {
            set txt [lindex $item 0]

            if {$txt == "separator"} {
                # Put a separator
                .test.main.$submenu add separator
            } else {
                # Put a menu item
                set under [lindex $item 1]
                set key [lindex $item 2]
                set cmd [lindex $item 3]
                .test.main.$submenu add command -label $txt -accel $key -command $cmd -underline $under
            }
        }
    }

#    test:updateScriptMenu
}

