#include "wav.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/*
 *@brief 打开并解析wav文件
 *@param file 文件完整路径
 *@param wavx 保存wav信息结构
 *@return 成功返回文件描述符，失败返回负数
 */
int open_wav_file(const char *file, wavctrl_t *wavx)
{
    ChunkRIFF *riff;
    ChunkFMT *fmt;
    ChunkFACT *fact;
    ChunkDATA *data;
    int ret = 0;

    if (wavx == NULL)
        return -1;

    int fd = open(file, O_RDONLY);
    if (0 > fd)
    {
        fprintf(stderr, "open error: %s: %s\n", file, strerror(errno));
        return -1;
    }

    char *buf = (char *)malloc(512);

    ret = read(fd, buf, 512); // 读取512字节在数据
    if (ret > 0)
    {
        riff = (ChunkRIFF *)buf;        // 获取RIFF块
        if (riff->Format == 0X45564157) // 是WAV文件
        {
            fmt = (ChunkFMT *)(buf + 12);                        // 获取FMT块
            fact = (ChunkFACT *)(buf + 12 + 8 + fmt->ChunkSize); // 读取FACT块
            if (fact->ChunkID == 0X74636166 || fact->ChunkID == 0X5453494C)
                wavx->datastart = 12 + 8 + fmt->ChunkSize + 8 + fact->ChunkSize; // 具有fact/LIST块的时候(未测试)
            else
                wavx->datastart = 12 + 8 + fmt->ChunkSize;
            data = (ChunkDATA *)(buf + wavx->datastart); // 读取DATA块
            if (data->ChunkID == 0X61746164)             // 解析成功!
            {
                wavx->audioformat = fmt->AudioFormat; // 音频格式
                wavx->nchannels = fmt->NumOfChannels; // 通道数
                wavx->samplerate = fmt->SampleRate;   // 采样率
                wavx->bitrate = fmt->ByteRate * 8;    // 得到位速
                wavx->blockalign = fmt->BlockAlign;   // 块对齐
                wavx->bps = fmt->BitsPerSample;       // 位数,16/24/32位

                wavx->datasize = data->ChunkSize;      // 数据块大小
                wavx->datastart = wavx->datastart + 8; // 数据流开始的地方.
            }
            else
                ret = -3; // data区域未找到
        }
        else
            ret = -2; // 非wav文件
    }
    else
        ret = -1; // 读取失败

    free((void *)buf);

    return ret > 0 ? fd : ret;
}

/*
 *@brief 根据文件名后缀判断是否是wav文件
 *@param file 文件名
 *@return 是返回true，否返回false
 */
bool is_wav(const char *file)
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

    return !(strcasecmp(attr, ".wav"));
}

/*
 *@brief 获取wav当前播放时间
 *@param fd wav文件描述符
 *@param wavx wav信息结构
 *@return 返回已播放的秒数
 */
u_int32_t wav_get_curtime(int fd, wavctrl_t *wavx)
{
    long long fpos;

    int pos = lseek(fd, 0, SEEK_CUR); // 获取文件读写指针位置

    wavx->totsec = wavx->datasize / (wavx->bitrate / 8); // 歌曲总长度(单位:秒)
    fpos = pos - wavx->datastart;                        // 得到当前文件播放到的地方
    wavx->cursec = fpos * wavx->totsec / wavx->datasize; // 当前播放到第多少秒了?

    return wavx->cursec;
}

/*
 *@brief 设置wav当前播放时间点
 *@param fd wav文件描述符
 *@param wavx wav信息结构
 */
void wav_set_curtime(int fd, wavctrl_t *wavx, u_int32_t sec_offst)
{
    long long fpos = 0;

    if (sec_offst > wavx->totsec)
        return;

    fpos += sec_offst * (wavx->bitrate / 8); // 偏移秒 * 每秒数据量 = 数据偏移量

    lseek(fd, wavx->datastart + fpos, SEEK_SET); // 设置文件偏移
}