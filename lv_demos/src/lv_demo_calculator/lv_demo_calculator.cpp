#include "lv_demo_calculator.h"
#include <string>
#include <sstream>
#include <unistd.h>

#if ENABLE_GIAC
using namespace giac;
#endif
using namespace std;

static lv_obj_t* textArea;
static lv_obj_t* textArea2;
static lv_obj_t* menu_btnmatrix;
static lv_obj_t* operatorWindow;
void lv_demo_calculator::createDemo(){
	
	createTextArea();
	createMenu();
}
void lv_demo_calculator::fetchAndSolve(){
  #if ENABLE_GIAC
  static context ct;
  static gen g;
  static string line;
  stringstream output;
  //getline(cin, line);
  lv_textarea_del_char(textArea);
  line = lv_textarea_get_text(textArea);
  cout << line;
  g = gen(line, &ct);
  line += " = ";
  output << line;
      try {
    output << eval(g,1,&ct);
  } catch (...) {
    output << "ERROR: Something went wrong!\n";
  }
  output << "\n";
  lv_textarea_set_text(textArea2,output.str().c_str());
  lv_textarea_set_text(textArea,"");
  #else 
  
  #endif
} 

void lv_demo_calculator::createMenu(){
	
static const char * btnm_map[] = {"1", "2", "3", "4", "5", "\n",
                                  "6", "7", "8", "9", "0", "\n",
                                  "Operators", "Trig", "Misc.","X"};
	menu_btnmatrix = lv_btnmatrix_create(lv_scr_act());
    lv_btnmatrix_set_map(menu_btnmatrix, btnm_map);
    lv_btnmatrix_set_btn_width(menu_btnmatrix, 10, 2);        /*Make "Action1" twice as wide as "Action2"*/
    lv_btnmatrix_set_btn_ctrl(menu_btnmatrix, 10, LV_BTNMATRIX_CTRL_CHECKABLE);
    lv_btnmatrix_set_btn_ctrl(menu_btnmatrix, 11, LV_BTNMATRIX_CTRL_CHECKED);
    lv_obj_align(menu_btnmatrix, LV_ALIGN_CENTER, 0, 50);
    lv_obj_add_event_cb(menu_btnmatrix, menu_cb, LV_EVENT_ALL,NULL);								  
}
void lv_demo_calculator::menu_cb(lv_event_t* e){
	lv_event_code_t event = lv_event_get_code(e);
	lv_obj_t* obj = lv_event_get_target(e);
    if(event == LV_EVENT_VALUE_CHANGED) {
		uint32_t id = lv_btnmatrix_get_selected_btn(obj);
        const char * txt = lv_btnmatrix_get_btn_text(obj,id);
        if(txt == "Operators"){
		}else if(txt == "Trig"){
		}else if(txt == "Misc."){
		}else if(txt == "X"){
			lv_textarea_del_char(textArea);	
		}else{
			lv_textarea_add_text(textArea, txt);
		}
    }		
}
void lv_demo_calculator::createTextArea(){
   /*Create the text area for keyboard*/
    textArea = lv_textarea_create(lv_scr_act());
    lv_obj_set_size(textArea, 320, 240);
    //lv_obj_set_pos(ta2, 0, 0);
    lv_obj_add_event_cb(textArea, textArea_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_textarea_set_one_line(textArea, true);
    lv_textarea_set_text(textArea, "");
   /* Create the text area for solution */
    textArea2 = lv_textarea_create(lv_scr_act());
    lv_obj_set_size(textArea2, 320, 240);
    lv_obj_set_pos(textArea2, 0, 75);
    lv_textarea_set_one_line(textArea, true);
    lv_textarea_set_text(textArea, "");

}

void lv_demo_calculator::textArea_cb(lv_event_t* e){
	const char * txt = lv_textarea_get_text(textArea);
       int length = strlen(txt);
       if(txt[length-1] == '='){
        fetchAndSolve();
        }
}
void lv_demo_calculator::createOperatorWindow(){
	
}
void lv_demo_calculator::operatorWindow_cb(lv_event_t* e){
}
