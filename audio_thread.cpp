#include "ddaudio.h"
#include <cstdio>
#include <iostream>
#include <string>
using namespace std;
/********************************************************
Function:	 audio_callback
Description: 音频解码回调函数
Input:	*userdata
OutPut:
Return:
Others:
 *  This function is called when the audio device needs more data.
 *
 *  \param userdata An application-specific parameter saved in
 *                  the SDL_AudioSpec structure
 *  \param stream A pointer to the audio data buffer.
 *  \param len    The length of that buffer in bytes.
 *
 *  Once the callback returns, the buffer will no longer be valid.
 *  Stereo samples are stored in a LRLRLR ordering.
 *
 *  You can choose to avoid callbacks and use SDL_QueueAudio() instead, if
 *  you like. Just open your audio device with a NULL callback.
Author: ZhiYu Liu
Date:	2019-06-14
*********************************************************/


static void audio_callback(void *userdata, Uint8 *stream, int len)
{
 #if Decipher_DEBUG
   cout << <<__FUNCTION__"audio_callback len:"<<len<<endl;
#endif
   ddwork_s *d_pstMusicsState = (ddwork_s *) userdata;
    SDL_memset(stream,0,len);
    while(d_pstMusicsState->isPause == true)
    {
        SDL_Delay(10);
        continue;
    }
    if(d_pstMusicsState->audio_pkt_size == 0 || d_pstMusicsState->quit ==1)
    {
 #if Decipher_DEBUG
      cout <<__FUNCTION__<< "audio_callback return "<<endl;
#endif
      return ;
    }
    len = (len>(int)d_pstMusicsState->audio_pkt_size? len:(int)d_pstMusicsState->audio_pkt_size);   //尽可能为最大音频量
//SDL_MIX_MAXVOLUME
    SDL_MixAudio(stream,d_pstMusicsState->audio_pkt_pos,len,SDL_MIX_MAXVOLUME); //这里的音量设置为函数要求，不影响硬件音量

    d_pstMusicsState->audio_pkt_pos += len; //音频播放位置
    d_pstMusicsState->audio_pkt_size -= len; //剩余音频长度

}
/********************************************************
Function: audio_component_init
Description: Audio related settings
Input:	d_pstMusicsState，stream_index
Author: ZhiYu Liu
Date:   2021-07-18
*********************************************************/
int music_thread(void *arg)
{
    ddwork_s *d_pstMusicsState= (ddwork_s *) arg;

    int stream_index = d_pstMusicsState->s32AStreamSubscript;

    /** Define an AVFormatContext pointer to point to media information for obtaining audio streams **/
       AVFormatContext *pFormatContext = d_pstMusicsState->pFormatContext;
       AVCodecContext  *codecCtx       = d_pstMusicsState->p_pstACodecCtx;

    SDL_AudioSpec spec;         /** Output actual audio information **/
    SDL_AudioSpec wanted_spec;  /** Input  actual audio information **/
    SDL_memset(&spec, 0, sizeof(spec));
    SDL_memset(&wanted_spec, 0, sizeof(wanted_spec));


   if(pFormatContext == nullptr || codecCtx ==nullptr)
   {
     cout <<__FUNCTION__<< "[audio_component_init] failed,d_pstMusicsState error"<<endl;
     return -1;
   }
   else
   {
 //    cout <<__FUNCTION__<< "[audio_component_init] start" <<endl;
    }

   //根据声道数返回默认输入声道格式
   int64_t in_channel_layout = av_get_default_channel_layout(codecCtx->channels);

   uint64_t out_channel_layout  = AV_CH_LAYOUT_STEREO;  //声道格式
   AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;   //采样格式
   int out_nb_samples=codecCtx->frame_size;	 //nb_samples: AAC-1024 MP3-1152  格式大小 /*有的是视频格式数据头为非标准格式，从frame_size中得不到正确的数据大小，只能解码一帧数据后才可以获得*/
   int out_sample_rate = 44100;//采样率	pCodecCtx->sample_rate
   int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);	 //根据声道格式返回声道个数

   int out_buffer_size = av_samples_get_buffer_size(NULL,out_channels,out_nb_samples,out_sample_fmt,1);//获取输出缓冲大小

   uint8_t         *out_buffer;//解析数据
   out_buffer = (uint8_t*)av_malloc(MAX_AUDIO_FRAME_SIZE);
   memset(out_buffer,0,MAX_AUDIO_FRAME_SIZE);

   wanted_spec.freq = out_sample_rate;	//采样率
   wanted_spec.format = AUDIO_S16SYS;	//告诉SDL我们将要给的格式
   wanted_spec.channels = out_channels;	 //声音的通道数
   wanted_spec.silence = 0; 				 //用来表示静音的值
   wanted_spec.samples = out_nb_samples;   //格式大小

   wanted_spec.callback = audio_callback;
   wanted_spec.userdata = d_pstMusicsState;     //Callback parameter
#if Decipher_DEBUG
    cout <<__FUNCTION__<< "[audio_component_init]Turn on the specific audio device and start the callback function to decode" << endl;
#endif
    if (SDL_OpenAudio(&wanted_spec, NULL)<0)
    {
          printf("can't open audio.\n");
          return -1;
    }

      d_pstMusicsState->audio_stream = pFormatContext->streams[stream_index];
      d_pstMusicsState->audio_buf_size = 0;
      d_pstMusicsState->audio_buf_index = 0;

    //音频格式转换准备
      d_pstMusicsState->swr_ctx = swr_alloc();//等同于au_convert_ctx  = NULL;
   //参数设置：输出格式PCM -- 输入格式	MP3
      d_pstMusicsState->swr_ctx = swr_alloc_set_opts(d_pstMusicsState->swr_ctx,out_channel_layout, out_sample_fmt, out_sample_rate,
                                                  in_channel_layout,codecCtx->sample_fmt ,codecCtx->sample_rate,0, NULL);
      if(swr_is_initialized(d_pstMusicsState->swr_ctx)==0)
      {
          swr_init(d_pstMusicsState->swr_ctx);//初始化
      }
      if(d_pstMusicsState->audio_buf == nullptr)
      {
        d_pstMusicsState->audio_buf = (uint8_t*)av_malloc(MAX_AUDIO_FRAME_SIZE);
      }
      memset(d_pstMusicsState->audio_buf,0,MAX_AUDIO_FRAME_SIZE);
      memset(&d_pstMusicsState->audio_pkt, 0, sizeof(d_pstMusicsState->audio_pkt));
      AVPacket *packet =  &d_pstMusicsState->audio_pkt;
#if Decipher_DEBUG
      cout <<__FUNCTION__<< "[music_thread]Audio pre-work is ready" << endl;
#endif
      //解析数据包
while(true)
{
   if(d_pstMusicsState->audioq.size>0 || d_pstMusicsState->quit == true)
   {
       av_packet_unref(packet);

       while(d_pstMusicsState->isPause == true)
       {
           SDL_Delay(2000);
       }

       if(d_pstMusicsState->quit == true)
       {
           cout << "[music_thread]d_pstMusicsState->quit " << endl;
           if(d_pstMusicsState->audio_buf!=nullptr)
           av_freep(&d_pstMusicsState->audio_buf);
           av_frame_unref(d_pstMusicsState->audio_frame);

           av_frame_free(&d_pstMusicsState->audio_frame);

           if(d_pstMusicsState->audio_frame== nullptr)
           {
  #if Decipher_DEBUG
               cout <<__FUNCTION__<< "[music_thread]av_frame_free successfully" << endl;
  #endif
           }
           else
           {
            cout << "[music_thread]av_frame_free failed" << endl;
           }
           break;
       }

       /** 从音频队列中获取音频数据 **/
       if (packet_queue_get(&d_pstMusicsState->audioq, packet, 0) <= 0)
       {
           qDebug()<<__FUNCTION__<<"[audio_decode_frame]packet_queue_get failed"<<endl;
           av_packet_unref(packet);
           return -1;
       }
       else
       {
    //       qDebug()<<__FUNCTION__<<"[audio_decode_frame]get audio packet data size:"<<packet->size<<endl;;
       }

       /**收到这个数据 说明刚刚执行过跳转 现在需要把解码器的数据 清除一下**/
       if(strcmp((char*)packet->data,FLUSH_DATA) == 0)
       {
 #if Decipher_DEBUG
           qDebug()<<__FUNCTION__<<"[audio_decode_frame]FLUSH_DATA audio data"<<endl;
 #endif
           avcodec_flush_buffers(codecCtx);
           av_packet_unref(packet);
           continue;
       }
       if (!d_pstMusicsState->audio_frame)
       {
           if (!(d_pstMusicsState->audio_frame = av_frame_alloc())) //分配内存,-020
            {
                qDebug()<<__FUNCTION__<<"[audio_decode_frame]av_frame_alloc failed"<<endl;
                return AVERROR(ENOMEM);
            }
           else
            {
 //             qDebug()<<__FUNCTION__<<"[audio_decode_frame]first audio_frame_frame_alloc"<<endl;//只在第一次使用时分配内存,之后重复使用
            }
       }
       else
       {
         /**Unreference all the buffers referenced by frame and reset the frame fields.**/
          av_frame_unref(d_pstMusicsState->audio_frame);
       }
       ////解码一帧音频压缩数据，得到音频像素数据
       if ( avcodec_send_packet(codecCtx, packet) != 0)
       {
          qDebug()<<__FUNCTION__<<"[audio_decode_frame] avcodec_send_packet failed"<<endl;
       }
       else
       {
 //      qDebug()<<__FUNCTION__<<"[audio_decode_frame] avcodec_send_packet successfully"<<endl;
       }

       while(avcodec_receive_frame(codecCtx, d_pstMusicsState->audio_frame) >= 0)
       {
 //           qDebug()<<"[audio_decode_frame] avcodec_receive_frame successfully"<<endl;
//d_pstMusicsState->audio_frame->format -> float 4bit
  //         printf("diff-1 sample_rate(%d-%d)\n",d_pstMusicsState->audio_frame->nb_samples,out_sample_rate);
 //          printf("diff-1 sample_fmt(%d-%d)\n",d_pstMusicsState->audio_frame->format,out_sample_fmt);
           //数据格式转换
           swr_convert(d_pstMusicsState->swr_ctx,&d_pstMusicsState->audio_buf,MAX_AUDIO_FRAME_SIZE,(const uint8_t**)d_pstMusicsState->audio_frame->data,d_pstMusicsState->audio_frame->nb_samples);

  //         printf("diff-2 sample_rate(%d-%d)\n",d_pstMusicsState->audio_frame->nb_samples,out_sample_rate);
    //       printf("diff-2 sample_fmt(%d-%d)\n",d_pstMusicsState->audio_frame->format,out_sample_fmt);


           d_pstMusicsState->audio_buf_index++;
       }
       //用av_p2d()把时基(time_base)转换成double类型，再乘以当前packet.pts即可得到当前packet的播放时间(audio_clock)。
       if (packet->pts != AV_NOPTS_VALUE)
        d_pstMusicsState->audio_clock = av_q2d(d_pstMusicsState->audio_stream->time_base) * packet->pts;
       //输出一帧包大小

   //    printf("index:%5d\t pts:%lld\t packet size:%d,time %f\n",d_pstMusicsState->audio_buf_index,packet->pts,packet->size,d_pstMusicsState->audio_clock);

  // qDebug() << QString("[music-info]index:%1,pts:%2 ,packet size:%3,time %4").arg(d_pstMusicsState->audio_buf_index).arg(packet->pts).arg(packet->size).arg(d_pstMusicsState->audio_clock)<< endl;
     //    qDebug()<<__FUNCTION__<<"[audio_decode_frame] wait audio callback"<<endl;
           while(d_pstMusicsState->audio_pkt_size>0)
             SDL_Delay(2);//延时1ms

           if (d_pstMusicsState->seek_flag_audio)
           {
               /**发生了跳转 则跳过关键帧到目的时间的这几帧**/
        //      if(d_pstMusicsState->audio_clock < d_pstMusicsState->seek_time)
      //            continue;
     //         else
                  d_pstMusicsState->seek_flag_audio = 0;
           }

          //指向音频数据 (PCM data)
          d_pstMusicsState->audio_chunk = (Uint8 *) d_pstMusicsState->audio_buf;
          //out_buffer_size
          //音频长度
          d_pstMusicsState->audio_pkt_size =out_buffer_size;
          //当前播放位置
          d_pstMusicsState->audio_pkt_pos = d_pstMusicsState->audio_chunk;
          //开始播放
          SDL_PauseAudio(0);

   }
   else if(d_pstMusicsState->readFinished == true)
   {
 #if Decipher_DEBUG
      cout <<__FUNCTION__<< "[music_thread] play end" <<endl;
 #endif
      break;
   }
}
   cout << "[music_thread] quit" <<endl;
   d_pstMusicsState->readThreadFinished = true ;
   d_pstMusicsState->quit = 1;
   swr_free(&d_pstMusicsState->swr_ctx);
    return 0;

}
