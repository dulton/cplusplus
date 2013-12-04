/************************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/


/********************************************************************************************
 *                                TAP_init.h
 *
 * This file is used for initiating the test application.
 * It also used for defining global variables and common functions.
 *
 *
 *
 *
 *      Written by                        Version & Date                        Change
 *     ------------                       ---------------                      --------
 *      Oren Libis                          04-Mar-2000
 *
 ********************************************************************************************/



#ifndef _INIT_H
#define _INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <tcl.h>
#include <tk.h>


#if (TCL_MAJOR_VERSION < 8)
#error TCL version is too old
#endif

#include "RTPT_defs.h"



extern RvBool       LogWrappers;



/********************************************************************************************
 * PutError
 * purpose : Notify the user about errors that occured
 * input   : title  - Title of the error
 *           reason - Reason that caused the error
 * output  : none
 * return  : none
 ********************************************************************************************/
void PutError(const char* title, const char* reason);


/********************************************************************************************
 * InitTcl
 * purpose : Initialize the TCL part of the test application
 * input   : executable     - Program executable
 *           versionString  - Stack version string
 * output  : reason         - Reason of failure on failure
 * return  : Tcl_Interp interpreter for tcl commands
 *           NULL on failure
 ********************************************************************************************/
Tcl_Interp* InitTcl(const char* executable, char* versionString, char** reason);

/********************************************************************************************
 * InitStack
 * purpose : Initialize the H.323 stack
 * input   : configFile - Configuration file to use.
 * output  : none
 * return  : RV_Success on success
 *           other on failure
 ********************************************************************************************/
int InitStack(IN const RvChar *configFile);

/********************************************************************************************
* EndStack
* purpose : End the H.323 stack
* input   : none
* output  : none
* return  : none
********************************************************************************************/
void EndStack(void);

/********************************************************************************************
 * InitApp
 * purpose : Initialize the test application
 *           This includes parts as RTP/RTCP support, etc.
 * input   : none
 * output  : none
 * return  : Non-negative value on success
 *           Negative value on failure
 ********************************************************************************************/
int InitApp(void);

/******************************************************************************
 * rtptGetRtptObj
 * ----------------------------------------------------------------------------
 * General: retrieves the pointer to the rtptObj.
 *
 * Return Value:  the pointer to the rtptObj.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  None.
 * Output: None.
 *****************************************************************************/
void* RVCALLCONV rtptGetRtptObj(void);

#ifdef __cplusplus
}
#endif

#endif

