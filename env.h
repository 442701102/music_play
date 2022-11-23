#ifndef ENV_H
#define ENV_H
#include <QThread>
#include <QString>
#include <QFile>
#include <QTimer>
#include <QFileDialog>
#include <QWidget>
#include <QMouseEvent>
#include <QPixmap>
#include <QList>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QMessageBox>
#define MAX_AUDIO_FRAME_SIZE 192000
#define SDL_AUDIO_BUFFER_SIZE 4096
#define MAX_AUDIO_SIZE (25 * 16 * 4096)
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 /**1 second of 48khz 32bit audio**/

#define DB_DEBUG 0
#define Decipher_DEBUG 0
typedef struct musiclog_s
{
   QString m_name;
   QDateTime m_starttime;
   QDateTime m_endtime;
}musiclog_s_t;

enum audioevent
{
  Startfailed = 0,
  Firstbuild,
  Seekend,
  Playend
};
#ifdef __cplusplus
extern "C"
{
#endif
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/time.h"
    #include "libavutil/pixfmt.h"
    #include "libswscale/swscale.h"
    #include "libswresample/swresample.h"
    #include "libavutil/frame.h"
    #include "libavutil/imgutils.h"

    #include "SDL.h"
    #include "SDL_stdinc.h"
    #include "SDL_audio.h"
    #include "SDL_types.h"
    #include "SDL_name.h"
    #include "SDL_main.h"
    #include "SDL_config.h"

    int get_streamsscript(AVFormatContext  *l_pstFormatCtx,enum AVMediaType codec_type);
    AVFormatContext * Obtain_Format(const char * filename);

#ifdef __cplusplus
}
#endif
#endif // ENV_H
