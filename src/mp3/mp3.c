#include "mp3.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "mp3filo.h"

#define MP3_OUTPUT_SIZE 6912

static void *mp3_decode_thread_func(void *arg);
static int decode_preprocess(mp3ctrl_t *mp3x);

int open_mp3_file(const char *path, mp3ctrl_t *mp3x)
{
    if (path == NULL || mp3x == NULL)
        return -1;

    mp3x->fd = open(path, O_RDONLY);
    if (mp3x->fd < 0)
        return -1;

    mp3x->pcm_buff = (u_int8_t *)calloc(1, MP3_OUTPUT_SIZE);
    mp3x->pcm_buff_size = 0;

    fstat(mp3x->fd, &mp3x->stat);
    mp3x->lenght = mp3x->stat.st_size;

    mp3x->mmstart = mmap(0, mp3x->stat.st_size, PROT_READ, MAP_SHARED, mp3x->fd, 0);
    if (mp3x->mmstart == MAP_FAILED)
        return -1;

    mp3x->duration_frac = 0;
    mp3x->duration_sec = 0;

    mp3x->timelapse_frac = 0;
    mp3x->timelapse_sec = 0;

    mp3x->header_cnt = 0;
    mp3x->dst_cursec = -1;
    mp3x->dst_seted = false;

    // 遍历帧头，建立帧表
    mp3filo_alloc();
    decode_preprocess(mp3x);
    mp3x->totsec = (1.0f * mp3x->duration_frac / MAD_TIMER_RESOLUTION) + mp3x->duration_sec; // 计算总时长

    sem_init(&mp3x->pcm_ready_sem, 0, 0); // 初始化信号量
    sem_init(&mp3x->decode_ready_sem, 0, 1);

    mp3x->decode_thread_state = true;

    // 创建解码线程
    pthread_create(&mp3x->decode_thread, NULL, mp3_decode_thread_func, mp3x);

    return 0;
}

u_int8_t *mp3_read_decode_frame(mp3ctrl_t *mp3x, int *size)
{
    sem_wait(&mp3x->pcm_ready_sem); // 等待解码数据就绪信号量

    if (mp3x->decode_thread_state == false)
        return NULL;

    *size = mp3x->pcm_buff_size;
    u_int8_t *buf = malloc(mp3x->pcm_buff_size * 2 * mp3x->nchannels);
    memcpy(buf, mp3x->pcm_buff, mp3x->pcm_buff_size * 2 * mp3x->nchannels);

    sem_post(&mp3x->decode_ready_sem); // 发送解码就绪信号量

    return buf;
}

/*
 *@brief 根据文件名后缀判断是否是mp3文件
 *@param file 文件名
 *@return 是返回true，否返回false
 */
bool is_mp3(const char *file)
{
    if (file == NULL)
        return false;

    u_int32_t i = 0;
    const char *attr = NULL;

    while (i < 255 && file[i] != '\0')
        i++;

    for (int j = 0; j < 5; j++)
    {
        if (file[--i] == '.')
        {
            attr = &file[i];
            break;
        }
    }

    return !(strcasecmp(attr, ".mp3"));
}

/*
 *@brief 设置mp3当前播放时间点
 *@param mp3x mp3
 */
void mp3_set_curtime(mp3ctrl_t *mp3x, u_int32_t dst_sec)
{
    mp3x->dst_cursec = dst_sec;
    mp3x->dst_seted = true;
    printf("dst cur:%d\n", dst_sec);
}

void mp3_close(mp3ctrl_t *mp3x)
{
    mp3x->decode_thread_state = false;

    sem_post(&mp3x->pcm_ready_sem);
    sem_post(&mp3x->decode_ready_sem);

    pthread_join(mp3x->decode_thread, NULL);

    usleep(100000);

    mp3filo_free();
    sem_destroy(&mp3x->pcm_ready_sem);
    sem_destroy(&mp3x->decode_ready_sem);
    free((void *)mp3x->pcm_buff);

    munmap(mp3x->mmstart, mp3x->stat.st_size);
    close(mp3x->fd);
}

/*
 *@brief mp3解码数据输入回调函数
 */
static enum mad_flow input(void *user_data, struct mad_stream *stream)
{
    mp3ctrl_t *mp3x = (mp3ctrl_t *)user_data;

    if (!mp3x->lenght)
        return MAD_FLOW_STOP;

    mad_stream_buffer(stream, mp3x->mmstart, mp3x->lenght);

    mp3x->lenght = 0;

    return MAD_FLOW_CONTINUE;
}

/*
 *@brief mp3解码数据缩放
 */
static inline signed int scale(mad_fixed_t sample)
{
    sample += (1L << (MAD_F_FRACBITS - 16));

    if (sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if (sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;

    return sample >> (MAD_F_FRACBITS + 1 - 16);
}

/*
 *@brief mp3解码帧头读取后回调函数
 */
static enum mad_flow header(void *user_data, struct mad_header const *header)
{
    int percent;
    char ch;
    ch = 0;
    int ret = MAD_FLOW_CONTINUE;
    mp3ctrl_t *mp3x = user_data;
    mp3_header_pos_t headpos;

    if (mp3x->decode_thread_state == false)
        return MAD_FLOW_STOP;

    // 帧头计数
    mp3x->header_cnt++;

    // 设置播放时间点
    if (mp3x->dst_seted != false)
    {
        mp3x->header_cnt = 0;        // 从头开始

        // 重新填装解码buff
        mad_stream_buffer(&(mp3x->decoder.sync->stream), mp3x->mmstart, mp3x->stat.st_size);
        mp3x->timelapse_sec = 0;
        mp3x->timelapse_frac = 0;
        mp3x->dst_seted = false;
    }

    // 计算当前播放时间点
    mp3x->timelapse_frac += header->duration.fraction;
    mp3x->timelapse_sec += header->duration.seconds;
    mp3x->cursec = (1.0f * mp3x->timelapse_frac / MAD_TIMER_RESOLUTION) + mp3x->timelapse_sec;

    if (mp3x->dst_cursec >= 0)
    {
        if (mp3x->cursec < mp3x->dst_cursec) // 设置的时间点大于当前时间点
            ret = MAD_FLOW_IGNORE;           // 不解码
        else
            mp3x->dst_cursec = -1;
    }

    return ret;
}

/*
 *@brief mp3解码数据输出回调函数
 */
static enum mad_flow output(void *user_data, struct mad_header const *header, struct mad_pcm *pcm)
{
    unsigned int nchannels, nsamples;
    mad_fixed_t const *left_ch, *right_ch;
    mp3ctrl_t *mp3x = (mp3ctrl_t *)user_data;

    sem_wait(&mp3x->decode_ready_sem); // 等待解码就绪信号量

    nchannels = pcm->channels;
    nsamples = pcm->length;
    left_ch = pcm->samples[0];
    right_ch = pcm->samples[1];
    mp3x->samplerate = pcm->samplerate;
    mp3x->nchannels = nchannels;

    u_int8_t *output = mp3x->pcm_buff;

    mp3x->pcm_buff_size = nsamples;

    for (int i = 0; i < nsamples; i++)
    {
        signed int sample;

        sample = scale(*left_ch++); // 获取左声道数据
        *(output++) = sample >> 0;
        *(output++) = sample >> 8;
        if (nchannels == 2)
        {
            sample = scale(*right_ch++); // 获取右声道数据
            *(output++) = sample >> 0;
            *(output++) = sample >> 8;
        }
    }

    // 发送数据就绪信号量
    sem_post(&mp3x->pcm_ready_sem);

    return MAD_FLOW_CONTINUE;
}

/*
 *@brief mp3解码数据错误回调函数
 */
static enum mad_flow error(void *user_data, struct mad_stream *stream, struct mad_frame *frame)
{
    mp3ctrl_t *mp3x = (mp3ctrl_t *)user_data;

    printf("mp3 file decode error\n");
    fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n", stream->error,
            mad_stream_errorstr(stream), stream->this_frame - mp3x->mmstart);

    return MAD_FLOW_CONTINUE;
}

/*
 *@brief mp3帧头遍历
 */
static enum mad_flow header_preprocess(void *user_data, struct mad_header const *header)
{
    mp3_header_pos_t head_pos;
    mp3ctrl_t *mp3x = (mp3ctrl_t *)user_data;

    /* Count duration */
    mp3x->duration_frac += header->duration.fraction;
    mp3x->duration_sec += header->duration.seconds;

    /* Fill in filo_headpos */
    head_pos.offset = (mp3x->decoder.sync->stream.this_frame) - (mp3x->decoder.sync->stream.buffer);
    head_pos.timelapse = (1.0 * mp3x->duration_frac / MAD_TIMER_RESOLUTION) + mp3x->duration_sec;
    mp3filo_push(&head_pos);

    /* Do not decode */
    return MAD_FLOW_IGNORE;
}

/*
 *@brief mp3前处理，遍历帧头
 */
static int decode_preprocess(mp3ctrl_t *mp3x)
{
    int result;

    /* configure input, output, and error functions */
    mad_decoder_init(&mp3x->decoder, mp3x,
                     input, header_preprocess /* header */, 0 /* filter */, 0, /* output */
                     0, /* error */ 0 /* message */);

    /* start decoding */
    result = mad_decoder_run(&mp3x->decoder, MAD_DECODER_MODE_SYNC);

    /* release the decoder */
    mad_decoder_finish(&mp3x->decoder);

    mp3x->lenght = mp3x->stat.st_size;

    return result;
}

static void *mp3_decode_thread_func(void *arg)
{
    mp3ctrl_t *mp3x = (mp3ctrl_t *)arg;

    mad_decoder_init(&mp3x->decoder, mp3x, input, header, NULL, output, error, NULL); // 初始化解码器

    printf("decode run\n");
    int ret = mad_decoder_run(&mp3x->decoder, MAD_DECODER_MODE_SYNC);

    mad_decoder_finish(&mp3x->decoder);

    printf("decode finish, code:%d\n", ret);
    mp3x->decode_thread_state = false;
    return NULL;
}