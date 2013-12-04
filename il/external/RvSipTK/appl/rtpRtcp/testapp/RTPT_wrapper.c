/***********************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
 *                                TAP_wrapper.c
 *
 * This file contains all the functions that are used for
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


#ifdef __cplusplus
extern "C" {
#endif

#include "rtptObj.h"

#include <stdlib.h>
#include <ctype.h>
#include "RTPT_init.h"
#include "RTPT_general.h"
#include "RTPT_session.h"
#include "RTPT_wrapper.h"


RvBool LogWrappers = RV_FALSE;
extern Tcl_Interp* interp;
/*-----------------------------------------------------------------------*/
/*                           TYPE DEFINITIONS                            */
/*-----------------------------------------------------------------------*/

typedef struct
{
    const RvChar    *strValue;
    RvInt32         enumValue;
} ConvertValue;
/*-----------------------------------------------------------------------*/
/*                           STATIC VARIABLES                            */
/*-----------------------------------------------------------------------*/
static ConvertValue cvGenStatus[] = {
    {"Success",                 (RvInt32)RV_OK},
    {"Error",                   (RvInt32)RV_ERROR_UNKNOWN},
    {"Resources problem",       (RvInt32)RV_ERROR_OUTOFRESOURCES},
    {"Bad parameter passed",    (RvInt32)RV_ERROR_BADPARAM},
    {"Null pointer",            (RvInt32)RV_ERROR_NULLPTR},
    {"Out of range",            (RvInt32)RV_ERROR_OUTOFRANGE},
    {"Object destructed",       (RvInt32)RV_ERROR_DESTRUCTED},
    {"Operation not supported", (RvInt32)RV_ERROR_NOTSUPPORTED},
    {"Object uninitialized",    (RvInt32)RV_ERROR_UNINITIALIZED},
    {"Try again",               (RvInt32)RV_ERROR_TRY_AGAIN},
    {"Illegal action",          (RvInt32)RV_ERROR_ILLEGAL_ACTION},
    {"Network problem",         (RvInt32)RV_ERROR_NETWORK_PROBLEM},
    {"Invalid handle",          (RvInt32)RV_ERROR_INVALID_HANDLE},
    {"Not found",               (RvInt32)RV_ERROR_NOT_FOUND},
    {"Insufficient buffer",     (RvInt32)RV_ERROR_INSUFFICIENT_BUFFER},
    {NULL, -1}
};

/********************************************************************************************
*
*  General Functions
*
*********************************************************************************************/
/******************************************************************************
 * enum2String
 * ----------------------------------------------------------------------------
 * General: Convert an enumeration value into a string.
 *          Used as a general conversion function.
 *
 * Return Value: Parameter enumeration value.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  value      - Enumeration value to convert.
 *         cvTable    - Conversion table to use.
 * Output: None.
 *****************************************************************************/
static const RvChar* enum2String(
    IN RvInt32      value,
    IN ConvertValue *cvTable)
{
    ConvertValue* val = cvTable;

    while (val->strValue != NULL)
    {
        if (value == val->enumValue)
            return val->strValue;
        val++;
    }

    return "-unknown-";
}

const RvChar *Status2String(IN int status)
{
    static RvChar result[40];

    if (status >= 0)
        sprintf(result, "Success (%d)", status);
    else
    {
        const RvChar* strName = enum2String(RvErrorGetCode(status), cvGenStatus);
        if (strcmp(strName, "-unknown-") != 0)
            sprintf(result, "Failure (%s)", strName);
        else
            sprintf(result, "Failure (%d)", status);
    }

    return result;
}

int String2Status(IN const RvChar *string)
{
    if (strncmp("Success", string, 7) == 0)
        return 0;
    else
        return RV_ERROR_UNKNOWN;
}

/********************************************************************************************
 * WrapperEnter
 * purpose : Log wrapper function on entry
 * input   : interp     - Interpreter to use
 *           term       - Terminal object used.
 *           argc       - Number of arguments
 *           argv       - Argument values
 * output  : none
 * return  : TCL_OK or TCL_ERROR
 ********************************************************************************************/
int WrapperEnter(Tcl_Interp* interp, rtptObj *term, int argc, char *argv[])
{
    if (LogWrappers)
    {
        char buf[4096];
        char* ptr = buf+7;
        int i;
        strcpy(buf, "Enter: ");

        for (i = 0; i < argc; i++)
        {
            strcpy(ptr, argv[i]);
            ptr += strlen(ptr);
            *ptr = ' ';
            ptr++;
        }
        *ptr = '\0';

        /*RvRtpLogMessage(term->hApp, buf);*/
    }
    Tcl_SetResult(interp, NULL, TCL_STATIC);

    RV_UNUSED_ARG(term);
    return TCL_OK;
}


/********************************************************************************************
 * WrapperExit
 * purpose : Log wrapper function on exit
 * input   : interp     - Interpreter to use
 *           term       - Terminal object used.
 *           argc       - Number of arguments
 *           argv       - Argument values
 * output  : none
 * return  : TCL_OK or TCL_ERROR
 ********************************************************************************************/
int WrapperExit(Tcl_Interp* interp, rtptObj *term, int argc, char *argv[])
{
    if (LogWrappers)
    {
        char buf[4096];
        char* ptr = buf+6;
        int i;
        strcpy(buf, "Exit: ");

        for (i = 0; i < argc; i++)
        {
            strcpy(ptr, argv[i]);
            ptr += strlen(ptr);
            *ptr = ' ';
            ptr++;
        }
        *ptr = '\0';

        if (strlen(Tcl_GetStringResult(interp)) != 0)
            sprintf(ptr, " (%s)", Tcl_GetStringResult(interp));
/*        Rv3G324mLogMessage(term->hApp, buf);*/
    }
    RV_UNUSED_ARG(term);
    return TCL_OK;
}


/********************************************************************************************
 * WrapperBadParams
 * purpose : Return an error result due to bad function parameters
 * input   : interp     - Interpreter to use
 *           usage      - The right parameters
 * output  : none
 * return  : TCL_OK or TCL_ERROR
 ********************************************************************************************/
int WrapperBadParams(Tcl_Interp* interp, const char* usage)
{
    char buf[1024];
    sprintf(buf, "Bad parameters given. Usage: %s", usage);
    Tcl_SetResult(interp, buf, TCL_VOLATILE);
    return TCL_ERROR;
}


/********************************************************************************************
 * WrapperReply
 * purpose : Create and use the return result from the API and sent it back to TCL
 * input   : interp     - Interpreter to use
 *           status     - Return status got from API
 * output  : none
 * return  : TCL_OK or TCL_ERROR
 ********************************************************************************************/
int WrapperReply(Tcl_Interp* interp, int status, int argc, char *argv[])
{
    char buf[4096];
    char* ptr = buf;
    int i;
    Tcl_CmdInfo info;

    if (status >= 0)
        return TCL_OK;

    if (Tcl_GetCommandInfo(interp, (char *)"cb:app:Error", &info) == 0)
    {
        sprintf(buf, "Error executing %s - %s",
                argv[0], Status2String(status));
        /* Error command doesn't exist - we can throw an exception */
        Tcl_SetResult(interp, buf, TCL_VOLATILE);
        return TCL_ERROR;
    }

    /* We have to call the error function */
    for (i = 0; i < argc; i++)
    {
        strcpy(ptr, argv[i]);
        ptr += strlen(ptr);
        *ptr = ' ';
        ptr++;
    }
    *ptr = '\0';

    TclExecute("cb:app:Error {%s} {%s}",
               buf,
               Status2String(status));

    Tcl_SetResult(interp, (char *)"", TCL_STATIC);
    return TCL_OK;
}


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
int WrapperFunc(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    TclWrapperData  *wData = (TclWrapperData *)clientData;
    int             result = TCL_ERROR;

    WrapperEnter(interp, *(wData->rtpt), argc, argv);

    if (wData->proc != NULL)
        result = wData->proc((ClientData)(wData->rtpt), interp, argc, argv);

    WrapperExit(interp, *(wData->rtpt), argc, argv);

    return result;
}


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
int CallbackFunc(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    char* spacer;
    int result;
    Tcl_CmdInfo info;

    RV_UNUSED_ARG(clientData);

    if (argc != 2)
    {
        PutError(argv[0], "Callback wrapper called with too many arguments");
    }

    spacer = (char *)strchr(argv[1], ' ');

    if (spacer != NULL) *spacer = '\0';
    result = Tcl_GetCommandInfo(interp, argv[1], &info);
    *spacer = ' ';

    if (result == 0)
    {
        /* Callback command doesn't exist - skip it */
        return TCL_OK;
    }

    result = TclExecute(argv[1]);
    if (result == TCL_OK)
        Tcl_SetResult(interp, NULL, TCL_STATIC);

    return result;
}

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
int test_Version(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    const char* ver;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(argc);
    RV_UNUSED_ARG(argv);

    ver = RvRtpGetVersion();
    if (ver != NULL)
        Tcl_SetResult(interp, (char*)ver, TCL_VOLATILE);

    return WrapperReply(interp, 0, argc, argv);
}
