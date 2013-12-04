#ifndef __ABRINTERFACES_H
#define __ABRINTERFACES_H

#include <string>

using namespace std;

struct SystemInterface
{
protected:
    virtual int Time() = 0;

    ////////////////////////////////////////////////////////////////////////////////////
    virtual long StartTimeout(int,int) = 0;
    virtual void CancelTimeout(long) = 0;
    virtual void NotifyTimeoutExpired(int id)=0;

    ////////////////////////////////////////////////////////////////////////////////////


    virtual int ConfigureConnector(string ServerIp, string ServerPort)=0;
    virtual int Connect()=0;

    virtual bool IsConnected() = 0;
    virtual int Disconnect()=0;

    virtual void ClearToWrite()=0;
    virtual int Write(void*, int)=0;
    virtual void ClearToRead()=0;
    virtual int Read(void*, int)=0;

    virtual void NotifyConnected() = 0;
    virtual void NotifyConnectionBroken()= 0;
    virtual void NotifyDisconnected()= 0;
    virtual void NotifyClearToWrite()=0;
    virtual void NotifyClearToRead()=0;

    virtual void NotifyReadError() = 0;
    virtual void NotifyWriteError() = 0;

    virtual void NotifyStatusChanged(int uStatus)=0;
    ////////////////////////////////////////////////////////////////////////////////////
    virtual ~SystemInterface()
    {
    }
};


////////////////////////////////////////////////////////////////////////////////////////
struct ConfigurationInterface
{
public:
    enum HTTP_VERSION
    {
        HTTP_VERSION_10,
        HTTP_VERSION_11,
        HTTP_VERSION_20
    };

    enum
    {
        ALGORITHM_CONST,
        ALGORITHM_NORM,
        ALGORITHM_SMART
    };

public:
    virtual void SetHttpVersion(int aParam)=0;
    virtual void SetKeepAliveFlag(bool aParam)=0;
    virtual void SetUserAgentString(string aParam)=0;
    virtual void SetMaximumConnectionAttempts(int aParam)=0;
    virtual void SetConnectionTimeout(int aParam)=0;
    virtual void SetMaximumTransactionAttempts(int aParam)=0;
    virtual void SetTransactionTimeout(int aParam)=0;
    virtual void SetShiftAlgorithm(int aShiftAlgorithm, int aParam1, int aParam2)=0;
    virtual void SetPlaybackBufferMaxSize(int aParam)=0;
    virtual void SetPlaybackBufferMinSize(int aParam)=0;
    virtual void SetProgramID(int aParam)=0;
    virtual void SetPlayDuration(int aParam)=0;
    virtual void SetBitrateMax(int aParam)=0;
    virtual void SetBitrateMin(int aParam)=0;
protected:
    virtual ~ConfigurationInterface()
    {
    }
};


////////////////////////////////////////////////////////////////////////////////////////
struct StatusInfo
{
public:
    enum
    {
        RATE_0_040,

        RATE_0_064,
        RATE_0_096,
        RATE_0_150,
        RATE_0_240,
        RATE_0_256,
        RATE_0_440,
        RATE_0_640,
        RATE_0_800,
        RATE_0_840,
        RATE_1_240,
        RATE_MAX
    };
    ///////////////////////////////////////////////////////////////////////////////

    enum
    {
        TRANSACTION_ATTEMPTED,
        TRANSACTION_SUCESSFUL,
        TRANSACTION_FAILED,
        TRANSACTION_MAX
    };
    ///////////////////////////////////////////////////////////////////////////////

    enum
    {
        HTTP_STATUS_200,
        HTTP_STATUS_400,
        HTTP_STATUS_404,
        HTTP_STATUS_405,
        HTTP_STATUS_MAX
    };
    ///////////////////////////////////////////////////////////////////////////////

    int mHttpStatus[HTTP_STATUS_MAX];
    int mHttpResponseTime;

    ///////////////////////////////////////////////////////////////////////////////
    int mBytesSent;
    int mBytesReceived;

    ///////////////////////////////////////////////////////////////////////////////
    int mBufferingDuration;
    int mBufferingCount;

    ///////////////////////////////////////////////////////////////////////////////
    int mGetManifestTransactions[TRANSACTION_MAX];
    int mGetFragmentTransactions[RATE_MAX];
    int mPlaybackDuration[RATE_MAX];

};

///////////////////////////////////////////////////////////////////////////////////
struct StatusInterface
{
public:
    StatusInfo mCurrentInfo;
    StatusInfo mPreviousInfo;
public:
    virtual void GetStatusInfo(StatusInfo* aResult);
protected:
    void InitStatus();
    StatusInterface();
    virtual ~StatusInterface(){};
};

///////////////////////////////////////////////////////////////////////////////////
struct PlayerInterface
{
public:
    virtual int Play(string Url)=0;
    virtual int PlayAgain()=0;
    virtual void Stop()=0;
    virtual ~PlayerInterface()
    {
    }
};


///////////////////////////////////////////////////////////////////////////////////
struct URL
{
    string mSchemeName;
    string mDomainName;
    string mIpAddress;
    string mPortNumber;
    string mRoot;
    string mPath;
    string mProgram;
    string mQuery;
};

#endif
