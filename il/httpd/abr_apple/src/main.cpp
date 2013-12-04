#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <set>
#include <string>
#include <vector>

#include <ace/Reactor.h>


using namespace std;

#include "my_player.h"

struct ControlBlock:GenericAbrObserver
{
    vector<GenericAbrPlayer*> mPlayer;

    int mPlayersCount;


    ControlBlock()
    {
        mPlayersCount = 0;
    };

    void Run()
    {
        mPlayer.reserve(16);

        for(int uID= 0; uID<1;uID++)
        {
            mPlayer[uID] = CreateAppleAbrPlayer(ACE_Reactor::instance(), 0);
            mPlayer[uID]->ReportToWithId(this, uID);
//                virtual void SetHttpVersion(int aParam)=0;
//                virtual void SetKeepAliveFlag(bool aParam)=0;
//                virtual void SetUserAgentString(string aParam)=0;
//                virtual void SetMaximumConnectionAttempts(int aParam)=0;
//                virtual void SetConnectionTimeout(int aParam)=0;
//                virtual void SetMaximumTransactionAttempts(int aParam)=0;
//                virtual void SetTransactionTimeout(int aParam)=0;
//                virtual void SetShiftAlgorithm(int aShiftAlgorithm, int aParam1, int aParam2)=0;
//                virtual void SetPlaybackBufferMaxSize(int aParam)=0;
//                virtual void SetPlaybackBufferMinSize(int aParam)=0;
//                virtual void SetProgramID(int aParam)=0;

//                virtual void SetBitrateMax(int aParam)=0;
//                virtual void SetBitrateMin(int aParam)=0;
            //mPlayer[uID]->BindToIfName(srcIfName);//SetIpv4Tos...
            mPlayer[uID]->SetPlayDuration(30*1000);
            mPlayer[uID]->Play("http://10.44.12.240:80/streamingvideo/stream_multi.m3u8");
            mPlayersCount += 1;
        }

        while (mPlayersCount)
        {
            ACE_Reactor::instance()->handle_events();
        }
        cout<<"All Done\n";
    }

    void NotifyStatusChanged(int uID, int uStatus)
    {
        StatusInfo Info;
        mPlayer[uID]->GetStatusInfo(&Info);

        cout<<"Done With "<<uID<<endl;

        delete mPlayer[uID];
        mPlayersCount -=1;
    }

};

int main(int argc, char* argv[])
{
    ControlBlock mControlBlock;
    mControlBlock.Run();
    return 0;
}
