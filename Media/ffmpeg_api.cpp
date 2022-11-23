
#include <QDebug>
#include <iostream>
#include <QString>
#include <sstream>
#include <cstdio>
#include <cstdarg>
#include <env.h>
using namespace std;

int get_streamsscript(AVFormatContext  *l_pstFormatCtx,enum AVMediaType codec_type)
{
   int StreamSubscript = -1;
   if(l_pstFormatCtx!=nullptr)
   {
     for (unsigned int i = 0; i < l_pstFormatCtx->nb_streams; ++i)
     {
       if (l_pstFormatCtx->streams[i]->codecpar->codec_type == codec_type)
         StreamSubscript = i;//Audio stream sequence number
     }
   }
   return StreamSubscript;
}

AVFormatContext * Obtain_Format(const char * filename)
{
    AVFormatContext *l_pstFormatCtx = NULL;
    /**
     * Allocate the structure AVFormatContext space, you must use avformat_close_input() to close it after use
     * 001:
     * It is the encapsulation format context, a structure that governs the overall situation,
     * and saves the relevant information of the video file encapsulation format
    * ***************************************************************
     * AVFormatContext *avformat_alloc_context(void);
     * void avformat_close_input(AVFormatContext **s);
     * ******************************************************
     **/
    l_pstFormatCtx = avformat_alloc_context(); //Allocation memory token -001
    /**
     * (Decapsulation)
     * 002:
     * Open the input file in the unpacked format and fill in the AVFormatContext structure data.
     * ***************************************************************
     * int avformat_open_input(AVFormatContext **ps, const char *url, AVInputFormat *fmt, AVDictionary **options);
     **/
    if( avformat_open_input(&l_pstFormatCtx,filename,NULL,NULL) < 0)
    {
        fprintf(stderr, "[%s]Could not open source file (%s)\n",__FUNCTION__, filename);
        return NULL;
    }
    else
    {
        fprintf(stdout, "[%s]Open the media file :(%s) successfully\n",__FUNCTION__,  filename);
    }

    /**
     * Obtain media file stream and bit rate information according to the opened file structure
     * int avformat_find_stream_info(AVFormatContext *ic, AVDictionary **options);
     **/
     if (avformat_find_stream_info(l_pstFormatCtx, NULL) < 0)
     {
        fprintf(stdout, "[%s]Could't find stream infomation\n",__FUNCTION__);
        avformat_close_input(&l_pstFormatCtx);
        return NULL;
     }
     /*****  After obtaining the media information, print the debugging media file information   *****/
#if Decipher_DEBUG
   // av_dump_format(l_pstFormatCtx, 0, filename, 0);
#endif
     return l_pstFormatCtx;
}

