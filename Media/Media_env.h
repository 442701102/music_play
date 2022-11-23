#ifndef FFMPEG_SDL_H
#define FFMPEG_SDL_H

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

   AVFormatContext * Obtain_Format(const char * filename);
   int get_streamsscript(AVFormatContext  *l_pstFormatCtx,enum AVMediaType codec_type);
#ifdef __cplusplus
}
#endif
int video_thread(void *arg);
enum Decodingmethod
{
    D_NULL=0,
    D_AUDIO=1,
    D_VEDIO=2,
    D_ALL=3,
};

#endif // FFMPEG_SDL_H
