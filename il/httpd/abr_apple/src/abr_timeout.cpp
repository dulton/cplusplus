#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <set>
#include <string>
#include <string.h>

using namespace std;


#include "abr_player.h"



////////////////////////////////////////////////////////////////////////////////////
void Player::NotifyTimeoutExpired(int id)
{
    switch (id)
    {
        case TIMEOUT_CONNECT:
            //cout<<"timeout ["<< mID<< "]"<< " mConnectionState " << mConnectionState <<endl;
            if (mConnectionState == HTTP_CONNECTION_OPENING)
            {
                if (IsConnected())
                {
//                    cout<<"+"<<endl;
                    TryToConnect();
                    //NotifyConnected();
                }
                else
                {
//                    cout<<"-"<<endl;
                    TryToConnect();
                }
            }
        break;
        case TIMEOUT_TRASACTION:
            NotifyConnectionBroken();
        break;
        case TIMEOUT_PLAYER:
            if (mAction == ACTION_TIMEOUT) Continue();
        break;
        case 0xFFFF:
            StartTimeout(0xFFFF, 1000);
            printf("."); fflush(stdout);
            break;
    }
}
