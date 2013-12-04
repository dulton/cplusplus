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
#  Tsahi Levent-Levi                        14-Aug-2000
#
##############################################################################################



##############################################################################################
#
#   ABOUT operations
#
##############################################################################################

# about:create
# Creation procedure of the About window
# This window is opened when Help|About is selected from the main menu
# input  : none
# output : none
# return : none
proc about:create {} {
    global tmp app

    if {[winfo exists .about]} {
        wm deiconify .about
        return
    }

    ###################
    # CREATING WIDGETS
    ###################
    toplevel .about -class Toplevel
    wm focusmodel .about passive
    wm geometry .about +350+400
    wm overrideredirect .about 0
    wm resizable .about 0 0
    wm deiconify .about
    wm title .about "About RADVISION H.323 Test Application"
    wm protocol .about WM_DELETE_WINDOW {
        .about.ok invoke
        focus .test
    }
    wm transient .about .dummy
    focus .dummy
    gui:centerWindow .about .test

    set addons {
        test.NetSupported "Network Protocols"
        test.SrtpSupported "SRTP"
    }

    text .about.info -cursor arrow -borderwidth 0 -background [.about cget -background] \
        -width 70 -height [expr 11+([llength $addons]/2)]
    .about.info tag configure header -font {Helvetica -14 bold} -foreground black -lmargin1 5
    .about.info tag configure normal -font {Helvetica -11}
    .about.info tag configure no -font {Helvetica -10}
    .about.info tag configure yes -font {Helvetica -11 bold}

    .about.info insert end "\nRTP Test Application\n" header
    .about.info insert end "$tmp(version)\n" normal
    .about.info insert end "\n"
    .about.info insert end "Copyright © RADVISION LTD.\n"
    .about.info insert end "\n" normal

    foreach {f n} $addons {
        set support 0
        catch {set support [eval $f]}
        if {$support != 0} {
            if {$support == "1" } {
                .about.info insert end "- $n supported\n" yes
            } else {
                .about.info insert end "- $n supported: $support\n" yes
            }
        } else {
            .about.info insert end "- $n not supported\n" no
        }
    }

    .about.info insert end "\n"
    .about.info configure -state disabled

    image create photo about -format gif -data $tmp(about)
    frame .about.pic -background black -borderwidth 0 -relief ridge
    label .about.pic.bmp -image about -borderwidth 0 -anchor s

    button .about.ok -text "OK" -pady 0 -width 8 -command {Window close .about
        focus .test}

    ###################
    # SETTING GEOMETRY
    ###################
    grid rowconf .about 0 -weight 1
    grid columnconf .about 0 -weight 1
    grid .about.info -in .about -column 0 -row 0 -sticky nesw -rowspan 2 -padx 5 -pady 5
    grid .about.pic -in .about -column 0 -row 0 -pady 8 -padx 8 -sticky ne
    grid .about.pic.bmp -in .about.pic -column 0 -row 0
    grid .about.ok -in .about -column 0 -row 1 -sticky se -padx 10 -pady 5

    ###########
    # BINDINGS
    ###########
    bind .about <Key-Return> {.about.ok invoke}
}

