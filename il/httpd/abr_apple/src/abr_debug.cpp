#include <stdlib.h>
#include <string>
#include <iomanip>
#include <iostream>
#include "abr_player.h"

//#define PRINT_REQUEST
//#define DUMP_MANIFEST
//#define PRINT_STATUS

void Player::PrintPlaylist()
{
#ifdef DUMP_MANIFEST
    cout << "<Variants>" << endl;
    for (unsigned int uVariantNo = 0; uVariantNo < mVariant.size(); uVariantNo++)
    {
        cout << "  <Variant=" << uVariantNo << ", ";
        cout << mVariant[uVariantNo].mCompressionRate << ", ";
        cout << mVariant[uVariantNo].mUrl.mSchemeName+mVariant[uVariantNo].mUrl.mDomainName + ":" + mVariant[uVariantNo].mUrl.mPortNumber;
        cout << mVariant[uVariantNo].mUrl.mRoot + mVariant[uVariantNo].mUrl.mPath + mVariant[uVariantNo].mUrl.mProgram;
        cout << ">" << endl;

        if (uVariantNo == mVariantCurrent)
        {
            for (unsigned int uFragmentNo = mFragmentCurrent; uFragmentNo < mFragment.size(); uFragmentNo++)
            {
                cout << "       <" << uFragmentNo << "> ";
                cout << "Duration =" << mFragment[uFragmentNo].mDuration;
                cout << ", Url =" << mFragment[uFragmentNo].mUrl;
                cout << " </" << uFragmentNo << ">" << endl;
            }
        }
        cout << "  </Variant>" << endl;

    }
    cout << "</Variants>" << endl<<endl;
#endif
}

void Player::PrintRequest()
{
#ifdef PRINT_REQUEST
    cout << "<Request "<< mID<<" >" << endl;
    cout << this->mRequest;
    cout << "</Request>" << endl<<endl;
#endif
}

void Player::PrintStatus()
{
#ifdef PRINT_STATUS
    cout << "[" << setfill(' ')<< setw(4)<< right  << mID << "," << setw(4) << right << mSessionDuration / 1000 << '.';
    cout << setfill('0') << setw(3) << right << mSessionDuration % 1000 << " ]  ";
    cout << setfill(' ');

    switch (mPlaybackState)
    {

        case BUFFERING:
            cout << "[ BUFFERING ";
            cout << setw(6) << right << mPlaybackBufferSize / 1000;
            cout <<  ",  "<<setw(9) << right <<mFragmentDownloadRate;
            cout << " ] ";
            break;

        case PLAYING:
            cout << "[   PLAYING ";
            cout << setw(6) << right << mPlaybackBufferSize / 1000;
            cout <<  ",  "<<setw(9) << right <<mFragmentDownloadRate;
            cout << " ] ";
            break;
    }

    switch (mAction)
    {
        case ACTION_NONE:
            cout << "State Machine Error" << endl;
            break;

        case ACTION_START:
            break;

        case ACTION_GET_MANIFEST:
            cout << "[Loading Variants  URL=";
            cout << mCfg.mUrl.mSchemeName;
            cout << mCfg.mUrl.mDomainName << ":" << mCfg.mUrl.mPortNumber;
            cout << mCfg.mUrl.mRoot << mCfg.mUrl.mPath << mCfg.mUrl.mProgram;
            cout << " ]";
            break;

        case ACTION_GET_PLAYLIST:
            cout << "[Loading Playlist URL = ";
            cout << mVariant[mVariantCurrent].mUrl.mSchemeName;
            cout << mVariant[mVariantCurrent].mUrl.mDomainName << ":" << mVariant[mVariantCurrent].mUrl.mPortNumber;
            cout << mVariant[mVariantCurrent].mUrl.mRoot << mVariant[mVariantCurrent].mUrl.mPath
                    << mVariant[mVariantCurrent].mUrl.mProgram;
            cout << " ]";
            break;

        case ACTION_GET_FRAGMENT:
            cout << "[Loading Fragment ";
            cout << mFragment[mFragmentCurrent].mDuration;
            cout << " s, ";
            cout << mVariant[mVariantCurrent].mCompressionRate << " rate, ";
            cout << "URL = ";
            cout << mFragment[mFragmentCurrent].mUrl;
            cout << " ]";
            break;

        case ACTION_TIMEOUT:
            cout << "[Nothing for the next  " << float(mPlayerTimeout) / 1000 << " seconds]";
            break;

        case ACTION_STOP:
            cout << "[ STOP ]" << endl;
            cout << endl;
            break;

        case ACTION_ABORT:

            break;

    }
    cout << endl;
#endif
}
