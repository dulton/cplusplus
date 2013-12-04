#include "abr_interfaces.h"


void StatusInterface::GetStatusInfo(StatusInfo* uResult)
{
    StatusInfo uSnapShot;
    uSnapShot = mCurrentInfo;

    ///////////////////////////////////////////////////////////////////////////////
    int i;

    for (i = StatusInfo::HTTP_STATUS_200; i < StatusInfo::HTTP_STATUS_MAX; i++)
    {
        uResult->mHttpStatus[i] = uSnapShot.mHttpStatus[i] - mPreviousInfo.mHttpStatus[i];
    }

    ///////////////////////////////////////////////////////////////////////////////
    uResult->mHttpResponseTime = uSnapShot.mHttpResponseTime - mPreviousInfo.mHttpResponseTime;
    uResult->mBytesSent = uSnapShot.mBytesSent - mPreviousInfo.mBytesSent;
    uResult->mBytesReceived = uSnapShot.mBytesReceived - mPreviousInfo.mBytesReceived;
    uResult->mBufferingDuration = uSnapShot.mBufferingDuration - mPreviousInfo.mBufferingDuration;
    uResult->mBufferingCount = uSnapShot.mBufferingCount - mPreviousInfo.mBufferingCount;

    ///////////////////////////////////////////////////////////////////////////////
    for (i = 0; i < StatusInfo::TRANSACTION_MAX; i++)
    {
        uResult->mGetManifestTransactions[i] = uSnapShot.mGetManifestTransactions[i] - mPreviousInfo.mGetManifestTransactions[i];
    }

    ///////////////////////////////////////////////////////////////////////////////
    for (i = 0; i < StatusInfo::RATE_MAX; i++)
    {
        uResult->mGetFragmentTransactions[i] = uSnapShot.mGetFragmentTransactions[i] - mPreviousInfo.mGetFragmentTransactions[i];
    }

    mPreviousInfo = uSnapShot;
}

StatusInterface::StatusInterface()
{
    InitStatus();
}

void StatusInterface::InitStatus()
{
    mPreviousInfo = mCurrentInfo;
}

