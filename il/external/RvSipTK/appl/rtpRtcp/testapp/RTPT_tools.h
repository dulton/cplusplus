/***********************************************************************
        Copyright (c) 2002 RADVISION Ltd.
************************************************************************
NOTICE:
This document contains information that is confidential and proprietary
to RADVISION Ltd.. No part of this document may be reproduced in any
form whatsoever without written prior approval by RADVISION Ltd..

RADVISION Ltd. reserve the right to revise this publication and make
changes without obligation to notify any person of such revisions or
changes.
***********************************************************************/


/********************************************************************************************
 *                                TAP_tools.h
 *
 * Application tools handling
 *
 * 1. Status
 *      Status information about the stack (All the resources information)
 * 2. Hooks
 *      Stack hooks that are catched by the test application
 *
 ********************************************************************************************/



#ifndef _TAP_tools_H
#define _TAP_tools_H

#ifdef __cplusplus
extern "C" {
#endif



/********************************************************************************************
 * Options_GetLocalIP
 * purpose : Get the Local IP address of the machine
 * syntax  : Options.GetLocalIP
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : Local IP string
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Options_GetLocalIP(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * Options_Crash
 * purpose : Cause the test application to crash.
 * syntax  : Options.Crash
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : Local IP string
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Options_Crash(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * Status_Display
 * purpose : This function get the resources of Timers, ASN, RAS etc and displays them inside
 *           the status window
 * syntax  : Status.Display
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Status_Display(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/********************************************************************************************
 * Status_Check
 * purpose : This function gets the resources of Timers, ASN, RAS etc and compares them to
 *           the stored stack variables.
 * syntax  : status:Check <tpktSize> <messagesSize> <pvtSize> <timersSize>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Status_Check(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/********************************************************************************************
 * LogFile_Reset
 * purpose : This function calls ms Close and msOpen to restart the log with the current 
 *           rvtele.ini.
 * syntax  : LogFile:Reset
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int LogFile_Reset(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


#ifdef __cplusplus
}
#endif

#endif  /* _TAP_tools_H */

