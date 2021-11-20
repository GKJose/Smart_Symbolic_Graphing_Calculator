#ifndef LV_CALCULATOR_H
#define LV_CALCULATOR_H

#include "../../../calc_conf.h"
#if ENABLE_GIAC
#include<giac/giac.h>
#endif
#include <iostream>
#include "../../lv_demo.h"

namespace lv_demo_calculator{
        void createDemo();
	void fetchAndSolve();
	void createTextArea();
        void createMenu();
        void menu_cb(lv_event_t* e);
	void textArea_cb(lv_event_t* e);
        void createOperatorWindow();
        void operatorWindow_cb(lv_event_t* e);
}

#endif
