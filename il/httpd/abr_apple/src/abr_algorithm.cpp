#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <set>
#include <string>
#include <string.h>
#include <iomanip>

using namespace std;

#include "abr_player.h"

////////////////////////////////////////////////////////////////////////////////////
void Player::Stop()
{
    CancelTimeout(mConnectionTimer);
    CancelTimeout(mTransactionTimer);
    CancelTimeout(mPlayerTimer);
    Disconnect();
}

////////////////////////////////////////////////////////////////////////////////////
int Player::Play(string Url)
{
    mCfg.mUrl.mSchemeName = "http://";
    int result = ParseUrl(&mCfg.mUrl, Url);
    if (result) PlayAgain();
    return result;
}

////////////////////////////////////////////////////////////////////////////////////
int Player::PlayAgain()
{
    InitStatus();
    mAction = ACTION_NONE;
    //mPlayerTimer = StartTimeout(TIMEOUT_PLAYER, 1000);
    Continue();
    return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
void Player::NotifyManifestRecieved()
{
    Continue();
}

////////////////////////////////////////////////////////////////////////////////////////
void Player::NotifyPlaylistRecieved()
{
    Continue();
}

////////////////////////////////////////////////////////////////////////////////////////
void Player::NotifyFragmentRecieved()
{
    Continue();
}

////////////////////////////////////////////////////////////////////////////////////////
int Player::FindVariant(int uRate)
{
    unsigned int uNo = 0;
    unsigned int uVariantNo = 0;

    for (uNo = 0; uNo < mVariant.size(); uNo++)
    {
        if (uRate > mVariant[uVariantNo].mCompressionRate) uVariantNo = uNo;
    }
    return uVariantNo;
}

////////////////////////////////////////////////////////////////////////////////////////
void Player::Continue()
{
    ACTION uActionDone = mAction;
    ACTION uActionToDo = ACTION_NONE;
    int uEstimationOfPlaybackBufferSize;
    int uRate =0;

    mAction = ACTION_NONE;

    mTimeNow = Time();
    mSessionDuration = mTimeNow - mSessionStartTime;
    mIntervalDuration = mTimeNow - mIntervalStartTime;


    mIntervalStartTime = mTimeNow;

    switch (uActionDone)
    {
        case ACTION_GET_MANIFEST:
        case ACTION_GET_PLAYLIST:
        case ACTION_GET_FRAGMENT:
            CancelTimeout(mTransactionTimer);
        break;
        default:
        break;
    }

    //////////////////////LOCAL STATS////////////////////////////////////////
    if (uActionDone != ACTION_NONE)
        uRate = mVariant[mVariantCurrent].mCompressionRate;

    switch (uActionDone)
    {
        case ACTION_GET_MANIFEST:
            mCurrentInfo.mGetManifestTransactions[StatusInfo::TRANSACTION_SUCESSFUL]++;
        break;

        case ACTION_GET_PLAYLIST:
            mCurrentInfo.mGetManifestTransactions[StatusInfo::TRANSACTION_SUCESSFUL]++;
            PrintPlaylist();
        break;

        case ACTION_GET_FRAGMENT:
            if (uRate < 96000) mCurrentInfo.mGetFragmentTransactions[StatusInfo::RATE_0_064]++;
            else if (uRate < 150000) mCurrentInfo.mGetFragmentTransactions[StatusInfo::RATE_0_096]++;
            else if (uRate < 240000) mCurrentInfo.mGetFragmentTransactions[StatusInfo::RATE_0_150]++;
            else if (uRate < 256000) mCurrentInfo.mGetFragmentTransactions[StatusInfo::RATE_0_240]++;
            else if (uRate < 440000) mCurrentInfo.mGetFragmentTransactions[StatusInfo::RATE_0_256]++;
            else if (uRate < 640000) mCurrentInfo.mGetFragmentTransactions[StatusInfo::RATE_0_440]++;
            else if (uRate < 800000) mCurrentInfo.mGetFragmentTransactions[StatusInfo::RATE_0_640]++;
            else if (uRate < 840000) mCurrentInfo.mGetFragmentTransactions[StatusInfo::RATE_0_800]++;
            else if (uRate < 1240000) mCurrentInfo.mGetFragmentTransactions[StatusInfo::RATE_0_840]++;
            else mCurrentInfo.mGetFragmentTransactions[StatusInfo::RATE_1_240]++;
        break;

        default:
        break;
    }

    //////////////////////GLOBAL STATS////////////////////////////////////////
    switch (uActionDone)
    {
        case ACTION_GET_MANIFEST:
        case ACTION_GET_PLAYLIST:
        case ACTION_GET_FRAGMENT:
            mCurrentInfo.mHttpStatus[StatusInfo::HTTP_STATUS_200]++;

            mCurrentInfo.mBytesSent += mBytesSent;
            mCurrentInfo.mBytesReceived += mBytesReceived;
            mCurrentInfo.mHttpResponseTime += mIntervalDuration;

        break;
        default:
        break;
    }

    ////////////////////////MACHINE/////////////////////////////////////////////
    switch (uActionDone)
    {
        case ACTION_NONE:

            mIntervalStartTime = mTimeNow;
            mIntervalDuration = 0;

            ///////////////////////////////////////////////////////////////////////////////
            mSessionStartTime = mTimeNow;
            mSessionDuration = 0;

            ///////////////////////////////////////////////////////////////////////////////
            mPlaybackStartTime = mTimeNow;
            mPlaybackDuration = 0;

            ///////////////////////////////////////////////////////////////////////////////
            mPlaybackBufferSize = mCfg.mPlaybackBufferInitialSize;

            if (mPlaybackBufferSize > 0) mPlaybackState = BUFFERING;
            else mPlaybackState = PLAYING;

            mBufferingStartTime = mTimeNow;
            mBufferingDuration = 0;

            ///////////////////////////////////////////////////////////////////////////////
            mFragmentDownloadRate = 0;
            ///////////////////////////////////////////////////////////////////////////////
            mVariant.clear();
            mFragment.clear();

            mVariantCurrent = 0;
            mVariantFirst = 1;
            mVariantLast = 1;

            mFragmentsPlayed = 0;
            mFragmentLast = 0;
            mFragmentCurrent = 0;
            mFragmentsAvailable = 0;
            mEndList = false;

            ///////////////////////////////////////////////////////////////////////////////
            mBytesReceived = 0;
            mBytesSent = 0;
            mConnectionState = HTTP_CONNECTION_CLOSED;
            uActionToDo = ACTION_GET_MANIFEST;
        break;

        case ACTION_GET_FRAGMENT:
            mFragmentDownloadRate = mBytesReceived / mIntervalDuration / mFragment[mFragmentCurrent].mDuration * 8000;
            mPlaybackBufferSize += mFragment[mFragmentCurrent].mDuration * 1000;
            mFragmentCurrent += 1;
            mFragmentsPlayed += 1;
            mFragmentsAvailable -= 1;
        break;

        default:
        break;
    }

    //////////////////////////////////////////////////////////////////////////////
    if (mPlaybackState == PLAYING)
    {
        mPlaybackBufferSize -= mIntervalDuration;
        mPlaybackDuration += mIntervalDuration;

        if (mPlaybackBufferSize <= 0)
        {
            mBufferingStartTime = mTimeNow;
            mPlaybackState = BUFFERING;
        }
    }

    uEstimationOfPlaybackBufferSize = mPlaybackBufferSize;
    if (mFragmentsAvailable)
    {
        uEstimationOfPlaybackBufferSize += mFragment[mFragmentCurrent].mDuration * 1000;
    }

    if (mPlaybackState == BUFFERING)
    {
        mBufferingDuration += mIntervalDuration;

        if (uEstimationOfPlaybackBufferSize >= mCfg.mPlaybackBufferMaxSize)
        {

            mCurrentInfo.mBufferingDuration += mTimeNow - mBufferingStartTime;
            mCurrentInfo.mBufferingCount += 1;

            mPlaybackStartTime = mTimeNow;
            mPlaybackState = PLAYING;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (mBufferingDuration>30)
    {
        // make it unsuccessful
    }

    // CHOSING RATE
    if (uActionDone == ACTION_GET_FRAGMENT && uActionToDo == ACTION_NONE)
    {
        int uVariantNo   = FindVariant(mFragmentDownloadRate);
        int uDelta       = uVariantNo - mVariantCurrent;

        if (this->mCfg.mShiftAlgorithm == ALGORITHM_NORM)
        {
            if (uDelta > 1) uDelta = 1;
            if (uDelta < -1) uDelta = -1;
        }

        if (uDelta)
        {
            mVariantCurrent += uDelta;
//            cout <<" DownloadRate  "<< mFragmentDownloadRate <<endl;
//            cout << "Chosing mVariant["<<mVariantCurrent<<"]="<<mVariant[mVariantCurrent].mCompressionRate <<" bps"  << endl;
            mFragment.clear();
            mFragmentLast = mFragmentsPlayed;
            mFragmentCurrent = 0;
            mFragmentsAvailable = 0;
            uActionToDo = ACTION_GET_PLAYLIST;
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    //  GET_PLAYLIST
    if (uActionDone != ACTION_GET_PLAYLIST && uActionToDo == ACTION_NONE)
    {
        if (!mEndList)
        {
            if (mFragmentsAvailable < 3)
            {
               //Check For Interval;
                uActionToDo = ACTION_GET_PLAYLIST;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // DO_TIMEOUT OR GET_FRAGMENT
    if (uActionToDo == ACTION_NONE)
    {
        mPlayerTimeout = uEstimationOfPlaybackBufferSize - mCfg.mPlaybackBufferMaxSize;

        if (mPlayerTimeout >= 0) uActionToDo = ACTION_TIMEOUT;
        else if (mFragmentsAvailable) uActionToDo = ACTION_GET_FRAGMENT;
    }

    //////////////////////////////////////////////////////////////////////////////
    // DO_TIMEOUT  // Optional timeout until play back buffer will be empty
    if (uActionToDo == ACTION_NONE )
    {
        if (mFragmentsAvailable <= 0)
        {
            int uRemainingSizeOfPlaybackBuffer = mPlaybackBufferSize - mCfg.mPlaybackBufferInitialSize;

            if (uRemainingSizeOfPlaybackBuffer > 0)
            {
                mPlayerTimeout = uRemainingSizeOfPlaybackBuffer;
                uActionToDo = ACTION_TIMEOUT;
            }
            else
            {
                uActionToDo = ACTION_STOP;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////


    if (mSessionDuration > mCfg.mPlaybackMaxDuration) uActionToDo = ACTION_STOP;

    mAction = uActionToDo;


    PrintStatus();

    NotifyStatusChanged(uActionToDo);

    switch (uActionToDo)
    {
        case ACTION_NONE:
        break;

        case ACTION_START:
        break;

        case ACTION_GET_MANIFEST:
            mTransactionTimer = StartTimeout(TIMEOUT_TRASACTION, mCfg.mTransactionTimeout);
            GetManifest();
        break;

        case ACTION_GET_PLAYLIST:
            mTransactionTimer = StartTimeout(TIMEOUT_TRASACTION, mCfg.mTransactionTimeout);
            GetPlaylist();
        break;

        case ACTION_GET_FRAGMENT:
            mTransactionTimer = StartTimeout(TIMEOUT_TRASACTION, mCfg.mTransactionTimeout);
            GetFragment();
        break;

        case ACTION_TIMEOUT:
            mPlayerTimer = StartTimeout(TIMEOUT_PLAYER, mPlayerTimeout+100);
        break;

        case ACTION_ABORT:
            break;

        case ACTION_STOP:
            break;

    }
}

////////////////////////////////////////////////////////////////////////////////////////
Player::Player()
{
    mID = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
Player::~Player()
{

}


