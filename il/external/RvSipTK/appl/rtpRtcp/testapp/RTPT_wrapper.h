#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
 *                                TAP_wrapper.h
 *
 *  This file contains all the functions that are used for
 * wrapping the CM api functions so we can use them as tcl commands.
 * This is done for writing scripts in tcl that have specific scenarios.
 *
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *      Oren Libis                          04-Mar-2000
 *
 ********************************************************************************************/


#ifndef _WRAPPER_H
#define _WRAPPER_H

#include "rtptObj.h"

#include <tcl.h>
#include <tk.h>

//#include "TAP_wrapper.h"


typedef struct 
{
    rtptObj     **rtpt; /* Terminal instance */
    Tcl_CmdProc *proc; /* Procedure to execute */
} TclWrapperData;



/********************************************************************************************
 * WrapperFunc
 * purpose : wrapping stack functions - this function is called whenever we wrap a
 *           function for the scripts' use
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int WrapperFunc(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * CallbackFunc
 * purpose : wrapping callback functions - this function is called whenever we wrap a
 *           callback function for the scripts' use
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int CallbackFunc(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);

/********************************************************************************************
 * WrapperBadParams
 * purpose : Return an error result due to bad function parameters
 * input   : interp     - Interperter to use
 *           usage      - The right parameters
 * output  : none
 * return  : TCL_OK or TCL_ERROR
 ********************************************************************************************/
int WrapperBadParams(Tcl_Interp* interp, const char* usage);

/* WRAPPING FUNCTIONS */

/********************************************************************************************
 * test_Version
 * purpose : wrapping the original RTP API function - RvRtpGetVersion
 * syntax  : test_Version
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully or TCL_ERROR if not.
 ********************************************************************************************/
int test_Version(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


#endif

#ifdef __cplusplus
}
#endif

