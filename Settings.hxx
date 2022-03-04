#ifndef SETTINGS_HXX
#define SETTINGS_HXX

#include "lvgl/lvgl.h"

#if ENABLE_PI
#include <wiringPi.h>
#include <softPwm.h>
#endif

#include <cstdio>

static void slider_event_cb(lv_event_t * e);
void createSettingsTab(lv_obj_t* parent);



#endif