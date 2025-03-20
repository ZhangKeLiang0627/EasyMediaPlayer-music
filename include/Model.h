#pragma once

#include <string>
#include <functional>
#include <dirent.h>
#include "common_inc.h"
#include "View.h"
#include "../libs/lvgl/lvgl.h"

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

    private:
        static void *threadProcHandler(void *);
        void update(void);
        static void onTimerUpdate(lv_timer_t *timer);


        // funtion for View


    public:
        Model(std::function<void(void)> exitCb, pthread_mutex_t &mutex);
        ~Model();
    };
}