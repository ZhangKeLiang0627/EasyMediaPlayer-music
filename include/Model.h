#pragma once

#include <string>
#include <functional>
#include <dirent.h>
#include "common_inc.h"
#include "View.h"
#include "../libs/lvgl/lvgl.h"
#include "MusicObj.h"
#include "lrc.h"
namespace Page
{
    class Model
    {
    public:
    private:
        pthread_t _pthread;      // 数据处理线程
        bool _threadExitFlag;    // 线程退出标志位
        pthread_mutex_t *_mutex; // 互斥量
        View _view;              // View的实例
        lv_timer_t *_timer;      // LVGL软定时器

        int _musicNum;                           // 保存音乐数目
        int _musicIndex = 0;                     // 当前播放音乐索引
        MusicObj *_musicObj = nullptr;           // 当前播放的音乐
        lyric_t *_musicLyric = nullptr;          // 当前播放音乐的歌词
        int _musicLyricSentences = 0;            // 当前播放音乐的歌词句数
        PlayMode _musicMode = PlayMode_ListLoop; // 音乐播放模式

    private:
        static void *threadProcHandler(void *);
        void update(void);
        static void onTimerUpdate(lv_timer_t *timer);

        int searchMusic(string path);

        // Funtion for View
        int getState(void);
        void pause(void);
        void play(const char *name, int index);
        void setCur(int cur_sec);
        void setMode(PlayMode mode);
        PlayMode getMode(void);
        void changeMusic(void);

    public:
        Model(std::function<void(void)> exitCb, pthread_mutex_t &mutex);
        ~Model();
    };
}