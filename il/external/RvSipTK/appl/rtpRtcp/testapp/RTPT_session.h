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



#ifndef _TAP_session_H
#define _TAP_session_H

#ifdef __cplusplus
extern "C" {
#endif



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
int Session_Open(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


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
int Session_Close(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


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
int Session_AddRemoteAddr(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int Session_RemoveRemoteAddr(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int Session_RemoveAllRemoteAddrs(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/********************************************************************************************
 * Session_SetPayload
 * purpose : Set the session payload.
 * syntax  : Session.SetPayload <sid> <payload>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_SetPayload(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int Session_StartRead(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int Addrs_DisplayAddressList(ClientData clientData, Tcl_Interp *interp,int argc, char *argv[]);


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
int Session_SendSR(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int Session_SendRR(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int Session_SendBye(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int Session_Shutdown(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int Session_SendAPP(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int Session_ListenMulticast(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int Session_SetMulticastTtl(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/********************************************************************************************
 * Session_SetTos
 * purpose : Sets multicast type of service for RTP and RTCP.
 * syntax  : Session.SetRtpMulticastTtl <sid> <rtpTtl> <rtcpTtl>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_SetTos(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int Session_SetBandwidth(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/********************************************************************************************
 * SessionSession_SetEncryption
 * purpose : Sets the encryption algorithm for the session: DES or 3DES.
 * syntax  : Session.SetEncryption <sid> <agorithm> <eKey> <dKey>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int Session_SetEncryption(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);

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
int SessionSRTP_Construct(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);

/********************************************************************************************
 * SessionSRTP_Init
 * purpose : Constructs and initializes the SRTP module.
 * syntax  : SessionSRTP.Init <sid>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_Init(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/********************************************************************************************
 * SessionSRTP_Destruct
 * purpose : Constructs and initializes the SRTP module.
 * syntax  : SessionSRTP.Destruct <sid>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_Destruct(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int SessionSRTP_AddMK(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int SessionSRTP_DelMK(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/********************************************************************************************
 * SessionSRTP_DelAllMK
 * purpose : Removes all the session's master keys.
 * syntax  : SessionSRTP.DelAllMK <sid> <mki>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_DelAllMK(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int SessionSRTP_AddRemoteSrc(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int SessionSRTP_DelRemoteSrc(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int SessionSRTP_DelAllRemoteSrc(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int SessionSRTP_Notify(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/********************************************************************************************
 * SessionSRTP_SendIndexes
 * purpose : Send current session indexes(RTP and RTCP) to the remote pary.
 * syntax  : SessionSRTP.Notify <sid>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SendIndexes(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/********************************************************************************************
 * SessionSRTP_SetDestContext
 * purpose : Set destination context.
 * syntax  : SessionSRTP.SetDestContext <sid> <ip> <port> <type> <trigger> <mki>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetDestContext(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int SessionSRTP_SetSourceContext(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int SessionSRTP_MessageDispatcher(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int SessionSRTP_SetMasterKeySizes(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int SessionSRTP_KeyDerivation(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int SessionSRTP_PrefixLength(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/********************************************************************************************
 * SessionSRTP_SetEncryption
 * purpose : Defines encryption
 * syntax  : SessionSRTP.PrefixLength <sid> <isRtp> <NONE or AESCM or AESF8> <useMki>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetEncryption(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/********************************************************************************************
 * SessionSRTP_SetAuthentication
 * purpose : Sets Authentication algorithm and tag size
 * syntax  : SessionSRTP.SetAuthentication <sid> <isRtp> <NONE or HMACSHA1> <tagSize>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetAuthentication(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/********************************************************************************************
 * SessionSRTP_SetKeySizes
 * purpose : Sets key sizes
 * syntax  : SessionSRTP.SetKeySizes <sid> <isRtp> <encryptKeySize> <authKeySize> <saltSize>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetKeySizes(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/********************************************************************************************
 * SessionSRTP_SetReplayListSize
 * purpose : Sets replay list size
 * syntax  : SessionSRTP.ReplayListSize <sid> <isRtp> <replayListSize>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_ReplayListSizes(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/********************************************************************************************
 * SessionSRTP_SetHistory
 * purpose : Sets history size
 * syntax  : SessionSRTP.ReplayListSize <sid> <isRtp> <historySize>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetHistory(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int SessionSRTP_SetKeyPool(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


/********************************************************************************************
 * SessionSRTP_SetStreamPool
 * purpose : Sets Stream Pool configuration
 * syntax  : SessionSRTP.SSetStreamPool <sid> <pooltype> <pageItems> <pageSize> <maxItems> <minItems> <freeLevel>
 * input   : clientData - Not used
 *           interp     - Interpreter for tcl commands
 *           argc       - Number of arguments
 *           argv       - Arguments of the Tcl/Tk command
 * output  : none
 * return  : TCL_OK     - The command was invoked successfully.
 ********************************************************************************************/
int SessionSRTP_SetStreamPool(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int SessionSRTP_SetContextPool(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int SessionSRTP_SetKeyHash(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int SessionSRTP_SetSourceHash(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);


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
int SessionSRTP_SetDestHash(ClientData clientData, Tcl_Interp *interp, int argc, char *argv[]);

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
    IN const RvChar         *eventStr);

#ifdef __cplusplus
}
#endif

#endif  /* _TAP_session_H */

