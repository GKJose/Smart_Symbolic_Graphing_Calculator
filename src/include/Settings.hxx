#ifndef SETTINGS_HXX
#define SETTINGS_HXX

#include <lvgl/lvgl.h>
#include <calc_conf.h>
#if ENABLE_PI
#include <wiringPi.h>
#include <softPwm.h>
#endif
#include <stdlib.h>
#include <cstdio>
// #include "utils.hxx"
enum {
    LV_MENU_ITEM_BUILDER_VARIANT_1,
    LV_MENU_ITEM_BUILDER_VARIANT_2
};
typedef uint8_t lv_menu_builder_variant_t;
static void updateButton_cb(lv_event_t * e);
static void back_event_handler(lv_event_t * e);
static void switch_handler(lv_event_t * e);
static lv_obj_t * create_text(lv_obj_t * parent, const char * icon, const char * txt,
                              lv_menu_builder_variant_t builder_variant);
static lv_obj_t * create_slider(lv_obj_t * parent,
                                const char * icon, const char * txt, int32_t min, int32_t max, int32_t val);
static lv_obj_t * create_switch(lv_obj_t * parent,
                                const char * icon, const char * txt, bool chk);
static void slider_event_cb(lv_event_t * e);
static lv_obj_t * create_button(lv_obj_t * parent, const char * txt);
void createSettingsTab(lv_obj_t* parent);



#endif