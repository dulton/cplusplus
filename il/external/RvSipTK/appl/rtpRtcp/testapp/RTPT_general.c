

/***********************************************************************************************************************

Notice:
This document contains information that is proprietary to RADVISION LTD..
No part of this publication may be reproduced in any form whatsoever without
written prior approval by RADVISION LTD..

RADVISION LTD. reserves the right to revise this publication and make changes
without obligation to notify any person of such revisions or changes.

*************************************************************************************************************************/

/********************************************************************************************
 * General purpose functions
 *
 * This file provides general functions for the Test Application.
 * These functions are used for initiating the test application.
 * They contain no H.323 - specific include files for the header.
 *
 ********************************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

#include "rtptUtils.h"

#include "RTPT_init.h"
#include "RTPT_general.h"
#include "RTPT_threads.h"
/* Synchronization mechanisms. This allows other threads the access to the
   TCL wrapper commands that we have */

static int tclThreadId; /* ThreadId of the thread that is running the TCL interp */

#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)
#define TCL_COMMAND (WM_USER+200) /* Event used to update the GUI */
static HWND tclEventsWindow = NULL; /* Window that handles the TCL events for updating the GUI */

#else
#include <unistd.h>

int tclPipefd[2]; /* Pipe to use for synchronization of GUI events */

#endif /* (RV_OS_TYPE == RV_OS_TYPE_WIN32) */

/********************************************************************************************
 *                                                                                          *
 *                                  Private functions                                       *
 *                                                                                          *
 ********************************************************************************************/
#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)

/* GUI handling window */
static LRESULT CALLBACK appTclGuiWinFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_CLOSE:
        {
            /* Closing the window - just kill the damn timer and the window */
            KillTimer(hWnd, 1);
            DestroyWindow(hWnd);
            break;
        }

        case TCL_COMMAND:
        {
            /* Execute the TCL command and deallocate the memory it used */
            char *cmd = (char *)wParam;

            if (Tcl_GlobalEval(interp, cmd) != TCL_OK)
                PutError("ERROR: GlobalEval", cmd);
            free(cmd);
            break;
        }

        default:
            return DefWindowProc(hWnd,uMsg,wParam,lParam);
    }

    return 0L;
}


#else
/* We use a pipe event for GUI synchronization */

/* GUI handling window */
static void appTclGuiFunc(IN ClientData clientData, IN int mask)
{
    char* message;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(mask);
   
    /* Read message from the pipe */
    if (read(tclPipefd[0], &message, sizeof(message)) == sizeof(message))
    {
        if (Tcl_GlobalEval(interp, message) != TCL_OK)
            PutError("ERROR: GlobalEval", message);
        free(message);
    }
}


#endif

/********************************************************************************************
 * Hex2Byte
 * purpose : Convert a hex character to a byte value
 * input   : ch - Character to convert
 * output  : none
 * return  : Byte value
 ********************************************************************************************/
RvUint8 Hex2Byte(char ch)
{
    switch (tolower(ch))
    {
        case '0': return 0x0;
        case '1': return 0x1;
        case '2': return 0x2;
        case '3': return 0x3;
        case '4': return 0x4;
        case '5': return 0x5;
        case '6': return 0x6;
        case '7': return 0x7;
        case '8': return 0x8;
        case '9': return 0x9;
        case 'a': return 0xa;
        case 'b': return 0xb;
        case 'c': return 0xc;
        case 'd': return 0xd;
        case 'e': return 0xe;
        case 'f': return 0xf;
        default:
            return 0xff;
    }
}


/********************************************************************************************
 * Buffer2String
 * purpose : Convert a buffer of bytes into a readable string
 * input   : buffer - Buffer to convert
 *         : result - Resulting string holding the field
 *         : size   - Size of buffer
 * output  : none
 * return  : none
 ********************************************************************************************/
void Buffer2String(void* buffer, char* result, int size)
{
    static const char* hexChars = "0123456789abcdef";
    RvUint8* p = (RvUint8 *)buffer;
    int i;

    for (i = 0; i < size; i++)
    {
        result[i*2+1] = hexChars[p[i] & 0x0f];
        result[i*2] = hexChars[p[i] >> 4];
    }
    result[size*2] = '\0';
}


/********************************************************************************************
 * String2Buffer
 * purpose : Convert a readble string into a buffer struct
 * input   : string - String to convert
 *         : buffer - Resulting buffer struct
 *         : size   - Size of buffer
 * output  : none
 * return  : Number of bytes in buffer
 ********************************************************************************************/
int String2Buffer(char* string, void* buffer, int size)
{
    RvUint8* p = (RvUint8 *)buffer;
    int i;
    int len;
    len = strlen(string) / 2;
    if (len > size) len = size;

    for (i = 0; i < len; i++)
    {
        p[i] = (RvUint8)
            (Hex2Byte(string[i*2+1]) |
             ((RvUint8)(Hex2Byte(string[i*2]) << 4)));
    }
    return len;
}





/********************************************************************************************
 *                                                                                          *
 *                                  Public functions                                        *
 *                                                                                          *
 ********************************************************************************************/

 /******************************************************************************
 * termMalloc
 * ----------------------------------------------------------------------------
 * General: Dynamic allocation function to call (the equivalent of malloc()).
 *
 * Return Value: Pointer to allocated memory on success, NULL on failure.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  term         - Terminal object to use.
 *         size         - Size of allocation requested.
 * Output: None.
 *****************************************************************************/
void* termMalloc(
    IN rtptObj              *term,
    IN RvSize_t             size)
{
    void* p;

    p = malloc(size + sizeof(void*));

    if (p != NULL)
    {
        int* pBlock = (int *)p;
        *pBlock = size;

        term->curAllocs++;
        term->curMemory += (RvUint32)size;

        if (term->curAllocs > term->maxAllocs)
            term->maxAllocs = term->curAllocs;
        if (term->curMemory > term->maxMemory)
            term->maxMemory = term->curMemory;

        /*TclExecute("test:Log {alloc %d: %p}", size, p);*/

        return ((void *)((char *)p + sizeof(void*)));
    }

    return NULL;
}


/******************************************************************************
 * termFree
 * ----------------------------------------------------------------------------
 * General: Request user to allocate a resource identifier for an object.
 *
 * Return Value: RV_OK on success, other on failure.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  term         - Terminal object to use.
 *         ptr          - Pointer of memory to free.
 * Output: None.
 *****************************************************************************/
RvStatus termFree(
    IN rtptObj              *term,
    IN void                 *ptr)
{
    int* myAlloc;
    int size;

    myAlloc = (int *)((char *)ptr - sizeof(void*));
    size = *myAlloc;

    /*TclExecute("test:Log {free %d: %p}", size, myAlloc);*/

    free(myAlloc);

    term->curAllocs--;
    term->curMemory -= (RvUint32)size;

    return RV_OK;
}

/******************************************************************************************
 * TclInit
 * purpose : Initialize the TCL interface
 * input   : none
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ******************************************************************************************/
int TclInit(void)
{
#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)
    /* Create Window that we will use to synchronize Tcl commands */
    WNDCLASS wc;
    memset (&wc, 0, sizeof (WNDCLASS));
    wc.lpfnWndProc    = appTclGuiWinFunc;
    wc.hInstance      = NULL;
    wc.lpszClassName  = "appTclGui";

    RegisterClass(&wc);
    tclEventsWindow = CreateWindow("appTclGui", NULL,WS_OVERLAPPED | WS_MINIMIZE,
        0, 0, 0, 0, NULL, NULL, NULL, NULL);

#else
    /* Use TCL for events generation through a pipe */
    pipe(tclPipefd);

    Tcl_CreateFileHandler(tclPipefd[0], TCL_READABLE, appTclGuiFunc, NULL);

#endif

    tclThreadId = termThreadCurrent(NULL);

    return TCL_OK;
}


/******************************************************************************************
 * TclEnd
 * purpose : Deinitialize the TCL interface
 * input   : none
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ******************************************************************************************/
int TclEnd(void)
{
#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)
    /* Kill the associated window if there is one */
    if (tclEventsWindow != NULL)
    {
        SendMessage(tclEventsWindow, WM_CLOSE, (WPARAM)0, (LPARAM)0);
        tclEventsWindow = NULL;
    }

    UnregisterClass("appTclGui", NULL);

#else
    close(tclPipefd[0]);
    close(tclPipefd[1]);

#endif

    return TCL_OK;
}

/******************************************************************************************
 * TclExecute
 * purpose : execute a command in tcl
 * input   : cmd - the command that is going to be executed
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ******************************************************************************************/
int TclExecute(const char* cmd, ...)
{
    va_list v;
    char ptr[8096];

    va_start(v, cmd);
    vsprintf(ptr, cmd, v);
    va_end(v);

    if (tclThreadId == termThreadCurrent(NULL))
    {
        if (Tcl_GlobalEval(interp, ptr) != TCL_OK)
        {
            if ((strlen(ptr) + strlen(Tcl_GetStringResult(interp))) < sizeof(ptr))
                sprintf(ptr + strlen(ptr), ": %s", Tcl_GetStringResult(interp));
            PutError("ERROR: GlobalEval", ptr);
        }
    }
    else
    {
        /* Seems like we're not in the right thread... */
        char *message;
        RvSize_t len = strlen(ptr);
        message = (char *)malloc(len + 1);
        memcpy(message, ptr, len + 1);

#if (RV_OS_TYPE == RV_OS_TYPE_WIN32)
        PostMessage(tclEventsWindow, TCL_COMMAND, (WPARAM)message, (LPARAM)0);

#else
        write(tclPipefd[1], &message, sizeof(message));
#endif
    }

    return TCL_OK;
}


/******************************************************************************************
 * TclString
 * purpose : make a given string a printable TCL string
 * input   : line    - Line to convert
 * output  : none
 * return  : Converted string
 ******************************************************************************************/
char* TclString(const char* line)
{
    static char buf[2048];
    char* destPtr;
    char* srcPtr;
    srcPtr = (char *)line;
    destPtr = buf;

    while (*srcPtr)
    {
        switch (*srcPtr)
        {
            case '[':
            case ']':
            case '{':
            case '}':
            case '\\':
                *destPtr++ = '\\';
            default:
                break;
        }

        *destPtr = *srcPtr;
        destPtr++;
        srcPtr++;
    }

    *destPtr = '\0';
    return buf;
}


/******************************************************************************************
 * TclGetVariable
 * purpose : get variable from tcl
 * input   : varName - the name of the variable that is going to be imported
 * input   : none
 * output  : none
 * return  : The variable's string value
 ******************************************************************************************/
char* TclGetVariable(const char* varName)
{
    char arrayName[25];
    char indexName[128];
    char* token;
    char* result;

    /* See if it's an array variable */
    token = (char *)strchr(varName, '(');

    if (token == NULL)
        result = Tcl_GetVar(interp, (char *)varName, TCL_GLOBAL_ONLY);
    else
    {
        /* Array - let's first split it into array name and index name */
        int arrayLen = token - varName;
        int indexLen = strlen(token+1)-1;

        if ((arrayLen >= (int)sizeof(arrayName)) || (indexLen >= (int)sizeof(indexName)))
        {
            PutError(varName, "Length of array name or index out of bounds in GetVar");
            return (char *)"-unknown-";
        }

        memcpy(arrayName, varName, arrayLen);
        arrayName[arrayLen] = '\0';
        memcpy(indexName, token+1, indexLen);
        indexName[indexLen] = '\0';
        result = Tcl_GetVar2(interp, arrayName, indexName, TCL_GLOBAL_ONLY);
    }

    if (result == NULL)
    {
        PutError(varName, Tcl_GetStringResult(interp));
        return (char *)"-unknown-";
    }

    return result;
}


/******************************************************************************************
 * TclSetVariable
 * purpose : set variable in tcl
 * input   : varName - the name of the variable that is going to be set
 * input   : value - the value that is entered to the tcl variable
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ******************************************************************************************/
int TclSetVariable(const char* varName, const char* value)
{
    char arrayName[25];
    char indexName[128];
    char* token;
    char* result;

    /* See if it's an array variable */
    token = (char *)strchr(varName, '(');

    if (token == NULL)
        result = Tcl_SetVar(interp, (char *)varName, (char *)value, TCL_GLOBAL_ONLY);
    else
    {
        /* Array - let's first split it into array name and index name */
        int arrayLen = token - varName;
        int indexLen = strlen(token+1)-1;

        if ((arrayLen >= (int)sizeof(arrayName)) || (indexLen >= (int)sizeof(indexName)))
        {
            PutError(varName, "Length of array name or index out of bounds in SetVar");
            return TCL_OK;
        }

        memcpy(arrayName, varName, arrayLen);
        arrayName[arrayLen] = '\0';
        memcpy(indexName, token+1, indexLen);
        indexName[indexLen] = '\0';
        result = Tcl_SetVar2(interp, (char *)arrayName, (char *)indexName, (char *)value, TCL_GLOBAL_ONLY);
    }

    if (result == NULL)
        PutError(varName, Tcl_GetStringResult(interp));

    return TCL_OK;
}


/******************************************************************************
 * TclReturnValue
 * ----------------------------------------------------------------------------
 * General: Check the return value for a TCL wrapper function and set the
 *          error string accordingly.
 *
 * Return Value: Appropriate TCL return value.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  rtpt         - Endpoint object to use.
 *         status       - Returned status.
 * Output: None.
 *****************************************************************************/
int TclReturnValue(
    IN rtptObj        *rtpt,
    IN RvStatus     status)
{
    if (status < 0)
    {
        const RvChar *errStr = NULL;
        rtptUtilsGetError(rtpt, &errStr);
        if (errStr == NULL)
        {
            Tcl_SetResult(interp, "Unknown error...", TCL_VOLATILE);
        }
        else
        {
            Tcl_SetResult(interp, (RvChar *)errStr, TCL_VOLATILE);
        }
        return TCL_ERROR;
    }

    return TCL_OK;
}


/******************************************************************************
 * rtptMalloc
 * ----------------------------------------------------------------------------
 * General: Dynamic allocation function to call (the equivalent of malloc()).
 *
 * Return Value: Pointer to allocated memory on success, NULL on failure.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  rtpt           - Endpoint object to use.
 *         size         - Size of allocation requested.
 * Output: None.
 *****************************************************************************/
void* rtptMalloc(
    IN rtptObj                *rtpt,
    IN RvSize_t             size)
{
    void* p;

    p = malloc(size + sizeof(void*));

    if (p != NULL)
    {
        int* pBlock = (int *)p;
        *pBlock = size;
        if(rtpt!=NULL)
        {
            rtpt->curAllocs++;
            rtpt->curMemory += (RvUint32)size;
            
            if (rtpt->curAllocs > rtpt->maxAllocs)
                rtpt->maxAllocs = rtpt->curAllocs;
            if (rtpt->curMemory > rtpt->maxMemory)
                rtpt->maxMemory = rtpt->curMemory;
        }
        return ((void *)((char *)p + sizeof(void*)));
    }
    return NULL;
}


/******************************************************************************
 * rtptFree
 * ----------------------------------------------------------------------------
 * General: Request user to allocate a resource identifier for an object.
 *
 * Return Value: RV_OK on success, other on failure.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  rtpt           - Endpoint object to use.
 *         ptr          - Pointer of memory to free.
 * Output: None.
 *****************************************************************************/
RvStatus rtptFree(
    IN rtptObj                *rtpt,
    IN void                 *ptr)
{
    int* myAlloc;
    int size;

    myAlloc = (int *)((char *)ptr - sizeof(void*));
    size = *myAlloc;

    free(myAlloc);

    rtpt->curAllocs--;
    rtpt->curMemory -= (RvUint32)size;

    return RV_OK;
}




#ifdef __cplusplus
}
#endif

