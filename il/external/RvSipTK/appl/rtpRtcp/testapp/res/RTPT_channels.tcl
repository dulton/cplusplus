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
#                                 RTPT_addresses.tcl
#
#   Addresses handling.
#   This file holds all the GUI procedures for addresses.
#   This includes new outgoing address, incoming address, address information, etc.
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
#   CHANNEL operations (general)
#
##############################################################################################


# address:createAddressesInfo
# Creation procedure for the addresses information INSIDE other windows.
# This function is called for the main window and for the call information windows.
# input  : base       - The base frame to create the addresses information in
#                       Each call should have a different base window handle
#          selectable - Indicates if items in listboxes can be selected
# output : none
# return : none
proc address:createAddressesInfo {base selectable} {
    global tmp app

    ###################
    # CREATING WIDGETS
    ###################
    listbox $base.inList -tooltip "Local Address" \
        -background White -height 1 -width 1 -exportselection 0 -selectmode single
    listbox $base.outList -tooltip "Remote Address" \
        -yscrollcommand "$base.scrl set" -background White -height 1 -width 1 -exportselection 0 -selectmode single
    listbox $base.mcList -tooltip "Multicast Address" \
        -background White -height 1 -width 1 -exportselection 0 -selectmode single
    scrollbar $base.scrl -command "address:yviewScrl $base"

    ###################
    # SETTING GEOMETRY
    ###################
    grid rowconf $base 0 -minsize 4
    grid rowconf $base 1 -weight 0
    grid rowconf $base 2 -weight 1
    grid rowconf $base 3 -weight 0
    grid columnconf $base 0 -weight 1

    grid $base.inList -in $base -column 0 -row 1 -sticky nesw -pady 1
    grid $base.outList -in $base -column 0 -row 2 -sticky nesw -pady 1
    grid $base.mcList -in $base -column 0 -row 3 -sticky nesw -pady 1
    grid $base.scrl -in $base -column 1 -row 1 -rowspan 3 -sticky ns -pady 1

    ###########
    # BINDINGS
    ###########
    if {$selectable} {
        bind $base.inList <<ListboxSelect>> "$base.outList selection clear 0 end; $base.mcList selection clear 0 end"
        bind $base.outList <<ListboxSelect>> "$base.inList selection clear 0 end; $base.mcList selection clear 0 end"
        bind $base.mcList <<ListboxSelect>> "$base.inList selection clear 0 end; $base.outList selection clear 0 end"
    } else {
        # Make sure we can't select anything
        bind $base.inList <<ListboxSelect>> "$base.inList selection clear 0 end"
        bind $base.outList <<ListboxSelect>> "$base.outList selection clear 0 end"
        bind $base.mcList <<ListboxSelect>> "$base.mcList selection clear 0 end"
    }
    placeHeader $base "Addresses: Local/Remote/Multicast"
}


# address:yviewScrl
# Set the listboxes view according to the scrollbar changes
# input  : base - Base frame widget holding the address lists
#          args - Arguments added to the procedure when a scrollbar event occurs
# output : none
# return : none
proc address:yviewScrl {base args} {
    eval "$base.outList yview $args"
}

