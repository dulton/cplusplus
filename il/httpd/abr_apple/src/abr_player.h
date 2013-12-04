#ifndef __ABRPLAYER_H
#define __ABRPLAYER_H

#include <string>
#include <vector>

#include "abr_interfaces.h"
#include "abr_config.h"

using namespace std;


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
struct Player: virtual PlayerInterface, virtual StatusInterface, virtual Config, virtual SystemInterface
{
public:
    int     mID;
private:
    ////////////////////////////////////////////////////////////////////////////////////

// Any value, should be more than max length of parsed strings,
// set to typically delayed ack value, i.e 2*(TCP pay load, i.e 1460 bytes)
static const int PARSER_BUFFER_SIZE = 2920;
// Any value less than PARSER_BUFFER_SIZE,  equal to max length of parsed strings
static const int MINIMAL_NUMBER_OF_BYTES_TO_READ = 128;

private:
    ////////////////////////////////////////////////////////////////////////////////////
    struct FragmentDescriptor
    {
        int mNo;
        int mExpiredBefore;
        int mExpiredAfter;
        int mDuration;
        string mUrl;

        FragmentDescriptor()
        {
            mExpiredBefore = 0;
            mExpiredAfter = 0;
            mDuration = 0;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////

    struct VariantDescriptor
    {
        int mCompressionRate;
        URL mUrl;

       // vector<FragmentDescriptor> mFragment;

    };
    ////////////////////////////////////////////////////////////////////////////////////

    enum
    {
        TIMEOUT_DONE,
        TIMEOUT_CONNECT,
        TIMEOUT_TRASACTION,
        TIMEOUT_PLAYER
    };

    ////////////////////////////////////////////////////////////////////////////////////
    int mTimeNow;

    ////////////////////////////////////////////////////////////////////////////////////
    int mSessionStartTime;
    int mSessionDuration;

    ////////////////////////////////////////////////////////////////////////////////////
    int mIntervalStartTime;
    int mIntervalDuration;

    ////////////////////////////////////////////////////////////////////////////////////
    enum
    {
        BUFFERING,
        PLAYING
    } mPlaybackState;


    ////////////////////////////////////////////////////////////////////////////////////

    long mPlayerTimer;
    long mPlayerTimeout;
    int mFragmentDownloadRate;
    ////////////////////////////////////////////////////////////////////////////////////

    int mPlaybackBufferSize;
    int mPlaybackStartTime;
    int mPlaybackDuration;

    int mBufferingStartTime;
    int mBufferingDuration;

    ////////////////////////////////////////////////////////////////////////////////////

    vector<VariantDescriptor> mVariant;

    int mVariantCurrent;
    int mVariantFirst;
    int mVariantLast;

    vector<FragmentDescriptor> mFragment;
//    int mFragmentFirst;
    int mFragmentsPlayed;
    int mFragmentLast;
    int mFragmentCurrent;
    int mFragmentsAvailable;

    ////////////////////////////////////////////////////////////////////////////////////
    enum
    {
        HTTP_CONNECTION_CLOSED,
        HTTP_CONNECTION_OPENING,
        HTTP_CONNECTION_OPENED,
        HTTP_CONNECTION_SENDING,
        HTTP_CONNECTION_WAITING_FOR_REPLY,
        HTTP_CONNECTION_PARSING_HEADER,
        HTTP_CONNECTION_PARSING_BODY,
        HTTP_CONNECTION_REPLY_RECIEVED,
        HTTP_CONNECTION_CLOSING
    } mConnectionState;
public:
    enum ACTION
    {
        ACTION_NONE,
        ACTION_START,
        ACTION_STOP,
        ACTION_ABORT,
        ACTION_TIMEOUT,
        ACTION_GET_MANIFEST,
        ACTION_GET_PLAYLIST,
        ACTION_GET_FRAGMENT
    } mAction;
private:
    string mRequest;


    int mConnectionAttempt;
    long mConnectionTimer;
    int mTransactionAttempts;
    long mTransactionTimer;

    // should be accumulated on Read/Notify an Write/Notify operation Level
    int mBytesReceived;
    int mBytesSent;
    int mReceiveTime;
    int mSentTime;

    char mParserBuffer[PARSER_BUFFER_SIZE];
    char* mParserBegin;
    char* mParserEnd;
    int mBytesParsed;
    int mBytesUnparsed;

    struct
    {
        string mVersion;
        int mStatus;
        int mHeaderLength;
        int mContentLength;
        string mContentType;
        int mReplyLength;
    } mHttp;

    ////////////////////////////////////////////////////////////////////////////////////


    int mMediaSequence;
    int mTargetDuration;
    bool mEndList;

public:
    ////////////////////////////////////////////////////////////////////////////////////////
    int Play(string Url);
    int PlayAgain();
    void Stop();

    ////////////////////////////////////////////////////////////////////////////////////////
protected:
    int FindVariant(int uRate);

    ////////////////////////////////////////////////////////////////////////////////////////
protected:
    void NotifyTimeoutExpired(int id);

    void NotifyConnected();
    void NotifyConnectionBroken();
    void NotifyConnectError();

    void NotifyDisconnected();
    void NotifyClearToWrite();
    void NotifyClearToRead();

    void NotifyReadError();
    void NotifyWriteError();

    ////////////////////////////////////////////////////////////////////////////////////////
private:
    void GetPlaylist();
    void GetManifest();
    void GetFragment();

    void NotifyManifestRecieved();
    void NotifyPlaylistRecieved();
    void NotifyFragmentRecieved();

    void Continue();
    void NotifyTransactionError();

    ////////////////////////////////////////////////////////////////////////////////////////
    void InvokeFixedRateAlgorithm();
    void InvokeVariableReateNormAlgorithm();
    void InvokeVariableReateSmartAlgorithm();

    ////////////////////////////////////////////////////////////////////////////////////////
    void FatalError(string msg);

protected:
    string mServerIp;
    string mServerPort;

    int  AquireHttpConnection();
    int  AquireHttpConnection(string ServerIp, string ServerPort);
    int  AquireHttpConnection(string ServerId);
    void SendGetRequest(string mHost,string mPort,string mRequestLine);
    void ReleaseHttpConnection();
    void NotifyHttpConnectionReleased();
    void DestroyHttpConnection();

    void NoftifyHttpConnectionIsAvailable();
    void NoftifyHttpConnectionIsNotAvailable();

    void TryToConnect();

    void InitHttpHeaderParser();
    int  ParseHttpHeader(const char* mParserBegin, const char* mParserEnd);
    int  ParseHttpBody(const char* mParserBegin, const char* mParserEnd);
    void NotifyReplyRecieved();
    ////////////////////////////////////////////////////////////////////////////////////////

private:
    string mUrl;// private parser's variable

    void InitParsers();
    bool ParseUrl(URL* mUrl, string str);
    int ParseManifest(const char* mParserBegin, const char* mParserEnd);
    int ParseFragment(const char* mParserBegin, const char* mParserEnd);
    ////////////////////////////////////////////////////////////////////////////////////////
public:
    void PrintPlaylist();
    void PrintRequest();
    void PrintStatus();
    ////////////////////////////////////////////////////////////////////////////////////////
public:
    Player();
    ~Player();
};


#endif


