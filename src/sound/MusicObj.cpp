#include "MusicObj.h"
#include "wav.h"
#include "mp3.h"
#include <stdlib.h>

static void music_play_thread_func(MusicObj *obj);

/*
 *@brief 创建music对象
 *@param path 文件完整路径
 */
MusicObj::MusicObj(string path)
{
    playState = false;
    threadState = true;
    cur_sec = 0;
    fd = -1;

    if (is_wav(path.c_str()) != false)
    {
        type = MusicType_WAV;
        ctrl = malloc(sizeof(wavctrl_t));
        wavctrl_t *wavx = (wavctrl_t *)ctrl;

        fd = open_wav_file(path.c_str(), wavx);
        lseek(fd, wavx->datastart, SEEK_SET); // 跳过文件头

        printf("audio format:%d\n", wavx->audioformat);
        printf("n channels:%d\n", wavx->nchannels);
        printf("samplerate:%d\n", wavx->samplerate);
        printf("bitrate:%d\n", wavx->bitrate);
        printf("blockalign:%d\n", wavx->blockalign);
        printf("bps:%d\n", wavx->bps);
        printf("datasize:%d\n", wavx->datasize);
        printf("datastart:%d\n", wavx->datastart);
    }
    else if (is_mp3(path.c_str()) != false)
    {
        type = MusicType_MP3;
        ctrl = calloc(1, sizeof(mp3ctrl_t));
        mp3ctrl_t *mp3x = (mp3ctrl_t *)ctrl;
        fd = 0;

        int ret = open_mp3_file(path.c_str(), mp3x);
        if (ret == 0)
        {
            printf("mp3 file name:%s\n", path.c_str());
        }
        else
            printf("mp3 file open error\n");
    }
    else
        printf("not wav file:%d\n", fd);

    file = path;

    int v = countTotalTime(); // 计算总播放时间
    setTotalValue(v);         // 设置变量值

    playThread = new thread(music_play_thread_func, this); // 创建播放线程
}

MusicObj::~MusicObj()
{
    if (type == MusicType_MP3)
        mp3_close((mp3ctrl_t *)ctrl);
    else
        close(fd);

    free(ctrl);
    threadState = false;
    playState = true;
    playThread->join();
    delete playThread;
}

void MusicObj::play(void)
{
    playState = true;

    snd_pcm_t *pcm = snd_pcm_get();

    int ret = snd_pcm_prepare(pcm); // 播放
    if (ret < 0)
        fprintf(stderr, "snd_pcm_prepare error:%s\n", snd_strerror(ret));
}
void MusicObj::pause(void)
{
    playState = false;

    snd_pcm_t *pcm = snd_pcm_get();

    int ret = snd_pcm_drop(pcm); // 暂停
    if (ret < 0)
        fprintf(stderr, "snd_pcm_drop error:%s\n", snd_strerror(ret));
}

/*
 *@brief 计算当前播放时间点
 *@return 当前播放时间点，单位sec
 */
u_int32_t MusicObj::countCurTime(void)
{
    u_int32_t pos = 10;

    switch (type)
    {
    case MusicType_WAV:
        pos = wav_get_curtime(fd, (wavctrl_t *)ctrl);
        break;
    case MusicType_MP3:
        pos = ((mp3ctrl_t *)ctrl)->cursec;
        break;

    default:;
    }

    return pos;
}

/*
 *@brief 计算总播放时间
 *@return 总播放时间，单位sec
 */
u_int32_t MusicObj::countTotalTime(void)
{
    u_int32_t total = 100;

    switch (type)
    {
    case MusicType_WAV:
        wav_get_curtime(fd, (wavctrl_t *)ctrl);
        total = ((wavctrl_t *)ctrl)->totsec;
        break;
    case MusicType_MP3:
        total = ((mp3ctrl_t *)ctrl)->totsec;
        break;

    default:;
    }

    return total;
}

/*
 *@brief 设置当前播放时间点
 *@param cur 播放时间点，单位sec
 */
void MusicObj::setCurTime(u_int32_t cur)
{
    switch (type)
    {
    case MusicType_WAV:
        wav_set_curtime(fd, (wavctrl_t *)ctrl, cur);
        break;
    case MusicType_MP3:
        mp3_set_curtime((mp3ctrl_t *)ctrl, cur);
        break;

    default:;
    }
}

/*
 *@brief 获取音频文件数据格式
 *@return 数据格式
 */
snd_pcm_format_t MusicObj::getFormat(void)
{
    snd_pcm_format_t fmt = SND_PCM_FORMAT_S16;

    switch (type)
    {
    case MusicType_WAV:
    {
        wavctrl_t *wavx = (wavctrl_t *)ctrl;
        if (wavx->bps == 16)
            fmt = SND_PCM_FORMAT_S16;
        else if (wavx->bps == 24)
            fmt = SND_PCM_FORMAT_S24;
    }
    break;
    case MusicType_MP3:
        fmt = SND_PCM_FORMAT_S16;
        break;

    default:;
    }

    return fmt;
}

/*
 *@brief 获取音频采样率
 *@return 采样率
 */
u_int32_t MusicObj::getRate(void)
{
    u_int32_t rate = 44100;

    switch (type)
    {
    case MusicType_WAV:
    {
        wavctrl_t *wavx = (wavctrl_t *)ctrl;
        rate = wavx->samplerate;
    }
    break;
    case MusicType_MP3:
    {
        // mp3ctrl_t *mp3x = (mp3ctrl_t *)ctrl;
        rate = 44100;
    }
    break;

    default:;
    }

    return rate;
}

/*
 *@brief 获取音频声道数
 *@return 声道数
 */
u_int32_t MusicObj::getChannels(void)
{
    u_int32_t channels = 2;
    switch (type)
    {
    case MusicType_WAV:
    {
        wavctrl_t *wavx = (wavctrl_t *)ctrl;
        channels = wavx->nchannels;
    }
    break;
    case MusicType_MP3:
    {
        // mp3ctrl_t *mp3x = (mp3ctrl_t *)ctrl;
        channels = 2;
    }
    break;

    default:;
    }

    return channels;
}

u_int32_t MusicObj::getFrameSize(void)
{
    u_int32_t size = 4;
    switch (type)
    {
    case MusicType_WAV:
    {
        wavctrl_t *wavx = (wavctrl_t *)ctrl;
        size = wavx->blockalign;
    }
    break;

    default:;
    }

    return size;
}

u_int32_t MusicObj::getPeriodSize(void)
{
    u_int32_t size = 1024;
    switch (type)
    {
    case MusicType_WAV:
    {
        size = 1152;
    }
    break;
    case MusicType_MP3:
    {
        size = 1152;
    }
    break;

    default:;
    }

    return size;
}

u_int8_t *MusicObj::getPcmData(int *size, int offset)
{
    void *buf = NULL;

    switch (type)
    {
    case MusicType_WAV:
    {
        wavctrl_t *wavx = (wavctrl_t *)ctrl;
        buf = calloc(1, getFrameSize() * getPeriodSize());
        if (offset > 0)
            lseek(fd, offset * getFrameSize(), SEEK_CUR);

        int ret = read(fd, buf, getFrameSize() * getPeriodSize());
        if (ret <= 0)
        {
            free(buf);
            buf = NULL;
        }
        *size = ret / getFrameSize();
    }
    break;
    case MusicType_MP3:
    {
        mp3ctrl_t *mp3x = (mp3ctrl_t *)ctrl;
        buf = mp3_read_decode_frame(mp3x, size);
    }
    break;

    default:;
    }

    return (u_int8_t *)buf;
}

/*
 *@brief music播放线程函数
 *@param obj 播放的对象
 */
static void music_play_thread_func(MusicObj *obj)
{
    snd_pcm_t *pcm = snd_pcm_get(); // 获取PCM设备
    if (pcm == NULL || obj->fd < 0)
        return;

    snd_pcm_set_format(obj->getFormat());     // 设置数据格式
    snd_pcm_set_rate(obj->getRate());         // 设置采样率
    snd_pcm_set_channels(obj->getChannels()); // 设置声道数
    // snd_pcm_set_period_size(obj->getPeriodSize()); // 设置周期大小
    snd_pcm_prepare(pcm);

    int ret = 0;
    int size = 0;

    void *buff = NULL;
    int offset = 0;

    buff = obj->getPcmData(&size, offset);
    if (buff == NULL)
        obj->threadState = false;

    while (obj->threadState)
    {
        int v = obj->countCurTime(); // 计算播放时间点
        obj->setCurValue(v);         // 设置变量值
        offset = 0;

        ret = snd_pcm_writei(pcm, buff, size);
        if (ret < 0)
        {
            // fprintf(stderr, "snd_pcm_writei error: %s\n", snd_strerror(ret));
            usleep(100000);
        }
        else
        {
            if (ret < obj->getPeriodSize())
                offset = ret - obj->getPeriodSize();

            if (buff != NULL)
            {
                free(buff);
                buff = NULL;
            }

            buff = obj->getPcmData(&size, offset);
            if (buff == NULL)
                obj->threadState = false;
        }
    }

    snd_pcm_drop(pcm);
    if (buff != NULL)
        free(buff);
}