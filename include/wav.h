#ifndef _WAV_H_
#define _WAV_H_

#include <sys/types.h>
#include <stdbool.h>

// WAV 音频文件解析相关数据结构
// RIFF块
typedef struct
{
    u_int32_t ChunkID;   // chunk id;这里固定为"RIFF",即0X46464952
    u_int32_t ChunkSize; // 集合大小;文件总大小-8
    u_int32_t Format;    // 格式;WAVE,即0X45564157
} __attribute__((packed)) ChunkRIFF;
// fmt块
typedef struct
{
    u_int32_t ChunkID;       // chunk id;这里固定为"fmt ",即0X20746D66
    u_int32_t ChunkSize;     // 子集合大小(不包括ID和Size);这里为:20.
    u_int16_t AudioFormat;   // 音频格式;0X01,表示线性PCM;0X11表示IMA ADPCM
    u_int16_t NumOfChannels; // 通道数量;1,表示单声道;2,表示双声道;
    u_int32_t SampleRate;    // 采样率;0X1F40,表示8Khz
    u_int32_t ByteRate;      // 字节速率;
    u_int16_t BlockAlign;    // 块对齐(字节);
    u_int16_t BitsPerSample; // 单个采样数据大小;4位ADPCM,设置为4
    //	u_int16_t ByteExtraData;		//附加的数据字节;2个; 线性PCM,没有这个参数
} __attribute__((packed)) ChunkFMT;
// fact块
typedef struct
{
    u_int32_t ChunkID;      // chunk id;这里固定为"fact",即0X74636166;
    u_int32_t ChunkSize;    // 子集合大小(不包括ID和Size);这里为:4.
    u_int32_t NumOfSamples; // 采样的数量;
} __attribute__((packed)) ChunkFACT;
// LIST块
typedef struct
{
    u_int32_t ChunkID;   // chunk id;这里固定为"LIST",即0X74636166;
    u_int32_t ChunkSize; // 子集合大小(不包括ID和Size);这里为:4.
} __attribute__((packed)) ChunkLIST;

// data块
typedef struct
{
    u_int32_t ChunkID;   // chunk id;这里固定为"data",即0X5453494C
    u_int32_t ChunkSize; // 子集合大小(不包括ID和Size)
} __attribute__((packed)) ChunkDATA;

// wav头
typedef struct
{
    ChunkRIFF riff; // riff块
    ChunkFMT fmt;   // fmt块
    //	ChunkFACT fact;	//fact块 线性PCM,没有这个结构体
    ChunkDATA data; // data块
} __attribute__((packed)) __WaveHeader;

// wav 播放控制结构体
typedef struct
{
    u_int16_t audioformat; // 音频格式;0X01,表示线性PCM;0X11表示IMA ADPCM
    u_int16_t nchannels;   // 通道数量;1,表示单声道;2,表示双声道;
    u_int16_t blockalign;  // 块对齐(字节);
    u_int32_t datasize;    // WAV数据大小

    u_int32_t totsec; // 整首歌时长,单位:秒
    u_int32_t cursec; // 当前播放时长

    u_int32_t bitrate;    // 比特率(位速)
    u_int32_t samplerate; // 采样率
    u_int16_t bps;        // 位数,比如16bit,24bit,32bit

    u_int32_t datastart; // 数据帧开始的位置(在文件里面的偏移)
} __attribute__((packed)) wavctrl_t;

#ifdef __cplusplus
extern "C"
{
#endif

    int open_wav_file(const char *file, wavctrl_t *wavx);
    bool is_wav(const char *file);
    u_int32_t wav_get_curtime(int fd, wavctrl_t *wavx);
    void wav_set_curtime(int fd, wavctrl_t *wavx, u_int32_t sec_offst);

#ifdef __cplusplus
}
#endif

#endif
