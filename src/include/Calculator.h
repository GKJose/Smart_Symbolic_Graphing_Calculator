#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "calc_conf.h"
#if ENABLE_GIAC
#if ENABLE_LINUX
#include <giac/config.h>
#endif
#include <giac/gen.h>
#include <giac/giac.h>
#endif
#include <iostream>

#if ENABLE_MCP_KEYPAD
#include <Keypad.hxx>
#endif
#include <lvgl/lvgl.h>
#include <string>
#include <sstream>
#include <unistd.h>
//#include <giac/config.h>
#include <tabs.hxx>

#include <iostream>
#include <fstream>
#include <gmpxx.h>
#include <nlohmann/json.hpp>
#include <ctime>

namespace Calculator{
    void createDemo();
	void update(lv_timer_t * timer);
	lv_obj_t* lv_textarea_input(lv_obj_t* parent);
    lv_obj_t* lv_textarea_output(lv_obj_t* parent);
    void main_screen_driver(lv_obj_t* parent, bool first_screen);
	lv_obj_t* lv_input_history_ta(lv_obj_t* parent, std::string output, lv_obj_t* active_ta);
	lv_obj_t* lv_result_ta(lv_obj_t* parent, std::string output, lv_obj_t* active_ta);	
	static void kb_event_cb(lv_event_t* e);
	static void toggle_kb_event_handler(lv_event_t* e);
	static void active_ta_event_handler(lv_event_t* e);
	void storeFunctionTA(lv_obj_t* ta);
	void storeWifiTA(lv_obj_t* ta);
	static void clear_scr_btn_event_handler(lv_event_t* e);
	static void input_history_ta_event_handler(lv_event_t* e);


	
}

#endif
