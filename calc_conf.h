#ifndef SMART_CALC_H
#define SMART_CALC_H

#define ENABLE_GIAC 0
#define ENABLE_PI 0
#define ENABLE_LINUX 0
#define ENABLE_WINDOWS 1

#if ENABLE_WINDOWS
#define USE_WIN32DRV 1
#define USE_FBDEV 0
#define USE_EVDEV 0
#endif

#if ENABLE_LINUX
#define USE_MONITOR 1
#define USE_MOUSEWHEEL 1
#define USE_MOUSE 1
#define USE_KEYBOARD 1
#endif

#endif