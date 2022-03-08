CMAKE_MINIMUM_REQUIRED(VERSION 3.22.3)
PROJECT (SmartSymbolicGraphingCalculator)
SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG")

include_directories(${CMAKE_SOURCE_DIR})
set(SOURCE_FILES main.cpp Calculator.cpp graph.cpp graphing.cpp Keypad.cpp Settings.cpp tabs.cpp)
add_link_options(-lm -lpthread -lSDL2 -lgmp -lgiac)
file(GLOB_RECURSE LVGL_SOURCE_FILES "lvgl/*.c")
file(GLOB_RECURSE LVGL_DRIVERS_SOURCE_FILES "lv_drivers/*.c")


ADD_EXECUTABLE(demo ${LVGL_DRIVERS_SOURCE_FILES} ${LVGL_SOURCE_FILES} ${SOURCE_FILES})

