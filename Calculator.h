#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "calc_conf.h"
#if ENABLE_GIAC
#include<giac/giac.h>
#endif
#include <iostream>

#include "Keypad.h"
#include "lvgl/lvgl.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include <unistd.h>
//#include <giac/config.h>
#include <giac/gen.h>
#include <giac/giac.h>
#include "tabs.hxx"

#include <iostream>
#include <gmpxx.h>

namespace Calculator{
        void createDemo();
	void update(lv_timer_t * timer);
	void fetchAndSolve();
	lv_obj_t* lv_textarea_input(lv_obj_t* parent);
    	lv_obj_t* lv_textarea_output(lv_obj_t* parent);
    	void main_screen_driver(lv_obj_t* parent);
	lv_obj_t* lv_input_history_ta(lv_obj_t* parent, std::string output);
	lv_obj_t* lv_result_ta(lv_obj_t* parent, std::string output);	
	static void kb_event_cb(lv_event_t* e);
	static void toggle_kb_event_handler(lv_event_t* e);
	void main_screen_driver(lv_obj_t* parent);
	static void active_ta_event_handler(lv_event_t* e);

	
}

#endif
