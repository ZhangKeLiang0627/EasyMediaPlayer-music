#include "sound.h"
#include <string.h>

#define PCM_PLAYBACK_DEV "default"
#define MIXER_DEV "default"

static snd_pcm_t *pcm = NULL;                // pcm 句柄
static snd_pcm_uframes_t period_size = 1152; // 周期大小（单位: 帧）
static unsigned int periods = 4;             // 周期数（设备驱动层 buffer 的大小）
static snd_pcm_hw_params_t *hwparams = NULL;

static int max_volume = 100;                       // 保存最大音量
static snd_mixer_t *mixer = NULL;                  // 混音器
static snd_mixer_elem_t *playback_vol_elem = NULL; // 混音器播放音量元素

/**
 *@brief 初始化pcm
 *@return 成功返回0  失败返回-1
 */
int snd_pcm_init(void)
{
    int ret;
    /** 打开 PCM 设备 */
    ret = snd_pcm_open(&pcm, PCM_PLAYBACK_DEV, SND_PCM_STREAM_PLAYBACK, 0);
    if (0 > ret)
    {
        fprintf(stderr, "snd_pcm_open error: %s: %s\n",
                PCM_PLAYBACK_DEV, snd_strerror(ret));
        return -1;
    }
    /** 实例化 hwparams 对象 */
    snd_pcm_hw_params_malloc(&hwparams);
    /** 获取 PCM 设备当前硬件配置,对 hwparams 进行初始化 */
    ret = snd_pcm_hw_params_any(pcm, hwparams);
    if (0 > ret)
    {
        fprintf(stderr, "snd_pcm_hw_params_any error: %s\n", snd_strerror(ret));
        goto err2;
    }
    /***************
    设置参数
    ***************/
    /** 设置访问类型: 交错模式 */
    ret = snd_pcm_hw_params_set_access(pcm, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (0 > ret)
    {
        fprintf(stderr, "snd_pcm_hw_params_set_access error: %s\n", snd_strerror(ret));
        goto err2;
    }
    /** 设置数据格式: 有符号 16 位、小端模式 */
    ret = snd_pcm_hw_params_set_format(pcm, hwparams, SND_PCM_FORMAT_S16_LE);
    if (0 > ret)
    {
        fprintf(stderr, "snd_pcm_hw_params_set_format error: %s\n", snd_strerror(ret));
        goto err2;
    }
    /** 设置采样率 */
    ret = snd_pcm_hw_params_set_rate(pcm, hwparams, 44100, 0);
    if (0 > ret)
    {
        fprintf(stderr, "snd_pcm_hw_params_set_rate error: %s\n", snd_strerror(ret));
        goto err2;
    }
    /** 设置声道数: 双声道 */
    ret = snd_pcm_hw_params_set_channels(pcm, hwparams, 2);
    if (0 > ret)
    {
        fprintf(stderr, "snd_pcm_hw_params_set_channels error: %s\n", snd_strerror(ret));
        goto err2;
    }
    /** 设置周期大小: period_size */
    ret = snd_pcm_hw_params_set_period_size_near(pcm, hwparams, &period_size, 0);
    if (0 > ret)
    {
        fprintf(stderr, "snd_pcm_hw_params_set_period_size error: %s\n", snd_strerror(ret));
        goto err2;
    }
    printf("period size(frame):%d\n", period_size);
    /** 设置周期数（驱动层 buffer 的大小）: periods */
    ret = snd_pcm_hw_params_set_periods(pcm, hwparams, periods, 0);
    if (0 > ret)
    {
        fprintf(stderr, "snd_pcm_hw_params_set_periods error: %s\n", snd_strerror(ret));
        goto err2;
    }
    /** 使配置生效 */
    ret = snd_pcm_hw_params(pcm, hwparams);
    if (0 > ret)
    {
        fprintf(stderr, "snd_pcm_hw_params error: %s\n", snd_strerror(ret));
        goto err1;
    }
    return 0;

err2:
    snd_pcm_hw_params_free(hwparams); // 释放内存
err1:
    snd_pcm_close(pcm); // 关闭 pcm 设备
    return -1;
}

/**
 *@brief 设置PCM数据格式
 *@param format 数据格式
 */
void snd_pcm_set_format(snd_pcm_format_t format)
{
    int ret = snd_pcm_hw_params_set_format(pcm, hwparams, format); // 设置数据格式
    if (0 > ret)
        fprintf(stderr, "snd_pcm_hw_params_set_format error: %s\n", snd_strerror(ret));
    else
        snd_pcm_hw_params(pcm, hwparams); // 写入配置到硬件
}

/**
 *@brief 设置采样率
 *@param rate 采样率
 */
void snd_pcm_set_rate(unsigned int rate)
{
    int ret = snd_pcm_hw_params_set_rate(pcm, hwparams, rate, 0); // 设置采样率
    if (0 > ret)
        fprintf(stderr, "snd_pcm_hw_params_set_rate error: %s\n", snd_strerror(ret));
    else
        snd_pcm_hw_params(pcm, hwparams); // 写入配置到硬件
}

/**
 *@brief 设置声道数
 *@param channel 声道数
 */
void snd_pcm_set_channels(unsigned int channels)
{
    int ret = snd_pcm_hw_params_set_channels(pcm, hwparams, channels); // 设置声道数
    if (0 > ret)
        fprintf(stderr, "snd_pcm_hw_params_set_channels error: %s\n", snd_strerror(ret));
    else
        snd_pcm_hw_params(pcm, hwparams); // 写入配置到硬件
}

/**
 *@brief 获取PCM周期大小
 *@return 周期大小(帧)
 */
unsigned int snd_pcm_get_period_size(void)
{
    return period_size;
}

/**
 *@brief 设置周期大小
 *@param size 周期大小(单位: 帧)
 */
void snd_pcm_set_period_size(unsigned int size)
{
    period_size = size;
    int ret = snd_pcm_hw_params_set_period_size(pcm, hwparams, period_size, 0);
    if (0 > ret)
        fprintf(stderr, "snd_pcm_hw_params_set_period_size error: %s\n", snd_strerror(ret));
    else
        snd_pcm_hw_params(pcm, hwparams); // 写入配置到硬件
}

/**
 *@brief 获取PCM句柄
 *@return PCM句柄
 */
snd_pcm_t *snd_pcm_get(void)
{
    return pcm;
}

void snd_pcm_dev_close(void)
{
    snd_pcm_hw_params_free(hwparams); // 释放 hwparams 对象占用的内存
    snd_pcm_close(pcm);               // 关闭 pcm 设备
}

/**
 *@brief 初始化混音器mixer
 *@return 成功返回0  失败返回-1
 */
int snd_mixer_init(void)
{
    snd_mixer_elem_t *elem = NULL;
    const char *elem_name = NULL;
    long max, min;

    int ret = snd_mixer_open(&mixer, 0); // 打开混音器
    if (ret < 0)
    {
        fprintf(stderr, "snd_mixer_open error:%s\n", snd_strerror(ret));
        mixer = NULL;
        return -1;
    }

    ret = snd_mixer_attach(mixer, MIXER_DEV); // 关联声卡控制设备
    if (ret < 0)
    {
        fprintf(stderr, "snd_mixer_attach error:%s\n", snd_strerror(ret));
        goto err;
    }

    ret = snd_mixer_selem_register(mixer, NULL, NULL); // 注册混音器
    if (ret < 0)
    {
        fprintf(stderr, "snd_mixer_selem_register error:%s\n", snd_strerror(ret));
        goto err;
    }

    ret = snd_mixer_load(mixer); // 加载混音器
    if (ret < 0)
    {
        fprintf(stderr, "snd_mixer_load error:%s\n", snd_strerror(ret));
        goto err;
    }

    // 遍历混音器元素
    for (elem = snd_mixer_first_elem(mixer); elem != NULL; elem = snd_mixer_elem_next(elem))
    {
        elem_name = snd_mixer_selem_get_name(elem);
        // if(strcmp(elem_name, "Headphone volume") == 0)
        // {
        //     snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
        //     snd_mixer_selem_set_playback_volume_all(elem, (max - min) *0.5f + min);       //默认音量50%
        //     printf("Volume max:%d\n", max);
        //     printf("Volume min:%d\n", min);
        // }
        // else if(strcmp(elem_name, "DAC volume") == 0)
        // {
        //     snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
        //     snd_mixer_selem_set_playback_volume_all(elem, (max - min) *0.7f + min);       //默认音量70%
        //     printf("DAC max:%d\n", max);
        //     printf("DAC min:%d\n", min);
        // }
        // else
        if (strcmp(elem_name, "Soft Volume Master") == 0)
        {
            snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
            snd_mixer_selem_set_playback_volume_all(elem, (max - min) * 0.8f + min); // 默认音量80%
            playback_vol_elem = elem;
            max_volume = max;
            printf("Soft max:%d\n", max);
            printf("Soft min:%d\n", min);
        }
    }

    return 0;

err:
    snd_mixer_close(mixer);
    mixer = NULL;
    return -1;
}

void snd_mixer_dev_close(void)
{
    snd_mixer_close(mixer);
}

void snd_get_volume(int *cur, int *max)
{
    long v;

    snd_mixer_selem_get_playback_volume(playback_vol_elem, 0, &v);
    *cur = v;
    *max = max_volume;
}

void snd_set_volume(int v)
{
    if (v > max_volume)
        v = max_volume;

    snd_mixer_selem_set_playback_volume_all(playback_vol_elem, v); // 设置音量
}