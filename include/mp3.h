#ifndef _MP3_H_
#define _MP3_H_

#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>
#include <mad.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>

// wav 播放控制结构体
typedef  struct
{
    pthread_t decode_thread;            //MP3解码线程
    sem_t pcm_ready_sem;              //数据就绪信号量
    sem_t decode_ready_sem;         //解码就绪信号量
    bool decode_thread_state;       
    struct mad_decoder decoder;     //解码器

    u_int8_t *mmstart;
    u_int32_t lenght;
    struct stat stat;
    int fd;

    u_int8_t *pcm_buff;         //解码后的pcm数据缓存区
    u_int32_t pcm_buff_size;          //pcm数据缓存区数据字节数

    u_int32_t samplerate; //采样率
    u_int32_t nchannels;   //通道数量;1,表示单声道;2,表示双声道;

    u_int32_t totsec; //整首歌时长,单位:秒
    int32_t cursec; //当前播放时长
    int32_t dst_cursec;       //目标时间点，用于设置播放时间点
    bool dst_seted;

    u_int64_t duration_frac;        
    u_int32_t duration_sec;

    u_int64_t timelapse_frac;
    u_int32_t timelapse_sec;
                                      
    u_int32_t header_cnt;
    
}__attribute__((packed)) mp3ctrl_t;


#ifdef __cplusplus
extern "C"
{
#endif

int open_mp3_file(const char *path, mp3ctrl_t *mp3x);
u_int8_t * mp3_read_decode_frame(mp3ctrl_t *mp3x, int *size);
bool is_mp3(const char *file);
void mp3_set_curtime(mp3ctrl_t *mp3x, u_int32_t dst_sec);
void mp3_close(mp3ctrl_t *mp3x);

#ifdef __cplusplus
}
#endif


#endif
