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
 *                                RTPT_log.c
 *
 * Log API usage sample
 *
 * This file uses the log API of the stack as an example of how to use it.
 * Up to version 4 of the H.323 Protocol Stack, the internal logger also included a Window
 * logger on Win32 platforms. This logger is now part of the test application, and is not
 * implemented at all in the stack.
 *
 ********************************************************************************************/

#include <assert.h>
#include "RTPT_log.h"
#include "RTPT_init.h"
#include "RTPT_general.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Counters for the types of messages we're interested in */
static int logWarnings = 0;
static int logExceptions = 0;
static int logErrors = 0;

static RvBool showLogging;
#if defined(RTPT_TA)
    FILE *f1=NULL;
#endif

#ifdef WIN32


/* Define some window-specific commands - their actual values is not important and can
   be changed if they interfere with other messages in the application */
#define LOGWND_PENDING  (WM_APP+102) /* Indication that there are probably pending messages */
#define LOGWND_CLEAR    (WM_APP+101) /* Clear window's contents */
#define LOGWND_KILL     (WM_APP+100) /* Destroy the window */

#define WINDOW_LISTENER_BUFFER_SIZE (100000) /* Buffer size to allocate for this logger */

typedef struct
{
    HWND                hWnd; /* Window displaying the log messages */
    WNDPROC             prevWinFunc; /* Pointer to the previous window handler function */
    RvChar*             bufferedMessages; /* Pointer to buffered messages space */
    RvUint32            bufferSize; /* Size of buffer we support */
    RvUint32            curUsedBufSize; /* Current size of buffer used */
    CRITICAL_SECTION    lock; /* Lock of pending messages buffer */
    DWORD               thread; /* Thread this window is running in */
} WindowListener;

static RvBool tapWindowInitialized = RV_FALSE;
static WindowListener tapWindowListener;


/* Window function of our logging window */
static LRESULT CALLBACK
WindowLoggerFunc(IN HWND hWnd, IN UINT uMsg, IN WPARAM wParam, IN LPARAM lParam)
{
    WindowListener* logInfo;

#ifdef GetWindowLongPtr
    /* 64bit support in Windows */
    logInfo = (WindowListener*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
#else
    logInfo = (WindowListener *)GetWindowLong(hWnd, GWL_USERDATA);
#endif

    switch(uMsg)
    {
        case WM_COMMAND:
        {
            if (LOWORD(wParam) == LOGWND_CLEAR)
                SendMessage(hWnd, LB_RESETCONTENT, 0, 0);
            else
            {
                /* Get the default window handler for this one */
                return CallWindowProc(logInfo->prevWinFunc, hWnd, uMsg, wParam, lParam);
            }
            break;
        }

        case WM_CLOSE:
        {
            ShowWindow(hWnd, SW_HIDE);
            break;
        }

        case LOGWND_PENDING:
        {
            EnterCriticalSection(&logInfo->lock);

            if (logInfo->curUsedBufSize > 0)
            {
                int safetyCounter;
                RvUint32 processed = 0;
                size_t len;
                RvChar* logMessage = logInfo->bufferedMessages;

                /* Let's send all the messages we currently store */
                while (processed < logInfo->curUsedBufSize)
                {
                    safetyCounter = 0;

                    /* If there's not enough room for the new line - remove older ones from the list */
                    while ((SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM)logMessage) == LB_ERRSPACE) &&
                        (safetyCounter++ < 10))
                        SendMessage(hWnd, LB_DELETESTRING, 0, 0);

                    len = strlen(logMessage) + 1;
                    processed += len;
                    logMessage += len;

                }

                /* We're done with it */
                logInfo->curUsedBufSize = 0;
            }

            LeaveCriticalSection(&logInfo->lock);
        }
        break;

        case LOGWND_KILL:
            {
                DestroyWindow(logInfo->hWnd);
                return 0l;
            }

        default:
            return CallWindowProc(logInfo->prevWinFunc, hWnd, uMsg, wParam, lParam);
    }

    return 0l;
}


/********************************************************************************************
 * WindowLoggerThread
 *
 * purpose : Thread function for the thread that will create and activate the log window
 * input   : lpParam - Window listener struct used
 * output  : None
 * return  : 0
 ********************************************************************************************/
static DWORD WINAPI WindowLoggerThread(IN LPVOID lpParam)
{
    WindowListener* wndListener = (WindowListener *)lpParam;
    HINSTANCE hInstance;
    MENUITEMINFO mii;
    HMENU menu;

    /* Create a window with Clear option in the menu */
    memset(&mii, 0, sizeof(mii));
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_TYPE | MIIM_ID;
    mii.fType = MFT_STRING;
    mii.wID = LOGWND_CLEAR;
    mii.dwItemData = LOGWND_CLEAR;
    mii.dwTypeData = "Clear";
    mii.cch = 5;
    menu = CreateMenu();
    InsertMenuItem(menu, 0, TRUE, &mii);

    hInstance = (HINSTANCE)GetModuleHandle(NULL);

    wndListener->hWnd =
        CreateWindow("LISTBOX", "TestApp Debug Window", WS_OVERLAPPEDWINDOW|WS_VSCROLL|WS_HSCROLL|LBS_NOINTEGRALHEIGHT,
            CW_USEDEFAULT, CW_USEDEFAULT, 300, 400, NULL, menu, hInstance, NULL);

#ifdef GetWindowLongPtr
    /* 64bit support in Windows */
    wndListener->prevWinFunc = (WNDPROC)GetWindowLongPtr(wndListener->hWnd, GWLP_WNDPROC);
    SetWindowLongPtr(wndListener->hWnd, GWLP_WNDPROC, (LONG_PTR)WindowLoggerFunc);
    SetWindowLongPtr(wndListener->hWnd, GWLP_USERDATA, (LONG_PTR)wndListener);
#else
    wndListener->prevWinFunc = (WNDPROC)GetWindowLong(wndListener->hWnd, GWL_WNDPROC);
    SetWindowLong(wndListener->hWnd, GWL_WNDPROC, (LONG)WindowLoggerFunc);
    SetWindowLong(wndListener->hWnd, GWL_USERDATA, (LONG)wndListener);
#endif
    SendMessage(wndListener->hWnd, WM_SETFONT, (WPARAM)GetStockObject(SYSTEM_FIXED_FONT), 0);
    SendMessage(wndListener->hWnd, LB_SETCURSEL, 0, 0l);

    /* Display the debug window we created */
    ShowWindow(wndListener->hWnd, SW_SHOW);
    UpdateWindow(wndListener->hWnd);


    /* Loop until eternity */
    {
        MSG msg;
        BOOL res;

        while ((res = GetMessage(&msg, NULL, 0, 0)) != 0)
        {
            if (res == -1)
            {
                /* We've got an error - exit this darn loop */
                break;
            }
            else
            {
                DispatchMessage(&msg);
            }

            if (tapWindowInitialized == RV_FALSE)
                break; /* No need for this thread anymore */
        }
    }

    return 0;
}


/********************************************************************************************
 * CreateWindowLogger
 *
 * purpose : Create the windows logger and start activating it
 * input   : None
 * output  : None
 * return  : None
 ********************************************************************************************/
static void CreateWindowLogger(void)
{
    HANDLE th;

    if (tapWindowInitialized)
        return;

    memset(&tapWindowListener, 0, sizeof(tapWindowListener));

    /* Allocate a lock */
    InitializeCriticalSection(&tapWindowListener.lock);

    /* Allocate a buffer for pending messages */
    tapWindowListener.bufferedMessages = (RvChar*)rtptMalloc(NULL, WINDOW_LISTENER_BUFFER_SIZE);
    tapWindowListener.bufferSize = WINDOW_LISTENER_BUFFER_SIZE;

    /* Create the darn window in another thread so it doesn't get in our way */
    th = CreateThread(NULL, 0, WindowLoggerThread, &tapWindowListener, 0, &tapWindowListener.thread);
    CloseHandle(th);

    tapWindowInitialized = RV_TRUE;
}




/********************************************************************************************
 * KillWindowLogger
 *
 * purpose : Deallocate the windows logger
 * input   : None
 * output  : None
 * return  : None
 ********************************************************************************************/
static void KillWindowLogger(void)
{
    if (!tapWindowInitialized)
        return;

    SendMessage(tapWindowListener.hWnd, LOGWND_KILL, 0, 0);

    /* Deallocate the lock */
    DeleteCriticalSection(&tapWindowListener.lock);

    /* Free the buffer */
    rtptFree(NULL, tapWindowListener.bufferedMessages);

    tapWindowInitialized = RV_FALSE;
}


#endif  /* WIN32 */


#if (RV_LOGMASK != RV_LOGLEVEL_NONE)
static void checkForBadMessages(IN const RvChar* message)
{
    static int typePos = -1;

    if (typePos < 0)
    {
        /* First time around - find out the place of the type in the message.
           We do that instead of hard-code it to be more resilient to changes
           between versions and patches. */
        RvChar *p;
        p = strstr(message, ": DEBUG  -");
        if (p == NULL)
            p = strstr(message, ": INFO  -");
        if (p == NULL)
            p = strstr(message, ": ENTER  -");
        if (p == NULL)
            p = strstr(message, ": LEAVE  -");
        if (p == NULL)
            p = strstr(message, ": WARN  -");
        if (p == NULL)
            p = strstr(message, ": ERROR  -");
        if (p == NULL)
            p = strstr(message, ": EXCEP  -");
        if (p == NULL)
            p = strstr(message, ": SYNC  -");
        assert(p != NULL);

        typePos = (p - message) + 2;
    }

    if (message[typePos] == 'W')
        logWarnings++;
    else if (message[typePos+1] == 'X')
        logExceptions++;
    else if (message[typePos+1] == 'R')
        logErrors++;
    else
    {
        /* Nothing more to do in this function */
        TclExecute("test:Log {%s}", message + typePos);
        return;
    }

    TclExecute("test:SetLoggerStatus %d %d %d", logWarnings, logExceptions, logErrors);
    TclExecute("test:Error {%s}", message + typePos);
}


static void Log_Print(IN char* line, ...)
{
    RvChar message[1024];
    va_list v;

    va_start(v, line);
#ifndef WIN32
    vsnprintf(message, sizeof(message), line, v);
#else
    _vsnprintf(message, sizeof(message), line, v);
#endif
    message[sizeof(message)-1] = '\0';
    va_end(v);

    /* First, let's see if we have to deal with any "special" messages */
    checkForBadMessages(message);

    if (showLogging)
    {
#ifndef WIN32
        /* Not windows - just use printf to terminal */
        printf("%s\n", message);
#else
        int length;

        EnterCriticalSection(&tapWindowListener.lock);

        if (tapWindowListener.curUsedBufSize + 2000 > WINDOW_LISTENER_BUFFER_SIZE)
        {
            /* Seems like we might finish too much work here */
            LeaveCriticalSection(&tapWindowListener.lock);
            SendMessage(tapWindowListener.hWnd, LOGWND_PENDING, 0, 0);
            EnterCriticalSection(&tapWindowListener.lock);
        }

        /* Now let's put this string inside the buffer */
        length = strlen(message);
        strcpy(tapWindowListener.bufferedMessages + tapWindowListener.curUsedBufSize, message);

        /* Move the pointer... */
        tapWindowListener.curUsedBufSize += length;
        assert(tapWindowListener.curUsedBufSize <= WINDOW_LISTENER_BUFFER_SIZE);

        /* We're done... */
        LeaveCriticalSection(&tapWindowListener.lock);
        PostMessage(tapWindowListener.hWnd, LOGWND_PENDING, 0, 0);
#endif  /* WIN32 */
    }
}
#endif

void RVCALLCONV RvRtpLogPrintFunc(
    IN void*           context,
    IN RvRtpLoggerFilter filter,
    IN const RvChar   *formattedText)
{
    Log_Print((RvChar*)formattedText);
#if defined(RTPT_TA)
    fprintf(f1,"%s\n", formattedText);
    fflush(f1);
#endif
    RV_UNUSED_ARG(filter);
    RV_UNUSED_ARG(context);	
}
/********************************************************************************************
 * LogInit
 * purpose : Initialize logging in the test application
 * input   : none
 * output  : none
 * return  : none
 ********************************************************************************************/
void LogInit(RvRtpLogger *logmngr)
{
    /* Set the stack's logging callback to the application's event handler */
/*
    msSetStackNotify(Log_Print);
*/
#if defined(RTPT_TA)
    if ((f1= fopen("rtplog.txt","wt+")) == NULL)
    {
        Log_Print((RvChar*)"Cannot open log file");
        return;
    }
#endif
	RvRtpCreateLogManager(logmngr);
    RvRtpSetPrintEntryCallback(RvRtpLogPrintFunc, (void*)logmngr);

    RvRtpSetLogModuleFilter(RVRTP_RTP_MODULE,  RVRTP_LOG_ERROR_FILTER|RVRTP_LOG_EXCEP_FILTER|RVRTP_LOG_INFO_FILTER);
    RvRtpSetLogModuleFilter(RVRTP_RTCP_MODULE, RVRTP_LOG_ERROR_FILTER|RVRTP_LOG_EXCEP_FILTER|RVRTP_LOG_INFO_FILTER);
    RvRtpSetLogModuleFilter(RVRTP_SRTP_MODULE, RVRTP_LOG_ERROR_FILTER|RVRTP_LOG_EXCEP_FILTER|RVRTP_LOG_INFO_FILTER);    
   
}


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
int Log_Update(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    int wantToLog = atoi(argv[1]);

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);

    if (wantToLog)
    {
#ifdef WIN32
        CreateWindowLogger();
#endif
        showLogging = RV_TRUE;
    }
    else
    {
        /* We never disable the callback function. */
        /*msSetStackNotify(NULL);*/
#ifdef WIN32
        KillWindowLogger();
#endif
        showLogging = RV_FALSE;
    }

    TclExecute("test:SetLoggerStatus %d %d %d", logWarnings, logExceptions, logErrors);

    return TCL_OK;
}


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
int Log_Reset(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);
    RV_UNUSED_ARG(argv);

    logWarnings = 0;
    logExceptions = 0;
    logErrors = 0;

    TclExecute("test:SetLoggerStatus %d %d %d", logWarnings, logExceptions, logErrors);

    return TCL_OK;
}


/******************************************************************************
 * rtptLog
 * ----------------------------------------------------------------------------
 * General: Indication of a message that can be logged somewhere.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  rtpt               - Endpoint object to use.
 *         call             - Call this event belongs to (NULL if none).
 *         logMessage       - Log message.
 * Output: None.
 *****************************************************************************/
void rtptLog(
    IN rtptObj                *rtpt,
    IN rtptSessionObj            *call,
    IN const RvChar         *logMessage)
{
    RV_UNUSED_ARG(rtpt);

    if (call != NULL)
    {
        TclExecute("call:Log %d {%s}", call->id, logMessage);
    }
    else
    {
        TclExecute("test:Log {%s}", logMessage);
    }
}



#ifdef __cplusplus
}
#endif

