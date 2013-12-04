#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <set>
#include <string>
#include <string.h>

using namespace std;


#include "abr_player.h"


void Player::FatalError(string msg)
{
//    cout << msg;
    Stop();
    NotifyStatusChanged(ACTION_ABORT);
}

void Player::NoftifyHttpConnectionIsNotAvailable()
{
    //    cout<<"NoftifyHttpConnectionIsNotAvailable("<<mID<<")"<<endl;
    Stop();
    mAction  = ACTION_ABORT;
    NotifyStatusChanged(ACTION_ABORT);
}

void Player::NotifyConnectionBroken()
{
    //    cout << "********************************************************* NotifyConnectionBroken("<<mID<<")" << endl;
    Stop();
    mAction  = ACTION_ABORT;
    NotifyStatusChanged(ACTION_ABORT);
}

void Player::NotifyReadError()
{
    //    cout << "NotifyReadError("<<mID<<")" << endl;
    Stop();
    mAction  = ACTION_ABORT;
    NotifyStatusChanged(ACTION_ABORT);
}

void Player::NotifyWriteError()
{
    //    cout << "NotifyWriteError("<<mID<<")" << endl;
    Stop();
    mAction  = ACTION_ABORT;
    NotifyStatusChanged(ACTION_ABORT);
}
