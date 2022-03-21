#include <calc_conf.h>
#include <SDL2/SDL.h>
#include "lv_drivers/sdl/sdl_gpu.h"
#include <lvgl/lvgl.h>
#include <Calculator.h>
#include <chrono>
#include <thread>

void* sdl_display;

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

//#if ENABLE_LINUX && ENABLE_PI == 0
//#include "lv_drivers/sdl/sdl.h"
// #include "lv_drivers/display/monitor.h"
// #include "lv_drivers/indev/mouse.h"
// #include "lv_drivers/indev/keyboard.h"
// #include "lv_drivers/indev/mousewheel.h"
//#endif

// #include "lv_drivers/display/monitor.h"
// #include "lv_drivers/indev/mouse.h"
// #include "lv_drivers/indev/keyboard.h"
// #include "lv_drivers/indev/mousewheel.h"

#if ENABLE_PI 
#include <wiringPi.h>
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#define PI_EXEC(x) x
#else 
#define PI_EXEC(x)
#endif

//#define DISP_BUF_SIZE (64 * 320)

static void calc_init(void);


int main(void)
{
    calc_init();
    PI_EXEC(wiringPiSetup());

    Calculator::createDemo();
    lv_timer_create(Calculator::update,350,NULL);
    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
        lv_timer_handler();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    return 0;
}

// /*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
// uint32_t custom_tick_get(void)
// {
//     static auto start = std::chrono::steady_clock::now();
//     auto now = std::chrono::steady_clock::now();
//     return std::chrono::duration_cast<std::chrono::milliseconds>(now-start).count();
// }

static void calc_init(void){
    /*LittlevGL init*/
    lv_init();
    monitor_init();

    static lv_disp_draw_buf_t disp_buf1;
    static lv_disp_drv_t disp_drv;
    sdl_gpu_disp_draw_buf_init(&disp_buf1);
    sdl_gpu_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf1;
    disp_drv.flush_cb = sdl_display_flush;
    disp_drv.hor_res = SDL_HOR_RES;
    disp_drv.ver_res = SDL_VER_RES;
    disp_drv.antialiasing = 1;

    lv_disp_t* disp = lv_disp_drv_register(&disp_drv);
    sdl_display = disp;
    lv_theme_t * th = lv_theme_default_init(disp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), LV_THEME_DEFAULT_DARK, LV_FONT_DEFAULT);
    lv_disp_set_theme(disp, th);

    lv_group_t* g = lv_group_create();
    lv_group_set_default(g);

    /* Add the mouse as input device
    * Use the 'mouse' driver which reads the PC's mouse*/
    static lv_indev_drv_t indev_drv_1;
    lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
    indev_drv_1.type = LV_INDEV_TYPE_POINTER;

    /*This function will be called periodically (by the library) to get the mouse position and state*/
    indev_drv_1.read_cb = sdl_mouse_read;
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv_1);

    static lv_indev_drv_t indev_drv_2;
    lv_indev_drv_init(&indev_drv_2); /*Basic initialization*/
    indev_drv_2.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv_2.read_cb = sdl_keyboard_read;
    lv_indev_t *kb_indev = lv_indev_drv_register(&indev_drv_2);
    lv_indev_set_group(kb_indev, g);

    static lv_indev_drv_t indev_drv_3;
    lv_indev_drv_init(&indev_drv_3); /*Basic initialization*/
    indev_drv_3.type = LV_INDEV_TYPE_ENCODER;
    indev_drv_3.read_cb = sdl_mousewheel_read;
    lv_indev_t * enc_indev = lv_indev_drv_register(&indev_drv_3);
    lv_indev_set_group(enc_indev, g);
}