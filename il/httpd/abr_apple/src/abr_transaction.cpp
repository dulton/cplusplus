#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <set>
#include <string>
#include <string.h>

using namespace std;


#include "abr_player.h"

//#define DUMP_REQUEST
//#define DUMP_REPLY

////////////////////////////////////////////////////////////////////////////////////
int Player::AquireHttpConnection(string ServerIp, string ServerPort)
{
    if (mServerIp!=ServerIp || mServerPort!=ServerPort)
    {
        mServerIp=ServerIp;
        mServerPort=ServerPort;
        ConfigureConnector(mServerIp, mServerPort);
    }
    AquireHttpConnection();
    return 1;
}

////////////////////////////////////////////////////////////////////////////////////
int Player::AquireHttpConnection(string ServerId)
{
    if (mServerIp != ServerId)
    {
        mServerIp = ServerId;
        mServerPort = "80";
        ConfigureConnector(mServerIp, mServerPort);
    }
    AquireHttpConnection();
    return 1;
}


////////////////////////////////////////////////////////////////////////////////////
void Player::TryToConnect()
{

    if (mConnectionAttempt < mCfg.mMaximumConnectionAttempts)
    {
        mConnectionState = HTTP_CONNECTION_OPENING;
        mConnectionAttempt += 1;
        mConnectionTimer = StartTimeout(TIMEOUT_CONNECT, mCfg.mConnectionTimeout);
        Connect();
    }
    else
    {
        mConnectionState = HTTP_CONNECTION_CLOSED;
        NoftifyHttpConnectionIsNotAvailable();
    }
}

////////////////////////////////////////////////////////////////////////////////////
int Player::AquireHttpConnection()
{
    switch (mConnectionState)
    {
        case HTTP_CONNECTION_OPENED:
//            NoftifyHttpConnectionIsAvailable();
//        break;

        case HTTP_CONNECTION_CLOSED:
            mConnectionState = HTTP_CONNECTION_OPENING;
            mConnectionAttempt = 0;
            TryToConnect();
        break;

        default:
        break;
    }
    return 0;
}


    ////////////////////////////////////////////////////////////////////////////////////
void Player::NotifyConnected()
{
    CancelTimeout(mConnectionTimer);
    mConnectionState = HTTP_CONNECTION_OPENED;
    NoftifyHttpConnectionIsAvailable();
}

////////////////////////////////////////////////////////////////////////////////////
void Player::NotifyConnectError()
{
    CancelTimeout(mConnectionTimer);
    mConnectionState = HTTP_CONNECTION_CLOSED;
    NoftifyHttpConnectionIsNotAvailable();
}

////////////////////////////////////////////////////////////////////////////////////
void Player::ReleaseHttpConnection()
{
//    if (!mCfg.mKeepAlive)
//    {
        mConnectionState = HTTP_CONNECTION_CLOSING;
        Disconnect();
//    }
//    else
//    {
//        mConnectionState = HTTP_CONNECTION_CLOSED;
//        NotifyHttpConnectionReleased();
//    }
}

///////////////////////////////////////////////////////////////////////////////////
void Player::NotifyDisconnected()
{
    switch (mConnectionState)
    {
        case HTTP_CONNECTION_CLOSING:
            mConnectionState = HTTP_CONNECTION_CLOSED;
            NotifyHttpConnectionReleased();
        break;

        default:
//            mConnectionState = HTTP_CONNECTION_CLOSED;
        break;
    }
}


////////////////////////////////////////////////////////////////////////////////////
void Player::NotifyClearToRead()
{
    int nBytesRead = 0;

    switch (mConnectionState)
    {
        case HTTP_CONNECTION_WAITING_FOR_REPLY:
            mBytesReceived = 0;
            mParserBegin = mParserBuffer;
            mParserEnd = mParserBuffer;
            mBytesParsed = 0;
            mBytesUnparsed = 0;

            mHttp.mHeaderLength = 0;
            mHttp.mContentLength = 0;
            mHttp.mReplyLength = 0;
            mConnectionState = HTTP_CONNECTION_PARSING_HEADER;
#ifdef DUMP_REPLY
            cout << "<Relply>"<<endl;
            cout << "  <Header>"<<endl;
#endif
        case HTTP_CONNECTION_PARSING_BODY:
        case HTTP_CONNECTION_PARSING_HEADER:
            if (nBytesRead < MINIMAL_NUMBER_OF_BYTES_TO_READ)
            {
                nBytesRead = mParserEnd - mParserBegin;
                memcpy(mParserBuffer, mParserBegin, nBytesRead);
                mParserBegin = mParserBuffer;
                mParserEnd = mParserBuffer + nBytesRead;
                mBytesUnparsed = nBytesRead;
                nBytesRead = PARSER_BUFFER_SIZE - mBytesUnparsed;
            }
        break;
        default:
        break;
    }

    if (nBytesRead) nBytesRead = Read(mParserBuffer + mBytesUnparsed, nBytesRead);
    if (nBytesRead)
    {
        int uBytesUnparsed;
        int uBytesParsed;

        mBytesReceived += nBytesRead;
        mBytesUnparsed += nBytesRead;

        mParserEnd = mParserBegin + mBytesUnparsed;

        if (mBytesUnparsed && mConnectionState == HTTP_CONNECTION_PARSING_HEADER)
        {
            uBytesUnparsed = ParseHttpHeader((const char*) mParserBegin, (const char*) mParserEnd);

            uBytesParsed = mParserEnd - mParserBegin - uBytesUnparsed;
#ifdef DUMP_REPLY
            cout<<string(mParserBegin,uBytesParsed);
#endif
            mBytesUnparsed -= uBytesParsed;
            mBytesParsed += uBytesParsed;
            mParserBegin += uBytesParsed;

            if (mBytesParsed == mHttp.mHeaderLength)
            {
                mConnectionState = HTTP_CONNECTION_PARSING_BODY;
#ifdef DUMP_REPLY
                cout << "  </Header>"<<endl;
                cout << "  <Body>"<<endl;
#endif
            }

        }

        if (mBytesUnparsed && mConnectionState == HTTP_CONNECTION_PARSING_BODY)
        {
            uBytesUnparsed = ParseHttpBody((const char*) mParserBegin, (const char*) mParserEnd);

            uBytesParsed = mParserEnd - mParserBegin - uBytesUnparsed;
#ifdef DUMP_REPLY
            if (mAction == ACTION_GET_MANIFEST || mAction == ACTION_GET_PLAYLIST) cout << string(mParserBegin, uBytesParsed);
#endif
            mBytesUnparsed -= uBytesParsed;
            mBytesParsed += uBytesParsed;
            mParserBegin += uBytesParsed;

        }

        if (mBytesParsed == mHttp.mReplyLength)
        {
#ifdef DUMP_REPLY
            cout << "  </Body>"<<endl;
            cout << "</Relply>"<<endl<<endl;
#endif
            mConnectionState = HTTP_CONNECTION_REPLY_RECIEVED;
            NotifyReplyRecieved();
        }
        else ClearToRead();
    }
}

void Player::SendGetRequest(string mHost,string mPort, string mResource)
{
    switch (mCfg.mHttpVersion)
    {
        case HTTP_VERSION_10:
            mRequest = "GET " + mResource + " HTTP/1.0\r\n";
            mRequest += "Host:" + mHost + "\r\n";
            mRequest += "Agent:" + mCfg.mUserAgent + "\r\n";
            mRequest += "\r\n";
        break;

        case HTTP_VERSION_11:
            mRequest = "GET " + mResource + " HTTP/1.1\r\n";
            mRequest += "Host:" + mHost + "\r\n";
            mRequest += "Agent:" + mCfg.mUserAgent + "\r\n";
            if (mCfg.mKeepAlive)
            {
                mRequest += "Connection: Keep-Alive\r\n";
            }
            else
             {
                 mRequest += "Connection: close\r\n";
             }
            mRequest += "\r\n";
        break;
        case HTTP_VERSION_20:
        break;
    }

    AquireHttpConnection(mHost,mPort);
}

////////////////////////////////////////////////////////////////////////////////////
void Player::GetManifest()
{
    string mDomainName = mCfg.mUrl.mDomainName;
    string mResource = mCfg.mUrl.mRoot + mCfg.mUrl.mPath + mCfg.mUrl.mProgram;
    string mPortNumber = mCfg.mUrl.mPortNumber;
    SendGetRequest(mDomainName, mPortNumber, mResource);
}

////////////////////////////////////////////////////////////////////////////////////
void Player::GetPlaylist()
{
    string mDomainName = mVariant[mVariantCurrent].mUrl.mDomainName;
    string mResource = mVariant[mVariantCurrent].mUrl.mRoot + mVariant[mVariantCurrent].mUrl.mPath + mVariant[mVariantCurrent].mUrl.mProgram;
    string mPortNumber = mVariant[mVariantCurrent].mUrl.mPortNumber;

    SendGetRequest(mDomainName, mPortNumber, mResource);
}

////////////////////////////////////////////////////////////////////////////////////
void Player::GetFragment()
{
    URL mUrl= mVariant[mVariantCurrent].mUrl;

    ParseUrl(&mUrl, mFragment[mFragmentCurrent].mUrl);

    string mDomainName = mUrl.mDomainName;
    string mResource = mUrl.mRoot + mUrl.mPath+mUrl.mProgram;
    string mPortNumber = mUrl.mPortNumber;

    SendGetRequest(mDomainName, mPortNumber, mResource);
}

////////////////////////////////////////////////////////////////////////////////////
void Player::NotifyClearToWrite()
{
    if (mConnectionState == HTTP_CONNECTION_SENDING)
    {
        mBytesSent = mRequest.size();
        mConnectionState = HTTP_CONNECTION_WAITING_FOR_REPLY;
        PrintRequest();
        Write((void*) mRequest.c_str(), mRequest.size());
        ClearToRead();
    }
}

////////////////////////////////////////////////////////////////////////////////////
void Player::NoftifyHttpConnectionIsAvailable()
{
    mConnectionState = HTTP_CONNECTION_SENDING;
    ClearToWrite();
}

////////////////////////////////////////////////////////////////////////////////////
int Player::ParseHttpBody(const char* mParserBegin, const char* mParserEnd)
{
    int uAction = mAction;
    switch (uAction)
    {
        case ACTION_NONE:
            return mParserEnd - mParserBegin;
        case ACTION_GET_MANIFEST:
            return ParseManifest(mParserBegin, mParserEnd);
        case ACTION_GET_PLAYLIST:
            return ParseManifest(mParserBegin, mParserEnd);
        case ACTION_GET_FRAGMENT:
            return ParseFragment(mParserBegin, mParserEnd);
    }
    return 0;
}


////////////////////////////////////////////////////////////////////////////////////
void Player::NotifyReplyRecieved()
{
    ReleaseHttpConnection();
}

////////////////////////////////////////////////////////////////////////////////////
void Player::NotifyHttpConnectionReleased()
{
    int Id = mAction;

    switch (Id)
    {
        case ACTION_NONE:
        break;

        case ACTION_GET_MANIFEST:
            NotifyManifestRecieved();
        break;
        case ACTION_GET_PLAYLIST:
            NotifyPlaylistRecieved();
        break;
        case ACTION_GET_FRAGMENT:
            NotifyFragmentRecieved();
        break;
    }
}



