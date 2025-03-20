#include "../include/common_inc.h"
#include "Model.h"
#include "MusicObj.h"

pthread_mutex_t lv_mutex;
static pthread_t threadLvgl;
static Page::Model *model;

MusicObj *musicObj = nullptr; // 当前播放的音乐

static void exitCallback(void);
void *threadLvglHandler(void *);

int main(int argc, char *argv[])
{
    printf("[Sys] EasyMediaPlayer begin!\n");

    // 清除fb0
    // system("dd if=/dev/zero of=/dev/fb0");
    // 打开音频通路并设置音量
    system("amixer sset Headphone unmute");
    system("amixer sset \"Headphone volume\" 2");

    // Init HAL
    HAL::Init();

    snd_pcm_init();
    snd_mixer_init();

    // model初始化
    model = new Page::Model(exitCallback, lv_mutex);

    // lv_obj_fade_in(lv_scr_act(), 350, 0);

    /* Handle LitlevGL tasks (tickless mode) */
    pthread_create(&threadLvgl, NULL, threadLvglHandler, NULL);

    musicObj = new MusicObj("/mnt/UDISK/music1.mp3");
    musicObj->play();

    while (1)
    {
        // ...
    }

    return 0;
}

/**
 * @brief LVGL处理线程
 *
 * @return void*
 */
void *threadLvglHandler(void *)
{
    HAL::LVGL_Proc();
}

/**
 * @brief 退出回调函数
 */
static void exitCallback(void)
{
    delete model;

    exit(0);
}