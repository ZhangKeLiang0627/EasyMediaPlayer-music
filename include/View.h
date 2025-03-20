#pragma once

#include "../libs/lvgl/lvgl.h"
#include "lv_obj_ext_func.h"
#include "lv_anim_timeline_wrapper.h"
#include <functional>

namespace Page
{
    using ExitCb = std::function<void(void)>;

    struct Operations
    {
        ExitCb exitCb;
    };

    class View
    {
    private:
        Operations _opts; // View回调函数集

    public:
        struct
        {
            lv_obj_t *cont;

            struct
            {
                lv_obj_t *cont;
                lv_obj_t *btn;
            } btnCont;

            struct
            {
                lv_obj_t *cont;
                lv_obj_t *slider;
            } sliderCont;

            struct
            {
                lv_obj_t *cont;
            } listCont;

            lv_anim_timeline_t *anim_timeline;
            lv_anim_timeline_t *anim_timelineClick;
        } ui;

        void create(Operations &opts);
        void release(void);
        void appearAnimStart(bool reverse = false);
        void appearAnimClick(bool reverse = false);
        void addVideoList(const char *name, const void *img_src);
        void setPlayProgress(int cur, int total);

    private:
        void AttachEvent(lv_obj_t *obj);

        void contCreate(lv_obj_t *obj);
        void btnContCreate(lv_obj_t *obj);
        void sliderContCreate(lv_obj_t *obj);
        void listContCreate(lv_obj_t *obj);

        static void onEvent(lv_event_t *event);
        static void buttonEventHandler(lv_event_t *event);
        static void listBtnEventHandler(lv_event_t *event);
        static void sliderEventHandler(lv_event_t *event);

        lv_obj_t *roundRectCreate(lv_obj_t *par, lv_coord_t x_ofs, lv_coord_t y_ofs);
        lv_obj_t *btnCreate(lv_obj_t *par, const void *img_src, lv_coord_t y_ofs);
        lv_obj_t *sliderCreate(lv_obj_t *par, const void *img_src, lv_coord_t x_ofs = 0, lv_coord_t y_ofs = 0, int32_t min = 0, int32_t max = 255, int32_t val = 0);
        lv_obj_t *listCreate(const char *name, const void *img_src);
    };

}