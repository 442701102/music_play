#ifndef DDAUDIO_H
#define DDAUDIO_H

#include <QObject>
#include <QThread>
#include <env.h>
#include "qsqlite.h"
#include "Media/AVPacketManager.h"
#define FLUSH_DATA       "FLUSH"
int music_thread(void *arg);
typedef struct ddwork_t
{
   PacketQueue audioq;  //AVPacket队列
//public
    int s32AStreamSubscript;            //音频流序号
    double audio_clock;             /**音频时 **/
    bool isPause;                   /**暂停标志**/
    bool quit;                      /**停止**/
    bool readFinished;              /**文件读取完毕**/
    bool readThreadFinished;
    int seek_req;                   /**跳转请求**/
    int seek_flag_audio;
    int64_t seek_pos;
    int64_t seek_time;

 //   DECLARE_ALIGNED(16,uint8_t,audio_buf2) [AVCODEC_MAX_AUDIO_FRAME_SIZE * 4];
    Uint8* audio_chunk;
    uint8_t *audio_buf;
    uint8_t *audio_pkt_pos;


    Uint32 audio_pkt_size;
    int s32AStreamCount; /**音频流数量**/

    unsigned int audio_buf_size;
    unsigned int audio_buf_index;

    int64_t audio_src_channel_layout;
////ffmpeg
    AVFormatContext *pFormatContext;
    AVCodecContext * p_pstACodecCtx;    ////音频编解码结构体
    AVStream * audio_stream;            ////音频流结构体

    AVPacket audio_pkt;
    AVFrame *audio_frame; /** 解码音频过程中的使用缓存 **/
////SDL
    SDL_Thread *music_tid;       /**  解码视频SDL线程的id **/
    SDL_AudioDeviceID audioID;   /** SDL Audio Device IDs. **/
    struct SwrContext *swr_ctx; /** 用于解码后的音频格式转换 **/
}ddwork_s;
class ddaudio : public QObject
{
    Q_OBJECT
public:
    explicit ddaudio(QObject *parent = nullptr);
    ~ddaudio();
    qsqlite *mlog;
    int work_main(const char* filename,int flag);
    enum state_e
    {
        No=0,
        Start=1,
        Stop=2,
        Pause=3,
        Error=4,
        Busy_exit=5,
        Busy_seek
    };
    int status();
    bool play(QString filename=NULL); //开始播放
    bool pause();//暂停播放
    bool stop(bool isWait = true); //关闭播放，参数表示是否等待所有的线程执行完毕再返回
    void seek(int64_t pos); //单位是微秒
    double getCurrentTime();
    bool db_init();
    musiclog_s_t musicinfo;
    QVector<musiclog_s_t> musicinfos;
    void deallog();
private:
    int setErrorMsg(QString);
    qint64 getTotalTime(AVFormatContext  *l_pstFormatCtx);

    ddwork_s dworker;   //Customized main playback state structure
    state_e  PlayState; //播放状态
    qint32 audio_errorflag = 0;
    QString musicfile;
private slots:

signals:
    void sig_TotalTimeChanged(qint64 uSec); //获取到视频时长的时候激发此信号
    void sig_CurrentTimeChanged(qint64 uSec); //获取到视频时长的时候激发此信号
    void sig_audioevent(int); //获取到视频时长的时候激发此信号

};

#endif // DDAUDIO_H
