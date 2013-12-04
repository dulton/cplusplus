set tmp(version) "1.0.0.0"


# Initialization procedure
proc init {argc argv} {
    global app tmp

    # Set some default values and other necessary variable values
    set app(port) 8000

    # load defaults
    catch {source "broadcaster.def"}

    gui:init
    conn:init
}


# Exit function to use
proc end {} {
    global app

    conn:end

    # save defaults
    catch {
        set f [open "broadcaster.def" w]
        foreach {vName vValue} [array get app] {
            #set vValue [string map { "\"" "\\\"" "\\" "\\\\" "\[" "\\\[" "\]" "\\\]" "\$" "\\\$" } $vValue]
            puts $f "set app($vName) {$vValue}"
        }
        close $f
    }

    # we're done
    exit
}


# Main application
proc main {argc argv} {
    global app

    # Display the main manager window
    win:mgr

    gui:log "Waiting on port $app(port)"
}




# Main manager window
proc win:mgr {} {
    global app tmp
    set base .bcast

    set tmp(w,log) "$base.clients.log"
    set tmp(w,idx) "$base.clients.idx"
    set tmp(w,socks) "$base.clients.sock"
    set tmp(w,addrs) "$base.clients.addr"
    set tmp(w,lst,socks) [list $tmp(w,idx) $tmp(w,socks) $tmp(w,addrs)]

    ###################
    # CREATING WIDGETS
    ###################
    toplevel $base -class Toplevel
    wm state $base withdrawn
    wm focusmodel $base passive
    catch {wm geometry $base $app($base)}
    wm overrideredirect $base 0
    wm resizable $base 0 0
    wm title $base "Broadcaster v$tmp(version)"
    wm protocol $base WM_DELETE_WINDOW "set app($base) \"+\[winfo x $base\]+\[winfo y $base\]\";end"

    frame $base.separator -borderwidth 2 -height 2 -relief sunken

    frame $base.clients -borderwidth 2 -relief flat
    label $base.clients.clientsLab -padx 1 -pady 1 -text "connected clients"
    foreach {i w l s} {
        0 idx     index       5
        1 sock    ""          10
        2 addr    address     40
    } {
        label $base.clients.${w}Lab -background darkblue -foreground white -pady 1 -padx 10 -text $l
        listbox $base.clients.$w -background white -exportselection 0 -takefocus 0 -height 4 -selectmode single \
            -yscrollcommand "$base.clients.scrl set" -width $s
        bind $base.clients.$w <<ListboxSelect>> "$base.clients.$w selection clear 0 end"
        grid $base.clients.${w}Lab -in $base.clients -column $i -row 1 -sticky w
        grid $base.clients.$w -in $base.clients -column $i -row 2 -sticky nesw
    }
    scrollbar $base.clients.scrl -command "gui:scrl [list $tmp(w,lst,socks)]"

    label $base.clients.messagesLab -padx 1 -pady 1 -text messages
    listbox $base.clients.log -background white -exportselection 0 -takefocus 0 -height 4 \
        -xscrollcommand "$base.clients.logScrlX set" -yscrollcommand "$base.clients.logScrlY set"
    scrollbar $base.clients.logScrlY -command "$base.clients.log yview"
    scrollbar $base.clients.logScrlX -command "$base.clients.log xview" -orient horizontal
    button $base.clients.logClear -borderwidth 0 -command "$tmp(w,log) delete 0 end" \
        -highlightthickness 0 -image bmpClear -padx 0 -pady 0 -takefocus 0

    ###################
    # SETTING GEOMETRY
    ###################
    grid columnconf $base 0 -weight 1
    grid rowconf $base 0 -minsize 2
    grid rowconf $base 2 -weight 1

    grid $base.separator -in $base -column 0 -row 0 -sticky ew

    grid $base.clients -in $base -column 0 -row 1 -pady 2 -sticky nesw
    grid columnconf $base.clients 3 -weight 1
    grid rowconf $base.clients 2 -weight 1
    grid rowconf $base.clients 4 -weight 1
    grid $base.clients.clientsLab -in $base.clients -column 0 -row 0 -columnspan 3 -padx 30 -sticky w
    grid configure $base.clients.sockLab -columnspan 7 -sticky ew
    grid $base.clients.scrl -in $base.clients -column 6 -row 2 -sticky ns

    grid $base.clients.messagesLab -in $base.clients -column 0 -row 3 -columnspan 3 -padx 30 -sticky w
    grid $base.clients.log -in $base.clients -column 0 -row 4 -columnspan 6 -sticky nesw
    grid $base.clients.logScrlY -in $base.clients -column 6 -row 4 -sticky ns
    grid $base.clients.logScrlX -in $base.clients -column 0 -row 5 -columnspan 6 -sticky ew
    grid $base.clients.logClear -in $base.clients -column 6 -row 5 -sticky nesw

    bind $base <<DeleteWindow>> "+end"

    wm deiconify $base
    wm state $base normal
}


proc gui:init {} {
    image create bitmap bmpClear -data "#define down_width 8 #define down_height 8
        static unsigned char down_bits[] = {
            0x00, 0x63, 0x36, 0x1c, 0x1c, 0x36, 0x63, 0x00
        };"

    # Set the basic window we shall use
    wm withdraw .
    wm focusmodel . passive
    wm geometry . 200x200+110+110; update
    wm maxsize . 1156 849
    wm minsize . 104 1
    wm overrideredirect . 0
    wm resizable . 1 1
    wm title . "Broadcaster"

    bind all <MouseWheel> "+_gui:WheelEvent %X %Y %D"
}


# Print a log message to the log
# msg           - Message to print
proc gui:log {msg} {
    global tmp
    $tmp(w,log) insert end "$msg"
    $tmp(w,log) see end
}


# Print an error log message to the log
# msg           - Message to print
proc gui:error {msg} {
    global tmp
    $tmp(w,log) insert end "$msg"
    $tmp(w,log) itemconfigure end -foreground red
    $tmp(w,log) see end
}


# Allow scrolling of several widgets at once.
# widgets           - List of widgets to scroll
# args              - Arguments passed by -yscrollcommand autmoatically
proc gui:scrl {widgets args} {
    foreach w $widgets {
        eval "$w yview $args"
    }
}


# Handle mouse wheel events
proc _gui:WheelEvent {x y delta} {
    # Find out what's the widget we're on
    set act 0

    set widget [winfo containing $x $y]

    while {$widget != ""} {
        # Make sure we've got a vertical scrollbar for this widget
        if {(![catch "$widget cget -yscrollcommand" cmd]) && ($cmd != "")} {
            # Find out the scrollbar widget we're using
            set scroller [lindex $cmd 0]

            # Make sure we act
            set act 1
            break
        }

        set widget [winfo parent $widget]
        if {$widget == "."} {set widget ""}
    }

    if {$act == 1} {
        # Now we know we have to process the wheel mouse event
        set xy [$widget yview]
        set factor [expr [lindex $xy 1]-[lindex $xy 0]]

        # Make sure we activate the scrollbar's command
        set cmd "[$scroller cget -command] scroll [expr -int($delta/(120*$factor))] units"
        eval $cmd
    }
}


# Initialize the connections module.
# This actually starts listening for incoming connections
proc conn:init {} {
    global app tmp

    set tmp(listeningSocket) [socket -server _conn:accept $app(port)]
}


# Close the connections module.
# This actually closes the listening socket we're using
proc conn:end {} {
    global tmp
    close $tmp(listeningSocket)
}


# get a free (low) ID for a client
proc getLowestFreeId {} {
    global tmp

    set i 1
    while {$i < 10000} {
        set n [array names tmp "idx,$i"]
        if {$n == ""} {
            set tmp(idx,$i) 1
            return $i
        }
        incr i
    }
    return -1
}


# Accept an incoming connection
# sock      - Accepted socket
# addr      - Address of remote side of the connection
# port      - Port of remote side of the connection
proc _conn:accept {sock addr port} {
    global tmp

    # Make sure that everything we send is sent immediatly
    fconfigure $sock -buffering line
    fconfigure $sock -blocking 0

    # Set up a callback for when we receive data
    fileevent $sock readable "_conn:incomingData $sock"

    array unset tmp "$sock,*"
    set idx [getLowestFreeId]
    set tmp($sock,addr) "$addr:$port"
    set tmp($sock,idx) "$idx"
    set tmp($sock,ip) "$addr"

    _conn:refreshData $sock
}


# Find if there's an existing connection by a given socket value
# Returns the index of the connection (-1 if not found)
proc _conn:find {sock} {
    global tmp
    set i 0
    foreach client [$tmp(w,socks) get 0 end] {
        if {$client == $sock} {return $i}
        incr i
    }
    return -1
}


# Handle incoming data on a connection.
# This proc is called automatically whenever there's data waiting on a connection.
# sock          - Socket with pending incoming data
proc _conn:incomingData {sock} {
    global tmp app test

    if {[eof $sock]} {
        # got disconnected from this client...
        set id $tmp($sock,idx)
        array unset tmp "idx,$id"

        set i [_conn:find $sock]
        if {$i >= 0} {
            foreach w $tmp(w,lst,socks) {
                $w delete $i
            }
        }
        close $sock

        gui:log "Client $sock got disconnected"
        return
    }

    set str [string trim [gets $sock]]
    if {$str == ""} return

    foreach s [$tmp(w,socks) get 0 end] {
        puts $s "$tmp($sock,idx) $str"
    }
}


# Refresh the data displayed in the GUI lists for a given connection
# sock          - Socket we want to refresh
proc _conn:refreshData {sock} {
    global tmp

    $tmp(w,idx) insert end $tmp($sock,idx)
    $tmp(w,socks) insert end $sock
    $tmp(w,addrs) insert end $tmp($sock,addr)
}





init $argc $argv
main $argc $argv

