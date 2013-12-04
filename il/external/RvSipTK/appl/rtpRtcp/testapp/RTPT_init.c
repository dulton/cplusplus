/***********************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
 *                                init.c
 *
 * This file include functions that initiate the stack and the tcl.
 *
 *
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

#include "RTPT_log.h"
#include "RTPT_init.h"
#include "RTPT_tools.h"
#include "RTPT_session.h"
#include "RTPT_wrapper.h"

#define TCL_FILENAME    "RTPT_testapp.tcl"
#define TCL_LIBPATH     "lib/tcl8.3"
#define TK_LIBPATH      "lib/tk8.3"

#ifdef WIN32
#define CONFIG_FILE     ".\\config.val"
#else
#define CONFIG_FILE     "./config.val"
#endif


/* Endpoint object used */
static rtptObj rtpt;
static rtptObj* pRtpt = &rtpt;

Tcl_Interp *interp;

RvBool       LogWrappers;

void* RVCALLCONV rtptGetRtptObj(void)
{
    return &rtpt;
}

/********************************************************************************************
 *                                                                                          *
 *                                  Private functions                                       *
 *                                                                                          *
 ********************************************************************************************/


/******************************************************************************
 * rtptAllocateResourceId
 * ----------------------------------------------------------------------------
 * General: Request user to allocate a resource identifier for an object.
 *
 * Return Value: Allocated resource on success, negative value on failure.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  rtpt           - Endpoint object to use.
 *         resourceType - Type of resource we need id for.
 * Output: None.
 *****************************************************************************/
static RvInt32 rtptAllocateResourceId(
    IN rtptObj                *rtpt,
    IN RtptResourceType       resourceType)
{
    static RvInt32 resCount[3] = {0, 0, 0};
    RvInt32 resValue = 0;

    RV_UNUSED_ARG(rtpt);

    switch (resourceType)
    {
    case RtptResourceChannel:
        TclExecute("incr tmp(totalChannels)");
        /* Fall through to the next case */

    case RtptResourceCall:
    case RtptResourceCompletionService:
        resCount[(int)resourceType]++;
        resValue = resCount[(int)resourceType];
        break;

    case RtptResourceInConnection:
        resValue = atoi(TclGetVariable("tmp(record,cnt,conn,in)"));
        break;

    case RtptResourceOutConnection:
        resValue = atoi(TclGetVariable("tmp(record,cnt,conn,out)"));
        break;
    }

    return resValue;
}


/********************************************************************************************
 * tclGetFile
 * purpose : This function is automatically generated with the tcl scripts array in
 *           RTPT_scripts.tcl.
 *           It returns the buffer holding the .tcl file information
 * input   : name   - Name of file to load
 * output  : none
 * return  : The file's buffer if found
 *           NULL if file wasn't found
 ********************************************************************************************/
char* tclGetFile(IN char* name);


/********************************************************************************************
 * test_Source
 * purpose : This function replaces the "source" command of the TCL
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK
 ********************************************************************************************/
int test_Source(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    FILE* exists;
    char* fileBuf;

    RV_UNUSED_ARG(clientData);

    if (argc != 2)
    {
        Tcl_SetResult(interp, (char *)"wrong # args: should be \"source <filename>\"", TCL_STATIC);
        return TCL_ERROR;
    }

    /* First see if we've got such a file on the disk */
    exists = fopen(argv[1], "r");
    if (exists == NULL)
    {
        /* File doesn't exist - get from compiled array */
        fileBuf = tclGetFile(argv[1]);
        if (fileBuf == NULL)
        {
            /* No such luck - we don't have a file to show */
            char error[300];
            sprintf(error, "file %s not found", argv[1]);
            Tcl_SetResult(interp, error, TCL_VOLATILE);
            return TCL_ERROR;
        }
        else
        {
            /* Found! */
            int retCode;
            retCode = Tcl_Eval(interp, fileBuf);
            if (retCode == TCL_ERROR)
            {
                char error[300];
                sprintf(error, "\n    (file \"%s\" line %d)", argv[1], interp->errorLine);
                Tcl_AddErrorInfo(interp, error);
            }
            return retCode;
        }
    }

    /* Close this file - we're going to read it */
    fclose(exists);

    /* File exists - evaluate from the file itself */
    return Tcl_EvalFile(interp, argv[1]);
}


/********************************************************************************************
 * test_File
 * purpose : This function replaces the "file" command of the TCL, to ensure that
 *           when checking if a file exists, we also look inside our buffers.
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK
 ********************************************************************************************/
int test_File(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int i, retCode;
    Tcl_DString str;

    RV_UNUSED_ARG(clientData);

    if ((argc == 3) && (strncmp(argv[1], "exis", 4)) == 0)
    {
        /* "file exist" command - overloaded... */
        if (tclGetFile(argv[2]) != NULL)
        {
            Tcl_SetResult(interp, (char *)"1", TCL_STATIC);
            return TCL_OK;
        }
    }

    /* Continue executing the real "file" command */
    Tcl_DStringInit(&str);
    Tcl_DStringAppendElement(&str, "fileOverloaded");
    for(i = 1; i < argc; i++)
        Tcl_DStringAppendElement(&str, argv[i]);
    retCode = Tcl_Eval(interp, Tcl_DStringValue(&str));
    Tcl_DStringFree(&str);
    return retCode;
}


/********************************************************************************************
 * test_Quit
 * purpose : This function is called when the test application is closed from the GUI.
 *           It is responsible for closing the application gracefully.
 * syntax  : test.Quit
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK
 ********************************************************************************************/
int test_Quit(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(argc);
    RV_UNUSED_ARG(argv);

    TclExecute("test:Log {Shutting down the application};update");
    TclExecute("init:SaveOptions 0");

    /* We should kill the stack only after we save the application's options */
    EndStack();
    
    TclEnd();
    /* Finish with the interpreter */
    Tcl_DeleteInterp(interp);
    exit(0);
    return TCL_OK;
}


/********************************************************************************************
 * test_Reset
 * purpose : This function resets global information it the endpoint object.
 * syntax  : test.Restart
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK
 ********************************************************************************************/
int test_Reset(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);
    RV_UNUSED_ARG(argv);

//    rtptReset(&rtpt);

    return TCL_OK;
}


/********************************************************************************************
 * test_Restart
 * purpose : This function restarts the stack while in the middle of execution.
 * syntax  : test.Restart
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK
 ********************************************************************************************/
int test_Restart(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);
    RV_UNUSED_ARG(argv);

    EndStack();

    TclExecute("after 5000 test.Init");

    return TCL_OK;
}
/********************************************************************************************
 * test_Init
 * purpose : This function initialized the stack as part of the restart operation.
 *           It should not be called from TCL scripts, but from test_Restart only
 * syntax  : test.Restart
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK
 ********************************************************************************************/
int test_Init(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);
    RV_UNUSED_ARG(argv);

    if (InitStack(NULL) < 0)
    {
        TclExecute("msgbox Error picExclamation {Unable to restart the stack. Stopping the application} { { Ok -1 <Key-Return> } }");
        TclExecute("test.Quit");
    }

    TclExecute("foreach tmpv $tmp(tvars) {set app($tmpv) $app($tmpv)};unset tmpv");

    TclExecute("test:updateGui");
    TclExecute("msgbox Restart picInformation {Stack restarted successfully} { { Ok -1 <Key-Return> } }");

    return TCL_OK;
}


/********************************************************************************************
 * BooleanStr
 * purpose : Return a string representing a boolean value
 * input   : value  - The value to convert
 * output  : none
 * return  : String value of the boolean
 ********************************************************************************************/
const RvChar* BooleanStr(IN RvBool value)
{
    switch (value)
    {
        case RV_TRUE:  return "1";
        case RV_FALSE: return "0";
        default:    return "-? ERROR ?-";
    }
}


/********************************************************************************************
 * test_Support
 * purpose : This function check for support of specific stack functionality
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK
 ********************************************************************************************/
int test_Support(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(argc);

    if (strcmp(argv[0], "test.NetSupported") == 0)
    {
#if (RV_NET_TYPE == RV_NET_IPV4)
        Tcl_SetResult(interp, (char*)"IPv4 only", TCL_STATIC);
#elif (RV_NET_TYPE == RV_NET_IPV6)
        Tcl_SetResult(interp, (char*)"IPv4/IPv6", TCL_STATIC);
#else
        Tcl_SetResult(interp, (char*)"0", TCL_STATIC);
#endif
        return TCL_OK;
    }
#ifdef SRTP_ADD_ON
    if (strcmp(argv[0], "test.SrtpSupported") == 0)
    {
        Tcl_SetResult(interp, (char*)"1", TCL_STATIC);
        return TCL_OK;
    }
#endif

    /* Not compiled with this support at all... */
    return TCL_ERROR;
}


#define WRAPPER_COMMAND(tclName, cName) \
{                                                                                                           \
    static TclWrapperData _data;                                                                            \
    _data.rtpt = &pRtpt;                                                                                         \
    _data.proc = cName;                                                                                     \
    Tcl_CreateCommand(interp, (char *)tclName, WrapperFunc, (ClientData)&_data, (Tcl_CmdDeleteProc *)NULL); \
}

#define CREATE_COMMAND(tclName, cName) \
    Tcl_CreateCommand(interp, (char *)tclName, cName, (ClientData)&rtpt, (Tcl_CmdDeleteProc *)NULL)



/********************************************************************************************
 * CreateTclCommands
 * purpose : Create all tcl commands that was written in c language.
 * input   : interp - interpreter for tcl commands
 * output  : none
 * return  : none
 ********************************************************************************************/
void CreateTclCommands(Tcl_Interp* interp)
{
    /********
     * TOOLS
     ********/
    CREATE_COMMAND("test.Quit",                     test_Quit);
    CREATE_COMMAND("test.Reset",                    test_Reset);
    CREATE_COMMAND("test.Restart",                  test_Restart);
    CREATE_COMMAND("test.Init",                     test_Init);
    CREATE_COMMAND("test.NetSupported",             test_Support);
    CREATE_COMMAND("test.SrtpSupported",            test_Support);
    CREATE_COMMAND("Log.Update",                    Log_Update);
    CREATE_COMMAND("Log:Reset",                     Log_Reset);
    CREATE_COMMAND("Options.GetLocalIP",            Options_GetLocalIP);
    CREATE_COMMAND("Options.Crash",                 Options_Crash);
    CREATE_COMMAND("Status.Display",                Status_Display);
    CREATE_COMMAND("status:Check",                  Status_Check);
    CREATE_COMMAND("LogFile:Reset",                 LogFile_Reset);

    /***********
     * SESSIONS
     ***********/
    CREATE_COMMAND("Session.Open",                  Session_Open);
    CREATE_COMMAND("Session.Close",                 Session_Close);
    CREATE_COMMAND("Session.AddRemoteAddr",         Session_AddRemoteAddr);
    CREATE_COMMAND("Session.RemoveRemoteAddr",      Session_RemoveRemoteAddr);
    CREATE_COMMAND("Session.RemoveAllRemoteAddrs",  Session_RemoveAllRemoteAddrs);

    CREATE_COMMAND("Session.ListenMulticast",       Session_ListenMulticast);
    CREATE_COMMAND("Session.SetMulticastTtl",       Session_SetMulticastTtl);
    CREATE_COMMAND("Session.SetTos",                Session_SetTos);
    CREATE_COMMAND("Session.SetPayload",            Session_SetPayload);
    CREATE_COMMAND("Session.SetBandwidth",          Session_SetBandwidth);
    CREATE_COMMAND("Session.StartRead",             Session_StartRead);
    CREATE_COMMAND("Address.DisplayAddressList",    Addrs_DisplayAddressList);

    CREATE_COMMAND("Session.SendSR",                Session_SendSR);
    CREATE_COMMAND("Session.SendRR",                Session_SendRR);
    CREATE_COMMAND("Session.SendBye",               Session_SendBye);
    CREATE_COMMAND("Session.Shutdown",              Session_Shutdown);
    CREATE_COMMAND("Session.SendAPP",               Session_SendAPP);

    CREATE_COMMAND("Session.SetEncryption",         Session_SetEncryption);

    /***********
     * SRTP
     ***********/
    CREATE_COMMAND("SessionSRTP.Construct",         SessionSRTP_Construct);
    CREATE_COMMAND("SessionSRTP.Init",              SessionSRTP_Init);
    CREATE_COMMAND("SessionSRTP.Destruct",          SessionSRTP_Destruct);
    CREATE_COMMAND("SessionSRTP.AddMK",             SessionSRTP_AddMK);
    CREATE_COMMAND("SessionSRTP.DelMK",             SessionSRTP_DelMK);
    CREATE_COMMAND("SessionSRTP.DelAllMK",          SessionSRTP_DelAllMK);
    CREATE_COMMAND("SessionSRTP.AddRemoteSrc",      SessionSRTP_AddRemoteSrc);
    CREATE_COMMAND("SessionSRTP.DelRemoteSrc",      SessionSRTP_DelRemoteSrc);
    CREATE_COMMAND("SessionSRTP.DelAllRemoteSrc",   SessionSRTP_DelAllRemoteSrc);
    CREATE_COMMAND("SessionSRTP.Notify",            SessionSRTP_Notify);
    CREATE_COMMAND("SessionSRTP.SetDestContext",    SessionSRTP_SetDestContext);
    CREATE_COMMAND("SessionSRTP.SetSourceContext",  SessionSRTP_SetSourceContext);
    CREATE_COMMAND("SessionSRTP.MessageDispatcher", SessionSRTP_MessageDispatcher);
    CREATE_COMMAND("SessionSRTP.SendIndexes",       SessionSRTP_SendIndexes);
    CREATE_COMMAND("SessionSRTP.SetMasterKeySizes", SessionSRTP_SetMasterKeySizes);
    CREATE_COMMAND("SessionSRTP.KeyDerivation",     SessionSRTP_KeyDerivation);
    CREATE_COMMAND("SessionSRTP.PrefixLength",      SessionSRTP_PrefixLength);
    CREATE_COMMAND("SessionSRTP.SetEncryption",     SessionSRTP_SetEncryption);
    CREATE_COMMAND("SessionSRTP.SetAuthentication", SessionSRTP_SetAuthentication);
    CREATE_COMMAND("SessionSRTP.SetKeySizes",       SessionSRTP_SetKeySizes);
    CREATE_COMMAND("SessionSRTP.ReplayListSizes",   SessionSRTP_ReplayListSizes);
    CREATE_COMMAND("SessionSRTP.SetHistory",        SessionSRTP_SetHistory);
    
    CREATE_COMMAND("SessionSRTP.SetKeyPool",        SessionSRTP_SetKeyPool);    
    CREATE_COMMAND("SessionSRTP.SetStreamPool",     SessionSRTP_SetStreamPool);    
    CREATE_COMMAND("SessionSRTP.SetContextPool",    SessionSRTP_SetContextPool);
    CREATE_COMMAND("SessionSRTP.SetKeyHash",        SessionSRTP_SetKeyHash);    
    CREATE_COMMAND("SessionSRTP.SetSourceHash",     SessionSRTP_SetSourceHash);    
    CREATE_COMMAND("SessionSRTP.SetDestHash",       SessionSRTP_SetDestHash);    
    
    Tcl_CreateCommand(interp, (char *)"script:cb", CallbackFunc, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);
    WRAPPER_COMMAND("test.Version", test_Version);
#if 0
    /************************
     * WRAPPER API FUNCTIONS
     ************************/
    Tcl_CreateCommand(interp, (char *)"script:cb", CallbackFunc, (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

    /* MS functions */
    WRAPPER_COMMAND("api:ms:GetLogFilename", api_ms_GetLogFilename);

    /* Application functions */
    WRAPPER_COMMAND("api:app:SetCallMode", api_app_SetCallMode);
    WRAPPER_COMMAND("api:app:SetChannelMode", api_app_SetChannelMode);
    WRAPPER_COMMAND("api:app:GetDataTypes", api_app_GetDataTypes);
    WRAPPER_COMMAND("api:app:ChannelKill", api_app_ChannelKill);
    WRAPPER_COMMAND("api:app:GetChannelList", api_app_GetChannelList);
    WRAPPER_COMMAND("api:app:Vt2Address", api_app_Vt2Address);
    WRAPPER_COMMAND("api:app:Address2Vt", api_app_Address2Vt);
    WRAPPER_COMMAND("api:app:FSGetChanNum", api_app_FSGetChanNum);
    WRAPPER_COMMAND("api:app:FSGetAltChanNum", api_app_FSGetAltChanNum);
    WRAPPER_COMMAND("api:app:FSGetChanIndex", api_app_FSGetChanIndex);
    WRAPPER_COMMAND("api:app:FSGetChanName", api_app_FSGetChanName);
    WRAPPER_COMMAND("api:app:FSGetChanRTCP", api_app_FSGetChanRTCP);
    WRAPPER_COMMAND("api:app:FSGetChanRTP", api_app_FSGetChanRTP);

    /* Annexes */
    WRAPPER_COMMAND("api:app:CallCreateAnnexMMessage", api_cm_CallCreateAnnexMMessage);
    WRAPPER_COMMAND("api:app:CallSendAnnexMMessage", api_cm_CallSendAnnexMMessage);
    WRAPPER_COMMAND("api:app:CallCreateAnnexLMessage", api_cm_CallCreateAnnexLMessage);
    WRAPPER_COMMAND("api:app:CallSendAnnexLMessage", api_cm_CallSendAnnexLMessage);
#endif /* 0 */
}






/********************************************************************************************
 *                                                                                          *
 *                                  Public functions                                        *
 *                                                                                          *
 ********************************************************************************************/


/********************************************************************************************
 * PutError
 * purpose : Notify the user about errors that occured
 * input   : title  - Title of the error
 *           reason - Reason that caused the error
 * output  : none
 * return  : none
 ********************************************************************************************/
void PutError(const char* title, const char* reason)
{
    static RvBool RTPT_inError = RV_FALSE;
    if ((reason == NULL) || (strlen(reason) == 0))
        reason = "Undefined error was encountered";

    fprintf(stderr, "%s: %s\n", title, reason);

    /* Make sure we don't get into an endless loop over this one */
    if (RTPT_inError) return;
    RTPT_inError = RV_TRUE;

#if defined (WIN32) && defined (_WINDOWS)
    if (interp == NULL)
        MessageBox(NULL, reason, title, MB_OK | MB_ICONERROR);
#endif  /* WIN32 */

    if (interp != NULL)
    {
        TclExecute("Log:Write {%s: %s}", title, reason);
        TclExecute("update;msgbox {%s} picExclamation {%s} {{Ok -1 <Key-Return>}}", title, reason);
    }

    RTPT_inError = RV_FALSE;
}


/********************************************************************************************
 * InitTcl
 * purpose : Initialize the TCL part of the test application
 * input   : executable     - Program executable
 *           versionString  - Stack version string
 * output  : reason         - Reason of failure on failure
 * return  : Tcl_Interp interpreter for tcl commands
 *           NULL on failure
 ********************************************************************************************/
Tcl_Interp* InitTcl(const char* executable, char* versionString, char** reason)
{
    static char strBuf[1024];
    int retCode;

    /* Find TCL executable and create an interpreter */
    Tcl_FindExecutable(executable);
    interp = Tcl_CreateInterp();

    if (interp == NULL)
    {
        *reason = (char*)"Failed to create Tcl interpreter";
        return NULL;
    }

    TclInit();
    /* Overload file and source commands */
    TclExecute("rename file fileOverloaded");
    CREATE_COMMAND("file", test_File);
    CREATE_COMMAND("source", test_Source);

    /* Reroute tcl libraries - we'll need this one later */
    /*TclSetVariable("tcl_library", TCL_LIBPATH);
    TclSetVariable("env(TCL_LIBRARY)", TCL_LIBPATH);
    TclSetVariable("tk_library", TK_LIBPATH);
    TclSetVariable("env(TK_LIBRARY)", TK_LIBPATH);*/

    /* Initialize TCL */
    retCode = Tcl_Init(interp);
    if (retCode != TCL_OK)
    {
        sprintf(strBuf, "Error in Tcl_Init: %s", Tcl_GetStringResult(interp));
        *reason = strBuf;
        TclEnd();
        Tcl_DeleteInterp(interp);
        return NULL;
    }

    /* Initialize TK */
    retCode = Tk_Init(interp);
    if (retCode != TCL_OK)
    {
        sprintf(strBuf, "Error in Tk_Init: %s", Tcl_GetStringResult(interp));
        *reason = strBuf;
        TclEnd();
        Tcl_DeleteInterp(interp);
        return NULL;
    }

    /* Set argc and argv parameters for the script.
       This allows us to work with C in the scripts. */
#ifdef RV_RELEASE
    retCode = TclExecute("set tmp(version) {%s Test Application (Release)}", versionString);
#elif RV_DEBUG
    retCode = TclExecute("set tmp(version) {%s Test Application (Debug)}", versionString);
#else
#error RV_RELEASE or RV_DEBUG must be defined!
#endif
    if (retCode != TCL_OK)
    {
        *reason = (char*)"Error setting stack's version for test application";
        return interp;
    }

    /* Create new commands that are used in the tcl script */
    CreateTclCommands(interp);

    Tcl_LinkVar(interp, (char *)"scriptLogs", (char *)&LogWrappers, TCL_LINK_BOOLEAN);

    /* Evaluate the Tcl script of the test application */
    retCode = Tcl_Eval(interp, (char*)"source " TCL_FILENAME);
    if (retCode != TCL_OK)
    {
        sprintf(strBuf, "Error reading testapp script (line %d): %s\n", interp->errorLine, Tcl_GetStringResult(interp));
        *reason = strBuf;
        return NULL;
    }

    /* Return the created interpreter */
    *reason = NULL;
    return interp;
}


/********************************************************************************************
 * InitStack
 * purpose : Initialize the H.323 stack
 * input   : configFile - Configuration file to use.
 * output  : none
 * return  : RV_Success on success
 *           other on failure
 ********************************************************************************************/
int InitStack(IN const RvChar *configFile)
{
    RvStatus            status;
    rtptCallbacks       cb;

    memset(&cb, 0, sizeof(cb));
    cb.rtptMalloc = rtptMalloc;
    cb.rtptFree = rtptFree;
    cb.rtptAllocateResourceId = rtptAllocateResourceId;
    cb.rtptLog = rtptLog;

    if (configFile == NULL)
        configFile = CONFIG_FILE;

    /* Initialize the stack */
    status = rtptStackInit(&rtpt);

    rtpt.cb.rtptEventIndication = (rtptEventIndication_CB) rtptEventIndication;

    if (status != RV_OK)
        return status;

	LogInit(&rtpt.rtptLogger);
    
    RvLogSourceConstruct((RvLogMgr*)rtpt.rtptLogger, &rtpt.appLogSource, "RTPT", "RTP TA");
    rtpt.appLogSourceInited = RV_TRUE;
    RvLogSourceSetMask(&rtpt.appLogSource, RV_LOGLEVEL_ERROR|RV_LOGLEVEL_EXCEP|RV_LOGLEVEL_WARNING|RV_LOGLEVEL_INFO);
    if (interp != NULL)
    {
        Tcl_CmdInfo info;
        RvChar fname[1024];
        RvChar *p;
        strncpy(fname, configFile, sizeof(fname));
        fname[sizeof(fname)-1] = '\0';
        p = fname;
        while (*p)
        {
            if (*p == '\\')
                *p = '/';
            p++;
        }
        TclExecute("set tmp(configFilename) %s", fname);

        Tcl_GetCommandInfo(interp, "status:Check", &info);
        info.clientData = (ClientData)&rtpt;
        Tcl_SetCommandInfo(interp, "status:Check", &info);
        Tcl_GetCommandInfo(interp, "status:Pvt", &info);
        info.clientData = (ClientData)&rtpt;
        Tcl_SetCommandInfo(interp, "status:Pvt", &info);

        TclExecute("test:SetStackStatus Running");
    }

    return 0;
}

/********************************************************************************************
 * EndStack
 * purpose : End the H.323 stack
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void EndStack(void)
{
    Tcl_CmdInfo info;

    TclExecute("Window close .status");

    Tcl_GetCommandInfo(interp, "status:Check", &info);
    info.clientData = NULL;
    Tcl_SetCommandInfo(interp, "status:Check", &info);
    Tcl_GetCommandInfo(interp, "status:Pvt", &info);
    info.clientData = NULL;
    Tcl_SetCommandInfo(interp, "status:Pvt", &info);

    rtptStackEnd(&rtpt);


    TclExecute("test:SetStackStatus Finished");
}



static char *tclVarUpdate(
    IN ClientData   clientData,
    IN Tcl_Interp   *interp,
    IN char         *name1,
    IN char         *name2,
    IN int          flags)
{
    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(name1);
    RV_UNUSED_ARG(name2);
    RV_UNUSED_ARG(flags);
#if 0
    if (strcmp(name2, "newcall,parallel") == 0)
    {
        rtpt.cfg.callConfig.parellel = atoi(TclGetVariable("app(newcall,parallel)"));
    }
    else if (strcmp(name2, "newcall,tunneling") == 0)
    {
        rtpt.cfg.callConfig.tunneling = atoi(TclGetVariable("app(newcall,tunneling)"));
    }
    else if (strcmp(name2, "newcall,efc") == 0)
    {
        rtpt.cfg.callConfig.efc = atoi(TclGetVariable("app(newcall,efc)"));
    }
    else if (strcmp(name2, "newcall,multiplexed") == 0)
    {
        rtpt.cfg.callConfig.multiplexing = atoi(TclGetVariable("app(newcall,multiplexed)"));
    }
    else if (strcmp(name2, "newcall,maintain") == 0)
    {
        rtpt.cfg.callConfig.maintainConnection = atoi(TclGetVariable("app(newcall,maintain)"));
    }
    else if (strcmp(name2, "newcall,canOvsp") == 0)
    {
        rtpt.cfg.callConfig.canOverlapSend = atoi(TclGetVariable("app(newcall,canOvsp)"));
    }
    else if (strcmp(name2, "newcall,autoAns") == 0)
    {
        rtpt.cfg.bAutoAnswer = (RvBool)atoi(TclGetVariable("app(newcall,autoAns)"));
    }
    else if (strcmp(name2, "newcall,fastStart") == 0)
    {
        rtpt.cfg.bSupportFaststart = (RvBool)atoi(TclGetVariable("app(newcall,fastStart)"));
        rtpt.cfg.bAutomaticFaststart = (RvBool)atoi(TclGetVariable("app(options,autoAccrtpttFs)"));
        if (!rtpt.cfg.bSupportFaststart)
            rtpt.cfg.bAutomaticFaststart = RV_FALSE;
    }
#endif /* #if0 */
    return NULL;
}


static void TraceTclVariable(IN const RvChar    *varName)
{
    Tcl_TraceVar2(interp, (char*)"app", (char*)varName, TCL_TRACE_WRITES, tclVarUpdate, NULL);

    TclExecute("if {[lsearch $tmp(tvars) %s] < 0} {lappend tmp(tvars) %s}", varName, varName);

    /* Make sure we get the initial value here */
    tclVarUpdate(NULL, interp, (char*)"app", (char*)varName, TCL_TRACE_WRITES);
}



/********************************************************************************************
 * InitApp
 * purpose : Initialize the test application
 *           This includes parts as RTP/RTCP support, etc.
 * input   : none
 * output  : none
 * return  : Non-negative value on success
 *           Negative value on failure
 ********************************************************************************************/
int InitApp(void)
{
    /* Set tracing on specific variables */
    TraceTclVariable("newcall,parallel");
    TraceTclVariable("newcall,tunneling");
    TraceTclVariable("newcall,efc");
    TraceTclVariable("newcall,multiplexed");
    TraceTclVariable("newcall,maintain");
    TraceTclVariable("newcall,canOvsp");
    TraceTclVariable("newcall,autoAns");
    TraceTclVariable("newcall,fastStart");

    TclExecute("test:updateGui");

    return 0;
}



#ifdef __cplusplus
}
#endif

