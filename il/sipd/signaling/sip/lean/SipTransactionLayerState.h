/// @file
/// @brief SIP transaction layer state class definitions
///
///  Copyright (c) 2008 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#ifndef _SIP_TRANSACTION_LAYER_STATE_H_
#define _SIP_TRANSACTION_LAYER_STATE_H_

#if !defined(_DEFINE_SIP_TRANSACTION_LAYER_STATES_) && !defined(_DECLARE_SIP_TRANSACTION_LAYER_STATES_)
# error SipTransactionLayerState.h is an implementation-private header file and should not be \
included by files other than those implementing SipTransactionLayer or SipTransactionLayerState classes.
#endif

DECL_SIPLEAN_NS

///////////////////////////////////////////////////////////////////////////////

// Base state class definition
struct SipTransactionLayer::State
{
    virtual ~State() { }

    // State name
    virtual const std::string Name(void) const = 0;

    // Protocol events
    virtual bool SendMessage(Transaction& trans, const SipMessage& msg) const = 0;
    virtual void ReceiveMessage(Transaction& trans, const SipMessage& msg) const = 0;

    // Timer events
    virtual void RetransmitTimerExpired(Transaction& trans) const = 0;
    virtual void TransactionTimerExpired(Transaction& trans) const = 0;
};
    
///////////////////////////////////////////////////////////////////////////////

// Concrete state class utility macros
#define CONCRETE_SIP_TRANSACTION_LAYER_STATE_CLASS(cls, name)                           \
struct SipTransactionLayer::cls : public SipTransactionLayer::State                     \
{                                                                                       \
    ~cls() { }                                                                          \
    const std::string Name(void) const { return std::string(#name); }                   \
    bool SendMessage(Transaction& trans, const SipMessage& msg) const;                  \
    void ReceiveMessage(Transaction& trans, const SipMessage& msg) const;               \
    void RetransmitTimerExpired(Transaction& trans) const;                              \
    void TransactionTimerExpired(Transaction& trans) const;                             \
}
    
#ifdef _DEFINE_SIP_TRANSACTION_LAYER_STATES_
# define CONCRETE_SIP_TRANSACTION_LAYER_STATE(cls, var) CONCRETE_SIP_TRANSACTION_LAYER_STATE_CLASS(cls, var); const SipTransactionLayer::cls SipTransactionLayer::var
#else
# define CONCRETE_SIP_TRANSACTION_LAYER_STATE(cls, var) CONCRETE_SIP_TRANSACTION_LAYER_STATE_CLASS(cls, var)
#endif

// Concrete state classes (client INVITE transactions)
CONCRETE_SIP_TRANSACTION_LAYER_STATE(ClientInviteCallingState, CLIENT_INVITE_CALLING_STATE);
CONCRETE_SIP_TRANSACTION_LAYER_STATE(ClientInviteProceedingState, CLIENT_INVITE_PROCEEDING_STATE);
CONCRETE_SIP_TRANSACTION_LAYER_STATE(ClientInviteCompletedState, CLIENT_INVITE_COMPLETED_STATE);
CONCRETE_SIP_TRANSACTION_LAYER_STATE(ClientInviteTerminatedState, CLIENT_INVITE_TERMINATED_STATE);

// Concrete state classes (client non-INVITE transactions)
CONCRETE_SIP_TRANSACTION_LAYER_STATE(ClientNonInviteTryingState, CLIENT_NON_INVITE_TRYING_STATE);
CONCRETE_SIP_TRANSACTION_LAYER_STATE(ClientNonInviteProceedingState, CLIENT_NON_INVITE_PROCEEDING_STATE);
CONCRETE_SIP_TRANSACTION_LAYER_STATE(ClientNonInviteCompletedState, CLIENT_NON_INVITE_COMPLETED_STATE);
CONCRETE_SIP_TRANSACTION_LAYER_STATE(ClientNonInviteTerminatedState, CLIENT_NON_INVITE_TERMINATED_STATE);

// Concrete state classes (server INVITE transactions)
CONCRETE_SIP_TRANSACTION_LAYER_STATE(ServerInviteProceedingState, SERVER_INVITE_PROCEEDING_STATE);
CONCRETE_SIP_TRANSACTION_LAYER_STATE(ServerInviteCompletedState, SERVER_INVITE_COMPLETED_STATE);
CONCRETE_SIP_TRANSACTION_LAYER_STATE(ServerInviteConfirmedState, SERVER_INVITE_CONFIRMED_STATE);
CONCRETE_SIP_TRANSACTION_LAYER_STATE(ServerInviteTerminatedState, SERVER_INVITE_TERMINATED_STATE);

// Concrete state classes (server non-INVITE transactions)
CONCRETE_SIP_TRANSACTION_LAYER_STATE(ServerNonInviteTryingState, SERVER_NON_INVITE_TRYING_STATE);
CONCRETE_SIP_TRANSACTION_LAYER_STATE(ServerNonInviteProceedingState, SERVER_NON_INVITE_PROCEEDING_STATE);
CONCRETE_SIP_TRANSACTION_LAYER_STATE(ServerNonInviteCompletedState, SERVER_NON_INVITE_COMPLETED_STATE);
CONCRETE_SIP_TRANSACTION_LAYER_STATE(ServerNonInviteTerminatedState, SERVER_NON_INVITE_TERMINATED_STATE);

///////////////////////////////////////////////////////////////////////////////

END_DECL_SIPLEAN_NS

#endif
