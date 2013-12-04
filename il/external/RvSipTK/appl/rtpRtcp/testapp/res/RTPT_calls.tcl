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
#                                 RTPT_sessions.tcl
#
#   Sessions handling.
#   This file holds all the GUI procedures for sessions: incoming session, outgoing session, overlapped
#   sending, session information, etc.
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
#   CALL INFORMATION operations
#
##############################################################################################


# session:create
# Creation procedure of a session information window.
# Such a window can be opened for each of the sessions of the test application and it displays the
# current session status, along with previous messages, channel information, etc.
# input  : base     - The base handle of the window
#                     Each session should have a different base window handle
#          title    - Title string to display for window
# output : none
# return : none
proc session:create {base title} {
    global tmp app

    if {[winfo exists $base]} {wm deiconify $base; return}

    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel
    wm focusmodel $base passive
    wm geometry $base 400x300+600+150
    wm maxsize $base 1028 753
    wm minsize $base 106 2
    wm overrideredirect $base 0
    wm resizable $base 1 1
    wm title $base $title
    wm transient $base .dummy
    wm protocol $base WM_DELETE_WINDOW "destroy $base
            focus .test"

    frame $base.status -borderwidth 1 -relief groove
    label $base.status.q931StatusLab -borderwidth 0 -text "Q931 Status"
    label $base.status.q931Status -borderwidth 1 -relief sunken
    label $base.status.h245StatusLab -borderwidth 0 -text "H245 Status"
    label $base.status.h245Status -borderwidth 1 -relief sunken
    label $base.status.bwLab -borderwidth 0 -text "Band Width"
    label $base.status.bw -borderwidth 1 -relief sunken
    listbox $base.status.sessionInfo -yscrollcommand "$base.status.scrl set" -height 1 -width 0 -background White
    scrollbar $base.status.scrl -command "$base.status.sessionInfo yview"
    frame $base.chan -borderwidth 2 -relief groove
    frame $base.messages -borderwidth 2 -relief groove
    label $base.messages.header -text Messages
    listbox $base.messages.txt -xscrollcommand "$base.messages.xscrl set" -yscrollcommand "$base.messages.yscrl set" \
        -width 0 -height 1 -background White
    scrollbar $base.messages.yscrl -command "$base.messages.txt set"
    scrollbar $base.messages.xscrl -orient horizontal -command "$base.messages.txt set"
    button $base.messages.clear -tooltip "Clear message box" \
        -borderwidth 0 -highlightthickness 0 -image bmpClear -command "$base.messages.txt delete 0 end"

    address:createAddressesInfo $base.chan 0

    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 1 -weight 1
    grid rowconf $base 2 -weight 1
    grid $base.status -in $base -column 0 -row 0 -sticky nesw -pady 4 -padx 2
    grid columnconf $base.status 1 -weight 1
    grid columnconf $base.status 2 -weight 1
    grid $base.status.q931StatusLab -in $base.status -column 0 -row 0
    grid $base.status.q931Status -in $base.status -column 1 -row 0 -sticky ew -pady 2
    grid $base.status.h245StatusLab -in $base.status -column 0 -row 1
    grid $base.status.h245Status -in $base.status -column 1 -row 1 -sticky ew
    grid $base.status.bwLab -in $base.status -column 0 -row 2
    grid $base.status.bw -in $base.status -column 1 -row 2 -sticky ew -pady 2
    grid $base.status.sessionInfo -in $base.status -column 2 -row 0 -rowspan 3 -sticky nesw
    grid $base.status.scrl -in $base.status -column 3 -row 0 -rowspan 3 -sticky nesw
    grid $base.chan -in $base -sticky nesw -column 0 -row 1 -pady 4 -padx 2
    grid $base.messages -in $base -column 0 -row 2 -sticky nesw -pady 4 -padx 2
    grid columnconf $base.messages 0 -weight 1
    grid rowconf $base.messages 1 -weight 1
    grid $base.messages.txt -in $base.messages -column 0 -row 1 -sticky nesw -padx 5 -pady 5
    grid $base.messages.yscrl -in $base.messages -column 1 -row 1 -sticky nesw
    grid $base.messages.xscrl -in $base.messages -column 0 -row 2 -sticky nesw
    grid $base.messages.clear -in $base.messages -column 1 -row 2 -sticky nesw

    ###########
    # BINDINGS
    ###########
    bind $base.status.sessionInfo <<ListboxSelect>> "$base.status.sessionInfo selection clear 0 end"
    bind $base.messages.txt <<ListboxSelect>> "$base.messages.txt selection clear 0 end"

    ########
    # OTHER
    ########
    placeHeader $base.messages Messages
}



# session:doubleclickedSession
# This procedure is invoked when the user double-clicks on a session in the sessions list box on the main
# window.
# It just opens the session's information window
# input  : none
# output : none
# return : none
proc {session:doubleclickedSession} {} {
    Session.Info [selectedItem .test.sess.list]
}


# Session.FindSessionIndex
# This procedure finds out the index of a session inside the main sessions window.
# It is used by C functions when a specific session needs to be located.
# input  : SessionCounter - Counter information of the session
#                        This is the first number of the session information inside the main sessions window.
# output : none
# return : The index of the session or -1 if session wasn't found
proc Session.FindSessionIndex {SessionCounter} {

    # Set search string for the session counter
    set SearchStr "$SessionCounter *"

    # Get the list and find the session's index inside it
    set i [lsearch -glob [.test.sess.list get 0 end] $SearchStr]

    return $i
}



# Session:ChangeState
# This procedure changes the string representing a session inside the main window.
# It makes sure the current selection is not changed in the sessions listbox.
# input  : counter      - Session counter for the changed session
#          stateString  - String to display instead of the last one
# output : none
# return : none
proc Session:ChangeState {counter stateString} {
    global app

    # Remember the last selection we had
    set selected [.test.sess.list curselection]

    # Find out the line we're looking for
    set i [Session.FindSessionIndex $counter]

    if {$i != -1} then {
        # Remove the older element
        .test.sess.list delete $i
    } else {
        set i end
        set selected end
    }

    # Set the new state for the session
    .test.sess.list insert $i $stateString

    if { [.test.sess.list size] == 1 } then {
        # We mark a session if it's the only one we've got
        .test.sess.list selection set 0
    } elseif {$selected != ""} {
        # Make sure the selection is still fine
        .test.sess.list selection clear 0 end
        .test.sess.list selection set $selected
    }

    event generate .test.sess.list <<ListboxSelect>>
}

# Session:AddColor
# This procedure adds the confrence color to the color listbox
# input  : counter     - Session counter for the changed session
#          stateString - String to display instead of the last one
# output : none
# return : none
proc Session:AddColor {counter conf conn} {
    global app

    # Find out the line we're looking for
    set i [Session.FindSessionIndex $counter]

    if {($i != -1) && ($app(options,suppressMessages) == 0)} then {
        .test.sess.conf itemconfigure $i -background $conf
        .test.sess.conn itemconfigure $i -background $conn
    }
}

# Session:AddRemark
# This procedure adds a remark to the string representing a session inside the main window.
# It makes sure the current selection is not changed in the sessions listbox.
# input  : counter   - Session counter for the changed session
#          remString - String to add
# output : none
# return : none
proc Session:AddRemark {counter remString} {

    # Remember the last selection we had
    set selected [.test.sess.list curselection]

    # Find out the line we're looking for
    set i [Session.FindSessionIndex $counter]

    if {$i != -1} then {
        # Remove the older element
        set String [.test.sess.list get $i]
        .test.sess.list delete $i
    } else {
        return
    }

    # Add the remark to the string
    append String ", $remString"
    # Set the new state for the session
    .test.sess.list insert $i $String

    if { [.test.sess.list size] == 1 } then {
        # We mark a session if it's the only one we've got
        .test.sess.list selection set 0
    } elseif {$selected != ""} {
        # Make sure the selection is still fine
        .test.sess.list selection clear 0 end
        .test.sess.list selection set $selected
    }

    event generate .test.sess.list <<ListboxSelect>>
}

# Session:RemoveRemark
# This procedure removes any remarks from the string representing a session inside the main window.
# It makes sure the current selection is not changed in the sessions listbox.
# input  : counter   - Session counter for the changed session
# output : none
# return : none
proc Session:RemoveRemark {counter} {

    # Remember the last selection we had
    set selected [.test.sess.list curselection]

    # Find out the line we're looking for
    set i [Session.FindSessionIndex $counter]

    if {$i != -1} then {
        # Remove the older element
        set String [.test.sess.list get $i]
        .test.sess.list delete $i
    } else {
        return
    }

    # Remove the remark from the string
    set rangeEnd [string last , $String]
    if {$rangeEnd != -1} {
        incr rangeEnd -1
        set String [string range $String 0 $rangeEnd]
    }
    # Set the new state for the session
    .test.sess.list insert $i $String

    if { [.test.sess.list size] == 1 } then {
        # We mark a session if it's the only one we've got
        .test.sess.list selection set 0
    } elseif {$selected != ""} {
        # Make sure the selection is still fine
        .test.sess.list selection clear 0 end
        .test.sess.list selection set $selected
    }

    event generate .test.sess.list <<ListboxSelect>>
}

# Session:cleanup
# This procedure closes all windows that belong to a specific session
# input  : sessionInfo      - Session counter for the changed session
# output : none
# return : none
proc Session:cleanup {sessionInfo} {
    global tmp app

    if {[array get tmp $sessionInfo] != ""} {
        # close all windows that belong to specific session
        foreach win $tmp($sessionInfo) {
            after idle "Window close $win"
        }

        array unset app $sessionInfo
    }
}

# Session:addWindowToList
# This procedure inserts all a window that belong to a specific session to a global list
# input  : sessionInfo      - Session counter for the changed session
# output : none
# return : none
proc Session:addWindowToList {sessionInfo win} {
     global tmp app

    # Set list of windows for a session
    if {[array get tmp $sessionInfo] != ""} {
        # Already have windows for this session
        lappend tmp($sessionInfo) $win
    } else {
        # First window in session - make a list for it
        set tmp($sessionInfo) [list $win]

    }

    # Make sure this window knows its session
    set tmp($win) $sessionInfo
}

# session:SetButtonsState
# This procedure makes sure buttons are disabled in the main window when no session is selected.
# input  : none
# output : none
# return : none
proc sess:SetButtonsState {base} {

    # Check if any session is selected from the list
    if {[.test.sess.list curselection] != ""} {
        set stateStr "normal"
    } else {
        set stateStr "disable"
    }

#    $base.sessionBut.drop configure -state $stateStr
}

# session:SetAddrButtonsState
# This procedure makes sure buttons are disabled in the main window when no session or
# no channel is selected.
# input  : none
# output : none
# return : none
proc sess:SetAddrButtonsState {base} {
    # Check if any session is selected from the list
    if {[.test.sess.list curselection] != ""} {
        $base.addrs.new configure -state "normal"
    } else {
        $base.addrs.new configure -state "disable"
        return
    }

    # Check if any channles are selected from the lists
    set inSel [.test.addrs.inList curselection]
    set outSel [.test.addrs.outList curselection]
    if { $inSel != "" } {set inSel [.test.addrs.inList get $inSel]}
    if { $outSel != "" } {set outSel [.test.addrs.outList get $outSel]}

    if { $inSel != ""} {
        set stateStr "normal"
    } else {
        set stateStr "disable"
    }

    if { $outSel != ""} {
#        $base.chanBut.loop configure -state $stateStr
    } else {
#        $base.chanBut.same configure -state $stateStr
    }
    # set drop to active if either is selected
#    $base.chanBut.drop configure -state $stateStr
}

# session:Log
# This procedure logs a message about a session
# input  : sessionCounter  - Session's counter value (this is part of the window's handle
#          message      - The message to log
# output : none
# return : none
proc session:Log {sessionCounter message} {
    global tmp app

    # See if we want to display the message at all
    if {$app(options,suppressMessages) == 1} return

    set base ".session$sessionCounter"

    if {[winfo exists $base]} {
        $base.messages.txt insert end $message
    }

    test:Log "Session $sessionCounter: $message"
}


##############################################################################################
#
#   INCOMING CALL operations
#
##############################################################################################


# insession:create
# Creation procedure of an incoming session window.
# This window is opened whenever there's an incoming session to an application which is not set to
# receive the session in automatic mode
# input  : none
# output : none
# return : none
proc insession:create {} {
    global tmp app

    if {[winfo exists .insession]} {
        wm deiconify .insession
        .insession.buttons.ica configure -state normal
        return
    }

    ###################
    # CREATING WIDGETS
    ###################
    toplevel .insession -class Toplevel
    wm focusmodel .insession passive
    wm geometry .insession $app(insession,size)
    wm maxsize .insession 1028 753
    wm minsize .insession 150 150
    wm overrideredirect .insession 0
    wm resizable .insession 1 1
    wm title .insession "Incoming Session"
    wm transient .insession .dummy
    wm protocol .insession WM_DELETE_WINDOW {.insession.buttons.clos invoke}

    # Remote side params
    frame .insession.remote -borderwidth 2 -relief groove
    checkbutton .insession.remote.fs -state disabled -text "Fast Start"
    checkbutton .insession.remote.e245 -state disabled -text Early
    checkbutton .insession.remote.tun -state disabled -text Tunneling
    checkbutton .insession.remote.sendCmplt -state disabled -text "Sending Complete"
    checkbutton .insession.remote.canOvsp -state disabled -text "Can Overlap Send"

    # General session information
    frame .insession.info -borderwidth 2 -relief groove
    label .insession.info.uuiLab -text "User User Information" -width 18
    label .insession.info.uui -width 0 -relief sunken -anchor w
    label .insession.info.dispLab -borderwidth 0 -text Display -width 18
    label .insession.info.disp -width 0 -relief sunken -anchor w
    frame .insession.info.sourDest -height 75 -width 107
    scrollbar .insession.info.sourDest.scr -command {.insession.info.sourDest.lis yview}
    listbox .insession.info.sourDest.lis -height 12 -width 33 -yscrollcommand {.insession.info.sourDest.scr set} -background White

    # RTP/RTCP
#    if {[test.RtpSupported]} {
#        frame .insession.media -borderwidth 2 -relief groove
#        media:create .insession.media
#    }

    # Buttons frame
    frame .insession.buttons -height 75 -width 125
    label .insession.buttons.sessionInfo
    button .insession.buttons.clos -padx 0 -pady 0 -text Close -width 7 \
        -command {
                set app(insession,size) [winfo geometry .insession]
                Window close .insession
                focus .test
        }
    button .insession.buttons.wait -padx 0 -pady 0 -text Wait -width 7 \
        -command { H450.sessionWait [.insession.buttons.sessionInfo cget -text]
            .insession.buttons.clos invoke}
    button .insession.buttons.ica -padx 0 -pady 0 -text Incomplete -width 7 \
        -command {Session.IncompleteAddress [.insession.buttons.sessionInfo cget -text]
                .insession.buttons.clos invoke}
    button .insession.buttons.ac -padx 0 -pady 0 -text Complete -width 7 \
        -command {Session.AddressComplete [.insession.buttons.sessionInfo cget -text]
                .insession.buttons.clos invoke}
    button .insession.buttons.conn -padx 0 -pady 0 -text Connect -width 7 \
        -command {Session.SendConnect [.insession.buttons.sessionInfo cget -text]
                .insession.buttons.clos invoke}

    ###########
    # BINDINGS
    ###########
    bind .insession <Return> { .insession.buttons.clos invoke }
    bind .insession <Escape> { .insession.buttons.clos invoke }

    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf .insession 0 -weight 1
    grid rowconf .insession 0 -minsize 7
    grid rowconf .insession 1 -weight 1
    grid rowconf .insession 4 -minsize 4

    grid .insession.remote -in .insession -column 1 -row 1 -sticky nesw -pady 2 -padx 2
    grid columnconf .insession.remote 0 -weight 1
    grid rowconf .insession.remote 0 -minsize 5
    foreach i {1 2 3 4 5} {grid rowconf .insession.remote $i -weight 1}
    grid .insession.remote.fs -in .insession.remote -column 0 -row 0 -sticky w
    grid .insession.remote.e245 -in .insession.remote -column 0 -row 2 -sticky w
    grid .insession.remote.tun -in .insession.remote -column 0 -row 3 -sticky w
    grid .insession.remote.sendCmplt -in .insession.remote -column 0 -row 4 -sticky w
    grid .insession.remote.canOvsp -in .insession.remote -column 0 -row 5 -sticky w

    grid .insession.info -in .insession -column 0 -row 1 -sticky news -pady 2 -padx 2
    grid columnconf .insession.info 1 -weight 1
    grid rowconf .insession.info 3 -weight 1
    grid .insession.info.uuiLab -in .insession.info -column 0 -row 1
    grid .insession.info.uui -in .insession.info -column 1 -row 1 -ipady 1 -pady 6 -sticky ew -padx 2
    grid .insession.info.dispLab -in .insession.info -column 0 -row 2
    grid .insession.info.disp -in .insession.info -column 1 -row 2 -ipady 1 -pady 0 -sticky ew -padx 2
    grid .insession.info.sourDest -in .insession.info -column 0 -row 3 -columnspan 2 -pady 3 -padx 1 -sticky nesw
    grid columnconf .insession.info.sourDest 0 -weight 1
    grid rowconf .insession.info.sourDest 1 -weight 1
    grid .insession.info.sourDest.scr -in .insession.info.sourDest -column 1 -row 1 -sticky nesw
    grid .insession.info.sourDest.lis -in .insession.info.sourDest -column 0 -row 1 -pady 1 -sticky nesw

    set culspan 2

#    if {[test.RtpSupported]} {
#        grid .insession.media -in .insession -column 2 -row 1 -sticky nesw -pady 2 -padx 2
#        incr culspan
#    }

    grid .insession.buttons -in .insession -column 0 -row 6 -columnspan $culspan -sticky new
    foreach i {0 1} {grid columnconf .insession.buttons $i -weight 1}
    grid rowconf .insession.buttons 0 -weight 1
    pack .insession.buttons.conn -in .insession.buttons -fill both -padx 2 -side right
    pack .insession.buttons.ac   -in .insession.buttons -fill both -padx 2 -side right
    pack .insession.buttons.ica  -in .insession.buttons -fill both -padx 2 -side right
    if {[test.H450Supported] == 0} {
        pack .insession.buttons.wait -in .insession.buttons -fill both -padx 2 -side right
    }
    pack .insession.buttons.clos -in .insession.buttons -fill both -padx 2 -side right

    #########
    # OTHERS
    #########
    placeHeader .insession.info "Session Information"
    placeHeader .insession.remote "Remote"

#    if {[test.RtpSupported]} {
#        placeHeader .insession.media "Media channels"
#    }
}


# insession:clear
# Clear the incoming session window
# Called when window is closed
# input  : none
# output : none
# return : none
proc insession:clear {} {
    set base .insession

    $base.remote.fs deselect
    $base.remote.e245 deselect
    $base.remote.tun deselect
    $base.remote.sendCmplt deselect
    $base.remote.canOvsp deselect
    $base.info.uui configure -text ""
    $base.info.disp configure -text ""
    $base.info.sourDest.lis delete 0 end
}

# insession:terminate
# Terminates the incoming session window
# Called when a session is connected
# input  : sessionInfo string
# output : none
# return : none
proc insession:terminate { sessionInfo } {
    if {[winfo exists .insession]} {
        if { $sessionInfo == [.insession.buttons.sessionInfo cget -text] } {
            Window close .insession }
    }
}









##############################################################################################
#
#   MEDIA (RTP/RTCP) operations
#
##############################################################################################


# media:create
# Creation procedure for the media options available - record/replay etc.
# input  : base     - Base frame to work in
# output : none
# return : none
#proc media:create {base} {
#    global app
#
#    ###################
#    # CREATING WIDGETS
#    ###################
#    radiobutton $base.none -text "None" -value 0 -variable app(media,rtpMode)
#    radiobutton $base.play -text "Play" -value 1 -variable app(media,rtpMode) -state disabled
#    radiobutton $base.rec -text "Record" -value 2 -variable app(media,rtpMode) -state disabled
#    radiobutton $base.replay -text "Replay media" -value 3 -variable app(media,rtpMode)
#    radiobutton $base.recreplay -text "Record & Replay media" -value 4 -variable app(media,rtpMode) -state disabled
#    entry $base.filename -textvariable app(media,filename)
#    button $base.browse -padx 0 -pady 0 -text Browse -width 7 -command media:browse
#
#    ###################
#    # SETTING GEOMETRY
#    ###################
#    grid columnconf $base 1 -weight 1
#    foreach i {1 2 3 4 5} {grid rowconf $base $i -weight 1}
#    grid rowconf $base 0 -minsize 5
#    grid $base.none      -in $base -column 0 -row 1 -sticky w
#    grid $base.play      -in $base -column 0 -row 2 -sticky w
#    grid $base.rec       -in $base -column 0 -row 3 -sticky w -columnspan 2
#    grid $base.replay    -in $base -column 0 -row 4 -sticky w -columnspan 2
#    grid $base.recreplay -in $base -column 0 -row 5 -sticky w -columnspan 2
#    grid $base.filename  -in $base -column 1 -row 1 -sticky ew -padx 2
#    grid $base.browse    -in $base -column 1 -row 2 -sticky e  -padx 2
#
#    ###########
#    # BALLOONS
#    ###########
#    balloon:set $base.none "Don't do anything with the media on the channels"
#    balloon:set $base.rec "Record incoming media to a file"
#    balloon:set $base.play "Play a file on the outgoing media channels"
#    balloon:set $base.replay "Replay incoming media channels"
#    balloon:set $base.recreplay "Record incoming media to a file and replay them at once"
#
#    #########
#    # OTHERS
#    #########
#    $base.none select
#}
#
#
# media:browse
# Open file lise to select a file for read/write
# input  : none
# output : none
# return : none
#proc media:browse {} {
#    global tmp app
#
#    if {($app(media,rtpMode) == 0) || ($app(media,rtpMode) == 1)} {
#        # Open a file for playback
#        set app(media,filename) [tk_getOpenFile \
#            -initialfile $app(media,filename) \
#            -defaultextension "rec" \
#            -filetypes {{ "Recodrings" {*.rec} }} \
#            -title "Playback file"]
#    } else {
#        # Open a file for record
#        set app(media,filename) [tk_getSaveFile \
#            -initialfile $app(media,filename) \
#            -defaultextension "rec" \
#            -filetypes {{ "Recordings" {*.rec} }} \
#            -title "Record file"]
#    }
#}
