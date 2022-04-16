#include <Calculator.h>
#include <ctime>
#include <string>
#include <sstream>
#include <unistd.h>
#if ENABLE_MCP_KEYPAD
#include <Keypad.hxx>
#endif
#if ENABLE_GIAC
using namespace giac;
#endif
using namespace std;

/*Declarations*/
static lv_obj_t* textArea;
#if ENABLE_MCP_KEYPAD
static Keypad keypad;
#endif

/*Areas holds a list of pointers to the active text areas.
  (1 Input, 1 Output) = 1 Entry to the calculator.
  After 50 Entries the main screen clears to prevent memory overfill.
  Segmentation Fault occurs at 60 Entries, 120 Working Text areas.*/
lv_obj_t* areas[100];
int total;

static lv_obj_t* kb;
static lv_obj_t* toggle_kb_btn;
static lv_obj_t* clear_scr_btn;
static lv_obj_t* tabview;
static lv_obj_t* functionTextArea;
static lv_obj_t* wifiTextArea;
class Solve
{
    public:
    giac::context ctx;

	Solve(){
		giac::gen g("approx_mode:=1", &ctx);
		giac::eval(g, &ctx);
		assert(giac::approx_mode(&ctx) == true);
	}

    std::string call_giac(std::string input)
    {      
		//giac::approx_mode(false, &ctx); // Change graphing to calculate approximate values.
        giac::gen g(input, &ctx);
        std::cout << giac::eval(g, &ctx) << "\n";    
        giac::gen args(input, &ctx);
        std::string output = input + "\n";
        try{
            output = giac::gen2string(giac::eval(args, &ctx));
        }
        catch(...){
            output = "ERROR: Something went wrong!\n";
        }
        return output;
    }
};

void Calculator::createDemo(){
	tabview = lv_main_screen_tabs();
	std::cout << "tabview assigned\n";
	textArea = areas[0];
}
void Calculator::update(lv_timer_t * timer){
	#if ENABLE_MCP_KEYPAD
	keypad.poll();
	if(keypad.isBothPressed(ALT_BUTTON,ALPHA_BUTTON)){
		keypad.reset();
	}else if(keypad.isPressed(ALT_BUTTON)){
		if(keypad.isPressed(X_SQUARED_BUTTON)){
			lv_textarea_add_text(textArea,"sqrt(");
		}else if(keypad.isPressed(LOG_BUTTON)){
			lv_textarea_add_text(textArea,"10^");
		}else if(keypad.isPressed(LN_BUTTON)){
			lv_textarea_add_text(textArea,"e^");
		}else if(keypad.isPressed(MODE_BUTTON)){
			lv_textarea_add_text(textArea,"^");
		}else if(keypad.isPressed(SETTINGS_BUTTON)){
			//DO NOTHING
		}else if(keypad.isPressed(LEFT_PARATHESIS_BUTTON)){
			lv_textarea_add_text(textArea,"sin(");
		}else if(keypad.isPressed(LEFT_BUTTON)){
			lv_textarea_add_text(textArea,"csc(");
		}else if(keypad.isPressed(Y_EQUALS_BUTTON)){
			//PI BUTTON
			lv_textarea_add_text(textArea,"3.14");
		}else if(keypad.isPressed(SEVEN_BUTTON)){
			//DO NOTHING
		}else if(keypad.isPressed(FOUR_BUTTON)){
			//DO NOTHING
		}else if(keypad.isPressed(ONE_BUTTON)){
			//DO NOTHING
		}else if(keypad.isPressed(ZERO_BUTTON)){
			//DO NOTHING
		}else if(keypad.isPressed(UP_BUTTON)){
			lv_textarea_add_text(textArea,"cos(");
		}else if(keypad.isPressed(SELECT_BUTTON)){
			lv_textarea_add_text(textArea,"sec(");
		}else if(keypad.isPressed(DOWN_BUTTON)){
			//DO NOTHING
		}else if(keypad.isPressed(EIGHT_BUTTON)){
			//DO NOTHING
		}else if(keypad.isPressed(FIVE_BUTTON)){
			//DO NOTHING
		}else if(keypad.isPressed(TWO_BUTTON)){
			//DO NOTHING;
		}else if(keypad.isPressed(DOT_SIGN_BUTTON)){
			//DO NOTHING
		}else if(keypad.isPressed(RIGHT_PARATHESIS_BUTTON)){
			lv_textarea_add_text(textArea,"tan(");
		}else if(keypad.isPressed(RIGHT_BUTTON)){
			lv_textarea_add_text(textArea,"cot(");
		}else if(keypad.isPressed(GRAPH_BUTTON)){
			//DO NOTHING
		}else if(keypad.isPressed(NINE_BUTTON)){
			//DO NOTHING
		}else if(keypad.isPressed(SIX_BUTTON)){
			//DO NOTHING
		}else if(keypad.isPressed(THREE_BUTTON)){
			//DO NOTHING
		}else if(keypad.isPressed(MINUS_SIGN_BUTTON)){
			//ANS BUTTON
		}else if(keypad.isPressed(CLEAR_BUTTON)){
			//DO NOTHING
		}else if(keypad.isPressed(DELETE_BUTTON)){
			//DO NOTHING
		}else if(keypad.isPressed(DIVIDE_BUTTON)){
			lv_textarea_add_text(textArea,"diff(");
		}else if(keypad.isPressed(MULTIPLY_BUTTON)){
			lv_textarea_add_text(textArea,"integrate(");
		}else if(keypad.isPressed(SUBTRACT_BUTTON)){
			lv_textarea_add_text(textArea,"factor(");
		}else if(keypad.isPressed(ADD_BUTTON)){
			//DO NOTHING
		}else if(keypad.isPressed(ENTER_BUTTON)){
			//ENTRY BUTTON
		}	
	}else if(keypad.isPressed(ALPHA_BUTTON)){
		if(keypad.isPressed(X_SQUARED_BUTTON)){
			lv_textarea_add_text(textArea,"e");
		}else if(keypad.isPressed(LOG_BUTTON)){
			lv_textarea_add_text(textArea,"j");
		}else if(keypad.isPressed(LN_BUTTON)){
			lv_textarea_add_text(textArea,"o");
		}else if(keypad.isPressed(MODE_BUTTON)){
			lv_textarea_add_text(textArea,"T");
		}else if(keypad.isPressed(SETTINGS_BUTTON)){
			lv_textarea_add_text(textArea,":");
		}else if(keypad.isPressed(LEFT_PARATHESIS_BUTTON)){
			lv_textarea_add_text(textArea,"[");
		}else if(keypad.isPressed(LEFT_BUTTON)){
			lv_textarea_add_text(textArea,"a");
		}else if(keypad.isPressed(Y_EQUALS_BUTTON)){
			lv_textarea_add_text(textArea,"f");
		}else if(keypad.isPressed(SEVEN_BUTTON)){
			lv_textarea_add_text(textArea,"k");
		}else if(keypad.isPressed(FOUR_BUTTON)){
			lv_textarea_add_text(textArea,"p");
		}else if(keypad.isPressed(ONE_BUTTON)){
			lv_textarea_add_text(textArea,"u");
		}else if(keypad.isPressed(ZERO_BUTTON)){
			lv_textarea_add_text(textArea,"y");
		}else if(keypad.isPressed(UP_BUTTON)){
			lv_textarea_add_text(textArea,",");
		}else if(keypad.isPressed(SELECT_BUTTON)){
			lv_textarea_add_text(textArea,"b");
		}else if(keypad.isPressed(DOWN_BUTTON)){
			lv_textarea_add_text(textArea,"g");
		}else if(keypad.isPressed(EIGHT_BUTTON)){
			lv_textarea_add_text(textArea,"l");
		}else if(keypad.isPressed(FIVE_BUTTON)){
			lv_textarea_add_text(textArea,"q");
		}else if(keypad.isPressed(TWO_BUTTON)){
			lv_textarea_add_text(textArea,"v");
		}else if(keypad.isPressed(DOT_SIGN_BUTTON)){
			lv_textarea_add_text(textArea,"z");
		}else if(keypad.isPressed(RIGHT_PARATHESIS_BUTTON)){
			lv_textarea_add_text(textArea,"]");
		}else if(keypad.isPressed(RIGHT_BUTTON)){
			lv_textarea_add_text(textArea,"c");
		}else if(keypad.isPressed(GRAPH_BUTTON)){
			lv_textarea_add_text(textArea,"h");
		}else if(keypad.isPressed(NINE_BUTTON)){
			lv_textarea_add_text(textArea,"m");
		}else if(keypad.isPressed(SIX_BUTTON)){
			lv_textarea_add_text(textArea,"r");
		}else if(keypad.isPressed(THREE_BUTTON)){
			lv_textarea_add_text(textArea,"w");
		}else if(keypad.isPressed(MINUS_SIGN_BUTTON)){
			lv_textarea_add_text(textArea,"=");
		}else if(keypad.isPressed(CLEAR_BUTTON)){
			//DO NOTHING
		}else if(keypad.isPressed(DELETE_BUTTON)){
			lv_textarea_add_text(textArea,"d");
		}else if(keypad.isPressed(DIVIDE_BUTTON)){
			lv_textarea_add_text(textArea,"i");
		}else if(keypad.isPressed(MULTIPLY_BUTTON)){
			lv_textarea_add_text(textArea,"n");
		}else if(keypad.isPressed(SUBTRACT_BUTTON)){
			lv_textarea_add_text(textArea,"s");
		}else if(keypad.isPressed(ADD_BUTTON)){
			lv_textarea_add_text(textArea,"x");
		}else if(keypad.isPressed(ENTER_BUTTON)){
			//SOLVE BUTTON
		}		
	}else{
		if(keypad.isPressed(X_SQUARED_BUTTON)){
			lv_textarea_add_text(textArea,"^2");
		}else if(keypad.isPressed(LOG_BUTTON)){
			lv_textarea_add_text(textArea,"log(");
		}else if(keypad.isPressed(LN_BUTTON)){
			lv_textarea_add_text(textArea,"ln(");
		}else if(keypad.isPressed(MODE_BUTTON)){
			//HOME BUTTON
			lv_tabview_set_act(tabview, 0, LV_ANIM_OFF);
			textArea = areas[0];

		}else if(keypad.isPressed(SETTINGS_BUTTON)){
			lv_tabview_set_act(tabview, 2, LV_ANIM_OFF);
		}else if(keypad.isPressed(LEFT_PARATHESIS_BUTTON)){
			lv_textarea_add_text(textArea,"(");
		}else if(keypad.isPressed(LEFT_BUTTON)){
			lv_textarea_cursor_left(textArea);
		}else if(keypad.isPressed(Y_EQUALS_BUTTON)){
			//Y= BUTTON 
		}else if(keypad.isPressed(SEVEN_BUTTON)){
			lv_textarea_add_text(textArea,"7");
		}else if(keypad.isPressed(FOUR_BUTTON)){
			lv_textarea_add_text(textArea,"4");
		}else if(keypad.isPressed(ONE_BUTTON)){
			lv_textarea_add_text(textArea,"1");
		}else if(keypad.isPressed(ZERO_BUTTON)){
			lv_textarea_add_text(textArea,"0");
		}else if(keypad.isPressed(UP_BUTTON)){
			//UP
		}else if(keypad.isPressed(SELECT_BUTTON)){
			//SELECT
		}else if(keypad.isPressed(DOWN_BUTTON)){
			//DOWN
		}else if(keypad.isPressed(EIGHT_BUTTON)){
			lv_textarea_add_text(textArea,"8");
		}else if(keypad.isPressed(FIVE_BUTTON)){
			lv_textarea_add_text(textArea,"5");
		}else if(keypad.isPressed(TWO_BUTTON)){
			lv_textarea_add_text(textArea,"2");
		}else if(keypad.isPressed(DOT_SIGN_BUTTON)){
			lv_textarea_add_text(textArea,".");
		}else if(keypad.isPressed(RIGHT_PARATHESIS_BUTTON)){
			lv_textarea_add_text(textArea,")");
		}else if(keypad.isPressed(RIGHT_BUTTON)){
			lv_textarea_cursor_right(textArea);
		}else if(keypad.isPressed(GRAPH_BUTTON)){
			textArea = functionTextArea;
			lv_tabview_set_act(tabview, 1, LV_ANIM_OFF);
		}else if(keypad.isPressed(NINE_BUTTON)){
			lv_textarea_add_text(textArea,"9");
		}else if(keypad.isPressed(SIX_BUTTON)){
			lv_textarea_add_text(textArea,"6");
		}else if(keypad.isPressed(THREE_BUTTON)){
			lv_textarea_add_text(textArea,"3");
		}else if(keypad.isPressed(MINUS_SIGN_BUTTON)){
			lv_textarea_add_text(textArea,"-");
		}else if(keypad.isPressed(CLEAR_BUTTON)){
			lv_textarea_set_text(textArea,"");
		}else if(keypad.isPressed(DELETE_BUTTON)){
			lv_textarea_del_char(textArea);
		}else if(keypad.isPressed(DIVIDE_BUTTON)){
			lv_textarea_add_text(textArea,"/");
		}else if(keypad.isPressed(MULTIPLY_BUTTON)){
			lv_textarea_add_text(textArea,"*");
		}else if(keypad.isPressed(SUBTRACT_BUTTON)){
			lv_textarea_add_text(textArea,"-");
		}else if(keypad.isPressed(ADD_BUTTON)){
			lv_textarea_add_text(textArea,"+");
		}else if(keypad.isPressed(ENTER_BUTTON)){
			if(strcmp(lv_textarea_get_text(textArea),"") != 0){
				lv_event_send(textArea,LV_EVENT_READY,NULL);
			}
		}
	}
	#endif
}
void Calculator::main_screen_driver(lv_obj_t* parent, bool first_screen)
{
    static Solve solution;
    total = 0;
    /*Create a keyboard*/
    kb = lv_keyboard_create(parent);
    lv_obj_set_size(kb, LV_HOR_RES, LV_VER_RES / 3);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_ALL, kb);

    /*Initial Input text area*/
    //lv_obj_t* active_ta = lv_active_ta(parent);
    lv_obj_t* active_ta = lv_textarea_create(parent);
	textArea = active_ta;
    areas[total] = active_ta;
    total++;
    lv_textarea_set_one_line(active_ta, true);
    lv_obj_set_width(active_ta, 320);
    lv_obj_align(active_ta, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(active_ta, active_ta_event_handler, LV_EVENT_ALL, &solution);
    lv_obj_add_state(active_ta, LV_STATE_FOCUSED);

    lv_keyboard_set_textarea(kb, active_ta); /*Focus it on one of the text areas to start*/

	if(first_screen)
	{
		/*Create a button to toggle the keyboard*/
		toggle_kb_btn = lv_btn_create(lv_scr_act());
		lv_obj_add_flag(toggle_kb_btn,LV_OBJ_FLAG_CHECKABLE);
		lv_obj_align(toggle_kb_btn, LV_ALIGN_TOP_RIGHT, 0, 0);
		lv_color_t grey = lv_palette_main(LV_PALETTE_GREY);
		lv_obj_set_style_bg_color(toggle_kb_btn, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
		lv_obj_set_size(toggle_kb_btn, 18, 18);
		lv_obj_t* kb_img = lv_img_create(toggle_kb_btn);
		lv_img_set_src(kb_img, LV_SYMBOL_KEYBOARD);
		lv_obj_align_to(kb_img, NULL, LV_ALIGN_CENTER, 0, 0);
		lv_obj_add_event_cb(toggle_kb_btn, Calculator::toggle_kb_event_handler, LV_EVENT_ALL, toggle_kb_btn);
		/**/

		/*Create a button to clear the screen manually*/
		clear_scr_btn = lv_btn_create(lv_scr_act());
		lv_obj_align(clear_scr_btn, LV_ALIGN_TOP_LEFT, 0, 0);
		lv_obj_set_style_bg_color(clear_scr_btn, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
		lv_obj_set_size(clear_scr_btn, 18, 18);
		lv_obj_t* clear_scr_img = lv_img_create(clear_scr_btn);
		lv_img_set_src(clear_scr_img, LV_SYMBOL_CLOSE);
		lv_obj_align_to(clear_scr_img, NULL, LV_ALIGN_CENTER, 0, 0);
		lv_obj_add_event_cb(clear_scr_btn, Calculator::clear_scr_btn_event_handler, LV_EVENT_ALL, parent);
		/**/
	}

    /*Put kb in view*/
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_scroll_by(parent, 0, 25, LV_ANIM_OFF);

}

/*Callback functions*/
static void Calculator::active_ta_event_handler(lv_event_t* e)
{   
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* ta = lv_event_get_target(e);
    lv_obj_t* parent = lv_obj_get_parent(ta);
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED)
    {
        /*Focus on the clicked text area*/
        if (kb != NULL) lv_keyboard_set_textarea(kb, ta);
		lv_obj_scroll_to_view(ta, LV_ANIM_OFF);
		lv_obj_scroll_by(parent, 0, 15, LV_ANIM_OFF);
    }
    else if (code == LV_EVENT_READY)
    {
        if(total > 60)
        {
        	lv_obj_clean(parent);
            Calculator::main_screen_driver(parent, false);
            return;
        }
        LV_LOG_USER("Ready, current text: %s", lv_textarea_get_text(ta));

        auto solution = static_cast<Solve*>(e->user_data);
        std::string func_expression = std::string(lv_textarea_get_text(e->target));
        std::string output = solution->call_giac(func_expression);
        std::cout << output;

		/*Reading Time Info For Logging.*/
		time_t rawtime;
  		struct tm * timeinfo;
  		char current_time [80];
  		time (&rawtime);
  		timeinfo = localtime (&rawtime);
  		strftime (current_time,80,"%x-%X",timeinfo);
  		puts (current_time);
		std::string str(current_time);
		//std::cout << current_time;
		/**/

		//playing with json
		// read a JSON file
		//Expecting a file, implement try, catch.
		std::ifstream i("history.json");
		nlohmann::json j;

		try
		{
			i >> j;

			/*Check which entry number we are adding.*/
			//TODO: Expetcting Int, implement try, catch.
			int json_length = j["length"];

			if(json_length <= 100)
			{
				//Append the new entry and increment the counter
				j["length"] = json_length + 1;
				nlohmann::json k =
				{
					{"Input ID", current_time},
					{"user","guest"},
					{"input", func_expression},
					{"output", output}
				};/**/

				j["entries"].push_back(k);

				//Write the new json contents to the file
				std::ofstream o("history.json");
				o << std::setw(4) << j << std::endl;
				o.close();
			}
			else
			{
				//Either history.json was empty or non-existant. Create the base file.
				std::cout << "There was an error related to history.json" << std::endl;
				nlohmann::json base = nlohmann::json::object();
				base["entries"] = {};
				base["length"] = 0;
				std::ofstream o("history.json");
				o << std::setw(4) << base << std::endl;
			}
			/**/
		}
		catch(...)
		{
			//Either history.json was empty or non-existant. Create the base file.
			std::cout << "There was an error related to history.json" << std::endl;
			nlohmann::json base = nlohmann::json::object();
			base["entries"] = {};
			base["length"] = 0;
			std::ofstream o("history.json");
			o << std::setw(4) << base << std::endl;
		}

        const char *copy_input = lv_textarea_get_text(ta);
        
        /*Create the new text areas*/
        Calculator::lv_input_history_ta(parent, copy_input, ta);
	    Calculator::lv_result_ta(parent, output, ta);
        lv_obj_align(ta, LV_ALIGN_BOTTOM_MID, 0, ( 35 * total) + 35);
        lv_textarea_set_text(ta, "");
        lv_obj_scroll_to_view(ta, LV_ANIM_OFF);

        /*Put kb in view*/
        lv_obj_align_to(kb, parent, LV_ALIGN_BOTTOM_MID, 0, 0);
		lv_obj_scroll_to_view(kb, LV_ANIM_OFF);
		if(!lv_obj_has_flag(kb, LV_OBJ_FLAG_HIDDEN))
		{
			lv_obj_set_y(ta, lv_obj_get_y_aligned(ta) - 80);
		}
        lv_obj_scroll_by(parent, 0, 15, LV_ANIM_OFF);
    }

}

static void Calculator::input_history_ta_event_handler(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* ta = lv_event_get_target(e);
	lv_obj_t* active_ta = static_cast<lv_obj_t*>(lv_event_get_user_data(e));

	if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED)
	{
		const char *copy_input = lv_textarea_get_text(ta);
		lv_textarea_set_text(active_ta, copy_input);
	}	
}


static void Calculator::kb_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* target = lv_event_get_target(e);
    if (code == LV_EVENT_CANCEL)
    {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

}

static void Calculator::clear_scr_btn_event_handler(lv_event_t* e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t* parent = static_cast<lv_obj_t*>(lv_event_get_user_data(e));
	if (code == LV_EVENT_CLICKED)
	{
		lv_obj_clean(parent);
        Calculator::main_screen_driver(parent, false);
        return;
	}
	lv_obj_move_foreground(toggle_kb_btn);
}


static void Calculator::toggle_kb_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        if (lv_obj_has_flag(kb, LV_OBJ_FLAG_HIDDEN))
        {
            lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
            for (int i = 0; i < total; i++)
            {
                lv_obj_set_y(areas[i], lv_obj_get_y_aligned(areas[i]) - 80);
            }
        }
        else
        {
            lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
            for (int i = 0; i < total; i++)
            {
                lv_obj_set_y(areas[i], lv_obj_get_y_aligned(areas[i]) + 80);
            }
        }
    }
    lv_obj_move_foreground(toggle_kb_btn);
}


/*Graphical widget functions*/
lv_obj_t* Calculator::lv_input_history_ta(lv_obj_t* parent, std::string input, lv_obj_t* active_ta)
{
    lv_obj_t* ta = lv_textarea_create(parent);
    areas[total] = ta;
    total++;
    lv_textarea_set_one_line(ta, true);
    lv_obj_set_width(ta, 320);
    lv_obj_align(ta, LV_ALIGN_BOTTOM_MID, 0, ( 35 * total));
    // Todo create dedicated eventhandler.
    lv_obj_add_state(ta, LV_STATE_DEFAULT);
    lv_obj_scroll_by(parent, 0, 25, LV_ANIM_OFF);
    lv_textarea_set_text(ta, input.c_str());
    if (!lv_obj_has_flag(kb, LV_OBJ_FLAG_HIDDEN))
    {
        lv_obj_set_y(ta, lv_obj_get_y_aligned(ta) - 80);
    }
	lv_obj_add_event_cb(ta, input_history_ta_event_handler, LV_EVENT_ALL, active_ta);
    return ta;

}

lv_obj_t* Calculator::lv_result_ta(lv_obj_t* parent, std::string output, lv_obj_t* active_ta)
{
    lv_obj_t* ta = lv_textarea_create(parent);
    areas[total] = ta;
    total++;
    lv_textarea_set_one_line(ta, true);
    lv_obj_set_width(ta, 320);
    lv_obj_align(ta, LV_ALIGN_BOTTOM_MID, 0, ( 35 * total));
    // Todo create dedicated eventhandler.
    lv_obj_add_state(ta, LV_STATE_DEFAULT); /*To be sure the cursor is visible*/
    lv_obj_set_style_text_align(ta, LV_TEXT_ALIGN_RIGHT, 0);
    lv_textarea_add_text(ta, output.c_str());
    if (!lv_obj_has_flag(kb, LV_OBJ_FLAG_HIDDEN))
    {
        lv_obj_set_y(ta, lv_obj_get_y_aligned(ta) - 80);
    }
	lv_obj_add_event_cb(ta, input_history_ta_event_handler, LV_EVENT_ALL, active_ta);
    return ta;
}
void Calculator::storeFunctionTA(lv_obj_t* ta){
    functionTextArea = ta;
}
void Calculator::storeWifiTA(lv_obj_t* ta){
    textArea = ta;
}