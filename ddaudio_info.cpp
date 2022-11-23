#include "ddaudio.h"
#include <cstdio>
#include <iostream>
#include <string>
using namespace std;
/********************************************************
Function:    getTotalTime
Description: Get the total duration of the currently playing file
Input:  none
OutPut: none
Return:
Others:
Date:
*********************************************************/
qint64 ddaudio::getTotalTime(AVFormatContext  *l_pstFormatCtx)
{
    //时长（单位：微秒us，转换为秒需要除以1000000）
    if(l_pstFormatCtx!=NULL)
     return l_pstFormatCtx->duration;
    else
     return 0;
}
int ddaudio::status()
{
    return PlayState;
}
/********************************************************
Function:    getCurrentTime
Description: 获取当前的pts
Input:  none
OutPut: none
Return:
Others:
Date:   2020-01-22
*********************************************************/
double ddaudio::getCurrentTime()
{
    return dworker.audio_clock;
}
void ddaudio::deallog()
{
  QList<musiclog_s_t>  dblist=mlog->get_musicloglist();
  for(int i=0;i<dblist.size();++i)
  {
#if DB_DEBUG
 //     qDebug()<<__FUNCTION__<<dblist.at(i).m_name<<endl;
 //     qDebug()<<__FUNCTION__<<dblist.at(i).m_starttime<<endl;
 //     qDebug()<<__FUNCTION__<<dblist.at(i).m_endtime<<endl;
#endif
      musicinfos.append(dblist.at(i));
  }
    emit sig_audioevent(Firstbuild);
}
bool ddaudio::db_init()
{
    mlog = new qsqlite(nullptr,"QSQLITE","musiclog.db","ll4427","4427");
    mlog->db_init();

    return 0;
}
int ddaudio::setErrorMsg(QString msg)
{
  qDebug()<<QString("[music_work_error]%1").arg(msg)<<endl;
  return 0;
}
