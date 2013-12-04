/// @file
/// @brief SIP transaction layer concrete state class implementations
///
///  Copyright (c) 2008 by Spirent Communications Inc.
///  All Rights Reserved.
///
///  This software is confidential and proprietary to Spirent Communications Inc.
///  No part of this software may be reproduced, transmitted, disclosed or used
///  in violation of the Software License Agreement without the expressed
///  written consent of Spirent Communications Inc.
///

#include "VoIPLog.h"
#include "SipMessage.h"
#include "SipTransactionLayer.h"

USING_SIPLEAN_NS;

///////////////////////////////////////////////////////////////////////////////

#define _DECLARE_SIP_TRANSACTION_LAYER_STATES_
#include "SipTransactionLayerState.h"

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::ClientInviteCallingState::SendMessage(Transaction& trans, const SipMessage& msg) const
{
    if (trans.SendMessage(msg))
    {
        trans.mRTT = TIMER_T1;
        trans.mRetransmitTimer.Start(trans.mRTT);  
        trans.mTransactionTimer.Start(64 * TIMER_T1);
        return true;
    }
    else
    {
        trans.ChangeState(&CLIENT_INVITE_TERMINATED_STATE);
        trans.InvokeTerminationHook();
        return false;
    }
}

void SipTransactionLayer::ClientInviteCallingState::ReceiveMessage(Transaction& trans, const SipMessage& msg) const
{
    if (msg.status.Category() == SipMessage::SIP_STATUS_PROVISIONAL)  // 1xx
    {
        trans.mRetransmitTimer.Stop();
        trans.mOwner.mClientMessageDelegate(&msg, msg.fromTag);
        trans.ChangeState(&CLIENT_INVITE_PROCEEDING_STATE);
    }
    else if (msg.status.Category() == SipMessage::SIP_STATUS_SUCCESS)  // 2xx
    {
        trans.mOwner.mClientMessageDelegate(&msg, msg.fromTag);
        trans.ChangeState(&CLIENT_INVITE_TERMINATED_STATE);
        trans.InvokeTerminationHook();
    }
    else  // 300-699
    {
        if (trans.SendInternalAckRequest(msg))
        {
            trans.mRetransmitTimer.Stop();
            trans.mTransactionTimer.Stop();
            trans.mTerminationTimer.Start(64 * TIMER_T1);
            trans.mOwner.mClientMessageDelegate(&msg, msg.fromTag);
            trans.ChangeState(&CLIENT_INVITE_COMPLETED_STATE);
        }
        else
        {
            trans.mOwner.mClientMessageDelegate(&msg, msg.fromTag);
            trans.ChangeState(&CLIENT_INVITE_TERMINATED_STATE);
            trans.InvokeTerminationHook();
        }
    }
}

void SipTransactionLayer::ClientInviteCallingState::RetransmitTimerExpired(Transaction& trans) const
{
    if (trans.RetransmitMessage())
    {
        trans.mRTT = std::min(2 * trans.mRTT, TIMER_T2);
        trans.mRetransmitTimer.Start(trans.mRTT);
        TC_LOG_DEBUG_LOCAL(trans.mOwner.mPort, LOG_SIP, "[SipTransactionLayer::ClientInviteCallingState::RetransmitTimerExpired] Retransmitted with timeout = " << trans.mRTT.msec() << " msec");
    }
    else
    {
        trans.mOwner.mClientMessageDelegate(0, trans.mLocalTag);
        trans.ChangeState(&CLIENT_INVITE_TERMINATED_STATE);
        trans.InvokeTerminationHook();
    }
}

void SipTransactionLayer::ClientInviteCallingState::TransactionTimerExpired(Transaction& trans) const
{
    trans.mOwner.mClientMessageDelegate(0, trans.mLocalTag);
    trans.ChangeState(&CLIENT_INVITE_TERMINATED_STATE);
    trans.InvokeTerminationHook();
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::ClientInviteProceedingState::SendMessage(Transaction& trans, const SipMessage& msg) const
{
    throw EPHXInternal("SipTransactionLayer::ClientInviteProceedingState::SendMessage");
}

void SipTransactionLayer::ClientInviteProceedingState::ReceiveMessage(Transaction& trans, const SipMessage& msg) const
{
    if (msg.status.Category() == SipMessage::SIP_STATUS_PROVISIONAL)  // 1xx
    {
        trans.mOwner.mClientMessageDelegate(&msg, msg.fromTag);
    }
    else if (msg.status.Category() == SipMessage::SIP_STATUS_SUCCESS)  // 2xx
    {
        trans.mOwner.mClientMessageDelegate(&msg, msg.fromTag);
        trans.ChangeState(&CLIENT_INVITE_TERMINATED_STATE);
        trans.InvokeTerminationHook();
    }
    else  // 300-699
    {
        if (trans.SendInternalAckRequest(msg))
        {
            trans.mTransactionTimer.Stop();
            trans.mTerminationTimer.Start(64 * TIMER_T1);
            trans.mOwner.mClientMessageDelegate(&msg, msg.fromTag);
            trans.ChangeState(&CLIENT_INVITE_COMPLETED_STATE);
        }
        else
        {
            trans.mOwner.mClientMessageDelegate(&msg, msg.fromTag);
            trans.ChangeState(&CLIENT_INVITE_TERMINATED_STATE);
            trans.InvokeTerminationHook();
        }
    }
}

void SipTransactionLayer::ClientInviteProceedingState::RetransmitTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ClientInviteProceedingState::RetransmitTimerExpired");
}

void SipTransactionLayer::ClientInviteProceedingState::TransactionTimerExpired(Transaction& trans) const
{
    trans.mOwner.mClientMessageDelegate(0, trans.mLocalTag);
    trans.ChangeState(&CLIENT_INVITE_TERMINATED_STATE);
    trans.InvokeTerminationHook();
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::ClientInviteCompletedState::SendMessage(Transaction& trans, const SipMessage& msg) const
{
    throw EPHXInternal("SipTransactionLayer::ClientInviteCompletedState::SendMessage");
}

void SipTransactionLayer::ClientInviteCompletedState::ReceiveMessage(Transaction& trans, const SipMessage& msg) const
{
    if (msg.status.Category() >= SipMessage::SIP_STATUS_REDIRECTION)  // 300-699
    {
        if (!trans.SendInternalAckRequest(msg))
        {
            trans.mOwner.mClientMessageDelegate(0, trans.mLocalTag);
            trans.ChangeState(&CLIENT_INVITE_TERMINATED_STATE);
            trans.InvokeTerminationHook();
        }
    }
}

void SipTransactionLayer::ClientInviteCompletedState::RetransmitTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ClientInviteCompletedState::RetransmitTimerExpired");
}

void SipTransactionLayer::ClientInviteCompletedState::TransactionTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ClientInviteCompletedState::TransactionTimerExpired");
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::ClientInviteTerminatedState::SendMessage(Transaction& trans, const SipMessage& msg) const
{
    throw EPHXInternal("SipTransactionLayer::ClientInviteTerminatedState::SendMessage");
}

void SipTransactionLayer::ClientInviteTerminatedState::ReceiveMessage(Transaction& trans, const SipMessage& msg) const
{
    throw EPHXInternal("SipTransactionLayer::ClientInviteTerminatedState::ReceiveMessage");
}

void SipTransactionLayer::ClientInviteTerminatedState::RetransmitTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ClientInviteTerminatedState::RetransmitTimerExpired");
}

void SipTransactionLayer::ClientInviteTerminatedState::TransactionTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ClientInviteTerminatedState::TransactionTimerExpired");
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::ClientNonInviteTryingState::SendMessage(Transaction& trans, const SipMessage& msg) const
{
    if (trans.SendMessage(msg))
    {
        trans.mRTT = TIMER_T1;
        trans.mRetransmitTimer.Start(trans.mRTT);  
        trans.mTransactionTimer.Start(64 * TIMER_T1);
        return true;
    }
    else
    {
        trans.ChangeState(&CLIENT_NON_INVITE_TERMINATED_STATE);
        trans.InvokeTerminationHook();
        return false;
    }
}

void SipTransactionLayer::ClientNonInviteTryingState::ReceiveMessage(Transaction& trans, const SipMessage& msg) const
{
    if (msg.status.Category() == SipMessage::SIP_STATUS_PROVISIONAL)  // 1xx
    {
        trans.mOwner.mClientMessageDelegate(&msg, msg.fromTag);
        trans.ChangeState(&CLIENT_NON_INVITE_PROCEEDING_STATE);
    }
    else  // 200-699
    {
        trans.mRetransmitTimer.Stop();
        trans.mTransactionTimer.Stop();
        trans.mTerminationTimer.Start(TIMER_T4);
        trans.mOwner.mClientMessageDelegate(&msg, msg.fromTag);
        trans.ChangeState(&CLIENT_NON_INVITE_COMPLETED_STATE);
    }
}

void SipTransactionLayer::ClientNonInviteTryingState::RetransmitTimerExpired(Transaction& trans) const
{
    if (trans.RetransmitMessage())
    {
        trans.mRTT = std::min(2 * trans.mRTT, TIMER_T2);
        trans.mRetransmitTimer.Start(trans.mRTT);
        TC_LOG_DEBUG_LOCAL(trans.mOwner.mPort, LOG_SIP, "[SipTransactionLayer::ClientNonInviteTryingState::RetransmitTimerExpired] Retransmitted with timeout = " << trans.mRTT.msec() << " msec");
    }
    else
    {
        trans.mOwner.mClientMessageDelegate(0, trans.mLocalTag);
        trans.ChangeState(&CLIENT_NON_INVITE_TERMINATED_STATE);
        trans.InvokeTerminationHook();
    }
}

void SipTransactionLayer::ClientNonInviteTryingState::TransactionTimerExpired(Transaction& trans) const
{
    trans.mOwner.mClientMessageDelegate(0, trans.mLocalTag);
    trans.ChangeState(&CLIENT_NON_INVITE_TERMINATED_STATE);
    trans.InvokeTerminationHook();
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::ClientNonInviteProceedingState::SendMessage(Transaction& trans, const SipMessage& msg) const
{
    throw EPHXInternal("SipTransactionLayer::ClientNonInviteProceedingState::SendMessage");
}

void SipTransactionLayer::ClientNonInviteProceedingState::ReceiveMessage(Transaction& trans, const SipMessage& msg) const
{
    if (msg.status.Category() == SipMessage::SIP_STATUS_PROVISIONAL)  // 1xx
    {
        trans.mOwner.mClientMessageDelegate(&msg, msg.fromTag);
    }
    else  // 200-699
    {
        trans.mRetransmitTimer.Stop();
        trans.mTransactionTimer.Stop();
        trans.mTerminationTimer.Start(TIMER_T4);
        trans.mOwner.mClientMessageDelegate(&msg, msg.fromTag);
        trans.ChangeState(&CLIENT_NON_INVITE_COMPLETED_STATE);
    }
}

void SipTransactionLayer::ClientNonInviteProceedingState::RetransmitTimerExpired(Transaction& trans) const
{
    if (trans.RetransmitMessage())
    {
        trans.mRTT = TIMER_T2;
        trans.mRetransmitTimer.Start(trans.mRTT);
        TC_LOG_DEBUG_LOCAL(trans.mOwner.mPort, LOG_SIP, "[SipTransactionLayer::ClientNonInviteProceedingState::RetransmitTimerExpired] Retransmitted with timeout = " << trans.mRTT.msec() << " msec");
    }
    else
    {
        trans.mOwner.mClientMessageDelegate(0, trans.mLocalTag);
        trans.ChangeState(&CLIENT_NON_INVITE_TERMINATED_STATE);
        trans.InvokeTerminationHook();
    }
}

void SipTransactionLayer::ClientNonInviteProceedingState::TransactionTimerExpired(Transaction& trans) const
{
    trans.mOwner.mClientMessageDelegate(0, trans.mLocalTag);
    trans.ChangeState(&CLIENT_NON_INVITE_TERMINATED_STATE);
    trans.InvokeTerminationHook();
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::ClientNonInviteCompletedState::SendMessage(Transaction& trans, const SipMessage& msg) const
{
    throw EPHXInternal("SipTransactionLayer::ClientNonInviteCompletedState::SendMessage");
}

void SipTransactionLayer::ClientNonInviteCompletedState::ReceiveMessage(Transaction& trans, const SipMessage& msg) const
{
}

void SipTransactionLayer::ClientNonInviteCompletedState::RetransmitTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ClientNonInviteCompletedState::RetransmitTimerExpired");
}

void SipTransactionLayer::ClientNonInviteCompletedState::TransactionTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ClientNonInviteCompletedState::TransactionTimerExpired");
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::ClientNonInviteTerminatedState::SendMessage(Transaction& trans, const SipMessage& msg) const
{
    throw EPHXInternal("SipTransactionLayer::ClientNonInviteTerminatedState::SendMessage");
}

void SipTransactionLayer::ClientNonInviteTerminatedState::ReceiveMessage(Transaction& trans, const SipMessage& msg) const
{
    throw EPHXInternal("SipTransactionLayer::ClientNonInviteTerminatedState::ReceiveMessage");
}

void SipTransactionLayer::ClientNonInviteTerminatedState::RetransmitTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ClientNonInviteTerminatedState::RetransmitTimerExpired");
}

void SipTransactionLayer::ClientNonInviteTerminatedState::TransactionTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ClientNonInviteTerminatedState::TransactionTimerExpired");
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::ServerInviteProceedingState::SendMessage(Transaction& trans, const SipMessage& msg) const
{
    if (trans.SendMessage(msg))
    {
        if (msg.status.Category() == SipMessage::SIP_STATUS_SUCCESS)  // 2xx
        {
            trans.ChangeState(&SERVER_INVITE_TERMINATED_STATE);
            trans.InvokeTerminationHook();
        }
        else if (msg.status.Category() >= SipMessage::SIP_STATUS_REDIRECTION)  // 300-699
        {
            trans.mRTT = TIMER_T1;
            trans.mRetransmitTimer.Start(trans.mRTT);
            trans.mTransactionTimer.Start(64 * TIMER_T1);
            trans.ChangeState(&SERVER_INVITE_COMPLETED_STATE);
        }

        return true;
    }
    else
    {
        // NB: no need to invoke server message delegate, UA core should be checking return value
        trans.ChangeState(&SERVER_INVITE_TERMINATED_STATE);
        trans.InvokeTerminationHook();
        return false;
    }
}

void SipTransactionLayer::ServerInviteProceedingState::ReceiveMessage(Transaction& trans, const SipMessage& msg) const
{
    if (!trans.SendInternalTryingResponse(msg))
    {
        // NB: no need to invoke server message delegate, UA core hasn't seen request yet
        trans.ChangeState(&SERVER_INVITE_TERMINATED_STATE);
        trans.InvokeTerminationHook();
    }
}

void SipTransactionLayer::ServerInviteProceedingState::RetransmitTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ServerInviteProceedingState::RetransmitTimerExpired");
}

void SipTransactionLayer::ServerInviteProceedingState::TransactionTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ServerInviteProceedingState::TransactionTimerExpired");
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::ServerInviteCompletedState::SendMessage(Transaction& trans, const SipMessage& msg) const
{
    throw EPHXInternal("SipTransactionLayer::ServerInviteCompletedState::SendMessage");
}

void SipTransactionLayer::ServerInviteCompletedState::ReceiveMessage(Transaction& trans, const SipMessage& msg) const
{
    if (msg.method == SipMessage::SIP_METHOD_INVITE)
    {
        if (trans.RetransmitMessage())
        {
            TC_LOG_DEBUG_LOCAL(trans.mOwner.mPort, LOG_SIP, "[SipTransactionLayer::ServerInviteCompletedState::ReceiveMessage] Retransmitted due to duplicate request");
        }
        else
        {
            trans.mOwner.mServerMessageDelegate(0, trans.mLocalTag);
            trans.ChangeState(&SERVER_INVITE_TERMINATED_STATE);
            trans.InvokeTerminationHook();
        }
    }
    else if (msg.method == SipMessage::SIP_METHOD_ACK)
    {
        trans.mRetransmitTimer.Stop();
        trans.mTransactionTimer.Stop();
        trans.mTerminationTimer.Start(TIMER_T4);
        trans.ChangeState(&SERVER_INVITE_CONFIRMED_STATE);
    }
    else
        throw EPHXInternal("SipTransactionLayer::ServerInviteCompletedState::ReceiveMessage");
}

void SipTransactionLayer::ServerInviteCompletedState::RetransmitTimerExpired(Transaction& trans) const
{
    if (trans.RetransmitMessage())
    {
        trans.mRTT = std::min(2 * trans.mRTT, TIMER_T2);
        trans.mRetransmitTimer.Start(trans.mRTT);
        TC_LOG_DEBUG_LOCAL(trans.mOwner.mPort, LOG_SIP, "[SipTransactionLayer::ServerInviteCompletedState::RetransmitTimerExpired] Retransmitted with timeout = " << trans.mRTT.msec() << " msec");
    }
    else
    {
        trans.mOwner.mServerMessageDelegate(0, trans.mLocalTag);
        trans.ChangeState(&SERVER_INVITE_TERMINATED_STATE);
        trans.InvokeTerminationHook();
    }
}

void SipTransactionLayer::ServerInviteCompletedState::TransactionTimerExpired(Transaction& trans) const
{
    trans.mOwner.mServerMessageDelegate(0, trans.mLocalTag);
    trans.ChangeState(&SERVER_INVITE_TERMINATED_STATE);
    trans.InvokeTerminationHook();
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::ServerInviteConfirmedState::SendMessage(Transaction& trans, const SipMessage& msg) const
{
    throw EPHXInternal("SipTransactionLayer::ServerInviteConfirmedState::SendMessage");
}

void SipTransactionLayer::ServerInviteConfirmedState::ReceiveMessage(Transaction& trans, const SipMessage& msg) const
{
    if (msg.method != SipMessage::SIP_METHOD_ACK)
        throw EPHXInternal("SipTransactionLayer::ServerInviteConfirmedState::ReceiveMessage");
}

void SipTransactionLayer::ServerInviteConfirmedState::RetransmitTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ServerInviteConfirmedState::RetransmitTimerExpired");
}

void SipTransactionLayer::ServerInviteConfirmedState::TransactionTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ServerInviteConfirmedState::TransactionTimerExpired");
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::ServerInviteTerminatedState::SendMessage(Transaction& trans, const SipMessage& msg) const
{
    throw EPHXInternal("SipTransactionLayer::ServerInviteTerminatedState::SendMessage");
}

void SipTransactionLayer::ServerInviteTerminatedState::ReceiveMessage(Transaction& trans, const SipMessage& msg) const
{
    throw EPHXInternal("SipTransactionLayer::ServerInviteTerminatedState::ReceiveMessage");
}

void SipTransactionLayer::ServerInviteTerminatedState::RetransmitTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ServerInviteTerminatedState::RetransmitTimerExpired");
}

void SipTransactionLayer::ServerInviteTerminatedState::TransactionTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ServerInviteTerminatedState::TransactionTimerExpired");
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::ServerNonInviteTryingState::SendMessage(Transaction& trans, const SipMessage& msg) const
{
    trans.SendMessage(msg);

    if (msg.status.Category() == SipMessage::SIP_STATUS_PROVISIONAL)  // 1xx
    {
        trans.ChangeState(&SERVER_NON_INVITE_PROCEEDING_STATE);
    }
    else  // 200-699
    {
        trans.mTerminationTimer.Start(64 * TIMER_T1);
        trans.ChangeState(&SERVER_NON_INVITE_COMPLETED_STATE);
    }

    return true;
}

void SipTransactionLayer::ServerNonInviteTryingState::ReceiveMessage(Transaction& trans, const SipMessage& msg) const
{
}

void SipTransactionLayer::ServerNonInviteTryingState::RetransmitTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ServerNonInviteTryingState::RetransmitTimerExpired");
}

void SipTransactionLayer::ServerNonInviteTryingState::TransactionTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ServerNonInviteTryingState::TransactionTimerExpired");
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::ServerNonInviteProceedingState::SendMessage(Transaction& trans, const SipMessage& msg) const
{
    trans.SendMessage(msg);

    if (msg.status.Category() >= SipMessage::SIP_STATUS_SUCCESS)  // 200-699
    {
        trans.mTerminationTimer.Start(64 * TIMER_T1);
        trans.ChangeState(&SERVER_NON_INVITE_COMPLETED_STATE);
    }

    return true;
}

void SipTransactionLayer::ServerNonInviteProceedingState::ReceiveMessage(Transaction& trans, const SipMessage& msg) const
{
    if (!trans.RetransmitMessage())
    {
        trans.mOwner.mServerMessageDelegate(0, trans.mLocalTag);
        trans.ChangeState(&SERVER_NON_INVITE_TERMINATED_STATE);
        trans.InvokeTerminationHook();
    }
}

void SipTransactionLayer::ServerNonInviteProceedingState::RetransmitTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ServerNonInviteProceedingState::RetransmitTimerExpired");
}

void SipTransactionLayer::ServerNonInviteProceedingState::TransactionTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ServerNonInviteProceedingState::TransactionTimerExpired");
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::ServerNonInviteCompletedState::SendMessage(Transaction& trans, const SipMessage& msg) const
{
    throw EPHXInternal("SipTransactionLayer::ServerNonInviteCompletedState::SendMessage");
}

void SipTransactionLayer::ServerNonInviteCompletedState::ReceiveMessage(Transaction& trans, const SipMessage& msg) const
{
    if (!trans.RetransmitMessage())
    {
        trans.mOwner.mServerMessageDelegate(0, trans.mLocalTag);
        trans.ChangeState(&SERVER_NON_INVITE_TERMINATED_STATE);
        trans.InvokeTerminationHook();
    }
}

void SipTransactionLayer::ServerNonInviteCompletedState::RetransmitTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ServerNonInviteCompletedState::RetransmitTimerExpired");
}

void SipTransactionLayer::ServerNonInviteCompletedState::TransactionTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ServerNonInviteCompletedState::TransactionTimerExpired");
}

///////////////////////////////////////////////////////////////////////////////

bool SipTransactionLayer::ServerNonInviteTerminatedState::SendMessage(Transaction& trans, const SipMessage& msg) const
{
    throw EPHXInternal("SipTransactionLayer::ServerNonInviteTerminatedState::SendMessage");
}

void SipTransactionLayer::ServerNonInviteTerminatedState::ReceiveMessage(Transaction& trans, const SipMessage& msg) const
{
    throw EPHXInternal("SipTransactionLayer::ServerNonInviteTerminatedState::ReceiveMessage");
}

void SipTransactionLayer::ServerNonInviteTerminatedState::RetransmitTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ServerNonInviteTerminatedState::RetransmitTimerExpired");
}

void SipTransactionLayer::ServerNonInviteTerminatedState::TransactionTimerExpired(Transaction& trans) const
{
    throw EPHXInternal("SipTransactionLayer::ServerNonInviteTerminatedState::TransactionTimerExpired");
}

///////////////////////////////////////////////////////////////////////////////
