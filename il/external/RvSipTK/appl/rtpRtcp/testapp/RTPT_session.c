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
 *                                RTPT_tools.c
 *
 * Application tools handling
 *
 * 1. Status
 *      Status information about the stack (All the resources information)
 * 2. Hooks
 *      Stack hooks that are catched by the test application
 *
 ********************************************************************************************/
#include "rvtypes.h" /* for  RvMin */
#include "rvstdio.h" /* for  stricmp */
#include "rv64ascii.h"

#include "RTPT_init.h"
#include "RTPT_general.h"
#include "RTPT_session.h"
#include "rtptSession.h"
#include "rtptSessionSRTP.h"

#if defined(SRTP_ADD_ON)
#include "rv_srtp.h"
/* Master Key defaults */
#define RTPT_SRTP_MAX_MKISIZE            (RV_SRTP_DEFAULT_MKISIZE*2)
#define RTPT_SRTP_MAX_MASTERKEYSIZE      (RV_SRTP_DEFAULT_MASTERKEYSIZE*2)
#define RTPT_SRTP_MAX_SALTSIZE           (RV_SRTP_DEFAULT_SALTSIZE*2)
#else
#define RTPT_SRTP_MAX_MKISIZE           4
#define RTPT_SRTP_MAX_MASTERKEYSIZE     16
#define RTPT_SRTP_MAX_SALTSIZE          14
#endif

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************************************
 *                                                                                          *
 *                                  Private functions                                        *
 *                                                                                          *
 ********************************************************************************************/

/*******************************************************************************************
 * UpdateSessionNum
 * Description: Maintain the number of current and total session.
 * Input : raise - true = ++; false = --
 * Output: none
 * return: none
 ******************************************************************************************/
static void UpdateSessionNum(RvBool raise)
{
    static RvUint32 curSessions = 0;
    static RvUint32 totalSessions = 0;

    if (raise)
    {
        curSessions++;
        totalSessions++;
    }
    else
        curSessions--;
    TclExecute("test:SetSessions %d %d", curSessions, totalSessions);
}


/********************************************************************************************
 * DisplayAddrsList
 * purpose : Display the list of channels inside the channels window by the selected call
 * input   : rtpt   - RTP Test object
 *           sid    - index if the session in the array
 *           widget - Listbox widget to fill with channel's information
 *                    If set to NULL, the default main channels list is used
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
void DisplayAddrsList(
    IN rtptObj      *rtpt,
    IN int          sid,
    IN const RvChar *widget)
{
    const RvChar *chanWidget;
    RvChar ipAddress[64];
    int i;

    if (widget == NULL)
    {
        char * result;
        chanWidget = ".test.addrs";

        /* Make sure the selected call is the one that we're updating */
        TclExecute(".test.sess.list selection includes [Session.FindSessionIndex %d]", sid);
        result = Tcl_GetStringResult(interp);
        if (result[0] == '0')
            return;
    }
    else
        chanWidget = widget;

    /* Delete channels list */
    TclExecute("%s.inList delete 0 end\n"
               "%s.outList delete 0 end\n"
               "%s.mcList delete 0 end\n",
               chanWidget, chanWidget, chanWidget);

    /* Display listening address information */
    TclExecute("%s.inList insert end {%s:%d}", chanWidget,
        RvAddressGetString(&rtpt->session[sid].localAddr, sizeof(ipAddress), ipAddress),
        RvAddressGetIpPort(&rtpt->session[sid].localAddr));
    TclExecute("balloon:set %s.inList {Listening Address}", chanWidget);

    for (i=0; i<rtpt->session[sid].numRemoteAddrs; i++)
    {
        /* Display remote address information */
        TclExecute("%s.outList insert end {%s:%d}", chanWidget,
            RvAddressGetString(&rtpt->session[sid].remoteAddrs[i], sizeof(ipAddress), ipAddress),
            RvAddressGetIpPort(&rtpt->session[sid].remoteAddrs[i]));
    }
    if (rtpt->session[sid].numRemoteAddrs > 1)
        TclExecute("balloon:set %s.outList {Remote Addresses (%d)}", chanWidget, rtpt->session[sid].numRemoteAddrs);
    else
        TclExecute("balloon:set %s.outList {Remote Address}", chanWidget);

    /* Display multicast listening address information */
    if (RvAddressGetIpPort(&rtpt->session[sid].mcastAddr) != 0)
    {
        TclExecute("%s.mcList insert end {%s:%d}", chanWidget,
            RvAddressGetString(&rtpt->session[sid].mcastAddr, sizeof(ipAddress), ipAddress),
            RvAddressGetIpPort(&rtpt->session[sid].mcastAddr));
        TclExecute("balloon:set %s.mcList {Multicast Address}", chanWidget);
    }
}


RvStatus setAddressFromString(
    IN  RvChar *addrString,
    OUT RvAddress *addr)
{
    RvUint32 port32 = 0;
    RvUint16 port = 0;
    int c;
    int result = -1;

    if ((result = sscanf(addrString, "%d.%d.%d.%d:%d",&c,&c,&c,&c,&c)) == 5)
    {
        /* has IPv4 format */
        RvAddressConstruct(RV_ADDRESS_TYPE_IPV4, addr);

        /* get to the ':' before the port */
        for (c=0; (addrString[c] != ':') && (addrString[c] != '\0'); c++);
        if (addrString[c] == '\0')
        {
            RvAddressDestruct(addr);
            return RV_ERROR_UNKNOWN;
        }

        /* set the ip */
        addrString[c] = '\0';
        if (RvAddressSetString(addrString, addr) == NULL)
        {
            RvAddressDestruct(addr);
            addrString[c] = ':';
            return RV_ERROR_UNKNOWN;
        }

        /* get the port */
        sscanf(addrString+c+1, "%u", &port32);
        port = (RvUint16) port32;
        RvAddressSetIpPort(addr, port);
        addrString[c] = ':';
        return RV_OK;
    }
#if (RV_NET_TYPE & RV_NET_IPV6)
    else if (addrString[0] == '[')
    {
        RvUint32 scope = 0;
        RvInt32 prev=0;
        /* has IPv6 format */
        RvAddressConstruct(RV_ADDRESS_TYPE_IPV6, addr);

        /* get to the ':' before the port */
        for (c=0; (addrString[c] != '|') && (addrString[c] != '\0'); c++);
        if (addrString[c] == '\0')
        {
            RvAddressDestruct(addr);
            return RV_ERROR_UNKNOWN;
        }

        /* set the ip */
        addrString[c] = '\0';
        if (RvAddressSetString(addrString+1, addr) == NULL)
        {
            RvAddressDestruct(addr);
            addrString[c] = '|';
            return RV_ERROR_UNKNOWN;
        }
        addrString[c] = '|';
        prev = c+1;
        /* get the port */

        for (c+=1; (addrString[c] != '/') && (addrString[c] != '\0'); c++);
        if (addrString[c] == '/')
        {
            addrString[c] = '\0';
            sscanf(addrString+prev, "%d", &port32);
            RvAddressSetIpPort(addr, (RvUint16)port32);
            addrString[c] = '/';
        }
        else
        {
            addrString[c] = '\0';
            sscanf(addrString+prev, "%d", &port32);
            RvAddressSetIpPort(addr, (RvUint16)port32);
            RvAddressSetIpv6Scope(addr, 0);
            return RV_OK;
        }
        /* skip '/' before the scope */
        prev = c+1;
        sscanf(addrString+prev, "%d", &scope);
        RvAddressSetIpv6Scope(addr, scope);
        return RV_OK;
    }
#endif

    return RV_ERROR_UNKNOWN;
}

/********************************************************************************************
 *                                                                                          *
 *                                  Public functions                                        *
 *                                                                                          *
 ********************************************************************************************/


/********************************************************************************************
 * session_Open
 * purpose : Open a new session
 * syntax  : Session.Open <address>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : Local IP string
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_Open(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvChar ipString[64];
    int sid;
    RvStatus res;
    RvUint16 seqNum = 0;
    RvUint32 ssrc = 0;
    RvBool bUseSsrc = RV_FALSE;

    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);

    for (sid=0; sid<RTP_TEST_APPLICATION_SESSIONS_MAX; sid++)
    {
        if (!rtpt->session[sid].open)
            break;
    }

    if (sid == RTP_TEST_APPLICATION_SESSIONS_MAX)
        return TCL_ERROR;

    /* get the listening address */
    res = setAddressFromString(argv[1], &rtpt->session[sid].localAddr);
    if (res != RV_OK)
        return TCL_ERROR;

    if (atoi(TclGetVariable("app(force,bPsn)")) != 0)
    {
        seqNum = (RvUint16)atoi(TclGetVariable("app(force,psn)"));
    }
    bUseSsrc = (atoi(TclGetVariable("app(force,bSsrc)")) != 0);
    if (bUseSsrc)
    {
        RvUint64 num;
        Rv64AstoRv64(&num, TclGetVariable("app(force,ssrc)"));
        ssrc = RvUint64ToRvUint32(num);
    }

    /* open the session using the address we just got */
    res = rtptSessionOpen(&rtpt->session[sid],
        RvAddressGetIpPort(&rtpt->session[sid].localAddr),
        RvAddressGetString(&rtpt->session[sid].localAddr,sizeof(ipString), ipString),
        (RvUint32)RvAddressGetIpv6Scope(&rtpt->session[sid].localAddr),
        seqNum, bUseSsrc, ssrc);

    if (res != RV_OK)
        return TCL_ERROR;

    TclExecute("Session:ChangeState %d \"%d \"", sid, sid);
    UpdateSessionNum(RV_TRUE);

    return TCL_OK;
}

/* closes GUI objects of the session */
int GUI_Session_Close(IN rtptSessionObj* session)
{
    rtptObj* rtpt  = session->rtpt;
    RvUint32 index = (session - &rtpt->session[0])/(&rtpt->session[1]- &rtpt->session[0]);
    
    if ((index >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[index].open))
        return TCL_ERROR;

    /* delete the SRTP entries belonging to this session */
    TclExecute("srtpCleanSession {%d S}", index);
    /* Make sure we clean the call from the main window */
    TclExecute("set index [Session.FindSessionIndex %d]\n"
               ".test.sess.list delete $index\n",
               index);
    /* Close all windows that belong to that session */
    TclExecute("Session:cleanup %d", index);
    /* Delete the session's information window */
    TclExecute("Window delete .sess%d", index);
    /* Clear the address lists */
    TclExecute(".test.addrs.inList delete 0 end");
    TclExecute(".test.addrs.outList delete 0 end");

    UpdateSessionNum(RV_FALSE);
    return TCL_OK;
}

/********************************************************************************************
 * Session_Close
 * purpose : Close a given session.
 * syntax  : Session.Close <sid>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : Local IP string
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_Close(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvStatus res;
    int sid;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);

    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    /* delete the SRTP entries belonging to this session */
    TclExecute("srtpCleanSession {%s}", argv[1]);
    /* Make sure we clean the call from the main window */
    TclExecute("set index [Session.FindSessionIndex %d]\n"
               ".test.sess.list delete $index\n",
               sid);
    /* Close all windows that belong to that session */
    TclExecute("Session:cleanup %d", sid);
    /* Delete the session's information window */
    TclExecute("Window delete .sess%d", sid);
    /* Clear the address lists */
    TclExecute(".test.addrs.inList delete 0 end");
    TclExecute(".test.addrs.outList delete 0 end");

    UpdateSessionNum(RV_FALSE);

	res = rtptSessionClose(&rtpt->session[sid]);
    if (res != RV_OK)
        return TCL_ERROR;

    return TCL_OK;
}


/********************************************************************************************
 * Session_AddRemoteAddr
 * purpose : Add a remote address to the session.
 * syntax  : Session.AddRemoteAddr <sid> <address>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_AddRemoteAddr(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvStatus res;
    int sid;
    RvChar ipString[64];
    RvChar* strPrev = argv[2], *strDelim = argv[2]; 
    RvAddress *remAddr;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);

    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    if (rtpt->session[sid].numRemoteAddrs == RTPT_MAX_REMOTE_ADDRS)
        return TCL_ERROR;
    do 
    {
        strDelim=strstr(strPrev,";");
        if (strDelim!=NULL)
            *strDelim='\0';
        remAddr = &rtpt->session[sid].remoteAddrs[rtpt->session[sid].numRemoteAddrs];
        /* get the remote address */
        res = setAddressFromString(strPrev, remAddr);
        if (res != RV_OK)
            return TCL_ERROR;
        
        /* Set the remote address we just got */
        res = rtptSessionAddRemoteAddr(&rtpt->session[sid],
            RvAddressGetIpPort(remAddr),
            RvAddressGetString(remAddr, sizeof(ipString), ipString),
            (RvUint32)RvAddressGetIpv6Scope(remAddr));
        
        if (res != RV_OK)
        {
            RvAddressDestruct(remAddr);
            return TCL_ERROR;
        }
        
        rtpt->session[sid].numRemoteAddrs++;
        DisplayAddrsList(rtpt, sid, NULL);
        if (strDelim!=NULL)
        {
           *strDelim=';';
           strPrev = strDelim+1;
        }
    }
    while (strPrev!=NULL&&strDelim!=NULL);
    
    return TCL_OK;
}


/********************************************************************************************
 * Session_RemoveRemoteAddr
 * purpose : Remove a remote address from the session.
 * syntax  : Session.RemoveRemoteAddr <sid> <index>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_RemoveRemoteAddr(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvStatus res;
    int sid, addrInd;
    RvChar ipString[64];
    RvAddress *remAddr;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);

    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;
    if ((sscanf(argv[2], "%d", &addrInd) != 1) || (addrInd >= rtpt->session[sid].numRemoteAddrs))
        return TCL_ERROR;

    remAddr = &rtpt->session[sid].remoteAddrs[addrInd];

    /* remove the indicated address from the session */
    res = rtptSessionRemoveRemoteAddr(&rtpt->session[sid],
        RvAddressGetIpPort(remAddr),
        RvAddressGetString(remAddr, sizeof(ipString), ipString));
    if (res != RV_OK)
        return TCL_ERROR;

    RvAddressDestruct(remAddr);
    rtpt->session[sid].numRemoteAddrs--;
    if (rtpt->session[sid].numRemoteAddrs != addrInd)
    {
        memmove(remAddr, remAddr+sizeof(RvAddress),
            (rtpt->session[sid].numRemoteAddrs - addrInd)*sizeof(RvAddress));
    }
    DisplayAddrsList(rtpt, sid, NULL);
    return TCL_OK;
}


/********************************************************************************************
 * Session_RemoveAllRemoteAddrs
 * purpose : Remove all the remote addresses from the session.
 * syntax  : Session.RemoveAllRemoteAddrs <sid>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_RemoveAllRemoteAddrs(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvStatus res;
    int sid, i;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);

    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    /* remove all remote addresses from the session */
    res = rtptSessionRemoveAllRemoteAddresses(&rtpt->session[sid]);
    if (res != RV_OK)
        return TCL_ERROR;

    for (i=0; i<rtpt->session[sid].numRemoteAddrs; i++)
    {
        RvAddressDestruct(&rtpt->session[sid].remoteAddrs[i]);
    }
    rtpt->session[sid].numRemoteAddrs = 0;
    DisplayAddrsList(rtpt, sid, NULL);
    return TCL_OK;
}

/********************************************************************************************
 * Session_SetPayload
 * purpose : Set a session payload.
 * syntax  : Session.SetPayload <sid> <payload>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_SetPayload(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvStatus res;
    int sid;
    RvUint32 payload;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);

    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    if (sscanf(argv[2], "%d", &payload) != 1)
        return TCL_ERROR;

	res = rtptSessionSetPayload(&rtpt->session[sid], payload);
    if (res != RV_OK)
        return TCL_ERROR;

    return TCL_OK;
}


/********************************************************************************************
 * Session_StartRead
 * purpose : Start reading on the session.
 * syntax  : Session.StartRead <sid> <blocked>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_StartRead(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvStatus res;
    int sid, blocked;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);

    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    if (sscanf(argv[2], "%d", &blocked) != 1)
        return TCL_ERROR;

	res = rtptSessionStartRead(&rtpt->session[sid], (RvBool)blocked);
    if (res != RV_OK)
        return TCL_ERROR;

    return TCL_OK;
}


/********************************************************************************************
 * Addrs_DisplayAddressList
 * purpose : Display the list of addresses inside the addresses window by the selected call
 * syntax  : Address.DisplayAddressList <callInfo> <frameWidget>
 *           <callInfo>     - Call information (counter and handle)
 *           <frameWidget>  - Frame widget holding channel listboxes
 * input   : clientData - used for creating new command in tcl
 *           interp - interpreter for tcl commands
 *           argc - number of parameters entered to the new command
 *           argv - the parameters entered to the tcl command
 * output  : none
 * return  : TCL_OK - the command was invoked successfully.
 ********************************************************************************************/
int Addrs_DisplayAddressList(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    char* widget;
    int sid;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    /* Make sure we have a call related to the channel */
    if ((argv[1] == NULL) || (strlen(argv[1]) == 0)) return TCL_OK;

    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    /* Check if we have the second paremeter (listbox widget to display in) */
    if ((argc < 2) || (argv[2] == NULL) || (strlen(argv[2]) == 0))
        widget = NULL;
    else
        widget = argv[2];

    /* Display the channels */
    DisplayAddrsList(rtpt, sid, widget);

    return TCL_OK;
}


/********************************************************************************************
 * Session_SendSR
 * purpose : Send Manual RTCP SR on the session.
 * syntax  : Session.SendSR <sid>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_SendSR(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvStatus res;
    int sid;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);

    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    res = rtptSessionSendManualRTCPSR(&rtpt->session[sid]);
    if (res != RV_OK)
        return TCL_ERROR;

    return TCL_OK;
}


/********************************************************************************************
 * Session_SendRR
 * purpose : Send Manual RTCP RR on the session.
 * syntax  : Session.SendRR <sid>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_SendRR(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvStatus res;
    int sid;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);

    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    res = rtptSessionSendManualRTCPRR(&rtpt->session[sid]);
    if (res != RV_OK)
        return TCL_ERROR;

    return TCL_OK;
}


/********************************************************************************************
 * Session_SendBye
 * purpose : Send Manual RTCP immediate BYE on the session.
 * syntax  : Session.SendBye <sid> <reason>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_SendBye(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvStatus res;
    int sid;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);

    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    res = rtptSessionSendManualBYE(&rtpt->session[sid], argv[2]);
    if (res != RV_OK)
        return TCL_ERROR;

    return TCL_OK;
}


/********************************************************************************************
 * Session_Shutdown
 * purpose : Send Manual RTCP BYE on the session after apropriate delay.
 * syntax  : Session.Shutdown <sid> <reason>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_Shutdown(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvStatus res;
    int sid;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);

    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    res = rtptSessionSendShutdown(&rtpt->session[sid], argv[2]);
    if (res != RV_OK)
        return TCL_ERROR;

    return TCL_OK;
}


/********************************************************************************************
 * Session_SendAPP
 * purpose : Send RTCP APP on the session.
 * syntax  : Session.SendAPP <sid>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_SendAPP(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvStatus res;
    int sid;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);

    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    res = rtptSessionSendRTCPAPP(&rtpt->session[sid]);
    if (res != RV_OK)
        return TCL_ERROR;

    return TCL_OK;
}


/********************************************************************************************
 * Session_ListenMulticast
 * purpose : Joins the RTP and RTCP session to listen to multicast address.
 * syntax  : Session.ListenMulticast <sid> <address>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_ListenMulticast(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvStatus res;
    int sid;
    RvChar ipString[64];
    RvAddress *addr;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(argc);

    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    addr = &rtpt->session[sid].mcastAddr;

    /* get the listening address */
    res = setAddressFromString(argv[2], addr);
    if (res != RV_OK)
        return TCL_ERROR;

    /* Listen to the multicast address we just got */
    res = rtptSessionListenMulticast(&rtpt->session[sid],
        RvAddressGetIpPort(addr),
        RvAddressGetString(addr, sizeof(ipString), ipString),
        (RvUint32)RvAddressGetIpv6Scope(addr));

    if (res != RV_OK)
    {
        RvAddressDestruct(&rtpt->session[sid].mcastAddr);
        memset(&rtpt->session[sid].mcastAddr, 0, sizeof(rtpt->session[sid].mcastAddr));
        return TCL_ERROR;
    }
    DisplayAddrsList(rtpt, sid, NULL);
    return TCL_OK;
}


/********************************************************************************************
 * Session_SetMulticastTtl
 * purpose : Sets multicast time to live for RTP and RTCP.
 * syntax  : Session.SetMulticastTtl <sid> <rtpTtl> <rtcpTtl>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_SetMulticastTtl(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvStatus res;
    int sid, rtpTtl, rtcpTtl;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 4)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;
    if (sscanf(argv[2], "%d", &rtpTtl) != 1)
        return TCL_ERROR;
    if (sscanf(argv[3], "%d", &rtcpTtl) != 1)
        return TCL_ERROR;

    res = rtptSessionSetRtpMulticastTTL(&rtpt->session[sid], rtpTtl);
    if (res != RV_OK)
        return TCL_ERROR;
    res = rtptSessionSetRtcpMulticastTTL(&rtpt->session[sid], rtcpTtl);
    if (res != RV_OK)
        return TCL_ERROR;

    return TCL_OK;
}


/********************************************************************************************
 * Session_SetTos
 * purpose : Sets multicast type of service for RTP and RTCP.
 * syntax  : Session.SetTos <sid> <rtpTos> <rtcpTos>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_SetTos(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvStatus res;
    int sid, rtpTos, rtcpTos;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 4)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;
    if (sscanf(argv[2], "%d", &rtpTos) != 1)
        return TCL_ERROR;
    if (sscanf(argv[3], "%d", &rtcpTos) != 1)
        return TCL_ERROR;

    res = rtptSessionSetRtpTOS(&rtpt->session[sid], rtpTos);
    if (res != RV_OK)
        return TCL_ERROR;
    res = rtptSessionSetRtcpTOS(&rtpt->session[sid], rtcpTos);
    if (res != RV_OK)
        return TCL_ERROR;

    return TCL_OK;
}


/********************************************************************************************
 * Session_SetBandwidth
 * purpose : Sets bandwidth for RTCP.
 * syntax  : Session.SetBandwidth <sid> <bandwidth>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_SetBandwidth(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvStatus res;
    int sid, bw;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 3)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;
    if (sscanf(argv[2], "%d", &bw) != 1)
        return TCL_ERROR;

    res = rtptSessionSetRtcpBandwith(&rtpt->session[sid], bw);
    if (res != RV_OK)
        return TCL_ERROR;

    return TCL_OK;
}


/********************************************************************************************
 * Session_SetEncryption
 * purpose : Sets the encryption algorithm for the session: DES or 3DES.
 * syntax  : Session.SetEncryption <sid> <agorithm> <eKey> <dKey>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_SetEncryption(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvStatus res;
    int sid;
    RvChar ekey[7], dkey[7]; /* 56 bit key */
    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 5)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;
    
    memset(ekey, 0, 7);
    memset(dkey, 0, 7);
    strncpy(ekey, argv[3], 7);
    strncpy(dkey, argv[4], 7);
	if (strcmp(argv[2],"DES") == 0)
    {
		res = rtptSessionSetDesEncryption(&rtpt->session[sid], ekey, dkey);
    }
    else if (strcmp(argv[2],"3DES") == 0)
    {
		res = rtptSessionSet3DesEncryption(&rtpt->session[sid], ekey, dkey);
    }
    else
        res = RV_ERROR_BADPARAM;

    if (res != RV_OK)
        return TCL_ERROR;

    return TCL_OK;
}

/********************************************************************************************
 * SessionSRTP_Construct
 * purpose : Constructs the SRTP module for the session.
 * syntax  : SessionSRTP.Construct <sid>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_Construct(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 2)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;
    if ((res = rtptSessionSRTPConstruct(&rtpt->session[sid])) != RV_OK)
        return TCL_ERROR;

    TclExecute("Session:ChangeState %d \"%d S\"", sid, sid);
    return TCL_OK;
}

/********************************************************************************************
 * SessionSRTP_Init
 * purpose : initializes initializes the SRTP module for the session.
 * syntax  : SessionSRTP.Init <sid>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_Init(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 2)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

 /*   if ((res = rtptSessionSRTPConstruct(&rtpt->session[sid])) != RV_OK)
        return TCL_ERROR;*/
    if ((res = rtptSessionSRTPInit(&rtpt->session[sid])) != RV_OK)
        return TCL_ERROR;

    TclExecute("Session:ChangeState %d \"%d S\"", sid, sid);
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_Destruct
 * purpose : Destructs the SRTP module for the session.
 * syntax  : SessionSRTP.Destruct <sid>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_Destruct(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvStatus res = RV_OK;
    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 2)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    if ((res = rtptSessionSRTPDestruct(&rtpt->session[sid])) != RV_OK)
        return TCL_ERROR;

    TclExecute("Session:ChangeState %d \"%d \"", sid, sid);
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_AddMK
 * purpose : Adds a master key to the session.
 * syntax  : SessionSRTP.AddMK <sid> <mki> <key> <salt>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_AddMK(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvUint8 mki[RTPT_SRTP_MAX_MKISIZE], mkey[RTPT_SRTP_MAX_MASTERKEYSIZE], salt[RTPT_SRTP_MAX_SALTSIZE];
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 5)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;
    memset(mki, 0, sizeof(mki));
    memcpy(mki, argv[2], RvMin(strlen(argv[2]), sizeof(mki)));
    memset(mkey, 0, sizeof(mkey));
    memcpy(mkey, argv[3], RvMin(strlen(argv[3]), sizeof(mkey)));
    memset(salt, 0, sizeof(salt));
    memcpy(salt, argv[4], RvMin(strlen(argv[4]), sizeof(salt)));

    if ((res = rtptSessionSRTPMasterKeyAdd(&rtpt->session[sid], mki, mkey, salt)) != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_DelMK
 * purpose : Removes a master key from the session.
 * syntax  : SessionSRTP.DelMK <sid> <mki>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_DelMK(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvUint8 mki[RTPT_SRTP_MAX_MKISIZE];
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 3)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;
    memset(mki, 0, sizeof(mki));
    memcpy(mki, argv[2], RvMin(strlen(argv[2]), sizeof(mki)));

    if ((res = rtptSessionSRTPMasterKeyRemove(&rtpt->session[sid], mki)) != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_DelAllMK
 * purpose : Removes all the session's master keys.
 * syntax  : SessionSRTP.DelAllMK <sid>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_DelAllMK(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 2)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    if ((res = rtptSessionSRTPMasterKeyRemoveAll(&rtpt->session[sid])) != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_AddRemoteSrc
 * purpose : Adds a remote source client and session to the session.
 * syntax  : SessionSRTP.AddRemoteSrc <sid> <type> <client> <session>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_AddRemoteSrc(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid, client, session;
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 5)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;
    if ((sscanf(argv[3], "%d", &client) != 1) || (client > RTPT_MAX_NUMBER_OF_CLIENTS) || (client <= 0))
        return TCL_ERROR;
    if ((sscanf(argv[4], "%d", &session) != 1) || (session >= RTPT_MAX_NUMBER_OF_SESSIONS_IN_TEST_CLIENT) || (session < 0))
        return TCL_ERROR;

    if ((strcmp(argv[2], "rtp") == 0) || (strcmp(argv[2], "both") == 0))
    {
        if ((res = rtptSessionSRTPRemoteSourceAddRtp(&rtpt->session[sid], client, session)) != RV_OK)
            return TCL_ERROR;
    }
    if ((strcmp(argv[2], "rtcp") == 0) || (strcmp(argv[2], "both") == 0))
    {
        if ((res = rtptSessionSRTPRemoteSourceAddRtcp(&rtpt->session[sid], client, session)) != RV_OK)
            return TCL_ERROR;
    }
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_DelRemoteSrc
 * purpose : Removes a remote source client and session from the session.
 * syntax  : SessionSRTP.DelRemoteSrc <sid> <type client session>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_DelRemoteSrc(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid, client, session;
    RvBool isRtp, isBoth;
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 3)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;
    if (sscanf(argv[2], "%d %d", &client, &session) != 2)
        return TCL_ERROR;
    isRtp = (strncmp(argv[2], "rtp", 3) == 0);
    isBoth = (strncmp(argv[2], "both", 4) == 0);

    if (isBoth)
        res = rtptSessionSRTPRemoteSourceRemove(&rtpt->session[sid], client, session, !isRtp);
    if ((res = rtptSessionSRTPRemoteSourceRemove(&rtpt->session[sid], client, session, isRtp)) != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_DelAllRemoteSrc
 * purpose : Removes all remote source client and session from the session.
 * syntax  : SessionSRTP.DelAllRemoteSrc <sid>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_DelAllRemoteSrc(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 2)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    if ((res = rtptSessionSRTPRemoteSourceRemoveAll(&rtpt->session[sid])) != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_Notify
 * purpose : Send local session parameters to the remote pary.
 * syntax  : SessionSRTP.Notify <sid>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_Notify(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 2)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    /* TODO: should we call this? */
    if ((res = rtptSessionSRTPGetLocalParams(&rtpt->session[sid])) != RV_OK)
        return TCL_ERROR;
#if defined(SRTP_ADD_ON)
    {      
        rtptSessionObj* s =  &rtpt->session[sid];
        RvChar          addressStr[64];
 
        s->localsrtp.ssrc    = RvRtpGetSSRC(s->hRTP);
        s->localsrtp.seqnum  = RvRtpGetRtpSequenceNumber(s->hRTP);
        s->localsrtp.roc     = 0;
        s->localsrtp.index   = 0;
        switch(RvAddressGetType((RvAddress*)&s->localsrtp.address))
        {
            
#if (RV_NET_TYPE & RV_NET_IPV6)
        case RV_ADDRESS_TYPE_IPV6:
            {
        TclExecute("bcast:send {notify: _%s|%d/%d %u %u %u %u %u}", 
            RvAddressGetString((RvAddress*)&s->localsrtp.address, sizeof(addressStr), addressStr),
            RvAddressGetIpPort((RvAddress*)&s->localsrtp.address), 
            RvAddressGetIpv6Scope((RvAddress*)&s->localsrtp.address),
            s->localsrtp.ssrc, s->localsrtp.seqnum, s->localsrtp.roc, s->localsrtp.index, sid);                
                break;
            }
#endif
        case RV_ADDRESS_TYPE_IPV4:
            {
        TclExecute("bcast:send {notify: %s:%d %u %u %u %u %u}", 
            RvAddressGetString((RvAddress*)&s->localsrtp.address, sizeof(addressStr), addressStr),
            RvAddressGetIpPort((RvAddress*)&s->localsrtp.address), 
            s->localsrtp.ssrc, s->localsrtp.seqnum, s->localsrtp.roc, s->localsrtp.index, sid);                  
                break;
            }
        default:
            return TCL_ERROR;
        }

    }
#endif
    return TCL_OK;
}

/********************************************************************************************
 * SessionSRTP_SendIndexes
 * purpose : Send current session indexes(RTP and RTCP) to the remote pary.
 * syntax  : SessionSRTP.SendIndexes <sid>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SendIndexes(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvStatus res = RV_OK;
    RvChar           IPstring[64]; 
    RvInt32          portRtp;
    RvInt32          portRtcp;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);
    RV_UNUSED_ARG(res);
    
    if (argc != 5)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;
    if (sscanf(argv[2], "%s", IPstring) != 1)
        return TCL_ERROR;
    if (sscanf(argv[3], "%d", &portRtp) != 1)
        return TCL_ERROR;
    if (sscanf(argv[4], "%d", &portRtcp) != 1)
        return TCL_ERROR;

    {      
        rtptSessionObj* s =  &rtpt->session[sid];
        RvChar          addressStr[64];
        
        if ((res = rtptSessionSRTPGetLocalIndexes(s, IPstring, portRtp, portRtcp)) != RV_OK)
            return TCL_ERROR;

        switch(RvAddressGetType((RvAddress*)&s->localsrtp.address))
        {
            
#if (RV_NET_TYPE & RV_NET_IPV6)
        case RV_ADDRESS_TYPE_IPV6:
            {
        TclExecute("bcast:send {sendindexes: _%s|%d/%d %u %u %u %u %u}", 
            RvAddressGetString((RvAddress*)&s->localsrtp.address, sizeof(addressStr), addressStr),
            RvAddressGetIpPort((RvAddress*)&s->localsrtp.address), 
            RvAddressGetIpv6Scope((RvAddress*)&s->localsrtp.address),
            RvUint64ToRvUint32(RvUint64ShiftRight(s->localsrtp.indexRTP,32)), 
            RvUint64ToRvUint32(RvUint64ShiftRight(RvUint64ShiftLeft(s->localsrtp.indexRTP,32),32)), 
            RvUint64ToRvUint32(RvUint64ShiftRight(s->localsrtp.indexRTCP,32)), 
            RvUint64ToRvUint32(RvUint64ShiftRight(RvUint64ShiftLeft(s->localsrtp.indexRTCP,32),32)),
            sid);                
                break;
            }
#endif
        case RV_ADDRESS_TYPE_IPV4:
            {
        TclExecute("bcast:send {sendindexes: %s:%d %u %u %u %u %u}", 
            RvAddressGetString((RvAddress*)&s->localsrtp.address, sizeof(addressStr), addressStr),
            RvAddressGetIpPort((RvAddress*)&s->localsrtp.address),
            RvUint64ToRvUint32(RvUint64ShiftRight(s->localsrtp.indexRTP,32)), 
            RvUint64ToRvUint32(RvUint64ShiftRight(RvUint64ShiftLeft(s->localsrtp.indexRTP,32),32)), 
            RvUint64ToRvUint32(RvUint64ShiftRight(s->localsrtp.indexRTCP,32)), 
            RvUint64ToRvUint32(RvUint64ShiftRight(RvUint64ShiftLeft(s->localsrtp.indexRTCP,32),32)),
            sid);                      
                break;
            }
        default:
            return TCL_ERROR;
        }
    }

    return TCL_OK;
}

/********************************************************************************************
 * SessionSRTP_SetDestContext
 * purpose : Set destination context.
 * syntax  : SessionSRTP.SetDestContext <sid> <ip> <port> <type> <index> <trigger> <mki>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetDestContext(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid, index;
    RvBool isRtp, isBoth, isTrigger;
    RvUint16 port;
    RvUint8 mki[RTPT_SRTP_MAX_MKISIZE];
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 8)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    port = (RvUint16)atoi(argv[3]);
    isRtp = (strncmp(argv[4], "rtp", 3) == 0);
    isBoth = (strncmp(argv[4], "both", 4) == 0);
    index = atoi(argv[5]);
    isTrigger = (argv[6][0] == '1');
    memset(mki, 0, sizeof(mki));
    memcpy(mki, argv[7], RvMin(strlen(argv[7]), sizeof(mki)));

    if (index == 0)
    {
        if (isBoth)
        {
            res = rtptSessionSRTPAddDestinationContext(&rtpt->session[sid], RV_TRUE, argv[2], port, mki, isTrigger);
            port++;
        }
        res = rtptSessionSRTPAddDestinationContext(&rtpt->session[sid], isRtp, argv[2], port, mki, isTrigger);
    }
    else
    { 
        if (isBoth)
        {
            /* same index - problem */
            res = rtptSessionSRTPAddDestinationContextAt(&rtpt->session[sid], RV_TRUE, argv[2], port, mki, index, isTrigger);
            port++;
        }
        res = rtptSessionSRTPAddDestinationContextAt(&rtpt->session[sid], isRtp, argv[2], port, mki, index, isTrigger);
    }
    if (res != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_SetSourceContext
 * purpose : Set destination context.
 * syntax  : SessionSRTP.SetSourceContext <sid> <index> <trigger> <mki> <type client session>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetSourceContext(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid, client, session, index;
    RvBool isRtp, isBoth, isTrigger;
    RvUint8 mki[RTPT_SRTP_MAX_MKISIZE];
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 6)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    index = atoi(argv[2]);
    isTrigger = (argv[3][0] == '1');
    memset(mki, 0, sizeof(mki));
    memcpy(mki, argv[4], RvMin(strlen(argv[4]), sizeof(mki)));
    isRtp = (strncmp(argv[5], "rtp", 3) == 0);
    isBoth = (strncmp(argv[5], "both", 4) == 0);
    if (sscanf(argv[5]+4+!isRtp, "%d %d", &client, &session) != 2)
        return TCL_ERROR;

    if (index == 0)
    {
        if (isBoth)
            res = rtptSessionSRTPSourceChangeKey(&rtpt->session[sid], client, session, mki, !isRtp, isTrigger);
        res = rtptSessionSRTPSourceChangeKey(&rtpt->session[sid], client, session, mki, isRtp, isTrigger);
    }
    else
    {
        if (isBoth)
            res = rtptSessionSRTPSourceChangeKeyAt(&rtpt->session[sid], client, session, mki, index, !isRtp, isTrigger);
        res = rtptSessionSRTPSourceChangeKeyAt(&rtpt->session[sid], client, session, mki, index, isRtp, isTrigger);
    }
    if (res != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}

/********************************************************************************************
 * SessionSRTP_MessageDispatcher
 * purpose : obtains the broadcasted messages
 * syntax  : SessionSRTP_MessageDispatcher 
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_MessageDispatcher(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    RvChar*  str = NULL, *delimiter = NULL;
    RvStatus res = RV_OK;
    RvChar   parameter[64];
    RvAddress senderAddress;
    RvUint32 client = 0;
    rtptSrtpParams params;
    RvUint32 locind = 0;
    RvUint32 remotesid = 0;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 3)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%u", &client) != 1) && client >= RTPT_MAX_NUMBER_OF_CLIENTS)
        return TCL_ERROR;

#if defined(SRTP_ADD_ON)    
    str = argv[2];
    delimiter = strstr(str,":");
    memcpy(parameter, str, delimiter - str);
    parameter[delimiter - str] = '\0';

    if (strcmp(parameter, "notify") == 0)
    {
        RvChar          addressStr[64];

        str = str + (delimiter - str)+2;
        delimiter = strstr(str," ");

        if (delimiter == NULL)
            return TCL_ERROR; 
        memcpy(parameter, str, delimiter - str);
        parameter[delimiter - str] = '\0';
        /* get the listening address */
        if (parameter[0]=='_')
            parameter[0]='[';
        res = setAddressFromString(parameter, &senderAddress);
        if (res != RV_OK)
            return TCL_ERROR;
                
        str = str + (delimiter - str)+1;
        if ((sscanf(str, "%u %u %u %u %u", &params.ssrc, &params.seqnum, &params.roc, &params.index, &remotesid) != 5) && (remotesid >= RTP_TEST_APPLICATION_SESSIONS_MAX))
            return TCL_ERROR;
        
        for (locind=0; locind < RTP_TEST_APPLICATION_SESSIONS_MAX; locind++)
        {
//            TclExecute("test:Log {received notify command from %s:%d session=%u : ssrc=%u sn=%u roc=%u index=%u }", 
            if (rtpt->session[locind].open && 
                ((!RvAddressCompare(&senderAddress, &rtpt->session[locind].localAddr,RV_ADDRESS_FULLADDRESS)) ||
                 (RvAddressCompare(&senderAddress, &rtpt->session[locind].localAddr,RV_ADDRESS_BASEADDRESS) && remotesid != locind)))
            {
                TclExecute("test:Log \"Session %d. Received notify command from %s:%d (client=%u) session=%u : ssrc=%u sn=%u roc=%u index=%u\"",
                locind, RvAddressGetString(&senderAddress, sizeof(addressStr), addressStr),
                RvAddressGetIpPort(&senderAddress), client, remotesid,
                params.ssrc, params.seqnum, params.roc, params.index);
                rtptSessionSRTP_ReceiveLocalParams(&rtpt->session[locind], client, remotesid, &params, &senderAddress);
            }
        }
    }
    else if (strcmp(parameter, "sendindexes") == 0)
    {
        RvChar   addressStr[64];
        RvChar   int64Str[64];
        RvChar   int64Str1[64];
        RvUint32 rtpIndexMsb;
        RvUint32 rtpIndexLsb;
        RvUint32 rtcpIndexMsb;
        RvUint32 rtcpIndexLsb;

        str = str + (delimiter - str)+2;
        delimiter = strstr(str," ");

        if (delimiter == NULL)
            return TCL_ERROR; 
        memcpy(parameter, str, delimiter - str);
        parameter[delimiter - str] = '\0';
        if (parameter[0]=='_') /* IPV6 */
            parameter[0]='[';        
        /* get the listening address */
        res = setAddressFromString(parameter, &senderAddress);
        if (res != RV_OK)
            return TCL_ERROR;
                
        str = str + (delimiter - str)+1;
        if ((sscanf(str, "%u %u %u %u %u", &rtpIndexMsb, &rtpIndexLsb, &rtcpIndexMsb, &rtcpIndexLsb, &remotesid) != 5) && (remotesid >= RTP_TEST_APPLICATION_SESSIONS_MAX))
            return TCL_ERROR;

        params.indexRTP  = RvUint64Const(rtpIndexMsb,  rtpIndexLsb);
        params.indexRTCP = RvUint64Const(rtcpIndexMsb, rtcpIndexLsb);       
        
        for (locind=0; locind < RTP_TEST_APPLICATION_SESSIONS_MAX; locind++)
        {
            //            TclExecute("test:Log {received notify command from %s:%d session=%u : ssrc=%u sn=%u roc=%u index=%u }", 
            if (rtpt->session[locind].open && remotesid != locind)
            {
                TclExecute("test:Log {Session %u. Received \"sendinexes\" command from %s:%d (client=%u, session=%u): RTP index=%s, RTCP index=%s }",
                    locind, RvAddressGetString(&senderAddress, sizeof(addressStr), addressStr),
                    RvAddressGetIpPort(&senderAddress), client, remotesid,
                    Rv64UtoA(params.indexRTP, int64Str), Rv64UtoA(params.indexRTCP, int64Str1));
                rtptSessionSRTP_ReceiveIndexes(&rtpt->session[locind], client, remotesid, &params, &senderAddress);
            }
        }
    }
    /*else if ()*/
#else
    RV_UNUSED_ARG(rtpt);
    RV_UNUSED_ARG(delimiter);
    RV_UNUSED_ARG(str);
    RV_UNUSED_ARG(res);
    RV_UNUSED_ARG(parameter);
    RV_UNUSED_ARG(&senderAddress);
    RV_UNUSED_ARG(&params);
    RV_UNUSED_ARG(locind);
    RV_UNUSED_ARG(remotesid);
#endif /* #if defined(SRTP_ADD_ON)     */
    return TCL_OK;
}
/********************************************************************************************
 * SessionSRTP_SetMasterKeySizes
 * purpose : Sets Master Key sizes for the session
 * syntax  : SessionSRTP.SetMasterKeySizes <sid> <mkiSize> <keySize> <saltSize>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetMasterKeySizes(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvInt32 mkiSize, keySize, saltSize;
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 5)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    mkiSize = atoi(argv[2]);
    keySize = atoi(argv[3]);
    saltSize = atoi(argv[4]);

    res = rtptSessionSRTPSetMasterKeySizes(&rtpt->session[sid], mkiSize, keySize, saltSize);
    if (res != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_KeyDerivation
 * purpose : Defines key derivation algorithm and rate
 * syntax  : SessionSRTP.KeyDerivation <sid> <AESCM or NONE> <rate>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_KeyDerivation(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvInt32 rate;
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 4)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    rate = atoi(argv[3]);

    res = rtptSessionSRTPKeyDerivation(&rtpt->session[sid], argv[2], rate);
    if (res != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_PrefixLength
 * purpose : Sets prefix length
 * syntax  : SessionSRTP.PrefixLength <sid> <length>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_PrefixLength(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvInt32 length;
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 3)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    length = atoi(argv[2]);

    res = rtptSessionSRTPPrefixLength(&rtpt->session[sid], length);
    if (res != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_SetEncryption
 * purpose : Defines encryption
 * syntax  : SessionSRTP.SetEncryption <sid> <rtp/rtcp> <NONE or AESCM or AESF8> <useMki>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetEncryption(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvBool isRtp, isBoth, bUseMki;
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 5)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    isRtp = (strncmp(argv[2], "rtp", 3) == 0);
    isBoth = (strncmp(argv[2], "both", 4) == 0);
    bUseMki = (argv[4][0] == '1');

    if (isBoth)
        res = rtptSessionSRTPSetEncryption(&rtpt->session[sid], !isRtp, argv[3], bUseMki);
    res = rtptSessionSRTPSetEncryption(&rtpt->session[sid], isRtp, argv[3], bUseMki);
    if (res != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_SetAuthentication
 * purpose : Sets Authentication algorithm and tag size
 * syntax  : SessionSRTP.SetAuthentication <sid> <rtp/rtcp> <NONE or HMACSHA1> <tagSize>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetAuthentication(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvBool isRtp, isBoth;
    RvInt32 tagSize;
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 5)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    isRtp = (strncmp(argv[2], "rtp", 3) == 0);
    isBoth = (strncmp(argv[2], "both", 4) == 0);
    tagSize = atoi(argv[4]);

    if (isBoth)
        res = rtptSessionSRTPSetAuthentication(&rtpt->session[sid], !isRtp, argv[3], tagSize);
    res = rtptSessionSRTPSetAuthentication(&rtpt->session[sid], isRtp, argv[3], tagSize);
    if (res != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_SetKeySizes
 * purpose : Sets key sizes
 * syntax  : SessionSRTP.SetKeySizes <sid> <rtp/rtcp> <encryptKeySize> <authKeySize> <saltSize>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetKeySizes(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvBool isRtp, isBoth;
    RvSize_t encryptKeySize, authKeySize, saltSize;
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 6)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    isRtp = (strncmp(argv[2], "rtp", 3) == 0);
    isBoth = (strncmp(argv[2], "both", 4) == 0);
    encryptKeySize = (RvSize_t)atoi(argv[3]);
    authKeySize = (RvSize_t)atoi(argv[4]);
    saltSize = (RvSize_t)atoi(argv[5]);

    if (isBoth)
        res = rtptSessionSRTPSetKeySizes(&rtpt->session[sid], !isRtp, encryptKeySize, authKeySize, saltSize);
    res = rtptSessionSRTPSetKeySizes(&rtpt->session[sid], isRtp, encryptKeySize, authKeySize, saltSize);
    if (res != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_SetReplayListSize
 * purpose : Sets replay list size
 * syntax  : SessionSRTP.ReplayListSizes <sid> <rtp/rtcp> <replayListSize>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_ReplayListSizes(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvBool isRtp, isBoth;
    RvSize_t replayListSize;
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 4)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    isRtp = (strncmp(argv[2], "rtp", 3) == 0);
    isBoth = (strncmp(argv[2], "both", 4) == 0);
    replayListSize = (RvSize_t)atoi(argv[3]);

    if (isBoth)
        res = rtptSessionSRTPSetReplayListSize(&rtpt->session[sid], !isRtp, replayListSize);
    res = rtptSessionSRTPSetReplayListSize(&rtpt->session[sid], isRtp, replayListSize);
    if (res != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_SetHistory
 * purpose : Sets history size
 * syntax  : SessionSRTP.SetHistory <sid> <rtp/rtcp> <historySize>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetHistory(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvBool isRtp, isBoth;
    RvSize_t historySize;
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 4)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    isRtp = (strncmp(argv[2], "rtp", 3) == 0);
    isBoth = (strncmp(argv[2], "both", 4) == 0);
    historySize = (RvSize_t)atoi(argv[3]);

    if (isBoth)
        res = rtptSessionSRTPSetHistory(&rtpt->session[sid], !isRtp, historySize);
    res = rtptSessionSRTPSetHistory(&rtpt->session[sid], isRtp, historySize);
    if (res != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}

/********************************************************************************************
 * SessionSRTP_SetKeyPool
 * purpose : Sets Master Key Pool configuration
 * syntax  : SessionSRTP.SetKeyPool <sid> <pooltype> <pageItems> <pageSize> <maxItems> <minItems> <freeLevel>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetKeyPool(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvSize_t pageItems, pageSize, maxItems, minItems, freeLevel; 
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 8)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    if (!((strncmp(argv[2],  "EXPANDING", 9) == 0)
        || (strncmp(argv[2], "DYNAMIC", 7) == 0)
        || (strncmp(argv[2], "FIXED", 5) == 0)))
        return TCL_ERROR;

    pageItems = (RvSize_t)atoi(argv[3]);
    pageSize  = (RvSize_t)atoi(argv[4]);
    maxItems  = (RvSize_t)atoi(argv[5]);
    minItems  = (RvSize_t)atoi(argv[6]);    
    freeLevel  = (RvSize_t)atoi(argv[7]);   

    res = rtptSessionSRTPSetMasterKeyPool(&rtpt->session[sid], argv[2], pageItems, pageSize, maxItems, minItems, freeLevel);
    if (res != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_SetStreamPool
 * purpose : Sets Stream Pool configuration
 * syntax  : SessionSRTP.SetStreamPool <sid> <pooltype> <pageItems> <pageSize> <maxItems> <minItems> <freeLevel>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetStreamPool(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvSize_t pageItems, pageSize, maxItems, minItems, freeLevel; 
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 8)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    if (!((strncmp(argv[2],  "EXPANDING", 9) == 0)
        || (strncmp(argv[2], "DYNAMIC", 7) == 0)
        || (strncmp(argv[2], "FIXED", 5) == 0)))
        return TCL_ERROR;

    pageItems = (RvSize_t)atoi(argv[3]);
    pageSize  = (RvSize_t)atoi(argv[4]);
    maxItems  = (RvSize_t)atoi(argv[5]);
    minItems  = (RvSize_t)atoi(argv[6]);    
    freeLevel  = (RvSize_t)atoi(argv[7]);   

    res = rtptSessionSRTPSetStreamPool(&rtpt->session[sid], argv[2], pageItems, pageSize, maxItems, minItems, freeLevel);
    if (res != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_SetContextPool
 * purpose : Sets Stream Pool configuration
 * syntax  : SessionSRTP.SetContextPool <sid> <pooltype> <pageItems> <pageSize> <maxItems> <minItems> <freeLevel>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetContextPool(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvSize_t pageItems, pageSize, maxItems, minItems, freeLevel; 
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 8)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    if (!((strncmp(argv[2],  "EXPANDING", 9) == 0)
        || (strncmp(argv[2], "DYNAMIC", 7) == 0)
        || (strncmp(argv[2], "FIXED", 5) == 0)))
        return TCL_ERROR;

    pageItems = (RvSize_t)atoi(argv[3]);
    pageSize  = (RvSize_t)atoi(argv[4]);
    maxItems  = (RvSize_t)atoi(argv[5]);
    minItems  = (RvSize_t)atoi(argv[6]);    
    freeLevel  = (RvSize_t)atoi(argv[7]);   

    res = rtptSessionSRTPSetContextPool(&rtpt->session[sid], argv[2], pageItems, pageSize, maxItems, minItems, freeLevel);
    if (res != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_SetKeyHash
 * purpose : Sets Master Key Hash configuration
 * syntax  : SessionSRTP.SetKeyHash <sid> <hashtype> <startSize>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetKeyHash(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvSize_t startSize; 
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 4)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    if (!((strncmp(argv[2],  "EXPANDING", 9) == 0)
        || (strncmp(argv[2], "DYNAMIC", 7) == 0)
        || (strncmp(argv[2], "FIXED", 5) == 0)))
        return TCL_ERROR;

    startSize = (RvSize_t)atoi(argv[3]);
  
    res = rtptSessionSRTPSetKeyHash(&rtpt->session[sid], argv[2], startSize);
    if (res != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}


/********************************************************************************************
 * SessionSRTP_SetSourceHash
 * purpose : Sets Remote Source Hash configuration
 * syntax  : SessionSRTP.SetSourceHash <sid> <hashtype> <startSize>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetSourceHash(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvSize_t startSize; 
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 4)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    if (!((strncmp(argv[2],  "EXPANDING", 9) == 0)
        || (strncmp(argv[2], "DYNAMIC", 7) == 0)
        || (strncmp(argv[2], "FIXED", 5) == 0)))
        return TCL_ERROR;

    startSize = (RvSize_t)atoi(argv[3]);
  
    res = rtptSessionSRTPSetSourceHash(&rtpt->session[sid], argv[2], startSize);
    if (res != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}

/********************************************************************************************
 * SessionSRTP_SetDestHash
 * purpose : Sets Destination Hash configuration
 * syntax  : SessionSRTP.SetDestHash <sid> <hashtype> <startSize>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetDestHash(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[])
{
    rtptObj *rtpt = (rtptObj *)clientData;
    int sid;
    RvSize_t startSize; 
    RvStatus res = RV_OK;

    RV_UNUSED_ARG(clientData);
    RV_UNUSED_ARG(interp);

    if (argc != 4)
        return TCL_ERROR;
    if ((sscanf(argv[1], "%d", &sid) != 1) || (sid >= RTP_TEST_APPLICATION_SESSIONS_MAX) || (!rtpt->session[sid].open))
        return TCL_ERROR;

    if (!((strncmp(argv[2],  "EXPANDING", 9) == 0)
        || (strncmp(argv[2], "DYNAMIC", 7) == 0)
        || (strncmp(argv[2], "FIXED", 5) == 0)))
        return TCL_ERROR;

    startSize = (RvSize_t)atoi(argv[3]);
  
    res = rtptSessionSRTPSetDestHash(&rtpt->session[sid], argv[2], startSize);
    if (res != RV_OK)
        return TCL_ERROR;
    return TCL_OK;
}

/******************************************************************************
 * rtptEventIndication
 * ----------------------------------------------------------------------------
 * General: Indication of a message being sent or received by the endpoint.
 *
 * Return Value: None.
 * ----------------------------------------------------------------------------
 * Arguments:
 * Input:  rtpt             - rtpt object to use.
 *         eventType        - Type of the event indicated.
 *         session          - session this event belongs to (NULL if none).
 *         eventStr         - Event string.
 * Output: None.
 *****************************************************************************/
void rtptEventIndication(
    IN rtptObj              *rtpt,
    IN const RvChar         *eventType,
    IN rtptSessionObj       *session,
    IN const RvChar         *eventStr)
{
    RV_UNUSED_ARG(rtpt);
    RV_UNUSED_ARG(session);
    
    if (strcmp(eventType, "Rec") == 0)
    {
        TclExecute("catch {rmgr:send \"btn\" {%s}}", eventStr);
        /* also print to the app log */
        TclExecute("test:Log {Command: \"%s\"}", eventStr);
    }
    else if (strcmp(eventType, "TCL") == 0)
    {
        TclExecute("%s", eventStr);
    }
    else if (strcmp(eventType, "RtpShutdown") == 0)
    {
        GUI_Session_Close(session);
    }
#if 0
    if (strcmp(eventType, "CallState") == 0)
    {
        TclExecute("catch {record:event %s %d %s}", eventType, call->id, eventStr);
        TclExecute("Call:ChangeState %d {%s}", call->id, CallInfoStr(call, eventStr));
    }
    else if (strcmp(eventType, "ChanState") == 0)
    {
        TclExecute("catch {record:event %s %d %s}", eventType, call->id, eventStr);
    }
    else if (strcmp(eventType, "TCL") == 0)
    {
        TclExecute("%s", eventStr);
    }
    else if (strcmp(eventType, "TCPdial") == 0)
    {
        TcpCallDial(call, eventStr);
    }
    else if (strcmp(eventType, "TCPrebind") == 0)
    {
        TcpCallRebind(call);
    }
    else if (strcmp(eventType, "TCPdrop") == 0)
    {
        TcpCallDrop(call);
    }
    else if (strcmp(eventType, "Rec") == 0)
    {
        TclExecute("catch {rmgr:send \"btn\" {%s}}", eventStr);
    }
    else if (strcmp(eventType, "Script") == 0)
    {
        TclExecute("script:cb {%s}", eventStr);
    }
    else if (strcmp(eventType, "Tag") == 0)
    {
        TclExecute("catch {rmgr:setTag %s}", eventStr);
    }
    else if (strcmp(eventType, "Ev") == 0)
    {
        TclExecute("catch {record:event %s %d %s}", eventType, CALL_ID(call), eventStr);
    }
    else if (strcmp(eventType, "Media") == 0)
    {
    }
    else if (strcmp(eventType, "H223Skew") == 0)
    {
    }
    else if (strcmp(eventType, "UserInput") == 0)
    {
    }
    else if (strcmp(eventType, "VideoFastUpdate") == 0)
    {
    }
    else if (strcmp(eventType, "VTST") == 0)
    {
    }
#endif
    else
    {
        TclExecute("test:Error {Bad event name '%s'}", eventType);
    }

}
#ifdef __cplusplus
}
#endif


