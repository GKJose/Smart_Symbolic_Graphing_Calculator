#ifndef SETTINGS_HXX
#define SETTINGS_HXX

#include <lvgl/lvgl.h>
#include <calc_conf.h>
#if ENABLE_MCP_KEYPAD
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
//static void switch_handler(lv_event_t * e);
//static void slider_event_cb(lv_event_t * e);
void createSettingsTab(lv_obj_t* parent);
void forceDisconnect();


#endif