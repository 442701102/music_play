#ifndef AVPACKET_MANAGER_H
#define AVPACKET_MANAGER_H
#include <env.h>


#ifdef __cplusplus
extern "C"
{
#endif
typedef struct MyAVPacketList
{
    AVPacket pkt;
    struct MyAVPacketList *next;
    int serial;
} MyAVPacketList;

typedef struct PacketQueue
{
    MyAVPacketList *first_pkt, *last_pkt; /**队列头指针 ,尾指针**/
    int nb_packets; /**队列长度，即包的个数**/
    int size;   /**该队列数据占用空间的总字节数**/
    int64_t duration;
    int abort_request;   //是否要中止队列操作，用于安全快速退出播放
    int serial;
    SDL_mutex *mutex;  /**互斥量,用于维持PacketQueue的多线程安全(类似pthread_mutex_t）**/
    SDL_cond *cond;   /**状态变量,用于读、写线程相互通知(类似pthread_cond_t)**/
} PacketQueue;

int packet_queue_init(PacketQueue *Packetq);
int packet_queue_put(PacketQueue *Packetq, AVPacket *pkt);
int packet_queue_get(PacketQueue *Packetq, AVPacket *pkt, int block);
void packet_queue_flush(PacketQueue *Packetq);


#ifdef __cplusplus
}
#endif


#endif // AVPACKET_MANAGER_H
