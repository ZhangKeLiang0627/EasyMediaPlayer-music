#ifndef _MUSICOBJ_H_
#define _MUSICOBJ_H_

#include "sound.h"
#include <string>
#include <thread>

using namespace std;

class MusicObj;

class MusicObj
{
public:
    enum MusicType
    {
        MusicType_WAV = 0,
        MusicType_MP3,
    };

    int fd;           // 文件描述符
    string file;      // 文件名称
    bool threadState; // 线程状态，用于退出播放线程
    MusicType type;   // 音乐文件类型

private:
    thread *playThread; // 播放线程
    bool playState;     // 播放状态
    void *ctrl;         // 音乐控制信息
    int cur_sec;        // 当前播放时间点
    int total_src;      // 播放总时间

public:
    MusicObj(string path);
    ~MusicObj();

    void setCurValue(int v) { cur_sec = v; }
    void setTotalValue(int v) { total_src = v; }

    void play(void);
    void pause(void);

    bool getPlayState(void) const { return playState; }
    int getCurTime(void) { return cur_sec; }
    int getTotalTime(void) { return total_src; }

    snd_pcm_format_t getFormat(void);
    u_int32_t getRate(void);
    u_int32_t getChannels(void);
    u_int32_t getFrameSize(void);
    u_int32_t getPeriodSize(void);
    u_int8_t *getPcmData(int *size, int offset);
    bool isOver(void) { return !threadState; }

    u_int32_t countCurTime(void);
    u_int32_t countTotalTime(void);
    void setCurTime(u_int32_t cur);
};

#endif
