#include "lv_demo_calculator.h"
#include <string>
#include <sstream>
#include <unistd.h>

#if ENABLE_GIAC
using namespace giac;
#endif
using namespace std;

static lv_obj_t* ta;
static lv_obj_t* ta2;
static lv_obj_t* kb;

void lv_demo_calculator::fetchAndSolve(){
  #if ENABLE_GIAC
  static context ct;
  static gen g;
  static string line;
  stringstream output;
  //getline(cin, line);
  lv_textarea_del_char(ta);
  line = lv_textarea_get_text(ta);
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
  lv_textarea_set_text(ta2,output.str().c_str());
  lv_textarea_set_text(ta,"");
  #else 
  
  #endif
} 


void lv_demo_calculator::createTextArea(){
   /*Create the text area for keyboard*/
    ta = lv_textarea_create(lv_scr_act());
    lv_obj_set_size(ta, 320, 240);
    //lv_obj_set_pos(ta2, 0, 0);
    lv_obj_add_event_cb(ta, textArea_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_text(ta, "");
   /* Create the text area for solution */
    ta2 = lv_textarea_create(lv_scr_act());
    lv_obj_set_size(ta2, 320, 240);
    lv_obj_set_pos(ta2, 0, 75);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_text(ta, "");


 /* Create a keyboard */
    kb = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(kb,  LV_HOR_RES, LV_VER_RES / 2);

    lv_keyboard_set_textarea(kb, ta); /* Focus it on one of the text areas to start */
   // lv_keyboard_set_cursor_manage(kb, true); /* Automatically show/hide cursors on text areas */
}

void lv_demo_calculator::textArea_cb(lv_event_t* e){
	const char * txt = lv_textarea_get_text(ta);
       int length = strlen(txt);
       if(txt[length-1] == '='){
        fetchAndSolve();
        }
}
