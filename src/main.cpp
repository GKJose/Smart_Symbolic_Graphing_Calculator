#include <calc_conf.h>
#include <lvgl/lvgl.h>
#include <Calculator.h>
#include <chrono>
#include <thread>
#if ENABLE_WINDOWS
#include<Windows.h>
#define IDI_LVGL                       101
// Next default values for new objects
//
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        102
#define _APS_NEXT_COMMAND_VALUE         40001
#define _APS_NEXT_CONTROL_VALUE         1001
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif

#if _MSC_VER >= 1200
 // Disable compilation warnings.
#pragma warning(push)
// nonstandard extension used : bit field types other than int
#pragma warning(disable:4214)
// 'conversion' conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable:4244)
#endif

#include "lv_drivers/win32drv/win32drv.h"

#if _MSC_VER >= 1200
// Restore compilation warnings.
#pragma warning(pop)
#endif

#endif

#if ENABLE_LINUX
#include <SDL2/SDL.h>
#include "lv_drivers/display/monitor.h"
#include "lv_drivers/indev/mouse.h"
#include "lv_drivers/indev/keyboard.h"
#include "lv_drivers/indev/mousewheel.h"
#endif

#if ENABLE_LINUX_FRAMEBUFFER
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#endif

#if ENABLE_MCP_KEYPAD
#include <wiringPi.h>
#endif

#define DISP_BUF_SIZE (64 * 320)


static void calc_init(void);
int main(void)
{
    calc_init();
    #if ENABLE_MCP_KEYPAD
    wiringPiSetup();
    #endif
    Calculator::createDemo();
    lv_timer_create(Calculator::update,350,NULL);
    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
        lv_task_handler();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now-start).count();
}

static void calc_init(void){
    /*LittlevGL init*/
    lv_init();
    #if ENABLE_WINDOWS 
    lv_win32_init(GetModuleHandleW(NULL), SW_SHOW, 320, 240, LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCE(IDI_LVGL)));
    lv_win32_add_all_input_devices_to_group(NULL);
    #else
    #if ENABLE_LINUX
    monitor_init();
    SDL_CreateThread((SDL_ThreadFunction)custom_tick_get, "tick", nullptr);
    #elif ENABLE_LINUX_FRAMEBUFFER 
    /*Linux frame buffer device init*/
    fbdev_init();
    #endif

    static lv_disp_draw_buf_t disp_buf;
    #if ENABLE_LINUX_FRAMEBUFFER
    static lv_color_t buf[DISP_BUF_SIZE];
	static lv_color_t buf2[DISP_BUF_SIZE];
    lv_disp_draw_buf_init(&disp_buf, buf, &buf2, DISP_BUF_SIZE);
    #else 
    static lv_color_t buf1[MONITOR_HOR_RES * 100];
    static lv_color_t buf2[MONITOR_HOR_RES * 100];
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, MONITOR_HOR_RES * 100);
    #endif
    

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf   = &disp_buf;
    #if ENABLE_LINUX
    disp_drv.flush_cb   = monitor_flush;
    disp_drv.hor_res    = MONITOR_HOR_RES;
    disp_drv.ver_res    = MONITOR_VER_RES;
    disp_drv.antialiasing = 1;
    #elif ENABLE_LINUX_FRAMEBUFFER
    disp_drv.flush_cb   = fbdev_flush;
    disp_drv.hor_res    = 320;
    disp_drv.ver_res    = 240;
    #endif
    
    lv_disp_drv_register(&disp_drv);

    #if ENABLE_LINUX
    lv_group_t* g = lv_group_create();
    lv_group_set_default(g);

    /* Add the mouse as input device
    * Use the 'mouse' driver which reads the PC's mouse*/
    mouse_init();
    static lv_indev_drv_t indev_drv_1;
    lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
    indev_drv_1.type = LV_INDEV_TYPE_POINTER;
    indev_drv_1.read_cb = mouse_read;
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv_1);

    keyboard_init();
    static lv_indev_drv_t indev_drv_2;
    lv_indev_drv_init(&indev_drv_2); /*Basic initialization*/
    indev_drv_2.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv_2.read_cb = keyboard_read;
    lv_indev_t *kb_indev = lv_indev_drv_register(&indev_drv_2);
    lv_indev_set_group(kb_indev, g);

    mousewheel_init();
    static lv_indev_drv_t indev_drv_3;
    lv_indev_drv_init(&indev_drv_3); /*Basic initialization*/
    indev_drv_3.type = LV_INDEV_TYPE_ENCODER;
    indev_drv_3.read_cb = mousewheel_read;

    lv_indev_t * enc_indev = lv_indev_drv_register(&indev_drv_3);
    lv_indev_set_group(enc_indev, g);
    #elif ENABLE_LINUX_FRAMEBUFFER
    evdev_init();
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;
    lv_indev_drv_register(&indev_drv);
    #endif
    #endif
}
