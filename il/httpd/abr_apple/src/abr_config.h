#ifndef __ABRCONFIG_H
#define __ABRCONFIG_H

#include "abr_interfaces.h"


#define DEFAULT_MAXIMUM_CONNECTION_ATEMPTS (10 )
#define DEFAULT_CONNECTION_TIMEOUT (4*1000)
#define DEFAULT_MAXIMUM_TRANSACTION_ATEMPTS (1 )
#define DEFAULT_TRANSACTION_TIMEOUT (60*1000)

////////////////////////////////////////////////////////////////////////////////////////
struct Config: virtual ConfigurationInterface
{
    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////

    struct
    {
        URL mUrl;
        int mHttpVersion;
        string mUserAgent;

        int mProgramID;
        int mBitrateMin;
        int mBitrateMax;

        int mShiftAlgorithm;
        int mParam1;
        int mParam2;

        int mPlaybackBufferInitialSize;
        int mPlaybackBufferMinSize;
        int mPlaybackBufferMaxSize;

        int mPlaybackMaxDuration;

        int mMaximumConnectionAttempts;
        int mConnectionTimeout;

        int mMaximumTransactionAttempts;
        int mTransactionTimeout;

        bool mKeepAlive;
    } mCfg;

    ////////////////////////////////////////////////////////////
    void SetHttpVersion(int aParam = HTTP_VERSION_10)
    {
        mCfg.mHttpVersion = aParam;
    }

    ////////////////////////////////////////////////////////////
    void SetKeepAliveFlag(bool aParam = false)
    {
        mCfg.mKeepAlive = aParam;
    }

    ////////////////////////////////////////////////////////////
    void SetUserAgentString(string aParam)
    {
        mCfg.mUserAgent = aParam;
    }

    ////////////////////////////////////////////////////////////
    void SetMaximumConnectionAttempts(int aParam)
    {
        mCfg.mMaximumConnectionAttempts = aParam;
    }

    ////////////////////////////////////////////////////////////
    void SetConnectionTimeout(int aParam)
    {
        mCfg.mConnectionTimeout = aParam;
    }

    ////////////////////////////////////////////////////////////
    void SetMaximumTransactionAttempts(int aParam)
    {
        mCfg.mMaximumTransactionAttempts = aParam;
    }

    ////////////////////////////////////////////////////////////
    void SetTransactionTimeout(int aParam)
    {
        mCfg.mTransactionTimeout = aParam;
    }

    ////////////////////////////////////////////////////////////
    void SetShiftAlgorithm(int aShiftAlgorithm, int aParam1, int aParam2)
    {
        mCfg.mShiftAlgorithm = aShiftAlgorithm;
        mCfg.mParam1 = aParam1;
        mCfg.mParam2 = aParam2;
    }


    ////////////////////////////////////////////////////////////
    void SetPlaybackBufferInitialSize(int aParam)
    {
        mCfg.mPlaybackBufferInitialSize = aParam;
    }

    ////////////////////////////////////////////////////////////
    void SetPlaybackBufferMaxSize(int aParam)
    {
        mCfg.mPlaybackBufferMaxSize = aParam;
    }

    ////////////////////////////////////////////////////////////
    void SetPlaybackBufferMinSize(int aParam)
    {
        mCfg.mPlaybackBufferMinSize = aParam;
    }

    ////////////////////////////////////////////////////////////
    void SetProgramID(int aParam)
    {
        mCfg.mProgramID = aParam;
    }

    ////////////////////////////////////////////////////////////
    void SetPlayDuration(int aParam)
    {
        mCfg.mPlaybackMaxDuration = aParam;
    }

    ////////////////////////////////////////////////////////////
    void SetBitrateMax(int aParam)
    {
        mCfg.mBitrateMax = aParam;
    }
    ////////////////////////////////////////////////////////////
    void SetBitrateMin(int aParam)
    {
        mCfg.mBitrateMin = aParam;
    }

    Config()
    {
        SetHttpVersion(HTTP_VERSION_10);
        SetUserAgentString("Generic");
        SetKeepAliveFlag(false);

        SetMaximumConnectionAttempts(DEFAULT_MAXIMUM_CONNECTION_ATEMPTS);
        SetConnectionTimeout(DEFAULT_CONNECTION_TIMEOUT);
        SetMaximumTransactionAttempts(DEFAULT_MAXIMUM_TRANSACTION_ATEMPTS);
        SetTransactionTimeout(DEFAULT_TRANSACTION_TIMEOUT);

        SetProgramID(0);
        SetBitrateMin(0);
        SetBitrateMax(1000000000);

        SetShiftAlgorithm(ALGORITHM_CONST, 50, 80);
        SetPlayDuration(((0 * 60 + 1) * 60 + 0) * 1000);

        SetPlaybackBufferInitialSize(30*1000);
        SetPlaybackBufferMaxSize(30 * 1000);
        SetPlaybackBufferMinSize(0 * 1000);
    }
};

#endif
