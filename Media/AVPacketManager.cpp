#include "AVPacketManager.h"
#include <QDebug>
#include <iostream>
#include <QString>
#include <sstream>
#include <cstdio>
#include <cstdarg>

using namespace std;
/********************************************************
 * file:AVPacketManager.cpp
 *
 * update: 2022/1/23
*********************************************************/


/********************************************************
Function:	 packet_queue_init
Description:  数据包队列初始化
Input:  PacketQueue
OutPut: PacketQueue
Return:success 0,otherwise error number.
Date:	2022-01-05
*********************************************************/
int packet_queue_init(PacketQueue *Packetq)
{
    SDL_memset(Packetq,0,sizeof(PacketQueue));
    /** Create a mutex, initialized unlocked. **/
    Packetq->mutex = SDL_CreateMutex();
    if (!Packetq->mutex)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    /** Create a condition variable. **/
    Packetq->cond = SDL_CreateCond();
    if (!Packetq->cond)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    Packetq->abort_request = 0;
    return 0;
}
/********************************************************
Function:	 packet_queue_put
Description: 数据包压入队列
Input:  PacketQueue,AVPacket
OutPut: PacketQueue
Return:success 0,otherwise -1
Others:
Date:   2022-01-05
*********************************************************/
int packet_queue_put(PacketQueue *Packetq, AVPacket *pkt)
{
    MyAVPacketList *pkt1 = NULL;

    if (Packetq->abort_request) return -1;

    pkt1 = (MyAVPacketList*)av_malloc(sizeof(MyAVPacketList));
    if (!pkt1)  return -1;

    SDL_LockMutex(Packetq->mutex);   ////Exclusive access

//If pkt->buffer is empty, pktl->pkt creates a new reference and copies the src field
    if (av_packet_ref(&pkt1->pkt, pkt) < 0)
    {
    //	printf("av_packet_ref fail\n");
        av_free(pkt1);
        return -1;
    }
    pkt1->next = NULL;

// The queue is empty, and the newly inserted element is the first element
// When the queue is not empty, insert the end of the queue
    if (Packetq->last_pkt == NULL)
        Packetq->first_pkt = pkt1;
    else
        Packetq->last_pkt->next = pkt1;

//Set the current enqueue packet as the end of the tea
//Total update queue size
    Packetq->last_pkt = pkt1;
    Packetq->nb_packets++;
    Packetq->size += pkt1->pkt.size + sizeof(*pkt1);

    Packetq->duration += pkt1->pkt.duration;
//After joining the queue, wake up the waiting thread
    SDL_CondSignal(Packetq->cond);
    SDL_UnlockMutex(Packetq->mutex);
    return 0;
}
/********************************************************
Function:	 packet_queue_get
Description: 数据包从队列取出
Input:  PacketQueue *AVPacket * ,Blocking sign
OutPut: AVPacket
Return:success 1,otherwise -1 or 0.
Others: 
Date:   2022-01-06
*********************************************************/
int packet_queue_get(PacketQueue *Packetq, AVPacket *pkt, int block)
{
    MyAVPacketList *pkt1=NULL;
    int ret = 0;
    SDL_LockMutex(Packetq->mutex);  //Exclusive access
    for(;;)
    {
       if (Packetq->abort_request)
       {
            ret = -1;
            break;
       }
        pkt1 = Packetq->first_pkt;// 队列头取包
        if (pkt1)
        {
            Packetq->first_pkt = pkt1->next;  
            if (Packetq->first_pkt==NULL)   //如果取得的是最后一个包的话，那么设置下last_pkt，否则它会错指向刚才已经取走的包
              Packetq->last_pkt = NULL;

            Packetq->nb_packets--;   //队列中的包数减少
            Packetq->size -= pkt1->pkt.size + sizeof(*pkt1);
         //尝试共享同一个数据缓存,进行计数管理
         if (av_packet_ref(pkt, &pkt1->pkt) < 0 ) //成功则增加引用计数
         {
            ret = 0;
            break;
         }
      //Dereference before releasing the memory, otherwise it will cause a memory leak
            av_packet_unref(&pkt1->pkt);//减少引用计数
            av_free(pkt1);              //取出的这个队列节点已经不用了,因为他是在前面的put里av_malloc来的
            ret = 1;                    //Success returns 1
            break;
        }
        else if (block == 0)  //队列为空，block是阻塞标志，为0表示不阻，立即返回0
        {
            ret = 0;
            break;
        }
        else
            SDL_CondWait(Packetq->cond, Packetq->mutex); // 在队列无数据的时候，设定为阻塞等待
    }
    SDL_UnlockMutex(Packetq->mutex);
    return ret;
}
/********************************************************
Function:    packet_queue_flush
Description: 清空队列
Input:  PacketQueue *
OutPut: nullptr
Return:null
Others:
Date:   2022-01-06
*********************************************************/
void packet_queue_flush(PacketQueue *Packetq)
{
    MyAVPacketList *pkt, *pkt1;

    SDL_LockMutex(Packetq->mutex);
    for(pkt = Packetq->first_pkt; pkt != NULL; pkt = pkt1)
    {
        pkt1 = pkt->next;
        av_packet_unref(&pkt->pkt);
        av_freep(&pkt);
    }
    //Reset
    Packetq->size = 0;
    Packetq->nb_packets = 0;
    Packetq->first_pkt = NULL;
    Packetq->last_pkt = NULL;
    SDL_UnlockMutex(Packetq->mutex);
}
