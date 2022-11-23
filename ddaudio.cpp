#include "ddaudio.h"
#include <cstdio>
#include <iostream>
#include <string>
using namespace std;
ddaudio::ddaudio(QObject *parent) : QObject(parent)
{
    qDebug()<<"myffmpeg build"<<endl;
    avformat_network_init();
    PlayState = No ;
    /**SDL初始化音频子系统**/
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER))
    {
        fprintf(stderr,"Could not initialize SDL - %s. \n", SDL_GetError());
        exit(1);
    }

  //   connect(ui->horizontalSlider_time,SIGNAL(sliderReleased()),this,SLOT(slotSliderReleased()));
    db_init();
    mlog->db_queryAll(1);
}
ddaudio::~ddaudio()
{
  SDL_Quit();
}
int ddaudio::work_main(const char* filename,int flag)
{
   qDebug()<<QString("[work_main]use to %1 mode to play music").arg(flag)<<endl;
   ddwork_s *ddstatus=&dworker;
   QString errstring;
   SDL_memset(ddstatus,0,sizeof(ddwork_s));
  // AVFormatContext->AVStream->AVCodecContext->AVCodec
   AVFormatContext  *l_pstFormatCtx = nullptr; //
   AVStream         *l_pstAStream   = nullptr; // audio stream info
   AVCodecContext   *l_pstACodecCtx = nullptr; // audio stream detailed context
   AVCodec          *l_pstACodec    = nullptr;
   AVPacket         *packet         = nullptr;
   int ret = -1;
 //int l_s32AudioPacketNum   = 0;
   int l_s32AStreamSubscript = -1;

  /** (Help debugging) Set Av log level, and output function **/
   av_log_set_level(AV_LOG_DEBUG);
 //  av_log_set_callback(FFmpegLogFunc);
   av_log(NULL,AV_LOG_INFO, "av_log src_filename:%s\n", filename);

// 1.Open the input file in the unpacked format
   l_pstFormatCtx = avformat_alloc_context();
   char startfile[256] = {0};
   SDL_strlcpy((char *)startfile,filename,sizeof(startfile));
   while(avformat_open_input(&l_pstFormatCtx,startfile,nullptr,nullptr)!=0)
   {
      setErrorMsg(errstring.sprintf("[music_work]Could not open source file,exit work%s\n", filename));
      PlayState = Stop;
      ++audio_errorflag;
      if(audio_errorflag == 1 )
      {
        emit sig_audioevent(Startfailed);
      }
      while(true)
      {
          if(PlayState == Start)
          {
              SDL_strlcpy(startfile,musicfile.toUtf8().data(),sizeof(startfile));//将文件名写入数组
              break;
          }
          av_usleep(70000);
      }
   }

//2.Obtain media file stream and bit rate information
   if (avformat_find_stream_info(l_pstFormatCtx, nullptr) < 0)
   {
      setErrorMsg(errstring.sprintf("[music_work]Could't find stream infomation !!\n"));
      goto free_format;
   }
 #if Decipher_DEBUG
  qDebug() <<__FUNCTION__<< "----------------- File Information --------------" << endl;
  av_dump_format(l_pstFormatCtx, 0, filename, 0);
  qDebug() <<__FUNCTION__<< "-------------------------------------------------" << endl;
#endif
  /* 3.select the audio stream */
   ret = av_find_best_stream(l_pstFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &l_pstACodec, 0);
   if (ret < 0)
   {
      setErrorMsg(errstring.sprintf("[work_error]Didn't find a audio stream\n"));
      goto free_format;
   }
   l_s32AStreamSubscript = ret;


    l_pstAStream= l_pstFormatCtx->streams[l_s32AStreamSubscript];
    l_pstACodec =  avcodec_find_decoder(l_pstAStream->codecpar->codec_id);

   if(l_pstACodec == nullptr)// According to the decoder context, get the decoder ID, get it
   {
     setErrorMsg(errstring.sprintf("[work_error]Cannot find the corresponding decoder or the file is encrypted\n"));
     goto free_format;
   }

  /** Send the length of the media playback event to update the display progress bar **/
      emit sig_TotalTimeChanged(getTotalTime(l_pstFormatCtx));
      qDebug() <<__FUNCTION__<< QString("[music-info]Update display progress bar:%1s").arg(getTotalTime(l_pstFormatCtx)/1000000)<< endl;
      /** Print audio encoding type **/
      qDebug() <<__FUNCTION__<<"[music-info]AUDIO type:" << avcodec_get_name(l_pstAStream->codecpar->codec_id) << endl;

      /* create decoding context */
      l_pstACodecCtx = avcodec_alloc_context3(l_pstACodec); //Allocation of AVCodecContext memory
      if(l_pstACodecCtx == nullptr || avcodec_parameters_to_context(l_pstACodecCtx, (const AVCodecParameters *)l_pstAStream->codecpar)<0)
      {
        setErrorMsg(errstring.sprintf("[work_error]failed to avcodec_parameters_to_context"));
      }
      /* init the audio decoder */
      if (avcodec_open2(l_pstACodecCtx, l_pstACodec, nullptr) < 0 || l_pstACodec == nullptr)
      {
        setErrorMsg(errstring.sprintf("[work_error]Cannot find the corresponding decoder or the file is encrypted"));
        goto free_AVCodecCtx;
      }

      /** Save relevant information into a custom structure **/
          ddstatus->pFormatContext      =  l_pstFormatCtx; //Pass to the audio thread
          ddstatus->p_pstACodecCtx      =  l_pstACodecCtx; //Pass to the audio thread
          ddstatus->audio_stream        =  l_pstAStream;
          ddstatus->s32AStreamSubscript =  l_s32AStreamSubscript;

          musicinfo.m_name= (char *)startfile;
          musicinfo.m_starttime= QDateTime::currentDateTime();
          qDebug()<<QString("Start playing music at ").arg(musicinfo.m_name)<<musicinfo.m_starttime<<endl;
          /** 创建一个SDL线程专门用来解码视频 **/
          ddstatus->music_tid = SDL_CreateThread(music_thread, "music_thread", (void *)ddstatus);

          if (NULL == ddstatus->music_tid)
           {
              printf("Create SDL Thread failed: %s\n", SDL_GetError());
              goto free_AVCodecCtx;
           }
          else
            SDL_DetachThread(ddstatus->music_tid);  /* will go away on its own upon completion. */

 //   mVideoState->player = this;
   /**分配一个AVPacket结构体大小的内存的packet 用来存放读取的视频**/
   packet = (AVPacket *)av_mallocz(sizeof(AVPacket));  //分配内存处 -003
   if(!packet)
   {
      qDebug() <<__FUNCTION__<< "av_packet_alloc error!" << endl;
      goto free_AVCodecCtx;
   }
   if(ddstatus->audioq.cond==nullptr)
   packet_queue_init(&ddstatus->audioq);

   if(PlayState!=Start) PlayState = Start;
   while (true)
   {
       /**停止播放了**/
       if (ddstatus->quit)
           break;

       if (ddstatus->seek_req)
       {
          int64_t seek_target = ddstatus->seek_pos;
   //       qDebug()<< QString("seek time_1:%1").arg(seek_target)<<endl;
     //    AVRational aVRational = {1, AV_TIME_BASE};

         if (ddstatus->s32AStreamSubscript >= 0)
         {
             /**重新调整时间位置**/
             /**Rescale a 64-bit integer by 2 rational numbers.**/
             seek_target = av_rescale_q(seek_target, AV_TIME_BASE_Q,l_pstFormatCtx->streams[ddstatus->s32AStreamSubscript]->time_base);
         }
 #if Decipher_DEBUG
         qDebug()<<__FUNCTION__<< QString("seek time_2:%1").arg(seek_target)<<endl;
#endif
         /**跳转到需要偏移时间位置附近的音频关键帧位置**/
         /**Seek to the keyframe at timestamp.**/
         if (av_seek_frame(l_pstFormatCtx, ddstatus->s32AStreamSubscript, seek_target, AVSEEK_FLAG_BACKWARD) < 0)
             fprintf(stderr, "%s: error while seeking\n",ddstatus->pFormatContext->url);
         else
         {
             if (ddstatus->s32AStreamCount >= 0)
             {
                 /**分配一个packet**/
                 AVPacket *packet = (AVPacket *) malloc(sizeof(AVPacket));
                 av_new_packet(packet, 10);
                 SDL_memcpy((char*)packet->data,FLUSH_DATA,sizeof(FLUSH_DATA));
                 /**清除队列**/
                 packet_queue_flush(&ddstatus->audioq);
                 /**往队列中存入用来清除的包**/
                 packet_queue_put(&ddstatus->audioq, packet);
 #if Decipher_DEBUG
                 qDebug()<<__FUNCTION__<< QString("put audio packet again")<<endl;
 #endif
             }
         }

         ddstatus->seek_req = 0 ;
         ddstatus->seek_time = ddstatus->seek_pos / AV_TIME_BASE;
         ddstatus->seek_flag_audio = 1;
#if Decipher_DEBUG
         qDebug()<<__FUNCTION__<<QString("seek %1 end").arg(ddstatus->seek_time)<<endl;
#endif
         emit sig_audioevent(Seekend);
         if(PlayState == Busy_seek)
             PlayState = Start;
       }
       /**这里做了个限制  当队列里面的数据超过某个大小的时候 就暂停读取  防止一下子读太多，导致的空间分配不足**/
       /**这里audioq.size是指队列中的所有数据包带的音频数据的总量或者视频数据总量，并不是包的数量**/
       if (ddstatus->audioq.size > MAX_AUDIO_SIZE)
       {
           SDL_Delay(5000);
    //       qDebug()<<"wait to read" <<endl;
           continue;
       }
       else if (ddstatus->isPause == true)
       {
           SDL_Delay(10);
           continue;
       }

       if (av_read_frame(l_pstFormatCtx, packet) < 0)
       { //read finish
 #if Decipher_DEBUG
          qDebug()<<__FUNCTION__<<"[work_main]av_read_frame failed "<<endl;
 #endif
          ddstatus->readFinished = true;
          if (ddstatus->quit)
               break; /**解码线程也执行完了 可以退出了**/
 #if Decipher_DEBUG
          printf("[work_main]The decoding thread has completed the task \n");
 #endif
          SDL_Delay(10);
           continue;
       }

       if( packet->stream_index == l_s32AStreamSubscript )
       {
           /**将音频数据存入音频队列中**/
           packet_queue_put(&ddstatus->audioq, packet);
#if Decipher_DEBUG
           printf("audio input num %d \n",l_s32AudioPacketNum++);
#endif
           av_packet_unref(packet);
       }
   }

   /**文件读取结束 跳出循环的情况**/
   /**等待播放完毕**/
   while (!ddstatus->quit)
       SDL_Delay(1000);
   /**清除队列**/
   if(ddstatus->audioq.size>0)
   packet_queue_flush(&ddstatus->audioq);
 /**** end ****/
 #if Decipher_DEBUG
      qDebug()<<__FUNCTION__<<QString("End of player code")<<endl;
 #endif
      stop();
      musicinfo.m_endtime= QDateTime::currentDateTime();
      qDebug()<<__FUNCTION__<<QString("End playing music at ").arg(musicinfo.m_name)<<musicinfo.m_endtime<<endl;
      mlog->db_add(musicinfo);//将记录存进数据库
      musicinfos.append(musicinfo);//同时备份一份本地记录存进vector容器
      emit sig_audioevent(Playend);

free_AVCodecCtx:
//Close the AVCodecContext structure and release the memory
      avcodec_close(l_pstACodecCtx);

      if(l_pstACodecCtx!=nullptr)
      {
       avcodec_free_context(&l_pstACodecCtx);   // Free memory -002-video
       qDebug() <<__FUNCTION__<<"l_pst VCodecCtx Released" <<endl;
      }

free_format:
  avformat_close_input(&l_pstFormatCtx);
  PlayState = No;
  return 0 ;
}
