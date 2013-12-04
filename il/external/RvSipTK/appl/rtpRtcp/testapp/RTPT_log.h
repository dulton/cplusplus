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
 *                                TAP_log.h
 *
 * Log API usage sample
 *
 * This file uses the log API of the stack as an example of how to use it.
 * Up to version 4 of the H.323 Protocol Stack, the internal logger also included a Window
 * logger on Win32 platforms. This logger is now part of the test application, and is not
 * implemented at all in the stack.
 *
 ********************************************************************************************/

#ifndef _TAP_log_H
#define _TAP_log_H

#include <tcl.h>
#include "rtptObj.h"

#ifdef __cplusplus
extern "C" {
#endif


/********************************************************************************************
 * LogInit
 * purpose : Initialize logging in the test application
 * input   : logmngr pointer to RTP log manager
 * output  : none
 * return  : none
 ********************************************************************************************/
void LogInit(RvRtpLogger *logmngr);


/********************************************************************************************
 * Log_Update
 * purpose : Set the log notification function or remove it.
 * syntax  : Log.Update <on/off>
 *           <on/off>       - "1" if we should catch logs, "0" if we shouldn't
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Log_Update(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


/********************************************************************************************
 * Log_Reset
 * purpose : This function reset the log error counters.
 * syntax  : Log:Reset
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Log_Reset(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/******************************************************************************
 * rtptLog
 * ----------------------------------------------------------------------------
 * General: Indication of a message that can be logged somewhere.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  rtpt             - Endpoint object to use.
 *         call             - Call this event belongs to (NULL if none).
 *         logMessage       - Log message.
 * Output: None.
 *****************************************************************************/
void rtptLog(
    IN rtptObj              *rtpt,
    IN rtptSessionObj       *call,
    IN const RvChar         *logMessage);



#ifdef __cplusplus
}
#endif


#endif  /* _TAP_log_H */

