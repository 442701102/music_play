#include "ddaudio.h"
#include <cstdio>
#include <iostream>
#include <string>
using namespace std;
bool ddaudio::play(QString filename)
{
    if(PlayState == Pause)
    {
       PlayState = Start ;
       dworker.isPause = false;
    }
    else if(PlayState == Stop && filename!=nullptr)
    {
       PlayState = Start ;
       musicfile = filename;
    }

    return true;
}

bool ddaudio::pause()
{
    if(PlayState == Start)
    {
       PlayState = Pause ;
       dworker.isPause = true;
    }
    else
    {
        qDebug()<<"[ddaudio]PlayState:"<<PlayState <<endl;
    }

    return true;
}
/********************************************************
Function:    seek
Description: 发生了拖动事件
Input:  pos
OutPut: none
Return:NULL
Others:
Date:   2020-01-22
*********************************************************/
void ddaudio::seek(int64_t pos)
{
 #if Decipher_DEBUG
    qDebug() <<__FUNCTION__<< "seek to" << pos << endl;
#endif
    if(PlayState == No )
        return;

    if(PlayState == Start)
    PlayState = Busy_seek;
 //   if(!dworker.seek_req)
    {
        dworker.seek_pos = pos;
        dworker.seek_req = 1;
    }
}
/********************************************************
Function:	 stop
Description: 关闭视频播放
Input:	isWait
OutPut: none
Return:
Others:
Author:  Zhiyu Liu
Date:	2021-07-11
*********************************************************/
bool ddaudio::stop(bool isWait)
{
    qDebug() <<"[ddaudio]stop music." << endl;
    if (PlayState == Stop || PlayState == No || PlayState == Busy_exit)
    {
        return false;
    }
    dworker.quit = 1;
    dworker.audio_pkt_size = 0;
    dworker.audio_clock = 0 ;
    if(PlayState == Pause)
    {
      qDebug() <<__FUNCTION__<< "Out of pause" <<endl;
      dworker.isPause = false;
   //   SDL_Delay(2000);
    }
    PlayState = Busy_exit;
    if(isWait)
    {
 #if Decipher_DEBUG
        qDebug() << "wait to readThreadFinished" <<endl;
#endif
        while(!dworker.readThreadFinished )
           SDL_Delay(10);
    }
    qDebug() <<__FUNCTION__<< "readThreadFinished successfully" <<endl;
    dworker.readThreadFinished = false ;
      SDL_LockAudio();
      SDL_PauseAudio(1);
      SDL_UnlockAudio();
      SDL_CloseAudio();
      emit sig_TotalTimeChanged(0);
      return true;
}
